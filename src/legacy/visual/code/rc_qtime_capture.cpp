/*
 *  rc_qtime_capture.cpp
 *  framebuf
 *
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */
#include <QuickTime/QuickTime.h>
#include <Carbon/Carbon.h>
#include <rc_qtime_capture.h>
#include <rc_capture.h>
#include <rc_assemblyfcts.h>
#include <assert.h>
#include <stdlib.h> // Defines exit()
#include <string.h>
#include <rc_imageprocessing.h>
#include <rc_resource_ctrl.h> // Defines rcSharedMemoryUser class
#include <rc_timestamp.h>
#include <rc_sparsehist.h>

#define DEBUGGING

#define BailErr(x) {err = x; if(err != noErr) goto bail;}
#define DisplayError(x,y) fprintf(stderr,"Error %d Msg string ; %s\n",(int)(x),y)
#define DisplayAndBail(x, y) { err = x; if(err != noErr) \
{ DisplayError(err, y); goto bail;} }

#define VDGETVIDEODEFAULTS VDGetVideoDefaults
#define VDSETCONTRAST VDSetContrast
#define VDGETCONTRAST VDGetContrast
#define VDSETBRIGHTNESS VDSetBrightness
#define VDGETBRIGHTNESS VDGetBrightness


unsigned short VDcontrast = 666, VDbrightness = 666, VDdContrast = 666,
VDdBrightness = 666;

VideoDigitizerError VDGETVIDEODEFAULTS(VideoDigitizerComponent,
									   unsigned short*,
									   unsigned short*,
									   unsigned short* brightness,
									   unsigned short*,
									   unsigned short*,
									   unsigned short* contrast,
									   unsigned short*)
{
	if (brightness)
		*brightness = VDdBrightness;
	
	if (contrast)
		*contrast = VDdContrast;
	
	return noErr;
}

VideoDigitizerError VDSETCONTRAST (VideoDigitizerComponent,
								   unsigned short *contrast )
{
	assert(contrast);
	
	VDcontrast = *contrast;
	
	return noErr;
}

VideoDigitizerError VDGETCONTRAST (VideoDigitizerComponent,
								   unsigned short *contrast )
{
	assert(contrast);
	
	*contrast = VDcontrast;
	
	return noErr;
}

VideoDigitizerError VDSETBRIGHTNESS (VideoDigitizerComponent,
									 unsigned short *brightness )
{
	assert(brightness);
	
	VDbrightness = *brightness;
	
	return noErr;
}

VideoDigitizerError VDGETBRIGHTNESS (VideoDigitizerComponent,
									 unsigned short *brightness )
{
	assert(brightness);
	
	*brightness = VDbrightness;
	
	return noErr;
}

#endif

// constants
const EventTime kTimerInterval = kEventDurationSecond/60; // idle timer interval

// mung data struct
typedef struct {
	WindowRef        	pWindow;	// window
	Rect 			bounds;		// bounds rect
	GWorldPtr 		pGWorld;	// offscreen
	SeqGrabComponent 	seqGrab;	// sequence grabber
	VideoDigitizerComponent vDig;
	TimeValue 		lastTime;
#ifdef DEBUGGING
	TimeValue 		lastCaptureTime;
#endif
	double 		timeScale;
	rcTimestamp           curTime;
	long 			frameCount;
	uint32              imgWidth;       // Raw image width, in pixels
	uint32              imgHeight;      // Raw image height
	uint32              rowUpdate;      // Raw image row update, in bytes
	uint32              imgSizeInBytes; // Raw image size
	uint32              stopProcess;
	uint32              decCounter;     // Decimation counter
	bool                initing;
	bool                extractLum;
	rcPixel         pixelDepth;     // Desired pixel depth
	bool                isGray;         // Desired pixel "type"
	rcSharedMemoryUser*   shMem;
	rcAcqInfo             acqInfo;
	rcAcqControl          acqControl;
	Boolean		isGrabbing;
	EventLoopTimerRef	timerRef;
	OSErr			err;
} MungDataRecord, *MungDataPtr;

/* releaseMemory - Release shared memory back to peer process. Since
 * failure is considered a fatal error, the error messages should be
 * kept. The caller is expected to do any further error handling.
 */
bool releaseMemory(rcSharedMemoryUser& shMem)
{
	rcSharedMemError smErr;
	
	switch (smErr = shMem.releaseSharedMemory())
	{
		case rcSharedMemNoError:
			break;
			
		default:
			fprintf(stderr, "Child: error freeing shared data, err: %d\n", smErr);
			perror("Child: ");
			return false;
	}
	return true;
}

/* writeAcqInfo - Write acquisition information structure out to
 * shared memory. This operation should always succeed -- wherever it
 * is used shared memory has already been acquired. Because of this,
 * the error messages that get printed out should be kept. Because
 * different situations want to handle the error differently, all
 * other error handling is left to the caller.
 */
bool writeAcqInfo(rcSharedMemoryUser& shMem, rcAcqInfo& acqInfo, bool relMemory)
{
	rcSharedMemError smErr;
	
	rcVideoSharedMemLayout* sharedBuf = 
    (rcVideoSharedMemLayout*)(shMem.acquireSharedMemory(smErr, false));
	
	if (!sharedBuf)
	{
		fprintf(stderr, "Child: error acquiring shared data. err: %d\n", smErr);
		perror("Child: ");
		return false;
	}
	
	assert(sizeof(acqInfo) == sizeof(sharedBuf->videoState));
	memmove(&(sharedBuf->videoState), &acqInfo, sizeof(acqInfo));
	
	if (relMemory)
		return releaseMemory(shMem);
	
	return true;
}

