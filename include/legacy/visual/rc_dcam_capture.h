// Copyright 2003-2004 Reify, Inc.

#ifndef _rcDCAM_CAPTURE_H_
#define _rcDCAM_CAPTURE_H_

#include <rc_gen_capture.h>
#include <rc_capture.hpp>

class rcSharedMemoryUser;

void rfDCAMVideoCapture(rcSharedMemoryUser& shMem);

rcCameraMapper rf_dcam_get_cameras ();





#endif // _rcDCAM_CAPTURE_H_


