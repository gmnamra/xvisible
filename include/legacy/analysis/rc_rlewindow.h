/*
 *  @file rc_rleWindow.h
 *  
 *  $Id: rc_rlewindow.h 7297 2011-03-07 00:17:55Z arman $
 *  Created by Arman Garakani on Tue Jun 04 2002.
 *  Copyright (c) 2002 Reify Corp. . All rights reserved.
 *
 */

#ifndef _RC_RLE_WINDOW_H_
#define _RC_RLE_WINDOW_H_

#if 0

#include <rc_window.h>
#include <rc_point.h>
#include <rc_rectangle.h>
#include <rc_vector2d.h>
#include <rc_graphics.h>
#include <vector>
#include <iostream>

using namespace std;

//#define RC_DEBUG_ALLOC 1

/* @note add label and make runs have int32 value and lengths

   Run length encoded (RLE) representation of an image.  A run is a
   group of horizontally adjacent pixels that have the same value. 
   For support in region labeling, runs are generated from the 32 bit
   region image but correspond to the uint8 image.

   For images whose RLE representation has longish runs of constant
   pixel values, the number of runs needed to represent the image is
   much smaller than the number of pixels in the image.  For such
   images, many operations may be performed more efficiently on images
   in RLE form.

   Some operations on RLE images are implemented as member functions of
   rcRleWindow.  

*/


// Forward declarations

class rcPolygon;
class rcRleWindow;


struct rcRleBuf;


struct rcRLEBuf
{
  rcRLEBuf() : width_(0)
#ifdef notyet    
	, binary_(0)
#endif
  { }
	
  /* default copy, assign, dtor OK */
	
  int32 width_;
  vector<rcRleWindow::Run *> runTable_; // allocated to one entry per row
  vector<rcRleWindow::Run *> allocs_;
#ifdef notyet    
  bool binary_;
#endif    
};


#define OUT_RUN(runs)				\
while(lastLength > rcUINT16_MAX)	\
{							        \
runs->value = lastValue;			\
runs++->length = rcUINT16_MAX;	\
lastLength -= rcUINT16_MAX;		\
}							        \
if(lastLength > 0)					\
runs->value = lastValue,  runs++->length = (uint16)lastLength


struct rcRleRun
{
   uint16 value;
   uint16 length;
};

// The underlying run holder. Notice there is no depth implications.


// a rcRleWindow is the image-relative definition of a group of runs

class rcRleWindow
{
public:

  typedef rcRleRun Run;

  rcRleWindow(): mRep(0), mMaxPixelValue( 0 ) {};

  /*
    effect     Constructs a "degenerate" RLE image.  Most operations
               are forbidden on default-constructed RLE images.  See
	       encode().
  */

  ~rcRleWindow();

  rcRleWindow(const rcRleWindow&);
  rcRleWindow& operator= (const rcRleWindow&);
  /*
    effect     Copies the RLE buffer.  No sharing occurs among copies.
    note       The createTime() is set according to how long the copy
               operation takes.
  */

  bool isBound () const;
  /* effect    Checks to see if this rleWindow is bound to runs
  */

  uint32 maxPixelValue() const;
  /* effect    Returns maximum pixel value in this RLE
   */
  
  rcWindow image() const;
  void image(rcWindow&) const;
 
  /*
    effect     Makes a pixel image from this RLE image. Image depth is chosen to
               accommodate maximum pixel value.
               Takes time proportional to number of pels plus number of runs.

               The second form requires an unbound rcWindow.
  */

  void polygon(rcPolygon&) const;
  /* effect    Generate a polygon from this rle.
   *
   * requires  Only 1 connected region
   */

  rcIRect rectangle() const;
  /* effect    Returns bounding rectangle of this rle
   */
  
  void rectangle(const rcIRect& r)
  {
    rmAssert (isBound());
    rmAssert (r.width() == width() && r.height() == height());
    mRect = r;
  }
  /*
    effect     Stores the rect (x, y, w, h) corresponding to this rle

  */

  const rc2Fvector& origin() const;
  void origin (rc2Fvector& orig);
  /*
    effect     Stores/Gets a user defined origin for this RLE

  */

  const rc2Fvector& centerOfMass() const;
  /*
    effect     Calculates center of mass of this RLE
    Requires:  horizonatal and vertical projections

  */

  const rc2Fvector medianCenter() const;
  /*
    effect     Returns the Median point. Calculates everytime.
    Requires:  horizonatal and vertical projections

  */

  int32 width() const;
  int32 height() const;
  /*
    effect     Returns dimensions of this RLE image.
  */

  const Run* const * pointToRat() const;
  /*
    effect     Returns a pointer to the row address table of the
    	       RLE buffer. This pointer can be considered the base
	       of an array where the ith element of the array points
	       to the first run of the ith row.
	       All rows have at least two Run structs (every row
	       has at least one run, and each row is terminated by a
	       0-length Run).
    note       Read-only access to runs.
	       Returned run pointers are good only until any non-const
	       operation is performed on this RLE buffer.
  */

