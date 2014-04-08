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

#ifndef _RC_MUSCLETRACKER_H_
#define _RC_MUSCLETRACKER_H_

#include <rc_types.h>
#include <rc_window.h>

class rcMuscleTracker {
 public:
  /* ctor - Create muscle tracker.
   *
   * smoothingRadius defines the amount of gaussian smoothing to
   * perform in the vertical direction when generating edge
   * points. This value is specified in pixels. The smoothing kernel
   * will be 1 X (2*smoothingRadius + 1) in size. Specifying a 0 means
   * no smoothing is performed.
   *
   * Design Note: This argument is set here so that subsequent calls
   * to calcualteCellLocation() can use this to deciding how to make
   * models/search windows that will fit within the movie images. For
   * maximum flexibility, this value could be passed as an argument to
   * calculateCellLcation(), but that didn't seem right.
   */
  rcMuscleTracker(uint32 smoothingRadius);

  /* setEdgeFilter - Create the edge filter to be used by
   * calculateCellLocation(). The filter will be of the form:
   * { -1, -1, . . ., 0, 0, . . ., 1, 1, . . .}
   *
   * where there are oneCnt number of -1's and 1's, and zeroCnt
   * number of 0's.
   *
   * oneCnt must be >= 1. The default filter is:
   * { -1, -1, -1, -1, 1, 1, 1, 1}
   */
  void setEdgeFilter(uint32 oneCnt, uint32 zeroCnt);

  /* setEdgeStrength - Sets the minimum relative edge strength
   * required for an edge to be accepted as "strong". Used by
   * calculateCellLocation().
   *
   * The default edge stength is 4.0.
   */
  void setEdgeStrength(double stdDev);

  /* setEdgePersistence - Sets the minimum percentage of frames a
   * "strong" edge must exist for it to be considered "persistent".
   * Used by calculateCellLocation().
   *
   * The default edge persistence is .6 (60 pct).
   */
  void setEdgePersistence(double percent);

  typedef void (*mtDebug)(rcWindow& vProj, rcWindow& hProj);

  /* calculateCellLocation - Calculate the location and sizes of the
   * left and right cell edge models and search windows.
   *
   * Returns true if cell boundaries were found, false otherwise.
   */
  bool calculateCellLocation(vector<rcWindow>& images,
			     mtDebug showDiags,
                             rcProgressIndicator* progressIndicator = 0 );

  /* genEdgePoints - Calculate the edge locations of the cell for the
   * given image.
   *
   * The x component of the return value contains the left edge and
   * the y component contains the right edge location.
   *
   * When either lCorrSpace or rCorrSpace is nonNull, it will be
   * filled with a compressed-in-y-dimension version of the
   * correlation space calculated as a part of processing. Its
   * intended to be used to display a graph of the correlation space
   * on the monitor.
   *
   * Note: calculateCellLocation() must be called before this function
   * gets called.
   */
  rcDPair genEdgePoints(rcWindow image,	vector<double>* lCorrSpace,
			vector<double>* rCorrSpace);

  /* Accessors to quantities generated by calculateCellLocation.
   * Intended for use in debugging displays.
   */
  rcRect leftModel() const { return _leftModel; }
  rcRect rightModel() const { return _rightModel; }
  rcRect leftSpace() const { return _leftSpace; }
  rcRect rightSpace() const { return _rightSpace; }
  rcIPair leftOffset() const { return _leftOffset; }
  rcIPair rightOffset() const { return _rightOffset; }
  const vector<uint32>& vCandPosDist() const { return _vCandPosDist; }
  const vector<uint32>& vCandNegDist() const { return _vCandNegDist; }
  const vector<uint32>& hCandPosDist() const { return _hCandPosDist; }
  const vector<uint32>& hCandNegDist() const { return _hCandNegDist; }

 private:

  void genSpace(rcWindow& srchWin, rcWindow& model,
		vector<vector<double> >& space,
		uint32& peakOffsetX, uint32& peakOffsetY);

  rcRect                  _leftModel, _rightModel;
  rcRect                  _leftSpace, _rightSpace;
  rcIPair                 _leftOffset, _rightOffset;
  vector<uint32>        _vCandPosDist, _vCandNegDist;
  vector<uint32>        _hCandPosDist, _hCandNegDist;

  uint32                _leftFrameIndex, _rightFrameIndex;
  rcWindow                _lMdlWin, _rMdlWin;
  rcWindow                _lSrchWin, _rSrchWin;
  vector<vector<double> > _lResSpace, _rResSpace;
  double                  _rightToLeftOffset;

  vector <int32>        _filter;
  uint32                _filterOffset;
  double                  _stdDev;
  double                  _persistPct;
  vector<uint16>        _smoothing;
  uint32                _smoothingNorm;
};

#endif