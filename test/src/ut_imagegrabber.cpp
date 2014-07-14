// Copyright (c) 2002 Reify Corp. All rights reserved.

#include <rc_framebuf.h>

#include "ut_imagegrabber.h"

UT_Imagegrabber::UT_Imagegrabber()
{
}

UT_Imagegrabber::~UT_Imagegrabber()
{
    printSuccessMessage( "rcImageGrabber test", mErrors );
}

uint32
UT_Imagegrabber::run() {

    // Failure test
    {
        std::string s1;
        vector<std::string> v1;

        v1.push_back( s1 );
        
        rcImageGrabber i1( v1, NULL );

        // Virtually everything should fail
        rcUNITTEST_ASSERT( i1.isValid() == false );
        rcUNITTEST_ASSERT( i1.getLastError() != eFrameErrorOK );
        rcUNITTEST_ASSERT( i1.start() == false );
        rcUNITTEST_ASSERT( i1.getLastError() != eFrameErrorOK );
        rcUNITTEST_ASSERT( i1.frameCount() == -1 );
        rcUNITTEST_ASSERT( i1.stop() == true ); // Since nothing was done stop() succeeds
        
        rcFrameRef ptr = 0;
        rcUNITTEST_ASSERT( i1.getNextFrame( ptr, true ) != eFrameStatusOK );
        // Non-blocking mode is not implemented yet
        // Non-blocking mode should fail, too
        rcUNITTEST_ASSERT( i1.getNextFrame( ptr, false ) != eFrameStatusOK );
        rcUNITTEST_ASSERT( i1.getLastError() == eFrameErrorNotImplemented );
    }

    return mErrors;
}
