/****************************************************************************
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 *   File Name      rc_musclesegment.cpp
 *   Creation Date  01/10/2004
 *   Author         Peter Roberts
 *
 * Muscle Segmentation code
 *
 ***************************************************************************/

#include <rc_musclesegment.h>
#include <rc_analysis.h>
#include <rc_kineticimage.h>
#include <rc_draw.h>
#include <rc_gen_movie_file.h>
#include <rc_similarity.h>
#include <map>
#include <rc_stats.h>
#include <rc_ipconvert.h>
#include <rc_ip.h>

#define MSDBG1 0
#define MSDBG2 0

#define rmPrintBinaryImageV(a,v){				    \
  for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {    \
    for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)	    \
      fprintf(stderr, "%1d ", ((a).getPixel(j__temp,i__temp)==(v)));\
    fprintf (stderr, "\n");					    \
  }}


extern void rfPixel8Max3By3(const rcWindow& srcImg, rcWindow& dstImg);
extern void rfPixel8Min3By3(const rcWindow& srcImg, rcWindow& dstImg);

rcMuscleSegmenter::rcMuscleSegmenter(rcGenMovieFile* debugMovie)
  : _debugMovie(debugMovie), _dSize(0, 0), _emi(-1), _framesPerPeriod(-1.0)
{
}

int32 rcMuscleSegmenter::getEMI(const vector<rcWindow>& movie,
				  int32 startFrame, int32 endFrame)
{
  /* First, generate the energy curve for the sample period. This will
   * be used by calcExtendedMuscleCellFrame() to calculate the
   * location of a frame with the muscle cell fully expanded.
   */
  deque<double> signal;
		  
  const uint32 windowSz = 2, cacheSz = 0;
  
  rcSimilarator energy(rcSimilarator::eExhaustive,
		       rcPixel8, windowSz, cacheSz);
		  
  vector<rcWindow> eMovie;
  
  int32 fi;
  int32 lastWinFrame = startFrame + (int32)windowSz;
  for (fi = startFrame; fi < lastWinFrame; fi++)
    eMovie.push_back(movie[fi]);
		    
  bool rval = energy.fill(eMovie);
  rmAssert(rval);
		    
  const int32 targetValue = windowSz/2;
  deque<double> tempSig;
  energy.entropies(tempSig, rcSimilarator::eVisualEntropy);
  signal.push_back(tempSig[targetValue]);
		    
  for ( ; fi < endFrame; fi++) {
    rval = energy.update(movie[fi]);
    rmAssert(rval);
		      
    energy.entropies(tempSig, rcSimilarator::eVisualEntropy);
    signal.push_back(tempSig[targetValue]);
  }

  return calcExtendedMuscleCellFrame(signal, movie, startFrame);
}

void
rcMuscleSegmenter::genPolygons(const rcWindow& segImg, const int32 minDim,
			       vector<rcPolygon>& p, const rcIPair delta) const
{
  p.clear();

  uint32 blobCnt;
  rcWindow blobWin;

  rfGetComponents(segImg, blobCnt, blobWin);

  if (blobCnt == 0)
    return;

  const bool combineRects = false;
  vector<rcIRect> rects;
  vector<uint32> rectLabel;
  const rcIPair minPairSz(0, 0);

  rfGetRects(blobWin, blobCnt, minPairSz, combineRects, rects, rectLabel);

  uint32 polyCnt = 0;
  for (uint32 i = 0; i < blobCnt; i++) 
	{
#if 0		
    const rcIRect r = rects[i];
    if ((r.width() < minDim) && (r.height() < minDim))
      continue;
    
    const rcWindow rw(blobWin, r.ul().x(), r.ul().y(), r.width(), r.height());
    rcRleWindow rle;
    rle.encode(rw, blobCnt, rectLabel[i]);

    rcPolygon poly;
    rle.polygon(poly);
    p.push_back(poly);
    
    p[polyCnt++].translate(rc2Dvector(r.ul().x() + delta.x(),
				      r.ul().y() + delta.y()));
#endif		
    if (MSDBG2) cout << polyCnt-1 << " has polygon: " << p[polyCnt-1] << endl;
  }

  if (MSDBG1) printf("Blob count %d Polygon count %d\n", blobCnt, polyCnt);
}

