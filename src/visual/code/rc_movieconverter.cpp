/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 * $Id: rc_movieconverter.cpp 7284 2011-03-01 23:32:47Z arman $
 *
 * Movie converter classes
 * @file
 ***************************************************************************/

#include <libgen.h>
#include <unistd.h>
#include <iostream>
#include <rc_movieconverter.h>
#include <rc_videocache.h>
#include <rc_imagegrabber.h>
#include <file_system.hpp>
#include <rc_stlplus.h>



#define kVideoTimeScale 1000 // 1000 units per second
// Video cache size
static uint32 cVideoCacheMem = 1024*1024*32;
static uint32 cVideoCacheSize = 10;

#define cleanupAndReturn                        \
    {                                           \
        SetGWorld(mySavedPort, mySavedDevice);  \
        if (ctabHdl != NULL)                    \
            DisposeHandle((Handle)ctabHdl);     \
        if (myImageDesc != NULL)                \
            DisposeHandle((Handle)myImageDesc); \
        if (myComprDataHdl != NULL)             \
            DisposeHandle(myComprDataHdl);      \
        if (myGWorld != NULL)                   \
            DisposeGWorld(myGWorld);            \
        if (myErr)                              \
            printf("Died %d\n", (int)myErr);    \
        return myErr;                           \
    }

static movieChannelType channelType( rcChannelConversion c )
{
    switch ( c ) {
        case rcSelectRed:
            return movieChannelRed;
        case rcSelectGreen:
            return movieChannelGreen;
        case rcSelectBlue:
            return movieChannelBlue;
        case rcSelectAverage:
            return movieChannelAvg;
        case rcSelectMax:
            return movieChannelMax;
        case rcSelectAll:
            return movieChannelAll;
    }

    return movieChannelUnknown;
}

//
// rcMovieConverterOptions class implementation
//

rcMovieConverterOptions::rcMovieConverterOptions() :
        mFrameInterval( -1.0 ),
        mFirstFrameIndex( 0 ),
        mFrameCount( -1 ),
        mFrameOffset( 0 ),
        mSamplePeriod( 1 ),
        mCreator( "Unknown" ),
        mClipRect( rcRect() ),
        mOverWrite( true ),
        mReversePixels( false ),
        mVerbose( true )
{
}

rcMovieConverterOptions::rcMovieConverterOptions( const rcMovieConverterOptions& o ) :
        mFrameInterval( o.frameInterval() ),
        mFirstFrameIndex( o.firstFrameIndex() ),
        mFrameCount( o.frameCount() ),
        mFrameOffset( o.frameOffset() ),
        mSamplePeriod( o.samplePeriod() ),
        mCreator( o.creator() ),
        mClipRect( o.clipRect() ),
        mOverWrite( o.overWrite() ),
        mReversePixels( o.reversePixels() ),
        mVerbose( o.verbose() )
{
}

rcMovieConverterOptions::~rcMovieConverterOptions()
{
}

//
// rcMovieConverterOptionsRfy class implementation
//

rcMovieConverterOptionsRfy::rcMovieConverterOptionsRfy() :
        rcMovieConverterOptions::rcMovieConverterOptions(),
        mChannelConversion( rcSelectAverage ),
        mRev( movieFormatRevLatest )
{
}

rcMovieConverterOptionsRfy::rcMovieConverterOptionsRfy( const rcMovieConverterOptions& o ) :
        rcMovieConverterOptions( o ),
        mChannelConversion( rcSelectAverage ),
        mRev( movieFormatRevLatest )
{
}

rcMovieConverterOptionsRfy::~rcMovieConverterOptionsRfy()
{
}

//
// rcMovieConverterOptionsQT class implementation
//

rcMovieConverterOptionsQT::rcMovieConverterOptionsQT() :
        rcMovieConverterOptions::rcMovieConverterOptions()
{
}

rcMovieConverterOptionsQT::rcMovieConverterOptionsQT( const rcMovieConverterOptions& o ) :
        rcMovieConverterOptions( o )
{
}

rcMovieConverterOptionsQT::~rcMovieConverterOptionsQT()
{
}

//
// rcMovieConverter class implementation
//

std::string rcMovieConverter::getErrorString( const rcMovieConverterError& error )
{

    switch ( error ) {
        case eMovieConverterErrorOK:
            return std::string("rcMovieConverterToRfy: OK");
        case eMovieConverterErrorUnknown:
            return std::string("rcMovieConverterToRfy: unknown error");
        case eMovieConverterErrorInternal:
            return std::string("rcMovieConverterToRfy: internal error");
        case eMovieConverterErrorNull:
            return std::string("rcMovieConverterToRfy: NULL error");
        case eMovieConverterErrorHeaderWrite:
            return std::string("rcMovieConverterToRfy: cannot write output file header");
        case eMovieConverterErrorExists:
            return std::string("rcMovieConverterToRfy: output file exists");
        case eMovieConverterErrorOpen:
            return std::string("rcMovieConverterToRfy: cannot open input file");
        case eMovieConverterErrorRead:
            return std::string("rcMovieConverterToRfy: cannot read input file");
        case eMovieConverterErrorWriteOpen:
            return std::string("rcMovieConverterToRfy: cannot open output file");
        case eMovieConverterErrorWrite:
            return std::string("rcMovieConverterToRfy: cannot write to output file");
        case eMovieConverterErrorSeek:
            return std::string("rcMovieConverterToRfy: seek failed for input file");
        case eMovieConverterErrorClose:
            return std::string("rcMovieConverterToRfy: cannot close output file");
        case eMovieConverterErrorFlush:
            return std::string("rcMovieConverterToRfy: cannot flush output file");
        case eMovieConverterErrorUnsupported:
            return std::string("rcMovieConverterToRfy: unsupported operation");
    }

    return std::string("rcMovieConverterToRfy: undefined error");
}

