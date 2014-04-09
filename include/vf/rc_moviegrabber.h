/*
 *  rc_moviegrabber.h
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcMOVIEGRABBER_H_
#define _rcMOVIEGRABBER_H_

#include <vector>

#include <rc_qtime.h>
#include <rc_framegrabber.h>
#include <rc_filegrabber.h>


using namespace std;



//
// Class to grab frames from a movie file
// in seconds (calls setTimestamp())
//

class rcMovieGrabber : public rcFileGrabber {
  public:
    // ctor
    rcMovieGrabber( const std::string fileName,   // Input file
                    rcCarbonLock* lock,        // Carbon locker
                    double frameInterval = 0.0, // Forced frame interval 
		    int32  startAfterFrame = -1, 
		    int32  frames = -1 );
    // virtual dtor
    virtual ~rcMovieGrabber();

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
    void getPixelInfoFromImageDesc (ImageDescriptionHandle anImageDesc);
    OSErr getNextVideoSample(rcWindow& image, Media media, TimeValue fromTimePoint,
			     bool reduceGrayTo8bit);

    const std::string          mFileName;
    FSSpec                  mFileSpec;
    GraphicsImportComponent mImporter;
    int32                 mFrameCount;       // Number of frames in a movie
    short                   mRefnum;           // Movie ref num
    Movie                   mMovie;
    Media                   mMedia;
    int32                 mMovieStartFrame;      
    int32                 mClipFrames;
    TimeValue               mTimeScale;        // Movie time scale
    TimeValue               mCurrentTime;      // Current time within movie
    int32                 mCurrentIndex;     // Current index within movie
    double                  mFrameInterval;    //  force a fixed frame interval
    rcTimestamp             mCurrentTimeStamp; // Current frame timestamp
    std::vector<TimeValue>  mFrameTimes;
    // our format from QuickTime definitions
    rcPixel m_pd; 
    uint32 m_pf;
    CTabHandle m_ctb;
    bool m_isGray;
    bool m_isWhiteReversed;
	::Rect m_qt_movie_rect;
	int16 m_width, m_height;
};



#endif // _rcMOVIEGRABBER_H_