void rcMuscleSegmenter::mergePolygons(vector<rcPolygon>& poly,
				      vector<rcPolygon>& polyCH,
				      vector<rcPolygon>& mergedCH,
				      vector<rcIRect>& mergedCHR,
				      vector<polyInterInfo>& mergedInfo) const
{
  rmAssert(poly.size() == polyCH.size());
  
  mergedCH.clear();
  mergedCHR.clear();
  mergedInfo.clear();

  //@note less<> is the default sorting
  // merging start with largest. Hence the reverse iterators

  multimap<double, uint32> areaToIndex;
  for (uint32 si = 0; si < poly.size(); si++) {
    double area = poly[si].area();
    areaToIndex.insert(pair<double, uint32>(area,si));
  }

  vector<uint32> mergedToPoly(poly.size());

  for (multimap<double, uint32>::reverse_iterator si = areaToIndex.rbegin();
       si != areaToIndex.rend(); si++) {
    rcPolygon& pCH = polyCH[si->second];
    rcIRect boxCH = pCH.orthogonalEnclosingRect();
    double sAreaCH = -1.0;
    uint32 mi;
    for (mi = 0; mi < mergedCH.size(); mi++) {
      if (mergedCHR[mi].overlaps(boxCH, true)) {
	if (sAreaCH < 0.0)
	  sAreaCH = pCH.area()*0.5;
	uint32 mpi = mergedToPoly[mi];
	rcPolygon intPoly;
	polyCH[mpi].convexIntersection(pCH, intPoly);
	if (intPoly.area() > sAreaCH) {
	  mergedInfo[mi].area += si->first;
	  mergedInfo[mi].polyInter.push_back(poly[si->second]);
	  
	  if (intPoly != pCH) {
	    rcPolygon unionPoly;
	    mergedCH[mi].convexUnion(pCH, unionPoly);
	    mergedCH[mi] = unionPoly;
	    mergedCHR[mi] = mergedCH[mi].orthogonalEnclosingRect();
	  }
	  break;
	}
      }
    }

    if (mi == mergedCH.size()) {
      mergedCH.push_back(pCH);
      mergedCHR.push_back(boxCH);
      mergedInfo.push_back(polyInterInfo());
      mergedInfo.back().area = si->first;
      mergedInfo.back().polyInter.push_back(poly[si->second]);
      mergedToPoly[mergedCH.size()-1] = si->second;
    }
  }

  if (MSDBG1) printf("merged count %d\n", (int)mergedCH.size());
}

void rcMuscleSegmenter::accumPolygons(vector<rcPolygon>& fg,
				      vector<rcIRect>& fgR,
				      vector<rcPolygon>& freq,
				      vector<rcIRect>& freqR,
				      vector<polyInterInfo>& accumInfo) const
{
  const uint32 fgCnt = fg.size();

  rmAssert(fgCnt == fgR.size());
  rmAssert(freq.size() == freqR.size());

  accumInfo.clear();
  accumInfo.resize(fgCnt);
  for (uint32 fi = 0; fi < fgCnt; fi++)
    accumInfo[fi].area = 0.0;

  for (uint32 freqi = 0; freqi < freq.size(); freqi++) {
    for (uint32 fi = 0; fi < fgCnt; fi++) {
      if (fgR[fi].overlaps(freqR[freqi], true) &&
	  fg[fi].intersects(freq[freqi])) {
	if (fg[fi].contains(freq[freqi])) {
	  accumInfo[fi].polyInter.push_back(freq[freqi]);
	  const double area = freq[freqi].area();
	  accumInfo[fi].area += area;
	  break;
	}
	else {
	  accumInfo[fi].polyInter.push_back(rcPolygon());
	  fg[fi].convexIntersection(freq[freqi],
				    accumInfo[fi].polyInter.back());
	  const double area = accumInfo[fi].polyInter.back().area();
	  accumInfo[fi].area += area;
	}
      }
    }
  }
}

