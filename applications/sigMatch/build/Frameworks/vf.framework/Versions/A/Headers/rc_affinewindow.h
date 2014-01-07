/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_affinewindow.h
 *   Creation Date  09/04/2003
 *   Author         Peter Roberts
 *
 * rcAffineWindow is an rcWindow derived class that uses an
 * rcAffineRectangle, instead of an rcIRect, to define its geometry.
 *
 ***************************************************************************/

#ifndef _rcAFFINEWINDOW_H_
#define _rcAFFINEWINDOW_H_

#include <rc_window.h>
#include <rc_affine.h>
#include <rc_generator.h>

class rcAffineWindow : public rcWindow {
 public:

  rcAffineWindow ()  : _pixGen (NULL), _imgGen (NULL)  , _affine (rcAffineRectangle () ) {}

  
  /* ctors:
   *
   * 1) Copy ctor
   * 2) Child window using affine rectangle to define location within
   *    existing window.
   * 3) Create a framebuffer large enough to contain affine rectangle.
   *
   * In case 2), an error will occur if the bounding box of the affine
   * rectangle is not contained within the parent.
   */
  rcAffineWindow(const rcAffineWindow& parent);
  rcAffineWindow(const rcWindow& parent, const rcAffineRectangle& affine);
  rcAffineWindow(const rcWindow& parent, const rcAffineRectangle& affine, 
		 bool& isInside);
  rcAffineWindow(const rcAffineRectangle& affine,
		 rcPixel depth  = rcPixel8);

  /* Operators
   */
  rcAffineWindow& operator=(const rcAffineWindow& rhs)
  {
    if (this != &rhs) {
      ((rcWindow*)this)->operator=(*(rcWindow*)&rhs);
      _affine = rhs._affine;
      _pixGen = rhs._pixGen;
      _imgGen = rhs._imgGen;
    	mVa2i = rhs.mVa2i;
			mVi2a = rhs.mVi2a;
    }
    return *this;
  }

  /* Accessors
   */
  const rcAffineRectangle& ar() const
  { return _affine; }
  rc2Dvector getPixelLoc(int32 x, int32 y) const
  { return _affine.getPixelLoc(x, y); }
  rc2Dvector getPixelLoc(double x, double y) const
  { return _affine.getPixelLoc(x, y); }
  rcIPair getImageSz() const
  { return _affine.cannonicalSize(); }
  rcGeneratePixel& pixelGenerator()
  { return *_pixGen; }
  rcGenerateImage& imageGenerator()
  { return *_imgGen; }

  /* 
   * vImageAffineTransform:
   * Fill a vImage_AffineTransform structure (Apple's Accelerate FrameWork)
   * vImage Affine transform requires bottom left origin
   * 
   */
  vImage_AffineTransform vImageToAffine () const;
  vImage_AffineTransform vAffineToImage () const;

  
  /* Mutators
   */
  void pixelGenerator(rcGeneratePixel& pixGen)
  { _pixGen = &pixGen; }
  void imageGenerator(rcGenerateImage& imgGen)
  { _imgGen = &imgGen; }

  void vImageTransform ();

  /* Image processors
   */
  double genPixel(int32 x, int32 y, rcGeneratePixel* pixGen = 0) const
  { rc2Dvector v(getPixelLoc(x, y));
    if (pixGen)
      return (*pixGen).genPixel(this, v.x(), v.y());
    return (*_pixGen).genPixel(this, v.x(), v.y());
  }
  void genImage(rcWindow& dest, const rcIRect& region,
		rcGenerateImage* imgGen = 0) const
  { if (imgGen)
      (*imgGen).genImage(this, dest, region);
    else
      (*_imgGen).genImage(this, dest, region);
  }

  void project(vector<float>& dest, rc2Xform& xform, const rcWindow& mask, rcGenerateImage* imgGen = 0) const
  { if (imgGen)
      (*imgGen).project(this, dest, mask, xform);
    else
      (*_imgGen).project(this, dest, mask, xform);
  }

  /* Special setup fct. Should get called before any threads get set
   * up.
   */
  static rcGeneratePixel* createDefGen();

 private:
  rcAffineRectangle _affine;
  rcGeneratePixel*  _pixGen;
  rcGenerateImage*  _imgGen;
  vImage_AffineTransform mVa2i;
  vImage_AffineTransform mVi2a;

  static rcGeneratePixel* _defPixGen;
  static rcGenerateImage* _defImgGen;
};

#endif
