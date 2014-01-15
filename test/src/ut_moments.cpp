/*
 *	ut_moments.cpp 06/05/2003 
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */
#include <rc_moments.h>
#include <rc_analysis.h>
#include <rc_time.h>
#include <rc_windowhist.h>
#include "ut_moments.h"


#define rmPrintImage(a){ \
for (int32 i = 0; i < (a).height(); i++) \
{ \
   fprintf (stderr, "\n"); \
      for (int32 j = 0; j < (a).width(); j++) \
         fprintf (stderr, "%3d ", (a).getPixel (j, i)); \
            fprintf (stderr, "\n");\
}}

#define rmPrintBinaryImage(a){ \
for (int32 i = 0; i < (a).height(); i++) \
{ \
   fprintf (stderr, "\n"); \
      for (int32 j = 0; j < (a).width(); j++) \
	fprintf (stderr, "%c", (a).getPixel (j, i) == 0 ? '0' : '1');	\
            fprintf (stderr, "\n");\
}}

#define RDM_16_BITS ((uint32)((rand() >> 4) & 0xFFFF))

static void genTestImage(rcWindow img, uint32 baseValue,
			 uint32 fgbgDelta, uint32 noisePct)
{
  const uint32 width = img.width(), height = img.height();
  
  /* Put basic figure into image. Make cross that is approximately
   * 1/3 source image width/height and centered.
   */
  uint32 strokeWidth = width/3;
  if ((width & 1) != (strokeWidth & 1))
    strokeWidth--;
  
  uint32 strokeHeight = height/3;
  if ((height & 1) != (strokeHeight & 1))
    strokeHeight--;
  
  img.setAllPixels(baseValue);
  rcWindow horWin(img, 0, (height-strokeHeight)/2, width, strokeHeight);
  horWin.setAllPixels(baseValue + fgbgDelta);
  rcWindow verWin(img, (width-strokeWidth)/2, 0, strokeWidth, height);
  verWin.setAllPixels(baseValue + fgbgDelta);
  
  /* Now throw the user specified amount of noise into the image.
   */
  for (uint32 noiseCnt = (width*height*noisePct)/100; noiseCnt--; ) {
    uint32 xOff = (uint32)(((int)RDM_16_BITS * (int)width - 1)/65535);
    uint32 yOff = (uint32)(((int)RDM_16_BITS * (int)height - 1)/65535);
    uint32 pRandom = (RDM_16_BITS * (fgbgDelta-1))/65535 + 1;
    rmAssert(pRandom > 0); 
    rmAssert(pRandom <= fgbgDelta);
    img.setPixel(xOff, yOff, baseValue+pRandom);
  }
}

UT_moments::UT_moments()
{
}

UT_moments::~UT_moments()
{
  printSuccessMessage( "MOMENTS test", mErrors );
}

uint32 UT_moments::run()
{
#ifdef __ppc__
  testAutoCorrGeneralAnd16 ();
  testMomentGenerator();
  testAutoCorr3by3Point();
  timeAutoCorr3by3Point();
  testAutoCorr3by3Line();
  timeAutoCorr3by3Line();
  testAutoCorr5by5Point();
  timeAutoCorr5by5Point();
  testAutoCorr5by5Line();
  timeAutoCorr5by5Line();
#endif
  testAutoCorrPoint();
  timeAutoCorrPoint();

  return mErrors;
}

inline uint32 genSum(rcWindow& w)
{
  uint32 sum = 0;
  for (int32 y = 0; y < w.height(); y++)
    for (int32 x = 0; x < w.width(); x++)
      sum += w.getPixel(x, y);
    
  return sum;
}

inline int64 genSumSq(rcWindow& w)
{
  int64 sumSq = 0;
  for (int32 y = 0; y < w.height(); y++)
    for (int32 x = 0; x < w.width(); x++) {
      int64 p = w.getPixel(x, y);
      sumSq += p*p;
    }
  return sumSq;
}

inline int64 genSumIM(rcWindow& i, rcWindow& m)
{
  rmAssert(i.width() == m.width());
  rmAssert(i.height() == m.height());

  int64 sumIM = 0;
  for (int32 y = 0; y < i.height(); y++)
    for (int32 x = 0; x < i.width(); x++)
      sumIM += (int64)i.getPixel(x, y) * (int64)m.getPixel(x, y);

  return sumIM;
}

inline void genVProj(rcWindow& pWin, rcWindow& expProj)
{
  const int32 h = pWin.height();
  const int32 w = pWin.width();

  for (int32 xo = 0; xo < w; xo++) {
    uint32 sum = 0;

    for (int32 yo = 0; yo < h; yo++) {
      sum += pWin.getPixel(xo, yo);
    }
    expProj.setPixel(xo, 0, sum);
  }
}

inline void genHProj(rcWindow& pWin, rcWindow& expProj)
{
  const int32 h = pWin.height();
  const int32 w = pWin.width();

  for (int32 yo = 0; yo < h; yo++) {
    uint32 sum = 0;

    for (int32 xo = 0; xo < w; xo++) {
      sum += pWin.getPixel(xo, yo);
    }
    expProj.setPixel(yo, 0, sum);
  }
}

void UT_moments::testMomentGenerator()
{
  /* Test alignment cases by:
   *
   *   creating a "parent" window
   *
   *   create all possible "child" windows
   *
   *   use the child window as the "frame" to pass to update()
   *
   *   Verify that the sum and sum square values for all the
   *   rectangular regions within "frame" are valid.
   *
   * This is done for both the 1D and 2D cases. Since this is 
   * an exhaustive test, use a relatively small "parent" window.
   */
  {
    const int32 parW = 48;
    const int32 parH = 6;
  
    rcWindow parWin(parW, parH);
      
    /* Initialize the parent image.
     */
    uint8 v = 0xFF;
    for (int32 y = 0; y < parWin.height(); y++)
      for (int32 x = 0; x < parWin.width(); x++)
	parWin.setPixel(x, y, v--);
      
    rcMomentGenerator gen1D(rcMomentGenerator::eMoment1D);
    rcMomentGenerator gen2D(rcMomentGenerator::eMoment2D);
    rcMomentGenerator gen2DFast(rcMomentGenerator::eMoment2DFast);

    for (int32 frmYOff = 0; frmYOff < parH; frmYOff++)
      for (int32 frmXOff = 0; frmXOff < parW; frmXOff++) {
	/* Initialize the "child" window or "frame".
	 */
	const int32 fWidth = parW - frmXOff;
	const int32 fHeight = parH - frmYOff;
	rcWindow frame(parWin, frmXOff, frmYOff, fWidth, fHeight);
	gen1D.update(frame);
	gen2D.update(frame);
	gen2DFast.update(frame);
	rcWindow expVProj(fWidth, 1, rcPixel32);
	for (int32 yOff = 0; yOff < fHeight; yOff++)
	  for (int32 xOff = 0; xOff < fWidth; xOff++)
	    for (int32 yDim = 1; yDim <= (fHeight - yOff); yDim++) {
	      rcWindow pVWin(frame, xOff, yOff, fWidth - xOff, yDim);
	      // Calculate expected projection results
	      genVProj(pVWin, expVProj);
	      for (int32 xDim = 1; xDim <= (fWidth - xOff); xDim++) {
		/* Generate values for all possible rectangles within
		 * the "frame".
		 */
		rcWindow curWin(frame, xOff, yOff, xDim, yDim);
		rcRect curLoc(xOff, yOff, xDim, yDim);

		/* Calculate the expected sum/sum sq results.
		 */
		const float expSum = genSum(curWin);
		const float expSumSq = genSumSq(curWin);

		/* Calculate the actual sum/sum sq results.
		 */
		float actSum1D, actSumSq1D, actSum2D, actSumSq2D;
		float actSum2DFast, actSumSq2DFast;
		gen1D.genSumSumSq(curLoc, actSum1D, actSumSq1D);
		gen2D.genSumSumSq(curLoc, actSum2D, actSumSq2D);
		const bool testFast = xDim*yDim <= 65536;
		if (testFast)
		  gen2DFast.genSumSumSq(curLoc, actSum2DFast, actSumSq2DFast);

		/* Validate sum/sum sq.
		 */
		rcUNITTEST_ASSERT(actSum1D == expSum);
		rcUNITTEST_ASSERT(actSumSq1D == expSumSq);
		rcUNITTEST_ASSERT(actSum2D == expSum);
		rcUNITTEST_ASSERT(actSumSq2D == expSumSq);
		if (testFast) {
		  rcUNITTEST_ASSERT(actSum2DFast == expSum);
		  rcUNITTEST_ASSERT(actSumSq2DFast == expSumSq);
		}

		/* Calculate the actual vertical projection results.
		 */
		rcWindow actVProj1D(xDim, 1, rcPixel32);
		rcWindow actVProj2D(xDim, 1, rcPixel32);
		rcWindow actVProj2DFast(xDim, 1, rcPixel32);
		gen1D.vProject(curLoc, actVProj1D);
		gen2D.vProject(curLoc, actVProj2D);
		if (testFast)
		  gen2DFast.vProject(curLoc, actVProj2DFast);

		/* Validate vertical projection.
		 */
		for (int32 i = 0; i < xDim; i++) {
		  const uint32 expValue = expVProj.getPixel(i, 0);
		  rcUNITTEST_ASSERT(expValue == actVProj1D.getPixel(i, 0));
		  rcUNITTEST_ASSERT(expValue == actVProj2D.getPixel(i, 0));
		  if (testFast)
		    rcUNITTEST_ASSERT(expValue == actVProj2DFast.getPixel(i, 0));
		}
	      } // End of: for ( ... ; xDim <= (fWidth - xOff); xDim++) {
	    } // End of: for ( ... ; yDim <= (fHeight - yOff); yDim++) {

	rcWindow expHProj(fHeight, 1, rcPixel32);
	for (int32 yOff = 0; yOff < fHeight; yOff++)
	  for (int32 xOff = 0; xOff < fWidth; xOff++)
	    for (int32 xDim = 1; xDim <= (fWidth - xOff); xDim++) {
	      rcWindow pHWin(frame, xOff, yOff, xDim, fHeight - yOff);
	      genHProj(pHWin, expHProj);
	      for (int32 yDim = 1; yDim <= (fHeight - yOff); yDim++) {
		rcRect curLoc(xOff, yOff, xDim, yDim);
		const bool testFast = xDim*yDim <= 65536;

		/* Calculate the actual horizontal projection results.
		 */
		rcWindow actHProj2D(yDim, 1, rcPixel32);
		rcWindow actHProj2DFast(yDim, 1, rcPixel32);
		gen2D.hProject(curLoc, actHProj2D);
		if (testFast)
		  gen2DFast.hProject(curLoc, actHProj2DFast);

		/* Validate horizontal projection.
		 */
		for (int32 i = 0; i < yDim; i++) {
		  const uint32 expValue = expHProj.getPixel(i, 0);
		  rcUNITTEST_ASSERT(expValue == actHProj2D.getPixel(i, 0));
		  if (testFast)
		    rcUNITTEST_ASSERT(expValue == actHProj2DFast.getPixel(i, 0));
		}
	      } // End of: for ( ... ; yDim <= (fHeight - yOff); yDim++) {
	    } // End of: for ( ... ; xDim <= (fWidth - xOff); xDim++) {
	
      } // End of: for (int32 frmXOff = 0; frmXOff < parW; frmXOff++) {
  } // End of: Alignment Test

  /* Perform numerical accuracy test. This is basically verifies that,
   * as integral values become large, the results are still as
   * expected. There are 2 problem cases being tested here:
   *
   *   Large Rectangle - In this case, the final values can become very
   *                     large. The test verifies that the the results
   *                     are valid in this case (Note: Since a float
   *                     isn't large enough to hold large values with
   *                     complete fidelity the test only checks that
   *                     it is correct to within the expected accuracy
   *                     of a float).
   *
   *   Small Rectangle - In this case, the final values are small, but
   *   At Bottom Right   the intermediate values used in doing the
   *   Corner Of Image   calculation are large. Verify that this
   *                     doesn't affect the accuracy of the result.
   */
  {
    const int32 maxDim = 2048;
    const uint8 pVal = 0xFF;

    double   bigNumD = (double)maxDim*pVal*pVal;
    int32 bigNumI = maxDim*pVal*pVal;
    rcUNITTEST_ASSERT((double)bigNumI == bigNumD);

    double  bigNumDD = (double)maxDim*bigNumD;
    int64 bigNumII = ((int64)maxDim)*bigNumI;
    rcUNITTEST_ASSERT((double)bigNumII == bigNumDD);

    rcMomentGenerator gen1D(rcMomentGenerator::eMoment1D);
    rcMomentGenerator gen2D(rcMomentGenerator::eMoment2D);
    rcWindow bigKahuna(maxDim, maxDim);

    bigKahuna.setAllPixels(pVal);
    gen1D.update(bigKahuna);
    gen2D.update(bigKahuna);

    for (int32 y = 1; y < 8; y++)
      for (int32 x = 1; x < 8; x++) {
	rcWindow curWinSmall(bigKahuna, maxDim-x, maxDim-y, x, y);
	rcWindow curWinLarge(bigKahuna, 0, 0, maxDim-x+1, maxDim-y+1);
	rcRect curLocSmall(maxDim-x, maxDim-y, x, y);
	rcRect curLocLarge(0, 0, maxDim-x+1, maxDim-y+1);

	/* Calculate the expected results.
	 */
	const float expSumLarge = genSum(curWinLarge);
	const float expSumSqLarge = genSumSq(curWinLarge);
	const int64 expSumSmall = genSum(curWinSmall);
	const int64 expSumSqSmall = genSumSq(curWinSmall);

	/* Calculate the actual results.
	 */
	float actSum1DLarge, actSumSq1DLarge, actSum2DLarge, actSumSq2DLarge;
	gen1D.genSumSumSq(curLocLarge, actSum1DLarge, actSumSq1DLarge);
	gen2D.genSumSumSq(curLocLarge, actSum2DLarge, actSumSq2DLarge);
	
	float actSum1DSmall, actSumSq1DSmall, actSum2DSmall, actSumSq2DSmall;
	gen1D.genSumSumSq(curLocSmall, actSum1DSmall, actSumSq1DSmall);
	gen2D.genSumSumSq(curLocSmall, actSum2DSmall, actSumSq2DSmall);
	
	/* Validate.
	 */
	rcUNITTEST_ASSERT(actSum1DLarge == expSumLarge);
	rcUNITTEST_ASSERT(actSumSq1DLarge == expSumSqLarge);
	rcUNITTEST_ASSERT(actSum2DLarge == expSumLarge);
	rcUNITTEST_ASSERT(actSumSq2DLarge == expSumSqLarge);

	rcUNITTEST_ASSERT((int64)actSum1DSmall == expSumSmall);
	rcUNITTEST_ASSERT((int64)actSumSq1DSmall == expSumSqSmall);
	rcUNITTEST_ASSERT((int64)actSum2DSmall == expSumSmall);
	rcUNITTEST_ASSERT((int64)actSumSq2DSmall == expSumSqSmall);
      }
  } // End of: Numerical Accuracy Test for eMoment1D, eMoment2D
  {
    const int32 maxDim = 256;
    const int32 winDim = 2048;
    const uint8 pVal = 0xFF;

    rcMomentGenerator gen2DFast(rcMomentGenerator::eMoment2DFast);
    rcWindow bigKahuna(winDim, winDim);

    bigKahuna.setAllPixels(pVal);
    gen2DFast.update(bigKahuna);

    for (int32 y = 1; y < 8; y++)
      for (int32 x = 1; x < 8; x++) {
	rcWindow curWinSmall(bigKahuna, winDim-x, winDim-y, x, y);
	rcWindow curWinLarge(bigKahuna, 0, 0, maxDim-x+1, maxDim-y+1);
	rcRect curLocSmall(winDim-x, winDim-y, x, y);
	rcRect curLocLarge(0, 0, maxDim-x+1, maxDim-y+1);

	/* Calculate the expected results.
	 */
	const float expSumLarge = genSum(curWinLarge);
	const float expSumSqLarge = genSumSq(curWinLarge);
	const int64 expSumSmall = genSum(curWinSmall);
	const int64 expSumSqSmall = genSumSq(curWinSmall);

	/* Calculate the actual results.
	 */
	float actSum2DLarge, actSumSq2DLarge;
	gen2DFast.genSumSumSq(curLocLarge, actSum2DLarge, actSumSq2DLarge);
	
	float actSum2DSmall, actSumSq2DSmall;
	gen2DFast.genSumSumSq(curLocSmall, actSum2DSmall, actSumSq2DSmall);
	
	/* Validate.
	 */
	rcUNITTEST_ASSERT(actSum2DLarge == expSumLarge);
	rcUNITTEST_ASSERT(actSumSq2DLarge == expSumSqLarge);

	rcUNITTEST_ASSERT((int64)actSum2DSmall == expSumSmall);
	rcUNITTEST_ASSERT((int64)actSumSq2DSmall == expSumSqSmall);
      }
  } // End of: Numerical Accuracy Test for eMoment2DFast
}