void rcMuscleSegmenter::debugWrite(const rcWindow& frame) const
{
  if(!_debugMovie)
    return;

  const rcIPair fSize = frame.size();

  if (_dSize == fSize) {
    _debugMovie->addFrame(frame);
    return;
  }

  if ((fSize.x() >= _dSize.x()) && (fSize.y() >= _dSize.y())) {
    const int32 xOff = (fSize.x() - _dSize.x())/2;
    const int32 yOff = (fSize.y() - _dSize.y())/2;
    rcWindow fWin(frame, xOff, yOff, _dSize.x(), _dSize.y());
    _debugMovie->addFrame(fWin);
    return;
  }

  rcWindow tImg(_dSize);
  tImg.setAllPixels(0);

  if ((fSize.x() <= _dSize.x()) && (fSize.y() <= _dSize.y())) {
    const int32 xOff  = (_dSize.x() - fSize.x())/2;
    const int32 yOff = (_dSize.y() - fSize.y())/2;

    rcWindow tWin(tImg, xOff, yOff, fSize.x(), fSize.y());
    tWin.copyPixelsFromWindow(frame);
    _debugMovie->addFrame(tImg);
    return;
  }

  int32 xOffF, yOffF, xOffT, yOffT, w, h;
  rcWindow fWin;

  if (fSize.x() > _dSize.x()) {
    rmAssert(_dSize.y() > fSize.y());

    xOffF = (fSize.x() - _dSize.x())/2;
    yOffF = 0;
    xOffT = 0;
    yOffT = (_dSize.y() - fSize.y())/2;
    w = _dSize.x();
    h = fSize.y();
  }
  else {
    rmAssert(_dSize.x() > fSize.x());
    rmAssert(fSize.y() > _dSize.y());

    xOffF = 0;
    yOffF = (fSize.y() - _dSize.y())/2;
    xOffT = (_dSize.x() - fSize.x())/2;
    yOffT = 0;
    w = fSize.x();
    h = _dSize.y();
  }

  fWin = rcWindow(frame, xOffF, yOffF, w, h);

  rcWindow tWin(tImg, xOffT, yOffT, w, h);
  tWin.copyPixelsFromWindow(frame);
  _debugMovie->addFrame(tImg);
}

void rcMuscleSegmenter::hackInsertSegmentedImages(rcWindow& segSdImg,
						  rcWindow& segFreqImg,
						  rcWindow& emiImg)
{
  _segSdImg = segSdImg;
  _segFreqImg = segFreqImg;
  _emiImg = emiImg;
  _emi = 0;
}

void
rcMuscleSegmenter::genSpatialSegmentation(const vector<rcWindow>& movie,
					  int32 startFrame, int32 endFrame,
					  int32& emi, rcWindow& segSdImg)
{
  rmAssert(startFrame >= 0);
  rmAssert(endFrame < (int32)movie.size());
  rmAssert((endFrame - startFrame) >= 2);

  /* Use the following algorithm to generate a segmented image:
   *
   * 1) Calculate the index of a frame that has fully extended
   *    muscle cells.
   *
   * 2) Generate the std dev image for the emi frame. Calculate a
   *    segmentation point and generate a segmented version of of the
   *    std dev image based on this. The foreground pixels in the
   *    segmented std dev image serve as the candidate muscle cell
   *    areas.
   */

  /* Step 1
   */
  emi = getEMI(movie, startFrame, endFrame);
  rmAssert(emi >= startFrame);
  rmAssert(emi < endFrame);
  
  /* Step 2
   */
  const int32 radius = 1;
  const int32 szOffset = radius+1;
  const int32 szAdjust = 2*szOffset;
  const int32 segWidth = movie[0].width() - szAdjust;
  const int32 segHeight = movie[0].height() - szAdjust;
  rcWindow sdImg(segWidth, segHeight, rcPixel32);

  genSpatialSDImg(movie[emi], sdImg, radius);

  const float minBinVal = 3.0, binSz = 0.2;
  const int32 binCnt = 40;
  const bool getPeak = true;
  const float segSdVal = segPoint(sdImg, minBinVal, binSz, binCnt, getPeak);
  
  const uint8 bg = 0, fg = 0xFF;
  rcWindow segSdImgBase(movie[0].width(), movie[0].height());
  segSdImgBase.setAllPixels(bg);
  segSdImg = rcWindow(segSdImgBase, 1, 1, segWidth + 2, segHeight + 2);
  rcWindow segSdWin(segSdImgBase, szAdjust, szAdjust, segWidth, segHeight);
  binarizeFloatWindow(sdImg, segSdWin, segSdVal, fg, bg);
  //@todo  rfImageConvertFloat8 (sdImg, segSdWin, segSdVal - 1e-10, segSdVal + 1e-10);
}

