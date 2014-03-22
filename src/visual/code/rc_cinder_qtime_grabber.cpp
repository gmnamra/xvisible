

#include "rc_cinder_qtime_grabber.h"
#include <cinder/qtime/QuickTime.h>
#include <stlplus_lite.hpp>


using namespace ci;
using namespace ci::qtime;


#include <rc_ipconvert.h>

#define kBogusStartingTime  -1      // an invalid starting time
#define Debug_Log

//
// rcCinderGrabber implementation
//

rcCinderGrabber::rcCinderGrabber( const std::string fileName, rcCarbonLock* cLock, 
                               double frameInterval, int32 startFrame, int32 frames) :
rcFileGrabber( cLock ),
mFileName( fileName ),
mFrameCount(-1),mTimeScale( 0 ), mCurrentTime( kBogusStartingTime ), mCurrentIndex( 0 ), mGotFirstFrame( false ),
mFrameInterval( frameInterval ), mCurrentTimeStamp( frameInterval )
{
    mValid = file_exists ( fileName ) && file_readable ( fileName );
    if (isValid () )
    {
        mMovie = MovieSurface( fileName );
        m_width = mMovie.getWidth ();
        m_height = mMovie.getHeight ();
        mFrameCount = mMovie.getNumFrames();
        mMovie.setLoop( true, true );
        
        std::cerr << "Dimensions:" << mMovie.getWidth() << " x " << mMovie.getHeight() << std::endl;
        std::cerr << "Duration:  " << mMovie.getDuration() << " seconds" << std::endl;
        std::cerr << "Frames:    " << mMovie.getNumFrames() << std::endl;
        std::cerr << "Framerate: " << mMovie.getFramerate() << std::endl;
        std::cerr << "Alpha channel: " << mMovie.hasAlpha() << std::endl;		
        std::cerr << "Has audio: " << mMovie.hasAudio() << " Has visuals: " << mMovie.hasVisuals() << std::endl;
    }
    else
    {
       setLastError( eFrameErrorFileInit );
    }
    

}

bool rcCinderGrabber::isValid ()  { return mValid; }


rcCinderGrabber::~rcCinderGrabber()
{
	
}

// Start grabbing
bool rcCinderGrabber::start()
{
    mCurrentIndex = 0;    
    if (isValid () && mMovie.checkPlayable ())
    {
      mMovie.seekToFrame(mCurrentIndex);
        
    }
    else
      setLastError( eFrameErrorUnsupportedFormat );

    return isValid () && mMovie.checkPlayable ();

}

// Stop grabbing
bool rcCinderGrabber::stop()
{
        return true;
}

// Returns the number of frames available
int32 rcCinderGrabber::frameCount()
{
    return mFrameCount;
}

// Get next frame, assign the frame to ptr
rcFrameGrabberStatus
rcCinderGrabber::getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking )
{

    rcFrameGrabberStatus ret =  eFrameStatusOK;
    setLastError( eFrameErrorUnknown );
           
    if (mCurrentIndex >= 0 && mCurrentIndex < mFrameCount)
    {
        if ( mMovie.checkNewFrame () )
        {
            double tp = mMovie.getCurrentTime ();
            ptr = rcSharedFrameBufPtr ( new rcFrame ( mMovie.getSurface ().getChannelGreen () ) );
            ptr->setTimestamp(tp);
            ret = eFrameStatusOK;
            setLastError( eFrameErrorOK );
            mMovie.stepForward ();
            mCurrentIndex++;
        }
        else
        {
            setLastError( eFrameErrorFileRead );
            ret = eFrameStatusError;        
        }
        
    }
    else
    {
        ret = eFrameStatusEOF;      
    }
    
    return ret;
}

//
// Private methods
//


// Get name of input source, ie. file name, camera name etc.
const std::string rcCinderGrabber::getInputSourceName()
{
    return mFileName;
}




