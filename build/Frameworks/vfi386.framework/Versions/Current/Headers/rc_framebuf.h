/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_framebuf.h 7291 2011-03-05 01:57:35Z arman $
 *
 *****************************************************************************/

#ifndef _rcFRAMEBUF_H_
#define _rcFRAMEBUF_H_

#include "rc_timestamp.h"
#include "rc_thread.h"
#include "rc_pixel.hpp"
#include <boost/intrusive_ptr.hpp>
#include <boost/atomic.hpp>
#include <opencv2/core/core.hpp>
#include "rc_pixel.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "singleton.hpp"

#ifdef CINDER_BUILTIN
#include <cinder/Channel.h>
#endif


using namespace cv;

// Frame buffer.base class
// Clients may NOT use this class directly, reference-counted
// rcFrameRef must be used instead.


class rcFrame
{
   // friend class rcVideoCache;
    
public:
    enum { ROW_ALIGNMENT_MODULUS = 16 }; // Mod for row alignment. Default for AltiVec
    
    
    // Constructors
    rcFrame();
    rcFrame( int32 width, int32 height, rcPixel depth, int32 alignMod = ROW_ALIGNMENT_MODULUS );
    /*
     * In call cases pixels are copied from client storage to newly created memory.
     * Row update for SRC pixels, not DEST frame
     */
    
    rcFrame (const char* rawPixels, int32 rawPixelsRowUpdate, int32 width, int32 height, rcPixel pixelDepth, bool isGray);
    rcFrame ( const cv::Mat& );
    
#ifdef CINDER_BUILTIN
    rcFrame ( const ci::Channel8u&  );
#endif
    
    
    
    // Destructor
    virtual ~rcFrame();
    
    // Accessors
    
    // Raw pixel data pointer
    inline const uint8* rawData() const { return mRawData; }
    inline uint8* rawData() { return mRawData; }
    // Aligned raw pixel data pointer
    inline const uint8* alignedRawData() const { return mStartPtr; }
    inline uint8* alignedRawData() { return mStartPtr; }
    
    // Get rowPointer address
    inline const uint8* rowPointer ( int32 y ) const {
        rmAssertDebug (y >= 0 && y < mHeight);
        return (mStartPtr + y * mRowUpdate);
    }
    
    inline uint8* rowPointer ( int32 y ) {
        rmAssertDebug (y >= 0 && y < mHeight);
        return (mStartPtr + y * mRowUpdate);
    }
    
    // Get pixel address
    inline const uint8* pelPointer ( int32 x, int32 y ) const {
        rmAssertDebug (y >= 0 && y < mHeight);
        rmAssertDebug (x >= 0 && x < mWidth);
        return (mStartPtr + y * mRowUpdate + x * bytes());
    }
    
    inline uint8* pelPointer ( int32 x, int32 y ) {
        rmAssertDebug (y >= 0 && y < mHeight);
        rmAssertDebug (x >= 0 && x < mWidth);
        return (mStartPtr + y * mRowUpdate + x * bytes());
    }
    
    // Color map pointer
    inline const uint32* colorMap() const { return mColorMap; };
    // Get color map size (number of colors)
    inline uint32 colorMapSize() const { return mColorMapSize; };
    // Set color map value at index
    void setColor( uint32 index, uint32 value );
    // Get color map value at index
    uint32 getColor( uint32 index ) const;
    // Initialize colormap with default gray values
    void initGrayColorMap();
    
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
    inline rcPixel depth() const { return mPixelDepth; }
    // Total number of pixels
    inline uint32 pixelCount() const { return mWidth * mHeight; }
    // Whether or not pixels are all 8-bit gray scale
    inline bool isGray() const { return mIsGray; }
    
    // Row alignment in bytes
    inline int32 alignment() const { return mAlignMod; }
    // Return pixel value for (x,y)
    uint32 getPixel( int32 x, int32 y ) const;
    // Return a double pixel value for (x,y)
    double getDoublePixel( int32 x, int32 y ) const;
    // Return (creation) time stamp
    inline rcTimestamp timestamp() const { return mTimestamp; };
    // Return z value
    inline double zVal() const { return mZvalue; };
    // Return number of bits and bytes in this pixel
    int32 bits () const;
    int32 bytes () const;
    
    double operator() (double, double) const;
    
    // Mutators
    
    // Set pixel value for (x,y). Return new value
    uint32 setPixel( int32 x, int32 y, uint32 value );
    double setDoublePixel( int32 x, int32 y, double value );
    
    // Set boolean indicating whether or not all pixels can be treated as 8 bit gray scale
    inline bool setIsGray(bool isGray) { return mIsGray = isGray; }
    
    
    // Set (creation) time stamp
    inline void setTimestamp( rcTimestamp time ) { mTimestamp = time; }
    inline void zVal(double dv) { mZvalue = dv; }
    
    // Load the image pointed to by rawPixels into this frame
    void loadImage(const char* rawPixels, int32 rawPixelsRowUpdate, int32 width,
                   int32 height, rcPixel pixelDepth, bool isGray);
    