int32
rcMuscleSegmenter::genFrequencySegmentation(const vector<rcWindow>& movie,
					    int32 startFrame,
					    int32 endFrame,
					    int32 emi,
					    float maxFramesPerPeriod,
					    float framesPerPeriod,
					    rcWindow& segFreqImg)
{
  rmAssert(startFrame >= 0);
  rmAssert(endFrame < (int32)movie.size());
  rmAssert((endFrame - startFrame) >= 2);

  /* Use the following algorithm to segment images:
   *
   * 1) Calculate the number of frames per beat. This requires the
   *    passed in emi value to use as a starting point.
   *
   * 2) Generate a frequency image based on the frames per beat value
   *    calculated in step 1). The foreground pixels in the segmented
   *    frequency image serve as the candidate active muscle cell
   *    areas.
   */

  /* Step 1
   */
  if (framesPerPeriod <= 0.0)
    framesPerPeriod = calcMusclePeriod(movie, startFrame, endFrame,
				       emi, maxFramesPerPeriod);

  if (MSDBG1) printf("FRAMES PER PERIOD %f\n", framesPerPeriod);
  _framesPerPeriod = framesPerPeriod;

  if (framesPerPeriod == 0.0)
    return cInvalidPeriod;

  /* Step 2
   */
  const uint8 bg = 0, fg = 0xFF;

  rcWindow maskImg(movie[0].width(), movie[0].height());
  maskImg.setAllPixels(bg);

  for (uint32 ci = 0; ci < _cells.size(); ci++)
    rcFillPolygon(_cells[ci], maskImg, fg);
  debugWrite(maskImg);

  rcOptoKineticImage kinetic(rcOptoKineticImage::eKineTypeDummy);

  for (int32 fi = startFrame; fi < endFrame; fi++)
    kinetic.push(movie[fi]);

  const int32 radius = 1;
  const int32 szOffset = radius+1;
  const int32 szAdjust = 2*szOffset;
  const int32 segWidth = movie[0].width() - szAdjust;
  const int32 segHeight = movie[0].height() - szAdjust;

  rcWindow freqImg(movie[0].width(), movie[0].height(), rcPixel32);
  kinetic.genFreqImg(freqImg, framesPerPeriod, &maskImg);
  rcWindow freqImgWin(freqImg, szOffset, szOffset, segWidth, segHeight);

  segFreqImg = rcWindow(segWidth + 2, segHeight + 2);
  segFreqImg.setAllPixels(bg);
  rcWindow segFreqWin(segFreqImg, 1, 1, segWidth, segHeight);

  const float segFreqVal = 0.10;
  binarizeFloatWindow(freqImgWin, segFreqWin, segFreqVal, fg, bg);
  return 0;
}

