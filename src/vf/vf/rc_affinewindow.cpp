/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_affinewindow.cpp
 *   Creation Date  09/04/2003
 *   Author         Peter Roberts
 *
 * rcAffineWindow is an rcWindow derived class that uses an
 * rcAffineRectangle, instead of an rcIRect, to define its geometry.
 *
 ***************************************************************************/

#include <rc_affinewindow.h>
#include <rc_def_generators.h>
#include <rc_cubic_generators.h>
#include <rc_lin_generators.h>



rcGeneratePixel* rcAffineWindow::_defPixGen = rcAffineWindow::createDefGen();
rcGenerateImage* rcAffineWindow::_defImgGen = 0;

rcAffineWindow::rcAffineWindow(const rcAffineWindow& parent)
  : rcWindow(parent), _affine(parent.ar()),
    _pixGen(parent._pixGen), _imgGen(parent._imgGen)
{
}

rcAffineWindow::rcAffineWindow(const rcWindow& parent,
			       const rcAffineRectangle& affine)
  : rcWindow(parent), _affine(affine), _pixGen(_defPixGen), _imgGen(_defImgGen)
{
  rmAssert(_pixGen && _imgGen);
  rmAssert(isWithin(affine.boundingBox()));
}

rcAffineWindow::rcAffineWindow(const rcWindow& parent, 
	       const rcAffineRectangle& affine, 
	       bool& isInside)
  : rcWindow(parent), _affine(affine), _pixGen(_defPixGen), _imgGen(_defImgGen)
{
  rmAssert(_pixGen && _imgGen);
  isInside = isWithin(affine.boundingBox());
}


rcAffineWindow::rcAffineWindow(const rcAffineRectangle& affine,
			       rcPixel depth)
  : rcWindow(affine.boundingBox().lr(), depth), _affine(affine),
    _pixGen(_defPixGen), _imgGen(_defImgGen)
{
  rmAssert(_pixGen && _imgGen);
  rmAssert(isWithin(affine.boundingBox()));
}

rcGeneratePixel* rcAffineWindow::createDefGen()
{
  if (!_defPixGen) {
    _defPixGen = new rcDefGeneratePixel();
    rmAssert(_defPixGen);
    
    _defImgGen = new rcDefGenerateImage(*_defPixGen);
    rmAssert(_defImgGen);
  }

  return _defPixGen;
}

#if 0
// @note The following should go to the platform stuff
#define CG2VIMAGE(v,g) (v).a=(g).a,(v).b=(g).b,(v).c=(g).c,(v).d=(g).d,(v).tx=(g).tx,(v).ty=(g).ty

void rcAffineWindow::vImageTransform () 
{

/*      @note: (from /System/Library/Frameworks/Accelerate.framework/Frameworks/vImage.framework/Headers/Geometry.h)
 *	For the Affine Transform function the coordinate space places the origin at the bottom left corner
 * 	of the image. Positive movement in the X and Y direction moves you right and up. Both source and destination
 *	images are assumed to place their bottom left hand corner at the origin. 
 */	

  const rc2Dvector bl = ar().affineToImage(rc2Dvector(0.0, 1.0));

  // Angle goes from +x -> + y to +x -> - y. 
  rcRadian r = arctan (-sin (ar().angle ()), cos (ar().angle()));

  // Image coordinates wrt top left of the image and then bottom left
    CGPoint blg;
    blg.x =  bl.x();
    blg.y = height() - bl.y();

  CGAffineTransform a2i = CGAffineTransformRotate (CGAffineTransformMakeTranslation (blg.x, blg.y),
				   (float) r.basic ());
  CGAffineTransform i2a = CGAffineTransformInvert (a2i);
  CG2VIMAGE(mVa2i, a2i);
  CG2VIMAGE(mVi2a, i2a);

}

vImage_AffineTransform rcAffineWindow::vAffineToImage () const
{
  const_cast<rcAffineWindow *>(this)->vImageTransform ();
  return mVa2i;
}

vImage_AffineTransform rcAffineWindow::vImageToAffine () const
{
  const_cast<rcAffineWindow *>(this)->vImageTransform ();
  return mVi2a;
}
#endif

