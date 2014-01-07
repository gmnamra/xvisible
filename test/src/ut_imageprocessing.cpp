// Copyright 2002 Reify, Inc.

#include "ut_imageprocessing.h"
#include <stdlib.h>
#include <rc_window.h>
#include <time.h>
#include <rc_ipconvert.h>

void rcCverGenHalfRes(char* src, char* dest, const uint32 widthInPixels,
		      const uint32 heightInLines, const uint32 rowUpdate,
		      const bool invert);
#ifdef __ppc__
void rcAltivecGenHalfRes(char* src, char* dest, const uint32 widthInPixels,
			 const uint32 heightInLines, const uint32 rowUpdate,
			 const bool invert);
#endif

//
// Local utilities
//

static void init_buffer(char* b, int pixelCnt)
{
  unsigned int* p = (unsigned int*)b;

  for (int i = 0; i < pixelCnt/4; i++)
    *p++ = rand();
}

static void dump_buffer(char* msg, char* b, int pixelCnt)
{
  printf("Dumping %s buffer:\n", msg);

  int blkCnt = pixelCnt/32;
  unsigned char* ub = (unsigned char*)b;

  for (int i = 0; i < blkCnt; i++)
  {
    printf("Blk %d:", i);
    for (int j = 0; j < 32; j++)
      printf(" %02X", *ub++);
    printf("\n");
  }

  printf("\n");
}

//
// Unit test implementation
//

UT_ImageProcessing::UT_ImageProcessing()
{
}

UT_ImageProcessing::~UT_ImageProcessing()
{
    printSuccessMessage( "Image Processing test", mErrors );
}


uint32
UT_ImageProcessing::run() {

//   rcWindow dst (3, 3);
//   vector<rcWindow> src (3);
//   for (uint32 i = 0; i < 3; i++) src[i] = rcWindow (3, 3);
//   uint8 c (0);
//   for (uint32 i = 0; i < 3; i++)
//     {
//       for (uint32 ii = 0; ii < 3; ii++)
// 	for (uint32 jj = 0; jj < 3; jj++)
// 	  {
// 	    src[i].setPixel (ii, jj, c++);
// 	  }
//     }

//   dst.setAllPixels (0);

//   for (uint32 ii = 0; ii < 3; ii++)
//     cerr << src[ii] << endl;

//   rfTemporalMedian (src, dst);
  
//   cerr << dst;
//   dst.setAllPixels (0);

//   vector<rcWindow> src2 (3);
//   src2[0] = src[0];
//   src2[1] = src[2];
//   src2[2] = src[1];


//   rfTemporalMedian (src2, dst);
//   for (uint32 ii = 0; ii < 3; ii++)
//     cerr << src2[ii] << endl;
  
//   cerr << dst;
//   dst.setAllPixels (0);

//   vector<rcWindow> src3 (3);
//   src3[0] = src[1];
//   src3[1] = src[0];
//   src3[2] = src[2];

//   rfTemporalMedian (src3, dst);

//   for (uint32 ii = 0; ii < 3; ii++)
//     cerr << src3[ii] << endl;
  
//   cerr << dst;
//   dst.setAllPixels (0);

  testConvertPixels16();
  testAndImage();

  // Test 8-bit pixel reversal
  testReversePixels8( 1, 1 );
  testReversePixels8( 3, 3 );
  testReversePixels8( 64, 64 );
  testReversePixels8( 255, 1 );
  testReversePixels8( 1, 255 );
  testReversePixels8( 255, 17 );
  testReversePixels8( 17, 255 );
  testReversePixels8( 255, 255 );
 
  // Test color bitmap reduction
  testRfRcWindow32to8( false );
  // Test gray scale bitmap reduction
  testRfRcWindow32to8( true );
   // Test color bitmap promotion
  testRfRcWindow8to32( false );
  // Test gray scale bitmap promotion
  testRfRcWindow8to32( true );
   // Test half resolution image generation
  testGenHalfRes();
  
  return mErrors;
}

