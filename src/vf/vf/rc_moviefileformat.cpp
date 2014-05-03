/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_moviefileformat.cpp 7240 2011-02-21 22:37:44Z arman $
 *
 *	This file contains Reify movie file format implementation
 *
 ******************************************************************************/

#include "rc_moviefileformat.h"
#include "rc_systeminfo.h"
    //#include "rc_capture.hpp>
#include <algorithm> //required for std::swap
#include "rc_framework_core.hpp"


// Only use this for doubles and floats for now
#define ByteSwapN(x) ByteSwap(reinterpret_cast<unsigned char*> (&x),sizeof(x))

static void ByteSwap(unsigned char * b, int n)
{
   int i = 0;
   int j = n-1;
   while (i<j)
   {
      std::swap(b[i], b[j]);
      i++, j--;
   }
}
//
// Utilities
//

static void setMagic( char* buf, movieFormatRev rev )
{
    switch ( rev ) {
        case movieFormatRev0:
        case movieFormatRev1:
        case movieFormatInvalid:
            strncpy(buf, MOVIEFILE_MAGIC_REV0, MOVIEFILE_MAGIC_LTH_REV0);
            break;
        case movieFormatRev2:
            strncpy(buf, MOVIEFILE_MAGIC_REV2, MOVIEFILE_MAGIC_LTH_REV2);
            break;
    }
}

//
// rcMovieFileIdentifier class implementation
//

rcMovieFileIdentifier::rcMovieFileIdentifier() :
        mRev( movieFormatRevLatest )
{
    setMagic( mMagic, static_cast<movieFormatRev>(mRev) );
}

rcMovieFileIdentifier::rcMovieFileIdentifier( movieFormatRev rev ) :
        mRev( rev )
{
    setMagic( mMagic, rev );	
}

bool rcMovieFileIdentifier::isValid() const
{
    if ( mRev == movieFormatInvalid )
        return false;
    
    switch ( mRev ) {
        case movieFormatRev0:
        case movieFormatRev1:
            if ( strncmp(mMagic, MOVIEFILE_MAGIC_REV0, MOVIEFILE_MAGIC_LTH_REV0) )
                return false;
            break;
        case movieFormatRev2:
            if ( strncmp(mMagic, MOVIEFILE_MAGIC_REV2, MOVIEFILE_MAGIC_LTH_REV2) )
                return false;
            break;
    }

    return true;
}

// Do endian reversal if necessary
void rcMovieFileIdentifier::fixEndian()
{
    // Header is always stored in big endian form
    if ( rfPlatformByteOrder() != eByteOrderBigEndian )
		{
			ByteSwapN (mRev);
		}
}

//
// rcMovieFileFormat class implementation
//

// ctors
rcMovieFileFormat::rcMovieFileFormat() :
        mId(movieFormatInvalid), mWidth(0), mHeight(0), mRowUpdate(0),
        mAverageFrameRate(0.0), mFrameCount(0), mBaseTime(0)
{
}

rcMovieFileFormat::rcMovieFileFormat( movieFormatRev r ) :
        mId(r), mWidth(0), mHeight(0), mRowUpdate(0),
        mAverageFrameRate(0.0), mFrameCount(0), mBaseTime(0)
{
}

rcMovieFileFormat::rcMovieFileFormat( const rcSharedFrameBufPtr& frame,
                                      movieFormatRev r ) :
        mId(r), mWidth(frame->width()), mHeight(frame->height()),
        mRowUpdate(frame->rowUpdate()),
        mAverageFrameRate(0.0), mFrameCount(0), mBaseTime(frame->timestamp().tick_type_value())
{
}

// Write header to stream in binary format
bool rcMovieFileFormat::write( FILE* saveStream ) const
{
	// We write in BigEndian format
    if ( rfPlatformByteOrder() != eByteOrderBigEndian ) {
	const_cast<rcMovieFileFormat *>(this)->fixEndian ();
    }
        
    if (fwrite(this, sizeof(*this), 1, saveStream) != 1) {
        perror("MovieHdr fwrite failed");
        if (fclose(saveStream))
            perror("fclose failed");
        return true;
    }
    return false;
}

// Do endian reversal if necessary
void rcMovieFileFormat::fixEndian()
{
    // Header is always stored in big endian form
    if ( rfPlatformByteOrder() != eByteOrderBigEndian )
{
	ByteSwapN (mWidth);
	ByteSwapN (mHeight);	
	ByteSwapN (mRowUpdate);
	ByteSwapN (mAverageFrameRate);
   
	
    /* Values to allow proper navigation through the rest of the file.
     */
	ByteSwapN (mFrameCount);	
	ByteSwapN (mBaseTime);
	mId.fixEndian ();  // Common to all formats	
}
	
}

