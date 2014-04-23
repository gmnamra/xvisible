// Copyright 2002 Reify, Inc.

#include "rc_imageprocessing.h"
#include "rc_types.h"
#include "rc_ip.h"

#define FAST_GEN_MODULUS 32
#define ALTIVEC_MODULUS 16
#define EVEN_MODULUS 2
#define ALT_UNROLL_MODULUS 8
#define ALT_BIG_UNROLL_MODULUS 16
#define CIMP_UNROLL_MODULUS 32

#define VALUE_MODULO(VALUE,MODULUS) (((int)VALUE) & (MODULUS-1))
#define FAST_VALUE_MODULO(VALUE) VALUE_MODULO(VALUE, FAST_GEN_MODULUS)
#define ALTIVEC_VALUE_MODULO(VALUE) VALUE_MODULO(VALUE, ALTIVEC_MODULUS)
#define EVEN_VALUE_MODULO(VALUE) VALUE_MODULO(VALUE, EVEN_MODULUS)
#define ALT_UNROLL_VALUE_MODULO(VALUE) VALUE_MODULO(VALUE, ALT_UNROLL_MODULUS)
#define ALT_BIG_UNROLL_VALUE_MODULO(VALUE) VALUE_MODULO(VALUE, ALT_BIG_UNROLL_MODULUS)
#define CIMP_UNROLL_VALUE_MODULO(VALUE) VALUE_MODULO(VALUE, CIMP_UNROLL_MODULUS)

void rcCverGenHalfRes(char* src, char* dest, const uint32 widthInPixels,
		      const uint32 heightInLines, const uint32 rowUpdate,
		      const bool invert);
#ifdef __ppc__
void rcAltivecGenHalfRes(char* src, char* dest, const uint32 widthInPixels,
			 const uint32 heightInLines, const uint32 rowUpdate,
			 const bool invert);
#endif

int myfirsttime = 1;

/* Generate a version of an 8 bit, grey scale src image that is
 * half-res in x and y. Odd pixels/rows will be ignored. In other
 * words, a source image that is 5 by 5 will result in a 2 by 2, not a
 * 3 by 3, destination image. The row update for the generated image
 * will always be widthInPixels/2.
 */
void rcGenHalfRes(char* src, char* dest, uint32 widthInPixels,
		  uint32 heightInLines, uint32 srcRowUpdate, bool invert)
{
  assert(src);
  assert(dest);
  assert(widthInPixels <= srcRowUpdate);
  assert(widthInPixels > 1);
  assert(heightInLines > 1);

#ifdef __ppc__
  if ((ALTIVEC_VALUE_MODULO(src) == 0) && (ALTIVEC_VALUE_MODULO(dest) == 0) &&
      (ALTIVEC_VALUE_MODULO(srcRowUpdate) == 0) && (FAST_VALUE_MODULO(widthInPixels) == 0))
    rcAltivecGenHalfRes(src, dest, widthInPixels, heightInLines, srcRowUpdate, invert);
  else
#endif
    rcCverGenHalfRes(src, dest, widthInPixels, heightInLines, srcRowUpdate, invert);
}


#define COMPRESS2 { \
  accum = (unsigned short)se[0] + (unsigned short)se[1] + \
      (unsigned short)so[0] + (unsigned short)so[1];      \
  *d++ = (unsigned char)((accum + 2) >> 2) ^ inv;         \
  se += 2; so += 2; }

