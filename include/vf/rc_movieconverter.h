/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 * $Id: rc_movieconverter.h 6565 2009-01-30 03:24:44Z arman $
 *
 * Movie converter classes
 *
 ***************************************************************************/

#ifndef _rcMOVIE_CONVERTER_H_
#define _rcMOVIE_CONVERTER_H_

#include "rc_gen_movie_file.h"
#include "rc_framegrabber.h"

    //using namespace qtime;

enum rcMovieConverterError {
  eMovieConverterErrorOK = 0,
  eMovieConverterErrorUnknown,
  eMovieConverterErrorInternal,
  eMovieConverterErrorNull,
  eMovieConverterErrorHeaderWrite,
  eMovieConverterErrorExists,
  eMovieConverterErrorOpen,
  eMovieConverterErrorRead,
  eMovieConverterErrorWrite,
  eMovieConverterErrorWriteOpen,
  eMovieConverterErrorSeek,
  eMovieConverterErrorClose,
  eMovieConverterErrorFlush,
  eMovieConverterErrorUnsupported
};

//
// Conversion options base class
//
class rcMovieConverterOptions {
  public:
    // ctors
    rcMovieConverterOptions();
    rcMovieConverterOptions( const rcMovieConverterOptions& o );
    
    // dtor
    virtual ~rcMovieConverterOptions();

    // Accessors
    float   frameInterval() const     { return mFrameInterval; }
    int32 firstFrameIndex() const   { return mFirstFrameIndex; }
    int32 frameCount() const        { return mFrameCount; }
    int32 frameOffset() const       { return mFrameOffset; }
    int32 samplePeriod() const      { return mSamplePeriod; }
    const   std::string& creator() const { return mCreator; }
    const   rcRect& clipRect() const  { return mClipRect; }
    bool    overWrite() const         { return mOverWrite; }
    bool    reversePixels() const     { return mReversePixels; }
    bool    verbose() const           { return mVerbose; }
    
    // Mutators
    void frameInterval( const float& v )     { mFrameInterval = v; }
    void firstFrameIndex( const int32& v ) { mFirstFrameIndex = v; }
    void frameCount( const int32& v )      { mFrameCount = v; }
    void frameOffset( const int32& v )     { mFrameOffset = v; }
    void samplePeriod(const int32& v )     { mSamplePeriod = v; }
    void creator( const std::string& v )        { mCreator = v; }
    void clipRect( const rcRect& v )         { mClipRect = v; }
    void overWrite( bool v )                 { mOverWrite = v; }
    void reversePixels( bool v )             { mReversePixels = v; }
    void verbose( bool v )                   { mVerbose = v; }
     
  protected:
    float    mFrameInterval;   // Frame interval (-1 = use native interval)
    int32  mFirstFrameIndex; // First frame index
    int32  mFrameCount;      // Number of frames to convert (-1 = all frames )
    int32  mFrameOffset;     // Offset from beginning of input movie
    int32  mSamplePeriod;    // Frame sampling pedriod ( 1 = all frames )
    std::string mCreator;         // Creator app name
    rcRect   mClipRect;        // Input clip rect
    bool     mOverWrite;       // Overwrite output file if it exists
    bool     mReversePixels;   // Reverse 8-bit pixels
    bool     mVerbose;         // Verbose error output
};

//
// Conversion options to Rfy 
//
class rcMovieConverterOptionsRfy : public rcMovieConverterOptions {
  public:
    // ctors
    rcMovieConverterOptionsRfy();
    rcMovieConverterOptionsRfy( const rcMovieConverterOptions& o );
    // dtor
    virtual ~rcMovieConverterOptionsRfy();

    // Accessors
    rcChannelConversion channelConversion() const { return mChannelConversion; }
    movieFormatRev      rev() const               { return mRev; }
    
    // Mutators
    void channelConversion( const rcChannelConversion& v ) { mChannelConversion = v; }
    void rev( const movieFormatRev& v )                    { mRev = v; }
    
  private:
    rcChannelConversion mChannelConversion; // Color channel conversion for 32-bit input
    movieFormatRev      mRev;               // Output file revision
};

