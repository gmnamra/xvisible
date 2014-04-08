/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_muscletracker.h
 *   Creation Date  08/06/2003
 *   Author         Peter Roberts
 *
 * Simple muscle tracking capability
 *
 ***************************************************************************/

#include <rc_muscletracker.h>
#include <rc_filter1d.h>
#include <rc_moments.h>
#include <rc_analysis.h>

rcMuscleTracker::rcMuscleTracker(uint32 smoothingRadius)
  : _filterOffset(4), _stdDev(4.0),
    _persistPct(.6), _smoothing(smoothingRadius*2 + 1)
{
  uint32 curValue = 1;
  uint32 curSum = 0;
  uint32 backI = smoothingRadius*2;
  uint32 frtI = 0;
  while (frtI < smoothingRadius) {
    _smoothing[frtI++] = _smoothing[backI--] = curValue;
    curValue <<= 1;
    curSum += curValue;
  }
  _smoothing[frtI] = curValue;
  curSum += curValue;

  _smoothingNorm = curSum >> 1;

  setEdgeFilter(4, 0);
}

void rcMuscleTracker::setEdgeFilter(uint32 oneCnt, uint32 zeroCnt)
{
  rcEdgeFilter1D::genFilter(_filter, oneCnt, zeroCnt);  

  _filterOffset = oneCnt + (zeroCnt+1)/2;
}

void rcMuscleTracker::setEdgeStrength(double stdDev)
{
  _stdDev = stdDev;
}

void rcMuscleTracker::setEdgePersistence(double percent)
{
  _persistPct = percent;
}

