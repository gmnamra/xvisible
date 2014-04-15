/*
 *  File.
 *  
 *
 *	$Id: ut_rlewindow.cpp 4788 2006-11-30 23:40:42Z armanmg $
 *
 *  
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */


#include <rc_window.h>
#include <rc_vector2d.h>
#include <rc_time.h>
#include <rc_rlewindow.h>
#include <rc_polygon.h>

#include "ut_pmcorr.h"

// Vanilla image printing
#define cmPrintImage(a){                                    \
    for (int32 i = 0; i < (a).height(); i++)             \
    {                                                       \
        fprintf (stderr, "\n");                             \
        for (int32 j = 0; j < (a).width(); j++)          \
            fprintf (stderr, " %3d ", (a).getPixel (j, i)); \
        fprintf (stderr, "\n");                             \
    }}

#define cmHexPrintImage(a){                                \
    for (int32 i = 0; i < (a).height(); i++)             \
    {                                                       \
        for (int32 j = 0; j < (a).width(); j++)          \
            fprintf (stderr, " %2X ", (a).getPixel (j, i)); \
        fprintf (stderr, "\n");                             \
    }}

UT_RLE::UT_RLE()
{
}

UT_RLE::~UT_RLE()
{
    printSuccessMessage( "rcRLEWindow test", mErrors );
}

uint32
UT_RLE::run() {
    // Test different sizes and shapes
    {
	
		testGenPoly();
		
        testRLECross( 3, 3, rcPixel8 );
        testRLECross( 3, 3, rcPixel16 );
        testRLECross( 3, 3, rcPixel32S );
        testRLECross( 7, 7, rcPixel8 );
        testRLECross( 7, 7, rcPixel16 );
        testRLECross( 7, 7, rcPixel32S );
        testRLECross( 33, 33, rcPixel8 );
        testRLECross( 33, 33, rcPixel16 );
        testRLECross( 33, 33, rcPixel32S );
        testRLECross( 513, 513, rcPixel8 );
        testRLECross( 513, 513, rcPixel16 );
        testRLECross( 513, 513, rcPixel32S ); 
        testRLECross( 1025, 1025, rcPixel8 );
        testRLECross( 1025, 1025, rcPixel16 );
        testRLECross( 1025, 1025, rcPixel32S, true );  // Last test displays success message
        
        testRLEHourGlass( 3, 3, rcPixel8 );
        testRLEHourGlass( 3, 3, rcPixel16 );
        testRLEHourGlass( 3, 3, rcPixel32S );
        testRLEHourGlass( 7, 7, rcPixel8 );
        testRLEHourGlass( 7, 7, rcPixel16 );
        testRLEHourGlass( 7, 7, rcPixel32S );
        testRLEHourGlass( 33, 33, rcPixel8 );
        testRLEHourGlass( 33, 33, rcPixel16 );
        testRLEHourGlass( 33, 33, rcPixel32S );
        testRLEHourGlass( 513, 513, rcPixel8 );
        testRLEHourGlass( 513, 513, rcPixel16 );
        testRLEHourGlass( 513, 513, rcPixel32S );  
        testRLEHourGlass( 1025, 1025, rcPixel8 );
        testRLEHourGlass( 1025, 1025, rcPixel16 );
        testRLEHourGlass( 1025, 1025, rcPixel32S, true );  // Last test displays success message

	testRLEFgBgHourGlass( 3, 3, rcPixel8 );
        testRLEFgBgHourGlass( 3, 3, rcPixel16 );
        testRLEFgBgHourGlass( 3, 3, rcPixel32S );
        testRLEFgBgHourGlass( 7, 7, rcPixel8 );
        testRLEFgBgHourGlass( 7, 7, rcPixel16 );
        testRLEFgBgHourGlass( 7, 7, rcPixel32S );
        testRLEFgBgHourGlass( 33, 33, rcPixel8 );
        testRLEFgBgHourGlass( 33, 33, rcPixel16 );
        testRLEFgBgHourGlass( 33, 33, rcPixel32S );
        testRLEFgBgHourGlass( 513, 513, rcPixel8 );
        testRLEFgBgHourGlass( 513, 513, rcPixel16 );
        testRLEFgBgHourGlass( 513, 513, rcPixel32S );  
        testRLEFgBgHourGlass( 1025, 1025, rcPixel8 );
        testRLEFgBgHourGlass( 1025, 1025, rcPixel16 );
        testRLEFgBgHourGlass( 1025, 1025, rcPixel32S, true );  // Last test displays success message
        
        testRLE (3, 3, rcPixel8);
        testRLE (3, 3, rcPixel16);
        testRLE (3, 3, rcPixel32S);
        testRLE (4, 4, rcPixel8);
        testRLE (4, 4, rcPixel16);
        testRLE (4, 4, rcPixel32S);
        testRLE (5, 5, rcPixel8);
        testRLE (5, 5, rcPixel16);
        testRLE (5, 5, rcPixel32S);
        
        testRLE (64, 64, rcPixel8);
        testRLE (64, 64, rcPixel16);
        testRLE (64, 64, rcPixel32S);

        testRLE (512, 512, rcPixel8);
        testRLE (512, 512, rcPixel16);
        testRLE (512, 512, rcPixel32S);
        
        testRLE (1280, 960, rcPixel8, true); // Last test displays success message

#if defined (PERFORMANCE)
        testTimes ( 32, 32, rcPixel8 );
        testTimes ( 64, 64, rcPixel8 );
        testTimes ( 128, 128, rcPixel8 ); 
        testTimes ( 256, 256, rcPixel8 ); 
        testTimes ( 512, 512, rcPixel8 ); 
        testTimes ( 1024, 1024, rcPixel8 );
        testTimes ( 1280, 960, rcPixel8 ); 
        testTimes ( 2048, 2048, rcPixel8 );

        testTimes ( 32, 32, rcPixel32S );
        testTimes ( 64, 64, rcPixel32S );
        testTimes ( 128, 128, rcPixel32S ); 
        testTimes ( 256, 256, rcPixel32S ); 
        testTimes ( 512, 512, rcPixel32S ); 
        testTimes ( 1024, 1024, rcPixel32S );
        testTimes ( 1280, 960, rcPixel32S ); 
        testTimes ( 2048, 2048, rcPixel32S );
#endif
		
    }
    return mErrors;
}

