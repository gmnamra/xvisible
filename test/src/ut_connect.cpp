/*
 *  ut_connect.cpp
 *  
 *
 *  Created by Arman Garakani on Tue Aug 20 2002.
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_window.h>
#include <rc_vector2d.h>
#include <rc_time.h>
#include <rc_utdrawutils.h>
#include <rc_labelRLE.h>

#include "ut_pmcorr.h"

// Vanilla image printing
#define cmPrintImage(a){                                    \
    for (int32 i = 0; i < (a).height(); i++)              \
    {                                                       \
        fprintf (stderr, "\n");                             \
        for (int32 j = 0; j < (a).width(); j++)	    \
            fprintf (stderr, " %3d ", (a).getPixel (j, i)); \
        fprintf (stderr, "\n");                             \
    }}

int ut_diffhist ();

UT_Connect::UT_Connect()
{
}

UT_Connect::~UT_Connect()
{
        printSuccessMessage( "rcConnect test", mErrors );
}

uint32
UT_Connect::run() {



    // Test different sizes
    {        
      testConnect ();

      // Enable these when rect/RLE size definition is clear
      // They cause an assertion failure now
      //testRLEs( 3, 3 );
      //testRLEs( 4, 4 );
      testRLEs( 7, 7 );
      testRLEs( 9, 9 );
      testRLEs( 16, 16 );
      testRLEs( 27, 27 );
      testRLEs( 379, 379 );
      testRLEs( 512, 512 );
      testRLEs( 1280, 960, true ); // Last test displays success message

      testCircles( 256 );
      testCircles( 1024 );
      
      testPerformance ( 1024 );
      testPerformance ( 1280 );


    }

    return mErrors;
}



void UT_Connect::testConnect()
{
   char *shape[] =
{
   "10100100",
   "00110101",
   "00011000",
   "01000100",
   "01010001",0};


   rcWindow ti;

   rfDrawShape (ti, shape);

   rcWindow label;
   uint32 regions;

   rfGetComponents (ti, regions, label);

   rcUNITTEST_ASSERT (regions == 6);
   rcUNITTEST_ASSERT (label.width() == ti.width());
   rcUNITTEST_ASSERT (label.height() == ti.height());

   // Add checking the label image
   rcUTCheck (label.getPixel (0, 0) == 1);
   rcUTCheck (label.getPixel (2, 0) == 2);
   rcUTCheck (label.getPixel (7, 1) == 3);
   rcUTCheck (label.getPixel (1, 3) == 4);
   rcUTCheck (label.getPixel (3, 4) == 5);
   rcUTCheck (label.getPixel (7, 4) == 6);


   printSuccessMessage( "Simple shape connectivity unit test" , mErrors);
}

void UT_Connect::testPerformance( int32 width )
{
   // Test Performance
   // Image with n circles in it
   rcWindow plate;
   uint32 nc = drawCells (plate, width, 25, 255, 0);

   double dSeconds;
   rcTime timer;
   rcWindow labels;
   uint32 regions = 0;

   timer.start ();
   rfGetComponents (plate, regions, labels);
   timer.end ();

   dSeconds = timer.seconds ();

   rcUNITTEST_ASSERT (regions == nc);   

   // Per Byte in Useconds
   double perByte = timer.microseconds () / (plate.width() * plate.height() * plate.depth());
   fprintf(stderr,
           "Performance: rfGetComponents [%i x %i x %i]  %d regions : %f milliseconds, %f usecond/ 8bit pixel %f MBytes per second\n",
           plate.width(), plate.height(), plate.depth()*8,
           int32 (regions), timer.milliseconds (), perByte, 1 / perByte);

   vector<rcRLEWindow> gRLEs;
   rcIPair minSize( 0, 0 );
   
   timer.start ();
   rfGetRLEs (labels, regions, minSize, true, gRLEs);
   timer.end ();

   rcUNITTEST_ASSERT( gRLEs.size() == regions );
    
   dSeconds = timer.seconds ();
   
   // Per Byte in Useconds
   perByte = timer.microseconds () / (plate.width() * plate.height() * plate.depth());
   fprintf(stderr,
           "Performance: rfGetRLEs       [%i x %i x %i]  %d regions : %f milliseconds, %f usecond/ 8bit pixel %f MBytes per second\n",
           plate.width(), plate.height(), plate.depth()*8,
           int32 (regions), timer.milliseconds (), perByte, 1 / perByte);

   timer.start ();
   for ( uint32 i = 0; i < gRLEs.size(); ++i ) {
       rcVisualSegmentCollection lines;
       gRLEs[i].vectorize( lines );
       
   }
   timer.end ();

   // Per region in milliseconds
   perByte = timer.milliseconds () / regions;
   fprintf(stderr,
           "Performance: RLE vectorize   [%i x %i x %i]  %d regions : %f milliseconds, %f ms/region\n",
           plate.width(), plate.height(), plate.depth()*8,
           int32 (regions), timer.milliseconds (), perByte );
   
   vector <rcIRect> gRects;
   vector <uint32> gRectLabel;

   timer.start ();
   rfGetRects (labels, regions, minSize, true, gRects, gRectLabel);
   timer.end ();

   rcUNITTEST_ASSERT( gRects.size() == regions );
    
   dSeconds = timer.seconds ();
   
   // Per Byte in Useconds
   perByte = timer.microseconds () / (plate.width() * plate.height() * plate.depth());
   fprintf(stderr,
           "Performance: rfGetRects      [%i x %i x %i]  %d regions : %f milliseconds, %f usecond/ 8bit pixel %f MBytes per second\n",
           plate.width(), plate.height(), plate.depth()*8,
           int32 (regions), timer.milliseconds (), perByte, 1 / perByte);
   
}

void UT_Connect::testCircles( int32 width )
{
   // Image with n circles in it
   rcWindow plate;
   uint32 nc = drawCells (plate, width, 25, 255, 0);

   rcWindow labels;
   uint32 regions = 0;

   rfGetComponents (plate, regions, labels);

   rcUNITTEST_ASSERT (regions == nc);   

   vector<rcRLEWindow> gRLEs;
   rcIPair minSize( 0, 0 );
   
   rfGetRLEs (labels, regions, minSize, true, gRLEs);

   rcUNITTEST_ASSERT( gRLEs.size() == regions );

   uint32 totalSize = 0;
   uint32 totalRuns = 0;
   double meanLength = 0;
   for ( uint32 i = 0; i < gRLEs.size(); ++i ) {
       rcRLEWindow rle = gRLEs[i];
       rcIRect rect = rle.rectangle();

       totalSize += rle.sizeOf();
       totalRuns += rle.n();
       meanLength += rle.meanLength();
       // Test rects
       rcUNITTEST_ASSERT( !rect.isNull() );

       // Test RLEs
       rcUNITTEST_ASSERT( rle.isBound() );
#ifdef DEBUG_LOG
       cerr << rle;
#endif       
   }
   meanLength = meanLength / regions ;
   
#ifdef DEBUG_LOG   
   cerr << "CircleImage " << width << " x " << width << ", regions " << regions << ", runs " << totalRuns;
   cerr << ", meanRunLength " << meanLength <<  ", size " << totalSize << endl;
#endif   
}

// Test RLE generation from a label image

//#define DEBUG_LOG 1

void UT_Connect::testRLEs( int32 width, int32 height, bool displayMessage )
{
    uint32 oldErrors = mErrors;
        
    testCorners( width, height );
    // Test enclosures for large windows with odd dimensions
    if ( (width >= 5) && (height >= 5) && (width % 2) && (width % 2) ) {
        testEnclosures( width, height, false );
        testEnclosures( width, height, true );
    }
    if ( displayMessage ) {
        char buf[512];
        snprintf( buf, rmDim(buf), "rfGetRLEs unit test" );
        printSuccessMessage( buf , mErrors = oldErrors );
    }
}

void UT_Connect::testCorners( int32 width, int32 height )
{
    // Put a cell in each corner, leave at least one pixel of space between cells
    // Cell width = width/2 -1, cell height = height/2 -1

    // 11022
    // 11022
    // 00000
    // 33044
    // 33044

    uint32 dummy;
    rcPixel d = rcPixel16;
    rcWindow ti (width, height, d);
    int32 cellWidth = rfRound(0.5 + width/2.0 - 1, dummy);
    int32 cellHeight = rfRound(0.5 + height/2.0 - 1, dummy);
    uint32 maxPixelvalue = 4;
    vector<rcIPair> origins( maxPixelvalue );
    
    rmAssert( cellWidth > 0 );
    rmAssert( cellHeight > 0 );

    // Clear image
    for (int32 i = 0; i < ti.width(); i++) 
        for (int32 j = 0; j < ti.height(); j++) 
            ti.setPixel (i, j, 0);

    origins[0] = rcIPair( 0, 0 );
    origins[1] = rcIPair( ti.width()-cellWidth, 0 );
    origins[2] = rcIPair( 0, ti.height() - cellHeight );
    origins[3] = rcIPair( ti.width()-cellWidth, ti.height() - cellHeight );
    
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
#ifdef DEBUG_LOG
    cmPrintImage( ti );
    cerr << "--------------" << endl;
#endif

    vector<rcRLEWindow> gRLEs;
    rcIPair minSize( 0, 0 );
    
    rfGetRLEs (ti, maxPixelvalue, minSize, true, gRLEs);

    rcUNITTEST_ASSERT( gRLEs.size() == maxPixelvalue );

    uint32 totalSize = 0;
    uint32 totalRuns = 0;
    for ( uint32 i = 0; i < gRLEs.size(); ++i ) {
        rcRLEWindow rle = gRLEs[i];
        rcIRect rect = rle.rectangle();

        totalSize += rle.sizeOf();
        totalRuns += rle.n();
        // Test rects
        rcUNITTEST_ASSERT( !rect.isNull() );
        rcUNITTEST_ASSERT( rect.width() == int(cellWidth) );
        rcUNITTEST_ASSERT( rect.height() == int(cellHeight) );
        rcUNITTEST_ASSERT( rect.ul() == origins[i] );
        
        // Test RLEs
        rcUNITTEST_ASSERT( rle.isBound() );
        rcUNITTEST_ASSERT( rle.maxPixelValue() == maxPixelvalue );
        
        rcWindow proj = rle.image();
        rcUNITTEST_ASSERT( proj.getPixel( 0, 0 ) == i+1 );
        rcUNITTEST_ASSERT( proj.width() == rle.width() );
        rcUNITTEST_ASSERT( proj.height() == rle.height() );
     
#ifdef DEBUG_LOG
        cerr << "--------------" << endl;
        cmPrintImage( proj );
        cerr << rect << endl;
        cerr << rle;
#endif
    }

    // Test minimum size
    minSize = rcIPair( cellWidth+1, cellHeight+1 );
    rfGetRLEs (ti, maxPixelvalue, minSize, true, gRLEs);
    // All cells should be rejected
    rcUNITTEST_ASSERT( gRLEs.size() == 0 );
    
#ifdef DEBUG_LOG       
    cerr << "CornerImage " << width << " x " << height << ", regions " << gRLEs.size() << ", runs " << totalRuns <<  ", size " << totalSize << endl;
#endif    
}

void UT_Connect::testEnclosures( int32 width, int32 height, bool combineRegions )
{
    // Test for an enclosed region
    // Minimum size is 5x5
    
    // 11111
    // 10001
    // 10201
    // 10001
    // 11111

	//@note assumes label image is 16bit
    rcPixel d = rcPixel16;
    rcWindow ti (width, height, d);
    uint32 cellWidth = width - 4;
    uint32 cellHeight = cellWidth;
    uint32 maxPixelvalue = 2;
    
    rmAssert( cellWidth > 0 );
    rmAssert( cellHeight > 0 );

    // Clear image
    for (int32 i = 0; i < ti.width(); i++) 
        for (int32 j = 0; j < ti.height(); j++) 
            ti.setPixel (i, j, 0);

    // Create region 1
    for (int32 i = 0; i < ti.width(); i++) {
        ti.setPixel (i, 0, 1);
        ti.setPixel (i, ti.height()-1, 1);
    }
    for (int32 j = 0; j < ti.height(); j++) {
        ti.setPixel (0, j, 1);
        ti.setPixel (ti.width()-1, j, 1);
    }

    // Create region 2
    for (int32 i = ti.width()-1; i >= 0; --i) 
        for (int32 j = 0; j < ti.height(); j++) {
            if ( i > 1 && (i < int(ti.width()-2)) )
                if ( j > 1 && (j < ti.height()-2) )
                ti.setPixel (i, j, 2);

        }
#ifdef DEBUG_LOG
    cmPrintImage( ti );
    cerr << "--------------" << endl;
#endif

    vector<rcRLEWindow> gRLEs;
    rcIPair minSize( 0, 0 );
    
    rfGetRLEs (ti, maxPixelvalue, minSize, combineRegions, gRLEs);

    if ( combineRegions )
        rcUNITTEST_ASSERT( gRLEs.size() == 1 );
    else
        rcUNITTEST_ASSERT( gRLEs.size() == maxPixelvalue );

    for ( uint32 i = 0; i < gRLEs.size(); ++i ) {
        rcRLEWindow rle = gRLEs[i];
        rcIRect rect = rle.rectangle();

        // Test rects
        rcUNITTEST_ASSERT( !rect.isNull() );
        
        // Test RLEs
        rcUNITTEST_ASSERT( rle.isBound() );
        rcUNITTEST_ASSERT( rle.maxPixelValue() == maxPixelvalue );
        
        rcWindow proj = rle.image();
        rcUNITTEST_ASSERT( proj.getPixel( 0, 0 ) == i+1 );
        rcUNITTEST_ASSERT( proj.width() == rle.width() );
        rcUNITTEST_ASSERT( proj.height() == rle.height() );

#ifdef DEBUG_LOG
        cerr << "--------------" << endl;
        cmPrintImage( proj );
        cerr << rect << endl;
        cerr << rle;
#endif
    }
}




static void vectorize (int32 xMag,int32 yMag,double &r,double& theta, double norm);
static void goldSobel(rcWindow& src, rcWindow& mag, rcWindow& ang);
static void checkRLE (rcLabelRLE& lr, rcWindow& ti);


#if defined (UT_DIFFHIST)
int ut_diffhist ()
{
  { 
  char *shape[] =
     {
   "10100100",
   "00110101",
   "00011000",
   "01000100",
   "01010001",0};
   rcWindow ti;
   rfDrawShape (ti, shape);
   int32 psum = rfGradHist1d ((uint8 *) ti.pelPointer (0,0), (uint8 *) ti.pelPointer (ti.width()-2,0));
   assert (psum == 5);
  }
  {
   char *shape[] =
     {
   "102001007",
   "001101011",
   "000110001",
   "010001000",
   "010100010",0};
   rcWindow ti;
   rfDrawShape (ti, shape);
   rcLabelRLE lr (ti);
   assert (lr.numRuns () == 34);
   checkRLE (lr, ti);

   rcWindow win;int32 width=64;
   uint32 nc = drawCells (win, 64, 5, 255, 0);
   rcWindow mag (win.width()-2, win.height()-2, rcPixel8);
   rcWindow ang (win.width()-2, win.height()-2, rcPixel8);
   goldSobel(win, mag, ang);
   rcLabelRLE alr (ang);   
   checkRLE (alr, ang);
  }
}
#endif

   static void checkRLE (rcLabelRLE& lr, rcWindow& ti)
   {

     for (int32 j = 0; j < ti.height(); j++)
       {
	 rcLabelRLE::Run * runs = lr.pointToRow (j);
	 int32 n = 0;
	 int32 p = 0;
	 while (runs->length)
	   {
	     for (int32 i=0;i<runs->length;i++)
	       {
		 assert (ti.getPixel (p+i,j) == runs->value);
		 p+=i;
	       }
	     assert (runs->label == 0);
	     runs++;
	   }
       }
   }



static void vectorize (int32 xMag,int32 yMag,double &r,double& theta, double norm)
{
  double mag(sqrt((double) (rmSquare(xMag) + rmSquare(yMag))));
  r = rmMin (mag * 255.0 / 256.0, 255.0) + 0.499;
  
  const double ascale(norm / (2*rkPI));
  rcRadian tt (atan2((double)yMag, (double)xMag));
  tt = tt.norm ();
  theta = tt.basic ();
  theta = theta*ascale + 0.5;
}

static void goldSobel(rcWindow& src, rcWindow& mag, rcWindow& ang)
{
   double r,theta;
   bool a16 = ang.depth() == rcPixel16;
   for(int32 i = 0; i < mag.width(); i++)
      for(int32 j = 0; j < mag.height(); j++)
         {
         int32 xMag,yMag;
         xMag = src.getPixel(i+2,j) + 2*src.getPixel(i+2,j+1) + src.getPixel(i+2,j+2) -
            (src.getPixel(i,j) + 2*src.getPixel(i,j+1) + src.getPixel(i,j+2));

         yMag = src.getPixel(i,j+2) + 2*src.getPixel(i+1,j+2) + src.getPixel(i+2,j+2) -
            (src.getPixel(i,j) + 2*src.getPixel(i+1,j) + src.getPixel(i+2,j));

         xMag = xMag / 8;
         yMag = yMag  /8;

	 if (!a16)
	   {
	     vectorize (xMag,yMag,r,theta,256.0);
	     mag.setPixel(i,j,uint8(r));
	     ang.setPixel(i,j,uint8(theta));
	   }
	 else
	   {
	     vectorize (xMag,yMag,r,theta,65536.0);
	     mag.setPixel(i,j,uint8(r));
	     ang.setPixel(i,j,uint16(theta));
	   }
	 }
}



   
   

