
/*
 *  File.
 *  
 *
 *	$Id: rc_ip.cpp 7299 2011-03-07 22:02:55Z arman $
 *
 *  
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include "rc_window.h"
#include "rc_rowfunc.h"
#include "rc_ip.h"
#include "rc_histstats.h"
#include "rc_mathmodel.h"


#include <vector>

using namespace std;

#define rmPrintFloatImage(a){					    \
    for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {  \
      fprintf (stderr, "\n");					    \
      float* vp = (float*)(a).rowPointer(i__temp);		    \
      for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)   \
	fprintf (stderr, "-%4.1f-", vp[j__temp]);				    \
      fprintf (stderr, "\n");					    \
    }}

#define rfHasSIMD false

static void rfGauss3by3 (const rcWindow& src, rcWindow& dest);
static void rfGauss163by3 (const rcWindow& src, rcWindow& dest);
static void rfGauss3(const rcWindow& src, rcWindow& dest);


static const short sGaussKernel3 [3][3] = {{1,2,1},{2,4,2},{1,2,1}};
static const short sGaussKernel5 [5][5] = {{1, 4, 6, 4, 1}, {4, 16, 24, 16, 4}, {6, 24, 36, 24, 6},
				    {4, 16, 24, 16, 4}, {1, 4, 6, 4, 1}};
static const float sGaussFloatKernel3 [3][3] = {{1/16.0f,2/16.0f,1/16.0f},{2/16.0f,4/16.0f,2/16.0f},{1/16.0f,2/16.0f,1/16.0f}};
static const float sGaussFloatKernel5 [5][5] = {{1/256.0f, 4/256.0f, 6/256.0f, 4/256.0f, 1/256.0f}, 
						{4/256.0f, 16/256.0f, 24/256.0f, 16/256.0f, 4/256.0f}, 
						{6/256.0f, 24/256.0f, 36/256.0f, 24/256.0f, 6/256.0f},
						{4/256.0f, 16/256.0f, 24/256.0f, 16/256.0f, 4/256.0f}, 
						{1/256.0f, 4/256.0f, 6/256.0f, 4/256.0f, 1/256.0f}};

/////////////// Gaussian Convolution Kernel Generation ///////////////////////////////


template <class T, int ksize>
rcGaussKernel2D<T,ksize>::rcGaussKernel2D ()
{
	if (ksize < 7)
	{
		for (int32 j = 0; j < ksize; j++)
			for (int32 i = 0; i < ksize; i++)
		{
			mKernel[j][i] = sGaussFloatKernel3[j][i];		
			mUSkernel[j][i] = sGaussKernel3[j][i];				
		}

		return;
	}

	rcWindow buf (ksize, ksize, rcPixel16);
	rcMathematicalImage g;
	g.gauss (buf, rcMathematicalImage::eHiPeak);
	rcHistoStats h (buf, 65536);
	double sum = h.mean () * h.n();
	for (int32 j = 0; j < buf.height (); j++)
		for (int32 i = 0; i < buf.width (); i++)
	{
		mKernel[j][i] = buf.getPixel(i, j) / sum;
		mUSkernel[j][i] = buf.getPixel(i, j);	
	}



}

template class rcGaussKernel2D<float,7>;
template class rcGaussKernel2D<float,9>;
template class rcGaussKernel2D<float,11>;
template class rcGaussKernel2D<float,13>;
template class rcGaussKernel2D<float,15>;
template class rcGaussKernel2D<float,17>;

rcGaussKernel2D<float,7> sGaussFloatKernel7;
rcGaussKernel2D<float,9> sGaussFloatKernel9;
rcGaussKernel2D<float,11> sGaussFloatKernel11;
rcGaussKernel2D<float,13> sGaussFloatKernel13;
rcGaussKernel2D<float,15> sGaussFloatKernel15;
rcGaussKernel2D<float,17> sGaussFloatKernel17;
#define LOOP4KERNEL(a)\
case (a):\
if (rfHasSIMD () && kernelSize == (a) && src.depth() == rcPixel16)\
{\
	vImage_Buffer s16, sf, d16, df;\
	src.vImage (s16);\
	dest.vImage (d16);\
	rcWindow f (src.width(), dest.height(), rcPixel32S);\
	f.vImage (sf);\
	rcWindow ff (src.width(), dest.height(), rcPixel32S);\
	ff.vImage (df);\
	vImage_Error ve;       \
	ve = vImageConvert_16UToF (&s16, &sf, 0.0f, 1.0f, kvImageNoFlags);\
	rmAssert (!ve);\
	ve = vImageConvolve_PlanarF( &sf, &df, NULL, \
		0, 0, sGaussFloatKernel##a.kernel (),\
		(a), (a), 0, kvImageEdgeExtend);\
	rmAssert (!ve);\
	ve = vImageConvert_FTo16U (&df, &d16, 0.0f, 1.0f, kvImageNoFlags);\
	rmAssert (!ve);\
	return;\
}

/////////////// Gaussian Convolution  ///////////////////////////////

// 
// The client passes in a bounded rcWindow of the same sizs as source.
// Upon return dest is a window in to the image client passed with the
// correct size

   /*
    * Run Gauss around the edges
    */