//#define DEBUG_LOG 1

// Test runlength results
void UT_RLE::testResults ( const rcWindow& original, const rcRLEWindow& run,
                           uint32 expectedRunCount, uint32 maxPixelValue )
{
    // Expected bproj depth
    rcPixel expectedDepth = rcPixelUnknown;
    
    if ( maxPixelValue <= rcUINT8_MAX )
        expectedDepth = rcPixel8;
    else if ( maxPixelValue <= rcUINT16_MAX )
        expectedDepth = rcPixel16;
    else
        expectedDepth = rcPixel32S;
    
#ifdef DEBUG_LOG
    fprintf (stderr, "Original %d----------------\n", expectedDepth*8 );
    cmPrintImage (original);
#endif   
    // Test both image accessors
    rcWindow bproj = run.image();
    rcWindow bproj2;
    run.image( bproj2 );

    // Images should be identical
    rcUNITTEST_ASSERT( bproj.width() == bproj2.width() );
    rcUNITTEST_ASSERT( bproj.height() == bproj2.height() );
    rcUNITTEST_ASSERT( bproj.depth() == bproj2.depth() );
    for (int32 i = 0; i < bproj.width(); i++)
        for (int32 j = 0; j < bproj.height(); j++)
            rcUNITTEST_ASSERT (bproj.getPixel (i, j) == bproj2.getPixel (i, j));

    // Get contour image
    rcWindow contour;
    run.contourImage( contour );

    // Don't run this test for really large images, vectorization
    // creates a bitmap 4X original
    if ( run.width() <= 1024 && run.height() <= 1024 ) {
        // TODO: add verification
        rcVisualSegmentCollection segments;
        run.vectorize( segments );
    }
    
#ifdef DEBUG_LOG
    fprintf (stderr, "Projection %d----------------\n", expectedDepth*8 );
    cmPrintImage (bproj);
    fprintf (stderr, "Contour %d----------------\n", expectedDepth*8 );
    cmPrintImage (contour);
#endif

    testImage( original, run, bproj, expectedRunCount, maxPixelValue, expectedDepth );
    testContourImage( original, run, contour, maxPixelValue, expectedDepth );
}

// Test one image
void  UT_RLE::testImage ( const rcWindow& original, const rcRLEWindow& run, const rcWindow& projection,
                          uint32 expectedRunCount, uint32 maxPixelValue, rcPixel expectedDepth )
{
    rcUNITTEST_ASSERT (run.width() == original.width());
    rcUNITTEST_ASSERT (run.height() == original.height());
    rcUNITTEST_ASSERT (run.n () == expectedRunCount);
    if ( run.n() != expectedRunCount )
        cerr << "run.n() returned " << run.n() << ", expected " << expectedRunCount << endl;
    rcUNITTEST_ASSERT (projection.width() == original.width());
    rcUNITTEST_ASSERT (projection.height() == original.height());
    rcUNITTEST_ASSERT (projection.depth() ==  expectedDepth);

    // Test every pixel value
    for (int32 i = 0; i < original.width(); i++)
    {
        for (int32 j = 0; j < original.height(); j++)
        {
            const uint32 expectedPixel = original.getPixel (i, j);
            rcUNITTEST_ASSERT( expectedPixel <= maxPixelValue );
            rcUNITTEST_ASSERT (projection.getPixel (i, j) == expectedPixel);
            rcUNITTEST_ASSERT (run.getPel (i, j) == expectedPixel);
        }
    }
}

