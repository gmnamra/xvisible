/*
 COPYRIGHT © 2003 Aupperle Services and Contracting.  ALL RIGHTS RESERVED.
																												
 Copyright Notice and Disclaimer of Liability:															

 Aupperle Services and Contracting is henceforth referred to as ASC.						
 
 Purchaser henceforth refers to the original purchaser(s) of the equipment, and/or any legitimate user(s).

 ASC hereby explicitly prohibits any form of reproduction (with the express strict exception for backup
 and archival purposes, which are allowed as stipulated within the License Agreement for ASC
 Software), modification, and/or distribution of this software and/or its associated documentation unless
 explicitly specified in a written agreement signed by both parties.
																												
 To the extent permitted by law, ASC disclaims all other warranties or conditions of any kind, either
 express or implied, including but not limited to all warranties or conditions of merchantability and
 fitness for a particular purpose and those arising by statute or otherwise in law or from a course of	
 dealing or usage of trade. Other written or oral statements by ASC, its representatives, or others do
 not constitute warranties or conditions of ASC.

 ASC makes no guarantees or representations, including but not limited to: the use of, or the result(s)
 of the use of: the software and associated documentation in terms of correctness, accuracy, reliability,
 being current, or otherwise. The Purchaser hereby agree to rely on the software, associated hardware and
 documentation and results stemming from the use thereof solely at their own risk.
																												
 ASC is hereby absolved of any and all liability to the Purchaser, and/or a third party, financial or
 otherwise, arising from any subsequent loss, direct and indirect, and damage, direct and indirect,
 resulting from intended and/or unintended usage of its software, product(s) and documentation, as well
 as additional service(s) rendered by ASC, such as technical support, unless otherwise explicitly
 specified in a written agreement signed by both parties. Under no circumstances shall the terms and	
 conditions of such an agreement apply retroactively.																																						
*/

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <stdio.h>
#include <stdlib.h>

#include	<ASC_DCAM_DEV/ASC_DCAM_API.h>//this is located inside of the "ASC_DCAM_DEV.framework" in the headers folder..

//our callback structure..
typedef struct
{
	Boolean				shouldDisplay;
	Boolean				isBayerData;
	OSStatus			theErr;//keep track of errors
	UInt32				theWidth;
	UInt32				theHeight;
	UInt32				bitsPerPixel;
	OSType				theDisplayFormat;
	OSType				fourCCType;
	Rect					GWorldBounds;
	void *				theCameraPtr;//camera object
	WindowRef			thePreviewWindow;
	GWorldPtr			theGWorld;	
} yourProcessFrameCallBack, *yourProcessFrameCallBackPtr;


OSStatus useFirstAvailableFormatMode(void * theCameraPtr)
{
	OSStatus						err;
	short							theMode;
	CameraCapabilitiesStruct		theProperties;
	Boolean						done;
	
	err = ASCDCAMGetCameraProperties(theCameraPtr, &theProperties);
	if (err!=0)
		return err;

	done = false;
	theMode = 0;
	if (theProperties.format0 == true)//supports format 0
	{
		//pick a Mode
		while ((done == false) && (theMode<8))
		{
			if (theProperties.format0Modes[theMode] ==true)
				done = true;
			else
				theMode++;
		}
		if (done==true)//60.00 means as fast as possible
			err = ASCDCAMSetFMR(theCameraPtr,0,theMode,(double)60.00);
		else
			err = 0xFFFFFFFF;
		return err;
	}
	
	if (theProperties.format1 == true)//supports format 1
	{
		//pick a Mode
		while ((done == false) && (theMode<8))
		{
			if (theProperties.format1Modes[theMode] ==true)
				done = true;
			else
				theMode++;
		}
		if (done==true)//60.00 means as fast as possible
			err = ASCDCAMSetFMR(theCameraPtr,1,theMode,(double)60.00);
		else
			err = 0xFFFFFFFF;
		return err;
	}
	if (theProperties.format2 == true)//supports format 2
	{
		//pick a Mode
		while ((done == false) && (theMode<8))
		{
			if (theProperties.format2Modes[theMode] ==true)
				done = true;
			else
				theMode++;
		}
		if (done==true)//as fast as possible
			err = ASCDCAMSetFMR(theCameraPtr,2,theMode,(double)60.00);
		else
			err = 0xFFFFFFFF;
		return err;
	}
	//Only format 7 is supported and we don't want to do that one for this example..
	return (OSStatus) 0xFFFFFFFF;
}