void UT_ImageProcessing::testAndImage()
{
  uint32 depth[3] = { 1, 2, 4 };
  uint32 allones[3] = { 0xFF, 0xFFFF, 0xFFFFFFFF };

  for (uint32 i = 0; i < 3; i++) {
    for (int32 sz = 1; sz < 18; sz++) {
      rcWindow srcWin(16*sz, 16*sz, (rcPixel)depth[i]);
      rcWindow destWin(16*sz, 16*sz, (rcPixel)depth[i]);
      rcWindow maskWin(16*sz, 16*sz, (rcPixel)depth[i]);
      
      for (int32 y = 0; y < srcWin.height(); y++)
	for (int32 x = 0; x < srcWin.width(); x++) {
	  uint32 val = (x+y) & 0xFF;
	  uint32 val32 = (val << 24) | (val << 16) | (val << 8) | val;
	  srcWin.setPixel(x, y, (val32 & allones[i]));

	  if ((y&1) ^ (x&1))
	    maskWin.setPixel(x, y, allones[i]);
	  else
	    maskWin.setPixel(x, y, 0);
	}

      rfAndImage(srcWin, maskWin, destWin);

      for (int32 y = 0; y < srcWin.height(); y++)
	for (int32 x = 0; x < srcWin.width(); x++) {
	  if ((y&1) ^ (x&1))
	    rcUNITTEST_ASSERT(destWin.getPixel(x, y) ==
			       srcWin.getPixel(x, y));
	  else
	    rcUNITTEST_ASSERT(destWin.getPixel(x, y) == 0);
	}

      rcWindow srcWin2(srcWin, 2, 0, srcWin.width()-2, srcWin.height());
      rcWindow destWin2(destWin, 2, 0, destWin.width()-2, destWin.height());
      rcWindow maskWin2(maskWin, 2, 0, maskWin.width()-2, maskWin.height());
      destWin2.setAllPixels(0);

      rfAndImage(srcWin2, maskWin2, destWin2);

      for (int32 y = 0; y < srcWin2.height(); y++)
	for (int32 x = 0; x < srcWin2.width(); x++) {
	  if ((y&1) ^ (x&1))
	    rcUNITTEST_ASSERT(destWin2.getPixel(x, y) ==
			       srcWin2.getPixel(x, y));
	  else
	    rcUNITTEST_ASSERT(destWin2.getPixel(x, y) == 0);
	}
    }
  }
}

// Test half resolution image generation

