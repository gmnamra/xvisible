
/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_framebuf.cpp 7297 2011-03-07 00:17:55Z arman $
 *
 *****************************************************************************/

#include "rc_framebuf.h"
#include <sys/param.h>
#include "rc_thread.h"
#include "rc_videocache.h"
#include "opencv2/highgui/highgui.hpp"
#include "rc_pixel.hpp"

using namespace cv;

rcMutex* rcFrameRef::frameMutexP = 0;

rcMutex& rcFrameRef::getMutex()
{
  if (!frameMutexP)
    frameMutexP = new rcMutex();
  
  return *frameMutexP;
}

rcFrame::rcFrame() :
    refcount_(0),
    mRawData( 0 ),
    mStartPtr( 0 ),
    mWidth( 0 ),
    mHeight( 0 ),
    mPad( 0 ),
    mAlignMod ( 0 ),
    mPixelDepth( rcPixelUnknown ),
    mRowUpdate( 0 ),
    mColorMap( 0 ),
    mColorMapSize( 0 ),
    mIsGray( 0 ),
    mOwnPixels (false),
    mCacheCtrl( 0 ),
    mFrameIndex( 0 )
{
}

rcFrame::rcFrame( int32 width, int32 height, rcPixel depth, int32 alignMod) :
    refcount_(0),
    mWidth( width ),
    mHeight( height ),
    mAlignMod ( alignMod),
    mPixelDepth( depth ),
    mColorMap( 0 ),
    mColorMapSize( 0 ),
    mIsGray( false ),
    mOwnPixels (true),
    mCacheCtrl( 0 ),
    mFrameIndex( 0 )
{
    uint8 * startPtr;
    int32 n;

    rmAssert( depth != rcPixelUnknown );
    rmAssert( height > 0 );
    rmAssert( width > 0 );

    // Make sure starting addresses are at the correct alignment boundary
    // Start of each row pointer is mAlignMod aligned
    
    int32 rowBytes = mWidth * bytes ();
    mPad = rowBytes % mAlignMod;
    int32 size;

    if (mPad) mPad = mAlignMod - mPad;
    
    // Total storage need
    mRowUpdate = rowBytes + mPad;
    size = mRowUpdate * mHeight + alignMod - 1;

    // Allocate raw data buffer
    mRawData = new uint8[size];

    // Find an Aligned Start Ptr
    startPtr = (uint8 *) mRawData;
    while ((intptr_t) startPtr % mAlignMod) startPtr ++;
    n = startPtr - (uint8 *) mRawData;
    assert ((n < mAlignMod) && (n >=0));

    // Now have to setup the row pointer logic
    mStartPtr = startPtr;
}



rcFrame::rcFrame ( const cv::Mat & onec )
: rcFrame ( onec.size().width, onec.size().height, rcPixel8, ROW_ALIGNMENT_MODULUS )
{
    rmAssert(onec.channels () == 1 );
    IplImage ipli = onec;
    loadImage (reinterpret_cast<const char*>(ipli.imageData), ipli.widthStep, ipli.width , ipli.height, rcPixel8, true);    
}


#ifdef CINDER_BUILTIN
rcFrame::rcFrame ( const ci::Channel8u & onec )
: rcFrame ( onec.getWidth (), onec.getHeight (), rcPixel8, ROW_ALIGNMENT_MODULUS )
{
    loadImage (reinterpret_cast<const char*>(onec.getData()), onec.getRowBytes (), onec.getWidth (), onec.getHeight (), rcPixel8, true);    
}


const ci::Channel8u*  rcFrame::newCiChannel ()  // copies only one channel
{
    ci::Channel8u* ch8 = new ci::Channel8u ( width(), height () );
    
    const cinder::Area clippedArea = ch8->getBounds();
    int32_t rowBytes = ch8->getRowBytes();
    
	for( int32_t y = clippedArea.getY1(); y < clippedArea.getY2(); ++y )
    {
		uint8 *dstPtr = reinterpret_cast<uint8*>( reinterpret_cast<uint8_t*>( ch8->getData() + clippedArea.getX1() ) + y * rowBytes );
        const uint8 *srcPtr = rowPointer(y);
		for( int32_t x = 0; x < clippedArea.getWidth(); ++x )
        {
			*dstPtr++ = *srcPtr++;
		}
	}	
    
    return ch8;
    
}

#endif


rcFrame::rcFrame (const char* rawPixels,
                  int32 rawPixelsRowUpdate, /* Row update for SRC pixels, not DEST frame */
                  int32 width, int32 height,
                  rcPixel pixelDepth, bool isGray)
        : refcount_(0), mWidth( width ), mHeight( height ), mAlignMod (ROW_ALIGNMENT_MODULUS), 
            mPixelDepth( pixelDepth ), mIsGray( isGray ), mOwnPixels (true), mCacheCtrl( 0 ), mFrameIndex( 0 ),   mColorMap( 0 ), mColorMapSize( 0 )

