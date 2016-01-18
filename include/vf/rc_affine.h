/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.16  2006/01/15 22:56:52  arman
 *selective myo
 *
 *Revision 1.15  2005/04/11 00:21:02  arman
 *removed vImage support
 *
 *Revision 1.14  2005/04/06 21:46:45  arman
 *switched to vImage_Affine
 *
 *Revision 1.13  2005/04/05 00:36:04  arman
 *added untest CGAffineTransform Support
 *
 *Revision 1.12  2004/08/06 18:15:50  arman
 *switched to CGAffine
 *
 *Revision 1.11  2004/05/26 17:09:20  arman
 *added declaration of rfAffine
 *
 *Revision 1.10  2004/05/18 01:23:43  arman
 *Working version of the vImage Support
 *
 *Revision 1.9  2004/05/06 12:32:11  arman
 *added TBD for scale of pixel sampling
 *
 *Revision 1.8  2004/04/18 03:52:45  arman
 *added mOrg. Also changed affine api
 *
 *Revision 1.7  2004/04/01 23:48:25  arman
 *added expand
 *
 *Revision 1.6  2004/03/19 17:21:26  arman
 *More Contractile Motion
 *
 *Revision 1.5  2004/03/11 13:24:34  arman
 *cosmetics
 *
 *Revision 1.4  2004/03/04 20:59:57  arman
 *added isValid ()
 *
 *Revision 1.3  2004/03/01 14:34:38  arman
 *cosmetic adds
 *
 *Revision 1.2  2003/11/16 19:57:58  proberts
 *polygon support
 *
 *Revision 1.1  2003/09/16 15:58:38  proberts
 *Added new affine support, interpolator interfaces and implementations, and movie-save-from-rcWindows classes
 *
 *Revision 1.2  2003/08/22 20:07:31  arman
 *corrected comments
 *
 *Revision 1.1  2003/08/22 19:48:23  arman
 *Affine rectangle class
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

/*! \class rcAffineRactangle rc_affine.h "analysis/include/rc_affine.h"
 *  \brief This class represents an affine rectangle used in specifying an image affine transformation
 *
 * Performing an image affine transformation is equivalent to
 * positioning an affine rectangle on underlying cartesian image
 * data. Affine transform can represent rotation, scale, and
 * translation.  Image transformation is performed by picking integer
 * pixel positions in AR, transforming them to a pixel position in the
 * source image (non-integer typically), obtaing a pixel value by
 * interplolated or uninterpolated means and storing it.
 *
 *      o  ______________________________________________ X-axis
 *        |
 *        |  Y-axis
 *        |                                TR
 *        |                               /\
 *        |                              /  \
 *        |                             /    \
 *        |                            /      \ 
 *        |                           /       /
 *        |                          /   O   /
 *        |                      TL /       /
 *        |                         \      /
 *        |                          \    /
 *        |                           \  / 
 *        |                            \/
 *        |                           BL
 *        |
 *        |
 *        |
 *        |
 *        |
 *        |
 *  
 *
 * As an example, consider a muscle cell that is positioned at an
 * angle (between major axis of its body and horizontal increasing x
 * axis. We have information (from UI or analysis) that a rotated
 * rectangle located at mX and mY (anchor) at angle of alpha
 * (horizontal increasing x to major axis)
 *
 * CGAffineTransform is CoreGraphics (Apple) structure for describing a coordinate
 * It defines BL as the origin and its y is reverse of TV coordinate system.
 *
 * TBD: Handle scale of sampling 
 */

#ifndef __rcAFFINE_H
#define __rcAFFINE_H


#include "rc_xforms.h"
#include "rc_rect.h"


//#include <vImage/vImage_Types.h>

class rcAffineRectangle : public rc2Xform
{
 public:

 //! default constructor takes no arguments
    /*!
    */
  rcAffineRectangle ();

