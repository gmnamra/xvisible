/*
 *
 *$Id $
 *$Log$
 *Revision 1.2  2005/08/30 21:08:51  arman
 *Cell Lineage
 *
 *Revision 1.2  2005/07/30 06:06:50  arman
 * added roundRectangle
 *
 *Revision 1.1  2003/02/24 17:36:44  arman
 *implementation of rc_rectangle
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "rc_rectangle.h"

template rcIRect rfRoundRectangle<float> (const rcRectangle<float>&);
template rcIRect rfRoundRectangle<double> (const rcRectangle<double>&);

void rfBoundingRect (const deque<rc2Dvector>& pts, rcDRect& box)
{
  double x, y, minx, miny, maxx, maxy;

  rmAssert(pts.size() >= 1);

  deque<rc2Dvector>::const_iterator point = pts.begin();
  deque<rc2Dvector>::const_iterator lastplus = pts.end();

  minx = maxx = point->x();
  miny = maxy = point->y();

  for(point++; point != lastplus; point++)
    {
      x = point->x();
      y = point->y();

      if (x < minx)
	minx = x;

      if (x > maxx)
	maxx = x;

      if (y < miny)
	miny = y;

      if (y > maxy)
	maxy = y;
    }

  box = rcDRect (rcDPair (minx, miny), rcDPair (maxx, maxy));
}
  
