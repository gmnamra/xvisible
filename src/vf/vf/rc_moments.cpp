/*
 *  rc_moments.cpp
 *  
 *
 *  Created by Arman Garakani on Tue Aug 13 2002.
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_window.h>
#include <rc_histstats.h>
#include <rc_vector2d.h>
#include <rc_analysis.h>
#include <rc_moments.h>

void rfCorr5by5PointMomentFit (rsCorr5by5Point& s, float& x, float&y)
{
  float sx (0), sy (0);
  float mass (0);
  static const int32 centered (5 / 2 + 1);

  for (int32 j = - 5/2; j < centered; j++)  
    for (int32 i = - 5/2; i < centered; i++)
      {
	float val = s.score [j + 5/2][i + 5/2];
	mass += val;
	sx += i * val; sy +=  j * val;
      }

  if (mass != 0)
    x = sx / mass; y = sy / mass;
}

void rfCorr5by5LineMomentFit (rsCorr5by5Line& s, float& x, float&y)
{
  float sx (0), sy (0);
  float mass (0);
  static const int32 centered (5 / 2 + 1);

  for (int32 j = - 5/2; j < centered; j++)  
    for (int32 i = - 5/2; i < centered; i++)
      {
	float val = *(s.score [j + 5 / 2][i + 5 / 2]);
	mass += val;
	sx += i * val; sy +=  j * val;
      }

  if (mass != 0)
    x = sx / mass; y = sy / mass;
}
  
  
#include <string.h>

#define CIMP_UNROLL_MODULUS 32

#define VALUE_MODULO(VALUE,MODULUS) (((int)VALUE) & (MODULUS-1))
#define CIMP_UNROLL_VALUE_MODULO(VALUE) VALUE_MODULO(VALUE, CIMP_UNROLL_MODULUS)

#define INTEGRATEPIXEL_1D { \
  uint32 v = *sp++;       \
  *ip++ = *lip++ + v;       \
  *ip++ = *lip++ + v*v; }

/* rfGen1DMomentIntegrals - Calculate column ordered integrated
 * moments for src and store in integral image.
 *
 * At the end, the integral window contains a pair of pixels for each
 * pixel in the source image. These correspond to the integrated sum
 * and sum square of the source window pixels. The correspondence goes
 * as follows:
 *
 * integral[x*2][y+1], integral[x*2+1][y+1] map respectively to the
 * SUM(I) and SUM(I*I) of pixels source[x][0] through source[x][y],
 * inclusive.
 */
