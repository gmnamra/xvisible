/*
 *	ut_point_corr.cpp 06/05/2003 
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */
#include "ut_point_corr.h"
#include <rc_ncs.h>
#include <rc_time.h>
#include <rc_moments.h>

#define rmPrintImage(a){ \
for (int32 i = 0; i < (a).height(); i++) \
{ \
   fprintf (stderr, "\n"); \
      for (int32 j = 0; j < (a).width(); j++) \
         fprintf (stderr, "%3d ", (a).getPixel (j, i)); \
            fprintf (stderr, "\n");\
}}

UT_point_corr::UT_point_corr()
{
}

UT_point_corr::~UT_point_corr()
{
  printSuccessMessage( "POINT CORRELATION test", mErrors );
}

uint32 UT_point_corr::run()
{
  test3by3Space();
  time3by3Space();
  test5by5Space();
  time5by5Space();
  testSpace();
  timeSpace();

  return mErrors;
}
  
void UT_point_corr::test3by3Space()
{
  srand(0);
  const float maxCorrError = 1.0e-6;
  
  /* Test alignment cases. To limit the number of test cases to be
   * tried, testing is broken into two parts:
   *
   *   Test the different combinations of search space width and
   *   height dimensions, as well as all the possible (x, y) offset
   *   positions for both image and model. Since even this takes a
   *   relatively long amount of time to run, the exhaustive version
   *   of the test is only run when DO_EXHAUSTIVE_TEST is defined.
   *
   *   Test the use of subwindows as the main "frame" by first
   *   creating a "parent" window, then creating various possible
   *   "child" windows and then using the child windows as the "frame"
   *   to pass to update(). This is then used to create test cases
   *   using a fixed search space size and various image and model
   *   offsets within their respective windows. Once again, the number
   *   of cases tried was limited to prevent execution time from
   *   getting to large.
   *
   * In either of these cases, verification is done by checking that
   * the correlation values match within tolerances those returned by
   * rfCorrelate().
   */
  {
    const int32 iParW = 32;
    const int32 iParH = 18;
    rcWindow iParWin(iParW, iParH);
    iParWin.randomFill(15);
    
    const int32 mParW = 30;
    const int32 mParH = 16;
    rcWindow mParWin(mParW, mParH);
    mParWin.randomFill(77);

    rcAutoCorrelation acImage, acModel;
    rcPointCorrelation corr;
    acModel.update(mParWin, 1, rcAutoCorrelation::eCross,
		   rcAutoCorrelation::ePoint);
    acImage.update(iParWin, 1, rcAutoCorrelation::eCross,
		   rcAutoCorrelation::ePoint);
    corr.update(iParWin, mParWin, acImage.momGen(), acModel.momGen());

    float largestCorrError = 0.0;

#ifdef DO_EXHAUSTIVE_TEST    
    for (int32 ySrchSpcSz = 3; ySrchSpcSz <= 16; ySrchSpcSz++) {
      const int32 mHeight = ySrchSpcSz - 2;
      for (int32 xSrchSpcSz = 3; xSrchSpcSz <= 16; xSrchSpcSz++) {
	const int32 mWidth = xSrchSpcSz - 2;
	for (int32 yMdlOff = 0; yMdlOff <= (mParH - mHeight); yMdlOff++) {
	  for (int32 xMdlOff = 0; xMdlOff <= (mParW - mWidth); xMdlOff++) {
	    rcIPair modelOrigin(xMdlOff, yMdlOff);
	    rcWindow model(mParWin, xMdlOff, yMdlOff, mWidth, mHeight);
	    for (int32 yImgOff=0; yImgOff <= (iParH - ySrchSpcSz); yImgOff++) {
	      for (int32 xImgOff=0; xImgOff <= (iParW - xSrchSpcSz); xImgOff++) {
		rcRect srchSpace(xImgOff, yImgOff, xSrchSpcSz, ySrchSpcSz);
		rsCorr3by3Point fRes;
		corr.gen3by3Space(srchSpace, modelOrigin, fRes);

		rsCorrParams cp;
		rcCorr res;
		for (int32 y = 0; y < 3; y++) {
		  for (int32 x = 0; x < 3; x++) {
		    rcWindow image(iParWin, xImgOff+x, yImgOff+y, mWidth, mHeight);
		    rfCorrelate(image, model, cp, res);
		    float error = fabs(res.r() - fRes.score[y][x]);
		    rcUNITTEST_ASSERT(error < maxCorrError);
		    if (error > largestCorrError)
		      largestCorrError = error;
		  } // End of: for (int32 x = 0; x < 3; x++) {
		} // End of: for (int32 y = 0; y < 3; y++) {
	      } // End of: for ( ...; xImgOff <= (iParW - xSrchSpcSz); ...) {
	    } // End of: for ( ...; yImgOff <= (iParH - ySrchSpcSz); ...) {
	  } // End of: for ( ...; xMdlOff <= (mParW - mWidth); ...) {
	} // End of: for ( ...; yMdlOff <= (mParH - mHeight); ...) {
      } // End of: for ( ...; xSrchSpcSz <= 16; ...)
    } // End of: for ( ...; ySrchSpcSz <= 16; ...)
#else
    for (int32 ySrchSpcSz = 3; ySrchSpcSz <= 16; ySrchSpcSz++) {
      const int32 mHeight = ySrchSpcSz - 2;
      for (int32 xSrchSpcSz = 9; xSrchSpcSz <= 9; xSrchSpcSz++) {
	const int32 mWidth = xSrchSpcSz - 2;
	const int32 yMdlMaxOff =
	  ((mParH - mHeight) < 3) ? (mParH - mHeight) : 3;
	const int32 yImgMaxOff =
	  ((iParH - ySrchSpcSz) < 3) ? (iParH - ySrchSpcSz) : 3;
	for (int32 yMdlOff = 0; yMdlOff <= yMdlMaxOff; yMdlOff++) {
	  for (int32 xMdlOff = 0; xMdlOff <= (mParW - mWidth); xMdlOff++) {
	    rcIPair modelOrigin(xMdlOff, yMdlOff);
	    rcWindow model(mParWin, xMdlOff, yMdlOff, mWidth, mHeight);
	    for (int32 yImgOff=0; yImgOff <= yImgMaxOff; yImgOff++) {
	      for (int32 xImgOff=0; xImgOff <= (iParW - xSrchSpcSz); xImgOff++) {
		rcRect srchSpace(xImgOff, yImgOff, xSrchSpcSz, ySrchSpcSz);
		rsCorr3by3Point fRes;
		corr.gen3by3Space(srchSpace, modelOrigin, fRes);

		rsCorrParams cp;
		rcCorr res;
		for (int32 y = 0; y < 3; y++) {
		  for (int32 x = 0; x < 3; x++) {
		    rcWindow image(iParWin, xImgOff+x, yImgOff+y, mWidth, mHeight);
		    rfCorrelate(image, model, cp, res);
		    float error = fabs(res.r() - fRes.score[y][x]);
		    rcUNITTEST_ASSERT(error < maxCorrError);
		    if (error > largestCorrError)
		      largestCorrError = error;
		  } // End of: for (int32 x = 0; x < 3; x++) {
		} // End of: for (int32 y = 0; y < 3; y++) {
	      } // End of: for ( ...; xImgOff <= (iParW - xSrchSpcSz); ...) {
	    } // End of: for ( ...; yImgOff <= yImgMaxOff; ...) {
	  } // End of: for ( ...; xMdlOff <= (mParW - mWidth); ...) {
	} // End of: for ( ...; yMdlOff <= yMdlMaxOff; ...) {
      } // End of: for ( ...; xSrchSpcSz <= 16; ...)
    } // End of: for ( ...; ySrchSpcSz <= 16; ...)

    for (int32 ySrchSpcSz = 9; ySrchSpcSz <= 9; ySrchSpcSz++) {
      const int32 mHeight = ySrchSpcSz - 2;
      for (int32 xSrchSpcSz = 3; xSrchSpcSz <= 16; xSrchSpcSz++) {
	const int32 mWidth = xSrchSpcSz - 2;
	const int32 yMdlMaxOff =
	  ((mParH - mHeight) < 3) ? (mParH - mHeight) : 3;
	const int32 yImgMaxOff =
	  ((iParH - ySrchSpcSz) < 3) ? (iParH - ySrchSpcSz) : 3;
	for (int32 yMdlOff = 0; yMdlOff <= yMdlMaxOff; yMdlOff++) {
	  for (int32 xMdlOff = 0; xMdlOff <= (mParW - mWidth); xMdlOff++) {
	    rcIPair modelOrigin(xMdlOff, yMdlOff);
	    rcWindow model(mParWin, xMdlOff, yMdlOff, mWidth, mHeight);
	    for (int32 yImgOff=0; yImgOff <= yImgMaxOff; yImgOff++) {
	      for (int32 xImgOff=0; xImgOff <= (iParW - xSrchSpcSz); xImgOff++) {
		rcRect srchSpace(xImgOff, yImgOff, xSrchSpcSz, ySrchSpcSz);
		rsCorr3by3Point fRes;
		corr.gen3by3Space(srchSpace, modelOrigin, fRes);

		rsCorrParams cp;
		rcCorr res;
		for (int32 y = 0; y < 3; y++) {
		  for (int32 x = 0; x < 3; x++) {
		    rcWindow image(iParWin, xImgOff+x, yImgOff+y, mWidth, mHeight);
		    rfCorrelate(image, model, cp, res);
		    float error = fabs(res.r() - fRes.score[y][x]);
		    rcUNITTEST_ASSERT(error < maxCorrError);
		    if (error > largestCorrError)
		      largestCorrError = error;
		  } // End of: for (int32 x = 0; x < 3; x++) {
		} // End of: for (int32 y = 0; y < 3; y++) {
	      } // End of: for ( ...; xImgOff <= (iParW - xSrchSpcSz); ...) {
	    } // End of: for ( ...; yImgOff <= yImgMaxOff; ...) {
	  } // End of: for ( ...; xMdlOff <= (mParW - mWidth); ...) {
	} // End of: for ( ...; yMdlOff <= yMdlMaxOff; ...) {
      } // End of: for ( ...; xSrchSpcSz <= 16; ...)
    } // End of: for ( ...; ySrchSpcSz <= 16; ...)
#endif

    for (int32 mFrmYOff = 0; mFrmYOff < 3; mFrmYOff++) {
      for (int32 mFrmXOff = 0; mFrmXOff < 3; mFrmXOff++) {
	const int32 mFrameWidth = mParW - 2;
	const int32 mFrameHeight = mParH - 2;
	const int32 xSrchSpcSz = 9;
	const int32 ySrchSpcSz = 9;
	    
	/* Initialize the model "child" window or "frame".
	 */
	rcWindow mFrame(mParWin, mFrmXOff, mFrmYOff, mFrameWidth, mFrameHeight);
	acModel.update(mFrame, 1, rcAutoCorrelation::eCross,
		       rcAutoCorrelation::ePoint);

	for (int32 iFrmYOff = 0; iFrmYOff < 3; iFrmYOff++) {
	  for (int32 iFrmXOff = 0; iFrmXOff < 3; iFrmXOff++) {
	    /* Initialize the image "child" window or "frame".
	     */
	    const int32 iFrameWidth = iParW - 2;
	    const int32 iFrameHeight = iParH - 2;
	
	    rcWindow iFrame(iParWin, iFrmXOff, iFrmYOff, iFrameWidth, iFrameHeight);
	    
	    acImage.update(iFrame, 1, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::ePoint);
	    
	    corr.update(iFrame, mFrame, acImage.momGen(), acModel.momGen());

	    const int32 mHeight = ySrchSpcSz - 2;
	    const int32 mWidth = xSrchSpcSz - 2;
	    const int32 yMdlMaxOff =
	      ((mParH - mHeight) < 3) ? (mParH - mHeight) : 3;
	    const int32 xMdlMaxOff =
	      ((mParW - mWidth) < 3) ? (mParW - mWidth) : 3;
	    const int32 yImgMaxOff =
	      ((iParH - ySrchSpcSz) < 3) ? (iParH - ySrchSpcSz) : 3;
	    const int32 xImgMaxOff =
	      ((iParW - xSrchSpcSz) < 3) ? (iParW - xSrchSpcSz) : 3;
	    for (int32 yMdlOff = 0; yMdlOff <= yMdlMaxOff; yMdlOff++) {
	      for (int32 xMdlOff = 0; xMdlOff <= xMdlMaxOff; xMdlOff++) {
		rcIPair modelOrigin(xMdlOff, yMdlOff);
		rcWindow model(mFrame, xMdlOff, yMdlOff, mWidth, mHeight);
		for (int32 yImgOff=0; yImgOff <= yImgMaxOff; yImgOff++) {
		  for (int32 xImgOff=0; xImgOff <= xImgMaxOff; xImgOff++) {
		    rcRect srchSpace(xImgOff, yImgOff, xSrchSpcSz, ySrchSpcSz);
		    rsCorr3by3Point fRes;
		    corr.gen3by3Space(srchSpace, modelOrigin, fRes);
		    
		    rsCorrParams cp;
		    rcCorr res;
		    for (int32 y = 0; y < 3; y++) {
		      for (int32 x = 0; x < 3; x++) {
			rcWindow image(iFrame, xImgOff+x, yImgOff+y,
				       mWidth, mHeight);
			rfCorrelate(image, model, cp, res);
			float error = fabs(res.r() - fRes.score[y][x]);
			rcUNITTEST_ASSERT(error < maxCorrError);
			if (error > largestCorrError)
			  largestCorrError = error;
		      } // End of: for (int32 x = 0; x < 3; x++) {
		    } // End of: for (int32 y = 0; y < 3; y++) {
		  } // End of: for ( ...; xImgOff <= xImgMaxOff; ...) {
		} // End of: for ( ...; yImgOff <= yImgMaxOff; ...) {
	      } // End of: for ( ...; xMdlOff <= xMdlMaxOff; ...) {
	    } // End of: for ( ...; yMdlOff <= yMdlMaxOff; ...) {
	  }  // End of: for ( ,,,; iFrmXOff < 3; iFrmXOff++) {
	} // End of: for ( ...; iFrmYOff < 3; iFrmYOff++) {
      } // End of: for (int32 mFrmXOff = 0; mFrmXOff < 3; mFrmXOff++) {
    } // End of: for (int32 mFrmYOff = 0; mFrmYOff < 3; mFrmYOff++) {

    fprintf(stderr, "Performance: 3 x 3 Point Corr (Random) Largest Error %e\n",
	    largestCorrError);
  } // End of: Alignment Test

  /* Perform numerical accuracy test. This tries to generate worst
   * case for when the calculations of the form: (N*SumIM - SumI*SumM)
   * or (N*SumII - SumI*SumI). This is done by assuming the worst case
   * is when the products are at a maximum, but the differences are a
   * a minimum. This should happen when the average grey value is at
   * the maximum, but the standard deviation of the grey values is at
   * a minimum.
   */
  {
    float largestCorrError = 0.0;
    const int32 imgW = 16;
    const int32 imgH = 16;
    rcWindow imgWin(imgW, imgH);
    imgWin.setAllPixels(0xFF);
    
    const int32 mdlW = 16;
    const int32 mdlH = 16;
    rcWindow mdlWin(mdlW, mdlH);
    mdlWin.setAllPixels(0xFF);

    for (int32 loop = 0; loop < 2; loop++) {
      if (loop) {
	imgWin.setPixel(0, 0, 0xFE);
	imgWin.setPixel(1, 1, 0xFE);
	mdlWin.setPixel(0, 0, 0xFE);
      }
    
      rcAutoCorrelation acImage, acModel;
      rcPointCorrelation corr;
      acModel.update(mdlWin, 1, rcAutoCorrelation::eCross,
		     rcAutoCorrelation::ePoint);
      acImage.update(imgWin, 1, rcAutoCorrelation::eCross,
		     rcAutoCorrelation::ePoint);
      corr.update(imgWin, mdlWin, acImage.momGen(), acModel.momGen());
      for (int32 ySrchSpcSz = 3; ySrchSpcSz <= 16; ySrchSpcSz++) {
	const int32 mHeight = ySrchSpcSz - 2;
	for (int32 xSrchSpcSz = 3; xSrchSpcSz <= 16; xSrchSpcSz++) {
	  const int32 mWidth = xSrchSpcSz - 2;
	  rcIPair modelOrigin(0, 0);
	  rcWindow model(mdlWin, 0, 0, mWidth, mHeight);
	  rcRect srchSpace(0, 0, xSrchSpcSz, ySrchSpcSz);
	  rsCorr3by3Point fRes;
	  corr.gen3by3Space(srchSpace, modelOrigin, fRes);

	  rsCorrParams cp;
	  rcCorr res;
	  for (int32 y = 0; y < 3; y++) {
	    for (int32 x = 0; x < 3; x++) {
	      rcWindow image(imgWin, x, y, mWidth, mHeight);
	      rfCorrelate(image, model, cp, res);
	      float error = fabs(res.r() - fRes.score[y][x]);
	      rcUNITTEST_ASSERT(error < maxCorrError);
	      if (error > largestCorrError)
		largestCorrError = error;
	    } // End of: for (int32 x = 0; x < 3; x++) {
	  } // End of: for (int32 y = 0; y < 3; y++) {
	} // End of: for ( ...; xSrchSpcSz <= 16; ...)
      } // End of: for ( ...; ySrchSpcSz <= 16; ...)
    } // End of: for (int32 loop = 0; loop < 2; loop) {

    fprintf(stderr, "Performance: 3 x 3 Point Corr (Min Dev) Largest Error %e\n",
	    largestCorrError);
  } // End of: Numerical Accuracy Test
}