{
    uint8 * startPtr;
    int32 n;

    rmAssert( pixelDepth != rcPixelUnknown );
    rmAssert( height > 0 );
    rmAssert( width > 0 );
    rmAssert( rawPixels );

    // Make sure starting addresses are at the correct alignment boundary
    // Start of each row pointer is mAlignMod aligned
    
    int32 rowBytes = mWidth * bytes ();
    mPad = rowBytes % mAlignMod;

    if (mPad) mPad = mAlignMod - mPad;
    
    // Total storage need
    mRowUpdate = rowBytes + mPad;
    int32 size = mRowUpdate * mHeight + mAlignMod - 1;

    // Allocate raw data buffer
    mRawData = new uint8[size];

    // Find an Aligned Start Ptr
    startPtr = (uint8 *) mRawData;
    while ((intptr_t) startPtr % mAlignMod) startPtr ++;
    n = startPtr - (uint8 *) mRawData;
    assert ((n < mAlignMod) && (n >=0));

    // Now have to setup the row pointer logic
    mStartPtr = startPtr;

    // Now copy pixels into image
    for (int32 y = 0; y < mHeight; y++)
    {
      bcopy(rawPixels, rowPointer(y), mWidth);
      rawPixels += rawPixelsRowUpdate;
    }
}

void rcFrame::loadImage(const char* rawPixels,		  
                        int32 rawPixelsRowUpdate,
                        int32 width, int32 height,
                        rcPixel pixelDepth, bool isGray)
{
  rmAssert(rawPixels);
  rmAssert(mWidth == width);
  rmAssert(mHeight == height);
  rmAssert(mPixelDepth == pixelDepth);

  int32 bytesInRow = mWidth * bytes ();
  
  if ((mPad == 0) && (rawPixelsRowUpdate == bytesInRow)) {
      int32 bytesInImage = bytesInRow * mHeight;
      bcopy(rawPixels, mStartPtr, bytesInImage);
  }
  else {
      for (int32 y = 0; y < mHeight; ++y)
      {
          bcopy(rawPixels, rowPointer(y), mWidth);
          rawPixels += rawPixelsRowUpdate;
      }
  }
  
  mIsGray = isGray;
}

rcFrame::~rcFrame()
{
  if (mOwnPixels == true)
    delete [] mRawData;
  if (mColorMap != 0 ) delete [] mColorMap;
  mColorMap = 0;
}

int32 rcFrame::bits () const
{
  return bytes() * 8;
}

int32 rcFrame::bytes () const
{
  return get_bytes().count (depth());
}

uint32 
rcFrame::getPixel( int32 x, int32 y ) const
{
        uint32 val;
        
        rmAssertDebug (y >= 0);
        rmAssertDebug (y < mHeight);
        rmAssertDebug (x >= 0);
        rmAssertDebug (x < mWidth);

        switch (mPixelDepth)
        {
	   case rcPixel8:
	       val = *((uint8 *) (mStartPtr + y * mRowUpdate + x * bytes()));
	       return val;
	   case rcPixel16:
	       val = *((uint16 *) (mStartPtr + y * mRowUpdate + x * bytes()));
	       return val;
	   case rcPixel32S:
	      val = *((uint32 *) (mStartPtr + y * mRowUpdate + x * bytes () ));
	       return val;

	    default:
	       assert (1);

	}
        
        return 0;
}

// Set pixel value for (x,y). Return new value.
uint32 rcFrame::setPixel( int32 x, int32 y, uint32 val )
{
    rmAssertDebug (y >= 0);
    rmAssertDebug (y < mHeight);
    rmAssertDebug (x >= 0);
    rmAssertDebug (x < mWidth);

    switch (mPixelDepth)
      {
      case rcPixel8:
	* ((uint8 *) (mStartPtr + y * mRowUpdate + x * bytes())) = (uint8) val;
	break;
	      
      case rcPixel16:
	*((uint16 *) (mStartPtr + y * mRowUpdate + x * bytes())) = (uint16) val;
	break;

      case rcPixel32S:
	*((uint32 *) (mStartPtr + y * mRowUpdate + x * bytes())) = val;
	break;

      default:
	assert (1);
      }
    return val;
}

// Get color map value at index
uint32 rcFrame::getColor( uint32 index ) const
{
    rmAssert( index < mColorMapSize );

    if ( mColorMap )
        return mColorMap[ index ];
    else
        return 0;
}

// Set color map value at index
void rcFrame::setColor( uint32 index, uint32 value )
{
    rmAssert( index < mColorMapSize );

    if ( mColorMap )
        mColorMap[ index ] = value;
}

// Initialize colormap with default gray scale values
void rcFrame::initGrayColorMap()
{
    const double incr = 256.0/mColorMapSize;
    
    for (uint32 i = 0; i < mColorMapSize; ++i) {
        const uint32 c = uint32(i*incr);
        mColorMap[i] = rfRgb(c, c, c);
    }
}

