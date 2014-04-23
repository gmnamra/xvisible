/******************************************************************************
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 *  rc_draw.h
 *
 * Definitions for routines drawing into rcWindow objects.
 *
 *****************************************************************************/

#ifndef _rcDRAW_H_
#define _rcDRAW_H_

#include <math.h>
#include "rc_types.h"
#include "rc_window.h"
#include "rc_polygon.h>
#include "rc_affine.h"

#ifndef rmPrintImage
#define rmPrintImage(a){					    \
for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {    \
fprintf (stderr, "\n");					    \
for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)	    \
fprintf (stderr, "%3d ", (a).getPixel (j__temp, i__temp));    \
fprintf (stderr, "\n");					    \
}}
#endif


#define rmPrintFloatImage(a){					    \
for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {  \
fprintf (stderr, "\n");					    \
float* vp = (float*)(a).rowPointer(i__temp);		    \
for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)   \
fprintf (stderr, "%f ", *vp++);				    \
fprintf (stderr, "\n");					    \
}}

#define rmPrintBinaryImage(a){				    \
  for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {    \
    for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)	    \
      fprintf(stderr, "%1c", ((a).getPixel(j__temp,i__temp)>0) ? '#' : '+'); \
    fprintf (stderr, "\n");					    \
  }}


extern void rcDrawPolygon(const rcPolygon& p, rcWindow& dest, uint8 color,
			  bool fill = true);

extern void rcFillPolygon(const rcPolygon& p, rcWindow& dest, uint8 color);

extern void rcDrawAffineRectangle(const rcAffineRectangle& ar, rcWindow& dest,
				  uint8 color, bool fill = true);

extern void rcDrawIRect(const rcIRect& ir, rcWindow& dest, uint8 color,
			bool fill = true);

template <class T> void rcDrawLine(rc2dVector<T> p1, rc2dVector<T> p2,
				   rcWindow& w, uint8 color, bool fill = true)
{
  rcDrawLine(p1.x(), p1.y(), p2.x(), p2.y(), w, color, fill);
}

template <class T> void rcDrawLine(T p1x, T p1y, T p2x, T p2y,
				   rcWindow& w, uint8 color, bool fill = true)
{
  int32 curX = (int32)(p1x + 0.5), curY = (int32)(p1y + 0.5);
  int32 lastX = (int32)(p2x + 0.5), lastY = (int32)(p2y + 0.5);
  rmAssert(curX >= 0 && curX < w.width());
  rmAssert(curY >= 0 && curY < w.height());
  rmAssert(lastX >= 0 && lastX < w.width());
  rmAssert(lastY >= 0 && lastY < w.height());

  if (curX == lastX) {
    // Vertical line
    if (curY < lastY) {
      for (int32 i = curY; i <= lastY; i++)
	w.setPixel(curX, i, color);
    }
    else {
      for (int32 i = lastY; i <= curY; i++)
	w.setPixel(curX, i, color);
    }
    return;
  }
  else if (curY == lastY) {
    // Horizontal line
    if (curX < lastX) {
      for (int32 i = curX; i <= lastX; i++)
	w.setPixel(i, curY, color);
    }
    else {
      for (int32 i = lastX; i <= curX; i++)
	w.setPixel(i, curY, color);
    }
    return;
  }

  T w1x, w1y, w2x, w2y;
  bool cXnY = true;
  if (p1y > p2y)
    cXnY = false;
  
  if (curX < lastX) {
    w1x = p1x; w1y = p1y;
    w2x = p2x; w2y = p2y;
  }
  else {
    w1x = p2x; w1y = p2y;
    w2x = p1x; w2y = p1y;
    int32 temp = curX;
    curX = lastX;
    lastX = temp;
    temp = curY;
    curY = lastY;
    lastY = temp;
  }

  double theta = atan2(fabs((double)(w2y - w1y)),
		       fabs((double)(w2x - w1x)));
  double dx = cos(theta);
  double dy = sin(theta);
  if (w1y > w2y)
    dy = -dy;
  
  const float rkDegree = 180/rkPI;
  if (0) printf("theta %f p1(%f, %f) p2 (%f, %f) D(%f, %f) cXnY %d\n",
		theta*rkDegree, w1x, w1y, w2x, w2y, dx, dy, cXnY);
	 
  w.setPixel(curX, curY, color);
  if (0) printf("O (%d, %d)\n", curX, curY);

  w1x += dx; w1y += dy;

  while (w1x < w2x) {
    int32 nextX = (int32)(w1x + 0.5), nextY = (int32)(w1y + 0.5);

    if ((nextX != curX) || (nextY != curY)) {
      if (0) printf("S (%d, %d)\n", nextX, nextY);
      w.setPixel(nextX, nextY, color);
      if ((nextX != curX) && (nextY != curY) && fill) {
	if (cXnY) {
	  if (0) printf("FCurNxt (%d, %d)\n", curX, nextY);
	  w.setPixel(curX, nextY, color);
	}
	else {
	  if (0) printf("FNxtCur (%d, %d)\n", nextX, curY);
	  w.setPixel(nextX, curY, color);
	}
      }
      curX = nextX;
      curY = nextY;
    }

    w1x += dx; w1y += dy;
  }

  if (curX != lastX) {
    if (0) printf("Left (%d, %d) to (%d, %d)\n", curX, curY, lastX, curY);
    for (int32 i = curX; i <= lastX; i++)
      w.setPixel(i, curY, color);
  }
  else if (curY != lastY) { 
    if (0) printf("Left (%d, %d) to (%d, %d)\n", curX, curY, curX, lastY);
    if (curY < lastY) {
      for (int32 i = curY; i <= lastY; i++)
	w.setPixel(curX, i, color);
    }
    else {
      for (int32 i = lastY; i <= curY; i++)
	w.setPixel(curX, i, color);
    }
  }
}