/* getCtrlInfo - Get acquisition control information from shared
 * memory.  This operation may or may not be expected to succeed based
 * on the passed in value of isBlocking. Since only general system
 * failures are fatal errors only these generate error messages. Since
 * the "grab" may fail without a fatal error occurring, the caller
 * must pass in a reference to a "fatal error" variable that allows
 * this discrimination to be made.
 */
bool getCtrlInfo(rcSharedMemoryUser& shMem, rcAcqControl& acqControl,
				 bool& fatalError, bool isBlocking, bool relMemory)
{
	fatalError = 0;
	
	rcSharedMemError smErr;
	
	rcVideoSharedMemLayout* sharedBuf = 
    (rcVideoSharedMemLayout*)(shMem.acquireSharedMemory(smErr, isBlocking));
	
	if (!sharedBuf)
	{
		if (smErr != rcSharedMemNotAvailable)
		{
			fprintf(stderr, "Child: error acquiring shared data. err: %d\n", smErr);
			perror("Child: ");
			fatalError = 1;
		}
		return false;
	}
	
	assert(sizeof(acqControl) == sizeof(sharedBuf->videoControl));
	memmove(&acqControl, &(sharedBuf->videoControl), sizeof(acqControl));
	
	if (relMemory && !releaseMemory(shMem))
	{
		fatalError = 1;
		return false;
	}
	
	return true;
}

/* processControlRequests - Performs any control requests from coming
 * from peer process, write rcAcqInfo to shared memory and releases
 * shared memory.
 *
 * If doWrite is true, the write of rcAcqInfo to shared memory and the
 * release of shared memory will always happen. If doWrite is false, this
 * only happens if there are control requests to process.
 */
void processControlRequests(MungDataPtr pMungData, bool doWrite)
{
	assert(pMungData);
	assert(pMungData->shMem);
	bool fatalError;
	
	rcAcqControl acqControl;
	if (getCtrlInfo(*(pMungData->shMem), acqControl, fatalError, false, false)) {
		OSErr err;
		
		if (acqControl.acqCtrlUpdateID != pMungData->acqControl.acqCtrlUpdateID) {
			// Work to do!
			
			if (acqControl.doAcquire != pMungData->acqControl.doAcquire) {
				if (acqControl.doAcquire) {
					if (pMungData->initing) {
						fprintf(stderr, "...10!\n");
						
						// ...action
						if ((err = SGStartRecord(pMungData->seqGrab)) == noErr) {
							pMungData->initing = false;
							pMungData->isGrabbing = true;
							pMungData->acqInfo.acqState = rcAcqRunning;
						}
						else
							pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRPAUSE;
					}
					else if ((err = SGPause(pMungData->seqGrab,seqGrabUnpause)) != noErr)
						pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRPAUSE;
				}
				else {
					if ((err = SGPause(pMungData->seqGrab,seqGrabPause)) == noErr) {
						pMungData->isGrabbing = false;
						pMungData->acqInfo.acqState = rcAcqStopped;
					}
					else
						pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRPAUSE;
				}
			} // End of: if (acqControl.doAcquire != pMungData->acqControl.doAcquire)
			
			if (acqControl.resolution != pMungData->acqControl.resolution) {
				uint32 imgByteCnt = pMungData->imgSizeInBytes;
				
				if (pMungData->acqControl.resolution == rcAcqHalfResXY)
					imgByteCnt /= 4;
				
				if (pMungData->extractLum)
					imgByteCnt /= 2;
				
				if (imgByteCnt > rcMAX_BYTES_PER_FRAME) {
					fprintf(stderr, "Child: Image too large Sz: %d Max %d\n", imgByteCnt,
							rcMAX_BYTES_PER_FRAME);
					pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRRES;
				}
				else {
					fprintf(stderr, "Resolution changed to %s resolution\n",
							(acqControl.resolution == rcAcqFullResXY) ? "full" : "half");
					pMungData->acqControl.resolution = acqControl.resolution;
					pMungData->acqInfo.curResolution = acqControl.resolution;
				}
			}// End of: if (acqControl.resolution != pMungData->acqControl.resolution)
			
			if (acqControl.doShutdown != pMungData->acqControl.doShutdown) {
				fprintf(stderr, "SHUTDOWN REQUEST FROM VISIBLE\n");
				pMungData->stopProcess = 1;
				pMungData->acqControl.doShutdown = acqControl.doShutdown;
			}// End of: if (acqControl.doShutdown != pMungData->acqControl.doShutdown)
			
			if (acqControl.decimationRate != pMungData->acqControl.decimationRate)
			{
				if (acqControl.decimationRate == 0)
				{
					fprintf(stderr, "Invalid decimation factor\n");
					pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRDEC;
				}
				else
				{
					pMungData->acqControl.decimationRate = acqControl.decimationRate;
					pMungData->acqInfo.curDecimationRate = acqControl.decimationRate;
					pMungData->decCounter = 0;
				}
			} // End of: if (acqControl.decimationRate !=
			//             pMungData->acqControl.decimationRate)
			

			if (acqControl.gain != pMungData->acqControl.gain)
			{
				unsigned short temp = acqControl.gain;
				
				if ((err = VDSETCONTRAST(pMungData->vDig, &temp)))
				{
					fprintf(stderr, "Error setting gain %d\n", err);
					pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRGAIN;
				}
				else
				{
					fprintf(stderr, "Gain requested %d returned %d\n", 
							acqControl.gain, temp);
					pMungData->acqInfo.curGain = pMungData->acqControl.gain =
					acqControl.gain;
				}
			}
			
			
			pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_CTRL;
			pMungData->acqControl.acqCtrlUpdateID = acqControl.acqCtrlUpdateID;
			pMungData->acqInfo.acqCtrlUpdateID = acqControl.acqCtrlUpdateID;
			doWrite = true;
		}
		else if (!doWrite)
		{
			if (!releaseMemory(*(pMungData->shMem)))
			{
				pMungData->stopProcess = 1;
				return;
			}
		}
	} // End of: if (getCtrlInfo(pMungData->shMem, &acqControl, false, false))
	else if (fatalError)
	{
		pMungData->stopProcess = 1;
		return;
	}
	
	if (doWrite)
	{
		/* writeAcqInfo both writes and release memory.
		 */
		if (!writeAcqInfo(*(pMungData->shMem), pMungData->acqInfo, true))
			pMungData->stopProcess = 1;
		
		pMungData->acqInfo.ctrlFlags = 0;
	}
}	


