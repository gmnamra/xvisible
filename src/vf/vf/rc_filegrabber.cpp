/*
 *  rc_filegrabber.cpp
 *
 *  Created by Sami Kukkonen on 10/23/2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include "rc_filegrabber.h"

//
// rcFileGrabber implementation. This is a base class for file-based frame grabbers.
// It is not intended to  be used as a standalone class.
//

// public

rcFileGrabber::rcFileGrabber( rcCarbonLock* lock ) :
        mLastError( rcFrameGrabberError::eFrameErrorOK ),
        mCarbonLock( lock )
{
}

// Nothing to be done
rcFileGrabber::~rcFileGrabber()
{
}

// Get last error value.
rcFrameGrabberError rcFileGrabber::getLastError() const
{
    return mLastError;
}

// Returns grabber instance validity
bool rcFileGrabber::isValid() const
{
    if ( getLastError() == rcFrameGrabberError::eFrameErrorOK  )
        return true;
    else
        return false;
}

// protected

// Set last error value
void rcFileGrabber::setLastError( rcFrameGrabberError error ) 
{
    mLastError = error;
}

// Lock
void rcFileGrabber::lock()
{
    if ( mCarbonLock )
        mCarbonLock->lock();
}

// Unlock
void rcFileGrabber::unlock()
{
    if ( mCarbonLock )
        mCarbonLock->unlock();
}