void UT_point_corr::time3by3Space()
{
  srand(0);

  const uint32 imgCnt = 6;
  const uint32 loops = imgCnt - 1;

  vector<rcWindow> image(imgCnt);
  rcAutoCorrelation acImage[imgCnt];
  rcPointCorrelation corr;
  
  for (uint32 i = 0; i < imgCnt; i++) {
    image[i] = rcWindow(1280, 960);
    image[i].randomFill((i+1)*41);
    acImage[i].update(image[i], 1);
  }

  rsCorr3by3Point res;

  rcTime time;
  time.start();
  for (uint32 i = 0; i < loops; i++) {
    corr.update(image[i], image[i+1], acImage[i].momGen(), acImage[i+1].momGen());
    for(int32 by = 0; by < 960; by += 96)
      for(int32 bx = 0; bx < 960; bx += 96)
	for (int32 y = 0; y < 48; y += 3)
	  for (int32 x = 0; x < 48; x += 3) {
	    rcIPair modelOrigin(bx + x, by + y);
	    rcRect srchSpace(bx + x, by + y, 9, 9);
	    corr.gen3by3Space(srchSpace, modelOrigin, res);
	  }
  }
  time.end();

  /* Neatly print results.
   */
  double dMilliSeconds = time.milliseconds()/loops;
  double spaces = 10*10*16*16;

  fprintf(stderr, "Performance: Gen Point Correlation %d 3 x 3 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (int)spaces, dMilliSeconds,
	  time.microseconds()/((double)spaces*loops));
}