void rcCverGenHalfRes(char* src, char* dest, const uint32 widthInPixels,
		      const uint32 heightInLines, const uint32 rowUpdate,
		      const bool invert)
{
  const uint32 constProcessBlkCnt = widthInPixels / EVEN_MODULUS;
  const int32 constFullBlkCnt = constProcessBlkCnt/CIMP_UNROLL_MODULUS;
  const int32 constBlkRemainder = CIMP_UNROLL_VALUE_MODULO(constProcessBlkCnt);
  const int32 constInitialIndex = constBlkRemainder ? -1 : 0;
  const uint32 constSkipPixelsCnt = 2*rowUpdate - widthInPixels;
  uint8* se = (uint8*)src;
  uint8* so = se + rowUpdate;
  uint8* d = (uint8*)dest;
  uint8 inv = 0; // Assume invert is false

  if (invert)
    inv = 0xFF;

  uint16 accum;

  for (uint32 y = 0; y < heightInLines/EVEN_MODULUS; y++)
  {
    int32 index = constInitialIndex;

    if (0) // Make warnings about unreachable code go away
      goto compilerPlacebo;

    switch (constBlkRemainder)
    {
    compilerPlacebo:
      for ( ; index < constFullBlkCnt; index++)
      {
      case 0:  // 64 pixels
	COMPRESS2;
      case 31: // 62 pixels
	COMPRESS2;
      case 30: // 60 pixels
	COMPRESS2;
      case 29: // 58 pixels
	COMPRESS2;
      case 28: // 56 pixels
	COMPRESS2;
      case 27: // 54 pixels
	COMPRESS2;
      case 26: // 52 pixels
	COMPRESS2;
      case 25: // 50 pixels
	COMPRESS2;
      case 24: // 48 pixels
	COMPRESS2;
      case 23: // 46 pixels
	COMPRESS2;
      case 22: // 44 pixels
	COMPRESS2;
      case 21: // 42 pixels
	COMPRESS2;
      case 20: // 40 pixels
	COMPRESS2;
      case 19: // 38 pixels
	COMPRESS2;
      case 18: // 36 pixels
	COMPRESS2;
      case 17: // 34 pixels
	COMPRESS2;
      case 16: // 32 pixels
	COMPRESS2;
      case 15: // 30 pixels
	COMPRESS2;
      case 14: // 28 pixels
	COMPRESS2;
      case 13: // 26 pixels
	COMPRESS2;
      case 12: // 24 pixels
	COMPRESS2;
      case 11: // 22 pixels
	COMPRESS2;
      case 10: // 20 pixels
	COMPRESS2;
      case 9:  // 18 pixels
	COMPRESS2;
      case 8:  // 16 pixels
	COMPRESS2;
      case 7:  // 14 pixels
	COMPRESS2;
      case 6:  // 12 pixels
	COMPRESS2;
      case 5:  // 10 pixels
	COMPRESS2;
      case 4:  // 8 pixels
	COMPRESS2;
      case 3:  // 6 pixels
	COMPRESS2;
      case 2:  // 4 pixels
	COMPRESS2;
      case 1:  // 2 pixels
	COMPRESS2;
      } // End of: for ( ; index < constFullBlkCnt; index++)
    } // End of: switch (constBlkRemainder)

    se += constSkipPixelsCnt;
    so += constSkipPixelsCnt;
  } // End of: for (uint32 y = 0; y < heightInLines/EVEN_MODULUS; y++)
}

#ifdef __ppc__

#define COMPRESS32 { \
  vLine = *vse++;                                 \
  vAccum = vec_mulo(vLine, vc1);                  \
  vAccum = vec_add(vAccum, vec_mule(vLine, vc1)); \
  vLine = *vso++;                                 \
  vAccum = vec_add(vAccum, vec_mulo(vLine, vc1)); \
  vAccum = vec_add(vAccum, vec_mule(vLine, vc1)); \
  vAccum = vec_add(vAccum, vc2);                  \
  avg0 = vec_sr(vAccum, vc2);                     \
  vLine = *vse++;                                 \
  vAccum = vec_mulo(vLine, vc1);                  \
  vAccum = vec_add(vAccum, vec_mule(vLine, vc1)); \
  vLine = *vso++;                                 \
  vAccum = vec_add(vAccum, vec_mulo(vLine, vc1)); \
  vAccum = vec_add(vAccum, vec_mule(vLine, vc1)); \
  vAccum = vec_add(vAccum, vc2);                  \
  avg1 = vec_sr(vAccum, vc2);                     \
  *vd++ = vec_xor(vec_pack(avg0, avg1), inv); }

void rcAltivecGenHalfRes(char* src, char* dest, const uint32 widthInPixels,
			 const uint32 heightInLines, const uint32 rowUpdate,
			 const bool invert)
{
  vector unsigned char vc1 = vec_splat_u8(1);
  vector unsigned short vc2 = vec_splat_u16(2);
  vector unsigned char inv = vec_splat_u8(0); // Assume invert is false
  vector unsigned char* vse = (vector unsigned char*)src;
  vector unsigned char* vso = (vector unsigned char*)(src + rowUpdate);
  vector unsigned char* vd = (vector unsigned char*)dest;
  const uint32 constProcessBlkCnt = widthInPixels / FAST_GEN_MODULUS;
  const int32 constFullBlkCnt = constProcessBlkCnt/ALT_UNROLL_MODULUS;
  const int32 constBlkRemainder = ALT_UNROLL_VALUE_MODULO(constProcessBlkCnt);
  const int32 constInitialIndex = constBlkRemainder ? -1 : 0;
  const uint32 constSkipBlkCnt = (2*rowUpdate - widthInPixels) / ALTIVEC_MODULUS;

  vector unsigned char vLine;
  vector unsigned short vAccum, avg0, avg1;

  if (invert)
  {
    vector signed char t = vec_splat_s8(-1);
    inv = (vector unsigned char)t;
  }

  for (uint32 y = 0; y < heightInLines/EVEN_MODULUS; y++)
  {
    int32 index = constInitialIndex;

    if (0) // Make warnings about unreachable code go away
      goto compilerPlacebo;

    switch (constBlkRemainder)
    {
    compilerPlacebo:
      for ( ; index < constFullBlkCnt; index++)
      {
      case 0: // 256 pixels
	COMPRESS32;
      case 7: // 224 pixels
	COMPRESS32;
      case 6: // 192 pixels
	COMPRESS32;
      case 5: // 160 pixels
	COMPRESS32;
      case 4: // 128 pixels
	COMPRESS32;
      case 3: // 96 pixels
	COMPRESS32;
      case 2: // 64 pixels
	COMPRESS32;
      case 1: // 32 pixels
	COMPRESS32;
      } // End of: for ( ; index < constFullBlkCnt; index++)
    } // End of: switch (constBlkRemainder)
    
    vso += constSkipBlkCnt;
    vse += constSkipBlkCnt;
  } // End of: for (uint32 y = 0; y < heightInLines/EVEN_MODULUS; y++)
}