// --------------------
// InitializeMungData
//
MungDataPtr InitializeMungData(WindowRef inWindow, Rect inBounds,
							   SeqGrabComponent inSeqGrab)
{
	MungDataPtr pMungData = NULL;
	CGrafPtr theOldPort;
	GDHandle theOldDevice;
	OSErr err = noErr;
    
	// allocate memory for the data
	pMungData = (MungDataPtr)NewPtrClear(sizeof(MungDataRecord));
	if (MemError() || NULL == pMungData ) return NULL;
    
	pMungData->acqInfo.acqState = rcAcqDisabled;
	
	// create a GWorld
	err = QTNewGWorld(&(pMungData->pGWorld), // returned GWorld
					  k8IndexedGrayPixelFormat, // k32ARGBPixelFormat, // pixel format
					  &inBounds,             // bounds
					  0,                     // color table
					  NULL,                  // GDHandle
					  0);                    // flags
	BailErr(err);
    
	// lock the pixmap and make sure it's locked because
	// we can't decompress into an unlocked PixMap
	if(!LockPixels(GetGWorldPixMap(pMungData->pGWorld)))
		goto bail;
    
	/* Note: Leaving this crap in here because I think that at least the
	 * BackColor and ForeColor values need to be right for image
	 * decompression to work correctly (see the Apple docs for a
	 * description of "graphics transfer modes"). The call to
	 * EraseRect() may not be necessary, but that will be left in for
	 * now.
	 */
	GetGWorld(&theOldPort, &theOldDevice);    
	SetGWorld(pMungData->pGWorld, NULL);
	BackColor(blackColor);
	ForeColor(whiteColor);
	EraseRect(&inBounds);    
	SetGWorld(theOldPort, theOldDevice);
	
	pMungData->pWindow = inWindow;
	pMungData->bounds = inBounds;
	pMungData->seqGrab = inSeqGrab;
	pMungData->isGrabbing = false;
	pMungData->stopProcess = 0;
	pMungData->lastTime = 0;
#ifdef DEBUGGING
	pMungData->lastCaptureTime = 0;
#endif
	pMungData->timeScale = 0;
	pMungData->initing = true;
	pMungData->err = noErr;
	
	return pMungData;
 	
bail:
	// something's bust, clean up and get out
	if (pMungData)
	{
		if (pMungData->pGWorld) DisposeGWorld(pMungData->pGWorld);
		DisposePtr((Ptr)pMungData);
	}
	
	return NULL;
}

