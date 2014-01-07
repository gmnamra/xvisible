// Copyright 2002 Reify, Inc.

#ifndef _rcQTIME_CAPTURE_H_
#define _rcQTIME_CAPTURE_H_

#include <rc_types.h>
#include <rc_framebuf.h>
#include <rc_atomic.h>
#include <rc_gen_capture.h>

class rcSharedMemoryUser;

void rfQTVideoCapture(rcSharedMemoryUser& shMem);

#endif // _rcQTIME_CAPTURE_H_