// Test one contour image
void  UT_RLE::testContourImage ( const rcWindow& original, const rcRLEWindow& run, const rcWindow& contour,
                                 uint32 maxPixelValue, rcPixel expectedDepth )
{
    uint32 oldErrors = mErrors;
    
    rcUNITTEST_ASSERT (contour.width() == run.width());
    rcUNITTEST_ASSERT (contour.height() == run.height());
    rcUNITTEST_ASSERT (contour.depth() ==  expectedDepth);

    // Test every pixel value
    for (int32 i = 0; i < original.width(); i++)
    {
        for (int32 j = 0; j < original.height(); j++)
        {
            const uint32 expectedPixel = original.getPixel (i, j);
            const uint32 contourPixel = contour.getPixel (i, j);
            rcUNITTEST_ASSERT (contourPixel <= expectedPixel);
            rcUNITTEST_ASSERT( contourPixel <= maxPixelValue );
        }
    }

    // Vectorization does not quite work yet
#ifdef notyet
    rcVisualSegmentCollection segments;
    run.vectorize( segments );
#endif
    
    if ( mErrors != oldErrors ) {
#ifdef DEBUG_LOG        
        fprintf (stderr, "Original %d----------------\n", expectedDepth*8 );
        cmPrintImage (original);
        fprintf (stderr, "Contour %d----------------\n", expectedDepth*8 );
        cmPrintImage (contour);
#endif
    }
}

// Checker board pattern: every other pixel has a non-zero value

// 010203
// 010203
// 010203

void UT_RLE::testRLECheckerBoard (int32 width, int32 height, rcPixel d)
{
    rcWindow ti (width, height, d);
    uint32 expectedCount = width * height;
    uint32 maxPixelvalue = 1;

    if ( d == rcPixel8 )
        maxPixelvalue = rcUINT8_MAX;
    else if ( d == rcPixel16 )
        maxPixelvalue = rcUINT16_MAX;
    else if ( d == rcPixel32S )
        maxPixelvalue = rcUINT16_MAX;
     
    // Set values
    for (int32 i = 0; i < ti.width(); i++) 
        for (int32 j = 0; j < ti.height(); j++) {
            uint32 pixelValue = ((i % 2) ? i/2 +1 : 0);
            if ( pixelValue > maxPixelvalue )
                pixelValue = maxPixelvalue;
            ti.setPixel (i, j, pixelValue);
        }
   
    rcRLEWindow rw;
    rw.encode (ti, maxPixelvalue);     

    testResults( ti, rw, expectedCount, maxPixelvalue );
}

// Put a cell in each corner, leave at least one pixel of space between cells
// Cell width = width/2 -1, cell height = height/2 -1

// 11022
// 11022
// 00000
// 33044
// 33044

void UT_RLE::testRLECorner (int32 width, int32 height, rcPixel d)
{
    uint32 dummy;
    rcWindow ti (width, height, d);
    int32 cellWidth = rfRound(0.5 + width/2.0 - 1, dummy);
    int32 cellHeight = rfRound(0.5 + height/2.0 - 1, dummy);
    uint32 expectedCount = 2 * cellHeight * 3 + ((height % 2) ? 1 : 2);
    uint32 maxPixelvalue = 4;
    
    rmAssert( cellWidth > 0 );
    rmAssert( cellHeight > 0 );

#ifdef DEBUG_LOG_CORNER
    cerr << "cellWidth " << cellWidth << " cellHeight " << cellHeight << " runs " << expectedCount << endl;
#endif
    
    // Clear image
    for (int32 i = 0; i < ti.width(); i++) 
        for (int32 j = 0; j < ti.height(); j++) 
            ti.setPixel (i, j, 0);
   
    for (int32 i = 0; i < ti.width(); i++) 
        for (int32 j = 0; j < ti.height(); j++) {
            // Create cell 1
            if ( i < cellWidth && j < cellHeight )
                ti.setPixel (i, j, 1);
            // Create cell 3
            if ( i < cellWidth && j >= (ti.height() - cellHeight) )
                ti.setPixel (i, j, 3);
        }

    for (int32 i = ti.width()-1; i >= 0; --i) 
        for (int32 j = 0; j < ti.height(); j++) {
            // Create cell 2
            if ( i >= int(ti.width()-cellWidth) && j < cellHeight )
                ti.setPixel (i, j, 2);
            // Create cell 4
            if ( i >= int(ti.width()-cellWidth) && j >= (ti.height() - cellHeight) )
                ti.setPixel (i, j, 4);
        }
    
    rcRLEWindow rw;
    rw.encode (ti, maxPixelvalue);     

    testResults( ti, rw, expectedCount, maxPixelvalue );
}

// Cross pattern

// 00100
// 00100
// 11111
// 00100
// 00100

void UT_RLE::testRLECross (int32 width, int32 height, rcPixel d, bool displayMessage )
{
    rcWindow ti (width, height, d);
    uint32 expectedCount = 1 + (height-1)*3;
    uint32 maxPixelvalue = 1;

    rmAssert( width > 2 );
    rmAssert( height > 2 );
    
    // Clear image
    for (int32 i = 0; i < width; i++) 
        for (int32 j = 0; j < height; j++) 
            ti.setPixel (i, j, 0);

    int32 mid = height/2;
    // Set all rows
    for (int32 j = 0; j < height; j++) {
        // Determine row start/end empty gap
        int32 gap;
        if ( j == mid )
            gap = 0;
        else
            gap = width/2;
        for (int32 i = 0; i < width; i++) {
            if ( i < gap )
                ti.setPixel( i, j, 0 );
            else if ( i > (width - gap - 1) )
                ti.setPixel( i, j, 0 );
            else
                ti.setPixel( i, j, 1 );
        }
    }
    
    rcRLEWindow rw;
    rw.encode (ti, maxPixelvalue);     

    testResults( ti, rw, expectedCount, maxPixelvalue );

    if ( displayMessage ) {
        char buf[512];
        snprintf( buf, rmDim(buf), "rcRLEWindow cross pattern test" );
        printSuccessMessage( buf , mErrors);
    }
}

