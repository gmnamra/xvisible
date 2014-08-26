/*
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

// analysis
#include "self_similarity.h"
#include  "image_correlation.h"
#include "vf_math.h"



void self_similarity_producer::norm_scale (const std::deque<double>& src, std::deque<double>& dst, double pw) const
{
    deque<double>::const_iterator bot = std::min_element (src.begin (), src.end() );
    deque<double>::const_iterator top = std::max_element (src.begin (), src.end() );
	
    double scaleBy = *top - *bot;
    dst.resize (src.size ());
    for (int ii = 0; ii < src.size (); ii++)
    {
        double dd = (src[ii] - *bot) / scaleBy;
        dst[ii] = pow (dd, 1.0 / pw);
    }
}

static int32 log2max(int32 n);

self_similarity_producer::self_similarity_producer() : _matrixSz (0), _maskValid(false), _cacheSz (0), 
_type(eExhaustive),_depth (rpixel8),  _cdl (eNorm),  _notify(NULL), _finished(true),_guiUpdate(NULL), _tiny(1e-10)
{
}

self_similarity_producer::~self_similarity_producer () {}

self_similarity_producer::self_similarity_producer(rcMatrixGeneration type,
			     pixel_ipl_t depth,
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
    _corrParams.pd = ByteWise; // use each byte of a multibyte pixel
  switch (_depth) {
  case rpixel8:
  case rpixel16:
  case rpixel32:
    break;
  default:
    assert(0);
    break;
  }

  _log2MSz = log2(_matrixSz);
  _ltOn = false;
}


self_similarity_producer::self_similarity_producer(uint32 matrixSz,
			     bool notify,
			     double tiny)
  : _maskValid(false), _type(eExhaustive), _matrixSz(matrixSz), _depth (rpixel8),
    _notify(notify), _finished(true),_tiny(tiny), _cacheSz (matrixSz), _cdl (eNorm), _guiUpdate (NULL)
{
  _log2MSz = log2(_matrixSz);
}

bool self_similarity_producer::fill(vector<roi_window>& firstImages)
{
  assert(_matrixSz);

  vector<roi_window>::iterator start = firstImages.begin();

  if (firstImages.size() > _matrixSz)
    start += (firstImages.size() - _matrixSz);

  switch (_depth) {
  case rpixel8:
    _tw8.resize(0);
    return internalFill(start, firstImages.end(), _tw8);
  case rpixel16:
    _tw16.resize(0);
    return internalFill(start, firstImages.end(), _tw16);
  case rpixel32:
    _tw32.resize(0);
    return internalFill(start, firstImages.end(), _tw32);
  default:
    assert(0);
    break;
  }
  return false;
}


bool self_similarity_producer::fill(vector<double>& firstData)
{
  assert(_matrixSz);

  vector<double>::iterator start = firstData.begin();
  vector<double>::iterator end = firstData.end();

  if (firstData.size() > _matrixSz)
    start += (firstData.size() - _matrixSz);

  _finished = true;

  for (; start < end; start++) {
    _seqD.push_back(*start);
  }

  assert(_type == eExhaustive);

  if (_SMatrix.empty()) {
    _SMatrix.resize(_matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      _SMatrix[i].resize(_matrixSz);
  }
  else {
    assert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      assert(_SMatrix[i].size() == _matrixSz);
  }

  /* Initialize identity diagonal of _SMatrix.
   */
  unity();

  return (_finished = ssMatrixFill(_seqD)) && genMatrixEntropy(_seqD.size());

}