//
// rcMovieFileFormat2 class implementation
//

// ctors
rcMovieFileFormat2::rcMovieFileFormat2() :
        mId(movieFormatInvalid), mBom(rfPlatformByteOrderMark()), mWidth(0), mHeight(0), mDepth(rcPixel8), mRowUpdate(0),
        mAverageFrameRate(0.0), mFrameCount(0), mBaseTime(0), mExtensionOffset(0)
{
}

rcMovieFileFormat2::rcMovieFileFormat2( movieFormatRev r ) :
        mId(r), mBom(rfPlatformByteOrderMark()), mWidth(0), mHeight(0), mDepth(rcPixel8), mRowUpdate(0),
        mAverageFrameRate(0.0), mFrameCount(0), mBaseTime(0), mExtensionOffset(0)
{
}

rcMovieFileFormat2::rcMovieFileFormat2( const rcSharedFrameBufPtr& frame,
                                        movieFormatRev r ) :
        mId(r), mBom(rfPlatformByteOrderMark()), mWidth(frame->width()), mHeight(frame->height()),
        mDepth(frame->depth()), mRowUpdate(frame->rowUpdate()),
        mAverageFrameRate(0.0), mFrameCount(0), mBaseTime(frame->timestamp().tick_type_value()), mExtensionOffset(0)
{
}

// Write header to stream in binary format
bool rcMovieFileFormat2::write( FILE* saveStream ) const
{
	if ( rfPlatformByteOrder() != eByteOrderBigEndian ) {
		const_cast<rcMovieFileFormat2 *>(this)->fixEndian ();
    }
    if (fwrite(this, sizeof(*this), 1, saveStream) != 1) {
        perror("MovieHdr fwrite failed");
        if (fclose(saveStream))
            perror("fclose failed");
        return true;
    }
    return false;
}

// Do endian reversal if necessary
void rcMovieFileFormat2::fixEndian()
{
	   // Header is always stored in big endian form
	    if ( rfPlatformByteOrder() != eByteOrderBigEndian )
	{
		ByteSwapN (mBom);
		ByteSwapN (mWidth);
		ByteSwapN ( mDepth);		
		ByteSwapN (mHeight);	
		ByteSwapN (mRowUpdate);
	    ByteSwapN (mAverageFrameRate);

	    /* Values to allow proper navigation through the rest of the file.
	     */
		ByteSwapN (mFrameCount);	
		ByteSwapN (mBaseTime);
		ByteSwapN (mExtensionOffset); // Offset to start of extension headers	
		mId.fixEndian ();  // Common to all formats	
	}
}

//
// rcMovieFileExt class implementation
//

rcMovieFileExt::rcMovieFileExt() :
        mType( movieExtensionEOF ), mOffset(sizeof(rcMovieFileExt))
{
}

rcMovieFileExt::rcMovieFileExt( movieExtensionType t ) :
        mType( t ), mOffset( sizeof(rcMovieFileExt) )
{
}

rcMovieFileExt::rcMovieFileExt( movieExtensionType t, int64 o ) :
        mType( t ), mOffset( o )
{
}

// Write header to stream in binary format
bool rcMovieFileExt::write( FILE* saveStream ) const
{
	if ( rfPlatformByteOrder() != eByteOrderBigEndian ) {
		const_cast<rcMovieFileExt *>(this)->fixEndian (rfPlatformByteOrder(rfPlatformByteOrderMark()) );
    }
    if (fwrite(this, sizeof(*this), 1, saveStream) != 1) {
        perror("Hdr fwrite failed");
        if (fclose(saveStream))
            perror("fclose failed");
        return true;
    }
    return false;
}

// Do endian reversal if necessary
void rcMovieFileExt::fixEndian( const reByteOrder& fileByteOrder )
{
    if ( rfPlatformByteOrder() != fileByteOrder )
{
	ByteSwapN ( mType);
	ByteSwapN  (mOffset); // This is a byte offset to the beginning of the
                                // next extension starting at the beginning of this one.
}

}

//
// rcMovieFileTocExt class implementation
//

rcMovieFileTocExt::rcMovieFileTocExt() :
        rcMovieFileExt(movieExtensionTOC, sizeof(rcMovieFileTocExt)),
        mCount(0)
{
}

rcMovieFileTocExt::rcMovieFileTocExt( uint32 size ) :
        rcMovieFileExt(movieExtensionTOC, sizeof(rcMovieFileTocExt)+size*sizeof(int64)),
        mCount( size )
        
