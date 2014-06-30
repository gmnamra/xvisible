
#ifndef __rcRECTANGLE_H
#define __rcRECTANGLE_H

#include "rc_pair.h"
#include "rc_types.h"
#include "rc_vector2d.h"
#include "rc_math.h"
#include "rc_exception.h"
#include <deque>


using namespace std;


// Rectangle Class
// There are 2 uncoverntional operator overloads: & and | for intersect and enclose member functions
//

template <class T>
class rcRectangle
{
public:
  rcRectangle();
  /*
         Makes this rectangle have origin (0,0), with width and
	       height both equal to zero.
  */
  rcRectangle(T w, T h);
  /*
         Makes this rectangle have the indicated width and
	       height.  If w and h are both non-negative, the origin
	       will be (0,0).
    note       If w is negative, the rectangle's origin x-component
	       will be w (negative), and its width will be -w
	       (positive).
	       Similarly, if h is negative, the rectangle's origin
	       y-component will be h (negative), and its height will
	       be -h (positive).
  */
  rcRectangle(T x, T y, T w, T h);
  /*
         Makes this rectangle have the indicated origin, width,
	       and height.
    note       If w is negative, the rectangle's origin x-component
	       will be x+w, and its width will be -w (positive).
	       Similarly, if h is negative, the rectangle's origin
	       y-component will be y+h, and its height will be -h
	       (positive).
  */
  rcRectangle(const rcPair<T>& v1, const rcPair<T>& v2);
  /*
         Makes this rectangle the minimum enclosing rectangle
	       of the indicated two points.
  */


  /* default copy ctor, assignment, dtor OK */

  bool operator== (const rcRectangle<T>&) const;
  bool operator!= (const rcRectangle<T>& rhs) const { return !(*this == rhs); }
  /*
         Two rectanges are equal iff they have the same origin,
	       width, and height.
  */

  const rcPair<T>& origin() const {return mUpperLeft;}
  /*
         Returns the origin of this rectangle.  This is its
	       upper-left corner.
  */
  void origin(const rcPair<T>&);
  /*
         Sets the origin of this rectangle without modifying
	       its width or height.
  */
  void translate(const rcPair<T>&);
  /*
         Moves the origin of this rectangle without modifying
	       its width or height, by adding the indicated vector
	       displacement to this rectangle's origin.
  */

  const rcPair<T>& ul() const {return mUpperLeft;} /* same as origin() */
  rcPair<T> ur() const;
  rcPair<T> ll() const;
  const rcPair<T>& lr() const {return mLowerRight;}
  /*
         Returns the coordinates of the indicated corner of
	       this rectangle.
  */

  rcPair<T> center () const {return ((ul() + lr()) / (T) 2); }
 /*
         Returns the coordinates of the simple center of
	       this rectangle.
  */
  T width() const;
  /*
         Returns the width of this rectangle.
  */
  void width(T w);
  /*
         Sets the width of this rectangle without changing its
               origin.
    note       If w is negative, its value is added to the x
	       component of the rectangle's origin, and the
	       rectangle's width is set to -w (positive).
  */
  T height() const;
  /*
         Returns the height of this rectangle.
  */
  void height(T h);
  /*
         Sets the height of this rectangle without changing its
               origin.
    note       If h is negative, its value is added to the y
	       component of the rectangle's origin, and the
	       rectangle's height is set to -h (positive).
  */

  T area () const;
  /*
         Returns the area: width * height of this rect
  */

  rcPair<T> size() const;
  /*
         Returns the size of this rectangle (the width is the x
	       component of the returned pair; the height is the y
	       component).
  */
  void size(const rcPair<T>&);
  /*
         Sets the width from the x component of the argument,
	       and sets the height from the y component of the
	       argument.
    note       See width(T) and height(T) for a
	       description of how negative size values are handled.
  */

  bool isNull(void) const;
  /*
         Tells if this rectangle is a null rectangle (i.e., height==0
		OR width==0).
  */

