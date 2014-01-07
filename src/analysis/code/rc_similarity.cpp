/*
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

// analysis
#include <rc_similarity.h>
#include <rc_analysis.h>

#include <rc_filter1d.h>
#include <rc_histstats.h>
#include <rc_1dcorr.h>
#include <rc_ip.h>
#include <rc_ipconvert.h>
#include <rc_edge.h>
#include <rc_macro.h>

static int32 log2max(int32 n);

rcSimilarator::rcSimilarator() : _matrixSz (0), _maskValid(false), _cacheSz (0), 
_type(eExhaustive),_depth (rcPixel8),  _cdl (eNorm),  _notify(NULL), _finished(true),_guiUpdate(NULL), _tiny(1e-10)
{
}

rcSimilarator::~rcSimilarator () {}

rcSimilarator::rcSimilarator(rcMatrixGeneration type,
			     rcPixel depth,
			     uint32 matrixSz,
			     uint32 cacheSz,
			     rcCorrelationDefinition cdl,
			     bool notify,
			     rcProgressIndicator* guiUpdate,
			     double tiny)
  : _maskValid(false), _type(type), _depth(depth), _matrixSz(matrixSz),
    _cacheSz(cacheSz),  _cdl (cdl),
    _notify(notify), _finished(true),
    _guiUpdate(guiUpdate), _tiny(tiny)
{
  switch (_depth) {
  case rcPixel8:
  case rcPixel16:
  case rcPixel32:
    break;
  default:
    rmAssert(0);
    break;
  }

  _log2MSz = log2(_matrixSz);
  _ltOn = false;
}


rcSimilarator::rcSimilarator(uint32 matrixSz,
			     bool notify,
			     double tiny)
  : _maskValid(false), _type(eExhaustive), _matrixSz(matrixSz), _depth (rcPixel8),
    _notify(notify), _finished(true),_tiny(tiny), _cacheSz (matrixSz), _cdl (eNorm), _guiUpdate (NULL)
{
  _log2MSz = log2(_matrixSz);
}

bool rcSimilarator::fill(vector<rcWindow>& firstImages)
{
  rmAssert (_matrixSz);

  vector<rcWindow>::iterator start = firstImages.begin();

  if (firstImages.size() > _matrixSz)
    start += (firstImages.size() - _matrixSz);

  switch (_depth) {
  case rcPixel8:
    _tw8.resize(0);
    return internalFill(start, firstImages.end(), _tw8);
  case rcPixel16:
    _tw16.resize(0);
    return internalFill(start, firstImages.end(), _tw16);
  case rcPixel32:
    _tw32.resize(0);
    return internalFill(start, firstImages.end(), _tw32);
  default:
    rmAssert(0);
    break;
  }
  return false;
}

bool rcSimilarator::fill(const rcWindow& projective, bool isColumns)
{
  rmAssert (_matrixSz);

  vector<rcWindow> firstImages (isColumns ? projective.width() : 
				projective.height());
  
  vector<rcWindow>::iterator start = firstImages.begin();

  // Call transpose so that the columns are organised in rows 

  if (isColumns)
    {
      for (int32 x = 0; start < firstImages.end (); start++, x++)
	*start = rfImage8Transpose (rcWindow (projective.frameBuf (), 
					      x, 0, 1, projective.height()));
    }
  else
    {
      rmAssert (start == firstImages.begin());
      for (int32 y = 0; start < firstImages.end (); start++, y++)
	*start = rcWindow (projective.frameBuf (), 0, y, projective.width(), 1);
    }
      
			   

  start = firstImages.begin();
  if (firstImages.size() > _matrixSz)
    start += (firstImages.size() - _matrixSz);

  switch (_depth) {
  case rcPixel8:
    _tw8.resize(0);
    return internalFill(start, firstImages.end(), _tw8);
  case rcPixel16:
    _tw16.resize(0);
    return internalFill(start, firstImages.end(), _tw16);
  case rcPixel32:
    _tw32.resize(0);
    return internalFill(start, firstImages.end(), _tw32);
  default:
    rmAssert(0);
    break;
  }
  return false;
}

bool rcSimilarator::fill(vector<double>& firstData)
{
  rmAssert (_matrixSz);

  vector<double>::iterator start = firstData.begin();
  vector<double>::iterator end = firstData.end();

  if (firstData.size() > _matrixSz)
    start += (firstData.size() - _matrixSz);

  _finished = true;

  for (; start < end; start++) {
    _seqD.push_back(*start);
  }

  rmAssert (_type == eExhaustive);

  if (_SMatrix.empty()) {
    _SMatrix.resize(_matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      _SMatrix[i].resize(_matrixSz);
  }
  else {
    rmAssert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      rmAssert(_SMatrix[i].size() == _matrixSz);
  }

  /* Initialize identity diagonal of _SMatrix.
   */
  unity();

  return (_finished = ssMatrixFill(_seqD)) && genMatrixEntropy(_seqD.size());

}