  //! constructor making an affine rectangle at org oriented angle 
  /*!
   * org defines the translation of the affine relative to the image
   * plane, in image coordinates. affineOrigin defines the point to
   * use as the origin of the affine rectangle itself, and is in
   * affine coordinates.
   */
  rcAffineRectangle (const rc2Dvector& org, const rcRadian& angle,
		     const rcDPair& scale,  const rcIPair& imgSz,
		     const rcDPair& affineOrigin = rcDPair(0.5, 0.5));

 //! constructor taking endpoints and otherDimension
  /*!
   */
  rcAffineRectangle (const rc2Dvector& anch, const rc2Dvector& endpoint,
		     const double& otherDimension,const rcDPair& affineOrigin = rcDPair(0.5, 0.5));

  bool isValid () const
  {
    return (mImgSz.x() > 0 && mImgSz.y() > 0);
  }

  const rc2Dvector& origin() const
  { return trans(); }

  rcRadian angle() const
  { return rotation(); }

  rcDPair xyScale() const
  { return rcDPair(xScale(), yScale()); }

  const rcIPair& cannonicalSize() const
  { return mImgSz; }

  bool cannonicalXsizeIsGreater () const 
  {
    return (cannonicalSize().x() >= cannonicalSize().y());
  }

  /* boundingBox - Returns the affine's bounding box in image
   * coordinates. Note that the coordinates returned are guaranteed to
   * contain not just the affine itself, but also any additonal
   * rows/columns required by the interpolation functions.
   */
  rcRect boundingBox() const;

  /* imageToAffine - Compute an image point's affine coordinates.
   */
  rc2Dvector imageToAffine(const rc2Dvector& pt) const
  { return invMapPoint(pt); }

 rc2Fvector imageToAffine(const rc2Fvector& pt) const
  { return invMapPoint(pt); }

  /* affineToImage - Compute an affine point's image coordinates.
   */
  rc2Dvector affineToImage(const rc2Dvector& pt) const
  { return mapPoint(pt); }

  rc2Fvector affineToImage(const rc2Fvector& pt) const
  { return mapPoint(pt); }

  /* getPixelLoc - Affine "pixel" locations are translated to
   * positions within the affine coordinate system using the image
   * size arg passed to ctor. These are then used to calculate the x,
   * y location within the image coordinate system. This value is
   * returned to the caller.
   *
   * Generates an error if either x or y exceeds the image size
   * specified in the ctor.
   */
  rc2Dvector getPixelLoc(int32 x, int32 y) const;
  rc2Dvector getPixelLoc(double x, double y) const;

  /* 
   * Expand:
   * expand this affineRectangle by steps corresponding to 
   * 1/xSize, 1/ySize. 
   * affineOrigin used for this rcAffineRectangle is passed in
   * since there seems to be no way to get it back. 
   * 
   */
  rcAffineRectangle expand (const rcIPair& steps) const;

  friend ostream& operator<< (ostream& ous, const rcAffineRectangle& dis)
  {
    ous << "[AR: O: " << dis.origin() << " A: " << dis.angle().Double() << " D: "
	<< dis.cannonicalSize() << "S: " << dis.xScale() << " " << dis.yScale();
    ous << dis.matrix() << endl << "Axy: " << dis.xRot() << " " << dis.yRot() << endl;
    ous << dis.boundingBox () << " AR] " << endl;
    return ous;
  }

  bool operator==(const rcAffineRectangle& aRect) const
  { return (mImgSz == aRect.mImgSz) && ((rc2Xform*)this)->operator==(aRect); }
  
  bool operator!= (const rcAffineRectangle& aRect) const
  { return !(*this == aRect); }

 private:
  rc2Dvector mStep;
  rcIPair    mImgSz;
  rc2Dvector mOrg;
};

//void rfAffineRectangleToSegmentsCollection(const rcAffineRectangle& p, rcVisualSegmentCollection& v);


#endif /* __rcAFFINE_H */
