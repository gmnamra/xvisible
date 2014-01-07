/*
 *  $Id: rc_captureregs.cpp 6505 2009-01-29 05:39:26Z arman $
 *
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 */

#include "rc_captureregs.h"

// Display utilities

ostream& operator << ( ostream& os, const rcVideoCaptureCtrl& c )
{
    os << " decimationRate " << c.getDecimationRate()
       << " slidingWindowSize " << c.getSlidingWindowSize()
       << " slidingWindowOrigin " << c.getSlidingWindowOrigin()
       << " slidingWindowEnabled " << c.getSlidingWindowEnabled()
       << " aciEnabled " << c.getACIEnabled()
       << " entropyDef " << c.getEntropyDefinition()
       << " gain " << c.getGain()
       << " shutter " << c.getShutter()
       << " shutter " << c.getBinning()
       << endl;
    
    return os;
}