#define XOR32 { \
  *vd++ = vec_xor (*vse++, inv);		  \
  *vd++ = vec_xor (*vse++, inv);}


void Altivec8XOR(const uint8* src, uint8* dest, const uint32 widthInPixels,
                 const uint32 heightInLines, const uint32 rowUpdate,
                 const uint32 dstRup)
{
  vector unsigned char* vse = (vector unsigned char*)src;
  vector unsigned char* vd = (vector unsigned char*)dest;
  const uint32 constProcessBlkCnt = widthInPixels / FAST_GEN_MODULUS;
  const int32 constFullBlkCnt = constProcessBlkCnt/ALT_UNROLL_MODULUS;
  const int32 constBlkRemainder = ALT_UNROLL_VALUE_MODULO(constProcessBlkCnt);
  const int32 constInitialIndex = constBlkRemainder ? -1 : 0;
  const uint32 constSkipBlkCnt = (rowUpdate - widthInPixels) / ALTIVEC_MODULUS;
  const uint32 DstconstSkipBlkCnt = (dstRup - widthInPixels) / ALTIVEC_MODULUS;

  vector signed char t = vec_splat_s8(-1);
  vector unsigned char inv = (vector unsigned char)t;

  for (uint32 y = 0; y < heightInLines; y++)
  {
    int32 index = constInitialIndex;

    if (0) // Make warnings about unreachable code go away
      goto compilerPlacebo;

    switch (constBlkRemainder)
    {
    compilerPlacebo:
      for ( ; index < constFullBlkCnt; index++)
      {
      case 0: // 256 pixels
	XOR32;
      case 7: // 224 pixels
	XOR32;
      case 6: // 192 pixels
	XOR32;
      case 5: // 160 pixels
	XOR32;
      case 4: // 128 pixels
	XOR32;
      case 3: // 96 pixels
	XOR32;
      case 2: // 64 pixels
	XOR32;
      case 1: // 32 pixels
	XOR32;
      } // End of: for ( ; index < constFullBlkCnt; index++)
    } // End of: switch (constBlkRemainder)
    
    vse += constSkipBlkCnt;
    vd += DstconstSkipBlkCnt;

  } // End of: for (uint32 y = 0; y < heightInLines/EVEN_MODULUS; y++)
}
#endif


#define rmMaxOf3(a, b, c) (rmMax(rmMax(a, b), c))

// Create a 8 bit gray scale image from an image stored in a color format
void rfRcWindow32to8(const rcWindow& rgbInput, rcWindow& channelOutput, rcChannelConversion opt)
{
  return rfImageConvert32to8 (rgbInput, channelOutput, opt);
}

// Convert a vector of 32-bit gray scale image to 8-bit
void rfRcWindow32to8(const vector<rcWindow>& rgb, vector<rcWindow>& channel, rcChannelConversion opt)
{
   assert (rgb.size());
   channel.resize( rgb.size());
   assert (rgb.size() == channel.size());

   rcPixel pd = rcPixel8;

   for (vector<rcWindow>::const_iterator img = rgb.begin(); img != rgb.end(); img++)
   {
       rcWindow tmp (img->width(), img->height(), pd);
       rfRcWindow32to8( *img, tmp, opt );
       uint32 count = img - rgb.begin ();
       channel[count] = tmp;
   }
#ifdef DEBUG_LOG
   fprintf (stderr, " Converted to single Channel depth %i\n", pd );
#endif   
}