void UT_point_corr::test5by5Space()
{
  srand(0);
  const float maxCorrError = 1.0e-6;
  
  /* Test alignment cases. To limit the number of test cases to be
   * tried, testing is broken into two parts:
   *
   *   Test the different combinations of search space width and
   *   height dimensions, as well as all the possible (x, y) offset
   *   positions for both image and model. Since even this takes a
   *   relatively long amount of time to run, the exhaustive version
   *   of the test is only run when DO_EXHAUSTIVE_TEST is defined.
   *
   *   Test the use of subwindows as the main "frame" by first
   *   creating a "parent" window, then creating various possible
   *   "child" windows and then using the child windows as the "frame"
   *   to pass to update(). This is then used to create test cases
   *   using a fixed search space size and various image and model
   *   offsets within their respective windows. Once again, the number
   *   of cases tried was limited to prevent execution time from
   *   getting to large.
   *
   * In either of these cases, verification is done by checking that
   * the correlation values match within tolerances those returned by
   * rfCorrelate().
   */
  {
    const int32 iParW = 32;
    const int32 iParH = 18;
    rcWindow iParWin(iParW, iParH);
    iParWin.randomFill(15);
    
    const int32 mParW = 30;
    const int32 mParH = 16;
    rcWindow mParWin(mParW, mParH);
    mParWin.randomFill(77);

    rcAutoCorrelation acImage, acModel;
    rcPointCorrelation corr;
    acModel.update(mParWin, 1, rcAutoCorrelation::eCross,
		   rcAutoCorrelation::ePoint);
    acImage.update(iParWin, 1, rcAutoCorrelation::eCross,
		   rcAutoCorrelation::ePoint);
    corr.update(iParWin, mParWin, acImage.momGen(), acModel.momGen());

    float largestCorrError = 0.0;

#ifdef DO_EXHAUSTIVE_TEST
    for (int32 ySrchSpcSz = 5; ySrchSpcSz <= 16; ySrchSpcSz++) {
      const int32 mHeight = ySrchSpcSz - 4;
      for (int32 xSrchSpcSz = 5; xSrchSpcSz <= 16; xSrchSpcSz++) {
	const int32 mWidth = xSrchSpcSz - 4;
	for (int32 yMdlOff = 0; yMdlOff <= (mParH - mHeight); yMdlOff++) {
	  for (int32 xMdlOff = 0; xMdlOff <= (mParW - mWidth); xMdlOff++) {
	    rcIPair modelOrigin(xMdlOff, yMdlOff);
	    rcWindow model(mParWin, xMdlOff, yMdlOff, mWidth, mHeight);
	    for (int32 yImgOff=0; yImgOff <= (iParH - ySrchSpcSz); yImgOff++) {
	      for (int32 xImgOff=0; xImgOff <= (iParW - xSrchSpcSz); xImgOff++) {
		rcRect srchSpace(xImgOff, yImgOff, xSrchSpcSz, ySrchSpcSz);
		rsCorr5by5Point fRes;
		corr.gen5by5Space(srchSpace, modelOrigin, fRes);

		rsCorrParams cp;
		rcCorr res;
		for (int32 y = 0; y < 5; y++) {
		  for (int32 x = 0; x < 5; x++) {
		    rcWindow image(iParWin, xImgOff+x, yImgOff+y, mWidth, mHeight);
		    rfCorrelate(image, model, cp, res);
		    float error = fabs(res.r() - fRes.score[y][x]);
		    rcUNITTEST_ASSERT(error < maxCorrError);
		    if (error > largestCorrError)
		      largestCorrError = error;
		  } // End of: for (int32 x = 0; x < 5; x++) {
		} // End of: for (int32 y = 0; y < 5; y++) {
	      } // End of: for ( ...; xImgOff <= (iParW - xSrchSpcSz); ...) {
	    } // End of: for ( ...; yImgOff <= (iParH - ySrchSpcSz); ...) {
	  } // End of: for ( ...; xMdlOff <= (mParW - mWidth); ...) {
	} // End of: for ( ...; yMdlOff <= (mParH - mHeight); ...) {
      } // End of: for ( ...; xSrchSpcSz <= 16; ...)
    } // End of: for ( ...; ySrchSpcSz <= 16; ...)
#else
    for (int32 ySrchSpcSz = 5; ySrchSpcSz <= 16; ySrchSpcSz++) {
      const int32 mHeight = ySrchSpcSz - 4;
      for (int32 xSrchSpcSz = 9; xSrchSpcSz <= 9; xSrchSpcSz++) {
	const int32 mWidth = xSrchSpcSz - 4;
	const int32 yMdlMaxOff =
	  ((mParH - mHeight) < 3) ? (mParH - mHeight) : 3;
	const int32 yImgMaxOff =
	  ((iParH - ySrchSpcSz) < 3) ? (iParH - ySrchSpcSz) : 3;
	for (int32 yMdlOff = 0; yMdlOff <= yMdlMaxOff; yMdlOff++) {
	  for (int32 xMdlOff = 0; xMdlOff <= (mParW - mWidth); xMdlOff++) {
	    rcIPair modelOrigin(xMdlOff, yMdlOff);
	    rcWindow model(mParWin, xMdlOff, yMdlOff, mWidth, mHeight);
	    for (int32 yImgOff=0; yImgOff <= yImgMaxOff; yImgOff++) {
	      for (int32 xImgOff=0; xImgOff <= (iParW - xSrchSpcSz); xImgOff++) {
		rcRect srchSpace(xImgOff, yImgOff, xSrchSpcSz, ySrchSpcSz);
		rsCorr5by5Point fRes;
		corr.gen5by5Space(srchSpace, modelOrigin, fRes);

		rsCorrParams cp;
		rcCorr res;
		for (int32 y = 0; y < 5; y++) {
		  for (int32 x = 0; x < 5; x++) {
		    rcWindow image(iParWin, xImgOff+x, yImgOff+y, mWidth, mHeight);
		    rfCorrelate(image, model, cp, res);
		    float error = fabs(res.r() - fRes.score[y][x]);
		    rcUNITTEST_ASSERT(error < maxCorrError);
		    if (error > largestCorrError)
		      largestCorrError = error;
		  } // End of: for (int32 x = 0; x < 5; x++) {
		} // End of: for (int32 y = 0; y < 5; y++) {
	      } // End of: for ( ...; xImgOff <= (iParW - xSrchSpcSz); ...) {
	    } // End of: for ( ...; yImgOff <= yImgMaxOff; ...) {
	  } // End of: for ( ...; xMdlOff <= (mParW - mWidth); ...) {
	} // End of: for ( ...; yMdlOff <= yMdlMaxOff; ...) {
      } // End of: for ( ...; xSrchSpcSz <= 16; ...)
    } // End of: for ( ...; ySrchSpcSz <= 16; ...)

    for (int32 ySrchSpcSz = 9; ySrchSpcSz <= 9; ySrchSpcSz++) {
      const int32 mHeight = ySrchSpcSz - 4;
      for (int32 xSrchSpcSz = 5; xSrchSpcSz <= 16; xSrchSpcSz++) {
	const int32 mWidth = xSrchSpcSz - 4;
	const int32 yMdlMaxOff =
	  ((mParH - mHeight) < 3) ? (mParH - mHeight) : 3;
	const int32 yImgMaxOff =
	  ((iParH - ySrchSpcSz) < 3) ? (iParH - ySrchSpcSz) : 3;
	for (int32 yMdlOff = 0; yMdlOff <= yMdlMaxOff; yMdlOff++) {
	  for (int32 xMdlOff = 0; xMdlOff <= (mParW - mWidth); xMdlOff++) {
	    rcIPair modelOrigin(xMdlOff, yMdlOff);
	    rcWindow model(mParWin, xMdlOff, yMdlOff, mWidth, mHeight);
	    for (int32 yImgOff=0; yImgOff <= yImgMaxOff; yImgOff++) {
	      for (int32 xImgOff=0; xImgOff <= (iParW - xSrchSpcSz); xImgOff++) {
		rcRect srchSpace(xImgOff, yImgOff, xSrchSpcSz, ySrchSpcSz);
		rsCorr5by5Point fRes;
		corr.gen5by5Space(srchSpace, modelOrigin, fRes);

		rsCorrParams cp;
		rcCorr res;
		for (int32 y = 0; y < 5; y++) {
		  for (int32 x = 0; x < 5; x++) {
		    rcWindow image(iParWin, xImgOff+x, yImgOff+y, mWidth, mHeight);
		    rfCorrelate(image, model, cp, res);
		    float error = fabs(res.r() - fRes.score[y][x]);
		    rcUNITTEST_ASSERT(error < maxCorrError);
		    if (error > largestCorrError)
		      largestCorrError = error;
		  } // End of: for (int32 x = 0; x < 5; x++) {
		} // End of: for (int32 y = 0; y < 5; y++) {
	      } // End of: for ( ...; xImgOff <= (iParW - xSrchSpcSz); ...) {
	    } // End of: for ( ...; yImgOff <= yImgMaxOff; ...) {
	  } // End of: for ( ...; xMdlOff <= (mParW - mWidth); ...) {
	} // End of: for ( ...; yMdlOff <= yMdlMaxOff; ...) {
      } // End of: for ( ...; xSrchSpcSz <= 16; ...)
    } // End of: for ( ...; ySrchSpcSz <= 16; ...)
#endif

    for (int32 mFrmYOff = 0; mFrmYOff < 3; mFrmYOff++) {
      for (int32 mFrmXOff = 0; mFrmXOff < 3; mFrmXOff++) {
	const int32 mFrameWidth = mParW - 2;
	const int32 mFrameHeight = mParH - 2;
	const int32 xSrchSpcSz = 9;
	const int32 ySrchSpcSz = 9;
	    
	/* Initialize the model "child" window or "frame".
	 */
	rcWindow mFrame(mParWin, mFrmXOff, mFrmYOff, mFrameWidth, mFrameHeight);
	acModel.update(mFrame, 1, rcAutoCorrelation::eCross,
		       rcAutoCorrelation::ePoint);

	for (int32 iFrmYOff = 0; iFrmYOff < 3; iFrmYOff++) {
	  for (int32 iFrmXOff = 0; iFrmXOff < 3; iFrmXOff++) {
	    /* Initialize the image "child" window or "frame".
	     */
	    const int32 iFrameWidth = iParW - 2;
	    const int32 iFrameHeight = iParH - 2;
	
	    rcWindow iFrame(iParWin, iFrmXOff, iFrmYOff, iFrameWidth, iFrameHeight);
	    
	    acImage.update(iFrame, 1, rcAutoCorrelation::eCross,
			   rcAutoCorrelation::ePoint);
	    
	    corr.update(iFrame, mFrame, acImage.momGen(), acModel.momGen());

	    const int32 mHeight = ySrchSpcSz - 4;
	    const int32 mWidth = xSrchSpcSz - 4;
	    const int32 yMdlMaxOff =
	      ((mParH - mHeight) < 3) ? (mParH - mHeight) : 3;
	    const int32 xMdlMaxOff =
	      ((mParW - mWidth) < 3) ? (mParW - mWidth) : 3;
	    const int32 yImgMaxOff =
	      ((iParH - ySrchSpcSz) < 3) ? (iParH - ySrchSpcSz) : 3;
	    const int32 xImgMaxOff =
	      ((iParW - xSrchSpcSz) < 3) ? (iParW - xSrchSpcSz) : 3;
	    for (int32 yMdlOff = 0; yMdlOff <= yMdlMaxOff; yMdlOff++) {
	      for (int32 xMdlOff = 0; xMdlOff <= xMdlMaxOff; xMdlOff++) {
		rcIPair modelOrigin(xMdlOff, yMdlOff);
		rcWindow model(mFrame, xMdlOff, yMdlOff, mWidth, mHeight);
		for (int32 yImgOff=0; yImgOff <= yImgMaxOff; yImgOff++) {
		  for (int32 xImgOff=0; xImgOff <= xImgMaxOff; xImgOff++) {
		    rcRect srchSpace(xImgOff, yImgOff, xSrchSpcSz, ySrchSpcSz);
		    rsCorr5by5Point fRes;
		    corr.gen5by5Space(srchSpace, modelOrigin, fRes);
		    
		    rsCorrParams cp;
		    rcCorr res;
		    for (int32 y = 0; y < 5; y++) {
		      for (int32 x = 0; x < 5; x++) {
			rcWindow image(iFrame, xImgOff+x, yImgOff+y,
				       mWidth, mHeight);
			rfCorrelate(image, model, cp, res);
			float error = fabs(res.r() - fRes.score[y][x]);
			rcUNITTEST_ASSERT(error < maxCorrError);
			if (error > largestCorrError)
			  largestCorrError = error;
		      } // End of: for (int32 x = 0; x < 5; x++) {
		    } // End of: for (int32 y = 0; y < 5; y++) {
		  } // End of: for ( ...; xImgOff <= xImgMaxOff; ...) {
		} // End of: for ( ...; yImgOff <= yImgMaxOff; ...) {
	      } // End of: for ( ...; xMdlOff <= xMdlMaxOff; ...) {
	    } // End of: for ( ...; yMdlOff <= yMdlMaxOff; ...) {
	  }  // End of: for ( ,,,; iFrmXOff < 3; iFrmXOff++) {
	} // End of: for ( ...; iFrmYOff < 3; iFrmYOff++) {
      } // End of: for (int32 mFrmXOff = 0; mFrmXOff < 3; mFrmXOff++) {
    } // End of: for (int32 mFrmYOff = 0; mFrmYOff < 3; mFrmYOff++) {

    fprintf(stderr, "Performance: 5 x 5 Point Corr (Random) Largest Error %e\n",
	    largestCorrError);
  } // End of: Alignment Test

  /* Perform numerical accuracy test. This tries to generate worst
   * case for when the calculations of the form: (N*SumIM - SumI*SumM)
   * or (N*SumII - SumI*SumI). This is done by assuming the worst case
   * is when the products are at a maximum, but the differences are a
   * a minimum. This should happen when the average grey value is at
   * the maximum, but the standard deviation of the grey values is at
   * a minimum.
   */
  {
    float largestCorrError = 0.0;
    const int32 imgW = 16;
    const int32 imgH = 16;
    rcWindow imgWin(imgW, imgH);
    imgWin.setAllPixels(0xFF);
    
    const int32 mdlW = 16;
    const int32 mdlH = 16;
    rcWindow mdlWin(mdlW, mdlH);
    mdlWin.setAllPixels(0xFF);

    for (uint32 loop = 0; loop < 2; loop++) {
      if (loop) {
	imgWin.setPixel(0, 0, 0xFE);
	imgWin.setPixel(1, 1, 0xFE);
	mdlWin.setPixel(0, 0, 0xFE);
      }
    
      rcAutoCorrelation acImage, acModel;
      rcPointCorrelation corr;
      acModel.update(mdlWin, 1, rcAutoCorrelation::eCross,
		     rcAutoCorrelation::ePoint);
      acImage.update(imgWin, 1, rcAutoCorrelation::eCross,
		     rcAutoCorrelation::ePoint);
      corr.update(imgWin, mdlWin, acImage.momGen(), acModel.momGen());
      for (int32 ySrchSpcSz = 5; ySrchSpcSz <= 16; ySrchSpcSz++) {
	const int32 mHeight = ySrchSpcSz - 4;
	for (int32 xSrchSpcSz = 5; xSrchSpcSz <= 16; xSrchSpcSz++) {
	  const int32 mWidth = xSrchSpcSz - 4;
	  rcIPair modelOrigin(0, 0);
	  rcWindow model(mdlWin, 0, 0, mWidth, mHeight);
	  rcRect srchSpace(0, 0, xSrchSpcSz, ySrchSpcSz);
	  rsCorr5by5Point fRes;
	  corr.gen5by5Space(srchSpace, modelOrigin, fRes);

	  rsCorrParams cp;
	  rcCorr res;
	  for (int32 y = 0; y < 5; y++) {
	    for (int32 x = 0; x < 5; x++) {
	      rcWindow image(imgWin, x, y, mWidth, mHeight);
	      rfCorrelate(image, model, cp, res);
	      float error = fabs(res.r() - fRes.score[y][x]);
	      rcUNITTEST_ASSERT(error < maxCorrError);
	      if (error > largestCorrError)
		largestCorrError = error;
	    } // End of: for (int32 x = 0; x < 5; x++) {
	  } // End of: for (int32 y = 0; y < 5; y++) {
	} // End of: for ( ...; xSrchSpcSz <= 16; ...)
      } // End of: for ( ...; ySrchSpcSz <= 16; ...)
    } // End of: for (uint32 loop = 0; loop < 2; loop) {

    fprintf(stderr, "Performance: 5 x 5 Point Corr (Min Dev) Largest Error %e\n",
	    largestCorrError);
  } // End of: Numerical Accuracy Test
}