{
}

// Write header to stream in binary format
bool rcMovieFileTocExt::write( FILE* saveStream, const vector<int64>& tocTimes ) const
{
	if ( rfPlatformByteOrder() != eByteOrderBigEndian ) {
		const_cast<rcMovieFileTocExt *>(this)->fixEndian (rfPlatformByteOrder(rfPlatformByteOrderMark()) );
	}
	if (fwrite(this, sizeof(*this), 1, saveStream) != 1) {
		perror("TOC hdr fwrite failed");
		if (fclose(saveStream))
			perror("fclose failed");
		return true;
	}

	for (uint32 i = 0; i < tocTimes.size(); i++) {
		int64 timestamp = tocTimes[i];
		if ( rfPlatformByteOrder() != eByteOrderBigEndian ) {
			ByteSwapN (timestamp);
		}
		if (fwrite(&timestamp, sizeof(int64), 1, saveStream) != 1) {
			perror("fwrite of TOC timestamp failed");
			if (fclose(saveStream))
				perror("fclose failed");
			return true;
		}
	}

	return false;
}

// Do endian reversal if necessary
void rcMovieFileTocExt::fixEndian( const reByteOrder& fileByteOrder )
{
    if ( rfPlatformByteOrder() != fileByteOrder )
{
	rcMovieFileExt::fixEndian (fileByteOrder);
	ByteSwapN (mCount);
	
}
      
}

//
// rcMovieFileOrgExt class implementation
//

rcMovieFileOrgExt::rcMovieFileOrgExt() :
        rcMovieFileExt(movieExtensionORG, sizeof(rcMovieFileOrgExt)), mOrigin( movieOriginUnknown ),
        mOriginalBaseTime( 0 ), mOriginalFrameCount( 0 ), mOriginalWidth( 0 ), mOriginalHeight( 0 ),
        mOriginalDepth(rcPixelUnknown), mRev( movieFormatInvalid ), mId( 0 )
{
    memset( mComment, 0, rmDim(mComment) );
    snprintf( mComment, rmDim(mComment), "Unknown" );
}

rcMovieFileOrgExt::rcMovieFileOrgExt( movieOriginType o, int64 bt, uint32 fc,
                                      int32 w, int32 h, rcPixel depth,
                                      movieFormatRev r, const char* c ) :
        rcMovieFileExt(movieExtensionORG, sizeof(rcMovieFileOrgExt)),
        mOrigin( o ), mOriginalBaseTime( bt ), mOriginalFrameCount( fc ),
        mOriginalWidth( w ), mOriginalHeight( h ), mOriginalDepth(depth),
        mRev( r ), mId( 0 )
{
    memset( mComment, 0, rmDim(mComment) );
    if ( c )
        snprintf( mComment, rmDim(mComment), "%s", c );
    else
        snprintf( mComment, rmDim(mComment), "Unknown" );
}

// Write header to stream in binary format
bool rcMovieFileOrgExt::write( FILE* saveStream ) const
{
	if ( rfPlatformByteOrder() != eByteOrderBigEndian ) {
		const_cast<rcMovieFileOrgExt *>(this)->fixEndian (rfPlatformByteOrder(rfPlatformByteOrderMark()) );
	}
    if (fwrite(this, sizeof(*this), 1, saveStream) != 1) {
        perror("ORG hdr fwrite failed");
        if (fclose(saveStream))
            perror("fclose failed");
        return true;
    }
    return false;
}

// Do endian reversal if necessary
void rcMovieFileOrgExt::fixEndian( const reByteOrder& fileByteOrder )
{
	if ( rfPlatformByteOrder() != fileByteOrder )
	{
		rcMovieFileExt::fixEndian (fileByteOrder);
		ByteSwapN (mOriginalFrameCount);                // First frame from input movie
		ByteSwapN (mOriginalWidth);       
		ByteSwapN (mOriginalHeight);    
		ByteSwapN ( mOriginalDepth);	   
		ByteSwapN (mOriginalBaseTime);                       // Conversion date
		ByteSwapN ( mOrigin);
		ByteSwapN ( mRev);
		ByteSwapN (mId);                         // Unique id, reserved for future use

	}
}