OSErr InitializeMungData2(MungDataPtr pMungData, SGChannel sgchanVideo,
						  rcSharedMemoryUser* shMem)
{
	if ((pMungData == NULL) || (shMem == 0))
		return -1;
	
	pMungData->acqInfo.acqState = rcAcqUnknown;
	
	uint32 imgByteCnt;
	OSErr err = noErr;
	ImageDescriptionHandle imageDesc = (ImageDescriptionHandle)NewHandle(0);
	Str255 devName;
	Str255 inputName;
	short  inputNumber = -1;
	

	if ((err = VDGETVIDEODEFAULTS(pMungData,
								  ((uint32 *)&(pMungData->acqInfo.defaultGain)),
								  ((uint32 *)&(pMungData->acqInfo.defaultShutter)))))
	{
		fprintf(stderr, "RCDCAM: Error getting defaults %d\n", err);
		pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRINIT;
		err = noErr;
	}
	
	pMungData->acqInfo.curGain = 0;
	pMungData->acqInfo.minGain = 0;
	pMungData->acqInfo.maxGain = rcUINT32_MAX;
	if ((err = VDGETGAIN(pMungData, ((uint32 *)&(pMungData->acqInfo.curGain)),
						 ((uint32 *)&(pMungData->acqInfo.minGain)),
						 ((uint32 *) &(pMungData->acqInfo.maxGain)))))
	{
		fprintf(stderr, "RCDCAM: Error getting gain %d\n", err);
		pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRINIT;
		err = noErr;
	}
	
	pMungData->acqInfo.curShutter = 0;
	pMungData->acqInfo.minShutter = 0;
	pMungData->acqInfo.maxShutter = rcUINT32_MAX;
	if ((err = VDGETSHUTTER(pMungData, ((uint32 *)&(pMungData->acqInfo.curShutter)),
							((uint32 *)&(pMungData->acqInfo.minShutter)),
							((uint32 *)&(pMungData->acqInfo.maxShutter)))))
	{
		fprintf(stderr, "RCDCAM: Error getting shutter %d\n", err);
		pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRINIT;
		err = noErr;
	}
	
	pMungData->acqControl.gain = pMungData->acqInfo.curGain;
	pMungData->acqControl.shutter = pMungData->acqInfo.curShutter;

	if ((err = VDGETVIDEODEFAULTS(pMungData->vDig,
								  0,
								  0,
								  &(pMungData->acqInfo.defaultOffset),
								  0,
								  0,
								  &(pMungData->acqInfo.defaultGain),
								  0)))
	{
		fprintf(stderr, "Error getting defaults %d\n", err);
		pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRINIT;
		err = noErr;
	}
	
	pMungData->acqInfo.curGain = 0;
	if ((err = VDGETCONTRAST(pMungData->vDig, &(pMungData->acqInfo.curGain))))
	{
		fprintf(stderr, "Error getting gain %d\n", err);
		pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRINIT;
		err = noErr;
	}
	
		
	fprintf(stderr, "def gain %d curr gain %d\n",
			pMungData->acqInfo.defaultGain, pMungData->acqInfo.curGain);

	
	pMungData->acqInfo.curDecimationRate = 1;
	pMungData->acqInfo.curResolution = rcAcqFullResXY;
	pMungData->acqInfo.acqState = rcAcqStopped;
	pMungData->acqInfo.missedFrames = 0;
	// pMungData->acqInfo.cameraChangeCount = 0; // Will this ever be used??
	pMungData->acqInfo.acqFlags = 0;
	pMungData->acqInfo.ctrlFlags = 0;
	pMungData->acqInfo.acqCtrlUpdateID = 0;
	
	pMungData->acqControl.doAcquire = false;
	pMungData->acqControl.resolution = rcAcqFullResXY;
	pMungData->acqControl.doShutdown = 0;
	
	pMungData->decCounter = 0;
	pMungData->acqControl.gain = pMungData->acqInfo.curGain;
	pMungData->acqControl.acqCtrlUpdateID = pMungData->acqInfo.acqCtrlUpdateID;
	
	/* Retrieve a channel's current sample description, the channel
	 * returns a sample description that is appropriate to the type of
	 * data being captured
	 */
	BailErr(SGGetChannelSampleDescription(sgchanVideo, (Handle)imageDesc));
	
	pMungData->vDig = SGGetVideoDigitizerComponent(sgchanVideo);
	if (!pMungData->vDig)
		BailErr(err = -1);
	
	if ((err = SGGetChannelDeviceAndInputNames(sgchanVideo, devName, inputName,
											   &inputNumber)))
	{
		printf("SGGetChannelDeviceAndInputNames failed\n");
		devName[0] = 0;
		err = noErr;
	}
	else
	{
		/* Name has a bunch of non-printing characters embedded in it. For now
		 * just squeeze these out.
		 */
		int j = 0;
		for (int i = 0; i < 256; i++)
			if (devName[i] == 0)
				break;
			else if ((devName[i] >= 0x20) && (devName[i] <= 0x7e))
				pMungData->acqInfo.cameraName[j++] = (char)devName[i];
	}
	
	// Read out the image description information
	pMungData->imgWidth = (**imageDesc).width;
	pMungData->imgHeight = (**imageDesc).height;
	pMungData->imgSizeInBytes = (**imageDesc).dataSize;
	pMungData->isGray = false;
	pMungData->extractLum = false;
	
	if ((**imageDesc).depth == 40) // Grey scale image
	{
		pMungData->pixelDepth = rcPixel8;
		pMungData->isGray = true;
	}
	else if ((**imageDesc).depth == 8) // 8 bit color image
		pMungData->pixelDepth = rcPixel8;
	else if ((**imageDesc).depth == 16) // 16 bit color image
		pMungData->pixelDepth = rcPixel16;
	else if (((**imageDesc).depth == 24) ||
			 ((**imageDesc).depth == 32)) // color image as well
	{
		// pMungData->pixelDepth = rcPixel32;
		// pMungData->isGray = false;
		
		/* The following is a hack that assumes all color images are yuvu
		 * and that they should be converted to grey scale.
		 */
		pMungData->pixelDepth = rcPixel8;
		pMungData->extractLum = true;
		pMungData->isGray = true;
	}
	else
		BailErr(-1);
	
	if (pMungData->imgSizeInBytes % (pMungData->imgHeight))
	{
		fprintf(stderr, "Couldn't calculate row offset\n");
		BailErr(-1);
	}
	
	imgByteCnt = pMungData->imgSizeInBytes;
	pMungData->rowUpdate = imgByteCnt/(pMungData->imgHeight);
	
	if (pMungData->acqControl.resolution == rcAcqHalfResXY)
		imgByteCnt /= 4;
	if (pMungData->extractLum)
	{
		imgByteCnt /= 2;
		
		// Check that image lines are each a multiple of 4 in length o that
		// the "extract luminance" code in MungGrabDataProc will work.
		
		if (pMungData->imgWidth & 0x3)
		{
			fprintf(stderr, "Child: Image lines not modulo 4 in length\n");
			BailErr(-1);
		}
	}
	
	fprintf(stderr, "Init img info: width %d height %d pixelDepth %d row update %d"
			" pixel cnt %d isGray %d extract %d dataSize %ld\n",
			pMungData->imgWidth, pMungData->imgHeight,
			(uint32)pMungData->pixelDepth, pMungData->rowUpdate,
			imgByteCnt, pMungData->isGray,
			pMungData->extractLum, (**imageDesc).dataSize);
	
	pMungData->shMem = shMem;
	
	if (imgByteCnt > rcMAX_BYTES_PER_FRAME)
	{
		fprintf(stderr, "Child: Image too large Sz: %d Max %d\n", imgByteCnt,
				rcMAX_BYTES_PER_FRAME);
		BailErr(-1);
	}
	
	if ((**imageDesc).depth == 40)
	{
		if (pMungData->acqInfo.cameraName[0] == 0)
			strcpy((char *) pMungData->acqInfo.cameraName, "Grey Scale Camera");
		pMungData->acqInfo.cameraType = rcCameraGreyScale;
		
		// If SGGetFrameRate() returned useful numbers we could use its
		// return value. As things stand...
		if ((**imageDesc).width == 1024)
			pMungData->acqInfo.maxFramesPerSecond = rcRS170_FRAMES_PER_SEC/2;
		else
			pMungData->acqInfo.maxFramesPerSecond = rcRS170_FRAMES_PER_SEC/4;
	}
	else
	{
		if (pMungData->acqInfo.cameraName[0] == 0)
			strcpy((char *) pMungData->acqInfo.cameraName, "Color Camera");
		pMungData->acqInfo.cameraType = rcCameraColor;
		// If SGGetFrameRate() returned useful numbers we could use its
		// return value. As things stand...
		if ((**imageDesc).width == 640) // CameraireWire 640x480 does 30 fps
			pMungData->acqInfo.maxFramesPerSecond = rcRS170_FRAMES_PER_SEC;
		else
			pMungData->acqInfo.maxFramesPerSecond = rcRS170_FRAMES_PER_SEC/2;
	}
	
	pMungData->acqInfo.maxFrameWidth = (**imageDesc).width;
	pMungData->acqInfo.maxFrameHeight = (**imageDesc).height;
	pMungData->acqInfo.cameraPixelDepth = pMungData->pixelDepth;
	
bail:
	DisposeHandle((Handle)imageDesc);         
	return err;
}

