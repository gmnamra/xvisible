/*
 *  File.
 *  
 *
 *	$Id: ut_ip.cpp 5472 2007-11-29 22:08:45Z armanmg $
 *
 *  
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_utdrawutils.h>
#include <rc_window.h>
#include <rc_vector2d.h>
#include <rc_time.h>
#include <rc_ip.h>
#include <rc_moments.h>
#include "ut_ip.h"
#include "ut_pmcorr.h"

static void rfGaussGold(const rcWindow& src, rcWindow& dest, int32 kernelSize);
static void rfGaussGold(const rcWindow& src, rcWindow& dst);
static void rfGauss3Gold(const rcWindow& src, rcWindow& dst);
static void rfGauss3Gold16 (const rcWindow& src, rcWindow& dst);
static float variance (const rcWindow& image, const rcIPair site, rcWindow& varImg);

// Vanila image printing
#define rmPrintImage(a){ \
for (int32 i = 0; i < (a).height(); i++) \
{ \
   fprintf (stderr, "\n"); \
      for (int32 j = 0; j < (a).width(); j++) \
         fprintf (stderr, ".%3d.", (a).getPixel (j, i)); \
            fprintf (stderr, "\n");\
}}

#define rmPrintFloatImage(a){					    \
    if (a.isD32Float ()) fprintf(stderr, "Is marked float\n"); \
    else fprintf(stderr, "Is not marked float\n");			    \
    for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {  \
      fprintf (stderr, "\n");					    \
      float* vp = (float*)(a).rowPointer(i__temp);		    \
      for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)   \
	fprintf (stderr, "-%f-", vp[j__temp]);				    \
      fprintf (stderr, "\n");					    \
    }}

#define rmSetPatternPixels(a)  \
  (a).setPixel(0, 0, 0);      \
  (a).setPixel(1, 0, 10);	      \
  (a).setPixel(2, 0, 20);	      \
  (a).setPixel(3, 0, 30);	      \
  (a).setPixel(0, 1, 1);	      \
  (a).setPixel(1, 1, 11);	      \
  (a).setPixel(2, 1, 21);	      \
  (a).setPixel(3, 1, 31);	      \
  (a).setPixel(0, 2, 2);	      \
  (a).setPixel(1, 2, 12);	      \
  (a).setPixel(2, 2, 22);	      \
  (a).setPixel(3, 2, 32);	      \
  (a).setPixel(0, 3, 3);	      \
  (a).setPixel(1, 3, 13);	      \
  (a).setPixel(2, 3, 23);	      \
  (a).setPixel(3, 3, 33);	      \
  (a).setPixel(0, 4, 4);	      \
  (a).setPixel(1, 4, 14);	      \
  (a).setPixel(2, 4, 24);	      \
  (a).setPixel(3, 4, 34);	      \
  (a).setPixel(0, 5, 5);	      \
  (a).setPixel(1, 5, 15);	      \
  (a).setPixel(2, 5, 25);	      \
  (a).setPixel(3, 5, 35)


UT_moreip::UT_moreip()
{
}

UT_moreip::~UT_moreip()
{
        printSuccessMessage( "rcIP test", mErrors );
}

void UT_moreip::testmorph ()
    {
      rcWindow t8 (16, 16);
      rcWindow t16 (16, 16, rcPixel16);
      rcWindow d8 (16, 16);
      rcWindow d16 (16, 16, rcPixel16);

      t8.set (0.0);
      t16.set (0.0);
      t8.setPixel (11, 12, 137);
      t16.setPixel (11, 12, 137);
      rfPixelMax3By3 (t8, d8);

      for (int32 j = -1; j < 2; j++)
	for (int32 i = -1; i < 2; i++)
	  rcUTCheck (d8.getPixel (11+i, 12+j) == 137);

      rfPixelMax3By3 (t16, d16);
      for (int32 j = -1; j < 2; j++)
	for (int32 i = -1; i < 2; i++)
	  rcUTCheck (d16.getPixel (11+i, 12+j) == 137);
      
      printSuccessMessage( "Basic Pixel Min/Max Unit test unit test" , mErrors);   
    }

uint32 UT_moreip::run()
{
  // Test different sizes
  {        

    testmorph ();

    testReflect (50);

    testExpand ();

    testSample ();

    testIntegral ();
    testShift ();

    testGauss ();

    testPixelMap (16, 17);
    testPixelMap (41, 1);
    testPixelMap (1, 21);

#if defined (PERFORMANCE)	  
    testTimes ( 0, 8, 8 );   // Test speed without Altivec
    fprintf( stderr, "\n" );
    testTimes ( 0, 320, 240 ); // Test speed without Altivec
    fprintf( stderr, "\n" );
    testTimes ( 0, 640, 480 ); // Test speed without Altivec
    fprintf( stderr, "\n" );
    testTimes ( 0, 1024, 1024 ); // Test speed without Altivec
    fprintf( stderr, "\n" );
    testTimes ( 0, 1280, 960 ); // Test speed without Altivec
    fprintf( stderr, "\n" );
    testTimes ( 0, 2048, 2048 ); // Test speed without Altivec

      testTimes2 ( 0, 1024, 1024 ); // Test speed without Altivec
      testTimes2 ( 1, 1024, 1024 ); // Test speed with Altivec
      fprintf( stderr, "\n" );
      testTimes2 ( 0, 1280, 960 ); // Test speed without Altivec
      testTimes2 ( 1, 1280, 960 ); // Test speed with Altivec
      fprintf( stderr, "\n" );
#endif
	  
      // Size this big is very slow to test
      //testTimes ( 0, 2048, 2048 ); // Test speed without Altivec
      //testTimes ( 1, 2048, 2048 ); // Test speed with Altivec

  }

  return mErrors;
}


void UT_moreip::testGauss ()
{
  // First Check some basic sizes
  for (int32 i = 1; i < 3; i++)
    {
      rcPixel depth = (rcPixel) i;

      // Old style window phase test
      testGauss (16, 16, depth);
      testGauss (41, 5, depth);
      testGauss (5, 21, depth);
    }

  // Check for a number of different kernel sizes. 
  for (int32 i = 1; i < 3; i++)
    {
      rcPixel depth = (rcPixel) i;

  // First check the kernels (TODO: Is there a bug for kernel > 7)
      int32 width (33), height (34);
      rcWindow tmp (width, height, depth);
      tmp.setAllPixels (0);
      tmp.setPixel (tmp.width()/2, tmp.height()/2, i == 1 ? 255 : 65535);
      rcWindow mask (width, height, rcPixel8);
      mask.set (0.0);

      for (uint32 k = 3; k < 13; k+=2)
	{
	  rcWindow gold (width, height, depth);
	  rcWindow gauss (width, height, depth);
	  rcWindow goldbase (width, height, depth);
	  mask.setPixel (0, 0, 1);
	  mask.setPixel (0, height-1, 1);
	  mask.setPixel (width-1, 0, 1);
	  mask.setPixel (width-1, height-1, 1);

	  
	  rfGaussianConv (tmp, gauss, k);
	  rfGaussGold(tmp, gold, k);
	  if (gauss.contentMaskCompare( gold, mask) == 0)
	    {
	      cerr << i << "," << k << endl;
	      cerr << gold << endl;
	      cerr << gauss << endl;
	    }
	  rcUNITTEST_ASSERT (gauss.contentMaskCompare( gold, mask) != 0);
	}
    }

  printSuccessMessage( "Gauss Unit test " , mErrors);   
}


void UT_moreip::testGauss (uint32 width, uint32 height, rcPixel depth)
{
  // Create a buffer for testing at all shifts for src and dst
  rcWindow src (width+15, height,depth);
  rcWindow dst (width+15, height,depth);
  rcWindow gold (width, height,depth);

  // create an image set them to identity
  for (int32 i = 0; i < src.width(); i++)
    {
      for (int32 j = 0; j < src.height(); j++)
	{
	  src.setPixel (i, j, (j*src.width() + i) % (depth == rcPixel8 ? 255 : 65535 ));
	}
    }

  rcWindow mask3 (width,height,rcPixel8);
  mask3.set  (0.0);
  mask3.setPixel (0, 0, 1);
  mask3.setPixel (0, height-1, 1);
  mask3.setPixel (width-1, 0, 1);
  mask3.setPixel (width-1, height-1, 1);

   for (int32 ii = 0; ii < 16; ii++)
     {
       rcWindow ti (src, ii, 0, width, height);
       rcWindow di (dst, ii, 0, width, height);

       rfForceSIMD ( bool (0) );
       rfGaussianConv (ti, gold, 3);
       rfForceSIMD ( bool (1) );
       rfGaussianConv (ti, di, 3);

       rcUTCheck (gold.contentMaskCompare (di, mask3) == true);
     }
}



void UT_moreip::testPixelMap(uint32 width, uint32 height)
{

  // Create a buffer for testing at all shifts for src and dst
   rcWindow src (width+15, height);
   rcWindow dst (width+15, height);
   rcWindow gold (width, height);

   // Create an inversion table
   vector<uint8> lut(256);
   for (int i = 0; i < 256; i++) lut[i] = 255 - i;

   for (int32 i = 0; i < 16; i++)
     {
       rcWindow ti (src, i, 0, width, height);
       rcWindow di (dst, i, 0, width, height);

       // create an image set them to identity
       for (int32 i = 0; i < ti.width(); i++)
	 {
	   for (int32 j = 0; j < ti.height(); j++)
	     {
	       ti.setPixel (i, j, (j*ti.width() + i) % 255);
	     }
	 }

       rfForceSIMD ( bool (0) );

       rfPixel8Map (ti, gold, lut);

       rfForceSIMD ( bool (1) );

       rfPixel8Map (ti, di, lut);

       for (int32 i = 0; i < ti.width(); i++)
	 {
	   for (int32 j = 0; j < ti.height(); j++)
	     {
	       if (di.getPixel(i, j) != gold.getPixel(i,j))
		 {
		   fprintf (stderr, " %d != %d [%d %d] \n", di.getPixel(i, j) , gold.getPixel(i,j), i, j);
		   rcUNITTEST_ASSERT (di.getPixel(i, j) == gold.getPixel(i,j));
		 }
	     }
	 }
     }
   
   // Simple test for rfPixel8Invert
   rcWindow ti (32, 33);
   rcWindow tid (32, 33);

   for (int32 i = 0; i < ti.width(); i++)
     {
       for (int32 j = 0; j < ti.height(); j++)
	 {
	   ti.setPixel (i, j, (j*ti.width() + i) % 255);
	 }
     }

   rfPixel8Invert (ti, tid);

   for (int32 i = 0; i < tid.width(); i++)
     {
       for (int32 j = 0; j < tid.height(); j++)
	 {
	   uint8 pel = tid.getPixel (i, j);
	   rcUTCheck (pel == 255 -  ((j*ti.width() + i) % 255));
	 }
     }

   rcWindow t16 (23, 61, rcPixel16);
   rcWindow t16d (23, 61, rcPixel16);
   for (int32 i = 0; i < t16.width(); i++)
     {
       for (int32 j = 0; j < t16.height(); j++)
	 {
	   t16.setPixel (i, j, (uint16) ((i * j) + 2));
	 }
     }
   t16d.set (0.0);
   vector<uint16> subb2 (65536);subb2[0] = subb2[1] = 0;
   for (int32 i = 2; i < 65536; i++) subb2[i] = (uint16) (i - 2);
   rfPixel16Map (t16, t16d, subb2);
   for (int32 i = 0; i < t16.width(); i++)
     {
       for (int32 j = 0; j < t16.height(); j++)
	 {
	   uint16 p16 = t16d.getPixel (i, j);
	   rcUTCheck (p16 == (uint16) (i * j));
	 }
     }
   
   // Test in place operation
   rfPixel16Map (t16, t16, subb2);   
   for (int32 i = 0; i < t16.width(); i++)
     {
       for (int32 j = 0; j < t16.height(); j++)
	 {
	   uint16 p16 = t16.getPixel (i, j);
	   rcUTCheck (p16 == (uint16) (i * j));
	 }
     }
   
   printSuccessMessage( "Basic PixelMap Unit test unit test" , mErrors);   
}

void UT_moreip::testTimes2( bool useAltivec, int32 w, int32 h )
{

		int32 width = w * 4;
		int32 height = h * 4;

		const uint32 minPixels = 2000000;
		const uint32 pixels = width * height;
	// Adjust repeats depending on image size
		uint32 repeats = minPixels/pixels;

		if ( repeats < 1 )
			repeats = 1;
		else if ( repeats > 10000 )
			repeats = 10000;

// Create an identify table
		vector<uint8> lut(256);
		for (int i = 0; i < 256; i++) lut[i] = i;

		rcWindow imgA (width, height);
		rcWindow imgB (width, height);

		imgA.randomFill ();

		{
			rcTime timer;

// Alticev setting
			rfForceSIMD (useAltivec );

// Now Time the correlation stuff
// Run test in a loop to diminish the effect of system clock
// inaccuracy. The time spent by the loop code should be negligible

			timer.start ();
			for( uint32 i = 0; i < repeats; ++i )
				rfPixel8Map (imgA, imgB, lut); 
			timer.end ();

			double dMilliSeconds = timer.milliseconds () / repeats;
			double dMicroSeconds = timer.microseconds () / repeats;

// Per Byte in Useconds
			double perByte = dMicroSeconds / (imgA.width() * imgA.height() * imgA.depth());

			fprintf(stderr,
				"Performance: %s PixelMap [%i x %i %i]: %.2f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
				useAltivec ? "AltiVec" : "       ", imgA.width(), imgA.height(), imgA.depth()*8, dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);

		}
// Time Gaussian Convolution
		{


			rcTime timer;

// Alticev setting
			rfForceSIMD (useAltivec );

// Now Time the correlation stuff
// Run test in a loop to diminish the effect of system clock
// inaccuracy. The time spent by the loop code should be negligible

			timer.start ();
			for( uint32 i = 0; i < repeats; ++i )
				rfGaussianConv (imgA, imgB, 3); 
			timer.end ();

			double dMilliSeconds = timer.milliseconds () / repeats;
			double dMicroSeconds = timer.microseconds () / repeats;

// Per Byte in Useconds
			double perByte = dMicroSeconds / (imgA.width() * imgA.height() * imgA.depth());

			fprintf(stderr,
				"Performance: %s Gaussian Conv [3x3] [%i x %i %i]: %.2f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
				useAltivec ? "AltiVec" : "       ", imgA.width(), imgA.height(), imgA.depth()*8, dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);

		}


}

static void rfGaussGold(const rcWindow& src, rcWindow& dest, int32 kernelSize)
{
   assert(kernelSize >= 3);
   assert(kernelSize % 2 == 1);
   assert(dest.width() == src.width());
   assert(dest.height() == src.height());

   assert(&src != &dest);

   const uint32 width(src.width());
   const uint32 height(src.height());

   assert (width >= kernelSize && height >= kernelSize);

   if(kernelSize == 3)
   {
      rfGaussGold(src,dest);
      return;
   }

   rcWindow work(width,height, src.depth());
 
   // Ping pong between dest and work but make sure you end up in dest
   rcWindow& dest1 =  (kernelSize % 4 == 1) ? work : dest;
   rcWindow& dest2 =  (kernelSize % 4 == 3) ? work : dest;

   rfGaussGold (src,dest1);
   
   while(1)
   {
      kernelSize -= 2;

      rfGaussGold (dest1, dest2);
      if(kernelSize == 3)
         return;

      kernelSize -= 2;

      rfGaussGold (dest2, dest1);
      if(kernelSize == 3)
         return;
   }
}

static void rfGaussGold(const rcWindow& src, rcWindow& dst)
{
  switch (src.depth())
    {
    case rcPixel8:
      rfGauss3Gold (src, dst);
      return;
    case rcPixel16:
      rfGauss3Gold16 (src, dst);
      return;
    }
}

static void rfGauss3Gold(const rcWindow& src, rcWindow& dst)
{
  int32		x, y, wd, ht, ruc_src, ruc_dst;
  int32		left_cs, cent_cs, right_cs, new_pel;
  
  const uint8	*top_rp, *mid_rp, *bot_rp, *save_rp;
  uint8		curr_pel;
  uint8		*base_rp_dst, *dst_rp;
  
  
  wd = src.width() - 2;
  ht = src.height() - 2;
  ruc_src = src.rowUpdate();
  ruc_dst = dst.rowUpdate();
  
  top_rp = src.rowPointer(0);
  mid_rp = top_rp + ruc_src;
  bot_rp = mid_rp + ruc_src;
  save_rp = mid_rp;
  base_rp_dst = dst.rowPointer(1) + 1;

  // corners
  dst.setPixel(0,0,src.getPixel(0,0));
  dst.setPixel(0,src.height() - 1,src.getPixel(0,src.height() - 1));
  dst.setPixel(src.width() - 1,0,src.getPixel(src.width() - 1,0));
  dst.setPixel(src.width() -1,src.height() - 1,
          src.getPixel(src.width() -1,src.height() - 1));
  int i;
  for(i = 0; i < wd; i ++)
  {
    dst.setPixel(i + 1,0,uint8((3*src.getPixel(i,0) + src.getPixel(i,1) +
            2*(3*src.getPixel(i + 1,0) + src.getPixel(i + 1,1)) +
                     3*src.getPixel(i+2,0) + src.getPixel(i+2,1) + 8) / 16));
    dst.setPixel(i+1,ht + 1,uint8((src.getPixel(i,ht) + 3*src.getPixel(i,ht + 1) +
            2*(src.getPixel(i + 1,ht) + 3*src.getPixel(i + 1,ht + 1)) +
                        src.getPixel(i+2,ht) + 3*src.getPixel(i+2,ht + 1) + 8) / 16));
  }

  for(i = 0; i < ht; i ++)
  {
    dst.setPixel(0,i+1,uint8((3*src.getPixel(0,i) + src.getPixel(1,i) +
            2*(3*src.getPixel(0,i+1) + src.getPixel(1,i+1)) +
                     3*src.getPixel(0,i+2) + src.getPixel(1,i+2) + 8) / 16));
    dst.setPixel(wd + 1,i + 1,uint8((src.getPixel(wd,i) + 3*src.getPixel(wd + 1,i) +
            2*(src.getPixel(wd,i + 1) + 3*src.getPixel(wd+1,i + 1)) +
                        src.getPixel(wd,i+2) + 3*src.getPixel(wd+1,i+2) + 8) / 16));
  }
  
  for(y = ht; y; y--, base_rp_dst += ruc_dst)
  {
    //    Load up initial column sums
    left_cs = *top_rp++;
    curr_pel = *mid_rp++;
    left_cs += curr_pel + curr_pel;
    left_cs += *bot_rp++;
    
    cent_cs = *top_rp++;
    curr_pel = *mid_rp++;
    cent_cs += curr_pel + curr_pel;
    cent_cs += *bot_rp++;
    
    for(x = wd, dst_rp = base_rp_dst; x; x--)
    {
      right_cs = *top_rp++;
      curr_pel = *mid_rp++;
      right_cs += curr_pel + curr_pel;
      right_cs += *bot_rp++;
      
      new_pel = left_cs + cent_cs + cent_cs + right_cs;
      *dst_rp++ = (uint8)((new_pel+8) >> 4);
      
      left_cs = cent_cs;
      cent_cs = right_cs;
    }
    
    //    Update row pointers for next row iteration
    top_rp = save_rp;
    mid_rp = save_rp + ruc_src;
    bot_rp = mid_rp + ruc_src;
    save_rp = mid_rp;
  }
}



static void rfGauss3Gold16(const rcWindow& src, rcWindow& dst)
{
  int32		x, y, wd, ht, ruc_src, ruc_dst;
  int32		left_cs, cent_cs, right_cs, new_pel;
  
  const uint16	*top_rp, *mid_rp, *bot_rp, *save_rp;
  uint16		curr_pel;
  uint16		*base_rp_dst, *dst_rp;
  
  
  wd = src.width() - 2;
  ht = src.height() - 2;
  ruc_src = src.rowPixelUpdate();
  ruc_dst = dst.rowPixelUpdate();
  
  top_rp = (uint16 *) src.rowPointer(0);
  mid_rp = top_rp + ruc_src;
  bot_rp = mid_rp + ruc_src;
  save_rp = mid_rp;
  base_rp_dst = (uint16 *) dst.pelPointer(1, 1);

  // corners
  dst.setPixel(0,0,src.getPixel(0,0));
  dst.setPixel(0,src.height() - 1,src.getPixel(0,src.height() - 1));
  dst.setPixel(src.width() - 1,0,src.getPixel(src.width() - 1,0));
  dst.setPixel(src.width() -1,src.height() - 1,
          src.getPixel(src.width() -1,src.height() - 1));
  int i;
  for(i = 0; i < wd; i ++)
  {
    dst.setPixel(i + 1,0,uint16((3*src.getPixel(i,0) + src.getPixel(i,1) +
            2*(3*src.getPixel(i + 1,0) + src.getPixel(i + 1,1)) +
                     3*src.getPixel(i+2,0) + src.getPixel(i+2,1) + 8) / 16));
    dst.setPixel(i+1,ht + 1,uint16((src.getPixel(i,ht) + 3*src.getPixel(i,ht + 1) +
            2*(src.getPixel(i + 1,ht) + 3*src.getPixel(i + 1,ht + 1)) +
                        src.getPixel(i+2,ht) + 3*src.getPixel(i+2,ht + 1) + 8) / 16));
  }

  for(i = 0; i < ht; i ++)
  {
    dst.setPixel(0,i+1,uint16((3*src.getPixel(0,i) + src.getPixel(1,i) +
            2*(3*src.getPixel(0,i+1) + src.getPixel(1,i+1)) +
                     3*src.getPixel(0,i+2) + src.getPixel(1,i+2) + 8) / 16));
    dst.setPixel(wd + 1,i + 1,uint16((src.getPixel(wd,i) + 3*src.getPixel(wd + 1,i) +
            2*(src.getPixel(wd,i + 1) + 3*src.getPixel(wd+1,i + 1)) +
                        src.getPixel(wd,i+2) + 3*src.getPixel(wd+1,i+2) + 8) / 16));
  }
  
  for(y = ht; y; y--, base_rp_dst += ruc_dst)
  {
    //    Load up initial column sums
    left_cs = *top_rp++;
    curr_pel = *mid_rp++;
    left_cs += curr_pel + curr_pel;
    left_cs += *bot_rp++;
    
    cent_cs = *top_rp++;
    curr_pel = *mid_rp++;
    cent_cs += curr_pel + curr_pel;
    cent_cs += *bot_rp++;
    
    for(x = wd, dst_rp = base_rp_dst; x; x--)
    {
      right_cs = *top_rp++;
      curr_pel = *mid_rp++;
      right_cs += curr_pel + curr_pel;
      right_cs += *bot_rp++;
      
      new_pel = left_cs + cent_cs + cent_cs + right_cs;
      *dst_rp++ = (uint16)((new_pel+8) >> 4);
      
      left_cs = cent_cs;
      cent_cs = right_cs;
    }
    
    //    Update row pointers for next row iteration
    top_rp = save_rp;
    mid_rp = save_rp + ruc_src;
    bot_rp = mid_rp + ruc_src;
    save_rp = mid_rp;
  }
}



void UT_moreip::testReflect(int32 size)
{
  rcWindow fb (size *2 , size *2);  
  rcWindow src(fb, (fb.width() - size/2) / 2, 
	       (fb.height() - size) / 2, size / 2, size);

  for (int32 i = 0; i < src.height(); i++)
    {
      for (int32 j = 0; j < src.width(); j++)
	src.setPixel (j, i, i);
    }

  rcWindow fb2 (src.width () + 5, src.height() + 5);
  rcWindow dst (fb2, 2, 3, src.width(), src.height());

  rfImageVerticalReflect (src, dst);

  for (int32 i = 0; i < src.height(); i++)
    {
      for (int32 j = 0; j < src.width(); j++)
	rcUTCheck (dst.getPixel(j, i) == ((int32) (src.height() - i - 1)));
    }

  rcWindow fb3 (src.width () + 5, src.height() + 5);
  rcWindow dst2 (fb3, 3, 2, src.width(), src.height());
  dst2.copyPixelsFromWindow (src, true);

  for (int32 i = 0; i < src.height(); i++)
    {
      for (int32 j = 0; j < src.width(); j++)
	rcUTCheck (dst2.getPixel(j, i) ==  ((int32) (src.height() - i - 1)));
    }

  printSuccessMessage( "Basic Reflection unit test" , mErrors);     
}



void UT_moreip::testIntegral()
{
  {
  rcWindow src1(3, 2);
  
  src1.setPixel(0,0,1);
  src1.setPixel(1,0,2);
  src1.setPixel(2,0,3);
  src1.setPixel(0,1,3);
  src1.setPixel(1,1,2);
  src1.setPixel(2,1,1);

  rcWindow cp (3,2, rcPixel32);
  rcWindow rp (3,2, rcPixel32);

  rfImageIntegrals (src1, cp, rp);

  rcUTCheck (cp.getPixel(int32(0),int32(0)) == 1);
  rcUTCheck (cp.getPixel(int32(1),int32(0)) == 2);
  rcUTCheck (cp.getPixel(int32(2),int32(0)) == 3);
  rcUTCheck (cp.getPixel(int32(0),int32(1)) == 4);
  rcUTCheck (cp.getPixel(int32(1),int32(1)) == 4);
  rcUTCheck (cp.getPixel(int32(2),int32(1)) == 4);

  rcUTCheck (rp.getPixel(int32(0),int32(0)) == 1);
  rcUTCheck (rp.getPixel(int32(1),int32(0)) == 5);
  rcUTCheck (rp.getPixel(int32(2),int32(0)) == 14);
  rcUTCheck (rp.getPixel(int32(0),int32(1)) == 9);
  rcUTCheck (rp.getPixel(int32(1),int32(1)) == 13);
  rcUTCheck (rp.getPixel(int32(2),int32(1)) == 14);

  rcWindow csum (src1.width(), 1, rcPixel32);
  rcWindow rsum (src1.height(), 1, rcPixel32);
  rfImageOrthogonalProjections (src1, rsum, csum);

  rcUTCheck (csum.width () == src1.width());
  rcUTCheck (rsum.width () == src1.height());
  rcUTCheck (csum.getPixel(int32(0),int32(0)) == 4);
  rcUTCheck (csum.getPixel(int32(1),int32(0)) == 4);
  rcUTCheck (csum.getPixel(int32(2),int32(0)) == 4);
  rcUTCheck (rsum.getPixel(int32(0),int32(0)) == 6);
  rcUTCheck (rsum.getPixel(int32(1),int32(0)) == 6);
  

  printSuccessMessage( "Basic Pixel8Integral Unit test unit test" , mErrors); 
  }

  {
    rcWindow src1(3, 2, rcPixel16);
  
    src1.setPixel(0,0,1);
    src1.setPixel(1,0,2);
    src1.setPixel(2,0,3);
    src1.setPixel(0,1,3);
    src1.setPixel(1,1,2);
    src1.setPixel(2,1,1);

    rcWindow cp (3,2, rcPixel32);
    rcWindow rp (3,2, rcPixel32);

    rfImageIntegrals (src1, cp, rp);

    rcUTCheck (cp.getPixel(int32(0),int32(0)) == 1);
    rcUTCheck (cp.getPixel(int32(1),int32(0)) == 2);
    rcUTCheck (cp.getPixel(int32(2),int32(0)) == 3);
    rcUTCheck (cp.getPixel(int32(0),int32(1)) == 4);
    rcUTCheck (cp.getPixel(int32(1),int32(1)) == 4);
    rcUTCheck (cp.getPixel(int32(2),int32(1)) == 4);

    rcUTCheck (rp.getPixel(int32(0),int32(0)) == 1);
    rcUTCheck (rp.getPixel(int32(1),int32(0)) == 5);
    rcUTCheck (rp.getPixel(int32(2),int32(0)) == 14);
    rcUTCheck (rp.getPixel(int32(0),int32(1)) == 9);
    rcUTCheck (rp.getPixel(int32(1),int32(1)) == 13);
    rcUTCheck (rp.getPixel(int32(2),int32(1)) == 14);


    rcWindow csum (src1.width(), 1, rcPixel32);
    rcWindow rsum (src1.height(), 1, rcPixel32);
    rfImageOrthogonalProjections (src1, rsum, csum);

    rcUTCheck (csum.width () == src1.width());
    rcUTCheck (rsum.width () == src1.height());
    rcUTCheck (csum.getPixel(int32(0),int32(0)) == 4);
    rcUTCheck (csum.getPixel(int32(1),int32(0)) == 4);
    rcUTCheck (csum.getPixel(int32(2),int32(0)) == 4);
    rcUTCheck (rsum.getPixel(int32(0),int32(0)) == 6);
    rcUTCheck (rsum.getPixel(int32(1),int32(0)) == 6);
    printSuccessMessage( "Basic Pixel16Integral Unit test unit test" , mErrors); 

  }

  {
    rcIPair target (5, 5);
    rcWindow src1(6, 7, rcPixel16);
    src1.set (1);
    src1.setPixel (5,0,2);
    src1.setPixel (0,5,2);

	rcIRect field =  rfImageACfieldArea (src1, target);
    rcWindow tmp;
    rfImageACfield (src1, target, tmp);
	rcWindow ac (tmp, field.ul().x(), field.ul().y(), field.width (), field.height ());

	rcWindow tmp2;
    variance (src1, target, tmp2);
	rcWindow var (tmp2, field.ul().x(), field.ul().y(), field.width (), field.height ());
		
    rcUTCheck (var.contentCompare (ac));

    rcUTCheck (real_equal (*((float *) ac.pelPointer (0,0)), 0.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (1,0)), 24.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (0,1)), 24.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (1,1)), 0.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (0,2)), 24.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (1,2)), 0.0f, 0.0001f));

    printSuccessMessage( "Basic Small Pixel Variance Unit test unit test" , mErrors); 
  }


  {
    rcIPair target (5, 5);
    rcWindow src1(6, 7, rcPixel16);
    src1.set (999);
    src1.setPixel (5,0,1000);
    src1.setPixel (0,5, 1000);

	rcIRect field =  rfImageACfieldArea (src1, target);
    rcWindow tmp;
    rfImageACfield (src1, target, tmp);
	rcWindow ac (tmp, field.ul().x(), field.ul().y(), field.width (), field.height ());

	rcWindow tmp2;
    variance (src1, target, tmp2);
	rcWindow var (tmp2, field.ul().x(), field.ul().y(), field.width (), field.height ());

    rcUTCheck (var.contentCompare (ac));

    rcUTCheck (real_equal (*((float *) ac.pelPointer (0,0)), 0.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (1,0)), 24.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (0,1)), 24.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (1,1)), 0.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (0,2)), 24.0f, 0.0001f));
    rcUTCheck (real_equal (*((float *) ac.pelPointer (1,2)), 0.0f, 0.0001f));

    printSuccessMessage( "Basic Large Pixel Variance Unit test unit test" , mErrors); 
  }


{

	for (int32 depth = 1; depth < 3; depth++)
	{
		rcIPair target (7, 7);
		rcWindow src1(600, 700, (rcPixel) depth);
		uint32 maxAmp = (depth == 1) ? 255 : 2000;
		for (uint32 j = 0; j < src1.height (); j++)
			for (uint32 i = 0; i < src1.width (); i++)
			src1.setPixel (i, j, ((i+j) %2) ? 0 : maxAmp);

		rcIRect field =  rfImageACfieldArea (src1, target);
	    rcWindow tmp;
	    float maxvar = rfImageACfield (src1, target, tmp);
		rcWindow ac (tmp, field.ul().x(), field.ul().y(), field.width (), field.height ());

		rcWindow tmp2;
	    float truemax = variance (src1, target, tmp2);
		rcWindow var (tmp2, field.ul().x(), field.ul().y(), field.width (), field.height ());


		rcUTCheck (real_equal (maxvar, truemax, 0.1f));
		rcUTCheck (var.contentCompare (ac));
	}

	printSuccessMessage( "Basic Large Image Variance Unit test unit test" , mErrors); 
}


}

void UT_moreip::testHyst()
{
 // Test Hyst Thresholding
   char *shape[] =
     {
       "00000000",
       "00330300",
       "00043000",
       "03000300",
       "00000000",0};


   rcWindow ti;

   rfDrawShape (ti, shape);
   
   rcWindow dst;
   int32 left (0);
   rfHysteresisThreshold (ti, dst, 3, 4, left, -1, 6, 9);
}

void UT_moreip::testShift()
{
  rcWindow src1(30, 20);
  src1.setAllPixels (0);

  src1.setPixel(15,10,255);

  rcWindow shifted = rfCubicShiftImage (src1, 0.3, 0.7);

  rcUTCheck (shifted.width() == src1.width() - 3);
  rcUTCheck (shifted.height() == src1.height() - 3);

  //  rmPrintImage (shifted);
  
}


void UT_moreip::testExpand()
{
  rcWindow foo(10, 20);

  rcWindow win1 (foo,1, 1, 2, 2);
  win1.setPixel(0, 0, 0);
  win1.setPixel(1, 0, 10);
  win1.setPixel(0, 1, 1);
  win1.setPixel(1, 1, 11);

  rcWindow win3 = rfPixelExpand(win1, 2, 2);
  rcUTCheck (win3.size() == rcIPair(4, 4));

  rcUTCheck(win3.getPixel(0, 0) == 0);
  rcUTCheck(win3.getPixel(1, 0) == 0);
  rcUTCheck(win3.getPixel(2, 0) == 10);
  rcUTCheck(win3.getPixel(3, 0) == 10);
  rcUTCheck(win3.getPixel(0, 1) == 0);
  rcUTCheck(win3.getPixel(1, 1) == 0);
  rcUTCheck(win3.getPixel(2, 1) == 10);
  rcUTCheck(win3.getPixel(3, 1) == 10);
  rcUTCheck(win3.getPixel(0, 2) == 1);
  rcUTCheck(win3.getPixel(1, 2) == 1);
  rcUTCheck(win3.getPixel(2, 2) == 11);
  rcUTCheck(win3.getPixel(3, 2) == 11);
  rcUTCheck(win3.getPixel(0, 3) == 1);
  rcUTCheck(win3.getPixel(1, 3) == 1);
  rcUTCheck(win3.getPixel(2, 3) == 11);
  rcUTCheck(win3.getPixel(3, 3) == 11);

  rcUTCheck(win3.frameBuf() != win1.frameBuf());

  {
    rcWindow foo(100, 200);

    rcWindow bar1(foo,1, 1, 2, 2);
    bar1.setPixel(0, 0, 0);
    bar1.setPixel(1, 0, 10);
    bar1.setPixel(0, 1, 1);
    bar1.setPixel(1, 1, 11);

    rcWindow dest;

    rfPixelExpand(bar1, dest, 2, 3);
    rcUTCheck(dest.size() == rcIPair(4, 6));

    rcWindow bar2 (bar1, 0, 0, 1, 1);
    dest = rcWindow(3, 4);
    dest.setAllPixels (uint8(5));

    rfPixelExpand(bar2, dest, 2L, 2L);
    rcUTCheck(dest.getPixel(0, 0) == 0);
    rcUTCheck(dest.getPixel(1, 0) == 0);
    rcUTCheck(dest.getPixel(2, 0) == 5);
    rcUTCheck(dest.getPixel(0, 1) == 0);
    rcUTCheck(dest.getPixel(1, 1) == 0);
    rcUTCheck(dest.getPixel(2, 1) == 5);
    rcUTCheck(dest.getPixel(0, 2) == 5);
    rcUTCheck(dest.getPixel(1, 2) == 5);
    rcUTCheck(dest.getPixel(2, 2) == 5);
    rcUTCheck(dest.getPixel(0, 3) == 5);
    rcUTCheck(dest.getPixel(1, 3) == 5);
    rcUTCheck(dest.getPixel(2, 3) == 5);


  /* check cosizing -- dest larger than source */
    rcWindow bar3 (bar1,0, 0, 1, 1);
    dest = rcWindow(3, 3);
    dest.setAllPixels (uint8(5));
    rfPixelExpand(bar3, dest, 2, 2);
    rcUTCheck(dest.getPixel(0, 0) == 0);
    rcUTCheck(dest.getPixel(1, 0) == 0);
    rcUTCheck(dest.getPixel(2, 0) == 5);
    rcUTCheck(dest.getPixel(0, 1) == 0);
    rcUTCheck(dest.getPixel(1, 1) == 0);
    rcUTCheck(dest.getPixel(2, 1) == 5);
    rcUTCheck(dest.getPixel(0, 2) == 5);
    rcUTCheck(dest.getPixel(1, 2) == 5);
    rcUTCheck(dest.getPixel(2, 2) == 5);
  }

   printSuccessMessage( "Basic PixelExpand Unit test unit test" , mErrors);   
}