// Create a 32-bit image from 8-bit image
void rfWindow8to32(const rcWindow& rgbInput, rcWindow& rgbOutput)
{
    rmAssert( rgbInput.width() == rgbOutput.width() );
    rmAssert( rgbInput.height() == rgbOutput.height() );
    rmAssert( rgbInput.depth() == rcPixel8 );
    rmAssert( rgbOutput.depth() == rcPixel32S );
    
    const uint32 width = rgbInput.width();
    const uint32 height = rgbInput.height();
 
    // TODO: use AltiVec for this
    for (uint32 j = 0; j < height; j++)
    {
        uint32* oRow = (uint32*) rgbOutput.rowPointer (j);
        uint8* iRow = (uint8*) rgbInput.rowPointer (j);
        
        for (uint32 i = 0; i < width; i++)
        {
	  *oRow++ = *iRow++;
        }
    }
}

// Create a 32-bit image from 8-bit image
void rfRcWindow8to32(const rcWindow& rgbInput, rcWindow& rgbOutput)
{
    rmAssert( rgbInput.width() == rgbOutput.width() );
    rmAssert( rgbInput.height() == rgbOutput.height() );
    rmAssert( rgbInput.depth() == rcPixel8 );
    rmAssert( rgbOutput.depth() == rcPixel32S );
    
    const uint32 width = rgbInput.width();
    const uint32 height = rgbInput.height();
    const rcSharedFrameBufPtr& iFrame = rgbInput.frameBuf();
 
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


static const uint8 sInvertMap [256] = 
  {
    0,
    1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
    17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
    33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
    49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
    65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
    81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,
    97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
    113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
    129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,
    145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
    161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,
    177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,
    193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,
    209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,
    225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,
    241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
  };

// Reverse 8-bit pixel values (pixel value P becomes 255-P)
void rfReversePixels8(rcWindow& image)
{
    rmAssert( image.depth() == rcPixel8 );
    const int32 width = image.width();
    const int32 height = image.height();

    // Use default gray color map
    image.frameBuf()->initGrayColorMap();

    for (int32 j = 0; j < height; j++) {
        uint8 *oneRow = image.rowPointer (j);
            
        for (int32 i = 0; i < width; ++i, ++oneRow)
            *oneRow = ~*oneRow;
    }
}

void rfReversePixels16(rcWindow& image)
{
    rmAssert( image.depth() == rcPixel16);
    const int32 width = image.width();
    const int32 height = image.height();

    // Use default gray color map
    image.frameBuf()->initGrayColorMap();

    // TODO: use AltiVec for better speed
    for (int32 j = 0; j < height; j++) {
      uint16 *oneRow = (uint16 *) image.rowPointer (j);
            
        for (int32 i = 0; i < width; ++i, ++oneRow)
            *oneRow = ~*oneRow;
    }
}

// Is white color at color index 0
bool rfIsWhiteReversed(const rcWindow& image)
{
    bool isWhiteReversed = false;
    const rcSharedFrameBufPtr& frame = image.frameBuf();
    
    if ( frame->colorMap() != 0 && frame->colorMapSize() > 0 &&
         frame->isGray() ) {
        uint32 cMapSize = frame->colorMapSize();

        // Test whether index 0 contains white
        isWhiteReversed = true;
           
        for (uint32 i = 1; i < cMapSize; ++i) {
            uint32 prev = rfRed(frame->getColor( i-1 ));
            uint32 cur = rfRed(frame->getColor( i ));
            // All RGB values must be in ascending order for white to
            // be considered reversed
            if ( prev < cur ) {
                isWhiteReversed = false;
                break;
            }
        }
    }

    return isWhiteReversed;
}

static void rfCverAndImages(const rcWindow& srcWin, const rcWindow& maskWin,
			    rcWindow& destWin)
{
  const int32 width = srcWin.width(), height = srcWin.height();

  for (int32 y = 0; y < height; y++)
    for (int32 x = 0; x < width; x++)
      destWin.setPixel(x, y, (maskWin.getPixel(x, y) & srcWin.getPixel(x, y)));
}

#ifdef __ppc__

#define AND16 { *vd++ = vec_and(*vs++, *vm++); }

static void rfAltivecAndImages(uint8* src, uint8* mask, uint8* dest,
			       int32 widthInPixels, int32 heightInLines,
			       int32 rowUpdate)
{
  vector unsigned char* vs = (vector unsigned char*)src;
  vector unsigned char* vm = (vector unsigned char*)mask;
  vector unsigned char* vd = (vector unsigned char*)dest;
  const uint32 constProcessBlkCnt = widthInPixels / ALTIVEC_MODULUS;
  const int32 constFullBlkCnt = constProcessBlkCnt/ALT_BIG_UNROLL_MODULUS;
  const int32 constBlkRemainder = ALT_BIG_UNROLL_VALUE_MODULO(constProcessBlkCnt);
  const int32 constInitialIndex = constBlkRemainder ? -1 : 0;
  const uint32 constSkipBlkCnt = (rowUpdate - widthInPixels) / ALTIVEC_MODULUS;

  for (int32 y = 0; y < heightInLines; y++)
  {
    int32 index = constInitialIndex;

    if (0) // Make warnings about unreachable code go away
      goto compilerPlacebo;

    switch (constBlkRemainder)
    {
    compilerPlacebo:
      for ( ; index < constFullBlkCnt; index++)
      {
      case 0: // 256 pixels
	AND16;
      case 15: // 240 pixels
	AND16;
      case 14: // 224 pixels
	AND16;
      case 13: // 208 pixels
	AND16;
      case 12: // 192 pixels
	AND16;
      case 11: // 176 pixels
	AND16;
      case 10: // 160 pixels
	AND16;
      case 9: // 144 pixels
	AND16;
      case 8: // 128 pixels
	AND16;
      case 7: // 112 pixels
	AND16;
      case 6: // 96 pixels
	AND16;
      case 5: // 80 pixels
	AND16;
      case 4: // 64 pixels
	AND16;
      case 3: // 48 pixels
	AND16;
      case 2: // 32 pixels
	AND16;
      case 1: // 16 pixels
	AND16;
      } // End of: for ( ; index < constFullBlkCnt; index++)
    } // End of: switch (constBlkRemainder)
    
    vs += constSkipBlkCnt;
    vm += constSkipBlkCnt;
    vd += constSkipBlkCnt;
  } // End of: for (uint32 y = 0; y < heightInLines/EVEN_MODULUS; y++)
}

#endif

void rfAndImage(const rcWindow& srcWin, const rcWindow& maskWin,
		rcWindow& destWin)
{
  if ((srcWin.depth() != rcPixel8 &&
      srcWin.depth() != rcPixel16 &&
       srcWin.depth() != rcPixel32S) ||
      srcWin.depth() != maskWin.depth() ||
      srcWin.depth() != destWin.depth() ||
      srcWin.width() != maskWin.width() || 
      srcWin.width() != destWin.width() ||
      srcWin.height() != maskWin.height() ||
      srcWin.height() != destWin.height())
    {
      rmExceptionMacro(<< "IP MisMatch");
    }
#ifdef __ppc__
  uint8* src = (uint8*)srcWin.rowPointer(0);
  uint8* dest = (uint8*)destWin.rowPointer(0);
  uint8* mask = (uint8*)maskWin.rowPointer(0);

  int32 srcRowUpdate = srcWin.rowUpdate();
  int32 destRowUpdate = destWin.rowUpdate();
  int32 maskRowUpdate = maskWin.rowUpdate();

  int32 widthInPixels = srcWin.width()*((int32)srcWin.depth());


  if (rfHasSIMD () && (ALTIVEC_VALUE_MODULO(src) == 0) && (ALTIVEC_VALUE_MODULO(dest) == 0) &&
      (ALTIVEC_VALUE_MODULO(mask) == 0) &&
      (ALTIVEC_VALUE_MODULO(srcRowUpdate) == 0) && 
      (ALTIVEC_VALUE_MODULO(destRowUpdate) == 0) && 
      (ALTIVEC_VALUE_MODULO(maskRowUpdate) == 0) && 
      (ALTIVEC_VALUE_MODULO(widthInPixels) == 0))
    rfAltivecAndImages(src, mask, dest, widthInPixels, srcWin.height(),
		       srcRowUpdate);
  else
#endif
    rfCverAndImages(srcWin, maskWin, destWin);
}


/////////////////////////
//
// 8 bit to 16 bit copying with optional mapping
//
// 8bit to 16 bit altivec instructions from older internal gauss implementation
/////////////////////////


#define FETCHEXPAND(a,b,c) \
if (rmIsAlignedAddr ((a))) \
{\
tt = (vector unsigned char *) (a);\
t0 = tt[0];\
tt = (vector unsigned char *) (b);\
m0 = tt[0];\
tt = (vector unsigned char *) (c);\
b0 = tt[0];\
}\
else \
{\
perm1 = vec_lvsl(0, (a)); \
tt = (vector unsigned char *) (a);\
t0 = vec_perm(tt[0], tt[1], perm1);\
tt = (vector unsigned char *) (b);\
m0 = vec_perm(tt[0], tt[1], perm1);\
tt = (vector unsigned char *) (c);\
b0 = vec_perm(tt[0], tt[1], perm1);\
}\
st0Odd = vec_mulo ( sOne, t0);\
st0Even = vec_mule ( sOne, t0);\
sm0Odd = vec_mulo ( sOne, m0);\
sm0Even = vec_mule ( sOne, m0);\
sb0Odd = vec_mulo ( sOne, b0);\
sb0Even = vec_mule ( sOne, b0)










/////////////////////////
// Temporal Median
static void rfTemporalMedianPixel8 (const vector<rcWindow>& srcWin, rcWindow& destWin);
static void rfTemporalMedianPixel16 (const vector<rcWindow>& srcWin, rcWindow& destWin);

void rfTemporalMedian (const vector<rcWindow>& srcWin, rcWindow& destWin)
{
	rmAssert (srcWin.size() == 3);
	if (! destWin.isBound () )
	{
		destWin = rcWindow (srcWin[0].width(), srcWin[1].height (), srcWin[0].depth () );
	}
	
	for (uint32 i = 0; i < srcWin.size(); i++)
	{
		rmAssert(srcWin[i].depth() == destWin.depth());
		rmAssert(srcWin[i].width() == destWin.width());
		rmAssert(srcWin[i].height() == destWin.height());
	}
	
	switch (destWin.depth ())
	{
		case rcPixel8:
		rfTemporalMedianPixel8 (srcWin, destWin);
		break;
		case rcPixel16:
		rfTemporalMedianPixel16 (srcWin, destWin);
		break;
		default:
		rmAssert (1);
	}
}


void rfTemporalMedianPixel8 (const vector<rcWindow>& srcWin, rcWindow& destWin)
{
	rmAssert (srcWin.size() == 3);
	uint8 median;

	for (uint32 i = 0; i < srcWin.size(); i++)
	{
		rmAssert((srcWin[i].depth() == rcPixel8));
		rmAssert(srcWin[i].depth() == destWin.depth());
		rmAssert(srcWin[i].width() == destWin.width());
		rmAssert(srcWin[i].height() == destWin.height());
	}

	vector<uint8*> src (3);
	vector<uint8*> srcUP (3);

	int32 widthInPixels = srcWin[0].width();

	const uint32 opsPerLoop = 16;
	uint32 unrollCnt = widthInPixels / opsPerLoop;
	uint32 unrollRem = widthInPixels % opsPerLoop;
	uint32 lastRow = srcWin[0].height() - 1, row = 0;

	uint8 ab [16], ac[16], bc[16];
	uint8 abc[3];

	for ( ; row <= lastRow; row++)
	{

		for (uint32 i = 0; i < srcWin.size(); i++)
		{
			src[i] = (uint8*)srcWin[i].rowPointer(row);
		}

		uint8* dest = (uint8*)destWin.rowPointer(row);

	// Process a chunk
	// for pixels in the loop
	// calculate ab, ac, and bc
		for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
		{
			for (uint32 i = 0; i < opsPerLoop; i++, src[0]++, src[1]++, src[2]++, dest++)
			{
				abc[0] = *src[0]; abc[1] = *src[1]; abc[2] = *src[2];
				ab[i] = abc[0] > abc[1];
				ac[i] = abc[0] > abc[2];
				bc[i] = abc[1] > abc[2];

				int32 t0 = ab[i] + ac[i];
				int32 t1 = bc[i] - ab[i];
				int32 t2 = bc[i] + ac[i];

				if (t1 == 0 && (t0 % 2) != 1 && (t2 % 2) != 1)
					median = abc[1];
				else if (t1 != 0 && t0 == 1 && (t2 % 2) != 1)
					median = abc[0];		
				else if (t1 != 0 && t2 == 1 && (t0 % 2) != 1)
					median = abc[2];
				else
					rmAssert (1);

				*dest = median;
			}	      
		}

	// Left overs less than opsPerLoop
		memset(&ab[0], 0, 16);memset(&ac[0], 0, 16);memset(&bc[0], 0, 16);

		for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++, src[0]++, src[1]++, src[2]++, dest++)
		{
			abc[0] = *src[0]; abc[1] = *src[1]; abc[2] = *src[2];
			ab[touchCount] = abc[0] > abc[1];
			ac[touchCount] = abc[0] > abc[2];
			bc[touchCount] = abc[1] > abc[2];

			int32 t0 = ab[touchCount] + ac[touchCount];
			int32 t1 = bc[touchCount] - ab[touchCount];
			int32 t2 = bc[touchCount] + ac[touchCount];

			if (t1 == 0 && (t0 % 2) != 1 && (t2 % 2) != 1)
				median = abc[1];
			else if (t1 != 0 && t0 == 1 && (t2 % 2) != 1)
				median = abc[0];		
			else if (t1 != 0 && t2 == 1 && (t0 % 2) != 1)
				median = abc[2];
			else
				rmAssert (1);

			*dest = median;
		}
	}

}



