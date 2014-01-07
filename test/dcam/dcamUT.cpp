/*
  COPYRIGHT � 2003 Aupperle Services and Contracting.  ALL RIGHTS RESERVED.

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

#include <rc_timestamp.h>
#include <rc_sparsehist.h>
#include <rc_capture.hpp>
#include <rc_dcam.h>

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
	Rect				GWorldBounds;
	void *				theCameraPtr;//camera object
	WindowRef			thePreviewWindow;
	GWorldPtr			theGWorld;
} yourProcessFrameCallBack, *yourProcessFrameCallBackPtr;

static void dump_capabilities ( ostream& os, const CameraCapabilitiesStruct& cap );


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

OSStatus useLastAvailableFormatMode(void * theCameraPtr)
{
	OSStatus						err;
	short							theMode;
	CameraCapabilitiesStruct		theProperties;
	Boolean						done;

	err = ASCDCAMGetCameraProperties(theCameraPtr, &theProperties);
	if (err!=0)
		return err;
	done = false;
	theMode = 7;

	if (theProperties.format7 == true)
	{
		err = ASCDCAMSet7MR(theCameraPtr,1, 0, 0, 0, 0, kMONO8_ID, 0);
		return err;
	}

    if (theProperties.format2 == true)//supports format 2
	{
		//pick a Mode
		while ((done == false) && (theMode>0))
		{
			if (theProperties.format2Modes[theMode] ==true)
				done = true;
			else
				theMode--;
		}
		if (done==true)//as fast as possible
			err = ASCDCAMSetFMR(theCameraPtr,2,theMode,(double)60.00);
		else
			err = 0xFFFFFFFF;
		return err;
	}

    if (theProperties.format1 == true)//supports format 1
	{
		//pick a Mode
		while ((done == false) && (theMode>0))
		{
			if (theProperties.format1Modes[theMode] ==true)
				done = true;
			else
				theMode--;
		}
		if (done==true)//60.00 means as fast as possible
			err = ASCDCAMSetFMR(theCameraPtr,1,theMode,(double)60.00);
		else
			err = 0xFFFFFFFF;
		return err;
	}

	if (theProperties.format0 == true)//supports format 0
	{
		//pick a Mode
		while ((done == false) && (theMode>0))
		{
			if (theProperties.format0Modes[theMode] ==true)
				done = true;
			else
				theMode--;
		}
		if (done==true)//60.00 means as fast as possible
			err = ASCDCAMSetFMR(theCameraPtr,0,theMode,(double)60.00);
		else
			err = 0xFFFFFFFF;
		return err;
	}

	//Only format 7 is supported and we don't want to do that one for this example..
	return (OSStatus) 0xFFFFFFFF;
}

static rcSparseHistogram sHistogram;
static rcTimestamp sLastTime(0.0);
static uint32    sCount = 0;
static uint32 sHistogramRange = 0;

static void yourDisplayCallback(ASC_CallbackStruct theCallBackData)
{
    UnsignedWide microTickCount = {0,0};
    Microseconds( &microTickCount );

    int64 CPUTicks =
        (((int64)microTickCount.hi) << 32 )|
        ((int64)microTickCount.lo);

    rcTimestamp cur(CPUTicks);

	CGrafPtr					theWindowPort;
	OSStatus					err = noErr;
	yourProcessFrameCallBackPtr	theCallbackDataPtr;
	PixMapHandle 				hPixMap;
	Rect						theWinRect;
	Ptr						baseAddress;
	UInt32					pixRowBytes;

	if ((theCallBackData.userDataPtr != NULL) && (theCallBackData.ImageBufferPtr != NULL))
	{
        if ( sLastTime.secs() > 0.0 ) {
            ++sCount;
            double frameInt = (cur - sLastTime).secs()*1000;
            frameInt = theCallBackData.frameDurationMS;
            //fprintf( stderr, "%.2f ms\n", frameInt );
            sHistogram.add( uint16( frameInt ) );
            uint32 min, max;
            sHistogram.range(min, max);
            uint32 r = max-min;

            if (r > sHistogramRange) {
                sHistogramRange = r;
                fprintf( stderr, "Interval range [%i-%i]\n",
                         min, max );
            }
        }
        sLastTime = cur;

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
				else//treat this as a "normal" image..
					err = ASCDCAMConvertRawFrameToARGB32(theCallbackDataPtr->theCameraPtr,
                                                         theCallBackData.ImageBufferPtr,
                                                         theCallbackDataPtr->theWidth,
                                                         theCallbackDataPtr->theHeight,
                                                         theCallbackDataPtr->fourCCType,
                                                         theCallbackDataPtr->bitsPerPixel,
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
    fprintf( stderr, "Format: %u mode: %u fps: %f rect: [%u,%u,%u,%u] pixType: %i depth: %u\n",
             uint32(format), uint32(mode), frameRateDbl, uint32(left),
             uint32(top), uint32(yPFCBPtr->theWidth), uint32(yPFCBPtr->theHeight),
             (int)yPFCBPtr->fourCCType,
             uint32(yPFCBPtr->bitsPerPixel) );
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
	rcCameraMapper dcamMapper;
	dcamMapper.Fill_dcam_info ();
	dcamMapper.dump (std::cout);

	OSStatus										err;
	dcamCameraList							theCameraList;
	void *									theCameraPtr = 0;
    CameraCapabilitiesStruct theProperties;

	yourProcessFrameCallBackPtr	yPFCBPtr = NULL;
	//get list of cameras attached..

	err = ASCDCAMGetListOfCameras(&theCameraList);
	if (err!= 0)//an error occured..
		return 0;//bail..
	if (theCameraList.numCamerasFound == 0)//no cameras.
		return 0;//bail..

    cerr << theCameraList.numCamerasFound << " cameras connected:" << endl;
    // Display info for all cameras
     for ( uint32 i = 0; i < theCameraList.numCamerasFound; ++i ) {
        dcamCameraListStruct camera = theCameraList.theCameras[i];
        cerr << camera;
        void *theCameraPtr = ASCDCAMOpenACamera(camera.uniqueID);
        if ( theCameraPtr ) {
            OSStatus err = ASCDCAMGetCameraProperties(theCameraPtr, &theProperties);
            // Display camera properties
            dump_capabilities (cerr, theProperties);
            double maxRate = rfGetCameraMaxFrameRate( theCameraPtr );
            cerr << "MaxFrameRate: " << maxRate << endl;
            ASCDCAMCloseACamera(theCameraPtr);
            if (err!=0) {
                cerr << "ASCDCAMGetCameraProperties error " << err << endl;
                return err;
            }
        }

    }

	if (err!= 0)//an error occured..
		return 0;//bail..
    // open the first possible camera
    for ( uint32 i = 0; i < theCameraList.numCamerasFound; ++i ) {
        dcamCameraListStruct camera = theCameraList.theCameras[i];
        if ( camera.isInUse ) {
            cerr << "Error: camera " <<  camera.uniqueID << " in use already"
                 << endl;
        } else {
            theCameraPtr = ASCDCAMOpenACamera(camera.uniqueID);
            if ( theCameraPtr != NULL )
                break;
        }
    }

	if (theCameraPtr!=NULL)//we have it exclusively now..
	{
			//	err = useLastAvailableFormatMode(theCameraPtr);
		
		OSStatus err = ASCDCAMSet7MR(theCameraPtr,1, 0, 0, 0, 0, kMONO8_ID, 0);
		std::cerr << " Attempt to use Format 7 " << (int) err << std::endl;
		
		if ( err == 0 )
		{
			UInt32 theWidth;
			UInt32 format;
			UInt32 theHeight;
			UInt32 bitsPerPixel;
			OSType fourCCType;
			UInt32 left;
			UInt32 top;
			double frameRate;
			UInt32 mode;
				// Check what the actual values are, especially frame rate
			err = ASCDCAMGetFMRLTWHCB(theCameraPtr,
									  &format,
									  &mode,
									  &frameRate,
									  &left,
									  &top,
									  &theWidth,
									  &theHeight,
									  &fourCCType,
									  &bitsPerPixel);
			std::cerr << " Attempt to check after setting Format 7 " << (int) err << std::endl;
			std::cerr << " mode: " << (int) mode << " format: " << format << std::endl;
			std::cerr << " width: " << (int) theWidth << " height: " << theHeight << std::endl;		
			
		}

		
		if (err == 0)
		{
            // Display property control details
            vector<rcCameraProperty> props;
            int count = rfGetCameraProperties( theCameraPtr, props );
            if ( count > 0 ) {
                cerr << "Control details:" << endl;
                for ( uint32 i = 0; i < props.size(); ++i ) {
                    const rcCameraProperty& p = props[i];
                    if ( p.active() )
                        cerr << p << endl;
                }
            }
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
                        // Display frame interval histogram
                        const rcSparseHistogram::sparseArray& histMap = sHistogram.getArray();
                        rcSparseHistogram::sparseArray::const_iterator start = histMap.begin();
                        rcSparseHistogram::sparseArray::const_iterator end = histMap.end();

                        uint32 min = 0, max = 0;
                        sHistogram.range(min, max);
                        fprintf( stderr, "\nInterval range [%i-%i]\n",
                                 min, max );
                        for ( ; start != end; start++)
                            fprintf(stderr, "Interval %d ms Count %d\n", (*start).first,
                                    (*start).second);
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
	} else {
        fprintf( stderr, "Error: could not open camera\n" );
    }

	return 0;
}



static void dump_capabilities ( ostream& os, const CameraCapabilitiesStruct& cap )
{
	
	os << "Camera name: " << (char*)&cap.deviceName << " UID: " <<  cap.uniqueID
	<< endl;
	dcam_formats df;
	
	os << "Formats:" << endl;
	
	if ( cap.format0 ) {
		for ( int i = 0; i < 7; ++i )
			if ( cap.format0Modes[i] )
				os << "Format 0: " << " Mode : " << i << std::endl;
	}
	if ( cap.format1 ) {
		for ( int i = 0; i < 7; ++i )
			if ( cap.format1Modes[i] )
				os << "Format 1: " << " Mode : " << i << std::endl;					
	}
	if ( cap.format2 ) {
		for ( int i = 0; i < 7; ++i )
			if ( cap.format2Modes[i] )
				os << "Format 2: " << " Mode : " << i << std::endl;					
	}
	
	if ( cap.format7 ) {
		os << "Format 7:" << endl;
			// If we have format 7. This is camera specific. So we need it from some place @todo
		int num = sizeof (cap.format7Modes) / sizeof(bool);
		for (int i = 0; i < num; i++)
			if ( cap.format7Modes[i] )
				os << "Format 2: " << " Mode : " << i << std::endl;										
		
		os << endl;
	}
	
		
	os << "Controls:" << endl;
	if ( cap.hasBright )
		os << "Brightness ";
	if ( cap.hasExpos )
		os << "Exposure ";
	if ( cap.hasSharp )
		os << "Sharpness ";
	if ( cap.hasWhiteB )
		os << "WhiteBalance ";
	if ( cap.hasHue )
		os << "Hue ";
	if ( cap.hasSat )
		os << "Saturation ";
	if ( cap.hasGain )
		os << "Gain ";
	if ( cap.hasGamma )
		os << "Gamma ";
	if ( cap.hasShutter )
		os << "Shutter ";
	if ( cap.hasIris )
		os << "Iris ";
	if ( cap.hasFocus )
		os << "Focus ";
	if ( cap.hasTemp )
		os << "Tempr ";
	if ( cap.hasTrigger )
		os << "Trigger ";
	if ( cap.hasZoom )
		os << "Zoom ";
	if ( cap.hasPan )
		os << "Pan ";
	if ( cap.hasTilt )
		os << "Tilt ";
	if ( cap.hasOpticalFilter )
		os << "Filter ";
	
	os << endl;
	
}