// Write header to stream in human-readable log format
ostream& rcMovieFileOrgExt::log( ostream& os ) const
{
    os << "Origin: " << origin() << endl;
    os << " Creator: " << creatorName() << endl;
    os << " Capture date: ";
    // Capture dates are in absolute epoch time
    if (visible_framework_core::instance().check_distance_from_epoch(baseTime(), 3600*24 )){
        //    if ( baseTime() > convertSecondsToTimestamp(3600*24) ) {
        // Anything later than one day after start of epoch is 
        // considered a valid date
        rcTimestamp creationDate = rcTimestamp::from_tick_type( baseTime() );
        os << creationDate.localtime() << endl;
    } else {
        os << "Unknown" << endl;
    }
    
    if ( origin() != movieOriginUnknown ) {
        os << " FrameCount: " << frameCount()
           << " FrameDepth: " << get_bytes().count (depth())*8 << endl;
        os << " FrameSize: " << width() << "x" << height();
        if ( origin() == movieOriginCaptureCertified ||
             origin() == movieOriginCaptureUncertified )
            os << " FileRev: " << rev() << endl;
        else
            os << endl;
    } 
    
    return os;
}

//
// rcMovieFileConvExt class implementation
//

rcMovieFileConvExt::rcMovieFileConvExt() :
        rcMovieFileExt(movieExtensionCNV, sizeof(rcMovieFileConvExt)),
        mStartOffset(0), mFrameCount(0), mSample(1), mDate( rcTimestamp::now ().tick_type_value() ),
        mChannel( movieChannelUnknown ), mFrameInterval(0.0), 
        mRev(movieFormatRevLatest), mId( 0 ), mPixelsReversed(false)
{
    memset( mComment, 0, rmDim(mComment) );
    snprintf( mComment, rmDim(mComment), "Unknown" );
}

rcMovieFileConvExt::rcMovieFileConvExt( uint32 so, uint32 fc,
                                        uint32 sm, const rcRect& r,
                                        movieChannelType ch, float frameInt,
                                        bool pixRev, const char* c ) :
        rcMovieFileExt(movieExtensionCNV, sizeof(rcMovieFileConvExt)),
        mStartOffset(so), mFrameCount(fc), mSample(sm), mCropRect(r),
        mDate(rcTimestamp::now ().tick_type_value()), mChannel(ch), mFrameInterval(frameInt),
        mRev(movieFormatRevLatest), mId( 0 ), mPixelsReversed(pixRev)
{
    memset( mComment, 0, rmDim(mComment) );
    
    if ( c )
        snprintf( mComment, rmDim(mComment), "%s", c );
    else
        snprintf( mComment, rmDim(mComment), "Unknown" );
}

// Write header to stream
bool rcMovieFileConvExt::write( FILE* saveStream ) const
{
	if ( rfPlatformByteOrder() != eByteOrderBigEndian ) {
		const_cast<rcMovieFileConvExt *>(this)->fixEndian (rfPlatformByteOrder(rfPlatformByteOrderMark()) );
	}
    if (fwrite(this, sizeof(*this), 1, saveStream) != 1) {
        perror("CNV hdr fwrite failed");
        if (fclose(saveStream))
            perror("fclose failed");
        return true;
    }
    return false;
}

// Do endian reversal if necessary
void rcMovieFileConvExt::fixEndian( const reByteOrder& fileByteOrder )
{
	if ( rfPlatformByteOrder() != fileByteOrder )
	{
		rcMovieFileExt::fixEndian (fileByteOrder);
		ByteSwapN (mStartOffset);                // First frame from input movie
		ByteSwapN (mFrameCount);                 // Number of frames 
		ByteSwapN (mSample);                     // Sample interval 
		int32 x = mCropRect.x(), y = mCropRect.y();
		int32 lrx = mCropRect.lr().x(), lry = mCropRect.lr().y();
		ByteSwapN (x), ByteSwapN (y), ByteSwapN (lrx), ByteSwapN (lry);
		mCropRect = rcIRect (rcIPair (x, y), rcIPair (lrx, lry));
		ByteSwapN (mDate);                       // Conversion date
		ByteSwapN ( mChannel);
		ByteSwapN (mFrameInterval);              // Frame interval in seconds

		ByteSwapN ( mRev);
		ByteSwapN (mId);                         // Unique id, reserved for future use
		ByteSwapN (mPixelsReversed);
	}

}

// Write header to stream in human-readable log format
ostream& rcMovieFileConvExt::log( ostream& os ) const
{
    os << "Conversion date: " << rcTimestamp::from_tick_type(date()).localtime() << endl;
    os << " FileRev " << rev() << " FrameOffset: " << startOffset()
       << " FrameCount: " << frameCount() << endl;
    os << " FrameInterval " << frameInterval()
       << " Sample: " << sample() << endl;
    os << " CropRect: " << cropRect()
       << " Channel: " << channel() << endl;
    os << " PixelsReversed: ";
    if ( pixelsReversed() )
        os << "yes" << endl;
    else
        os << "no" << endl;
    os << " Converter: " << creatorName() << endl;

    return os;
}

