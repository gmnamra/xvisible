/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_moviefileformat.h 6278 2009-01-26 20:41:46Z arman $
 *
 *	This file contains Reify movie file format definition
 *
 ******************************************************************************/

#ifndef _rcMOVIEFILEFORMAT_H_
#define _rcMOVIEFILEFORMAT_H_

#include "rc_types.h"
#include "rc_framebuf.h"
#include "rc_rect.h"
#include <vector>

//#include "rc_dcam_capture.h>

// Notice: this movie file format is defined to be big-endian.
// Byte-swapping will have to be performed on little-endian platforms.

// Warning: when adding new enumerations, they MUST be added at the end

enum movieFormatRev {
    movieFormatInvalid = 0
    , movieFormatRev0
    , movieFormatRev1 // Same as rev 0, but supports extensions at end of file
    , movieFormatRev2 // New base header format, new extensions
    , movieFormatRevLatest = movieFormatRev2 // Latest supported rev
};

enum movieExtensionType {
    movieExtensionEOF = 0     // Always movie's last extension
    , movieExtensionTOC = 1   // Table of contents
    , movieExtensionORG = 2   // Origin data
    , movieExtensionCNV = 3   // Conversion data
    , movieExtensionCAM = 4   // Camera data
    , movieExtensionEXP = 5   // Experiment data
};

enum movieOriginType {
    movieOriginUnknown = 0          // Unknown origin
    , movieOriginCaptureUncertified // Uncertified Reify capture process (Rev0)
    , movieOriginCaptureCertified   // Certifid Reify capture (Rev1 and later)
    , movieOriginConversionExt      // Conversion from a non-Reify movie
    , movieOriginSynthetic          // Synthetically generated movie
    , movieOriginGrayIsVisibleAlphaIsFlu // put together from 2 channels corresponding to visible and Fluorescence
    , movieOriginMultiChannelFlu // put together from 2 channels corresponding to visible and Fluorescence
    , movieOriginGrayPlusMultiChannelFlu // put together from 2 channels corresponding to visible and Fluorescence
};

enum movieChannelType {
    movieChannelUnknown = 0   // Unknown channel
    , movieChannelAll         // All channels
    , movieChannelAvg         // Average of channels  
    , movieChannelMax         // Max of channels
    , movieChannelRed         // Red channel
    , movieChannelGreen       // Green channel
    , movieChannelBlue        // Blue channel
};

// Revision magic
#define MOVIEFILE_MAGIC_REV0     "Reify Corp"
#define MOVIEFILE_MAGIC_LTH_REV0 10
#define MOVIEFILE_MAGIC_REV2     "Reify Corp03"
#define MOVIEFILE_MAGIC_LTH_REV2 12
// Creator comment length
#define MOVIEFILE_GEN_LTH 64

//
// Basic headers
//

// This file identifier must be read first for all file revisions. Based on file
// revision number different header classes will be then read from file.

class rcMovieFileIdentifier
{
  public:
    rcMovieFileIdentifier();
    rcMovieFileIdentifier( movieFormatRev rev );

    // Validity check
    bool isValid() const;
    // Do endian reversal if necessary
    void fixEndian();
    
    // Accessors
    movieFormatRev rev() const { return static_cast<movieFormatRev>(mRev); }
    const char* magic() const { return mMagic; }
    
  private:
    char          mMagic[MOVIEFILE_MAGIC_LTH_REV2];
    uint32      mRev;             // This must be ALWAYS stored in big-endian form
};

/* Basic movie file format info for rev0 and rev1. WARNING! Do not add fields to
 * existing movie file header structures.  Instead, create a new
 * rcMovieFileExt structure and add corresponding movieFormatRev
 * and movieFileHdrSize enums.
 */

class rcMovieFileFormat
{
  public:
    // ctors
    rcMovieFileFormat();
    rcMovieFileFormat( movieFormatRev rev );
    rcMovieFileFormat( const rcFrameRef& frame,
                       movieFormatRev rev = movieFormatInvalid );

    // Write header to stream in binary format
    bool write( FILE* saveStream ) const;
    // Do endian reversal if necessary
    void fixEndian();
    
    // Accessors
    const uint32& width() const { return mWidth; }
    const uint32& height() const { return mHeight; }
    const uint32& rowUpdate() const { return mRowUpdate; }
    const double& averageFrameRate() const { return mAverageFrameRate; }
    const uint32& frameCount() const { return mFrameCount; }
    const int64& baseTime() const { return mBaseTime; }
    movieFormatRev rev() const { return mId.rev(); }
    const char* magic() const { return mId.magic(); }
    uint32 bytesInFrame() const { return height() * rowUpdate(); }
    
