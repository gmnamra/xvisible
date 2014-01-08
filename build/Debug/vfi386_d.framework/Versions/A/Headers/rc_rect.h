/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_rect.h 6239 2009-01-15 06:56:26Z arman $
 *
 *****************************************************************************/

#ifndef _rcRECT_H_
#define _rcRECT_H_

#include <rc_types.h>
#include <rc_rectangle.h>

// Rectangle geometry class
// It is drived from the template class rcRectangle.

class rcRect : public rcIRect
{
public:
   
  rcRect () : rcIRect () {}
  
  rcRect( int32 x, int32 y, int32 width, int32 height ) 
    : rcIRect (x, y, width, height) {}

  rcRect( const rcIRect& other)
    : rcIRect (other) {}

#ifdef rcQUICKTIME_RECT_COMPATIBILITY            
    // Construct our Rect from QuickDraw/Time's Rect
  rcRect(Rect bounds)
    : rcIRect (bounds.left, bounds.top, (bounds.right - bounds.left), (bounds.bottom - bounds.top)) {}
#endif
    
    // Accessors
  int32 x() const { return mUpperLeft.x(); }
  int32 y() const { return mUpperLeft.y(); }

  // Emptiness test
  bool empty() const { return isNull (); }

#ifdef rcQUICKTIME_RECT_COMPATIBILITY        
    void bound (Rect& bounds) const {
      bounds.right = short (x() + width());
      bounds.top = short (y() + height());
      bounds.left = short (x());
      bounds.bottom = short (y() + height()); }
#endif
    
    // Legacy Mutators: Use rcRectangle Instead
    int32 setX( int32 x ) { mUpperLeft.x() = x; return x; }
    int32 setY( int32 y ) { mUpperLeft.y() = y; return y; }
    int32 setWidth( int32 w ) { width(w); return w; }
    int32 setHeight( int32 h ) { height(h); return h; }

};

#endif // _rcRECT_H_