//
// Conversion options to QT 
//
class rcMovieConverterOptionsQT : public rcMovieConverterOptions {
  public:
    rcMovieConverterOptionsQT();
    rcMovieConverterOptionsQT( const rcMovieConverterOptions& o );
    virtual ~rcMovieConverterOptionsQT();
};

//
// Movie converter base class
//
class rcMovieConverter {
  public:
    // TODO: add progress indicator
    rcMovieConverter( bool verbose = true, rcCarbonLock* l = NULL, rcProgressIndicator* p = NULL ) :
    mLastError( eMovieConverterErrorUnknown ), mProgress( p ), mVerbose( verbose ), mCarbonLock( l ) {}
    virtual ~rcMovieConverter() {}

    static std::string getErrorString( const rcMovieConverterError& error );
    static bool isReifyMovie( const std::string& name );
    static bool isImageCollection( const std::string& name );
     
    //
    // Accessors
    //
    bool verbose() const { return mVerbose; }
    rcFrameRef clippedFrame( rcFrameRef orig,
                                       rcMovieConverterOptions opt );
    //
    // Mutators
    //
    void verbose( bool v ) { mVerbose = v; }
    
    //
    // Conversion operations
    //
    virtual rcMovieConverterError convert( const std::string& inputFile,
                                           const std::string& outputFile ) = 0;

  protected:
    // Lock mCarbonLock
    void lock();
    // Unlock mCarbonLock
    void unlock();
    
    rcMovieConverterError mLastError; // Last operation error
    rcProgressIndicator*  mProgress;  // Progress indicator
    bool                  mVerbose;   // Verbose mode
    rcCarbonLock*         mCarbonLock; // Carbon/QuickTime lock
};

//
// Converter from .rfymov to QT
//
class rcMovieConverterToQT : public rcMovieConverter {
 public:
    // ctor
    rcMovieConverterToQT( const rcMovieConverterOptionsQT& opt,
                          rcCarbonLock* l = NULL,
                          rcProgressIndicator* p = NULL );
    // dtor
    virtual ~rcMovieConverterToQT();

    //
    // Accessors
    //
    const rcMovieConverterOptionsQT& options() const { return mOptions; }
    
    //
    // Mutators
    //
    void options( const rcMovieConverterOptionsQT& o ) { mOptions = o; }

    //
    // Conversion operations
    //
    rcMovieConverterError convert( const std::string& inputFile,
                                   const std::string& outputFile );
    
  private:
    rcMovieConverterError createQTMovie( rcVideoCache* inputCache,
                                         const std::string& outputFile );
    
    rcMovieConverterOptionsQT mOptions;    // Conversion options
};

//
// Converter from QT/.rfymov to .rfymov
//
class rcMovieConverterToRfy : public rcMovieConverter {
 public:
    // ctor
    rcMovieConverterToRfy( const rcMovieConverterOptionsRfy& opt,
                           rcCarbonLock* l = NULL,
                           rcProgressIndicator* p = NULL );
    // dtor
    virtual ~rcMovieConverterToRfy();

    //
    // Accessors
    //
    const rcMovieConverterOptionsRfy& options() const { return mOptions; }
    
    //
    // Mutators
    //
    void options( const rcMovieConverterOptionsRfy& o ) { mOptions = o; }

    //
    // Conversion operations
    //
    rcMovieConverterError convert( const std::string& inputFile,
                                   const std::string& outputFile );

    
 private:
    rcMovieConverterError createRfyMovie( rcVideoCache* inputCache,
                                          const std::string& outputFile );

    rcMovieConverterError createRfyMovie( rcFrameGrabber& inputGrabber,
                                          const std::string& outputFile );

    rcMovieConverterOptionsRfy mOptions; // Conversion options
};

// Stream output operators
ostream& operator<< ( ostream& o, const rcMovieConverterOptions& opt );
ostream& operator<< ( ostream& o, const rcMovieConverterOptionsRfy& opt );
ostream& operator<< ( ostream& o, const rcMovieConverterOptionsQT& opt );
#endif