template<class P>
void gaussEdge (P* pixelPtr, const rcWindow& src, rcWindow& dest)
{
   const uint32 srcWidth(src.width());
   const uint32 srcHeight(src.height());
   const uint32 srcUpdate(src.rowPixelUpdate());
   const uint32 destUpdate(dest.rowPixelUpdate());
   const P *src0 = pixelPtr;
   P* dest0 = (P *) dest.rowPointer(0);

   // TL and TR pixels
   *dest0 = *src0;
   dest0[srcWidth - 1] = src0[srcWidth - 1];
   dest0++;

   // Top
   uint32 left(uint32 (3* *src0 + *(src0 + srcUpdate)));
   src0++;
   uint32 centre(uint32 (3* *src0 + *(src0 + srcUpdate)));
   src0++;
   uint32 width;
   for(width = srcWidth - 2; width--;)
   {
      uint32 right(uint32 (3* *src0 + *(src0 + srcUpdate)));
      *dest0++ = P ((left + centre + centre + right + 8 )/ 16);
      left = centre;
      centre = right;
      src0++;
   }

   // BL and BR pixels
   dest0 = (P *) dest.rowPointer (srcHeight - 1);
   src0 = (P *) src.rowPointer (srcHeight - 1);
   *dest0 = *src0;
   dest0[srcWidth - 1] = src0[srcWidth - 1];
   dest0++;

   // Bottom
   left = uint32 (3* *src0 + *(src0 - srcUpdate));
   src0++;
   centre = uint32 (3* *src0 + *(src0 - srcUpdate));
   src0++;
   for(width = srcWidth - 2; width--;)
   {
      uint32 right(uint32 (3* *src0 + *(src0 - srcUpdate)));

      *dest0++ = P ((left + centre + centre + right + 8 )/ 16);
      left = centre;
      centre = right;
      src0++;
   }

   // Left
   src0 = (P *) src.rowPointer (0);
   dest0 = (P *) dest.rowPointer (1);

   uint32 top(uint32 (3* *src0 + *(src0+1)));
   src0 += srcUpdate;
   uint32 middle(uint32 (3* *src0 + *(src0+1)));
   src0 += srcUpdate;

   uint32 height;
   for(height = srcHeight - 2; height--;)
   {
      uint32 bottom(uint32 (3* *src0 + *(src0 + 1)));
      *dest0 = P ((top + middle + middle + bottom + 8)/16);
      src0 += srcUpdate;
      dest0 += destUpdate;
      top = middle;
      middle = bottom;
   }

   // Right
   src0 = (P *) src.rowPointer (0) + srcWidth - 1;
   dest0 = (P *) dest.rowPointer (1) + srcWidth - 1;
   top = uint32 (3* *src0 + *(src0 - 1));
   src0 += srcUpdate;
   middle = uint32 (3* *src0 + *(src0 - 1));
   src0 += srcUpdate;

   for(height = srcHeight - 2; height--;)
   {
      uint32 bottom(uint32 (3* *src0 + *(src0 - 1)));
      *dest0 = P ((top + middle + middle + bottom + 8)/16);
      src0 += srcUpdate;
      dest0 += destUpdate;
      top = middle;
      middle = bottom;
   }
}


