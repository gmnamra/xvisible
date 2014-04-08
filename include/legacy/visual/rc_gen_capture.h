/*
 *
 *$Header $
 *$Id $
 *$Log: $
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcGEN_CAPTURE_H
#define __rcGEN_CAPTURE_H

#include <Carbon/Carbon.h>
#include <rc_types.h>
#include <rc_framebuf.h>
#include <rc_atomic.h>
#include <rc_moviefileformat.h>
#include <rc_ipconvert.h>


enum rcCameraType {
    rcCameraGreyScale = 0,
    rcCameraColor,
    rcCameraNone,          // No camera is attached
    rcCameraBayer,
    rcCameraUnknown        // No information yet about camera availability
};

enum rcAcqState {
    rcAcqRunning = 0,
    rcAcqStopped,
    rcAcqDown,
    rcAcqUnknown,
    rcAcqDisabled
};

enum rcAcqResolution {
  rcAcqFullResXY = 0,
  rcAcqBinBy2 = 1,
  rcAcqBinBy4 = 2,
  rcAcqBinBy8 = 3,
  rcAcqHalfResXY
};

/* Defines settings for rcAcqInfo acqFlags field.
 */
#define rcACQFLAGS_FRAMEAVAIL   0x0001
#define rcACQFLAGS_FRAMEERROR   0x0002
#define rcACQFLAGS_FRAMESKIPPED 0x0004

#define rcACQFLAGS_FRAMEERR     (rcACQFLAGS_FRAMEERROR |  \
                                  rcACQFLAGS_FRAMESKIPPED)

/* Defines settings for rcAcqInfo ctrlFlags field.
 */
#define rcCTRLFLAGS_CTRL      0x0001
#define rcCTRLFLAGS_ERRINIT   0x0002
#define rcCTRLFLAGS_ERRPAUSE  0x0004
#define rcCTRLFLAGS_ERRRES    0x0008
#define rcCTRLFLAGS_ERRDEC    0x0010
#define rcCTRLFLAGS_ERRLIC    0x0020 // QCAM license error

#define rcCTRLFLAGS_ERRGAIN    0x0040
#define rcCTRLFLAGS_ERRSHUTTER 0x0080
#define rcCTRLFLAGS_ERRBINNING 0x00a0

#define rcCTRLFLAGS_ERRCTRL   (rcCTRLFLAGS_ERRSHUTTER | rcCTRLFLAGS_ERRGAIN | \
                                rcCTRLFLAGS_ERRPAUSE | rcCTRLFLAGS_ERRINIT | \
                                rcCTRLFLAGS_ERRRES | rcCTRLFLAGS_ERRDEC | \
                                rcCTRLFLAGS_ERRLIC)


/* Structure that contains information about the current state of the
 * acquisition system, as determined by the acquisition process.
 */
class rcAcqInfo
{
  public:
  rcAcqInfo ()
    : cameraType ( rcCameraGreyScale), maxFramesPerSecond (0.0), maxFrameWidth (0), maxFrameHeight (0),
      cameraPixelDepth (rcPixel8),
	  defaultGain (0), defaultShutter (0), defaultBinning (1), minGain (0), minShutter (0), minBinning (1), maxGain (0),
      maxShutter (0), maxBinning (8), curGain (0), curShutter (0), curBinning (1),
	  curDecimationRate (1), curResolution (  rcAcqFullResXY ), acqState ( rcAcqUnknown),
      missedFrames (0), acqFlags (0), ctrlFlags (0), acqCtrlUpdateID (0)
  {}

    rcCameraType  cameraType;
    double        maxFramesPerSecond;
    uint32      maxFrameWidth;      // in pixels
    uint32      maxFrameHeight;     // in rows
    rcPixel cameraPixelDepth;
	Str255  cameraName;
    int32      defaultGain;
    int32      defaultShutter;
  int32      defaultBinning;
    int32      minGain;
    int32      minShutter;
  int32      minBinning;
    int32      maxGain;
    int32      maxShutter;
  int32      maxBinning;
    int32      curGain;
    int32      curShutter;
  int32      curBinning;

    uint32      curDecimationRate;  // use 1 out of curDecimationRate frames
    rcAcqResolution curResolution;
    rcAcqState    acqState;
    uint32      missedFrames;
    uint32      acqFlags;
    uint32      ctrlFlags;
    uint32      acqCtrlUpdateID;   // ID of last acq control request accepted
   rcMovieFileCamExt cameraInfo;
};

/* Structure that contains setting requests by the application to
 * be acted upon by the acquisition process.
 */
typedef struct rcAcqControl
{
    bool          doAcquire;
    rcAcqResolution resolution;
    int32        gain;
    int32        shutter;
		int32        binning;
    uint32        decimationRate;    // Acquire 1 out of decimationRate frames
    uint32        doShutdown;        // To tell quicktime to kill itself
    uint32        acqCtrlUpdateID;   // ID of this acq control request

} rcAcqControl;

// Display utilities
ostream& operator << ( ostream& os, const struct rcAcqControl& c );
ostream& operator << ( ostream& os, const struct rcAcqInfo& c );


/* Allow for the size of Sony SX900. This should allow for two more
 * buffers of the same size, while still fitting within 4MB space
 * allowed by OS X using shmem calls.
 */
#define rcMAX_MONO_WIDTH 1280 // Max for 8-bit images
#define rcMAX_MONO_HEIGHT 960 // Max for 8-bit images
#define rcMAX_COLOR_WIDTH (rcMAX_MONO_WIDTH/2)   // Max for 32-bit images
#define rcMAX_COLOR_HEIGHT (rcMAX_MONO_HEIGHT/2) // Max for 32-bit images

#define rcMAX_BYTES_PER_FRAME (rcMAX_MONO_WIDTH*rcMAX_MONO_HEIGHT)

typedef struct rcVideoSharedMemLayout
	{
    char          rawPixels[rcMAX_BYTES_PER_FRAME];
    uint32      width;
    uint32      height;
    uint32      rowUpdate;
    rcPixel pixelDepth;
    bool        isGray;
    rcTimestamp   timestamp;
    rcAcqInfo     videoState;
    rcAcqControl  videoControl;
	} rcVideoSharedMemLayout;



#endif /* __rcGEN_CAPTURE_H */
