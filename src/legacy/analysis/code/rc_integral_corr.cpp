/*
 *  rc_integral_corr.cpp
 *
 *  Created by Peter Roberts on Tue Jun 03 2003.
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */

#include <stdio.h>
#include <rc_types.h>
#include <rc_analysis.h>
#include <rc_moments.h>

#define rmPrintSumSumSqImages(a) {			  \
  for (int32 i = 0; i < (a).height(); i++) {		  \
    uint32 addr = (uint32)(a).rowPointer(i);	  \
    fprintf (stderr, "\n");				  \
    for (int32 j = 0; j < (a).width()/3; j++) {	  \
      fprintf(stderr, "% 6d ", *(uint32*)(addr + j*12)); \
    }							  \
    fprintf (stderr, "\n");				  \
  }							  \
  for (int32 i = 0; i < (a).height(); i++) {		  \
    uint32 addr = (uint32)(a).rowPointer(i);	  \
    fprintf (stderr, "\n");				  \
    for (int32 j = 0; j < (a).width()/3; j++) {	  \
      int64 val = *(int64*)(addr + 4 + j*12);	  \
      fprintf(stderr, "% 6.0f ", (double)val);		  \
    }							  \
    fprintf (stderr, "\n");				  \
  }}

#define rmPrintIMImage(a) {				 \
  for (int32 i = 0; i < (a).height(); i++) {		 \
    uint32 addr = (uint32)(a).rowPointer(i);	 \
    fprintf (stderr, "\n");				 \
    for (int32 j = 0; j < (a).width(); j++) {		 \
      int64 val = *(int64*)(addr + j*8);		 \
      fprintf(stderr, "% 6.0f ", (double)val);		 \
    }							 \
    fprintf (stderr, "\n");				 \
  }}

typedef struct rsMoments {
  uint32 sumI;
  int64 sumII;
  uint32 sumM;
  int64 sumMM;
  int64 sumIM;
} rsMoments;

#define ALTIVEC_ALIGN(A) (((int)(A) + 15) & 0xFFFFFFF0)
#define IS_ALTIVEC_ALIGNED(A) (((int)(A) & 0xF) == 0)

/* Origin correction for each correlation location based on the
 * correction rules stated in rcAutoCorrelation::update(). These are
 * required to offset the artifacts of shifting the image relative to
 * itself in order to generate the IM integral images.
 */
static const rcIPair nwCorrection(0 ,0), seCorrection(-1, -1),
  neCorrection(-1, 0), swCorrection(0, -1),
  wCorrection(0, 0), eCorrection(-1, 0),
  nCorrection(0, 0), sCorrection(0, -1),
  iCorrection(0, 0),
  fnwCorrection(0, 0), nnwCorrection(0, 0), fnCorrection(0, 0),
  nneCorrection(-1, 0), fneCorrection(-2, 0), wnwCorrection(0, 0),
  eneCorrection(-2, -0), fwCorrection(0, 0), feCorrection(-2, 0),
  wswCorrection(0, -1), eseCorrection(-2, -1), fswCorrection(0, -2),
  sswCorrection(0, -2), fsCorrection(0, -2), sseCorrection(-1, -2),
  fseCorrection(-2, -2);

/* Origin correction, for 3 by 3 autocorrelation, based on the
 * relative location of the image within the search space.
 */
static const rcIPair
  nwOrigin3(0, 0), nOrigin3(1, 0), neOrigin3(2, 0),
  wOrigin3(0, 1),  iOrigin3(1, 1), eOrigin3(2, 1),
  swOrigin3(0, 2), sOrigin3(1, 2), seOrigin3(2, 2);

/* Final 3 by 3 correction into integral image.
 */
static const rcIPair nwOffset3(nwCorrection + nwOrigin3),
  seOffset3(seCorrection + seOrigin3),
  neOffset3(neCorrection + neOrigin3),
  swOffset3(swCorrection + swOrigin3),
  wOffset3(wCorrection + wOrigin3),
  eOffset3(eCorrection + eOrigin3),
  nOffset3(nCorrection + nOrigin3),
  sOffset3(sCorrection + sOrigin3),
  iOffset3(iCorrection + iOrigin3);

/* Origin correction, for 5 by 5 autocorrelation, based on the
 * relative location of the image within the search space.
 */
static const rcIPair
  fnwOrigin5(0, 0), nnwOrigin5(1, 0), fnOrigin5(2, 0), nneOrigin5(3, 0),
  fneOrigin5(4, 0),
  wnwOrigin5(0, 1), nwOrigin5(1, 1), nOrigin5(2, 1), neOrigin5(3, 1),
  eneOrigin5(4, 1),
  fwOrigin5(0, 2), wOrigin5(1, 2), iOrigin5(2, 2), eOrigin5(3, 2),
  feOrigin5(4, 2),
  wswOrigin5(0, 3), swOrigin5(1, 3), sOrigin5(2,3), seOrigin5(3, 3),
  eseOrigin5(4, 3),
  fswOrigin5(0, 4), sswOrigin5(1, 4), fsOrigin5(2,4), sseOrigin5(3, 4),
  fseOrigin5(4, 4);

/* Final 5 by 5 correction into integral image.
 */
static const rcIPair
  fnwOffset5(fnwCorrection + fnwOrigin5),
  nnwOffset5(nnwCorrection + nnwOrigin5),
  fnOffset5(fnCorrection + fnOrigin5),
  nneOffset5(nneCorrection + nneOrigin5),
  fneOffset5(fneCorrection + fneOrigin5),

  wnwOffset5(wnwCorrection + wnwOrigin5),
  nwOffset5(nwCorrection + nwOrigin5),
  nOffset5(nCorrection + nOrigin5),
  neOffset5(neCorrection + neOrigin5),
  eneOffset5(eneCorrection + eneOrigin5),

  fwOffset5(fwCorrection + fwOrigin5),
  wOffset5(wCorrection + wOrigin5),
  iOffset5(iCorrection + iOrigin5),
  eOffset5(eCorrection + eOrigin5),
  feOffset5(feCorrection + feOrigin5),

  wswOffset5(wswCorrection + wswOrigin5),
  swOffset5(swCorrection + swOrigin5),
  sOffset5(sCorrection + sOrigin5),
  seOffset5(seCorrection + seOrigin5),
  eseOffset5(eseCorrection + eseOrigin5),

  fswOffset5(fswCorrection + fswOrigin5),
  sswOffset5(sswCorrection + sswOrigin5),
  fsOffset5(fsCorrection + fsOrigin5),
  sseOffset5(sseCorrection + sseOrigin5),
  fseOffset5(fseCorrection + fseOrigin5);

