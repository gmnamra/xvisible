#ifndef __VF_UTILS__QTIME_
#define __VF_UTILS__QTIME_

#include <cinder/Channel.h>
#include <cinder/Area.h>
#include <limits>
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Surface.h"
#include "cinder/qtime/QuickTime.h"
#include "rc_window.h"
#include <boost/algorithm/string.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/math/special_functions.hpp>
#include "rc_fileutils.h"
#include <stlplus_lite.hpp>
#include "rc_filegrabber.h"
#include <random>

#include "sshist.hpp"

#include <fstream>

using namespace ci;
using namespace std;



namespace vf_utils
{
    
    namespace qtime_support
    {
        
        
        class CinderQtimeGrabber : public rcFrameGrabber
        {
        public:
            // ctor
            CinderQtimeGrabber( const std::string fileName,   // Input file
                               double frameInterval = -1.0, // Forced frame interval
                               int32  startAfterFrame = -1,
                               int32  frames = -1 ) :
            mFileName( fileName ), mFrameInterval( frameInterval ),mFrameCount(-1),
            mCurrentTimeStamp( rcTimestamp::from_seconds(frameInterval) ), mCurrentIndex( 0  )
            {
                boost::lock_guard<boost::mutex> (this->mMuLock);
                mValid = file_exists ( fileName ) && file_readable ( fileName );
                if ( mValid )
                {
                    mMovie = ci::qtime::MovieSurface( fileName );
                    m_width = mMovie.getWidth ();
                    m_height = mMovie.getHeight ();
                    mFrameCount = mMovie.getNumFrames();
                    mMovieFrameInterval = 1.0 / (mMovie.getFramerate() + std::numeric_limits<double>::epsilon() );
                    mFrameInterval = boost::math::signbit (frameInterval) == 1 ? mMovieFrameInterval : mFrameInterval;
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
            
            // virtual dtor
            virtual ~CinderQtimeGrabber() {}
            
            virtual bool isValid () const
            {
                return mValid && ( getLastError() == eFrameErrorOK );
            }
            
            //
            // rcFrameGrabber API
            //
            
            // Start grabbing
            virtual bool start()
            {
                boost::lock_guard<boost::mutex> (this->mMuLock);
                
                mCurrentIndex = 0;
                mMovie.seekToStart ();
                mMovie.play ();
                if (mMovie.isPlaying () )
                    return true;
                else
                    setLastError( eFrameErrorUnsupportedFormat );
                return false;
            }
            
            
            // Stop grabbing
            virtual bool stop()
            {
                boost::lock_guard<boost::mutex> (this->mMuLock);
                
                bool what = mMovie.isDone ();
                if (what) return what;
                what = mMovie.isPlaying ();
                if (! what ) return true;
                // It is not done and is playing
                mMovie.stop ();
                return ! mMovie.isPlaying ();
                
            }
            
            // Returns the number of frames available
            virtual int32 frameCount() { return mFrameCount; }
            
            // Movie grabbers don't have a cache.
            virtual int32 cacheSize() { return 0; }
            
            // Get next frame, assign the frame to ptr
            virtual rcFrameGrabberStatus getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking )
            {
                boost::lock_guard<boost::mutex> (this->mMuLock);
                rcFrameGrabberStatus ret =  eFrameStatusOK;
                setLastError( eFrameErrorUnknown );
                
                if (mCurrentIndex >= 0 && mCurrentIndex < mFrameCount)
                {
                    if ( mMovie.checkNewFrame () )
                    {
                        double tp = mMovie.getCurrentTime ();
                        mSurface = mMovie.getSurface ();
                        if (mSurface )
                        {
                            ptr = vf_utils::ci2rc2ci::NewFromChannel8u (mSurface.getChannelRed ());
                            ret = eFrameStatusOK;
                            setLastError( eFrameErrorOK );
                            mMovie.stepForward ();
                            mCurrentIndex++;
                        }
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
            
            // Get name of input source, ie. file name, camera name etc.
            virtual const std::string getInputSourceName() {  return mFileName; }
            
            
            // Get last error value.
            virtual rcFrameGrabberError getLastError() const
            {
                return mLastError;
            }
            
            
            // Set last error value
            void setLastError( rcFrameGrabberError error )
            {
                mLastError = error;
            }
            
            double frame_duration  () const { return mFrameInterval; }
            
        private:
            
            ci::qtime::MovieSurface	    mMovie;
            ci::Surface				mSurface;
            bool                mValid;
            const std::string          mFileName;
            double mFrameInterval;
            double mMovieFrameInterval;
            int32                 mFrameCount;       // Number of frames in a movie
            rcTimestamp             mCurrentTimeStamp; // Current frame timestamp
            int32                 mCurrentIndex;     // Current index within movie
            
            
            int32 m_width, m_height;
            rcFrameGrabberError  mLastError;
            
            void lock()  { this->mMuLock.lock (); }
            void unlock()  { this->mMuLock.unlock (); }
            boost::mutex    mMuLock;    // explicit mutex for locking QuickTime
            
        };
        
    }
}

#endif // __VF_UTILS__QTIME_
