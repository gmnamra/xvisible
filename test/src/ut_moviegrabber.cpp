// Copyright (c) 2002 Reify Corp. All rights reserved.

#include <rc_framebuf.h>

#include "ut_moviegrabber.h"

UT_Moviegrabber::UT_Moviegrabber()
{
}

UT_Moviegrabber::~UT_Moviegrabber()
{
    printSuccessMessage( "rcMovieGrabber test", mErrors );
}

uint32
UT_Moviegrabber::run() {

    // Failure test
    {
        std::string s1;

        rcMovieGrabber i1( s1, NULL );

        // Virtually everything should fail
        rcUNITTEST_ASSERT( i1.isValid() == false );
        rcUNITTEST_ASSERT( i1.getLastError() != eFrameErrorOK );
        rcUNITTEST_ASSERT( i1.start() == false );
        rcUNITTEST_ASSERT( i1.getLastError() != eFrameErrorOK );
        rcUNITTEST_ASSERT( i1.frameCount() == -1 );
        rcUNITTEST_ASSERT( i1.stop() == true ); // Since nothing was done stop() succeeds
        
        rcSharedFrameBufPtr ptr = 0;
        // Blocking mode should fail
        rcUNITTEST_ASSERT( i1.getNextFrame( ptr, true ) != eFrameStatusOK );
        // Non-blocking mode should fail, too
        rcUNITTEST_ASSERT( i1.getNextFrame( ptr, false ) != eFrameStatusOK );
        rcUNITTEST_ASSERT( i1.getLastError() == eFrameErrorNotImplemented );
    }

    return mErrors;
}
