/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_cubic_generators.h
 *   Creation Date  09/09/2003
 *   Author         Peter Roberts
 *
 * rcCubicGeneratePixel is a concrete derived version of rcGeneratePixel.
 * It implements a cubic spline pixel interpolation algorithm.
 *
 ***************************************************************************/

#ifndef _rcCUBIC_GENERATORS_H_
#define _rcCUBIC_GENERATORS_H_

#include "rc_generator.h"
#include "rc_affinewindow.h"

/* rcCubicGeneratePixel - Returns the pixel containing the point x, y.
 */
class rcCubicGeneratePixel : public rcGeneratePixel
{
 public:

  rcCubicGeneratePixel() {}

  virtual ~rcCubicGeneratePixel() {}

  /* genPixel - Generate interpolated value. The first time it is
   * called it will generate window specific data that will be used in
   * subsequent calls to this fct.  Before calling this fct using a
   * different window (or if it is unknown what the previous was)
   * clear() should get called first.
   */
  double genPixel(const rcAffineWindow* src, double x, double y);

  /* clear - Clear out cached image specific data. Should be called
   * any time the window being worked on changes (ot any of the pixels
   * within the window).
   */
  void clear();

 private:

  /* gen2D2ndD - Generate 2D vector of 2nd derivatives
   */
  void gen2D2ndD(const rcWindow& boundingBox);

  /* gen2ndD - Generate 1D vector of 2nd derivatives
   */
  void gen2ndD(const vector<float>& y, vector<float>& y2,
	       float yp1, float ypn, bool v1, bool vn);
  void gen2ndD(const rcWindow& src, int32 row, vector<float>& y2,
	       float yp1, float ypn, bool v1, bool vn);

  /* interpolate - Generate an interpolated value at location x within the
   * given vector/window, using as input both the given vector/window and
   * some previously calculated vector of 2nd derivatives.
   */
  double interpolate(const vector<float>& y, const vector<float>& y2d, float x);
  float interpolate(const rcWindow& src, int32 row,
		    const vector<float>& y2d, float x);

  vector<vector<float> > _y2d;
  vector<float> _ytmp, _y2tmp;
  double _maxVal;
};

#endif
