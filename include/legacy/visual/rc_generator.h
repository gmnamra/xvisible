/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_generator.h
 *   Creation Date  09/04/2003
 *   Author         Peter Roberts
 *
 * rcGeneratePixel and rcGenerateImage are a couple of pure virtual
 * classes used to encapsulate image processing functions that
 * transform the points/regions in rcAffineWindows into
 * pixels/rcWindows rectilinear w.r.t. the image plane.
 *
 ***************************************************************************/

#ifndef _rcGENERATOR_H_
#define _rcGENERATOR_H_

#include <rc_window.h>
#include <rc_xforms.h>

class rcAffineWindow;

class rcGeneratePixel
{
 public:

  rcGeneratePixel() {}

  virtual ~rcGeneratePixel() {}

  /* Go to image location x, y in src and return a pixel value for
   * that location.
   */
  virtual double genPixel(const rcAffineWindow* src, double x, double y) = 0;
};

class rcGenerateImage
{
 public:

  rcGenerateImage() {}

  virtual ~rcGenerateImage() {}
  
  /* Go to src and generate pixels for the locations specified by
   * region and store these in dest.
   *
   * region specifies a grid of pixel locations within src whose
   * values are to be generated and stored in dest. In other words,
   * genImage() will effectively call src's getPixelCenterLoc()
   * function once for each integral offset within region and then
   * calculate each destination pixel value.  An error will be thrown
   * if the values specified by region don't fit within the image size
   * specified in src's ctor.
   *
   * An error will also be thrown if either size dimension of dest is
   * >= that of src, or if their pixel sizes don't match.
   */
  virtual void genImage(const rcAffineWindow* src, rcWindow& dest,
			const rcIRect& region) = 0;

  /* Go to src and generate pixels for the locations specified by
   * region and project in to destination
   *
   * region specifies a grid of pixel locations within src whose
   * values are to be generated and stored in dest. In other words,
   * genImage() will effectively call src's getPixelCenterLoc()
   * function once for each integral offset within region and then
   * calculate each destination pixel value.  An error will be thrown
   * if the values specified by region don't fit within the image size
   * specified in src's ctor.
   *
   * An error will also be thrown if either size dimension of dest is
   * >= that of src, or if their pixel sizes don't match.
   *
   * Xform on input indicates horizontal or vertical projection. Specifically
   * the rc2matrix part has either [1, 0, 0, 0] form or [0, 0, 0, 1] indicating
   * cos equals 1 indicating projections of columns or sin equals 1 indicating
   * projections of rows. On output this matrix is composed with affineWindow
   * xform for mapping between 1d projection and image coordinate it came from
   *
   */
  virtual void project (const rcAffineWindow* src, vector<float>&, const rcWindow& mask, rc2Xform&) = 0;

};

#endif
