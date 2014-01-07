/*
 *
 *$Id $
 *$Log$
 *Revision 1.14  2005/10/27 22:58:19  arman
 *z additions and cleanup
 *
 *Revision 1.13  2005/09/12 15:28:01  arman
 *Pre 2.0
 *
 *Revision 1.12  2005/09/07 02:48:44  arman
 *fixed a bug in reflect. Added 32 bit support
 *
 *Revision 1.11  2005/08/30 20:57:16  arman
 *Cell Lineage
 *
 *Revision 1.14  2005/08/23 23:32:32  arman
 *Cell Lineage II
 *
 *Revision 1.13  2005/08/15 20:29:38  arman
 *Cell Lineage II
 *
 *Revision 1.12  2005/08/15 12:58:00  arman
 *Cell Lineage II
 *
 *Revision 1.11  2005/08/15 12:51:18  arman
 *added 16bit morphology support (using vImage)
 *
 *Revision 1.10  2005/03/29 16:10:46  arman
 *fixed rowupdate bug in doubel specialization of rfSetWindowBorder
 *
 *Revision 1.9  2004/07/21 21:22:52  arman
 *added reflect
 *
 *Revision 1.8  2004/04/20 15:41:24  arman
 *added rcCopyWindowBorder
 *
 *Revision 1.7  2004/04/19 20:49:49  arman
 *added borderCopy
 *
 *Revision 1.6  2003/12/19 06:58:04  arman
 *added specialization for double for SetWindowBorder
 *
 *Revision 1.5  2003/08/28 02:34:15  arman
 *unsigned to signed fixes
 *
 *Revision 1.4  2003/05/06 22:06:24  sami
 *Silenced compiler warning
 *
 *Revision 1.3  2003/05/06 22:04:44  sami
 *Silenced compiler warnings
 *
 *Revision 1.2  2003/03/03 14:10:35  arman
 *Added sample and expand
 *
 *Revision 1.1  2003/01/07 03:00:06  arman
 *Additional image processing functions
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include <rc_window.h>
#include <rc_math.h>
#include <rc_macro.h>
#include <rc_ip.h>

static const short sFlatKernel3 [3][3] = {{1,1,1},{1,1,1},{1,1,1}};

void rfPixelMin3By3(const rcWindow& srcImg, rcWindow& dstImg)
{
  rmAssert (srcImg.depth() == dstImg.depth());
  rmAssert (srcImg.size() == dstImg.size());
  if (srcImg.depth() == rcPixel8)
    {
      rcWindow newdest (dstImg, 1, 1, 
			srcImg.width()-2, srcImg.height()-2);
      rfPixel8Min3By3(srcImg, newdest);
    }
  else   if (srcImg.depth() == rcPixel16)
    {
      // Convert 16 to float
      vImage_Buffer s16, sf, d16, df;
      srcImg.vImage (s16);
      dstImg.vImage (d16);
      rcWindow f (srcImg.width(), dstImg.height(), rcPixel32);
      f.vImage (sf);
      rcWindow ff (srcImg.width(), dstImg.height(), rcPixel32);
      ff.vImage (df);
      vImage_Error ve;       
      ve = vImageConvert_16UToF (&s16, &sf, 0.0f, 1.0f, kvImageNoFlags);
      rmAssert (!ve);
      ve = vImageMin_PlanarF( &sf, &df, NULL, 0, 0, 3, 3, kvImageNoFlags);
      rmAssert (!ve);
      ve = vImageConvert_FTo16U (&sf, &df, 0.0f, 1.0f, kvImageNoFlags);      
      rmAssert (!ve);
    }
  else
    {
      rmExceptionMacro (<<"Unimplemented");
    }

}  


rcWindow rfPixelMin3By3 (const rcWindow& image)
{
  rmAssert (image.isBound());
  rcWindow newdest (image.width(), image.height(), image.depth());
  rfPixelMin3By3 (image, newdest);
  return newdest;
}




void rfPixelMax3By3(const rcWindow& srcImg, rcWindow& dstImg)
{
  rmAssert (srcImg.depth() == dstImg.depth());
  rmAssert (srcImg.size() == dstImg.size());
  if (srcImg.depth() == rcPixel8)
    {
      rcWindow newdest (dstImg, 1, 1, 
			srcImg.width()-2, srcImg.height()-2);
      rfPixel8Max3By3(srcImg, newdest);
    }
  else   if (srcImg.depth() == rcPixel16)
    {
      // Convert 16 to float
      vImage_Buffer s16, sf, d16, df;
      srcImg.vImage (s16);
      dstImg.vImage (d16);
      rcWindow f (srcImg.width(), dstImg.height(), rcPixel32);
      f.vImage (sf);
      rcWindow ff (srcImg.width(), dstImg.height(), rcPixel32);
      ff.vImage (df);
      vImage_Error ve;       
      ve = vImageConvert_16UToF (&s16, &sf, 0.0f, 1.0f, kvImageNoFlags);
      rmAssert (!ve);
      ve = vImageMax_PlanarF( &sf, &df, NULL, 0, 0, 3, 3, kvImageNoFlags);
      rmAssert (!ve);
      ve = vImageConvert_FTo16U (&sf, &df, 0.0f, 1.0f, kvImageNoFlags);      
      rmAssert (!ve);
    }
  else
    {
      rmExceptionMacro (<<"Unimplemented");
    }

}  

rcWindow rfPixelMax3By3 (const rcWindow& image)
{
  rmAssert (image.isBound());
  rcWindow newdest (image.width(), image.height(), image.depth());
  rfPixelMax3By3 (image, newdest);
  return newdest;
}

// Note: This function is used for acceleration of mirror function

void rfImageVerticalReflect (const rcWindow& src, rcWindow& dst)
{
  if (!dst.isBound())
    {
      dst = rcWindow (src.width(), src.height(), src.depth());
    }
  
  rcIPair delta = dst.size() - src.size();
  if (delta.x() < 0 || delta.y() < 0)
    rmExceptionMacro (<< "Size Mismatch");

  rcWindow dWin (dst, delta.x(), delta.y(),src.width(),src.height());
  vImage_Buffer vb, vd;
  src.vImage (vb);
  dWin.vImage (vd);
  vImage_Error ve;
  vImage_Flags vf (0);

  switch (src.depth ())
    {
    case rcPixel8:
      ve = vImageVerticalReflect_Planar8 (&vb, &vd, vf);
      rmAssert (ve == kvImageNoError);
      break;

    case rcPixel32:
      ve = vImageVerticalReflect_ARGB8888(&vb, &vd, vf);
      rmAssert (ve == kvImageNoError);
      break;

    default:
      rmExceptionMacro (<< "Not Implemented");
      break;
    }
}



/*
 * Set a 1 pixel border around a pelbuffer to a particular value
 */
