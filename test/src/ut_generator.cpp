/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      ut_generator.cpp
 *   Creation Date  09/09/2003
 *   Author         Peter Roberts
 *
 ***************************************************************************/

#include <ut_generator.h>
#include <rc_affinewindow.h>
#include <rc_gen_movie_file.h>
#include <rc_videocache.h>

static const uint32 savedCount = 10;
static const double savedAngles[savedCount] = { 0, 1, 13, 45, 90, 97, 180, 199, 270, 287 };

UT_generator::UT_generator(std::string nearestName, std::string bilinName, std::string bicubeName)
    : _nearestName(nearestName), _bilinName(bilinName), _bicubeName(bicubeName)
{
  rmAssert(!_nearestName.empty());
  rmAssert(!_bilinName.empty());
  rmAssert(!_bicubeName.empty());
}

UT_generator::~UT_generator()
{
  printSuccessMessage( "Image generating tests", mErrors );
}

uint32 UT_generator::run()
{
  genImages();	
 // project ();
//  def();
//  bilinear();
//  cubic();


#ifdef NDEBUG
  rotate();
#endif
  return mErrors;
}


// +/- with 128 as mid
static double functional (int32 size, const double &x,const double &y)
{
  double cx(size / 2),cy(size / 2);
  cx = x - cx; cy = y - cy;
  if (fabs (cx) <= 0.5 || fabs (cy) <= 0.5) return 8;
  else return 1;
}

static double distance (int32 size, const double &x,const double &y)
{
  static const double cx(size / 2),cy(size / 2);
  double f =  sqrt(rmSquare(x - cx) + rmSquare(y - cy));

  return f;
}