bool rcMovieConverter::isReifyMovie( const std::string& name )
{
    if ( !name.empty() ) {
        int lth = name.size();
        if ( lth > 7 && !strcasecmp(&name[lth-7], ".rfymov")) {
            return true;
        }
    }

    return false;
}

bool rcMovieConverter::isImageCollection( const std::string& name )
{
    if ( !name.empty() ) {
        if ( strchr( name.c_str(), ';' ) )
            return true;
    }

    return false;
}

rcSharedFrameBufPtr rcMovieConverter::clippedFrame( rcSharedFrameBufPtr orig,
                                                     rcMovieConverterOptions opt )
{
    const rcRect& clipRect = opt.clipRect();

    if ( clipRect.width() > 0 && clipRect.height() > 0 ) {
        // Clip
        rcWindow w( orig, clipRect );
        // Make a copy so input pixels are not mutated
        rcSharedFrameBufPtr newBuf = new rcFrame( clipRect.width(), clipRect.height(), orig->depth() );
        newBuf->setTimestamp( orig->timestamp() );
        rcWindow newW( newBuf );
        newW.copyPixelsFromWindow( w );
        return newBuf;
    }

    return orig;
}

// Lock
void rcMovieConverter::lock()
{
    if ( mCarbonLock )
        mCarbonLock->lock();
}

// Unlock
void rcMovieConverter::unlock()
{
    if ( mCarbonLock )
        mCarbonLock->unlock();
}


//
// rcMovieConverterToRfy class implementation
//

rcMovieConverterToRfy::rcMovieConverterToRfy( const rcMovieConverterOptionsRfy& opt,
                                              rcCarbonLock* l,
                                              rcProgressIndicator* p ) :
        rcMovieConverter( opt.verbose(), l, p ),
        mOptions( opt )
{
}

rcMovieConverterToRfy::~rcMovieConverterToRfy()
{
}


// Convert QT movie to .rfymov
rcMovieConverterError rcMovieConverterToRfy::convert( const std::string& inputFile,
                                                      const std::string& outputFile )
{
    mLastError = eMovieConverterErrorUnknown;
    bool srcReify = isReifyMovie( inputFile );

    if ( srcReify ) {
        rcVideoCache* cacheP = 0;

        cacheP = rcVideoCache::rcVideoCacheCtor( inputFile, cVideoCacheSize, false, false, false, cVideoCacheMem );

        if (!cacheP->isValid()) {
            if ( verbose() )
                cerr << rcVideoCache::getErrorString( cacheP->getFatalError() ) << endl;
            mLastError = eMovieConverterErrorOpen;
        } else {
            mLastError = createRfyMovie( cacheP, outputFile );
        }
        rcVideoCache::rcVideoCacheDtor( cacheP );
    } else {
        if ( !isImageCollection( inputFile ) ) {
            rcMovieGrabber grabber( inputFile, mCarbonLock, mOptions.frameInterval() );
            mLastError = createRfyMovie( grabber, outputFile );
        } else {
            vector<std::string> fileNames;
            // Export selected images
            if ( !inputFile.empty() ) {
                vector<const char*> files;
                std::string::size_type str = 0, idx = 0;
                int32 i = 0;
                int32 firstIndex = mOptions.firstFrameIndex();
                int32 frameCount = mOptions.frameCount();

                while ( str < inputFile.size() ) {
                    idx = inputFile.find( ';', str );
                    if ( idx != std::string::npos ) {
                        if ( i >= firstIndex ) {
                            if ( frameCount >= 0 && fileNames.size() == uint32(frameCount) )
                                break;
                            std::string oneFile = inputFile.substr( str, idx - str );
                            fileNames.push_back( oneFile );
                        }
                        str = idx + 1;
                        ++i;
                    } else
                        break;
                }
#ifdef DEBUG_LOG
                cerr << "Exporting " <<  fileNames.size() << " images " << endl;
                for ( uint32 x = 0; x < fileNames.size(); ++x ) {
                    cerr << fileNames[x] << endl;
                }
#endif
                // Hack: mutate frame interval for image grabber
                if ( mOptions.frameInterval() <= 0.0 )
                    mOptions.frameInterval(1.0);
                rcImageGrabber grabber( fileNames, mCarbonLock, mOptions.frameInterval() );
                mLastError = createRfyMovie( grabber, outputFile );
            }
        }
    }

    return mLastError;
}