#define dump3by3(res) {							   \
  printf("T: %0.6f %0.6f %0.6f\n",                                         \
	 res.score[0][0], res.score[0][1], res.score[0][2]);		   \
  printf("M: %0.6f %0.6f %0.6f\n", res.score[1][0], 1.0, res.score[1][2]); \
  printf("B: %0.6f %0.6f %0.6f\n\n",                                       \
	 res.score[2][0], res.score[2][1], res.score[2][2]); }
#define dump5by5(res) {							      \
  printf("T:  %0.6f %0.6f %0.6f %0.6f %0.6f\n", res.score[0][0],              \
	 res.score[0][1], res.score[0][2], res.score[0][3], res.score[0][4]); \
  printf("UM: %0.6f %0.6f %0.6f %0.6f %0.6f\n", res.score[1][0],              \
	 res.score[1][1], res.score[1][2], res.score[1][3], res.score[1][4]); \
  printf("M:  %0.6f %0.6f %0.6f %0.6f %0.6f\n", res.score[2][0],              \
	 res.score[2][1], 1.0, res.score[2][3], res.score[2][4]);             \
  printf("LM: %0.6f %0.6f %0.6f %0.6f %0.6f\n", res.score[3][0],              \
	 res.score[3][1], res.score[3][2], res.score[3][3], res.score[3][4]); \
  printf("L:  %0.6f %0.6f %0.6f %0.6f %0.6f\n", res.score[4][0],              \
	 res.score[4][1], res.score[4][2], res.score[4][3], res.score[4][4]); }

void UT_moments::testAutoCorr3by3Point()
{
  srand(0);

  /* Since integral based correlation uses floats, there is an
   * inherent inaccuracy in the result. Check that these errors don't
   * exceed the following somewhat arbitrary values.
   */
  const float maxCorrError = 1.0e-6;
  const float maxSDError = 1.0e-5;

  /* Test alignment cases by:
   *
   *   creating a "parent" window
   *
   *   create all possible "child" windows
   *
   *   use the child window as the "frame" to pass to update()
   *
   *   Verify that the correlation values match within tolerances
   *   those returned by rfCorrelate().
   *
   * This is done for both the cross and full cases. Since this is 
   * an exhaustive test, use a relatively small "parent" window.
   */
  {
    const int32 parW = 48;
    const int32 parH = 6;
  
    rcWindow parWin(parW, parH);
    parWin.randomFill(11);

    rcAutoCorrelation acFull, acFullFast(true), acCross, acCrossFast(true);
    rsCorr3by3Point fullResult, fullResultFast, crossResult, crossResultFast;
    rsCorrParams cp;
    rcCorr res;

    float largestCorrError = 0.0;
    float largestSDError = 0.0;

    for (int32 frmYOff = 0; frmYOff < (parH-2); frmYOff++)
      for (int32 frmXOff = 0; frmXOff < (parW-2); frmXOff++) {
	/* Initialize the "child" window or "frame".
	 */
	const int32 fWidth = parW - frmXOff;
	const int32 fHeight = parH - frmYOff;
	rcWindow frame(parWin, frmXOff, frmYOff, fWidth, fHeight);

	acFull.update(frame, 1, rcAutoCorrelation::eFull,
		      rcAutoCorrelation::ePoint);
	acFullFast.update(frame, 1, rcAutoCorrelation::eFull,
			  rcAutoCorrelation::ePoint);
	acCross.update(frame, 1, rcAutoCorrelation::eCross,
		       rcAutoCorrelation::ePoint);
	acCrossFast.update(frame, 1, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::ePoint);

	const int32 maxHeight = (fHeight <= 15) ? fHeight : 15;
	const int32 maxWidth = (fWidth <= 15) ? fWidth : 15;
	for (int32 ySrchSpcSz = 3; ySrchSpcSz <= maxHeight; ySrchSpcSz++) {
	  for (int32 xSrchSpcSz = 3; xSrchSpcSz <= maxWidth; xSrchSpcSz++) {
	    for (int32 yS = 0; yS <= (fHeight - ySrchSpcSz); yS++)
	      for (int32 xS = 0; xS <= (fWidth - xSrchSpcSz); xS++) {
		rcWindow srchSpace(frame, xS, yS, xSrchSpcSz, ySrchSpcSz);
		rcWindow curModel(frame, xS+1, yS+1, xSrchSpcSz-2, ySrchSpcSz-2);
		rcRect loc(xS, yS, xSrchSpcSz, ySrchSpcSz);
		const bool testFast = (xSrchSpcSz-2)*(ySrchSpcSz-2) <= 65536;
		acFull.gen3by3Point(loc, fullResult, -1);
		acCross.gen3by3Point(loc, crossResult, -1);
		if (testFast) {
		  acFullFast.gen3by3Point(loc, fullResultFast, -1);
		  acCrossFast.gen3by3Point(loc, crossResultFast, -1);
		}
		for (uint32 yM = 0; yM < 3; yM++)
		  for (uint32 xM = 0; xM < 3; xM++) {
		    rcWindow curImage(srchSpace, xM, yM, curModel.width(),
				      curModel.height());
		    rfCorrelate (curModel, curImage, cp, res);
		  
		    if ((xM & yM) == 1) { // Handle center location case
		      if (testFast) {
			rcUNITTEST_ASSERT(fullResult.score[1][1] ==
					   fullResultFast.score[1][1]);
			rcUNITTEST_ASSERT(crossResult.score[1][1] ==
					   crossResultFast.score[1][1]);
		      }
		      rcUNITTEST_ASSERT(fullResult.score[1][1] ==
					 crossResult.score[1][1]);
		      double N = curModel.width()*curModel.height();
		      double stddev = sqrt(fullResult.score[1][1]/(N*(N-1)));
		      float SDError = fabs(stddev - res.iStd());
		      if (SDError > largestSDError) {
			largestSDError = SDError;
			rcUNITTEST_ASSERT(largestSDError < maxSDError);
		      }
		    }
		    else { // Handle corner and cross location cases
		      rcUNITTEST_ASSERT(fullResult.score[yM][xM] >= 0.0);
		      rcUNITTEST_ASSERT(fullResult.score[yM][xM] <= 1.0);

		      float corrError = fabs(fullResult.score[yM][xM] - res.r());
		      if (corrError > largestCorrError) {
			largestCorrError = corrError;
			rcUNITTEST_ASSERT(largestCorrError < maxCorrError);
		      }
		      if (testFast) {
			rcUNITTEST_ASSERT(fullResult.score[yM][xM] ==
					   fullResultFast.score[yM][xM]);
		      }
		      if (((yM+xM) & 1) != 0) { // Also a cross location case
			rcUNITTEST_ASSERT(fullResult.score[yM][xM] ==
					   crossResult.score[yM][xM]);
			if (testFast)
			  rcUNITTEST_ASSERT(crossResult.score[yM][xM] ==
					     crossResultFast.score[yM][xM]);
		      }
		    } // End of: if ((xM & yM) == 1) ... else ...
		  } // End of: for (uint32 xM = 0; xM < 3; xM++) {
	      } // End of: for ( ... ; xS <= (width - xSrchSpcSz); xS++) {
	  }
	}
      } // End of: for (int32 frmXOff = 0; frmXOff < parW; frmXOff++) {

    fprintf(stderr, "Performance: 3 x 3 Point Integral (Random) Largest Corr. Error"
	    " %1.2e Largest Std Dev Error %1.2e\n",
	    largestCorrError, largestSDError);
  } // End of: Alignment Test

  /* Perform numerical accuracy test. This tries to generate worst case 
   * scenarios. The following cases are tested:
   *
   *   Maximum Std Dev - In this case, a large frame is created where,
   *                     for the "cross" images, half the pixels are
   *                     set to 0 and the other half are set to FF.
   *
   *   Minumum Std Dev - 
   */
  {
    rcAutoCorrelation acCross, acCrossFast(true);  
    rsCorr3by3Point crossResult, crossResultFast;
    rsCorrParams cp;
    rcCorr res;

    { // Maximum Std Dev Test
      const int32 maxDim = 2048;
      rcWindow bigKahuna(maxDim, maxDim);

      /* Set up for "maximum standard deviation" test.
       */
      bigKahuna.setAllPixels(0);

      int32 fudge = 85;
      rcWindow temp1(bigKahuna, fudge, maxDim/3, maxDim - fudge*2, maxDim/3+1);
      temp1.setAllPixels(0xFF);

      rcWindow temp2(bigKahuna, maxDim/3, fudge, maxDim/3, maxDim/3-fudge);
      temp2.setAllPixels(0xFF);

      rcWindow temp3(bigKahuna, maxDim/3, 2*maxDim/3,  maxDim/3, maxDim/3-fudge);
      temp3.setAllPixels(0xFF);
    
      /* Image set up, Ready to do calculations.
       */
      acCross.update(bigKahuna, 1, rcAutoCorrelation::eCross,
		     rcAutoCorrelation::ePoint);
      acCrossFast.update(bigKahuna, 1, rcAutoCorrelation::eCross,
			 rcAutoCorrelation::ePoint);

      int32 tDim[2] = {258, 2048};
      for (uint32 tIndex = 0; tIndex < 2; tIndex++) {
	int32 testDim = tDim[tIndex];
	rcWindow srchSpace(bigKahuna, 0, 0, testDim, testDim);
	rcWindow curModel(bigKahuna, 1, 1, testDim-2, testDim-2);
	rcRect loc(0, 0, testDim, testDim);
	const bool testFast = testDim != 2048;
	acCross.gen3by3Point(loc, crossResult, -1);
	if (testFast)
	  acCrossFast.gen3by3Point(loc, crossResultFast, -1);
	for (uint32 yM = 0; yM < 3; yM++)
	  for (uint32 xM = 0; xM < 3; xM++) {
	    rcWindow curImage(srchSpace, xM, yM, curModel.width(),
			      curModel.height());
	    rfCorrelate (curModel, curImage, cp, res);
	  
	    if ((xM & yM) == 1) { // Handle center location case
	      if (testFast)
		rcUNITTEST_ASSERT(crossResult.score[1][1] ==
				   crossResultFast.score[1][1]);
	      double N = curModel.width()*curModel.height();
	      double stddev = sqrt(crossResult.score[1][1]/(N*(N-1)));
	      float SDError = fabs(stddev - res.iStd());
	      rcUNITTEST_ASSERT(SDError < maxSDError);
	    }
	    else if (((yM+xM) & 1) != 0) {
	      rcUNITTEST_ASSERT(crossResult.score[yM][xM] >= 0.0);
	      rcUNITTEST_ASSERT(crossResult.score[yM][xM] <= 1.0);
	      
	      float corrError = fabs(crossResult.score[yM][xM] - res.r());
	      rcUNITTEST_ASSERT(corrError < maxCorrError);
	      if (testFast)
		rcUNITTEST_ASSERT(crossResult.score[yM][xM] ==
				   crossResultFast.score[yM][xM]);
	    } // End of: if ((xM & yM) == 1) ... else ...
	  } // End of: for (uint32 xM = 0; xM < 3; xM++) {
      } // End of: for (uint32 tIndex = 0; tIndex < 2; tIndex++) {
    } // End of: Maximum Std Dev Test

    { // Minimum Std Dev Test
      const uint32 noisePct = 10;
      const uint32 sizeCount = 8;
      const int32 imageSize[sizeCount] = {7, 11, 16, 32, 256, 512, 1024, 2048};
      const uint32 baseCount = 8;
      const uint32 baseValue[baseCount] = {0, 128, 192, 224, 240, 248, 252, 254};
      const uint32 deltaCount = 5;
      const uint32 deltaValue[deltaCount] = {1, 2, 4, 8, 16};

      float largestCorrError = 0.0;
      float largestSDError = 0.0;

      for (uint32 curSz = 0; curSz < sizeCount; curSz++) {
	const int32 dim = imageSize[curSz]; 
	rcWindow frame(dim, dim);
	for (uint32 curBase = 0; curBase < baseCount; curBase++)
	  for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	    if ((baseValue[curBase] + deltaValue[curDelta]) > 255)
	      continue;
	    genTestImage(frame, baseValue[curBase], deltaValue[curDelta], noisePct);
	    /* Image set up, Ready to do calculations.
	     */
	    acCross.update(frame, 1, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::ePoint);
	    acCrossFast.update(frame, 1, rcAutoCorrelation::eCross,
			       rcAutoCorrelation::ePoint);
	    const bool testFast = (dim-2)*(dim-2) <= 65536;
	    
	    acCross.gen3by3Point(frame.rcBound(), crossResult, -1);
	    if (testFast)
	      acCrossFast.gen3by3Point(frame.rcBound(), crossResultFast, -1);
	    rcWindow srchSpace(frame, 0, 0, dim, dim);
	    rcWindow curModel(frame, 1, 1, dim-2, dim-2);
	    for (uint32 yM = 0; yM < 3; yM++)
	      for (uint32 xM = 0; xM < 3; xM++) {
		rcWindow curImage(srchSpace, xM, yM, curModel.width(),
				  curModel.height());
		rfCorrelate (curModel, curImage, cp, res);
		
		if ((xM & yM) == 1) { // Handle center location case
		  if (testFast)
		    rcUNITTEST_ASSERT(crossResult.score[1][1] ==
				       crossResultFast.score[1][1]);
		  double N = curModel.width()*curModel.height();
		  double stddev = sqrt(crossResult.score[1][1]/(N*(N-1)));
		  float SDError = fabs(stddev - res.iStd());
		  if (SDError > largestSDError) {
		    largestSDError = SDError;
		    rcUNITTEST_ASSERT(SDError < maxSDError);
		  }
		}
		else if (((yM+xM) & 1) != 0) {
		  rcUNITTEST_ASSERT(crossResult.score[yM][xM] >= 0.0);
		  rcUNITTEST_ASSERT(crossResult.score[yM][xM] <= 1.0);
		  
		  float corrError = fabs(crossResult.score[yM][xM] - res.r());
		  if (corrError > largestCorrError) {
		    largestCorrError = corrError;
		    rcUNITTEST_ASSERT(corrError < maxCorrError);
		  }
		  if (testFast)
		    rcUNITTEST_ASSERT(crossResult.score[yM][xM] ==
				       crossResultFast.score[yM][xM]);
		} // End of: if ((xM & yM) == 1) ... else ...
	      } // End of: for (uint32 xM = 0; xM < 3; xM++) {
	  } // End of: for ( ... ; curDelta < fgDeltaCount; curDelta++) {
      } // End of: for (uint32 curSz = 0; curSz < sizeCount; curSz++) {
      fprintf(stderr, "Performance: (%d%%) 3 x 3 Point Integral (Min Dev) Largest"
	      " Corr. Error %1.2e Largest Std Dev Error %1.2e\n", noisePct,
	      largestCorrError, largestSDError);
    } // End of: Minimumm Std Dev Test
  } // End of: Numerical Accuracy Test
}