void UT_moreip::testSample()
{

  for (int32 i = 1; i < 3; i++)
    {
      rcPixel depth = (rcPixel) i;

      rcWindow img1(10, 20, depth);
      rcWindow win1 (img1,1, 1, 5, 8);

      // Set a useful pattern in the image
      rmSetPatternPixels(win1);  

      rcWindow win3 = rfPixelSample(win1, 2, 3);
      rcUTCheck(win3.size() == rcIPair(2, 2));

      rcUTCheck(win3.getPixel(0, 0) == 1);
      rcUTCheck(win3.getPixel(1, 0) == 21);
      rcUTCheck(win3.getPixel(0, 1) == 4);
      rcUTCheck(win3.getPixel(1, 1) == 24);

      rcUTCheck(win3.frameBuf() != win1.frameBuf());

      rcWindow win4 = rfPixelSample(win1, 1, 1);
      rcUTCheck(win4.size() == rcIPair(5, 8));
      rcUTCheck(win1.contentCompare (win4));



      rcWindow foo(100, 200, depth);

      rcWindow bar1(foo,1, 1, 5, 8);

      rmSetPatternPixels(bar1);

      rcWindow dest;

      rfPixelSample(bar1, dest, 2, 3);
      rcUTCheck(dest.size() == rcIPair(2, 2));
      rcUTCheck(dest.depth() == depth);

      /* Leftover Width */
      rcWindow dest1;
      bar1 = rcWindow (foo, 0, 0, 3, 8);
      rfPixelSample(bar1, dest1, 2, 3);
      rcUTCheck(dest1.size() == rcIPair(1, 2));

      /* Leftover Height */
      rcWindow dest2;
      bar1 = rcWindow (5, 5, depth);
      rfPixelSample(bar1, dest2, 2, 3);
      rcUTCheck(dest2.size() == rcIPair(2, 1));

      /* Source Larger */
      bar1 = rcWindow (foo, 1, 1, 3, 8);
      dest = rcWindow(1, 2, depth);
      rfPixelSample(bar1, dest, 2, 3);
      rcUTCheck(dest.size() == rcIPair(1, 2));
      rcUTCheck(dest.getPixel(0, 0) == 1);
      rcUTCheck(dest.getPixel(0, 1) == 4);

      /* In-place operation */
      dest = bar1;
      rfPixelSample(bar1, dest, 2, 3);
      rcUTCheck(dest.getPixel(0, 0) == 1);
      rcUTCheck (dest.getPixel(0, 1) == 4);
    }

  printSuccessMessage( "Basic PixelSample %d Unit test unit test" , mErrors);   
}


