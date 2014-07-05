#ifndef __QTIME_VC_IMPL__
#define __QTIME_VC_IMPL__

#include <cinder/Channel.h>
#include <cinder/Area.h>
#include <limits>
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Surface.h"
#include "cinder/qtime/QuickTime.h"
#include "cinder/Thread.h"
#include <boost/algorithm/string.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/math/special_functions.hpp>
#include "rc_fileutils.h"
#include <stlplus_lite.hpp>
#include "qtime_cache.h"

#include "sshist.hpp"

#include <fstream>

using namespace ci;
using namespace std;



class QtimeCache::qtImpl
{
public:
    // ctor
    qtImpl( const std::string fqfn) : mWidth(0), mHeight(0), mFps (-1.0)
    {
        
        boost::lock_guard<boost::mutex> lock (mMuLock);
        mValid = file_exists ( fqfn ) && file_readable ( fqfn );
        if ( mValid )
        {
            mMovie = qtime::MovieSurface (fqfn);
            mWidth = mMovie.getWidth();
            mHeight = mMovie.getHeight();
            mFps = mMovie.getFramerate();
            mEmbeddedCount = mMovie.getFramerate();
        }
        
    }
    
    bool isValid () const
    {
        return mValid;
    }
    
    uint32 frame_width () { return mWidth; }
    uint32 frame_height () { return mHeight; }
    uint32 embeddedCount () { return mEmbeddedCount; }
    
    double frame_rate () { return mFps; }
    
        const std::ostream& print_to_ (std::ostream& std_stream)
        {
            std_stream << " -- Embedded Movie Info -- " << std::endl;
            std_stream << "Dimensions:" << mMovie.getWidth() << " x " << mMovie.getHeight() << std::endl;
            std_stream << "Duration:  " << mMovie.getDuration() << " seconds" << std::endl;
            std_stream << "Frames:    " << mMovie.getNumFrames() << std::endl;
            std_stream << "Framerate: " << mMovie.getFramerate() << std::endl;
            std_stream << "Alpha channel: " << mMovie.hasAlpha() << std::endl;
            std_stream << "Has audio: " << mMovie.hasAudio() << " Has visuals: " << mMovie.hasVisuals() << std::endl;
            return std_stream;
        }
        
        
        uint32  getTOC (std::vector<rcTimestamp>& tocItoT, std::map<rcTimestamp,uint32>& tocTtoI)
        {
            boost::lock_guard<boost::mutex> (this->mMuLock);

            if (! isValid () ) return false;
            mMovie.seekToStart ();
            
            tocItoT.resize (embeddedCount());
            tocTtoI.clear ();
            long        frameCount = 0;
            TimeValue   curMovieTime = 0;
            auto movObj = mMovie.getMovieHandle();
            
            // MediaSampleFlags is defined in ImageCompression.h:
            OSType types[] = { VisualMediaCharacteristic };
            
            while( curMovieTime >= 0 )
            {
                ::GetMovieNextInterestingTime( movObj, nextTimeStep, 1, types, curMovieTime, fixed1, &curMovieTime, NULL );
                double timeSecs (curMovieTime / ::GetMovieTimeScale( movObj ) );
                rcTimestamp timestamp = rcTimestamp::from_seconds (timeSecs);
                
                tocItoT[frameCount] = timestamp;
                tocTtoI[timestamp] = frameCount;
                
                
                frameCount++;
            }
            return frameCount - 1;
        }
    
    void getSurfaceAndCopy (rcSharedFrameBufPtr& ptr)
    {
        boost::lock_guard<boost::mutex> (this->mMuLock);
        
        mSurface = mMovie.getSurface ();
        ptr->setIsGray (true);
        Surface::Iter iter = mSurface.getIter ( mSurface.getBounds() );
        int rows = 0;
        while (iter.line () )
        {
            uint8_t* pels = ptr->rowPointer (rows++);
            while ( iter.pixel () ) *pels++ = iter.g ();
        }
    }
    
    void seekToFrame( int frame )
    {
        mMovie.seekToFrame (frame);
    }
    
    void seekToStart()
    {
        mMovie.seekToStart ();
    }
    
    void seekToEnd()
    {
        mMovie.seekToStart ();
    }
    
    bool checkPlayable ()
    {
        return mMovie.checkPlayable ();
    }
    
    float getCurrentTime() const
    {
        return mMovie.getCurrentTime ();
    }
    bool checkNewFrame ()
    {
        return mMovie.checkNewFrame ();
    }

    private:
        ci::qtime::MovieSurface	    mMovie;
        ci::Surface				mSurface;
        bool                mValid;
        uint32 mEmbeddedCount;
        uint32 mWidth;
        uint32 mHeight;
        double mFps;
        void lock()  { this->mMuLock.lock (); }
        void unlock()  { this->mMuLock.unlock (); }
        boost::mutex    mMuLock;    // explicit mutex for locking QuickTime
        
    };
    
    
#endif // __VF_UTILS__QTIME__IMPL__