static int rfGetPrefetchConstant(uint32 blockSizeInVectors,
				 uint32 blockCount,
				 int blockStride)
{
  rmAssert(blockSizeInVectors > 0 && blockSizeInVectors <= 32);
  rmAssert(blockCount > 0 && blockCount <= 256);
  rmAssert(blockStride > -32769 && blockStride <= 32767);
  return ((blockSizeInVectors << 24) & 0x1F000000) |
    ((blockCount << 16) && 0x00FF0000) |
    (blockStride & 0xFFFF);
}


#define PRINT_DEBUG  0
#define PRINT_DEBUG2 0
#define PRINT_DEBUG3 0
#define PRINT_DEBUG5 0


rcMomentGenerator::rcMomentGenerator(integralType type)
  : _type(type),
    _momGen((_type == eMoment1D) ? &rcMomentGenerator::genSumSumSq1D :
	    ((_type == eMoment2D) ? &rcMomentGenerator::genSumSumSq2D :
	     &rcMomentGenerator::genSumSumSq2DFast)),
    _vProject((_type == eMoment1D) ? &rcMomentGenerator::vProject1D :
	      ((_type == eMoment2D) ? &rcMomentGenerator::vProject2D :
	       &rcMomentGenerator::vProject2DFast)),
    _hProject((_type == eMoment1D) ? &rcMomentGenerator::hProject1D :
	      ((_type == eMoment2D) ? &rcMomentGenerator::hProject2D :
	       &rcMomentGenerator::hProject2DFast)),
    _imgSqBase(0)
{
}

rcMomentGenerator::~rcMomentGenerator()
{
}

void rcMomentGenerator::update(const rcWindow& frame)
{
  /* First, if necessary, resize _imgSq. Next, generate new moment
   * integrals.
   */
  if (_type == eMoment1D) {
    int32 validWidth = VALID_1D_MOM_INT_WIDTH(frame.width());
    int32 validHeight = VALID_1D_MOM_INT_HEIGHT(frame.height());

    if ((_imgSqBase == 0) || (validWidth != _imgSq.width()) ||
	(validHeight != _imgSq.height())) {
      _imgSq = rcWindow(validWidth, validHeight, rcPixel32);
      _imgSqBase = (uint32*)_imgSq.rowPointer(0);
      _imgSqRowUpdate = _imgSq.rowUpdate()/sizeof(uint32);
    }
    
    rfGen1DMomentIntegrals(frame, _imgSq);
  }
  else if (_type == eMoment2D) {
    int32 validWidth = VALID_2D_MOM_INT_WIDTH(frame.width());
    int32 validHeight = VALID_2D_MOM_INT_HEIGHT(frame.height());

    if ((_imgSqBase == 0) || (validWidth != _imgSq.width()) ||
	(validHeight != _imgSq.height())) {
      _imgSq = rcWindow(validWidth, validHeight, rcPixel32);
      _imgSqBase = (uint32*)_imgSq.rowPointer(0);
      _imgSqRowUpdate = _imgSq.rowUpdate()/sizeof(uint32);
    }
    
    rfGen2DMomentIntegrals(frame, _imgSq);
  }
  else if (_type == eMoment2DFast) {
    int32 validWidth = VALID_2D_MOM_INT_FAST_WIDTH(frame.width());
    int32 validHeight = VALID_2D_MOM_INT_FAST_HEIGHT(frame.height());

    if ((_imgSqBase == 0) || (validWidth != _imgSq.width()) ||
	(validHeight != _imgSq.height())) {
      _imgSq = rcWindow(validWidth, validHeight, rcPixel32);
      _imgSqBase = (uint32*)_imgSq.rowPointer(0);
      _imgSqRowUpdate = _imgSq.rowUpdate()/sizeof(uint32);
    }
    
    rfGen2DMomentIntegralsFast(frame, _imgSq);
  }
  else
    rmAssert(0);
}

void rcMomentGenerator::genSumSumSq1D(const rcRect& loc, float& sum,
				      float& sumSq) const
{
  rmAssert(_imgSqBase);

  const int32 x0 = loc.x()*2;
  const int32 y0 = loc.y();
  const int32 x1 = x0 + (loc.width() - 1)*2;
  const int32 y1 = y0 + loc.height();
  rmAssert(0 <= x0);
  rmAssert(x0 <= x1);
  rmAssert(x1 <= _imgSq.width());
  rmAssert(0 <= y0);
  rmAssert(y0 < y1);
  rmAssert(y1 < (_imgSq.height()-1)); // Subtract 1 to discount guard row
  
  uint32* iBottom = imgSumPtr(x0, y1);
  const uint32* iBLast = iBottom + (x1 - x0);
  uint32* iTop = imgSumPtr(x0, y0);
  const uint32* iTLast = iTop + (x1 - x0);

  int64 iSum = 0;
  int64 iSumSq = 0;
  while (iBottom <= iBLast) {
    iSum += *iBottom++ - *iTop++;
    iSumSq += *iBottom++ - *iTop++;
  }
  sum = iSum;
  sumSq = iSumSq;
  rmAssert(iBottom == (iBLast + 2));
  rmAssert(iTop == (iTLast + 2));
}