void UT_moments::testAutoCorr3by3Line()
{
  srand(0);

  /* Since integral based correlation uses floats, there is an
   * inherint inaccuracy in the result. Check that these errors don't
   * exceed the following somewhat arbitrary values.
   */
  const float maxCorrError = 1.0e-6;
  const float maxSDError = 1.2e-5;

  /* Test alignment cases by:
   *
   *   creating a "parent" window
   *
   *   create all possible "child" windows
   *
   *   use the child window as the "frame" to pass to update()
   *
   *   Verify that the correlation values match within tolerances
   *   those returned by rfCorrelate().
   *
   * This is done for both the cross and full cases. Since this is 
   * an exhaustive test, use a relatively small "parent" window.
   */
  {
    const int32 parW = 48;
    const int32 parH = 6;
  
    rcWindow parWin(parW, parH);
    parWin.randomFill(11);

    rcAutoCorrelation acLineCross, acLineFull;
    rsCorr3by3Line lnRsltCross, lnRsltFull;
    rsCorrParams cp;
    rcCorr res;

    float largestCorrError = 0.0;
    float largestSDError = 0.0;

    for (int32 frmYOff = 0; frmYOff < (parH-2); frmYOff++)
      for (int32 frmXOff = 0; frmXOff < (parW-2); frmXOff++) {
	/* Initialize the "child" window or "frame".
	 */
	const int32 fWidth = parW - frmXOff;
	const int32 fHeight = parH - frmYOff;
	rcWindow frame(parWin, frmXOff, frmYOff, fWidth, fHeight);

	acLineCross.update(frame, 1, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::eLine);
	acLineFull.update(frame, 1, rcAutoCorrelation::eFull,
			  rcAutoCorrelation::eLine);

	const int32 maxHeight = (fHeight <= 15) ? fHeight : 15;
	const int32 maxWidth = (fWidth <= 15) ? fWidth : 15;
	for (int32 ySrchSpcSz = 3; ySrchSpcSz <= maxHeight; ySrchSpcSz++) {
	  for (int32 xSrchSpcSz = 3; xSrchSpcSz <= maxWidth; xSrchSpcSz++) {
	    rcIPair srchSize(xSrchSpcSz, ySrchSpcSz);
	    for (int32 yS = 0; yS <= (fHeight - ySrchSpcSz); yS++) {
	      acLineCross.gen3by3Line(srchSize, yS, lnRsltCross);
	      rcUNITTEST_ASSERT((int32)lnRsltCross.count ==
				 (fWidth - xSrchSpcSz + 1));
	      acLineFull.gen3by3Line(srchSize, yS, lnRsltFull);
	      rcUNITTEST_ASSERT((int32)lnRsltFull.count ==
				 (fWidth - xSrchSpcSz + 1));
	      for (int32 xS = 0; xS <= (fWidth - xSrchSpcSz); xS++) {
		rcWindow srchSpace(frame, xS, yS, xSrchSpcSz, ySrchSpcSz);
		rcWindow curModel(frame, xS+1, yS+1, xSrchSpcSz-2, ySrchSpcSz-2);
	     
		for (uint32 yM = 0; yM < 3; yM++)
		  for (uint32 xM = 0; xM < 3; xM++) {
		    rcWindow curImage(srchSpace, xM, yM, curModel.width(),
				      curModel.height());
		    rfCorrelate(curModel, curImage, cp, res);
		  
		    if ((xM & yM) == 1) { // Handle center location case
		      double N = curModel.width()*curModel.height();
		      double stddev =
			sqrt(*(lnRsltFull.score[1][1] + xS)/(N*(N-1)));
		      float SDError = fabs(stddev - res.iStd());
		      if (SDError > largestSDError) {
			largestSDError = SDError;
			rcUNITTEST_ASSERT(largestSDError < maxSDError);
		      }
		      rcUNITTEST_ASSERT(*(lnRsltCross.score[1][1] + xS) ==
					 *(lnRsltFull.score[1][1] + xS));
		    }
		    else {
		      rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM] + xS) >= 0.0);
		      rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM] + xS) <= 1.0);
		      float corrError =
			fabs(*(lnRsltFull.score[yM][xM] + xS) - res.r());
		      if (corrError > largestCorrError) {
			largestCorrError = corrError;
			rcUNITTEST_ASSERT(largestCorrError < maxCorrError);
		      }
		      if (((yM+xM) & 1) != 0) { // Handle cross location cases
			rcUNITTEST_ASSERT(*(lnRsltCross.score[yM][xM] + xS) ==
					   *(lnRsltFull.score[yM][xM] + xS));
		      }
		    } // End of: if ((xM & yM) == 1) ... else ...
		  } // End of: for (uint32 xM = 0; xM < 3; xM++) {
	      } // End of: for ( ... ; xS <= (width - xSrchSpcSz); xS++) {
	    } // End of: for ( ... ; yS <= (height - ySrchSpcSz); yS++) {
	  }
	}
      } // End of: for (int32 frmXOff = 0; frmXOff < parW; frmXOff++) {
    fprintf(stderr, "Performance: 3 x 3 Line Integral (Random) Largest Corr. Error %1.2e"
	    " Largest Std Dev Error %1.2e\n", largestCorrError, largestSDError);
  } // End of: Alignment Test

  /* Perform numerical accuracy test. This tries to generate worst case 
   * scenarios. The following cases are tested:
   *
   *   Maximum Std Dev - In this case, a large frame is created where,
   *                     for the "cross" images, half the pixels are
   *                     set to 0 and the other half are set to FF.
   *
   *   Minumum Std Dev - 
   */
  {
    rcAutoCorrelation acLineCross, acLineFull;
    rsCorr3by3Line lnRsltCross, lnRsltFull;
    rsCorrParams cp;
    rcCorr res;

    { // Maximum Std Dev Test
      const int32 maxDim = 2048;
      rcWindow bigKahuna(maxDim, maxDim);

      /* Set up for "maximum standard deviation" test.
       */
      bigKahuna.setAllPixels(0);

      int32 fudge = 85;
      rcWindow temp1(bigKahuna, fudge, maxDim/3, maxDim - fudge*2, maxDim/3+1);
      temp1.setAllPixels(0xFF);

      rcWindow temp2(bigKahuna, maxDim/3, fudge, maxDim/3, maxDim/3-fudge);
      temp2.setAllPixels(0xFF);

      rcWindow temp3(bigKahuna, maxDim/3, 2*maxDim/3,  maxDim/3, maxDim/3-fudge);
      temp3.setAllPixels(0xFF);
    
      /* Image set up, Ready to do calculations.
       */
      acLineCross.update(bigKahuna, 1, rcAutoCorrelation::eCross,
			 rcAutoCorrelation::eLine);
      acLineFull.update(bigKahuna, 1, rcAutoCorrelation::eFull,
			rcAutoCorrelation::eLine);

      rcIPair srchSize(maxDim, maxDim);
      rcWindow srchSpace(bigKahuna, 0, 0, maxDim, maxDim);
      rcWindow curModel(bigKahuna, 1, 1, maxDim-2, maxDim-2);
      acLineCross.gen3by3Line(srchSize, 0, lnRsltCross);
      acLineFull.gen3by3Line(srchSize, 0, lnRsltFull);
      for (uint32 yM = 0; yM < 3; yM++)
	for (uint32 xM = 0; xM < 3; xM++) {
	  rcWindow curImage(srchSpace, xM, yM, curModel.width(),
			    curModel.height());
	  rfCorrelate (curModel, curImage, cp, res);
	  
	  if ((xM & yM) == 1) { // Handle center location case
	    double N = curModel.width()*curModel.height();
	    double stddev = sqrt(*(lnRsltFull.score[1][1])/(N*(N-1)));
	    float SDError = fabs(stddev - res.iStd());
	    rcUNITTEST_ASSERT(SDError < maxSDError);
	    rcUNITTEST_ASSERT(*(lnRsltCross.score[1][1]) ==
			       *(lnRsltFull.score[1][1]));
	  }
	  else {
	    rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM]) >= 0.0);
	    rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM]) <= 1.0);
	  
	    float corrError = fabs(*(lnRsltFull.score[yM][xM]) - res.r());
	    rcUNITTEST_ASSERT(corrError < maxCorrError);
	    if (((yM+xM) & 1) != 0) {
	      rcUNITTEST_ASSERT(*(lnRsltCross.score[yM][xM]) ==
				 *(lnRsltFull.score[yM][xM]));
	    }
	  } // End of: if ((xM & yM) == 1) ... else ...
	} // End of: for (uint32 xM = 0; xM < 3; xM++) {
    } // End of: Maximum Std Dev Test

    { // Minimum Std Dev Test
      const uint32 noisePct = 10;
      const uint32 sizeCount = 8;
      const int32 imageSize[sizeCount] = {7, 11, 16, 32, 256, 512, 1024, 2048};
      const uint32 baseCount = 8;
      const uint32 baseValue[baseCount] = {0, 128, 192, 224, 240, 248, 252, 254};
      const uint32 deltaCount = 5;
      const uint32 deltaValue[deltaCount] = {1, 2, 4, 8, 16};

      float corrError[sizeCount][baseCount][deltaCount];
      float SDError[sizeCount][baseCount][deltaCount];
      float actualSD[sizeCount][baseCount][deltaCount];

      for (uint32 curSz = 0; curSz < sizeCount; curSz++)
	for (uint32 curBase = 0; curBase < baseCount; curBase++)
	  for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	    corrError[curSz][curBase][curDelta] =
	      SDError[curSz][curBase][curDelta] = 0.0;
	  }

      for (uint32 curSz = 0; curSz < sizeCount; curSz++) {
	const int32 dim = imageSize[curSz]; 
	rcWindow frame(dim, dim);
	for (uint32 curBase = 0; curBase < baseCount; curBase++)
	  for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	    if ((baseValue[curBase] + deltaValue[curDelta]) > 255)
	      continue;
	    genTestImage(frame, baseValue[curBase], deltaValue[curDelta],
			 noisePct);

	    /* Image set up, Ready to do calculations.
	     */
	    acLineCross.update(frame, 1, rcAutoCorrelation::eCross,
			       rcAutoCorrelation::eLine);
	    acLineFull.update(frame, 1, rcAutoCorrelation::eFull,
			      rcAutoCorrelation::eLine);
	    rcIPair size(dim, dim);
	    acLineCross.gen3by3Line(size, 0, lnRsltCross);
	    acLineFull.gen3by3Line(size, 0, lnRsltFull);
	    rcWindow srchSpace(frame, 0, 0, dim, dim);
	    rcWindow curModel(frame, 1, 1, dim-2, dim-2);
	    for (uint32 yM = 0; yM < 3; yM++)
	      for (uint32 xM = 0; xM < 3; xM++) {
		rcWindow curImage(srchSpace, xM, yM, curModel.width(),
				  curModel.height());
		rfCorrelate (curModel, curImage, cp, res);
		
		if ((xM & yM) == 1) { // Handle center location case
		  double N = curModel.width()*curModel.height();
		  double stddev = sqrt(*(lnRsltFull.score[1][1])/(N*(N-1)));
		  SDError[curSz][curBase][curDelta] = fabs(stddev - res.iStd());
		  actualSD[curSz][curBase][curDelta] = stddev;
		  rcUNITTEST_ASSERT(*(lnRsltCross.score[1][1]) ==
				     *(lnRsltFull.score[1][1]));
		}
		else {
		  rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM]) >= 0.0);
		  rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM]) <= 1.0);
		  
		  corrError[curSz][curBase][curDelta] =
		    fabs(*(lnRsltFull.score[yM][xM]) - res.r());
		  if (((yM+xM) & 1) != 0) {
		    rcUNITTEST_ASSERT(*(lnRsltCross.score[yM][xM]) ==
				       *(lnRsltFull.score[yM][xM]));
		  }
		} // End of: if ((xM & yM) == 1) ... else ...
	      } // End of: for (uint32 xM = 0; xM < 3; xM++) {
	  } // End of: for ( ... ; curDelta < fgDeltaCount; curDelta++) {
      } // End of: for (uint32 curSz = 0; curSz < sizeCount; curSz++) {
      
