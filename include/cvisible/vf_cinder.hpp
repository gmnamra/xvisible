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
#include "vf_utils.hpp"
#include "sshist.hpp"

#include <fstream>
#include <mutex>
#include <memory>
#include <functional>

using namespace ci;
using namespace std;



class QtimeCache::qtImpl
{
 public:
    // ctor
    qtImpl( const std::string fqfn) : mFqfn (fqfn), mValid (false)
    {
        std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
    }
    
    void load_movie ()
    {
        std::lock_guard<std::mutex> lk(mx);
        
        if ( file_exists ( mFqfn ) && file_readable ( mFqfn ) )
        {
            mMovie = cinder::qtime::MovieSurface::create (mFqfn);
            mValid = mMovie != 0;
            if (mValid)
            {
                mInfo.mWidth = mMovie->getWidth();
                mInfo.mHeight = mMovie->getHeight();
                mInfo.mFps = mMovie->getFramerate();
                mInfo.mEmbeddedCount = mMovie->getNumFrames ();
                auto movObj = mMovie->getMovieHandle();
                mInfo.mTscale = ::GetMovieTimeScale( movObj );
                mMovie->checkPlayable();
                mMovie->seekToStart ();
                mMovie->setLoop (false, false);
                for (int fn=0; fn<mInfo.mEmbeddedCount; fn++)
                {
                    m_raw.push_back(static_cast<int32>(mMovie->getCurrentTime() * mInfo.mTscale));
                    mMovie->stepForward ();
                }
                assert(m_raw.size() == mInfo.mEmbeddedCount);
                
            }
        }

    }
    
    bool isValid () const
    {
        std::lock_guard<std::mutex> lk(mx);
        return mValid;
    }
    
    vf_utils::general_movie::info movie_info ()
    {
        std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
        std::lock_guard<std::mutex> lk(mx);            
        return mInfo;
    }
    
    
    const std::ostream& print_to_ (std::ostream& std_stream)
    {
        std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
        
        std_stream << " -- Embedded Movie Info -- " << std::endl;
        std_stream << "Dimensions:" << mMovie->getWidth() << " x " << mMovie->getHeight() << std::endl;
        std_stream << "Duration:  " << mMovie->getDuration() << " seconds" << std::endl;
        std_stream << "Frames:    " << mMovie->getNumFrames() << std::endl;
        std_stream << "Framerate: " << mMovie->getFramerate() << std::endl;
        std_stream << "Alpha channel: " << mMovie->hasAlpha() << std::endl;
        std_stream << "Has audio: " << mMovie->hasAudio() << " Has visuals: " << mMovie->hasVisuals() << std::endl;
        return std_stream;
    }
    
    
    double get_time_index_map (std::shared_ptr<std::vector<int32> >& ti_map)
    {
        std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
        std::lock_guard<std::mutex> lk(mx);
        std::shared_ptr<std::vector<int32> >  res(new std::vector<int32> (m_raw));
        ti_map.swap(res);
        return mInfo.mTscale;
    }
    
    bool getSurfaceAndCopy (cached_frame_ref& ptr)
    {
        std::lock_guard<std::mutex> lk(mx);
        if (!mMovie) return false;
        mSurface = mMovie->getSurface ();
        
        if (!mSurface) return false;
        
        ptr->setIsGray (true);
        Surface::Iter iter = mSurface.getIter ( mSurface.getBounds() );
        int rows = 0;
        while (iter.line () )
        {
            uint8_t* pels = ptr->rowPointer (rows++);
            while ( iter.pixel () ) *pels++ = iter.g ();
        }
        return true;
    }
    
    void seekToFrame( int frame )
    {
        std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
        std::lock_guard<std::mutex> lk(mx);
        mMovie->seekToFrame (frame);
        
    }
    
    void seekToStart()
    {
     //   std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
        std::lock_guard<std::mutex> lk(mx);
        mMovie->seekToStart ();
    }
    
    void seekToEnd()
    {
        std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
        std::lock_guard<std::mutex> lk(mx);
        mMovie->seekToStart ();
    }
    
    bool checkPlayable ()
    {
        std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
        std::lock_guard<std::mutex> lk(mx);
        return mMovie->checkPlayable ();
    }
    
    float getCurrentTime()
    {
        std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
        std::lock_guard<std::mutex> lk(mx);
        return mMovie->getCurrentTime ();
    }
    bool checkNewFrame ()
    {
        std::call_once (mMovie_loaded_flag, &qtImpl::load_movie, this);
        std::lock_guard<std::mutex> lk(mx);
        return mMovie->checkNewFrame ();

    }
    
private:
    vf_utils::general_movie::info mInfo;
    mutable std::once_flag mMovie_loaded_flag;
    std::string mFqfn;
    ci::qtime::MovieSurfaceRef    mMovie;
    ci::Surface				mSurface;
    mutable bool                mValid;
    mutable std::mutex          mx;
    
    
    std::vector<int32> m_raw;
};


#endif // __VF_UTILS__QTIME__IMPL__