void rfGen1DMomentIntegrals(rcWindow src, rcWindow integral)
{
  rmAssert(src.depth() == rcPixel8);
  rmAssert(integral.depth() == rcPixel32);
  rmAssert(sizeof(uint32) == rcPixel32);
  rmAssert(VALID_1D_MOM_INT_WIDTH(src.width()) == integral.width());
  rmAssert(VALID_1D_MOM_INT_HEIGHT(src.height()) == integral.height());

  /* Initialize loop constants
   */
  const int32 constFullBlkCnt = src.width()/CIMP_UNROLL_MODULUS;
  const int32 constBlkRemainder = CIMP_UNROLL_VALUE_MODULO(src.width());
  const int32 constInitialIndex = constBlkRemainder ? -1 : 0;
  const uint32 heightInLines = src.height();

  uint8* srcPtr = (uint8*)src.rowPointer(0);
  const uint32 srcUpdate = src.rowUpdate();
  uint32* intgrlPtr = (uint32*)integral.rowPointer(0);
  const uint32 intgrlUpdate = integral.rowUpdate()/sizeof(uint32);

  /* Set 1st row of results info to all zeros.
   */
  memset(intgrlPtr, 0, integral.width()*sizeof(uint32));
  intgrlPtr += intgrlUpdate;
  
  /* Now generate moment data for the entire image.
   */
  for (uint32 y = 0; y < heightInLines; y++) {
    uint8* sp = srcPtr;
    srcPtr += srcUpdate;
    uint32* ip = intgrlPtr;
    uint32* lip = ip - intgrlUpdate;
    intgrlPtr += intgrlUpdate;
    
    int32 index = constInitialIndex;

    if (0) // Make warnings about unreachable code go away
      goto compilerPlacebo;

    switch (constBlkRemainder)
    {
    compilerPlacebo:
      for ( ; index < constFullBlkCnt; index++)
      {
      case 0:  // 32 pixels
	INTEGRATEPIXEL_1D;
      case 31:
	INTEGRATEPIXEL_1D;
      case 30:
	INTEGRATEPIXEL_1D;
      case 29:
	INTEGRATEPIXEL_1D;
      case 28:
	INTEGRATEPIXEL_1D;
      case 27:
	INTEGRATEPIXEL_1D;
      case 26:
	INTEGRATEPIXEL_1D;
      case 25:
	INTEGRATEPIXEL_1D;
      case 24:
	INTEGRATEPIXEL_1D;
      case 23:
	INTEGRATEPIXEL_1D;
      case 22:
	INTEGRATEPIXEL_1D;
      case 21:
	INTEGRATEPIXEL_1D;
      case 20:
	INTEGRATEPIXEL_1D;
      case 19:
	INTEGRATEPIXEL_1D;
      case 18:
	INTEGRATEPIXEL_1D;
      case 17:
	INTEGRATEPIXEL_1D;
      case 16:
	INTEGRATEPIXEL_1D;
      case 15:
	INTEGRATEPIXEL_1D;
      case 14:
	INTEGRATEPIXEL_1D;
      case 13:
	INTEGRATEPIXEL_1D;
      case 12:
	INTEGRATEPIXEL_1D;
      case 11:
	INTEGRATEPIXEL_1D;
      case 10:
	INTEGRATEPIXEL_1D;
      case 9:
	INTEGRATEPIXEL_1D;
      case 8:
	INTEGRATEPIXEL_1D;
      case 7:
	INTEGRATEPIXEL_1D;
      case 6:
	INTEGRATEPIXEL_1D;
      case 5:
	INTEGRATEPIXEL_1D;
      case 4:
	INTEGRATEPIXEL_1D;
      case 3:
	INTEGRATEPIXEL_1D;
      case 2:
	INTEGRATEPIXEL_1D;
      case 1:
	INTEGRATEPIXEL_1D;
      } // End of: for ( ; index < constFullBlkCnt; index++)
    } // End of: switch (constBlkRemainder)
  } // End of: for (uint32 y = 0; y < heightInLines; y++)
}

#define INTEGRATEPIXEL_2D {                             \
  uint32 v = *sp++;                                   \
  accum += v;                                           \
  accumSq += v*v;                                       \
  *ip++ = *lip++ + accum;                               \
  int64* ip64 ((int64*)ip);				\
  int64* lip64 ((int64*)lip);				\
  *(ip64)++ = *(lip64)++ + accumSq; }

/* rfGen2DMomentIntegrals - Calculate rectangularly oriented
 * integrated moments for src and store in integral image.
 *
 * At the end, the integral window contains a "pair" of pixels for each
 * pixel in the source image. These correspond to the integrated sum
 * and sum square of the source window pixels. The correspondence goes
 * as follows:
 *
 * integral[(x+1)*3][y+1] maps to SUM(I) and
 * (integral[(x+1)*3+1][y+1], integral[(x+1)*3+2][y+1]) map to
 * SUM(I*I) in the rectangle of pixels having source[0][0] and
 * source[x][y] as its inclusive upper-left and lower-right bounding
 * points.
 *
 * Note that the sum squared requires more than 32 bits so it is an 64
 * bit int stored in a pair of consecutive 32 bit ints.
 */