void UT_generator::project()
{
  int32 oldErrors = mErrors;
  rcWindow model(31, 31);

  const double testAngles[] = { 0., 1., 13., 45., 90., 97., 180., 199., 270., 287., 359. };
  const uint32 count = rmDim(testAngles);


  /* Create the synthetic image. 
   * TBD: Use MathModel stuff
   * Value is for the center of the pixel at (0.5, 0.5)
   * Round value
   */

  for (int32 j = 0; j < model.height(); j++)
    for (int32 i = 0; i < model.width(); i++)
      {
	double p = functional (model.width(), i + 0.5, j + 0.5);
	model.setPixel (i, j, uint32 (p));
      }

  /* Now calculate how large a window needs to be to be able to handle
   * all the rotated cases.  since 45 degress is the worst case,
   * figure out its size requirements.
   */
  rc2Dvector o(15.5, 15.5);
  rcDPair sc(13.0, 13.0);
  rcIPair sz(13, 13);
    
  /* Final setup step is to create geometry necessary to make affine
   * window that will be used to create rotated versions of model.
   */
  rcMatrix_2d mV (1.0, 0.0, 0.0, 0.0), mH (0.0, 0.0, 0.0, 1.0);
  vector<vector <float> >Vw (3);
  vector<vector <float> >Hw (3);

  for (uint32 degIndex = 0; degIndex < count; degIndex++) {
    rcDegree d(testAngles[degIndex]);
    rcRadian r(d);

    rcAffineRectangle ar(o, r, sc, sz);
    rcCubicGeneratePixel cpGen;
    rcLinGeneratePixel lpGen;
    rcWindow noMask;

    rcAffineWindow win(model, ar);

    rc2Xform xH, xV;

    for (int32 vv = 0; vv < 3; vv++)
      {
	Vw[vv].resize (ar.cannonicalSize().x());
	Hw[vv].resize (ar.cannonicalSize().y());
      }

    rcDefGenerateImage biGen (lpGen);
    rcDefGenerateImage cuGen (cpGen);

    for (int32 v = 0; v < 3; v++)
      { 
	if (v == 0)
	  {
	    xH.matrix (mH);xV.matrix (mV);
	    win.project (Vw[v], xV, noMask);
	    win.project (Hw[v], xH, noMask);
	  }

	if (v == 1)
	  {
	    xH.matrix (mH);xV.matrix (mV);
	    win.project (Vw[v], xV, noMask, &biGen);
	    win.project (Hw[v], xH, noMask, &biGen);
	  }
	if (v == 2)
	  {
	    xH.matrix (mH);xV.matrix (mV);
	    win.project (Vw[v], xV, noMask, &cuGen);
	    win.project (Hw[v], xH, noMask, &cuGen);
	  }
      }
    // Compute the projection of the underlying function (The closest 
    // thing to the correct)
    rc2Dvector step (1.0 / ar.cannonicalSize().x(), 1.0 / ar.cannonicalSize().y ());

    double z(0);
    vector<double> vProj (ar.cannonicalSize().x(), z);
    vector<double> hProj (ar.cannonicalSize().y(), z);

    // Walk through affine window in steps

    for (int32 j = 0; j < ar.cannonicalSize().y(); j++)
      {
	for (int32 i = 0; i < ar.cannonicalSize().x(); i++) 
	  {
	    rc2Dvector p (i * step.x() + step.x() / 2,
			  j * step.y() + step.y() / 2);
	    p = ar.affineToImage (p);
	    double v = functional (model.width(), p.x() + 0.5, p.y() + 0.5);
	    vProj[i] += v;
	    hProj[j] += v;
	  }
      }

    double eH[3], eV[3];

    for (int32 v = 0; v < 3; v++)
      {
	for (uint32 p = 0; p < Vw[v].size(); p++) vProj[p] = (double) (uint32 (vProj[p] + 0.5));
	for (uint32 p = 0; p < Hw[v].size(); p++) hProj[p] = (double) (uint32 (hProj[p] + 0.5));

	eV[v] = eH[v] = 0.0;

	for (uint32 p = 0; p < Vw[v].size(); p++) eV[v] += (Vw[v][p] - vProj[p]) / vProj[p];
	for (uint32 p = 0; p < Hw[v].size(); p++) eH[v] += (Hw[v][p] - hProj[p]) / hProj[p];

	// Was 1.0 for pure projection. Increased it to 8.0 for variance. TBD: analyze
	rcUTCheck (sqrt (fabs (eV[v] / rmSquare (Vw[v].size()))) < 3.0);
	rcUTCheck (sqrt (fabs (eH[v] / rmSquare (Hw[v].size()))) < 3.0);
      }
  }


  // Draw a distance in the middle and check to see if projections at all angles are about equal
  /* Create the synthetic image. 
   * TBD: Use MathModel stuff
   * For now use periodicity of 1/2
   * Value is for the center of the pixel at (0.5, 0.5)
   * Round value
   */
  for (int32 j = 0; j < model.height(); j++)
    for (int32 i = 0; i < model.width(); i++)
      {
	double p = distance (model.width(), i + 0.5, j + 0.5);
	model.setPixel (i, j, uint32 (p + 0.5));
      }

  for (uint32 degIndex = 0; degIndex < count; degIndex++) {
    rcDegree d(testAngles[degIndex]);
    rcRadian r(d);

    rcAffineRectangle ar(o, r, sc, sz);
    rcCubicGeneratePixel cpGen;
    rcLinGeneratePixel lpGen;
    rcWindow noMask;    

    rcAffineWindow win(model, ar);

    rc2Xform xH, xV;


    for (int32 vv = 0; vv < 3; vv++)
      {
	Vw[vv].resize (ar.cannonicalSize().x());
	Hw[vv].resize (ar.cannonicalSize().y());
      }

    rcDefGenerateImage biGen (lpGen);
    rcDefGenerateImage cuGen (cpGen);

    for (int32 v = 0; v < 3; v++)
      { 
	if (v == 0)
	  {
	    xH.matrix (mH);xV.matrix (mV);
	    win.project (Vw[v], xV, noMask);
	    win.project (Hw[v], xH, noMask);
	  }

	if (v == 1)
	  {
	    xH.matrix (mH);xV.matrix (mV);
	    win.project (Vw[v], xV, noMask, &biGen);
	    win.project (Hw[v], xH, noMask, &biGen);
	  }
	if (v == 2)
	  {
	    xH.matrix (mH);xV.matrix (mV);
	    win.project (Vw[v], xV, noMask, &cuGen);
	    win.project (Hw[v], xH, noMask, &cuGen);
	  }
      }
    // Compute the projection of the underlying function (The closest 
    // thing to the correct)
    rc2Dvector step (1.0 / ar.cannonicalSize().x(), 1.0 / ar.cannonicalSize().y ());

    double z(0);
    vector<double> vProj (ar.cannonicalSize().x(), z);
    vector<double> hProj (ar.cannonicalSize().y(), z);

    // Walk through affine window in steps
    for (int32 j = 0; j < ar.cannonicalSize().y(); j++)
      {
	//	cout << endl;
	for (int32 i = 0; i < ar.cannonicalSize().x(); i++) 
	  {
	    rc2Dvector p (i * step.x(), j * step.y());
	    p = ar.affineToImage (p);
	    double v = distance (model.width(), p.x() + 0.5, p.y() + 0.5);
	    vProj[i] += v;
	    hProj[j] += v;

	    // cout << setprecision (1) << (int32)(v + 0.5) << " ";

	  }
      }


    double eH[3], eV[3];

    for (int32 v = 0; v < 3; v++)
      {
	for (uint32 p = 0; p < Vw[v].size(); p++) vProj[p] = (double) (uint32 (vProj[p] + 0.5));
	for (uint32 p = 0; p < Hw[v].size(); p++) hProj[p] = (double) (uint32 (hProj[p] + 0.5));

	eV[v] = eH[v] = 0.0;

	for (uint32 p = 0; p < Vw[v].size(); p++) eV[v] += (Vw[v][p] - vProj[p]) / vProj[p];
	for (uint32 p = 0; p < Hw[v].size(); p++) eH[v] += (Hw[v][p] - hProj[p]) / hProj[p];

	rcUTCheck (sqrt (fabs (eV[v] / rmSquare (Vw[v].size()))) <= 1.2);
	rcUTCheck (sqrt (fabs (eH[v] / rmSquare (Hw[v].size()))) <= 1.2);
      }
  }
  
  printSuccessMessage( "Projection", mErrors-oldErrors );
}