void rfSetWindowBorder(const rcWindow& win, double val)
{
  const int32 width(win.width());
  const int32 height(win.height());
  const int32 rowUpdate(win.rowUpdate() / sizeof (double) );

  rcWindow top (win, 0, 0, width, 1);
  top.setAllDoublePixels (val);
  rcWindow bot (win, 0, height-1, width, 1);
  bot.setAllDoublePixels (val);

  double *p = (double *) win.rowPointer (1);
  for(int32 i = height - 2; i--; p += rowUpdate)
  {
    *p = val;
    *(p + width - 1) = val;
  }
}


void rfCopyWindowBorder(rcWindow& win, const rcWindow& src)
{
  const int32 width(win.width());
  const int32 height(win.height());

  rcWindow top (win, 0, 0, width, 1);
  rcWindow dtop (src, 0, 0, width, 1);
  top.copyPixelsFromWindow (dtop);

  rcWindow bot (win, 0, height-1, width, 1);
  rcWindow dbot (src, 0, height-1, width, 1);
  top.copyPixelsFromWindow (dbot);

  rcWindow left (win, 0, 0, 1, height);
  rcWindow dleft (src, 0, 0, 1, height);
  left.copyPixelsFromWindow (dleft);

  rcWindow right (win, width-1, 0, 1, height);
  rcWindow dright (src, width-1, 0, 1, height);
  right.copyPixelsFromWindow (dright);

}



