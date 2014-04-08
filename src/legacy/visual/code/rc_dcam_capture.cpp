/*
 *  rc_dcam_capture.cpp
 *
 *  Copyright (c) 2002-2004 Reify Corp. All rights reserved.
 *
 */


#include <rc_dcam.h>
#include <rc_dcam_capture.h>
#include <rc_capture.hpp>
#include <rc_assemblyfcts.h>
#include <assert.h>
#include <stdlib.h> // Defines exit()
#include <string.h>
#include <rc_ipconvert.h>
#include <rc_resource_ctrl.h> // Defines rcSharedMemoryUser class
#include <rc_timestamp.h>
#include <rc_sparsehist.h>

// Debugging controls
// #define INTERVAL_LOGGING
// #define DEBUGGING
//#define EXPLICIT

//#define DEBUG_LOG
//#define DEBUG_INIT
//#define DEBUG_CALLBACK
//#define DEBUG_TIMER

#define BailErr(x) {err = x; if(err != noErr) goto bail;}
#define DisplayError(x,y) fprintf(stderr,"Error %d Msg string ; %s\n",(int)(x),y)
#define DisplayAndBail(x, y) { err = x; if(err != noErr)                \
{ DisplayError(err, y); goto bail;} }

#define rcDCAM_SIZE_EXCEEDED ((OSStatus)0xFFFFFFFE)
#define rcDCAM_NO_MODE       ((OSStatus)0xFFFFFFFF)

static int didit = 0;

// mung data struct
typedef struct {
	void *			      theCameraPtr;
	rcTimestamp 		  lastCaptureTime;
	rcTimestamp           curTime;
	long 			      frameCount;
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
	Boolean		          isGrabbing;
	EventLoopTimerRef	  timerRef;
	OSErr			      err;
	CameraCapabilitiesStruct cameraProperties;
	rcSharedFrameBufPtr imgBuf32;  // Image buffer for format conversions
	rcSharedFrameBufPtr imgBuf8;   // Image buffer for format conversions
	rcChannelConversion  imgConversion;
} MungDataRecord, *MungDataPtr;

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
	void *				theCameraPtr;//camera object
	MungDataPtr         pMungData;
} yourProcessFrameCallBack, *yourProcessFrameCallBackPtr;

// constants
const EventTime kTimerInterval = kEventDurationSecond/30; // idle timer interval

OSStatus VDGETGAIN (MungDataPtr pMungData,
                    uint32 *gain,
                    uint32 *minGain,
                    uint32 *maxGain);
OSStatus VDSETGAIN (MungDataPtr pMungData,
                    uint32 *gain );
OSStatus VDGETSHUTTER (MungDataPtr pMungData,
                       uint32 *shutter,
                       uint32 *minShutter,
                       uint32 *maxShutter);
OSStatus VDSETSHUTTER (MungDataPtr pMungData,
                       uint32 *shutter );

// Get one proprety from camera
static OSStatus getOneProperty( void* cameraPtr, UInt32 type, UInt32 cntl, UInt32* value )
{
	OSStatus err = ASCDCAMGetPropertyValue( cameraPtr,
																				 type,
																				 cntl,
																				 value );
	
	return err;
}


// Get one cntl property
static rcCameraProperty GetCameraProperty( void* cameraPtr, UInt32 type, UInt32 cntl )
{
	rcCameraProperty prop( type, cntl );
	
	OSStatus err = ASCDCAMGetPropertyCapability( cameraPtr,
																							type,
																							&prop.minValue(),
																							&prop.maxValue(),
																							&prop.canOnePush(),
																							&prop.canReadOut(),
																							&prop.canOnOff(),
																							&prop.canAuto(),
																							&prop.canManual() );
	if ( err == 0 ) {
		if ( prop.canReadOut() ) {
			prop.active() = 1;
			err = getOneProperty( cameraPtr,
													 type,
													 cntl,
													 &prop.curValue() );
			if ( err != 0 )
				prop.active() = 0;
		}
	} else {
#ifdef DEBUG_LOG
		cerr << "ASCDCAMGetPropertyCapability " <<  propertyName( type ) << " error " << err << endl;
#endif
	}
	return prop;
}

// Copy image data to shared memory
static void copyToSharedMem( rcVideoSharedMemLayout* vidDataBuf,
														const unsigned char* const srcBuf,
														uint32 srcRowUpdate )
{
	if ( vidDataBuf->rowUpdate == srcRowUpdate ) {
		// Copy data in one chunk
		//cerr << "copyToSharedMem buf memcpy" << endl;
		memcpy( vidDataBuf->rawPixels,
					 srcBuf,
					 srcRowUpdate * vidDataBuf->height );
	} else {
		//cerr << "copyToSharedMem line memmove" << endl;
		// Copy row by row
		for (uint32 line = 0; line < vidDataBuf->height; line++)
			memmove((vidDataBuf->rawPixels + line * vidDataBuf->rowUpdate),
							(srcBuf + line * srcRowUpdate),
							vidDataBuf->rowUpdate);
	}
}

/* releaseMemory - Release shared memory back to peer process. Since
 * failure is considered a fatal error, the error messages should be
 * kept. The caller is expected to do any further error handling.
 */
