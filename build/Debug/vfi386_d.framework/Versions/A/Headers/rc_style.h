/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_style.h 6271 2009-01-16 04:11:38Z arman $
 *
 * Definitions for graphics style class
 *
 *****************************************************************************/

#ifndef _rcSTYLE_H_
#define _rcSTYLE_H_

#include <rc_line.h>

// Drawing style, used by drawable graphics objects
class rcStyle {
  public:
    // Ctors
    rcStyle();
    rcStyle( uint32 color );
    rcStyle( uint32 color, uint32 lineWidth );
    rcStyle( uint32 color, uint32 lineWidth, const rc2Fvector& origin );
    
    // Accessors
    uint32 color() const { return mColor; };
    uint32 lineWidth() const { return mLineWidth; };
    rc2Fvector pixelOrigin() const { return mPixelOrigin; };
    
    // Mutators
    void color( uint32 color ) { mColor = color; };
    void lineWidth( uint32 lineWidth ) { mLineWidth = lineWidth; };
    void pixelOrigin( rc2Fvector origin ) { mPixelOrigin = origin; };

    // Operators
    bool operator==(const rcStyle& v) const
        { return (mColor == v.color() && mLineWidth == v.lineWidth() && mPixelOrigin == v.pixelOrigin()); }
    bool operator!=(const rcStyle& v) const { return !(*this == v); }

  protected:
    // TODO: add other attributes like brush style etc.
    
    uint32   mColor;       // RGB color, component (r,g,b) value range is 0-255
    uint32   mLineWidth;   // Line width in display pixels
    rc2Fvector mPixelOrigin; // Pixel origin for sub-pixel drawing
};

#endif // _rcSTYLE_H_