uint32
UT_ImageProcessing::testGenHalfRes()
{
  srand(0);

  char* actP = (char*)malloc(320*240 + 16); // make this one big enough for timing test
  char* expP = (char*)malloc(4096 + 16);

  if (!actP || !expP)
  {
    if (actP) { free(actP); actP = 0; }
    
    rcUNITTEST_ASSERT(0);
    return mErrors;
  }

  char* actDP = actP + (16 - ((int)actP & 0xF));
  char* expDP = expP + (16 - ((int)expP & 0xF));

  for (unsigned int cnt = 1; cnt < 0x10000; cnt++)
  {
    int blkCnt;
    do
    {
      blkCnt = rand() & 0x1F;
    } while ((blkCnt < 1) || (blkCnt > 23));
    
    int lineCnt = ((rand() & 0x3) + 1) * 2;

    int imgSzInPixels = blkCnt*32*lineCnt;

    /* Now 0 < blkCnt < 24 and lineCnt in the set {2, 4, 6, 8}. Use these
     * values to create a pair of 8 bit images to work within.
     */
    rcWindow src(blkCnt*32, lineCnt);

    char* srcP = (char*)(src.frameBuf()->alignedRawData());
    if (((int)srcP & 0xF) != 0)
    {
      printf("srcP 0x%X\n", (unsigned int)srcP);
      rcUNITTEST_ASSERT(0);
      break;
    }
    if (src.frameBuf()->rowUpdate() != (blkCnt*32))
    {
      printf("row update 0x%X img width 0x%X\n", src.frameBuf()->rowUpdate(),
	     (unsigned int)(blkCnt*32));
      rcUNITTEST_ASSERT(0);
      break;
    }

    init_buffer(srcP, imgSzInPixels);

    rcCverGenHalfRes(srcP, actDP, blkCnt*32, lineCnt, blkCnt*32, 0);
#ifdef __ppc__
	// C version is all that is availble for x86 for now
    rcAltivecGenHalfRes(srcP, expDP, blkCnt*32, lineCnt, blkCnt*32, 0);

    if (strncmp(actDP, expDP, imgSzInPixels/4) != 0)
    {
      printf("Failure iteration %d. Buffer lth %d.\n", cnt, imgSzInPixels/4);
      printf("Src width %d height %d\n", blkCnt*32, lineCnt);

      dump_buffer("src img", srcP, imgSzInPixels);
      dump_buffer("act result", actDP, imgSzInPixels/4);
      dump_buffer("exp result", expDP, imgSzInPixels/4);
      if (actP) {free(actP); actP = 0; }
      if (expP) {free(expP); expP = 0; }
      rcUNITTEST_ASSERT(0);
      break;
    }
#endif

  }

#if defined (PERFORMANCE)
	
  /* Generate timing info */
  if (!mErrors)
  {
    const int iterateCount = 100;

    rcWindow src(640, 480);

    char* srcP = (char*)(src.frameBuf()->alignedRawData());

    if (((int)srcP & 0xF) != 0)
    {
      printf("srcP 0x%X\n", (unsigned int)srcP);
      rcUNITTEST_ASSERT(0);
      goto done;
    }
    if (src.frameBuf()->rowUpdate() != 640)
    {
      printf("row update 0x%X img width 0x%X\n", src.frameBuf()->rowUpdate(), 640);
      rcUNITTEST_ASSERT(0);
      goto done;
    }

    clock_t startTimeC = clock();
    for (int i = 0; i < iterateCount; i++)
      rcCverGenHalfRes(srcP, actDP, 640, 480, 640, 0);
    clock_t endTimeC = clock();


    clock_t startTimeA = clock();
#ifdef __ppc__
    for (int i = 0; i < iterateCount; i++)
      rcAltivecGenHalfRes(srcP, actDP, 640, 480, 640, 0);
#endif
    clock_t endTimeA = clock();


    double totC = (double)(endTimeC - startTimeC);
    double totA = (double)(endTimeA - startTimeA);
    double msC = (totC*1000)/(CLOCKS_PER_SEC*iterateCount);
    double msA = (totA*1000)/(CLOCKS_PER_SEC*iterateCount);
    
    printf("Performance: Gen 1/2 res inv 640x480, C version: %3.3f ms, %3.3f MB/s, %.2f fps\n",
           msC,
           (src.height() * src.width())/(1000*msC),
           1000/msC );

    printf("Performance: Gen 1/2 res inv 640x480, Altivec version: %3.3f ms, %3.3f MB/s, %.2f fps\n",
           msA,
           (src.height() * src.width())/(1000*msA),
           1000/msA );
  }
	
#endif // Performace
 done:
  if (actP) {free(actP); actP = 0; }
  if (expP) {free(expP); expP = 0; }
    
  return mErrors;
}

// 8-bit pixel reversal
uint32
UT_ImageProcessing::testReversePixels8( int32 width, int32 height )
{
    rcUNITTEST_ASSERT( width < 256 );
    
    rcWindow image( width, height );

    // Test 0 and 255
    image.setAllPixels( 255 );
    testPixels( image, 255 );
    rfReversePixels8( image );
    testPixels( image, 0 );
    rfReversePixels8( image );
    testPixels( image, 255 );

    // Test various pixel values
    for ( int32 y = 0; y < image.height(); ++y ) 
        for ( int32 x = 0; x < image.width(); ++x )
            image.setPixel( x, y, x );
    rfReversePixels8( image );
    for ( int32 y = 0; y < image.height(); ++y ) 
        for ( int32 x = 0; x < image.width(); ++x )
            rcUNITTEST_ASSERT(image.getPixel( x, y ) == uint32(255-x));
    
    return mErrors;
}