// Convert QT movie to .rfymov
rcMovieConverterError rcMovieConverterToRfy::convert( const std::string& inputFile, const std::string& input2File,
                                                      const std::string& outputFile, movieOriginType mot, std::string& typeInfo )
{
    mLastError = eMovieConverterErrorUnknown;
    bool srcReify = isReifyMovie( inputFile ) && isReifyMovie ( input2File );

    if ( srcReify ) {
        rcVideoCache* cacheP = 0;
        rcVideoCache* cache2P = 0;

        cacheP = rcVideoCache::rcVideoCacheCtor( inputFile, cVideoCacheSize, false, false, false, cVideoCacheMem );
        cache2P = rcVideoCache::rcVideoCacheCtor( input2File, cVideoCacheSize, false, false, false, cVideoCacheMem );

        if (!cacheP->isValid() || !cache2P->isValid()) {
	  if ( verbose() )
	    cerr << rcVideoCache::getErrorString( cacheP->getFatalError() ) << endl;
	  mLastError = eMovieConverterErrorOpen;
        } else {
	  mLastError = createRfyMovie( cacheP, cache2P, outputFile, mot, typeInfo );
        }
        rcVideoCache::rcVideoCacheDtor( cacheP );
        rcVideoCache::rcVideoCacheDtor( cache2P );
    }

    return mLastError;
}

// Create Rfy movie from another Rfy movie
rcMovieConverterError rcMovieConverterToRfy::createRfyMovie( rcVideoCache* inputCache,
                                                             const std::string& outputFile )
{
    rcMovieConverterError error = eMovieConverterErrorOK;
    rmAssert(inputCache);
    rcVideoCache& cache = *inputCache;

    // Copy origin headers
    const vector<rcMovieFileOrgExt>& orgHdrs = cache.movieFileOrigins();
    rmAssert( !orgHdrs.empty() );

    // Reify movie generator
    rcGenMovieFile generator( outputFile,
                              mOptions.channelConversion(),
                              mOptions.rev(),
                              mOptions.overWrite(),
                              mOptions.frameInterval() );

    bool failed = !generator.valid();
    if ( failed ) {
        if ( verbose() )
            cerr << "rcGenMovieFile ctor failed" << endl;
        return eMovieConverterErrorInternal;
    }

    for ( uint32 i = 0; i < orgHdrs.size(); ++i ) {
        const rcMovieFileOrgExt& org = orgHdrs[i];
        generator.addHeader( org );
    }

    // Copy camera headers
    const vector<rcMovieFileCamExt>& camHdrs = cache.movieFileCameras();

    for ( uint32 i = 0; i < camHdrs.size(); ++i ) {
        const rcMovieFileCamExt& cam = camHdrs[i];
        generator.addHeader( cam );
    }

    // Copy experiment headers
    const vector<rcMovieFileExpExt>& expHdrs = cache.movieFileExperiments();

    for ( uint32 i = 0; i < expHdrs.size(); ++i ) {
        const rcMovieFileExpExt& exp = expHdrs[i];
        generator.addHeader( exp );
    }

    rcSharedFrameBufPtr frameBuf;
    rcVideoCacheError verror;
    rcVideoCacheStatus status = cache.getFrame(mOptions.frameOffset(), frameBuf, &verror);
    if (status != eVideoCacheStatusOK) {
        if ( verbose() )
            cerr << "Couldn't read frame: " << 0 << " error: "
                 << rcVideoCache::getErrorString(verror) << endl;
        return eMovieConverterErrorRead;
    }

    uint32 converted = 0;
    int32 frameCount = mOptions.frameCount();
    if ( frameCount < 0 )
        frameCount = cache.frameCount();
    const uint32 endMark = mOptions.frameOffset() +
        (mOptions.firstFrameIndex() + frameCount)*mOptions.samplePeriod();
    for (uint32 i = mOptions.frameOffset() + mOptions.firstFrameIndex()*mOptions.samplePeriod(); i < endMark; i += mOptions.samplePeriod()) {
        status = cache.getFrame(i, frameBuf, &verror);
        if (status != eVideoCacheStatusOK) {
            if ( verbose() )
                cerr << "Couldn't read frame: " << i << " error: "
                     << rcVideoCache::getErrorString(verror) << endl;
            return eMovieConverterErrorRead;
        }

        frameBuf = clippedFrame( frameBuf, mOptions );

        if ( mOptions.reversePixels() ) {
            rcWindow w( frameBuf );
            rfReversePixels8( w );
        }

        rcGenMovieFileError gerr = generator.addFrame( frameBuf );
        if ( gerr != eGenMovieFileErrorOK ) {
            if ( verbose() )
                cerr << "Output movie addFrame error: " << gerr << " " << outputFile << endl;
            return eMovieConverterErrorWrite;
        }

        ++converted;
        if ( mProgress )
            mProgress->progress( double(converted)/frameCount * 100 );

    } // End of: for (i = offset + firstIndex*period; i < endMark; i += period) {

    if (mOptions.rev() >= movieFormatRev1) {
        // Copy old conversion headers
        const vector<rcMovieFileConvExt>& conversions = cache.movieFileConversions();
        if ( !conversions.empty() ) {
            for ( uint32 i = 0; i < conversions.size(); ++i ) {
                rcMovieFileConvExt cnvHdr = conversions[i];
                 rcGenMovieFileError gerr = generator.addHeader( cnvHdr );
                 if ( gerr != eGenMovieFileErrorOK ) {
                     if ( verbose() )
                         cerr << "Output movie addHeader error: " << gerr << " " << outputFile << endl;
                     return eMovieConverterErrorHeaderWrite;
                 }
            }
        }

        // Add new conversion header
        rcMovieFileConvExt cnvHdr( mOptions.firstFrameIndex() + mOptions.frameOffset(),
                                   frameCount,
                                   mOptions.samplePeriod(),
                                   mOptions.clipRect(),
                                   movieChannelAll,
                                   mOptions.frameInterval(),
                                   mOptions.reversePixels(),
                                   mOptions.creator().c_str() );
        rcGenMovieFileError gerr = generator.addHeader( cnvHdr );
        if ( gerr != eGenMovieFileErrorOK ) {
            if ( verbose() )
                cerr << "Output movie addHeader error: " << gerr << " " << outputFile << endl;
            return eMovieConverterErrorHeaderWrite;
        }
    }

    // Flush and close file
    rcGenMovieFileError gerr = generator.flush();
    if ( gerr != eGenMovieFileErrorOK ) {
        if ( verbose() )
             cerr << "Output movie flush error: " << gerr << " " << outputFile << endl;
        return eMovieConverterErrorFlush;
    }

    return error;
}


