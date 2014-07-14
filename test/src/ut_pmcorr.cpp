/*
 *  corrut.cpp
 *  correlation
 *
 *  Created by Arman Garakani on Sun Apr 28 2002.
 *  Copyright (c) 2001 __ReifyCorp__. All rights reserved.
 *
 */

#include "ut_pmcorr.h"
#include <rc_utdrawutils.h>

#include <stdlib.h>
#include <vector>
#include <map>
#include <algorithm>
#include <rc_correlationwindow.h>
#include <rc_mathmodel.h>
#include <rc_kinepyr.h>
#include <rc_1dcorr.h>
#include <rc_diamondscan.h>

#define UTCHECKVEC(a, b, c, d) {\
rcUTCheck (real_equal((a).x(), (b), (d)));	\
rcUTCheck (real_equal((a).y(), (c), (d))); }


using namespace std;

#include <rc_time.h>


/// Image Find Unit Tests

void UT_Correlation::testcorrfind ()
{
	uint32 oldErrors = mErrors;
	int32 w (32), h(41);
	uint8 color (128), background (240);

	// First test moving target 3x3 positions covering edges
	rcWindow big (w+4, h+4);
	rcWindow scene (big, 1, 1, w+2, h+2);
	rcWindow plate (w, h);
	rfDrawCircle (rc2Dvector (16.0, 16.0), 16, plate, color, background);

	for (int32 j = 0; j < (scene.height() - plate.height() + 1); j++)
		for (int32 i = 0; i < (scene.width() - plate.width() + 1); i++)
		{
			rcFindList finds;
			rcWindow space;

			big.randomFill ();
			rcWindow tmp (scene, i, j, w, h);
			tmp.copyPixelsFromWindow (plate);
			rfImageFind (scene, plate, finds, space, true);

			// rfPrintFindList (finds, cerr);

			rcUTCheck (space.width() == (scene.width() - plate.width() + 1));
			rcUTCheck (space.height() == (scene.height() - plate.height() + 1));
			rcUTCheck (space.getPixel (i, j) == mQuantz);

			// Random image actual interpolated value will vary but the edges are integer positions
			if (! (i == 1 && j == 1) )
			{
				UTCHECKVEC(finds.begin()->interp, (float) i, (float) j, 0.001f);
			}
			else
			{
				UTCHECKVEC(finds.begin()->interp, (float) i, (float) j, 0.1f);
			}
		}

	printSuccessMessage("Image Find Basic test ", mErrors - oldErrors);
}

