// Copyright 2002 Reify, Inc.

#ifndef _rcPOINT_H_
#define _rcPOINT_H_

#include "rc_types.h"

// Point class

class  rcPoint {
  public:
    rcPoint( uint32 x, uint32 y ) : mX( x ), mY( y ) { };

    const uint32& x() const { return mX; }
    const uint32& y() const { return mY; }

    uint32& x() { return mX; }
    uint32& y() { return mY; }
    
  private:
    uint32 mX;
    uint32 mY;
};

#endif // _rcPOINT_H_

