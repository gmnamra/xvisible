/*
 *  ut_pmcorr.h
 *  correlation
 *	$Id: ut_pmcorr.h 4633 2006-09-13 04:34:19Z armanmg $   
 *  Copyright (c) 2001-2003 Reify Corporation. All rights reserved.
 *
 */

#ifndef _rcUT_CORR_H_
#define _rcUT_CORR_H_

#include <rc_unittest.h>
#include <rc_analysis.h>
#include <rc_ip.h>
#include <rc_window.h>

class UT_Correlation : public rcUnitTest {
public:

   UT_Correlation();
   ~UT_Correlation();

   virtual uint32 run();

private:
  void testOptoKinetic ( uint32 width, uint32 height, rcPixel d );
  void testCorrelation ( uint32 width, uint32 height, rcPixel d, bool useAltivec );
  void testTileCorrelation ( rcPixel d, uint32 tw, uint32 th );
  
  void testTimes ( bool useAltivec, int32 width, int32 height, rcPixel depth );
  void testokmap ();
  void testimgvar ();
  void testcorrfind ();
  void test1dcorr ();

  void testMaskedCorrelation();
  void testGenLatticeDimensions ();
  void testLatticeCorrelation ( bool useAltivec );
  void testLatticeCorrelationSpeed (rcUIPair winDim, rcIPair latticeDim,
				    bool partial);
  void testEnergyIntegrate ();
  void testMath( char* message );
      
  // Vary correlation calculation order and test differences
  void testOptoKineticPermutation( rcWindow& w1, rcWindow& w2, rcWindow& w3, uint32 seed );
  // Compare two results and display an error if they differ
  void compareResults( bool useAltivec, const rcCorr& r1, const rcCorr& r2 );
  // Test all cache variations
  void testCachedCorrelation( bool useAltivec, const rcWindow& wI, const rcWindow& wM,
			      const rsCorrParams& params, const rcCorr& res );
};

class UT_Connect : public rcUnitTest {
public:

   UT_Connect();
   ~UT_Connect();

   virtual uint32 run();

private:
   void testConnect ();
   void testPerformance ( int32 width );
   void testCircles ( int32 width );
   void testRLEs ( int32 width, int32 height, bool displayMessage = false );
   void testCorners ( int32 width, int32 height );
   void testEnclosures ( int32 width, int32 height, bool combineRegions );
};

#if 0
class UT_RLE : public rcUnitTest {
public:

   UT_RLE();
   ~UT_RLE();

   virtual uint32 run();

private:
   // Test runlength results
   void testResults ( const rcWindow& original, const rcRLEWindow& run,
                      uint32 expectedRunCount, uint32 maxPixelvalue );
   // Test one image
   void testImage ( const rcWindow& original, const rcRLEWindow& run, const rcWindow& projection,
                    uint32 expectedRunCount, uint32 maxPixelvalue, rcPixel expectedDepth );
   // Test one contour image
   void testContourImage ( const rcWindow& original, const rcRLEWindow& run, const rcWindow& contour,
                           uint32 maxPixelvalue, rcPixel expectedDepth );
   // Run all tests
   void testRLE (int32 width, int32 height, rcPixel, bool displayMessage = false );

   // Diagonal test pattern
   void testRLEDiagonal (int32 width, int32 height, rcPixel );
   
   // Checker board pattern: every other pixel has the same value
   void testRLECheckerBoard (int32 width, int32 height, rcPixel );

   // Corner test pattern: put a square cell in each corner
   void testRLECorner (int32 width, int32 height, rcPixel );

   // Hourglass test pattern
   void testRLEHourGlass (int32 width, int32 height, rcPixel, bool displayMessage = false );

   // Hourglass test pattern
   void testRLEFgBgHourGlass (int32 width, int32 height, rcPixel, bool displayMessage = false );

   // Cross test pattern
   void testRLECross (int32 width, int32 height, rcPixel, bool displayMessage = false );
   
   // Performance tests
   void testTimes ( int32 width, int32 height, rcPixel );

   void testGenPoly();
};

class UT_counter : public rcUnitTest {
public:

   UT_counter();
   ~UT_counter();

   virtual uint32 run();

private:
   void testrcCell ();
   void testcounter ();

};

#endif


#endif
