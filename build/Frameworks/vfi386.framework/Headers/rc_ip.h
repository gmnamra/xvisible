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

#include "rc_window.h"
#include "rc_vector2d.h"

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



/** 
 *   \enum rcChannelConversion
 *   Conversion Options 
 */

/** 
 *   \var rcChannelConversion::rcSelect12BitCamera
 *   Selects 12 bit range for a 16 bit image
 */

enum rcChannelConversion
{
    rcSelectGreen = 0,
    rcSelectRed,
    rcSelectBlue,
    rcSelectAverage,
    rcSelectMax,
    rcSelect12BitCamera,
    rcSelectAll   // Keep all channels
};


/*! \fn void rfImageConvert8888ToARGB (vector<rcWindow>& iargb, rcWindow& argb);
 *  \brief Convert 4 8bit images to ARGB
 *  \param iargb a vector of rcWindows
 *  \param argb is a 4 byte rcWindow
 *  \return void 
 */ 
void rfImageConvert8888ToARGB (vector<rcWindow>& iargb, rcWindow& argb);
void rfImageConvertARGBto8888 (rcWindow& argb, vector<rcWindow>& iargb);

void rfImageConvert8ToARGB (const rcWindow& byteImage, const rcWindow& alpha, rcWindow& argb);
rcWindow rfImageConvert8ToARGB (const rcWindow& byteImage, const rcWindow& alpha);


void rfImageConvertFloat8 (const rcWindow& floatImage, rcWindow& byteImage,
                           float minVal = 0.0f, float maxVal = 1.0f);
rcWindow rfImageConvertFloat8 (const rcWindow& floatImage, float minVal = 0.0f, float maxVal = 1.0f);

/*! \fn void rfImageConvert816 (const rcWindow& byteImage, rcWindow& two byteImage)
 *  \brief Convert 8 bit images to 16 bit ones
 *  \param twoByteImage is a 16 bit rcWindow
 *  \param byteImage is a single byte rcWindow
 *  \return void 
 */ 

void rfImageConvert816 (const rcWindow& byteImage, rcWindow& twobyteImage);
rcWindow rfImageConvert816 (const rcWindow& byteImage);

/*! \fn void rfImageConvert168 (const rcWindow& twobyteImage, rcWindow& byteImage, rcChannelConversion opt)
 *  \brief Convert 16 bit images to 8 bit ones
 *  \param twoByteImage is a 16 bit rcWindow
 *  \param byteImage is a single byte rcWindow
 *  \param opt is an enum of type rcChannelConversion 
 *  \return void 
 */ 

void rfImageConvert168 (const rcWindow& twobyteImage, rcWindow& byteImage, 
                        rcChannelConversion opt = rcSelectMax);
rcWindow rfImageConvert168 (const rcWindow& twobyteImage, rcChannelConversion opt = rcSelectMax);

// Create a 8 bit gray scale image from an image stored in a color format
void rfImageConvert32to8( const rcWindow& rgbInput, rcWindow& channelOutput, rcChannelConversion opt = rcSelectAverage);

rcWindow rfImageConvert32to8( const rcWindow& rgbInput, rcChannelConversion opt = rcSelectAverage);

// Create a vector of 8 bit gray scale images from a vector of images stored in a color format
void rfImageConvert32to8( const vector<rcWindow>& rgbInput, vector<rcWindow>& channelOutput, 
                         rcChannelConversion opt = rcSelectAverage);
// Create a 32-bit image from 8-bit image
void rfImageConvert8to32(const rcWindow& rgbInput, rcWindow& rgbOutput);

rcWindow rfImage8Transpose (const rcWindow& image);


/*! \fn void rfRc* Functions are depracated. Use the above. 
 *  \brief Older and slower conversion routines
 */ 

// Create a 8 bit gray scale image from an image stored in a color format
void rfRcWindow32to8( const rcWindow& rgbInput, rcWindow& channelOutput, rcChannelConversion opt = rcSelectAverage);
// Create a vector of 8 bit gray scale images from a vector of images stored in a color format
void rfRcWindow32to8( const vector<rcWindow>& rgbInput, vector<rcWindow>& channelOutput, rcChannelConversion opt = rcSelectAverage);
// Create a 32-bit image from 8-bit image
void rfRcWindow8to32(const rcWindow& rgbInput, rcWindow& rgbOutput);

// Create an 8 bit image from a 16 bit image. Assumed to have data in the range 0 - 4095 (12 bits)
void rfRcWindow16to8( const rcWindow& rgbInput, rcWindow& channelOutput);

// Reverse 8-bit pixel values (pixel value P becomes 255-P)
void rfReversePixels8(rcWindow& image);

// Reverse 16-bit pixel values (pixel value P becomes 255-P)
void rfReversePixels16(rcWindow& image);

// Is white color at color index 0
bool rfIsWhiteReversed(const rcWindow& image);

// set mask pixels to zero if mask is 255
void rfImageAddMask (const rcWindow& image, const rcWindow& mask, rcWindow& fourbyteImage, uint32 maskVal);
rcWindow rfImageAddMask (const rcWindow& image, const rcWindow& mask, uint32 maskVal);


#endif // _rcIP_H_