void UT_generator::def()
{
  int32 oldErrors = mErrors;
  rc2Dvector o(0.0, 0.0);
  rcRadian r(rcDegree(0.00));
  rcDPair sc(3.0, 3.0);
  rcIPair sz(3, 3);
  rcAffineRectangle ar(o, r, sc, sz, rcDPair(0.0, 0.0));
  rcDefGeneratePixel pGen;

  rcAffineWindow win8(ar);
  win8.pixelGenerator(pGen);

  for (int32 y = 0; y < sz.y(); y++)
    for (int32 x = 0; x < sz.x(); x++)
      win8.setPixel(x, y, y*sz.y() + x);

  
  for (int32 y = 0; y < sz.y() - 1; y++)
    for (int32 x = 0; x < sz.x() - 1; x++)
      rcUNITTEST_ASSERT((double)win8.getPixel(x, y) ==
			 win8.genPixel(x, y));

  
  rcAffineWindow winD(ar, rcPixelDouble);
  winD.pixelGenerator(pGen);

  for (int32 y = 0; y < sz.y(); y++)
    for (int32 x = 0; x < sz.x(); x++)
      winD.setDoublePixel(x, y, y*sz.y() + x);

  for (int32 y = 0; y < sz.y(); y++)
    for (int32 x = 0; x < sz.x(); x++)
      rcUNITTEST_ASSERT(winD.getDoublePixel(x, y) ==
			 winD.genPixel(x, y));

  double xloc = 0.4999999, yloc = 0.4999999;
  int32 xILoc = 0, yILoc = 0;
  rcUNITTEST_ASSERT(pGen.genPixel(&win8, xloc, yloc) ==
		     (double)win8.getPixel(xILoc, yILoc));
  rcUNITTEST_ASSERT(pGen.genPixel(&winD, xloc, yloc) ==
		     winD.getDoublePixel(xILoc, yILoc));

  xloc = 2.4999999, yloc = 2.4999999;
  xILoc = 2, yILoc = 2;
  rcUNITTEST_ASSERT(pGen.genPixel(&win8, xloc, yloc) ==
		     (double)win8.getPixel(xILoc, yILoc));
  rcUNITTEST_ASSERT(pGen.genPixel(&winD, xloc, yloc) ==
		     winD.getDoublePixel(xILoc, yILoc));

  printSuccessMessage( "rcDefGeneratePixel", mErrors-oldErrors );
}

#define cmPrintImage(a) { \
  for (int i = 0; i < (a).height(); i++) \
  { \
     fprintf (stderr, "\n"); \
        for (int j = 0; j < (a).width(); j++) \
           fprintf (stderr, "%2X ", (a).getPixel (j, i)); \
  } \
  fprintf (stderr, "\n");}

#define cmPrintDoubleImage(a) { \
  for (int i = 0; i < (a).height(); i++) \
  { \
     fprintf (stderr, "\n"); \
        for (int j = 0; j < (a).width(); j++) \
           fprintf (stderr, "%2f ", (a).getDoublePixel (j, i)); \
  } \
  fprintf (stderr, "\n");}

void UT_generator::bilinear()
{
  int32 oldErrors = mErrors;
  rc2Dvector o(0.0, 0.0);
  rcRadian r(rcDegree(0.00));
  rcDPair sc(3.0, 3.0);
  rcIPair sz(3, 3);
  rcAffineRectangle ar(o, r, sc, sz, rcDPair(0.0, 0.0));
  rcLinGeneratePixel pGen;

  rcAffineWindow win8(ar);
  win8.pixelGenerator(pGen);

  for (int32 y = 0; y < sz.y(); y++)
    for (int32 x = 0; x < sz.x(); x++)
      win8.setPixel(x, y, y*sz.y() + x);
  
  for (int32 y = 0; y < sz.y(); y++)
    for (int32 x = 0; x < sz.x(); x++)
      rcUNITTEST_ASSERT((double)win8.getPixel(x, y) ==
			 win8.genPixel(x, y));
  
  rcAffineWindow winD(ar, rcPixelDouble);
  winD.pixelGenerator(pGen);

  for (int32 y = 0; y < sz.y(); y++)
    for (int32 x = 0; x < sz.x(); x++)
      winD.setDoublePixel(x, y, y*sz.y() + x);

  for (int32 y = 0; y < sz.y(); y++)
    for (int32 x = 0; x < sz.x(); x++)
      rcUNITTEST_ASSERT(winD.getDoublePixel(x, y) ==
			 winD.genPixel(x, y));

  double xloc = 0.5, yloc = 0.5;
  double expValue = (0.0*0.5 + 1.0*0.5)*0.5 + (3.0*0.5 + 4.0*0.5)*0.5;

  rcUNITTEST_ASSERT(rfRealEq(pGen.genPixel(&win8, xloc, yloc),expValue,0.0000001));
  rcUNITTEST_ASSERT(rfRealEq(pGen.genPixel(&winD, xloc, yloc),expValue,0.0000001));

  xloc = 0.9, yloc = 0.2;
  expValue = (0.0*0.1 + 1.0*0.9)*0.8 + (3.0*0.1 + 4.0*0.9)*0.2;

  rcUNITTEST_ASSERT(rfRealEq(pGen.genPixel(&win8, xloc, yloc),expValue,0.0000001));
  rcUNITTEST_ASSERT(rfRealEq(pGen.genPixel(&winD, xloc, yloc),expValue,0.0000001));

  printSuccessMessage( "rcLinGeneratePixel", mErrors-oldErrors );
}