void UT_point_corr::time5by5Space()
{
  srand(0);

  const uint32 imgCnt = 6;
  const uint32 loops = imgCnt - 1;

  vector<rcWindow> image(imgCnt);
  rcAutoCorrelation acImage[imgCnt];
  rcPointCorrelation corr;
  
  for (uint32 i = 0; i < imgCnt; i++) {
    image[i] = rcWindow(1280, 960);
    image[i].randomFill((i+1)*41);
    acImage[i].update(image[i], 1);
  }

  rsCorr5by5Point res;

  rcTime time;
  time.start();
  for (uint32 i = 0; i < loops; i++) {
    corr.update(image[i], image[i+1], acImage[i].momGen(), acImage[i+1].momGen());
    for(int32 by = 0; by < 960; by += 96)
      for(int32 bx = 0; bx < 960; bx += 96)
	for (int32 y = 0; y < 48; y += 3)
	  for (int32 x = 0; x < 48; x += 3) {
	    rcIPair modelOrigin(bx + x, by + y);
	    rcRect srchSpace(bx + x, by + y, 9, 9);
	    corr.gen5by5Space(srchSpace, modelOrigin, res);
	  }
  }
  time.end();

  /* Neatly print results.
   */
  double dMilliSeconds = time.milliseconds()/loops;
  double spaces = 10*10*16*16;

  fprintf(stderr, "Performance: Gen Point Correlation %d 5 x 5 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (int)spaces, dMilliSeconds,
	  time.microseconds()/((double)spaces*loops));
}
 