bool rcSimilarator::fill(deque<double>& firstData)
{
  rmAssert (_matrixSz);

  deque<double>::iterator start = firstData.begin();
  deque<double>::iterator end = firstData.end();

  if (firstData.size() > _matrixSz)
    start += (firstData.size() - _matrixSz);

  _finished = true;

  for (; start < end; start++) {
    _seqD.push_back(*start);
  }

  rmAssert (_type == eExhaustive);

  if (_SMatrix.empty()) {
    _SMatrix.resize(_matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      _SMatrix[i].resize(_matrixSz);
  }
  else {
    rmAssert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      rmAssert(_SMatrix[i].size() == _matrixSz);
  }

  /* Initialize identity diagonal of _SMatrix.
   */
  unity();

  return (_finished = ssMatrixFill(_seqD)) && genMatrixEntropy(_seqD.size());

}

bool rcSimilarator::fill(deque<rcWindow>& firstImages)
{
  rmAssert (_matrixSz);
  deque<rcWindow>::iterator start = firstImages.begin();

  if (firstImages.size() > _matrixSz)
    start += (firstImages.size() - _matrixSz);

  switch (_depth) {
  case rcPixel8:
    _tw8.resize(0);
    return internalFill(start, firstImages.end(), _tw8);
  case rcPixel16:
    _tw16.resize(0);
    return internalFill(start, firstImages.end(), _tw16);
  case rcPixel32:
    _tw32.resize(0);
    return internalFill(start, firstImages.end(), _tw32);
  default:
    rmAssert(0);
    break;
  }
  return false;
}

rcIPair rcSimilarator::fillImageSize() const 
{
  rmAssert (_matrixSz);
  switch (_depth) {
  case rcPixel8:
    return rcIPair (_tw8[0].width(), _tw8[0].height());
  case rcPixel16:
    return rcIPair (_tw16[0].width(), _tw16[0].height());
  case rcPixel32:
    return rcIPair (_tw32[0].width(), _tw32[0].height());
  default:
    rmAssert(0);
    break;
  }

  return rcIPair (0,0);
}

bool rcSimilarator::update(rcWindow nextImage)
{
  rmAssert (_matrixSz);
  switch (_depth) {
  case rcPixel8:
    return internalUpdate(nextImage, _tw8);
  case rcPixel16:
    return internalUpdate(nextImage, _tw16);
  case rcPixel32:
    return internalUpdate(nextImage, _tw32);
  default:
    rmAssert(0);
    break;
  }

  return false;
}

bool rcSimilarator::update(double& nextData)
{
  rmAssert (_matrixSz);
  return internalUpdate(nextData, _seqD);
}

void rcSimilarator::setMask(const rcWindow& mask)
{
  rmAssert (_matrixSz);
  rmAssert(mask.depth() == _depth);

  _maskValid = true;
  if ((_mask.width() != mask.width()) ||
      (_mask.height() != mask.height()))
    _mask = rcWindow(mask.width(), mask.height(), mask.depth());
  _mask.copyPixelsFromWindow(mask);
  _maskN = 0;
  uint32 allOnes = 0xFF;
  if (_depth == rcPixel16)
    allOnes = 0xFFFF;
  else if (_depth == rcPixel32)
    allOnes = 0xFFFFFFFF;
  else
    rmAssert(_depth == rcPixel8);
  
  _maskN = 0;
  for (int32 y = 0; y < _mask.height(); y++)
    for (int32 x = 0; x < _mask.width(); x++) {
      uint32 val = _mask.getPixel(x, y);
      if (val == allOnes)
	_maskN++;
      else
	rmAssert(val == 0);
    }
}

void rcSimilarator::clearMask()
{
  rmAssert (_matrixSz);
  _maskValid = false;
  _mask = rcWindow();
}

bool rcSimilarator::entropies(deque<double>& signal, rcEntropyDefinition definition,
			      		 rcFilteringOp) const
{
  rmAssert (_matrixSz);

  if (definition == eVisualEntropy)
    return entropiesVisualEntropy (signal);

  if (_finished && !_entropies.empty())
    {
      signal = _entropies;
      for (unsigned int i = 0; i < signal.size(); i++)
	{
	  signal[i] = 1 - signal[i];
	}

      return true;
    }
  return false;
}


bool rcSimilarator::entropiesVisualEntropy(deque<double>& signal) const
{
  rmAssert (_matrixSz);
  if (_finished && !_entropies.empty())
    {
      signal = _entropies;
      return true;
    }
  return false;
}


bool
rcSimilarator::selfSimilarityMatrix(deque<deque<double> >& matrix) const
{
  rmAssert (_matrixSz);

  if (_finished && !_entropies.empty() && !_SMatrix.empty()) {
    matrix = _SMatrix;
    return true;
  }

  return false;
}

bool rcSimilarator::sequentialCorrelations (deque<double>& slist) const
{
  rmAssert (_matrixSz);

  if (_finished && !_SList.empty()) {
    slist = _SList;
    return true;
  }

  return false;
}  

bool rcSimilarator::longTermCache (bool onOrOff)
{
  rmAssert (_matrixSz);
  _ltEntropies.resize (_matrixSz);
  _ltOn = onOrOff;
  return _ltOn;
}

bool rcSimilarator::longTermCache () const
{
  return _ltOn;
}

const vector<float>& rcSimilarator::longTermEntropy () const
{
  return _ltEntropies;
}

template <class I, class T>
bool rcSimilarator::internalFill(I start, I end, 
				 deque<rcCorrelationWindow<T> >& tWin)
{
  _finished = true;

  for (; start < end; start++) {
    tWin.push_back(rcCorrelationWindow<T>(*start));
    (*start).frameBuf().unlock();
  }

  if ((_type == eApproxNoMatrix) || (_type == eApproximate)) {
    if (tWin.empty()) {
      _SList.resize(0);
      
      if ((_type == eApproximate) && !_SMatrix.empty())
	_SMatrix.resize(0);

      return false;
    }

    if (_SList.empty())
      _SList.resize(_matrixSz - 1);
    else
      rmAssert(_SList.size() == (_matrixSz - 1));
  }

  if (_type == eApproxNoMatrix)
    return (_finished = ssListFill(tWin)) && genListEntropy(tWin.size());

  if (tWin.empty()) {
    if (!_SMatrix.empty())
      _SMatrix.resize(0);

    return false;
  }

  if (_SMatrix.empty()) {
    _SMatrix.resize(_matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      _SMatrix[i].resize(_matrixSz);
  }
  else {
    rmAssert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      rmAssert(_SMatrix[i].size() == _matrixSz);
  }

  /* Initialize identity diagonal of _SMatrix.
   */
  unity();

  if (_type == eApproximate)
    return (_finished = ssListFill(tWin)) && ssMatrixApproxFill(tWin) &&
      genMatrixEntropy(tWin.size());

  /*
   * if longtermCache is on, write the first matrix size entropies
   */ 
  bool rtn = (_finished = ssMatrixFill(tWin)) && genMatrixEntropy(tWin.size());
  if (rtn && _ltOn && !_entropies.empty())
    {
      for (uint32 i = 0; i < matrixSz(); i++)
	_ltEntropies[i] = (float) _entropies[i];
    }

  return rtn;
}

bool rcSimilarator::ssMatrixFill(deque<double>& data)
{
  rmAssert(_SMatrix.size() == _matrixSz);

  const int32 dataSz = data.size();
  rmAssert(dataSz <= (int32)_matrixSz);

  for (int32 i = 0; i < dataSz; i++)
    {
      for (int32 j = i + 1; j < dataSz; j++)
	{
	  const double r = distance (data[i], data[j]);
	  _SMatrix[j][i] = _SMatrix[i][j] = r;
	}
    }
  return true;
}


template <class T>
bool rcSimilarator::ssMatrixFill(deque<rcCorrelationWindow<T> >& tWin)
{
  rmAssert(_SMatrix.size() == _matrixSz);

  const int32 tWinSz = tWin.size();
  rmAssert(tWinSz <= (int32)_matrixSz);

  int32 cacheSz = _cacheSz;
  if (cacheSz <= 2)
    cacheSz = tWinSz + 2;
  const int32 cacheBlkSz = cacheSz - 2;
  progressNotification* progress = 0;
  if (_notify) progress =
    new progressNotification(tWinSz*(tWinSz-1)/2, _guiUpdate);

  /* The following was designed to work well when the number of images
   * that can be stored in memory at any one time is space limited.
   * In the absence of other consumers of cache resources, it reads in
   * O((N^2)/C) images from disk, where N is the number of images in
   * the movie fifo and C is the count of cache entries available.
   * The basic algorithm works as follows:
   *
   * Step 1 - Use the first cache block size number of images
   * remaining and perform cross correlations on all of them.
   *
   * Step 2 - For all the remaining images, read in one image at a
   * time, correlating each with every image from step 1 before moving
   * on to the next image.
   *
   * Note: In both steps 1) and 2), passes through the cached images
   * are done in alternating directions to guarantee that the most
   * recently used images get processed first. This is done to make
   * performance degradation more linear in the presence of other
   * consumers of the cache. In particular, it limits pathological
   * cases where reading needed images into the cache causes other
   * needed images to be flushed from the cache before they get
   * used. Note that this assumes an LRU type caching policy.
   *
   * Step 3 - Update the current starting point by the cache block
   * size.
   *
   * Repeat the entire process until the new starting point is >= the
   * fifo size.
   */
  for (int32 i = 0; i < tWinSz;
       /* Step 3 - Update start */ i += cacheBlkSz) {
    int32 cacheIncr = 1, cacheBegin, cacheEnd;
    int32 firstUncachedFrame = i + cacheBlkSz;
    if (firstUncachedFrame > tWinSz)
      firstUncachedFrame = tWinSz;

    /* Step 1 - Fill cache.
     */
    for (int32 j = i + 1; j < firstUncachedFrame; j++) {
      if (cacheIncr == 1) {
	cacheIncr = -1;	cacheBegin = j - 1; cacheEnd = i - 1;
      }
      else {
	cacheIncr = 1;	cacheBegin = i; cacheEnd = j;
      }

      for (int32 k = cacheBegin; k != cacheEnd; k += cacheIncr) {
	rmAssert((j >= 0) && (j < tWinSz));
	rmAssert((k >= 0) && (k < tWinSz));
	const double r = correlate(tWin[j], tWin[k]);
	_SMatrix[j][k] = _SMatrix[k][j] = r;
	tWin[k].frameBuf().unlock();
	if (progress && progress->update()) {
	  tWin[j].frameBuf().unlock();
	  delete progress;
	  return false;
	}
      } // End of: for ( k = cacheBegin; k != cacheEnd; k += cacheIncr)
      tWin[j].frameBuf().unlock();
    } // End of: for (int32 j = i + 1; j < firstUncachedFrame; j++)

    /* Step 2 - Correlate remaining images against cached images.
     */
    for (int32 j = firstUncachedFrame; j < tWinSz; j++) {
      if (cacheIncr == 1) {
	cacheIncr = -1;	cacheBegin = firstUncachedFrame-1; cacheEnd = i-1;
      }
      else {
	cacheIncr = 1; cacheBegin = i; cacheEnd = firstUncachedFrame;
      }

      for (int32 k = cacheBegin; k != cacheEnd; k += cacheIncr) {
	rmAssert((j >= 0) && (j < tWinSz));
	rmAssert((k >= 0) && (k < tWinSz));
	const double r = correlate(tWin[j], tWin[k]);
	_SMatrix[j][k] = _SMatrix[k][j] = r;
	tWin[k].frameBuf().unlock();
	if (progress && progress->update()) {
	  tWin[j].frameBuf().unlock();
	  delete progress;
	  return false;
	}
      } // End of: for (k = cacheBegin; k != cacheEnd; k += cacheIncr)
      tWin[j].frameBuf().unlock();
    } // End of: for (int32 j = i + 1; j < firstUncachedFrame; j++)
  }

  if (progress) delete progress;

  return true;
}

/* N^^2 storage but faster algorithm by Peter Roberts...
 *
 * ssMatrixApproxFill - Generate a square matrix of correlation
 * scores, using as input a sequence of "distance 1" correlation
 * scores. By "distance 1" is meant that these are scores for
 * consecutive images from within some movie sequence. So a "distance
 * 2" correlation score is for 2 images that are separated by 1
 * intervening image, and in general, a "distance N" correlation score
 * is for 2 images separated by N-1 intervening images.
 *
 * The scores for "distance 0" and "distance 1" are generated in the
 * obvious way, which doesn't require approximation.
 *
 * "Distance D" correlation scores are approximated using the
 * geometric mean of the intervening pairwise correlations. For
 * example, the approximate correlation score between the 4th and 6th
 * images in a sequence is calculated as the SQUARE root of the
 * following product: corr[4] * corr[5]. The approximate correlation
 * score between the 4th and 7th images in a sequence is calculated as
 * the CUBE root of the following product: corr[4] * corr[5] * corr[6].
 */
template <class T> bool
rcSimilarator::ssMatrixApproxFill(deque<rcCorrelationWindow<T> >& tWin)
{
  uint32 matrixInUse = tWin.size();
  rmAssert(matrixInUse <= _SMatrix.size());
  rmAssert(matrixInUse <= (_SList.size()+1));
  
  if (matrixInUse < 2)
    return true;

  /* First, use the original correlation scores to set up all the
   * "distance 1" entries in the scores array.
   */
  for (uint32 i = 0; i < (matrixInUse-1); i++)
    if (_SList[i] == 0.0)
      _SMatrix[i][i+1] = _SMatrix[i+1][i] = _tiny;
    else
      _SMatrix[i][i+1] = _SMatrix[i+1][i] = _SList[i];

  /* Finally, generate approximate values for all the remaining scores
   * entries.
   */
  for (uint32 i = 0; i < (matrixInUse-2); i++) {
    double product = _SList[i];
    uint32 distance = 2;
    for (uint32 j = i + 2; j < matrixInUse; j++, distance++) {
      product *= _SList[j-1];
      double corrApprox = pow(product, (1./distance));
      if (corrApprox == 0.0)
	_SMatrix[i][j] = _SMatrix[j][i] = _tiny;
      else
	_SMatrix[i][j] = _SMatrix[j][i] = corrApprox;
    }
  }

  return true;
}

template <class T>
bool rcSimilarator::ssListFill(deque<rcCorrelationWindow<T> >& tWin)
{
  if (tWin.size() < 2)
    return true;

  rmAssert(_SList.size() >= (tWin.size()-1));

  progressNotification* progress = 0;
  if (_notify) progress =
    new progressNotification((int32)tWin.size()-1, _guiUpdate);

  for (uint32 i = 1; i < tWin.size(); i++) {
    _SList[i-1] = correlate(tWin[i-1], tWin[i]);
    tWin[i-1].frameBuf().unlock();
    if (progress && progress->update()) {
      tWin[i].frameBuf().unlock();
      delete progress;
      return false;
    }
  }

  tWin[tWin.size()-1].frameBuf().unlock();
  if (progress) delete progress;

  return true;
}


bool rcSimilarator::internalUpdate(double& nextImage, deque<double>& tWin)
{
  /* First, see if an image and its associated results needs to be
   * removed.
   */
  if (tWin.size() == _matrixSz) {
    tWin.pop_front();

    if ((_type == eApproximate) || (_type == eExhaustive))
      shiftSMatrix();
  }
  
  if ((_type == eApproximate) || (_type == eExhaustive)) {
    if (_SMatrix.empty()) {
      _SMatrix.resize(_matrixSz);
      for (uint32 i = 0; i < _matrixSz; i++)
	_SMatrix[i].resize(_matrixSz);
    }

    rmAssert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      rmAssert(_SMatrix[i].size() == _matrixSz);
  }

  tWin.push_back(nextImage);
  return (_finished=ssMatrixUpdate(tWin)) && genMatrixEntropy(tWin.size());

}


template <class T>
bool rcSimilarator::internalUpdate(rcWindow& nextImage,
				   deque<rcCorrelationWindow<T> >& tWin)
{
  /* First, see if an image and its associated results needs to be
   * removed.
   */
  if (tWin.size() == _matrixSz) {
    tWin.pop_front();

    if ((_type == eApproxNoMatrix) || (_type == eApproximate))
      shiftSList();

    if ((_type == eApproximate) || (_type == eExhaustive))
      shiftSMatrix();
  }
  
  if ((_type == eApproxNoMatrix) || (_type == eApproximate)) {
    if (_SList.empty())
      _SList.resize(_matrixSz - 1);
    else
      rmAssert(_SList.size() == (_matrixSz - 1));
  }

  if ((_type == eApproximate) || (_type == eExhaustive)) {
    if (_SMatrix.empty()) {
      _SMatrix.resize(_matrixSz);
      for (uint32 i = 0; i < _matrixSz; i++)
	_SMatrix[i].resize(_matrixSz);
    }

    rmAssert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      rmAssert(_SMatrix[i].size() == _matrixSz);
  }

  tWin.push_back(rcCorrelationWindow<T>(nextImage));
  nextImage.frameBuf().unlock();

  if (_type == eApproxNoMatrix)
    return (_finished = ssListUpdate(tWin)) && genListEntropy(tWin.size());
  else if (_type == eApproximate)
    return (_finished = ssListUpdate(tWin)) && ssMatrixApproxUpdate(tWin)
      && genMatrixEntropy(tWin.size());

  /*
   * if longtermCache is on, write the last entropy value to the cache line
   */ 

  bool rtn = (_finished=ssMatrixUpdate(tWin)) && genMatrixEntropy(tWin.size());
  if (rtn && _ltOn && !_entropies.empty())
    {
      _ltEntropies.push_back ((float) _entropies.back ());
    }

  return rtn;
}

void rcSimilarator::shiftSList()
{
  rmAssert(!_SList.empty());
  _SList.pop_front();
  _SList.push_back(-1.0);
}

void rcSimilarator::shiftSMatrix()
{
  if (_SMatrix.size() != _matrixSz)
    rmExceptionMacro(<<"Similarity Engine Failure");

  _SMatrix.pop_front();

  for (uint32 i = 0; i < (_matrixSz - 1); i++) {
    _SMatrix[i].pop_front();
    _SMatrix[i].push_back(-1.0);
  }

  deque<double> empty;
  _SMatrix.push_back(empty);
  _SMatrix.back().resize(_matrixSz);
}


bool rcSimilarator::ssMatrixUpdate(deque<double>& tWin)
{
  rmAssert(_SMatrix.size() == _matrixSz);
  rmAssert(!tWin.empty());

  const uint32 lastImgIndex = tWin.size() - 1;
  rmAssert(lastImgIndex < _matrixSz);

  _SMatrix[lastImgIndex][lastImgIndex] = 1.0 + _tiny;

  for (uint32 i = 0; i < lastImgIndex; i++) {
    _SMatrix[i][lastImgIndex] = _SMatrix[lastImgIndex][i] = 
      distance (tWin[i], tWin[lastImgIndex]);
  }

  return true;
}


template <class T>
bool rcSimilarator::ssMatrixUpdate(deque<rcCorrelationWindow<T> >& tWin)
{
  rmAssert(_SMatrix.size() == _matrixSz);
  rmAssert(!tWin.empty());

  const uint32 lastImgIndex = tWin.size() - 1;
  rmAssert(lastImgIndex < _matrixSz);

  progressNotification* progress = 0;
  if (_notify) progress =
    new progressNotification(lastImgIndex, _guiUpdate);

  _SMatrix[lastImgIndex][lastImgIndex] = 1.0 + _tiny;

  for (uint32 i = 0; i < lastImgIndex; i++) {
    _SMatrix[i][lastImgIndex] = _SMatrix[lastImgIndex][i] = 
      correlate(tWin[i], tWin[lastImgIndex]);
    tWin[i].frameBuf().unlock();
    if (progress && progress->update()) {
      tWin[lastImgIndex].frameBuf().unlock();
      delete progress;
      return false;
    }
  }

  tWin[lastImgIndex].frameBuf().unlock();

  if (progress) delete progress;

  return true;
}

template <class T>
bool rcSimilarator::ssMatrixApproxUpdate(deque<rcCorrelationWindow<T> >& tWin)
{
  rmAssert(!tWin.empty());

  const uint32 lastImgIndex = tWin.size() - 1;
  rmAssert(lastImgIndex < _matrixSz);

  /* Set up identity value.
   */
  _SMatrix[lastImgIndex][lastImgIndex] = 1.0 + _tiny;
  
  if (lastImgIndex == 0)
    return true;

  /* Fill in "distance 1" values.
   */
  if (_SList[lastImgIndex-1] == 0.0)
      _SMatrix[lastImgIndex][lastImgIndex-1] =
	_SMatrix[lastImgIndex-1][lastImgIndex] = _tiny;
  else
    _SMatrix[lastImgIndex][lastImgIndex-1] =
      _SMatrix[lastImgIndex-1][lastImgIndex] = _SList[lastImgIndex-1];

  if (lastImgIndex == 1)
    return true;

  /* Finally, generate approximate values for all the remaining scores
   * entries.
   */
  double product = _SList[lastImgIndex-1];
  uint32 distance = 2;
  for (uint32 i = lastImgIndex - 2; distance <= lastImgIndex;
       i--, distance++) {
    product *= _SList[i];
    double corrApprox = pow(product, (1./distance));
    if (corrApprox == 0.0)
      _SMatrix[i][lastImgIndex] = _SMatrix[lastImgIndex][i] = _tiny;
    else
      _SMatrix[i][lastImgIndex] = _SMatrix[lastImgIndex][i] = corrApprox;
  }

  return true;
}

template <class T>
bool rcSimilarator::ssListUpdate(deque<rcCorrelationWindow<T> >& tWin)
{
  rmAssert(_SList.size() == (_matrixSz-1));
  rmAssert(!tWin.empty());

  const uint32 lastImgIndex = tWin.size() - 1;

  rmAssert(lastImgIndex <= _SList.size());

  if (lastImgIndex) {
    _SList[lastImgIndex-1] = correlate(tWin[lastImgIndex-1],
				       tWin[lastImgIndex]);
    tWin[lastImgIndex-1].frameBuf().unlock();
    tWin[lastImgIndex].frameBuf().unlock();
  }

  return true;
}

template <class T>
double rcSimilarator::correlate(rcCorrelationWindow<T>& i,
				rcCorrelationWindow<T>& m) const
{
  rcCorr res;

  if (corrDefinition () == rcSimilarator::eHintersect)
    {
      rcHistoStats ih (i.window ());
      rcHistoStats mh (m.window());
      double ir = rf1dSignalIntersection (ih.histogram().begin(), ih.histogram().end(), 
					  mh.histogram().begin(), mh.histogram().end());
      return ir + _tiny;
    }

  if (corrDefinition () == rcSimilarator::eLowSpT)
    {
      rcWindow isampled (i.width(), i.height(), i.depth());
      rcWindow msampled (m.width(), m.height(), m.depth());
      rfGaussianConv (i.window(), isampled, 5);
      rfGaussianConv (m.window(), msampled, 5);
      rfCorrelate(isampled, msampled, _corrParams, res);
      return res.r() + _tiny;
    }

  if (corrDefinition () == rcSimilarator::eShading)
    {
      rcWindow tmp (i.width(), i.height(), i.depth());
      rcWindow mag (i.width(), i.height(), i.depth());
      rcWindow g (i.width(), i.height(), i.depth());
      rcWindow gg (i.width(), i.height(), i.depth());      
      rfSetWindowBorder(mag, uint8 (0));
      rfSetWindowBorder(g, uint8 (0));
      rfSetWindowBorder(gg, uint8 (0));

      rcWindow gwin (g, 1, 1, g.width()-2, g.height()-2);
      rcWindow ggwin (gg, 1, 1, gg.width()-2, gg.height()-2);
      rcWindow mwin (mag, 1, 1, m.width()-2, m.height()-2);
      rfGaussianConv (i.window(), tmp, 5);
      rfSobelEdge (tmp, mwin, gwin);
      rfGaussianConv (m.window(), tmp, 5);
      rfSobelEdge (tmp, mwin, ggwin);
      rfCorrelate(g, gg, _corrParams, res);
      return res.r() + _tiny;
    }

  if (_maskValid)
    rfCorrelate(i.window(), m.window(), _mask, _corrParams, res, _maskN);
  else
    rfCorrelateWindow(i, m, _corrParams, res);

  if (corrDefinition () == rcSimilarator::eNorm)
    return res.r() + _tiny;
  else if (corrDefinition () == rcSimilarator::eRelative)
    return res.relative() + _tiny;
  else
    {
      rmAssert (1);
      return 0;
    }
}


template <class T>
double rcSimilarator::relativeCorrelate(rcCorrelationWindow<T>& i,
				rcCorrelationWindow<T>& m) const
{
  rcCorr res;

  if (_maskValid)
    rfCorrelate(i.window(), m.window(), _mask, _corrParams, res, _maskN);
  else
    rfCorrelateWindow(i, m, _corrParams, res);

  return res.relative() + _tiny;
}

double rcSimilarator::distance (double& i, double& m) const
{
  return (rmSquare ((i-m)/(i+m)));
}

bool rcSimilarator::genMatrixEntropy(uint32 tWinSz)
{
  if (tWinSz != _matrixSz)
    return false;

  /* Create sums array and initialize all the elements.
   */
  vector<double> sums(_matrixSz);
  
  rmAssert(_SMatrix.size() == _matrixSz);

  if (_entropies.empty())
    _entropies.resize(_matrixSz);
  else
    rmAssert(_entropies.size() == _matrixSz);

  for (uint32 i = 0; i < _matrixSz; i++) {
    rmAssert(_SMatrix[i].size() == _matrixSz);
    sums[i] = _SMatrix[i][i];
    _entropies[i] = 0.0;
  }

  for (uint32 i = 0; i < (_matrixSz-1); i++)
    for (uint32 j = (i+1); j < _matrixSz; j++) {
      sums[i] += _SMatrix[i][j];
      sums[j] += _SMatrix[i][j];
    }

  for (uint32  i = 0; i < _matrixSz; i++) {
    for (uint32 j = i; j < _matrixSz; j++) {
      double rr =
	_SMatrix[i][j]/sums[i]; // Normalize for total energy in samples
      _entropies[i] += shannon(rr);
      
      if (i != j) {
	rr = _SMatrix[i][j]/sums[j];//Normalize for total energy in samples
	_entropies[j] += shannon(rr);
      }
    }
    _entropies[i] = _entropies[i]/_log2MSz;// Normalize for cnt of samples
  }

  return true;
}

/* N memory requirement but slower.
 *
 * genListEntropy - Generate a sequence of energy scores, using as
 * input a sequence of "distance 1" correlation scores.  This function
 * uses an algorithm that requires only O(N) space (where N is the
 * size of the temporal window).
 */
bool rcSimilarator::genListEntropy(uint32 tWinSz)
{
  if (tWinSz < _matrixSz)
    return false;

  const uint32 nCorr = _SList.size();

  rmAssert((nCorr + 1) == _matrixSz);

  if (_entropies.empty())
    _entropies.resize(_matrixSz);
  else
    rmAssert(_entropies.size() == _matrixSz);

  /* Create sums array and initialize it.
   */
  deque<double> sums(_matrixSz);

  /* Initialize the sums array with all the distance "0" values.
   */
  for (uint32 i = 0; i < _matrixSz; i++)
    sums[i] = 1.0 + _tiny; // xyzzy 

  /* One pass through to generate sums used in normalization.
   */
  {
    /* First, use the original correlation scores to add in all the
     * "distance 1" values into the sums array.
     */
    for (uint32 i = 0; i < nCorr; i++) {
      if (_SList[i] == 0.0) {
	sums[i] += _tiny;
	sums[i+1] += _tiny;
      }
      else {
	sums[i] += _SList[i];
	sums[i+1] += _SList[i];
      }
    }

    /* Finally, generate approximate values for all the remaining
     * distances and add them to sums.
     */
    for (uint32 i = 0; i < (_matrixSz - 2); i++) {
      double product = _SList[i];
      uint32 distance = 2;
      for (uint32 j = i + 2; j < _matrixSz; j++, distance++) {
	product *= _SList[j-1];
	double corrApprox = pow(product, (1./distance));
	if (corrApprox == 0.0) {
	  sums[i] += _tiny;
	  sums[j] += _tiny;
	}
	else {
	  sums[i] += corrApprox;
	  sums[j] += corrApprox;
	}
      }
    }
  }

  /* Second pass to generate energy scores.
   */
  {
    /* First, accumulate energy scores for identity positions.
     */
    for (uint32 i = 0; i < _matrixSz; i++) {
      double rr = 1.0/sums[i]; // Normalize for total energy in samples
      _entropies[i] = shannon(rr);
    }

    /* Second, use the caller provided correlation scores to add in
     * all the "distance 1" values into the energy array.
     */
    for (uint32 i = 0; i < nCorr; i++) {
      double corrVal = (_SList[i] == 0.0) ? _tiny : _SList[i];

      double rr = corrVal/sums[i]; // Normalize for total energy in samples
      _entropies[i] += shannon(rr);

      rr = corrVal/sums[i+1]; // Normalize for total energy in samples
      _entropies[i + 1] += shannon(rr);
    }

    /* Finally, approximate the remaining correlation scores and use
     * them to finish generating energy scores.
     */
    for (uint32 i = 0; i < (_matrixSz - 2); i++) {
      double product = _SList[i];
      uint32 distance = 2;
      for (uint32 j = i + 2; j < _matrixSz; j++, distance++) {
	product *= _SList[j-1];
	double corrApprox = pow(product, (1./distance));
	if (corrApprox == 0.0)
	  corrApprox = _tiny;
	double rr = corrApprox/sums[i]; // Norm for total energy in samples
	_entropies[i] += shannon(rr);

	rr = corrApprox/sums[j]; // Normalize for total energy in samples
	_entropies[j] += shannon(rr);
      }
    }
  }
  
  /* Do final normalization for the total number of samples used.
   */
  for (uint32 i = 0; i < _matrixSz; i++)
    _entropies[i] = _entropies[i]/_log2MSz;

  return true;
}

void rcSimilarator::unity ()
{
  rmAssert(_SMatrix.size() == _matrixSz);

  for (uint32 i = 0; i < _matrixSz; i++)
    _SMatrix[i][i] = 1.0 + _tiny;
}

ostream& operator<< (ostream& ous, const rcSimilarator& rc)
{
  rcSimilarator* rc2 = const_cast<rcSimilarator*>(&rc);
  deque<deque<double> >& cm = rc2->_SMatrix;
  if (!cm.empty()) {
    ous << "{";
    for (uint32 i = 0; i < cm.size(); i++) {
      ous << "{";
      deque<double>::iterator ds = cm[i].begin();
      for (uint32 j = 0; j < cm.size() - 1; j++, ds++)	   
	ous << *ds << ",";

      if (i < cm.size() - 1)
	ous << *ds << "}," << endl;
      else ous << *ds << "}" << endl;
    }
    ous << "}" << endl;
  }
  return ous;
}

#if 0
  
template <class T>
bool rcSimilarator::filterOp (vector<T>& signal)
{
  float zf (0.0f);
  Vec_O_DP tmp(9);

  _kernel.resize(9);

  NR::savgol (tmp, _kernel.size(),_kernel.size()/2,
	      _kernel.size()/2,0,4);

  // Copy from the wrap arounb order of savgol
  int32 i (0), j;
  for (j = 0, i = (int32) _kernel.size()/2; i>= 0; i--, j++)
    _kernel[j] = tmp[i];
  for (i = 0; i < (int32) _kernel.size()/2; i++, j++)
    _kernel[j] = tmp[_kernel.size()-i-1];

  int32 pow2 = log2max (signal.size());
  vector<double> dSig (1 << pow2, zf);
  dSig.assign (signal.begin(), signal.end());
  if ((1 << pow2) != (int32) dSig.size())
    dSig.resize (1<< pow2, zf);

  Vec_O_DP vsig (&dSig[0], dSig.size());
  Vec_O_DP vfilt (dSig.size());
  
  NR::convlv (vsig, tmp, 1, vfilt);

  for (i = 0; i < (int32) signal.size(); i++)
    signal[i] = vfilt[i];

  return true;
}

template <class T>
double rcSimilarator::genPeriodicity (const vector<T>& signal, const vector<T>& absc, rcDPair& freq)
{
  const double ofac (4.0f);
  const double hfac (1.0f);
  const double nout (0.5f * ofac * hfac * signal.size());
  Vec_O_DP px (signal.size() * 2);
  Vec_O_DP py (px.size());
  double prob;
  int32 jmax, nf;

  Vec_O_DP dSig (&signal[0], signal.size());
  Vec_O_DP vSig (&absc[0], absc.size());

  NR::period (vSig, dSig, ofac, hfac, px, py, nf, jmax, prob);

  freq.x() = jmax;
  freq.y() = py[jmax];

  vector<uint32> peakLocs;
  vector<double> interLocs, interVals, dSigVec (py.size());
  for (int32 i = 0; i < py.size(); i++)
    dSigVec[i] = py[i];

  rf1DPeakDetect(dSigVec, peakLocs, interLocs,
		 interVals, 0.10);

  // Assume min frequency is 1/2 as many frames per period
  // ==> 2 
  float minFrequency = 2.0;

  /* Find the the frequency, greater than minFrequency, with the
   * greatest amplitude.
   */
  uint32 maxFreqIndex = 0;
  double maxAmplitude = -1.0;
  for (uint32 ii = 0; ii < peakLocs.size(); ii++) {
    if ((interLocs[ii] > minFrequency) && (interVals[ii] > maxAmplitude)) {
      maxFreqIndex = ii;
      maxAmplitude = interVals[ii];
    }
    if (1)
      fprintf(stderr, " (%d %f) inter: (%f, %f)\n",
	      peakLocs[ii], dSigVec[peakLocs[ii]],
	      interLocs[ii], interVals[ii]);
  }
  
  if (1)
    fprintf(stderr, "maxAmplitude %f maxFreqIndex %d minFrequency %f\n",
	    maxAmplitude, maxFreqIndex, minFrequency);

  if (maxAmplitude == -1.0)
    return 0.0;

  if (1)
    fprintf(stderr, "Return max peak - @ %d: loc %f amp %f\n",
	    maxFreqIndex, interLocs[maxFreqIndex],
	    interVals[maxFreqIndex]);

  return interLocs[maxFreqIndex]/nout;

}


bool rcSimilarator::filter (vector<double>& signal)
{
  return filterOp (signal);
}

double rcSimilarator::periodicity (const vector<double>& signal, const vector<double>& absc, rcDPair& freq)
{
  return genPeriodicity (signal, absc, freq);
}

#endif


////////////////////////////////////////////////////////////////////////////////
//
//	log2max
//
//	This function computes the ceiling of log2(n).  For example:
//
//	log2max(7) = 3
//	log2max(8) = 3
//	log2max(9) = 4
//
////////////////////////////////////////////////////////////////////////////////
static int32 log2max(int32 n)
{
  int32 power = 1;
  int32 k = 1;
  
  if (n==1) {
    return 0;
  }
	
  while ((k <<= 1) < n) {
    power++;
  }
	
  return power;
}

      

  


// 1D Distance Histogram
//
void rf1DdistanceHistogram (const vector<double>& signal, vector<double>& dHist)
{
  int32 n;

  n = signal.size();
  rmAssert (n);
  dHist.resize (n);

  // Get container value type
  vector<double>::const_iterator ip, jp;

  for (ip = signal.begin(); ip < signal.end(); ip++)
    for (jp = ip + 1; jp < signal.end(); jp++)
    {
      double iv (*ip);
      double jv (*jp);
      int32 dj = jp - signal.begin();
      int32 di = ip - signal.begin();
      dj = dj - di;
      rmAssert (dj >= 0 && dj < n);
      dHist[dj] += rmSquare (iv - jv);
    }

}
