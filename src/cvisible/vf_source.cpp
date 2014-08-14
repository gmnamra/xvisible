

#include "vf_sm_producer.h"
#include "vf_sm_producer_impl.h"
#include "rc_similarity.h"
#include "rc_videocache.h"
#include "rc_reifymoviegrabber.h"
#include "rc_tiff.h"
#include "rc_fileutils.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>
#include <signaler.h>
#include <static.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <thread>
#include "vf_cinder_qtime_grabber.h"
#include "vf_image_conversions.hpp"
#include "vf_utils.hpp"
#include <future>


using namespace boost;
using namespace vf_utils;
using namespace vf_utils::gen_filename;

static boost::mutex         s_mutex;


#include "rc_types.h"
#include "rc_reifymoviegrabber.h"


template<> rcFrameGrabberError
rcReifyMovieGrabberT<QtimeCache,QtimeCacheError>::errorNoTranslate(QtimeCacheError error)
{
    switch (error) {
        case QtimeCacheError::FileInit:
            return rcFrameGrabberError::FileInit;
        case QtimeCacheError::FileSeek:
        case QtimeCacheError::FileRead:
            return rcFrameGrabberError::FileRead;
        case QtimeCacheError::FileClose:
            return rcFrameGrabberError::FileClose;
        case QtimeCacheError::FileFormat:
            return rcFrameGrabberError::FileFormat;
        case QtimeCacheError::FileUnsupported:
            return rcFrameGrabberError::FileUnsupported;
        case QtimeCacheError::FileRevUnsupported:
            return rcFrameGrabberError::FileRevUnsupported;
        case QtimeCacheError::SystemResources:
            return rcFrameGrabberError::SystemResources;
        case QtimeCacheError::NoSuchFrame:
        case QtimeCacheError::CacheInvalid:
            return rcFrameGrabberError::Internal;
        case QtimeCacheError::OK:
            return rcFrameGrabberError::OK;
        case QtimeCacheError::BomUnsupported:
            return rcFrameGrabberError::FileUnsupported;
        case QtimeCacheError::DepthUnsupported:
            return rcFrameGrabberError::UnsupportedDepth;
    }
    
    return rcFrameGrabberError::Unknown;
}


template<>
rcReifyMovieGrabberT<QtimeCache,QtimeCacheError>::rcReifyMovieGrabberT(QtimeCache& cache)
: rcFileGrabber(0), _framesLeft(0), _curFrame(0), _started(false),
_cache(cache)
{
    if (!_cache.isValid())
        setLastError(errorNoTranslate(_cache.getFatalError()));
}

template <>
rcReifyMovieGrabberT<QtimeCache,QtimeCacheError>::~rcReifyMovieGrabberT()
{
}


template<>
bool rcReifyMovieGrabberT<QtimeCache,QtimeCacheError>::start()
{
    if (!isValid())
        return false;
    
    if (_started == false) {
        _framesLeft = _cache.frameCount();
        _curFrame = 0;
        _started = true;
    }
    
    return true;
}

template<>
bool rcReifyMovieGrabberT<QtimeCache,QtimeCacheError>::stop()
{
    if (!isValid())
        return false;
    
    _started = false;
    
    return true;
}


template<>
rcFrameGrabberStatus rcReifyMovieGrabberT<QtimeCache,QtimeCacheError>::getNextFrame(QtimeCache::frame_ref_t& ptr,
                                                                                        bool isBlocking)
{
    if (!isValid())
        return eFrameStatusError;
    
    if (!isBlocking) {
        setLastError(rcFrameGrabberError::NotImplemented);
        return eFrameStatusError;
    }
    
    if (!_started && !start())
        return eFrameStatusError;
    
    if (_framesLeft == 0)
        return eFrameStatusEOF;
    
    _framesLeft--;
    
    /*
     * Get reference to frame, but don't lock it so it won't be forced
     * into memory before it is needed.
     */
    QtimeCacheError error;
    QtimeCacheStatus status = _cache.getFrame(_curFrame++, ptr, &error, false);
    
    if (status != QtimeCacheStatus::OK){
        setLastError(errorNoTranslate(error));
        return eFrameStatusError;
    }
    
    return eFrameStatusOK;
}



sm_producer::sm_producer ()
{
    
    boost::lock_guard<boost::mutex> guard ( s_mutex );
    _impl = boost::shared_ptr<spImpl> (new spImpl);
    
}


bool sm_producer::load_content_file (const std::string& movie_fqfn)
{
    boost::lock_guard<boost::mutex> guard ( s_mutex );
    
    if (_impl) return _impl->load_content_file(movie_fqfn);
    return false;
}