bool self_similarity_producer::fill(deque<double>& firstData)
{
  assert(_matrixSz);

  deque<double>::iterator start = firstData.begin();
  deque<double>::iterator end = firstData.end();

  if (firstData.size() > _matrixSz)
    start += (firstData.size() - _matrixSz);

  _finished = true;

  for (; start < end; start++) {
    _seqD.push_back(*start);
  }

  assert(_type == eExhaustive);

  if (_SMatrix.empty()) {
    _SMatrix.resize(_matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      _SMatrix[i].resize(_matrixSz);
  }
  else {
    assert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      assert(_SMatrix[i].size() == _matrixSz);
  }

  /* Initialize identity diagonal of _SMatrix.
   */
  unity();

  return (_finished = ssMatrixFill(_seqD)) && genMatrixEntropy(_seqD.size());

}

bool self_similarity_producer::fill(deque<roi_window>& firstImages)
{
  assert(_matrixSz);
  deque<roi_window>::iterator start = firstImages.begin();

  if (firstImages.size() > _matrixSz)
    start += (firstImages.size() - _matrixSz);

  switch (_depth) {
  case rpixel8:
    _tw8.resize(0);
    return internalFill(start, firstImages.end(), _tw8);
  case rpixel16:
    _tw16.resize(0);
    return internalFill(start, firstImages.end(), _tw16);
  case rpixel32:
    _tw32.resize(0);
    return internalFill(start, firstImages.end(), _tw32);
  default:
    assert(0);
    break;
  }
  return false;
}

std::pair<int32,int32> self_similarity_producer::fillImageSize() const 
{
  assert(_matrixSz);
  switch (_depth) {
  case rpixel8:
    return std::pair<int32,int32> (_tw8[0].width(), _tw8[0].height());
  case rpixel16:
    return std::pair<int32,int32> (_tw16[0].width(), _tw16[0].height());
  case rpixel32:
    return std::pair<int32,int32> (_tw32[0].width(), _tw32[0].height());
  default:
    assert(0);
    break;
  }

  return std::pair<int32,int32> (0,0);
}

bool self_similarity_producer::update(roi_window nextImage)
{
  assert(_matrixSz);
  switch (_depth) {
  case rpixel8:
    return internalUpdate(nextImage, _tw8);
  case rpixel16:
    return internalUpdate(nextImage, _tw16);
  case rpixel32:
    return internalUpdate(nextImage, _tw32);
  default:
    assert(0);
    break;
  }

  return false;
}

bool self_similarity_producer::update(double& nextData)
{
  assert(_matrixSz);
  return internalUpdate(nextData, _seqD);
}

void self_similarity_producer::setMask(const roi_window& mask)
{
  assert(_matrixSz);
  assert(mask.depth() == _depth);

  _maskValid = true;
  if ((_mask.width() != mask.width()) ||
      (_mask.height() != mask.height()))
    _mask = roi_window(mask.width(), mask.height(), mask.depth());
  _mask.copyPixelsFromWindow(mask);
  _maskN = 0;
  uint32 allOnes = 0xFF;
  if (_depth == rpixel16)
    allOnes = 0xFFFF;
  else if (_depth == rpixel32)
    allOnes = 0xFFFFFFFF;
  else
    assert(_depth == rpixel8);
  
  _maskN = 0;
  for (int32 y = 0; y < _mask.height(); y++)
    for (int32 x = 0; x < _mask.width(); x++) {
      uint32 val = _mask.getPixel(x, y);
      if (val == allOnes)
	_maskN++;
      else
	assert(val == 0);
    }
}

void self_similarity_producer::clearMask()
{
  assert(_matrixSz);
  _maskValid = false;
  _mask = roi_window();
}

bool self_similarity_producer::entropies(deque<double>& signal, rcEntropyDefinition definition,
			      		 rcFilteringOp) const
{
  assert(_matrixSz);

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

/*
 *  Visual Entropy is used for mean projection versus ACI an entropic projection
 */
bool self_similarity_producer::entropiesVisualEntropy(deque<double>& signal) const
{
  assert(_matrixSz);
  if (_finished && !_sums.empty())
    {
      signal = _sums;
      return true;
    }
  return false;
}


bool
self_similarity_producer::selfSimilarityMatrix(deque<deque<double> >& matrix) const
{
  assert(_matrixSz);

  if (_finished && !_entropies.empty() && !_SMatrix.empty()) {
    matrix = _SMatrix;
    return true;
  }

  return false;
}

bool self_similarity_producer::sequentialCorrelations (deque<double>& slist) const
{
  assert(_matrixSz);

  if (_finished && !_SList.empty()) {
    slist = _SList;
    return true;
  }

  return false;
}  

bool self_similarity_producer::longTermCache (bool onOrOff)
{
  assert(_matrixSz);
  _ltEntropies.resize (_matrixSz);
  _ltOn = onOrOff;
  return _ltOn;
}

bool self_similarity_producer::longTermCache () const
{
  return _ltOn;
}

const vector<float>& self_similarity_producer::longTermEntropy () const
{
  return _ltEntropies;
}

template <class I, class T>
bool self_similarity_producer::internalFill(I start, I end, 
				 deque<roi_window_t<T> >& tWin)
{
  _finished = true;

  for (; start < end; start++) {
    tWin.push_back(roi_window_t<T>(*start));
    (*start).frameBuf().unlock();
  }


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
    assert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      assert(_SMatrix[i].size() == _matrixSz);
  }

  /* Initialize identity diagonal of _SMatrix.
   */
  unity();

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

bool self_similarity_producer::ssMatrixFill(deque<double>& data)
{
  assert(_SMatrix.size() == _matrixSz);

  const int32 dataSz = data.size();
  assert(dataSz <= (int32)_matrixSz);

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
bool self_similarity_producer::ssMatrixFill(deque<roi_window_t<T> >& tWin)
{
  assert(_SMatrix.size() == _matrixSz);

  const int32 tWinSz = tWin.size();
  assert(tWinSz <= (int32)_matrixSz);

  int32 cacheSz = _cacheSz;
  if (cacheSz <= 2)
    cacheSz = tWinSz + 2;
  const int32 cacheBlkSz = cacheSz - 2;


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
	assert((j >= 0) && (j < tWinSz));
	assert((k >= 0) && (k < tWinSz));
	const double r = correlate(tWin[j], tWin[k]);
	_SMatrix[j][k] = _SMatrix[k][j] = r;
	tWin[k].frameBuf().unlock();
//	if (progress && progress->update()) {
//	  tWin[j].frameBuf().unlock();
//	  delete progress;
//	  return false;
//	}
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
	assert((j >= 0) && (j < tWinSz));
	assert((k >= 0) && (k < tWinSz));
	const double r = correlate(tWin[j], tWin[k]);
	_SMatrix[j][k] = _SMatrix[k][j] = r;
	tWin[k].frameBuf().unlock();
//	if (progress && progress->update()) {
//	  tWin[j].frameBuf().unlock();
//	  delete progress;
//	  return false;
//	}
      } // End of: for (k = cacheBegin; k != cacheEnd; k += cacheIncr)
      tWin[j].frameBuf().unlock();
    } // End of: for (int32 j = i + 1; j < firstUncachedFrame; j++)
  }



  return true;
}


