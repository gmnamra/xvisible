

#include "vf_sm_producer.h"
#include "vf_sm_producer_impl.h"
#include "self_similarity.h"
//#include "rc_fileutils.h"
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
#include "vf_utils.hpp"
#include <future>


using namespace boost;
using namespace vf_utils;
using namespace vf_utils::gen_filename;
using namespace vf_utils::file_system;

static boost::mutex         s_mutex;



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
grabber_error sm_producer::last_error () const { if (_impl) return _impl->last_error(); return grabber_error::Unknown; }

const sm_producer::sMatrixType& sm_producer::similarityMatrix () const { return _impl->m_SMatrix; }

//const sMatrixProjectionType& sm_producer::meanProjection () const;

const sm_producer::sMatrixProjectionType& sm_producer::shannonProjection () const { return _impl->m_entropies; }


// Get appropriate Grabber for Content
// @todo if is directory: check and process as directory of images
int sm_producer::spImpl::loadMovie( const std::string& movieFile, grabber_error& error )
{
	images_vector zstk;
    m_grabber_ref.reset();
    
	if ( !movieFile.empty() )
    {
        
		if (vf_utils::file_system::ext_is_rfymov (movieFile ) || ext_is_stk (movieFile))
		{
            return -1; // not implemented
        }
			else if (vf_utils::file_system::ext_is_mov(movieFile) )
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
	time_spec_t duration = time_spec_t::get_system_time().get_frac_secs();
    _currentTime = time_spec_t ();
    time_spec_t prevTimeStamp;
   
	// Grab everything
	if ( m_grabber_ref->isValid() && m_grabber_ref->start())
	{
        int fc = m_grabber_ref->frameCount ();
        int fc_1 = fc - 1;
        
        // Note: infinite loop
		for( count = 0; count < fc; count++ )
		{
			time_spec_t curTimeStamp;
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
					assert(0);
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
			time_spec_t frameInt = curTimeStamp  - prevTimeStamp;
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
	duration = time_spec_t::get_system_time().get_full_secs() - duration;
    
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
    
    self_similarity_producer simi(self_similarity_producer::eExhaustive,
                     rpixel8,
                     _frameCount,
                     0, self_similarity_producer::eNorm,
                     false,
                     0,
                     tiny);
    
//    for (int ii=0; ii < _frameCount; ii++) m_loaded_ref->at(ii).print(dummy, std::cout);
    
    simi.fill(*m_loaded_ref);
    m_entropies.resize (0);
    bool ok = simi.entropies (m_entropies, self_similarity_producer::eVisualEntropy);
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
//    time_spec_t duration = time_spec_t::now();
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
//    duration = time_spec_t::now() - duration;
//    cout <<  endl << i << " Images Exported in " << duration.secs() << "Seconds" << endl;
//
//    return i;
//}

void sm_producer::spImpl::ipp (const roi_window& tmp, roi_window& image, time_spec_t& current)
{
    UnusedParameter( current);
    
    // VideoCache case: Do not access frameData. Access for invalidate cache
    if (m_qtime_cache_ref)
    {
        image = tmp;
        image.frameBuf()->setTimestamp (current);
        return;
    }
    
    if (tmp.depth() == rpixel8 || !tmp.isGray ())
    {
        image = tmp;
        image.frameBuf()->setTimestamp (current);
        return;
    }
    
    if (tmp.depth() == rpixel16)
    {
        image = tmp;
        image.frameBuf()->setTimestamp (current);
        return;
    }
    
    if (tmp.depth() == rpixel32)
    {
        
        //        image = rfImageConvert32to8 (tmp, _channelConversion);
        //        image.frameBuf()->setTimestamp (current);
        return;
    }
}


template boost::signals2::connection sm_producer::registerCallback(const boost::function<sm_producer::sig_cb_content_loaded>&);
template boost::signals2::connection sm_producer::registerCallback(const boost::function<sm_producer::sig_cb_frame_loaded>&);


