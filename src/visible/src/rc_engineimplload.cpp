/*
 * @files
 *$Id $
 *$Log$
 *Revision 1.8  2005/12/02 23:48:18  arman
 *removed motion compensation at image load
 *
 *Revision 1.7  2005/12/02 00:42:37  arman
 *added ui support for motion compensation
 *
 *Revision 1.6  2005/11/07 17:32:09  arman
 *cell lineage iv and bug fix
 *
 *Revision 1.5  2005/10/25 22:36:04  arman
 *added setRegister support. Not tested yet.
 *
 *Revision 1.4  2005/10/24 21:27:39  arman
 *now using register for motion compensation
 *
 *Revision 1.3  2005/10/21 17:21:16  arman
 *added gui for rateChoice and motion compensation
 *
 *Revision 1.2  2005/10/20 19:35:40  arman
 *bugfix and motion comp proto
 *
 *Revision 1.1  2005/09/11 19:04:30  arman
 *moved all image / movie loading function of engineImpl here.
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include "rc_cinder_qtime_grabber.h"
#include "rc_engineimpl.h"


#include <rc_tiff.h>
#include <rc_window2jpg.h>
#include <rc_fileutils.h>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>



using namespace ci;
using namespace boost;

bool insensitive_case_compare (const std::string& str1, const std::string& str2)
{
    for(unsigned int i=0; i<str1.length(); i++){
        if(toupper(str1[i]) != toupper(str2[i]))
            return false;
    }
    return true;
}


// Generic method to load frames from a rcFrameGrabber
int rcEngineImpl::loadFrames( rcFrameGrabber& grabber, rcFrameGrabberError& error )
{
	error = grabber.getLastError();
	int i = 0;

	// Remove old images
	if ( ! _fileImages.empty() )
		_fileImages.clear();

	rcTimestamp duration = rcTimestamp::now();

	// Grab everything
	if ( grabber.isValid() && grabber.start())
	{
		uint32 frameMax = grabber.frameCount();
		const double updateStatusInterval = 1.5; // Status display update interval in seconds
		const double updateMovieInterval = 3.0;  // Movie display update interval in seconds

		_currentTime = 0.0;

		createSharedWriters();

	// Gentlemen, start your engines
		if (_inputMode != eCmd)
			setInternalState( eEngineRunning );
		++_processCount;

	// Timestamps for last updates
	// Initial value will cause an immediate update
		rcTimestamp lastStatusUpdateTime = rcTimestamp::now() - updateStatusInterval;
		rcTimestamp lastMovieUpdateTime = lastStatusUpdateTime - updateMovieInterval;

	// Time stamp of previous frame
		rcTimestamp prevTimeStamp = cZeroTime;
		bool firstFrame = true;

	// Set the analysis rect to zero-default for all modes
		setSettingValue( cAnalysisRectSettingId, rcRect( 0, 0, 0, 0 ) );

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

			if ( getState() == eEngineShuttingDown )
			{
				break;
			}

		// @note
		// Note curTimeStamp fetching under VideoCache and normal fetch
		// Any access to the pixel data or time stamp of the frames cached will force a frame load.
		// ipp process needs to make sure that it only does that if setting is to other than none.
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

			rcTimestamp frameInt = curTimeStamp - prevTimeStamp;

			if ( firstFrame )
			{
				firstFrame = false;_startTime = curTimeStamp;_currentTime = _startTime;

		// Update video frame size (readjusts monitor size)
				setFrameSize( videoFrame.width(), videoFrame.height(), tmp.depth());
				_observer->notifyVideoRect( videoFrame );
				_observer->notifyAnalysisRect( videoFrame );
			}
			else
			{
		// Use true frame interval
				_currentTime += frameInt;
				rmAssert(frameInt > 0.0 );// Zero time between frames is not allowed
			}

			prevTimeStamp = curTimeStamp;

		// PreProcess the image if needed for Display
		// @note see and understand the above before you alter this code!
			ipp (tmp, image, curTimeStamp);
			_fileImages.push_back( image );
                    
			rcVideoWriter* videoWriter;
			videoWriter = _videoWriter.getValue(videoWriter);
			if (videoWriter != 0)
				videoWriter->writeValue( curTimeStamp, rcRect(), &image );

			const rcTimestamp curTime =  rcTimestamp::now();
			rcTimestamp updateInterval = curTime - lastStatusUpdateTime;

		// Update status bar
			if ( updateInterval.secs() > updateStatusInterval)  {
				lastStatusUpdateTime = curTime;
				char buf[512];
				snprintf( buf, rmDim( buf ), "Loading %i frames...%i%% complete", frameMax, int(double(i)/frameMax * 100) );
				_observer->notifyStatus( buf );
			}
			updateInterval = curTime - lastMovieUpdateTime;

			if ( updateInterval.secs() > updateMovieInterval ) {
		// Update movie display
				lastMovieUpdateTime = curTime;
				if ( _observer->acceptingImageBlits() )
					_observer->notifyBlitData( &image );
			}

		}// End of For i++

	// @note this is the place to do motion compensation while we load images

	// Update elapsed time
		_observer->notifyTime( getElapsedTime() );
		_observer->notifyTimelineRange( 0.0, getElapsedTime() );
		flushSharedWriters ();

		if (_inputMode != eCmd)
			setInternalState( eEngineStopped );

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

	if ( !_fileImages.empty() ) {
		// Set analysis rect            
		setSettingValue( cAnalysisRectSettingId, rcRect(0, 0, _frameWidth, _frameHeight ) );
		_observer->notifyAnalysisRect( getSettingValue( cAnalysisRectSettingId ) );                
		// Display final status
		{
			char buf[512];
			snprintf( buf, rmDim( buf ), "%li %s frames [%d by %d] loaded\n",
				_fileImages.size(),
				_fileImages[0].isGray() ? "gray scale" : "color",
				_fileImages[0].size().x(), _fileImages[0].size().y());
			_observer->notifyStatus( buf );
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
		// Nothing loaded
		cout << "done" << endl;
		cout << _fileImages.size() << " frames loaded in " << duration.secs() << " seconds" << endl;
	}

	updateHeaderLogs();



	return i;
}

// Load images to memory, return number of images loaded
int rcEngineImpl::loadImages()
{
	int count = 0;

	if ( getState() != eEngineRunning ) {
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
				rcImageGrabber grabber( fileNames, this, frameInterval );

			// Load frames
				rcFrameGrabberError error = eFrameErrorUnknown;
				count = loadFrames( grabber, error );

				if ( error != eFrameErrorOK )
				{
					strstream s;
					s << rcFrameGrabber::getErrorString( error ) << " " << grabber.getInputSourceName() << ends;
					_observer->notifyError( s.str() );
					_observer->notifyStatus( s.str() );
					s.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
					cerr << s.str() << endl;
				}

			}
		}
		return count;
	}

// Load movie frames to memory, return number of frames loaded
int rcEngineImpl::loadMovie()
{
    int count = 0;

    if ( getState() != eEngineRunning ) {
        if ( !_movieFile.empty() ) {
            rcFrameGrabberError error = eFrameErrorUnknown;

            count = loadMovie( _movieFile, error );

	    cerr << count << " Frames in the movie " << _movieFile << endl;

            if ( error != eFrameErrorOK ) {

	      cerr << "Loading Movie Again: " << endl;

                if ( error == eFrameErrorFileInit ) {
                    // File not found, look for file in the current directory

                    // Get file name without preceding path
                    std::string rawFileName = _movieFile;
                    std::string slash( "/" );
                    uint32 s = _movieFile.find_last_of( slash );
                    if ( s != std::string::npos ) {
                        uint32 len = _movieFile.size() - s - 1;
                        if ( len > 0 )
                            rawFileName = _movieFile.substr( s+1, len );
                    }
                    rcPersistenceManager* pm = rcPersistenceManagerFactory::getPersistenceManager();
                    // Get current import directory
                    std::string importDir = pm->getImportDirectory();
                    // Concatenate import directory and movie file name
                    std::string newMovieFile = importDir + rawFileName;

                    std::string origMovieFile = _movieFile;
                    // Change movie name to the new name
                    _movieFile = newMovieFile;

                    error = eFrameErrorUnknown;
                    // Try loading with the same file name but current import directory
                    count = loadMovie( newMovieFile, error );

		    cerr << count << " Frames in the movie " << _movieFile << endl;

                    if ( error == eFrameErrorOK ) {
                        cerr << "Loaded " << newMovieFile << " because ";
                        cerr << origMovieFile << " could not be found" << endl;
                    } else {
                        // Report failed file name without path as status
                        {
                            strstream s;
                            s << rcFrameGrabber::getErrorString( error ) << " " << rawFileName << ends;
                            _observer->notifyStatus( s.str() );
                            s.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
                        }
                        // Report both load failure paths as a popup error
                        strstream s;
                        s << rcFrameGrabber::getErrorString( error ) << " " << origMovieFile << endl;
                        if ( _movieFile != origMovieFile )
                            s << rcFrameGrabber::getErrorString( error ) << " " << _movieFile;
                        s << ends;
                        s.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
                        _observer->notifyError( s.str() );

                        cerr << s.str() << endl;
                    }
                } else {
                    // Load failed, display an error dialog
                    if ( error != eFrameErrorOK ) {
                        strstream s;
                        s << rcFrameGrabber::getErrorString( error ) << " " << _movieFile << ends;
                        _observer->notifyError( s.str() );
                        _observer->notifyStatus( s.str() );
                        s.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
                        cerr << s.str() << endl;
                    }
                }
            }
        }
    }

    return count;
}

// Load movie frames to memory, return number of frames loaded
int rcEngineImpl::loadMovie( const std::string& movieFile, rcFrameGrabberError& error )
{
	int count = 0;
	vector<rcWindow> zstk;

	if ( !movieFile.empty() ) {
		rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
		std::string reifyMovieExt = persistenceManager->fileFormatExportExtension( eExperimentNativeMovieFormat );
		std::string stkMovieExt = persistenceManager->fileFormatExportExtension( eExperimentMolDevSTKFormat );

		// Create movie grabber
		rcFrameGrabber* grabber = 0;

		std::string fileExt = rfStripPath( movieFile );
		fileExt = rfGetExtension (fileExt);

		if ( insensitive_case_compare (fileExt, reifyMovieExt ) )
		{
			rmAssert(_videoCacheP == 0);
			double physMem(rfSystemRam ());
			const uint32 maxMem = (1024*1024*256);
			physMem = rmMin (physMem / 4, maxMem);
			_videoCacheP =
				rcVideoCache::rcVideoCacheCtor(movieFile, 0, true, false,
				true, physMem, _videoCacheProgress );
			cerr << "Video cache size " << physMem/(1024*1024) << " MB" << endl;
			grabber = new rcReifyMovieGrabber(*_videoCacheP);
		}
		else if (insensitive_case_compare(fileExt, stkMovieExt) )
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
				// Forced frame rate
			double frameInterval = _frameRate ? 1.0/_frameRate : 0.0;
			grabber = reinterpret_cast<rcFrameGrabber*> (new rcCinderGrabber( movieFile, this, frameInterval ));
		}


		// Load frames
		count = loadFrames( *grabber, error );
		delete grabber;

		if ( error != eFrameErrorOK ) {
			if (_videoCacheP) {
				rcVideoCache::rcVideoCacheDtor(_videoCacheP);
				_videoCacheP = 0;
			}
		}



		if (_videoCacheP) {
			// Experiment headers
			const vector<rcMovieFileExpExt>& expExts = _videoCacheP->movieFileExperiments();
			if ( !expExts.empty() ) {
				rcMovieFileExpExt exp = expExts[0];
				// Set experiment attributes
				rcExperimentAttributes attrs = attributes( exp );
				_observer->setExperimentAttributes( attrs );
			}
		}
	}

	return count;
}


// Generic method to load frames from a rcFrameGrabber
int rcEngineImpl::saveFrames(std::string imageExportDir) const
{
    if (imageExportDir.empty()) return 0;

    vector<rcWindow>::const_iterator imgItr = _fileImages.begin ();

    rcTimestamp duration = rcTimestamp::now();

    int32 i = 0;
    for (; imgItr != _fileImages.end(); imgItr++, i++)
    {
        std::ostringstream oss;
        oss << _exportImagesDir << "/" << "image" << setfill ('0') << setw(4) << i << ".jpg";
        std::string fn (oss.str ());
        rfImageExport2JPG ( *imgItr, fn);
        fn = std::string ("chmod 644 ") + fn;
        ::system( fn.c_str() );

    }

    // Done. Report
    duration = rcTimestamp::now() - duration;
    cout <<  endl << i << " Images Exported in " << duration.secs() << "Seconds" << endl;

    return i;
}


/******************************************************************************
 *	Engine IPP process
 ******************************************************************************/
void rcEngineImpl::ipp (const rcWindow& tmp, rcWindow& image, rcTimestamp& current)
{
  rmUnused (current);

  // What the user has selected

  rcIPPMode userMode = (rcIPPMode) (int) getSettingValue(cImagePreMappingSettingId);

  // VideoCache case: Do not access frameData. Access for invalidate cache
  if (userMode == eIPPNone && _videoCacheP)
    {
      image = tmp;
      image.frameBuf()->setTimestamp (current);
      return;
    }

  if (userMode == eIPPNone && (tmp.depth() == rcPixel8 || !tmp.isGray ()))
    {
      image = tmp;
      image.frameBuf()->setTimestamp (current);      
      return;
    }

  if (userMode == eIPPNone && tmp.depth() == rcPixel16)
    {
      image = tmp;
      image.frameBuf()->setTimestamp (current);      
      return;
    }

  if (userMode == eIPPNone && tmp.depth() == rcPixel32)
    {
      image = rfImageConvert32to8 (tmp, _channelConversion);
      image.frameBuf()->setTimestamp (current);      
      return;
    }  
}
