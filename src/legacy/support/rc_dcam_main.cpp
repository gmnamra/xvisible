/*
 *  rc_dcam_main.cpp
 *
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 */
#include <stdio.h>
#include <rc_dcam_capture.h>
#include <rc_resource_ctrl.h>
#include <rc_platform.h>



// Main for DCAM capture process

int main(int argc, char** argv)
{

  rcCreateChildShmem memoryCtrl(argc, argv);

  rfDCAMVideoCapture(memoryCtrl.getShmemUser());

  memoryCtrl.setChildDone(); // Let parent process know before returning

  return 0;
}