void UT_moreip::testTimes( bool useAltivec, int32 width, int32 height )
{
	width *= 4;
	height *= 4;

	const uint32 minPixels = 10000000;
	const uint32 pixels = width * height;
	// Adjust repeats depending on image size
	uint32 repeats = minPixels/pixels;

	if ( repeats < 1 )
		repeats = 1;
	else if ( repeats > 10000 )
		repeats = 10000;

	rcWindow imgA (width, height);
	rcWindow imgB (width/4, height/4);

	imgA.randomFill ();

	rcTime timer;

// Run test in a loop to diminish the effect of system clock
// inaccuracy. The time spent by the loop code should be negligible

	timer.start ();
	for( uint32 i = 0; i < repeats; ++i )
		rfPixelExpand (imgB, imgA, 4, 4); 
	timer.end ();

	double dMilliSeconds = timer.milliseconds () / repeats;
	double dMicroSeconds = timer.microseconds () / repeats;

// Per Byte in Useconds
	double perByte = dMicroSeconds / (imgA.width() * imgA.height() * imgA.depth());

	fprintf(stderr,
		"Performance: PixelExpand [%i x %i %i]: %.2f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
		imgB.width(), imgB.height(), imgB.depth()*8, dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);

// Run test in a loop to diminish the effect of system clock
// inaccuracy. The time spent by the loop code should be negligible

	timer.start ();
	for( uint32 i = 0; i < repeats; ++i )
		rcWindow shiftedB = rfCubicShiftImage (imgB, 0.3, 0.7);
	timer.end ();

	dMilliSeconds = timer.milliseconds () / repeats;
	dMicroSeconds = timer.microseconds () / repeats;

// Per Byte in Useconds
	perByte = dMicroSeconds / (imgB.width() * imgB.height() * imgB.depth());

	fprintf(stderr,
		"Performance: Cubic Image Shift [%i x %i %i]: %.2f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
		imgB.width(), imgB.height(), imgB.depth()*8, dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);

// Run test in a loop to diminish the effect of system clock
// inaccuracy. The time spent by the loop code should be negligible


	rcIPair target (7, 7);  

		timer.start ();
			rcWindow varImage;				
		for( uint32 i = 0; i < repeats; ++i )
		{
			variance (imgB, target, varImage);
		}
		timer.end ();


	dMilliSeconds = timer.milliseconds () / repeats;
	dMicroSeconds = timer.microseconds () / repeats;

// Per Byte in Useconds
	perByte = dMicroSeconds / (imgB.width() * imgB.height() * imgB.depth());

	fprintf(stderr,
		"Performance: ACField Generation [%i x %i %i]: %.2f ms, %.6f us/8bit pixel %.2f MB/s, %.2f fps\n",
	imgB.width(), imgB.height(), imgB.depth()*8, dMilliSeconds, perByte, 1 / perByte, 1000/dMilliSeconds);

}

