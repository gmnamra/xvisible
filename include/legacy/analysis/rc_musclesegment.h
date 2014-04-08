/****************************************************************************
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 *   File Name      rc_musclesegment.h
 *   Creation Date  01/10/2004
 *   Author         Peter Roberts
 *
 * Muscle Segmentation code
 *
 ***************************************************************************/

#ifndef _RC_MUSCLESEGMENT_H_
#define _RC_MUSCLESEGMENT_H_

#include <rc_types.h>
#include <rc_window.h>
#include <rc_polygon.h>

#include <vector>

class rcGenMovieFile;

class rcMuscleSegmenter {
 public:

  enum errorCode {
    cInvalidPeriod = -1,
    cWindowTooSmall = -2
  };

  enum rejectionCode {
    cRejCodeSize = 0,
    cRejCodeAR,
    cRejCodeEdge,
    cRejCodeActivity,
    cRejCodeCount
  };

  /* ctor -
   *
   * If debugMovie is non-NULL, various intermediate stage images will
   * be added to the specified movie file during calls to segment and
   * the various filtering functions.
   */
  rcMuscleSegmenter(rcGenMovieFile* debugMovie = 0);

  /* extendedMuscleCellIndex - Return index of a frame that is thought
   * to have muscle cells fully extended.
   *
   * Note: This value is calculated as a byproduct of the call to
   * segment().  Until that function is called, this function will
   * return -1.
   */
  int32 extendedMuscleCellIndex() const { return _emi; }

  /* framesPerPeriod - Return the calculated/specified frames per
   * period value.
   *
   * Note: This value is calculated as a byproduct of the call to
   * filterByActivity().  Until that function is called, this function
   * will return -1.
   */
  float framesPerPeriod() const { return _framesPerPeriod; }

  /* segment - Performs two basic functions:
   *
   *   1) Generates spatially segmented image.
   *
   *   2) Creates the initial list of candidate muscle cells.
   *
   *
   * The image is generated using the the frames [startFrame, endFrame)
   * from movie.
   *
   * Note: This function must be called before any filtering or
   * other processing functions can be called.
   *
   * Note: Returns 0 of segmentation step succeeded, one of the
   * above listed error codes otherwise.
   */
  int32 segment(const vector<rcWindow>& movie,
		  int32 startFrame, int32 endFrame, bool fluo = false);

  /* filterBySize - Removes all cells smaller than minArea. minArea is
   * specified in pixels.
   */
  void filterBySize(double minArea);

  /* filterByAR - Removes all cells whose aspect ratio (as determined
   * by the cells minimum enclosing rectangle) as smaller than
   * minAspectRatio.
   */
  void filterByAR(double minAspectRatio);

  /* filterByActivity - Performs two functions:
   *
   *    1) Generates temporally segmented image.
   *
   *    2) Removes all cells whose percent of "active" area (as
   *       determined by the ratio of total cell area to cell area
   *       which is changing at the frames per period rate passed-in/
   *       calculated), are less than minPct.
   * 
   * The image is generated using the the frames [startFrame, endFrame)
   * from movie.
   *
   * If framesPerPeriod is <= 0, then segment attempts to calculate
   * the number of frames per period. In this case maxFramesPerPeriod
   * is used, otherwise this value is ignored.
   *
   * Note: Returns 0 of segmentation step succeeded, one of the
   * above listed error codes otherwise.
   */
  int32 filterByActivity(double minPct, const vector<rcWindow>& movie,
			   int32 startFrame, int32 endFrame,
			   float maxFramesPerPeriod,
			   float framesPerPeriod = -1.0);

  /* filterAtEdge - Remove all cells whose enclosing rectangle is less
   * than minDelta number of pixels from the edge of the movie frames.
   */
  void filterAtEdge(int32 minDelta);

  /* getCells - Returns the list of polygons that correspond to the
   * current set of candidate muscle cells.
   */
  void getCells(vector<rcPolygon>& cells) const;

  /* getRejects - Returns the list of polygons that correspond to the
   * current set of polygons rejected by the various filter*
   * functions.
   */
  void getRejects(vector<rcPolygon>& rejects) const;

  /* rejectRange - Returns a pair if indices [startIndex, endIndex)
   * that specify which elements in the vector returned by
   * getRejects() correspond to the specified reason code. If a filter
   * is used multiple times, there will be multiple reject ranges. The
   * instance argument controls which range to return.
   *
   * Note: If there were no rejections for the specified reason, then
   * startIndex == endIndex.
   *
   * Note: Only those polygons rejected by filtering functions are
   * placed here.
   */
  void rejectRange(int32& startIndex, int32& endIndex,
		   rejectionCode reason, int32 instance = 1) const;

  /* Development only hack - ditch
   */
  void hackInsertSegmentedImages(rcWindow& segSdImg, rcWindow& segFreqImg,
				 rcWindow& emiImg);

 private:

  /* getEMI - Determine a frame with extended muscle cells.
   *
   * Returns: Index to frame.
   */
  int32 getEMI(const vector<rcWindow>& movie,
		 int32 startFrame, int32 endFrame);

  typedef struct polyInterInfo {
    double area;
    vector<rcPolygon> polyInter;
  } polyInterInfo;

  /* genPolygons - Generate a vector of polygons from the connected
   * image passed in.
   */
  void genPolygons(const rcWindow& segImg, const int32 minDim,
		   vector<rcPolygon>& p, const rcIPair delta) const;

  void mergePolygons(vector<rcPolygon>& poly, vector<rcPolygon>& polyCH,
		     vector<rcPolygon>& mergedCH, vector<rcIRect>& mergedCHR,
		     vector<polyInterInfo>& mergedInfo) const;

  void accumPolygons(vector<rcPolygon>& fg, vector<rcIRect>& fgR,
		     vector<rcPolygon>& freq, vector<rcIRect>& freqR,
		     vector<polyInterInfo>& accumInfo) const;

  void genSpatialSegmentation(const vector<rcWindow>& movie,
			      int32 startFrame, int32 endFrame,
			      int32& emi, rcWindow& segSdImg);
    
  int32 genFrequencySegmentation(const vector<rcWindow>& movie,
				   int32 startFrame, int32 endFrame,
				   int32 emi, float maxFramesPerPeriod,
				   float framesPerPeriod, rcWindow& segFreqImg);

  void debugWrite(const rcWindow& frame) const;

  rcWindow        _segSdImg, _segFreqImg, _emiImg; // Development only hack - ditch

  rcWindow        _segSdImgDbg, _emiImgDbg;
  rcGenMovieFile* _debugMovie;
  rcIPair         _dSize;

  int32               _emi;
  float                 _framesPerPeriod;
  vector<rcPolygon>     _cells, _activeRegions, _rejects;
  vector<rcIRect>       _cellsR;
  vector<polyInterInfo> _cellsInfo;
  vector<int32>       _rejectIndex;
  vector<rejectionCode> _rejectReason;
  rcIPair               _segSdImgSz;
};

#endif