bool rcMuscleTracker::calculateCellLocation(vector<rcWindow>& images,
					    mtDebug showDiags,
                                            rcProgressIndicator* progressIndicator)
{
  if (images.size() == 0) {
    _lMdlWin = rcWindow();
    _rMdlWin = rcWindow();
    return false;
  }

  rcEdgeFilter1D edgeGen;
  edgeGen.filter(_filter);

  rcMomentGenerator mom(rcMomentGenerator::eMoment2D);
  const uint32 width = images[0].width();
  const uint32 height = images[0].height();
  rcWindow vProj(width, 1, rcPixel32);
  rcWindow hProj(height, 1, rcPixel32);
  
  /* The following vectors will hold the number of times a strong edge
   * is found at each vertical/horizontal position. The array names
   * are chosen to reflect the fact that each position is a potential
   * candidate location for a cell edge.
   */
  if (_vCandPosDist.size() != width)
    _vCandPosDist.resize(width);
  if (_vCandNegDist.size() != width)
    _vCandNegDist.resize(width);
  for (uint32 i = 0; i < width; i++)
    _vCandPosDist[i] = _vCandNegDist[i] = 0;

  if (_hCandPosDist.size() != height)
    _hCandPosDist.resize(height);
  if (_hCandNegDist.size() != height)
    _hCandNegDist.resize(height);
  for (uint32 i = 0; i < height; i++)
    _hCandPosDist[i] = _hCandNegDist[i] = 0;

  /* Place to store the indices of frames that can be used as a source
   * of model pixels.
   */
  vector<uint32> vCandPosFrmNum(width), vCandNegFrmNum(width);

  rcRect loc(0, 0, width, height);

  fprintf(stderr, "loc x %d y %d w %d h %d fi x %d y %d w %d h %d\n",
	  loc.x(), loc.y(), loc.width(), loc.height(),
	  images[0].x(), images[0].y(),
	  images[0].width(), images[0].height());

  /* Generate the frequency distribution of strong "positive" and
   * "negative" edges.
   */
  for (uint32 i = images.size() - 1; i != 0xFFFFFFFF; i--) {
      if ( progressIndicator )
          if ( progressIndicator->progress( 100.0 * (images.size() - i)/images.size() ) )
              return false;
    mom.update(images[i]);
    images[i].frameBuf().unlock();
    mom.vProject(loc, vProj);
    mom.hProject(loc, hProj);

    if (showDiags)
      (*showDiags)(vProj, hProj);

    vector<rcEdgeFilter1DPeak> vPeaks, hPeaks;

    /* Generate edge points that have a strength value >= _StdDev
     * std. dev. of norm.
     */
    edgeGen.genPeaks(vProj, 0, vPeaks, _filterOffset, 0, _stdDev);

    /* Now update running history of peaks found at each location,
     * both in the vertical and horizontal dimensions. Also store
     * indice of frame where, given a particular location is chosen,
     * model pixels may be found. Note that this is only needed in the
     * vertical direction, since it assumed that cells only move along
     * the horizontal axis.
     */
    for (uint32 pIndex = 0; pIndex < vPeaks.size(); pIndex++) {
      const int32 curLoc = vPeaks[pIndex]._location;
      if (vPeaks[pIndex]._value > 0) {
	_vCandPosDist[curLoc]++;
	vCandPosFrmNum[curLoc] = i;
      }
      else {
	_vCandNegDist[curLoc]++;
	vCandNegFrmNum[curLoc] = i;
      }
    }

    /* Lather, Rinse, and Repeat in the horizontal direction.
     */
    edgeGen.genPeaks(hProj, 0, hPeaks, _filterOffset, 0, _stdDev);

    for (uint32 pIndex = 0; pIndex < hPeaks.size(); pIndex++) {
      const int32 curLoc = hPeaks[pIndex]._location;
      if (hPeaks[pIndex]._value > 0)
	_hCandPosDist[curLoc]++;
      else
	_hCandNegDist[curLoc]++;
    }
  }

  /* The following heurisitic is used to calculate the edge positions
   * that will form the boundary of the cell:
   *
   * 1) Edge persistence is used to cull out transient edges. The
   *    previously calculated frequency distribution is used to
   *    determine this. An edge will be considered to be persisitent
   *    if it was found at least _persistPct percent of the time.
   *
   * 2) It is assumed that the scene is not confusing, so the
   *    outermost persistent edges will be chosen as the final cell
   *    limits.
   */
  uint32 pCutOff = (uint32)(images.size()*_persistPct);
  uint32 leftEdge = 0xFFFFFFFF, rightEdge = 0;
  uint32 topEdge = 0xFFFFFFFF, bottomEdge = 0;

  for (uint32 x = 0; x < width; x++)
    if (_vCandPosDist[x] > pCutOff) {
      leftEdge = x;
      _leftFrameIndex = vCandPosFrmNum[x];
      break;
    }
    else if (_vCandNegDist[x] > pCutOff) {
      leftEdge = x;
      _leftFrameIndex = vCandNegFrmNum[x];
      break;
    }

  for (uint32 x = width; x-- > 0; )
    if (_vCandPosDist[x] > pCutOff) {
      rightEdge = x;
      _rightFrameIndex = vCandPosFrmNum[x];
      break;
    }
    else if (_vCandNegDist[x] > pCutOff) {
      rightEdge = x;
      _rightFrameIndex = vCandNegFrmNum[x];
      break;
    }

  for (uint32 y = 0; y < height; y++)
    if ((_hCandPosDist[y] > pCutOff) || (_hCandNegDist[y] > pCutOff)) {
      topEdge = y;
      break;
    }

  for (uint32 y = height; y-- > 0; )
    if ((_hCandPosDist[y] > pCutOff) || (_hCandNegDist[y] > pCutOff)) {
      bottomEdge = y;
      break;
    }

  if ((leftEdge >= rightEdge) || (topEdge >= bottomEdge)) {
    printf("Couldn't find cell boundaries l %d r %d t %d b %d\n",
	   leftEdge, rightEdge, topEdge, bottomEdge);
    _lMdlWin = rcWindow();
    _rMdlWin = rcWindow();
    return false;
  }

  /* Use the previously calculated edge locations to generate the results:
   * models, search spaces, and initial offset within search space.
   */

  const uint32 cellWidth = rightEdge - leftEdge + 1;
  const uint32 cellHeight = bottomEdge - topEdge + 1;
  const uint32 outerSlop = 6;
  const uint32 innerSlop = cellWidth/20 > 8 ? cellWidth/20 : 8;
  
  const uint32 heightSlop = 2;

  _leftModel = rcRect(leftEdge - outerSlop,
		      topEdge - heightSlop,
		      innerSlop + outerSlop,
		      cellHeight + heightSlop*2);

  _leftSpace = rcRect(_leftModel.x() - 4,
		      _leftModel.y() - 2,
		      _leftModel.width() + 4 + (cellWidth*10)/100,
		      _leftModel.height() + 4);

  _leftOffset = rcIPair(outerSlop, 0);

  _rightModel = rcRect(rightEdge - innerSlop,
		       topEdge - heightSlop,
		       innerSlop + outerSlop,
		       cellHeight + heightSlop*2);

  _rightSpace = rcRect(_rightModel.x() - (cellWidth*10)/100,
		       _rightModel.y() - 2,
		       _rightModel.width() + 4 + (cellWidth*10)/100,
		       _rightModel.height() + 4);

  _rightOffset = rcIPair(innerSlop, 0);

  _lMdlWin = rcWindow(_leftModel.width(), _leftModel.height());
  rcWindow lp(images[_leftFrameIndex],
	      _leftModel.x(), _leftModel.y() - 2,
	      _leftModel.width(), _leftModel.height() + 4);
  for (int32 yo = 0; yo < _lMdlWin.height(); yo++) {
    for (int32 xo = 0; xo < _lMdlWin.width(); xo++) {
      uint32 pixel =
	(lp.getPixel(xo, yo) + lp.getPixel(xo, yo+4))*3 + 
	(lp.getPixel(xo, yo+1) + lp.getPixel(xo, yo+3))*11 + 
	lp.getPixel(xo, yo+2)*17;
      _lMdlWin.setPixel(xo, yo, (pixel+22)/45);
    }
  }

  _rMdlWin = rcWindow(_rightModel.width(), _rightModel.height());
  rcWindow rp(images[_rightFrameIndex],
	      _rightModel.x(), _rightModel.y() - 2,
	      _rightModel.width(), _rightModel.height() + 4);
  for (int32 yo = 0; yo < _rMdlWin.height(); yo++) {
    for (int32 xo = 0; xo < _rMdlWin.width(); xo++) {
      uint32 pixel =
	(rp.getPixel(xo, yo) + rp.getPixel(xo, yo+4))*3 + 
	(rp.getPixel(xo, yo+1) + rp.getPixel(xo, yo+3))*11 + 
	rp.getPixel(xo, yo+2)*17;
      _rMdlWin.setPixel(xo, yo, (pixel+22)/45);
    }
  }

  _lSrchWin = rcWindow(_leftSpace.width(), _leftSpace.height());
  _rSrchWin = rcWindow(_rightSpace.width(), _rightSpace.height());

  _rightToLeftOffset = _rightSpace.x() - _leftSpace.x();

  return true;
}