// --------------------
// MakeAWindow
/* More crap that it would be nice to ditch */
//
OSErr MakeAWindow(WindowRef *outWindow)
{
	Rect	windowRect = {0, 0, 960, 1280};
	//Rect	windowRect = {0, 0, 480, 640};
	Rect    bestRect;
	WindowAttributes wAttributes = kWindowCloseBoxAttribute |
	kWindowCollapseBoxAttribute |
	kWindowStandardHandlerAttribute |
	kWindowInWindowMenuAttribute;
	OSErr err = noErr;
	
	// figure out the best monitor for the window
	GetBestDeviceRect(NULL, &bestRect);
	
	// put the window in the top left corner of that monitor
	OffsetRect(&windowRect, bestRect.left + 10, bestRect.top + 50);
    
	BailErr(CreateNewWindow(kDocumentWindowClass, wAttributes, &windowRect,
							outWindow));
	
	// set the port to the new window
	SetPortWindowPort(*outWindow);
	
bail:
	return err;
}

// --------------------
// DisposeMungData
//
void DisposeMungData(MungDataPtr pMungData)
{
	// clean up the bits
	if(pMungData)
	{
		if (pMungData->seqGrab)
			CloseComponent(pMungData->seqGrab);
		
		if (pMungData->pGWorld)
		{
			DisposeGWorld(pMungData->pGWorld);
			pMungData->pGWorld = NULL;
		}
		
		DisposePtr((Ptr)pMungData);
		pMungData = NULL;
	}
}

// --------------------
// MakeSequenceGrabber
//
SeqGrabComponent MakeSequenceGrabber(WindowRef pWindow)
{
	SeqGrabComponent seqGrab = NULL;
	OSErr err = noErr;
	
	// open the default sequence grabber
	seqGrab = OpenDefaultComponent(SeqGrabComponentType, 0);
	if (seqGrab != NULL)
	{ 
		// initialize the default sequence grabber component
		err = SGInitialize(seqGrab);
		
		if (err == noErr) // set its graphics world to the specified window
			err = SGSetGWorld(seqGrab, GetWindowPort(pWindow), NULL);
    	
		if (err == noErr)
			// specify the destination data reference for a record operation
			// tell it we're not making a movie if the flag
			// seqGrabDontMakeMovie is used, the sequence grabber still
			// calls your data function, but does not write any data to the
			// movie file writeType will always be set to seqGrabWriteAppend
			err = SGSetDataRef(seqGrab, 0, 0, seqGrabDontMakeMovie);
	}
	
	if (err && (seqGrab != NULL)) // clean up on failure
	{
		CloseComponent(seqGrab);
		seqGrab = NULL;
	}
	
	return seqGrab;
}