void rfGaussianConv (const rcWindow& src, rcWindow& dest, int32 kernelSize) 
{
	assert(kernelSize >= 3);
	assert(kernelSize % 2 == 1);
	assert(dest.width() == src.width());
	assert(dest.height() == src.height());
	assert(dest.depth() == src.depth());
	assert(&src != &dest);

	const uint32 width(src.width());
	const uint32 height(src.height());

	assert (width >= kernelSize && height >= kernelSize);

#ifdef VIMAGE_LARGE_KERNEL_TEST_OK
	switch (kernelSize)
	{
		LOOP4KERNEL(7);		
		LOOP4KERNEL(9);	
		LOOP4KERNEL(11);	
		LOOP4KERNEL(13);	
		LOOP4KERNEL(15);
	}
#endif

	if (rfHasSIMD  && kernelSize == 5)
	{
		if (src.depth() == rcPixel8)
		{
			vImage_Buffer vb, vd;
			src.vImage (vb);	  dest.vImage (vd);
			vImage_Error ve;
			ve = vImageConvolve_Planar8(&vb, &vd, NULL,
				0, 0, sGaussKernel5[0],
				5, 5, 256, 0,
				kvImageEdgeExtend);
			rmAssert (!ve);
			return;
		}
		else   if (src.depth() == rcPixel16)
		{
	// Convert 16 to float
			vImage_Buffer s16, sf, d16, df;
			src.vImage (s16);
			dest.vImage (d16);
			rcWindow f (src.width(), dest.height(), rcPixel32S);
			f.vImage (sf);
			rcWindow ff (src.width(), dest.height(), rcPixel32S);
			ff.vImage (df);
			vImage_Error ve;       
			ve = vImageConvert_16UToF (&s16, &sf, 0.0f, 1.0f, kvImageNoFlags);
			rmAssert (!ve);
			ve = vImageConvolve_PlanarF( &sf, &df, NULL, 
				0, 0, sGaussFloatKernel5[0],
				5, 5, 0, kvImageEdgeExtend);
			rmAssert (!ve);
			ve = vImageConvert_FTo16U (&df, &d16, 0.0f, 1.0f, kvImageNoFlags);      
			rmAssert (!ve);
			return;
		}
		else if (src.depth() == rcPixelFloat )
		{
			vImage_Buffer vb, vd;
			src.vImage (vb);	  dest.vImage (vd);
			vImage_Error ve;
			ve = vImageConvolve_PlanarF(&vb, &vd, NULL,
				0, 0, sGaussFloatKernel5[0],
				5, 5, 0, kvImageEdgeExtend);
			rmAssert (!ve);
			return;
		}
		else if (src.depth() == rcPixel32S)
		{
			unsigned char edgeFill[4] = { 0, 0, 0, 0 };
			vImage_Buffer vb, vd;
			src.vImage (vb);	  dest.vImage (vd);
			vImage_Error ve;
			ve = vImageConvolve_ARGB8888(&vb, &vd, NULL,
				0, 0, sGaussKernel5[0],
				5, 5, 25, edgeFill, kvImageEdgeExtend);
			rmAssert (!ve);
			return;
		}
	}

	if(kernelSize == 3)
	{
		rfGauss3(src,dest);
		return;
	}

	rcWindow work(width,height, src.depth ());

// Ping pong between dest and work but make sure you end up in dest
	rcWindow& dest1 =  (kernelSize % 4 == 1) ? work : dest;
	rcWindow& dest2 =  (kernelSize % 4 == 3) ? work : dest;

	rfGauss3 (src,dest1);

	while(1)
	{
		kernelSize -= 2;

		rfGauss3 (dest1, dest2);
		if(kernelSize == 3)
			return;

		kernelSize -= 2;

		rfGauss3 (dest2, dest1);
		if(kernelSize == 3)
			return;
	}

}