  bool overlaps(const rcRectangle<T>&, bool touching=false) const;
  /*
         Tells if the indicated rectangle overlaps this
	       rectangle.
	       If the touching flag is set, rectangles that just
	       barely touch at their borders are considered to
	       overlap.
	       If the flag is not set, rectangles are considered to
	       overlap only if a portion of one rectangle's border is
	       within the other rectangle.
  */

  bool contains(const rcRectangle<T>&) const;
  /*
         Tells if the indicated rectangle is contained within
	       this rectangle.
  */

  bool contains(const rcPair<T>&) const;
  /*
         Tells if the indicated point is contained within
	       this rectangle.
  */

  rcRectangle<T> intersect(const rcRectangle<T>& other,
		      bool throwIfNoOverlap=false) const;
  /*
         Returns a rectangle which is the intersection of the
	       two rectangles.
	       If the two rectangles do not overlap (as indicated
	       by overlaps(other, true)),  and throwIfNoOverlap
	       is false, then a null rectangle is returned; the
	       returned rectangle's origin is undefined; it is the
	       client's responsibility to not use the null rectangle
	       returned by this function for further operations.
    exceptions     general_exception  The two rectangles do not overlap
	          and throwIfNoOverlap is set.
    note       Rectangles that just barely touch will produce a null
	       rcRectangle whose origin is valid.
  */
  rcRectangle operator& (const rcRectangle<T>& other) const;
  /*
         rcRectangle<T>::intersect(other, false).
         Greatest Common Rectangle of several rectangles can be
	       computed as destRect = (rect1 & rect2 & rect3 & rect4 & ...);
	 followed by a check for a null rectangle
  */
  rcRectangle<T>& operator&= (const rcRectangle<T>& other);
  /*
         Modifies this rectangle to be the intersection (operator&)
               of the two rectangles. Returns a reference to this object.
  */

  rcRectangle<T> enclose(const rcRectangle<T>&) const;

  rcRectangle<T> operator| (const rcRectangle<T>&) const;
  /*
         Returns a rectangle which is the minimum enclosing
	       rectangle of the two rectangles.
	       If rcRectangle<T>s A or B are null, (A|B) will be the same as
	       other (non-null) rcRectangle<T>.
	       If rcRectangle<T>s A and B are both null, (A|B) will return a
	       null rcRectangle<T> whose origin is undefined, unless,
	       A.origin()==B.origin().
    note
	       The minimum enclosing rectangle of several rectangles can be
	       computed compactly as
	          destRect = (rect1 | rect2 | rect3 | rect4 | ...);
  */
  rcRectangle<T>& operator|= (const rcRectangle<T>&);
  /*
         Modifies this rectangle to be the minimum enclosing
	       rectangle of the two rectangles.
  */

  rcRectangle<T> trim(T left, T right, T top, T bottom,
                 bool throwIfNegativeSize=false) const;

  rcRectangle<T> trim(T pad, bool throwIfNegativeSize=false) const;

  /*
         Returns a rectangle which is a border adjusted copy of this
               rectangle. The arguments indicate how much the returned
               rectangle is to be shrunk at each of its borders.
               Negative argument values are permitted and mean
               that the rectangle is to be grown rather than shrunk
               at the indicated border.
               If left+right > width() or top+bottom > height(),
               and throwIfNegativeSize is false, then negative size
               values are handled as described in width(T)
               and height(T).
    exceptions     general_exception.  The trimming requested results
                  in negative size values for rectangle and
                  throwIfNegativeSize is set.
  */

  rcRectangle<T> transpose () const;
  /*
         Returns the transpose of this object. The transpose
	       operation is defined as follows:
	       If two rcRectangle<T>s A and B are related by A = B.transpose(), then
		   	A.origin().x() = B.origin.y();
		   	A.origin().y() = B.origin.x();
		   	A.height()     = B.width();
		   	A.width()      = B.height();
  */

  friend ostream& operator<< (ostream& ous, const rcRectangle<T>& dis)
  {
    ous << dis.ul() << "," << dis.lr();
    return ous;
  }


protected:
  void normalize();		/* fix negative width and/or height */