// Create Rfy movie from another Rfy movie
rcMovieConverterError rcMovieConverterToRfy::createRfyMovie( rcVideoCache* inputCache, rcVideoCache* input2Cache,
                                                             const std::string& outputFile, movieOriginType mot, std::string& typeInfo)
{
    rcMovieConverterError error = eMovieConverterErrorOK;
    rmAssert(inputCache);
    rmAssert(input2Cache);
    rcVideoCache& cache = *inputCache;
    rcVideoCache& cache2 = *input2Cache;

    // Copy origin headers
    // @note for now copy headers only from the first movie
    const vector<rcMovieFileOrgExt>& orgHdrs = cache.movieFileOrigins();
    rmAssert( !orgHdrs.empty() );

    // Reify movie generator
    // @ note this is fixed for movieOriginGrayIsVisibleAlphaIsFlu.
    // @todo cleanup
    rcGenMovieFile generator( outputFile, mot,
                              rcSelectAll, mOptions.creator().c_str(),
                              mOptions.rev(),
                              mOptions.overWrite(),
                              mOptions.frameInterval() );

    bool failed = !generator.valid();
    if ( failed ) {
        if ( verbose() )
            cerr << "rcGenMovieFile ctor failed" << endl;
        return eMovieConverterErrorInternal;
    }

    //@note: this is a hack for now. Needs to be throught through
    const char * combined = "movieOriginGrayIsVisibleAlphaIsFlu";
    rcMovieFileOrgExt comb (mot,
			    orgHdrs[0].baseTime(),
			    orgHdrs[0].frameCount(),
			    orgHdrs[0].width(),
			    orgHdrs[0].height(),
			    rcPixel32,
			    orgHdrs[0].rev(),
			    typeInfo.c_str());

    generator.addHeader (comb);

    for ( uint32 i = 0; i < orgHdrs.size(); ++i ) {
        const rcMovieFileOrgExt& org = orgHdrs[i];
        generator.addHeader( org );
    }

    // Copy camera headers
    const vector<rcMovieFileCamExt>& camHdrs = cache.movieFileCameras();

    for ( uint32 i = 0; i < camHdrs.size(); ++i ) {
        const rcMovieFileCamExt& cam = camHdrs[i];
        generator.addHeader( cam );
    }

    // Copy experiment headers
    const vector<rcMovieFileExpExt>& expHdrs = cache.movieFileExperiments();

    for ( uint32 i = 0; i < expHdrs.size(); ++i ) {
        const rcMovieFileExpExt& exp = expHdrs[i];
        generator.addHeader( exp );
    }

    rcSharedFrameBufPtr frameBuf, frame2Buf;
    rcVideoCacheError verror;
    rcVideoCacheStatus status = cache.getFrame(mOptions.frameOffset(), frameBuf, &verror);

    if (status != eVideoCacheStatusOK) {
        if ( verbose() )
            cerr << "Couldn't read frame: " << 0 << " error: "
                 << rcVideoCache::getErrorString(verror) << endl;
        return eMovieConverterErrorRead;
    }

    status = cache2.getFrame(mOptions.frameOffset(), frame2Buf, &verror);

    // @note both movies have exact number of frames
    uint32 converted = 0;
    int32 frameCount = mOptions.frameCount();
    if ( frameCount < 0 )
        frameCount = cache.frameCount();
    const uint32 endMark = mOptions.frameOffset() +
        (mOptions.firstFrameIndex() + frameCount)*mOptions.samplePeriod();

    for (uint32 i = mOptions.frameOffset() + mOptions.firstFrameIndex()*mOptions.samplePeriod();
	 i < endMark;
	 i += mOptions.samplePeriod())
      {
        status = cache.getFrame(i, frameBuf, &verror);
        if (status != eVideoCacheStatusOK) {
            if ( verbose() )
                cerr << "Couldn't read frame: " << i << " error: "
                     << rcVideoCache::getErrorString(verror) << endl;
            return eMovieConverterErrorRead;
        }

        status = cache2.getFrame(i, frame2Buf, &verror);
        if (status != eVideoCacheStatusOK) {
            if ( verbose() )
                cerr << "Couldn't read frame: " << i << " error: "
                     << rcVideoCache::getErrorString(verror) << endl;
            return eMovieConverterErrorRead;
        }

        frameBuf = clippedFrame( frameBuf, mOptions );
        frame2Buf = clippedFrame( frame2Buf, mOptions );

        if ( mOptions.reversePixels() ) {
            rcWindow w( frameBuf );
            rfReversePixels8( w );
        }

        rcGenMovieFileError gerr = generator.addFrame( frameBuf, frame2Buf );
        if ( gerr != eGenMovieFileErrorOK ) {
            if ( verbose() )
                cerr << "Output movie addFrame error: " << gerr << " " << outputFile << endl;
            return eMovieConverterErrorWrite;
        }

        ++converted;
        if ( mProgress )
            mProgress->progress( double(converted)/frameCount * 100 );

    } // End of: for (i = offset + firstIndex*period; i < endMark; i += period) {

    if (mOptions.rev() >= movieFormatRev1) {
        // Copy old conversion headers
        const vector<rcMovieFileConvExt>& conversions = cache.movieFileConversions();
        if ( !conversions.empty() ) {
            for ( uint32 i = 0; i < conversions.size(); ++i ) {
                rcMovieFileConvExt cnvHdr = conversions[i];
                 rcGenMovieFileError gerr = generator.addHeader( cnvHdr );
                 if ( gerr != eGenMovieFileErrorOK ) {
                     if ( verbose() )
                         cerr << "Output movie addHeader error: " << gerr << " " << outputFile << endl;
                     return eMovieConverterErrorHeaderWrite;
                 }
            }
        }

        // Add new conversion header
        rcMovieFileConvExt cnvHdr( mOptions.firstFrameIndex() + mOptions.frameOffset(),
                                   frameCount,
                                   mOptions.samplePeriod(),
                                   mOptions.clipRect(),
                                   movieChannelAll,
                                   mOptions.frameInterval(),
                                   mOptions.reversePixels(),
                                   mOptions.creator().c_str() );
        rcGenMovieFileError gerr = generator.addHeader( cnvHdr );
        if ( gerr != eGenMovieFileErrorOK ) {
            if ( verbose() )
                cerr << "Output movie addHeader error: " << gerr << " " << outputFile << endl;
            return eMovieConverterErrorHeaderWrite;
        }
    }

    // Flush and close file
    rcGenMovieFileError gerr = generator.flush();
    if ( gerr != eGenMovieFileErrorOK ) {
        if ( verbose() )
             cerr << "Output movie flush error: " << gerr << " " << outputFile << endl;
        return eMovieConverterErrorFlush;
    }

    return error;
}