void rfGen2DMomentIntegrals(rcWindow src, rcWindow integral)
{
  rmAssert(src.depth() == rcPixel8);
  rmAssert(integral.depth() == rcPixel32);
  rmAssert(sizeof(uint32) == rcPixel32);
  rmAssert(VALID_2D_MOM_INT_WIDTH(src.width()) == integral.width());
  rmAssert(VALID_2D_MOM_INT_HEIGHT(src.height()) == integral.height());

  /* Initialize loop constants
   */
  const int32 constFullBlkCnt = src.width()/CIMP_UNROLL_MODULUS;
  const int32 constBlkRemainder = CIMP_UNROLL_VALUE_MODULO(src.width());
  const int32 constInitialIndex = constBlkRemainder ? -1 : 0;
  const uint32 heightInLines = src.height();

  uint8* srcPtr = (uint8*)src.rowPointer(0);
  const uint32 srcUpdate = src.rowUpdate();
  uint32* intgrlPtr = (uint32*)integral.rowPointer(0);
  const uint32 intgrlUpdate = integral.rowUpdate()/sizeof(uint32);

  /* Set 1st row of results info to all zeros.
   */
  memset(intgrlPtr, 0, integral.width()*sizeof(uint32));

  /* Set 1st column "pair" of results info all to zeros.
   */
  for (uint32 y = intgrlUpdate; y < integral.height()*intgrlUpdate;
       y +=intgrlUpdate) {
    *(intgrlPtr + y) = *(intgrlPtr + y + 1) = *(intgrlPtr + y + 2) = 0;
  }

  /* Init intgrlPtr to point to location (3,1) in the integral image.
   * Note that this corresponds to where to store integral data for
   * location (0,0) in the souce image.
   */
  intgrlPtr += intgrlUpdate + 3;
  
  /* Now generate moment data for the entire image.
   */
  for (uint32 y = 0; y < heightInLines; y++) {
    uint32 accum = 0;
    int64 accumSq = 0;
    uint8* sp = srcPtr;
    srcPtr += srcUpdate;
    uint32* ip = intgrlPtr;
    uint32* lip = ip - intgrlUpdate;
    intgrlPtr += intgrlUpdate;
    
    int32 index = constInitialIndex;

    if (0) // Make warnings about unreachable code go away
      goto compilerPlacebo;

    switch (constBlkRemainder)
    {
    compilerPlacebo:
      for ( ; index < constFullBlkCnt; index++)
      {
      case 0:  // 32 pixels
	INTEGRATEPIXEL_2D;
      case 31:
	INTEGRATEPIXEL_2D;
      case 30:
	INTEGRATEPIXEL_2D;
      case 29:
	INTEGRATEPIXEL_2D;
      case 28:
	INTEGRATEPIXEL_2D;
      case 27:
	INTEGRATEPIXEL_2D;
      case 26:
	INTEGRATEPIXEL_2D;
      case 25:
	INTEGRATEPIXEL_2D;
      case 24:
	INTEGRATEPIXEL_2D;
      case 23:
	INTEGRATEPIXEL_2D;
      case 22:
	INTEGRATEPIXEL_2D;
      case 21:
	INTEGRATEPIXEL_2D;
      case 20:
	INTEGRATEPIXEL_2D;
      case 19:
	INTEGRATEPIXEL_2D;
      case 18:
	INTEGRATEPIXEL_2D;
      case 17:
	INTEGRATEPIXEL_2D;
      case 16:
	INTEGRATEPIXEL_2D;
      case 15:
	INTEGRATEPIXEL_2D;
      case 14:
	INTEGRATEPIXEL_2D;
      case 13:
	INTEGRATEPIXEL_2D;
      case 12:
	INTEGRATEPIXEL_2D;
      case 11:
	INTEGRATEPIXEL_2D;
      case 10:
	INTEGRATEPIXEL_2D;
      case 9:
	INTEGRATEPIXEL_2D;
      case 8:
	INTEGRATEPIXEL_2D;
      case 7:
	INTEGRATEPIXEL_2D;
      case 6:
	INTEGRATEPIXEL_2D;
      case 5:
	INTEGRATEPIXEL_2D;
      case 4:
	INTEGRATEPIXEL_2D;
      case 3:
	INTEGRATEPIXEL_2D;
      case 2:
	INTEGRATEPIXEL_2D;
      case 1:
	INTEGRATEPIXEL_2D;
      } // End of: for ( ; index < constFullBlkCnt; index++)
    } // End of: switch (constBlkRemainder)
  } // End of: for (uint32 y = 0; y < heightInLines; y++)
}