//
// rcMovieFileCamExt class implementation
//

rcMovieFileCamExt::rcMovieFileCamExt() :
        rcMovieFileExt(movieExtensionCAM, sizeof(rcMovieFileCamExt)),
        mMfgId(rcUINT32_MAX), mUniqueId(rcUINT32_MAX), mFormat(rcUINT32_MAX), mMode(rcUINT32_MAX),
        mFrameRate(-1.0),
        mBrightness(rcUINT32_MAX), mExposure(rcUINT32_MAX), mSharpness(rcUINT32_MAX),
        mWhiteBalUB(rcUINT32_MAX), mWhiteBalVR(rcUINT32_MAX), mHue(rcUINT32_MAX),
        mSaturation(rcUINT32_MAX), mGamma(rcUINT32_MAX), mShutter(rcUINT32_MAX),
        mGain(rcINT32_MAX), mIris(rcUINT32_MAX), mFocus(rcUINT32_MAX),
        mTemperature(rcUINT32_MAX), mZoom(rcUINT32_MAX), mPan(rcUINT32_MAX),
        mTilt(rcUINT32_MAX), mFilter(rcUINT32_MAX),
        mId( 0 )
{
    memset( mName, 0, rmDim(mName) );
    snprintf( mName, rmDim(mName), "Unknown" );
}

// Write header to stream
bool rcMovieFileCamExt::write( FILE* saveStream ) const
{
	if ( rfPlatformByteOrder() != eByteOrderBigEndian ) {
		const_cast<rcMovieFileCamExt *>(this)->fixEndian (rfPlatformByteOrder(rfPlatformByteOrderMark()) );
	}
    if (fwrite(this, sizeof(*this), 1, saveStream) != 1) {
        perror("CAM hdr fwrite failed");
        if (fclose(saveStream))
            perror("fclose failed");
        return true;
    }
    return false;
}

// Do endian reversal if necessary
void rcMovieFileCamExt::fixEndian( const reByteOrder& fileByteOrder )
{
    if ( rfPlatformByteOrder() != fileByteOrder )
{
	rcMovieFileExt::fixEndian (fileByteOrder);
	ByteSwapN (mMfgId) ;                       // Manufacturer ID
    ByteSwapN (  mUniqueId) ;                   // Unique hardware ID
    ByteSwapN ( mFormat);                      // IIDC format
    ByteSwapN (mMode );                        // IIDC mode
    ByteSwapN (mFrameRate);
    ByteSwapN( mBrightness);                  // Brightness setting
    ByteSwapN (mExposure );                    // Exposure setting
    ByteSwapN (mSharpness );                   // Sharpness setting
    ByteSwapN (mWhiteBalUB );                  // White balance U/B
    ByteSwapN (mWhiteBalVR );                  // White balance V/R
    ByteSwapN (mHue );                         // Hue setting
    ByteSwapN (mSaturation );                  // Saturation setting
    ByteSwapN ( mGamma);                       // Gamma setting
    ByteSwapN (mShutter );                     // Shutter setting
    ByteSwapN ( mGain);                        // gain setting
    ByteSwapN (mIris );                        // Iris setting
    ByteSwapN ( mFocus);                       // Focus setting
    ByteSwapN (mTemperature );                 // Temperature setting
    ByteSwapN (mZoom );                        // Zoom setting
    ByteSwapN (mPan );                         // Pan setting
    ByteSwapN (mTilt );                        // Tilt setting
    ByteSwapN (mFilter );                      // Optical filter setting
	int32 x = mCropRect.x(), y = mCropRect.y();
	int32 lrx = mCropRect.lr().x(), lry = mCropRect.lr().y();
	ByteSwapN (x), ByteSwapN (y), ByteSwapN (lrx), ByteSwapN (lry);
	mCropRect = rcIRect (rcIPair (x, y), rcIPair (lrx, lry));
   	ByteSwapN ( mId);                          // Unique id, reserved for future use
}

}