bool sm_producer::operator() (int start_frame, int frames ) const
{
    boost::lock_guard<boost::mutex> guard ( s_mutex );
    if (_impl)
    {
        std::future<bool> bright = std::async(std::launch::deferred, &sm_producer::spImpl::generate_ssm, _impl, start_frame, frames);
        return bright.get();
    }
    return false;
}

void sm_producer::load_images(const images_vector &images)
{
    boost::lock_guard<boost::mutex> guard ( s_mutex );
    if (_impl) _impl->loadImages (images);
}

template<typename T> boost::signals2::connection
sm_producer::registerCallback (const boost::function<T> & callback)
{
    return _impl->registerCallback  (callback);
}

template<typename T> bool
sm_producer::providesCallback () const
{
    return _impl->providesCallback<T> ();
}

bool sm_producer::set_auto_run_on() const { return (_impl) ? _impl->set_auto_run_on() : false; }
bool sm_producer::set_auto_run_off() const { return (_impl) ? _impl->set_auto_run_off() : false; }
int sm_producer::process_start_frame() const { return (_impl) ? _impl->first_process_index() : -1; }
int sm_producer::process_last_frame() const { return (_impl) ? _impl->last_process_index() : -1; }
int sm_producer::frames_in_content() const { return (_impl) ? _impl->frame_count(): -1; }
bool sm_producer::has_content () const { if (_impl) return _impl->has_content (); return false; }
rcFrameGrabberError sm_producer::last_error () const { if (_impl) return _impl->last_error(); return rcFrameGrabberError::Unknown; }

const sm_producer::sMatrixType& sm_producer::similarityMatrix () const { return _impl->m_SMatrix; }

//const sMatrixProjectionType& sm_producer::meanProjection () const;

const sm_producer::sMatrixProjectionType& sm_producer::shannonProjection () const { return _impl->m_entropies; }


// Get appropriate Grabber for Content
// @todo if is directory: check and process as directory of images
int sm_producer::spImpl::loadMovie( const std::string& movieFile, rcFrameGrabberError& error )
{
	images_vector zstk;
    m_grabber_ref.reset();
    
	if ( !movieFile.empty() )
    {
        
		if ( rf_ext_is_rfymov (movieFile ) || rf_ext_is_stk (movieFile))
		{
            return -1; // not implemented
        }
			else if ( rf_ext_is_mov(movieFile) )
		{
            _shared_qtime_cache_create_simple(tmp, movieFile,0);
            m_qtime_cache_ref = tmp;
            m_grabber_ref = boost::shared_ptr<cache_grabber> (new cache_grabber (m_qtime_cache_ref));
		}
        
		_frameCount = loadFrames();
        error = m_last_error;
    }
    return _frameCount;
}

void sm_producer::spImpl::loadImages (const images_vector& images)
{
    m_loaded_ref.reset (new images_vector );
    vector<roi_window>::const_iterator vitr = images.begin();
    do
    {
        m_loaded_ref->push_back (*vitr++);
    }
    while (vitr != images.end());
    _frameCount = m_loaded_ref->size ();
    if (m_auto_run) generate_ssm (0,0);
        
}