static void yourDisplayCallback(ASC_CallbackStruct theCallBackData)
{
	CGrafPtr					theWindowPort;
	OSStatus					err = noErr;
	yourProcessFrameCallBackPtr	theCallbackDataPtr;
	PixMapHandle 				hPixMap;
	Rect						theWinRect;
	Ptr						baseAddress;
	UInt32					pixRowBytes; 
	
	if ((theCallBackData.userDataPtr != NULL) && (theCallBackData.ImageBufferPtr != NULL))
	{
		theCallbackDataPtr = (yourProcessFrameCallBackPtr) theCallBackData.userDataPtr;
		if (theCallbackDataPtr->shouldDisplay == true)
		{
			hPixMap = GetGWorldPixMap(theCallbackDataPtr->theGWorld);
			if (LockPixels(hPixMap) == true)
			{
				baseAddress = GetPixBaseAddr(hPixMap);
				pixRowBytes = (UInt32) GetPixRowBytes(hPixMap);
				
				if (theCallbackDataPtr->isBayerData == true)//we need to trest this as a Bayer frame..
				{
						err = ASCDCAMConvertRawFrameToARGB32(  theCallbackDataPtr->theCameraPtr,  theCallBackData.ImageBufferPtr, 
																		theCallbackDataPtr->theWidth, theCallbackDataPtr->theHeight,
																		theCallbackDataPtr->theDisplayFormat,theCallbackDataPtr->bitsPerPixel,
																		baseAddress,pixRowBytes);
				}
				else//treat this as a "normla" image..
					err = ASCDCAMConvertRawFrameToARGB32(theCallbackDataPtr->theCameraPtr, theCallBackData.ImageBufferPtr, 
																		theCallbackDataPtr->theWidth, theCallbackDataPtr->theHeight,
																		theCallbackDataPtr->fourCCType,theCallbackDataPtr->bitsPerPixel,
																		baseAddress,pixRowBytes);
				if (err == 0)
				{
					theWindowPort = GetWindowPort(theCallbackDataPtr->thePreviewWindow);
					GetWindowPortBounds(theCallbackDataPtr->thePreviewWindow, &theWinRect);//in case i'ts differnt..
	 				SetPortWindowPort(theCallbackDataPtr->thePreviewWindow);
	 				if (LockPortBits(theWindowPort) == noErr)
	 				{
						CopyBits(((BitMapPtr)*hPixMap), GetPortBitMapForCopyBits(theWindowPort), &theCallbackDataPtr->GWorldBounds,&theWinRect, srcCopy, NULL);
						UnlockPixels(hPixMap);						
						if (QDIsPortBuffered(theWindowPort))//this port is buffered..
							QDFlushPortBuffer(theWindowPort,NULL);//update the window..
						UnlockPortBits(theWindowPort);
					}
					else//failed LockPortBits
					{
						UnlockPixels(hPixMap);
						err = 0xFFFFFFFF;
					}
				}
				else//failed..
					UnlockPixels(hPixMap);
			}
			else//failed LockPixels
				err = 0xFFFFFFFF;
			theCallbackDataPtr->theErr = err;
			if (theCallbackDataPtr->shouldDisplay == true)
				theCallbackDataPtr->shouldDisplay = (err == 0);
		}
	}
}