// --------------------
// MakeSequenceGrabChannel
//
OSErr MakeSequenceGrabChannel(SeqGrabComponent seqGrab, SGChannel *sgchanVideo,
							  Rect const *rect)
{
	long  flags = 0;
	OSErr err = noErr;
    
	DisplayAndBail(SGNewChannel(seqGrab, VideoMediaType, sgchanVideo),
				   "MSGC new channel");
	DisplayAndBail(SGSetChannelBounds(*sgchanVideo, rect), "MSGC channel bounds");
	
	// set usage for new video channel to avoid playthrough
	// note we don't set seqGrabPlayDuringRecord
	err = SGSetChannelUsage(*sgchanVideo, flags | seqGrabRecord);
	
	if (err != noErr)
	{
		DisplayError(err, "MSGC channel usage");
		
		// clean up on failure
		SGDisposeChannel(seqGrab, *sgchanVideo);
		*sgchanVideo = NULL;
	}
	
bail:
	return err;
}

int didit = 0;

#ifdef DEBUGGING
rcSparseHistogram* histP = 0;
#endif

/* ----------------------------------------------------------------------
 sequence grabber data procedure - this is where the work is done
 ---------------------------------------------------------------------- */
/* MungGrabDataProc - the sequence grabber calls the data function
 * whenever any of the grabbers channels write digitized data to the
 * destination movie file.
 
 * NOTE: We really mean any, if you have an audio and video channel
 * then the DataProc will be called for either channel whenever data
 * has been captured. Be sure to check which channel is being passed
 * in. In this example we never create an audio channel so we know
 * we're always dealing with video.
 
 * This data function does two things, it first decompresses captured
 * video data into an offscreen GWorld, draws some status information
 * onto the frame then transfers the frame to an onscreen window.
 
 * For more information refer to Inside Macintosh: QuickTime
 * Components, page 5-120
 *
 * c         - the channel component that is writing the digitized data.
 * p         - a pointer to the digitized data.
 * len       - the number of bytes of digitized data.
 * offset    - a pointer to a field that may specify where you are to
 *             write the digitized data, and that is to receive a value
 *             indicating where you wrote the data. (Not Used)
 * chRefCon  - per channel reference constant specified using
 *             SGSetChannelRefCon. (Not Used)
 * time	     - the starting time of the data, in the channels time scale.
 * writeType - the type of write operation being performed. (Not Used)
 * refCon    - the reference constant you specified when you assigned your
 *             data function to the sequence grabber.
 */