#define INTEGRATEPIXEL_2D_FAST { \
  uint32 v = *sp++;            \
  accum += v;                    \
  accumSq += v*v;                \
  *ip++ = *lip++ + accum;        \
  *ip++ = *lip++ + accumSq; }

/* rfGen2DMomentIntegralsFast - Calculate rectangularly oriented
 * integrated moments for src and store in integral image. These
 * intergral images are only guaranteed to generate integrals for
 * subwindows containing <= 65536 pixels.
 *
 * At the end, the integral window contains a pair of pixels for each
 * pixel in the source image. These correspond to the integrated sum
 * and sum square of the source window pixels. The correspondence goes
 * as follows:
 *
 * integral[(x+1)*2][y+1], integral[(x+1)*2+1][y+1] map respectively
 * to the SUM(I) and SUM(I*I) in the rectangle of pixels having
 * source[0][0] and source[x][y] as its inclusive upper-left and
 * lower-right bounding points.
 */
void rfGen2DMomentIntegralsFast(rcWindow src, rcWindow integral)
{
  rmAssert(src.depth() == rcPixel8);
  rmAssert(integral.depth() == rcPixel32);
  rmAssert(sizeof(uint32) == rcPixel32);
  rmAssert(VALID_2D_MOM_INT_FAST_WIDTH(src.width()) == integral.width());
  rmAssert(VALID_2D_MOM_INT_FAST_HEIGHT(src.height()) == integral.height());

  /* Initialize loop constants
   */
  const int32 constFullBlkCnt = src.width()/CIMP_UNROLL_MODULUS;
  const int32 constBlkRemainder = CIMP_UNROLL_VALUE_MODULO(src.width());
  const int32 constInitialIndex = constBlkRemainder ? -1 : 0;
  const uint32 heightInLines = src.height();

  uint8* srcPtr = (uint8*)src.rowPointer(0);
  const uint32 srcUpdate = src.rowUpdate();
  uint32* intgrlPtr = (uint32*)integral.rowPointer(0);
  const uint32 intgrlUpdate = integral.rowUpdate()/sizeof(uint32);

  /* Set 1st row of results info to all zeros.
   */
  memset(intgrlPtr, 0, integral.width()*sizeof(uint32));

  /* Set 1st column "pair" of results info all to zeros.
   */
  for (uint32 y = intgrlUpdate; y < integral.height()*intgrlUpdate;
       y +=intgrlUpdate) {
    *(intgrlPtr + y) = *(intgrlPtr + y + 1) = 0;
  }

  /* Init intgrlPtr to point to location (2,1) in the integral image.
   * Note that this corresponds to where to store integral data for
   * location (0,0) in the souce image.
   */
  intgrlPtr += intgrlUpdate + 2;
  
  /* Now generate moment data for the entire image.
   */
  for (uint32 y = 0; y < heightInLines; y++) {
    uint32 accum = 0;
    int32 accumSq = 0;
    uint8* sp = srcPtr;
    srcPtr += srcUpdate;
    uint32* ip = intgrlPtr;
    uint32* lip = ip - intgrlUpdate;
    intgrlPtr += intgrlUpdate;
    
    int32 index = constInitialIndex;

    if (0) // Make warnings about unreachable code go away
      goto compilerPlacebo;

    switch (constBlkRemainder)
    {
    compilerPlacebo:
      for ( ; index < constFullBlkCnt; index++)
      {
      case 0:  // 32 pixels
	INTEGRATEPIXEL_2D_FAST;
      case 31:
	INTEGRATEPIXEL_2D_FAST;
      case 30:
	INTEGRATEPIXEL_2D_FAST;
      case 29:
	INTEGRATEPIXEL_2D_FAST;
      case 28:
	INTEGRATEPIXEL_2D_FAST;
      case 27:
	INTEGRATEPIXEL_2D_FAST;
      case 26:
	INTEGRATEPIXEL_2D_FAST;
      case 25:
	INTEGRATEPIXEL_2D_FAST;
      case 24:
	INTEGRATEPIXEL_2D_FAST;
      case 23:
	INTEGRATEPIXEL_2D_FAST;
      case 22:
	INTEGRATEPIXEL_2D_FAST;
      case 21:
	INTEGRATEPIXEL_2D_FAST;
      case 20:
	INTEGRATEPIXEL_2D_FAST;
      case 19:
	INTEGRATEPIXEL_2D_FAST;
      case 18:
	INTEGRATEPIXEL_2D_FAST;
      case 17:
	INTEGRATEPIXEL_2D_FAST;
      case 16:
	INTEGRATEPIXEL_2D_FAST;
      case 15:
	INTEGRATEPIXEL_2D_FAST;
      case 14:
	INTEGRATEPIXEL_2D_FAST;
      case 13:
	INTEGRATEPIXEL_2D_FAST;
      case 12:
	INTEGRATEPIXEL_2D_FAST;
      case 11:
	INTEGRATEPIXEL_2D_FAST;
      case 10:
	INTEGRATEPIXEL_2D_FAST;
      case 9:
	INTEGRATEPIXEL_2D_FAST;
      case 8:
	INTEGRATEPIXEL_2D_FAST;
      case 7:
	INTEGRATEPIXEL_2D_FAST;
      case 6:
	INTEGRATEPIXEL_2D_FAST;
      case 5:
	INTEGRATEPIXEL_2D_FAST;
      case 4:
	INTEGRATEPIXEL_2D_FAST;
      case 3:
	INTEGRATEPIXEL_2D_FAST;
      case 2:
	INTEGRATEPIXEL_2D_FAST;
      case 1:
	INTEGRATEPIXEL_2D_FAST;
      } // End of: for ( ; index < constFullBlkCnt; index++)
    } // End of: switch (constBlkRemainder)
  } // End of: for (uint32 y = 0; y < heightInLines; y++)
}