  rcPair<T> mUpperLeft;
  rcPair<T> mLowerRight;
};

template<class T>
inline rcPair<T> rcRectangle<T>::ur() const
{
  return rcPair<T>(mLowerRight.x(), mUpperLeft.y());
}

template<class T>
inline rcPair<T> rcRectangle<T>::ll() const
{
  return rcPair<T>(mUpperLeft.x(), mLowerRight.y());
}

template<class T>
inline T rcRectangle<T>::width() const
{
  return mLowerRight.x() - mUpperLeft.x();
}

template<class T>
inline T rcRectangle<T>::height() const
{
  return mLowerRight.y() - mUpperLeft.y();
}

template<class T>
inline rcPair<T> rcRectangle<T>::size() const
{
  return rcPair<T>(width(), height());
}

template<class T>
inline T rcRectangle<T>::area() const
{
  return width() * height();
}

template<class T>
inline bool rcRectangle<T>::isNull(void) const
{
  return (width()==0 || height()==0);
}

template<class T>
inline rcRectangle<T> rcRectangle<T>::operator& (const rcRectangle<T>& other) const
{
  return intersect(other, false);
}

template<class T>
inline rcRectangle<T> rcRectangle<T>::operator| (const rcRectangle<T>& other) const
{
  return enclose(other);
}

typedef rcRectangle<float> rcFRect;
typedef rcRectangle<double> rcDRect;
typedef rcRectangle<int32> rcIRect;
typedef rcRectangle<uint32> rcUIRect;


template<class T>
rcRectangle<T>::rcRectangle()
  : mUpperLeft(0, 0), mLowerRight(0, 0)
{
}


// We always call normalize. Normalize for unsigned type does nothing. This is a bit inefficient.
template<class T>
rcRectangle<T>::rcRectangle(T w, T h)
  : mUpperLeft(0, 0), mLowerRight(w, h)
{

    normalize();
}

template<class T>
rcRectangle<T>::rcRectangle(T x, T y, T w, T h)
  : mUpperLeft(x, y), mLowerRight(x+w, y+h)
{
    normalize();
}

template<class T>
rcRectangle<T>::rcRectangle(const rcPair<T>& v1, const rcPair<T>& v2)
  : mUpperLeft(rmMin(v1.x(),v2.x()), rmMin(v1.y(),v2.y()) ),
    mLowerRight(rmMax(v1.x(),v2.x()), rmMax(v1.y(),v2.y()))
{
}

template<class T>
void rcRectangle<T>::normalize()
{
  T w = width();
  T h = height();

  if (w<0 || h<0) {
    T x = mUpperLeft.x();
    T y = mUpperLeft.y();
    if (w < 0)
    {
      x += w;
      w = -w;
    }
    if (h < 0)
    {
      y += h;
      h = -h;
    }

    mUpperLeft = rcPair<T>(x, y);
    mLowerRight = rcPair<T>(x+w, y+h);
  }
}

template<class T>
bool rcRectangle<T>::operator== (const rcRectangle<T>& rhs) const
{
  return (mUpperLeft.x() == rhs.mUpperLeft.x()) &&
         (mUpperLeft.y() == rhs.mUpperLeft.y()) &&
         (mLowerRight.x() == rhs.mLowerRight.x()) &&
         (mLowerRight.y() == rhs.mLowerRight.y());
}

template<class T>
void rcRectangle<T>::origin(const rcPair<T>& newOrigin)
{
  rcPair<T> temp(newOrigin.x()-mUpperLeft.x(),
                 newOrigin.y()-mUpperLeft.y());
  mUpperLeft = newOrigin;
  mLowerRight.x() += temp.x();
  mLowerRight.y() += temp.y();
}
template<class T>
void rcRectangle<T>::translate(const rcPair<T>& disp)
{
  mUpperLeft.x() += disp.x();
  mUpperLeft.y() += disp.y();
  mLowerRight.x() += disp.x();
  mLowerRight.y() += disp.y();
}

