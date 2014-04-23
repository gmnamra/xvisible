/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_def_generators.h
 *   Creation Date  09/04/2003
 *   Author         Peter Roberts
 *
 * rcDefGeneratePixel and rcDefGenerateImage are simple, concrete
 * derived versions of classes rcGeneratePixel and rcGenerateImage,
 * respectively.
 *
 ***************************************************************************/

#ifndef _rcDEF_GENERATORS_H_
#define _rcDEF_GENERATORS_H_

#include "rc_generator.h"
#include "rc_affinewindow.h"

/* rcDefGeneratePixel - Returns the pixel containing the point x, y.
 */
class rcDefGeneratePixel : public rcGeneratePixel
{
 public:

  rcDefGeneratePixel() {}

  virtual ~rcDefGeneratePixel() {}

  double genPixel(const rcAffineWindow* src, double x, double y)
  {
    rmAssert(src && (src->depth() <= rcPixelDouble));
    if (src->depth() == rcPixelDouble)
      return src->getDoublePixel((int32)(x + 0.5), (int32)(y + 0.5));

    return (double)(src->getPixel((int32)(x + 0.5), (int32)(y + 0.5)));
  }
};

class rcDefGenerateImage : public rcGenerateImage
{
 public:

  rcDefGenerateImage(rcGeneratePixel& genPixel) : _genPixel(genPixel) {}

  virtual ~rcDefGenerateImage() {}
  
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
  void genImage(const rcAffineWindow* src, rcWindow& dest,
		const rcIRect& region);
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
  void project (const rcAffineWindow* src, vector<float>&, const rcWindow& mask, rc2Xform&);


 private:

  rcGeneratePixel& _genPixel;
};

#endif
