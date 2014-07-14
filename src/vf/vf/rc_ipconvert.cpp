
#include "rc_types.h"
#include "rc_ip.h"
#include "rc_histstats.h"
#include "rc_imageprocessing.h"

#define rfHasSIMD false

static void _rfRcWindow32to8(const rcWindow& rgbInput, rcWindow& channelOutput, rcChannelConversion opt);

/*
 * @note conversion files leave timestamps alone. 
 */ 

void rfImageConvert8888ToARGB (vector<rcWindow>& iargb, rcWindow& argb)
{

  rmAssert (iargb.size() == 4);
  rmAssert (argb.isBound());
  int32 width (argb.width());
  int32 height (argb.height());
  //@todo add if has SIMD 
  vImage_Buffer v[4], vargb;
  argb.vImage (vargb);

  for (uint32 i = 0; i < 4; i++)
    {
      rmAssert (iargb[i].isBound());
      rmAssert (width == iargb[i].width());
      rmAssert (height == iargb[i].height());
      iargb[i].vImage(v[i]);
    }

  vImage_Error ve; 
  ve = vImageConvert_Planar8toARGB8888 (&v[0], &v[1], &v[2], &v[3], &vargb, kvImageNoFlags);
  rmAssert (!ve);
}

void rfImageConvertARGBto8888 (rcWindow& argb, vector<rcWindow>& iargb)
{

  rmAssert (iargb.empty());
  rmAssert (argb.isBound());
  int32 width (argb.width());
  int32 height (argb.height());
  //@todo add if has SIMD 
  vImage_Buffer v[4], vargb;
  argb.vImage (vargb);

  // Create 4 images and put them in the vector
  for (uint32 i = 0; i < 4; i++)
    {
      rcWindow tmp (width, height);
      iargb.push_back (tmp);
    }

  for (uint32 i = 0; i < 4; i++)
    {
      rmAssert (iargb[i].isBound());
      rmAssert (width == iargb[i].width());
      rmAssert (height == iargb[i].height());
      iargb[i].vImage(v[i]);
    }

  vImage_Error ve; 
  ve = vImageConvert_ARGB8888toPlanar8 (&vargb, &v[0], &v[1], &v[2], &v[3], kvImageNoFlags);
  rmAssert (!ve);
}

void rfImageConvert8ToARGB (const rcWindow& byteImage, const rcWindow& alpha, rcWindow& argb)
{
  rmAssert (alpha.isBound());
  rmAssert (byteImage.isBound());
  rmAssert (argb.isBound());
  rmAssert (argb.depth() == rcPixel32S);

  //@todo add if has SIMD 
  vImage_Buffer va, v8, vargb;
  alpha.vImage (va);
  byteImage.vImage (v8);
  argb.vImage (vargb);

  vImage_Error ve; 
  ve = vImageConvert_Planar8toARGB8888 (&va, &v8, &v8, &v8, &vargb, kvImageNoFlags);
  rmAssert (!ve);
}

rcWindow rfImageConvert8ToARGB (const rcWindow& byteImage, const rcWindow& alpha) 
{
  rmAssert (byteImage.isBound());
  rcWindow newdest (byteImage.width(), byteImage.height(), rcPixel32S);
  rfImageConvert8ToARGB (byteImage, alpha, newdest);
  return newdest;
}

  
void rfImageConvertFloat8 (const rcWindow& floatImage, rcWindow& byteImage,
			   float minVal, float maxVal)
{
  rmAssert (floatImage.isBound());
  rmAssert (byteImage.isBound());

  //@todo add if has SIMD 
  vImage_Buffer vf, v8;
  floatImage.vImage (vf);
  byteImage.vImage (v8);
  
  vImage_Error ve; 
  ve = vImageConvert_PlanarFtoPlanar8 (&vf, &v8, maxVal, minVal, kvImageNoFlags);
  rmAssert (!ve);
}

rcWindow rfImageConvertFloat8 (const rcWindow& floatImage, float minVal, float maxVal)
{
  rmAssert (floatImage.isBound());
  rcWindow newdest (floatImage.width(), floatImage.height(), rcPixel8);
  rfImageConvertFloat8 (floatImage, newdest, minVal, maxVal);
  return newdest;
}

///// Basic Un-accelerated Conversion 8 to 16 //////
void rfImageConvert816 (const rcWindow& byteImage, rcWindow& twobyteImage)
{
  rmAssert (byteImage.isBound());
  rmAssert (twobyteImage.isBound());

  for (int32 j = 0; j < twobyteImage.height(); j++)
     {
       uint16 * p16 = (uint16 *) twobyteImage.rowPointer (j);
       const uint8 * p8 = byteImage.rowPointer (j);
       for (int32 i = 0; i < twobyteImage.width(); i++, p8++, p16++)
	 *p16 = uint16 (*p8);
     }

}


