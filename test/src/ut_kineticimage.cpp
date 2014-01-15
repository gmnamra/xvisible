/*
 *	ut_kineticimage.cpp 06/05/2003 
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */
#include "ut_kineticimage.h"
#include <rc_analysis.h>
#include <rc_similarity.h>
#include <rc_draw.h>


#define real_equalFloat(A,B,D) real_equal((float)(A), (float)(B), (float)(D))

UT_kineticimage::UT_kineticimage() 
{
}

UT_kineticimage::~UT_kineticimage()
{
  printSuccessMessage( "KINETIC IMAGE test", mErrors );
}

extern int TestRealFFT();

void segment(rcWindow src, rcWindow dest, float segPoint)
{
  rmAssert(src.depth() == rcPixel32);
  const int32 width = src.width(), height = src.height();

  rmAssert(dest.depth() == rcPixel8);
  rmAssert(dest.width() == width);
  rmAssert(dest.height() == height);

  for (int32 y = 0; y < height; y++) {
    float* sp = (float*)src.rowPointer(y);
    uint8* dp = (uint8*)dest.rowPointer(y);
    for (int32 x = 0; x < width; x++) {
      dp[x] = sp[x] <= segPoint ? 0 : 1;
    }
  }
}

void testSegPoint()
{
  float l0[8] = {0.10, 0.09, 0.12, 0.21, 0.31, 0.14, 0.02, 0.29 };
  float l1[8] = {0.11, 0.06, 0.13, 0.19, 0.26, 0.11, 0.10, 0.15 };
  float l2[8] = {0.14, 0.12, 0.43, 0.47, 0.41, 0.58, 0.09, 0.25 };
  float l3[8] = {0.19, 0.13, 0.48, 0.67, 0.61, 0.59, 0.21, 0.15 };
  float l4[8] = {0.12, 0.21, 0.40, 0.66, 0.63, 0.52, 0.19, 0.19 };
  float l5[8] = {0.11, 0.17, 0.44, 0.46, 0.43, 0.50, 0.16, 0.09 };
  float l6[8] = {0.13, 0.03, 0.23, 0.20, 0.24, 0.16, 0.12, 0.18 };

  float* ti[7] = { l0, l1, l2, l3, l4, l5, l6 };

  rcWindow tImg(8, 7, rcPixel32);

  for (int32 y = 0; y < tImg.height(); y++) {
    float* xp = (float*)tImg.rowPointer(y);
    for (int32 x = 0; x < tImg.width(); x++) {
      xp[x] = ti[y][x];
    }
  }

  rmPrintFloatImage(tImg);


  float segMin = 0.11, binSz = 0.05;
  int32 binCnt = 8;

  printf("retVal %f\n", segPoint(tImg, segMin, binSz, binCnt, false));
  
  vector<rcWindow> sWin(binCnt);
  for (int32 i = 0; i < binCnt; i++) {
    sWin[i] = rcWindow(8, 7);
    segment(tImg, sWin[i], segMin + binSz*i);
#if 0
    printf("\nSegment Value %f\n", segMin + binSz*i);
    rmPrintImage(sWin[i]);
#endif
  }

#if 0
  vector<vector<double> > corrMatrix(binCnt);
  for (int32 i = 0; i < binCnt; i++)
    corrMatrix[i].resize(binCnt);

  /* Fill in correlation matrix.
   */
  for (int32 i = 0; i < binCnt; i++)
    corrMatrix[i][i] = 1.0;
  
  rsCorrParams params;
  rcCorr res;

  for (int32 m = 0; m < binCnt; m++)
    for (int32 i = m+1; i < binCnt; i++) {
      rfCorrelate (sWin[m], sWin[i], params, res);
      corrMatrix[i][m] = corrMatrix[m][i] = res.r();
    }

  printf("Full Corr Results\n");
  for (int32 y = 0; y < binCnt; y++) {
    for (int32 x = 0; x < binCnt; x++)
      printf(" %f", corrMatrix[y][x]);

    printf("\n");
  }
#endif
}

uint32 UT_kineticimage::run()
{

  testVBigDspRealFFT();
  testVarianceGenerator();
  testStdDevGenerator();
  testVelEntropyGenerator();
  testAccelEntropyGenerator();
  testThetaEntropyGenerator();
  testEightBitImageGenerator();


  return mErrors;
}