// Reset image pixel values
uint32
UT_ImageProcessing::resetRfRcWindow32to8( rcWindow& img32, rcWindow& img8 )
{
    // Set 32-bit pixel values
    for ( int32 row = 0; row < img32.height(); row++ ) {
        for ( int32 column = 0; column < img32.width(); column++ ) {
            uint32 colorComp = (row*column) % 256;
            // Create a shade of gray
            uint32 color = rfRgb( colorComp, colorComp, colorComp );
            rcUNITTEST_ASSERT( rfRed( color ) == colorComp );
            rcUNITTEST_ASSERT( rfGreen( color ) == colorComp );
            rcUNITTEST_ASSERT( rfBlue( color ) == colorComp );
            // Set img32 pixel
            img32.setPixel( column, row, color );
            // Reset img8 pixel
            img8.setPixel( column, row, 37 );
        }
    }

    return mErrors;
}

// Verify image pixel values
uint32
UT_ImageProcessing::verifyRfRcWindow32to8( rcWindow& img32, rcWindow& img8 )
{
    for ( int32 row = 0; row < img8.height(); row++ ) {
        for ( int32 column = 0; column < img8.width(); column++ ) {
            uint32 color32 = img32.getPixel( column, row );
            uint32 colorComp32 = rfGreen( color32 );
            uint32 color = img8.getPixel( column, row );
            
            // 8-bit pixel value should equal one 32-bit component
            rcUNITTEST_ASSERT( colorComp32 == color );
            if ( colorComp32 != color ) {
                fprintf( stderr, "rfRcWindow32to8: 32-bit color component %i != 8-bit color %i\n",
                         colorComp32, color );
            }
        }
    }
       
    return mErrors;
}

// Reset image pixel values
uint32
UT_ImageProcessing::resetRfRcWindow8to32( rcWindow& img32, rcWindow& img8 )
{
    // Set 32-bit pixel values
    for ( int32 row = 0; row < img32.height(); row++ ) {
        for ( int32 column = 0; column < img32.width(); column++ ) {
            uint32 colorComp = (row*column) % 256;
            // Set img32 pixel
            img32.setPixel( column, row, 37 );
            // Reset img8 pixel
            img8.setPixel( column, row, colorComp );
        }
    }

    return mErrors;
}

// Verify image pixel values
uint32
UT_ImageProcessing::verifyRfRcWindow8to32( rcWindow& img32, rcWindow& img8 )
{
    const rcSharedFrameBufPtr& frame = img8.frameBuf();
     
    for ( int32 row = 0; row < img8.height(); row++ ) {
        for ( int32 column = 0; column < img8.width(); column++ ) {
            uint32 color32 = img32.getPixel( column, row );
            uint32 color = frame->getColor(img8.getPixel( column, row ));
            
            // 8-bit pixel value should equal 32-bit color value
            rcUNITTEST_ASSERT( color32 == color );
            if ( color32 != color ) {
                fprintf( stderr, "rfRcWindow8to32: 32-bit color %i != 8-bit color %i\n",
                         color32, color );
            }
        }
    }
       
    return mErrors;
}

// Test 32-bit to 8-bit reduction
uint32
UT_ImageProcessing::rfRcWindow32to8( rcWindow& img32, rcWindow& img8 )
{
    // Reset pixels
    resetRfRcWindow32to8( img32, img8 );
    // Reduce bit depth from 32 to 8
    ::rfRcWindow32to8( img32, img8 );
    // Verify reduction
    verifyRfRcWindow32to8( img32, img8 );

    // Reset pixels
    resetRfRcWindow32to8( img32, img8 );
    // Create vectors
    vector<rcWindow> vector32;
    vector<rcWindow> vector8;
    const uint32 size = 7;
    for( uint32 i = 0; i < size; ++i ) {
        vector32.push_back( img32 );
        vector8.push_back( img8 );
    }
    // Reduce bit depth from 32 to 8
    ::rfRcWindow32to8( vector32, vector8 );
    // Verify reduction
    for( uint32 i = 0; i < size; ++i ) {
        verifyRfRcWindow32to8( vector32[i], vector8[i] );
    }
    
    return mErrors;
}

