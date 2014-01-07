/*
 *  ut_edge.cpp
 *  framebuf
 *
 *	$Id: ut_edge.cpp 4420 2006-05-15 18:48:40Z armanmg $
 *  Created by Arman Garakani on Mon May 27 2002.
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */


#include <stdlib.h>
#include "ut_edge.h"
#include <rc_window.h>
#include <rc_edge.h>
#include <rc_time.h>
#include <rc_vector2d.h>



UT_Edge::UT_Edge ()
{
}

UT_Edge::~UT_Edge ()
{
   printSuccessMessage( "Edge Unit Test", mErrors );
}

uint32 UT_Edge ::run()
{
   edgeTest();
	
#if defined (PERFORMANCE)	
   edgeTime( 512 );
#endif
	
   return mErrors;
}

void UT_Edge::edgeTime ( uint32 width )
{
  rcWindow win (width, width + 7);
  uint8 value = 255;
  int size = width;
  win.setAllPixels (0);
  rcWindow bar (win, size / 2 - size / 4, 0, size / 2, size);
  bar.setAllPixels (value);
  rcWindow foo (win, 0, size / 2 - size / 4, size, size / 2);
  foo.setAllPixels (value);

   rcWindow mag ( win.width()-2, win.height()-2, rcPixel8);
   rcWindow ang ( win.width()-2, win.height()-2, rcPixel8);
   rcWindow mag2 ( win.width()-2, win.height()-2, rcPixel8);
   rcWindow ang2 (win.width()-2, win.height()-2, rcPixel8);


   rcWindow peaks, peaks2;
   rcTime timer;

   timer.start ();
   rfSobelEdge (win, mag, ang);
   timer.end ();

   fprintf (stderr, "Performance: SobelEdge for [%d x %d 8] in %3.3f ms, %3.3f MB/s, %.2f fps\n", 	
	    win.width (), win.height(), timer.milliseconds(), (win.height() * win.width())/timer.microseconds(), 1000/timer.milliseconds());   
   
   timer.start ();
   rfSpatialEdge (mag2, ang2, peaks2, 10);
   timer.end ();
   
   fprintf (stderr, "Performance: SpatialEdge for [%d x %d 8] in %3.3f ms, %3.3f MB/s, %.2f fps\n", 	
	    win.width(), win.height(), timer.milliseconds(), (win.height() * win.width())/timer.microseconds(), 1000/timer.milliseconds());   
}


static void goldSobel(rcWindow& src, rcWindow& mag, rcWindow& ang);

#define TEST_8BIT 2
#define TEST_8BIT_AND_16BIT 3

void UT_Edge::edgeTest ()
{
  for (uint32 ii = 1; ii < TEST_8BIT; ii++)
    for (uint32 aa = 1; aa < TEST_8BIT; aa++)
    {
      cerr << "Testing: Image Depth: " << ii*8 << "\tAngle Depth: " << aa*8 << endl;

      rcWindow win (16, 24, (rcPixel) ii);
      double value = 64;
      int w (win.width ()), h (win.height());
      win.set (0.0);
      rcWindow bar (win, w / 2 - w / 8, 0, w / 4, h);
      bar.set (value);
      rcWindow foo (win, 0, h / 2 - h / 8, w, h / 4);
      foo.set (value);

      rcWindow mag2 ( win.width()-2, win.height()-2, rcPixel8);
      rcWindow ang2 ( win.width()-2, win.height()-2, rcPixel (aa) );   
      rcWindow mask ( win.width()-2, win.height()-2, rcPixel8 );   
      mask.set (0.0);
      mask.setPixel (8,7,1);      mask.setPixel (9,7,1);
      mask.setPixel (8,8,1);      mask.setPixel (9,8,1);
      mask.setPixel (4,13, 1);      mask.setPixel (5,13,1); mask.setPixel (9,13, 1);
      mask.setPixel (4, 14,1);      mask.setPixel (5, 14,1); mask.setPixel (8, 14, 1);

      rcWindow imag, iang, peaks, peaks2;
      rfSobel (win, imag, iang, aa == rcPixel16);
      rcUTCheck (imag.isBound ());
      rcUTCheck (iang.isBound ());
      rcWindow mag (imag, 1, 1,  win.width()-2, win.height()-2, rcPixel8);
      rcWindow ang ( iang, 1, 1, win.width()-2, win.height()-2, rcPixel (aa) );

      goldSobel (win, mag2, ang2);

      rcUNITTEST_ASSERT(mag.contentCompare (mag2));
      rcUNITTEST_ASSERT(ang.contentMaskCompare (ang2,mask));

      // @note for now since we changed the calculation to a more precise one
      //   rcUNITTEST_ASSERT(imageDiff (ang, ang2) == 0);   
      uint32 n = rfSpatialEdge (mag, ang, peaks, 10);
      uint32 n2 = rfSpatialEdge (mag2, ang2, peaks2, 10);
      rcUNITTEST_ASSERT(peaks.contentCompare(peaks2));
      rcUNITTEST_ASSERT(n == n2);   
    }
      // Test Gradient Direction Selector
      rcGradientDir<int32> gd;
      rcIPair first;
      rcIPair tfirst;

      for (uint32 i = 0; i < 256; i++)
	{
	  int32 ax = ((i + 16) / 32 ) % 8; 
	  switch(ax)
	    {
	    case 0:
	      first = rcIPair (-1, 0); //*(mag - 1);
	      break;
	    case 4:
	      first = rcIPair (+1, 0); //*(mag + 1);
	      break;

	    case 1:
	      first = rcIPair (-1, -1); //*(mag - 1 - magUpdate);
	      break;
	    case 5:
	      first = rcIPair (+1, +1); //*(mag + 1 + magUpdate);
	      break;

	    case 2:
	      first = rcIPair (0, -1); //*(mag - magUpdate);
	      break;
	    case 6:
	      first = rcIPair (0, +1); //*(mag + magUpdate);
	      break;
          
	    case 3:
	      first = rcIPair (+1, -1); //*(mag + 1 - magUpdate);
	      break;
	    case 7:
	      first = rcIPair (-1, +1); //*(mag - 1 + magUpdate);
	      break;
	    default:
	      rcUTCheck (0);
	    }

	  tfirst = gd.grad (ax);
	
	  rcUTCheck (tfirst.x() == first.x());
	  rcUTCheck (tfirst.y() == first.y());
	  if ( tfirst.y() != first.y() || tfirst.x() != first.x() )
	    {
	      cerr << "\ti " << i << ": first " << tfirst << " != " << first << endl;
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



   