void UT_Correlation::test1dcorr ()
{
	uint32 oldErrors = mErrors;
	static const float k[5] = {1, 4, 6, 4, 1};
	static const float s[140] = {6972., 6903., 6820., 6743., 6686., 6661., 6638., 6611., 6573., 6524., 6476.,
		6431., 6375., 6353., 6399., 6460., 6505., 6527., 6548., 6556., 6562., 6553.,
		6537., 6519., 6494., 6461., 6443., 6442., 6456., 6463., 6469., 6482., 6504.,
		6510., 6514., 6515., 6508., 6487., 6471., 6434., 6381., 6330., 6282., 6238.,
		6198., 6180., 6159., 6140., 6153., 6171., 6202., 6214., 6185., 6136., 6111.,
		6090., 6076., 6061., 6038., 5993., 5967., 5960., 5973., 5980., 6007., 6036.,
		6050., 6078., 6114., 6140., 6148., 6153., 6146., 6143., 6135., 6144., 6186.,
		6223., 6248., 6265., 6270., 6261., 6241., 6225., 6210., 6222., 6216., 6195.,
		6203., 6241., 6291., 6331., 6362., 6405., 6443., 6485., 6524., 6549., 6553.,
		6539., 6498., 6435., 6362., 6288., 6240., 6239., 6298., 6453., 6697., 6912.,
	7038., 7098., 7119., 7129., 7127.};

	std::vector<float> kernel, signal;
	for (uint32 i = 0; i < 5; i++) kernel.push_back (k[i]);
	for (uint32 i = 0; i < 140; i++) signal.push_back (s[i]);

	std::vector<double> cspace;
	std::vector<float>::iterator ms, me;
	ms = me = signal.begin(); advance (me, kernel.size());
	for (; me != signal.end() && ms != signal.end(); me++, ms++)
    {
		cspace.push_back ( rf1DNormalizedCorr (ms, me, kernel.begin(), kernel.end()));
    }

	// The first peak correlation corresponding to the first valley
	rcUTCheck (real_equal(cspace[11], 0.9020523, 0.0001));

	double pose (-100.0);
	ms = signal.begin(); me = signal.end ();
	double alignment = rf1DRegister (ms, me, ms, me, 32, pose);
	rcUTCheck (real_equal(alignment, 32.0, 0.001));
	rcUTCheck (real_equal(pose, 1.0, 0.001));

	ms = signal.begin(); me = signal.end ();
	alignment = rf1DRegister (ms, me, ms, me, 16, 1, pose);
	rcUTCheck (real_equal(alignment, 1.0, 0.001));
	rcUTCheck (real_equal(pose, 1.0, 0.001));

	ms = signal.begin(); me = signal.end ();
	alignment = rf1DRegister (ms, me, ms, me, 1, 16, pose);
	rcUTCheck (real_equal(alignment, 16.0, 0.001));
	rcUTCheck (real_equal(pose, 1.0, 0.001));

	printSuccessMessage("1d correlation and registration tests ", mErrors - oldErrors);
}


// Vanila image printing
#define cmPrintImage(a){ \
for (int i = 0; i < (a).height(); i++) \
{ \
   fprintf (stderr, "\n"); \
      for (int j = 0; j < (a).width(); j++) \
	fprintf (stderr, " %3d ", (a).getPixel (j, i));	\
            fprintf (stderr, "\n");\
}}

UT_Correlation::UT_Correlation()
{
}

UT_Correlation::~UT_Correlation()
{
   printSuccessMessage( "Correlation test", mErrors );
}

uint32
UT_Correlation::run() {
   {
     test1dcorr ();
     testcorrfind ();
      testMath( "Math utilities" );
      testMaskedCorrelation();

      testCorrelation (51, 47, rcPixel8, true);
      testCorrelation (51, 47, rcPixel16, true);
      testCorrelation (51, 47, rcPixel32S, true);
      testCorrelation (1280, 960, rcPixel32S, true);

      testCorrelation (51, 47, rcPixel8, false);
      testCorrelation (51, 47, rcPixel16, false);
      testCorrelation (51, 47, rcPixel32S, false);
      testCorrelation (1280, 960, rcPixel32S, false);

      rfForceSIMD ( true );

      for ( int32 width = 320; width <= 1280; width *= 2 ) {
          int32 height = (width * 3)/4;
          testOptoKinetic (width, height, rcPixel8);
          testOptoKinetic (width, height, rcPixel32S);
      }


      // Compose a vector of image sizes for performance test
      std::vector<rcIPair> imageSizes;
      imageSizes.push_back( rcIPair( 1, 1 ) );      // Power-of-2 sizes
      //imageSizes.push_back( rcIPair( 2, 2 ) );
      //imageSizes.push_back( rcIPair( 8, 8 ) );
      imageSizes.push_back( rcIPair( 16, 16 ) );
      //imageSizes.push_back( rcIPair( 32, 32 ) );
      //imageSizes.push_back( rcIPair( 128, 128 ) );
      //imageSizes.push_back( rcIPair( 512, 512 ) );
      //imageSizes.push_back( rcIPair( 2048, 2048 ) );
      imageSizes.push_back( rcIPair( 1024, 768 ) ); // Sony 15 fps camera size
      imageSizes.push_back( rcIPair( 1280, 960 ) ); // Sony 7.5 fps camera size
      imageSizes.push_back( rcIPair( 359, 479 ) );  // Odd non-aligned size

#if defined (PERFORMANCE)
      for ( uint32 i = 0; i < imageSizes.size(); ++i ) {
          // TODO: implement tests for all depths
          for ( rcPixel depth = rcPixel8; depth < rcPixel16; depth = rcPixel(depth * 2) ) {
              // Non-Altivec test
              testTimes( 0, imageSizes[i].x(), imageSizes[i].y(), depth );
              // Altivec test
              testTimes( 1, imageSizes[i].x(), imageSizes[i].y(), depth );
              fprintf( stderr, "\n" );
          }
      }
#endif // Performance

   }
   return mErrors;
}