template<class T>
void rcRectangle<T>::width(T newWidth)
{
  mLowerRight.x() = mUpperLeft.x() + newWidth;
  normalize();
}

template<class T>
void rcRectangle<T>::height(T newHeight)
{
  mLowerRight.y() = mUpperLeft.y() + newHeight;
  normalize();
}

template<class T>
void rcRectangle<T>::size(const rcPair<T>& newSize)
{
  mLowerRight.x() = mUpperLeft.x() + newSize.x();
  mLowerRight.y() = mUpperLeft.y() + newSize.y();
  normalize();
}

template<class T>
bool rcRectangle<T>::overlaps(const rcRectangle<T>& other, bool touching) const
{
  rcPair<T> overlapUpperLeft(rmMax(mUpperLeft.x(),other.mUpperLeft.x()),
                           rmMax(mUpperLeft.y(),other.mUpperLeft.y()));
  rcPair<T> overlapLowerRight(rmMin(mLowerRight.x(),other.mLowerRight.x()),
                           rmMin(mLowerRight.y(),other.mLowerRight.y()));

  if (touching)
  {
    return (overlapUpperLeft.x() <= overlapLowerRight.x()) &&
	(overlapUpperLeft.y() <= overlapLowerRight.y());
  }
  else
  {
    /* must actually overlap */
    return (overlapUpperLeft.x() < overlapLowerRight.x()) &&
	(overlapUpperLeft.y() < overlapLowerRight.y());
  }
}

template<class T>
bool rcRectangle<T>::contains(const rcRectangle<T>& other) const
{
  return (other.ul().x() >= ul().x()) &&
      (other.lr().x() <= lr().x()) &&
      (other.ul().y() >= ul().y()) &&
      (other.lr().y() <= lr().y());
}

template<class T>
bool rcRectangle<T>::contains(const rcPair<T>& point) const
{
  return ul().x() <= point.x() && point.x() < lr().x() &&
         ul().y() <= point.y() && point.y() < lr().y();
}

template<class T>
rcRectangle<T> rcRectangle<T>::intersect(const rcRectangle<T>& other,
		               bool throwIfNoOverlap) const
{
  rcPair<T> overlapUpperLeft(rmMax(mUpperLeft.x(),other.mUpperLeft.x()),
                           rmMax(mUpperLeft.y(),other.mUpperLeft.y()));
  rcPair<T> overlapLowerRight(rmMin(mLowerRight.x(),other.mLowerRight.x()),
                           rmMin(mLowerRight.y(),other.mLowerRight.y()));

  if ((overlapUpperLeft.x() > overlapLowerRight.x()) ||
	 (overlapUpperLeft.y() > overlapLowerRight.y()))
    {    
      if (throwIfNoOverlap)
        throw general_exception("rcRectErrors::NoOverlap");
      else
        overlapLowerRight = overlapUpperLeft; // Force a null rectangle
    }

  return rcRectangle<T>(overlapUpperLeft, overlapLowerRight);
}

template<class T>
rcRectangle<T>& rcRectangle<T>::operator&= (const rcRectangle<T>& other)
{
  mUpperLeft.x() = rmMax(mUpperLeft.x(), other.mUpperLeft.x());
  mUpperLeft.y() = rmMax(mUpperLeft.y(), other.mUpperLeft.y());
  mLowerRight.x() = rmMin(mLowerRight.x(),other.mLowerRight.x());
  mLowerRight.y() = rmMin(mLowerRight.y(),other.mLowerRight.y());

  if ((mUpperLeft.x() > mLowerRight.x()) ||
	 (mUpperLeft.y() > mLowerRight.y()))
      mLowerRight = mUpperLeft; // Force a null rectangle
  return *this;
}

