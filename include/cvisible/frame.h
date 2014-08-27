

#ifndef _FRAMEBUF_H_
#define _FRAMEBUF_H_

#include "vf_utils.hpp"
#include <boost/atomic.hpp>
#include <opencv2/core/core.hpp>
#include "rpixel.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include "intrusive_ptr_base.hpp"

#ifdef CINDER_BUILTIN
#include <cinder/Channel.h>
#endif


using namespace cv;
using namespace vf_utils;

// Frame buffer.base class
// Clients may NOT use this class directly, reference-counted
// raw_frameRef must be used instead.



class raw_frame : public intrusive_ptr_base<raw_frame>
{
    
public:
    
    
    enum { ROW_ALIGNMENT_MODULUS = 16 }; // Mod for row alignment. Default for AltiVec
    
    
    // Constructors
    raw_frame() :
    mRawData( 0 ), mStartPtr( 0 ),mWidth( 0 ),mHeight( 0 ), mPad( 0 ), mAlignMod ( 0 ),
    mPixelDepth( rpixel_unknown ), mRowUpdate( 0 ), mOwnPixels (false),
    mCacheCtrl( 0 ), mFrameIndex( 0 )
    {}
    
    raw_frame( int32 width, int32 height, pixel_ipl_t depth, int32 alignMod = ROW_ALIGNMENT_MODULUS ) :
    mWidth( width ),  mHeight( height ),  mAlignMod ( alignMod), mPixelDepth( depth ),
    mOwnPixels (true),  mCacheCtrl( 0 ), mFrameIndex( 0 )
    {
        m_type_ref = pixel_type_registry::instance().get_shared (depth);
        
        int32 n;
        
        assert( depth !=  rpixel_unknown );
        assert( height > 0 );
        assert( width > 0 );
        
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
        mStartPtr = (uint8 *) mRawData;
        while ((intptr_t) mStartPtr % mAlignMod) mStartPtr ++;
        n = mStartPtr - (uint8 *) mRawData;
        assert ((n < mAlignMod) && (n >=0));
     
    }

    /*
     * In call cases pixels are copied from client storage to newly created memory.
     * Row update for SRC pixels, not DEST frame
     */
    