#define INTEGRATEIM_2D {    \
    int64 v = *spi++;     \
    accum += v * (*spm++);  \
    *ip++ = *lip++ + accum; }

/* rfGen2DIMIntegrals - Calculate rectangularly oriented
 * integrated moments for src and store in integral image.
 *
 * At the end, the integral window contains a pixel for each pixel in
 * the "intersection" of the image and model windows. These correspond
 * to the integrated product of the image and model pixels. The
 * correspondence goes as follows:
 *
 * integral[x+1][y+1] maps to SUM(I*M) in the rectangle of pixels
 * having source[0][0] and source[x][y] as its inclusive upper-left
 * and lower-right bounding points.
 */
void rfGen2DIMIntegrals(rcWindow image, rcWindow model, rcWindow integral)
{
  rmAssert(image.depth() == rcPixel8);
  rmAssert(model.depth() == rcPixel8);
  rmAssert(integral.depth() == rcPixelDouble);
  rmAssert(sizeof(int64) == rcPixelDouble);
  const int32 width =
    (image.width() < model.width()) ? image.width() : model.width();
  const int32 height =
    (image.height() < model.height()) ? image.height() : model.height();
  rmAssert(VALID_2D_IM_INT_WIDTH(width) == integral.width());
  rmAssert(VALID_2D_IM_INT_HEIGHT(height) == integral.height());

  /* Initialize loop constants
   */
  const int32 constFullBlkCnt = width/CIMP_UNROLL_MODULUS;
  const int32 constBlkRemainder = CIMP_UNROLL_VALUE_MODULO(width);
  const int32 constInitialIndex = constBlkRemainder ? -1 : 0;
  const int32 heightInLines = height;

  uint8* imgPtr = (uint8*)image.rowPointer(0);
  const uint32 imgUpdate = image.rowUpdate();
  uint8* mdlPtr = (uint8*)model.rowPointer(0);
  const uint32 mdlUpdate = model.rowUpdate();
  int64* intgrlPtr = (int64*)integral.rowPointer(0);
  const uint32 intgrlUpdate = integral.rowUpdate()/sizeof(int64);

  /* Set 1st row of results info to all zeros.
   */
  memset(intgrlPtr, 0, integral.width()*sizeof(int64));

  /* Set 1st column of results info all to zeros.
   */
  for (uint32 y = intgrlUpdate; y < (integral.height() - 1)*intgrlUpdate;
       y +=intgrlUpdate) {
    *(intgrlPtr + y) = 0;
  }

  /* Init intgrlPtr to point to location (1,1) in the integral image.
   * Note that this corresponds to where to store integral data for
   * location (0,0) in the souce image.
   */
  intgrlPtr += intgrlUpdate + 1;
  
  /* Now generate moment data for the entire image.
   */
  for (int32 y = 0; y < heightInLines; y++) {
    int64 accum = 0;
    uint8* spi = imgPtr;
    imgPtr += imgUpdate;
    uint8* spm = mdlPtr;
    mdlPtr += mdlUpdate;
    int64* ip = intgrlPtr;
    int64* lip = ip - intgrlUpdate;
    intgrlPtr += intgrlUpdate;
    
    int32 index = constInitialIndex;

    if (0) // Make warnings about unreachable code go away
      goto compilerPlacebo;

    switch (constBlkRemainder)
    {
    compilerPlacebo:
      for ( ; index < constFullBlkCnt; index++)
      {
      case 0:  // 32 pixels
	INTEGRATEIM_2D;
      case 31:
	INTEGRATEIM_2D;
      case 30:
	INTEGRATEIM_2D;
      case 29:
	INTEGRATEIM_2D;
      case 28:
	INTEGRATEIM_2D;
      case 27:
	INTEGRATEIM_2D;
      case 26:
	INTEGRATEIM_2D;
      case 25:
	INTEGRATEIM_2D;
      case 24:
	INTEGRATEIM_2D;
      case 23:
	INTEGRATEIM_2D;
      case 22:
	INTEGRATEIM_2D;
      case 21:
	INTEGRATEIM_2D;
      case 20:
	INTEGRATEIM_2D;
      case 19:
	INTEGRATEIM_2D;
      case 18:
	INTEGRATEIM_2D;
      case 17:
	INTEGRATEIM_2D;
      case 16:
	INTEGRATEIM_2D;
      case 15:
	INTEGRATEIM_2D;
      case 14:
	INTEGRATEIM_2D;
      case 13:
	INTEGRATEIM_2D;
      case 12:
	INTEGRATEIM_2D;
      case 11:
	INTEGRATEIM_2D;
      case 10:
	INTEGRATEIM_2D;
      case 9:
	INTEGRATEIM_2D;
      case 8:
	INTEGRATEIM_2D;
      case 7:
	INTEGRATEIM_2D;
      case 6:
	INTEGRATEIM_2D;
      case 5:
	INTEGRATEIM_2D;
      case 4:
	INTEGRATEIM_2D;
      case 3:
	INTEGRATEIM_2D;
      case 2:
	INTEGRATEIM_2D;
      case 1:
	INTEGRATEIM_2D;
      } // End of: for ( ; index < constFullBlkCnt; index++)
    } // End of: switch (constBlkRemainder)
  } // End of: for (uint32 y = 0; y < heightInLines; y++)
}