void rcMomentGenerator::genSumSumSq2D(const rcRect& loc, float& sum,
				      float& sumSq) const
{
  rmAssert(_imgSqBase);

  const int32 x0 = loc.x()*3;
  const int32 y0 = loc.y();
  const int32 x1 = x0 + loc.width()*3;
  const int32 y1 = y0 + loc.height();
  rmAssert(0 <= x0);
  rmAssert(x0 < x1);
  rmAssert((x1+3) <= _imgSq.width());
  rmAssert(0 <= y0);
  rmAssert(y0 < y1);
  rmAssert(y1 < _imgSq.height()-1); // Subtract 1 to discount guard row
    
  sum = *imgSumPtr(x1, y1) - *imgSumPtr(x0, y1) +
    *imgSumPtr(x0, y0) - *imgSumPtr(x1, y0);
    
  sumSq = *imgSumSqPtr(x1+1, y1) - *imgSumSqPtr(x0+1, y1) +
    *imgSumSqPtr(x0+1, y0) - *imgSumSqPtr(x1+1, y0);
}

void rcMomentGenerator::genSumSumSq2DFast(const rcRect& loc, float& sum,
					  float& sumSq) const
{
  rmAssert(_imgSqBase);
  const int32 width = loc.width();
  const int32 height = loc.height();
  rmAssert(width*height <= 65536);
  const int32 x0 = loc.x()*2;
  const int32 y0 = loc.y();
  const int32 x1 = x0 + width*2;
  const int32 y1 = y0 + height;
  rmAssert(0 <= x0);
  rmAssert(x0 < x1);
  rmAssert((x1+2) <= _imgSq.width());
  rmAssert(0 <= y0);
  rmAssert(y0 < y1);
  rmAssert(y1 < _imgSq.height()-1); // Subtract 1 to discount guard row
    
  sum = *imgSumPtr(x1, y1) - *imgSumPtr(x0, y1) +
    *imgSumPtr(x0, y0) - *imgSumPtr(x1, y0);
    
  sumSq = *imgSumSqPtrFast(x1+1, y1) - *imgSumSqPtrFast(x0+1, y1) +
    *imgSumSqPtrFast(x0+1, y0) - *imgSumSqPtrFast(x1+1, y0);
}

void rcMomentGenerator::vProject1D(const rcRect& loc, rcWindow& project) const
{
  rmAssert(_imgSqBase);

  const int32 x0 = loc.x()*2;
  const int32 y0 = loc.y();
  const int32 x1 = x0 + (loc.width() - 1)*2;
  const int32 y1 = y0 + loc.height();
  rmAssert(0 <= x0);
  rmAssert(x0 <= x1);
  rmAssert(x1 <= _imgSq.width());
  rmAssert(0 <= y0);
  rmAssert(y0 < y1);
  rmAssert(y1 < (_imgSq.height()-1)); // Subtract 1 to discount guard row
  
  rmAssert(project.depth() == rcPixel32);
  rmAssert(project.height() == 1);
  rmAssert(project.width() == loc.width());

  uint32* iBottom = imgSumPtr(x0, y1);
  const uint32* iBLast = iBottom + (x1 - x0);
  uint32* iTop = imgSumPtr(x0, y0);
  const uint32* iTLast = iTop + (x1 - x0);
  uint32 index = 0;

  while (iBottom <= iBLast) {
    project.setPixel(index++, 0, *iBottom - *iTop);
    iBottom += 2;
    iTop += 2;
  }

  rmAssert(iBottom == (iBLast + 2));
  rmAssert(iTop == (iTLast + 2));
}

void rcMomentGenerator::vProject2D(const rcRect& loc, rcWindow& project) const
{
  rmAssert(_imgSqBase);

  const int32 x0 = loc.x()*3;
  const int32 y0 = loc.y();
  const int32 x1 = x0 + loc.width()*3;
  const int32 y1 = y0 + loc.height();
  rmAssert(0 <= x0);
  rmAssert(x0 < x1);
  rmAssert((x1+3) <= _imgSq.width());
  rmAssert(0 <= y0);
  rmAssert(y0 < y1);
  rmAssert(y1 < _imgSq.height()-1); // Subtract 1 to discount guard row

  rmAssert(project.depth() == rcPixel32);
  rmAssert(project.height() == 1);
  rmAssert(project.width() == loc.width());

  uint32* iBottom = imgSumPtr(x0, y1);
  const uint32* iBLast = iBottom + (x1 - x0);
  uint32* iTop = imgSumPtr(x0, y0);
  const uint32* iTLast = iTop + (x1 - x0);
  uint32 index = 0;

  uint32 prevSum = *iBottom - *iTop;
  iBottom += 3;
  iTop += 3;

  while (iBottom <= iBLast) {
    uint32 sum = *iBottom - *iTop;
    iBottom += 3;
    iTop += 3;
    project.setPixel(index++, 0, sum - prevSum);
    prevSum = sum;
  }

  rmAssert(iBottom == (iBLast + 3));
  rmAssert(iTop == (iTLast + 3));
}

void rcMomentGenerator::vProject2DFast(const rcRect& loc, rcWindow& project) const
{
  rmAssert(_imgSqBase);

  const int32 width = loc.width();
  const int32 height = loc.height();
  rmAssert(width*height <= 65536);
  const int32 x0 = loc.x()*2;
  const int32 y0 = loc.y();
  const int32 x1 = x0 + width*2;
  const int32 y1 = y0 + height;
  rmAssert(0 <= x0);
  rmAssert(x0 < x1);
  rmAssert((x1+2) <= _imgSq.width());
  rmAssert(0 <= y0);
  rmAssert(y0 < y1);
  rmAssert(y1 < _imgSq.height()-1); // Subtract 1 to discount guard row

  rmAssert(project.depth() == rcPixel32);
  rmAssert(project.height() == 1);
  rmAssert(project.width() == loc.width());

  uint32* iBottom = imgSumPtr(x0, y1);
  const uint32* iBLast = iBottom + (x1 - x0);
  uint32* iTop = imgSumPtr(x0, y0);
  const uint32* iTLast = iTop + (x1 - x0);
  uint32 index = 0;

  uint32 prevSum = *iBottom - *iTop;
  iBottom += 2;
  iTop += 2;

  while (iBottom <= iBLast) {
    uint32 sum = *iBottom - *iTop;
    iBottom += 2;
    iTop += 2;
    project.setPixel(index++, 0, sum - prevSum);
    prevSum = sum;
  }

  rmAssert(iBottom == (iBLast + 2));
  rmAssert(iTop == (iTLast + 2));
}