// Gaussuan Smoothing
// 3x3 Approximation to Gaussian. Note that the first and last row/column will not
// be used. Destination is 2 rows and columns smaller than source



static void rfGauss3(const rcWindow& src, rcWindow& dest)
{
   assert(src.width() >= 3);
   assert(src.height() >= 3);
   assert(src.width() == dest.width());
   assert(src.height() == dest.height());

   // Handle by vImage 
   if (rfHasSIMD ) 
     {
     if (src.depth() == rcPixel8)
       {
	 vImage_Buffer vb, vd;
	 src.vImage (vb);	  dest.vImage (vd);
	 vImage_Error ve;
	 ve = vImageConvolve_Planar8(&vb, &vd, NULL,
				     0, 0, sGaussKernel3[0],
				     3, 3, 16, 0,
				     kvImageEdgeExtend);
	 rmAssert (!ve);
	 return;
       }
     else   if (src.depth() == rcPixel16)
       {
	 // Convert 16 to float
	 vImage_Buffer s16, sf, d16, df;
	 src.vImage (s16);
	 dest.vImage (d16);
	 rcWindow f (src.width(), dest.height(), rcPixel32S);
	 f.vImage (sf);
	 rcWindow ff (src.width(), dest.height(), rcPixel32S);
	 ff.vImage (df);
	 vImage_Error ve;       
	 ve = vImageConvert_16UToF (&s16, &sf, 0.0f, 1.0f, kvImageNoFlags);
	 rmAssert (!ve);
	 ve = vImageConvolve_PlanarF( &sf, &df, NULL, 
				      0, 0, sGaussFloatKernel3[0],
				      3, 3, 0, kvImageEdgeExtend);
	 rmAssert (!ve);
	 ve = vImageConvert_FTo16U (&df, &d16, 0.0f, 1.0f, kvImageNoFlags);      
	 rmAssert (!ve);
	 return;
       }
     else if (src.depth() == rcPixelFloat )
       {
	 vImage_Buffer vb, vd;
	 src.vImage (vb);	  dest.vImage (vd);
	 vImage_Error ve;
	 ve = vImageConvolve_PlanarF(&vb, &vd, NULL,
				     0, 0, sGaussFloatKernel3[0],
				     3, 3, 0, kvImageEdgeExtend);
	 rmAssert (!ve);
	 return;
       }
     else if (src.depth() == rcPixel32S)
       {
	 unsigned char edgeFill[4] = { 0, 0, 0, 0 };
	 vImage_Buffer vb, vd;
	 src.vImage (vb);	  dest.vImage (vd);
	 vImage_Error ve;
	 ve = vImageConvolve_ARGB8888(&vb, &vd, NULL,
				      0, 0, sGaussKernel3[0],
				      3, 3, 0, edgeFill, kvImageEdgeExtend);
	 rmAssert (!ve);
	 return;
       }
     }


   // This windowing pushes the unaligned version to be run
   // TBD: fix the perm vectors or use and aligned dst and copy
   //      Or use the entire dest but write offset from alignment

   rcWindow window (dest, 1, 1, dest.width() - 2, dest.height() - 2);

   switch (src.depth ())
     {
     case rcPixel8:
       rfGauss3by3(src, window);
       gaussEdge<uint8> ((uint8 *) src.rowPointer (0), src, dest);
       break;
     case rcPixel16:
       rfGauss163by3(src, window);
       gaussEdge<uint16> ((uint16 *) src.rowPointer (0), src, dest);
       break;
     default:
       rmExceptionMacro(<< "IP Window::init");
     }       
}