void rfTemporalMedianPixel16 (const vector<rcWindow>& srcWin, rcWindow& destWin)
{
	rmAssert (srcWin.size() == 3);
	uint16 median;

	for (uint32 i = 0; i < srcWin.size(); i++)
	{
		rmAssert((srcWin[i].depth() == rcPixel16));
		rmAssert(srcWin[i].depth() == destWin.depth());
		rmAssert(srcWin[i].width() == destWin.width());
		rmAssert(srcWin[i].height() == destWin.height());
	}

	vector<uint16*> src (3);
	vector<uint16*> srcUP (3);

	int32 widthInPixels = srcWin[0].width();

	const uint32 opsPerLoop = 8; // 16 / numBytes 
	uint32 unrollCnt = widthInPixels / opsPerLoop;
	uint32 unrollRem = widthInPixels % opsPerLoop;
	uint32 lastRow = srcWin[0].height() - 1, row = 0;

	uint16 ab [8], ac[8], bc[8]; // same as opsPerLoop
	uint16 abc[3];

	for ( ; row <= lastRow; row++)
	{

		for (uint32 i = 0; i < srcWin.size(); i++)
		{
			src[i] = (uint16*)srcWin[i].rowPointer(row);
		}

		uint16* dest = (uint16*)destWin.rowPointer(row);

	// Process a chunk
	// for pixels in the loop
	// calculate ab, ac, and bc
		for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
		{
			for (uint32 i = 0; i < opsPerLoop; i++, src[0]++, src[1]++, src[2]++, dest++)
			{
				abc[0] = *src[0]; abc[1] = *src[1]; abc[2] = *src[2];
				ab[i] = abc[0] > abc[1];
				ac[i] = abc[0] > abc[2];
				bc[i] = abc[1] > abc[2];

				int32 t0 = ab[i] + ac[i];
				int32 t1 = bc[i] - ab[i];
				int32 t2 = bc[i] + ac[i];

				if (t1 == 0 && (t0 % 2) != 1 && (t2 % 2) != 1)
					median = abc[1];
				else if (t1 != 0 && t0 == 1 && (t2 % 2) != 1)
					median = abc[0];		
				else if (t1 != 0 && t2 == 1 && (t0 % 2) != 1)
					median = abc[2];
				else
					rmAssert (1);

				*dest = median;
			}	      
		}

	// Left overs less than opsPerLoop. OpsPerLoop * numBytes
		memset(&ab[0], 0, 16);memset(&ac[0], 0, 16);memset(&bc[0], 0, 16);

		for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++, src[0]++, src[1]++, src[2]++, dest++)
		{
			abc[0] = *src[0]; abc[1] = *src[1]; abc[2] = *src[2];
			ab[touchCount] = abc[0] > abc[1];
			ac[touchCount] = abc[0] > abc[2];
			bc[touchCount] = abc[1] > abc[2];

			int32 t0 = ab[touchCount] + ac[touchCount];
			int32 t1 = bc[touchCount] - ab[touchCount];
			int32 t2 = bc[touchCount] + ac[touchCount];

			if (t1 == 0 && (t0 % 2) != 1 && (t2 % 2) != 1)
				median = abc[1];
			else if (t1 != 0 && t0 == 1 && (t2 % 2) != 1)
				median = abc[0];		
			else if (t1 != 0 && t2 == 1 && (t0 % 2) != 1)
				median = abc[2];
			else
				rmAssert (1);

			*dest = median;
		}
	}

}