rcDPair rcMuscleTracker::genEdgePoints(rcWindow image, vector<double>* lCorrSpace,
				       vector<double>* rCorrSpace)
{
  rmAssert(_lMdlWin.isBound() && _rMdlWin.isBound());

  /* Create smoothed versions of image pixels and generate xorrelation
   * spaces.
   */
  rcWindow lp(image, _leftSpace.x(), _leftSpace.y() - 2,
	      _leftSpace.width(), _leftSpace.height() + 4);
  for (int32 yo = 0; yo < _lSrchWin.height(); yo++) {
    for (int32 xo = 0; xo < _lSrchWin.width(); xo++) {
      uint32 pixel =
	(lp.getPixel(xo, yo) + lp.getPixel(xo, yo+4))*3 + 
	(lp.getPixel(xo, yo+1) + lp.getPixel(xo, yo+3))*11 + 
	lp.getPixel(xo, yo+2)*17;
	_lSrchWin.setPixel(xo, yo, (pixel+22)/45);
    }
  }
  uint32 lPeakOffsetX, lPeakOffsetY;
  genSpace(_lSrchWin, _lMdlWin, _lResSpace, lPeakOffsetX, lPeakOffsetY);

  rcWindow rp(image, _rightSpace.x(), _rightSpace.y() - 2,
	      _rightSpace.width(), _rightSpace.height() + 4);
  for (int32 yo = 0; yo < _rSrchWin.height(); yo++) {
    for (int32 xo = 0; xo < _rSrchWin.width(); xo++) {
      uint32 pixel =
	(rp.getPixel(xo, yo) + rp.getPixel(xo, yo+4))*3 + 
	(rp.getPixel(xo, yo+1) + rp.getPixel(xo, yo+3))*11 + 
	rp.getPixel(xo, yo+2)*17;
      _rSrchWin.setPixel(xo, yo, (pixel+22)/45);
    }
  }
  uint32 rPeakOffsetX, rPeakOffsetY;
  genSpace(_rSrchWin, _rMdlWin, _rResSpace, rPeakOffsetX, rPeakOffsetY);

  /* Generate interpolated position information.
   */
  double lPeakInterpolatedOffsetX, rPeakInterpolatedOffsetX;
	
  if ((lPeakOffsetX == 0) ||
      (lPeakOffsetX == _lResSpace[lPeakOffsetY].size() - 1)) {
    fprintf(stderr, "WARNING: Left peak at edge!!!: ");
    lPeakInterpolatedOffsetX = lPeakOffsetX;
  }
  else {
    double offset = parabolicFit(_lResSpace[lPeakOffsetY][lPeakOffsetX-1],
				 _lResSpace[lPeakOffsetY][lPeakOffsetX],
				 _lResSpace[lPeakOffsetY][lPeakOffsetX+1],
				 (double*)0);
    lPeakInterpolatedOffsetX = lPeakOffsetX + offset;
  }

  if ((rPeakOffsetX == 0) ||
      (rPeakOffsetX == _rResSpace[rPeakOffsetY].size() - 1)) {
    printf("WARNING: Right peak at edge!!!: ");
    rPeakInterpolatedOffsetX = rPeakOffsetX;
  }
  else {
    double offset = parabolicFit(_rResSpace[rPeakOffsetY][rPeakOffsetX-1],
				 _rResSpace[rPeakOffsetY][rPeakOffsetX],
				 _rResSpace[rPeakOffsetY][rPeakOffsetX+1],
				 (double*)0);
    rPeakInterpolatedOffsetX = rPeakOffsetX + offset;
  }

  /* If client has passed a pointer to a 1D correlation space vector,
   * fill it with a compressed version of the the actual correlation
   * space: for each location in x, pick the highest score in the y
   * dimension.
   */
  if (lCorrSpace) {
    if (lCorrSpace->size() != _lResSpace[0].size())
      lCorrSpace->resize(_lResSpace[0].size());
    for (uint32 x = 0; x < _lResSpace[0].size(); x++) {
      double maxX = -1.0;
      for (uint32 y = 0; y < _lResSpace.size(); y++)
	if (_lResSpace[y][x] > maxX)
	  maxX = _lResSpace[y][x];
      (*lCorrSpace)[x] = maxX;
    }
  }

  if (rCorrSpace) {
    if (rCorrSpace->size() != _rResSpace[0].size())
      rCorrSpace->resize(_rResSpace[0].size());
    for (uint32 x = 0; x < _rResSpace[0].size(); x++) {
      double maxX = -1.0;
      for (uint32 y = 0; y < _rResSpace.size(); y++)
	if (_rResSpace[y][x] > maxX)
	  maxX = _rResSpace[y][x];
      (*rCorrSpace)[x] = maxX;
    }
  }

  return rcDPair(lPeakInterpolatedOffsetX,
		 rPeakInterpolatedOffsetX + _rightToLeftOffset);
}

