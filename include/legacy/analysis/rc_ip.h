/*
 *  File.
 *  
 *
 *	$Id: rc_ip.h 7297 2011-03-07 00:17:55Z arman $
 *
 *  
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcIP_H_
#define _rcIP_H_

#include <rc_window.h>
#include <rc_vector2d.h>
#include <rc_macro.h>
//
// Effect: Computes the Gaussian filtered of an image.
void rfGaussianConv (const rcWindow& src, rcWindow& dest, int32 kernelSize);

void rfPixel8Map (const rcWindow& src, rcWindow& dst, const vector<uint8>& lut);

void rfPixel16Map (const rcWindow& src, rcWindow& dst, const vector<uint16>& lut);

void rfPixel8Invert (const rcWindow& src, rcWindow& dst);

void rfPixelExpand(const rcWindow& src, rcWindow& dest,  int32 xmag, int32 ymag);

rcWindow rfPixelExpand(const rcWindow& src, int32 xmag, int32 ymag);

void rfPixelSample(const rcWindow& src, rcWindow& dest,  int32 xskip, int32 yskip);

rcWindow rfPixelSample(const rcWindow& src, int32 xskip, int32 yskip);

rcWindow rfCubicShiftImage (rcWindow& src, double, double);

void rfPixelMin3By3(const rcWindow& srcImg, rcWindow& dstImg);

rcWindow rfPixelMin3By3 (const rcWindow& image);

void rfPixelMax3By3(const rcWindow& srcImg, rcWindow& dstImg);

rcWindow rfPixelMax3By3 (const rcWindow& image);

void rfPixel8Min3By3(const rcWindow& srcImg, rcWindow& dstImg);

rcWindow rfPixel8Min3By3 (const rcWindow& image);

void rfPixel8Max3By3(const rcWindow& srcImg, rcWindow& dstImg);

rcWindow rfPixel8Max3By3 (const rcWindow& image);

void rfMedian (const rcWindow& srcImg, rcWindow& dstImg);

rcWindow rfPixelMedian3x3 (const rcWindow& image);

void rfImageVerticalReflect (const rcWindow& src, rcWindow& dst);

float rfSurfaceHessian (rcWindow& src);

void rfHysteresisThreshold (const rcWindow& magImage,
														rcWindow& dst,
														uint8 magLowThreshold, uint8 magHighThreshold,
														int32& nSurvivors, int32 nPels, 
														uint8 outVal,uint8 inVal);

void rfCopyWindowBorder(rcWindow& win, const rcWindow& src);

/** 
 * Two-dimensional Gaussian filter kernel
 * @todo finish support for variances
 */

template<class T, int ksize=3>
class rcGaussKernel2D
{
public:
  /**
   * constructor
   * @param size is the dimension: size x size kernel
   * @param theVariance variance of the kernel.  @todo negative
	 
	 */
  rmStaticConstMacro(size, int, ksize);
  
  rcGaussKernel2D();
	
  /** 
   * returns a pointer to the kernel
   */
  const T* kernel () const
  {
    return mKernel[0];
  }
	
  const int16* usKernel () const
	{
		return mUSkernel[0];
	}
private:
  double mVariance;
  T mKernel[ksize][ksize];
  int16 mUSkernel [ksize][ksize];
};




class rcLNTable
{
public:
  // ctor
  rcLNTable(int32 n)
  {
    mTable.resize (256);
    for (int32 i = 0; i < n; i++)
      mTable[i] = (int32) (32 * log2 (1 + i));
		
  };
	
  const vector<uint8>& lut () const
  {
    return mTable;
  }
	
private:
  vector<uint8>mTable;
};


template<class T>
int32 rfGradHist1d(const T *first, const T *last)
{
  int32 length = last - first + 1; //@note pointer difference. We do one less
  rmAssert(length > 0);
  const uint32 opsPerLoop = 8;
  uint32 unrollCnt = length / opsPerLoop;
  uint32 unrollRem = length % opsPerLoop;
  int32 diffsum = 0;
	
  const T *pixelPtr = first;
	
  for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
	{
		diffsum += (pixelPtr[0] != pixelPtr[1]); 
		diffsum += (pixelPtr[1] != pixelPtr[2]); 
		diffsum += (pixelPtr[2] != pixelPtr[3]); 
		diffsum += (pixelPtr[3] != pixelPtr[4]); 
		diffsum += (pixelPtr[4] != pixelPtr[5]); 
		diffsum += (pixelPtr[5] != pixelPtr[6]); 
		diffsum += (pixelPtr[6] != pixelPtr[7]); 
		diffsum += (pixelPtr[7] != pixelPtr[8]); 
		pixelPtr += opsPerLoop;
	}
	
  for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++)
	{ diffsum += (pixelPtr[0] != pixelPtr[1]); pixelPtr++;}
	
  return diffsum;
}

#endif // _rcIP_H_