int32 rcMuscleSegmenter::segment(const vector<rcWindow>& movie,
				   int32 startFrame, int32 endFrame,
				   bool fluo)
{
  rmAssert(endFrame > startFrame);
  rmAssert((int32)movie.size() >= endFrame);

  /* minWinDim of 10 may be overly large, but just wanted a value big
   * enough to keep window sizes from getting too small.
   */
  const int32 minWinDim = 10;
  if ((movie[0].width() < minWinDim) || (movie[0].height() < minWinDim))
    return cWindowTooSmall;

  _cells.clear();
  _cellsR.clear();
  _cellsInfo.clear();
  _activeRegions.clear();
  _rejects.clear();
  _rejectIndex.clear();
  _rejectReason.clear();

  if (_debugMovie) {
    _dSize = _debugMovie->frameSize();
    if (_dSize == rcIPair(0, 0))
      _dSize = movie[0].size();
  }

  rcWindow segSdImg, emiImg;

  //@note placeholder for passing in binary / pre-mapped images in to segmentation
  if (0) {
  const int32 radius = 1;
  const int32 szOffset = radius+1;
  const int32 szAdjust = 2*szOffset;
  const int32 segWidth = movie[0].width() - szAdjust;
  const int32 segHeight = movie[0].height() - szAdjust;

    rcWindow segSdImgBase(movie[0].width(), movie[0].height());
    segSdImgBase.setAllPixels(0);
    segSdImg = rcWindow(segSdImgBase, 1, 1, segWidth + 2, segHeight + 2);
    _emi = startFrame;
    emiImg = movie[_emi];
    static const uint8 zu (0);
    vector<uint8> bmap (256, zu);
    bmap[255] = 1;
    rfPixel8Map (emiImg, segSdImgBase, bmap);
  }
  else {
    genSpatialSegmentation(movie, startFrame, endFrame, _emi, segSdImg);
    emiImg = movie[_emi];
  }

  _segSdImgSz = segSdImg.size();

  if (_debugMovie) {
    _emiImgDbg = emiImg;
    debugWrite(_emiImgDbg);

    _segSdImgDbg = segSdImg;
    debugWrite(_segSdImgDbg);
  }
  else {
    _emiImgDbg = rcWindow();
    _segSdImgDbg = rcWindow();
  }

  const int32 minDim = 4;

  vector<rcPolygon> fgPoly;
  genPolygons(segSdImg, minDim, fgPoly, rcIPair(1, 1));

  if (_debugMovie) {
    rcWindow scribble(_emiImgDbg.width(), _emiImgDbg.height());
    const uint8 bgColor = 0;
    scribble.setAllPixels(bgColor);
    rcWindow scribWin(scribble, 1, 1, _segSdImgDbg.width(), _segSdImgDbg.height());
    const bool fill = true;
    const uint8 segColor = 0x7F;

    scribWin.copyPixelsFromWindow(_segSdImgDbg);
    for (uint32 fi = 0; fi < fgPoly.size(); fi++)
      rcDrawPolygon(fgPoly[fi], scribble, segColor, fill);
    debugWrite(scribble);

    const uint8 imgColor = 0xFF;
    scribble.copyPixelsFromWindow(_emiImgDbg);
    for (uint32 i = 0; i < fgPoly.size(); i++)
      rcDrawPolygon(fgPoly[i], scribble, imgColor, fill);
    debugWrite(scribble);
  }

  vector<rcPolygon> fgPolyCH(fgPoly.size());
  for (uint32 fi = 0; fi < fgPoly.size(); fi++)
    fgPoly[fi].convexHull(fgPolyCH[fi]);

  if (_debugMovie) {
    rcWindow scribble(_emiImgDbg.width(), _emiImgDbg.height());
    const bool fill = true;

    const uint8 imgColor = 0xFF;
    scribble.copyPixelsFromWindow(_emiImgDbg);
    for (uint32 fi = 0; fi < fgPolyCH.size(); fi++)
      rcDrawPolygon(fgPolyCH[fi], scribble, imgColor, fill);
    debugWrite(scribble);
  }

  mergePolygons(fgPoly, fgPolyCH, _cells, _cellsR, _cellsInfo);

  if (MSDBG1) printf("%d merged cell polygons\n", (int)_cells.size());

  if (_debugMovie) {
    rcWindow scribble(_emiImgDbg.width(), _emiImgDbg.height());

    const uint8 color = 0xFF;
    const bool fill = true;
    scribble.copyPixelsFromWindow(_emiImgDbg);
    for (uint32 ci = 0; ci < _cells.size(); ci++)
      rcDrawPolygon(_cells[ci], scribble, color, fill);
    debugWrite(scribble);
  }

  return 0;
}

