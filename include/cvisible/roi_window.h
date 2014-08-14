
#ifndef _roiWINDOW_H_
#define _roiWINDOW_H_

#include "rc_framebuf.h"
#include "cached_frame_buffer.h"
#include "rc_rect.h"
#include <iomanip>
#include <Accelerate/Accelerate.h>

#ifdef CINDER_BUILTIN
#include <cinder/Channel.h>
#endif

#include <vector>
#include <memory>
#include "cached_frame_buffer.h"
#include "qtime_cache.h"
using namespace std;
using namespace cv;

// Window class

class  roi_window
{
public:

  typedef boost::function<int(uint32, cached_frame_ref&)> proxy_fetch_frame_fn;
    typedef boost::function<int(std::pair<uint32,uint32>&)> proxy_fetch_size_fn;
    
  // Constructors
 //! default constructor takes no arguments
    /*!
    */
    roi_window() : mFrameBuf(NULL), mGeometry (0,0,0,0), mSum( 0.0 ), mSumSquares( 0.0 ), mSumValid( false )  {}

  
  //! copy / assignment  takes one arguments 
    /*!
      \param other
      \desc  Copy and assignment constructors increment the ref_count
    */
  roi_window( const roi_window& other) :
    mFrameBuf( other.frameBuf() ),
    mGeometry( other.x(), other.y(), other.width(), other.height()) {}

//! copy / assignment  takes one arguments 
    /*!
      \param rhs
      \desc  Copy and assignment constructors increment the ref_count
    */
  const roi_window& operator= (const roi_window& rhs)
  {
    if (this == &rhs) return *this;
      
    mFrameBuf = rhs.frameBuf();
    mGeometry = rhs.bound ();
    return *this;
  }
    
  // Become an entire window to a buffer in cache
  roi_window(SharedQtimeCache& cache, uint32 frameIndex);
  roi_window(proxy_fetch_size_fn cache_get_frame_size, proxy_fetch_frame_fn cache_get_frame_data,  uint32 frameIndex);
    

 //! Attach to an existing buffer
 //! takes up to four arguments
    /*!
      \param ptr is a frame buffer pointer (ref counted) 
      \param icenter is a pair of integers indicating center position in the frame buffer
      \param span is the span on each side. width & height are 2*span + 1
    */
  roi_window( cached_frame_ref ptr, int32 x, int32 y, int32 width, int32 height );
  roi_window( cached_frame_ref ptr, const rcIPair& icenter, const rcIPair& span, bool& withIn);
  roi_window( cached_frame_ref ptr, const rcRect& cropRect );
  roi_window( cached_frame_ref ptr);

 //! Attach to an existing window
 //! takes up to four arguments
    /*!
      \param ptr is a frame buffer pointer (ref counted) 
      \param icenter is a pair of integers indicating center position in the frame buffer
      \param span is the span on each side. width & height are 2*span + 1
    */
  roi_window( const roi_window& parentWindow, int32 x, int32 y, int32 width, int32 height, bool clip = false );
  roi_window( const roi_window& parentWindow, int32 width, int32 height );
  roi_window( const roi_window& parentWindow, const rcRect& cropRect, bool clip = false );
  roi_window( const roi_window& parentWindow, const rcIRect& cropRect, int32& isInside );
  roi_window( const roi_window& parentWindow, const rcIPair& icenter, const rcIPair& span, bool& withIn);

 //! Create a frameBuffer and become a entire window to its frameBuf 
 //! takes up to four arguments
    /*!
      \param size is a pair of integers indicating size of the to be created frame buffer
      \param depth is the the pixel depth of the to be created framebuf
    */
  roi_window (int32 width, int32 height, rcPixel depth  = rcPixel8 );
  roi_window (const rcIPair& size, rcPixel depth  = rcPixel8 );

  // Destructor
  virtual ~roi_window() {  }

   // Accessors
  const cached_frame_ref& frameBuf() const { return mFrameBuf; }
  cached_frame_ref& frameBuf() { return mFrameBuf; }

  rcPixel depth() const { return mFrameBuf->depth(); }
  uint32 pixelCount() const { return mGeometry.width () * mGeometry.height (); }
  int32 n () const { return mGeometry.width () * mGeometry.height (); }
  bool isGray() const { return mFrameBuf->isGray(); }

