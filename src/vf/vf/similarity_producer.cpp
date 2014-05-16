

#include "rc_similarity_producer.h"
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
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/ref.hpp>
#include <random>

using namespace boost;

 static boost::mutex         s_mutex;   

bool insensitive_case_compare (const std::string& str1, const std::string& str2)
{
    for(unsigned int i=0; i<str1.length(); i++){
        if(toupper(str1[i]) != toupper(str2[i]))
            return false;
    }
    return true;
}

class random_name
{
    
    std::string _chars;
    std::mt19937 mBase;
public:
    
    random_name ()
    {
        _chars = std::string (
                              "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "1234567890");
    }

	int32_t nextInt( int32_t v )
	{
		if( v <= 0 ) return 0;
		return mBase() % v;
	}

    std::string get_anew ()
    {
        std::string ns;
        for(int i = 0; i < 8; ++i) ns.push_back (_chars[nextInt(_chars.size()-1)]);
        rmAssert (ns.length() == 8);
        return ns;
    }
};


SINGLETON_FCN(random_name,get_name_generator);

class SimilarityProducer::spImpl : public rc_signaler 
{
public:
    spImpl () 
    {
        m_name = get_name_generator().get_anew();  
        signal_movie_loaded = createSignal<SimilarityProducer::sig_cb_movie_loaded>();         
        signal_frame_loaded = createSignal<SimilarityProducer::sig_cb_frame_loaded>();
        signal_sm1d_available = createSignal<SimilarityProducer::sig_cb_sm1d_available> ();
        signal_sm2d_available = createSignal<SimilarityProducer::sig_cb_sm2d_available> ();  
        _videoCacheP = 0;
    }
    
    bool load_content_file (const std::string& movie_fqfn) 
    {
        _movieFile = movie_fqfn;
        rcFrameGrabberError fr;
        _frameCount = loadMovie (movie_fqfn, fr);
        _has_content = fr == eFrameErrorOK;
        return has_content ();
    }

    int loadFrames( rcFrameGrabber& grabber, rcFrameGrabberError& error );
    bool has_content () const { return _has_content; }
    bool generate_ssm (int start_frame, int frames);
    int first_process_index () { return _analysisFirstFrame; } 
    int last_process_index () { return _analysisLastFrame; }     
    int frame_count () { return _frameCount; }
    int process_count () { return _processCount; }
    std::string  getName () const { return m_name; }
    
private:    
    int loadMovie( const std::string& movieFile, rcFrameGrabberError& error );

    int loadImages();
    int loadMovie ();

    
    int saveFrames(std::string imageExportDir) const;
    void lock () { if (mCarbonLock) { mCarbonLock->lock (); }  }
    void unlock () { if (mCarbonLock) { mCarbonLock->unlock (); }  }

    void ipp (const rcWindow& tmp, rcWindow& image, rcTimestamp& current);
   
