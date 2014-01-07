/****************************************************************************
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 *   $Id: ut_capture.cpp 4391 2006-05-02 18:40:03Z armanmg $
 *
 ***************************************************************************/

#include <ut_capture.h>

// public

UT_capture::UT_capture()
{
}

UT_capture::~UT_capture()
{
  printSuccessMessage( "Capture tests", mErrors );
}

uint32 UT_capture::run()
{
    testCameraMapper();
        
    return mErrors;
}

// private


// Test camera spec mapper
void UT_capture::testCameraMapper()
{
	rcCameraMapper cm;
	cm.Fill_dcam_info ();
	
	
 }