void UT_generator::cubic()
{
  int32 oldErrors = mErrors;
  rc2Dvector o(0.0, 0.0);
  rcRadian r(rcDegree(0.00));
  rcDPair sc(3.0, 3.0);
  rcIPair sz(3, 3);
  rcAffineRectangle ar(o, r, sc, sz, rcDPair(0.0, 0.0));
  rcCubicGeneratePixel pGen;

  rcAffineWindow win8(ar);
  win8.pixelGenerator(pGen);

  for (int32 y = 0; y < win8.height(); y++)
    for (int32 x = 0; x < win8.width(); x++)
      win8.setPixel(x, y, y*sz.y() + x);
  
  for (int32 y = 0; y < sz.y(); y++)
    for (int32 x = 0; x < sz.x(); x++)
      rcUNITTEST_ASSERT((double)win8.getPixel(x, y) ==
			 win8.genPixel(x, y));
  
  rcAffineWindow winD(ar, rcPixelDouble);
  winD.pixelGenerator(pGen);
  pGen.clear();

  for (int32 y = 0; y < win8.height(); y++)
    for (int32 x = 0; x < win8.width(); x++)
      winD.setDoublePixel(x, y, y*sz.y() + x);

  for (int32 y = 0; y < sz.y(); y++)
    for (int32 x = 0; x < sz.x(); x++)
      rcUNITTEST_ASSERT(winD.getDoublePixel(x, y) ==
			 winD.genPixel(x, y));

  double xloc = 0.5, yloc = 0.5;
  double expValue = (0.0*0.5 + 1.0*0.5)*0.5 + (3.0*0.5 + 4.0*0.5)*0.5;

  pGen.clear();
  rcUNITTEST_ASSERT(rfRealEq(pGen.genPixel(&win8, xloc, yloc),expValue,0.0000001));

  pGen.clear();
  rcUNITTEST_ASSERT(rfRealEq(pGen.genPixel(&winD, xloc, yloc),expValue,0.0000001));

  xloc = 0.9, yloc = 0.2;
  expValue = (0.0*0.1 + 1.0*0.9)*0.8 + (3.0*0.1 + 4.0*0.9)*0.2;

  pGen.clear();
  rcUNITTEST_ASSERT(rfRealEq(pGen.genPixel(&win8, xloc, yloc),expValue,0.0000001));

  pGen.clear();
  rcUNITTEST_ASSERT(rfRealEq(pGen.genPixel(&winD, xloc, yloc),expValue,0.0000001));
  printSuccessMessage( "rcCubicGeneratePixel", mErrors-oldErrors );
}

bool imgEqual(rcWindow& a, rcWindow& b, bool verbose)
{
  if (a.width() != b.width() || a.height() != b.height()
      || (int)a.depth() != (int)b.depth()) {
    if (verbose)
      fprintf(stderr, "Img Diff@ %d x %d != %d x %d or depth %d != %d\n",
	      a.width(), b.width(), a.height(), b.height(), a.depth(), b.depth());
    return false;
  }

  for (int32 y = 0; y < a.height(); y++)
    for (int32 x = 0; x < a.width(); x++) {
      uint32 ap = a.getPixel(x, y);
      uint32 bp = b.getPixel(x, y);
      uint32 diff = (ap > bp) ? ap - bp : bp - ap;
      if (diff > 1) {
	if (verbose)
	  fprintf(stderr, "Img Diff@ (%d, %d) %d != %d\n",
		  x, y, a.getPixel(x, y), b.getPixel(x, y));
	return false;
      }
    }
  return true;
}