void rcMuscleSegmenter::filterBySize(double minArea)
{

  // Remove too small
  for (uint32 ci = 0; ci < _cells.size();) {
    if (_cells[ci].area() < minArea) {
      _rejects.push_back(_cells[ci]);
      _cells[ci] = _cells.back();
      _cells.pop_back();
      _cellsR[ci] = _cellsR.back();
      _cellsR.pop_back();
      _cellsInfo[ci] = _cellsInfo.back();
      _cellsInfo.pop_back();
    }
    else
      ci++;
  }


  // Now find the median of the remaining
  if (_cells.size() > 2)
    {
      vector<double> sizes (_cells.size());
      for (uint32 ci = 0; ci < _cells.size(); ci++)
	{
	  sizes[ci] = _cells[ci].area();
	}

      double cmed = rfMedian (sizes);
      cmed *= 1.5;

      // Get rid of anything 1.5 times the size of the median cell
      // ToDo: this is should be a parameter

      for (uint32 ci = 0; ci < _cells.size();) {
	double csize = _cells[ci].area();
	if (csize > cmed) {
	  _rejects.push_back(_cells[ci]);
	  _cells[ci] = _cells.back();
	  _cells.pop_back();
	  _cellsR[ci] = _cellsR.back();
	  _cellsR.pop_back();
	  _cellsInfo[ci] = _cellsInfo.back();
	  _cellsInfo.pop_back();
	}
	else
	  ci++;
      }
    }

  if (_debugMovie) {
    rcWindow scribble(_emiImgDbg.width(), _emiImgDbg.height());
    const uint8 color = 0xFF;
    const bool fill = true;
    scribble.copyPixelsFromWindow(_emiImgDbg);
    for (uint32 ci = 0; ci < _cells.size(); ci++)
      rcDrawPolygon(_cells[ci], scribble, color, fill);
    debugWrite(scribble);
  }

  _rejectIndex.push_back((int32)_rejects.size());
  _rejectReason.push_back(cRejCodeSize);

  if (MSDBG1) printf("size filtered count %d\n", (int)_cells.size());
}

void rcMuscleSegmenter::filterByAR(double minAspectRatio)
{
  for (uint32 ci = 0; ci < _cells.size();) {
    rcAffineRectangle aRect = _cells[ci].minimumAreaEnclosingRect();
    rcIPair sz = aRect.cannonicalSize();
    int32 lSide = (sz.y() > sz.x()) ? sz.y() : sz.x();
    int32 sSide = (sz.y() > sz.x()) ? sz.x() : sz.y();
    double ar = (double)lSide/(double)sSide;
    if (MSDBG2) printf("%d: Long x Short %d x %d AR: %f\n", ci, lSide, sSide, ar);

    if (ar < minAspectRatio) {
      _rejects.push_back(_cells[ci]);
      _cells[ci] = _cells.back();
      _cells.pop_back();
      _cellsR[ci] = _cellsR.back();
      _cellsR.pop_back();
      _cellsInfo[ci] = _cellsInfo.back();
      _cellsInfo.pop_back();
    }
    else
      ci++;
  }

  if (_debugMovie) {
    rcWindow scribble(_emiImgDbg.width(), _emiImgDbg.height());
    const uint8 color = 0xFF;
    const bool fill = true;
    scribble.copyPixelsFromWindow(_emiImgDbg);
    for (uint32 ci = 0; ci < _cells.size(); ci++)
      rcDrawPolygon(_cells[ci], scribble, color, fill);
    debugWrite(scribble);
  }

  _rejectIndex.push_back((int32)_rejects.size());
  _rejectReason.push_back(cRejCodeAR);

  if (MSDBG1) printf("ar filtered count %d\n", (int)_cells.size());
}