// Test 32-bit to 8-bit reduction
uint32
UT_ImageProcessing::testRfRcWindow32to8( bool isGray )
{
    const uint32 width = 379;
    const uint32 height = 777;

    // Create 32-bit buffer and image
    rcFrame* buf32 = new rcFrame( width, height, rcPixel32 );
    buf32->setIsGray( isGray );
    rcWindow img32( buf32 );
       
    // Create 8-bit buffer and image
    rcFrame* buf8 = new rcFrame( width, height, rcPixel8 );
    buf8->setIsGray( isGray );
    rcWindow img8( buf8 );

    // Test reduction
    rfRcWindow32to8( img32, img8 );
    
    return mErrors;
}

// Test 8-bit to 32-bit promotion
uint32
UT_ImageProcessing::rfRcWindow8to32( rcWindow& img32, rcWindow& img8 )
{
    // Reset pixels
    resetRfRcWindow8to32( img32, img8 );
    // Increase bit depth from 8 to 32
    ::rfRcWindow8to32( img8, img32 );
    // Verify 
    verifyRfRcWindow8to32( img32, img8 );

    return mErrors;
}

// Test 8-bit to 32-bit promotion
uint32
UT_ImageProcessing::testRfRcWindow8to32( bool isGray )
{
    const uint32 width = 379;
    const uint32 height = 777;

    // Create 32-bit buffer and image
    rcFrame* buf32 = new rcFrame( width, height, rcPixel32 );
    buf32->setIsGray( isGray );
    rcWindow img32( buf32 );
       
    // Create 8-bit buffer and image
    rcFrame* buf8 = new rcFrame( width, height, rcPixel8 );
    buf8->setIsGray( isGray );
    rcWindow img8( buf8 );

    // Test promotion
    rfRcWindow8to32( img32, img8 );
    
    return mErrors;
}

// Test all pixel values
void
UT_ImageProcessing::testPixels( const rcWindow& image, uint32 expectedValue )
{
    for ( int32 y = 0; y < image.height(); ++y ) 
        for ( int32 x = 0; x < image.width(); ++x ) {
            rcUNITTEST_ASSERT( image.getPixel( x, y ) == expectedValue );
        }
}


// 16-bit pixel reversal
uint32
UT_ImageProcessing::testConvertPixels16()
{
    rcWindow image( 15, 23, rcPixel16 );

    uint16 z = 666;
    float minz (z), maxz (z+image.n()-1);

    for (int32 y = 0; y < image.height(); ++y ) 
      for ( int32 x = 0; x < image.width(); ++x, z++ )
	  {
            image.setPixel( x, y, (int32) z);
	  }

    rcWindow image8 =  rfImageConvert168 (image);
    // @note ipconvert uses 1/2 percent tail.
    // @todo when that changes this code will fail.
    //    minz += 1.0f;
    //    maxz -= 1.0f;

    for ( int32 y = 0; y < image.height(); ++y ) 
      for ( int32 x = 0; x < image.width(); ++x )
	{
	  float ep = (float) image.getPixel (x, y);
	  ep = ( 255.0f * (ep - minz ) / (maxz - minz) );
	  int32 epi = (int32) ep;
	  if (epi < 0) epi = 0;
	  if (epi > 255) epi = 255;
	  int32 diff = image8.getPixel (x, y) - epi;
	  rcUTCheck (rmABS (diff) <= 4);
	  if (rmABS (diff) > 4)
	    cerr << image.getPixel (x, y) << " M: " << epi << " ~ " << image8.getPixel (x, y) << " - = " << diff << endl;
	}
    return mErrors;
}