void UT_kineticimage::testVBigDspRealFFT()
{
#ifdef __ppc__	
  int retVal = TestRealFFT();
  rcUNITTEST_ASSERT(retVal == 0);
#endif
}

void UT_kineticimage::testVarianceGenerator()
{
  /* Test for known results
   */
  {
    const int32 width = 2;
    const int32 height = 2;
    const uint32 imgCnt = 4;
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++)
      img[i] = rcWindow(width, height);

    img[0].setPixel(0, 0, 0); img[0].setPixel(0, 1, 1);
    img[1].setPixel(0, 0, 2); img[1].setPixel(0, 1, 4);
    img[2].setPixel(0, 0, 1); img[2].setPixel(0, 1, 10);
    img[3].setPixel(0, 0, 3); img[3].setPixel(0, 1, 7);
    img[0].setPixel(1, 0, 2); img[0].setPixel(1, 1, 2);
    img[1].setPixel(1, 0, 4); img[1].setPixel(1, 1, 6);
    img[2].setPixel(1, 0, 0); img[2].setPixel(1, 1, 14);
    img[3].setPixel(1, 0, 6); img[3].setPixel(1, 1, 10);

    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeVariance);
    genKinetics.push(img);

    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);
    
    float* r0 = (float*)actual.rowPointer(0);
    float* r1 = (float*)actual.rowPointer(1);
    rcUNITTEST_ASSERT(real_equalFloat((5./3.), r0[0], 0.000001));
    rcUNITTEST_ASSERT(real_equalFloat((20./3.), r0[1], 0.000001));
    rcUNITTEST_ASSERT(real_equalFloat(15., r1[0], 0.000001));
    rcUNITTEST_ASSERT(real_equalFloat((80./3.), r1[1], 0.000001));
  }

  /* Do some random tests
   */
  const uint32 testCount = 8;
  srand(0);

  for (uint32 ti = 0; ti < testCount; ti++) {
    /* Step 1 - Generate test conditions and data
     */
    const int32 width = ((rand() >> 4) & 0x1F) + 1;
    const int32 height = ((rand() >> 4) & 0x1F) + 1;
    const uint32 imgCnt = (ti < 2) ? 300 : (uint32)(((rand() >> 4) & 0x1F) + 2);
    const float N = imgCnt;
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++) {
      img[i] = rcWindow(width, height);
      img[i].randomFill(i+1);
    }

    /* Step 2 - Calculate expected results
     */
    vector<vector<float> > expected(height);

    for (int32 i = 0; i < height; i++)
      expected[i].resize(width);
    
    for (int32 y = 0; y < height; y++) {
      for (int32 x = 0; x < width; x++) {
	float sum = 0.0, sumSq = 0.0;
	for (uint32 f = 0; f < img.size(); f++) {
	  uint16 val = img[f].getPixel(x, y);
	  sum += val;
	  sumSq += val*val;
	}
	expected[y][x] = (N*sumSq - sum*sum)/(N*N - N);
      }
    }

    /* Step 3 - Generate kinetic image and check results
     */
    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeVariance);
    if (ti & 1) {
      genKinetics.push(img);
    }
    else {
      for (uint32 i = 0; i < img.size(); i++)
	genKinetics.push(img[i]);
    }
    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);

    for (int32 y = 0; y < height; y++) {
      float* actualP = (float*)actual.rowPointer(y);
      for (int32 x = 0; x < width; x++) {
	rcUNITTEST_ASSERT(real_equalFloat(expected[y][x], *actualP, 0.000001));
	actualP++;
      }
    }
  }
}