    // Mutators
    void rowUpdate( const uint32& f ) { mRowUpdate = f; }
    void averageFrameRate( const double& f ) { mAverageFrameRate = f; }
    void frameCount( const uint32& f ) { mFrameCount = f; }
    void baseTime( const int64& t ) { mBaseTime = t; }
    
  private:
    rcMovieFileIdentifier mId; // Common to all formats
    
    /* General description of frames within a file. Frames are assumed to
     * all share these characteristics.
     */
    uint32      mWidth;
    uint32      mHeight;
    uint32      mRowUpdate;
    double        mAverageFrameRate; // In frames per second

    /* Values to allow proper navigation through the rest of the file.
     */
    uint32      mFrameCount;
    int64       mBaseTime;
};

// Pixel data storage
typedef struct rcMovieFrameFormat
{
    int64      timestamp;
    char         rawPixels[1];    // Start of pixels for this frame.
} rcMovieFrameFormat;

/* Basic movie file format info for rev2. WARNING! Do not add fields to
c * existing movie file header structures.  Instead, create a new
 * rcMovieFileExt structure and add corresponding movieFormatRev
 * and movieFileHdrSize enums.
 */
class rcMovieFileFormat2
{
  public:
    // ctors
    rcMovieFileFormat2();
    rcMovieFileFormat2( movieFormatRev rev );
    rcMovieFileFormat2( const rcFrameRef& frame,
                        movieFormatRev rev = movieFormatInvalid );
    
    // Write header to stream in binary format
    bool write( FILE* saveStream ) const;
    // Do endian reversal if necessary
    void fixEndian();
    
    // Accessors
    const uint32& width() const { return mWidth; }
    const uint32& height() const { return mHeight; }
    const uint32& rowUpdate() const { return mRowUpdate; }
    const double& averageFrameRate() const { return mAverageFrameRate; }
    const uint32& frameCount() const { return mFrameCount; }
    const int64& baseTime() const { return mBaseTime; }
    movieFormatRev rev() const { return mId.rev(); }
    const char* magic() const { return mId.magic(); }
    uint32 bytesInFrame() const { return height() * rowUpdate(); }
    const uint16& bom() const { return mBom; }
    const rcPixel& depth() const { return mDepth; }
    const int64& extensionOffset() const { return mExtensionOffset;}
    
    /// Mutators
    void rowUpdate( const uint32& f ) { mRowUpdate = f; }
    void averageFrameRate( const double& f ) { mAverageFrameRate = f; }
    void frameCount( const uint32& f ) { mFrameCount = f; }
    void baseTime( const int64& t ) { mBaseTime = t; }
    void extensionOffset( const int64& o ) { mExtensionOffset = o; }
    void depth( const rcPixel& d ) { mDepth = d; }
    
  private:
    rcMovieFileIdentifier mId;  // Common to all formats
    uint16              mBom; // Byte order mark
    /* General description of frames within a file. Frames are assumed to
     * all share these characteristics.
     */
    uint32      mWidth;
    uint32      mHeight;
    rcPixel  mDepth;           // Depth of video frames
    uint32      mRowUpdate;
    double        mAverageFrameRate; // In frames per second

    /* Values to allow proper navigation through the rest of the file.
     */
    uint32      mFrameCount;
    int64       mBaseTime;
    int64       mExtensionOffset; // Offset to start of extension headers
};

//
// Extension headers
//

/* Base class from which all movie file extension classes must be
 * derived.
 */
class rcMovieFileExt
{
  public:
    // ctors
    rcMovieFileExt();
    rcMovieFileExt( movieExtensionType t );
    rcMovieFileExt( movieExtensionType t, int64 o );

    // Accessors
    const movieExtensionType& type() const { return mType; };
    const int64& offset() const { return mOffset; };

    // Write header to stream in binary format
    bool write( FILE* saveStream ) const;
    // Do endian reversal if necessary
    void fixEndian( const reByteOrder& fileByteOrder );
    
  private:    
    movieExtensionType mType;
    int64            mOffset; // This is a byte offset to the beginning of the
                                // next extension starting at the beginning of this one. 
};

// Table-of-contents header
// Supported in movieFormatRev1
class rcMovieFileTocExt : public rcMovieFileExt
{
  public:
    // ctors
    rcMovieFileTocExt();
    rcMovieFileTocExt( uint32 size );

