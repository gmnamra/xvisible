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

#if __MACH__
	#include <Carbon/Carbon.h>
	#include <QuickTime/QuickTime.h>
	#include	<ASC_DCAM_DEV/ASC_DCAM_API.h>//this is located inside of the "ASC_DCAM_DEV.framework" in the headers folder..
	#include <stdio.h>
	#include <stdlib.h>
#else //we assume this is for OSX Carbon..
	#include <Carbon.h>
	#include <QuickTime.h>
	#include	"ASC_DCAM_API Carbon.h"
#endif




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

static 			OSStatus 			myGetDefaultLocation(short *theVRefNum, long *theParID)
{
//this locates the Folder that the Application was launched from 
//and returns the location that it's (vrefnum and parId) are from.
	OSStatus theErr = noErr;
	ProcessSerialNumber thePSN;
	ProcessInfoRec theInfo;
	FSSpec theSpec;
	
	thePSN.highLongOfPSN = 0;
	thePSN.lowLongOfPSN = kCurrentProcess;
	
	theInfo.processInfoLength = sizeof(theInfo);
	theInfo.processName = NULL;
	theInfo.processAppSpec = &theSpec;
	
	#if __MACH__
		//for OSX packages, we recurse back through them..
		/* Find the application FSSpec */
		theErr = GetProcessInformation(&thePSN, &theInfo);

		//recursively move out through the directories of the applicaiton bundle..
		// Find the "Contents" (the parent of the "MacOS" directory)
		if (theErr == noErr)
			theErr = FSMakeFSSpec(theSpec.vRefNum, theSpec.parID, "\p", &theSpec);
		
		// Find the parent Bundle folder (the parent of the "Contents" directory)
		if (theErr == noErr)
			theErr = FSMakeFSSpec(theSpec.vRefNum, theSpec.parID, "\p", &theSpec);

		// Find the parent of the Bundle folder
		if (theErr == noErr)
			theErr = FSMakeFSSpec(theSpec.vRefNum, theSpec.parID, "\p", &theSpec);
	
	#else
	//we are in Os 9, therefore we are done..
	#endif
	if (theErr == noErr)
	{
		/* Return the folder which contains the application package */
		*theVRefNum = theSpec.vRefNum;
		*theParID = theSpec.parID;
	}
	return theErr;
}

int main (void)
{
	OSStatus				err;
	dcamCameraList	theCameraList;
	void *					theCameraPtr;
	FSSpec					theSpec;
	short 					theVRefNum;
	long 						theParID;
	//get list of cameras attached..

	err = myGetDefaultLocation(&theVRefNum,&theParID);
	if (err!= 0)
		return 0;//bail..
	err = FSMakeFSSpec(theVRefNum, theParID, "\pATiffFile.tif", &theSpec);
	if (err!= -43)//a -43 indicates the file does not exist at this location,
		return 0;//if it's anything else, Quit..
#if __MACH__
	err = ASCDCAMGetListOfCameras(&theCameraList);
#else
	//carbon call to initate library..
	err = EnterASCDCAMCarbonLib();
	if (err == 0)
	{
		err = ASCDCAMGetListOfCameras(&theCameraList);
		if (err!= 0)//an error occured..
			ExitASCDCAMCarbonLib();//clean it up..
	}
#endif
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
		{//start the isoch stream
			err = ASCDCAMAllocateReleaseIsochChannel(theCameraPtr,true);
			if (err == 0)
			{	//Start the camera transmitting video..
				err = ASCDCAMStartStopVideo(theCameraPtr,true);
				if (err == 0)
				{//camera is running..
					/* NOTE:
						mainy IIDC cameras start up in an "auto mode" 
						and then balance colors, saturation,  brightness etc.
						So we give the camera 2 seconds to "settle down" before grabbing a snap shot
					*/
					#if __MACH__
					CFRunLoopRunInMode( kCFRunLoopDefaultMode,2, false );//wait 2 seconds..
					#else
					{
						UInt32		numTicks = 120;//wait 2 seconds
						UInt32		finalTicks =0;
						Delay(numTicks, &finalTicks);
					}
					#endif
					//Grab a frame..
					err = ASCDCAMSaveFrameToDisk(theCameraPtr,
										theSpec,
										kQTFileTypeTIFF,//tiff
										kASC_ARGB,//ARGB only..
										(UInt32)1000);//wait up to 1 second..

					ASCDCAMStartStopVideo(theCameraPtr,false);//stop the camera
				}
				//release the Isoch Resources..
				ASCDCAMAllocateReleaseIsochChannel(theCameraPtr,false);
			}
		}
		//all done so release the camera and System Resources
		ASCDCAMCloseACamera(theCameraPtr);
	}
	#if __MACH__
	#else
	ExitASCDCAMCarbonLib();//clean it up the CFM calls..
	#endif
	return 0;
}