static bool releaseMemory(rcSharedMemoryUser& shMem)
{
	rcSharedMemError smErr;
	
	switch (smErr = shMem.releaseSharedMemory())
	{
		case rcSharedMemNoError:
			break;
			
		default:
			fprintf(stderr, "RCDCAM: Child: error freeing shared data, err: %d\n", smErr);
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
static bool writeAcqInfo(rcSharedMemoryUser& shMem, rcAcqInfo& acqInfo, bool relMemory)
{
	rcSharedMemError smErr;
	
	rcVideoSharedMemLayout* sharedBuf =
	(rcVideoSharedMemLayout*)(shMem.acquireSharedMemory(smErr, false));
	
	if (!sharedBuf)
	{
		fprintf(stderr, "RCDCAM: Child: error acquiring shared data. err: %d\n", smErr);
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
static bool getCtrlInfo(rcSharedMemoryUser& shMem, rcAcqControl& acqControl,
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
			fprintf(stderr, "RCDCAM: Child: error acquiring shared data. err: %d\n", smErr);
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
static void processControlRequests(MungDataPtr pMungData, bool doWrite)
{
	assert(pMungData);
	assert(pMungData->shMem);
	bool fatalError = true;
	
	rcAcqControl acqControl;
	if (getCtrlInfo(*(pMungData->shMem), acqControl, fatalError, false, false)) {
		if (acqControl.acqCtrlUpdateID != pMungData->acqControl.acqCtrlUpdateID) {
			// Work to do!
			
			if (acqControl.doAcquire != pMungData->acqControl.doAcquire) {
				if (acqControl.doAcquire) {
					if (pMungData->initing) {
						// ...action
						pMungData->initing = false;
						pMungData->isGrabbing = true;
						pMungData->acqInfo.acqState = rcAcqRunning;
					}
				} else {
					if ( 1 )
					{
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
				
				if (imgByteCnt > rcMAX_BYTES_PER_FRAME) {
					fprintf(stderr, "RCDCAM: Child: Image too large Sz: %d Max %d\n", imgByteCnt,
									rcMAX_BYTES_PER_FRAME);
					pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRRES;
				}
				else {
					fprintf(stderr, "RCDCAM: Resolution changed to %s resolution\n",
									(acqControl.resolution == rcAcqFullResXY) ? "full" : "half");
					pMungData->acqControl.resolution = acqControl.resolution;
					pMungData->acqInfo.curResolution = acqControl.resolution;
				}
			}// End of: if (acqControl.resolution != pMungData->acqControl.resolution)
			
			if (acqControl.doShutdown != pMungData->acqControl.doShutdown) {
				fprintf(stderr, "RCDCAM: SHUTDOWN REQUEST FROM VISIBLE\n");
				pMungData->stopProcess = 1;
				pMungData->acqControl.doShutdown = acqControl.doShutdown;
			}// End of: if (acqControl.doShutdown != pMungData->acqControl.doShutdown)
			
			if (acqControl.decimationRate != pMungData->acqControl.decimationRate)
			{
				if (acqControl.decimationRate == 0)
				{
					fprintf(stderr, "RCDCAM: Invalid decimation factor\n");
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
				uint32 temp = acqControl.gain;
				OSStatus err = VDSETGAIN(pMungData, &temp);
				
				if ( err != 0 ) {
					pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRGAIN;
					// Keep old value
					pMungData->acqInfo.curGain = pMungData->acqControl.gain;
					acqControl.gain = pMungData->acqControl.gain;
					pMungData->acqInfo.cameraInfo.gain( acqControl.gain );
				} else {
					pMungData->acqInfo.curGain = pMungData->acqControl.gain =
					acqControl.gain;
					pMungData->acqInfo.cameraInfo.gain( acqControl.gain );
				}
			}
			
			if (acqControl.shutter != pMungData->acqControl.shutter)
			{
				uint32 temp = acqControl.shutter;
				OSStatus err = VDSETSHUTTER(pMungData, &temp);
				
				if ( err != 0 ) {
					pMungData->acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRSHUTTER;
					// Keep old value
					pMungData->acqInfo.curShutter = pMungData->acqControl.shutter;
					acqControl.shutter = pMungData->acqControl.shutter;
					pMungData->acqInfo.cameraInfo.shutter( acqControl.shutter );
				} else {
					pMungData->acqInfo.curShutter = pMungData->acqControl.shutter =
					acqControl.shutter;
					pMungData->acqInfo.cameraInfo.shutter( acqControl.shutter );
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
				cerr << "RCDCAM: processControlRequests releaseMemory error " << endl;
				return;
			}
		}
	} // End of: if (getCtrlInfo(pMungData->shMem, &acqControl, false, false))
	else if (fatalError)
	{
		pMungData->stopProcess = 1;
		cerr << "RCDCAM: processControlRequests error " << fatalError << endl;
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
#ifdef DEBUG_CALLBACK
	cerr << "RCDCAM: processControlRequests end   " << doWrite << " " << rcTimestamp::now() << endl;
#endif
}

#ifdef INTERVAL_LOGGING
static rcSparseHistogram sHistogram;
static uint32 sHistogramRange = 0;

// Update frame interval histogram
static void intervalStatistics( const rcTimestamp& cur,
															 MungDataPtr pMungData,
															 ASC_CallbackStruct theCallBackData )
{
	if (pMungData->lastCaptureTime != cZeroTime) {
		double timeDiff = theCallBackData.frameDurationMS;
		sHistogram.add(uint16(timeDiff));
		if ( timeDiff >= 65535.0 )
			cerr << "RCDCAM: unusually long frame interval: " << timeDiff << " ms" << " at " << cur.localtime().c_str() << endl;
		uint32 min, max;
		sHistogram.range(min, max);
		uint32 r = max-min;
		
		if (r > sHistogramRange) {
			sHistogramRange = r;
			if (sHistogram.binsUsed() > 2) {
				pMungData->acqInfo.acqFlags |= rcACQFLAGS_FRAMESKIPPED;
				fprintf(stderr, "RCDCAM: Frame interval fluctuation! Last frame timestamp %s\n",
								pMungData->curTime.localtime().c_str());
				
				const rcSparseHistogram::sparseArray& histMap = sHistogram.getArray();
				rcSparseHistogram::sparseArray::const_iterator start = histMap.begin();
				rcSparseHistogram::sparseArray::const_iterator end = histMap.end();
				
				for ( ; start != end; start++)
					fprintf(stderr, "Interval %d ms Count %d\n", (*start).first,
									(*start).second);
			}
		}
	}
	pMungData->lastCaptureTime = cur;
}
#endif

static void yourDisplayCallback(ASC_CallbackStruct theCallBackData)
{
	if ((theCallBackData.userDataPtr != NULL) && (theCallBackData.ImageBufferPtr != NULL))
	{
		yourProcessFrameCallBackPtr	theCallbackDataPtr = (yourProcessFrameCallBackPtr) theCallBackData.userDataPtr;
		MungDataPtr pMungData = theCallbackDataPtr->pMungData;
		
		if (pMungData->initing)
		{
			// Don't accept anything during initialization
			return;
		}
		
		rcTimestamp cur = rcTimestamp::now();
		OSStatus err = noErr;
		
		if (theCallbackDataPtr->shouldDisplay == true)
		{
			if (theCallbackDataPtr->isBayerData == true)
			{
				// Not supported yet
				err = -2;
				theCallbackDataPtr->theErr = err;
			}
			else 
			{ //treat this as a "normal" image..

				if (err == 0)
				{
					if (pMungData->stopProcess)
					{
						theCallbackDataPtr->theErr = -666;						
						return;
					}
#ifdef INTERVAL_LOGGING
					intervalStatistics( cur, pMungData, theCallBackData );
#endif
					if (pMungData->curTime != cZeroTime)
						pMungData->curTime += rcTimestamp( theCallBackData.frameDurationMS/1000.0 );
					
					bool transmitFrame = (pMungData->decCounter == 0);
					
					if (++(pMungData->decCounter) == pMungData->acqInfo.curDecimationRate)
						pMungData->decCounter = 0;
					
					if (!transmitFrame)
						return /*noErr*/;
					
					if (pMungData->curTime == cZeroTime) {
						// first time here
						pMungData->curTime = cur;
						fprintf(stderr, "RCDCAM: start time %s\n", pMungData->curTime.localtime().c_str());
						pMungData->frameCount = 1;
					} else {
						pMungData->frameCount++;
					}
					
					rcSharedMemError smErr;
					
					rcVideoSharedMemLayout* vidDataBuf =
					(rcVideoSharedMemLayout*)(pMungData->shMem->acquireSharedMemory(smErr,
																																					false));
					if (!vidDataBuf) {
						if (smErr == rcSharedMemNotAvailable) {
							pMungData->acqInfo.missedFrames++;
							if (pMungData->acqInfo.missedFrames == 1) //xyzzy ditch me
								fprintf(stderr, "RCDCAM: Missed frame count (no shmem) %d\n",
												pMungData->acqInfo.missedFrames);
						}
						else {
							fprintf(stderr, "RCDCAM: Child: error acquiring shared data. err: %d\n", smErr);
							perror("Child: ");
							pMungData->stopProcess = 1;
						}
						return /*noErr*/;
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
					
					Ptr p = (Ptr) theCallBackData.ImageBufferPtr;

					if ( theCallbackDataPtr->bitsPerPixel == 8 )
					{
						if (pMungData->acqControl.resolution == rcAcqHalfResXY) {
							rcGenHalfRes((char*)p, vidDataBuf->rawPixels,
													 (pMungData->imgWidth)*(uint32)(pMungData->pixelDepth),
													 pMungData->imgHeight, pMungData->rowUpdate, 0);
						} else {
							copyToSharedMem( vidDataBuf,
															(const unsigned char*)p,
															pMungData->rowUpdate );
						}
					} else {
						if ( pMungData->imgBuf32 ) {
							// Must perform color format conversion
							OSStatus stat = ASCDCAMConvertRawFrameToARGB32( theCallbackDataPtr->theCameraPtr,
																														 theCallBackData.ImageBufferPtr,
																														 theCallbackDataPtr->theWidth,
																														 theCallbackDataPtr->theHeight,
																														 theCallbackDataPtr->fourCCType,
																														 theCallbackDataPtr->bitsPerPixel,
																														 pMungData->imgBuf32->rowPointer(0),
																														 pMungData->imgBuf32->rowUpdate() );
							if ( stat == 0 ) {
								if ( pMungData->extractLum ) {
									// Convert color image to gray scale
									const rcWindow rgbInput32( pMungData->imgBuf32 );
									rcWindow output8( pMungData->imgBuf8 );
									// 32-bit to 8-bit conversion
									rfRcWindow32to8( rgbInput32, output8, pMungData->imgConversion );
									copyToSharedMem( vidDataBuf,
																	output8.frameBuf()->rowPointer(0),
																	output8.frameBuf()->rowUpdate() );
								} else {
									// Transmit color image
									copyToSharedMem( vidDataBuf,
																	pMungData->imgBuf32->rowPointer(0),
																	pMungData->imgBuf32->rowUpdate() );
								}
							} else {
								cerr << "RCDAM: ASCDCAMConvertRawFrameToARGB32 error " << stat << endl;
							}
						} else {
							cerr << "RCDAM: error, no image buffer for RGB conversion" << endl;
						}
					}

					
					// Let processControlRequests process peer control requests, write
					// acqInfo to shared memory and release shared memory
					pMungData->acqInfo.acqFlags |= rcACQFLAGS_FRAMEAVAIL;
					processControlRequests(pMungData, true);
					pMungData->acqInfo.acqFlags = 0;
					pMungData->err = err;
				}
				
				theCallbackDataPtr->theErr = err;
				if (theCallbackDataPtr->shouldDisplay == true)
					theCallbackDataPtr->shouldDisplay = (err == 0);
				
			}
		} else
			fprintf( stderr, "RCDCAM: noDisplay\n" );
		if ( err != noErr )
			fprintf( stderr, "RCDCAM: yourDisplayCallback err %i\n", int(err) );
	}
}


static OSStatus useFormatMode( MungDataPtr pMungData,
															UInt32 format,
															UInt32 mode )
{
	double frameRate = 60.0; // as fast as possible
	void * theCameraPtr = pMungData->theCameraPtr;
	
	return ASCDCAMSetFMR(theCameraPtr,format,mode,frameRate);
}


static OSStatus useFormat7 ( void* theCameraPtr, UInt32 mode )
{

	OSStatus err = ASCDCAMSet7MR(theCameraPtr,mode, 0, 0, 0, 0, kMONO8_ID, 0);
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
	return err;
	
}


// Find 8-bit gray mode with largest size
static OSStatus useLargestAvailableMonoMode( void* theCameraPtr,
																						const vector<rcCameraFormat>& supportedFormats,
																						UInt32& format,
																						UInt32& mode,
																						double& frameRate,
																						uint32 maxWidth,
																						uint32 maxHeight )
{
  OSStatus err = rcDCAM_NO_MODE;
	
  if ( !supportedFormats.empty() )
	{
		frameRate = 60.0; // as fast as possible
		uint32 imax (supportedFormats.size());
		int32 areaMax (0);
		
		// Find (format & mode) that support largest area
		for ( uint32 i = 0; i < supportedFormats.size(); ++i )
		{
			const rcCameraFormat& f = supportedFormats[i];
			if ( f.isMono8() ) // Only 8bit is currently supported
	    {
	      // Cannot exceeed max size
	      if ( f.width() <= maxWidth && f.height() <= maxHeight )
				{
					int32 area = f.width() * f.height();
					format = f.format();
					mode = f.mode();
					err = ASCDCAMSetFMR(theCameraPtr, format, mode, frameRate);
					if (err == 0)
					{
						if (area > areaMax)
						{
							areaMax = area;
							imax = i;
						}
					}
				}
	    }
		}
		
		if (imax >= supportedFormats.size())
			err = rcDCAM_SIZE_EXCEEDED;
		
		const rcCameraFormat& f = supportedFormats[imax];
		format = f.format();
		mode = f.mode();
		err = ASCDCAMSetFMR(theCameraPtr, format, mode, frameRate);
		
		if ( err == 0 )
		{
			UInt32 theWidth;
			UInt32 theHeight;
			UInt32 bitsPerPixel;
			OSType fourCCType;
			UInt32 left;
			UInt32 top;
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
		}
		return err;
	}
  return err;
}


// Find color mode with largest suitable size
static OSStatus useLargestAvailableColorMode( void* theCameraPtr,
																						 const vector<rcCameraFormat>& supportedFormats,
																						 UInt32& format,
																						 UInt32& mode,
																						 double& frameRate,
																						 uint32 maxWidth,
																						 uint32 maxHeight )
{
	OSStatus err = rcDCAM_NO_MODE;
	
	if ( !supportedFormats.empty() ) {
		frameRate = 60.0; // as fast as possible
		
		for ( uint32 i = 0; i < supportedFormats.size(); ++i ) {
			const rcCameraFormat& f = supportedFormats[i];
			//cerr << "useLargestAvailableColorMode " << f << endl;
			if ( !f.isMono() ) {
				// Cannot exceeed max size
				if ( f.width() <= maxWidth && f.height() <= maxHeight ) {
					format = f.format();
					mode = f.mode();
					err = ASCDCAMSetFMR(theCameraPtr, format, mode, frameRate);
					if ( err == 0 ) {
						UInt32 theWidth;
						UInt32 theHeight;
						UInt32 bitsPerPixel;
						OSType fourCCType;
						UInt32 left;
						UInt32 top;
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
					}
					break;
				} else {
					err = rcDCAM_SIZE_EXCEEDED;
				}
			}
		}
	}
	
	return err;
}

static OSStatus SetUpDisplayCallback(MungDataPtr pMungData, yourProcessFrameCallBackPtr yPFCBPtr)
{
	OSStatus	err;
	UInt32 		format;
	UInt32 		mode;
	double		frameRateDbl;
	UInt32 		left;
	UInt32 		top;
	
	//initalize everything..
	yPFCBPtr->pMungData = pMungData;
	yPFCBPtr->theCameraPtr = pMungData->theCameraPtr;
	yPFCBPtr->shouldDisplay = true;
	yPFCBPtr->theErr = 0;
	pMungData->imgBuf32 = 0;
	pMungData->imgBuf8 = 0;
	
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
	if (err != 0) {
		cerr << "RCDCAM: ASCDCAMGetFMRLTWHCB error " << err << endl;
		return err;//bail
	}
	
#ifdef DEBUG_LOG
	const char* fourCC = (const char*)(&yPFCBPtr->fourCCType);
	fprintf( stderr, "RCDCAM: Format: %u mode: %u fps: %f rect: [%u,%u,%u,%u] pixType: %c%c%c%c depth: %u\n",
					
					uint32(format), uint32(mode), frameRateDbl,
					uint32(left), uint32(top), uint32(yPFCBPtr->theWidth), uint32(yPFCBPtr->theHeight),
					fourCC[0],
					fourCC[1],
					fourCC[2],
					fourCC[3],
					uint32(yPFCBPtr->bitsPerPixel) );
#endif
	
	//if your FW camera does bayer video for this format and mode, set isBayerData to true
	yPFCBPtr->isBayerData = false;
	//Change this to whatever kLQ_ASCBAYER_RGGB or RGGB pattern the FW Camera uses for the Bayer data..
	if (yPFCBPtr->isBayerData == true)
		yPFCBPtr->theDisplayFormat = kLQ_ASCBAYER_RGGB;
	else
		yPFCBPtr->theDisplayFormat = kASC_ARGB;
	
	// Read out the image description information
	pMungData->imgWidth = yPFCBPtr->theWidth;
	pMungData->imgHeight = yPFCBPtr->theHeight;
	pMungData->isGray = false;
	
	switch ( yPFCBPtr->bitsPerPixel ) {
		case 8:
			pMungData->pixelDepth = rcPixel8;
			break;
		case 16:
			// Will be converted to mono8
			if ( pMungData->extractLum )
				pMungData->pixelDepth = rcPixel8;
			else
				pMungData->pixelDepth = rcPixel16;
		case 24:
		case 32:
			// Will be converted to mono8
			if ( pMungData->extractLum )
				pMungData->pixelDepth = rcPixel8;
			else
				pMungData->pixelDepth = rcPixel32;
			break;
		default:
			pMungData->pixelDepth = rcPixelUnknown;
			break;
	}
	
	if ( pMungData->extractLum || (pMungData->pixelDepth == rcPixel8) )
		pMungData->isGray = true;
	pMungData->imgSizeInBytes = yPFCBPtr->theWidth * yPFCBPtr->theHeight * pMungData->pixelDepth;
	
	if ( yPFCBPtr->bitsPerPixel != 8 ) {
		// Buffer for RGB data
		pMungData->imgBuf32 = new rcFrame(yPFCBPtr->theWidth, yPFCBPtr->theHeight, rcPixel32);
		pMungData->imgBuf8 = new rcFrame(yPFCBPtr->theWidth, yPFCBPtr->theHeight, rcPixel8);
	}
	
	if (pMungData->imgSizeInBytes % (pMungData->imgHeight))
	{
		fprintf(stderr, "RCDCAM: Couldn't calculate row offset\n");
		return -1;
	}
	
	pMungData->rowUpdate = pMungData->imgSizeInBytes/(pMungData->imgHeight);
	
	return err;
}

OSStatus VDGETVIDEODEFAULTS(MungDataPtr pMungData,
                            uint32* gain,
                            uint32* shutter)
{
	OSStatus err = noErr;
	
	if ( gain ) {
		if ( pMungData->cameraProperties.hasGain ) {
			uint32 min, max;
			err = VDGETGAIN( pMungData, gain, &min, &max );
		} else
			*gain = 65535;
	}
	
	if ( shutter ) {
		if ( pMungData->cameraProperties.hasShutter ) {
			uint32 min, max;
			err = VDGETSHUTTER( pMungData, shutter, &min, &max );
		} else
			*shutter = 1000;
	}
	
	return err;
}

OSStatus VDGETGAIN (MungDataPtr pMungData,
                    uint32 *gain,
                    uint32 *minGain,
                    uint32 *maxGain)
{
	assert(gain);
	
	OSStatus err = noErr;
	
	if ( pMungData->cameraProperties.hasGain ) {
		UInt32 minVal, maxVal;
		Boolean canOnePush, canReadOut, canOnOff, canAuto, canManual;
		err = ASCDCAMGetPropertyCapability(
																			 pMungData->theCameraPtr,
																			 ASC_GAIN_REG,
																			 &minVal,
																			 &maxVal,
																			 &canOnePush,
																			 &canReadOut,
																			 &canOnOff,
																			 &canAuto,
																			 &canManual
																			 );
		if ( err != 0 ) {
			fprintf( stderr, "RCDCAM: ASCDCAMGetPropertyCapability error %i\n", int(err) );
		} else {
			*minGain = minVal;
			*maxGain = maxVal;
		}
		if ( canReadOut ) {
			UInt32 value = 65535;
			err = ASCDCAMGetPropertyValue( pMungData->theCameraPtr,
																		ASC_GAIN_REG,
																		MANUAL_CNTL,
																		&value );
#ifdef DEBUG_LOG
			cerr << "RCDCAM: ASCDCAMGetPropertyValue ASC_GAIN_REG " << value << endl;
#endif
			if ( err != 0 ) {
				fprintf( stderr, "RCDCAM: ASCDCAMGetPropertyCapability error %i\n", int(err) );
			} else
				*gain = value;
		}
	} else
		*gain = 65535;
	
	return err;
}

OSStatus VDSETGAIN (MungDataPtr pMungData,
                    uint32 *gain )
{
	assert(gain);
	
	OSStatus err = noErr;
	
	if ( pMungData->cameraProperties.hasGain ) {
		UInt32 minVal, maxVal;
		Boolean canOnePush, canReadOut, canOnOff, canAuto, canManual;
		err = ASCDCAMGetPropertyCapability(
																			 pMungData->theCameraPtr,
																			 ASC_GAIN_REG,
																			 &minVal,
																			 &maxVal,
																			 &canOnePush,
																			 &canReadOut,
																			 &canOnOff,
																			 &canAuto,
																			 &canManual
																			 );
		if ( err != 0 ) {
			fprintf( stderr, "RCDCAM: ASCDCAMGetPropertyCapability error %i\n", int(err) );
		}
		if ( canManual ) {
			UInt32 value = *gain;
			err = ASCDCAMSetPropertyValue( pMungData->theCameraPtr,
																		ASC_GAIN_REG,
																		MANUAL_CNTL,
																		value );
#ifdef DEBUG_LOG
			cout << "RCDCAM: ASCDCAMSetPropertyValue ASC_GAIN_REG " << value << endl;
#endif
			if ( err != 0 ) {
				fprintf( stderr, "RCDCAM: ASCDCAMSetPropertyValue error %i\n", int(err) );
			} else
				*gain = value;
		}
	}
	
	return err;
}

OSStatus VDGETSHUTTER (MungDataPtr pMungData,
                       uint32 *shutter,
                       uint32 *minShutter,
                       uint32 *maxShutter)
{
	assert(shutter);
	
	OSStatus err = noErr;
	
	if ( pMungData->cameraProperties.hasShutter ) {
		UInt32 minVal, maxVal;
		Boolean canOnePush, canReadOut, canOnOff, canAuto, canManual;
		err = ASCDCAMGetPropertyCapability(
																			 pMungData->theCameraPtr,
																			 ASC_SHUTTER_REG,
																			 &minVal,
																			 &maxVal,
																			 &canOnePush,
																			 &canReadOut,
																			 &canOnOff,
																			 &canAuto,
																			 &canManual
																			 );
		if ( err != 0 ) {
			fprintf( stderr, "RCDCAM: ASCDCAMGetPropertyCapability error %i\n", int(err) );
		} else {
			*minShutter = minVal;
			*maxShutter = maxVal;
		}
		if ( canReadOut ) {
			UInt32 value = 65535;
			err = ASCDCAMGetPropertyValue( pMungData->theCameraPtr,
																		ASC_SHUTTER_REG,
																		MANUAL_CNTL,
																		&value );
#ifdef DEBUG_LOG
			cout << "RCDCAM: ASCDCAMGetPropertyValue ASC_SHUTTER_REG " << value << endl;
#endif
			if ( err != 0 ) {
				fprintf( stderr, "RCDCAM: ASCDCAMGetPropertyCapability error %i\n", int(err) );
			} else
				*shutter = value;
		}
	} else
		*shutter = 65535;
	
	return err;
}

OSStatus VDSETSHUTTER (MungDataPtr pMungData,
                       uint32 *shutter )
{
	assert(shutter);
	
	OSStatus err = noErr;
	
	if ( pMungData->cameraProperties.hasGain ) {
		UInt32 minVal, maxVal;
		Boolean canOnePush, canReadOut, canOnOff, canAuto, canManual;
		err = ASCDCAMGetPropertyCapability(
																			 pMungData->theCameraPtr,
																			 ASC_SHUTTER_REG,
																			 &minVal,
																			 &maxVal,
																			 &canOnePush,
																			 &canReadOut,
																			 &canOnOff,
																			 &canAuto,
																			 &canManual
																			 );
		if ( err != 0 ) {
			fprintf( stderr, "RCDCAM: ASCDCAMGetPropertyCapability error %i\n", int(err) );
		}
		if ( canManual ) {
			UInt32 value = *shutter;
			err = ASCDCAMSetPropertyValue( pMungData->theCameraPtr,
																		ASC_SHUTTER_REG,
																		MANUAL_CNTL,
																		value );
#ifdef DEBUG_LOG
			cout << "RCDCAM: ASCDCAMSetPropertyValue ASC_SHUTTER_REG " << value << endl;
#endif
			if ( err != 0 ) {
				fprintf( stderr, "RCDCAM: ASCDCAMSetPropertyValue error %i\n", int(err) );
			} else
				*shutter = value;
		}
	}
	
	return err;
}



// --------------------
// InitializeMungData
//
MungDataPtr InitializeMungData(void* theCameraPtr)
{
	MungDataPtr pMungData = NULL;
	
	// allocate memory for the data
	pMungData = (MungDataPtr)NewPtrClear(sizeof(MungDataRecord));
	if (MemError() || NULL == pMungData )
		return NULL;
	
	pMungData->theCameraPtr = theCameraPtr;
	pMungData->acqInfo.cameraInfo = rcMovieFileCamExt();
	pMungData->acqInfo.acqState = rcAcqDisabled;
	
	pMungData->isGrabbing = false;
	pMungData->stopProcess = 0;
#ifdef INTERVAL_LOGGING
	pMungData->lastCaptureTime = cZeroTime;
#endif
	pMungData->initing = true;
	pMungData->err = noErr;
	
	return pMungData;
}

OSErr InitializeMungData2(MungDataPtr pMungData,
                          rcSharedMemoryUser* shMem)
{
	if ((pMungData == NULL) || (shMem == 0))
		return -1;
	
	pMungData->acqInfo.acqState = rcAcqUnknown;
	
	uint32 imgByteCnt;
	OSErr err = noErr;
	
	pMungData->acqInfo.curDecimationRate = 1;
	pMungData->acqInfo.curResolution = rcAcqFullResXY;
	pMungData->acqInfo.missedFrames = 0;
	pMungData->acqInfo.acqFlags = 0;
	pMungData->acqInfo.ctrlFlags = 0;
	pMungData->acqInfo.acqCtrlUpdateID = 0;
	
	pMungData->acqControl.doAcquire = false;
	pMungData->acqControl.resolution = rcAcqFullResXY;
	pMungData->acqControl.doShutdown = 0;
	
	pMungData->decCounter = 0;
	pMungData->acqControl.acqCtrlUpdateID = pMungData->acqInfo.acqCtrlUpdateID;
	
	yourProcessFrameCallBackPtr yPFCBPtr = NULL;
	
	// Convert all image data to 8-bit gray
	// If extractLum is false, 32-bit color images will be produced for all non-mono8 modes
	pMungData->extractLum = true;
	pMungData->imgConversion = rcSelectAverage;
	
	if (pMungData->theCameraPtr!=NULL)//we have it exclusively now..
	{
		OSStatus status = ASCDCAMGetCameraProperties( pMungData->theCameraPtr, &pMungData->cameraProperties);
		if (status!=0) {
			cerr << "RCDCAM: ASCDCAMGetCameraProperties error " << status << endl;
			goto bail;
		}
		
		UInt32 format = 0;
		UInt32 mode = 0;
		uint32 maxWidth = rcMAX_MONO_WIDTH;
		uint32 maxHeight = rcMAX_MONO_HEIGHT;
		double actualFrameRate = 0.0;

		cerr << " Setting Format 7 " << endl;
		
		OSStatus err = ASCDCAMSet7MR(pMungData->theCameraPtr,1, 0, 0, 0, 0, kMONO8_ID, 0);

		err = useFormat7 (pMungData->theCameraPtr, 1);

		if ( err != 0 )
		{
			rmExceptionMacro ("No Suitable Format Found");
		}
		
    if (err == 0)
		{
			// Update capabilities with mode-specific data
			rfGetCameraCapabilities( pMungData->theCameraPtr, pMungData->cameraProperties );
			rcCameraProperty gainProp = GetCameraProperty( pMungData->theCameraPtr,	ASC_GAIN_REG,MANUAL_CNTL );
			rcCameraProperty shutterProp = GetCameraProperty( pMungData->theCameraPtr, ASC_SHUTTER_REG, MANUAL_CNTL );
			if ( gainProp.active() || shutterProp.active() )
				cout << "Control parameters:" << endl;
			if ( gainProp.active() ) cout << "gain Property" << endl;
			if ( shutterProp.active() ) cout << "shutter Property"  << endl;
			
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
			
			yPFCBPtr = (yourProcessFrameCallBackPtr) NewPtrClear(sizeof(yourProcessFrameCallBack));
			err = MemError();
			if (err == 0)
				err = SetUpDisplayCallback(pMungData,yPFCBPtr);
		}
		
		if (err == 0) {
			imgByteCnt = pMungData->imgSizeInBytes;
			pMungData->rowUpdate = imgByteCnt/(pMungData->imgHeight);
			
			if (pMungData->acqControl.resolution == rcAcqHalfResXY)
				imgByteCnt /= 4;
#ifdef DEBUG_LOG
			fprintf(stderr, "RCDCAM: Init img info: width %d height %d pixelDepth %d row update %d"
							" pixel cnt %d isGray %d extract %d\n",
							pMungData->imgWidth, pMungData->imgHeight,
							(uint32)pMungData->pixelDepth, pMungData->rowUpdate,
							imgByteCnt, pMungData->isGray,
							pMungData->extractLum);
#endif
			pMungData->shMem = shMem;
			
			if (imgByteCnt > rcMAX_BYTES_PER_FRAME)
			{
				fprintf(stderr, "RCDCAM: Child: Image too large Sz: %d Max %d\n", imgByteCnt,
								rcMAX_BYTES_PER_FRAME);
				BailErr(-1);
			}
			
			pMungData->acqInfo.maxFramesPerSecond = rfGetCameraMaxFrameRate( pMungData->theCameraPtr );
			
#ifdef DEBUG_LOG
			cerr << "maxFrameRate " << pMungData->acqInfo.maxFramesPerSecond  << " actualFrameRate " << actualFrameRate << endl;
#endif
			if ( pMungData->acqInfo.maxFramesPerSecond <= 0.0 ) {
				// Camera does not specify any max frame rate, use actual rate
				pMungData->acqInfo.maxFramesPerSecond = actualFrameRate;
			}
			pMungData->acqInfo.cameraInfo.frameRate( pMungData->acqInfo.maxFramesPerSecond );
			pMungData->acqInfo.cameraInfo.format( format );
			pMungData->acqInfo.cameraInfo.mode( mode );
			pMungData->acqInfo.maxFrameWidth =  pMungData->imgWidth;
			pMungData->acqInfo.maxFrameHeight = pMungData->imgHeight;
			pMungData->acqInfo.cameraPixelDepth = pMungData->pixelDepth;
			err = rfGetCameraInformation( pMungData->theCameraPtr,
																	 pMungData->cameraProperties,
																	 pMungData->acqInfo.cameraInfo );
			if ( err != 0 )
				cerr << "RCDCAM: rfGetCameraInformation error " << err << endl;
#ifdef DEBUG_LOG
			cout << "INIT acqInfo:\n" << pMungData->acqInfo << endl;
			cout << "INIT acqCtrl:\n" << pMungData->acqControl << endl;
#endif
			//start the isoch stream
			err = ASCDCAMAllocateReleaseIsochChannel(pMungData->theCameraPtr,true);
			if (err == 0) {
				//Start the camera transmitting video..
				err = ASCDCAMStartStopVideo(pMungData->theCameraPtr,true);
				if (err == 0) {
					err = ASCDCAMAssignCallback(pMungData->theCameraPtr, yourDisplayCallback, yPFCBPtr);
					if (err==0)
						yPFCBPtr->shouldDisplay = true;
					else
						cerr << "RCDCAM: ASCDCAMAssignCallback error " << err << endl;
				} else {
					cerr << "RCDCAM: ASCDCAMStartStopVideo error " << err << endl;
				}
			} else {
				cerr << "RCDCAM: ASCDCAMAllocateReleaseIsochChannel error " << err << endl;
			}
		} else {
		}
		if ( err != 0 )
			goto bail;
	}
	
bail:
	if ( err != noErr ) {
		if (pMungData->theCameraPtr) {
			ASCDCAMStartStopVideo(pMungData->theCameraPtr,false);
			ASCDCAMAllocateReleaseIsochChannel(pMungData->theCameraPtr,false);
			ASCDCAMCloseACamera(pMungData->theCameraPtr);
		}
	}
	
	pMungData->acqInfo.acqState = rcAcqStopped;
	
	return err;
}

// --------------------
// DisposeMungData
//
static void DisposeMungData(MungDataPtr pMungData)
{
	// clean up the bits
	if(pMungData)
	{
		delete pMungData->imgBuf32;
		delete pMungData->imgBuf8;
		if (pMungData->theCameraPtr) {
#if 0
			if (yPFCBPtr != NULL)//clean up the callback data..
			{
				DisposePtr((Ptr) yPFCBPtr);
			}
#endif
			ASCDCAMStartStopVideo(pMungData->theCameraPtr,false);
			ASCDCAMAllocateReleaseIsochChannel(pMungData->theCameraPtr,false);
			ASCDCAMCloseACamera(pMungData->theCameraPtr);
		}
		
		DisposePtr((Ptr)pMungData);
		pMungData = NULL;
	}
}

// --------------------
// MGIdleTimer
//
// Munggrab idle timer to idle the sequence grabber, call this at least
// as much as the desired frame rate - more is better
static pascal void MGIdleTimer(EventLoopTimerRef, void *inUserData)
{
	MungDataPtr pMungData = MungDataPtr(inUserData);
	if (NULL == pMungData)
		return;
	
	if (pMungData->stopProcess)
	{
		//stop the camera
		if (pMungData->theCameraPtr) {
			ASCDCAMStartStopVideo(pMungData->theCameraPtr,false);
			ASCDCAMAllocateReleaseIsochChannel(pMungData->theCameraPtr,false);
			ASCDCAMCloseACamera(pMungData->theCameraPtr);
		}
		RemoveEventLoopTimer(pMungData->timerRef);
		QuitApplicationEventLoop();
	}
	
	if (pMungData->acqInfo.acqState == rcAcqStopped)
		processControlRequests(pMungData, false);
	else if ( pMungData->acqInfo.acqState == rcAcqRunning && pMungData->acqControl.doShutdown )
	{
		cerr << "MGIdleTimer: Visible requested shutdown" << endl;
	}
}

// --------------------
// InstallEventHandlers
//
// install carbon event handlers and timer
// you can find more information about carbon events here:
// http://developer.apple.com/techpubs/macosx/Carbon/carbon.html
// Http://developer.apple.com/sdk/index.html
static OSErr InstallEventHandlers(MungDataPtr inMungData)
{
	OSStatus err;
	
	err = InstallEventLoopTimer(GetMainEventLoop(), kEventDurationNoWait,
															kTimerInterval,
															NewEventLoopTimerUPP(MGIdleTimer), inMungData,
															&inMungData->timerRef);
	return err;
}

static MungDataPtr pMungData = NULL;

void rfDCAMVideoCapture(rcSharedMemoryUser& shMem)
{
	OSErr            err = noErr;
	OSStatus         status = 0;
	dcamCameraListStruct camera;
	void *theCameraPtr = 0;
	
#ifdef DEBUG_INIT
	cerr << "RCDCAM init 1";
#endif
	
	// Get list of cameras
	dcamCameraList theCameraList;
	status = ASCDCAMGetListOfCameras(&theCameraList);
	if ( status == -128 ) {
		// No license for this host
		cerr << "RCDCAM: no DCAM license installed for this host" << endl;
		BailErr(status);
	}
	if (status!= 0) {
		cerr << "RCDCAM: ASCDCAMGetListOfCameras error " << status << endl;
		BailErr(status);
	}
	
	if (theCameraList.numCamerasFound == 0) {
		cerr << "RCDCAM: No camera connected" << endl;
		BailErr(-1);
	}
#ifdef DEBUG_INIT
	fprintf(stderr, "...2");
#endif
	// Use first available camera
	camera = theCameraList.theCameras[0];
	// Display camera basic info
	
	theCameraPtr = ASCDCAMOpenACamera(camera.uniqueID);
	
	if ( !theCameraPtr ) {
		cerr << "RCDCAM: ASCDCAMOpenACamera failed" << endl;
		BailErr(-1);
	}
	
	
#ifdef DEBUG_INIT
	fprintf(stderr, "...3");
#endif
	// initialize our data that's going to be passed around
	pMungData = InitializeMungData(theCameraPtr);
	BailErr(NULL == pMungData);
#ifdef DEBUG_INIT
	fprintf(stderr, "...4");
#endif
	// install carbon event handlers
	err = InstallEventHandlers(pMungData);
	BailErr(err);
#ifdef DEBUG_INIT
	fprintf(stderr, "...5\n");
#endif
	// Now have values available to setup image copy specific values
	err = InitializeMungData2(pMungData, &shMem);
	BailErr(err);
	
	pMungData->acqInfo.cameraInfo.mid( int32(camera.manufacturerID) );
#ifdef DEBUG_INIT
	fprintf(stderr, "...6");
#endif
	// Let peer process know that we are ready to start acquiring frames
	if (!writeAcqInfo(shMem, pMungData->acqInfo, true))
		goto bail2; // Pretty ugly, huh?
	
#ifdef DEBUG_INIT
	fprintf(stderr, "...7");
#endif
	
	// run the application
	RunApplicationEventLoop();
	//stop the callback..
	ASCDCAMAssignCallback(pMungData->theCameraPtr, NULL,NULL);
	fprintf(stderr, "RCDCAM: RETURNED FROM EVENTLOOP\n");
	
#ifdef INTERVAL_LOGGING
	if (sHistogram.binsUsed() > 2) {
		uint32 min, max;
		sHistogram.range(min, max);
		fprintf(stderr, "RCDCAM: Irregular frame intervals, interval range %i-%i ms\n",
						min, max);
		const rcSparseHistogram::sparseArray& histMap = sHistogram.getArray();
		rcSparseHistogram::sparseArray::const_iterator start = histMap.begin();
		rcSparseHistogram::sparseArray::const_iterator end = histMap.end();
		
		for ( ; start != end; start++)
			fprintf(stderr, "Interval %d ms Count %d\n", (*start).first,
							(*start).second);
	}
	sHistogram.reset();
	sHistogramRange = 0;
#endif
	
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
			if ( err == -128 ) {
				// DCAM license error
				acqInfo.ctrlFlags |= rcCTRLFLAGS_ERRLIC;
			}
			acqInfo.acqState = rcAcqDisabled;
			ip = &acqInfo;
		}
		
		writeAcqInfo(shMem, *ip, true);
	}
	
bail2:
	// clean up
	printf( "\n" );
	
	DisposeMungData(pMungData);
	pMungData = NULL;
	didit = 0;
	fprintf(stderr, "RCDCAM: DONE\n");
	return;
}