void UT_kineticimage::testStdDevGenerator()
{
  /* Test for known results
   */
  {
    const int32 width = 2;
    const int32 height = 2;
    const uint32 imgCnt = 4;
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++)
      img[i] = rcWindow(width, height);

    img[0].setPixel(0, 0, 0); img[0].setPixel(0, 1, 1);
    img[1].setPixel(0, 0, 2); img[1].setPixel(0, 1, 4);
    img[2].setPixel(0, 0, 1); img[2].setPixel(0, 1, 10);
    img[3].setPixel(0, 0, 3); img[3].setPixel(0, 1, 7);
    img[0].setPixel(1, 0, 2); img[0].setPixel(1, 1, 2);
    img[1].setPixel(1, 0, 4); img[1].setPixel(1, 1, 6);
    img[2].setPixel(1, 0, 0); img[2].setPixel(1, 1, 14);
    img[3].setPixel(1, 0, 6); img[3].setPixel(1, 1, 10);

    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeStdDev);
    genKinetics.push(img);

    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);
    
    float* r0 = (float*)actual.rowPointer(0);
    float* r1 = (float*)actual.rowPointer(1);
    rcUNITTEST_ASSERT(real_equalFloat(1.2909944, r0[0], 0.000001));
    rcUNITTEST_ASSERT(real_equalFloat(2.5819889, r0[1], 0.000001));
    rcUNITTEST_ASSERT(real_equalFloat(3.8729833, r1[0], 0.000001));
    rcUNITTEST_ASSERT(real_equalFloat(5.1639778, r1[1], 0.000001));
  }

  /* Do some random tests
   */
  const uint32 testCount = 8;
  srand(16);

  for (uint32 ti = 0; ti < testCount; ti++) {
    /* Step 1 - Generate test conditions and data
     */
    const int32 width = ((rand() >> 4) & 0x1F) + 1;
    const int32 height = ((rand() >> 4) & 0x1F) + 1;
    const uint32 imgCnt = (ti < 2) ? 300 : (uint32)(((rand() >> 4) & 0x1F) + 2);
    const float N = imgCnt;
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++) {
      img[i] = rcWindow(width, height);
      img[i].randomFill(i+1);
    }

    /* Step 2 - Calculate expected results
     */
    vector<vector<float> > expected(height);

    for (int32 i = 0; i < height; i++)
      expected[i].resize(width);
    
    for (int32 y = 0; y < height; y++) {
      for (int32 x = 0; x < width; x++) {
	float sum = 0.0, sumSq = 0.0;
	for (uint32 f = 0; f < img.size(); f++) {
	  uint16 val = img[f].getPixel(x, y);
	  sum += val;
	  sumSq += val*val;
	}
	expected[y][x] = sqrt((N*sumSq - sum*sum)/(N*N - N));
      }
    }

    /* Step 3 - Generate kinetic image and check results
     */
    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeStdDev);
    if (ti & 1) {
      genKinetics.push(img);
    }
    else {
      for (uint32 i = 0; i < img.size(); i++)
	genKinetics.push(img[i]);
    }
    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);

    for (int32 y = 0; y < height; y++) {
      float* actualP = (float*)actual.rowPointer(y);
      for (int32 x = 0; x < width; x++) {
	rcUNITTEST_ASSERT(real_equalFloat(expected[y][x], *actualP, 0.000001));
	actualP++;
      }
    }
  }
}

template <class T>
float genShannonsEntropy(vector<T>& values)
{
  float vSum = 0.0;

  for (uint32 i = 0; i < values.size(); i++)
    vSum += (float)(values[i]);

  float eSum = 0.0;
  for (uint32 i = 0; i < values.size(); i++) {
    float r = (float)(values[i])/vSum;
    eSum += -1 * r * log2(r);
  }

  return eSum/log2((float)values.size());
}