OSStatus	SetUpDisplayCallback(void *theCameraPtr, yourProcessFrameCallBackPtr yPFCBPtr)
{
	Rect			windowRect;
	Rect    	deviceRect;
	
	OSStatus	err;
	UInt32 		format;
	UInt32 		mode;
	double		frameRateDbl;
	UInt32 		left;
	UInt32 		top;
	
	//initalize everything..
	yPFCBPtr->theCameraPtr = theCameraPtr;
	yPFCBPtr->thePreviewWindow = NULL;
	yPFCBPtr->theGWorld = NULL;
	yPFCBPtr->shouldDisplay = false;
	yPFCBPtr->theErr = 0;
	err = ASCDCAMGetFMRLTWHCB(yPFCBPtr->theCameraPtr,
								&format,
								&mode,
								&frameRateDbl,
								&left,
								&top,
								&yPFCBPtr->theWidth,
								&yPFCBPtr->theHeight,
								&yPFCBPtr->fourCCType,
								&yPFCBPtr->bitsPerPixel);
	if (err != 0)
		return err;//bail
	//if your FW camera does bayer video for this format and mode, set isBayerData to true
	yPFCBPtr->isBayerData = false;
	//Change this to whatever kLQ_ASCBAYER_RGGB or RGGB pattern the FW Camera uses for the Bayer data..
	if (yPFCBPtr->isBayerData == true)
		yPFCBPtr->theDisplayFormat = kLQ_ASCBAYER_RGGB;
	else
		yPFCBPtr->theDisplayFormat = kASC_ARGB;
	// find the best monitor for the window
	GetBestDeviceRect(NULL, &deviceRect);
	SetRect(&windowRect,0,0,(short) (yPFCBPtr->theWidth),(short) (yPFCBPtr->theHeight));
	SetRect(&yPFCBPtr->GWorldBounds,0,0,(short) (yPFCBPtr->theWidth),(short) (yPFCBPtr->theHeight));
	//create the GWorld Buffer for the ARGB32 frames
	err =  NewGWorld(&(yPFCBPtr->theGWorld),32,&yPFCBPtr->GWorldBounds,NULL,NULL,0);
	if (err == noErr)
	{
		// put the window near the top left of this monitor
		OffsetRect(&windowRect, (short)(deviceRect.left + 12),(short)(deviceRect.top + 48));
		err = CreateNewWindow(kDocumentWindowClass, kWindowCloseBoxAttribute, &windowRect,
													&yPFCBPtr->thePreviewWindow);
		if (err == noErr)
		{
			SetWTitle(yPFCBPtr->thePreviewWindow, "\pCommand-Quit to exit");
			SetPortWindowPort(yPFCBPtr->thePreviewWindow);
			ShowWindow(yPFCBPtr->thePreviewWindow);
		}
	}
	return err;
}

int main (void)
{
	OSStatus										err;
	dcamCameraList							theCameraList;
	void *											theCameraPtr;

	yourProcessFrameCallBackPtr	yPFCBPtr = NULL;
	//get list of cameras attached..
	
	err = ASCDCAMGetListOfCameras(&theCameraList);
	if (err!= 0)//an error occured..
		return 0;//bail..
	if (theCameraList.numCamerasFound == 0)//no cameras.
		return 0;//bail..
	//open the first camera found - we assume it is not in use..

	theCameraPtr = ASCDCAMOpenACamera(theCameraList.theCameras[0].uniqueID);
	if (theCameraPtr!=NULL)//we have it exclusively now..
	{
		err = useFirstAvailableFormatMode(theCameraPtr);
		if (err == 0)
		{
			yPFCBPtr = (yourProcessFrameCallBackPtr) NewPtrClear(sizeof(yourProcessFrameCallBack));
			err = MemError();
			if (err == 0)
				err = SetUpDisplayCallback(theCameraPtr,yPFCBPtr);
		}
		if (err == 0)
		{//start the isoch stream
			err = ASCDCAMAllocateReleaseIsochChannel(theCameraPtr,true);
			if (err == 0)
			{//Start the camera transmitting video..
				err = ASCDCAMStartStopVideo(theCameraPtr,true);
				if (err == 0)
				{//camera is running..
					//Assign the Callback..
					err = ASCDCAMAssignCallback(theCameraPtr, yourDisplayCallback,yPFCBPtr);
					if (err==0)
					{
						//display video..
						yPFCBPtr->shouldDisplay = true;
						RunApplicationEventLoop();//loop forever until the user Command-Quits..
						yPFCBPtr->shouldDisplay = false;
						//stop the callback..
						ASCDCAMAssignCallback(theCameraPtr, NULL,NULL);
					}
					//stop the camera
					ASCDCAMStartStopVideo(theCameraPtr,false);
				}
				//release the Isoch System Resources and buffers..
				ASCDCAMAllocateReleaseIsochChannel(theCameraPtr,false);
			}
		}
		//all done so release the camera and System Resources
		ASCDCAMCloseACamera(theCameraPtr);
		if (yPFCBPtr != NULL)//clean up the callback data..
		{
			if (yPFCBPtr->thePreviewWindow != NULL)
				DisposeWindow(yPFCBPtr->thePreviewWindow);
			if (yPFCBPtr->theGWorld != NULL)
				DisposeGWorld(yPFCBPtr->theGWorld);
			DisposePtr((Ptr) yPFCBPtr);
		}
	}
	return 0;
}