void rcMomentGenerator::hProject1D(const rcRect&, rcWindow&) const
{
  rmAssert(0);
}

void rcMomentGenerator::hProject2D(const rcRect& loc, rcWindow& project) const
{
  rmAssert(_imgSqBase);

  const int32 height = loc.height();
  const int32 x0 = loc.x()*3;
  const int32 y0 = loc.y();
  const int32 x1 = x0 + loc.width()*3;
  const int32 y1 = y0 + height;
  rmAssert(0 <= x0);
  rmAssert(x0 < x1);
  rmAssert((x1+3) <= _imgSq.width());
  rmAssert(0 <= y0);
  rmAssert(y0 < y1);
  rmAssert(y1 < _imgSq.height()-1); // Subtract 1 to discount guard row

  rmAssert(project.depth() == rcPixel32);
  rmAssert(project.height() == 1);
  rmAssert(project.width() == loc.height());

  const int32 rowUpdate = _imgSqRowUpdate;

  uint32* iLeft = imgSumPtr(x0, y0);
  uint32* iRight = imgSumPtr(x1, y0);

  uint32 prevSum = *iRight - *iLeft;
  iLeft += rowUpdate;
  iRight += rowUpdate;

  for (int32 index = 0; index < height; index++) {
    uint32 sum = *iRight - *iLeft;
    iLeft += rowUpdate;
    iRight += rowUpdate;
    project.setPixel(index, 0, sum - prevSum);
    prevSum = sum;
  }
}

void rcMomentGenerator::hProject2DFast(const rcRect& loc, rcWindow& project) const
{
  rmAssert(_imgSqBase);

  const int32 width = loc.width();
  const int32 height = loc.height();
  rmAssert(width*height <= 65536);
  const int32 x0 = loc.x()*2;
  const int32 y0 = loc.y();
  const int32 x1 = x0 + width*2;
  const int32 y1 = y0 + height;
  rmAssert(0 <= x0);
  rmAssert(x0 < x1);
  rmAssert((x1+2) <= _imgSq.width());
  rmAssert(0 <= y0);
  rmAssert(y0 < y1);
  rmAssert(y1 < _imgSq.height()-1); // Subtract 1 to discount guard row

  rmAssert(project.depth() == rcPixel32);
  rmAssert(project.height() == 1);
  rmAssert(project.width() == loc.height());

  const int32 rowUpdate = _imgSqRowUpdate;

  uint32* iLeft = imgSumPtr(x0, y0);
  uint32* iRight = imgSumPtr(x1, y0);

  uint32 prevSum = *iRight - *iLeft;
  iLeft += rowUpdate;
  iRight += rowUpdate;

  for (int32 index = 0; index < height; index++) {
    uint32 sum = *iRight - *iLeft;
    iLeft += rowUpdate;
    iRight += rowUpdate;
    project.setPixel(index, 0, sum - prevSum);
    prevSum = sum;
  }
}

rcAutoCorrelation::rcAutoCorrelation() : _updatePixelDepth (rcPixel8)
{
}
 
rcAutoCorrelation::~rcAutoCorrelation()
{
  clearCache();
}

inline int64* rcAutoCorrelation::nwIMSumPtr(const int32 x,
					      const int32 y) const
{
  return _nwBase + y*_nwRowUpdate + x;
}

inline int64* rcAutoCorrelation::neIMSumPtr(const int32 x,
					      const int32 y) const
{
  return _neBase + y*_neRowUpdate + x;
}

inline int64* rcAutoCorrelation::nIMSumPtr(const int32 x,
					     const int32 y) const
{
  return _nBase + y*_nRowUpdate + x;
}

inline int64* rcAutoCorrelation::wIMSumPtr(const int32 x,
					     const int32 y) const
{
  return _wBase + y*_wRowUpdate + x;
}

inline int64* rcAutoCorrelation::fnwIMSumPtr(const int32 x,
					       const int32 y) const
{
  return _fnwBase + y*_fnwRowUpdate + x;
}

inline int64* rcAutoCorrelation::fneIMSumPtr(const int32 x,
					       const int32 y) const
{
  return _fneBase + y*_fneRowUpdate + x;
}

inline int64* rcAutoCorrelation::fnIMSumPtr(const int32 x,
					      const int32 y) const
{
  return _fnBase + y*_fnRowUpdate + x;
}

inline int64* rcAutoCorrelation::fwIMSumPtr(const int32 x,
					      const int32 y) const
{
  return _fwBase + y*_fwRowUpdate + x;
}

inline int64* rcAutoCorrelation::nnwIMSumPtr(const int32 x,
					       const int32 y) const
{
  return _nnwBase + y*_nnwRowUpdate + x;
}

inline int64* rcAutoCorrelation::wnwIMSumPtr(const int32 x,
					       const int32 y) const
{
  return _wnwBase + y*_wnwRowUpdate + x;
}

inline int64* rcAutoCorrelation::wswIMSumPtr(const int32 x,
					       const int32 y) const
{
  return _wswBase + y*_wswRowUpdate + x;
}

inline int64* rcAutoCorrelation::sswIMSumPtr(const int32 x,
					       const int32 y) const
{
  return _sswBase + y*_sswRowUpdate + x;
}

inline uint32* rcAutoCorrelation::nwIMSumPtr32(const int32 x,
						 const int32 y) const
{
  return (uint32*)(_nwBase + y*_nwRowUpdate + x);
}

inline uint32* rcAutoCorrelation::neIMSumPtr32(const int32 x,
						 const int32 y) const
{
  return (uint32*)(_neBase + y*_neRowUpdate + x);
}

inline uint32* rcAutoCorrelation::nIMSumPtr32(const int32 x,
						const int32 y) const
{
  return (uint32*)(_nBase + y*_nRowUpdate + x);
}