rcWindow rfImageConvert816 (const rcWindow& byteImage)
{
  rmAssert (byteImage.isBound());
  rcWindow newdest (byteImage.width(), byteImage.height(), rcPixel16);
  rfImageConvert816 (byteImage, newdest);
  return newdest;
}


/*! \fn void rfImageConvert168 (const rcWindow& twobyteImage, rcWindow& byteImage, rcChannelConversion opt)
 *  \brief Convert 16 bit images to 8 bit ones
 *  \param twoByteImage is a 16 bit rcWindow
 *  \param byteImage is a single byte rcWindow
 *  \param opt is an enum of type rcChannelConversion 
 *  \return void 
 */ 

void rfImageConvert168 (const rcWindow& twobyteImage, rcWindow& byteImage, rcChannelConversion opt)
{
  rmAssert (twobyteImage.isBound());
  rmAssert (twobyteImage.depth() == rcPixel16);

  if (opt == rcSelectAll) 
      {
	byteImage = twobyteImage;
	return;
      }

  int32 range = (opt ==   rcSelect12BitCamera) ? (12+1) : twobyteImage.bits ();
  range = 1 << (range);

/*! 
 *  \brief Approach
 *      Convert to float with no offset and scaling
 *      Calculate a histogram using 12Bit range if possible
 *      Convert float to 8 setting the min and max as ends of a linear scale 
 *      @note that Apple vImage API for converting float to 8 has max parameter before min :)
 * Using direct 16 to 8 from vImage. 
 vImage_Buffer vu16, vf, v8;
  twobyteImage.vImage (vu16);
  rcWindow e (twobyteImage.width(), twobyteImage.height(), rcPixel8);
  e.vImage (v8);

  vImage_Error ve;
  ve = vImageConvert_16UToPlanar8( &vu16, &v8,kvImageNoFlags);
  rmAssert (!ve);
  byteImage = e;
 */
  rcHistoStats h16 (twobyteImage, range);
  float minFloat =  h16.min (5);
  float maxFloat = h16.max (5);

  vImage_Buffer vu16, vf, v8;
  twobyteImage.vImage (vu16);
  rcWindow f (twobyteImage.width(), twobyteImage.height(), rcPixel32S);
  f.vImage (vf);
  rcWindow e (twobyteImage.width(), twobyteImage.height(), rcPixel8);
  e.vImage (v8);

  vImage_Error ve;       
  ve = vImageConvert_16UToF (&vu16, &vf, 0.0f, 1.0f, kvImageNoFlags);
  rmAssert (!ve);

  ve = vImageConvert_PlanarFtoPlanar8 (&vf, &v8, maxFloat, minFloat, kvImageNoFlags);
  byteImage = e;
}


rcWindow rfImageConvert168 (const rcWindow& twobyteImage, rcChannelConversion opt)
{
  rmAssert (twobyteImage.isBound());

  if (opt == rcSelectAll) 
      {
	return rcWindow (twobyteImage);
      }

  rcWindow newdest (twobyteImage.width(), twobyteImage.height(), rcPixel8);
  rfImageConvert168 (twobyteImage, newdest);
  return newdest;
}




// Create a 8 bit gray scale image from an image stored in a color format
void rfImageConvert32to8(const rcWindow& rgbInput, rcWindow& channelOutput, rcChannelConversion opt)
{
    rmAssert( rgbInput.width() == channelOutput.width() );
    rmAssert( rgbInput.height() == channelOutput.height() );
    rmAssert( rgbInput.depth() == rcPixel32S );

    if (1)
      {
	return _rfRcWindow32to8 (rgbInput, channelOutput, opt);
      }

    if (opt == rcSelectAll) 
      {
	channelOutput = rgbInput;
	return;
      }

    const uint32 width = rgbInput.width();
    const uint32 height = rgbInput.height();

    //@todo add hasSIMD 
    vImage_Buffer v32, vr, vg, vb, va;

    rgbInput.vImage (v32);
    vector<rcWindow> channels(4);

    if ( rgbInput.isGray() )
      {
	channels[(int32) rcSelectGreen] = channelOutput;
      }
    else
      {
	switch (opt)
	  {
	  case rcSelectAverage:
	    channels[(int32) rcSelectGreen] = channelOutput;
	    break;
	  case rcSelectRed:
	    channels[(int32) rcSelectRed] = channelOutput;
	    break;
	  case rcSelectGreen:
	    channels[(int32) rcSelectGreen] = channelOutput;
	    break;
	  case rcSelectBlue:
	    channels[(int32) rcSelectBlue] = channelOutput;
	    break;
	  case rcSelectMax:
	    channels[(int32) rcSelectBlue] = channelOutput;
	    break;
	  case rcSelectAll:
	  default:
	    // @todo API to support this.
	    rmAssert( 0 ); 
	  }
      }

    for (uint32 i = 0; i < channels.size(); i++)
      {
	if (!channels[i].isBound())
	  {
	    rcWindow f (width, height, rcPixel8);
	    channels[i] = f;
	  }
      }

    channels[(int32) rcSelectGreen].vImage (vg);
    channels[(int32) rcSelectRed].vImage (vr);
    channels[(int32) rcSelectBlue].vImage (vb);
    channels[channels.size() - 1].vImage (va);

    vImage_Error ve;
    ve = vImageConvert_ARGB8888toPlanar8 (&v32, 
					  &va, &vr, &vg, &vb, 
					  kvImageNoFlags);    
    
}