double rcFrame::getDoublePixel( int32 x, int32 y ) const
{
   double val;

   rmAssert (mPixelDepth == rcPixelDouble);
   rmAssertDebug (y >= 0);
   rmAssertDebug (y < mHeight);
   rmAssertDebug (x >= 0);
   rmAssertDebug (x < mWidth);

   val = *((double *) (mStartPtr + y * mRowUpdate + x * bytes()));
   return val;
}

// Set pixel value for (x,y). Return new value.
double rcFrame::setDoublePixel( int32 x, int32 y, double val )
{
   rmAssert (mPixelDepth == rcPixelDouble && val <= rcDBL_MAX);
   rmAssertDebug (y >= 0);
   rmAssertDebug (y < mHeight);
   rmAssertDebug (x >= 0);
   rmAssertDebug (x < mWidth);

   *((double *) (mStartPtr + y * mRowUpdate + x * bytes())) = val;

   return val;
}

double rcFrame::operator() (double xDb, double yDb) const
{
  int32 x = (int32) xDb;
  int32 y = (int32) yDb;
  
  switch (mPixelDepth)
    {
    case rcPixel8:
    case rcPixel16:
      return (double) getPixel (x, y);
    case rcPixelFloat:
	return (double) (*((float *) (mStartPtr + y * mRowUpdate + x * bytes())));
    case rcPixel32S:
	return (double) (*((uint32 *) (mStartPtr + y * mRowUpdate + x * bytes())));

    case rcPixelDouble:
      return (double) getDoublePixel (x, y);

    default:
      rmAssert (0);
    }
  rmAssert (0);
}

void rcFrameRef::lock() const
{
    const_cast<rcFrameRef*>(this)->internalLock();
}


void rcFrameRef::unlock() const
{
    const_cast<rcFrameRef*>(this)->internalUnlock(false);
}

void rcFrameRef::prefetch() const
{
  if ((mFrameBuf == 0) && mCacheCtrl)
  {
     rcVideoCache::cachePrefetch(mCacheCtrl, mFrameIndex);
  }
}

void rcFrameRef::internalLock()
{
#ifdef DEBUG_MEM        
    cerr << "rcFrameRef lock: " << mFrameBuf << endl;
#endif
    if ( !mFrameBuf && mCacheCtrl ) {
        rcVideoCacheStatus status;
	rcVideoCacheError error;
	status = rcVideoCache::cacheLock(mCacheCtrl, mFrameIndex, *this, &error);
#ifdef DEBUG_MEM        
	cerr << "   Cached rcFrameRef " << mFrameBuf
	     << " Status " << status << " refCount ";
	rcLock frmLock(getMutex());
	if ( mFrameBuf )
	    cerr << mFrameBuf->refCount() << endl;
	else
	    cerr << "UNDEFINED" << endl;
#endif
    }
}

void rcFrameRef::internalUnlock(bool force)
{
#ifdef DEBUG_MEM        
    cerr << "rcFrameRef unlock: " << mFrameBuf << endl;
#endif
    if ( mFrameBuf && (mCacheCtrl || force)) {
        bool cacheUnlock = false;
        {
            rcLock frmLock(getMutex());

            mFrameBuf->remRef();
            if ((mFrameBuf->refCount() == 1) && mCacheCtrl)
                cacheUnlock = true;

#ifdef DEBUG_MEM
            cerr << "   refCount " << mFrameBuf->refCount()
                 << " cache unlock " << cacheUnlock << endl;
#endif
            mFrameBuf = 0;
        }
        if (cacheUnlock)
            rcVideoCache::cacheUnlock(mCacheCtrl, mFrameIndex);
    }
}

// Return a RGB color vector from HSI color vector
uint32 rfHSI2RGB (float H, float S, float I)
{
 float Htmp;
 float R,B,G;
     
 if (H == 0.0)
   R = G = B = I;
 else if (H > 0.0 && H < rk2PI/3. )
   {
     Htmp = 1 / sqrt(3.0) * tan(H - rkPI/3.);
     B = (1.0 - S) * I;
     G = (1.5 + 1.5*Htmp ) * I - ((0.5 + 1.5*Htmp) * B);
     R = 3.0 * I - G - B;
   }
 else if (H >= rk2PI/3. && H < (rk2PI+rk2PI)/3.)
   {
     Htmp = 1 / sqrt(3.0) * tan(H - rkPI);
     R = (1.0 - S) * I;
     B = (1.5 + 1.5*Htmp ) * I - ((0.5 + 1.5*Htmp) * R);
     G = 3.0 * I - B - R;
   }
 else
   { 
     Htmp = 1 / sqrt(3.0) * tan(H - (5.0 * rkPI)/3.);
     G = (1.0 - S) * I;
     R = (1.5 + 1.5*Htmp ) * I - ((0.5 + 1.5*Htmp) * G);
     B = 3.0 * I - R - G;
   }

 return rfRgb (uint32 (floor (R * 255)), uint32 (floor (G * 255)), uint32 (floor (B * 255)));

} 