inline uint32* rcAutoCorrelation::wIMSumPtr32(const int32 x,
						const int32 y) const
{
  return (uint32*)(_wBase + y*_wRowUpdate + x);
}

inline uint32* rcAutoCorrelation::fnwIMSumPtr32(const int32 x,
						  const int32 y) const
{
  return (uint32*)(_fnwBase + y*_fnwRowUpdate + x);
}

inline uint32* rcAutoCorrelation::fneIMSumPtr32(const int32 x,
						  const int32 y) const
{
  return (uint32*)(_fneBase + y*_fneRowUpdate + x);
}

inline uint32* rcAutoCorrelation::fnIMSumPtr32(const int32 x,
						 const int32 y) const
{
  return (uint32*)(_fnBase + y*_fnRowUpdate + x);
}

inline uint32* rcAutoCorrelation::fwIMSumPtr32(const int32 x,
						 const int32 y) const
{
  return (uint32*)(_fwBase + y*_fwRowUpdate + x);
}

inline uint32* rcAutoCorrelation::nnwIMSumPtr32(const int32 x,
						  const int32 y) const
{
  return (uint32*)(_nnwBase + y*_nnwRowUpdate + x);
}

inline uint32* rcAutoCorrelation::wnwIMSumPtr32(const int32 x,
						  const int32 y) const
{
  return (uint32*)(_wnwBase + y*_wnwRowUpdate + x);
}

inline uint32* rcAutoCorrelation::wswIMSumPtr32(const int32 x,
						  const int32 y) const
{
  return (uint32*)(_wswBase + y*_wswRowUpdate + x);
}

inline uint32* rcAutoCorrelation::sswIMSumPtr32(const int32 x,
						  const int32 y) const
{
  return (uint32*)(_sswBase + y*_sswRowUpdate + x);
}

inline uint32* rcAutoCorrelation::nwIM32SumPtr(const int32 x,
						 const int32 y) const
{
  return (uint32*)_nwBase + y*_nwRowUpdate + x;
}

inline uint32* rcAutoCorrelation::neIM32SumPtr(const int32 x,
						 const int32 y) const
{
  return (uint32*)_neBase + y*_neRowUpdate + x;
}

inline uint32* rcAutoCorrelation::nIM32SumPtr(const int32 x,
						const int32 y) const
{
  return (uint32*)_nBase + y*_nRowUpdate + x;
}

inline uint32* rcAutoCorrelation::wIM32SumPtr(const int32 x,
						const int32 y) const
{
  return (uint32*)_wBase + y*_wRowUpdate + x;
}

inline uint32* rcAutoCorrelation::fnwIM32SumPtr(const int32 x,
						  const int32 y) const
{
  return (uint32*)_fnwBase + y*_fnwRowUpdate + x;
}

inline uint32* rcAutoCorrelation::fneIM32SumPtr(const int32 x,
						  const int32 y) const
{
  return (uint32*)_fneBase + y*_fneRowUpdate + x;
}

inline uint32* rcAutoCorrelation::fnIM32SumPtr(const int32 x,
						 const int32 y) const
{
  return (uint32*)_fnBase + y*_fnRowUpdate + x;
}

inline uint32* rcAutoCorrelation::fwIM32SumPtr(const int32 x,
						 const int32 y) const
{
  return (uint32*)_fwBase + y*_fwRowUpdate + x;
}

inline uint32* rcAutoCorrelation::nnwIM32SumPtr(const int32 x,
						  const int32 y) const
{
  return (uint32*)_nnwBase + y*_nnwRowUpdate + x;
}

inline uint32* rcAutoCorrelation::wnwIM32SumPtr(const int32 x,
						  const int32 y) const
{
  return (uint32*)_wnwBase + y*_wnwRowUpdate + x;
}

inline uint32* rcAutoCorrelation::wswIM32SumPtr(const int32 x,
						  const int32 y) const
{
  return (uint32*)_wswBase + y*_wswRowUpdate + x;
}

inline uint32* rcAutoCorrelation::sswIM32SumPtr(const int32 x,
						  const int32 y) const
{
  return (uint32*)_sswBase + y*_sswRowUpdate + x;
}




void rcAutoCorrelation::genPoint(const rcRect& srchSpace,
				 vector<vector<rcCorr> >& res, const int32 acRadius,  resultSpace space ) const  
{
  rmAssert(acRadius > 0);
  const int32 diameter = acRadius*2 + 1;
  int32 xOrigin = srchSpace.x();
  int32 yOrigin = srchSpace.y();

  /* For a diameter by diameter correlation space, target model
   * dimensions are the search space dimensions reduced by
   * (diameter-1) in both width and height.
   */
  rmAssert(srchSpace.width() > (diameter-1));
  rmAssert(srchSpace.height() > (diameter-1));
  const int32 width = srchSpace.width() - (diameter-1);
  const int32 height = srchSpace.height() - (diameter-1);

  int32 xBound = xOrigin + width + (diameter-2);
  int32 yBound = yOrigin + height + (diameter-2);
  rmAssert(xOrigin < xBound);
  rmAssert(yOrigin < yBound);

  const bool useIntegrals = false;
    rmAssert(_updateFrame.frameBuf());
    rmAssert(xBound <= _updateFrame.width());
    rmAssert(yBound <= _updateFrame.height());

    
  /* Both verify that the 2D result vector is large enough and
   * initialize result entries all to -1.0. The latter allows for a
   * simple mechanism to determine whether or not a correlation score
   * needs to be calculated.
   */
  rmAssert((int32)(res.size()) >= diameter);
  for (int32 i = 0; i < diameter; i++)
    {
    rmAssert((int32)(res[i].size()) >= diameter);
    for (int32 j = 0; j < diameter; j++)
      res[i][j].r (-1.0);
    }

  rsCorrParams cp;
  rcCorr cr;
  rcWindow model(_updateFrame, xOrigin + acRadius, yOrigin + acRadius,
		 width, height);
	
  rfCorrelate(model, model, cp, res[acRadius][acRadius]);

  if (space == eFull)
    {
    for (int32 y = 0; y < diameter; y++)
      for (int32 x = 0; x < diameter; x++)
	if (res[y][x].r()  == -1.0)
          {
	  rcWindow image(_updateFrame, xOrigin + x, yOrigin + y, width, height);
	  rfCorrelate(image, model, cp, res[y][x]);
	}
  }
  else
    {
    for (int32 i = 0; i < diameter; i++) {
      if (res[acRadius][i].r()  == -1.0) {
	rcWindow image(_updateFrame, xOrigin + i, yOrigin + acRadius,
		       width, height);
	rfCorrelate(image, model, cp, res[acRadius][i]);
      }
      if (res[i][acRadius].r() == -1.0) {
	rcWindow image(_updateFrame, xOrigin + acRadius, yOrigin + i,  width, height);
	rfCorrelate(image, model, cp, res[i][acRadius]);
      }
    }
  }

  
}

