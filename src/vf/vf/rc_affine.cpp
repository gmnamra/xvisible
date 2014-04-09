/*
 *
 *$Id $
 *$Log$
 *Revision 1.10  2005/04/11 00:16:17  arman
 *moved vImage support out of here.
 *
 *Revision 1.9  2005/04/06 21:47:26  arman
 *switched to vImage_Affine
 *
 *Revision 1.8  2005/04/05 00:35:47  arman
 *added untest CGAffineTransform Support
 *
 *Revision 1.7  2004/08/06 18:17:12  arman
 *added new vImage support. UT needs following
 *
 *Revision 1.6  2004/05/26 17:08:38  arman
 *added rfAffineSegment graphics
 *
 *Revision 1.5  2004/05/18 01:22:53  arman
 *working version of the vImage Support
 *
 *Revision 1.4  2004/04/18 03:53:52  arman
 *fixed implementation of expand
 *
 *Revision 1.3  2004/04/01 23:48:47  arman
 *added expand
 *
 *Revision 1.2  2004/03/04 21:00:22  arman
 *added isValid
 *
 *Revision 1.1  2003/09/16 15:58:39  proberts
 *Added new affine support, interpolator interfaces and implementations, and movie-save-from-rcWindows classes
 *
 *Revision 1.1  2003/08/22 19:48:56  arman
 *affine rectangle class
 *
 *
 */

#include <rc_affine.h>
#include <iostream>

#if 0
void rfAffineRectangleToSegmentsCollection(const rcAffineRectangle& p,
					   rcVisualSegmentCollection& v)
{
  const rc2Dvector ul = p.affineToImage(rc2Dvector(0, 0));
  const rc2Dvector ur = p.affineToImage(rc2Dvector(1, 0));
  const rc2Dvector ll = p.affineToImage(rc2Dvector(0, 1));
  const rc2Dvector lr = p.affineToImage(rc2Dvector(1, 1));

  v.push_back(rcVisualLine(rc2Fvector((float)ul.x(), (float)ul.y()),
			   rc2Fvector((float)ur.x(), (float)ur.y())));
  v.push_back(rcVisualLine(rc2Fvector((float)ur.x(), (float)ur.y()),
			   rc2Fvector((float)lr.x(), (float)lr.y())));
  v.push_back(rcVisualLine(rc2Fvector((float)lr.x(), (float)lr.y()),
			   rc2Fvector((float)ll.x(), (float)ll.y())));
  v.push_back(rcVisualLine(rc2Fvector((float)ll.x(), (float)ll.y()),
			   rc2Fvector((float)ul.x(), (float)ul.y())));

  v.push_back(rcVisualEmpty());
}
#endif

rcAffineRectangle::rcAffineRectangle()
{
  // Negative Size is a No No
  mImgSz.x() =   mImgSz.y() = -1;
}


rcAffineRectangle::rcAffineRectangle(const rc2Dvector& anch, const rc2Dvector& ep, 
				     const double& otherDimension, const rcDPair& affineOrigin)
{
  // set angle to angle connecting anchor to end point
  // angle (positive x to positive y
  rcRadian angle = (ep - anch).angle();
  rcDPair scale (anch.distance (ep), otherDimension);
  mImgSz = rcIPair ((int32) scale.x(), (int32) scale.y());
  rmAssert(mImgSz.x() > 0);
  rmAssert(mImgSz.y() > 0);
  mStep = rc2Dvector(1./mImgSz.x(), 1./mImgSz.y());

  matrix (rcMatrix_2d (angle, scale));
  trans (anch);
  mOrg.x(affineOrigin.x() == 0.0 ? 0.0 : -affineOrigin.x());
  mOrg.y(affineOrigin.y() == 0.0 ? 0.0 : -affineOrigin.y());
  
  if ((mOrg.x() != 0.0) || (mOrg.y() != 0.0))
    trans(affineToImage(mOrg));
}


rcAffineRectangle::rcAffineRectangle(const rc2Dvector& org, const rcRadian& ang,
				     const rcDPair& scale, const rcIPair& imgSz,
				     const rcDPair& affineOrigin)
  : rc2Xform(rcMatrix_2d(ang, scale), org), mImgSz(imgSz)
{
  rmAssert(mImgSz.x() > 0);
  rmAssert(mImgSz.y() > 0);

  mStep = rc2Dvector(1./mImgSz.x(), 1./mImgSz.y());

  mOrg.x(affineOrigin.x() == 0.0 ? 0.0 : -affineOrigin.x());
  mOrg.y(affineOrigin.y() == 0.0 ? 0.0 : -affineOrigin.y());
  
  if ((mOrg.x() != 0.0) || (mOrg.y() != 0.0))
    trans(affineToImage(mOrg));
}

rcRect rcAffineRectangle::boundingBox() const
{
  double minX, minY, maxX, maxY;

  const rc2Dvector four[4] = { affineToImage(rc2Dvector(0.0, 0.0)),
			       affineToImage(rc2Dvector(0.0, 1.0)),
			       affineToImage(rc2Dvector(1.0, 0.0)),
			       affineToImage(rc2Dvector(1.0, 1.0)) };

  minX = maxX = four[0].x();
  minY = maxY = four[0].y();
  for (uint32 i = 1; i < 4; i++) {
    if (four[i].x() > maxX)
      maxX = four[i].x();

    if (four[i].x() < minX)
      minX = four[i].x();

    if (four[i].y() > maxY)
      maxY = four[i].y();

    if (four[i].y() < minY)
      minY = four[i].y();
  }

  int32 xo = int32(floor(minX)), yo = int32(floor(minY));
  int32 aMaxX = (int32)ceil(maxX) + 1, aMaxY = (int32)ceil(maxY) + 1;
  int32 w = aMaxX - xo, h = aMaxY - yo;

  return rcRect (xo, yo, w, h);
}

rc2Dvector rcAffineRectangle::getPixelLoc(int32 x, int32 y) const
{
  rmAssert(x >= 0 && x < mImgSz.x());
  rmAssert(y >= 0 && y < mImgSz.y());

  return affineToImage(rc2Dvector(x*mStep.x(), y*mStep.y()));
}

rc2Dvector rcAffineRectangle::getPixelLoc(double x, double y) const
{
  rmAssert(x >= 0 && x < (double)mImgSz.x());
  rmAssert(y >= 0 && y < (double)mImgSz.y());

  return affineToImage(rc2Dvector(x*mStep.x(), y*mStep.y()));
}


rcAffineRectangle rcAffineRectangle::expand (const rcIPair& steps) const
{

  // Get the center in image coordiantes
  // Assumption currently about the origin in this code
  // TBD: fix
  rmAssert (mOrg.x() == 0 && mOrg.y() == 0);

  rcDPair half (0.5, 0.5);
  rc2Dvector ctr = rc2Dvector (half) - mOrg;
  ctr = mapPoint (ctr);

  // Now create a Xform scaled appropriately
  rcIPair ic = steps + steps + cannonicalSize ();
  rcDPair scale ((double) ic.x() / (double) cannonicalSize().x(),
		 (double) ic.y() / (double) cannonicalSize().y());
  rcRadian none (0.0); rc2Fvector org (0.0f, 0.0f);
  rcMatrix_2d mat (none, scale);
  rc2Xform exp (mat, org);
  exp = exp * *this; // Note: Order of composition is left to right

  // Now place this affine rectangle anchored at the center
  scale = scale * xyScale();
  return rcAffineRectangle (ctr, angle(), scale, ic, half);
}