static float variance (const rcWindow& image, const rcIPair site, rcWindow& floatdest)
{
	rcIRect field = rfImageACfieldArea (image, site);	
	rcWindow& dest = floatdest;
	rcWindow varImg;
	if (dest.isBound() && dest.size() == image.size () && dest.frameBuf()->isD32Float ())
    {
      varImg = rcWindow (dest, site.x() / 2, site.y() / 2, field.width(), field.height());
    }
  else
    {
      dest = rcWindow (image.width(), image.height(), rcPixel32);
      dest.frameBuf()->markD32Float (true);
      varImg = rcWindow (dest, field.ul().x(), field.ul().y(), field.width (), field.height());
    }
  
  dest.set (0.0);
  double N = site.x() * site.y();
  float maxvar (0.0f);

  for (uint32 j = 0; j < varImg.height(); j++)
    for (uint32 i = 0; i < varImg.width(); i++)
      {
	double sum (0.0), sumsq (0.0);
	for (uint32 p = 0; p < site.y(); p++)
	  for (uint32 q = 0; q < site.x(); q++)
	    {
	      uint32 pel = image.getPixel (i+q, j+p);
	      sum += pel;
	      sumsq += pel * pel;
	    }
	double var = (N*sumsq - sum*sum);
	float * fp = (float *) varImg.pelPointer (i, j);
	*fp = (float) var;
	if (var > maxvar) maxvar = var;
      }

  return maxvar;
}


  