template<class T>
rcRectangle<T> rcRectangle<T>::enclose(const rcRectangle<T>& other) const
{
  if (other.isNull())
      return *this;
  if (isNull())
      return other;
  rcPair<T> tempMin(rmMin(mUpperLeft.x(),other.mUpperLeft.x()),
                  rmMin(mUpperLeft.y(),other.mUpperLeft.y()));
  rcPair<T> tempMax(rmMax(mLowerRight.x(),other.mLowerRight.x()),
                  rmMax(mLowerRight.y(),other.mLowerRight.y()));

  return rcRectangle<T>( tempMin, tempMax );
}

template<class T>
rcRectangle<T>& rcRectangle<T>::operator|= (const rcRectangle<T>& other)
{
  if (!other.isNull())
  {
    if (isNull())
        *this = other;
    else {
        mUpperLeft.x() = rmMin(mUpperLeft.x(), other.mUpperLeft.x());
        mUpperLeft.y() = rmMin(mUpperLeft.y(), other.mUpperLeft.y());
        mLowerRight.x() = rmMax(mLowerRight.x(), other.mLowerRight.x());
        mLowerRight.y() = rmMax(mLowerRight.y(), other.mLowerRight.y());
    }
  }

  return *this;
}


template<class T>
rcRectangle<T> rcRectangle<T>::trim(T pad, bool throwIfNegativeSize) const
{
  return trim (pad, pad, pad, pad, throwIfNegativeSize);
}

template<class T>
rcRectangle<T> rcRectangle<T>::trim(T left, T right, T top,
	      T bottom, bool throwIfNegativeSize) const
{
  T newWidth = width() - (left + right);
  T newHeight = height() - (top + bottom);
  if (newWidth < 0 || newHeight < 0)
      if (throwIfNegativeSize)
	throw general_exception("rcRectErrors::NegativeSize");

  return rcRectangle<T>(mUpperLeft + rcPair<T>(left, top),
                   mLowerRight - rcPair<T>(right, bottom));
}

template<class T>
rcRectangle<T> rcRectangle<T>::transpose() const
{
    rcPair<T> transposeUpperLeft(mUpperLeft.y(), mUpperLeft.x());
    rcPair<T> transposeLowerRight(mLowerRight.y(), mLowerRight.x());
    return rcRectangle<T>(transposeUpperLeft, transposeLowerRight);
}

template<class P>
rcIRect rfRoundRectangle (const rcRectangle<P>& fr)
{
  static const int32 dummy (0);

  rcIPair iSize (rfRound (fr.width(), dummy), rfRound (fr.height(), dummy));

  int32 right, left, top, bottom;


  if ((fr.ul().x() - (int32) (fr.ul().x())) < ((int32) fr.lr().x() + P (1) -fr.lr().x()))
    {
      left = (int32) fr.origin().x(); right = left + iSize.x();
    }
  else
    {
      right = (int32) fr.lr().x() +1; left = right - iSize.x();
    }

  if ((fr.ul().y() - (int32) fr.ul().y()) < ((int32) fr.lr().y() + P (1) -fr.lr().y()))
    {
      top = rfRound (fr.ul().y(), dummy); bottom = top + iSize.y();
    }
  else
    {
      bottom = (int32) fr.lr().y() + 1; top = bottom - iSize.y();
    }

  return rcIRect (left, top, right - left, bottom - top);
}


// Specializations of Unsigned rects

template<>
void rcRectangle<uint32>::normalize();
template<>
void rcRectangle<uint16>::normalize();
template<>
void rcRectangle<uint8>::normalize();
template<>
rcRectangle<uint32> rcRectangle<uint32>::trim(uint32 left, uint32 right, uint32 top, uint32 bottom, bool throwIfNegativeSize) const;
template<>
rcRectangle<uint16> rcRectangle<uint16>::trim(uint16 left, uint16 right, uint16 top, uint16 bottom, bool throwIfNegativeSize) const;
template<>
rcRectangle<uint8> rcRectangle<uint8>::trim(uint8 left, uint8 right, uint8 top, uint8 bottom, bool throwIfNegativeSize) const;


// TBD: implement this as a template function of iterators

void rfBoundingRect (const deque<rc2Dvector>& pts, rcDRect& box);


#endif /* __rcRECTANGLE_H */
