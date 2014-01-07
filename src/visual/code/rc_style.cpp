/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_style.cpp 89 1970-01-01 00:03:19Z sami $
 *
 * Implementation for graphics style class
 *
 *****************************************************************************/

#include <rc_framebuf.h> // For rfRgb()
#include <rc_style.h>

//
// Class rcStyle implementation
//

// public

rcStyle::rcStyle() :
        mColor( rfRgb( 255, 0, 0 ) ), mLineWidth( 1 ), mPixelOrigin( 0.0f, 0.0f )
{
}

rcStyle::rcStyle( uint32 color ) :
        mColor( color ), mLineWidth( 1 ), mPixelOrigin( 0.0f, 0.0f )
{
}

rcStyle::rcStyle( uint32 color, uint32 lineWidth ) :
        mColor( color ), mLineWidth( lineWidth ), mPixelOrigin( 0.0f, 0.0f )
{
}

rcStyle::rcStyle( uint32 color, uint32 lineWidth, const rc2Fvector& origin ) :
        mColor( color ), mLineWidth( lineWidth ), mPixelOrigin( origin )
{
}

