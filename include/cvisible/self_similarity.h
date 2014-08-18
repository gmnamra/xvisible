

#ifndef _SIMILARITY_H
#define _SIMILARITY_H

#include <stdio.h>
#include <math.h>

#include <vector>
#include <deque>
#include <memory>

// util
#include "rc_types.h"

// visual
#include "roi_window.h"

// analysis
#include  "image_correlation.h"

#include <boost/shared_ptr.hpp>

class rcProgressIndicator;

/* self_similarity_producer - Entropy signal generating class. There are two
 * steps to this process.  First, a self-similarity matrix is
 * calculated between a set of images.  Second, a measure of the
 * entropy between the images in this set is calculated based upon the
 * previously calculated self-similarity matrix.
 *
 * Note: Sometimes we generate the matrix info on the fly, removing
 * the need to store what can become a large 2D array. In this case it
 * is still valid, from a conceptual point of view, to think of this
 * as a two step process. To simplify things, the following
 * descriptions will assume both steps are being performed.
 * 
 */
class self_similarity_producer 
{
 public:
  self_similarity_producer ();

 // Destructor
  virtual ~self_similarity_producer();


  /* rcMatrixGeneration - Self-similarity matrix creation options.
   *
   * eExhaustive        - Perform pairwise correlations on all images
   *                      within the temporal window.
   *
   * eApproxNoMatrix    - Perform pairwise correlation only on
   *                      temporally adjacent images. Use algorithm
   *                      that doesn't require storing entire matrix
   *                      of approximate results to generate final
   *                      entropy signal.
   *
   * eApproximate       - Perform pairwise correlation only on
   *                      temporally adjacent images. Use algorithm
   *                      that first approximates the entire self
   *                      similarity matrix before generating final
   *                      entropy signal.
   */
  enum rcMatrixGeneration {
    eExhaustive,
    eApproxNoMatrix,
    eApproximate
  };

  enum rcEntropyDefinition {
    eACI,
    eVisualEntropy,
  };

  enum rcFilteringOp {
    eSavgol,
    eMovingAverage,
    eNone
  };

  /*
   * Correlation Definition Control
   *
   * eNorm:      Normalized Correlation
   * eRelative:  Relative Correlation
   *
   */
  enum rcCorrelationDefinition {
    eNorm,
    eRelative,
    eHintersect,
    eLowSpT,
    eShading
  };

  /* ctor - Build an self_similarity_producer object that will generate entropy
   * based upon the following arguments:
   *
   * matrixSz -  Used to size the internally used self-similarity
   *             matrix and must be greater than 1. It can also be
   *             thought of as defining the temporal window size to be
   *             used.
   *
   * depth -     Pixel depth of images to be analyzed.
   *
   * cacheSz -   The count of video cache entries the algorithm used to
   *             generate the self-similarity matrix should assume are
   *             available to it.
   *
   * notify -    If true, progress indication will be performed during
   *             potentially slow calculations.
   *
   * guiUpdate - Hook for GUI based user interface code to both get
   *             progress update information to display to the user
   *             and to allow users to terminate slow operations.
   *
   *             Note: guiUpdate is ignored if notify is false.  If
   *             notify is true, but guiUpdate is NULL, progress
   *             indication information is printed to stderr.
   *
   * tiny      - Give client a way to screw up signal generating
   *             calculation.
   *
   * 2nd ctor is for exhaustive a double scalar data
   *
   */
  self_similarity_producer(rcMatrixGeneration type,
		rcPixel depth,
		uint32 matrixSz,
		uint32 cacheSz,
		rcCorrelationDefinition cdl = eNorm,
		bool notify = false,
		rcProgressIndicator* guiUpdate = 0,
		double tiny = 1e-10);

  self_similarity_producer(uint32 matrixSz,
		bool notify,
		double tiny);

  /* Mutator Functions
   */
  /* fill - Allow client to specify an initial temporal window's worth
   * of images. If a full temporal window's worth of images are passed
   * in, an entropy signal is generated.
   *
   * Clears any existing intermediate results, so passing a 0 length
   * set of images can be used to just clear intermediate resutls.
   *
   * If firstImages is larger than the temporal window size, the
   * beginning images are ignored. In all events, up to a temporal
   * window's worth of self-similarity calculations are performed.
   * 
   * Up to the last temporal window size number of images from
   * firstImages are retained within the object for use in subsequent
   * calls to update().
   *
   * If firstImages is at least temporal window size large, then there
   * are enough images to calculate an entropy signal. In this case,
   * an entropy signal is generated and true will be returned.
   * Otherwise, no calculations are done and false is returned.
   */
  bool fill(vector<roi_window>& firstImages);
  bool fill(deque<roi_window>& firstImages);
  bool fill(vector<double>& firstData);
  bool fill(deque<double>& firstData);
  bool fill(const roi_window& projective, bool isColumns = true);  

