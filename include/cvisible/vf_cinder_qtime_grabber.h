#ifndef _rcCINDERGRABBER_H_
#define _rcCINDERGRABBER_H_
#pragma once

#include "cinder/Surface.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/ImageIo.h"

#include <vfi386_d/rc_framegrabber.h>
#include <vfi386_d/rc_filegrabber.h>

using namespace std;
using namespace ci;



//
// Class to grab frames from a movie file
// in seconds (calls setTimestamp())
//


class rcCinderGrabber : public rcFileGrabber {
public:
    // ctor
    rcCinderGrabber( const std::string fileName,   // Input file
                   rcCarbonLock* lock,        // Carbon locker
                   double frameInterval = 0.0, // Forced frame interval 
                   int32  startAfterFrame = -1, 
                   int32  frames = -1 );
    // virtual dtor
    virtual ~rcCinderGrabber();
    bool isValid ();
    
    //
    // rcFrameGrabber API
    //
    
    // Start grabbing
    virtual bool start();
    
    // Stop grabbing
    virtual bool stop();
    
    // Returns the number of frames available
    virtual int32 frameCount();
    
    // Movie grabbers don't have a cache.
    virtual int32 cacheSize() { return 0; }
    
    // Get next frame, assign the frame to ptr
    virtual rcFrameGrabberStatus getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking );
    
    // Get name of input source, ie. file name, camera name etc.
    virtual const std::string getInputSourceName();
    
private:
    
    ci::qtime::MovieSurface	    mMovie;
	Surface				mSurface;
    bool                mValid;
    const std::string          mFileName;
    int32                 mFrameCount;       // Number of frames in a movie
    TimeValue               mTimeScale;        // Movie time scale
    TimeValue               mCurrentTime;      // Current time within movie
    int32                 mCurrentIndex;     // Current index within movie
    bool                    mGotFirstFrame;    // true after first frame captured
    double                  mFrameInterval;    //  force a fixed frame interval
    rcTimestamp             mCurrentTimeStamp; // Current frame timestamp
    std::vector<TimeValue>  mFrameTimes;
    // our format from QuickTime definitions
    bool m_isGray;
    bool m_isWhiteReversed;
	int16 m_width, m_height;
};




#endif // _rcCINDERGRABBER_H_