int32
rcMuscleSegmenter::filterByActivity(double minPct,
				    const vector<rcWindow>& movie,
				    int32 startFrame, int32 endFrame,
				    float maxFramesPerPeriod,
				    float framesPerPeriod)
{
  rmAssert(endFrame > startFrame);
  rmAssert((int32)movie.size() >= endFrame);
  rmAssert((endFrame - startFrame) >= 2);
  rmAssert(_emi >= startFrame);
  rmAssert(_emi < endFrame);

  /* minWinDim of 10 may be overly large, but just wanted a value big
   * enough to keep window sizes from getting too small.
   */
  const int32 minWinDim = 10;
  if ((movie[0].width() < minWinDim) || (movie[0].height() < minWinDim))
    return cWindowTooSmall;

  rcWindow segFreqImg;

  bool hack = _segFreqImg.isBound();

  _framesPerPeriod = framesPerPeriod;

  if (hack)
    segFreqImg = _segFreqImg;
  else {
    int32 retVal = genFrequencySegmentation(movie, startFrame, endFrame,
					      _emi, maxFramesPerPeriod,
					      _framesPerPeriod, segFreqImg);
    if (retVal < 0)
      return retVal;
  }

  const int32 minDim = 4;

  genPolygons(segFreqImg, minDim, _activeRegions, rcIPair(1, 1));

  if (_debugMovie) {
    debugWrite(segFreqImg);

    rcWindow scribble(_emiImgDbg.width(), _emiImgDbg.height());
    const uint8 bgColor = 0;
    scribble.setAllPixels(bgColor);
    rcWindow scribWin(scribble, 1, 1, segFreqImg.width(), segFreqImg.height());
    const bool fill = true;
    const uint8 segColor = 0x7F;

    scribWin.copyPixelsFromWindow(segFreqImg);
    for (uint32 ai = 0; ai < _activeRegions.size(); ai++)
      rcDrawPolygon(_activeRegions[ai], scribble, segColor, fill);
    debugWrite(scribble);

    const uint8 imgColor = 0xFF;
    scribble.copyPixelsFromWindow(_emiImgDbg);
    for (uint32 ai = 0; ai < _activeRegions.size(); ai++)
      rcDrawPolygon(_activeRegions[ai], scribble, imgColor, fill);
    debugWrite(scribble);
  }

  vector<rcIRect> activeRegionsR(_activeRegions.size());
  for (uint32 ai = 0; ai < _activeRegions.size(); ai++)
    activeRegionsR[ai] = _activeRegions[ai].orthogonalEnclosingRect();

  vector<uint32> ciToAiMap(_cells.size());
  for (uint32 ci = 0; ci < _cells.size(); ci++)
    ciToAiMap[ci] = ci;

  vector<polyInterInfo> activeInfo;
  accumPolygons(_cells, _cellsR, _activeRegions, activeRegionsR, activeInfo);
  rmAssert(activeInfo.size() == _cells.size());

  for (uint32 ci = 0; ci < _cells.size(); ) {
    uint32 ai = ciToAiMap[ci];
    if (MSDBG2) printf("%d: ACTIVE %f FG %f FAR %f\n", ai,
		       activeInfo[ai].area, _cellsInfo[ci].area,
		       activeInfo[ai].area/_cellsInfo[ci].area);

    if (activeInfo[ai].area/_cellsInfo[ci].area < minPct) {
      ai = _cells.size() - 1;
      ciToAiMap[ci] = ai;
      _rejects.push_back(_cells[ci]);
      _cells[ci] = _cells[ai];
      _cells.pop_back();
      _cellsR[ci] = _cellsR[ai];
      _cellsR.pop_back();
      _cellsInfo[ci] = _cellsInfo[ai];
      _cellsInfo.pop_back();
    }
    else
      ci++;
  }

  if (_debugMovie) {
    rcWindow scribble(_emiImgDbg.width(), _emiImgDbg.height());
    const uint8 color = 0xFF;
    const bool fill = true;
    scribble.copyPixelsFromWindow(_emiImgDbg);
    for (uint32 ci = 0; ci < _cells.size(); ci++)
      rcDrawPolygon(_cells[ci], scribble, color, fill);
    debugWrite(scribble);
  }

  _rejectIndex.push_back((int32)_rejects.size());
  _rejectReason.push_back(cRejCodeActivity);

  if (MSDBG1) printf("activity filtered count %d\n", (int)_cells.size());

  return 0;
}