// Create Rfy movie from non-Rfy movie
rcMovieConverterError rcMovieConverterToRfy::createRfyMovie( rcFrameGrabber& inputGrabber,
                                                             const std::string& outputFile )
{
    rcMovieConverterError err = eMovieConverterErrorUnknown;

    int32 frames = 0;
    bool pixelsReversed = false;
    rcFrameGrabberError error = inputGrabber.getLastError();

    // Reify movie generator
    rcGenMovieFile generator( outputFile,
                              mOptions.channelConversion(),
                              mOptions.rev(),
                              mOptions.overWrite(),
                              mOptions.frameInterval() );
    if ( !generator.valid() ) {
        if ( verbose() )
            cerr << "rcGenMovieFile ctor failed" << endl;;
        return eMovieConverterErrorInternal;
    }

    // Grab everything
    if ( inputGrabber.isValid() ) {
        bool started = inputGrabber.start();

        if ( started ) {
            const int32 first = mOptions.frameOffset() + mOptions.firstFrameIndex() * mOptions.samplePeriod();
            int32 frameCount = mOptions.frameCount();
            if ( frameCount < 0 )
                frameCount = inputGrabber.frameCount();
            // Time stamp of previous frame
            rcTimestamp prevTimeStamp = 0.0;
            bool firstFrame = true;
            int lastIndex = -mOptions.samplePeriod();

            // Note: infinite loop
            for( int32 i = 0; ; ++i ) {
                rcSharedFrameBufPtr framePtr;
                rcFrameGrabberStatus status = inputGrabber.getNextFrame( framePtr, true );

                if ( status == eFrameStatusOK ) {
                    if ( i < first )
                        continue;
                    if ( frames == frameCount )
                        break;
                    if ( (i - lastIndex) < mOptions.samplePeriod() )
                        continue;

                    rcTimestamp curTimeStamp = framePtr->timestamp();
                    rcTimestamp frameInt = curTimeStamp - prevTimeStamp;

                    if ( firstFrame ) {
                        firstFrame = false;
                    } else {
                        // Zero time between frames is not allowed
                        if ( frameInt <= 0.0 )
                            if ( verbose() )
                                fprintf( stderr, "Error: invalid frame interval %f for frame %i\n", frameInt.secs(), i );
                        rmAssert( frameInt > 0.0 );
                    }
                    framePtr = clippedFrame( framePtr, mOptions );
                    lastIndex = i;
                    prevTimeStamp = curTimeStamp;
                    rcWindow image;
                    image = rcWindow( framePtr );
                    ++frames;

                    if ( image.depth() == rcPixel8 ) {
                        if ( mOptions.reversePixels() ) {
                            rfReversePixels8( image );
                            pixelsReversed = true;
                        }
                    }
                    // Write a frame to disk
                    rcGenMovieFileError err = generator.addFrame( image );
                    if ( err != eGenMovieFileErrorOK )
                        break;

                    if ( mProgress )
                        mProgress->progress( double(frames)/frameCount * 100 );
                } else if ( status == eFrameStatusEOF ) {
                    // no more frames available
                    break;
                }
                else {
                    // getNextFrame() failed, bail out
                    error = inputGrabber.getLastError();
                    break;
                }
            }

        } else {
            // start() failed
            error = inputGrabber.getLastError();
            err = eMovieConverterErrorRead;
        }

        if ( ! inputGrabber.stop() ) {
            // stop() failed
            error = inputGrabber.getLastError();
            err = eMovieConverterErrorRead;
        }
    } else {
        // isValid() failed
        error = inputGrabber.getLastError();
        err = eMovieConverterErrorOpen;
    }

    if ( error != eFrameErrorOK ) {
        if ( verbose() )
            cerr << rcFrameGrabber::getErrorString( error ) << endl;
        return err;
    } else
        err = eMovieConverterErrorOK;

    // Add conversion header
    rcMovieFileConvExt cnvHdr( mOptions.firstFrameIndex() + mOptions.frameOffset(),
                               frames,
                               mOptions.samplePeriod(),
                               mOptions.clipRect(),
                               channelType(generator.conversion()),
                               mOptions.frameInterval(),
                               pixelsReversed,
                               mOptions.creator().c_str() );
    rcGenMovieFileError gerr = generator.addHeader( cnvHdr );
    if ( gerr != eGenMovieFileErrorOK ) {
        if ( verbose() )
            cerr << "Output movie addHeader error: " << gerr << " " << generator.fileName() << endl;
        return eMovieConverterErrorHeaderWrite;
    }

    // Flush and close output file
    gerr = generator.flush();
    if ( gerr != eGenMovieFileErrorOK ) {
        if ( verbose() )
            cerr << "Output movie flush error: " << gerr << " " << generator.fileName() << endl;
        return eMovieConverterErrorFlush;
    }

    return err;
}

