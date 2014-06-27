#ifndef __VF_UTILS__QTIME_VC_IMPL__
#define __VF_UTILS__QTIME_VC_IMPL__

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


        
        class vfVideoCache::qtImpl
        {
        public:
            // ctor
            qtImpl( const std::string fqfn)
            {
      
                boost::lock_guard<boost::mutex> (this->mMuLock);
                mValid = file_exists ( fqfn ) && file_readable ( fqfn );
                if ( mValid )
                {
                    mMovie = qtime::MovieSurface (fqfn);
                }
                
            }
            
            bool isValid () const
            {
                return mValid;
            }
            
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
            
            
            bool  getTOC (std::vector<rcTimestamp>& tocItoT, std::map<rcTimestamp,uint32>& tocTtoI)
            {
                if (! isValid () ) return false;
                boost::lock_guard<boost::mutex> (this->mMuLock);
                mMovie.seekToStart ();
                
                tocItoT.resize (0);
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
                return true;
            }
            
            
        private:
            ci::qtime::MovieSurface	    mMovie;
            ci::Surface				mSurface;
            bool                mValid;
                      
            void lock()  { this->mMuLock.lock (); }
            void unlock()  { this->mMuLock.unlock (); }
            boost::mutex    mMuLock;    // explicit mutex for locking QuickTime
            
        };


#endif // __VF_UTILS__QTIME__IMPL__