void rcMuscleSegmenter::filterAtEdge(int32 minDelta)
{
  const rcIPair size = _segSdImgSz - rcIPair(1, 1);

  if (MSDBG1) printf("edge filter : img sz (%d, %d) min delta %d\n",
		     size.x(), size.y(), minDelta);

  for (uint32 ci = 0; ci < _cells.size();) {
    const rcIRect bBox = _cells[ci].orthogonalEnclosingRect();
    const rcIPair ul = bBox.ul();
    const rcIPair lr = size - bBox.lr();

    if (MSDBG2) printf("ef @ %d: ul (%d, %d) lr (%d, %d) sz - lr (%d, %d)\n",
		       ci, ul.x(), ul.y(), _cellsR[ci].lr().x(),
		       _cellsR[ci].lr().y(), lr.x(), lr.y());

    if ((ul.x() < minDelta) || (ul.y() < minDelta) ||
	(lr.x() < minDelta) || (lr.y() < minDelta)) {
      _rejects.push_back(_cells[ci]);
      _cells[ci] = _cells.back();
      _cells.pop_back();
      _cellsR[ci] = _cellsR.back();
      _cellsR.pop_back();
      _cellsInfo[ci] = _cellsInfo.back();
      _cellsInfo.pop_back();
    }
    else
      ci++;
  }

  if (_debugMovie) {
    rcWindow scribble(_emiImgDbg.width(), _emiImgDbg.height());
    const uint8 color = 0xFF;
    const bool fill = true;
    scribble.copyPixelsFromWindow(_emiImgDbg);
    for (uint32 ci = 0; ci < _cells.size(); ci++)
      rcDrawPolygon(_cells[ci], scribble, color, fill);
    debugWrite(scribble);
  }

  _rejectIndex.push_back((int32)_rejects.size());
  _rejectReason.push_back(cRejCodeEdge);

  if (MSDBG1) printf("edge filtered count %d\n", (int)_cells.size());
}

void rcMuscleSegmenter::getCells(vector<rcPolygon>& cells) const
{
  cells.resize(_cells.size());

  for (uint32 ci = 0; ci < _cells.size(); ci++)
    cells[ci] = _cells[ci];

  if (_debugMovie) {
    rcWindow scribble(_emiImgDbg.width(), _emiImgDbg.height());
    const uint8 polyColor = 0x7F, boxColor = 0xFF;
    const bool fill = true;
    scribble.copyPixelsFromWindow(_emiImgDbg);
    for (uint32 ci = 0; ci < cells.size(); ci++) {
      const rcAffineRectangle aRect = cells[ci].minimumAreaEnclosingRect();
      rcDrawAffineRectangle(aRect, scribble, boxColor);
      rcDrawPolygon(cells[ci], scribble, polyColor, fill);
    }
    debugWrite(scribble);
  }
}

void rcMuscleSegmenter::getRejects(vector<rcPolygon>& rejects) const
{
  rejects.resize(_rejects.size());

  for (uint32 ri = 0; ri < _rejects.size(); ri++)
    rejects[ri] = _rejects[ri];
}

void
rcMuscleSegmenter::rejectRange(int32& startIndex, int32& endIndex,
			       rejectionCode reason, int32 instance) const
{
  rmAssert(reason < cRejCodeCount);
  const int32 rejSz = (int32)_rejectReason.size();
  rmAssert(rejSz == (int32)_rejectIndex.size());

  startIndex = endIndex = 0;

  if (instance < 1)
    return;

  int32 ri = -1;

  while (instance--) {
    ri++;
    for ( ;ri < rejSz; ri++) {
      if (_rejectReason[ri] == reason)
	break;
    }
  }

  if (ri != rejSz) {
    if (ri == 0)
      startIndex = 0;
    else
      startIndex = _rejectIndex[ri-1];
    endIndex = _rejectIndex[ri];
  }
}

