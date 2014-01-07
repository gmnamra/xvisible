/* @file
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.5  2005/05/01 19:38:40  arman
 *added minimum orth bounding rect
 *
 *Revision 1.4  2005/04/24 02:29:12  arman
 *untested but fixed mRadii.x, y
 *
 *Revision 1.3  2005/04/23 17:51:07  arman
 *incremental
 *
 *Revision 1.2  2005/04/10 00:57:23  arman
 *added includes
 *
 *Revision 1.1  2005/04/10 00:55:03  arman
 *initial ellipse class
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcELLIPSE_H
#define __rcELLIPSE_H

#include <rc_matrix.h>
#include <rc_rectangle.h>

// @note simple ellipse characterising class using normalization to a unit circle
// This class is mainly used to draw a circle by using a within test.
// Ellipse equation can be derived but I do not need it now. 

class rcEllipse
{
 public:
  rcEllipse ()  {}
  rcEllipse (const rc2Dvector & c, const rc2Dvector& r, rcRadian o)
    : mCenter (c), mRadii(r), mOrient(o) {}

  bool degenerate () const 
  { return mRadii.x() <= 0. 
      || mRadii.y() <= 0.;
  }

  const rc2Dvector& center () const
  {
    return mCenter;
  }

  struct rcEllipseNormUnit
  {
    rc2Dvector mCtr;
    rc2Matrix mUnit;
    rcEllipseNormUnit () {}
    rcEllipseNormUnit (const rc2Dvector& point, const rc2Matrix& norm)
      : mCtr(point), mUnit(norm) { }
  };

  rcEllipseNormUnit unit() const
  {
    if (degenerate()) 
      return rcEllipseNormUnit (mCenter, rc2Matrix());

    mOrient.norm();

    double sn = sin (mOrient) / mRadii.x();
    double cs = cos (mOrient) / mRadii.x();
    double tg = tan (rcRadian (0.0));

    rc2Matrix mm (cs + sn * tg, 
		  sn - cs * tg,
		  -sn / (mRadii.y() / mRadii.x()), // y / x is aspect ratio
		  cs / (mRadii.y() / mRadii.x()));

    return rcEllipseNormUnit (mCenter, mm );
  }


  bool contains (const rc2Dvector& p) const
  {
    if (degenerate ()) return (p - center()).len() == 0.;
    rcEllipseNormUnit u = unit ();
    return (u.mUnit* (p - u.mCtr)).len() <= 1.;
  }

  rcDRect boundingOrthRectangle () const
  {
    double c, s, x2, y2;
    c = rmSquare (cos (mOrient));
    s = rmSquare (sin (mOrient));
    x2 = rmSquare (mRadii.x());
    y2 = rmSquare (mRadii.y());
    rc2Dvector r (sqrt (y2 * s + x2 * c), sqrt (x2 * s + y2 * c));
    rc2Dvector tl = center() - r;
    rc2Dvector size = r * 2.;
    return rcDRect (tl.x(), tl.y(), size.x(), size.y());
  }

  // STL needs this
  bool operator==(const rcEllipse&  e) const;
  bool operator!=(const rcEllipse&  e) const;
    
 private:
  rc2Dvector mCenter;
  rc2Dvector mRadii;
  rcRadian mOrient;
};


inline bool rcEllipse::operator==(const rcEllipse&  e) const
{ return (mCenter == e.mCenter &&
	  mRadii == e.mRadii &&
	  mOrient == e.mOrient);}


inline bool rcEllipse::operator!=(const rcEllipse& e) const
{ return !(*this == e);}

#endif /* __rcELLIPSE_H */