// Write header to stream in human-readable log format
ostream& rcMovieFileCamExt::log( ostream& os, bool showUndefined ) const
{
    bool shown = false;
    if ( showUndefined || (!showUndefined && strcmp( name(), "Unknown" )) )
        os << " Name: " << name(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }
    if ( showUndefined || (!showUndefined && mid() != rcUINT32_MAX))
        os << " MfgID: " << mid(), shown = true;
    if ( showUndefined || (!showUndefined && uid() != rcUINT32_MAX))
        os << " UID: " << uid(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }

    if ( showUndefined || (!showUndefined && format() != rcUINT32_MAX))
        os << " Format: " << format(), shown = true;
    if ( showUndefined || (!showUndefined && mode() != rcUINT32_MAX))
        os << " Mode: " << mode(), shown = true;
    if ( showUndefined || (!showUndefined && frameRate() >= 0.0))
        os << " MaxFrameRate: " << frameRate(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }

    if ( showUndefined || (!showUndefined && brightness() != rcUINT32_MAX))
        os << " Brightness: " << brightness(), shown = true;
    if ( showUndefined || (!showUndefined && exposure() != rcUINT32_MAX))
        os << " Exposure: " << exposure(), shown = true;
    if ( showUndefined || (!showUndefined && sharpness() != rcUINT32_MAX))
        os << " Sharpness: " << sharpness(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }

    if ( showUndefined || (!showUndefined && whiteBalanceUB() != rcUINT32_MAX))
        os << " WhiteBalanceUB: " << whiteBalanceUB(), shown = true;
    if ( showUndefined || (!showUndefined && whiteBalanceVR() != rcUINT32_MAX))
        os << " WhiteBalanceVR: " << whiteBalanceVR(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }

    if ( showUndefined || (!showUndefined && hue() != rcUINT32_MAX))
        os << " Hue: " << hue(), shown = true;
    if ( showUndefined || (!showUndefined && saturation() != rcUINT32_MAX))
        os << " Saturation: " << saturation(), shown = true;
    if ( showUndefined || (!showUndefined && gamma() != rcUINT32_MAX))
        os << " Gamma: " << gamma(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }

    if ( showUndefined || (!showUndefined && shutter() != rcUINT32_MAX))
        os << " Shutter: " << shutter(), shown = true;
    if ( showUndefined || (!showUndefined && gain() != rcINT32_MAX))
        os << " Gain: " << gain(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }
 
    if ( showUndefined || (!showUndefined && iris() != rcUINT32_MAX))
        os << " Iris: " << iris(), shown = true;
    if ( showUndefined || (!showUndefined && focus() != rcUINT32_MAX))
        os << " Focus: " << focus(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }
    if ( showUndefined || (!showUndefined && temperature() != rcUINT32_MAX))
        os << " Temperature: " << temperature(), shown = true;
    if ( showUndefined || (!showUndefined && filter() != rcUINT32_MAX))
        os << " Filter: " << filter(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }
    
    if ( showUndefined || (!showUndefined && zoom() != rcUINT32_MAX))
        os << " Zoom: " << zoom(), shown = true;
    if ( showUndefined || (!showUndefined && pan() != rcUINT32_MAX))
        os << " Pan: " << pan(), shown = true;
    if ( showUndefined || (!showUndefined && tilt() != rcUINT32_MAX))
        os << " Tilt: " << tilt(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }

    const rc2Fvector pg = pixelGeometry();
    if ( showUndefined || (!showUndefined && pg != rc2Fvector(-1.0f,-1.0f)))
        os << " SensorElementSize: " << pg << " um", shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }
    
    if ( showUndefined || (!showUndefined && cropRect().width() != 0))
           os << " CropRect: " << cropRect(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }
    
    return os;
}

 // Pixel geometry in microns, (-1.0,-1.0) denotes unknown geometry
rc2Fvector rcMovieFileCamExt::pixelGeometry() const
{
	return rc2Fvector ();
}

//
// rcMovieFileExpExt class implementation
//

rcMovieFileExpExt::rcMovieFileExpExt() :
        rcMovieFileExt(movieExtensionEXP, sizeof(rcMovieFileExpExt)),
        mLensMag(-1.0f), mOtherMag(-1.0f), mTemperature(-1.0f), 
        mCO2(-1.0f), mO2(-1.0f), mId(0)
{
    memset( mTitle, 0, rmDim(mTitle) );
    memset( mUserName, 0, rmDim(mUserName) );
    memset( mTreatment1, 0, rmDim(mTreatment1) );
    memset( mTreatment2, 0, rmDim(mTreatment2) );
    memset( mCellType, 0, rmDim(mCellType) );
    memset( mImagingMode, 0, rmDim(mImagingMode) );
    memset( mComment, 0, rmDim(mComment) );
    
    snprintf( mCellType, rmDim(mCellType), "Unspecified" );
    snprintf( mImagingMode, rmDim(mImagingMode), "Unspecified" );
}