  int32 x() const { return mGeometry.x(); }
  int32 y() const { return mGeometry.y(); }

  int32 width() const { return mGeometry.width(); }
  int32 height() const { return mGeometry.height(); }

  const rcIPair& position() const { return mGeometry.origin(); }
  rcIPair position() { return rcIPair (mGeometry.origin()); }
  rcIPair size () const { return rcIPair (mGeometry.width(), mGeometry.height()); }

  rcRect bound () const { return mGeometry; }
  rcIRect frame () const { rmAssert (isBound());
    return rcIRect (0, 0, mFrameBuf->width(), mFrameBuf->height()); }
  const rcRect& rectangle () const { return mGeometry; }

  // return true if this window contains the other and has the same frame buf
  bool contains (const roi_window& other) const;

  // return true if this window contains the rect
  inline bool contains (const rcIRect& other) const {
    return (isBound() && mGeometry.contains (other));
  }

  // return true if we can peakOutside with a border
  bool canPeekOutSide (const rcIPair range) const;

  // return true if pixel addresses in this window contains this pointer (for asserts)
  bool contains (const uint8* ptr) const;

  // Is this window bound ?
  uint32 isBound () const { return mFrameBuf != 0; }

//! isWithin / isWithin takes a position in the frameBuf coordinate
    /*!
      \param rhs
      \desc  return if the point is in this window
    */
  bool isWithin (int32 x, int32 y) const { rcIPair where (x, y); return (mGeometry.contains (where)); }
  bool isWithin (const rcIPair& where) const { return (mGeometry.contains (where)); }
  bool isWithin (const rcRect& rect) const { return (mGeometry.contains (rect)); }

//! canTrim / can takes a desired trim tba to all sides
    /*!
      \param delta
      \desc  returns if the window can be trimmed in the framebuf by delta
    */
  bool canTrim (int32 delta) const;
  void maxTrim (rcIPair& tl, rcIPair& br) const;
  int32 maxTrim () const;

 	
 // Return number of bits and bytes in this pixel
  int32 bits () const;
  int32 bytes () const;

  // Slow Pixel Address
 // Return pixel value for (x,y)
  uint32 getPixel( int32 x, int32 y ) const;
  uint32 getPixel(const rcIPair& where) const;

  // Pointer Access
  // Raw pixel row pointer
  // Raw pixel pointer
  const uint8* rowPointer (int32 y) const;
  uint8 * rowPointer (int32 y) ;
  const uint8* pelPointer (int32 x, int32 y) const;
  uint8* pelPointer (int32 x, int32 y);
  int32 rowUpdate () const { return mFrameBuf->rowUpdate (); }
  int32 rowPixelUpdate () const { return mFrameBuf->rowPixelUpdate (); }

  inline bool sameTime (const roi_window& other) const
  {
    return frameBuf()->timestamp() == other.frameBuf()->timestamp();
  }

  // Return (creation) time stamp
  inline rcTimestamp timestamp() const
  { 
    return frameBuf()->timestamp();
  }

  // Window relative to an existing roi_window
  // Requires: New window to fit within parentWindow
  // Returns: Reference to this window.
  roi_window& windowRelativeTo( const roi_window& parentWindow, int32 x, int32 y, int32 width,
			      int32 height );

  // Mutators

  // Translate this window over the underlying frame buffer
  // Without modifying its width or height
  // If it results in a clip or invalid window, 
  // it returns false without translating. Else, it will translate
  // and return true
  // Note: this is implemented here to force inlining. Should be moved to .cpp

  bool translate (const rcIPair& delta);

 // become an entire window to this framebuf
  void entire ();
 
  // Trim/Shrink by a fixed amount in all sides
  roi_window& trim (uint32 delta);
  bool trim (int32 delta);
  bool trim (rcIPair  delta);

  // Set pixel value for (x,y). Return new value
  uint32 setPixel( int32 x, int32 y, uint32 value );
  uint32 setPixel(const rcIPair& where, uint32 value );
    
  // Window copy fct. Copy region can be thought of as the intersection of the two windows with their
  // origins aligned. If mirror is true, rows will be vertically mirrored.
  // Requires: Windows must both have the same pixel depth.
  // Returns: Reference to this window.
  roi_window& copyPixelsFromWindow(const roi_window& sroi_window, bool mirror = false);
    
