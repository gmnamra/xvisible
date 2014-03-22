/*
 *  rc_imagegrabber.h
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcIMAGEGRABBER_H_
#define _rcIMAGEGRABBER_H_

#include <vector>
//#include <QuickTime/QuickTimeComponents.h> // for GraphicsImportComponent support

#include <rc_framegrabber.h>
#include <rc_filegrabber.h>

using namespace std;

//
// Class to grab frames from image files
//

class rcImageGrabber : public rcFileGrabber {
  public:
    // ctor
    rcImageGrabber( const vector<std::string>& fileNames,
                    rcCarbonLock* lock,
                    double frameInterval = 0.0, // Forced frame interval in seconds (calls setTimestamp())
                    bool nameSort = true);
    // virtual dtor
    virtual ~rcImageGrabber();

    //
    // rcFrameGrabber API
    //
    
    // Start grabbing
    virtual bool start();

    // Stop grabbing
    virtual bool stop();
    
    // Returns the number of frames available
    virtual int32 frameCount();

    // Image grabbers don't have a cache.
    virtual int32 cacheSize() { return 0; }

    // Get next frame, assign the frame to ptr
    virtual rcFrameGrabberStatus getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking );

    // Get name of input source, ie. file name, camera name etc.
    virtual const std::string getInputSourceName();
    
  private:
    vector<std::string>        mFileNames; 
    vector<FSSpec*>         mFileHandles;
    //    GraphicsImportComponent mImporter;
    uint32                mCurrentIndex; // Current index to mFileHandles
    double                  mFrameInterval;    // Used to force a fixed frame interval
    rcTimestamp             mCurrentTimeStamp; // Current frame timestamp
};

#endif // _rcIMAGEGRABBER_H_