template <class T>
bool self_similarity_producer::ssListFill(deque<roi_window_t<T> >& tWin)
{
  if (tWin.size() < 2)
    return true;

  assert(_SList.size() >= (tWin.size()-1));

  for (uint32 i = 1; i < tWin.size(); i++) {
    _SList[i-1] = correlate(tWin[i-1], tWin[i]);
    tWin[i-1].frameBuf().unlock();
//    if (progress && progress->update()) {
//      tWin[i].frameBuf().unlock();
//      delete progress;
//      return false;
//    }
  }

  tWin[tWin.size()-1].frameBuf().unlock();
  return true;
}


bool self_similarity_producer::internalUpdate(double& nextImage, deque<double>& tWin)
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

    assert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      assert(_SMatrix[i].size() == _matrixSz);
  }

  tWin.push_back(nextImage);
  return (_finished=ssMatrixUpdate(tWin)) && genMatrixEntropy(tWin.size());

}


template <class T>
bool self_similarity_producer::internalUpdate(roi_window& nextImage,
				   deque<roi_window_t<T> >& tWin)
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

    assert(_SMatrix.size() == _matrixSz);
    for (uint32 i = 0; i < _matrixSz; i++)
      assert(_SMatrix[i].size() == _matrixSz);
  }

  tWin.push_back(roi_window_t<T>(nextImage));
  nextImage.frameBuf().unlock();

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

void self_similarity_producer::shiftSList()
{
  assert(!_SList.empty());
  _SList.pop_front();
  _SList.push_back(-1.0);
}

void self_similarity_producer::shiftSMatrix()
{
  if (_SMatrix.size() != _matrixSz)
    throw vf_exception::assertion_error ("Similarity Engine Failure");

  _SMatrix.pop_front();

  for (uint32 i = 0; i < (_matrixSz - 1); i++) {
    _SMatrix[i].pop_front();
    _SMatrix[i].push_back(-1.0);
  }

  deque<double> empty;
  _SMatrix.push_back(empty);
  _SMatrix.back().resize(_matrixSz);
}


bool self_similarity_producer::ssMatrixUpdate(deque<double>& tWin)
{
  assert(_SMatrix.size() == _matrixSz);
  assert(!tWin.empty());

  const uint32 lastImgIndex = tWin.size() - 1;
  assert(lastImgIndex < _matrixSz);

  _SMatrix[lastImgIndex][lastImgIndex] = 1.0 + _tiny;

  for (uint32 i = 0; i < lastImgIndex; i++) {
    _SMatrix[i][lastImgIndex] = _SMatrix[lastImgIndex][i] = 
      distance (tWin[i], tWin[lastImgIndex]);
  }

  return true;
}