// Write header to stream
bool rcMovieFileExpExt::write( FILE* saveStream ) const
{
	if ( rfPlatformByteOrder() != eByteOrderBigEndian ) {
		const_cast<rcMovieFileExpExt *>(this)->fixEndian (rfPlatformByteOrder(rfPlatformByteOrderMark()) );
	}
    if (fwrite(this, sizeof(*this), 1, saveStream) != 1) {
        perror("EXP hdr fwrite failed");
        if (fclose(saveStream))
            perror("fclose failed");
        return true;
    }
    return false;
}

// Do endian reversal if necessary
void rcMovieFileExpExt::fixEndian( const reByteOrder& fileByteOrder )
{
	if ( rfPlatformByteOrder() != fileByteOrder )
	{
		rcMovieFileExt::fixEndian (fileByteOrder);
		ByteSwapN (mId);                         // Unique id, reserved for future use
		ByteSwapN (mLensMag); // = (float) EndianU32_LtoB ((uint32) mLensMag);              
		ByteSwapN (mOtherMag); // = (float) EndianU32_LtoB ((uint32) mOtherMag);              		
		ByteSwapN (mTemperature); //  = (float) EndianU32_LtoB ((uint32) mTemperature);              
		ByteSwapN(mCO2); //  = (float) EndianU32_LtoB ((uint32) mCO2);             	
		ByteSwapN(mO2); //  = (float) EndianU32_LtoB ((uint32) mO2);              				
	}

}

// Write header to stream in human-readable log format
ostream& rcMovieFileExpExt::log( ostream& os, bool showUndefined ) const
{
    bool shown = false;
    if ( showUndefined || (!showUndefined && strlen(title())) )
        os << " Title: " << title() << endl;
    if ( showUndefined || (!showUndefined && strlen(userName())) )
        os << " UserName: " << userName() << endl;
    if ( showUndefined || (!showUndefined && strlen(treatment1())) )
        os << " Treatment1: " << treatment1() << endl;
    if ( showUndefined || (!showUndefined && strlen(treatment2())) )
        os << " Treatment2: " << treatment2() << endl;
    if ( showUndefined || (!showUndefined && strlen(cellType())) )
        os << " CellType: " << cellType() << endl;
    if ( showUndefined || (!showUndefined && strlen(imagingMode())) )
        os << " ImagingMode: " << imagingMode() << endl;
    if ( showUndefined || (!showUndefined && strlen(comment())) )
        os << " Comment: " << comment() << endl;
      
    if ( showUndefined || (!showUndefined && lensMag() >= 0))
        os << " LensMag: " << lensMag(), shown = true;
    if ( showUndefined || (!showUndefined && otherMag() >= 0))
        os << " OtherMag: " << otherMag(), shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }
    if ( showUndefined || (!showUndefined && temperature() >= 0))
        os << " Temp: " << temperature() << "C", shown = true;
    if ( showUndefined || (!showUndefined && CO2() >= 0))
        os << " CO2: " << CO2() << "%", shown = true;
    if ( showUndefined || (!showUndefined && O2() >= 0))
        os << " O2: " << O2() << "%", shown = true;
    if ( shown ) {
        os << endl;
        shown = false;
    }
    
    return os;
}

//
// Stream operators for debugging/display
//

ostream& operator << ( ostream& os, const rcMovieFileFormat& e )
{
    os << "Hdr rev " << e.rev() << " width " << e.width() << " height " << e.height()
       << " rowUpdate " << e.rowUpdate() << " frameCount " << e.frameCount()
    << " avgFrameRate " << e.averageFrameRate() << " baseTime " << rcTimestamp::from_tick_type(e.baseTime());
     
    return os;
}

ostream& operator << ( ostream& os, const rcMovieFileFormat2& e )
{
    os << "Hdr rev " << e.rev() << " BOM " << e.bom() << " width " << e.width()
       << " height " << e.height() << " depth " << e.depth()
       << " rowUpdate " << e.rowUpdate() << " frameCount " << e.frameCount()
       << " avgFrameRate " << e.averageFrameRate() << " baseTime " << rcTimestamp::from_tick_type(e.baseTime())
       << " extOffset " << e.extensionOffset();
     
    return os;
}
    
ostream& operator << ( ostream& os, const movieOriginType& e )
{
    switch ( e ) {
    case movieOriginUnknown:
      os << "Unknown";
      break;
    case movieOriginCaptureUncertified:
      os << "Uncertified Reify Capture";
      break;
    case movieOriginCaptureCertified:
      os << "Certified Reify Capture";
      break;
    case movieOriginConversionExt:
      os << "Non-Reify Capture";
      break;
    case movieOriginSynthetic:
      os << "Synthetic";
      break;
    case movieOriginGrayIsVisibleAlphaIsFlu:
      os << "GrayIsVisibleAlphaIsFlu";
      break;
    case movieOriginMultiChannelFlu:
      os << "movieOriginMultiChannelFlu";
      break;
    case movieOriginGrayPlusMultiChannelFlu:
      os << "movieOriginGrayPlusMultiChannelFlu";
      break;
    }

    return os;
}