  const Run* pointToRow(int32 rowNum) const;
  /*
    effect     Returns a pointer to the first run of the indicated
               row.  All of the runs of a given row are contiguously
	       allocated (Run array).  Between rows, Run allocation
	       can have discontinuities if isContiguous() is false; if
	       isContiguous() is true, then all runs are allocated
	       from one buffer.
	       All rows have at least two Run structs (every row
	       has at least one run, and each row is terminated by a
	       0-length Run).
    note       Read-only access to runs.
	       Returned run pointers are good only until any non-const
	       operation is performed on this RLE buffer.
    note       The first row of runs should be accessed as
	       this->pointToRow(this->offset().y()).
  */

  uint16 getPel(int32 x, int32 y) const;
  /*
    effect     Returns the pixel value at the indicated image
               coordinate.
    note       Execution time is proportional to the number of runs in
               the row.
    note       The uppermost leftmost pixel should be accessed as
	       this->getPel(this->offset()).
  */

  void encode (const rcWindow& image, uint32 maxPixelValue = rcUINT16_MAX,
	       uint32 maxPixelValue = rcUINT32_MAX);

  void projectHoriz(vector<uint32>&) const;
  void projectVert(vector<uint32>&) const;
  void projectDiag(vector<uint32>&) const;

  /*
    effect     Performs the indicated projection sums.  The argument
               blocks will be sized as follows (all are 0-based):
	         projectHoriz(): sized to height of image
	         projectVert(): sized to width of image
	         projectDiag(): sized to width+height-1 of image

    note       Diagonal projection rays are oriented such that x+y is
               constant along each projection ray.  Projection bin 0
	       corresponds to 0,0. This projection is NOT the same
               as projectAngle at 135 degrees
  */

  uint32 n() const;        // Number of all runs

  double meanLength() const;
  /*
    effect     Return number of runs and average run length over all
               runs in this RLE buffer.
    note       This runs in constant time.

  */

  uint32 n(uint16 val) const;
  double meanLength(uint16 val) const;
  double meanLengthNot(uint16 val) const;
  /*
    effect     Return number of runs and average run length for all
               runs in this RLE buffer with the indicated run value.
	       See also the second overload of histo().

  */

  uint32 setN() const;
  /* effect    Returns the number of non-zero pixels in this RLE buffer.
   */

  bool operator== (const rcRleWindow&) const;
  bool operator!= (const rcRleWindow& rhs) const { return !(*this == rhs); }

  // Vectorize RLE, produce graphics segment collection
  void vectorize( rcVisualSegmentCollection& segments ) const;
  
  // Debugging: returns approximate number of bytes allocated by this instance
  uint32 sizeOf() const;
  // Debugging: returns number of run structure allocations
  uint32 allocCount() const;
  
private:

  const rcRleRun* pointToRowRel(int32 rowNum) const;
  /*
    effect     Same as pointToRow(), except the argument rowNum
               indicates the relative position of the row from the first
               row of runs.
  */

  rcRleRun* allocRuns(int howMany);
  void copy(const rcRleWindow&); 
  int sizeRuns() const;
  void deleteRep();

  // Image generator methods
  void setRow8( rcWindow&, int32 y ) const;
  void setRow16( rcWindow&, int32 y ) const;
  void setRow32( rcWindow&, int32 y ) const;
  
  void image8(rcWindow&) const;
  void image16(rcWindow&) const;
  void image32(rcWindow&) const;

  void contourImage8(rcWindow&) const;
  void contourImage16(rcWindow&) const;
  void contourImage32(rcWindow&) const;

  rcPixel maxPixelDepth() const;
  uint16 getPel(int32 x, const Run* runs) const;
  
   // Build polygon from RLE buffer
  bool internalPolygon(rcPolygon& p, bool fixup) const;
  
  rcRleBuf* mRep;
  rcIRect mRect;
  uint32 mMaxPixelValue; // Maximum pixel value
  rc2Fvector mOrigin; // User defined origin
  rc2Fvector mCom; // Center Of mass
  double mMass;
};

// Debug output
ostream& operator << ( ostream& os, const rcRleWindow& run );


/*	*************************
	*                       *
	*     Inlines           *
	*                       *
	*************************
*/

inline bool rcRleWindow::isBound() const
{ return (mRep != 0) ? true : false; }

inline uint32 rcRleWindow::maxPixelValue() const
{ return mMaxPixelValue; }

inline rcIRect rcRleWindow::rectangle() const
{ return mRect; }

inline const rc2Fvector& rcRleWindow::origin () const
{
  return mOrigin;
}

inline void rcRleWindow::origin (rc2Fvector& org)
{
  mOrigin = org;
}

inline const rc2Fvector& rcRleWindow::centerOfMass () const
{
  return mCom;
}


#endif


#endif // _RC_RLE_WINDOW_H_