void rcMuscleTracker::genSpace(rcWindow& srchWin, rcWindow& model,
			       vector<vector<double> >& space,
			       uint32& peakOffsetX, uint32& peakOffsetY)
{
  rmAssert(srchWin.width() >= model.width());
  rmAssert(srchWin.height() >= model.height());

  uint32 spaceWidth = srchWin.width() - model.width() + 1;
  uint32 spaceHeight = srchWin.height() - model.height() + 1;

  if (space.size() != spaceHeight) {
    space.resize(spaceHeight);
    for (uint32 y = 0; y < spaceHeight; y++)
      space[y].resize(spaceWidth);
  }
  else {
    for (uint32 y = 0; y < spaceHeight; y++)
      if (space[y].size() != spaceWidth)
	space[y].resize(spaceWidth);
  }

  rsCorrParams params;
  rcCorr res;
  double maxScore = -1.0;
  const rcIPair shiftRightOne(1, 0);

  for (uint32 y = 0; y < spaceHeight; y++) {
    rcWindow image(srchWin, 0, y, model.width(), model.height());

    for (uint32 x = 0; x < spaceWidth; ) {
      rfCorrelate(model, image, params, res);
      const double score = res.r();

      if (score > maxScore) {
	maxScore = score;
	peakOffsetX = x;
	peakOffsetY = y;
      }
      space[y][x++] = score;
      if (x != spaceWidth)
	image.translate(shiftRightOne);
    }
  }
}