    inline uint32  cacheCtrl() const { return mCacheCtrl; }
    inline uint32  frameIndex() const {return mFrameIndex; }
	
	
    // Increase the reference count by one
	void addRef() const
	{
		intrusive_ptr_add_ref (this);
	}
	
    // Remove a reference (decreases the reference count by one). If
    // the count goes to 0, this object will delete itself
    void remRef() const
    {
		intrusive_ptr_release (this);
    }
	
	int refCount () const { return refcount_.load (); }
	
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
    rcPixel    mPixelDepth;// Bytes of storage per pixel
    int32         mRowUpdate; // Row Update Constant
    rcTimestamp     mTimestamp; // Creation time stamp
    uint32*       mColorMap;  // Color map
    uint32        mColorMapSize;  // Color map size
    bool          mIsGray;    // Indicates whether or not all pixels are 8 bit gray scale
    bool          mOwnPixels; // Do we own these pixels
    uint32        mCacheCtrl;
    uint32        mFrameIndex;
    double          mZvalue; // Z label for this frame coming from a z stack
    
    
private:
	mutable boost::atomic<int> refcount_;
	friend void intrusive_ptr_add_ref(const rcFrame * x)
	{
		x->refcount_.fetch_add(1, boost::memory_order_relaxed);
	}
	friend void intrusive_ptr_release(const rcFrame * x)
	{
		if (x->refcount_.fetch_sub(1, boost::memory_order_release)==1)
		{
			boost::atomic_thread_fence(boost::memory_order_acquire);
			delete x;
		}
	}
    // Prohibit direct copying and assignment
    rcFrame( const rcFrame& );
    rcFrame& operator=( const rcFrame& );
};

#undef DEBUG_MEM // ctor/ftor/asignment and refCount debugging

class rcVideoCache;

// Reference counted frame buffer. Frame buffer memory will be deallocated when reference count goes to zero.
// Windows and regions must use this class.

class  rcFrameRef
{
    
public:
    
    
    // Constructors
    rcFrameRef()
    : mCacheCtrl ( 0 ), mFrameIndex ( 0 ), mFrameBuf( 0 ) { }
    rcFrameRef( rcFrame* p )
    : mCacheCtrl ( 0 ), mFrameIndex ( 0 ), mFrameBuf( p ) {
        if (p) {
            mCacheCtrl = p->cacheCtrl();
            mFrameIndex = p->frameIndex();
        }
        if ( mFrameBuf ) {
            rcLock frmLock(getMutex());
#ifdef DEBUG_MEM
            cerr << "rcFrameRef ctor " << mFrameBuf
            << " refCount " << mFrameBuf->refCount() << endl;
#endif
            mFrameBuf->addRef();
        }
    }
    rcFrameRef( const rcFrameRef& p )
    : mCacheCtrl ( p.mCacheCtrl ), mFrameIndex ( p.mFrameIndex ),
    mFrameBuf( p.mFrameBuf ) {
        if ( mFrameBuf ) {
            rcLock frmLock(getMutex());
#ifdef DEBUG_MEM
            cerr << "rcFrameRef ctor " << mFrameBuf
            << " refCount " << mFrameBuf->refCount() << endl;
#endif
            mFrameBuf->addRef();
        }
    }
    
    // Destructor
    ~rcFrameRef() {
#ifdef DEBUG_MEM
        cerr << "rcFrameRef dtor " << endl;
#endif
        internalUnlock(true);
    }
    
    // Assignment operators
    rcFrameRef& operator= ( const rcFrameRef& p ) {
        if ( (mFrameBuf == p.mFrameBuf) && (mCacheCtrl == p.mCacheCtrl) &&
            (mFrameIndex == p.mFrameIndex) )
            return *this;
#ifdef DEBUG_MEM
        {
            rcLock frmLock(getMutex());
            cerr << "rcFrameRef asss " << mFrameBuf;
            if ( mFrameBuf )
                cerr << " refCount " << mFrameBuf->refCount();
            cerr << " = " << p.mFrameBuf;
            if ( p.mFrameBuf )
                cerr << " refCount " << p.mFrameBuf->refCount();
            cerr << endl;
        }
#endif
        internalUnlock(true);
        mCacheCtrl = p.mCacheCtrl;
        mFrameIndex = p.mFrameIndex;
        mFrameBuf = p.mFrameBuf;
        if ( mFrameBuf ) {
            rcLock frmLock(getMutex());
            mFrameBuf->addRef();
        }
        return *this;
    }
    
    rcFrameRef& operator= ( rcFrame* p ) {
        if ( (mFrameBuf == p) && (mFrameBuf || !mCacheCtrl) )
            return *this;
#ifdef DEBUG_MEM
        {
            rcLock frmLock(getMutex());
            cerr << "rcFrameRef assp " << mFrameBuf;
            if ( mFrameBuf )
                cerr << " refCount " << mFrameBuf->refCount();
            cerr << " = " << p;
            if ( p )
                cerr << " refCount " << p->refCount();
            cerr << endl;
        }
#endif
        internalUnlock(true);
        mFrameBuf = p;
        if ( mFrameBuf ) {
            mCacheCtrl = p->cacheCtrl();
            mFrameIndex = p->frameIndex();
            rcLock frmLock(getMutex());
            mFrameBuf->addRef();
        }
        else {
            mCacheCtrl = 0;
            mFrameIndex = 0;
        }
        
        return *this;
    }
    