  /* update - Input the next image in a video stream. If a full
   * temporal window's worth of images are available, a new entropy
   * signal is generated.
   *
   * The image passed in is appended to the end of the internal
   * temporal window queue. If the queue was already full, the first
   * image is removed, along with any information relating to this
   * image stored in the self-similarity matrix.
   *
   * Similar to fill(), the self-similarity calculations are
   * performed. If this results in a full matrix of self-similarity
   * information being available, a new entropy signal is generated
   * and true will be returned.  Otherwise, no calculations are done
   * and false is returned.
   *
   * fillImageSize() returns the size of the first image in the filled pipeline
   */
  bool update(roi_window nextImage);
  bool update(double& nextData);
  rcIPair fillImageSize () const;

  void setMask(const roi_window& mask);
  void clearMask();

  bool longTermCache (bool);
  bool longTermCache () const;
  const vector<float>& longTermEntropy () const;

  /* Accessor Functions
   */
  /* entropies - If an entropy signal has been calculated, store it
   * in signal and return true. Otherwise, return false.
   */
  bool entropies(deque<double>& signal, rcEntropyDefinition d = eACI, rcFilteringOp f = eNone) const;

  /* sequential correlations - 
   *
   */
  bool sequentialCorrelations (deque<double>& slist) const;
  /* selfSimilarityMatrix - If an entropy signal has been calculated,
   * and its self-similarity matrix has been saved, save a copy of it
   * in matrix. Otherwise, return false.
   */
  bool selfSimilarityMatrix(deque<deque<double> >& matrix) const;

  /*
   * Filtering operation on output signal
   */
  bool filter (vector<double>&);
  double periodicity (const vector<double>& signal, const vector<double>& absc, rcDPair& freq);

  /*
   * Mutual Information Measurement of a Signal
   */
  void mu (vector<double>& src, vector<double>& dst);

  /* aborted - Returns true if an update() or a fill() operation failed
   * because user aborted the operation, false otherwise.
   */
  bool aborted() const { return !_finished; }

  uint32 matrixSz() const { return _matrixSz; }
  rcMatrixGeneration matGenType() const { return _type; }
  uint32 cacheSz() const { return _cacheSz; }
  rcPixel depth() const { return _depth; }
  rcCorrelationDefinition corrDefinition () const { return _cdl; }

  friend ostream& operator<< (ostream&, const self_similarity_producer&);

 private:
  self_similarity_producer(const self_similarity_producer& rhs);
  self_similarity_producer& operator=(const self_similarity_producer& rhs);

  bool operator<(const self_similarity_producer& rhs) const;
  bool operator==(const self_similarity_producer& rhs) const;

  /*
   * specific entropy calculations
   */
  bool entropiesVisualEntropy (deque<double>& signal) const;
    void norm_scale (const std::deque<double>& src, std::deque<double>& dst, double pw) const;

  /* Internal helper fcts
   */
  /* internalFill - Called by fill() fcts to perform pixel size
   * specific fill() functionality.
   */
  template <class I, class T>
    bool internalFill(I start, I end, 
		      deque<roi_window_t<T> >& tWin);

  /* s*Fill - Take initial fill worth of images and perform
   * correlations required to calculate self-similarity information
   * required to generate entropy signal. If enough self-similarity
   * info is available, generate the entropy signal.
   */
  template <class T>
    bool ssMatrixFill(deque<roi_window_t<T> >& tWin);
  template <class T>
    bool ssMatrixApproxFill(deque<roi_window_t<T> >& tWin);
  template <class T>
    bool ssListFill(deque<roi_window_t<T> >& tWin);

  bool ssMatrixFill(deque<double>& data);

  /* internalUpdate - Called by update() fct to perform pixel size
   * specific update() functionality.
   */
  template <class T>
    bool internalUpdate(roi_window& nextImage,
			deque<roi_window_t<T> >& tWin);

  bool internalUpdate(double& nextImage,
			deque<double>& tWin);


  /* shiftS* - Fcts that shift self-similarity results by one image.
   */
  void shiftSList();
  void shiftSMatrix();

  /* s*Update - Perform correlations between the last image in the
   * temporal window and all the other images in the window. Use this
   * to update the current self-similarity information. If enough
   * self-similarity info is available, generate the entropy signal.
   */
  template <class T>
    bool ssMatrixUpdate(deque<roi_window_t<T> >& tWin);
  template <class T>
    bool ssMatrixApproxUpdate(deque<roi_window_t<T> >& tWin);
  template <class T>
    bool ssListUpdate(deque<roi_window_t<T> >& tWin);

