/* @file
 *  rc_point_corr.h
 *
 *  Created by Peter Roberts on Tue Jun 23 2003.
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 * Contains class and function definitions required to perform
 * correlations using integral images.
 */
#ifndef _rcPOINT_CORR_H_
#define _rcPOINT_CORR_H_

#include "rc_pair.h"

#include "rc_moments.h>

/* rcPointCorrelation - Calculates correlation space for a model.  Class is
 * optimized for performing high speed calculations in certain cases
 * by by both reading precalculated values from a set of integral
 * images and performing a high speed calculation of the IM values for
 * each location of the model within the search space.
 *
 * Current implementation is optimized for 3x3 and 5x5 cases where the
 * search space dimensions are <= 16 pixels in both dimensions.
 */
class rcPointCorrelation
{
 public:
  rcPointCorrelation();
  virtual ~rcPointCorrelation();

  /* gen3by3Space - Fast generation of a 3x3 correlation space.
   *
   * modelOrigin specifies the origin of the model, in pixels,
   * relative to the mdlWin specified in the last call to update().
   *
   * srchSpace sepcifies a rectangle, in pixels, relative to the
   * imgWin specified in the last call to update().
   *
   * Correlation scores will be generated using integral images
   * specified by the last call to update.
   *
   * See rsCorr3by3Point (in rc_moments.h) for a description of the
   * result structure.  The size of the model the operation is being
   * performed upon always has dimensions srchSpace.width()-2 by
   * srchSpace.height()-2.
   *
   * Unlike rcAutoCorrelation, the full correlation space is always
   * generated.
   *
   * Note: the "moments" version of update() must be called at least
   * once before any calls to this fct. If the other veriosn of
   * update() gets called, the "moments" version must, once again, be
   * called at least once before any calls to this fct. The search
   * space must refer to an rcRect that fits within the last imgWin
   * passed to update() and be at minimum, 3 by 3.  Unlike
   * rcAutoCorrelation, there is an additional restriction on the
   * search space size that it must be less than 16 by 16.
   * 
   */
  void gen3by3Space(const rcRect& srchSpace, const rcIPair& modelOrigin, 
		    rsCorr3by3Point& res) const;

  /* gen5by5Space - Fast generation of a 5x5 correlation space.
   *
   * modelOrigin specifies the origin of the model, in pixels,
   * relative to the mdlWin specified in the last call to update().
   *
   * srchSpace sepcifies a rectangle, in pixels, relative to the
   * imgWin specified in the last call to update().
   *
   * See rsCorr5by5Point (in rc_moments.h) for a description of the
   * result structure.  The size of the model the operation is being
   * performed upon always has dimensions srchSpace.width()-4 by
   * srchSpace.height()-4.
   *
   * Unlike rcAutoCorrelation, the full correlation space is always
   * generated.
   *
   * The fct will use the moment data specified by the last call to
   * update, to generate SumI, SumII, SumM, and SumMM values as well
   * as using the images specified by the last call to update to
   * calculate SumIM values required to calculate correlation scores.
   *
   * Note: the "moments" version of update() must be called at least
   * once before any calls to this fct. If the other veriosn of
   * update() gets called, the "moments" version must, once again, be
   * called at least once before any calls to this fct. The search
   * space must refer to an rcRect that fits within the last imgWin
   * passed to update() and be at minimum, 5 by 5.  Unlike
   * rcAutoCorrelation, there is an additional restriction on the
   * search space size that it must be less than 16 by 16.
   */
  void gen5by5Space(const rcRect& srchSpace, const rcIPair& modelOrigin,
		    rsCorr5by5Point& res) const;

  /* genSpace - Generate an NxN correlation space, where N = (1 +
   * acRadius*2).
   *
   * modelOrigin specifies the origin of the model, in pixels,
   * relative to the mdlWin specified in the last call to update().
   *
   * srchSpace sepcifies a rectangle, in pixels, relative to the
   * imgWin specified in the last call to update().
   *
   * The size of the model the operation is being performed upon
   * always has dimensions srchSpace.width-(acRadius*2) by
   * srchSpace.height()-(acRadius*2). acRadius must be > 0.
   *
   * Unlike rcAutoCorrelation, the full correlation space is always
   * generated.
   *
   * The fct will use the moment data specified by the last call to
   * update, to generate SumI, SumII, SumM, and SumMM values as well
   * as using the images specified by the last call to update to
   * calculate SumIM values required to calculate correlation scores.
   *
   * Note: update() must be called at least once before this function
   * gets called.  If the "moments" version of update() was the last
   * to be called, the moments data will be used in generating the
   * correlation space. The search space must refer to an rcRect that
   * fits within the last imgWin passed to update() and be at minimum,
   * N by N, where N = (1 + (acRadius*2).
   */
  void genSpace(const rcRect& srchSpace, const rcIPair& modelOrigin,
		vector<vector<float> >& res, const int32 acRadius) const;

  /* update - Store references to the specifed rcWindow's and
   * rcMomentGenerator's for use in subsequent calls to gen3by3Space(),
   * gen5by5Space(), and genSpace().
   *
   * If either imgWin was not the same image used to generate imgMom
   * or mdlWin was not the same image used to generate mdlMom, the
   * results are undefined.
   *
   * @note Within the context of kinetoscope's correspondence of fixed to
   * moving where fixed is before moving in time, "fixed" corresponds to 
   * mdlWin and "moving" correspondes to imgWin. 
   */
  void update(rcWindow imgWin, rcWindow mdlWin,
	      const rcMomentGenerator& imgMom,
	      const rcMomentGenerator& mdlMom);

  /* update - Store references to the specifed rcWindow's for use in
   * subsequent calls to genSpace().
   *
   * Note: Subsequent calls to either gen3by3Space() or gen5by5Space()
   * will result in an assertion. Call the "moments" version of
   * update() to use either of these functions.
   */
  void update(rcWindow imgWin, rcWindow mdlWin);

 private:
  rcWindow _imgWin, _mdlWin;
  unsigned char *_iRowBase, *_mRowBase;
  int32 _iRowUpdate, _mRowUpdate;
  const rcMomentGenerator *_imgMom, *_mdlMom;
  unsigned char *_destSpaceBase;
  uint32* _destSpacePtr;
  uint32* _vectorSumPtr;
  uint32* _vectorSumSqPtr;
  uint32* _resultPtr;
  int32   _destSpaceSz;
};

#endif