#ifdef GEN_ACCURACY_DATA
      for (uint32 curSz = 0; curSz < sizeCount; curSz++) {
	float largestCorrError = 0.0, largestSDError = 0.0, corrSD = 0.0, sdSD=0.0;
	uint32 maxCorrBase = 0, maxCorrDelta = 0;
	uint32 maxSDBase = 0, maxSDDelta = 0;
	for (uint32 curBase = 0; curBase < baseCount; curBase++)
	  for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	    if (corrError[curSz][curBase][curDelta] > largestCorrError) {
	      largestCorrError = corrError[curSz][curBase][curDelta];
	      maxCorrBase = curBase;
	      maxCorrDelta = curDelta;
	      corrSD = actualSD[curSz][curBase][curDelta];
	    }
	    if (SDError[curSz][curBase][curDelta] > largestSDError) {
	      largestSDError = SDError[curSz][curBase][curDelta];
	      maxSDBase = curBase;
	      maxSDDelta = curDelta;
	      sdSD = actualSD[curSz][curBase][curDelta];
	    }
	  }
	fprintf(stderr, "Performance: (%d%%) Image %d X %d, Max Corr Error %f "
		"@ base %d delta %d SD %f Max Std Dev Error %f @ base %d delta %d "
		"SD %f\n", noisePct, imageSize[curSz], imageSize[curSz],
		largestCorrError, baseValue[maxCorrBase],
		deltaValue[maxCorrDelta], corrSD, largestSDError,
		baseValue[maxSDBase], deltaValue[maxSDDelta], sdSD);
      }

      for (uint32 curBase = 0; curBase < baseCount; curBase++) {
	float largestCorrError = 0.0, largestSDError = 0.0, corrSD = 0.0, sdSD=0.0;
	uint32 maxCorrSize = 0, maxCorrDelta = 0;
	uint32 maxSDSize = 0, maxSDDelta = 0;
	for (uint32 curSz = 0; curSz < sizeCount; curSz++)
	  for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	    if (corrError[curSz][curBase][curDelta] > largestCorrError) {
	      largestCorrError = corrError[curSz][curBase][curDelta];
	      maxCorrSize = curSz;
	      maxCorrDelta = curDelta;
	      corrSD = actualSD[curSz][curBase][curDelta];
	    }
	    if (SDError[curSz][curBase][curDelta] > largestSDError) {
	      largestSDError = SDError[curSz][curBase][curDelta];
	      maxSDSize = curSz;
	      maxSDDelta = curDelta;
	      sdSD = actualSD[curSz][curBase][curDelta];
	    }
	  }
	fprintf(stderr,"Performance: (%d%%) Base %d, Max Corr Error %f @ "
		"image %d X %d delta %d SD %f Max Std Dev Error %f @ image %d "
		"X %d delta %d SD %f\n", noisePct,
		baseValue[curBase], largestCorrError, imageSize[maxCorrSize],
		imageSize[maxCorrSize], deltaValue[maxCorrDelta], corrSD,
		largestSDError, imageSize[maxSDSize], imageSize[maxSDSize],
		deltaValue[maxSDDelta], sdSD);
      }
      
      for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	float largestCorrError = 0.0, largestSDError = 0.0, corrSD = 0.0, sdSD=0.0;
	uint32 maxCorrSize = 0, maxCorrBase = 0;
	uint32 maxSDSize = 0, maxSDBase = 0;
	for (uint32 curSz = 0; curSz < sizeCount; curSz++)
	  for (uint32 curBase = 0; curBase < baseCount; curBase++) {
	    if (corrError[curSz][curBase][curDelta] > largestCorrError) {
	      largestCorrError = corrError[curSz][curBase][curDelta];
	      maxCorrSize = curSz;
	      maxCorrBase = curBase;
	      corrSD = actualSD[curSz][curBase][curDelta];
	    }
	    if (SDError[curSz][curBase][curDelta] > largestSDError) {
	      largestSDError = SDError[curSz][curBase][curDelta];
	      maxSDSize = curSz;
	      maxSDBase = curBase;
	      sdSD = actualSD[curSz][curBase][curDelta];
	    }
	  }
	fprintf(stderr, "Performance: (%d%%) Delta %d, Max Corr Error %f "
		"@ image %d X %d base %d SD %f Max Std Dev Error %f @ image %d "
		"X %d base %d SD %f\n", noisePct,
		deltaValue[curDelta], largestCorrError, imageSize[maxCorrSize],
		imageSize[maxCorrSize], baseValue[maxCorrBase], corrSD,
		largestSDError,	imageSize[maxSDSize], imageSize[maxSDSize],
		baseValue[maxSDBase], sdSD);
      }
#else
      {
	float largestCorrError = 0.0, largestSDError = 0.0, corrSD = 0.0, sdSD=0.0;

	for (uint32 curSz = 0; curSz < sizeCount; curSz++)
	  for (uint32 curBase = 0; curBase < baseCount; curBase++)
	    for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	      if (corrError[curSz][curBase][curDelta] > largestCorrError) {
		largestCorrError = corrError[curSz][curBase][curDelta];
		corrSD = actualSD[curSz][curBase][curDelta];
	      }
	      if (SDError[curSz][curBase][curDelta] > largestSDError) {
		largestSDError = SDError[curSz][curBase][curDelta];
		sdSD = actualSD[curSz][curBase][curDelta];
	      }
	    }
	fprintf(stderr, "Performance: (%d%%) 3 x 3 Line Integral (Min Dev) Largest "
		"Corr. Error %1.2e SD %f Largest Std Dev Error %1.2e SD %f\n",
		noisePct, largestCorrError, corrSD, largestSDError, sdSD);
      }
#endif // GEN_ACCURACY_DATA
    } // End of: Minimum Std Dev Test
  } // End of: Numerical Accuracy Test
}

void UT_moments::timeAutoCorr3by3Point()
{
  const uint32 fullGenLoops = 5, crossGenLoops = 10;
  const uint32 maxImages =
    (fullGenLoops > crossGenLoops) ? fullGenLoops : crossGenLoops;

  vector<rcWindow> ti;
  for (uint32 i = 0; i < maxImages; i++) {
    ti.push_back(rcWindow(1280,960));
    ti[i].randomFill(i);
  }
  
  rcAutoCorrelation acFull, acCross, acFullFast(true), acCrossFast(true);
  rsCorr3by3Point fullResult, crossResult;

  /* Do initial call to update here to allow any necessary allocations
   * to get done before timing test starts.
   */
  acCross.update(ti[maxImages-1], 1, rcAutoCorrelation::eCross,
		 rcAutoCorrelation::ePoint);
  acFull.update(ti[maxImages-1], 1, rcAutoCorrelation::eFull,
		rcAutoCorrelation::ePoint);
  acCrossFast.update(ti[maxImages-1], 1, rcAutoCorrelation::eCross,
		     rcAutoCorrelation::ePoint);
  acFullFast.update(ti[maxImages-1], 1, rcAutoCorrelation::eFull,
		    rcAutoCorrelation::ePoint);

  /* Time integral data generation fcts.
   */
  rcTime tCrossGenData;
  tCrossGenData.start();
  for (uint32 i = 0; i < crossGenLoops; i++)
    acCross.update(ti[i], 1, rcAutoCorrelation::eCross, rcAutoCorrelation::ePoint);
  tCrossGenData.end();
  
  rcTime tFullGenData;
  tFullGenData.start();
  for (uint32 i = 0; i < fullGenLoops; i++)
    acFull.update(ti[i], 1, rcAutoCorrelation::eFull, rcAutoCorrelation::ePoint);
  tFullGenData.end();
  
  rcTime tCrossFastGenData;
  tCrossFastGenData.start();
  for (uint32 i = 0; i < crossGenLoops; i++)
    acCrossFast.update(ti[i], 1, rcAutoCorrelation::eCross,
		       rcAutoCorrelation::ePoint);
  tCrossFastGenData.end();
  
  rcTime tFullFastGenData;
  tFullFastGenData.start();
  for (uint32 i = 0; i < fullGenLoops; i++)
    acFullFast.update(ti[i], 1, rcAutoCorrelation::eFull,
		      rcAutoCorrelation::ePoint);
  tFullFastGenData.end();
  
  /* Time autocorrelation space generation fcts.
   */
  const int32 height = ti[0].height() - 7;
  const int32 width = ti[0].width() - 7;

  rcTime tCrossResults;
  tCrossResults.start();
  for(int32 y = 0; y < height; y++)
    for(int32 x = 0; x < width; x++) {
      rcRect loc(x, y, 7, 7);
      acCross.gen3by3Point(loc, crossResult, -1);
    }
  tCrossResults.end();
  
  rcTime tFullResults;
  tFullResults.start();
  for(int32 y = 0; y < height; y++)
    for(int32 x = 0; x < width; x++) {
      rcRect loc(x, y, 7, 7);
      acFull.gen3by3Point(loc, fullResult, -1);
    }
  tFullResults.end();
  
  rcTime tCrossFastResults;
  tCrossFastResults.start();
  for(int32 y = 0; y < height; y++)
    for(int32 x = 0; x < width; x++) {
      rcRect loc(x, y, 7, 7);
      acCrossFast.gen3by3Point(loc, crossResult, -1);
    }
  tCrossFastResults.end();
  
  rcTime tFullFastResults;
  tFullFastResults.start();
  for(int32 y = 0; y < height; y++)
    for(int32 x = 0; x < width; x++) {
      rcRect loc(x, y, 7, 7);
      acFullFast.gen3by3Point(loc, fullResult, -1);
    }
  tFullFastResults.end();
  
  /* Neatly print results.
   */
  double dMilliSeconds = tCrossGenData.milliseconds()/crossGenLoops;
  fprintf(stderr, "Performance: Gen Cross Point Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  fprintf(stderr, "Performance: Gen Cross Point Results %d 3 x 3 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), tCrossResults.milliseconds(),
	  tCrossResults.microseconds()/((double)(height+1)*(width+1)));

  dMilliSeconds = tFullGenData.milliseconds()/fullGenLoops;
  fprintf(stderr, "Performance: Gen Full Point Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  fprintf(stderr, "Performance: Gen Full Point Results %d 3 x 3 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), tFullResults.milliseconds(),
	  tFullResults.microseconds()/((double)(height+1)*(width+1)));

  dMilliSeconds = tCrossFastGenData.milliseconds()/crossGenLoops;
  fprintf(stderr,
	  "Performance: Gen Cross Point (Fast) Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  fprintf(stderr, "Performance: Gen Cross Point (Fast) Results %d 3 x 3 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), tCrossFastResults.milliseconds(),
	  tCrossFastResults.microseconds()/((double)(height+1)*(width+1)));

  dMilliSeconds = tFullFastGenData.milliseconds()/fullGenLoops;
  fprintf(stderr,
	  "Performance: Gen Full Point (Fast) Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  fprintf(stderr, "Performance: Gen Full Point (Fast) Results %d 3 x 3 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), tFullFastResults.milliseconds(),
	  tFullFastResults.microseconds()/((double)(height+1)*(width+1)));
}