typedef struct IMGLOC { int32 x; int32 y; } IMGLOC;

void UT_Correlation::testMaskedCorrelation()
{
  srand(0);

  /* Test mask logic by:
   *
   * 1) Setting half the pixels in the test img/mdl images to the same
   *    values as all the pixels in the "expected" img/mdl images.
   *    (Half the mdl/img pixels are set to match to guarantee that
   *    correlation will get some non-zero result).
   *
   * 2) Only setting mask image pixels at the test image pixel
   *    locations used in 1).
   *
   * 3) Setting all remaining test images pixels to random values.
   *
   * 4) Verify that a standard correlation of the two "expected"
   *    images yields the same result as a masked correlation of the
   *    two test images.
   */
  for (int32 repCnt = 0; repCnt < 3; repCnt++) {
    multimap<int32, IMGLOC> pos;

    rcWindow testMdl(512, 128), testImg(512, 128), mask(512, 128);
    rcWindow expMdl(256, 128), expImg(256, 128);

    /* Generate randomly ordered list of pixel locations.
     */
    for (int32 y = 0; y < testMdl.height(); y++)
      for (int32 x = 0; x < testMdl.width(); x++) {
	IMGLOC loc;
	loc.x = x; loc.y = y;

	pair<const int32, IMGLOC> newval(rand(), loc);
	pos.insert(newval);
      }

    /* Take first half of randomly ordered locations and use them in
     * to figure out where to set pixels in the test images and the
     * mask image.
     */
    multimap<int32, IMGLOC>::iterator cur = pos.begin();
    int32 ctr = 0;
    mask.setAllPixels(0);

    for (int32 y = 0; y < expMdl.height(); y++)
      for (int32 x = 0; x < expMdl.width(); x++) {
	uint32 mVal = 1;
	uint32 iVal = 1;
	if ((x ^ y) & 1) {
	  mVal = (rand() >> 8) & 0xFF;
	  iVal = (rand() >> 8) & 0xFF;
	}
	expMdl.setPixel(x, y, mVal);
	expImg.setPixel(x, y, iVal);;

	rmAssert(cur != pos.end());
	IMGLOC loc = cur->second;
	cur++; ctr++;

	rmAssert(loc.x >= 0 && loc.x < testMdl.width());
	rmAssert(loc.y >= 0 && loc.y < testMdl.height());

	testMdl.setPixel(loc.x, loc.y, mVal);
	testImg.setPixel(loc.x, loc.y, iVal);
	mask.setPixel(loc.x, loc.y, 0xFF);
      }

    /* Set remaining pixels in test images.
     */
    while (cur != pos.end()) {
      uint32 mVal = (rand() >> 8) & 0xFF;
      uint32 iVal = (rand() >> 8) & 0xFF;

      IMGLOC loc = cur->second;
      cur++; ctr++;

      rmAssert(loc.x >= 0 && loc.x < testMdl.width());
      rmAssert(loc.y >= 0 && loc.y < testMdl.height());

      testMdl.setPixel(loc.x, loc.y, mVal);
      testImg.setPixel(loc.x, loc.y, iVal);
    }

    rmAssert(ctr == testMdl.width()*testMdl.height());

    rsCorrParams params;
    rcCorr resExp, resMask;

    rfCorrelate(testMdl, testImg, mask, params, resMask);
    rfCorrelate(expMdl, expImg, params, resExp);

    rcUNITTEST_ASSERT(resMask.r() == resExp.r());
  }
}

