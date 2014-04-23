/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.9  2004/09/13 15:04:15  arman
 *added direct inclusion of vector
 *
 *Revision 1.8  2004/05/03 19:15:17  arman
 *fixed a bug in the ctor
 *
 *Revision 1.7  2004/04/28 01:43:48  arman
 *added a new ctor and template implementations
 *
 *Revision 1.6  2004/04/27 21:05:44  arman
 *added rcLineSegment
 *
 *Revision 1.5  2004/04/26 17:20:47  arman
 *added InfLine
 *
 *Revision 1.4  2004/04/09 20:14:15  arman
 *added rcLine
 *
 *Revision 1.3  2003/03/21 18:40:05  sami
 *Use float-based line segment class rcFLineSegment
 *
 *Revision 1.2  2003/03/06 23:10:01  sami
 *Inlining fix
 *
 *Revision 1.1  2003/03/04 18:42:29  arman
 *lineSegment class
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef _rcLINE_H_
#define _rcLINE_H_

#include "rc_xforms.h"
#include "rc_types.h"
#include <vector>


/*        Line Segment
 */

template <class T>
class rcLineSegment
{
public:
  rcLineSegment () :
    mAngle (rcRadian (0.0)), mSin (T (0)), mCos (T (1.0)), mDist (T (0)) {}

  rcLineSegment (const rcRadian& angle, const T distance) :
  mAngle (angle), mSin (sin (angle)),  mCos (cos (angle))
  {
    rmAssert (distance >= T (0));
    mDist = distance;
  }

  rcLineSegment (const rcRadian& angle, const rc2dVector<T>& pb);

  rcLineSegment (const rc2dVector<T>& pa, const rc2dVector<T>& pb);

  T distanceFrom (const rc2dVector<T> & point) const
  { return (point.x() * mSin - point.y() * mCos + mDist); }

  T distanceAlong(const rc2dVector<T>& point) const
  { return (point.x() * mCos + point.y() * mSin); }

  vector<rc2dVector<T> > intersection(const rcLineSegment<T> & line) const;

  rc2dVector<T> closestPoint(const rc2dVector<T>& point) const;

  const rcRadian& angle() const { return mAngle; }

  T distance() const { return mDist; }
  T Sin() const { return mSin; }
  T Cos() const { return mCos; }

  bool operator==(const rcLineSegment<T>& ls) const;

  bool operator!=(const rcLineSegment<T>& ls) const;

 private:
  rcRadian mAngle;
  T mSin, mCos, mDist;

};

template<class T>
bool rcLineSegment<T>::operator==(const rcLineSegment<T>& ls) const
      { return mSin == ls.mSin && mCos == ls.mCos && mDist == ls.mDist && mAngle == ls.mAngle; }

template<class T>
 bool rcLineSegment<T>::operator!=(const rcLineSegment<T>& ls) const
     { return !(*this == ls); }


template<class T>
rcLineSegment<T>::rcLineSegment (const rc2dVector<T>& pa, const rc2dVector<T>& pb)
{
  if (pa == pb)
    throw general_exception ("Degenerate Line");

  mAngle = (pa - pb).angle();
  T sn = sin (- mAngle);
  T cs = cos (- mAngle);
  rc2dVector<T> p (cs*pa.x()-sn*pa.y(),sn*pa.x()+cs*pa.y());
  mDist = p.y();
  if (mDist < 0)
    {
      mDist = - mDist;
      mAngle = rcRadian (rkPI) + mAngle;
    }
  mSin = sin(mAngle);
  mCos = cos(mAngle);
}

template<class T>
rcLineSegment<T>::rcLineSegment (const rcRadian& angle,
				 const rc2dVector<T>& pa)
{

  mAngle = angle;
  T sn = sin (- mAngle);
  T cs = cos (- mAngle);
  rc2dVector<T> p (cs*pa.x()-sn*pa.y(),sn*pa.x()+cs*pa.y());
  mDist = p.y();
  if (mDist < 0)
    {
      mDist = - mDist;
      mAngle = rcRadian (rkPI) + mAngle;
    }
  mSin = sin(mAngle);
  mCos = cos(mAngle);
}

template<class T>
rc2dVector<T> rcLineSegment<T>::closestPoint(const rc2dVector<T> & point) const
{
  // This is the easy, not necessarily the efficient, thing to do:
  vector<rc2dVector<T> > intersecting = intersection(rcLineSegment<T>(angle()+
								      rcRadian(rkPI/2),
								      point));
  rmAssert(intersecting.size() == 1);
  return intersecting[0];
}

/* note: equation for a line segment yCos - xSin = Dist
 *
 */

template<class T>
vector<rc2dVector<T> >
rcLineSegment<T>::intersection(const rcLineSegment<T> & line) const
{
  vector<rc2dVector<T> > pts;
  double denom = line.mCos * mSin - mCos * line.mSin;
  if(denom == 0.0)
    return pts;
  pts.push_back((rc2dVector<T> (mCos,mSin)* line.mDist -
		 rc2dVector<T> (line.mCos,line.mSin) * mDist)/denom);
  return pts;
}


class rcInfLine
{
public:
  rcInfLine() : mDir(1.,0.), mPos(0.,0.) {}
  // effect     Constructs the line with direction (1,0), passing
  //            through the point (0,0).

  rcInfLine(const rc2Dvector& dir, const rc2Dvector& pos);
  // effect     Constructs the line in the direction "dir", passing through
  //            the point "pos".
  // requires   dir must have unit length
  // throws     ccShapesError::DegenerateShape if dir == (0,0).

  rcInfLine(const rcRadian& t, const rc2Dvector& pos);
  // effect     Constructs the line in the direction "t", passing through
  //            the point "pos".

  rcInfLine(const rc2Dvector& v);
  // effect     Constructs the line in the direction v.angle(), passing
  //            through the point v.x(), v.y().
  // throws     ccShapesError::DegenerateShape if v == (0,0).

  const rc2Dvector& dir() const { return mDir; }
  const rc2Dvector& pos() const { return mPos; }
  rc2Dvector& dir() { return mDir; }
  rc2Dvector& pos() { return mPos; }

  rcInfLine transform(const rc2Xform& c) const;
  void transform(const rc2Xform& c, rcInfLine& result) const;
  // returns    transformed line

  double toPoint(const rc2Dvector& v) const;
  // returns    The shortest distance from this line to v.

  rcRadian angle() const { return dir().angle(); }
  // effect     Returns the line angle. -PI < a <= PI

  void angle(rcRadian a) { mDir.x(cos(a)); mDir.y(sin(a)); }
  // effect     Set Direction

  rcInfLine parallel(const rc2Dvector&) const;
  // returns    Line through the specified point that is parallel to me

  bool isParallel(const rcInfLine& l) const;
  // returns    true if it is

  rcRadian angle (const rcInfLine&) const;
  // returns    Angle from me to this line. -PI <  <= PI

  rc2Dvector intersect(const rcInfLine&, bool isPar) const;
  // returns    intersection

  rcInfLine normal(const rc2Dvector&) const;
  // returns    +90 degrees line through the specified point.

  rc2Dvector project(const rc2Dvector&) const;
  // returns    The projection of point onto this line.

  double offset(const rc2Dvector&) const;
  // returns    Signed distance from me to the point

  bool operator==(const rcInfLine&) const;
  bool operator!=(const rcInfLine&) const;


private:
  rc2Dvector mDir, mPos;
};



#endif /* _rcLINE_H_ */