#define INTEGRATEIM_2D_FAST {     \
    accum += (*spi++) * (*spm++); \
    *ip++ = *lip++ + accum; }

/* rfGen2DIMIntegralsFast - Calculate rectangularly oriented
 * integrated moments for src and store in integral image.
 *
 * At the end, the integral window contains a pixel for each pixel in
 * the "intersection" of the image and model windows. These correspond
 * to the integrated product of the image and model pixels. The
 * correspondence goes as follows:
 *
 * integral[x+1][y+1] maps to SUM(I*M) in the rectangle of pixels
 * having source[0][0] and source[x][y] as its inclusive upper-left
 * and lower-right bounding points.
 */
void rfGen2DIMIntegralsFast(rcWindow image, rcWindow model, rcWindow integral)
{
  rmAssert(image.depth() == rcPixel8);
  rmAssert(model.depth() == rcPixel8);
  rmAssert(integral.depth() == rcPixel32);
  const int32 width =
    (image.width() < model.width()) ? image.width() : model.width();
  const int32 height =
    (image.height() < model.height()) ? image.height() : model.height();
  rmAssert(VALID_2D_IM_INT_WIDTH(width) == integral.width());
  rmAssert(VALID_2D_IM_INT_HEIGHT(height) == integral.height());

  /* Initialize loop constants
   */
  const int32 constFullBlkCnt = width/CIMP_UNROLL_MODULUS;
  const int32 constBlkRemainder = CIMP_UNROLL_VALUE_MODULO(width);
  const int32 constInitialIndex = constBlkRemainder ? -1 : 0;
  const int32 heightInLines = height;

  uint8* imgPtr = (uint8*)image.rowPointer(0);
  const uint32 imgUpdate = image.rowUpdate();
  uint8* mdlPtr = (uint8*)model.rowPointer(0);
  const uint32 mdlUpdate = model.rowUpdate();
  uint32* intgrlPtr = (uint32*)integral.rowPointer(0);
  const uint32 intgrlUpdate = integral.rowUpdate()/sizeof(uint32);

  /* Set 1st row of results info to all zeros.
   */
  memset(intgrlPtr, 0, integral.width()*sizeof(uint32));

  /* Set 1st column of results info all to zeros.
   */
  for (uint32 y = intgrlUpdate; y < (integral.height() - 1)*intgrlUpdate;
       y +=intgrlUpdate) {
    *(intgrlPtr + y) = 0;
  }

  /* Init intgrlPtr to point to location (1,1) in the integral image.
   * Note that this corresponds to where to store integral data for
   * location (0,0) in the souce image.
   */
  intgrlPtr += intgrlUpdate + 1;
  
  /* Now generate moment data for the entire image.
   */
  for (int32 y = 0; y < heightInLines; y++) {
    uint32 accum = 0;
    uint8* spi = imgPtr;
    imgPtr += imgUpdate;
    uint8* spm = mdlPtr;
    mdlPtr += mdlUpdate;
    uint32* ip = intgrlPtr;
    uint32* lip = ip - intgrlUpdate;
    intgrlPtr += intgrlUpdate;
    
    int32 index = constInitialIndex;

    if (0) // Make warnings about unreachable code go away
      goto compilerPlacebo;

    switch (constBlkRemainder)
    {
    compilerPlacebo:
      for ( ; index < constFullBlkCnt; index++)
      {
      case 0:  // 32 pixels
	INTEGRATEIM_2D_FAST;
      case 31:
	INTEGRATEIM_2D_FAST;
      case 30:
	INTEGRATEIM_2D_FAST;
      case 29:
	INTEGRATEIM_2D_FAST;
      case 28:
	INTEGRATEIM_2D_FAST;
      case 27:
	INTEGRATEIM_2D_FAST;
      case 26:
	INTEGRATEIM_2D_FAST;
      case 25:
	INTEGRATEIM_2D_FAST;
      case 24:
	INTEGRATEIM_2D_FAST;
      case 23:
	INTEGRATEIM_2D_FAST;
      case 22:
	INTEGRATEIM_2D_FAST;
      case 21:
	INTEGRATEIM_2D_FAST;
      case 20:
	INTEGRATEIM_2D_FAST;
      case 19:
	INTEGRATEIM_2D_FAST;
      case 18:
	INTEGRATEIM_2D_FAST;
      case 17:
	INTEGRATEIM_2D_FAST;
      case 16:
	INTEGRATEIM_2D_FAST;
      case 15:
	INTEGRATEIM_2D_FAST;
      case 14:
	INTEGRATEIM_2D_FAST;
      case 13:
	INTEGRATEIM_2D_FAST;
      case 12:
	INTEGRATEIM_2D_FAST;
      case 11:
	INTEGRATEIM_2D_FAST;
      case 10:
	INTEGRATEIM_2D_FAST;
      case 9:
	INTEGRATEIM_2D_FAST;
      case 8:
	INTEGRATEIM_2D_FAST;
      case 7:
	INTEGRATEIM_2D_FAST;
      case 6:
	INTEGRATEIM_2D_FAST;
      case 5:
	INTEGRATEIM_2D_FAST;
      case 4:
	INTEGRATEIM_2D_FAST;
      case 3:
	INTEGRATEIM_2D_FAST;
      case 2:
	INTEGRATEIM_2D_FAST;
      case 1:
	INTEGRATEIM_2D_FAST;
      } // End of: for ( ; index < constFullBlkCnt; index++)
    } // End of: switch (constBlkRemainder)
  } // End of: for (uint32 y = 0; y < heightInLines; y++)
}

