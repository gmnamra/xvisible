// Copyright 2002 Reify, Inc.

#include "ut_rect.h"
#include <rc_window.h>

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

UT_Rect::UT_Rect()
{
}

UT_Rect::~UT_Rect()
{
    printSuccessMessage( "rcRect test", mErrors );
}

uint32
UT_Rect::run() {

    // Test ctors and accessors
    {
        // Dummy object, everything should be undefined
        rcRect rect;

        rcUNITTEST_ASSERT( rect.x() == 0 );
        rcUNITTEST_ASSERT( rect.y() == 0 );
        rcUNITTEST_ASSERT( rect.height() == 0 );
        rcUNITTEST_ASSERT( rect.width() == 0 );
        rcUNITTEST_ASSERT( rect.empty() );
    }
    {
        int32 x = 1;
        int32 y = 2;
        int32 w = 44;
        int32 h = 66;

        rcRect rect( x, y, w, h );

        rcUNITTEST_ASSERT( rect.x() == x );
        rcUNITTEST_ASSERT( rect.y() == y );
        rcUNITTEST_ASSERT( rect.height() == h );
        rcUNITTEST_ASSERT( rect.width() == w );
        rcUNITTEST_ASSERT( !rect.empty() );
        
        rcRect rect2( x+7, y, w, h );
        rcRect rect3( x+7, y, w, h );
        
        rcUNITTEST_ASSERT( !(rect == rect2) );
        rcUNITTEST_ASSERT( rect2 == rect3 );

    }
    
    // Test mutators
    {
        int32 x = 1;
        int32 y = 7;
        int32 w = 44;
        int32 h = 66;

        rcRect rect( x, y, w, h );
        rcRect rect2( x, y, w, h );
        
        rect.setX( x + 1 );
        rcUNITTEST_ASSERT( rect.x() == (x + 1) );
        rcUNITTEST_ASSERT( !(rect == rect2) );
        
        rect.setY( y + 1 );
        rcUNITTEST_ASSERT( rect.y() == (y + 1) );
        rcUNITTEST_ASSERT( !(rect == rect2) );
        
        rect.setWidth( w + 1 );
        rcUNITTEST_ASSERT( rect.width() == (w + 1) );
        rcUNITTEST_ASSERT( !(rect == rect2) );
        
        rect.setHeight( h + 1 );
        rcUNITTEST_ASSERT( rect.height() == (h + 1) );
        rcUNITTEST_ASSERT( !(rect == rect2) );
    }

    return mErrors;
}
    