    raw_frame (const char* rawPixels, int32 rawPixelsRowUpdate, int32 width, int32 height, pixel_ipl_t pixelDepth)
    : mWidth( width ), mHeight( height ), mAlignMod (ROW_ALIGNMENT_MODULUS), mPixelDepth( pixelDepth ),  mOwnPixels (true),
    mCacheCtrl( 0 ), mFrameIndex( 0 )
    {
        uint8 * startPtr;
        int32 n;
        
        assert( pixelDepth != rpixel_unknown );
        assert( height > 0 );
        assert( width > 0 );
        assert( rawPixels );
        
        m_type_ref = pixel_type_registry::instance().get_shared (pixelDepth);
        
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

    
    
    /*
     * From cv::Mat @todo add depth support other than u8 and check
     */
    
    raw_frame ( const cv::Mat& onec):
    raw_frame ( onec.size().width, onec.size().height, rpixel8 , ROW_ALIGNMENT_MODULUS )
    {
        assert(onec.channels () == 1 );
        IplImage ipli = onec;
        loadImage (reinterpret_cast<const char*>(ipli.imageData), ipli.widthStep, ipli.width , ipli.height, rpixel8);
    }
#ifdef CINDER_BUILTIN
    raw_frame  ( const ci::Channel8u & onec )
    : raw_frame ( onec.getWidth (), onec.getHeight (), pixel_ipl_t8, ROW_ALIGNMENT_MODULUS )
    {
        loadImage (reinterpret_cast<const char*>(onec.getData()), onec.getRowBytes (), onec.getWidth (), onec.getHeight (), pixel_ipl_t8, true);
    }
    
    
    const ci::Channel8u*  newCiChannel ()  // copies only one channel
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
    
    
    // Destructor
    virtual ~raw_frame()
    {
        if (mOwnPixels == true)
            delete [] mRawData;
    }
    
    
    // Accessors
    
    // Raw pixel data pointer
    inline const uint8* rawData() const { return mRawData; }
    inline uint8* rawData() { return mRawData; }
    // Aligned raw pixel data pointer
    inline const uint8* alignedRawData() const { return mStartPtr; }
    inline uint8* alignedRawData() { return mStartPtr; }
    
    // Get rowPointer address
    inline const uint8* rowPointer ( int32 y ) const {
    //    assertDebug (y >= 0 && y < mHeight);
        return (mStartPtr + y * mRowUpdate);
    }
    
    inline uint8* rowPointer ( int32 y ) {
      //  assertDebug (y >= 0 && y < mHeight);
        return (mStartPtr + y * mRowUpdate);
    }
    
    // Get pixel address
    inline const uint8* pelPointer ( int32 x, int32 y ) const {
      //  assertDebug (y >= 0 && y < mHeight);
      //  assertDebug (x >= 0 && x < mWidth);
        return (mStartPtr + y * mRowUpdate + x * bytes());
    }
    
    inline uint8* pelPointer ( int32 x, int32 y ) {
      //  assertDebug (y >= 0 && y < mHeight);
     //   assertDebug (x >= 0 && x < mWidth);
        return (mStartPtr + y * mRowUpdate + x * bytes());
    }
    
    // Row update constant: constant address to add to next pixel next row
    inline int32 rowUpdate () const { return mRowUpdate; };
    
    // Row update constant: constant address to add to next pixel next row
    inline int32 rowPixelUpdate () const { return mRowUpdate / (bytes()) ; };
    
    // Row update constant: constant address from last pixel in row i to first pixel in i+1
    inline int32 rowPad () const { return mPad; };
    // Buffer width in pixels
    inline int32 width() const { return mWidth; }
    // Buffer height in pixels
    inline int32 height() const { return mHeight; }
    // Pixel depth in bytes
    inline pixel_ipl_t depth() const { return mPixelDepth; }
    // Total number of pixels
    inline uint32 pixelCount() const { return mWidth * mHeight; }
    // Whether or not pixels are all 8-bit gray scale
    inline bool isGray() const { return mIsGray; }
    
    // Row alignment in bytes
    inline int32 alignment() const { return mAlignMod; }
    
    
    /*
     *  Return pixel value for (x,y) @todo change to template functions
     */
    
    uint32 getPixel( int32 x, int32 y ) const
    {
        uint32 val;
        
        assert (y >= 0);
        assert (y < mHeight);
        assert (x >= 0);
        assert (x < mWidth);
        
        switch (mPixelDepth)
        {
            case rpixel8:
                val = *((uint8 *) (mStartPtr + y * mRowUpdate + x * bytes()));
                return val;
            case rpixel16:
                val = *((uint16 *) (mStartPtr + y * mRowUpdate + x * bytes()));
                return val;
            case rpixel32:
                val = *((uint32 *) (mStartPtr + y * mRowUpdate + x * bytes () ));
                return val;
                
            default:
                assert (1);
                
        }
        
        return 0;
    }

   
    // Return (creation) time stamp
    inline time_spec_t timestamp() const { return mTimestamp; };
    // Return z value

    int32 bits () const
    {
        return bytes() * 8;
    }

    int32 bytes () const
    {
        assert (m_type_ref);
        std::shared_ptr<pixel_type_base> sp = m_type_ref;
        return sp->bytes();

    }

    // Mutators
    
    /* Set pixel value for (x,y). Return new value.
     * @todo change to template functions
     */
    uint32 setPixel( int32 x, int32 y, uint32 val )
    {
        assert (y >= 0);
        assert (y < mHeight);
        assert (x >= 0);
        assert (x < mWidth);
        
        switch (mPixelDepth)
        {
            case rpixel8:
                * ((uint8 *) (mStartPtr + y * mRowUpdate + x * bytes())) = (uint8) val;
                break;
                
            case rpixel16:
                *((uint16 *) (mStartPtr + y * mRowUpdate + x * bytes())) = (uint16) val;
                break;
                
            case rpixel32:
                *((uint32 *) (mStartPtr + y * mRowUpdate + x * bytes())) = val;
                break;
                
            default:
                assert (1);
        }
        return val;
    }
    
  
     // Set (creation) time stamp
    inline void setTimestamp( time_spec_t time ) { mTimestamp = time; }
    
    // Load the image pointed to by rawPixels into this frame
    void loadImage(const char* rawPixels, int32 rawPixelsRowUpdate, int32 width,
                   int32 height, pixel_ipl_t pixelDepth)
    {
        assert(rawPixels);
        assert(mWidth == width);
        assert(mHeight == height);
        assert(mPixelDepth == pixelDepth);
        
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
        
    }
    
    inline uint32  cacheCtrl() const { return mCacheCtrl; }
    inline uint32  frameIndex() const {return mFrameIndex; }
	
    
    // Increase the reference count by one
	void addRef() const
	{
		intrusive_ptr_add_ref (this);
	}
	
    // Remove a reference (decreases the reference count by one). If
    // the count goes to 0, delete this object
    uint32 remRef() const
    {
        if (uint32 rc = intrusive_ptr_rem_ref (this)) return rc;
        intrusive_ptr_release (this);
        return 0;
    }
	
	int refCount () const
    
    {
        return intrusive_ptr_ref_count(this);
    }

    
#ifdef CINDER_BUILTIN
    const ci::Channel8u*  newCiChannel ();  // copies only one channel
#endif
    
    // Setters intended to only be called by rcVideoCache class code
    void  cacheCtrl(uint32 cacheCtrl) { mCacheCtrl = cacheCtrl; }
    void  frameIndex(uint32 frameIndex) { mFrameIndex=frameIndex; }

protected:
    uint8*         mRawData;   // Raw pixel data
    uint8*         mStartPtr;  // Alignment Adjusted Raw data pointer
    int32         mWidth;     // Width in pixels
    int32         mHeight;    // Height in pixels
    int32         mPad;       // Bytes from last pixel of row i to first pixel of row i+1
    int32         mAlignMod;  // Alignment modules optionally required
    pixel_ipl_t          mPixelDepth;// Bytes of storage per pixel
    int32         mRowUpdate; // Row Update Constant
    time_spec_t     mTimestamp; // Creation time stamp
    uint32*       mColorMap;  // Color map
    uint32        mColorMapSize;  // Color map size
    bool          mIsGray;    // Indicates whether or not all pixels are 8 bit gray scale
    bool          mOwnPixels; // Do we own these pixels
    uint32        mCacheCtrl;
    uint32        mFrameIndex;
    double          mZvalue; // Z label for this frame coming from a z stack
    pixel_type_base_ref m_type_ref;
    
    
private:
    
    // Prohibit direct copying and assignment
    raw_frame( const raw_frame& );
    raw_frame& operator=( const raw_frame& );
};



#endif