// Test hourglass shape. Width and height must be odd numbers greater than 2.

// 11111
// 01110
// 00100
// 01110
// 11111

void UT_RLE::testRLEHourGlass (int32 width, int32 height, rcPixel d, bool displayMessage )
{
    rcWindow ti (width, height, d);
    uint32 expectedCount = 2 + (height-2)*3;
    uint32 maxPixelvalue = 1;

    rmAssert( width > 2 );
    rmAssert( height > 2 );
    
    // Clear image
    for (int32 i = 0; i < width; i++) 
        for (int32 j = 0; j < height; j++) 
            ti.setPixel (i, j, 0);

    int32 mid = height/2;
    // Set all rows
    for (int32 j = 0; j < height; j++) {
        // Determine row start/end empty gap
        int32 gap;
        if ( j > mid )
            gap = height - j - 1;
        else
            gap = j;
        for (int32 i = 0; i < width; i++) {
            if ( i < gap )
                ti.setPixel( i, j, 0 );
            else if ( i > (width - gap - 1) )
                ti.setPixel( i, j, 0 );
            else
                ti.setPixel( i, j, 1 );
        }
    }
    
    rcRLEWindow rw;
    rw.encode (ti, maxPixelvalue);     

    testResults( ti, rw, expectedCount, maxPixelvalue );

    if ( displayMessage ) {
        char buf[512];
        snprintf( buf, rmDim(buf), "rcRLEWindow hourglass pattern test" );
        printSuccessMessage( buf , mErrors);
    }
}

// Test foreground/background version of encode using hourglass
// shape. Width and height must be odd numbers greater than 2.

// 11111
// 01110
// 00100
// 01110
// 11111

void UT_RLE::testRLEFgBgHourGlass (int32 width, int32 height,
				   rcPixel d, bool displayMessage )
{
  rcWindow ti (width, height, d), er (width, height, d);
    uint32 expectedCount = 2 + (height-2)*3;
    uint32 maxPixelvalue = 1;
    const uint32 maxBgValue =
      (d == rcPixel8) ? rcUINT8_MAX : rcUINT16_MAX;

    rmAssert( width > 2 );
    rmAssert( height > 2 );
    
    // Clear image
    for (int32 i = 0; i < width; i++) 
        for (int32 j = 0; j < height; j++) 
            ti.setPixel (i, j, 0);

    srand( 46 );
    
    int32 mid = height/2;
    // Set all rows
    for (int32 j = 0; j < height; j++) {
        // Determine row start/end empty gap
        int32 gap;
        if ( j > mid )
            gap = height - j - 1;
        else
            gap = j;
        for (int32 i = 0; i < width; i++) {
	    uint32 val;
	    do {
	      val = (uint16)rand();
	    } while ((val == 1) || (val > maxBgValue));

            if ( i < gap ) {
                ti.setPixel( i, j, val );
                er.setPixel( i, j, 0 );
	    }
            else if ( i > (width - gap - 1) ) {
                ti.setPixel( i, j, val );
                er.setPixel( i, j, 0 );
	    }
            else {
                ti.setPixel( i, j, 1 );
                er.setPixel( i, j, 1 );
	    }
        }
    }
    
    rcRLEWindow rw;
    rw.encode (ti, maxPixelvalue, 1);     

    testResults( er, rw, expectedCount, maxPixelvalue );

    if ( displayMessage ) {
        char buf[512];
        snprintf( buf, rmDim(buf), "rcRLEWindow BgFg hourglass pattern test" );
        printSuccessMessage( buf , mErrors);
    }
}

// Diagonal test pattern

// 100
// 020
// 003

void UT_RLE::testRLEDiagonal(int32 width, int32 height, rcPixel d)
{
    uint32 maxPixelValue = (width > height) ? width : height;
    uint32 expectedCount =  ((height - 2) * 3) + 2*2; // 2 for the top and bottom, 3 for every other row
    // Expected bproj depth
    rcPixel expectedDepth = rcPixelUnknown;
    
    if ( maxPixelValue <= rcUINT8_MAX )
        expectedDepth = rcPixel8;
    else if ( maxPixelValue <= rcUINT16_MAX )
        expectedDepth = rcPixel16;
    else
        expectedDepth = rcPixel32S;

    if ( width == height && expectedDepth <= d ) {
        rcWindow ti (width, height, d);
        
        // Create a diagonal pattern
        for (int32 i = 0; i < ti.width(); i++)
            for (int32 j = 0; j < ti.height(); j++)
                ti.setPixel (i, j, (i==j) ? i+1 : 0);
        
        rcRLEWindow rw;
        rw.encode (ti, maxPixelValue);     
        
        testResults( ti, rw, expectedCount, maxPixelValue );
    }
}