void UT_generator::rotate()
{
  int32 oldErrors = mErrors;
  rcVideoCache *nearP, *bilinearP, *cubicP;

  nearP = rcVideoCache::rcVideoCacheCtor(_nearestName, 10, true, false);
  if (!nearP || nearP->frameCount() != savedCount) {
    rcUNITTEST_ASSERT(nearP != 0);
    if (nearP) rcUNITTEST_ASSERT(nearP->frameCount() == savedCount);
    return;
  }
  rcUNITTEST_ASSERT( nearP->isValid() );
  
  bilinearP = rcVideoCache::rcVideoCacheCtor(_bilinName, 10, true, false);
  if (!bilinearP || bilinearP->frameCount() != savedCount) {
    rcUNITTEST_ASSERT(bilinearP != 0);
    if (bilinearP) rcUNITTEST_ASSERT(bilinearP->frameCount() == savedCount);
    return;
  }
  rcUNITTEST_ASSERT( bilinearP->isValid() );
  
  cubicP = rcVideoCache::rcVideoCacheCtor(_bicubeName, 10, true, false);
  if (!cubicP || cubicP->frameCount() != savedCount) {
    rcUNITTEST_ASSERT(cubicP != 0);
    if (cubicP) rcUNITTEST_ASSERT(cubicP->frameCount() == savedCount);
    return;
  }
  rcUNITTEST_ASSERT( cubicP->isValid() );
   
  rcVideoCache &nearMovie(*nearP), &bilinearMovie(*bilinearP), &cubicMovie(*cubicP);

  rcWindow model(101, 101);

  /* Create the synthetic image. A series of four increasing wide
   * vertical lines against a white background.
   */
  model.setAllPixels(255);
    
  rcWindow sq(model, 35, 40, 1, 21);
  sq.setAllPixels(0);
    
  sq = rcWindow(model, 43, 40, 2, 21);
  sq.setAllPixels(0);
    
  sq = rcWindow(model, 52, 40, 3, 21);
  sq.setAllPixels(0);
    
  sq = rcWindow(model, 62, 40, 4, 21);
  sq.setAllPixels(0);

  /* Now calculate how large a window needs to be to be able to handle
   * all the rotated cases.  since 45 degress is the worst case,
   * figure out its size requirements.
   */
  rc2Dvector o(50.0, 50.0);
  rcDPair sc(31.0, 31.0);
  rcIPair sz(31, 31);

  rcIRect bnd;
  {
    rcRadian r(rcDegree(45.0));
    rcAffineRectangle ar(o, r, sc, sz, rcDPair(0.5, 0.5));
    bnd = ar.boundingBox();
  }
    
  /* Final setup step is to create geometry necessary to make affine
   * window that will be used to create rotated versions of model.
   * Note that the origin is offset by (-0.5, -0.5).  This correction
   * is required because the boundingBox() fct. adds strips of pixels
   * to both the bottom and the right.
   *
   * Why the added row and column? boundingBox() takes into account
   * the extra data required to do interpolation.
   */
  rcIPair bndSz(bnd.size().x(), bnd.size().y());
  rcDPair bndSc(bndSz.x(), bndSz.y());
  rc2Dvector oRot(o.x()-0.5, o.y()-0.5);

  for (uint32 degIndex = 0; degIndex < savedCount; degIndex++) {
    rcDegree d(savedAngles[degIndex]);
    rcRadian r(d);
    rcAffineRectangle ar(oRot, r, bndSc, bndSz, rcDPair(0.5, 0.5));
    rcCubicGeneratePixel cpGen;
    rcLinGeneratePixel lpGen;
    rcDefGenerateImage ciGen(cpGen), biGen(lpGen);
    rcAffineWindow win(model, ar);
    rcSharedFrameBufPtr fbufp = 0;
    rcWindow actWin(bnd.size());
    win.genImage(actWin, bnd);
    rcVideoCacheStatus s1 = nearMovie.getFrame(degIndex, fbufp);
    if (s1 != eVideoCacheStatusOK) {
      rcUNITTEST_ASSERT(s1 == eVideoCacheStatusOK);
      return;
    }
    rcWindow expWin(fbufp);
    rcUNITTEST_ASSERT(imgEqual(actWin, expWin, 1));

    win.genImage(actWin, bnd, &biGen);
    s1 = bilinearMovie.getFrame(degIndex, expWin.frameBuf());
    if (s1 != eVideoCacheStatusOK) {
      rcUNITTEST_ASSERT(s1 == eVideoCacheStatusOK);
      return;
    }
    rcUNITTEST_ASSERT(imgEqual(actWin, expWin, 1));

    win.genImage(actWin, bnd, &ciGen);
    s1 = cubicMovie.getFrame(degIndex, expWin.frameBuf());
    if (s1 != eVideoCacheStatusOK) {
      rcUNITTEST_ASSERT(s1 == eVideoCacheStatusOK);
      return;
    }
    rcUNITTEST_ASSERT(imgEqual(actWin, expWin, 1));
  }
  
  printSuccessMessage( "Image rotation", mErrors-oldErrors );
}

/* genImages - test synthetic movie generation
 */