inline void gaussCompute (const uint8 *& top,
                          const uint8 *& mid,
                          const uint8 *& bot,
                          uint8 *&  dst,
                          uint32 &left, uint32 &cent, uint32 &right)
{
   uint32 new_pel;
   right = uint32 (*top++);
   uint8 curr_pel = *mid++;
   right += uint32 (curr_pel + curr_pel);
   right += uint32 (*bot++);
   new_pel = left + cent + cent + right;
   *dst++ = (uint8)((new_pel+8) >> 4);
}


inline void gauss16Compute (const uint16 *& top,
                          const uint16 *& mid,
                          const uint16 *& bot,
                          uint16 *&  dst,
                          uint32 &left, uint32 &cent, uint32 &right)
{
   uint32 new_pel;
   right = uint32 (*top++);
   uint16 curr_pel = *mid++;
   right += uint32 (curr_pel + curr_pel);
   right += uint32 (*bot++);
   new_pel = left + cent + cent + right;
   *dst++ = (uint16)((new_pel+8) >> 4);
}


static void rfGauss3by3 (const rcWindow& src, rcWindow& dest)
{
   assert(src.width() == dest.width() + 2);
   assert(src.height() == dest.height() + 2);

   uint32 ht(dest.height());
   uint32 wd(dest.width());
   uint32 x, y, sum1, sum2, sum3;
   uint32 kernel_wds, k;
   uint32 srcUpdate(src.rowPixelUpdate());
   uint32 destUpdate(dest.rowPixelUpdate());

   uint8 *base_dst = dest.rowPointer(0);
   uint8 *dst;

   const uint8 *top, *mid, *bot, *save;
   uint8 curr_pel;

   //  Initialize pointers to rows
   top = src.rowPointer(0);
   mid = top + srcUpdate;
   bot = mid + srcUpdate;
   save = mid;

   //  Compute number of kernel multiples contained in image width
   kernel_wds = wd / 3;
   k = wd % 3;

   for(y = ht; y; y--, base_dst += destUpdate)
   {
      //    SETUP FOR FIRST KERNEL POSITION:
      dst = base_dst;
      sum1 = uint32 (*top++);
      curr_pel = *mid++;
      sum1 += uint32 (curr_pel + curr_pel);
      sum1 += uint32 (*bot++);

      sum2 = uint32 (*top++);
      curr_pel = *mid++;
      sum2 += uint32 (curr_pel + curr_pel);
      sum2 += uint32 (*bot++);

      // Fill up the pipeline

      for(x = kernel_wds; x; x--)
      {
         gaussCompute(top,mid,bot,dst,sum1, sum2, sum3);

         gaussCompute(top,mid,bot,dst,sum2, sum3, sum1);

         gaussCompute(top,mid,bot,dst,sum3, sum1, sum2);
      }

      //    Compute any leftover pixels on the row
      for(x = k; x; x--)
      {
         gaussCompute (top,mid,bot,dst,sum1, sum2, sum3);
         sum1 = sum2;
         sum2 = sum3;
      }

      //    Update row pointers for next row iteration
      top = save;
      mid = save + srcUpdate;
      bot = mid + srcUpdate;
      save = mid;
   }
}