// Simple smoke test
void UT_Correlation::testOptoKinetic (uint32 width, uint32 height, rcPixel d )
{
    rcFrameRef ptr (new rcFrame( width, height, d ));
    rcFrameRef ptr2 (new rcFrame( width, height, d ));

    rcWindow imgA (ptr, 0, 0, width, height);
    rcWindow imgB (ptr2, 0, 0, width, height);
    rcWindow imgC (ptr2, 0, 0, width, height);

    // Set all pels to repeatable random values
    int32 seed = width;
    imgA.randomFill( seed );
    imgB.randomFill( seed*2 );
    imgC.randomFill( seed*3 );

    testOptoKineticPermutation( imgA, imgB, imgC, seed );

    // Set all pels to non-repeatable random values (a new seed is generated for each run)
    seed = imgA.randomFill( 0 );
    imgB.randomFill( 0 );
    imgC.randomFill( 0 );

    testOptoKineticPermutation( imgA, imgB, imgC, seed );
}

void UT_Correlation::testCorrelation (uint32 width, uint32 height, rcPixel d, bool useAltivec )
{
   uint32 oldErrors = mErrors;

   rfForceSIMD ( useAltivec );

   rcFrameRef ptr (new rcFrame( width, height, d ));
   rcFrameRef ptr2 (new rcFrame( width, height, d ));

   rcWindow imgA (ptr, 0, 0, width, height);
   rcWindow imgB (ptr2, 0, 0, width, height);

   uint32 val = (d == rcPixel8) ? rcUINT8_MAX : (d == rcPixel16) ? rcUINT16_MAX : rcUINT32_MAX;

   // Set all pels
   imgA.setAllPixels (val);
   imgB.setAllPixels (val);

   // Set a single Pel
   ptr->setPixel (imgA.width() / 2, imgA.height() / 2, val-1);
   ptr2->setPixel (imgB.width() / 2, imgB.height() / 2, val-1);

   rcCorr res, res2;
   rcUNITTEST_ASSERT( res == res2 );
   res2.Si( 666 );
   rcUNITTEST_ASSERT( res != res2 );
   res2 = res;

   rfForceSIMD ( useAltivec );

   rsCorrParams par;
   rfCorrelate (imgA, imgB, par, res);

   // Test sums caching
   if ( d == rcPixel8 ) {
       uint32 exceptions = 0;
       try {
           rcCorrelationWindow<uint8> cimgA( imgA );
           rcCorrelationWindow<uint8> cimgB( imgB );
           rfCorrelateWindow (cimgA, cimgB, par, res2);
           rcUNITTEST_ASSERT( res == res2 );
           compareResults( useAltivec, res, res2 );
       } catch ( general_exception& e ) {
           ++exceptions;
       }
       rcUNITTEST_ASSERT( exceptions == 0 );
   } else if ( d == rcPixel16 ) {
       uint32 exceptions = 0;
       try {
           rcCorrelationWindow<uint16> cimgA( imgA );
           rcCorrelationWindow<uint16> cimgB( imgB );
           rfCorrelateWindow (cimgA, cimgB, par, res2);
           rcUNITTEST_ASSERT( res == res2 );
           compareResults( useAltivec, res, res2 );
       } catch ( general_exception& e ) {
           ++exceptions;
       }
       rcUNITTEST_ASSERT( exceptions == 0 );
   } else if ( d == rcPixel32S ) {
       uint32 exceptions = 0;
       try {
           rcCorrelationWindow<uint32> cimgA( imgA );
           rcCorrelationWindow<uint32> cimgB( imgB );
           rfCorrelateWindow (cimgA, cimgB, par, res2);
           rcUNITTEST_ASSERT( res == res2 );
           compareResults( useAltivec, res, res2 );
       } catch ( general_exception& e ) {
           ++exceptions;
       }
       rcUNITTEST_ASSERT( exceptions == 0 );
   }

   double trueSi = imgA.width () * imgA.height() * imgA.depth () * (double)(rcUINT8_MAX) - 1.;
   double trueSm = imgB.width () * imgB.height() * imgB.depth () * (double)(rcUINT8_MAX) - 1.;
   double trueSii = (imgA.width () * imgA.height() * imgA.depth() - 1) *
      (double)(rcUINT8_MAX) * (double)(rcUINT8_MAX) + (double)(rcUINT8_MAX - 1) * (double)(rcUINT8_MAX - 1);

   double trueSmm = trueSii;
   double trueSim = trueSii;

   rcUNITTEST_ASSERT (res.r() == 1.0);
   rcUNITTEST_ASSERT (res.n() == int32(imgA.width() * imgB.depth() * imgB.height()));
   rcUNITTEST_ASSERT (res.Si() == trueSi);
   rcUNITTEST_ASSERT (res.Sm() == trueSm);
   rcUNITTEST_ASSERT (res.Sii() == trueSii);
   rcUNITTEST_ASSERT (res.Smm() == trueSmm);
   rcUNITTEST_ASSERT (res.Sim() == trueSim);

   if (res.r() != 1.0)
       cerr << d*8 << " res.r() == " << res.r() << endl;
   if (res.Si() != trueSi)
       cerr << "trueSi = " << trueSi << ", res.Si() = " << res.Si() << endl;
   if (res.Sm() != trueSm)
       cerr << "trueSm = " << trueSm << ", res.Sm() = " << res.Sm() << endl;

   char message[512];
   sprintf( message, "rfCorrelate: %d x %d x %d %s", width, height, d*8, useAltivec ? "Altivec" : "" );
   printSuccessMessage( message, mErrors - oldErrors );
}