pascal OSErr 
MungGrabDataProc(SGChannel c, Ptr p, long, long*, long, TimeValue time, short,
				 long refCon)
{
	ComponentResult err = noErr;
	
	MungDataPtr pMungData = (MungDataPtr)refCon;
	if (NULL == pMungData) {
		printf("What the heck!!!\n");
		return -1;
	}
	
#ifdef DEBUGGING
	if ((pMungData->timeScale != 0)) {
		if (pMungData->lastCaptureTime != 0) {
			double timeDiff = (1000*(time - pMungData->lastCaptureTime))/
			pMungData->timeScale;
			histP->add(uint16(timeDiff));
			
			uint32 min, max;
			histP->range(min, max);
			
			if ((histP->binsUsed() > 2) || ((max - min) > 1)) {
				pMungData->acqInfo.acqFlags |= rcACQFLAGS_FRAMESKIPPED;
				fprintf(stderr, "RCQTIME: Frame(s) Skipped! last timestamp %f\n",
						pMungData->curTime.secs());
				
				const rcSparseHistogram::sparseArray& histMap = histP->getArray();
				rcSparseHistogram::sparseArray::const_iterator start = histMap.begin();
				rcSparseHistogram::sparseArray::const_iterator end = histMap.end();
				
				for ( ; start != end; start++)
					fprintf(stderr, "Interval %d Count %d\n", (*start).first,
							(*start).second);
				
				delete histP;
				histP = new rcSparseHistogram(3);
				if (!histP)
					return -1;
			}
		}
	}
	pMungData->lastCaptureTime = time;
#endif
	
	if (pMungData->stopProcess)
		return noErr;
	
	bool transmitFrame = (pMungData->decCounter == 0);
	
	if (++(pMungData->decCounter) == pMungData->acqInfo.curDecimationRate)
		pMungData->decCounter = 0;
	
	if (!transmitFrame)
		return noErr;
	
#if 0
	fprintf(stderr, "MGDP pMungData 0x%X p 0x%X len 0x%lX\n", (int)pMungData,
			(int)p, len);
#endif
	
	
	if (pMungData->timeScale == 0) {
		// first time here so set the time scale
		TimeScale scale;
		
		if ((err = SGGetChannelTimeScale(c, &scale))) {
			fprintf(stderr, "error code %d SGGetChannelTimeScale", (int)err);
			return err;
		}
		
		pMungData->timeScale = scale;
		pMungData->curTime = rcTimestamp::now();
		fprintf(stderr, "RCQTIME: start time %f\n", pMungData->curTime.secs());
		pMungData->frameCount = 1;
		
#ifdef DEBUGGING
		if (histP)
			delete histP;
		
		histP = new rcSparseHistogram(3);
		if (!histP)
			return -1;
#endif
	}
	else {
		if (pMungData->lastTime > time) {
			// Need to look at what happens on rollover -- xyzzy
			fprintf(stderr, "ERROR! last time %ld > time %ld\n", pMungData->lastTime,
					time);
			pMungData->lastTime = 0;
#ifdef DEBUGGING
			pMungData->lastCaptureTime = 0;
#endif
		}
		else
			pMungData->curTime += rcTimestamp((time-pMungData->lastTime)/
											  pMungData->timeScale);
		pMungData->frameCount++;
	}
    
	pMungData->lastTime = time;
	
	if (!pMungData->pGWorld)
		return noErr;
	
	rcSharedMemError smErr;
	
	rcVideoSharedMemLayout* vidDataBuf = 
    (rcVideoSharedMemLayout*)(pMungData->shMem->acquireSharedMemory(smErr,
																	false));
	
	if (!vidDataBuf) {
		if (smErr == rcSharedMemNotAvailable) {
			pMungData->acqInfo.missedFrames++;
			if (pMungData->acqInfo.missedFrames == 1) //xyzzy ditch me
				fprintf(stderr, "RCQTIME: Missed frame count (no shmem) %d\n",
						pMungData->acqInfo.missedFrames);
		}
		else {
			fprintf(stderr, "Child: error acquiring shared data. err: %d\n", smErr);
			perror("Child: ");
			pMungData->stopProcess = 1;
		}
		return noErr;
	}
	
	/* Copy info into shared buffer */
	
	const unsigned int resAdjust =
    (pMungData->acqControl.resolution == rcAcqHalfResXY) ? 2 : 1;
	
	vidDataBuf->width = (pMungData->imgWidth)/resAdjust;
	vidDataBuf->height = (pMungData->imgHeight)/resAdjust;
	/* We always store bytes compactly, so just use desired width (in
     pixels) times pixel depth.
	 */
	vidDataBuf->rowUpdate = (vidDataBuf->width)*(uint32)(pMungData->pixelDepth);
	vidDataBuf->pixelDepth = pMungData->pixelDepth;
	vidDataBuf->isGray = pMungData->isGray;
	vidDataBuf->timestamp = pMungData->curTime.secs();
	
	if (didit == 0)
	{
		fprintf(stderr, "Set buf info to: width %d height %d pixelDepth %d"
				" row update %d isGray %d src 0x%X dest 0x%X\n",
				vidDataBuf->width, vidDataBuf->height, 
				(uint32)(vidDataBuf->pixelDepth), vidDataBuf->rowUpdate,
				vidDataBuf->isGray, (uint32)p, (uint32)(vidDataBuf->rawPixels));
	}
	
	if (pMungData->acqControl.resolution == rcAcqHalfResXY)
	{
		rcGenHalfRes((char*)p, vidDataBuf->rawPixels,
					 (pMungData->imgWidth)*(uint32)(pMungData->pixelDepth),
					 pMungData->imgHeight, pMungData->rowUpdate, 0);
	}
	else if (pMungData->extractLum)
	{
		const uint32 unrollCnt = 8;
		const uint32 intsPerLine = vidDataBuf->width/2; // assumes 16 bit pixels
		const uint32 unrolledLoopCnt = intsPerLine / unrollCnt;
		const uint32 unrolledRemainder = intsPerLine % unrollCnt;
		unsigned char* const baseSrcP = (unsigned char*)p;
		unsigned char* const baseDestP = (unsigned char*)(vidDataBuf->rawPixels);
		
		for (uint32 lineNum = 0; lineNum < vidDataBuf->height; lineNum++)
		{
			unsigned short* srcP = (unsigned short*)(baseSrcP +
													 lineNum * pMungData->rowUpdate);
			unsigned short* destP = (unsigned short*)(baseDestP +
													  lineNum * vidDataBuf->rowUpdate);
			
			for (uint32 i = 0; i < unrolledLoopCnt; i++)
			{
				*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
			}
			
			switch (unrolledRemainder)
			{
				case 7:
					*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				case 6:
					*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				case 5:
					*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				case 4:
					*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				case 3:
					*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				case 2:
					*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF); srcP += 2;
				case 1:
					*(destP++) = ((*srcP & 0xFF) << 8) | (*(srcP+1) & 0xFF);
			}
		} // End of: for (lineNum = 0; lineNum < vidDataBuf->height; lineNum++)
	} // End of: else if (pMungData->extractLum)
	else
	{
		unsigned char* const baseSrcP = (unsigned char*)p;
		unsigned char* const baseDestP = (unsigned char*)(vidDataBuf->rawPixels);
		
		for (uint32 lineNum = 0; lineNum < vidDataBuf->height; lineNum++)
			memmove((baseDestP + lineNum * vidDataBuf->rowUpdate),
					(baseSrcP + lineNum * pMungData->rowUpdate),
					vidDataBuf->rowUpdate);
	}
	
	if (didit == 0)
	{
		unsigned char *pp = (unsigned char*)p;
		unsigned char *vp = (unsigned char*)vidDataBuf;
		
		fprintf(stderr, "R 0x%X 0x%X 0x%X 0x%X P: 0x%X 0x%X 0x%X 0x%X\n",
				pp[8 + 1280*16], pp[128 + 1280*16], pp[8 + 1280*128],
				pp[128 + 1280*128],
				vp[4 + 640*8], vp[64 + 640*8], vp[4 + 6400*64], vp[64 + 640*64]);
	}
	
	didit++;
	
	// Let processControlRequests process peer control requests, write
	// acqInfo to shared memory and release shared memory
	pMungData->acqInfo.acqFlags |= rcACQFLAGS_FRAMEAVAIL;
	processControlRequests(pMungData, true);
	pMungData->acqInfo.acqFlags = 0;
	
	return pMungData->err = err;
}