// Create a mutual information image for this channel

void rfMutualChannel (const rcWindow& rgbInput, rcWindow& channelOutput)
{
  if ( rgbInput.isGray() ) return;

  rmAssert (rgbInput.isBound ());
  rmAssert( rgbInput.depth() == rcPixel32S );

  const uint32 width = rgbInput.width();
  const uint32 height = rgbInput.height();
  rcWindow mu (256, 256, rcPixel32S);
  mu.setAllPixels (0);
  float maxb (0.0f);
  rc2Fvector maxdv (256.0f, 256.0f);
  float maxd = maxdv.len ();

  for (uint32 j = 0; j < height; j++)
    {
      uint32 *rgbRow = (uint32 *) rgbInput.rowPointer (j);

      for (uint32 i = 0; i < width; i++, rgbRow++)
	{
	  const uint8* rgbComponents = (uint8*) rgbRow;

	  int32 red (rgbComponents[1]);
	  int32 green (rgbComponents[2]);
	  rmAssert (red >= 0);
	  rmAssert (green >= 0);
	  rc2Fvector v (red, green);
	  if (v.isLenNull()) continue;
	  float r = (v.angle() / rcRadian (rkPI/2.0)) * 255;
	  float l = (v.len() * 255) / maxd;
	  float *binPtr = (float *) mu.pelPointer ((int32) r, 
						   (int32) l);
	  *binPtr += 1;
	  float bin (*binPtr);
	  if (bin > maxb) maxb = bin;
	}
    }

  channelOutput = rfImageConvertFloat8 (mu, 0.0f, 255.0f);
}