    // Write header to stream in binary format
    bool write( FILE* saveStream, const vector<int64>& tocTimes ) const;
    // Do endian reversal if necessary
    void fixEndian( const reByteOrder& fileByteOrder );
    
    // Accessors
    const uint32& count() const { return mCount; };

  private:
    uint32 mCount;  // Count of TOC entries - must match frameCount from
    // rcMovieFileFormat
    //  Will be followed by an array of time entries (as int64) -
    //  in frame order.
};

// Data origin extension
// Supported in movieFormatRev2
class rcMovieFileOrgExt : public rcMovieFileExt
{
  public:
    // ctors
    rcMovieFileOrgExt();
    rcMovieFileOrgExt( movieOriginType o, int64 bt, uint32 fc,
                       int32 w, int32 h, rcPixel depth,
                       movieFormatRev r, const char* c );

    // Write header to stream in binary format
    bool write( FILE* saveStream ) const;
    // Write header to stream in human-readable log format
    ostream& log( ostream& os ) const;
    // Do endian reversal if necessary
    void fixEndian( const reByteOrder& fileByteOrder );
    
    // Accessors
    const movieOriginType& origin() const { return mOrigin; }
    const int64& baseTime() const { return mOriginalBaseTime; }
    const uint32& frameCount() const { return mOriginalFrameCount; }
    const int32& width() const { return mOriginalWidth; }
    const int32& height() const { return mOriginalHeight; }
    const rcPixel& depth() const { return mOriginalDepth; }
    const movieFormatRev& rev() const { return mRev; }
    const char* creatorName() const { return mComment; }
    const int32& id() const { return mId; }
    
    // Mutators
    void id( const int32& i ) { mId = i; }
    
  private:    
    movieOriginType mOrigin;                     // Data origin
    int64         mOriginalBaseTime;           // Base time of original movie
    uint32        mOriginalFrameCount;         // Frame count of original movie
    int32         mOriginalWidth;              // Width of original movie
    int32         mOriginalHeight;             // Height of original movie
    rcPixel    mOriginalDepth;              // Pixel depth of original movie
    movieFormatRev  mRev;                        // File format revision
    int32         mId;                         // Unique id, reserved for future use
    char            mComment[MOVIEFILE_GEN_LTH]; // Human-readable data generator name
};

// Data conversion extension
// Supported in movieFormatRev2
class rcMovieFileConvExt : public rcMovieFileExt
{
  public:
    // ctors
    rcMovieFileConvExt();
    rcMovieFileConvExt( uint32 so, uint32 fc, uint32 sm,
                        const rcRect& r, movieChannelType ch,
                        float frameInterval, bool pixelsRev, const char* c );

    // Write header to stream in binary format
    bool write( FILE* saveStream ) const;
    // Write header to stream in human-readable log format
    ostream& log( ostream& os ) const;
    // Do endian reversal if necessary
    void fixEndian( const reByteOrder& fileByteOrder );
    
    // Accessors
    const uint32& startOffset() const { return mStartOffset; };
    const uint32& frameCount() const { return mFrameCount; };
    const uint32& sample() const { return mSample; };
    const rcRect& cropRect() const { return mCropRect; };
    const int64& date() const { return mDate; };
    const movieChannelType& channel() const { return mChannel; };
    const float& frameInterval() const { return mFrameInterval; };
    const movieFormatRev& rev() const { return mRev; };
    const char* creatorName() const { return mComment; };
    const uint16& pixelsReversed() const { return mPixelsReversed; };
    const int32& id() const { return mId; }
    
    // Mutators
    void id( const int32& i ) { mId = i; }
    
  private:
    uint32         mStartOffset;                // First frame from input movie
    uint32         mFrameCount;                 // Number of frames 
    uint32         mSample;                     // Sample interval 
                                                  // (1=all frames)
    rcRect           mCropRect;                   // Crop rect 
    int64          mDate;                       // Conversion date
    movieChannelType mChannel;                    // Which channel of input movie was used
    float            mFrameInterval;              // Frame interval in seconds

    movieFormatRev   mRev;                        // File format revision for output
    char             mComment[MOVIEFILE_GEN_LTH]; // Human-readable data converter name
    int32          mId;                         // Unique id, reserved for future use
    uint16         mPixelsReversed;             // Were 8-bit pixels reversed
};