void UT_point_corr::testSpace()
{
  srand(0);
  const float maxCorrError = 1.0e-6;

  const int32 iParW = 17;
  const int32 iParH = 17;
  rcWindow iParWin(iParW, iParH);
  iParWin.randomFill(99);
    
  const int32 mParW = 17;
  const int32 mParH = 17;
  rcWindow mParWin(mParW, mParH);
  mParWin.randomFill(65);

  rcAutoCorrelation acImage, acModel;
  rcPointCorrelation corr;
  vector<vector<float> > fRes(11);
  for (uint32 i = 0; i < fRes.size(); i++)
    fRes[i].resize(fRes.size());

  float largestCorrError = 0.0;

  for (int32 mFrmYOff = 0; mFrmYOff < 2; mFrmYOff++) {
    for (int32 mFrmXOff = 0; mFrmXOff < 2; mFrmXOff++) {
      const int32 mFrameWidth = mParW - 1;
      const int32 mFrameHeight = mParH - 1;
      /* Initialize the model "child" window or "frame".
       */
      rcWindow mFrame(mParWin, mFrmXOff, mFrmYOff, mFrameWidth, mFrameHeight);
      acModel.update(mFrame, 1, rcAutoCorrelation::eCross,
		     rcAutoCorrelation::ePoint);

      for (int32 iFrmYOff = 0; iFrmYOff < 2; iFrmYOff++) {
	for (int32 iFrmXOff = 0; iFrmXOff < 2; iFrmXOff++) {
	  /* Initialize the image "child" window or "frame".
	   */
	  const int32 iFrameWidth = iParW - 1;
	  const int32 iFrameHeight = iParH - 1;
	  
	  rcWindow iFrame(iParWin, iFrmXOff, iFrmYOff, iFrameWidth, iFrameHeight);
	  
	  acImage.update(iFrame, 1, rcAutoCorrelation::eCross,
			 rcAutoCorrelation::ePoint);
	    
	  corr.update(iFrame, mFrame);

	  for (int32 ySrchSpcSz = 14; ySrchSpcSz <= 15; ySrchSpcSz++) {
	    for (int32 xSrchSpcSz = 14; xSrchSpcSz <= 15; xSrchSpcSz++) {
	      for (int32 yMdlOff = 0; yMdlOff <= 1; yMdlOff++) {
		for (int32 xMdlOff = 0; xMdlOff <= 1; xMdlOff++) {
		  for (int32 yImgOff=0; yImgOff <= 1; yImgOff++) {
		    for (int32 xImgOff=0; xImgOff <= 1; xImgOff++) {
		      for (int32 radius = 1; radius < 7; radius += 2) {
			const int32 mHeight = ySrchSpcSz - radius*2;
			const int32 mWidth = xSrchSpcSz - radius*2;
			const rcIPair modelOrigin(xMdlOff, yMdlOff);
			rcWindow model(mFrame, xMdlOff, yMdlOff, mWidth, mHeight);
			rcRect srchSpace(xImgOff, yImgOff, xSrchSpcSz, ySrchSpcSz);
			corr.genSpace(srchSpace, modelOrigin, fRes, radius);
			
			rsCorrParams cp;
			rcCorr res;
			int32 diameter = radius*2 + 1;
			for (int32 y = 0; y < diameter; y++) {
			  for (int32 x = 0; x < diameter; x++) {
			    rcWindow image(iFrame, xImgOff+x, yImgOff+y,
					   mWidth, mHeight);
			    rfCorrelate(image, model, cp, res);
			    float error = fabs(res.r() - fRes[y][x]);
			    rcUNITTEST_ASSERT(error < maxCorrError);
			    if (error > largestCorrError)
			      largestCorrError = error;
			  } // End of: for (int32 x = 0; x < diameter; x++) {
			} // End of: for (int32 y = 0; y < diameter; y++) {
		      } // End of: for ( ...; radius < 7; ...) {
		    } // End of: for ( ...; xImgOff <= 1; ...) {
		  } // End of: for ( ...; yImgOff <= 1; ...) {
		} // End of: for ( ...; xMdlOff <= 1; ...) {
	      } // End of: for ( ...; yMdlOff <= 1; ...) {
	    } // End of: for ( ...; xSrchSpcSz <= 15; ...) {
	  } // End of: for ( ...; ySrchSpcSz <= 15; ...) {
	}  // End of: for ( ...; iFrmXOff < 2; iFrmXOff++) {
      } // End of: for ( ...; iFrmYOff < 2; iFrmYOff++) {
    } // End of: for ( ...; mFrmXOff < 2; mFrmXOff++) {
  } // End of: for ( ...; mFrmYOff < 2; mFrmYOff++) {

  fprintf(stderr, "Performance: General Point Corr (Random) Largest Error %e\n",
	  largestCorrError);
}
 