  // Set all pixels in the window to the given value. pixelValue argument is truncated to match
  // windows pixel depth.
  // Returns: Reference to this window
  roi_window& set (double pixelValue);
  roi_window& setAllPixels(uint32 pixelValue);
  roi_window& setAllDoublePixels(double pixelValue);

  inline void atTime (const roi_window& other)
  {
    frameBuf()->setTimestamp(other.frameBuf()->timestamp());
  }
    
  // Fill this window with random pixel values and return the used seed.
  // If the seed is 0, a pseudo-random seed is produced
  // Using the same seed produces identical "random" values.
  uint32 randomFill ( uint32 seed = 0 );
    
  // Mirror image data vertically; first row becomes last row
  void mirror();

  // Output Functions
  friend ostream& operator<< (ostream&, const roi_window&);

  // Comparison Functions
  bool operator== (const roi_window&) const;
  /*
    effect     Returns true if the two windows have the same frameBuf
	       and have the same size, offset and root-offset. 
    note       If both windows are unbound, but have the same offset,
	       root-offset and size, returns true.
  */
  bool operator!= (const roi_window&) const;

  bool contentCompare (const roi_window&, bool verbose = false) const;
  /*
    effect     Returns true if the two windows have the same the same size, 
                 and image content 
    note       If both windows are unbound returns false;

  */
  bool contentMaskCompare (const roi_window&, const roi_window& mask) const;
  /*
    effect     Returns true if the two windows have the same the same size, 
                 and image content 
    note       If both windows are unbound returns false;

  */

    inline bool sumValid() const          { return mSumValid; }
    inline double sum() const             { rmAssertDebug( mSumValid ); return mSum; }
    inline double sumSquares() const      { rmAssertDebug( mSumValid ); return mSumSquares; }

    //
    // Mutators
    //
    inline void sum( double s )              { mSum = s; mSumValid = true; };
    inline void sumSquares( double s )       { mSumSquares = s; rmAssertDebug( mSumValid ); };
    inline void invalidate()                 { mSumValid = false; };

    template <class T>
    void print (T, ostream& = cerr);

    template <class T>
    void print (T);

#ifdef CINDER_BUILTIN
  ci::Channel8u* new_channel () const;
#endif
    
 protected:

  //@param w integer 
  //@param h integer
  //@param depth integer
  void init (int32 w, int32 h, rcPixel depth);

  // window is a helper function
  //@param parentWindow is a roi_window
  //@param x integer 
  //@param y integer
  //@param w integer
  //@param h integer
  //@param clip bool
  //@return bool 
  bool window (const roi_window& parentWindow, int32 x, int32 y, int32 width, int32 height, bool clip);
  cached_frame_ref mFrameBuf;  // Ref-counted pointer to frame buffer
  rcRect				 mGeometry;	 // Window geometry info
 
    double   mSum;                // Sum of pixel values
    double   mSumSquares;         // Sum of pixel value squares
    bool     mSumValid;           // Is sum valid
    
};


template <class T>
void roi_window::print (T, ostream& o)
{
  o << endl;
  for (int32 j = 0; j < height(); j++)
    {							
      for (int32 i = 0; i < width(); i++)
	{
	  o <<  getPixel (i, j);
	  if (i < width() - 1) 
	    o << " ";
	} 
      o << endl;
    }
  //  return o;
}

/*
 * Set a 1 pixel border around a pelbuffer to a particular value
 */
void rfSetWindowBorder (roi_window& win, double val);


/*
 * a very thin layer supporting templated pixel access
 */

template <class T>
class roi_window_t : public roi_window
{
public:
    //
    // ctors
    //
    roi_window_t() : roi_window () {}
    
    roi_window_t( const roi_window& w ) : roi_window ( w )
    {
        // Verify that depths match
        if ( sizeof(T) != static_cast<int>(w.bytes()) )
            throw general_exception( "roi_window_t ctor: invalid roi_window depth" );
    }
      // Warning: these are dangerous casts
    inline const T* rowPelPointer (uint32 y) const          { return (const T*)(rowPointer( y )); }
    inline const T* pelPointer (uint32 x, uint32 y) const { return (const T*)(pelPointer( x, y )); };

};


#endif // _roiWINDOW_H_