//
// rcMovieConverterToQT class implementation
//

rcMovieConverterToQT::rcMovieConverterToQT( const rcMovieConverterOptionsQT& opt,
                                            rcCarbonLock* l,
                                            rcProgressIndicator* p ) :
        rcMovieConverter( opt.verbose(), l, p ),
        mOptions( opt )
{
}

rcMovieConverterToQT::~rcMovieConverterToQT()
{
}

// Convert .rfymov movie to QuickTime
rcMovieConverterError rcMovieConverterToQT::convert( const std::string& inputFile,
                                                     const std::string& outputFile )
{
    mLastError = eMovieConverterErrorUnknown;
    bool srcReify = isReifyMovie( inputFile );

    if ( srcReify ) {
        rcVideoCache* cacheP = 0;

        cacheP = rcVideoCache::rcVideoCacheCtor( inputFile, cVideoCacheSize, false, false, false, cVideoCacheMem );

        if (!cacheP->isValid()) {
            if ( verbose() )
                cerr << rcVideoCache::getErrorString( cacheP->getFatalError() ) << endl;
            mLastError = eMovieConverterErrorOpen;
        } else {
            mLastError = eMovieConverterErrorUnsupported;
                //            mLastError = createQTMovie( cacheP, outputFile );
        }
        rcVideoCache::rcVideoCacheDtor( cacheP );
    } else {
        mLastError = eMovieConverterErrorUnsupported;
    }

    return mLastError;
}

