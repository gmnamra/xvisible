/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_lin_generators.h
 *   Creation Date  09/09/2003
 *   Author         Peter Roberts
 *
 * rcLinGeneratePixel is a concrete derived version of rcGeneratePixel.
 * It implements a bilinear pixel interpolation algorithm.
 *
 ***************************************************************************/

#ifndef _rcLIN_GENERATORS_H_
#define _rcLIN_GENERATORS_H_

#include "rc_generator.h"
#include "rc_affinewindow.h"

/* rcLinGeneratePixel - Returns the pixel containing the point x, y.
 */
class rcLinGeneratePixel : public rcGeneratePixel
{
 public:

  rcLinGeneratePixel() {}

  virtual ~rcLinGeneratePixel() {}

  double genPixel(const rcAffineWindow* src, double x, double y)
  {
    rmAssert(src && (src->depth() <= rcPixelDouble));
    int32 ix((uint32)x), iy((uint32)y);
    double bx = x - ix, by = y - iy;
    double ax = 1.0 - bx, ay = 1.0 - by;
    double p00, p10 = 0.0, p01 = 0.0, p11 = 0.0;

    const bool getXPlus1 = bx != 0.0;
    const bool getYPlus1 = by != 0.0;

    if (getXPlus1)
      rmAssert(ix+1 < src->width());
    else
      rmAssert(ix < src->width());

    if (getYPlus1)
      rmAssert(iy+1 < src->height());
    else
      rmAssert(iy < src->height());

    if (src->depth() == rcPixelDouble) {
      p00 = src->getDoublePixel(ix, iy);
      if (getXPlus1) p10 = src->getDoublePixel(ix+1, iy);
      if (getYPlus1) p01 = src->getDoublePixel(ix, iy+1);
      if (getXPlus1 & getYPlus1) p11 = src->getDoublePixel(ix+1, iy+1);
    }
    else {
      p00 = src->getPixel(ix, iy);
      if (getXPlus1) p10 = src->getPixel(ix+1, iy);
      if (getYPlus1) p01 = src->getPixel(ix, iy+1);
      if (getXPlus1 & getYPlus1) p11 = src->getPixel(ix+1, iy+1);
    }

    double v0 = p00*ax + p10*bx;
    double v1 = p01*ax + p11*bx;

    return v0*ay + v1*by;
  }
};

#endif