void UT_Correlation::testTimes ( bool useAltivec, int32 width, int32 height, rcPixel depth )
{
    const uint32 minPixels = 10000000;
    const uint32 pixels = width * height;
    // Adjust repeats depending on image size
    uint32 repeats = minPixels/pixels;

    if ( repeats < 1 )
        repeats = 1;
    else if ( repeats > 10000 )
        repeats = 10000;

   // Now setup a case that utilizes AltiVec
   rcFrameRef ptr (new rcFrame( width, height, depth ));
   rcFrameRef ptr2 (new rcFrame( width, height, depth ));

   rcWindow imgA (ptr);imgA.setAllPixels (255);
   rcWindow imgB (ptr2);imgA.setAllPixels (255);

   // Set a single Pel
   ptr->setPixel (imgA.width() / 2, imgA.height() / 2, 255-1);
   ptr2->setPixel (imgB.width() / 2, imgB.height() / 2, 255-1);

   rcCorr res;
   rcTime timer;

   // Alticev setting
   rfForceSIMD ( useAltivec );

   // Now Time the correlation stuff
   // Run test in a loop to diminish the effect of system clock
   // inaccuracy. The time spent by the loop code should be negligible
   rsCorrParams par;
   timer.start ();
   for( uint32 i = 0; i < repeats; ++i )
       rfCorrelate (imgA, imgB, par,res);
   timer.end ();

   double dMilliSeconds = timer.milliseconds () / repeats;
   double dMicroSeconds = timer.microseconds () / repeats;

   // Per Byte in Useconds
   double perByte = dMicroSeconds / (imgA.width() * imgA.height() * imgA.depth());

   fprintf(stderr,
           "Performance: %s rcWindow<%i> correlation                          [%i x %i]: %.3f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
           useAltivec ? "AltiVec" : "       ", imgA.depth() * 8, imgA.width(), imgA.height(), dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);

    //
    // Correlate, no cached sums
    //
    timer.start();
    {
        rcCorrelationWindow<uint8> imageA( imgA );
        rcCorrelationWindow<uint8> imageB( imgB );
        for( uint32 i = 0; i < repeats; ++i ) {
            rfCorrelateWindow (imageA, imageB, par, res);
            imageA.invalidate();
            imageB.invalidate();
        }
    }
    timer.end();

    dMilliSeconds = timer.milliseconds () / repeats;
    dMicroSeconds = timer.microseconds () / repeats;
    // Per Byte in Useconds
    perByte = dMicroSeconds / (imgA.width() * imgA.height() * imgA.depth());

    fprintf(stderr,
            "Performance: %s rcCorrelationWindow<%i> correlation   none cached [%i x %i]: %.3f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
            useAltivec ? "AltiVec" : "       ", imgA.depth() * 8, imgA.width(), imgA.height(), dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);

    //
    // Correlate, cached sums for imageA
    //
    timer.start();
    {
        rcCorrelationWindow<uint8> imageA( imgA );
        rcCorrelationWindow<uint8> imageB( imgB );
        rfCorrelateWindow (imageA, imageA, par, res);
        for( uint32 i = 0; i < repeats; ++i ) {
            rfCorrelateWindow (imageA, imageB, par, res);
            imageB.invalidate();
        }
    }
    timer.end();

    dMilliSeconds = timer.milliseconds () / repeats;
    dMicroSeconds = timer.microseconds () / repeats;
    // Per Byte in Useconds
    perByte = dMicroSeconds / (imgA.width() * imgA.height() * imgA.depth());

    fprintf(stderr,
            "Performance: %s rcCorrelationWindow<%i> correlation imageA cached [%i x %i]: %.3f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
            useAltivec ? "AltiVec" : "       ", imgA.depth() * 8, imgA.width(), imgA.height(), dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);

    //
    // Correlate, cached sums for imageB
    //
    timer.start();
    {
        rcCorrelationWindow<uint8> imageA( imgA );
        rcCorrelationWindow<uint8> imageB( imgB );
        rfCorrelateWindow (imageB, imageB, par, res);
        for( uint32 i = 0; i < repeats; ++i ) {
            rfCorrelateWindow (imageA, imageB, par, res);
            imageA.invalidate();
        }
    }
    timer.end();

    dMilliSeconds = timer.milliseconds () / repeats;
    dMicroSeconds = timer.microseconds () / repeats;
    // Per Byte in Useconds
    perByte = dMicroSeconds / (imgA.width() * imgA.height() * imgA.depth());

    fprintf(stderr,
            "Performance: %s rcCorrelationWindow<%i> correlation imageB cached [%i x %i]: %.3f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
            useAltivec ? "AltiVec" : "       ", imgA.depth() * 8,  imgA.width(), imgA.height(), dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);

    //
    // Correlate, cached sums for imageA and imageB
    //
    timer.start();
    {
        rcCorrelationWindow<uint8> imageA( imgA );
        rcCorrelationWindow<uint8> imageB( imgB );
        rfCorrelateWindow (imageA, imageA, par, res);
        rfCorrelateWindow (imageB, imageB, par, res);
        for( uint32 i = 0; i < repeats; ++i ) {
            rfCorrelateWindow (imageA, imageB, par, res);
        }
    }
    timer.end();

    dMilliSeconds = timer.milliseconds () / repeats;
    dMicroSeconds = timer.microseconds () / repeats;
    // Per Byte in Useconds
    perByte = dMicroSeconds / (imgA.width() * imgA.height() * imgA.depth());

    fprintf(stderr,
            "Performance: %s rcCorrelationWindow<%i> correlation   both cached [%i x %i]: %.3f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
            useAltivec ? "AltiVec" : "       ", imgA.depth() * 8, imgA.width(), imgA.height(), dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);
}