void UT_moments::timeAutoCorr3by3Line()
{
  const uint32 loops = 10;

  vector<rcWindow> ti;
  for (uint32 i = 0; i < loops; i++) {
    ti.push_back(rcWindow(1280,960));
    ti[i].randomFill(i);
  }
  
  rcAutoCorrelation acCross, acFull;
  rsCorr3by3Line crossResult, fullResult;

  /* Do initial call to update here to allow any necessary allocations
   * to get done before timing test starts.
   */
  acCross.update(ti[loops-1], 1, rcAutoCorrelation::eCross,
		 rcAutoCorrelation::eLine);
  acFull.update(ti[loops-1], 1, rcAutoCorrelation::eFull,
		rcAutoCorrelation::eLine);

  /* Time integral data generation fct.
   */
  rcTime tCrossGenData;
  tCrossGenData.start();
  for (uint32 i = 0; i < loops; i++)
    acCross.update(ti[i], 1, rcAutoCorrelation::eCross, rcAutoCorrelation::eLine);
  tCrossGenData.end();
  
  rcTime tFullGenData;
  tFullGenData.start();
  for (uint32 i = 0; i < loops; i++)
    acFull.update(ti[i], 1, rcAutoCorrelation::eFull, rcAutoCorrelation::eLine);
  tFullGenData.end();
  
  /* Time autocorrelation space generation fct.
   */
  const int32 dim = 7;
  const int32 height = ti[0].height() - dim;
  const int32 width = ti[0].width() - dim;
  const rcIPair size(dim, dim);

  rcTime tCrossResults;
  tCrossResults.start();
  for (uint32 i = 0; i < loops; i++) {
    for(int32 y = 0; y < height; y++)
      acCross.gen3by3Line(size, y, crossResult);
  }
  tCrossResults.end();
  
  rcTime tFullResults;
  tFullResults.start();
  for (uint32 i = 0; i < loops; i++) {
    for(int32 y = 0; y < height; y++)
      acFull.gen3by3Line(size, y, fullResult);
  }
  tFullResults.end();
  
  /* Neatly print results.
   */
  double dMilliSeconds = tCrossGenData.milliseconds()/loops;
  fprintf(stderr, "Performance: Gen Cross Line Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  dMilliSeconds = tCrossResults.milliseconds()/loops;
  double dMicroSeconds = tCrossResults.microseconds()/loops;
  fprintf(stderr, "Performance: Gen Cross Line Results %d 3 x 3 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), dMilliSeconds,
	  dMicroSeconds/((double)(height+1)*(width+1)));

  dMilliSeconds = tFullGenData.milliseconds()/loops;
  fprintf(stderr, "Performance: Gen Full Line Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  dMilliSeconds = tFullResults.milliseconds()/loops;
  dMicroSeconds = tFullResults.microseconds()/loops;
  fprintf(stderr, "Performance: Gen Full Line Results %d 3 x 3 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), dMilliSeconds,
	  dMicroSeconds/((double)(height+1)*(width+1)));
}

void UT_moments::testAutoCorr5by5Point()
{
  srand(0);

  /* Since integral based correlation uses floats, there is an
   * inherent inaccuracy in the result. Check that these errors don't
   * exceed the following somewhat arbitrary values.
   */
  const float maxCorrError = 1.0e-6;
  const float maxSDError = 1.0e-5;

  /* Test alignment cases by:
   *
   *   creating a "parent" window
   *
   *   create all possible "child" windows
   *
   *   use the child window as the "frame" to pass to update()
   *
   *   Verify that the correlation values match within tolerances
   *   those returned by rfCorrelate().
   *
   * This is done for both the cross and full cases. Since this is 
   * an exhaustive test, use a relatively small "parent" window.
   */
  {
    const int32 parW = 48;
    const int32 parH = 8;
  
    rcWindow parWin(parW, parH);
    parWin.randomFill(11);

    rcAutoCorrelation acFull, acFullFast(true), acCross, acCrossFast(true);
    rsCorr5by5Point fullResult, fullResultFast, crossResult, crossResultFast;
    rsCorrParams cp;
    rcCorr res;

    float largestCorrError = 0.0;
    float largestSDError = 0.0;

    for (int32 frmYOff = 0; frmYOff < (parH-4); frmYOff++)
      for (int32 frmXOff = 0; frmXOff < (parW-4); frmXOff++) {
	/* Initialize the "child" window or "frame".
	 */
	const int32 fWidth = parW - frmXOff;
	const int32 fHeight = parH - frmYOff;
	rcWindow frame(parWin, frmXOff, frmYOff, fWidth, fHeight);

	acFull.update(frame, 2, rcAutoCorrelation::eFull,
		      rcAutoCorrelation::ePoint);
	acFullFast.update(frame, 2, rcAutoCorrelation::eFull,
			  rcAutoCorrelation::ePoint);
	acCross.update(frame, 2, rcAutoCorrelation::eCross,
		       rcAutoCorrelation::ePoint);
	acCrossFast.update(frame, 2, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::ePoint);

	const int32 maxHeight = (fHeight <= 15) ? fHeight : 15;
	const int32 maxWidth = (fWidth <= 15) ? fWidth : 15;
	for (int32 ySrchSpcSz = 5; ySrchSpcSz <= maxHeight; ySrchSpcSz++) {
	  for (int32 xSrchSpcSz = 5; xSrchSpcSz <= maxWidth; xSrchSpcSz++) {
	    for (int32 yS = 0; yS <= (fHeight - ySrchSpcSz); yS++)
	      for (int32 xS = 0; xS <= (fWidth - xSrchSpcSz); xS++) {
		rcWindow srchSpace(frame, xS, yS, xSrchSpcSz, ySrchSpcSz);
		rcWindow curModel(frame, xS+2, yS+2, xSrchSpcSz-4, ySrchSpcSz-4);
		rcRect loc(xS, yS, xSrchSpcSz, ySrchSpcSz);
		const bool testFast = (xSrchSpcSz-4)*(ySrchSpcSz-4) <= 65536;
		acFull.gen5by5Point(loc, fullResult, -1);
		acCross.gen5by5Point(loc, crossResult, -1);
		if (testFast) {
		  acFullFast.gen5by5Point(loc, fullResultFast, -1);
		  acCrossFast.gen5by5Point(loc, crossResultFast, -1);
		}
		for (uint32 yM = 0; yM < 5; yM++)
		  for (uint32 xM = 0; xM < 5; xM++) {
		    rcWindow curImage(srchSpace, xM, yM, curModel.width(),
				      curModel.height());
		    rfCorrelate (curModel, curImage, cp, res);
		  
		    if ((xM == 2) && (yM == 2)) { // Handle center location case
		      if (testFast) {
			rcUNITTEST_ASSERT(fullResult.score[2][2] ==
					   fullResultFast.score[2][2]);
			rcUNITTEST_ASSERT(crossResult.score[2][2] ==
					   crossResultFast.score[2][2]);
		      }
		      rcUNITTEST_ASSERT(fullResult.score[2][2] ==
					 crossResult.score[2][2]);
		      double N = curModel.width()*curModel.height();
		      double stddev = sqrt(fullResult.score[2][2]/(N*(N-1)));
		      float SDError = fabs(stddev - res.iStd());
		      if (SDError > largestSDError) {
			largestSDError = SDError;
			rcUNITTEST_ASSERT(largestSDError < maxSDError);
		      }
		    }
		    else { // Handle corner and cross location cases
		      rcUNITTEST_ASSERT(fullResult.score[yM][xM] >= 0.0);
		      rcUNITTEST_ASSERT(fullResult.score[yM][xM] <= 1.0);

		      float corrError = fabs(fullResult.score[yM][xM] - res.r());
		      if (corrError > largestCorrError) {
			largestCorrError = corrError;
			rcUNITTEST_ASSERT(largestCorrError < maxCorrError);
		      }
		      if (testFast) {
			rcUNITTEST_ASSERT(fullResult.score[yM][xM] ==
					   fullResultFast.score[yM][xM]);
		      }
		      if ((xM == 2) || (yM == 2)) { // Also a cross location case
			rcUNITTEST_ASSERT(fullResult.score[yM][xM] ==
					   crossResult.score[yM][xM]);
			if (testFast)
			  rcUNITTEST_ASSERT(crossResult.score[yM][xM] ==
					     crossResultFast.score[yM][xM]);
		      }
		    } // End of: if ((xM == 2) && (yM == 2)) ... else ...
		  } // End of: for (uint32 xM = 0; xM < 5; xM++) {
	      } // End of: for ( ... ; xS <= (fWidth - xSrchSpcSz); xS++) {
	  }
	}
      } // End of: for (int32 frmXOff = 0; frmXOff < parW; frmXOff++) {

    fprintf(stderr, "Performance: 5 x 5 Point Integral (Random) Largest Corr. Error"
	    " %1.2e Largest Std Dev Error %1.2e\n",
	    largestCorrError, largestSDError);
  } // End of: Alignment Test

  /* Perform numerical accuracy test. This tries to generate worst case 
   * scenarios. The following cases are tested:
   *
   *   Maximum Std Dev - In this case, a large frame is created where,
   *                     for the "cross" images, half the pixels are
   *                     set to 0 and the other half are set to FF.
   *
   *   Minumum Std Dev - 
   */
  {
    rcAutoCorrelation acCross, acCrossFast(true);  
    rsCorr5by5Point crossResult, crossResultFast;
    rsCorrParams cp;
    rcCorr res;

    { // Maximum Std Dev Test
      const int32 maxDim = 2048;
      rcWindow bigKahuna(maxDim, maxDim);

      /* Set up for "maximum standard deviation" test.
       */
      bigKahuna.setAllPixels(0);

      int32 fudge = 85;
      rcWindow temp1(bigKahuna, fudge, maxDim/3, maxDim - fudge*2, maxDim/3+1);
      temp1.setAllPixels(0xFF);

      rcWindow temp2(bigKahuna, maxDim/3, fudge, maxDim/3, maxDim/3-fudge);
      temp2.setAllPixels(0xFF);

      rcWindow temp3(bigKahuna, maxDim/3, 2*maxDim/3,  maxDim/3, maxDim/3-fudge);
      temp3.setAllPixels(0xFF);
    
      /* Image set up, Ready to do calculations.
       */
      acCross.update(bigKahuna, 2, rcAutoCorrelation::eCross,
		     rcAutoCorrelation::ePoint);
      acCrossFast.update(bigKahuna, 2, rcAutoCorrelation::eCross,
			 rcAutoCorrelation::ePoint);

      int32 tDim[2] = {260, 2048};
      for (uint32 tIndex = 0; tIndex < 2; tIndex++) {
	int32 testDim = tDim[tIndex];
	rcWindow srchSpace(bigKahuna, 0, 0, testDim, testDim);
	rcWindow curModel(bigKahuna, 2, 2, testDim-4, testDim-4);
	rcRect loc(0, 0, testDim, testDim);
	const bool testFast = testDim != 2048;
	acCross.gen5by5Point(loc, crossResult, -1);
	if (testFast)
	  acCrossFast.gen5by5Point(loc, crossResultFast, -1);
	for (uint32 yM = 0; yM < 5; yM++)
	  for (uint32 xM = 0; xM < 5; xM++) {
	    rcWindow curImage(srchSpace, xM, yM, curModel.width(),
			      curModel.height());
	    rfCorrelate (curModel, curImage, cp, res);
	    
	    if ((xM == 2) && (yM == 2)) { // Handle center location case
	      if (testFast)
		rcUNITTEST_ASSERT(crossResult.score[2][2] ==
				   crossResultFast.score[2][2]);
	      double N = curModel.width()*curModel.height();
	      double stddev = sqrt(crossResult.score[2][2]/(N*(N-1)));
	      float SDError = fabs(stddev - res.iStd());
	      rcUNITTEST_ASSERT(SDError < maxSDError);
	    }
	    else if ((xM == 2) || (yM == 2)) {
	      rcUNITTEST_ASSERT(crossResult.score[yM][xM] >= 0.0);
	      rcUNITTEST_ASSERT(crossResult.score[yM][xM] <= 1.0);
	      
	      float corrError = fabs(crossResult.score[yM][xM] - res.r());
	      rcUNITTEST_ASSERT(corrError < maxCorrError);
	      if (testFast)
		rcUNITTEST_ASSERT(crossResult.score[yM][xM] ==
				   crossResultFast.score[yM][xM]);
	    } // End of: if ((xM == 2) && (yM == 2)) ... else ...
	  } // End of: for (uint32 xM = 0; xM < 5; xM++) {
      } // End of: for (int32 tIndex = 0; tIndex < 2; tIndex++) {
    } // End of: Maximum Std Dev Test

    { // Minimum Std Dev Test
      const uint32 noisePct = 10;
      const uint32 sizeCount = 8;
      const int32 imageSize[sizeCount] = {7, 11, 16, 32, 256, 512, 1024, 2048};
      const uint32 baseCount = 8;
      const uint32 baseValue[baseCount] = {0, 128, 192, 224, 240, 248, 252, 254};
      const uint32 deltaCount = 5;
      const uint32 deltaValue[deltaCount] = {1, 2, 4, 8, 16};

      float largestCorrError = 0.0;
      float largestSDError = 0.0;

      for (uint32 curSz = 0; curSz < sizeCount; curSz++) {
	const int32 dim = imageSize[curSz]; 
	rcWindow frame(dim, dim);
	for (uint32 curBase = 0; curBase < baseCount; curBase++)
	  for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	    if ((baseValue[curBase] + deltaValue[curDelta]) > 255)
	      continue;
	    genTestImage(frame, baseValue[curBase], deltaValue[curDelta], noisePct);
	    /* Image set up, Ready to do calculations.
	     */
	    acCross.update(frame, 2, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::ePoint);
	    acCrossFast.update(frame, 2, rcAutoCorrelation::eCross,
			       rcAutoCorrelation::ePoint);
	    const bool testFast = (dim-4)*(dim-4) <= 65536;
	    
	    acCross.gen5by5Point(frame.rcBound(), crossResult, -1);
	    if (testFast)
	      acCrossFast.gen5by5Point(frame.rcBound(), crossResultFast, -1);
	    rcWindow srchSpace(frame, 0, 0, dim, dim);
	    rcWindow curModel(frame, 2, 2, dim-4, dim-4);
	    for (uint32 yM = 0; yM < 5; yM++)
	      for (uint32 xM = 0; xM < 5; xM++) {
		rcWindow curImage(srchSpace, xM, yM, curModel.width(),
				  curModel.height());
		rfCorrelate (curModel, curImage, cp, res);
		
		if ((xM == 2) && (yM == 2)) { // Handle center location case
		  if (testFast)
		    rcUNITTEST_ASSERT(crossResult.score[2][2] ==
				       crossResultFast.score[2][2]);
		  double N = curModel.width()*curModel.height();
		  double stddev = sqrt(crossResult.score[2][2]/(N*(N-1)));
		  float SDError = fabs(stddev - res.iStd());
		  if (SDError > largestSDError) {
		    largestSDError = SDError;
		    rcUNITTEST_ASSERT(SDError < maxSDError);
		  }
		}
		else if ((xM == 2) || (yM == 2)) {
		  rcUNITTEST_ASSERT(crossResult.score[yM][xM] >= 0.0);
		  rcUNITTEST_ASSERT(crossResult.score[yM][xM] <= 1.0);
		  
		  float corrError = fabs(crossResult.score[yM][xM] - res.r());
		  if (corrError > largestCorrError) {
		    largestCorrError = corrError;
		    rcUNITTEST_ASSERT(corrError < maxCorrError);
		  }
		  if (testFast)
		    rcUNITTEST_ASSERT(crossResult.score[yM][xM] ==
				       crossResultFast.score[yM][xM]);
		} // End of: if ((xM == 2) && (yM == 2)) ... else ...
	      } // End of: for (uint32 xM = 0; xM < 5; xM++) {
	  } // End of: for ( ... ; curDelta < fgDeltaCount; curDelta++) {
      } // End of: for (uint32 curSz = 0; curSz < sizeCount; curSz++) {
      fprintf(stderr, "Performance: (%d%%) 5 x 5 Point Integral (Min Dev) "
	      "Largest Corr. Error %1.2e Largest Std Dev Error %1.2e\n",
	      noisePct, largestCorrError, largestSDError);
    } // End of: Minimumm Std Dev Test
  } // End of: Numerical Accuracy Test
}

void UT_moments::timeAutoCorr5by5Point()
{
  const uint32 fullGenLoops = 4, crossGenLoops = 8;
  const uint32 maxImages =
    (fullGenLoops > crossGenLoops) ? fullGenLoops : crossGenLoops;

  vector<rcWindow> ti;
  for (uint32 i = 0; i < maxImages; i++) {
    ti.push_back(rcWindow(1280,960));
    ti[i].randomFill(i);
  }
  
  rcAutoCorrelation acFull, acCross, acFullFast(true), acCrossFast(true);
  rsCorr5by5Point fullResult, crossResult;

  /* Do initial call to update here to allow any necessary allocations
   * to get done before timing test starts.
   */
  acCross.update(ti[maxImages-1], 2, rcAutoCorrelation::eCross,
		 rcAutoCorrelation::ePoint);
  acFull.update(ti[maxImages-1], 2, rcAutoCorrelation::eFull,
		rcAutoCorrelation::ePoint);
  acCrossFast.update(ti[maxImages-1], 2, rcAutoCorrelation::eCross,
		     rcAutoCorrelation::ePoint);
  acFullFast.update(ti[maxImages-1], 2, rcAutoCorrelation::eFull,
		    rcAutoCorrelation::ePoint);

  /* Time integral data generation fcts.
   */
  rcTime tCrossGenData;
  tCrossGenData.start();
  for (uint32 i = 0; i < crossGenLoops; i++)
    acCross.update(ti[i], 2, rcAutoCorrelation::eCross, rcAutoCorrelation::ePoint);
  tCrossGenData.end();
  
  rcTime tFullGenData;
  tFullGenData.start();
  for (uint32 i = 0; i < fullGenLoops; i++)
    acFull.update(ti[i], 2, rcAutoCorrelation::eFull, rcAutoCorrelation::ePoint);
  tFullGenData.end();
  
  rcTime tCrossFastGenData;
  tCrossFastGenData.start();
  for (uint32 i = 0; i < crossGenLoops; i++)
    acCrossFast.update(ti[i], 2, rcAutoCorrelation::eCross,
		       rcAutoCorrelation::ePoint);
  tCrossFastGenData.end();
  
  rcTime tFullFastGenData;
  tFullFastGenData.start();
  for (uint32 i = 0; i < fullGenLoops; i++)
    acFullFast.update(ti[i], 2, rcAutoCorrelation::eFull,
		      rcAutoCorrelation::ePoint);
  tFullFastGenData.end();
  
  /* Time autocorrelation space generation fcts.
   */
  const int32 height = ti[0].height() - 9;
  const int32 width = ti[0].width() - 9;

  rcTime tCrossResults;
  tCrossResults.start();
  for(int32 y = 0; y < height; y++)
    for(int32 x = 0; x < width; x++) {
      rcRect loc(x, y, 9, 9);
      acCross.gen5by5Point(loc, crossResult, -1);
    }
  tCrossResults.end();
  
  rcTime tFullResults;
  tFullResults.start();
  for(int32 y = 0; y < height; y++)
    for(int32 x = 0; x < width; x++) {
      rcRect loc(x, y, 9, 9);
      acFull.gen5by5Point(loc, fullResult, -1);
    }
  tFullResults.end();
  
  rcTime tCrossFastResults;
  tCrossFastResults.start();
  for(int32 y = 0; y < height; y++)
    for(int32 x = 0; x < width; x++) {
      rcRect loc(x, y, 9, 9);
      acCrossFast.gen5by5Point(loc, crossResult, -1);
    }
  tCrossFastResults.end();
  
  rcTime tFullFastResults;
  tFullFastResults.start();
  for(int32 y = 0; y < height; y++)
    for(int32 x = 0; x < width; x++) {
      rcRect loc(x, y, 9, 9);
      acFullFast.gen5by5Point(loc, fullResult, -1);
    }
  tFullFastResults.end();
  
  /* Neatly print results.
   */
  double dMilliSeconds = tCrossGenData.milliseconds()/crossGenLoops;
  fprintf(stderr, "Performance: Gen Cross Point Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  fprintf(stderr, "Performance: Gen Cross Point Results %d 5 x 5 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), tCrossResults.milliseconds(),
	  tCrossResults.microseconds()/((double)(height+1)*(width+1)));

  dMilliSeconds = tFullGenData.milliseconds()/fullGenLoops;
  fprintf(stderr, "Performance: Gen Full Point Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  fprintf(stderr, "Performance: Gen Full Point Results %d 5 x 5 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), tFullResults.milliseconds(),
	  tFullResults.microseconds()/((double)(height+1)*(width+1)));

  dMilliSeconds = tCrossFastGenData.milliseconds()/crossGenLoops;
  fprintf(stderr,
	  "Performance: Gen Cross Point (Fast) Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  fprintf(stderr, "Performance: Gen Cross Point (Fast) Results %d 5 x 5 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), tCrossFastResults.milliseconds(),
	  tCrossFastResults.microseconds()/((double)(height+1)*(width+1)));

  dMilliSeconds = tFullFastGenData.milliseconds()/fullGenLoops;
  fprintf(stderr,
	  "Performance: Gen Full Point (Fast) Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  fprintf(stderr, "Performance: Gen Full Point (Fast) Results %d 5 x 5 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), tFullFastResults.milliseconds(),
	  tFullFastResults.microseconds()/((double)(height+1)*(width+1)));
}