void UT_generator::genImages()
{
  int32 oldErrors = mErrors;
  rcWindow model(101, 101);

  /* Create the synthetic image. A series of four increasing wide
   * vertical lines against a white background.
   */
  model.setAllPixels(255);
    
  rcWindow sq(model, 35, 40, 1, 21);
  sq.setAllPixels(0);
    
  sq = rcWindow(model, 43, 40, 2, 21);
  sq.setAllPixels(0);
    
  sq = rcWindow(model, 52, 40, 3, 21);
  sq.setAllPixels(0);
    
  sq = rcWindow(model, 62, 40, 4, 21);
  sq.setAllPixels(0);

  /* Now calculate how large a window needs to be to be able to handle
   * all the rotated cases.  since 45 degress is the worst case,
   * figure out its size requirements.
   */
  rc2Dvector o(50.0, 50.0);
  rcDPair sc(31.0, 31.0);
  rcIPair sz(31, 31);

  rcIRect bnd;
  {
    rcRadian r(rcDegree(45.0));
    rcAffineRectangle ar(o, r, sc, sz, rcDPair(0.5, 0.5));
    bnd = ar.boundingBox();
  }
    
  /* Final setup step is to create geometry necessary to make affine
   * window that will be used to create rotated versions of model.
   * Note that the origin is offset by (-0.5, -0.5).  This correction
   * is required because the boundingBox() fct. adds strips of pixels
   * to both the bottom and the right.
   *
   * Why the added row and column? boundingBox() takes into account
   * the extra data required to do interpolation.
   */
  rcIPair bndSz(bnd.size().x(), bnd.size().y());
  rcDPair bndSc(bndSz.x(), bndSz.y());
  rc2Dvector oRot(o.x()-0.5, o.y()-0.5);
  
  std::string nearName = makeTmpName("nearest_rot.rfymov");
  std::string biName = makeTmpName("bilin_rot.rfymov");
  std::string cubicName = makeTmpName("bicube_rot.rfymov");
  std::string convName = makeTmpName("cnvHdr.rfymov");
  std::string camName = makeTmpName("camHdr.rfymov");
  std::string expName = makeTmpName("expHdr.rfymov");
   
  const char* creator = "UT_generator";
  const movieFormatRev rev = movieFormatRevLatest;
  const movieOriginType origin = movieOriginSynthetic;
  
  rcGenMovieFileError error = eGenMovieFileErrorOK;

  rcGenMovieFile nearestMovie(nearName, origin, creator,
                              rev, true, 1/30.0f);
  rcGenMovieFile biMovie(biName, origin, creator,
                         rev, true, 1/30.0f);
  rcGenMovieFile cubicMovie(cubicName, origin, creator,
                            rev, true, 1/30.0f);
  rcGenMovieFile convMovie(convName, origin, creator,
                           rev, true, 1/30.0f);
  rcGenMovieFile camMovie(camName, origin, creator,
                          rev, true, 1/30.0f);
  rcGenMovieFile expMovie(expName, origin, creator,
                          rev, true, 1/30.0f);
  
  rcUNITTEST_ASSERT( nearestMovie.valid() );
  rcUNITTEST_ASSERT( biMovie.valid() );
  rcUNITTEST_ASSERT( cubicMovie.valid() );
  rcUNITTEST_ASSERT( convMovie.valid() );
  rcUNITTEST_ASSERT( camMovie.valid() );
  rcUNITTEST_ASSERT( expMovie.valid() );
  
  for (uint32 degIndex = 0; degIndex < savedCount; degIndex++) {
    rcDegree d(savedAngles[degIndex]);
    rcRadian r(d);
    rcAffineRectangle ar(oRot, r, bndSc, bndSz, rcDPair(0.5, 0.5));
    rcCubicGeneratePixel cpGen;
    rcLinGeneratePixel lpGen;
    rcDefGenerateImage ciGen(cpGen), biGen(lpGen);
    rcAffineWindow win(model, ar);
    rcWindow rotNearestWin(bnd.size()), rotBilinearWin(bnd.size()), rotCubicWin(bnd.size());
    win.genImage(rotNearestWin, bnd);
    error = nearestMovie.addFrame(rotNearestWin);
    rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );
    win.genImage(rotBilinearWin, bnd, &biGen);
    error = biMovie.addFrame(rotBilinearWin);
    rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );
    win.genImage(rotCubicWin, bnd, &ciGen);
    error = cubicMovie.addFrame(rotCubicWin);
    rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );
    error = convMovie.addFrame(rotCubicWin);
    rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );
    error = camMovie.addFrame(rotCubicWin);
    rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );
    error = expMovie.addFrame(rotCubicWin);
    rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );
  }

  error = nearestMovie.flush();
  rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );
  error = biMovie.flush();
  rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );
  error = cubicMovie.flush();
  rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );

  // Add a conversion header
  rcMovieFileConvExt cnvHdr( 1, savedCount, 1, rcRect(0,0,128,256), movieChannelGreen,
                             0.133, false, creator );
  // Add two identical headers
  convMovie.addHeader( cnvHdr );
  convMovie.addHeader( cnvHdr );
  error = convMovie.flush();
  rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );

  // Add a camera header
  rcMovieFileCamExt camHdr;
  camHdr.mid( 2003 );
  camHdr.uid( 666 );
  camHdr.name( "SillyCam" );
  camMovie.addHeader( camHdr );
  error = camMovie.flush();
  rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );

  // Add an experiment header
  rcMovieFileExpExt expHdr;
  expHdr.lensMag( 10.0f );
  expHdr.otherMag( 0.45f );
  expHdr.temperature( 21.0f );
  expHdr.CO2( 0.035f );
  expHdr.O2( 21.0f );
  expHdr.title( "Generator test" );
  expHdr.userName( "Dr. Test" );
  expHdr.treatment1( "Crazy Treatment1" );
  expHdr.treatment2( "Crazy Treatment2" );
  expHdr.cellType( "C. elegans" );
  expHdr.imagingMode( "Phase Contrast" );
  expHdr.comment( "Testing, testing" );
  expMovie.addHeader( expHdr );
  error = expMovie.flush();
  rcUNITTEST_ASSERT( error == eGenMovieFileErrorOK );
  
  // Read generated movies
  // Nearest neighbor test
  {
      rcVideoCache* movie = rcVideoCache::rcVideoCacheCtor(nearName, 0, true, false, true, 0);
      rcUNITTEST_ASSERT( movie->isValid() );
      rcUNITTEST_ASSERT( movie->frameCount() == savedCount );
      rcUNITTEST_ASSERT( movie->frameWidth() == uint32(bndSz.x()) );
      rcUNITTEST_ASSERT( movie->frameHeight() == uint32(bndSz.y()) );
      const vector<rcMovieFileOrgExt>& orgs = movie->movieFileOrigins();
      rcUNITTEST_ASSERT( orgs.size() == 1 );
      if ( !orgs.empty() ) {
          rcMovieFileOrgExt org = orgs[0];
          rcUNITTEST_ASSERT( movie->frameCount() == org.frameCount() );
          rcUNITTEST_ASSERT( movie->frameWidth() ==  uint32(org.width()) );
          rcUNITTEST_ASSERT( movie->frameHeight() ==  uint32(org.height()) );
          rcUNITTEST_ASSERT( org.origin() == origin );
          rcUNITTEST_ASSERT( org.depth() == model.depth() );
          rcUNITTEST_ASSERT( org.rev() == rev );
          rcUNITTEST_ASSERT( !strcmp( creator, org.creatorName() ) );
          rcUNITTEST_ASSERT( org.id() >= 0 );
          rcUNITTEST_ASSERT( org.baseTime() > 0 );
      }
      const vector<rcMovieFileConvExt>& cnvs = movie->movieFileConversions();
      rcUNITTEST_ASSERT( cnvs.empty() );
      rcVideoCache::rcVideoCacheDtor( movie );
  }
  // Bilinear movie test
  {
      rcVideoCache* movie = rcVideoCache::rcVideoCacheCtor(biName, 0, true, false, true, 0);
      rcUNITTEST_ASSERT( movie->isValid() );
      rcUNITTEST_ASSERT( movie->frameCount() == savedCount );
      rcUNITTEST_ASSERT( movie->frameWidth() == uint32(bndSz.x()) );
      rcUNITTEST_ASSERT( movie->frameHeight() == uint32(bndSz.y()) );
      const vector<rcMovieFileOrgExt>& orgs = movie->movieFileOrigins();
      rcUNITTEST_ASSERT( orgs.size() == 1 );
      if ( !orgs.empty() ) {
          rcMovieFileOrgExt org = orgs[0];
          rcUNITTEST_ASSERT( movie->frameCount() == org.frameCount() );
          rcUNITTEST_ASSERT( movie->frameWidth() ==  uint32(org.width()) );
          rcUNITTEST_ASSERT( movie->frameHeight() ==  uint32(org.height()) );
          rcUNITTEST_ASSERT( org.origin() == origin );
          rcUNITTEST_ASSERT( org.depth() == model.depth() );
          rcUNITTEST_ASSERT( org.rev() == rev );
          rcUNITTEST_ASSERT( !strcmp( creator, org.creatorName() ) );
          rcUNITTEST_ASSERT( org.id() >= 0 );
          rcUNITTEST_ASSERT( org.baseTime() > 0 );
      }
      rcVideoCache::rcVideoCacheDtor( movie );
  }
  // Cubic movie test
  {
      rcVideoCache* movie = rcVideoCache::rcVideoCacheCtor(cubicName, 0, true, false, true, 0);
      rcUNITTEST_ASSERT( movie->isValid() );
      rcUNITTEST_ASSERT( movie->frameCount() == savedCount );
      rcUNITTEST_ASSERT( movie->frameWidth() == uint32(bndSz.x()) );
      rcUNITTEST_ASSERT( movie->frameHeight() == uint32(bndSz.y()) );
      const vector<rcMovieFileOrgExt>& orgs = movie->movieFileOrigins();
      rcUNITTEST_ASSERT( orgs.size() == 1 );
      if ( !orgs.empty() ) {
          rcMovieFileOrgExt org = orgs[0];
          rcUNITTEST_ASSERT( movie->frameCount() == org.frameCount() );
          rcUNITTEST_ASSERT( movie->frameWidth() ==  uint32(org.width()) );
          rcUNITTEST_ASSERT( movie->frameHeight() ==  uint32(org.height()) );
          rcUNITTEST_ASSERT( org.origin() == origin );
          rcUNITTEST_ASSERT( org.depth() == model.depth() );
          rcUNITTEST_ASSERT( org.rev() == rev );
          rcUNITTEST_ASSERT( !strcmp( creator, org.creatorName() ) );
          rcUNITTEST_ASSERT( org.id() >= 0 );
          rcUNITTEST_ASSERT( org.baseTime() > 0 );
      }
      rcVideoCache::rcVideoCacheDtor( movie );
  }
  // Conversion header test
  {
      rcVideoCache* movie = rcVideoCache::rcVideoCacheCtor(convName, 0, true, false, true, 0);
      rcUNITTEST_ASSERT( movie->isValid() );
      rcUNITTEST_ASSERT( movie->frameCount() == savedCount );
      rcUNITTEST_ASSERT( movie->frameWidth() == uint32(bndSz.x()) );
      rcUNITTEST_ASSERT( movie->frameHeight() == uint32(bndSz.y()) );
      const vector<rcMovieFileConvExt>& convs = movie->movieFileConversions();
      rcUNITTEST_ASSERT( convs.size() == 2 );
      for ( uint32 i = 0; i < convs.size(); ++i ) {
          rcMovieFileConvExt hd = convs[i];
          rcUNITTEST_ASSERT( hd.type() == movieExtensionCNV );
          rcUNITTEST_ASSERT( hd.frameCount() == cnvHdr.frameCount() );
          rcUNITTEST_ASSERT( hd.sample() == cnvHdr.sample() );
          rcUNITTEST_ASSERT( hd.cropRect() == cnvHdr.cropRect() );
          rcUNITTEST_ASSERT( hd.date() == cnvHdr.date() );
          rcUNITTEST_ASSERT( hd.channel() == cnvHdr.channel() );
          rcUNITTEST_ASSERT( hd.frameInterval() == cnvHdr.frameInterval() );
          rcUNITTEST_ASSERT( hd.rev() == cnvHdr.rev() );
          rcUNITTEST_ASSERT( !strcmp( hd.creatorName(), cnvHdr.creatorName() ) );
          rcUNITTEST_ASSERT( hd.pixelsReversed() == cnvHdr.pixelsReversed() );
          rcUNITTEST_ASSERT( hd.id() >= 0 );
      }
      rcVideoCache::rcVideoCacheDtor( movie );
  }

  // Camera header test
  {
      rcVideoCache* movie = rcVideoCache::rcVideoCacheCtor(camName, 0, true, false, true, 0);
      rcUNITTEST_ASSERT( movie->isValid() );
      rcUNITTEST_ASSERT( movie->frameCount() == savedCount );
      rcUNITTEST_ASSERT( movie->frameWidth() == uint32(bndSz.x()) );
      rcUNITTEST_ASSERT( movie->frameHeight() == uint32(bndSz.y()) );
      const vector<rcMovieFileCamExt>& cams = movie->movieFileCameras();
      rcUNITTEST_ASSERT( cams.size() == 1 );
      for ( uint32 i = 0; i < cams.size(); ++i ) {
          rcMovieFileCamExt hd = cams[i];
          rcUNITTEST_ASSERT( hd.type() == movieExtensionCAM );
          rcUNITTEST_ASSERT( hd.mid() == camHdr.mid() );
          rcUNITTEST_ASSERT( hd.uid() == camHdr.uid() );
          rcUNITTEST_ASSERT( hd.format() == rcUINT32_MAX );
          rcUNITTEST_ASSERT( hd.mode() == camHdr.mode() );
          rcUNITTEST_ASSERT( hd.frameRate() == camHdr.frameRate() );
          rcUNITTEST_ASSERT( hd.brightness() == camHdr.brightness() );
          rcUNITTEST_ASSERT( hd.exposure() == camHdr.exposure() );
          rcUNITTEST_ASSERT( hd.sharpness() == camHdr.sharpness() );
          rcUNITTEST_ASSERT( hd.whiteBalanceUB() == camHdr.whiteBalanceUB() );
          rcUNITTEST_ASSERT( hd.whiteBalanceVR() == camHdr.whiteBalanceVR() );
          rcUNITTEST_ASSERT( hd.hue() == camHdr.hue() );
          rcUNITTEST_ASSERT( hd.saturation() == camHdr.saturation() );
          rcUNITTEST_ASSERT( hd.gamma() == camHdr.gamma() );
          rcUNITTEST_ASSERT( hd.shutter() == camHdr.shutter() );
          rcUNITTEST_ASSERT( hd.gain() == camHdr.gain() );
          rcUNITTEST_ASSERT( hd.iris() == camHdr.iris() );
          rcUNITTEST_ASSERT( hd.focus() == camHdr.focus() );
          rcUNITTEST_ASSERT( hd.temperature() == camHdr.temperature() );
          rcUNITTEST_ASSERT( hd.zoom() == camHdr.zoom() );
          rcUNITTEST_ASSERT( hd.pan() == camHdr.pan() );
          rcUNITTEST_ASSERT( hd.tilt() == camHdr.tilt() );
          rcUNITTEST_ASSERT( hd.filter() == camHdr.filter() );
          rcUNITTEST_ASSERT( hd.cropRect() == camHdr.cropRect() );
          rcUNITTEST_ASSERT( hd.id() != camHdr.id() );
      }
      rcVideoCache::rcVideoCacheDtor( movie );
  }

  // Experiment header test
  {
      rcVideoCache* movie = rcVideoCache::rcVideoCacheCtor(expName, 0, true, false, true, 0);
      rcUNITTEST_ASSERT( movie->isValid() );
      rcUNITTEST_ASSERT( movie->frameCount() == savedCount );
      rcUNITTEST_ASSERT( movie->frameWidth() == uint32(bndSz.x()) );
      rcUNITTEST_ASSERT( movie->frameHeight() == uint32(bndSz.y()) );
      const vector<rcMovieFileExpExt>& exps = movie->movieFileExperiments();
      rcUNITTEST_ASSERT( exps.size() == 1 );
      for ( uint32 i = 0; i < exps.size(); ++i ) {
          rcMovieFileExpExt hd = exps[i];
          rcUNITTEST_ASSERT( hd.type() == movieExtensionEXP );
          rcUNITTEST_ASSERT( !strcmp(hd.userName(), expHdr.userName()) );
          rcUNITTEST_ASSERT( hd.id() != expHdr.id() );
      }
      rcVideoCache::rcVideoCacheDtor( movie );
  }
  
  // Delete movies
  std::string cmd = "rm -f " + nearName;
  system( cmd.c_str() );
  cmd = "rm -f " + biName;
  system( cmd.c_str() );
  cmd = "rm -f " + cubicName;
  system( cmd.c_str() );
  cmd = "rm -f " + convName;
  system( cmd.c_str() );
  cmd = "rm -f " + camName;
  system( cmd.c_str() );
  cmd = "rm -f " + expName;
  system( cmd.c_str() );
  
  printSuccessMessage( "rcGenMovieFile", mErrors-oldErrors );
}