// --------------------
// MGIdleTimer
//
// Munggrab idle timer to idle the sequence grabber, call this at least
// as much as the desired frame rate - more is better
static pascal void MGIdleTimer(EventLoopTimerRef, void *inUserData)
{
	MungDataPtr pMungData = MungDataPtr(inUserData);
	if (NULL == pMungData) return;
	
	if (pMungData->stopProcess)
	{
		SGStop(pMungData->seqGrab);
		RemoveEventLoopTimer(pMungData->timerRef);
		QuitApplicationEventLoop();
	}
	
	if (pMungData->acqInfo.acqState == rcAcqRunning)
	{
		OSErr err = SGIdle(pMungData->seqGrab);
		
		if (err != noErr) {
			// some error specific to SGIdle occurred - any errors returned from the
			// data proc will also show up here and we don't want to write over them
			
			// in QT 4 you would always encounter a cDepthErr error after a user drags
			// the window, this failure condition has been greatly relaxed in QT 5
			// it may still occur but should only apply to vDigs that really control
			// the screen
			
			// you don't always know where these errors originate from, some may come
			// from the VDig...
			if (pMungData->err == noErr) {
				DisplayError(err, "MGIdleTimer");
				pMungData->err = err;
			}
			
			// ...to fix this we simply call SGStop and SGStartRecord again
			// calling stop allows the SG to release and re-prepare for grabbing
			// hopefully fixing any problems, this is obviously a very relaxed
			// approach
			pMungData->timeScale = 0;
			pMungData->acqInfo.acqFlags |= rcACQFLAGS_FRAMEERROR;
			
#ifdef DEBUGGING
			if (histP) {
				delete histP;
				histP = 0;
			}
#endif
			SGStop(pMungData->seqGrab);
			SGStartRecord(pMungData->seqGrab);
		}
	}
	
	if (pMungData->acqInfo.acqState == rcAcqStopped)
		processControlRequests(pMungData, false);
}

// --------------------
// InstallEvenHandlers
//
// install carbon event handlers and timer
// you can find more information about carbon events here:
// http://developer.apple.com/techpubs/macosx/Carbon/carbon.html
// http://developer.apple.com/sdk/index.html
static OSErr InstallEvenHandlers(MungDataPtr inMungData)
{
	OSStatus err;
	
	err = InstallEventLoopTimer(GetMainEventLoop(), kEventDurationNoWait, 
								kTimerInterval,
								NewEventLoopTimerUPP(MGIdleTimer), inMungData,
								&inMungData->timerRef);
	return err;
}

static MungDataPtr pMungData = NULL;

void rfQTVideoCapture(rcSharedMemoryUser& shMem)
{
	WindowRef        pMainWindow = NULL;
	SeqGrabComponent seqGrab = 0;
	SGChannel        sgchanVideo = 0;
	Rect             portRect;
	OSErr            err = noErr;
	
	EnterMovies();
	
	fprintf(stderr, "made it 1");
	
	// create the window
	err = MakeAWindow(&pMainWindow);
	BailErr(err);
	
	fprintf(stderr, "...2");
	
	GetPortBounds(GetWindowPort(pMainWindow), &portRect);
	
	fprintf(stderr, "...3");
	
	// create and initialize the sequence grabber
	seqGrab = MakeSequenceGrabber(pMainWindow);
	BailErr(NULL == seqGrab);
	
	fprintf(stderr, "...4");
	
	// create the channel
	err = MakeSequenceGrabChannel(seqGrab, &sgchanVideo, &portRect);
	BailErr(err);
    
	fprintf(stderr, "...5");
	
	// initialize our data that's going to be passed around
	pMungData = InitializeMungData(pMainWindow, portRect, seqGrab);
	BailErr(NULL == pMungData);
    
	fprintf(stderr, "...6");
	
	// specify a sequence grabber data function
	err = SGSetDataProc(seqGrab, NewSGDataUPP(MungGrabDataProc), (long)pMungData);
	BailErr(err);
	
	fprintf(stderr, "...7");
	
	// install carbon event handlers
	err = InstallEvenHandlers(pMungData);
	BailErr(err);
	
	fprintf(stderr, "...8\n");
	
    // lights...camera...
	err = SGPrepare(seqGrab, false, true);
	BailErr(err); 
    
	// Now have values available to setup image copy specific values
	err = InitializeMungData2(pMungData, sgchanVideo, &shMem);
	BailErr(err); 
	
	fprintf(stderr, "made it 9");
	
	// Let peer process know that we are ready to start acquiring frames
	if (!writeAcqInfo(shMem, pMungData->acqInfo, true))
		goto bail2; // Pretty ugly, huh?
	
	// run the application
	RunApplicationEventLoop();
	fprintf(stderr, "RCQTIME: RETURNED FROM EVENTLOOP\n");
	
bail:
	if (err != noErr)
	{
		// Try to notify the peer process that acq process wasn't able to
		// come up.
		//
		rcAcqInfo  acqInfo;
		rcAcqInfo* ip = 0;
		if (pMungData)
			ip = &(pMungData->acqInfo);
		else
		{
			acqInfo.acqState = rcAcqDisabled;
			ip = &acqInfo;
		}
		
		writeAcqInfo(shMem, *ip, true);
	}
	
bail2:
	// clean up
	printf( "\n" );
	if (pMainWindow)
		DisposeWindow(pMainWindow);
    
	DisposeMungData(pMungData);
	
	fprintf(stderr, "RCQTIME: DONE\n");
	return;
}