void UT_moments::testAutoCorr5by5Line()
{
  srand(0);

  /* Since integral based correlation uses floats, there is an
   * inherint inaccuracy in the result. Check that these errors don't
   * exceed the following somewhat arbitrary values.
   */
  const float maxCorrError = 1.0e-6;
  const float maxSDError = 1.4e-5;

  /* Test alignment cases by:
   *
   *   creating a "parent" window
   *
   *   create all possible "child" windows
   *
   *   use the child window as the "frame" to pass to update()
   *
   *   Verify that the correlation values match within tolerances
   *   those returned by rfCorrelate().
   *
   * This is done for both the cross and full cases. Since this is 
   * an exhaustive test, use a relatively small "parent" window.
   */
  {
    const int32 parW = 48;
    const int32 parH = 12;
  
    rcWindow parWin(parW, parH);
    parWin.randomFill(11);

    rcAutoCorrelation acLineCross, acLineFull;
    rsCorr5by5Line lnRsltCross, lnRsltFull;
    rsCorrParams cp;
    rcCorr res;

    float largestCorrError = 0.0;
    float largestSDError = 0.0;

    for (int32 frmYOff = 0; frmYOff < (parH-4); frmYOff++)
      for (int32 frmXOff = 0; frmXOff < (parW-4); frmXOff++) {
	/* Initialize the "child" window or "frame".
	 */
	const int32 fWidth = parW - frmXOff;
	const int32 fHeight = parH - frmYOff;
	rcWindow frame(parWin, frmXOff, frmYOff, fWidth, fHeight);

	acLineCross.update(frame, 2, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::eLine);
	acLineFull.update(frame, 2, rcAutoCorrelation::eFull,
			  rcAutoCorrelation::eLine);

	const int32 maxHeight = (fHeight <= 15) ? fHeight : 15;
	const int32 maxWidth = (fWidth <= 15) ? fWidth : 15;
	for (int32 ySrchSpcSz = 5; ySrchSpcSz <= maxHeight; ySrchSpcSz++) {
	  for (int32 xSrchSpcSz = 5; xSrchSpcSz <= maxWidth; xSrchSpcSz++) {
	    rcIPair srchSize(xSrchSpcSz, ySrchSpcSz);
	    for (int32 yS = 0; yS <= (fHeight - ySrchSpcSz); yS++) {
	      acLineCross.gen5by5Line(srchSize, yS, lnRsltCross);
	      rcUNITTEST_ASSERT((int32)lnRsltCross.count ==
				 (fWidth - xSrchSpcSz + 1));
	      acLineFull.gen5by5Line(srchSize, yS, lnRsltFull);
	      rcUNITTEST_ASSERT((int32)lnRsltFull.count ==
				 (fWidth - xSrchSpcSz + 1));
	      for (int32 xS = 0; xS <= (fWidth - xSrchSpcSz); xS++) {
		rcWindow srchSpace(frame, xS, yS, xSrchSpcSz, ySrchSpcSz);
		rcWindow curModel(frame, xS+2, yS+2, xSrchSpcSz-4, ySrchSpcSz-4);
	     
		for (uint32 yM = 0; yM < 5; yM++)
		  for (uint32 xM = 0; xM < 5; xM++) {
		    rcWindow curImage(srchSpace, xM, yM, curModel.width(),
				      curModel.height());
		    rfCorrelate(curModel, curImage, cp, res);
		  
		    if ((xM == 2) && (yM == 2)) { // Handle center location case
		      double N = curModel.width()*curModel.height();
		      double stddev =
			sqrt(*(lnRsltFull.score[2][2] + xS)/(N*(N-1)));
		      float SDError = fabs(stddev - res.iStd());
		      if (SDError > largestSDError) {
			largestSDError = SDError;
			rcUNITTEST_ASSERT(largestSDError < maxSDError);
		      }
		      rcUNITTEST_ASSERT(*(lnRsltCross.score[2][2] + xS) ==
					 *(lnRsltFull.score[2][2] + xS));
		    }
		    else {
		      rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM] + xS) >= 0.0);
		      rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM] + xS) <= 1.0);
		      float corrError =
			fabs(*(lnRsltFull.score[yM][xM] + xS) - res.r());
		      if (corrError > largestCorrError) {
			largestCorrError = corrError;
			rcUNITTEST_ASSERT(largestCorrError < maxCorrError);
		      }
		      if ((xM == 2) || (yM == 2)) { // Handle cross location cases
			rcUNITTEST_ASSERT(*(lnRsltCross.score[yM][xM] + xS) ==
					   *(lnRsltFull.score[yM][xM] + xS));
		      }
		    } // End of: if ((xM == 2) && (yM == 2)) ... else ...
		  } // End of: for (uint32 xM = 0; xM < 5; xM++) {
	      } // End of: for ( ... ; xS <= (width - xSrchSpcSz); xS++) {
	    } // End of: for ( ... ; yS <= (height - ySrchSpcSz); yS++) {
	  }
	}
      } // End of: for (int32 frmXOff = 0; frmXOff < parW; frmXOff++) {
    fprintf(stderr, "Performance: 5 x 5 Line Integral (Random) Largest Corr."
	    " Error %1.2e Largest Std Dev Error %1.2e\n",
	    largestCorrError, largestSDError);
  } // End of: Alignment Test


  /* Perform numerical accuracy test. This tries to generate worst case 
   * scenarios. The following cases are tested:
   *
   *   Maximum Std Dev - In this case, a large frame is created where,
   *                     for the "cross" images, half the pixels are
   *                     set to 0 and the other half are set to FF.
   *
   *   Minumum Std Dev - 
   */
  {
    rcAutoCorrelation acLineCross, acLineFull;
    rsCorr5by5Line lnRsltCross, lnRsltFull;
    rsCorrParams cp;
    rcCorr res;

    { // Maximum Std Dev Test
      const int32 maxDim = 2048;
      rcWindow bigKahuna(maxDim, maxDim);

      /* Set up for "maximum standard deviation" test.
       */
      bigKahuna.setAllPixels(0);

      int32 fudge = 85;
      rcWindow temp1(bigKahuna, fudge, maxDim/3, maxDim - fudge*2, maxDim/3+1);
      temp1.setAllPixels(0xFF);

      rcWindow temp2(bigKahuna, maxDim/3, fudge, maxDim/3, maxDim/3-fudge);
      temp2.setAllPixels(0xFF);

      rcWindow temp3(bigKahuna, maxDim/3, 2*maxDim/3,  maxDim/3, maxDim/3-fudge);
      temp3.setAllPixels(0xFF);
    
      /* Image set up, Ready to do calculations.
       */
      acLineCross.update(bigKahuna, 2, rcAutoCorrelation::eCross,
			 rcAutoCorrelation::eLine);
      acLineFull.update(bigKahuna, 2, rcAutoCorrelation::eFull,
			rcAutoCorrelation::eLine);

      rcIPair srchSize(maxDim, maxDim);
      rcWindow srchSpace(bigKahuna, 0, 0, maxDim, maxDim);
      rcWindow curModel(bigKahuna, 2, 2, maxDim-4, maxDim-4);
      acLineCross.gen5by5Line(srchSize, 0, lnRsltCross);
      acLineFull.gen5by5Line(srchSize, 0, lnRsltFull);
      for (uint32 yM = 0; yM < 5; yM++)
	for (uint32 xM = 0; xM < 5; xM++) {
	  rcWindow curImage(srchSpace, xM, yM, curModel.width(),
			    curModel.height());
	  rfCorrelate (curModel, curImage, cp, res);
	  
	  if ((xM == 2) && (yM == 2)) { // Handle center location case
	    double N = curModel.width()*curModel.height();
	    double stddev = sqrt(*(lnRsltFull.score[2][2])/(N*(N-1)));
	    float SDError = fabs(stddev - res.iStd());
	    rcUNITTEST_ASSERT(SDError < maxSDError);
	    rcUNITTEST_ASSERT(*(lnRsltCross.score[2][2]) ==
			       *(lnRsltFull.score[2][2]));
	  }
	  else {
	    rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM]) >= 0.0);
	    rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM]) <= 1.0);
	  
	    float corrError = fabs(*(lnRsltFull.score[yM][xM]) - res.r());
	    rcUNITTEST_ASSERT(corrError < maxCorrError);
	    if ((xM == 2) || (yM == 2)) {
	      rcUNITTEST_ASSERT(*(lnRsltCross.score[yM][xM]) ==
				 *(lnRsltFull.score[yM][xM]));
	    }
	  } // End of: if ((xM == 2) && (yM == 2)) ... else ...
	} // End of: for (uint32 xM = 0; xM < 5; xM++) {
    } // End of: Maximum Std Dev Test

    { // Minimum Std Dev Test
      const uint32 noisePct = 10;
      const uint32 sizeCount = 8;
      const int32 imageSize[sizeCount] = {7, 11, 16, 32, 256, 512, 1024, 2048};
      const uint32 baseCount = 8;
      const uint32 baseValue[baseCount] = {0, 128, 192, 224, 240, 248, 252, 254};
      const uint32 deltaCount = 5;
      const uint32 deltaValue[deltaCount] = {1, 2, 4, 8, 16};

      float corrError[sizeCount][baseCount][deltaCount];
      float SDError[sizeCount][baseCount][deltaCount];
      float actualSD[sizeCount][baseCount][deltaCount];

      for (uint32 curSz = 0; curSz < sizeCount; curSz++)
	for (uint32 curBase = 0; curBase < baseCount; curBase++)
	  for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	    corrError[curSz][curBase][curDelta] =
	      SDError[curSz][curBase][curDelta] = 0.0;
	  }

      for (uint32 curSz = 0; curSz < sizeCount; curSz++) {
	const int32 dim = imageSize[curSz]; 
	rcWindow frame(dim, dim);
	for (uint32 curBase = 0; curBase < baseCount; curBase++)
	  for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	    if ((baseValue[curBase] + deltaValue[curDelta]) > 255)
	      continue;
	    genTestImage(frame, baseValue[curBase], deltaValue[curDelta],
			 noisePct);

	    /* Image set up, Ready to do calculations.
	     */
	    acLineCross.update(frame, 2, rcAutoCorrelation::eCross,
			       rcAutoCorrelation::eLine);
	    acLineFull.update(frame, 2, rcAutoCorrelation::eFull,
			      rcAutoCorrelation::eLine);
	    rcIPair size(dim, dim);
	    acLineCross.gen5by5Line(size, 0, lnRsltCross);
	    acLineFull.gen5by5Line(size, 0, lnRsltFull);
	    rcWindow srchSpace(frame, 0, 0, dim, dim);
	    rcWindow curModel(frame, 2, 2, dim-4, dim-4);
	    for (uint32 yM = 0; yM < 5; yM++)
	      for (uint32 xM = 0; xM < 5; xM++) {
		rcWindow curImage(srchSpace, xM, yM, curModel.width(),
				  curModel.height());
		rfCorrelate (curModel, curImage, cp, res);
		
		if ((xM == 2) && (yM == 2)) { // Handle center location case
		  double N = curModel.width()*curModel.height();
		  double stddev = sqrt(*(lnRsltFull.score[2][2])/(N*(N-1)));
		  SDError[curSz][curBase][curDelta] = fabs(stddev - res.iStd());
		  actualSD[curSz][curBase][curDelta] = stddev;
		  rcUNITTEST_ASSERT(*(lnRsltCross.score[2][2]) ==
				     *(lnRsltFull.score[2][2]));
		}
		else {
		  rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM]) >= 0.0);
		  rcUNITTEST_ASSERT(*(lnRsltFull.score[yM][xM]) <= 1.0);
		  
		  corrError[curSz][curBase][curDelta] =
		    fabs(*(lnRsltFull.score[yM][xM]) - res.r());
		  if ((xM == 2) || (yM == 2)) {
		    rcUNITTEST_ASSERT(*(lnRsltCross.score[yM][xM]) ==
				       *(lnRsltFull.score[yM][xM]));
		  }
		} // End of: if ((xM == 2) && (yM == 2)) ... else ...
	      } // End of: for (uint32 xM = 0; xM < 5; xM++) {
	  } // End of: for ( ... ; curDelta < fgDeltaCount; curDelta++) {
      } // End of: for (uint32 curSz = 0; curSz < sizeCount; curSz++) {
      
      {
	float largestCorrError = 0.0, largestSDError = 0.0, corrSD = 0.0, sdSD=0.0;

	for (uint32 curSz = 0; curSz < sizeCount; curSz++)
	  for (uint32 curBase = 0; curBase < baseCount; curBase++)
	    for (uint32 curDelta = 0; curDelta < deltaCount; curDelta++) {
	      if (corrError[curSz][curBase][curDelta] > largestCorrError) {
		largestCorrError = corrError[curSz][curBase][curDelta];
		corrSD = actualSD[curSz][curBase][curDelta];
	      }
	      if (SDError[curSz][curBase][curDelta] > largestSDError) {
		largestSDError = SDError[curSz][curBase][curDelta];
		sdSD = actualSD[curSz][curBase][curDelta];
	      }
	    }
	fprintf(stderr, "Performance: (%d%%) 5 x 5 Line Integral (Min Dev) Largest "
		"Corr. Error %1.2e SD %f Largest Std Dev Error %1.2e SD %f\n",
		noisePct, largestCorrError, corrSD, largestSDError, sdSD);
      }
    } // End of: Minimum Std Dev Test
  } // End of: Numerical Accuracy Test
}