    // static boost::mutex& get_mutex () { return spImpl::s_mutex; } 
protected:    
    boost::signals2::signal<SimilarityProducer::sig_cb_movie_loaded>* signal_movie_loaded;
    boost::signals2::signal<SimilarityProducer::sig_cb_frame_loaded>* signal_frame_loaded;
    boost::signals2::signal<SimilarityProducer::sig_cb_sm1d_available>* signal_sm1d_available;
    boost::signals2::signal<SimilarityProducer::sig_cb_sm2d_available>* signal_sm2d_available;    

private:
    mutable bool _has_content;    
    double            _physcial_memory_hint;
    int32             _analysisFirstFrame;
    int32             _analysisLastFrame;
    rcTimestamp       _currentTime, _startTime;
    int               _processCount;
    vector<rcWindow>    _fileImages;          // Loaded images
    std::string            _imageFileNames;  // image file names separated 
    mutable std::string            _movieFile;  // 
    int                          _frameRate;
    int                          _frameCount;
    rcVideoCache*                _videoCacheP;
    deque<deque<double> >        m_SMatrix;   // Used in eExhaustive and
    deque<double>                m_entropies; // Final entropy signal
    deque<double>                m_means; // Final entropy signal   
    rcCarbonLock*                mCarbonLock;
    rcPixel                      m_depth;
    std::string                  m_name;    
};



SimilarityProducer::SimilarityProducer () 
{

    boost::lock_guard<boost::mutex> guard ( s_mutex );
    _impl = boost::shared_ptr<spImpl> (new spImpl);
    
}


bool SimilarityProducer::load_content_file (const std::string& movie_fqfn) 
{
    if (_impl) return _impl->load_content_file(movie_fqfn);
    return false;
}

void SimilarityProducer::operator() (int start_frame, int frames ) const
{
    if (_impl) _impl->generate_ssm (start_frame, frames);
}


template<typename T> boost::signals2::connection
SimilarityProducer::registerCallback (const boost::function<T> & callback)
{
    return _impl->registerCallback  (callback);
}

template<typename T> bool
SimilarityProducer::providesCallback () const
{
    return _impl->providesCallback<T> ();
}


int SimilarityProducer::process_start_frame() const { return (_impl) ? _impl->first_process_index() : -1; }
int SimilarityProducer::process_last_frame() const { return (_impl) ? _impl->last_process_index() : -1; }
int SimilarityProducer::process_count () const { return (_impl) ? _impl->process_count () : -1; }
bool SimilarityProducer::has_content () const { if (_impl) return _impl->has_content (); return false; }


// @todo: update depth info
int SimilarityProducer::spImpl::loadMovie( const std::string& movieFile, rcFrameGrabberError& error )
{
	int count = 0;
	vector<rcWindow> zstk;
    
	if ( !movieFile.empty() )
    {
		rcFrameGrabber* grabber = 0;
		if ( rf_ext_is_rfymov (movieFile ) )
		{
			rmAssert(_videoCacheP == 0);
			double physMem = 1000000000.0;
            
			_videoCacheP =
            rcVideoCache::rcVideoCacheCtor(movieFile, 0, true, false, true, physMem, 0); // _videoCacheProgress );
			cerr << "Video cache size " << physMem/(1024*1024) << " MB" << endl;
			grabber = new rcReifyMovieGrabber(*_videoCacheP);
		}
		else if ( rf_ext_is_stk (movieFile) )
		{
            // Get a TIFF Importer
			TIFFImageIO t_importer;
			if (! t_importer.CanReadFile (movieFile.c_str () ) ) return 0;
			t_importer.SetFileName (movieFile.c_str ());
			t_importer.ReadImageInformation ();
			zstk = t_importer.ReadPages ();
			grabber = new rcVectorGrabber (zstk);
		}
		else 
		{
			// grabber = reinterpret_cast<rcFrameGrabber*> (new rcCinderGrabber( movieFile, mCarbonLock));
            throw not_implemented_error ();
		}
        
		count = loadFrames( *grabber, error );
		delete grabber;
        
		if ( error != eFrameErrorOK ) {
			if (_videoCacheP) {
				rcVideoCache::rcVideoCacheDtor(_videoCacheP);
				_videoCacheP = 0;
			}
		}
        if (signal_movie_loaded && signal_movie_loaded->num_slots() > 0 ) signal_movie_loaded->operator()();
    }   
    return count;
}

int SimilarityProducer::load_content_grabber (rcFrameGrabber& grabber, rcFrameGrabberError& error)
{
    return _impl->loadFrames(grabber, error);
}

// Generic method to load frames from a rcFrameGrabber
int SimilarityProducer::spImpl::loadFrames( rcFrameGrabber& grabber, rcFrameGrabberError& error )
{
	error = grabber.getLastError();
	int i = 0;
    
	// Remove old images
	_fileImages.resize (0);
    
	rcTimestamp duration = rcTimestamp::now();
    
	// Grab everything
	if ( grabber.isValid() && grabber.start())
	{
		uint32 frameMax = grabber.frameCount();
		const rcTimestamp updateStatusInterval = rcTimestamp::from_seconds (1.5); // Status display update interval in seconds
		const rcTimestamp updateMovieInterval = rcTimestamp::from_seconds(3.0);  // Movie display update interval in seconds
        
		_currentTime = cZeroTime;
		++_processCount;
        
		rcTimestamp lastStatusUpdateTime = rcTimestamp::now()  - updateStatusInterval;
		rcTimestamp lastMovieUpdateTime = lastStatusUpdateTime - updateMovieInterval;
        
        // Time stamp of previous frame
		rcTimestamp prevTimeStamp = cZeroTime;
		bool firstFrame = true;
        
        // Note: infinite loop
		for( i = 0; ; ++i )
		{
			rcTimestamp curTimeStamp;
			rcRect videoFrame;
			rcWindow image, tmp;
			rcVideoCacheError error;
			rcSharedFrameBufPtr framePtr;
			rcFrameGrabberStatus status = grabber.getNextFrame( framePtr, true );
            
			if ( status != eFrameStatusOK )
			{
				break;
			}
            // Note curTimeStamp fetching under VideoCache and normal fetch
            // Any access to the pixel data or time stamp of the frames cached will force a frame load.
			if (_videoCacheP)
			{
				if (_videoCacheP->frameIndexToTimestamp(i,curTimeStamp,&error) != eVideoCacheStatusOK)
				{
					cerr << "vfload: " << i << ": " << rcVideoCache::getErrorString(error) << endl;
					rmAssert(0);
				}
                
				videoFrame = rcRect( 0, 0, _videoCacheP->frameWidth(),
                                    _videoCacheP->frameHeight() );
				tmp = rcWindow( *_videoCacheP, i );
			}
			else
			{
				videoFrame = rcRect( 0, 0, framePtr->width(), framePtr->height() );
				tmp = rcWindow( framePtr );
				curTimeStamp = tmp.frameBuf()->timestamp();
			}
            
			rcTimestamp frameInt = curTimeStamp  - prevTimeStamp;
            
			if ( firstFrame )
			{
				firstFrame = false;_startTime = curTimeStamp;_currentTime = _startTime;
                
                // Update video frame size (readjusts monitor size) setFrameSize( videoFrame.width(), videoFrame.height(), tmp.depth()); _observer->notifyVideoRect( videoFrame );
                //				_observer->notifyAnalysisRect( videoFrame );
			}
			else
			{
                // Use true frame interval
				_currentTime += frameInt;
				rmAssert(frameInt > cZeroTime );// Zero time between frames is not allowed
			}
            
			prevTimeStamp = curTimeStamp;
          	ipp (tmp, image, curTimeStamp);
			_fileImages.push_back( image );

            // Post index and time if we have requesters
            double timestamp = curTimeStamp.secs ();
            if (signal_frame_loaded && signal_frame_loaded->num_slots() > 0 ) signal_frame_loaded->operator()(boost::ref (i), boost::ref (timestamp));
            
            
            //			rcVideoWriter* videoWriter;
            //			videoWriter = _videoWriter.getValue(videoWriter);
            //		if (videoWriter != 0)
            //				videoWriter->writeValue( curTimeStamp, rcRect(), &image );
            
			const rcTimestamp curTime =  rcTimestamp::now();
			rcTimestamp updateInterval = curTime - lastStatusUpdateTime;
            
            // Update status bar
			if ( updateInterval > updateStatusInterval)  {
				lastStatusUpdateTime = curTime;
				char buf[512];
				snprintf( buf, rmDim( buf ), "Loading %i frames...%i%% complete", frameMax, int(double(i)/frameMax * 100) );
                //				_observer->notifyStatus( buf );
			}
			updateInterval = curTime - lastMovieUpdateTime;
            
			if ( updateInterval > updateMovieInterval )
            {
                // Update movie display
				lastMovieUpdateTime = curTime;
                //			if ( _observer->acceptingImageBlits() )
                //				_observer->notifyBlitData( &image );
			}
            
		}// End of For i++
        
        // Update elapsed time _observer->notifyTime( getElapsedTime() ); _observer->notifyTimelineRange( 0.0, getElapsedTime() ); 	flushSharedWriters ();
		if ( ! grabber.stop() )
			error = grabber.getLastError();
	}
	else   // isValid() failed
		error = grabber.getLastError();
    
	// Done. Report
	duration = rcTimestamp::now() - duration;
    
	// Set analysis range
	_analysisFirstFrame = 0;
	_analysisLastFrame =  _fileImages.size();
    
#if 0    
	if ( !_fileImages.empty() ) {
        //	setSettingValue( cAnalysisRectSettingId, rcRect(0, 0, _frameWidth, _frameHeight ) ); //	_observer->notifyAnalysisRect( getSettingValue( cAnalysisRectSettingId ) );                
		{
			char buf[512];
			snprintf( buf, rmDim( buf ), "%li %s frames [%d by %d] loaded\n",
                     _fileImages.size(),
                     _fileImages[0].isGray() ? "gray scale" : "color",
                     _fileImages[0].size().x(), _fileImages[0].size().y());
            //		_observer->notifyStatus( buf );
		}
		// stdout message
		cout << _fileImages.size() << " " << _fileImages[0].bits () << "-bit ";
		if ( _fileImages[0].isGray() )
			cout << "gray scale";
		else
			cout << "16bit gray or color";
		cout << "[" << _fileImages[0].size().x() << " by " << _fileImages[0].size().y() << "]";
		cout << " frames loaded in " << duration.secs() << " seconds" << endl;
        
	} else {
		cout << "done" << endl;
		cout << _fileImages.size() << " frames loaded in " << duration.secs() << " seconds" << endl;
	}
#endif
    
    //	updateHeaderLogs();
    
	return i;
}

// Load images to memory, return number of images loaded
int SimilarityProducer::spImpl::loadImages()
{
	int count = 0;
    
    //	if ( getState() != eEngineRunning ) {
    vector<std::string> fileNames;
    
    if ( !_imageFileNames.empty() ) {
        vector<const char*> files;
        std::string::size_type str = 0, idx = 0;
        
        while ( str < _imageFileNames.size() ) {
            idx = _imageFileNames.find( ';', str );
            if ( idx != std::string::npos ) {
                std::string oneFile = _imageFileNames.substr( str, idx - str );
                fileNames.push_back( oneFile );
                str = idx + 1;
            } else
                break;
        }
        // No semicolon found, just keep the string as is
        if ( fileNames.empty() )
            fileNames.push_back( _imageFileNames );
        
        double frameInterval = _frameRate ? 1.0/_frameRate : 1.0;
        // Create image grabber with a forced frame rate
        //        rcImageGrabber grabber( fileNames, this, frameInterval );
        
        // Load frames
        rcFrameGrabberError error = eFrameErrorUnknown;
        //    count = loadFrames( grabber, error );
        
        if ( error != eFrameErrorOK )
        {
            //           strstream s;
            //           s << rcFrameGrabber::getErrorString( error ) << " " << grabber.getInputSourceName() << ends;
            //           _observer->notifyError( s.str() );
            //          _observer->notifyStatus( s.str() );
            //        s.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
            //        cerr << s.str() << endl;
        }
        
    }
    return count;
}

// Load movie frames to memory, return number of frames loaded
int SimilarityProducer::spImpl::loadMovie()
{
    int count = 0;
    
    if ( !_movieFile.empty() )
    {
        rcFrameGrabberError error = eFrameErrorUnknown;
        
        count = loadMovie( _movieFile, error );
        cerr << count << " Frames in the movie " << _movieFile << endl;
        
        if ( error != eFrameErrorOK )
        {
            //                 strstream s;
            //                        s << rcFrameGrabber::getErrorString( error ) << " " << _movieFile << ends;
            //                        _observer->notifyError( s.str() );
            //                        _observer->notifyStatus( s.str() );
            //                        s.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
            //                        cerr << s.str() << endl;
        }
    }
    return count;
}


// @todo add sampling and offset
bool SimilarityProducer::spImpl::generate_ssm (int start_frames, int frames)
{
    int framesToUse = _fileImages.size ();

    
    // Do not accesss images in the vector. They are cached.
    rcSimilaratorRef simi ( new rcSimilarator (rcSimilarator::eExhaustive, rcPixel8, framesToUse, 1000000000));
    simi->fill(_fileImages);
    m_entropies.resize (0);
    bool ok = simi->entropies (m_entropies, rcSimilarator::eVisualEntropy);
    m_SMatrix.resize (0);
    simi->selfSimilarityMatrix(m_SMatrix);
    return ok;
}



// Generic method to load frames from a rcFrameGrabber
int SimilarityProducer::spImpl::saveFrames(std::string imageExportDir) const
{
    if (imageExportDir.empty()) return 0;
    
    vector<rcWindow>::const_iterator imgItr = _fileImages.begin ();
    
    rcTimestamp duration = rcTimestamp::now();
    
    int32 i = 0;
    for (; imgItr != _fileImages.end(); imgItr++, i++)
    {
        //   std::ostringstream oss;
        //  oss << _exportImagesDir << "/" << "image" << setfill ('0') << setw(4) << i << ".jpg";
        //   std::string fn (oss.str ());
        //  rfImageExport2JPG ( *imgItr, fn);
        //   fn = std::string ("chmod 644 ") + fn;
        //    ::system( fn.c_str() );
        
    }
    
    // Done. Report
    duration = rcTimestamp::now() - duration;
    cout <<  endl << i << " Images Exported in " << duration.secs() << "Seconds" << endl;
    
    return i;
}

void SimilarityProducer::spImpl::ipp (const rcWindow& tmp, rcWindow& image, rcTimestamp& current)
{
    rmUnused (current);
    
    // VideoCache case: Do not access frameData. Access for invalidate cache
    if (_videoCacheP)
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

template boost::signals2::connection SimilarityProducer::registerCallback(const boost::function<SimilarityProducer::sig_cb_movie_loaded>&);
template boost::signals2::connection SimilarityProducer::registerCallback(const boost::function<SimilarityProducer::sig_cb_frame_loaded>&);