rcWindow rfImageConvert32to8 (const rcWindow& rgbInput, rcChannelConversion opt)
{
  rmAssert (rgbInput.isBound());

  if (opt == rcSelectAll) 
    {
      return rcWindow (rgbInput);
    }

  rcWindow newdest (rgbInput.width(), rgbInput.height(), rcPixel8);
  rfImageConvert32to8 (rgbInput, newdest, opt);
  return newdest;
}



// Convert a vector of 32-bit gray scale image to 8-bit
void rfImageConvert32to8(const vector<rcWindow>& rgb, vector<rcWindow>& channel, rcChannelConversion opt)
{
   assert (rgb.size());
   channel.resize( rgb.size());
   assert (rgb.size() == channel.size());

   rcPixel pd = rcPixel8;

   for (vector<rcWindow>::const_iterator img = rgb.begin(); img != rgb.end(); img++)
   {
       rcWindow tmp (img->width(), img->height(), pd);
       rfImageConvert32to8( *img, tmp, opt );
       uint32 count = img - rgb.begin ();
       channel[count] = tmp;
   }

}

// Create a 32-bit image from 8-bit image
void rfImageConvert8to32(const rcWindow& rgbInput, rcWindow& rgbOutput)
{
    rmAssert( rgbInput.width() == rgbOutput.width() );
    rmAssert( rgbInput.height() == rgbOutput.height() );
    rmAssert( rgbInput.depth() == rcPixel8 );
    rmAssert( rgbOutput.depth() == rcPixel32S );

    const uint32 width = rgbInput.width();
    const uint32 height = rgbInput.height();

    //@todo add hasSIMD 
    if (rfHasSIMD ) 
      {
	vImage_Buffer v8, v32, va;
	rgbInput.vImage (v8);
	rgbOutput.vImage (v32);
	rcWindow f (width, height, rcPixel8);
	f.setAllPixels (0);
	f.vImage (va);

	vImage_Error ve;	
	ve = vImageConvert_Planar8toARGB8888 (&va, &v8, &v8, &v8, &v32,
					  kvImageNoFlags);    
	return;
      }

    const rcFrameRef& iFrame = rgbInput.frameBuf();
 
    // TODO: use AltiVec for this
    for (uint32 j = 0; j < height; j++)
    {
      uint32* oRow = (uint32*) rgbOutput.rowPointer (j);
      uint8* iRow = (uint8*) rgbInput.rowPointer (j);
        
      for (uint32 i = 0; i < width; i++, oRow++, iRow++)
        {
	  const uint8 pix = *iRow;
	  *oRow = iFrame->getColor( uint32(pix) );
        }
    }
}



/////////////////////////////



