/*
 *  rc_qtime_main.cpp
 *
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */
#include <stdio.h>
#include <rc_qtime_capture.h>
#include <rc_resource_ctrl.h>

int main(int argc, char** argv)
{
  fprintf(stderr, "RCQTIME: file: %s\n", argv[0]);
  rcCreateChildShmem memoryCtrl(argc, argv);

  rfQTVideoCapture(memoryCtrl.getShmemUser());

  memoryCtrl.setChildDone(); // Let parent process know before returning

  return 0;
}