// Camera information extension
// Supported in movieFormatRev2
class rcMovieFileCamExt : public rcMovieFileExt
{
  public:
    // ctors
    rcMovieFileCamExt();
//    rcMovieFileCamExt( const rcAcqInfo* info );
    
    // Write header to stream in binary format
    bool write( FILE* saveStream ) const;
    // Write header to stream in human-readable log format
    ostream& log( ostream& os, bool showUndefined = true ) const;
    // Do endian reversal if necessary
    void fixEndian( const reByteOrder& fileByteOrder );
    
    // Accessors
    const uint32& mid() const { return mMfgId; };
    const uint32& uid() const { return mUniqueId; };
    const uint32& format() const { return mFormat; };
    const uint32& mode() const { return mMode; };
    const double& frameRate() const { return mFrameRate; };
    const uint32& brightness() const { return mBrightness; };
    const uint32& exposure() const { return mExposure; };
    const uint32& sharpness() const { return mSharpness; };
    const uint32& whiteBalanceUB() const { return mWhiteBalUB; };
    const uint32& whiteBalanceVR() const { return mWhiteBalVR; };
    const uint32& hue() const { return mHue; };
    const uint32& saturation() const { return mSaturation; };
    const uint32& gamma() const { return mGamma; };
    const uint32& shutter() const { return mShutter; };
    const int32& gain() const { return mGain; };
    const uint32& iris() const { return mIris; };
    const uint32& focus() const { return mFocus; };
    const uint32& temperature() const { return mTemperature; };
    const uint32& zoom() const { return mZoom; };
    const uint32& pan() const { return mPan; };
    const uint32& tilt() const { return mTilt; };
    const uint32& filter() const { return mFilter; };
    const rcRect& cropRect() const { return mCropRect; };
    const char* name() const { return mName; };

    const int32& id() const { return mId; }
    // Pixel geometry in microns, (-1.0,-1.0) denotes unknown geometry
    rc2Fvector pixelGeometry() const;
    
    // Mutators
    void mid( const int32& v ) { mMfgId = v; }
    void uid( const int32& v ) { mUniqueId = v; }
    void format( const int32& v ) { mFormat = v; }
    void mode( const int32& v ) { mMode = v; }
    void frameRate( const double& v ) { mFrameRate = v; }
    void brightness( const int32& v ) { mBrightness = v; }
    void exposure( const int32& v ) { mExposure = v; }
    void sharpness( const int32& v ) { mSharpness = v; }
    void whiteBalanceUB( const int32& v ) { mWhiteBalUB = v; }
    void whiteBalanceVR( const int32& v ) { mWhiteBalVR = v; }
    void hue( const int32& v ) { mHue = v; }
    void saturation( const int32& v ) { mSaturation = v; }
    void gamma( const int32& v ) { mGamma = v; }
    void shutter( const int32& v ) { mShutter = v; }
    void gain( const int32& v ) { mGain = v; }
    void iris( const int32& v ) { mIris = v; }
    void focus( const int32& v ) { mFocus = v; }
    void temperature( const int32& v ) { mTemperature = v; }
    void zoom( const int32& v ) { mZoom = v; }
    void pan( const int32& v ) { mPan = v; }
    void tilt( const int32& v ) { mTilt = v; }
    void filter( const int32& v ) { mFilter = v; }
    void cropRect( const rcRect& v ) { mCropRect = v; }
    void name( const char* n ) {
        if ( n ) {
            // Truncate if necessary
            uint32 len = strlen( n );
            if ( len >= rmDim(mName) )
                len = rmDim(mName)-1;
            strncpy( mName, n, len );
            // Null-terminate
            mName[len] = 0;
        }
    }
    void id( const int32& i ) { mId = i; }
    
  private:
    // Camera info/settings
    uint32         mMfgId;                       // Manufacturer ID
    uint32         mUniqueId;                    // Unique hardware ID
    uint32         mFormat;                      // IIDC format
    uint32         mMode;                        // IIDC mode
    double           mFrameRate;                   // Frame rate
    
    uint32         mBrightness;                  // Brightness setting
    uint32         mExposure;                    // Exposure setting
    uint32         mSharpness;                   // Sharpness setting
    uint32         mWhiteBalUB;                  // White balance U/B
    uint32         mWhiteBalVR;                  // White balance V/R
    uint32         mHue;                         // Hue setting
    uint32         mSaturation;                  // Saturation setting
    uint32         mGamma;                       // Gamma setting
    uint32         mShutter;                     // Shutter setting
    int32           mGain;                        // gain setting
    uint32         mIris;                        // Iris setting
    uint32         mFocus;                       // Focus setting
    uint32         mTemperature;                 // Temperature setting
    uint32         mZoom;                        // Zoom setting
    uint32         mPan;                         // Pan setting
    uint32         mTilt;                        // Tilt setting
    uint32         mFilter;                      // Optical filter setting
    rcRect           mCropRect;                    // Crop rect setting
     