void UT_RLE::testRLE(int32 width, int32 height, rcPixel d, bool displayMessage)
{
    // Diagonal test pattern
    testRLEDiagonal (width, height, d);
    // 4 cells, one in each corner
    testRLECorner (width, height, d);
    // A checker board pattern (worst case for run count)
    testRLECheckerBoard(width, height, d);

    if ( displayMessage ) {
        char buf[512];
        snprintf( buf, rmDim(buf), "rcRLEWindow diagonal/corner/checkerboard patterns test" );
        printSuccessMessage( buf , mErrors);
    }
}


void UT_RLE::testTimes( int32 width, int32 height, rcPixel d )
{
    const uint32 minPixels = 1024 * 512;
    const uint32 pixels = (width * height)/d + 1;
    // Adjust repeats depending on image size
    uint32 repeats = minPixels/pixels;
    uint32 randomSeed = 1; // use a repetable seed
    
    if ( repeats < 1 )
        repeats = 1;
    
    rcWindow imgA (width, height, d);

    if ( d > rcPixel8 ) {
        srandom( randomSeed );
        // Limit pixel values to avoid run overflows
        for (int32 i = 0; i < imgA.width(); i++)
            for (int32 j = 0; j < imgA.height(); j++)
                imgA.setPixel (i, j, uint32 (uint8 (random ())));
    } else {
        imgA.randomFill ( randomSeed );
    }
                
    rcTime timer;
    // Run test in a loop to diminish the effect of system clock
    // inaccuracy. The time spent by the loop code should be negligible

    timer.start ();
    for( uint32 i = 0; i < repeats; ++i )
    {
        rcRLEWindow rw;
        rw.encode (imgA);
    }
    timer.end ();

    double dMilliSeconds = timer.milliseconds () / repeats;
    double dMicroSeconds = timer.microseconds () / repeats;
 
    // Per Byte in Useconds
    double perByte = dMicroSeconds / (imgA.width() * imgA.height() * imgA.depth());

    fprintf(stderr,
            "Performance: rcRLEWindow::encode()   [%i x %i x %i]: %.2f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n", 
            imgA.width(), imgA.height(), imgA.depth()*8, dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);
}

#define POLYTST_CNT 24
#define POLYTST_IW  16
#define POLYTST_IH  16