//////////// Min && Max Pixel Routines /////////////
//////// (core of morphology) //////////////


void rfPixel8Min3By3(const rcWindow& srcImg, rcWindow& dstImg)
{
  const uint8  *srcPtr1, *srcPtr2, *srcRowBase;
  uint8        *dstPtr, *dstRowBase;
  uint8         currMinVal, pelVal;

  int       i, j, k, l;
  int       rowIter, colIter, srcRUC, dstRUC;

  colIter = srcImg.width() - 2;
  rowIter = srcImg.height() - 2;
  srcRUC = srcImg.rowUpdate();
  dstRUC = dstImg.rowUpdate();


  srcRowBase = srcImg.rowPointer(0);
  dstRowBase = dstImg.pelPointer (0,0);

  for(i = rowIter;
      i; i--, dstRowBase += dstRUC, srcRowBase += srcRUC)
  {
    dstPtr = dstRowBase;
    srcPtr1 = srcRowBase;
    for(j = colIter; j; j--, dstPtr++, srcPtr1++)
    {
      srcPtr2 = srcPtr1;
      currMinVal = 255;
      for(k = 3; k; k--)
      {
        for(l = 3; l; l--, srcPtr2++)
        {
          pelVal = *srcPtr2;
	  currMinVal = (pelVal < currMinVal) ?  pelVal : currMinVal;
	}
	srcPtr2 += srcRUC - 3;
      }
      *dstPtr = (uint8)currMinVal;
    }
  }
}


rcWindow rfPixel8Min3By3 (const rcWindow& image)
{
  rmAssert (image.isBound());
  rcWindow newdest (image.width()-2, image.height()-2);
  rfPixel8Min3By3 (image, newdest);
  return newdest;
}



void rfPixel8Max3By3(const rcWindow& srcImg, rcWindow& dstImg)
{
  const uint8  *srcPtr1, *srcPtr2, *srcRowBase;
  uint8        *dstPtr, *dstRowBase;
  uint8         currMaxVal, pelVal;

  int       i, j, k, l;
  int       rowIter, colIter, srcRUC, dstRUC;

  colIter = srcImg.width() - 2;
  rowIter = srcImg.height() - 2;
  srcRUC = srcImg.rowUpdate();
  dstRUC = dstImg.rowUpdate();

  srcRowBase = srcImg.rowPointer(0);
  dstRowBase = dstImg.pelPointer (0,0);

  for(i = rowIter; i;
      i--, dstRowBase += dstRUC, srcRowBase += srcRUC)
  {
    dstPtr = dstRowBase;
    srcPtr1 = srcRowBase;
    for(j = colIter; j; j--, dstPtr++, srcPtr1++)
    {
      srcPtr2 = srcPtr1;
      currMaxVal = 0;
      for(k = 3; k; k--)
      {
        for(l = 3; l; l--, srcPtr2++)
        {
          pelVal = *srcPtr2;
	  currMaxVal = (pelVal > currMaxVal) ?  pelVal : currMaxVal;
	}
	srcPtr2 += srcRUC - 3;
      }
      *dstPtr = currMaxVal;
    }
  }
}


rcWindow rfPixel8Max3By3 (const rcWindow& image)
{
  rmAssert (image.isBound());
  rcWindow newdest (image.width()-2, image.height()-2);
  rfPixel8Min3By3 (image, newdest);
  return newdest;
}

//////// Basic Two Image Operations ////////
/// Same size, 2 image in, One out ///////

