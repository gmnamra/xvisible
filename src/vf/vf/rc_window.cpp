

#include <rc_window.h>
#include <rc_videocache.h>
#include <sys/param.h>
#include <rc_macro.h>
#include <rc_tiff.h>
#include <tiff.h>
#include <rc_systeminfo.h>
#include <cinder/Channel.h>
#include <cinder/Area.h>

ci::Channel8u* rcWindow::new_channel () const
{
    if (!isBound()) return 0;
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


const uint8 *rcWindow::rowPointer (int32 y) const
 { 
   rmAssertDebug( y >= 0 ); 
   return (mFrameBuf->rowPointer (y + mGeometry.y() ) + mGeometry.x() * mFrameBuf->depth ()); 
 }

uint8 *rcWindow::rowPointer (int32 y) 
 { 
   rmAssertDebug( y >= 0 ); 
   return (mFrameBuf->rowPointer (y + mGeometry.y() ) + mGeometry.x() * mFrameBuf->depth ()); 
 }

const uint8* rcWindow::pelPointer (int32 x, int32 y) const
  { 
    rmAssertDebug( x >= 0 && y >= 0 );
    return (mFrameBuf->pelPointer (x + mGeometry.x(), y + mGeometry.y()));
  }

uint8*  rcWindow::pelPointer (int32 x, int32 y) 
{ 
  rmAssertDebug( x >= 0 && y >= 0 );
  return (mFrameBuf->pelPointer (x + mGeometry.x(), y + mGeometry.y()));
}
   
rcWindow::rcWindow(rcFrameBufPtr ptr, int32 x, int32 y, int32 width, int32 height ) :
        mFrameBuf( ptr ), 
        mGeometry( x, y, width, height ) 
{
    rmAssertDebug( x >= 0 && y >= 0 );
    rmAssertDebug( width > 0 && height > 0 );
    rmAssert( x < mFrameBuf->width() );
    rmAssert( (x + width - 1) < mFrameBuf->width() );
    rmAssert( y < mFrameBuf->height() );
    rmAssert( (y + height - 1) < mFrameBuf->height() );
}

rcWindow::rcWindow(rcFrameBufPtr ptr, const rcRect& cropRect ) :
        mFrameBuf( ptr ), 
        mGeometry( cropRect ) 
{
    rmAssertDebug( cropRect.x() >= 0 && cropRect.y() >= 0 );
    rmAssert( cropRect.x() < mFrameBuf->width() );
    rmAssert( (cropRect.x() + cropRect.width() - 1) < mFrameBuf->width() );
    rmAssert( cropRect.y() < mFrameBuf->height() );
    rmAssert( (cropRect.y() + cropRect.height() - 1) < mFrameBuf->height() );
}

rcWindow::rcWindow(rcFrameBufPtr ptr) :
        mFrameBuf( ptr ),
        mGeometry(0, 0, ptr->width(), ptr->height()) 
{
    rmAssert( mGeometry.x() < mFrameBuf->width() );
    rmAssert( mGeometry.y() < mFrameBuf->height() );
}

rcWindow::rcWindow(rcVideoCache& cache, uint32 frameIndex) :
        mFrameBuf(0), mGeometry(0, 0, cache.frameWidth(), cache.frameHeight())
{
    rmAssert( mGeometry.width() > 0 );
    rmAssert( mGeometry.height() > 0 );

    rcVideoCacheStatus status =
        cache.getFrame(frameIndex, mFrameBuf, 0, false);

    rmAssert(status == eVideoCacheStatusOK);
}



/**
 *
 *  From a Tiff File
 *
 */

rcWindow::rcWindow (std::string absPath_filename)
{
	// Get a TIFF Importer
	TIFFImageIO t_importer;
	if (! t_importer.CanReadFile (absPath_filename.c_str () ) ) 
		*this = rcWindow ();
	t_importer.SetFileName (absPath_filename.c_str ());
	t_importer.ReadImageInformation ();
	*this = t_importer.ReadSinglePage ();
	
}

        

rcWindow::rcWindow(rcFrameBufPtr ptr, const rcIPair& icenter, const rcIPair& span, bool& withIn) :
        mFrameBuf( ptr ),
        mGeometry(icenter.x() - span.x(), 
                  icenter.y() - span.y(), 
                  span.x()+span.x()+1, span.y()+span.y()+1)
{
    withIn = ((mGeometry.x() + mGeometry.width()) < mFrameBuf->width()) && 
        ((mGeometry.y() + mGeometry.height()) < mFrameBuf->height());
}


rcWindow::rcWindow(int32 width, int32 height, rcPixel depth)
{
    init (width, height, depth);
}

rcWindow::rcWindow(const rcIPair& size, rcPixel depth)
{
    rmAssert (size.x() >= 0);
    rmAssert (size.y() >= 0);
    init (size.x(), size.y(), depth);
}

void rcWindow::init(int32 width, int32 height, rcPixel depth)
{
  if ( width <= 0 || height <= 0)
      {
	rmExceptionMacro(<< "IP Window::init");
      }
    
    rcFrameBufPtr ptr = new rcFrame( width, height, depth );
    mFrameBuf = ptr;
    mGeometry = rcRect (0, 0, width, height);
    rmAssert( (width - 1) < mFrameBuf->width() );
    rmAssert( (height - 1) < mFrameBuf->height() );
}

bool rcWindow::contains(const uint8* ptr) const
{
    if (!isBound()) return 0;

    rmAssert(rowUpdate() > 0);
    const uint8 *pels = pelPointer (0,0);
    if ((ptr - pels) < 0) return 0;

    int32 y = (ptr - pels) / rowUpdate();
    int32 x = (ptr - pels) % rowUpdate();

    return ((x >= 0) && (x < width()) && (y >= 0) && (y < height()));
}

bool rcWindow::vImage(vImage_Buffer& vi) const
{
  if (!isBound()) return false;

  rmAssert(rowUpdate() > 0);

  // Get the top left pixel. vImage data pointer is a void *
  const uint8 *pels = pelPointer (0,0);
  vi.data = (void *) pels;
  vi.width = (uint32) width();
  vi.height = (uint32) height ();
  vi.rowBytes = (uint32) rowUpdate ();
  return true;
}



bool rcWindow::window( const rcWindow& parentWindow, int32 x, int32 y, int32 width, int32 height, bool clip )
{
  if (x < 0 || y < 0 || !parentWindow.isBound())
    rmExceptionMacro ("Window Geometry Error");

  *this = rcWindow (parentWindow);
  rcIRect frm = mGeometry;
  mGeometry = rcIRect (parentWindow.x() + x, parentWindow.y() + y, width, height);

  bool what (false);
  if (!frm.contains (mGeometry))
    {
      if (!clip) 
	rmExceptionMacro (<< "Window Geometry Error");
      what = true;
      mGeometry &= frm;
      if (!frm.contains (mGeometry))
	mGeometry.origin (rcIPair (0,0));
    }
  return what;
}


rcWindow::rcWindow(const rcWindow& parentWindow, const rcIPair& icenter, const rcIPair& span, bool& withIn)
{
  bool what = window (parentWindow, icenter.x() - span.x(), icenter.y() - span.y() , 
		      span.x()+span.x()+1, span.y()+span.y()+1, false);
  withIn = !what;
}

rcWindow::rcWindow( const rcWindow& parentWindow, int32 x, int32 y, int32 width, int32 height, bool clip )
{
  bool what = window (parentWindow, x, y, width, height, clip);
  rmUnused (what);
}

rcWindow::rcWindow( const rcWindow& parentWindow, const rcRect& cropRect, bool clip) 
{
  bool what = window (parentWindow, cropRect.origin().x(), cropRect.origin().y(), 
		      cropRect.width(), cropRect.height(), clip);
  rmUnused (what);
}

rcWindow::rcWindow( const rcWindow& parentWindow, int32 width, int32 height )
{
  bool what = window (parentWindow, 0, 0, width, height, false);
  rmUnused (what);
}


rcWindow::rcWindow( const rcWindow& parentWindow, const rcIRect& cropRect, int32& isInside) 
{
  bool what = window (parentWindow, cropRect.origin().x(), cropRect.origin().y(), 
		      cropRect.width(), cropRect.height(), false);
  isInside = (int32) !what;
}

rcWindow& rcWindow::windowRelativeTo( const rcWindow& parentWindow, int32 x, int32 y, int32 width,
                                      int32 height )
{
  bool what = window (parentWindow, x, y, width, height, false);
  rmUnused (what);
  return *this;
}


rcWindow& rcWindow::trim (uint32 delta)
{
  windowRelativeTo (*this, delta , delta , width() - 2*delta, height() - 2*delta);
  return *this;
}

bool rcWindow::trim (int32 delta)
{
  return trim (rcIPair (delta, delta));
}

bool rcWindow::trim (rcIPair delta)
{
    int32 x, y, w, h;
    bool flag = false;

    x = mGeometry.x() + delta.x();
    y = mGeometry.y() + delta.y();
    w = mGeometry.width() - 2 * delta.x();
    h = mGeometry.height() - 2 * delta.y();

    if (x >= 0 && w > 0 && (x+w-1) < (mFrameBuf->width()) &&
        h > 0 && y >= 0 && (y+h-1) < (mFrameBuf->height()))
    {
        mGeometry.origin(rcIPair (x,y));
        mGeometry.size(rcIPair (w,h));
        flag = true;
    }
    return flag;
}

void rcWindow::maxTrim (rcIPair& tl, rcIPair& br) const
{
  tl = position ();
  br = frame().size()  - size();
}

int32 rcWindow::maxTrim () const
{
  rcIPair tl, br;
  maxTrim (tl, br);
  return rmMin (rmMin (tl.x(), tl.y()), rmMin (br.x(), br.y()));
}

bool rcWindow::canTrim (int32 delta) const
{
    int32 x, y, w, h;
    bool flag = false;

    x = mGeometry.x() + delta;
    y = mGeometry.y() + delta;
    w = mGeometry.width() - 2 * delta;
    h = mGeometry.height() - 2 * delta;

    flag = (x >= 0 && w > 0 && (x+w-1) < (mFrameBuf->width()) &&
	    h > 0 && y >= 0 && (y+h-1) < (mFrameBuf->height()));
    return flag;
}
  
rcWindow& rcWindow::copyPixelsFromWindow(const rcWindow& srcWindow, bool mirror)
{
  if (srcWindow.depth() != depth())
    rmExceptionMacro(<< "IP Window::mismatch");    
    
    const int32 rowCount = rmMin(height(), srcWindow.height());
    const int32 rowLth = rmMin(width(), srcWindow.width()) * depth();

    // Note: lastRow is not rowCount - 1 but height() - 1. This does not matter if the
    // images are the same size or src is bigger but when the destination is larger 
    // it copies from height() - srcWindow.height(). 

    if ( mirror ) {
        // Mirror vertically
        const int32 lastRow = height()-1;
        for (int32 row = 0; row < rowCount; row++)
            bcopy(srcWindow.rowPointer(row), rowPointer(lastRow-row), rowLth);
    } else {
        for (int32 row = 0; row < rowCount; row++)
            bcopy(srcWindow.rowPointer(row), rowPointer(row), rowLth);
    }
    
    return *this;
}

// Mirror image data vertically; first row becomes last row
void rcWindow::mirror()
{
    const int32 mid = height()/2;
    const int32 lastRow = height()-1;
    const int32 rowLen = width() * depth();
    uint8 tmp[rowLen];
    
    for (int32 row = 0; row < mid; row++) {
        // Swap two rows
        bcopy( rowPointer(row), tmp, rowLen );
        bcopy( rowPointer(lastRow-row), rowPointer(row), rowLen );
        bcopy( tmp, rowPointer(lastRow-row), rowLen );
    }
}

rcWindow& rcWindow::set(double pixelValue)
{
  switch (depth())
    {

    case rcPixelDouble:
      return setAllDoublePixels (pixelValue);

    case rcPixel8:
    case rcPixel16:
      return setAllPixels ((uint32) pixelValue);

    case rcPixel32S:
      if (!isD32Float ())
	return setAllPixels ((uint32) pixelValue);	
      else
      {
      int32 lastRow = height() - 1, row = 0;
      const int32 unrollCount = 8;
      int32 unrollCnt = width() / unrollCount;
      int32 unrollRem = width() % unrollCount;

            for ( ; row <= lastRow; row++)
            {
                float* pixelPtr = (float *) rowPointer(row);
                
                for (int32 copyCount = 0; copyCount < unrollCnt; copyCount++)
                {
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                }
                
                for (int32 copyCount = 0; copyCount < unrollRem; copyCount++)
                    *pixelPtr++ = pixelValue;
            }
      }
      return *this;
    default:
      rmExceptionMacro(<< "IP Window::not implemented");      
    }
}


rcWindow& rcWindow::setAllPixels(uint32 pixelValue)
{
    int32 lastRow = height() - 1, row = 0;
    const int32 unrollCount = 8;
    int32 unrollCnt = width() / unrollCount;
    int32 unrollRem = width() % unrollCount;

    switch (depth())
    {
        case rcPixel8:
            for ( ; row <= lastRow; row++)
            {
                memset(rowPointer(row), pixelValue, width());
            }
            break;
            
        case rcPixel16:
            
            for ( ; row <= lastRow; row++)
            {
                uint16 valueAsShort = pixelValue;
                uint16* pixelPtr = (uint16*) rowPointer(row);
                
                for (int32 copyCount = 0; copyCount < unrollCnt; copyCount++)
                {
                    *pixelPtr++ = valueAsShort;
                    *pixelPtr++ = valueAsShort;
                    *pixelPtr++ = valueAsShort;
                    *pixelPtr++ = valueAsShort;
                    *pixelPtr++ = valueAsShort;
                    *pixelPtr++ = valueAsShort;
                    *pixelPtr++ = valueAsShort;
                    *pixelPtr++ = valueAsShort;
                }
                
                for (int32 copyCount = 0; copyCount < unrollRem; copyCount++)
                    *pixelPtr++ = valueAsShort;
            }
            break;
            
        case rcPixel32S:            
            for ( ; row <= lastRow; row++)
            {
                uint32* pixelPtr = (uint32*) rowPointer(row);
                
                for (int32 copyCount = 0; copyCount < unrollCnt; copyCount++)
                {
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                    *pixelPtr++ = pixelValue;
                }
                
                for (int32 copyCount = 0; copyCount < unrollRem; copyCount++)
                    *pixelPtr++ = pixelValue;
            }
            break;
            
        
        default:
            rmAssert(0);
    }

    return *this;
}

rcWindow& rcWindow::setAllDoublePixels(double pixelValue)
{
    int32 lastRow = height() - 1, row = 0;
    const uint32 unrollCount = 8;
    int32 unrollCnt = width() / unrollCount;
    int32 unrollRem = width() % unrollCount;

    for ( ; row <= lastRow; row++)
    {
        double* pixelPtr = (double *) rowPointer(row);

        for (int32 copyCount = 0; copyCount < unrollCnt; copyCount++)
        {
            *pixelPtr++ = pixelValue;
            *pixelPtr++ = pixelValue;
            *pixelPtr++ = pixelValue;
            *pixelPtr++ = pixelValue;
            *pixelPtr++ = pixelValue;
            *pixelPtr++ = pixelValue;
            *pixelPtr++ = pixelValue;
            *pixelPtr++ = pixelValue;
        }

        for (int32 copyCount = 0; copyCount < unrollRem; copyCount++)
            *pixelPtr++ = pixelValue;
    }

    return *this;
}

// Using the same seed produces identical "random" results
// Seed value 0 is special, it forces the generation of a new seed
uint32 rcWindow::randomFill( uint32 seed )
{
    if ( seed == 0 ) {
        // Set a seed from two clocks
        const rcTimestamp now = rcTimestamp::now();
        const uint32 secs = uint32(now.secs());
        const float s = (now.secs() - secs) / getTimestampResolution();
        seed = uint32( s + clock() );
    }
    
    // TODO: is this thread-safe? Probably not
    srandom( seed );

    if (depth() == rcPixel8)
        for (int32 j = 0; j < height(); j++) {
            uint8 *oneRow = rowPointer (j);
            for (int32 i = 0; i < width(); ++i, ++oneRow)
                *oneRow = uint8(random ());
        }
    else if (depth() == rcPixel16)
        for (int32 j = 0; j < height(); j++)
            for (int32 i = 0; i < width(); i++)
                setPixel (i, j, uint32 (uint16 (random ())));
    else if (depth() == rcPixel32S)
        for (int32 j = 0; j < height(); j++)
            for (int32 i = 0; i < width(); i++)
                setPixel (i, j, uint32 (random ()));
    else if (depth() == rcPixelDouble)
        for (int32 j = 0; j < height(); j++)
            for (int32 i = 0; i < width(); i++)
                setDoublePixel (i, j, random ());
      
    return seed;
}

double rcWindow::operator() (double xDb, double yDb) const
{
  return mFrameBuf->operator () (xDb + mGeometry.x(), yDb + mGeometry.y());
}


uint32 rcWindow::getPixel(const rcIPair& where) const
{
  return getPixel (where.x(), where.y());
}

double rcWindow::getDoublePixel(const rcIPair& where) const
{
  return getDoublePixel (where.x(), where.y());
}

uint32 rcWindow::setPixel(const rcIPair& where, uint32 value )
{
  return setPixel (where.x(), where.y(), value);
}

double rcWindow::setDoublePixel(const rcIPair& where, double value)
{
  return setDoublePixel (where.x(), where.y(), value);
}


ostream& operator<< (ostream& o, const rcWindow& w)
{
  o << "Size:      " << w.width() << "," << w.height() << endl;
  o << "RowUpdate: " << w.rowUpdate() << endl;
  for (int32 j = 0; j < w.height(); j++)
    {							
      o << "[" << setw (3) << j << "]";	  
      for (int32 i = 0; i < w.width(); i++)
	{
	  if (w.depth() != 8)  
	    o <<  setw(3) << (int32) (w.getPixel (i, j));
	  else o <<   fixed << setprecision(2) << w.getDoublePixel (i, j);
	} 
      o << endl;
    }
  return o;
}

bool rcWindow::operator== (const rcWindow& other) const
{
  return (mFrameBuf == other.mFrameBuf && mGeometry == other.mGeometry);
}

bool rcWindow::operator!= (const rcWindow& other) const
{
  return (mFrameBuf != other.mFrameBuf || mGeometry != other.mGeometry);
}

int32 rcWindow::bits () const
{
  return (bytes () * 8);
}

int32 rcWindow::bytes () const
{
    return (get_bytes().count (depth()));
}

bool rcWindow::canPeekOutSide (const rcIPair range) const
{
  if (range.x() < 0 || range.y() < 0)
    rmExceptionMacro (<< "Window Geometry Error");    

  // Try growing it to see if we can use the frameBuf
  rcRect expanded = mGeometry.trim (-range.x(), -range.x(), -range.y(), -range.y());
  return contains (expanded);
}

void rcWindow::entire () 
{
  mGeometry = rcRect (0, 0, mFrameBuf->width(), mFrameBuf->height());
  rmAssert( mGeometry.x() < mFrameBuf->width() );
  rmAssert( mGeometry.y() < mFrameBuf->height() );
}

// Translate this window if you can
bool rcWindow::translate (const rcIPair& delta)
  {
    int32 x = mGeometry.x() + delta.x();
    int32 y = mGeometry.y() + delta.y();
        
    if (x >= 0 && (x+width()-1) < mFrameBuf->width() &&
	y >= 0 && (y+height()-1) < mFrameBuf->height())
      {
	mGeometry.origin(rcIPair (x,y));
	return true;
      } 

    return false;
  }

uint32 rcWindow::setPixel( int32 x, int32 y, uint32 value )
 { 
  if (x < 0 || y < 0 || !isBound())
    rmExceptionMacro ("Window Geometry Error");
   return mFrameBuf->setPixel (x + mGeometry.x (), y + mGeometry.y (), value);
 }

double rcWindow::setDoublePixel( int32 x, int32 y, double value )
  { 
  if (x < 0 || y < 0 || !isBound())
    rmExceptionMacro ("Window Geometry Error");
    return mFrameBuf->setDoublePixel (x + mGeometry.x (), y + mGeometry.y (), value);
  }

 // return true if this window contains the other and has the same frame buf
bool rcWindow::contains (const rcWindow& other) const
{ 
  return (isBound() && other.isBound() && mFrameBuf == other.mFrameBuf && mGeometry.contains (other.rectangle()));
}

uint32 rcWindow::getPixel( int32 x, int32 y ) const
  { 
  if (x < 0 || y < 0 || !isBound())
    rmExceptionMacro ("Window Geometry Error");
    return mFrameBuf->getPixel (x + mGeometry.x (), y + mGeometry.y ());
  }

double rcWindow::getDoublePixel( int32 x, int32 y ) const
  { 
  if (x < 0 || y < 0 || !isBound())
    rmExceptionMacro ("Window Geometry Error");
    double r = mFrameBuf->getDoublePixel (x + mGeometry.x (), y + mGeometry.y ());
    return r;
  }


#define MASKEDPIXELCOMPARE(depth,pixelTypePtr,maskTypePtr)	\
  case depth:							\
    {\
      int32 w = width();\
      int32 h = height();\
      for (int32 y = 0 ; y < h ; y++)\
	{\
	  const pixelTypePtr pSrc1 = (pixelTypePtr) rowPointer(y);	\
	  const pixelTypePtr pOther = (pixelTypePtr) other.rowPointer(y); \
	  const maskTypePtr pMask = (maskTypePtr) mask.rowPointer(y);	\
	  for (int32 x = 0; x < w; x++, pMask++,pSrc1++, pOther++) \
	    {							     \
	      if (*pMask) continue;				     \
	      if (!(*pSrc1 == *pOther))	\
		{						\
		  cerr << x << "," << y << ":" << getPixel(x,y) << "=" << other.getPixel(x,y) << endl; \
		  return 0;						\
		}							\
	    }\
	}\
      return 1;\
    }

#define PIXELEQUAL(depth,pixelTypePtr,Epsilon)	\
  case depth:				\
    {\
      int32 w = width();\
      int32 h = height();\
      for (int32 y = 0 ; y < h ; y++)\
	{\
	  const pixelTypePtr pSrc1 = (pixelTypePtr) rowPointer(y);	\
	  const pixelTypePtr pOther = (pixelTypePtr) other.rowPointer(y); \
	  int32 x = w;\
	  while (x--)\
	    {\
	      if (!real_equal (*pSrc1++,*pOther++,Epsilon))	\
		{cerr << x << "," << y << " " << pSrc1[-1] << " ?? " << pOther[-1] << endl;\
		  return 0;}						\
	    }\
	}\
      return 1;\
    }

#define PIXELCOMPARE(depth,pixelTypePtr)\
  case depth:				\
    {\
      int32 w = width();\
      int32 h = height();\
      for (int32 y = 0 ; y < h ; y++)\
	{\
	  const pixelTypePtr pSrc1 = (pixelTypePtr) rowPointer(y);	\
	  const pixelTypePtr pOther = (pixelTypePtr) other.rowPointer(y); \
	  int32 x = w;\
	  while (x--)\
	    {\
	      if (!(*pSrc1++ == *pOther++))\
		return 0;\
	    }\
	}\
      return 1;\
    }

#define PIXELCOMPAREVERBOSE(depth,pixelTypePtr)\
	case depth:				\
	{\
		int32 w = width();\
		int32 h = height();\
		for (int32 y = 0 ; y < h ; y++)\
	{\
		const pixelTypePtr pSrc1 = (pixelTypePtr) rowPointer(y);	\
		const pixelTypePtr pOther = (pixelTypePtr) other.rowPointer(y); \
		int32 x = w;\
		while (x--)\
		{\
			if (!(*pSrc1++ == *pOther++))\
			{\
				cerr << "[" << x - 1 << "," << y << "]: " << pSrc1[-1] << endl;\
				return 0;\
				}\
			}\
		}\
		return 1;\
	}

	bool rcWindow::contentCompare (const rcWindow& other, bool verbose) const
	{
		if (!isBound()) return false;
		if (!other.isBound()) return false;
		if (depth() != other.depth()) return false;
		if (size() != other.size()) return false;

		if (isD32Float() && other.isD32Float())
		{
			switch (depth())
			{
				PIXELEQUAL(rcPixel32S, float*, 0.1f);
				default:
				rmExceptionMacro ("Pixel Type Not Implemented");
			}
		}

		if (! verbose)
		{
			switch (depth())
			{
				PIXELCOMPARE(rcPixel8, uint8*);
				PIXELCOMPARE(rcPixel16, uint16*);
				PIXELCOMPARE(rcPixel32S, uint32*);
				default:
				rmExceptionMacro ("Pixel Type Not Implemented");
			}
		}
		else
		{
				switch (depth())
				{
					PIXELCOMPAREVERBOSE(rcPixel8, uint8*);
					PIXELCOMPAREVERBOSE(rcPixel16, uint16*);
					PIXELCOMPAREVERBOSE(rcPixel32S, uint32*);
					default:
					rmExceptionMacro ("Pixel Type Not Implemented");
				}
		}
	}

bool rcWindow::contentMaskCompare (const rcWindow& other, const rcWindow& mask) const
{
  if (!isBound()) return false;
  if (!other.isBound()) return false;
  if (!mask.isBound()) return false;
  if (depth() != other.depth()) return false;
  if (size() != other.size()) return false;
  if (size() != mask.size()) return false;
  rmAssert (mask.depth() == rcPixel8);

  switch (depth())
    {
      MASKEDPIXELCOMPARE(rcPixel8, uint8*,uint8*);
      MASKEDPIXELCOMPARE(rcPixel16, uint16*,uint8*);
      MASKEDPIXELCOMPARE(rcPixel32S, uint32*,uint8*);
    default:
    rmExceptionMacro ("Pixel Type Not Implemented");

    }
}

template<class T>
void rf_SetWindowBorder(rcWindow& win, T val)
{
  const int32 width(win.width());
  const int32 height(win.height());
  rcWindow top (win, 0, 0, width, 1);
  rcWindow bot (win, 0, height-1, width, 1);
  top.set ((double) val);
  bot.set ((double) val);
  for(int32 i = 1; i < height - 1; i++)
    {
      T *p = (T*) win.rowPointer (i);
      *p = val;
      *(p + width - 1) = val;
    }
}

#define SETPIXELBORDER(depth,pixelType) \
    case depth:\
      rf_SetWindowBorder (win, (pixelType) val);\
      return

void rfSetWindowBorder (rcWindow& win, double val)
{
  switch (win.depth())
    {
      SETPIXELBORDER(rcPixel8,uint8);
      SETPIXELBORDER(rcPixel16,uint16);
      SETPIXELBORDER(rcPixelDouble,double);
    case rcPixel32S:
      if (!win.isD32Float())
	rf_SetWindowBorder (win, (uint32) val);
      else
	rf_SetWindowBorder (win, (float) val);
      return;
    default:
      rmExceptionMacro(<< "IP Window::not implemented");      
    }
}      

#if 0
/**
 * Returns CGImage rep of the window
 * @returns CGImageRef for the new image. (Remember to release it when finished with it.) 
 *@note CGBitmapInfo bmpInfo = imageCopy->hasAlphaBuffer() ? kCGImageAlphaFirst : kCGImageAlphaNoneSkipFirst;
 */
CGImageRef rcWindow::CGImage() const
{
	CGDataProviderRef provider = NULL;
	rcWindow root (frameBuf(), 0, 0, frameBuf()->width(), frameBuf()->height());
	CGColorSpaceRef colorSpace;
	CGImageRef rootImage, imageWin;
	CGBitmapInfo alphaInfo = kCGImageAlphaFirst;
	void  *bitmapData = (void *) root.rowPointer (0);
	int    bitmapBytesPerRow = ( root.rowUpdate ());
	
	colorSpace = depth () == rcPixel32S ? CGColorSpaceCreateDeviceRGB () : CGColorSpaceCreateDeviceGray();
	
	// Create a data provider with a pointer to the memory bits
	provider = CGDataProviderCreateWithData(NULL, bitmapData, bitmapBytesPerRow * root.height(), NULL);
	if (NULL == provider)
		rmExceptionMacro(<< "rcWindow::CGImage exporter not available ");
	
	
	// Create the root image
	rootImage = CGImageCreate(root.width(), root.height (), 8, root.bits (),bitmapBytesPerRow, colorSpace, alphaInfo, provider, 
												NULL, false, kCGRenderingIntentDefault);
	
	// rcWindows are refCounted ectangular regions in the root image
	float x = (float) position().x(); float y =  (float) position().y();
	float w = (float) width(); float h =  (float) height ();
	
	imageWin = CGImageCreateWithImageInRect (rootImage, CGRectMake (x, y, w, h));
	
	CGColorSpaceRelease (colorSpace);
    CGDataProviderRelease (provider);
	
    rmAssert (imageWin);
	return imageWin;

}
#endif