void UT_kineticimage::testVelEntropyGenerator()
{
  /* Test for known results
   */
  {
    const int32 width = 4;
    const int32 height = 4;
    const uint32 imgCnt = 4;
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++) {
      img[i] = rcWindow(width, height);
      img[i].setAllPixels(3);
    }

    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeVelEntropy);
    genKinetics.push(img);
    rcUNITTEST_ASSERT(genKinetics.pop(0) == img.size());

    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);
    
    for (int32 y = 0; y < height; y++) {
      float* act = (float*)actual.rowPointer(y);
      for (int32 x = 0; x < width; x++)
	rcUNITTEST_ASSERT(real_equalFloat(*act++, 1.0, 0.000001));
    }

    vector<rcWindow> img2(imgCnt);
    for (uint32 i = 0; i < img2.size(); i++)
      img2[i] = rcWindow(width, height);

    for (int32 y = 0; y < height; y++)
      for (int32 x = 0; x < width; x++)
	img2[0].setPixel(x, y, x + y*width);

    for (int32 y = 0; y < height; y++)
      for (int32 x = 0; x < width; x++) {
	uint32 pVal = x + y*width;

	for (uint32 i = 1; i < img2.size(); i++)
	    img2[i].setPixel(x, y, ++pVal);
      }

    genKinetics.update(img2);
    rcUNITTEST_ASSERT(genKinetics.pop(0) == img2.size());
    
    genKinetics.genKineticImg(actual);
    float exp = *((float*)actual.rowPointer(0));
    rcUNITTEST_ASSERT(!real_equalFloat(1.0, exp, 0.000001)); // pretty lame, huh?
    
    for (int32 y = 0; y < height; y++) {
      float* act = (float*)actual.rowPointer(y);
      for (int32 x = 0; x < width; x++)
	rcUNITTEST_ASSERT(real_equalFloat(*act++, exp, 0.000001));
    }

    vector<rcWindow> img3(imgCnt);
    for (uint32 i = 0; i < img3.size(); i++)
      img3[i] = rcWindow(width, height);

    for (int32 y = 0; y < height; y++)
      for (int32 x = 0; x < width; x++)
	img3[0].setPixel(x, y, x + y*width);

    for (int32 y = 0; y < height; y++)
      for (int32 x = 0; x < width; x++) {
	uint32 pVal = x + y*width;

	for (uint32 i = 1; i < img3.size(); i++) {
	  pVal += 2;
	  img3[i].setPixel(x, y, pVal);
	}
      }

    genKinetics.update(img3);
    rcUNITTEST_ASSERT(genKinetics.pop(0) == img3.size());
    
    genKinetics.genKineticImg(actual);
    float oldExp = exp;
    exp = *((float*)actual.rowPointer(0));
    rcUNITTEST_ASSERT(real_equalFloat(oldExp, exp, 0.000001));
    
    for (int32 y = 0; y < height; y++) {
      float* act = (float*)actual.rowPointer(y);
      for (int32 x = 0; x < width; x++)
	rcUNITTEST_ASSERT(real_equalFloat(*act++, exp, 0.000001));
    }
  }

  {
    const int32 width = 4;
    const int32 height = 4;
    const uint32 imgCnt = 300;
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++) {
      img[i] = rcWindow(width, height);
      img[i].setAllPixels(3);
    }

    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeVelEntropy);
    genKinetics.push(img);
    rcUNITTEST_ASSERT(genKinetics.pop(0) == img.size());

    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);
    
    for (int32 y = 0; y < height; y++) {
      float* act = (float*)actual.rowPointer(y);
      for (int32 x = 0; x < width; x++)
	rcUNITTEST_ASSERT(real_equalFloat(*act++, 1.0, 0.000001));
    }
  }
}