void rfImagePairMax (rcWindow& src1, rcWindow& src2, rcWindow& dst)
{
  rmAssert (src1.isBound());
  rmAssert (src2.isBound());
  rmAssert (dst.isBound());
  rmAssert (src1.width() == src2.width());
  rmAssert (src1.height() == src2.height());
  rmAssert (src1.height() == dst.height());
  rmAssert (src1.width() == dst.width());
  rmAssert (src1.depth() == dst.depth());
  rmAssert (src1.depth() == src2.depth());
  rmAssert (src1.depth() == rcPixel8);

  int32 width (src1.width());
  int32 height (src1.height());
  
  for (int32 j = 0; j < height; j++)
    {
      uint8* s1 = src1.rowPointer (j);
      uint8* s2 = src2.rowPointer (j);
      uint8* ds = dst.rowPointer (j);

      for (int32 i = 0; i < width; i++, s1++,s2++)
	{
	  *ds++ = rmMax (*s1, *s2);
	}
    }
}


// Can be significantly sped up with AltiVec
void rfImagePairMin (rcWindow& src1, rcWindow& src2, rcWindow& dst)
{
  rmAssert (src1.isBound());
  rmAssert (src2.isBound());
  rmAssert (dst.isBound());
  rmAssert (src1.width() == src2.width());
  rmAssert (src1.height() == src2.height());
  rmAssert (src1.height() == dst.height());
  rmAssert (src1.width() == dst.width());
  rmAssert (src1.depth() == dst.depth());
  rmAssert (src1.depth() == src2.depth());
  rmAssert (src1.depth() == rcPixel8);

  int32 width (src1.width());
  int32 height (src1.height());
  
  for (int32 j = 0; j < height; j++)
    {
      uint8* s1 = src1.rowPointer (j);
      uint8* s2 = src2.rowPointer (j);
      uint8* ds = dst.rowPointer (j);

      for (int32 i = 0; i < width; i++, s1++,s2++)
	{
	  *ds++ = rmMin (*s1, *s2);
	}
    }
}



void rfImagePairMaskNeq (rcWindow& src1, rcWindow& src2, rcWindow& dst)
{
  rmAssert (src1.isBound());
  rmAssert (src2.isBound());
  rmAssert (dst.isBound());
  rmAssert (src1.width() == src2.width());
  rmAssert (src1.height() == src2.height());
  rmAssert (src1.height() == dst.height());
  rmAssert (src1.width() == dst.width());
  rmAssert (src1.depth() == dst.depth());
  rmAssert (src1.depth() == src2.depth());
  rmAssert (src1.depth() == rcPixel8);

  int32 width (src1.width());
  int32 height (src1.height());
  
  for (int32 j = 0; j < height; j++)
    {
      uint8* s1 = src1.rowPointer (j);
      uint8* s2 = src2.rowPointer (j);
      uint8* ds = dst.rowPointer (j);

      for (int32 i = 0; i < width; i++, s1++,s2++, ds++)
	{
	  *ds = (*s1 == *s2) ? uint8 (255) : uint8 (0);
	}
    }
}



/*
 * Pixel Expansion with "smart" copy support
 */
void rfPixelExpand(const rcWindow& src, rcWindow& dest,  int32 xmag, int32 ymag);

rcWindow rfPixelExpand(const rcWindow& src, int32 xmag, int32 ymag)
{
  rcWindow dest;
  rfPixelExpand(src, dest, xmag, ymag);
  return dest;
}

