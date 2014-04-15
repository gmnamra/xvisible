/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 * $Id: rc_movieconverter.cpp 7284 2011-03-01 23:32:47Z arman $
 *
 * Movie converter classes
 * @file
 ***************************************************************************/

#include <rc_cinder_qtime_grabber.h>
#include <unistd.h>
#include <iostream>
#include <rc_movieconverter.h>
#include <rc_videocache.h>
#include <rc_imagegrabber.h>
#include <file_system.hpp>
#include <rc_fileutils.h>
#include <rc_stlplus.h>
#include <cinder/qtime/QuickTime.h>
#include <cinder/qtime/MovieWriter.h>

using namespace ci;




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
    bool srcImageCollection = isImageCollection( inputFile );
    bool srcNonRfy = ! srcReify && ! srcImageCollection;
    
    if ( srcReify )
    {
        rcVideoCache* cacheP = 0;
        
        cacheP = rcVideoCache::rcVideoCacheCtor( inputFile, cVideoCacheSize, false, false, false, cVideoCacheMem );
        
        if (!cacheP->isValid())
        {
            if ( verbose() )
                cerr << rcVideoCache::getErrorString( cacheP->getFatalError() ) << endl;
            mLastError = eMovieConverterErrorOpen;
        } 
        else
        {
            mLastError = createRfyMovie( cacheP, outputFile );
        }
        rcVideoCache::rcVideoCacheDtor( cacheP );
        return mLastError;
    } 

    if (srcNonRfy)
    {
        boost::shared_ptr<rcCinderGrabber> grabber( new rcCinderGrabber (inputFile, mCarbonLock)); // , mOptions.frameInterval() );
        mLastError = createRfyMovie( *grabber , outputFile );
        return mLastError;
    }

    if (srcImageCollection)
    {
            vector<std::string> fileNames;
        rfGetFilesFromSeparatedList (inputFile, fileNames, mOptions.frameCount(), mOptions.firstFrameIndex() ); // default is ';'

        // Hack: mutate frame interval for image grabber
        if ( mOptions.frameInterval() <= 0.0 ) mOptions.frameInterval(1.0);

        rcImageGrabber grabber( fileNames, mCarbonLock, mOptions.frameInterval() );
        mLastError = createRfyMovie( grabber, outputFile );
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
    for (uint32 i = mOptions.frameOffset() + mOptions.firstFrameIndex()*mOptions.samplePeriod(); i < endMark; i += mOptions.samplePeriod())
    {
        status = cache.getFrame(i, frameBuf, &verror);
        if (status != eVideoCacheStatusOK) {
            if ( verbose() )
                cerr << "Couldn't read frame: " << i << " error: "
                << rcVideoCache::getErrorString(verror) << endl;
            return eMovieConverterErrorRead;
        }
#if 0        
        frameBuf = clippedFrame( frameBuf, mOptions );
        
        if ( mOptions.reversePixels() ) {
            rcWindow w( frameBuf );
            rfReversePixels8( w );
        }
#endif        
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
            mLastError = createQTMovie( cacheP, outputFile );
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
    rmAssert ( ! is_relative_path (outputFile) );
    
    rcMovieConverterError error = eMovieConverterErrorOK;
    rcVideoCache& cache = *inputCache;
	int32 frameCount = mOptions.frameCount();
    if ( frameCount < 0 )
        frameCount = inputCache->frameCount();
    
    rcRect movieSize( 0, 0, cache.frameWidth(), cache.frameHeight() );
    if ( mOptions.clipRect().width() > 0 && mOptions.clipRect().height() > 0 )
        movieSize = rcRect( 0, 0, mOptions.clipRect().width(), mOptions.clipRect().height() );
    
    
    ci::qtime::MovieWriter::Format format (kRawCodecType, 1.0);
    ci::qtime::MovieWriterRef mMovieWriter = ci::qtime::MovieWriter::create (outputFile,
                                                                             movieSize.width(), 
                                                                             movieSize.height(), 
                                                                             format);
    
    
    if(! fileokandwritable (outputFile) )
        return eMovieConverterErrorWrite;
    
    
    /* Do some stupid stuff to make sure a "null" file exists. The file
     * needs to exist, otherwise rfMakeFSSpecFromPosixPath is unhappy.
     * The file gets deleted if it already exists just out of my own
     * personal paranoia.
     */
    
    int firstIndex = mOptions.firstFrameIndex();
    int offset = mOptions.frameOffset();
    int period = mOptions.samplePeriod();
    TimeValue frameDuration = 0;
    
    uint32 endMark = offset + (firstIndex + frameCount)*period;
    uint32 converted = 0;
    bool repeat = true;
    unlock();
    
    for (uint32 i = offset + firstIndex*period; repeat; i += period)
    {
        if (i >= endMark)
        {
            repeat = false;
            if ( i >= inputCache->frameCount())
                continue;
        }
        
        rcSharedFrameBufPtr frameBuf;
        rcVideoCacheError error;
        rcVideoCacheStatus status = inputCache->getFrame(i, frameBuf, &error);
        if (status != eVideoCacheStatusOK) {
            cerr << "Couldn't read frame: " << i << " error: "
            << rcVideoCache::getErrorString(error) << endl;
        }
        
        frameBuf = clippedFrame( frameBuf, mOptions );
        
        if ( mOptions.reversePixels() ) {
            rcWindow w( frameBuf );
            rfReversePixels8( w );
        }
        
        mMovieWriter->addFrame (ImageSourceRef ( * frameBuf->newCiChannel() ) );
        
        rcTimestamp startTime, endTime;
        if ((i+period) < cache.frameCount()) {
            status = cache.frameIndexToTimestamp(i+period, endTime, &error);
            if (status != eVideoCacheStatusOK) {
                cerr << "Couldn't read end time: " << i+period << " error: "
                << rcVideoCache::getErrorString(error) << endl;
            }
            endTime = endTime - startTime;
            frameDuration = (TimeValue)(endTime.secs() * kVideoTimeScale);
        }
        
        
        ++converted;
        if ( mProgress )
            mProgress->progress( double(converted)/frameCount * 100.0 );
        
    }
    
    return eMovieConverterErrorOK;
    
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