static void rfGauss163by3 (const rcWindow& src, rcWindow& dest)
{
   assert(src.width() == dest.width() + 2);
   assert(src.height() == dest.height() + 2);

   uint32 ht(dest.height());
   uint32 wd(dest.width());
   uint32 x, y, sum1, sum2, sum3;
   uint32 kernel_wds, k;
   uint32 srcUpdate(src.rowPixelUpdate());
   uint32 destUpdate(dest.rowPixelUpdate());

   uint16 *base_dst = (uint16 *) dest.rowPointer(0);
   uint16 *dst;

   const uint16 *top, *mid, *bot, *save;
   uint16 curr_pel;

   //  Initialize pointers to rows
   top = (uint16 *) src.rowPointer(0);
   mid = top + srcUpdate;
   bot = mid + srcUpdate;
   save = mid;

   //  Compute number of kernel multiples contained in image width
   kernel_wds = wd / 3;
   k = wd % 3;

   for(y = ht; y; y--, base_dst += destUpdate)
   {
      //    SETUP FOR FIRST KERNEL POSITION:
      dst = base_dst;
      sum1 = uint32 (*top++);
      curr_pel = *mid++;
      sum1 += uint32 (curr_pel + curr_pel);
      sum1 += uint32 (*bot++);

      sum2 = uint32 (*top++);
      curr_pel = *mid++;
      sum2 += uint32 (curr_pel + curr_pel);
      sum2 += uint32 (*bot++);

      // Fill up the pipeline

      for(x = kernel_wds; x; x--)
      {
         gauss16Compute(top,mid,bot,dst,sum1, sum2, sum3);

         gauss16Compute(top,mid,bot,dst,sum2, sum3, sum1);

         gauss16Compute(top,mid,bot,dst,sum3, sum1, sum2);
      }

      //    Compute any leftover pixels on the row
      for(x = k; x; x--)
      {
         gauss16Compute (top,mid,bot,dst,sum1, sum2, sum3);
         sum1 = sum2;
         sum2 = sum3;
      }

      //    Update row pointers for next row iteration
      top = save;
      mid = save + srcUpdate;
      bot = mid + srcUpdate;
      save = mid;
   }
}




/////////////// PixelMaps  ///////////////////////////////

// A class to encapsulate a table of inverted 8 bit pixel values
template<class T, uint32 s>
class rcInvertTable
{
public:
  // ctor
  rcInvertTable()
  {
    mTable.resize (s);
    for (uint32 i = 0; i < s; i++)
      mTable[i] = (T) ((s-1) - i);
  };

  const vector<T>& table () const
  {
    return mTable;
  }
    
private:
  vector<T> mTable;
};

template<class T, uint32 s>
class rcIdentityTable
{
public:
  // ctor
  rcIdentityTable()
  {
    mTable.resize (s);
    for (uint32 i = 0; i < s; i++)
      mTable[i] = (T) (i);
  };

  const vector<T>& table () const
  {
    return mTable;
  }
    
private:
  vector<T> mTable;
};

static const rcInvertTable<uint8, 256> sInvertTable;
static const rcIdentityTable<uint16, 65536> sIdentityTable;

void rfPixel8Map (const rcWindow& src, rcWindow& dst, const vector<uint8>& lut)
{

  assert (lut.size() == 256);
  assert (src.isBound());
  assert (dst.isBound());
  assert (src.depth() == rcPixel8);
  assert (dst.depth() == rcPixel8);

  // Direct all unaligned src and destinations to the basic calculations
  if (rfHasSIMD )
       {
	 vImage_Buffer vb, vd;
	 src.vImage (vb);	  dst.vImage (vd);
	 vImage_Error ve;
	 ve = vImageTableLookUp_Planar8(&vb, &vd, &lut[0],kvImageEdgeExtend);
	 rmAssert (!ve);
	 return;
       }
    else
      {
	rcBasicPixelMap<uint8> p8 (src, dst, lut);
	p8.areaFunc ();
      }
}

//@func rfPixel8Invert
void rfPixel8Invert (const rcWindow& src, rcWindow& dst)
{
  rfPixel8Map (src, dst, sInvertTable.table ());
}


void rfPixel16Map (const rcWindow& src, rcWindow& dst, const vector<uint16>& lut)
{
  rcBasicPixelMap<uint16> p16 (src, dst, lut);
  p16.areaFunc ();
  
}

void rfPixel16Map (const rcWindow& src, rcWindow& dst, const uint16 lessIsZero)
{
  vector<uint16> acopy (sIdentityTable.table());
  vector<uint16>::iterator lzb = acopy.begin();
  vector<uint16>::iterator lze = lzb; std::advance (lze, (uint32) lessIsZero);
  while (lzb != lze) *lzb++ = 0;
  rfPixel16Map (src, dst, acopy);
  
}