  bool ssMatrixUpdate(deque<double>& data);

  /* correlate - Perform correlation on the given two images and
   * return the correlation score.
   */
  template <class T>
    double relativeCorrelate(roi_window_t<T>& i,
		     roi_window_t<T>& m) const;

  /* correlate - Perform correlation on the given two images and
   * return the correlation score.
   */
  template <class T>
    double correlate(roi_window_t<T>& i,
		     roi_window_t<T>& m) const;

  /* HistogramInstersection - Perform histogram intersection on the given two images and
   * return the correlation score.
   */
  template <class T>
    float histogramIntersection(roi_window_t<T>& i,
		     roi_window_t<T>& m) const;

  /* 
   * (i - m) / (i + m) 
   */
  double distance (double& i, double& m) const;

  /* genMatrixEntropy - If a full matix worth of self-similarity info
   * is available, generate an entropy signal and return true.
   * Otherwise, just return false.
   */
  bool genMatrixEntropy(uint32 tWinSz);

  /* genListEntropy - If a full list worth of self-similarity info
   * is available, generate an entropy signal and return true.
   * Otherwise, just return false.
   */
  bool genListEntropy(uint32 tWinSz);

  /*
   * Filtering operation on output signal
   */
  template <class T>
    bool filterOp (vector<T>&);

  template <class T>
  double genPeriodicity (const vector<T>& signal, const vector<T>& absc, rcDPair& freq);

  /* unity - Initialize self-similarity matrix to have identity
   * value along the identity diagonal.
   */
  void unity();

  double shannon (double r) const { return (-1.0 * r * log2 (r)); }


  /* Masking related information
   */
  bool                               _maskValid;
  roi_window                           _mask;
  int32                            _maskN;

  /* Control information
   */
  const rcMatrixGeneration           _type;
  const rcPixel                _depth;
  const uint32                     _matrixSz;
  const uint32                     _cacheSz;
  const rcCorrelationDefinition      _cdl;
  const bool                         _notify;
  bool                               _finished;
  rcProgressIndicator*         const _guiUpdate;
  const double                       _tiny;
  double                             _log2MSz;
  rsCorrParams                       _corrParams;
  
  /* Inputs - Temporal windows used to store the images required to
   * calculate the entropy signal. Pixel depth is assumed to be same
   * in all images, so only one of these may be in use at any one
   * time.
   */
  deque<roi_window_t<uint8> >  _tw8;
  deque<roi_window_t<uint16> > _tw16;
  deque<roi_window_t<uint32> > _tw32;

  /*
   * Inputs - Storage of scalar sequence of data
   *
   */
  deque<double> _seqD;

  /* Outputs
   */
  deque<deque<double> >        _SMatrix;   // Used in eExhaustive and
                                           // eApproximate cases
  deque<double>                _SList;     // Used in approximate cases
  deque<double>                _entropies; // Final entropy signal
  mutable deque<double>                _sums;     // Final mean signal
  vector<double>               _kernel;    // Filtering Operation Kernel

  /* Long Term Output Caching
   */
  bool                         _ltOn;        // Do store entropy in call to update
  vector<float>                _ltEntropies; // Store entropy in call to update

  class progressNotification
  {
  public:
    progressNotification(int32 totCorr,
			 rcProgressIndicator* guiUpdate) :
    _totCorr(totCorr), _curCorr(0), _guiUpdate(guiUpdate), _abort(false)
    { 
      // The following is a hack to simulate the existing effect.
      if (guiUpdate) {
          _abort = _guiUpdate->progress(uint32(0));
	_pctUpdate = 1;
      }
      else {
	fprintf(stderr, "  0 pct done\n");
	_pctUpdate = 5;
      }
      _nextPct = _pctUpdate;
    }

    /* Return true if analysis should be aborted, false otherwise.
     */
    bool update()
    {
      if (_abort)
	return true;
      _curCorr++;
      if (_nextPct <= ((_curCorr*100)/_totCorr)) {
	if (_guiUpdate) {
	  bool abort = _guiUpdate->progress((uint32)_nextPct);
	  _nextPct += _pctUpdate;
	  return abort;
	}
	fprintf(stderr, "% 3d pct done\n", _nextPct);
	_nextPct += _pctUpdate;
      }
      return false;
    }

  private:
    int32 _totCorr, _curCorr, _nextPct, _pctUpdate;
    rcProgressIndicator* _guiUpdate;
    bool _abort;
  };
};


typedef boost::shared_ptr<self_similarity_producer> self_similarity_producerRef;

void rf1DdistanceHistogram (const vector<double>& signal, vector<double>& dHist);

#endif /* __RC_SIMILARITY_H */