// Create QT movie from a Rfy movie
rcMovieConverterError rcMovieConverterToQT::createQTMovie( rcVideoCache* inputCache,
                                                           const std::string& outputFile )
{
    rmAssert(inputCache);
    rcMovieConverterError error = eMovieConverterErrorOK;
    rcVideoCache& cache = *inputCache;

    Movie myMovie = NULL;
    Track myTrack = NULL;
    Media myMedia = NULL;
    FSSpec myFile;
    long myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
    short myResRefNum = 0;
    short myResID = movieInDataForkResID;
    OSErr myErr = noErr;
	int32 frameCount = mOptions.frameCount();
    if ( frameCount < 0 )
        frameCount = inputCache->frameCount();
    if(! fileokandwritable (outputFile) )
	return eMovieConverterErrorWrite;

    string dirName = folder_part (outputFile);
    std::string destFullName;
    std::string tmp = basename_part (outputFile);
    if (dirName == ".")
	{
	  std::vector<char> buffer (2048);
          if ( getcwd(&buffer[0], 2048) == 0)
            return eMovieConverterErrorWrite;
		
        destFullName = std::string(&buffer[0]) + std::string("/") + tmp;
    }
    else
        destFullName = std::string(dirName) + std::string("/") + tmp;

#ifdef DEBUG
    fprintf(stderr, "destfile %s converted %s\n", outputFile.c_str(), destFullName.c_str());
#endif

    /* Do some stupid stuff to make sure a "null" file exists. The file
     * needs to exist, otherwise rfMakeFSSpecFromPosixPath is unhappy.
     * The file gets deleted if it already exists just out of my own
     * personal paranoia.
     */
    FILE* tempFP = fopen(destFullName.c_str(), "r");
    if (tempFP) {
        fclose(tempFP);
        remove(destFullName.c_str());
    }
    tempFP = fopen(destFullName.c_str(), "w");
    fputc(0, tempFP);
    fclose(tempFP);

    lock();
	bool isDir;
    myFile = rfMakeFSSpecFromPosixPath(destFullName.c_str(), isDir);
    myErr = ValidFSSpec(&myFile);
    unlock();
    if (myErr != noErr)
        return eMovieConverterErrorWriteOpen;
    lock();
    myErr = EnterMovies();
    unlock();
    if (myErr != noErr)
        return eMovieConverterErrorWriteOpen;

    rcRect movieSize( 0, 0, cache.frameWidth(), cache.frameHeight() );
    if ( mOptions.clipRect().width() > 0 && mOptions.clipRect().height() > 0 )
        movieSize = rcRect( 0, 0, mOptions.clipRect().width(), mOptions.clipRect().height() );

    lock();
    // create a movie file for the destination movie
    myErr = CreateMovieFile(&myFile, smRoman /*sigMoviePlayer*/ ,
                            smCurrentScript, myFlags, &myResRefNum, &myMovie);
    unlock();
    if (myErr != noErr) {
        error = eMovieConverterErrorWriteOpen;
        goto bail;
    }

    lock();
    // create the movie track and media
    myTrack = NewMovieTrack(myMovie, FixRatio(movieSize.width(), 1),
                            FixRatio(movieSize.height(), 1), kNoVolume);
    myErr = GetMoviesError();
    unlock();
    if (myErr != noErr)
        goto bail;

    lock();
    myMedia = NewTrackMedia(myTrack, VideoMediaType, kVideoTimeScale, NULL, 0);
    myErr = GetMoviesError();
    unlock();
    if (myErr != noErr)
        goto bail;

    lock();
    // create the media samples
    myErr = BeginMediaEdits(myMedia);
    unlock();
    if (myErr != noErr)
        goto bail;

    myErr = addVideoSamplesToMedia(myMedia,
                                   cache,
                                   mOptions.firstFrameIndex(),
                                   frameCount,
                                   mOptions.frameOffset(),
                                   mOptions.samplePeriod(),
                                   movieSize);
    if (myErr != noErr)
        goto bail;

    lock();
    myErr = EndMediaEdits(myMedia);
    unlock();
    if (myErr != noErr)
        goto bail;

    lock();
    // add the media to the track
    myErr = InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);
    unlock();
    if (myErr != noErr)
        goto bail;

    lock();
    // add the movie atom to the movie file
    myErr = AddMovieResource(myMovie, myResRefNum, &myResID, NULL);
    unlock();

  bail:
    lock();
    if (myResRefNum != 0)
        CloseMovieFile(myResRefNum);

    if (myMovie != NULL)
        DisposeMovie(myMovie);

    if ( myErr != noErr) {
        if ( verbose() )
            cerr << "QT error " << myErr << endl;
        error = eMovieConverterErrorWrite;
    }
    unlock();

    return error;
}