void UT_point_corr::timeSpace()
{
  srand(0);

  const uint32 imgCnt = 2;
  const uint32 loops = imgCnt - 1;

  vector<rcWindow> image(imgCnt);
  rcPointCorrelation corr;
  
  for (uint32 i = 0; i < imgCnt; i++) {
    image[i] = rcWindow(1280, 960);
    image[i].randomFill((i+1)*41);
  }

  vector<vector<float> > res(5);
  for (uint32 i = 0; i < res.size(); i++)
    res[i].resize(res.size());

  rcTime time;
  time.start();
  for (uint32 i = 0; i < loops; i++) {
    corr.update(image[i], image[i+1]);
    for(int32 by = 0; by < 960; by += 96)
      for(int32 bx = 0; bx < 960; bx += 96)
	for (int32 y = 0; y < 48; y += 3)
	  for (int32 x = 0; x < 48; x += 3) {
	    rcIPair modelOrigin(bx + x, by + y);
	    rcRect srchSpace(bx + x, by + y, 9, 9);
	    corr.genSpace(srchSpace, modelOrigin, res, 2);
	  }
  }
  time.end();

  /* Neatly print results.
   */
  double dMilliSeconds = time.milliseconds()/loops;
  double spaces = 10*10*16*16;

  fprintf(stderr, "Performance: Gen Point Correlation %d 5 x 5 spaces: "
	  "%.2f ms, %.2f us per space\n",
	  (int)spaces, dMilliSeconds,
	  time.microseconds()/((double)spaces*loops));
}