static uint8 rleToPolyTstImg[POLYTST_CNT][POLYTST_IH][POLYTST_IW] = 
  {
    { // 0
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 1
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 2
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 3
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 4
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 5
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 6
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 7
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 8
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 9
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 10
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 11
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 12
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 13
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 14
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
    },
    { // 15
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 16
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 17
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 18
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 19
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 20
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 21
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 22
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    { // 23
      { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
      { 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
      { 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
      { 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
      { 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
      { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    }
  };

static uint8 rleToPolyBugImg[1][39][37] = 
  {
	{
	{  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0},
	{  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0},
	{  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  0,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  2,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  2,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  2,  2,  2,  2,  2},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  0,  0,  0,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  0,  0,  2,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14, 14, 14, 14, 14,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14, 14, 14, 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14, 14, 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0},
	{ 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0},
	{ 16,  0,  0,  0,  0,  0,  0,  0,  0,  0, 16, 16, 16, 16, 16, 16,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0},
	{ 16, 16, 16,  0,  0,  0,  0, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0},
	{ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,  0,  0,  0,  0,  0,  0,  0,  0,  2,  2,  2,  2,  2,  0,  0,  0,  0,  0,  0,  0}
	}
  };

static rc2Dvector rleToPolyRes0[] = 
  { 
    rc2Dvector(5,2),  rc2Dvector(5,3), rc2Dvector(6,3), rc2Dvector(6,4), 
    rc2Dvector(7,4), rc2Dvector(7,6), rc2Dvector(6,6), rc2Dvector(6,7), 
    rc2Dvector(5,7), rc2Dvector(5,8), rc2Dvector(3,8), rc2Dvector(3,7), 
    rc2Dvector(2,7), rc2Dvector(2,6), rc2Dvector(1,6), rc2Dvector(1,4), 
    rc2Dvector(2,4), rc2Dvector(2,3), rc2Dvector(3,3), rc2Dvector(3,2), 
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes1[] = 
  {
    rc2Dvector(5,2), rc2Dvector(5,3), rc2Dvector(6,3), rc2Dvector(6,4),
    rc2Dvector(7,4), rc2Dvector(7,6), rc2Dvector(8,7), rc2Dvector(10,7),
    rc2Dvector(10,8), rc2Dvector(11,8), rc2Dvector(11,9), rc2Dvector(12,9),
    rc2Dvector(12,11), rc2Dvector(11,11), rc2Dvector(11,12), rc2Dvector(10,12),
    rc2Dvector(10,13), rc2Dvector(8,13), rc2Dvector(8,12), rc2Dvector(7,12),
    rc2Dvector(7,11), rc2Dvector(6,11), rc2Dvector(6,9), rc2Dvector(5,8),
    rc2Dvector(3,8), rc2Dvector(3,7), rc2Dvector(2,7), rc2Dvector(2,6),
    rc2Dvector(1,6), rc2Dvector(1,4), rc2Dvector(2,4), rc2Dvector(2,3),
    rc2Dvector(3,3), rc2Dvector(3,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes2[] = 
  { 
    rc2Dvector(6,1), rc2Dvector(6,3), rc2Dvector(7,3), rc2Dvector(7,4),
    rc2Dvector(9,4), rc2Dvector(9,6), rc2Dvector(7,6),rc2Dvector(7,8),
    rc2Dvector(6,8), rc2Dvector(6,10), rc2Dvector(4,10), rc2Dvector(4,8),
    rc2Dvector(3,8), rc2Dvector(3,7), rc2Dvector(1,7), rc2Dvector(1,4),
    rc2Dvector(3,4), rc2Dvector(3,3), rc2Dvector(4,3), rc2Dvector(4,1),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes3[] = 
  { rc2Dvector(7,2), rc2Dvector(7,8), rc2Dvector(1,8), rc2Dvector(1,2), 
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes4[] = 
  {
    rc2Dvector(5,2), rc2Dvector(5,4), rc2Dvector(7,4), rc2Dvector(7,6),
    rc2Dvector(5,6), rc2Dvector(5,8), rc2Dvector(3,8), rc2Dvector(3,6),
    rc2Dvector(1,6), rc2Dvector(1,4), rc2Dvector(3,4), rc2Dvector(3,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes5[] = 
  { 
    rc2Dvector(3,2), rc2Dvector(3,3), rc2Dvector(5,3), rc2Dvector(5,4),
    rc2Dvector(6,4), rc2Dvector(6,6), rc2Dvector(4,6), rc2Dvector(4,4),
    rc2Dvector(1,4), rc2Dvector(1,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes6[] = 
  {     
    rc2Dvector(3,2), rc2Dvector(3,3), rc2Dvector(4,3), rc2Dvector(4,5),
    rc2Dvector(5,5), rc2Dvector(5,7), rc2Dvector(3,7), rc2Dvector(3,4),
    rc2Dvector(1,4), rc2Dvector(1,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes7[] = 
  {
    rc2Dvector(6,2), rc2Dvector(6,4), rc2Dvector(5,4), rc2Dvector(5,5),
    rc2Dvector(3,5), rc2Dvector(3,6), rc2Dvector(1,6), rc2Dvector(1,4),
    rc2Dvector(4,4), rc2Dvector(4,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes8[] = 
  {
    rc2Dvector(5,2), rc2Dvector(5,4), rc2Dvector(4,4), rc2Dvector(4,6),
    rc2Dvector(3,6), rc2Dvector(3,7), rc2Dvector(1,7), rc2Dvector(1,5),
    rc2Dvector(3,5), rc2Dvector(3,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes9[] = 
  { 
    rc2Dvector(3,2), rc2Dvector(3,3), rc2Dvector(4,4), rc2Dvector(5,4),
    rc2Dvector(5,5), rc2Dvector(6,5), rc2Dvector(6,7), rc2Dvector(4,7),
    rc2Dvector(4,5), rc2Dvector(3,4), rc2Dvector(1,4), rc2Dvector(1,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes10[] = 
  {
    rc2Dvector(6,2), rc2Dvector(6,4), rc2Dvector(5,4), rc2Dvector(4,5),
    rc2Dvector(4,6), rc2Dvector(3,6), rc2Dvector(3,7), rc2Dvector(1,7),
    rc2Dvector(1,5), rc2Dvector(3,5), rc2Dvector(4,4), rc2Dvector(4,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes11[] = 
  { rc2Dvector(2,1), rc2Dvector(1,1),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes12[] = 
  { rc2Dvector(3,6),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes13[] = 
  { rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes14[] = 
  { rc2Dvector(15,0), rc2Dvector(15,15), rc2Dvector(0,15), rc2Dvector(0,0),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes15[] = 
  {
    rc2Dvector(4,2), rc2Dvector(4,4), rc2Dvector(6,4), rc2Dvector(6,5),
    rc2Dvector(4,5), rc2Dvector(4,7), rc2Dvector(3,7), rc2Dvector(3,5),
    rc2Dvector(1,5), rc2Dvector(1,4), rc2Dvector(3,4), rc2Dvector(3,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes16[] = 
  {
    rc2Dvector(4,2), rc2Dvector(4,4), rc2Dvector(6,4), rc2Dvector(6,5),
    rc2Dvector(5,5), rc2Dvector(5,7), rc2Dvector(4,7), rc2Dvector(4,5),
    rc2Dvector(1,5), rc2Dvector(1,4), rc2Dvector(3,4), rc2Dvector(3,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes17[] = 
  {
    rc2Dvector(3,2), rc2Dvector(3,3), rc2Dvector(4,3), rc2Dvector(4,5),
    rc2Dvector(5,5), rc2Dvector(5,6), rc2Dvector(4,6), rc2Dvector(4,8),
    rc2Dvector(3,8), rc2Dvector(3,9), rc2Dvector(1,9), rc2Dvector(1,8),
    rc2Dvector(2,8), rc2Dvector(2,7), rc2Dvector(3,7), rc2Dvector(3,4),
    rc2Dvector(2,4), rc2Dvector(2,3), rc2Dvector(1,3), rc2Dvector(1,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes18[] = 
  {
    rc2Dvector(5,2), rc2Dvector(5,4), rc2Dvector(4,4), rc2Dvector(4,7),
    rc2Dvector(5,7), rc2Dvector(5,9), rc2Dvector(4,9), rc2Dvector(4,8),
    rc2Dvector(3,8), rc2Dvector(3,7), rc2Dvector(2,7), rc2Dvector(2,6),
    rc2Dvector(1,6), rc2Dvector(1,5), rc2Dvector(2,5), rc2Dvector(2,4),
    rc2Dvector(3,4), rc2Dvector(3,3), rc2Dvector(4,3), rc2Dvector(4,2), 
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes19[] = 
  {
    rc2Dvector(5,2), rc2Dvector(5,4), rc2Dvector(7,4), rc2Dvector(7,5),
    rc2Dvector(6,5), rc2Dvector(6,6), rc2Dvector(5,6), rc2Dvector(5,8),
    rc2Dvector(4,8), rc2Dvector(4,7), rc2Dvector(3,7), rc2Dvector(3,6),
    rc2Dvector(1,6), rc2Dvector(1,5), rc2Dvector(2,5), rc2Dvector(2,4),
    rc2Dvector(3,4), rc2Dvector(3,3), rc2Dvector(4,3), rc2Dvector(4,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes20[] = 
  {
    rc2Dvector(2,1), rc2Dvector(3,2), rc2Dvector(4,2), rc2Dvector(4,4),
    rc2Dvector(3, 4), rc2Dvector(3,3), rc2Dvector(2,2), rc2Dvector(1,2),
    rc2Dvector(1,1),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes21[] = 
  {
    rc2Dvector(4,2), rc2Dvector(4,5), rc2Dvector(3,5), rc2Dvector(3,3),
    rc2Dvector(2,3), rc2Dvector(2,2),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes22[] = 
  {
    rc2Dvector(5,3), rc2Dvector(5,5), rc2Dvector(4,5), rc2Dvector(4,4),
    rc2Dvector(2,4), rc2Dvector(2,3),
    rc2Dvector(-1,-1)
  };

static rc2Dvector rleToPolyRes23[] = 
  { 
    rc2Dvector(8,0), rc2Dvector(8,4), rc2Dvector(9,4), rc2Dvector(9,6),
    rc2Dvector(8,6), rc2Dvector(8,8), rc2Dvector(7,8), rc2Dvector(7,9),
    rc2Dvector(5,9), rc2Dvector(5,10), rc2Dvector(2,10), rc2Dvector(2,9),
    rc2Dvector(1,9), rc2Dvector(1,8), rc2Dvector(0,8), rc2Dvector(0,4),
    rc2Dvector(2,4), rc2Dvector(2,8), rc2Dvector(4,8), rc2Dvector(4,7),
    rc2Dvector(5,7), rc2Dvector(5,1), rc2Dvector(6,1), rc2Dvector(6,0),
    rc2Dvector(-1, -1)
  };

static rc2Dvector* rleToPolyTstRslt[POLYTST_CNT] = 
  {
    rleToPolyRes0, rleToPolyRes1, rleToPolyRes2, rleToPolyRes3,
    rleToPolyRes4, rleToPolyRes5, rleToPolyRes6, rleToPolyRes7,
    rleToPolyRes8, rleToPolyRes9, rleToPolyRes10, rleToPolyRes11,
    rleToPolyRes12, rleToPolyRes13, rleToPolyRes14, rleToPolyRes15,
    rleToPolyRes16, rleToPolyRes17, rleToPolyRes18, rleToPolyRes19,
    rleToPolyRes20, rleToPolyRes21, rleToPolyRes22, rleToPolyRes23
  };

void UT_RLE::testGenPoly()
{
  rcWindow test(POLYTST_IW, POLYTST_IH);
  
  const int32 fgValCnt = 5;
  uint16 fgVal[fgValCnt] = { 0x0001, 0xFE, 0xFF, 0x33, 0x22 };

  for (int32 fi = 0; fi < fgValCnt; fi++) {
    for (int32 ti = 0; ti < POLYTST_CNT; ti++) {
      for (int32 y = 0; y < test.height(); y++)
	for (int32 x = 0; x < test.width(); x++) {
	  if (rleToPolyTstImg[ti][y][x])
	    test.setPixel(x, y, fgVal[fi]);
	  else
	    test.setPixel(x, y, 0);
	}

      rcPolygon exp;

      rc2Dvector* pp = rleToPolyTstRslt[ti];
      while (*pp != rc2Dvector(-1,-1))
	exp.pushVertex(*pp++);

      rcPolygon p;
      rcRLEWindow rle;
      rle.encode(test, 1);
      rle.polygon(p);

      rcUNITTEST_ASSERT(exp == p);
      rcUNITTEST_ASSERT(p.isValid());
    }


  }


#if 0
    /* This is a great test in the tradition of montoCarlo testing.
     * Currently it produces failures in some case when the generated
     * polygon includes only portion of the random part and therefore
     * is in correct. Current guess is that the bug is in conversion of
     * RLE to polygon. A better and faster way to do that is to perform
     * it on a Label-Only run. This is a TBD. 
     */

#ifdef DYNAMIC_DEBUG
  const uint32 lastErrors = mErrors;
#endif

  srand(666);

  const uint32 lthBits = 3;
  const uint32 randLth = 1 << lthBits;
  const uint32 lthMask = randLth - 1;
  const uint32 maxPts = 1 << (lthBits*2);
  const uint32 maxPtsMask = maxPts - 1;
  const uint32 blkLth = 20;
  const double minArea = (blkLth - 1)*(blkLth - 1);
  const uint32 testSz = randLth + blkLth;
  rcWindow testRandom(randLth, randLth);
  rcWindow testArea(testSz+2, testSz+2);
  testArea.setAllPixels(0);
  rcWindow testAreaWin(testArea, 1, 1, testSz, testSz);

  const uint32 testCount = 10000;
  int64 testsDone = 0;
  for (uint32 ti = 0; ti < testCount; ti++) {
    /* Step 1 - Generate the random part of the test image. Do this by
     * creating an image with a random number of pixels set, using
     * connectivity to find the connected groups of pixels.
     */
    testRandom.setAllPixels(0);
    
    uint32 pts = 0;
    while (pts < 8)
      pts = (uint32)((rand() >> 4) & maxPtsMask);

    for (uint32 pi = 0; pi < pts; pi++) {
      int32 rval = rand() >> 4;
      int32 x = rval & lthMask;
      rval = rval >> lthBits;
      int32 y = rval & lthMask;
      testRandom.setPixel(x, y, 1);
    }

    if (0) { printf("\npts %d\n", pts); cmHexPrintImage(testRandom); }

    rcWindow connectedImg;
    uint32 connectedCnt;
    rfGetComponents(testRandom, connectedCnt, connectedImg);
    rmAssert(connectedCnt);
    
    vector<rcRLEWindow> blob;
    rfGetRLEs(connectedImg, connectedCnt, rcIPair(0, 0), false, blob);
    rmAssert(blob.size() == connectedCnt);

    /* Step 2 - Take each connected region and use it to do one test.
     * For each test, do the following:
     *
     *  a) Put a copy of the blob into testArea.
     *
     *  b) On the far righthand side of the blob, find the lower most
     *     pixel.
     *
     *  c) At the point discovered in b), place a large square
     *     connected square.
     *
     *  d) Use the newly created shape to form an rcRLEWindow. Create
     *     a rcPolygon from this, and verify that its location and
     *     area meet pass criterion.
     */
    for (uint32 bi = 0; bi < blob.size(); bi++) {
      uint32 setn = blob[bi].setN();
      if (setn == 1) // Ignore trivial case
	continue;
      else if ((setn < 5) && ((rand() & 0xC0) < 0x80)) // Limit small cases used
	continue;

      testAreaWin.setAllPixels(0);
      rcWindow isoBlob;
      blob[bi].image(isoBlob);
      rcIRect rect = blob[bi].rectangle();
      rcWindow randWin(testAreaWin, 0, 0, rect.width(), rect.height());
      rmAssert(testAreaWin.contains(randWin));
      randWin.copyPixelsFromWindow(isoBlob);
      
      if (0) { printf("\nRegion %d Random\n", pts); cmHexPrintImage(testArea); }

      const int32 xOff = rect.width() - 1;
      int32 yOff;
      for (yOff = rect.height() - 1; yOff >= 0; yOff--)
	if (randWin.getPixel(xOff, yOff))
	  break;
      rmAssert(yOff >= 0);
      const uint8 pixValue = (uint8)randWin.getPixel(xOff, yOff);
      rmAssert(pixValue);

      rcIRect sqLoc(xOff+2, yOff+2, blkLth, blkLth);
      rcWindow blkWin(testAreaWin, sqLoc);
      rmAssert(testAreaWin.contains(blkWin));
      blkWin.setAllPixels(pixValue);

      rcRLEWindow tRLE;
      tRLE.encode(testArea, pixValue, pixValue);

      rcPolygon poly;
      tRLE.polygon(poly);

      double area = poly.area();
      rcIRect ortho = poly.orthogonalEnclosingRect();

      rcUNITTEST_ASSERT(area > minArea);
      rcUNITTEST_ASSERT((ortho.origin().x() == 0)||(ortho.origin().x() == 1));
      rcUNITTEST_ASSERT((ortho.origin().y() == 0)||(ortho.origin().y() == 1));
      rcUNITTEST_ASSERT(ortho.lr().x() == (int32)(xOff + 1 + blkLth));
      rcUNITTEST_ASSERT(ortho.lr().y() == (int32)(yOff + 1 + blkLth));

#ifdef DYNAMIC_DEBUG
      if (lastErrors != mErrors) {
	printf("\nRegion %d lr: %d %d\n", pts, xOff, yOff);
	cmHexPrintImage(testArea);
	cout << endl << poly << endl << "area: " << area
	     << " oer: " << ortho << endl;

	extern int __polygonDebug;
	__polygonDebug = 1;
	rcPolygon perr;
	tRLE.polygon(perr);
	rmAssert(0);
      }
#endif

      testsDone++;
      if ((testsDone & (int64)0x3FFFF) == 0)
	cout << "Tests Done: " << testsDone << " Iterations: " << ti << endl;
    }
  }
#endif

  {
    char buf[512];
    snprintf( buf, rmDim(buf), "rcRLEWindow polygon generation test" );
    printSuccessMessage( buf , mErrors);
  }
}