    /* For cached frames, the cache controller is called to load the
     * underlying frame from either cache or disk and to lock it into
     * the cache so it is safe to use it.
     *
     * Note: This is a NOOP for uncached frames.
     */
    virtual void lock() const;
    
    /* For cached frames, the cache controller is called to mark this
     * frame as available for reuse. The frame's data is retained in
     * cache, so that a subsequent call to lock() can retrieve the
     * data. Caching is controlled using a least-recently-used
     * algorithm.
     *
     * WARNING: Any rcFrame related memory references, such as the
     * pointer returned by rawData, or a reference to the rcFrame
     * object itself is only valid while the frame is locked. To use
     * these values again, it is necessary to relock the frame and
     * then reread these values.
     *
     * Note: This is a NOOP for uncached frames.
     */
    virtual void unlock() const;
    
    /* Ask cache controller to try and read in this frame now without
     * actually locking it to this object.
     */
    virtual void prefetch() const;
    
    uint32 frameIndex() const {
        return mFrameIndex;
    }
    
    // Comparison operators
    bool operator== ( const rcFrameRef& p ) const {
        if ( mCacheCtrl || p.mCacheCtrl )
            return ((mCacheCtrl == p.mCacheCtrl) &&
                    (mFrameIndex == p.mFrameIndex));
        return ( mFrameBuf == p.mFrameBuf );
    }
    bool operator!= ( const rcFrameRef& p ) const {
        return !(this->operator==( p ));
    }
    bool operator== ( const rcFrame* p ) const {
        if ( mCacheCtrl )
            return (p && (mCacheCtrl == p->cacheCtrl()) &&
                    (mFrameIndex == p->frameIndex()));
        return ( mFrameBuf == p );
    }
    bool operator!= ( const rcFrame* p ) const {
        return !(this->operator==( p ));
    }
    bool operator!() const {
        return ( ( mFrameBuf == 0 ) && (mCacheCtrl == 0));
    }
    
    // Accessors
    operator rcFrame*() const {
        if ( !mFrameBuf && mCacheCtrl ) lock();
            return mFrameBuf;
    }
    const rcFrame& operator*() const {
        if ( !mFrameBuf && mCacheCtrl ) lock();
        rmAssertDebug( mFrameBuf );
        return *mFrameBuf;
    }
    rcFrame& operator*() {
        if ( !mFrameBuf && mCacheCtrl ) lock();
        rmAssertDebug( mFrameBuf );
        return *mFrameBuf;
    }
    const rcFrame* operator->() const {
        if ( !mFrameBuf && mCacheCtrl ) lock();
        return mFrameBuf;
    }
    rcFrame* operator->() {
        if ( !mFrameBuf && mCacheCtrl ) lock();
        return mFrameBuf;
    }
    
    int refCount() const {
        int retVal = 0;
        if ( !mFrameBuf && mCacheCtrl ) lock();
        if ( mFrameBuf ) {
            rcLock frmLock(getMutex());
            retVal = mFrameBuf->refCount();
        }
        return retVal;
    }
    
    void printState() const {
        cerr << "fb c, i, b [" << mCacheCtrl << ", " << mFrameIndex << ", " << mFrameBuf << "]" << endl;
    }

    
    /* Helper fct intended for use by rcVideoCache class only. It allows
     * a shared frame buffer pointer to be initialized pointing to a
     * cached frame.
     */
    void setCachedFrame(rcFrameRef& p, uint32 cacheID,
                        uint32 frameIndex)
    {
        rmAssert(p.mFrameBuf);
        rmAssert(!mFrameBuf);
        rmAssert(cacheID);
        mFrameBuf = p.mFrameBuf;
        mCacheCtrl = cacheID;
        mFrameIndex = frameIndex;
        
        rcLock frmLock(getMutex());
        mFrameBuf->addRef();
    }
    
    /* Helper fct intended for use by rcVideoCache class only. It allows
     * a shared frame buffer pointer to be initialized pointing to a
     * cached frame without reading that frame into memory.
     */
    void setCachedFrameIndex(uint32 cacheID, uint32 frameIndex)
    {
        rmAssert(!mFrameBuf);
        rmAssert(cacheID);
        mCacheCtrl = cacheID;
        mFrameIndex = frameIndex;
    }

    /*
     * public but only to avoid excessive friendship
     */
    uint32      mCacheCtrl;
    uint32      mFrameIndex;
    mutable rcFrame*    mFrameBuf;

protected:
    static rcMutex& getMutex();
    static rcMutex* frameMutexP;
    
private:
    void internalLock();
    void internalUnlock( bool force );

};



// Utility function to transform between color spaces
uint32 rfHSI2RGB (float H, float S, float I);



#define rcFrameBufPtr rcFrameRef


#endif
