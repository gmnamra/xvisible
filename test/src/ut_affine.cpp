/*
 *
 *$Id $
 *$Log$
 *Revision 1.8  2005/04/04 01:52:54  arman
 *added CGAffine Transformation equality test with AffineRectangle
 *
 *Revision 1.7  2004/08/12 21:02:05  arman
 *removed malfunctioning test
 *
 *Revision 1.6  2004/08/06 23:49:45  arman
 *changes update wrt vImage
 *
 *Revision 1.5  2004/05/18 01:34:00  arman
 *added vImage support unit tests
 *
 *Revision 1.4  2004/04/18 03:34:14  arman
 *updated according to changes in expand api
 *
 *Revision 1.3  2004/04/18 03:19:25  arman
 *added ut for expand
 *
 *Revision 1.2  2004/01/18 22:27:12  proberts
 *new muscle tracker and support code
 *
 *Revision 1.1  2003/09/16 16:07:10  proberts
 *Added new affine support, interpolator interfaces and implementations, and movie-save-from-rcWindows classes
 *
 *Revision 1.2  2003/08/25 12:14:27  arman
 *fixed warnings
 *
 *Revision 1.1  2003/08/22 19:46:29  arman
 *unit test for affineRectangle Class
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_affine.h"
#include <rc_affinewindow.h>
#include <rc_def_generators.h>
#include <Carbon/Carbon.h>

UT_affine::UT_affine()
{
}

UT_affine::~UT_affine()
{
  printSuccessMessage( "Affine Transformation tests", mErrors );
}

uint32 UT_affine::run()
{
  basic();
  getpixelloc();
  window();

  return mErrors;
}

void UT_affine::basic()
{
  {
    rc2Dvector o(2.0, 1.0);
    rcRadian r(rcDegree(0.00));
    rcDPair sc(2.0, 2.0);
    rcAffineRectangle ar(o, r, sc, rcIPair(2, 2), rcDPair(0.0, 0.0));

    rcIRect bb = ar.boundingBox();
    rcUTCheck (bb.ll() == rcIPair(2, 4));
    rcUTCheck (bb.ur() == rcIPair(5, 1));
    
    rcUTCheck(ar.xyScale() == sc);

    rc2Dvector d0(1.0, 1.0);
    rc2Dvector d1 = ar.affineToImage(d0);
    rcUTCheck(rfRealEq(d1.x(), 4.0, 0.0000001));
    rcUTCheck(rfRealEq(d1.y(), 3.0, 0.0000001));
    d0 = rc2Dvector(4.0, 2.0);
    rc2Dvector d2 = ar.imageToAffine(d0);
    rcUTCheck(rfRealEq(d2.x(), 1.0, 0.0000001));
    rcUTCheck(rfRealEq(d2.y(), 0.5, 0.0000001));
  }

  {
    rc2Dvector o(2.0, 1.0);
    rcRadian r(rcDegree(0.00));
    rcDPair sc(2.0, 2.0);
    rcAffineRectangle ar(o, r, sc, rcIPair(2, 2), rcDPair(0.5, 0.5));

    rcIRect bb = ar.boundingBox();
    rcUTCheck (bb.ll() == rcIPair(1, 3));
    rcUTCheck (bb.ur() == rcIPair(4, 0));

    rcUTCheck(ar.xyScale() == sc);

    rc2Dvector d0(1.0, 1.0);
    rc2Dvector d1 = ar.affineToImage(d0);
    rcUTCheck(rfRealEq(d1.x(), 3.0, 0.0000001));
    rcUTCheck(rfRealEq(d1.y(), 2.0, 0.0000001));
    rc2Dvector d2 = ar.imageToAffine(d0);
    rcUTCheck(rfRealEq(d2.x(), 0.0, 0.0000001));
    rcUTCheck(rfRealEq(d2.y(), 0.5, 0.0000001));
  }

  {
    rc2Dvector o(1.0, 1.0);
    rcRadian r(rcDegree(45.00));
    rcDPair sc(2.0, 2.0);
    rcAffineRectangle ar(o, r, sc, rcIPair(2, 2), rcDPair(0.0, 0.0));

    rcIRect bb = ar.boundingBox();
    rcUTCheck (bb.ll() == rcIPair(-1, 5));
    rcUTCheck (bb.ur() == rcIPair(4, 1));

    rcUTCheck(ar.xyScale() == sc);

    rc2Dvector d0(1.0, 1.0);
    rc2Dvector d1 = ar.affineToImage(d0);
    rcUTCheck(rfRealEq(d1.x(), 1, 0.0000001));
    rcUTCheck(rfRealEq(d1.y(), 1 + sqrt(2.0)*2, 0.0000001));
    rc2Dvector d2 = ar.imageToAffine(d0);
    rcUTCheck(rfRealEq(d2.y(), 0, 0.0000001));
    rcUTCheck(rfRealEq(d2.x(), 0, 0.0000001));
  }

  {
    rc2Dvector o(1.0, 1.0);
    rcRadian r(rcDegree(45.00));
    rcDPair sc(2.0, 2.0);
    rcAffineRectangle ar(o, r, sc, rcIPair(2, 2), rcDPair(0.5, 0.5));

    rcUTCheck(ar.xyScale() == sc);

    rcIRect bb = ar.boundingBox();
    rcUTCheck (bb.ll() == rcIPair(-1, 4));
    rcUTCheck (bb.ur() == rcIPair(4, -1));

    rc2Dvector d0(1.0, 1.0);
    rc2Dvector d1 = ar.affineToImage(d0);
    rcUTCheck(rfRealEq(d1.x(), 1, 0.0000001));
    rcUTCheck(rfRealEq(d1.y(), 1 + sqrt(2.0), 0.0000001));
    rc2Dvector d2 = ar.imageToAffine(d0);
    rcUTCheck(rfRealEq(d2.y(), 0.5, 0.0000001));
    rcUTCheck(rfRealEq(d2.x(), 0.5, 0.0000001));
  }

  {
    rc2Dvector o(6.0, 1.0);
    rcRadian r(rcDegree(30.00));
    rcDPair sc(3.0, 2.0);
    rcAffineRectangle ar(o, r, sc, rcIPair(3, 2), rcDPair(0.0, 0.0));

    rcIRect bb = ar.boundingBox();
    rcUTCheck (bb.ll() == rcIPair(5, 6));
    rcUTCheck (bb.ur() == rcIPair(10, 1));

    rcUTCheck(ar.xyScale() == sc);

    rc2Dvector d0(1.0, 1.0);
    rc2Dvector d1 = ar.affineToImage(d0);
    rcUTCheck(rfRealEq(d1.x(), 7.59807, 0.0001));
    rcUTCheck(rfRealEq(d1.y(), 4.23205, 0.0001));
    rc2Dvector d2 = ar.imageToAffine(rc2Dvector(5.00000, 2.73205));
    rcUTCheck(rfRealEq(d2.y(), 1.0, 0.0001));
    rcUTCheck(rfRealEq(d2.x(), 0.0, 0.0001));
  }

  {
    rc2Dvector o(6.0, 1.0);
    rcRadian r(rcDegree(30.00));
    rcDPair sc(3.0, 2.0);
    rcAffineRectangle ar(o, r, sc, rcIPair(3, 2), rcDPair(0.5, 0.5));

    rcIRect bb = ar.boundingBox();
    rcUTCheck (bb.ll() == rcIPair(4, 4));
    rcUTCheck (bb.ur() == rcIPair(9, -1));

    rcUTCheck(ar.xyScale() == sc);

    rc2Dvector d0(1.0, 1.0);
    rc2Dvector d1 = ar.affineToImage(d0);
    rcUTCheck(rfRealEq(d1.x(), 6.79904, 0.0001));
    rcUTCheck(rfRealEq(d1.y(), 2.61603, 0.0001));
    rc2Dvector d2 = ar.imageToAffine(rc2Dvector(6.79904, 2.61603));
    rcUTCheck(rfRealEq(d2.y(), 1.0, 0.0001));
    rcUTCheck(rfRealEq(d2.x(), 1.0, 0.0001));
  }

  {
    rc2Dvector o (0.0, 0.0);
    rcRadian r (rcDegree (45.00));
    rcDPair sc (1.0, 1.0);
    rcAffineRectangle ar (o, r, sc, rcIPair (1, 1), rcDPair(0.0, 0.0));

    rcIRect bb = ar.boundingBox();
    rcUTCheck (bb.ll() == rcIPair (-1, 3));
    rcUTCheck (bb.ur() == rcIPair(2, 0));

    rcUTCheck(ar.xyScale() == sc);

    rcUTCheck (rfRealEq (ar.matrix().element(0,0), 0.707107, 0.000001));
    rcUTCheck (rfRealEq (ar.matrix().element(0,1), - 0.707107, 0.000001));
    rcUTCheck (rfRealEq (ar.matrix().element(1, 0), 0.707107, 0.000001));
    rcUTCheck (rfRealEq (ar.matrix().element(1, 1), 0.707107, 0.000001));
    rcUTCheck (ar.origin().x() == 0);
    rcUTCheck (ar.origin().y() == 0);

    rc2Dvector d0 (1.0, 1.0);
    rc2Dvector d1 = ar.mapPoint (d0);
    rcUTCheck (rfRealEq (d1.x(), 0, 0.0000001));
    rcUTCheck (rfRealEq (d1.y(), sqrt(2.0), 0.0000001));
    rc2Dvector d2 = ar.invMapPoint (d0);
    rcUTCheck (rfRealEq (d2.y(), 0, 0.0000001));
    rcUTCheck (rfRealEq (d2.x(), sqrt(2.0), 0.0000001));
  }

  {
    double ah = 10, aw = 30;
    rc2Dvector o (21.0, 12.0);
    rcRadian r (rcDegree (0.00));
    rcDPair sc (aw, ah);
    rcDPair org (0.0, 0.0);
    rcAffineRectangle ar2 (o, r, sc, rcIPair (10, 30), org);

    rc2Dvector ul (0.0, 1.0);
    rc2Dvector ur (1.0, 1.0);
    rc2Dvector ll (0.0, 0.0);
    rc2Dvector lr (1.0, 0.0);


    rcIPair steps (3, 3);
    rcAffineRectangle art = ar2.expand (steps);

    rcUTCheck(rfRealEq(ar2.affineToImage(ul).x() - 
		       art.affineToImage(ul).x(), 9, 0.01));
    rcUTCheck(rfRealEq(ar2.affineToImage(ul).y() - 
		       art.affineToImage(ul).y(), -1, 0.01));
    rcUTCheck(rfRealEq(ar2.affineToImage(ur).x() - 
		       art.affineToImage(ur).x(), -9, 0.01));
    rcUTCheck(rfRealEq(ar2.affineToImage(ur).y() - 
		       art.affineToImage(ur).y(), -1, 0.01));

    rcUTCheck(rfRealEq(ar2.affineToImage(ll).x() - 
		       art.affineToImage(ll).x(), 9, 0.01));
    rcUTCheck(rfRealEq(ar2.affineToImage(ll).y() - 
		       art.affineToImage(ll).y(), 1, 0.01));
    rcUTCheck(rfRealEq(ar2.affineToImage(lr).x() - 
		       art.affineToImage(lr).x(), -9, 0.01));
    rcUTCheck(rfRealEq(ar2.affineToImage(lr).y() - 
		       art.affineToImage(lr).y(), 1, 0.01));
  }

  {
    for (double d = 0.0; d <= rk2PI; d+= rk2PI/360.0)
      {
	double ah = 10, aw = 30;
	rc2Dvector o (210.0, 120.0);
	rcRadian r (d);
	rcDPair sc (aw, ah);
	rcDPair org (0.0, 0.0);
	rcAffineRectangle ar2 (o, r, sc, rcIPair (10, 30), org);

	rc2Dvector ctr (0.5, 0.5); 
	// Center of Affine in Image Coordinates
	// And rotate it by our angle (CW)
	CGPoint mid = {(float)	ar2.affineToImage(ctr).x(), (float)	ar2.affineToImage(ctr).y() };
	CGAffineTransform va = CGAffineTransformRotate (CGAffineTransformMakeTranslation (mid.x, mid.y),
							    (float) r.basic ());
	// Now check rcAffine and CGAffine return the same point
	// First take a point in rcAffine (since it is tested) and map it to image coordinate
	// Then map these using the CGAffine to image coordinates. 

	// Map top left of the affine to image
	{
	  rc2Dvector atl (0.0, 0.0); 
	  CGPoint tlp = {(float)	ar2.affineToImage(atl).x(), (float)	ar2.affineToImage(atl).y() };

	  rcUTCheck(rfRealEq (tlp.x, (float) o.x(), 0.001f));
	  rcUTCheck(rfRealEq (tlp.y, (float) o.y(), 0.001f));

	  // Map top left of the CGAffine to the image
	  CGPoint gtl = {- aw/2.0, - ah/2.0};
	  CGPoint tl = CGPointApplyAffineTransform (gtl, va);

	  rcUTCheck(rfRealEq (tlp.x, tl.x, 0.001f));
	  rcUTCheck(rfRealEq (tlp.y, tl.y, 0.001f));
	}

	// Map bottom right of the affine to image
	{
	  rc2Dvector atl (1.0, 1.0); 
	  CGPoint tlp = {(float)	ar2.affineToImage(atl).x(), (float)	ar2.affineToImage(atl).y() };

	  // Map top left of the CGAffine to the image
	  CGPoint gtl = {aw/2.0, ah/2.0};
	  CGPoint tl = CGPointApplyAffineTransform (gtl, va);

	  rcUTCheck(rfRealEq (tlp.x, tl.x, 0.001f));
	  rcUTCheck(rfRealEq (tlp.y, tl.y, 0.001f));
	}

      }
    
  }

#if 0
  {
    double ah = 10, aw = 30;
    rc2Dvector o (21.0, 12.0);
    rcRadian r (rcDegree (0.00));
    rcDPair sc (aw, ah);
    rcAffineRectangle ar2 (o, r, sc, rcIPair (1, 1), rcDPair(0.5, 0.5));
    rcIRect bb2 = ar2.boundingBox();
    printf("ar2 bound: ul (%d, %d) ur (%d, %d) ll (%d, %d) lr (%d, %d)\n",
	   bb2.ul().x(), bb2.ul().y(), bb2.ur().x(), bb2.ur().y(),
	   bb2.ll().x(), bb2.ll().y(), bb2.lr().x(), bb2.lr().y());

    rc2Dvector ul (0.0, 1.0);
    rc2Dvector ur (1.0, 1.0);
    rc2Dvector ll (0.0, 0.0);
    rc2Dvector lr (1.0, 0.0);
    printf("ar2 M: ul (%f, %f) ur (%f, %f) ll (%f, %f) lr (%f, %f)\n",
	   ar2.affineToImage(ul).x(), ar2.affineToImage(ul).y(),
	   ar2.affineToImage(ur).x(), ar2.affineToImage(ur).y(),
	   ar2.affineToImage(ll).x(), ar2.affineToImage(ll).y(),
	   ar2.affineToImage(lr).x(), ar2.affineToImage(lr).y());

    rcRadian beta(atan2(ah, aw));
    rcRadian theta(rcDegree(8));
    rcRadian cum(beta + theta);
    double ratio = sin(beta.Double())/sin(cum.Double());

    rcDegree bd(beta), td(theta), cd(cum);
    printf("b %f t %f c %f ratio %f\n", bd.Double(), td.Double(),
	   cd.Double(), ratio);

    double aht = ah*ratio, awt = aw*ratio;

    rcDPair sct (awt, aht);
      
    rcAffineRectangle art (o, theta, sct, rcIPair (1, 1), rcDPair(0.5, 0.5));
    rcIRect bbt = art.boundingBox();

    printf("art bound: ul (%d, %d) ur (%d, %d) ll (%d, %d) lr (%d, %d)\n",
	   bbt.ul().x(), bbt.ul().y(), bbt.ur().x(), bbt.ur().y(),
	   bbt.ll().x(), bbt.ll().y(), bbt.lr().x(), bbt.lr().y());

    printf("art M: ul (%f, %f) ur (%f, %f) ll (%f, %f) lr (%f, %f)\n",
	   art.affineToImage(ul).x(), art.affineToImage(ul).y(),
	   art.affineToImage(ur).x(), art.affineToImage(ur).y(),
	   art.affineToImage(ll).x(), art.affineToImage(ll).y(),
	   art.affineToImage(lr).x(), art.affineToImage(lr).y());
    
  }
#endif

}

void UT_affine::getpixelloc()
{
  {
    rc2Dvector o(2.0, 1.0);
    rcRadian r(rcDegree(0.00));
    rcDPair sc(3.0, 3.0);
    rcIPair sz(3, 3);
    rcAffineRectangle ar(o, r, sc, sz, rcDPair(0.0, 0.0));

    rc2Dvector loc;

    for (int32 x = 0; x < sz.x(); x++)
      for (int32 y = 0; y < sz.y(); y++) {
	loc = ar.getPixelLoc(x, y);
	rcUTCheck(rfRealEq(loc.x(), o.x() + x, 0.0000001));
	rcUTCheck(rfRealEq(loc.y(), o.y() + y, 0.0000001));
      }

    for (double x = 0; x < (double)sz.x(); x += 1.0)
      for (double y = 0; y < (double)sz.y(); y += 1.0) {
	loc = ar.getPixelLoc(x, y);
	rcUTCheck(rfRealEq(loc.x(), o.x() + x, 0.0000001));
	rcUTCheck(rfRealEq(loc.y(), o.y() + y, 0.0000001));
      }
  }
  {
    rc2Dvector o(2.0, 1.0);
    rcRadian r(rcDegree(45.00));
    rcDPair sc(3.0, 3.0);
    rcIPair sz(3, 3);
    rcAffineRectangle ar(o, r, sc, sz, rcDPair(0.0, 0.0));
    double sq2 = sqrt(2.0);
    double expgpclX[3][3], expgpclY[3][3];

    expgpclX[0][0] = 0; expgpclX[0][1] = -1/sq2; expgpclX[0][2] = -sq2;
    expgpclX[1][0] = 1/sq2; expgpclX[1][1] = 0; expgpclX[1][2] = -1/sq2;
    expgpclX[2][0] = sq2; expgpclX[2][1] = 1/sq2; expgpclX[2][2] = 0;
    expgpclY[0][0] = 0; expgpclY[0][1] = 1/sq2; expgpclY[0][2] = sq2;
    expgpclY[1][0] = 1/sq2; expgpclY[1][1] = sq2; expgpclY[1][2] = (3*sq2)/2;
    expgpclY[2][0] = sq2; expgpclY[2][1] = (3*sq2)/2; expgpclY[2][2] = 2*sq2;

    rc2Dvector loc;

    for (int32 x = 0; x < sz.x(); x++)
      for (int32 y = 0; y < sz.y(); y++) {
	loc = ar.getPixelLoc(x, y);
	rcUTCheck(rfRealEq(loc.x(), o.x() + expgpclX[x][y], 0.0000001));
	rcUTCheck(rfRealEq(loc.y(), o.y() + expgpclY[x][y], 0.0000001));
      }

    double expgplX[3][3], expgplY[3][3];
    expgplX[0][0] = 0; expgplX[0][1] = -1/sq2; expgplX[0][2] = -sq2;
    expgplX[1][0] = 1/sq2; expgplX[1][1] = 0; expgplX[1][2] = -1/sq2;
    expgplX[2][0] = sq2; expgplX[2][1] = 1/sq2; expgplX[2][2] = 0;
    expgplY[0][0] = 0; expgplY[0][1] = 1/sq2; expgplY[0][2] = sq2;
    expgplY[1][0] = 1/sq2; expgplY[1][1] = sq2; expgplY[1][2] = (3*sq2)/2;
    expgplY[2][0] = sq2; expgplY[2][1] = (3*sq2)/2; expgplY[2][2] = 2*sq2;

    for (double x = 0; x < (double)sz.x(); x += 1.0)
      for (double y = 0; y < (double)sz.y(); y += 1.0) {
	loc = ar.getPixelLoc(x, y);
	rcUTCheck(rfRealEq(loc.x(), o.x() + expgplX[(int)x][(int)y], 0.0000001));
	rcUTCheck(rfRealEq(loc.y(), o.y() + expgplY[(int)x][(int)y], 0.0000001));
      }

    loc = ar.getPixelLoc(1.93, 1.22);
    rcUTCheck(rfRealEq(loc.x(), o.x() + .50204581, 0.0000001));
    rcUTCheck(rfRealEq(loc.y(), o.y() + 2.22738636, 0.0000001));
  }
}

void UT_affine::window()
{
  rc2Dvector o(2.0, 1.0);
  rcRadian r(rcDegree(0.00));
  rcDPair sc(3.0, 3.0);
  rcIPair sz(3, 3);
  rcAffineRectangle ar(o, r, sc, sz, rcDPair(0.0, 0.0));

  rcAffineWindow parent(ar);
  
  rcUTCheck(parent.ar() == ar);
  rcUTCheck(ar.boundingBox().width() + (int32)o.x() ==
	    parent.rcBound().width());
  rcUTCheck(ar.boundingBox().height() + (int32)o.y() ==
	    parent.rcBound().height());
  rcUTCheck(dynamic_cast<const rcDefGeneratePixel*>(&parent.pixelGenerator()) !=
	    (const rcDefGeneratePixel*)0);
  rcUTCheck(dynamic_cast<const rcDefGenerateImage*>(&parent.imageGenerator()) !=
	    (const rcDefGenerateImage*)0);

  rcAffineWindow child(parent);
  
  rcUTCheck(child.ar() == ar);
  rcUTCheck(ar.boundingBox().width() + (int32)o.x() ==
	    child.rcBound().width());
  rcUTCheck(ar.boundingBox().height() + (int32)o.y() ==
	    child.rcBound().height());

  rcUTCheck(child.frameBuf() == parent.frameBuf());
  rcUTCheck(&(child.pixelGenerator()) == &(parent.pixelGenerator()));
  rcUTCheck(&(child.imageGenerator()) == &(parent.imageGenerator()));

  rcWindow win((sz.x() + (int32)o.x() + 1), (sz.y() + (int32)o.y() + 1));
  
  rcAffineWindow wchild(win, ar);

  rcUTCheck(parent.ar() == ar);
  rcUTCheck(wchild.isWithin(ar.boundingBox()));

  rcUTCheck(wchild.frameBuf() != parent.frameBuf());
  wchild = parent;
  rcUTCheck(wchild.frameBuf() == parent.frameBuf());
  rcUTCheck(&(wchild.pixelGenerator()) == &(parent.pixelGenerator()));
  rcUTCheck(&(wchild.imageGenerator()) == &(parent.imageGenerator()));
}