void UT_kineticimage::testAccelEntropyGenerator()
{
  /* Test for known results
   */
  {
    const int32 width = 10;
    const int32 height = 10;
    const uint32 imgCnt = 4;
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++) {
      img[i] = rcWindow(width, height);
      img[i].setAllPixels(i);
    }

    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeAccelEntropy);
    genKinetics.push(img);
    rcUNITTEST_ASSERT(genKinetics.pop(0) == img.size());

    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);
    
    for (int32 y = 0; y < height; y++) {
      float* act = (float*)actual.rowPointer(0);
      for (int32 x = 0; x < width; x++)
	rcUNITTEST_ASSERT(real_equalFloat(*act++, 1.0, 0.000001));
    }

    vector<rcWindow> img2(imgCnt);
    for (uint32 i = 0; i < img2.size(); i++) {
      img2[i] = rcWindow(width, height);
      img2[i].setAllPixels(2);
    }

    actual.setAllPixels(0); // Make sure these get modified during test
    genKinetics.update(img2);
    rcUNITTEST_ASSERT(genKinetics.pop(0) == img2.size());
    
    genKinetics.genKineticImg(actual);
    
    for (int32 y = 0; y < height; y++) {
      float* act = (float*)actual.rowPointer(0);
      for (int32 x = 0; x < width; x++)
	rcUNITTEST_ASSERT(real_equalFloat(*act++, 1.0, 0.000001));
    }
  }

  /* Do some random tests
   */
  const uint32 testCount = 8;
  srand(32);

  for (uint32 ti = 0; ti < testCount; ti++) {
    /* Step 1 - Generate test conditions and data
     */
    const int32 width = ((rand() >> 4) & 0x1F) + 1;
    const int32 height = ((rand() >> 4) & 0x1F) + 1;
    const uint32 imgCnt = (ti < 2) ? 300 : (uint32)(((rand() >> 4) & 0x1F) + 2);
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++) {
      img[i] = rcWindow(width, height);
      img[i].randomFill(i+1);
    }

    /* Step 2 - Calculate expected results
     */
    vector<vector<float> > expected(height);

    for (int32 i = 0; i < height; i++)
      expected[i].resize(width);
    
    vector<float> data(img.size() - 1);
    for (int32 y = 0; y < height; y++) {
      for (int32 x = 0; x < width; x++) {
	for (uint32 f = 0; f < data.size(); f++) {
	  int16 val0 = img[f].getPixel(x, y);
	  int16 val1 = img[f+1].getPixel(x, y);
	  data[f] = val0 - val1 + 256;
	}
	expected[y][x] = genShannonsEntropy(data);
      }
    }

    /* Step 3 - Generate kinetic image and check results
     */
    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeAccelEntropy);
    if (ti & 1) {
      genKinetics.push(img);
    }
    else {
      for (uint32 i = 0; i < img.size(); i++)
	genKinetics.push(img[i]);
    }
    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);

    for (int32 y = 0; y < height; y++) {
      float* actualP = (float*)actual.rowPointer(y);
      for (int32 x = 0; x < width; x++) {
	rcUNITTEST_ASSERT(real_equalFloat(expected[y][x], *actualP, 0.000001));
	actualP++;
      }
    }
  }
}

void UT_kineticimage::testThetaEntropyGenerator()
{
  /* Test for known results
   */
  {
    const int32 width = 10;
    const int32 height = 10;
    const uint32 imgCnt = 4;
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++) {
      img[i] = rcWindow(width, height);
      img[i].setAllPixels(i);
    }

    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeThetaEntropy);
    genKinetics.push(img);
    rcUNITTEST_ASSERT(genKinetics.pop(0) == img.size());

    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);
    
    for (int32 y = 0; y < height; y++) {
      float* act = (float*)actual.rowPointer(0);
      for (int32 x = 0; x < width; x++)
	rcUNITTEST_ASSERT(real_equalFloat(*act++, 1.0, 0.000001));
    }

    vector<rcWindow> img2(imgCnt);
    for (uint32 i = 0; i < img2.size(); i++) {
      img2[i] = rcWindow(width, height);
      img2[i].setAllPixels(2);
    }

    actual.setAllPixels(0); // Make sure these get modified during test
    genKinetics.update(img2);
    rcUNITTEST_ASSERT(genKinetics.pop(0) == img2.size());
    
    genKinetics.genKineticImg(actual);
    
    for (int32 y = 0; y < height; y++) {
      float* act = (float*)actual.rowPointer(0);
      for (int32 x = 0; x < width; x++)
	rcUNITTEST_ASSERT(real_equalFloat(*act++, 1.0, 0.000001));
    }
  }

  /* Create theta array.
   */
  float theta[511];
  {
    float offset = -atan((float)-255) + 0.1;
    for (int32 i = -255; i < 256; i++)
      theta[255 + i] = atan((float)i) + offset;
  }

  /* Do some random tests
   */
  const uint32 testCount = 8;
  srand(32);

  for (uint32 ti = 0; ti < testCount; ti++) {
    /* Step 1 - Generate test conditions and data
     */
    const int32 width = ((rand() >> 4) & 0x1F) + 1;
    const int32 height = ((rand() >> 4) & 0x1F) + 1;
    const uint32 imgCnt = (ti < 2) ? 300 : (uint32)(((rand() >> 4) & 0x1F) + 2);
    vector<rcWindow> img(imgCnt);

    for (uint32 i = 0; i < img.size(); i++) {
      img[i] = rcWindow(width, height);
      img[i].randomFill(i+1);
    }

    /* Step 2 - Calculate expected results
     */
    vector<vector<float> > expected(height);

    for (int32 i = 0; i < height; i++)
      expected[i].resize(width);
    
    vector<float> data(img.size() - 1);
    for (int32 y = 0; y < height; y++) {
      for (int32 x = 0; x < width; x++) {
	for (uint32 f = 0; f < data.size(); f++) {
	  int16 val0 = img[f].getPixel(x, y);
	  int16 val1 = img[f+1].getPixel(x, y);
	  data[f] = theta[val0 - val1 + 255];
	}
	expected[y][x] = genShannonsEntropy(data);
      }
    }

    /* Step 3 - Generate kinetic image and check results
     */
    rcOptoKineticImage genKinetics(rcOptoKineticImage::eKineTypeThetaEntropy);
    if (ti & 1) {
      genKinetics.push(img);
    }
    else {
      for (uint32 i = 0; i < img.size(); i++)
	genKinetics.push(img[i]);
    }
    rcWindow actual(width, height, rcPixel32);
    genKinetics.genKineticImg(actual);

    for (int32 y = 0; y < height; y++) {
      float* actualP = (float*)actual.rowPointer(y);
      for (int32 x = 0; x < width; x++) {
	rcUNITTEST_ASSERT(real_equalFloat(expected[y][x], *actualP, 0.000001));
	actualP++;
      }
    }
  }
}