#if 0
void rfTemporalMedianAltiVec (const vector<rcWindow>& srcWin, rcWindow& destWin)
{
  rmAssert (srcWin.size() == 3);
  uint8 median;

  for (uint32 i = 0; i < srcWin.size(); i++)
    {
      rmAssert((srcWin[i].depth() == rcPixel8));
      rmAssert(srcWin[i].depth() == destWin.depth());
      rmAssert(srcWin[i].width() == destWin.width());
      rmAssert(srcWin[i].height() == destWin.height());
    }

  vector<uint8*> src (3);
  vector<uint8*> srcUP (3);

  int32 widthInPixels = srcWin[0].width()*((int32)srcWin[0].depth());

  const uint32 opsPerLoop = 16;
  uint32 unrollCnt = widthInPixels / opsPerLoop;
  uint32 unrollRem = widthInPixels % opsPerLoop;
  uint32 lastRow = srcWin[0].height() - 1, row = 0;

  uint8 ab [16], ac[16], bc[16];
  uint8 abc[3];

  for ( ; row <= lastRow; row++)
    {

      for (uint32 i = 0; i < srcWin.size(); i++)
	{
	  src[i] = (uint8*)srcWin[i].rowPointer(row);
	}

      uint8* dest = (uint8*)destWin.rowPointer(row);

      // Process a chunk
      // for pixels in the loop
      // calculate ab, ac, and bc
      if (unrollCnt)
	{
	  vector unsigned char *srcZero = (vector unsigned char *) src[0];
	  vector unsigned char *srcOne = (vector unsigned char *) src[1];
	  vector unsigned char *srcTwo = (vector unsigned char *) src[2];
	  vector unsigned char t0, t1, t2, ab, ac, bc;

	  for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
	    {
	      for (uint32 i = 0; i < opsPerLoop; i++, src[0]++, src[1]++, src[2]++, dest++)
	    {
	      abc[0] = *src[0]; abc[1] = *src[1]; abc[2] = *src[2];

	      ab = (vector unsigned char) vec_cmpgt (*srcZero, *srcOne);
	      ac = (vector unsigned char) vec_cmpgt (*srcZero, *srcTwo);
	      bc = (vector unsigned char) vec_cmpgt (*srcOne, *srcTwo);

	      ab[i] = abc[0] > abc[1];
	      ac[i] = abc[0] > abc[2];
	      bc[i] = abc[1] > abc[2];

	      t0 = vec_add (ab, ac);
	      t1 = vec_sub (bc, ab); // @todo replace sub with a cmp
	      t2 = vec_add (bc, ac);

	      int32 t0 = ab[touchCount] + ac[touchCount];
	      int32 t1 = bc[touchCount] - ab[touchCount];
	      int32 t2 = bc[touchCount] + ac[touchCount];

	      // select pixels from sources based on the selectio logic. vec_sel 
	      // that is compute a section vector for each source. (case of half vector for illus.)
	      // srcOneSelect = 00001100, srcTwoSelect = 10100000, srcThreeSelect = 01010011 

	      if (t1 == 0 && (t0 % 2) != 1 && (t2 % 2) != 1)
		median = abc[1];
	      else if (t1 != 0 && t0 == 1 && (t2 % 2) != 1)
		median = abc[0];		
	      else if (t1 != 0 && t2 == 1 && (t0 % 2) != 1)
		median = abc[2];
	      else
		rmAssert (1);

	      *dest = median;
	    }	      
        }

      // Left overs less than opsPerLoop
      memset(&ab[0], 0, 16);memset(&ac[0], 0, 16);memset(&bc[0], 0, 16);

      for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++, src[0]++, src[1]++, src[2]++, dest++)
	{
	  abc[0] = *src[0]; abc[1] = *src[1]; abc[2] = *src[2];
	  ab[touchCount] = abc[0] > abc[1];
	  ac[touchCount] = abc[0] > abc[2];
	  bc[touchCount] = abc[1] > abc[2];

	  int32 t0 = ab[touchCount] + ac[touchCount];
	  int32 t1 = bc[touchCount] - ab[touchCount];
	  int32 t2 = bc[touchCount] + ac[touchCount];

	  if (t1 == 0 && (t0 % 2) != 1 && (t2 % 2) != 1)
	    median = abc[1];
	  else if (t1 != 0 && t0 == 1 && (t2 % 2) != 1)
	    median = abc[0];		
	  else if (t1 != 0 && t2 == 1 && (t0 % 2) != 1)
	    median = abc[2];
	  else
	    rmAssert (1);

	  *dest = median;
	}
    }
    
}

#endif