void rfImageAddMask (const rcWindow& image, const rcWindow& mask, rcWindow& fourbyteImage, uint32 maskVal)
{
	rmAssert (image.isBound());
	rmAssert (mask.isBound());
	rmAssert (fourbyteImage.isBound());
	rmAssert (mask.depth() == rcPixel8);
	rmAssert (image.width() == mask.width());
	rmAssert (image.height() == mask.height());

	if (image.depth() == rcPixel8)
	{
		for (int32 j = 0; j < image.height(); j++)
		{
			uint8 * p32 =  fourbyteImage.rowPointer (j);
			const uint8 * p8 = image.rowPointer (j);
			const uint8 * m8 = mask.rowPointer (j);
			for (int32 i = 0; i < fourbyteImage.width(); i++, p8++, m8++, p32++)
			{
				uint8 gray = *p8;
				if (*m8 == 255) 	*p32 = 0;	 
				else *p32 = gray;
			}
		}
	}
	else if (image.depth() == rcPixel16)
	{
		for (int32 j = 0; j < image.height(); j++)
		{
			uint16 * p32 = (uint16 *) fourbyteImage.rowPointer (j);
			const uint16 * p16 = (uint16 *) image.rowPointer (j);
			const uint8 * m8 = mask.rowPointer (j);
			for (int32 i = 0; i < fourbyteImage.width(); i++, p16++, m8++, p32++)
			{
				uint16 gray = *p16;
				if (*m8 == 255) *p32 = 0;
				else *p32 = gray;
			}
		}
	}
	else if (image.depth() == rcPixel32S)   // 32 bit case needs to be reworked
	{
		for (int32 j = 0; j < image.height(); j++)
		{
			uint32 * p32 = (uint32 *) fourbyteImage.rowPointer (j);
			const uint32 * i32 = (uint32 *) image.rowPointer (j);
			const uint8 * m8 = mask.rowPointer (j);
			for (int32 i = 0; i < fourbyteImage.width(); i++, i32++, m8++, p32++)
			{
				uint32 pixel = *i32;
				if (*m8 == 255) *p32 = 0;
				else *p32 = pixel;				
			}
		}
	}
		else
			rmAssert (0);
}

rcWindow rfImageAddMask (const rcWindow& image, const rcWindow& mask, uint32 maskVal)
{
	rmAssert (image.isBound());
	rmAssert (mask.isBound());
	rcWindow newdest (image.width(), image.height(), image.depth ());
	rfImageAddMask (image, mask, newdest, maskVal);
	return newdest;
}

rcWindow rfImage8Transpose (const rcWindow& image)
{
  rmAssert (image.isBound());
  
  rcWindow dst (image.height(), image.width());
  int32 srup (image.rowUpdate());

  for (int32 y = 0; y < dst.height(); y++)
    {
      const uint8* sp = image.pelPointer(y, 0);
      uint8 * dp = dst.pelPointer (0, y);

      int32 x = dst.width();
      do
      {
	*dp++ = *sp;
	sp += srup;
      } while (--x);
    }

  return dst;
}



// Create a 8 bit gray scale image from an image stored in a color format
static void _rfRcWindow32to8(const rcWindow& rgbInput, rcWindow& channelOutput, rcChannelConversion opt)
{
    rmAssert( rgbInput.width() == channelOutput.width() );
    rmAssert( rgbInput.height() == channelOutput.height() );
    rmAssert( rgbInput.depth() == rcPixel32S );
  
    const uint32 width = rgbInput.width();
    const uint32 height = rgbInput.height();

    // TODO: use AltiVec for this
    // Separate color and gray cases for speed
    
    if ( rgbInput.isGray() ) {
        // Gray scale image

        for (uint32 j = 0; j < height; j++)
        {
            uint32 *rgbRow = (uint32 *) rgbInput.rowPointer (j);
            uint8 *oneRow = channelOutput.rowPointer (j);
            
            for (uint32 i = 0; i < width; i++, rgbRow++, oneRow++)
            {
                const uint8* rgbComponents = (uint8*) rgbRow;
                // ARGB or RGBA we are assuming all channels are the same
                *oneRow = rgbComponents[1];
            }
        }
    }
    else {
        // Color image
        for (uint32 j = 0; j < height; j++)
        {
            uint32 *rgbRow = (uint32 *) rgbInput.rowPointer (j);
            uint8 *oneRow = channelOutput.rowPointer (j);
            
            for (uint32 i = 0; i < width; i++, rgbRow++, oneRow++)
	      {
                const uint8* rgbComponents = (uint8*) rgbRow;

		switch (opt)
		  {
		  case rcSelectAverage:
		    *oneRow = rfGray( rgbComponents[1], rgbComponents[2], rgbComponents[3] );
		    break;
		  case rcSelectRed:
		    *oneRow = rgbComponents[1];
		    break;
		  case rcSelectGreen:
		    *oneRow = rgbComponents[2];
		    break;
		  case rcSelectBlue:
		    *oneRow = rgbComponents[3];
		    break;
		  case rcSelectMax:
		    *oneRow = rmMaxOf3 ( rgbComponents[1], rgbComponents[2], rgbComponents[3] );
           case rcSelectAll:
		  default:
             // Cannot keep all channels when going from 32 to 8
             rmAssert( 0 ); 
		  }
	      }
        }
    }
}