void rcAutoCorrelation::gen3by3Point (const rcRect& srchSpace, rsCorr3by3Point& res,  resultSpace space ) const
{
  /* For 3x3 correlation space, target dimensions are always (-2,-2)
   * of the search space dimensions.
   */
  vector < vector < rcCorr > > cres;
  cres.resize (3);
  for (int i = 0; i < 3; i++)
    cres[i].resize (3);

  genPoint (srchSpace, cres, 1, space);

  for (uint32 yM = 0; yM < 3; yM++)
    for (uint32 xM = 0; xM < 3; xM++)
      {
      if ((xM & yM) == 1)
	continue;
      else if (_crossOnly && (((xM+yM) & 1) == 0))
	res.score[yM][xM] = 1.;
      else
        {
	  res.score[yM][xM] = cres[yM][xM].r();
	}
      }
}



void rcAutoCorrelation::gen5by5Point(const rcRect& srchSpace,
					rsCorr5by5Point& res,  resultSpace space  ) const
{

 /* For 3x3 correlation space, target dimensions are always (-2,-2)
   * of the search space dimensions.
   */
  vector < vector < rcCorr > > cres;
  cres.resize (3);
  for (int i = 0; i < 3; i++)
    cres[i].resize (3);

  genPoint (srchSpace, cres, 1, space);

  for (uint32 yM = 0; yM < 5; yM++)
    for (uint32 xM = 0; xM < 5; xM++)
		{
      if ((xM == 2) && (yM == 2))
	continue;
      else if (_crossOnly && (xM != 2) && (yM != 2))
	res.score[yM][xM] = 1.;
      else
        {
	  res.score[yM][xM] = cres[yM][xM].r();          
				}
		}
}

void rcAutoCorrelation::genCache()
{
  const int32 width = _allocWidth;

  if (_momSS._imgSq.frameBuf()) {
    rmAssert(_baseP);
    free(_baseP);
  }

  int32 allocCnt =
    4*sizeof(float) +           // Space for storing pixel count
    (width+2)*sizeof(float)*3 +     // Space for image sums
    (width+2)*sizeof(float)*3 +     // Space for image "var X sum squares"
    (width-1)*sizeof(float) +   // Space for w & e correlation results
    (width-1)*sizeof(float)*2 + // Space for n & s correlation results
    16*10;                       // Allow for rounding to 0 mod 16 address

  if (_radius == 2) {
    allocCnt +=
      (width-1)*sizeof(float) +   // Space for fw & fe correlation results
      (width-1)*sizeof(float)*3 + // Space for fn & fs correlation results
      16*4;                       // Allow for rounding to 0 mod 16 address
  }

  if (!_crossOnly) {
    allocCnt +=
      (width-1)*sizeof(float)*2 + // Space for nw & se correlation results
      (width-1)*sizeof(float)*2 + // Space for ne & sw correlation results
      16*4;                       // Allow for rounding to 0 mod 16 address

    if (_radius == 2) {
      allocCnt +=
	(width-1)*sizeof(float)*2 + // Space for wnw & ese correlation results
	(width-1)*sizeof(float)*2 + // Space for wsw & ene correlation results
	(width-1)*sizeof(float)*3 + // Space for fnw & fse correlation results
	(width-1)*sizeof(float)*3 + // Space for fne & fsw correlation results
	(width-1)*sizeof(float)*3 + // Space for nnw & sse correlation results
	(width-1)*sizeof(float)*3 + // Space for ssw & nne correlation results
	16*16;                      // Allow for rounding to 0 mod 16 address
    }
  }
  
  _baseP = (char*)malloc(allocCnt);
  rmAssert(_baseP);
    
  _size =    (float*)ALTIVEC_ALIGN(_baseP);
  _sumP[0] = &_size[4];
  _sumP[1] = (float*)ALTIVEC_ALIGN(_sumP[0] + width);
  _sumP[2] = (float*)ALTIVEC_ALIGN(_sumP[1] + width);
  _vxsP[0] = (float*)ALTIVEC_ALIGN(_sumP[2] + width);
  _vxsP[1] = (float*)ALTIVEC_ALIGN(_vxsP[0] + width);
  _vxsP[2] = (float*)ALTIVEC_ALIGN(_vxsP[1] + width);
  _wP =      (float*)ALTIVEC_ALIGN(_vxsP[2] + width);
  _nP[0] =   (float*)ALTIVEC_ALIGN(_wP + width - 1);
  _nP[1] =   (float*)ALTIVEC_ALIGN(_nP[0] + width - 1);
    
  if (_crossOnly) {
    _nwP[0] = _neP[0] = _nwP[1] = _neP[1] = 0;
    _wnwP[0] = _wnwP[1] = 0;
    _wswP[0] = _wswP[1] = 0;
    _fnwP[0] = _fnwP[1] = _fnwP[2] = 0;
    _fneP[0] = _fneP[1] = _fneP[2] = 0;
    _nnwP[0] = _nnwP[1] = _nnwP[2] = 0;
    _sswP[0] = _sswP[1] = _sswP[2] = 0;
    
    if (_radius == 2) {
      _fwP = (float*)ALTIVEC_ALIGN(_nP[1] + width - 1);
      _fnP[0] = (float*)ALTIVEC_ALIGN(_fwP + width - 1);
      _fnP[1] = (float*)ALTIVEC_ALIGN(_fnP[0] + width - 1);
      _fnP[2] = (float*)ALTIVEC_ALIGN(_fnP[1] + width - 1);
    }
    else
      _fwP = _fnP[0] = _fnP[1] = _fnP[2] = 0;
  }
  else {
    _nwP[0] = (float*)ALTIVEC_ALIGN(_nP[1] + width - 1);
    _nwP[1] = (float*)ALTIVEC_ALIGN(_nwP[0] + width - 1);
    _neP[0] = (float*)ALTIVEC_ALIGN(_nwP[1] + width - 1);
    _neP[1] = (float*)ALTIVEC_ALIGN(_neP[0] + width - 1);

    if (_radius == 2) {
      _fwP = (float*)ALTIVEC_ALIGN(_neP[1] + width - 1);
      _fnP[0] = (float*)ALTIVEC_ALIGN(_fwP + width - 1);
      _fnP[1] = (float*)ALTIVEC_ALIGN(_fnP[0] + width - 1);
      _fnP[2] = (float*)ALTIVEC_ALIGN(_fnP[1] + width - 1);
      _wnwP[0] = (float*)ALTIVEC_ALIGN(_fnP[2] + width - 1);
      _wnwP[1] = (float*)ALTIVEC_ALIGN(_wnwP[0] + width - 1);
      _wswP[0] = (float*)ALTIVEC_ALIGN(_wnwP[1] + width - 1);
      _wswP[1] = (float*)ALTIVEC_ALIGN(_wswP[0] + width - 1);
      _fneP[0] = (float*)ALTIVEC_ALIGN(_wswP[1] + width - 1);
      _fneP[1] = (float*)ALTIVEC_ALIGN(_fneP[0] + width - 1);
      _fneP[2] = (float*)ALTIVEC_ALIGN(_fneP[1] + width - 1);
      _fnwP[0] = (float*)ALTIVEC_ALIGN(_fneP[2] + width - 1);
      _fnwP[1] = (float*)ALTIVEC_ALIGN(_fnwP[0] + width - 1);
      _fnwP[2] = (float*)ALTIVEC_ALIGN(_fnwP[1] + width - 1);
      _nnwP[0] = (float*)ALTIVEC_ALIGN(_fnwP[2] + width - 1);
      _nnwP[1] = (float*)ALTIVEC_ALIGN(_nnwP[0] + width - 1);
      _nnwP[2] = (float*)ALTIVEC_ALIGN(_nnwP[1] + width - 1);
      _sswP[0] = (float*)ALTIVEC_ALIGN(_nnwP[2] + width - 1);
      _sswP[1] = (float*)ALTIVEC_ALIGN(_sswP[0] + width - 1);
      _sswP[2] = (float*)ALTIVEC_ALIGN(_sswP[1] + width - 1);
    }
    else {
      _fwP = _fnP[0] = _fnP[1] = _fnP[2] = 0;
      _wnwP[0] = _wnwP[1] = 0;
      _wswP[0] = _wswP[1] = 0;
      _fnwP[0] = _fnwP[1] = _fnwP[2] = 0;
      _fneP[0] = _fneP[1] = _fneP[2] = 0;
      _nnwP[0] = _nnwP[1] = _nnwP[2] = 0;
      _sswP[0] = _sswP[1] = _sswP[2] = 0;
   }
  }

  rmAssert(repInvariant());
}