void binarizeFloatWindow(const rcWindow& src, rcWindow& dest,
			 const float thresh,
			 const uint8 fg, const uint8 bg);



/*	*************************
	*                       *
	*     1D plotting       *
	*                       *
	*************************

	Not using polyLine for now
*/


template <class Iterator>
void rfPlot1Dsignal (Iterator Ib, Iterator Ie, rcFRect& frame, rcVisualSegmentCollection& v, bool drawMeanLine = true)
{
  int32 length, width, dS (1), dB (1);
  double mini (rcDBL_MAX) , mean, 
    maxi (rcDBL_MIN), sum (0.0), sumsq (0.0);

  length = Ie - Ib;
  if (length < 3) return;
  width = (int32) frame.width();

  if (width > length)
    {
      dB = width / length;
    }

  if (length > width)
    {
      dS = length / width;
    }
  
  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  Iterator ip = Ib;

  while (ip < Ie)
    {
      value_type iv (*ip);
      mini = rmMin (mini, iv);
      maxi = rmMax (maxi, iv);
      sum += iv;
      sumsq += iv * iv;
      ip++;
    }
  
  mean = sum / length;
  sumsq = sqrt (length * sumsq - sum * sum) / length;

  float scale = maxi - mini;
  float fh (frame.height());
  float x0 (frame.ll().x());
  float y0 (frame.ll().y());
  // Check the frame dimension
  // Assume that rect fits in the viewing image
  // Draw the frame
  rcVisualRect rr (frame.ul(), frame.lr());
  v.push_back(rr);

  // Graph starts at frame.ul() + (frame.height(), 0)
  // Plot Mean and Std
  rc2Fvector meanLine (x0, y0 - rmMin ((mean - mini) * fh / scale, fh));

  if(drawMeanLine)
    v.push_back(rcVisualLine (meanLine, rc2Fvector ((float) frame.lr().x(), meanLine.y())));

  ip = Ib;
  value_type iv (*ip);  
  ip+=dS;
  float yc, yp = (iv - mini) * fh;
  yp = yp / scale;

  while (ip < Ie)
    {
      value_type iv (*ip);
      yc = (iv - mini) * fh;
      yc = yc / scale;
      v.push_back(rcVisualLine (rc2Fvector (x0, frame.ll().y() - yp), 
				rc2Fvector (x0+=dB, frame.ll().y() - yc)));
      ip+=dS;
      yp = yc;
    }
}


// Slighly different one using affine rectangles (and example of poor modularity!)

template <class Iterator>
void rfPlot1Dsignal (Iterator Ib, Iterator Ie, rcAffineRectangle& ar, rcVisualSegmentCollection& v)
{
  int32 length;

  length = Ie - Ib;
  if (length < 3) return;
  bool side = (length) == ar.cannonicalSize().y();

  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  Iterator ip = Ib;
  Iterator mind = min_element (Ib, Ie);
  Iterator maxd = max_element (Ib, Ie);
  double scale = 1.0 / (*maxd - *mind);

  rc2Dvector curr, last;
  last = ar.affineToImage (last);
  ip = Ib;
  double xPost (0.0);
  rcDPair ds (1.0 / (double) ar.cannonicalSize().x(), 1.0 / (double) ar.cannonicalSize().y());

  for (; ip < Ie; ip++, last = curr)
    {
      if (!side)
	{
	  curr.x(xPost);
	  xPost += ds.x();
	  curr.y((*ip - *mind) * scale);
	}
      else
	{
	  curr.y(xPost);
	  xPost += ds.y();
	  curr.x((*ip - *mind) * scale);
	}

      curr = ar.affineToImage (curr);
      rc2Fvector currd ((float) curr.x(), (float) curr.y());
      rc2Fvector lastd ((float) last.x(), (float) last.y());
      v.push_back(rcVisualLine (lastd, currd));
    }
}

#endif
