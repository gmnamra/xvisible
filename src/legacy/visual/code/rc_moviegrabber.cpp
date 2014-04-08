/*
 *  rc_moviegrabber.cpp
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_qtime.h>
#include <rc_moviegrabber.h>
#include <rc_ipconvert.h>

#define kBogusStartingTime  -1      // an invalid starting time
#define Debug_Log

//
// Local utilities
//

// Get the image description for the video media track

static OSErr getVideoMediaImageDesc ( Media theMedia, SampleDescriptionHandle anImageDesc )
{
    OSErr anErr = noErr;
    OSType mediaType;
	
    rmAssert( anImageDesc != NULL );
    rmAssert( theMedia != NULL );
    rmAssert( index > 0 );
	
    // Test if we are indeed dealing with video media.
    GetMediaHandlerDescription( theMedia, &mediaType, NULL, NULL );
	
    if ( mediaType != VideoMediaType )
        return invalidMedia;
	
    GetMediaSampleDescription( theMedia, 1, anImageDesc );
	
    // Check to see if there were any errors in the movie
    anErr = GetMoviesError();
	
    return anErr;
}

//
// rcMovieGrabber implementation
//

rcMovieGrabber::rcMovieGrabber( const std::string fileName, rcCarbonLock* cLock, 
                                double frameInterval, int32 startFrame, int32 frames) :
    rcFileGrabber( cLock ),
    mFileName( fileName ),
    mFrameCount(-1),
    mRefnum( 0 ), mMovie( 0 ), mMedia( 0 ), mTimeScale( 0 ),
    mCurrentTime( kBogusStartingTime ), mCurrentIndex( 0 ), mGotFirstFrame( false ),
    mFrameInterval( frameInterval ), mCurrentTimeStamp( frameInterval ), mMovieStartFrame (startFrame), mClipFrames (frames), m_ctb (NULL)
{
    lock();
    
    OSErr err;
	
    if ( mFileName.empty() )
        setLastError( eFrameErrorFileInit );
    else
    {
        bool isDir = false;
        mFileSpec = qtime::rfMakeFSSpecFromPosixPath( mFileName.c_str(), isDir);
        err = qtime::ValidFSSpec( &mFileSpec );
        // Cannot access file for some reason
        if ( err != noErr )
            setLastError( eFrameErrorFileInit );
		
        // Initialize QuickTime
        err = EnterMovies();
		
        if ( err != noErr ) {
            setLastError( eFrameErrorQuicktimeInit );
        }
    }
    
    unlock();
}


rcMovieGrabber::~rcMovieGrabber()
{
    if( m_ctb ) {
        printf( "\n dtor: remove color table.\n" );
        DisposeCTable( m_ctb );
    }
	
}

// Start grabbing
bool rcMovieGrabber::start()
{
    lock();
    ClearMoviesStickyError();  
	
    // open a Movie file using the FSSpec and create a Movie from that file.
    OSErr err = OpenMovieFile( &mFileSpec, &mRefnum, fsRdPerm );
    mFrameTimes.clear ();
	
    if ( err == noErr )
    {
        NewMovieFromFile( &mMovie, mRefnum, NULL, NULL, newMovieActive, NULL );
        
        mMedia = GetTrackMedia( GetMovieIndTrack ( mMovie, 1 ) );
        // Allocate a handle for sample description
        SampleDescriptionHandle anImageDesc = (SampleDescriptionHandle) NewHandle( sizeof(Handle) );
				
        // Get the image description for the movie
        err = getVideoMediaImageDesc( mMedia, anImageDesc );
				
        if  ( err == noErr ) 
        {
            getPixelInfoFromImageDesc ( (ImageDescriptionHandle) anImageDesc);
					
            mTimeScale = GetMovieTimeScale(mMovie);
            ::Rect tmp = {0,0,0,0};
            GetMovieBox(mMovie, &tmp);
            m_width =  tmp.right - tmp.left;
            m_height = tmp.bottom - tmp.top;
            m_qt_movie_rect.right = m_width; m_qt_movie_rect.bottom = m_height; 
            m_qt_movie_rect.left = tmp.left; m_qt_movie_rect.top = tmp.top;
            // count the number of video 'frames' in the movie by stepping through all of the
            // video 'interesting times', or in other words, the places where the movie displays
            // a new video sample. The time between these interesting times is not necessarily constant.
            {
                OSType      whichMediaType = VisualMediaCharacteristic;
                TimeValue   theTime        = -1;
						
                // find out movie start time
                GetMovieNextInterestingTime (mMovie, short (nextTimeMediaSample + nextTimeEdgeOK), 
                                             1, & whichMediaType, TimeValue (0), 0, & theTime, NULL);
                if (theTime == -1)
                {
                    fprintf (stderr, "Couldn't inquire first frame time\n");
                    return 0;
                }
                mCurrentTime  = theTime;
                // count all 'interesting times' of the movie
                while (theTime >= 0) 
                {
                    mFrameTimes.push_back (theTime);
                    GetMovieNextInterestingTime (mMovie, short (nextTimeMediaSample), 
                                                 1, & whichMediaType, theTime, 0, & theTime, NULL);
                }
                mFrameCount = mFrameTimes.size ();
            }
					
        } 
        else {
            setLastError( eFrameErrorUnsupportedFormat );
        }
        DisposeHandle( (Handle) anImageDesc );
    }

	
        unlock();
    
        if ( getLastError() == eFrameErrorOK )
            return true;
        else
            return false;
}

// Stop grabbing
bool rcMovieGrabber::stop()
{
    lock();
    
    DisposeMovie( mMovie );
	
    if ( mRefnum )
        CloseMovieFile( mRefnum );
	
    CloseComponent( mImporter );
	
    unlock();
    
    return true;
}

// Returns the number of frames available
int32 rcMovieGrabber::frameCount()
{
    return mFrameCount;
}

// Get next frame, assign the frame to ptr
rcFrameGrabberStatus
rcMovieGrabber::getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking )
{
    lock();
	
    rcFrameGrabberStatus ret =  eFrameStatusOK;
    setLastError( eFrameErrorUnknown );
    if (mCurrentIndex < mFrameTimes.size())
    {
        ptr = NULL;
        ret = eFrameStatusOK;
        TimeScale  timescale   = GetMovieTimeScale      (mMovie);
        short flags = nextTimeMediaSample;
        flags += nextTimeEdgeOK;
	
        rcWindow image;
		
        // Get the frame and convert it from 32 to 8 bit if possible
        OSErr status = getNextVideoSample(image, mMedia, mFrameTimes[mCurrentIndex], true );
		
        rcTimestamp tp ( ( mFrameInterval > 0.0 ) ? mCurrentIndex *  mFrameInterval 
                         : static_cast<double> (mFrameTimes[mCurrentIndex] / 1000.0 * timescale) );

        if ( status == noErr )
        {
            ptr = image.frameBuf();
            image.frameBuf()->setTimestamp(tp);
            ret = eFrameStatusOK;
            setLastError( eFrameErrorOK );
            mCurrentIndex++;
        } 
        else
        {
            ret = eFrameStatusError;
            // TODO: analyze OSErr and map it to proper eFrameError
            setLastError( eFrameErrorFileRead );
        }
    }
    else
        ret = eFrameStatusEOF;
	
    unlock();
    
    return ret;
}

//
// Private methods
//


// Get name of input source, ie. file name, camera name etc.
const std::string rcMovieGrabber::getInputSourceName()
{
    return mFileName;
}


// From an a quickTime imagedescription we can figure out a few things about the frame in the image:
// depth: Contains the pixel depth specified for the compressed image:
// Values of 1, 2, 4, 8, 16, 24, and 32 indicate the depth of color images. 
// Values of 34, 36, and 40 indicate 2-bit, 4-bit, and 8-bit grayscale, respectively, for grayscale images.
// For us the only acceptable images are 8, 16, 24, and 32 in color and 40 which is 8 bit grey.
// We detect a gray image through size and equality of color mappings
// TBD: what is the best way of handling 32 bit images? The issue here is how to treat alpha and also byte ordering. 

void rcMovieGrabber::getPixelInfoFromImageDesc (ImageDescriptionHandle anImageDesc)
{
	
    int16 aPixelDepth = (*anImageDesc)->depth;
    // Get the color table
    CTabHandle m_ctb = NULL;
    GetImageDescriptionCTable (anImageDesc,&m_ctb);
    int16 csize = (*m_ctb)->ctSize;
	
    m_isGray = false;
    m_isWhiteReversed = false;
	
    // Check grayness
    if ( csize > 0 ) {
        m_isGray = true;
        // Test each color table entry
        for ( int16 i = 0; i < csize + 1; ++i ) {
            const int r = (*m_ctb)->ctTable[i].rgb.red;
            const int g = (*m_ctb)->ctTable[i].rgb.green;
            const int b = (*m_ctb)->ctTable[i].rgb.blue;
            if ( r != g || g != b ) {
                m_isGray = false;
                break;
            }
        }
        if ( m_isGray ) {
            // Test whether index 0 contains white
            m_isWhiteReversed = true;
			
            for ( int16 i = 1; i < csize + 1; ++i ) {
                const int prev = (*m_ctb)->ctTable[i-1].rgb.red;
                const int cur = (*m_ctb)->ctTable[i].rgb.red;
                if ( prev < cur ) {
                    m_isWhiteReversed = false;
                    break;
                }
            }
        }
    }
	
	
    switch( aPixelDepth ) {
        case 8:
            m_pd = rcPixel8;
            m_pf = k8IndexedPixelFormat;
            break;
        case 16:
            m_pd = rcPixel32;
            m_pf = k32ARGBPixelFormat;
            break;
        case 24:
            m_pd = rcPixel32;
            m_pf = k32ARGBPixelFormat;
            break;
        case 32:
            m_pd = rcPixel32;
            m_pf = k32ARGBPixelFormat;
            break;
        case 40:
            m_pd = rcPixel8;
            m_pf = k8IndexedGrayPixelFormat;
            break;
        default:
            m_pd = rcPixelUnknown;
            m_pf = k32ARGBPixelFormat;
            break;
    }
	
#ifdef DEBUG_LOG
    fprintf (stderr, "Pixel depth in movie file is %d ==> %d bytes in depth - %s - %s - %s - color table size %i\n",
             aPixelDepth, m_pd, m_isGray ? "is Gray" : "is not Gray",
             m_isWhiteReversed ? "0 is white" : "0 is black",
             m_pf == k8IndexedPixelFormat ? "k8IndexedPixelFormat" :
             (m_pf == k8IndexedGrayPixelFormat) ? "k8IndexedGrayPixelFormat" :
             (m_pf == k32ARGBPixelFormat) ? "k32ARGBPixelFormat" : "Other",
             csize );
#endif            
}


OSErr rcMovieGrabber::getNextVideoSample(rcWindow& image, Media media, TimeValue fromTimePoint,
                                         bool reduceGrayTo8bit)
{
    OSErr 			anErr = noErr;
    GWorldPtr		gWorld = NULL;
    rcWindow 		tmp, tmp2;
    PixMapHandle	pixMap = NULL;

    assert(mMovie != NULL); 
	
    // Create aGWorld for writing the movie in to 
    // We will allocate GWorld memory ourselves as a framebuf ===> Dispose will not free this memory
    try
    {
        rcPixel depth = m_pd;
		
        rcSharedFrameBufPtr buf(new rcFrame (m_width, m_height, depth));
        buf->setIsGray(m_isGray);
        rcWindow rw (buf);
        tmp = rw;
        tmp2 = rw;
        assert (m_pf);
        anErr = QTNewGWorldFromPtr( &gWorld, m_pf, &m_qt_movie_rect, m_ctb, NULL, 0, (char *) (tmp.rowPointer(0)), tmp.rowUpdate ());
		
        if (noErr != anErr)
        {
            goto Closure; // Should throw an exception ????
        }
		
        if ( reduceGrayTo8bit && m_isGray && depth > rcPixel8 ) 
        {
            depth = rcPixel8;
            rcSharedFrameBufPtr buf8(new rcFrame (m_width, m_height, depth));
            buf8->setIsGray(m_isGray);
            // Produce color map
            qtime::rfFillColorMap( m_ctb, buf8 );
            rcWindow image8(buf8);
            // Reduce image depth
            rfRcWindow32to8( tmp, image8 );
            tmp2 = image8;
        } else {
            // Produce color map if necessary
            if ( depth < rcPixel32 && !m_isWhiteReversed) {
                qtime::rfFillColorMap( m_ctb, buf );              
            }
        }
    }
    catch (...)
    {
        goto Closure;
    }
	
    //? Draw the next first video sample (frame) to GWorld number.
    SetMovieTimeValue(mMovie, fromTimePoint);
    SetMovieGWorld(mMovie, gWorld, GetGWorldDevice(gWorld));
	
    // Note that the work in rebuilding the drawing pipeline is not actually done in SetMovieGWorld,
    // but during the next call to MoviesTask. 
    // invalidates the movie's display state ==>  
    // Movie Toolbox redraws the movie the next time MoviesTask is called
    UpdateMovie (mMovie);
    anErr = GetMoviesError ();
    if (anErr != noErr)
    {
        fprintf (stderr, "Couldn't UpdateMovie() \n");
        return 0;
    }
	
    //  (= redraw immediately)
    MoviesTask (mMovie, 0L);
    anErr = GetMoviesError ();
    if (anErr != noErr)
    {
        fprintf (stderr, "MoviesTask() didn't succeed \n");
        return 0;
    }
	
    pixMap = GetGWorldPixMap(gWorld);
    LockPixels(pixMap);
    UnlockPixels(pixMap);
	
    if ( m_isWhiteReversed && tmp2.depth() == rcPixel8 ) {
        // Color map indicates that pixel value 0 is white
        // Reverse pixels and use default color map
        rfReversePixels8( tmp2 );
    }
	
    // Copy it to the vector
    image = tmp2;
	
    //? Closure. Clean up if we have handles.
	
Closure:
    if(gWorld != NULL)
        DisposeGWorld(gWorld);
    return anErr;
}




