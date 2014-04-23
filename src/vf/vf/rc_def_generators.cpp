/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_def_generators.cpp
 *   Creation Date  09/04/2003
 *   Author         Peter Roberts
 *
 * rcDefGeneratePixel and rcDefGenerateImage are simple, concrete
 * derived versions of classes rcGeneratePixel and rcGenerateImage,
 * respectively.
 *
 ***************************************************************************/

#include "rc_def_generators.h"

void rcDefGenerateImage::genImage(const rcAffineWindow* src, rcWindow& dest,
				  const rcIRect& region)
{
  const int32 xOff = region.origin().x(), yOff = region.origin().y();
  const rcIPair endPt = region.lr();
  const int32 endX = endPt.x() - xOff;
  const int32 endY = endPt.y() - yOff;

  rmAssert(src && (src->depth() == dest.depth()));

  if (src->depth() == rcPixelDouble) {
    for (int32 x = 0; x < endX; x++)
      for (int32 y = 0; y < endX; y++) {
	rc2Dvector v(src->getPixelLoc(x, y));
	dest.setDoublePixel(x, y, _genPixel.genPixel(src, v.x(), v.y()));
      }
  }
  else {
    if (src->depth() == rcPixel8)
      {
          rmAssert(0); // not implemented
                       //	vImage_Error ve; 
                       //	vImage_Buffer vs, vd;
                       //	src->vImage (vs);
                       //	dest.vImage (vd);
                       //	vImage_AffineTransform ga = src->vImageToAffine ();
                       //	ve = vImageAffineWarp_Planar8( &vs, &vd, NULL, & ga, Pixel_8 (0), kvImageNoFlags);
                       //	rmAssert (!ve);
      }
    else
      {
    for (int32 x = 0; x < endX; x++)
      for (int32 y = 0; y < endY; y++) {
	rc2Dvector v(src->getPixelLoc(x, y));
	//printf("x y %d %d %f %f\n", x, y, v.x(), v.y());
	dest.setPixel(x, y, (uint32)_genPixel.genPixel(src, v.x(), v.y()));
      }
      }
  }
}


         
//                          1             (x0, y0)
//                          *                    0         1
//                         *  *                  ***********
//                        *     *                *         *
//                       *        * 2            *         *
//                   0  *        *               *         *
//           (x0,y0)      *     *                ***********
//                          *  *                 3         2
//                            *
//                            3


void rcDefGenerateImage::project(const rcAffineWindow* src, vector<float>& dst, const rcWindow& mask, rc2Xform& xform)
{
  rmAssert (dst.size());
  const rcMatrix_2d& mat = xform.matrix();

  bool yProj (mat.element(0,0) == 1.0 && mat.element(1,1) == 0.0 && (int32)dst.size() == src->ar().cannonicalSize().x());
  bool xProj (mat.element(0,0) == 0.0 && mat.element(1,1) == 1.0 && (int32)dst.size() == src->ar().cannonicalSize().y());
  bool maskOn = mask.isBound();

  rmAssert (xProj ^ yProj);
  int32 projectionWidth (dst.size());
  int32 projectionHeight = (yProj) ? src->ar().cannonicalSize().y() : src->ar().cannonicalSize().x();

  vector<float>::iterator accPtr = dst.begin();

  // Walk through projection image, project along the other dimension
  if (yProj)
    { // Projecting down columns
      for (int32 x = 0;  x < projectionWidth;  x++)
	{
	  for (int32 y = 0;  y < projectionHeight;  y++)
	    {
	      rc2Dvector v (src->getPixelLoc (x, y));

	      if (maskOn && mask.getPixel ((int32) v.x(), (int32) v.y()) == 0)
		continue;

	      float pel = (float)_genPixel.genPixel (src, v.x(), v.y());
	      accPtr[x] += pel;
	    }
	}
    }

  if (xProj)
    { // Projecting down rows
      for (int32 y = 0;  y < projectionWidth;  y++)
	{
	  for (int32 x = 0;  x < projectionHeight;  x++)
	    {
	      rc2Dvector v (src->getPixelLoc (x, y));

	      if (maskOn && mask.getPixel ((int32) v.x(), (int32) v.y()) == 0)
		continue;

	      float pel = (float)_genPixel.genPixel (src, v.x(), v.y());
	      accPtr[y] += pel;
	    }
	}
    }
  
  const rc2Xform dis (src->ar());
  xform = xform * dis;
}