// Generic method to load frames from a rcFrameGrabber
int32 sm_producer::spImpl::loadFrames(  )
{
    _has_content.store (false);
  
    
    // unique lock. forces new shared locks to wait untill this lock is release
    boost::unique_lock <mutex_t> lock(m_mutex);
    
	int count = 0;
	rcTimestamp duration = rcTimestamp::now();
    _currentTime = cZeroTime;
    rcTimestamp prevTimeStamp = cZeroTime;
   
	// Grab everything
	if ( m_grabber_ref->isValid() && m_grabber_ref->start())
	{
        int fc = m_grabber_ref->frameCount ();
        int fc_1 = fc - 1;
        
        // Note: infinite loop
		for( count = 0; ; ++count )
		{
			rcTimestamp curTimeStamp;
			rcRect videoFrame;
			roi_window image, tmp;
            QtimeCache::frame_ref_t framePtr;
			int status = m_grabber_ref->get_next_frame( framePtr );
            
			if ( status != 0 ) break;

            // Note curTimeStamp fetching under VideoCache and normal fetch
            // Any access to the pixel data or time stamp of the frames cached will force a frame load.
                QtimeCacheError error;
				if (m_qtime_cache_ref->frameIndexToTimestamp(count,curTimeStamp,&error) != QtimeCacheStatus::OK)
				{
					cerr << "vfload: " << count << ": " << QtimeCache::getErrorString(error) << endl;
					rmAssert(0);
				}
                
				videoFrame = rcRect( 0, 0, m_qtime_cache_ref->frameWidth(), m_qtime_cache_ref->frameHeight() );
				tmp = roi_window( m_qtime_cache_ref, count );
            ipp (tmp, image, curTimeStamp);
            m_loaded_ref->push_back (image);

            // Post index and time if we have requesters
            double timestamp = curTimeStamp.secs ();
            if (signal_frame_loaded && signal_frame_loaded->num_slots() > 0 ) signal_frame_loaded->operator()(boost::ref (count), boost::ref (timestamp));
            
            _has_content.fetch_add (1);
            if (_has_content.compare_exchange_strong (fc_1, fc) ) break;

#if COMPARE_AND_CLAMP_TS
			rcTimestamp frameInt = curTimeStamp  - prevTimeStamp;
			if ( firstFrame ) {firstFrame = false;_startTime = curTimeStamp;_currentTime = _startTime;}
			else _currentTime += frameInt;
			prevTimeStamp = curTimeStamp;
#endif
            // @todo if (videoWriter != 0) 	videoWriter->writeValue( curTimeStamp, rcRect(), &image );  Compare and clamp image update interval
        }// End of For i++
        
        // Update elapsed time _observer->notifyTime( getElapsedTime() ); _observer->notifyTimelineRange( 0.0, getElapsedTime() ); 	flushSharedWriters ();
		if ( ! m_grabber_ref->stop() ) std::cout << " Grabber failed to stop" << std::endl;
		
	}

    
	// Done. Report
	duration = rcTimestamp::now() - duration;
    
    _frameCount = m_loaded_ref->size();
    
	// Set analysis range
	_analysisFirstFrame = 0;
	_analysisLastFrame =  _frameCount - 1;
    
    
    //	updateHeaderLogs();
     if (signal_content_loaded && signal_content_loaded->num_slots() > 0 ) signal_content_loaded->operator()();
    
    return _frameCount;

}



// @todo add sampling and offset
bool sm_producer::spImpl::generate_ssm (int start_frames, int frames)
{
static    double tiny = 1e-10;
    
    rcSimilarator simi(rcSimilarator::eExhaustive,
                     rcPixel8,
                     _frameCount,
                     0, rcSimilarator::eNorm,
                     false,
                     0,
                     tiny);
    
//    for (int ii=0; ii < _frameCount; ii++) m_loaded_ref->at(ii).print(dummy, std::cout);
    
    simi.fill(*m_loaded_ref);
    m_entropies.resize (0);
    bool ok = simi.entropies (m_entropies, rcSimilarator::eVisualEntropy);
    m_SMatrix.resize (0);
    simi.selfSimilarityMatrix(m_SMatrix);
    return ok;
}


//
//// Generic method to load frames from a rcFrameGrabber
//int sm_producer::spImpl::saveFrames(std::string imageExportDir) const
//{
//    if (imageExportDir.empty()) return 0;
//
//    images_vector::const_iterator imgItr = _fileImages.begin ();
//
//    rcTimestamp duration = rcTimestamp::now();
//
//    int32 i = 0;
//    for (; imgItr != _fileImages.end(); imgItr++, i++)
//    {
//        std::ostringstream oss;
//        oss << imageExportDir << "/" << "image" << setfill ('0') << setw(4) << i << ".jpg";
//        std::string fn (oss.str ());
//        vf_utils::ci2rc2ci::ImageExport2JPG ( *imgItr, fn);
//        fn = std::string ("chmod 644 ") + fn;
//        ::system( fn.c_str() );
//
//    }
//
//    // Done. Report
//    duration = rcTimestamp::now() - duration;
//    cout <<  endl << i << " Images Exported in " << duration.secs() << "Seconds" << endl;
//
//    return i;
//}

void sm_producer::spImpl::ipp (const roi_window& tmp, roi_window& image, rcTimestamp& current)
{
    rmUnused (current);
    
    // VideoCache case: Do not access frameData. Access for invalidate cache
    if (m_qtime_cache_ref)
    {
        image = tmp;
        image.frameBuf()->setTimestamp (current);
        return;
    }
    
    if (tmp.depth() == rcPixel8 || !tmp.isGray ())
    {
        image = tmp;
        image.frameBuf()->setTimestamp (current);
        return;
    }
    
    if (tmp.depth() == rcPixel16)
    {
        image = tmp;
        image.frameBuf()->setTimestamp (current);
        return;
    }
    
    if (tmp.depth() == rcPixel32S)
    {
        
        //        image = rfImageConvert32to8 (tmp, _channelConversion);
        //        image.frameBuf()->setTimestamp (current);
        return;
    }
}


template boost::signals2::connection sm_producer::registerCallback(const boost::function<sm_producer::sig_cb_content_loaded>&);
template boost::signals2::connection sm_producer::registerCallback(const boost::function<sm_producer::sig_cb_frame_loaded>&);