void rcAutoCorrelation::clearCache()
{
  if (_baseP) {
    free(_baseP);
    _baseP = 0;
    _size = _wP = _nwP[0] = _nwP[1] = _neP[0] = _neP[1] = 0;
    _sumP[0] = _sumP[1] = _sumP[2] = 0;
    _vxsP[0] = _vxsP[1] = _vxsP[2] = 0;
    _nP[0] = _nP[1] = _fwP = 0;
    _wnwP[0] = _wnwP[1] = 0;
    _wswP[0] = _wswP[1] = 0;
    _fnP[0] = _fnP[1] = _fnP[2] = 0;
    _fnwP[0] = _fnwP[1] = _fnwP[2] = 0;
    _fneP[0] = _fneP[1] = _fneP[2] = 0;
    _nnwP[0] = _nnwP[1] = _nnwP[2] = 0;
    _sswP[0] = _sswP[1] = _sswP[2] = 0;
  }
}

bool rcAutoCorrelation::repInvariant()
{
  bool retVal = true;
  const int32 width = _allocWidth;

  retVal &= (_size && IS_ALTIVEC_ALIGNED(_size));
  retVal &= (_sumP[0] && IS_ALTIVEC_ALIGNED(_sumP[0]));
  retVal &= (_sumP[1] >= (_sumP[0] + width) && IS_ALTIVEC_ALIGNED(_sumP[1]));
  retVal &= (_sumP[2] >= (_sumP[1] + width) && IS_ALTIVEC_ALIGNED(_sumP[2]));
  retVal &= (_vxsP[0] >= (_sumP[2] + width) && IS_ALTIVEC_ALIGNED(_vxsP[0]));
  retVal &= (_vxsP[1] >= (_vxsP[0] + width) && IS_ALTIVEC_ALIGNED(_vxsP[1]));
  retVal &= (_vxsP[2] >= (_vxsP[1] + width) && IS_ALTIVEC_ALIGNED(_vxsP[2]));
  retVal &= (_wP >= (_vxsP[2] + width) && IS_ALTIVEC_ALIGNED(_wP));
  retVal &= (_nP[0] >= (_wP + width - 1) && IS_ALTIVEC_ALIGNED(_nP[0]));
  retVal &= (_nP[1] >= (_nP[0] + width - 1) && IS_ALTIVEC_ALIGNED(_nP[1]));
  
  if (_crossOnly) {
    retVal &= (_nwP[0] == 0);
    retVal &= (_nwP[1] == 0);
    retVal &= (_neP[0] == 0);
    retVal &= (_neP[1] == 0);
    retVal &= (_wnwP[0] == 0);
    retVal &= (_wnwP[1] == 0);
    retVal &= (_wswP[0] == 0);
    retVal &= (_wswP[1] == 0);
    retVal &= (_fnwP[0] == 0);
    retVal &= (_fnwP[1] == 0);
    retVal &= (_fnwP[2] == 0);
    retVal &= (_fneP[0] == 0);
    retVal &= (_fneP[1] == 0);
    retVal &= (_fneP[2] == 0);
    retVal &= (_nnwP[0] == 0);
    retVal &= (_nnwP[1] == 0);
    retVal &= (_nnwP[2] == 0);
    retVal &= (_sswP[0] == 0);
    retVal &= (_sswP[1] == 0);
    retVal &= (_sswP[2] == 0);

    if (_radius == 2) {
      retVal &= (_fwP >= (_nP[1] + width - 1) && IS_ALTIVEC_ALIGNED(_fwP));
      retVal &= (_fnP[0] >= (_fwP + width - 1) && IS_ALTIVEC_ALIGNED(_fnP[0]));
      retVal &= (_fnP[1] >= (_fnP[0] + width - 1) && IS_ALTIVEC_ALIGNED(_fnP[1]));
      retVal &= (_fnP[2] >= (_fnP[1] + width - 1) && IS_ALTIVEC_ALIGNED(_fnP[2]));
    }
    else {
      retVal &= (_fwP == 0);
      retVal &= (_fnP[0] == 0);
      retVal &= (_fnP[1] == 0);
      retVal &= (_fnP[2] == 0);
    }
  }
  else {
    retVal &= (_nwP[0] >= (_nP[1] + width - 1) && IS_ALTIVEC_ALIGNED(_nwP[0]));
    retVal &= (_nwP[1] >= (_nwP[0] + width - 1) && IS_ALTIVEC_ALIGNED(_nwP[0]));
    retVal &= (_neP[0] >= (_nwP[1] + width - 1) && IS_ALTIVEC_ALIGNED(_neP[0]));
    retVal &= (_neP[1] >= (_neP[0] + width - 1) && IS_ALTIVEC_ALIGNED(_neP[1]));

    if (_radius == 2) {
      retVal &= (_fwP >= (_neP[1] + width - 1) && IS_ALTIVEC_ALIGNED(_fwP));
      retVal &= (_fnP[0] >= (_fwP + width - 1) && IS_ALTIVEC_ALIGNED(_fnP[0]));
      retVal &= (_fnP[1] >= (_fnP[0] + width - 1) && IS_ALTIVEC_ALIGNED(_fnP[1]));
      retVal &= (_fnP[2] >= (_fnP[1] + width - 1) && IS_ALTIVEC_ALIGNED(_fnP[2]));
      retVal &= (_wnwP[0] >= (_fnP[2] + width-1) && IS_ALTIVEC_ALIGNED(_wnwP[0]));
      retVal &= (_wnwP[1] >= (_wnwP[0] + width-1) && IS_ALTIVEC_ALIGNED(_wnwP[1]));
      retVal &= (_wswP[0] >= (_wnwP[1] + width-1) && IS_ALTIVEC_ALIGNED(_wswP[0]));
      retVal &= (_wswP[1] >= (_wswP[0] + width-1) && IS_ALTIVEC_ALIGNED(_wswP[1]));
      retVal &= (_fneP[0] >= (_wswP[1] + width-1) && IS_ALTIVEC_ALIGNED(_fneP[0]));
      retVal &= (_fneP[1] >= (_fneP[0] + width-1) && IS_ALTIVEC_ALIGNED(_fneP[1]));
      retVal &= (_fneP[2] >= (_fneP[1] + width-1) && IS_ALTIVEC_ALIGNED(_fneP[2]));
      retVal &= (_fnwP[0] >= (_fneP[2] + width-1) && IS_ALTIVEC_ALIGNED(_fnwP[0]));
      retVal &= (_fnwP[1] >= (_fnwP[0] + width-1) && IS_ALTIVEC_ALIGNED(_fnwP[1]));
      retVal &= (_fnwP[2] >= (_fnwP[1] + width-1) && IS_ALTIVEC_ALIGNED(_fnwP[2]));
      retVal &= (_nnwP[0] >= (_fnwP[2] + width-1) && IS_ALTIVEC_ALIGNED(_nnwP[0]));
      retVal &= (_nnwP[1] >= (_nnwP[0] + width-1) && IS_ALTIVEC_ALIGNED(_nnwP[1]));
      retVal &= (_nnwP[2] >= (_nnwP[1] + width-1) && IS_ALTIVEC_ALIGNED(_nnwP[2]));
      retVal &= (_sswP[0] >= (_nnwP[2] + width-1) && IS_ALTIVEC_ALIGNED(_sswP[0]));
      retVal &= (_sswP[1] >= (_sswP[0] + width-1) && IS_ALTIVEC_ALIGNED(_sswP[1]));
      retVal &= (_sswP[2] >= (_sswP[1] + width-1) && IS_ALTIVEC_ALIGNED(_sswP[2]));
    }
    else {
      retVal &= (_fwP == 0);
      retVal &= (_fnP[0] == 0);
      retVal &= (_fnP[1] == 0);
      retVal &= (_fnP[2] == 0);
      retVal &= (_wnwP[0] == 0);
      retVal &= (_wnwP[1] == 0);
      retVal &= (_wswP[0] == 0);
      retVal &= (_wswP[1] == 0);
      retVal &= (_fnwP[0] == 0);
      retVal &= (_fnwP[1] == 0);
      retVal &= (_fnwP[2] == 0);
      retVal &= (_fneP[0] == 0);
      retVal &= (_fneP[1] == 0);
      retVal &= (_fneP[2] == 0);
      retVal &= (_nnwP[0] == 0);
      retVal &= (_nnwP[1] == 0);
      retVal &= (_nnwP[2] == 0);
      retVal &= (_sswP[0] == 0);
      retVal &= (_sswP[1] == 0);
      retVal &= (_sswP[2] == 0);
    }
  }  

  return retVal;
}