void rfImage8Integrals (const rcWindow& image, rcWindow& rproj, rcWindow& cproj)
{
  rmAssert (image.isBound());
  rmAssert (rproj.isBound());
  rmAssert (cproj.isBound());
  rmAssert (image.size() == rproj.size());
  rmAssert (image.size() == cproj.size());
  rmAssert (image.depth() == rcPixel8);
  rmAssert (cproj.depth() == rcPixel32);
  rmAssert (rproj.depth() == rcPixel32);   

  cproj.setAllPixels (0);
  rproj.setAllPixels (0);

  // Fillup the first row of col integrators, and the first col of row integrators
  const uint8 *p8 = image.pelPointer (0, 0);
  uint32 *r32 = (uint32 *) rproj.pelPointer (0, 0);
  for (int32 i = 0; i < image.width(); i++, p8++, r32++)
    *r32 = uint32 (*p8);

  uint32 *c32 = (uint32 *) cproj.rowPointer (0);
  uint32 crup = cproj.rowUpdate() / cproj.depth();
  for (int32 j = 0; j < image.height(); j++, c32+=crup)
    *c32 = *(image.rowPointer (j));

  // Fillup the first Column of the column integrators
  uint32 *rprev32 = (uint32 *) rproj.rowPointer (0);
  r32 = (uint32 *) rproj.rowPointer (1);
  p8 = image.rowPointer (1);
  uint32 rrup = rproj.rowUpdate() / rproj.depth();
  uint32 irup = image.rowUpdate() / image.depth();
  for (int32 j = 0; j < image.height(); j++, r32+=rrup, p8+=irup)
    {
      *r32 = *rprev32 + *p8;
      rprev32 = r32;
    }

  // Fillup the first row of the row integrators
  uint32 *cprev32 = (uint32 *) cproj.rowPointer (0);
  c32 = cprev32+1;
  p8 = image.pelPointer (1, 0);
  for (int32 i = 0; i < image.width(); i++, p8++, c32++)
    {
      *c32 = *cprev32 + *p8;
      cprev32 = c32;
    }


  // Now process from 1,1 onward. 

  
  for (int32 j = 1; j < image.height(); j++)
    {
      uint8 *p8 = (uint8 *) image.pelPointer (1, j);

      // current and previous rows for col integrators
      uint32 *rprev32 = (uint32 *) rproj.pelPointer (1, j-1);
      uint32 *r32 = (uint32 *) rproj.pelPointer (1, j);

      // current and previous integration pixel for row integrators
      uint32 *cprev32 = (uint32 *) cproj.rowPointer (j);
      uint32 *c32 = cprev32+1;

      // As we travel through pixels we are also traveling thorugh
      // equal size integrators. Update all the column integrators
      // and integration pixel of this row's integrator
      
	for (int32 i = 1; i < image.width(); i++, p8++, r32++, c32++, rprev32++)
	{
	  uint32 pel = uint32 (*p8);

	  // integrate row j of col integrators
	  *r32 = *rprev32 + pel;

	  // integrate col i of row integartors
	  *c32 = *cprev32 + pel;

	  // Roll over
	  cprev32 = c32;
	}
    }
}