void UT_moments::timeAutoCorr5by5Line()
{
  const uint32 loops = 10;

  vector<rcWindow> ti;
  for (uint32 i = 0; i < loops; i++) {
    ti.push_back(rcWindow(1280,960));
    ti[i].randomFill(i);
  }
  
  rcAutoCorrelation acCross, acFull;
  rsCorr5by5Line crossResult, fullResult;

  /* Do initial call to update here to allow any necessary allocations
   * to get done before timing test starts.
   */
  acCross.update(ti[loops-1], 2, rcAutoCorrelation::eCross,
		 rcAutoCorrelation::eLine);
  acFull.update(ti[loops-1], 2, rcAutoCorrelation::eFull,
		rcAutoCorrelation::eLine);

  /* Time integral data generation fct.
   */
  rcTime tCrossGenData;
  tCrossGenData.start();
  for (uint32 i = 0; i < loops; i++)
    acCross.update(ti[i], 2, rcAutoCorrelation::eCross, rcAutoCorrelation::eLine);
  tCrossGenData.end();
  
  rcTime tFullGenData;
  tFullGenData.start();
  for (uint32 i = 0; i < loops; i++)
    acFull.update(ti[i], 2, rcAutoCorrelation::eFull, rcAutoCorrelation::eLine);
  tFullGenData.end();
  
  /* Time autocorrelation space generation fct.
   */
  const int32 dim = 9;
  const int32 height = ti[0].height() - dim;
  const int32 width = ti[0].width() - dim;
  const rcIPair size(dim, dim);

  rcTime tCrossResults;
  tCrossResults.start();
  for (uint32 i = 0; i < loops; i++) {
    for(int32 y = 0; y < height; y++)
      acCross.gen5by5Line(size, y, crossResult);
  }
  tCrossResults.end();
  
  rcTime tFullResults;
  tFullResults.start();
  for (uint32 i = 0; i < loops; i++) {
    for(int32 y = 0; y < height; y++)
      acFull.gen5by5Line(size, y, fullResult);
  }
  tFullResults.end();
  
  /* Neatly print results.
   */
  double dMilliSeconds = tCrossGenData.milliseconds()/loops;
  fprintf(stderr, "Performance: Gen Cross Line Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  dMilliSeconds = tCrossResults.milliseconds()/loops;
  double dMicroSeconds = tCrossResults.microseconds()/loops;
  fprintf(stderr, "Performance: Gen Cross Line Results %d 5 x 5 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), dMilliSeconds,
	  dMicroSeconds/((double)(height+1)*(width+1)));

  dMilliSeconds = tFullGenData.milliseconds()/loops;
  fprintf(stderr, "Performance: Gen Full Line Integrals [%i x %i %i]: %.2f ms\n",
	  ti[0].width(), ti[0].height(), ti[0].depth()*8, dMilliSeconds);

  dMilliSeconds = tFullResults.milliseconds()/loops;
  dMicroSeconds = tFullResults.microseconds()/loops;
  fprintf(stderr, "Performance: Gen Full Line Results %d 5 x 5 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (height+1)*(width+1), dMilliSeconds,
	  dMicroSeconds/((double)(height+1)*(width+1)));
}

void UT_moments::testAutoCorrPoint()
{
  srand(0);

  /* Since integral based correlation uses floats, there is an
   * inherent inaccuracy in the result. Check that these errors don't
   * exceed the following somewhat arbitrary values.
   */
  const float maxCorrError = 1.0e-6;
  const float maxSDError = 1.0e-5;

  /* Test alignment cases by:
   *
   *   Creating a "parent" window
   *
   *   Create all possible "child" windows
   *
   *   Use the child window as the "frame" to pass to update()
   *
   *   Create five rcAutoCorrelation objects: one with no integral
   *   information, two with 3 x 3 integral information (full and
   *   cross) and two with 5 x 5 integral information (full and cross).
   *
   *   Against these three rcAutoCorrelation objects calculate 3 x 3,
   *   5 x 5 and 7 x 7 autocorrelation spaces. Check for exact match
   *   against results returned by point correlation fcts, where
   *   appropriate.
   *
   * This is done for both the cross and full cases. Since this is 
   * an exhaustive test, use a relatively small "parent" window.
   */
  {
    const int32 parW = 20;
    const int32 parH = 12;
  
    rcWindow parWin(parW, parH);
    parWin.randomFill(11);

    rcAutoCorrelation acNone, acCross3by3, acCross5by5, acFull3by3, acFull5by5;
    rsCorr5by5Point expFull5by5Result;
    vector<vector<float> > expResult(7);
    vector<vector<float> > actResult(expResult.size());
    rsCorrParams cp;
    rcCorr res;

    for (uint32 i = 0; i < expResult.size(); i++) {
      expResult[i].resize(expResult.size());
      actResult[i].resize(expResult.size());
    }

    for (int32 frmYOff = 0; frmYOff < (parH-8); frmYOff++)
      for (int32 frmXOff = 0; frmXOff < (parW-8); frmXOff++) {
	/* Initialize the "child" window or "frame".
	 */
	const int32 fWidth = parW - frmXOff;
	const int32 fHeight = parH - frmYOff;
	rcWindow frame(parWin, frmXOff, frmYOff, fWidth, fHeight);

	acNone.update(frame, 0, rcAutoCorrelation::eFull,
		      rcAutoCorrelation::ePoint, true);
	acFull3by3.update(frame, 1, rcAutoCorrelation::eFull,
			  rcAutoCorrelation::ePoint, true);
	acCross3by3.update(frame, 1, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::ePoint, true);
	acFull5by5.update(frame, 2, rcAutoCorrelation::eFull,
			  rcAutoCorrelation::ePoint, true);
	acCross5by5.update(frame, 2, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::ePoint, true);

	const int32 maxHeight = (fHeight <= 15) ? fHeight : 15;
	const int32 maxWidth = (fWidth <= 15) ? fWidth : 15;
	for (int32 ySrchSpcSz = 9; ySrchSpcSz <= maxHeight; ySrchSpcSz++) {
	  for (int32 xSrchSpcSz = 9; xSrchSpcSz <= maxWidth; xSrchSpcSz++) {
	    for (int32 yS = 0; yS <= (fHeight - ySrchSpcSz); yS++)
	      for (int32 xS = 0; xS <= (fWidth - xSrchSpcSz); xS++) {

		/* Step 1 - Calculate all the expected results.
		 */
		rcWindow srchSpace(frame, xS, yS, xSrchSpcSz, ySrchSpcSz);
		rcWindow curModel(frame, xS+3, yS+3, xSrchSpcSz-6, ySrchSpcSz-6);
		rcRect loc7(xS, yS, xSrchSpcSz, ySrchSpcSz);
		rcRect loc5(xS+1, yS+1, xSrchSpcSz-2, ySrchSpcSz-2);
		rcRect loc3(xS+2, yS+2, xSrchSpcSz-4, ySrchSpcSz-4);

		acFull5by5.gen5by5Point(loc5, expFull5by5Result, -1);

		for (uint32 yM = 0; yM < 7; yM++)
		  for (uint32 xM = 0; xM < 7; xM++) {
		    rcWindow curImage(srchSpace, xM, yM, curModel.width(),
				      curModel.height());
		    rfCorrelate (curModel, curImage, cp, res);
		  
		    if ((xM == 3) & (yM == 3)) { // Handle center location case
		      expResult[3][3] = res.n()*res.Sii() - res.Si()*res.Si();
		    }
		    else {
		      expResult[yM][xM] = res.r();
		    } // End of: if ((xM == 3) & (yM == 3)) ... else ...
		  } // End of: for (uint32 xM = 0; xM < 7; xM++) {

		/* Step 2 - Generate the actual results and compare
		 * with the expected ones.
		 *
		 * For each rcAutoCorrelation object, test in following order:
		 *   3x3 full, 3x3 cross, 5x5 full, 5x5 cross, 7x7 full, 7x7 cross
		 *
		 * For all except acNone, verify that any integral image
		 * based results match those returned by gen5by5Point(). For
		 * all others test against results returned by rfCorrelate().
		 */
		float error;
		rcAutoCorrelation* ac[5] = { &acNone, &acCross3by3, &acCross5by5,
					     &acFull3by3, &acFull5by5 };
		
		for (uint32 ai = 0; ai < 5; ai++) {
		  for (int32 sz = 3; sz < 9; sz += 2) {
		    rcRect& locR = (sz == 3) ? loc3 : ((sz == 5) ? loc5 : loc7);
		    const int32 ctr = (sz - 1)/2;
		    const int32 off = (9 - sz)/2 - 1;
		    ac[ai]->genPoint(locR, actResult, ctr, -1,
				     rcAutoCorrelation::eFull);
		    for (int32 y = 0; y < sz; y++)
		      for (int32 x = 0; x < sz; x++) {
			error = fabs(actResult[y][x] - expResult[y+off][x+off]);
			if ((y == x) && (y == ctr))
			  rcUNITTEST_ASSERT(error < maxSDError);
			else
			  rcUNITTEST_ASSERT(error < maxCorrError);
		      }

		    ac[ai]->genPoint(locR, actResult, ctr, -1,
				     rcAutoCorrelation::eCross);
		    for (int32 i = 0; i < sz; i++) {
		      error = fabs(actResult[i][ctr] - expResult[i+off][ctr+off]);
		      if (i == ctr) {
			rcUNITTEST_ASSERT(error < maxSDError);
			continue;
		      }
		      rcUNITTEST_ASSERT(error < maxCorrError);
		      
		      error = fabs(actResult[ctr][i] - expResult[ctr+off][i+off]);
		      rcUNITTEST_ASSERT(error < maxCorrError);
		    }
		  } // End of: for (int32 sz = 3; sz < 9; sz += 2) {
		} // End of: for (uint32 ai = 0; ai < 5; ai++) {
	      } // End of: for ( ... ; xS <= (width - xSrchSpcSz); xS++) {
	  }
	}
      } // End of: for (int32 frmXOff = 0; frmXOff < parW; frmXOff++) {
  } // End of: Alignment Test

	  
}

void UT_moments::timeAutoCorrPoint()
{
  rcWindow ti(1280, 960);
  ti.randomFill(26);
  rcWindow tiSmall(1280/8, 960/8);
  tiSmall.randomFill(26);
  
  vector<vector<float> > vecResult(7);
  for (uint32 i = 0; i < vecResult.size(); i++)
    vecResult[i].resize(vecResult.size());

  /* Set up integral images
   */
  rcAutoCorrelation acNone, acFull3, acFull5;
  acNone.update(ti, 0, rcAutoCorrelation::eFull, rcAutoCorrelation::ePoint, true);
  acFull3.update(ti, 1, rcAutoCorrelation::eFull, rcAutoCorrelation::ePoint, true);
  acFull5.update(ti, 2, rcAutoCorrelation::eFull, rcAutoCorrelation::ePoint, true);

  /* Time autocorrelation space generation fcts.
   */
  rcTime tResults3, tResults5, tResults7;
  {
    const int32 spcSz = 5;
    const int32 height = ti.height() - spcSz;
    const int32 width = ti.width() - spcSz;
  
    tResults3.start();
    for(int32 y = 0; y < height; y++)
      for(int32 x = 0; x < width; x++) {
	rcRect loc(x, y, spcSz, spcSz);
	acFull3.genPoint(loc, vecResult, 1, -1, rcAutoCorrelation::eFull);
      }
    tResults3.end();
  }
  
  {
    const int32 spcSz = 7;
    const int32 height = ti.height() - spcSz;
    const int32 width = ti.width() - spcSz;
  
    tResults5.start();
    for(int32 y = 0; y < height; y++)
      for(int32 x = 0; x < width; x++) {
	rcRect loc(x, y, spcSz, spcSz);
	acFull5.genPoint(loc, vecResult, 2, -1, rcAutoCorrelation::eFull);
      }
    tResults5.end();
  }
  
  {
    const int32 spcSz = 9;
    const int32 height = tiSmall.height() - spcSz;
    const int32 width = tiSmall.width() - spcSz;
  
    tResults7.start();
    for(int32 y = 0; y < height; y++)
      for(int32 x = 0; x < width; x++) {
	rcRect loc(x, y, spcSz, spcSz);
	acNone.genPoint(loc, vecResult, 3, -1, rcAutoCorrelation::eFull);
      }
    tResults7.end();
  }
  
  /* Neatly print results.
   */
  {
    const int32 spcSz = 5;
    const int32 height = ti.height() - spcSz;
    const int32 width = ti.width() - spcSz;

    fprintf(stderr, "Performance: Generic Full Point Results %d %d x %d spaces: "
	    "%.2f ms, %.2f us per space\n",
	    (height+1)*(width+1), 3, 3, tResults3.milliseconds(),
	    tResults3.microseconds()/((double)(height+1)*(width+1)));
  }
  {
    const int32 spcSz = 7;
    const int32 height = ti.height() - spcSz;
    const int32 width = ti.width() - spcSz;

    fprintf(stderr, "Performance: Generic Full Point Results %d %d x %d spaces: "
	    "%.2f ms, %.2f us per space\n",
	    (height+1)*(width+1), 5, 5, tResults5.milliseconds(),
	    tResults5.microseconds()/((double)(height+1)*(width+1)));
  }
  {
    const int32 spcSz = 9;
    const int32 height = tiSmall.height() - spcSz;
    const int32 width = tiSmall.width() - spcSz;

    fprintf(stderr, "Performance: Generic Full Point Results %d %d x %d spaces: "
	    "%.2f ms, %.2f us per space\n",
	    (height+1)*(width+1), 7, 7, tResults7.milliseconds(),
	    tResults7.microseconds()/((double)(height+1)*(width+1)));
  }
}

void UT_moments::testAutoCorrGeneralAnd16 ()

// Test AutoCorr with 16 bit image versus 8 bit 
{
  int32 side (11);
  rcIPair target (side, side);
  rcIPair target4moments (side + 4, side + 4);

  rcAutoCorrelation   ac38, ac8, ac16, *ac[2]; ac[0] = &ac8; ac[1] = &ac16;
  int32 h (target4moments.x()+3), w (target4moments.y()+3);
  rcWindow src16(w, h, rcPixel16);
  src16.setAllPixels (1);
  src16.setPixel (target.x()/2, target.y() / 2,2);
  src16.setPixel (target.x()/2 + target.x() - 1, target.y() / 2,2);
  rcWindow src8(w, h, rcPixel8);
  src8.setAllPixels (1);
  src8.setPixel (target.x()/2, target.y() / 2,2);
  src8.setPixel (target.x()/2 + target.x() - 1, target.y() / 2,2);

  rcIPair searchSpace = src8.size () - target4moments + 1;
  rsCorr5by5Line results8, results16;
  rsCorr3by3Line results38;
  ac8.update (src8, 2,  rcAutoCorrelation::eFull, rcAutoCorrelation::eLine);
		//  ac16.update (src16, 2,  rcAutoCorrelation::eFull, rcAutoCorrelation::eLine, false, target);
    
  for (uint32 j = 0; j < searchSpace.y(); j++)
    {
      ac8.gen5by5Line(target4moments, j, results8);
			//     ac16.genLine (target, j, results16);
	
      rcUTCheck (results8.count == searchSpace.x());
      rcUTCheck (results16.count == searchSpace.x());
  
      for (uint32 i = 0; i < searchSpace.x(); i++)
	{
	  float a8 =  (*(results8.score[2][2] + i ));
	  float a16 =  (*(results16.score[2][2] + i  ));
	  rcUTCheck (real_equal (a8, a16, 0.00001f));
	}
    }
}