void UT_Correlation::testEnergyIntegrate ()
{
  std::vector<double> euler (101);

  for (uint32 i = 0; i < 101; i++) euler [i] = i;

  rfIntegrateEnergy (euler);

  for (uint32 i = 0; i < 101; i++)
    {
      rcUNITTEST_ASSERT (euler[i] == ((i * i + i) / 2));
    }
}

void UT_Correlation::testOptoKineticPermutation( rcWindow& w1, rcWindow& w2, rcWindow& w3, uint32 seed )
{
    uint32 oldErrors = mErrors;

    std::vector<rcWindow> images1;
    images1.push_back( w1 );
    images1.push_back( w2 );
    images1.push_back( w3 );

    rsCorrParams parm;

    // Analyze vector 1
    std::vector<double> results1( images1.size(), 0.0 );
    rfOptoKineticEnergy( images1, results1, parm, 0 );

    // Values should be in the range [0-1]
    for( std::vector<double>::iterator i = results1.begin(); i < results1.end(); i++ ) {
        rcUNITTEST_ASSERT( *i >= 0 );
        rcUNITTEST_ASSERT( *i <= 1.0 );
    }

    // Analyze vector 2
    // images2 is a reverse of images1
    std::vector<rcWindow> images2( images1 );
    reverse( images2.begin(), images2.end() );
    std::vector<double> results2( results1 );
    // The images are the same, only their order is different
    // Theoretically the results should be the same...
    rfOptoKineticEnergy( images2, results2, parm, 0 );

     // Values should be in the range [0-1]
    for( std::vector<double>::iterator i = results2.begin(); i < results2.end(); i++ ) {
        rcUNITTEST_ASSERT( *i >= 0 );
        rcUNITTEST_ASSERT( *i <= 1.0 );
    }

    // Reverse second vector
    reverse( results2.begin(), results2.end() );

    // Scores should be at least this accurate...
    const double minAccuracy = 1.0/cmIntegerCorr;

    // Compare vectors, results should be the same
    for( uint32 i = 0; i < results1.size(); i++ ) {
        double diff = results1[i] - results2[i];

        rcUNITTEST_ASSERT( diff < minAccuracy );
        if ( diff >= minAccuracy ) {
            fprintf( stderr, "\tseed %u entropy value %.16f != entropy value %.16f, diff %.16f\n",
                     seed, results1[i], results2[i], diff );
        }
    }

    char message[512];
    sprintf( message, "rfOptoKineticEnergy: %d x %d x %d",
             w1.width(), w1.height(), w1.depth()*8 );

    printSuccessMessage( message, mErrors - oldErrors );
}