void rfPixelExpand(const rcWindow& src, rcWindow& dest, int32 xmag, int32 ymag)
{
  rmAssert(xmag >= 1);
  rmAssert(ymag >= 1);

  if (!src.isBound())
    return;

  rcIRect dstgcr;
  dstgcr = rcIRect (rcIPair (0,0), rcIPair(src.width() * xmag, src.height() * ymag));

  if (!dest.isBound())
    {
      dest = rcWindow (int32 (src.width() * xmag),
		       int32 (src.height() * ymag));
    }

  rmAssert (static_cast<int32>(dest.width()) >= dstgcr.width());
  rmAssert (static_cast<int32>(dest.height()) >= dstgcr.height());

  // Consider only whole blocks of src pixels
  int32 srcWidth  = src.width();
  int32 srcHeight = src.height();

  int32 srcRowUpdate = src.rowUpdate();
  int32 dstRowUpdate = dest.rowUpdate();
  int32 dstMagRowUpdate = dstRowUpdate * ymag;

  // Setup source and destination pointers according to magnification
  const uint8* srcPtr = src.pelPointer(0,0);
  uint8* dstPtr = dest.pelPointer(0,0);

  for (int32 y=0; y<srcHeight; y++)
    {
      uint8* pDest = dstPtr;
      const uint8* pSrc = srcPtr;
      for (int32 x=0; x<srcWidth; x++)
	{
	  // read the val to be expanded
	  uint8 destVal = (uint8) *pSrc;

	  uint8* pDestTemp = pDest;

	  // Write one block of xmag by ymag
	  for (int32 ycopy = ymag; ycopy; --ycopy)
	    {
	      uint8* pDestRow = pDestTemp;
	      for (int32 xcopy = xmag; xcopy; --xcopy)
		{
		  //		  rmAssert (dest.contains (pDestRow));
		  *pDestRow++ = destVal;
		}
	      // Next destination row
	      pDestTemp += dstRowUpdate;
	    }

	  pSrc++;
	  pDest += xmag;
	}

      // Source: next row, Destination: next ymag destination row
      srcPtr += srcRowUpdate;
      dstPtr += dstMagRowUpdate;
    }
}



/*
 * Pixel Sub Sampling
 *
 * Note that if skip is even then
 * center of pixel is shifted by -0.5;
 *
 */
void rfPixelSample(const rcWindow& src, rcWindow& dest,  int32 xskip, int32 yskip);

rcWindow rfPixelSample(const rcWindow& src, int32 xskip, int32 yskip)
{
  rcWindow dest;
  rfPixelSample(src, dest, xskip, yskip);
  return dest;
}


void rfPixelSample(const rcWindow& src, rcWindow& dest, int32 xskip, int32 yskip)
{
  rmAssert(xskip >= 1);
  rmAssert(yskip >= 1);

  // See the note above for Expand
  int32 ulx = 0; int32 uly = 0;
  int32 lrx = src.width() / xskip;  int32 lry = src.height()/ yskip;

  if (ulx >= lrx || uly >= lry)
    return;   

  rcIRect redSrc (rcIPair(ulx, uly), rcIPair(lrx, lry));

  if (!dest.isBound())
    {
      dest = rcWindow (redSrc.width(), redSrc.height());
    }

  // Holds coords of the pixel in source image coordinate frame where <0,0> in
  // destination image coordinate frame maps to.
  int32 xOrigin = (xskip-1) / 2 ;
  int32 yOrigin = (yskip-1) / 2;

  for (int32 y = 0; y < dest.height(); y++)
    {
      uint8* pDest = dest.rowPointer(y);
      const uint8* pSrc = src.pelPointer(xOrigin, y*yskip + yOrigin);
      int32 x = dest.width();
      do
	{
	  *pDest++ = uint8 (*pSrc);
	  pSrc += xskip;
	}
      while (--x);
    }
}

#if 0
/////////////////////////
int MaxFilterFP( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_F) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMax_PlanarF( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}

int MinFilterFP( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_F) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMin_PlanarF( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}


int MaxFilterFP( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_F) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMax_PlanarF( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}

int MinFilterFP( void *inData, unsigned int inRowBytes, void *outData, unsigned int outRowBytes, unsigned int height, unsigned int width, void *kernel, unsigned int kernel_height, unsigned int kernel_width, int divisor, vImage_Flags flags )
{
    vImage_Buffer	in = { inData, height, width, inRowBytes };
    vImage_Buffer		out = { outData, height, width, outRowBytes };
    const int		xOffset = 0;
    const int		yOffset = 0;
    size_t		tempSize = vImageGetMinimumTempBufferSizeForMinMax( &in, &out, kernel_height, kernel_width, flags, sizeof( Pixel_F) );

    if( tempSize > bufferSize )
    {
        if( NULL != tempBuffer )
            free( tempBuffer );
    
        tempBuffer = malloc( tempSize );
        bufferSize = tempSize;
    }

    return  vImageMin_PlanarF( &in, &out, tempBuffer, xOffset, yOffset, kernel_height, kernel_width, flags );
}

#endif