void UT_kineticimage::testEightBitImageGenerator()
{
  const int32 t1w = 2, t1h = 2, t2w = 4, t2h = 2;
  vector<rcWindow> t1(4), t2(4);

  for (uint32 i = 0; i < t1.size(); i++) {
    t1[i] = rcWindow(t1w, t1h);
    t2[i] = rcWindow(t2w, t2h);

    uint8 pVal = i;
    for (int32 y = 0; y < t1[i].height(); y++)
      for (int32 x = 0; x < t1[i].width(); x++) {
	t1[i].setPixel(x, y, pVal);
	t2[i].setPixel(x, y, pVal);
	t2[i].setPixel(x+2, y, pVal);
	pVal += i;
      }
  }
  
  vector<uint8> pMap(256);
  for (uint32 i = 0; i < pMap.size(); i++) pMap[i] = (uint8)(255 - i);
  
  rcOptoKineticImage gk1(rcOptoKineticImage::eKineTypeStdDev);
  gk1.push(t1);
  rcWindow a1(t1[0].width(), t1[0].height(), rcPixel32);
  gk1.genKineticImg(a1);
  
  rcOptoKineticImage gk2(rcOptoKineticImage::eKineTypeStdDev);
  gk2.push(t2);
  rcWindow a2(t2[0].width(), t2[0].height(), rcPixel32);
  gk2.genKineticImg(a2);

  rcWindow r1(t1[0].width(), t1[0].height());
  rcWindow r2(t2[0].width(), t2[0].height());

  gk1.genEightBitImg(a1, r1, 1.0, &pMap);
  gk2.genEightBitImg(a2, r2, 1.0, &pMap);

  rcUNITTEST_ASSERT(r1.getPixel(0, 0) == 255);
  rcUNITTEST_ASSERT(r2.getPixel(0, 0) == 255);
  rcUNITTEST_ASSERT(r2.getPixel(2, 0) == 255);

  rcUNITTEST_ASSERT(r1.getPixel(1, 0) == 177);
  rcUNITTEST_ASSERT(r2.getPixel(1, 0) == 181);
  rcUNITTEST_ASSERT(r2.getPixel(3, 0) == 181);

  rcUNITTEST_ASSERT(r1.getPixel(0, 1) == 78);
  rcUNITTEST_ASSERT(r2.getPixel(0, 1) == 74);
  rcUNITTEST_ASSERT(r2.getPixel(2, 1) == 74);

  rcUNITTEST_ASSERT(r1.getPixel(1, 1) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(1, 1) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(3, 1) == 0);

  gk1.genEightBitImg(a1, r1, 1.0, 0);
  gk2.genEightBitImg(a2, r2, 1.0, 0);

  rcUNITTEST_ASSERT(r1.getPixel(0, 0) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(0, 0) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(2, 0) == 0);

  rcUNITTEST_ASSERT(r1.getPixel(1, 0) == 78);
  rcUNITTEST_ASSERT(r2.getPixel(1, 0) == 74);
  rcUNITTEST_ASSERT(r2.getPixel(3, 0) == 74);

  rcUNITTEST_ASSERT(r1.getPixel(0, 1) == 177);
  rcUNITTEST_ASSERT(r2.getPixel(0, 1) == 181);
  rcUNITTEST_ASSERT(r2.getPixel(2, 1) == 181);

  rcUNITTEST_ASSERT(r1.getPixel(1, 1) == 255);
  rcUNITTEST_ASSERT(r2.getPixel(1, 1) == 255);
  rcUNITTEST_ASSERT(r2.getPixel(3, 1) == 255);

  gk1.genEightBitImg(a1, r1, 0.0, &pMap);
  gk2.genEightBitImg(a2, r2, 0.0, &pMap);

  rcUNITTEST_ASSERT(r1.getPixel(0, 0) == 255);
  rcUNITTEST_ASSERT(r2.getPixel(0, 0) == 255);
  rcUNITTEST_ASSERT(r2.getPixel(2, 0) == 255);

  rcUNITTEST_ASSERT(r1.getPixel(1, 0) == 170);
  rcUNITTEST_ASSERT(r2.getPixel(1, 0) == 170);
  rcUNITTEST_ASSERT(r2.getPixel(3, 0) == 170);

  rcUNITTEST_ASSERT(r1.getPixel(0, 1) == 85);
  rcUNITTEST_ASSERT(r2.getPixel(0, 1) == 85);
  rcUNITTEST_ASSERT(r2.getPixel(2, 1) == 85);

  rcUNITTEST_ASSERT(r1.getPixel(1, 1) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(1, 1) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(3, 1) == 0);

  gk1.genEightBitImg(a1, r1, 0.0, 0);
  gk2.genEightBitImg(a2, r2, 0.0, 0);

  rcUNITTEST_ASSERT(r1.getPixel(0, 0) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(0, 0) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(2, 0) == 0);

  rcUNITTEST_ASSERT(r1.getPixel(1, 0) == 85);
  rcUNITTEST_ASSERT(r2.getPixel(1, 0) == 85);
  rcUNITTEST_ASSERT(r2.getPixel(3, 0) == 85);

  rcUNITTEST_ASSERT(r1.getPixel(0, 1) == 170);
  rcUNITTEST_ASSERT(r2.getPixel(0, 1) == 170);
  rcUNITTEST_ASSERT(r2.getPixel(2, 1) == 170);

  rcUNITTEST_ASSERT(r1.getPixel(1, 1) == 255);
  rcUNITTEST_ASSERT(r2.getPixel(1, 1) == 255);
  rcUNITTEST_ASSERT(r2.getPixel(3, 1) == 255);

  vector<float> vMap(5);
  vMap[0] = 1.5; vMap[1] = 1.7; vMap[2] = 2.8; vMap[3] = 4.0; vMap[4] = 5.0;
  
  gk1.genEightBitImg(a1, r1, vMap);
  gk2.genEightBitImg(a2, r2, vMap);

  rcUNITTEST_ASSERT(r1.getPixel(0, 0) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(0, 0) == 0);
  rcUNITTEST_ASSERT(r2.getPixel(2, 0) == 0);

  rcUNITTEST_ASSERT(r1.getPixel(1, 0) == 2);
  rcUNITTEST_ASSERT(r2.getPixel(1, 0) == 2);
  rcUNITTEST_ASSERT(r2.getPixel(3, 0) == 2);

  rcUNITTEST_ASSERT(r1.getPixel(0, 1) == 3);
  rcUNITTEST_ASSERT(r2.getPixel(0, 1) == 3);
  rcUNITTEST_ASSERT(r2.getPixel(2, 1) == 3);

  rcUNITTEST_ASSERT(r1.getPixel(1, 1) == 5);
  rcUNITTEST_ASSERT(r2.getPixel(1, 1) == 5);
  rcUNITTEST_ASSERT(r2.getPixel(3, 1) == 5);
}