OSErr rcMovieConverterToQT::addVideoSamplesToMedia(Media theMedia, rcVideoCache& cache,
                                                   uint32 firstIndex, uint32 frameCount,
                                                   uint32 offset, uint32 period,
                                                   const rcRect& movieSize )
{
    GWorldPtr		myGWorld = NULL;
    PixMapHandle	myPixMap = NULL;
    CodecType		myCodecType = kRawCodecType; // kJPEGCodecType;
    const CodecQ    quality = codecLosslessQuality;
    long			myMaxComprSize = 0L;
    Handle		    myComprDataHdl = NULL;
    CTabHandle      ctabHdl = NULL;
    Ptr			    myComprDataPtr = NULL;
    ImageDescriptionHandle myImageDesc = NULL;
    CGrafPtr		mySavedPort = NULL;
    GDHandle		mySavedDevice = NULL;
    Rect			myRect;
    OSErr			myErr = noErr;

    MacSetRect(&myRect, 0, 0, movieSize.width(), movieSize.height());

    ctabHdl = (CTabHandle)NewHandle(sizeof(ColorTable) +
                                    sizeof(ColorSpec)*256);
    if (ctabHdl == NULL) {
        myErr = -1;
        cleanupAndReturn;
    }

    HLockHi((Handle)ctabHdl);
    {
        (**ctabHdl).ctSeed = 0;
        (**ctabHdl).ctFlags = 0;
        (**ctabHdl).ctSize = 255;
        ColorSpec* ctPtr = (ColorSpec*)&((**ctabHdl).ctTable);
        for (int i = 0; i <= (**ctabHdl).ctSize; i++) {
            ctPtr->value = 0;
            unsigned short val =  (unsigned short)(i*257);
            ctPtr->rgb.red = ctPtr->rgb.blue = ctPtr->rgb.green = val;
            ctPtr++;
        }
    }
    lock();
    myErr = NewGWorld(&myGWorld, 8, &myRect, ctabHdl, NULL, (GWorldFlags)0);
    unlock();
    if (myErr != noErr)
        cleanupAndReturn;

    lock();
    myPixMap = GetGWorldPixMap(myGWorld);
    unlock();
    if (myPixMap == NULL) {
        myErr = -1;
        cleanupAndReturn;
    }

    lock();
    LockPixels(myPixMap);
    myErr = GetMaxCompressionSize(myPixMap,
                                  &myRect,
                                  40, // 8 bit grey scale
                                  quality,
                                  myCodecType,
                                  (CompressorComponent)bestFidelityCodec,
                                  &myMaxComprSize);
    unlock();
    if (myErr != noErr)
        cleanupAndReturn;

    myComprDataHdl = NewHandle(myMaxComprSize);
    if (myComprDataHdl == NULL) {
        myErr = -1;
        cleanupAndReturn;
    }

    HLockHi(myComprDataHdl);
    myComprDataPtr = *myComprDataHdl;

    myImageDesc = (ImageDescriptionHandle)NewHandle(4);
    if (myImageDesc == NULL) {
        myErr = -1;
        cleanupAndReturn;
    }

    lock();
    GetGWorld(&mySavedPort, &mySavedDevice);
    SetGWorld(myGWorld, NULL);

    TimeValue frameDuration = 0;

    uint32 endMark = offset + (firstIndex + frameCount)*period;
    uint32 converted = 0;
    bool repeat = true;
    unlock();

    for (uint32 i = offset + firstIndex*period; repeat; i += period) {
        if (i >= endMark) {
            repeat = false;
            if ( i >= cache.frameCount())
                continue;
        }
        EraseRect(&myRect);

        rcSharedFrameBufPtr frameBuf;
        rcVideoCacheError error;
        rcVideoCacheStatus status = cache.getFrame(i, frameBuf, &error);
        if (status != eVideoCacheStatusOK) {
            cerr << "Couldn't read frame: " << i << " error: "
                 << rcVideoCache::getErrorString(error) << endl;
            myErr = -1;
            cleanupAndReturn;
        }

        frameBuf = clippedFrame( frameBuf, mOptions );

        if ( mOptions.reversePixels() ) {
            rcWindow w( frameBuf );
            rfReversePixels8( w );
        }

        lock();
        copyFrame(myPixMap, frameBuf);

        myErr = CompressImage(myPixMap,
                              &myRect,
                              quality,
                              myCodecType,
                              myImageDesc,
                              myComprDataPtr);
        unlock();
        if (myErr != noErr)
            cleanupAndReturn;

        rcTimestamp startTime, endTime;
        status = cache.frameIndexToTimestamp(i, startTime, &error);
        if (status != eVideoCacheStatusOK) {
            cerr << "Couldn't read start time: " << i << " error: "
                 << rcVideoCache::getErrorString(error) << endl;
            myErr = -1;
            cleanupAndReturn;
        }

        if ((i+period) < cache.frameCount()) {
            status = cache.frameIndexToTimestamp(i+period, endTime, &error);
            if (status != eVideoCacheStatusOK) {
                cerr << "Couldn't read end time: " << i+period << " error: "
                     << rcVideoCache::getErrorString(error) << endl;
                myErr = -1;
                cleanupAndReturn;
            }
            endTime = endTime - startTime;
            frameDuration = (TimeValue)(endTime.secs() * kVideoTimeScale);
        }
        TimeValue sampleTime;

        lock();
        myErr = AddMediaSample(theMedia,
                               myComprDataHdl,
                               0,     // no offset in data
                               (**myImageDesc).dataSize,
                               frameDuration,
                               (SampleDescriptionHandle)myImageDesc,
                               1,    // one sample
                               0,    // self-contained samples
                               &sampleTime);
        unlock();
        ++converted;
        if ( mProgress )
            mProgress->progress( double(converted)/frameCount * 100.0 );

#ifdef DEBUG
        fprintf(stderr, "  Adding frame %d, duration %ld, idepth %d  icid %d "
                "psize %d ptype %d sampleTime %ld\n", i, frameDuration,
                (**myImageDesc).depth, (**myImageDesc).clutID,
                (**myPixMap).pixelSize, (**myPixMap).pixelType,
                sampleTime);
#endif

        if (myErr != noErr)
            cleanupAndReturn;
    }

    cleanupAndReturn;
}

void rcMovieConverterToQT::copyFrame(PixMapHandle myPixMap, rcSharedFrameBufPtr& frameBuf)
{
    rmAssert(myPixMap);
    int32 width = (**myPixMap).bounds.right - (**myPixMap).bounds.left;
    int32 height = (**myPixMap).bounds.bottom - (**myPixMap).bounds.top;

    uint8* rowPtr = (uint8 *)GetPixBaseAddr(myPixMap);
    int32 rowUpdate = QTGetPixMapHandleRowBytes(myPixMap);

    rmAssert(frameBuf->width() == width);
    rmAssert(frameBuf->height() == height);
    rmAssert(width <= rowUpdate);
    for (int32 curLine = 0; curLine < height; curLine++) {
        memmove(rowPtr, frameBuf->rowPointer(curLine), width);
        rowPtr += rowUpdate;
    }
}

//
// Stream output operator implementations
//

ostream& operator<< ( ostream& o, const rcMovieConverterOptions& opt )
{
    o << "FrameInterval:   " << opt.frameInterval() << endl;
    o << "FirstFrameIndex: " << opt.firstFrameIndex() << endl;
    o << "FrameCount:      " << opt.frameCount() << endl;
    o << "FrameOffset:     " << opt.frameOffset() << endl;
    o << "SamplePeriod:    " << opt.samplePeriod() << endl;
    o << "OverWrite:       " << opt.overWrite() << endl;
    o << "Creator:         " << opt.creator() << endl;

    return o;
}

ostream& operator<< ( ostream& o, const rcMovieConverterOptionsRfy& opt )
{
    const rcMovieConverterOptions& baseOpt = opt;
    o << baseOpt;
    o << "ChannelConversion: " << opt.channelConversion() << endl;
    o << "ReversePixels:     " << opt.reversePixels() << endl;
    o << "FileRev:           " << opt.rev() << endl;

    return o;
}

ostream& operator<< ( ostream& o, const rcMovieConverterOptionsQT& opt )
{
    const rcMovieConverterOptions& baseOpt = opt;
    o << baseOpt;

    return o;
}