// Test math utility functions
void UT_Correlation::testMath( char* message )
{
    uint32 oldErrors = mErrors;

    // Test square table
    rcSquareTable squares;

    rcUNITTEST_ASSERT( squares.size() > 0 );
    for ( uint32 i = 0; i < squares.size(); i++ ) {
        rcUNITTEST_ASSERT( (i*i) == squares[i] );
    }

    printSuccessMessage( message, mErrors - oldErrors );
}

void UT_Correlation::compareResults( bool useAltivec, const rcCorr& r1, const rcCorr& r2 )
{
    if ( r1 != r2 ) {
        cerr << (useAltivec ? "Altivec results" : "Results") << " differ:" << endl;
        cerr << r1 << r2 << endl;
    }
}

// Test all cache variations
void UT_Correlation::testCachedCorrelation( bool useAltivec, const rcWindow& wI, const rcWindow& wM,
                                            const rsCorrParams& params, const rcCorr& res )
{
    rcCorr res2;
    rcCorrelationWindow<uint8> winIC( wI );
    rcCorrelationWindow<uint8> winMC( wM );

    // Neither cached
    rfCorrelateWindow(winIC, winMC, params, res2);
    rcUNITTEST_ASSERT( res == res2 );
    compareResults( useAltivec, res, res2 );
    // Both cached
    rfCorrelateWindow(winIC, winMC, params, res2);
    rcUNITTEST_ASSERT( res == res2 );
    compareResults( useAltivec, res, res2 );
    // Second cached
    rcCorrelationWindow<uint8> winIC2( wI );
    rfCorrelateWindow(winIC2, winMC, params, res2);
    rcUNITTEST_ASSERT( res == res2 );
    compareResults( useAltivec, res, res2 );
    // First cached
    winMC.invalidate();
    rfCorrelateWindow(winIC2, winMC, params, res2);
    rcUNITTEST_ASSERT( res == res2 );
    compareResults( useAltivec, res, res2 );
}


