// Copyright 2003-2004 Reify, Inc.

#ifndef _rcQCAM_CAPTURE_H_
#define _rcQCAM_CAPTURE_H_

#include <QCam/QCamApi.h>
#include <QCam/QImgTypes.h>
#include <rc_gen_capture.h>

class rcSharedMemoryUser;

void rfQCAMVideoCapture(rcSharedMemoryUser& shMem);

/* Allow for the size of QCAM cameras. This should allow for two more
 * buffers of the same size, while still fitting within 4MB space
 * allowed by OS X using shmem calls.
 */
#define rcMAX_MONO_WIDTH 1392 // Max for 8-bit images
#define rcMAX_MONO_HEIGHT 1040 // Max for 8-bit images
#define rcMAX_COLOR_WIDTH (rcMAX_MONO_WIDTH/2)   // Max for 32-bit images
#define rcMAX_COLOR_HEIGHT (rcMAX_MONO_HEIGHT/2) // Max for 32-bit images

#define rcMAX_BYTES_PER_FRAME (rcMAX_MONO_WIDTH*rcMAX_MONO_HEIGHT)

typedef struct rcVideoSharedMemLayout
{
  QCam_Frame frame;
  char          rawPixels[rcMAX_BYTES_PER_FRAME];
  uint32      width;
  uint32      height;
  uint32      rowUpdate;
  bool        isGray;
  rcPixel pixelDepth;
  rcTimestamp   timestamp;
  rcAcqInfo     videoState;
  rcAcqControl  videoControl;
} rcVideoSharedMemLayout;

// Display utilities
ostream& operator << ( ostream& os, const struct rcAcqControl& c );
ostream& operator << ( ostream& os, const struct rcAcqInfo& c );

#endif // _rcQCAM_CAPTURE_H_
