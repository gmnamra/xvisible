/*
 *  rc_filegrabber.h
 *
 *  Created by Sami Kukkonen on 10/23/2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcFILEGRABBER_H_
#define _rcFILEGRABBER_H_

#include "rc_framegrabber.h"

using namespace std;

//
// Base class for file-based frame grabbing
//

class rcFileGrabber : public rcFrameGrabber {
  public:
    // ctor
    rcFileGrabber( rcCarbonLock* lock );
    // virtual dtor
    virtual ~rcFileGrabber();

    //
    // rcFrameGrabber API
    //

    // Get last error
    virtual rcFrameGrabberError getLastError() const;
    
    // Returns instance validity
    virtual bool isValid() const;

    // Start grabbing
    virtual bool start() = 0;

    // Stop grabbing
    virtual bool stop() = 0;
    
    // Returns the number of frames available
    virtual int32 frameCount() = 0;

    // Get next frame, assign the frame to ptr
    virtual rcFrameGrabberStatus getNextFrame( rcFrameRef& , bool isBlocking ) = 0;

  protected:
    // Set last error
    void setLastError( rcFrameGrabberError );
    // Lock mCarbonLock
    void lock();
    // Unlock mCarbonLock
    void unlock();
    
  private:
    rcFrameGrabberError     mLastError;     // Last encountered error
    rcCarbonLock*           mCarbonLock;    // Carbon/QuickTime lock
};

#endif // _rcFILERABBER_H_