ostream& operator << ( ostream& os, const movieExtensionType& e )
{
    switch ( e ) {
        case movieExtensionEOF:
            os << "EOF";
            break;
        case movieExtensionTOC:
            os << "TOC";
            break;
        case movieExtensionORG:
            os << "ORG";
            break;
        case movieExtensionCNV:
            os << "CNV";
            break;
        case movieExtensionCAM:
            os << "CAM";
            break;
        case movieExtensionEXP:
            os << "EXP";
            break;
    }

    return os;
}

ostream& operator << ( ostream& os, const movieChannelType& e )
{
    switch ( e ) {
        case movieChannelUnknown:
            os << "Unknown";
            break;
        case movieChannelAll:
            os << "All";
            break;
        case movieChannelAvg:
            os << "Avg";
            break;
        case movieChannelMax:
            os << "Max";
            break;
        case movieChannelRed:
            os << "Red";
            break;
        case movieChannelGreen:
            os << "Green";
            break;
        case movieChannelBlue:
            os << "Blue";
            break;                  
    }

    return os;
}
    
ostream& operator << ( ostream& os, const rcMovieFileExt& e )
{
    os << "HdrExt " << e.type() << " offset " << e.offset();

    return os;
}

ostream& operator << ( ostream& os, const rcMovieFileTocExt& e )
{
    const rcMovieFileExt& base = e;
    os << base
       << " count " << e.count();
    
    return os;
}

ostream& operator << ( ostream& os, const rcMovieFileOrgExt& e )
{
    const rcMovieFileExt& base = e;
    os << base
       << " origin \"" << e.origin() << "\" baseTime " << rcTimestamp::from_tick_type(e.baseTime())
       << " frameCount " << e.frameCount() << " depth " << e.depth()
       << " width " << e.width() << " height " << e.height()
       << " rev " << e.rev() << " generator \"" << e.creatorName() << "\""
       << " id " << e.id();
    
    return os;
}

ostream& operator << ( ostream& os, const rcMovieFileConvExt& e )
{
    const rcMovieFileExt& base = e;
    os << base
       << " formatRev " << e.rev()
       << " startOffset " << e.startOffset() << " frameCount " << e.frameCount() << " sample " << e.sample()
       << " cropRect " << e.cropRect() << " channel " << e.channel() << " frameInt " << e.frameInterval()
       << " pixelsReversed " << e.pixelsReversed()
       << " converter \"" << e.creatorName() << "\"" << " date " << rcTimestamp::from_tick_type(e.date()).localtime()
       << " id " << e.id();
    
    return os;
}

ostream& operator << ( ostream& os, const rcMovieFileCamExt& e )
{
    const rcMovieFileExt& base = e;
    os << base
       << " name \"" << e.name() << "\"" << " mid " << e.mid() << " uid " << e.uid()
       << " format " << e.format() << " mode " << e.mode() << " maxFps " << e.frameRate()
       << " bright " << e.brightness() << " exp " << e.exposure() << " sharp " << e.sharpness()
       << " wBalUB " << e.whiteBalanceUB() << " wBalVR " << e.whiteBalanceVR()
       << " hue " << e.hue() << " sat " << e.saturation() << " gamma " << e.gamma()
       << " shutter " << e.shutter() << " gain " << e.gain() << " iris " << e.iris()
       << " focus " << e.focus() << " temp " << e.temperature() << " zoom " << e.zoom()
       << " pan " << e.pan() << " filter " << e.filter() << " crop " << e.cropRect()
       << " id " << e.id();
    os << endl;
    return os;
}

ostream& operator << ( ostream& os, const rcMovieFileExpExt& e )
{
    const rcMovieFileExt& base = e;
    os << base
       << " title " << e.title() << " user " << e.userName()
       << " treat1 " << e.treatment1() << " treat2 " << e.treatment2()
       << " cell " << e.cellType() << " imgMode " << e.imagingMode()
       << " com " << e.comment()
       << " lensMag " << e.lensMag() << " oMag " << e.otherMag()
       << " temp " << e.temperature() << " CO2 " << e.CO2() << " O2 " << e.O2()
       << " id " << e.id();
    os << endl;
    return os;
}
