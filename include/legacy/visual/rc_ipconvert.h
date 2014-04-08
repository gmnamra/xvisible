/* @files
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.5  2005/09/13 14:46:24  arman
 *16bit support
 *
 *Revision 1.4  2005/04/17 22:06:00  arman
 *oops incorrect name
 *
 *Revision 1.3  2005/04/17 22:03:04  arman
 *added transpose (need to add minimal ut)
 *
 *Revision 1.2  2005/03/30 00:14:24  arman
 *added argb to 4 planar8 frames
 *
 *Revision 1.1  2005/01/28 02:18:08  arman
 *Moved to Visual
 *
 *Revision 1.9  2005/01/21 18:27:48  arman
 *added more composing layers support
 *
 *Revision 1.8  2005/01/19 22:55:06  arman
 *added 8ToARGB
 *
 *Revision 1.7  2005/01/14 16:28:14  arman
 *added vImage supported float8 support
 *
 *Revision 1.6  2005/01/11 03:29:07  arman
 *added vImage support
 *
 *Revision 1.5  2005/01/09 20:59:32  arman
 *added support for 16bit. Removed shift
 *
 *Revision 1.4  2005/01/09 01:47:42  arman
 *removed shift from all api
 *
 *Revision 1.3  2005/01/09 01:32:46  arman
 *removed shift from 168 api
 *
 *Revision 1.2  2005/01/07 16:25:22  arman
 **** empty log message ***
 *
 *Revision 1.1  2002/12/31 22:16:41  arman
 *Image format conversion routines
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcIPCONVERT_H
#define __rcIPCONVERT_H

#include <rc_window.h>
#include <rc_imageprocessing.h>



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

#endif /* __rcIPCONVERT_H */