template <class T>
bool self_similarity_producer::ssMatrixUpdate(deque<roi_window_t<T> >& tWin)
{
  assert(_SMatrix.size() == _matrixSz);
  assert(!tWin.empty());

  const uint32 lastImgIndex = tWin.size() - 1;
  assert(lastImgIndex < _matrixSz);

//  progressNotification* progress = 0;
//  if (_notify) progress =
//    new progressNotification(lastImgIndex, _guiUpdate);

  _SMatrix[lastImgIndex][lastImgIndex] = 1.0 + _tiny;

  for (uint32 i = 0; i < lastImgIndex; i++) {
    _SMatrix[i][lastImgIndex] = _SMatrix[lastImgIndex][i] = 
      correlate(tWin[i], tWin[lastImgIndex]);
    tWin[i].frameBuf().unlock();
//    if (progress && progress->update()) {
//      tWin[lastImgIndex].frameBuf().unlock();
//      delete progress;
//      return false;
//    }
  }

  tWin[lastImgIndex].frameBuf().unlock();

  return true;
}


template <class T>
double self_similarity_producer::correlate(roi_window_t<T>& i,
				roi_window_t<T>& m) const
{
  rcCorr res;

  if (_maskValid)
  {
    rfCorrelate(i, m, _mask, _corrParams, res, _maskN);
  }
  else
    rfCorrelateWindow(i, m, _corrParams, res);

  if (corrDefinition () == self_similarity_producer::eNorm)
    return res.r() + _tiny;
  else if (corrDefinition () == self_similarity_producer::eRelative)
    return res.relative() + _tiny;
  else
    {
      assert(1);
      return 0;
    }
}


template <class T>
double self_similarity_producer::relativeCorrelate(roi_window_t<T>& i,
				roi_window_t<T>& m) const
{
  rcCorr res;

  if (_maskValid)
    rfCorrelate(i, m, _mask, _corrParams, res, _maskN);
  else
    rfCorrelateWindow(i, m, _corrParams, res);

  return res.relative() + _tiny;
}

double self_similarity_producer::distance (double& i, double& m) const
{
    return ( ((i-m)/(i+m)));
}

bool self_similarity_producer::genMatrixEntropy(uint32 tWinSz)
{
  if (tWinSz != _matrixSz)
    return false;

  /* Create sums array and initialize all the elements.
   */

  assert(_SMatrix.size() == _matrixSz);

    if (_sums.empty())
        _sums.resize(_matrixSz);
    else
        assert(_sums.size() == _matrixSz);

  if (_entropies.empty())
    _entropies.resize(_matrixSz);
  else
    assert(_entropies.size() == _matrixSz);

  for (uint32 i = 0; i < _matrixSz; i++) {
    assert(_SMatrix[i].size() == _matrixSz);
    _sums[i] = _SMatrix[i][i];
    _entropies[i] = 0.0;
  }

  for (uint32 i = 0; i < (_matrixSz-1); i++)
    for (uint32 j = (i+1); j < _matrixSz; j++) {
      _sums[i] += _SMatrix[i][j];
      _sums[j] += _SMatrix[i][j];
    }

    for (uint32  i = 0; i < _matrixSz; i++) {
    for (uint32 j = i; j < _matrixSz; j++) {
      double rr =
	_SMatrix[i][j]/_sums[i]; // Normalize for total energy in samples
      _entropies[i] += shannon(rr);
      
      if (i != j) {
	rr = _SMatrix[i][j]/_sums[j];//Normalize for total energy in samples
	_entropies[j] += shannon(rr);
      }
    }
    _entropies[i] = _entropies[i]/_log2MSz;// Normalize for cnt of samples
  }

    for (int ii = 0; ii < _sums.size (); ii++)
    {
        _sums[ii] /= _matrixSz;
//        _sums[ii] = _sums[ii] - 1;
    }

  return true;
}

void self_similarity_producer::unity ()
{
  assert(_SMatrix.size() == _matrixSz);

  for (uint32 i = 0; i < _matrixSz; i++)
    _SMatrix[i][i] = 1.0 + _tiny;
}

ostream& operator<< (ostream& ous, const self_similarity_producer& rc)
{
  self_similarity_producer* rc2 = const_cast<self_similarity_producer*>(&rc);
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
bool self_similarity_producer::filterOp (vector<T>& signal)
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
double self_similarity_producer::genPeriodicity (const vector<T>& signal, const vector<T>& absc, rcDPair& freq)
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


bool self_similarity_producer::filter (vector<double>& signal)
{
  return filterOp (signal);
}

double self_similarity_producer::periodicity (const vector<double>& signal, const vector<double>& absc, rcDPair& freq)
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
  assert(n);
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
      assert(dj >= 0 && dj < n);
      dHist[dj] += rmSquare (iv - jv);
    }

}