    char             mName[80];                    // Human-readable camera name
    int32          mId;                          // Unique id, reserved for future use
};

// Experiment information extension
// Supported in movieFormatRev2
class rcMovieFileExpExt : public rcMovieFileExt
{
  public:
    // ctors
    rcMovieFileExpExt();
    
    // Write header to stream in binary format
    bool write( FILE* saveStream ) const;
    // Write header to stream in human-readable log format
    ostream& log( ostream& os, bool showUndefined = true ) const;
    // Do endian reversal if necessary
    void fixEndian( const reByteOrder& fileByteOrder );
    
    // Accessors
    const int32& id() const { return mId; }
    const float&   lensMag() const { return mLensMag; }
    const float&   otherMag() const { return mOtherMag; }
    const float&   temperature() const { return mTemperature; }
    const float&   CO2() const { return mCO2; }
    const float&   O2() const { return mO2; }

    const char*    title() const { return mTitle; }
    const char*    userName() const { return mUserName; }
    const char*    treatment1() const { return mTreatment1; }
    const char*    treatment2() const { return mTreatment2; }
    const char*    cellType() const { return mCellType; }
    const char*    imagingMode() const { return mImagingMode; }
    const char*    comment() const { return mComment; }
    
    // Mutators
    void id( const int32& i ) { mId = i; }
    void lensMag( const float& v ) { mLensMag = v; }
    void otherMag( const float& v ) { mOtherMag = v; }
    void temperature( const float& v ) { mTemperature = v; }
    void CO2( const float& v ) { mCO2 = v; }
    void O2( const float& v ) { mO2 = v; }

    void title( const char* n ) { if ( n ) strncpy( mTitle, n, rmDim(mTitle) ); }
    void userName( const char* n ) { if ( n ) strncpy( mUserName, n, rmDim(mUserName) ); }
    void treatment1( const char* n ) { if ( n ) strncpy( mTreatment1, n, rmDim(mTreatment1) ); }
    void treatment2( const char* n ) { if ( n ) strncpy( mTreatment2, n, rmDim(mTreatment2) ); }
    void cellType( const char* n ) { if ( n ) strncpy( mCellType, n, rmDim(mCellType) ); }
    void imagingMode( const char* n ) { if ( n ) strncpy( mImagingMode, n, rmDim(mImagingMode) ); }
    void comment( const char* n ) { if ( n ) strncpy( mComment, n, rmDim(mComment) ); }
     
  private:
    float            mLensMag;          // Microscope lens maginifaction
    float            mOtherMag;         // Microscope other maginifaction
    float            mTemperature;      // Temperature in degrees C
    float            mCO2;              // CO2 level %
    float            mO2;               // O2 level %
    int32          mId;               // Unique id, reserved for future use
    
    char             mTitle[128];       // Experiment title
    char             mUserName[128];    // Experimenter name
    char             mTreatment1[128];  // Experiment treatment1
    char             mTreatment2[128];  // Experiment treatment1
    char             mCellType[128];    // Experiment cell/organism type
    char             mImagingMode[128]; // Experiment imaging mode
    char             mComment[128];     // Experiment comment/description
};

// Stream operators for debugging display
ostream& operator << ( ostream& os, const rcMovieFileFormat& e );
ostream& operator << ( ostream& os, const rcMovieFileFormat2& e );
ostream& operator << ( ostream& os, const movieExtensionType& e );
ostream& operator << ( ostream& os, const movieOriginType& e );
ostream& operator << ( ostream& os, const movieChannelType& e );

ostream& operator << ( ostream& os, const rcMovieFileExt& e );
ostream& operator << ( ostream& os, const rcMovieFileTocExt& e );
ostream& operator << ( ostream& os, const rcMovieFileOrgExt& e );
ostream& operator << ( ostream& os, const rcMovieFileConvExt& e );
ostream& operator << ( ostream& os, const rcMovieFileCamExt& e );
ostream& operator << ( ostream& os, const rcMovieFileExpExt& e );

#endif // _rcMOVIEFILEFORMAT_H_
