/*
 *  rc_moments.h
 *
 *  Created by Peter Roberts on Tue Jun 03 2003.
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 * Contains class and function definitions required to perform
 * correlations using integral images.
 */
#ifndef _RC_MOMENTS_H_
#define _RC_MOMENTS_H_



#include <rc_types.h>
#include <rc_rect.h>
#include <rc_window.h>
#include <rc_analysis.h>

#define VALID_1D_MOM_INT_WIDTH(IMG_WIDTH) ((IMG_WIDTH)*2)
#define VALID_1D_MOM_INT_HEIGHT(IMG_HEIGHT) ((IMG_HEIGHT) + 2)

#define VALID_2D_MOM_INT_WIDTH(IMG_WIDTH) (((IMG_WIDTH) + 1)*3)
#define VALID_2D_MOM_INT_HEIGHT(IMG_HEIGHT) ((IMG_HEIGHT) + 2)

#define VALID_2D_MOM_INT_FAST_WIDTH(IMG_WIDTH) (((IMG_WIDTH) + 1)*2)
#define VALID_2D_MOM_INT_FAST_HEIGHT(IMG_HEIGHT) ((IMG_HEIGHT) + 2)

#define VALID_2D_IM_INT_WIDTH(IMG_WIDTH) ((IMG_WIDTH) + 1)
#define VALID_2D_IM_INT_HEIGHT(IMG_HEIGHT) ((IMG_HEIGHT) + 2)

/* src must be 8 bit image, integral must be 32 bit image.
 * integral width must be src width times 2.
 * integral height must be src height plus 2.
 */
void rfGen1DMomentIntegrals(rcWindow src, rcWindow integral);

/* src must be 8 bit image, integral must be 32 bit image.
 * integral width must be src width plus 1 times 3.
 * integral height must be src height plus 2.
 */
void rfGen2DMomentIntegrals(rcWindow src, rcWindow integral);

/* src must be 8 bit image, integral must be 32 bit image.  integral
 * width must be src width plus 1 times 2.  integral height must be
 * src height plus 2.
 *
 * Note that the calculated integral image is only guaranteed to be
 * valid for subwindows <= 65536.
 */
void rfGen2DMomentIntegralsFast(rcWindow src, rcWindow integral);

/* image and model must both be 8 bit images, integral must be 64 bit
 * image.  intergral is generated for an image the size of the
 * "intersection" of image and model. (In other words, its
 * width/height is the minimum of the two source images'
 * width/height).
 * integral width must be "intersection" width plus 1.
 * integral height must be "intersection" height plus 2.
 */
void rfGen2DIMIntegrals(rcWindow image, rcWindow model, rcWindow integral);

/* image and model must both be 8 bit images, integral must be 32 bit
 * image.  intergral is generated for an image the size of the
 * "intersection" of image and model. (In other words, its
 * width/height is the minimum of the two source images'
 * width/height).
 * integral width must be "intersection" width plus 1.
 * integral height must be "intersection" height plus 2.
 *
 * Note that the calculated integral image is only guaranteed to be
 * valid for subwindows <= 65536.
 */
void rfGen2DIMIntegralsFast(rcWindow image, rcWindow model, rcWindow integral);

/* 3 by 3 2D array of correlation space scores. The inner/outer
 * indices specify vertical/horizontal locations in the image
 * coordinate system, respectively. In other words, score[0][2] is the
 * upper-right correlation score and score[1][0] is the middle-left
 * correlation score.
 */
typedef struct rsCorr3by3Point {
  float score[3][3];
} rsCorr3by3Point;

/* 5 by 5 2D array of correlation space scores. Used by optimized
 * autocorrelation space generating functions to return result
 * space. The inner/outer indices specify vertical/horizontal
 * locations in the image coordinate system, respectively. In other
 * words, score[0][4] is the upper-right correlation score and
 * score[2][0] is the middle-left correlation score.
 */
typedef struct rsCorr5by5Point {
  float score[5][5];
} rsCorr5by5Point;

/* 3 by 3 2D array of pointers to arrays of correlation space
 * scores. The inner/outer indices specify vertical/horizontal
 * locations in the image coordinate system, respectively. In other
 * words, score[0][2] is the upper-right correlation score array and
 * score[1][0] is the middle-left correlation score array.
 *
 * Note that the pointers returned are to data structures internal to
 * the score generating fct. From call to call to the score generating
 * function the underlying data will change. Clients must copy these
 * results to local buffers if they want to retain correlation scores
 * over calls to the score generating function.
 */
typedef struct rsCorr3by3Line {
  const float* score[3][3];
  uint32 count; // Number of correlation spaces available.
} rsCorr3by3Line;

/* 5 by 5 2D array of pointers to arrays of correlation space
 * scores. The inner/outer indices specify vertical/horizontal
 * locations in the image coordinate system, respectively. In other
 * words, score[0][4] is the upper-right correlation score array and
 * score[2][0] is the middle-left correlation score array.
 *
 * Note that the pointers returned are to data structures internal to
 * the score generating fct. From call to call to the score generating
 * function the underlying data will change. Clients must copy these
 * results to local buffers if they want to retain correlation scores
 * over calls to the score generating function.
 */
typedef struct rsCorr5by5Line {
  const float* score[5][5];
  uint32 count; // Number of correlation spaces available.
} rsCorr5by5Line;

/* rcMomentGenerator - Calculates pixel sum and sum square statistics
 * for a given rectangle within a frame.  Class is optimized to
 * support at high speed by reading precalculated values from a
 * special "integral" image. Depending upon how the moment generator
 * is constructed, this will be either a "1D" or "2D" integral image.
 *
 * A 1D integral image is defined as follows: integral(X,Y) contains
 * the sum/sum square of all the pixels(x,y) in the source buffer such
 * that, x == X and y <= Y.
 *
 * A 2D integral image is defined as follows: integral(X,Y) contains
 * the sum/sum square of all the pixels(x,y) in the source buffer such
 * that, x <= X and y <= Y.
 */
class rcMomentGenerator
{
  typedef void (rcMomentGenerator::*momGenFct)(const rcRect& loc, float& sum,
					       float& sumSq) const;

  typedef void (rcMomentGenerator::*projectFct)(const rcRect& loc,
						rcWindow& project) const;
 public:

  enum integralType {
    eMoment1D,
    eMoment2D,
    eMoment2DFast
  };

  rcMomentGenerator(integralType type = eMoment2D);
  virtual ~rcMomentGenerator();

  /* updateFrame - Based upon frame, generate a new integral image.
   */
  void update(const rcWindow& frame);

  /* genSumSumSq - Generate sum and sum square statistics for the last
   * buffer passed to update.
   *
   * Note: update() must be called at least once before this fct gets
   * called.
   *
   * Note: if the integral type is eMoment2DFast, then
   * loc.width()*loc.height() must be <= 65536.
   */
  void genSumSumSq(const rcRect& loc, float& sum, float& sumSq) const
  { (this->*_momGen)(loc, sum, sumSq); }

  /* vProject - Calculate the 32 bit projection of the given rectangle
   * for the last buffer passed to update.
   *
   * project must be a 32 bit wide image that is loc.width() by 1, in
   * size.
   *
   * Note: update() must be called at least once before this fct gets
   * called.
   *
   * Note: if the integral type is eMoment2DFast, then
   * loc.width()*loc.height() must be <= 65536.
   */
  void vProject(const rcRect& loc, rcWindow& project) const
  { (this->*_vProject)(loc, project); }

  /* hProject - Calculate the 32 bit projection of the given rectangle
   * for the last buffer passed to update.
   *
   * ONLY WORKS FOR TYPES eMoment2D and eMoment2DFast!
   *
   * project must be a 32 bit wide image that is loc.width() by 1, in
   * size.
   *
   * Note: update() must be called at least once before this fct gets
   * called.
   *
   * Note: if the integral type is eMoment2DFast, then
   * loc.width()*loc.height() must be <= 65536.
   */
  void hProject(const rcRect& loc, rcWindow& project) const
  { (this->*_hProject)(loc, project); }

 private:
  friend class rcAutoCorrelation;
  friend class rcPointCorrelation;

  uint32* imgSumPtr(const int32 x, const int32 y) const
  { return _imgSqBase + y*_imgSqRowUpdate + x; }

  int64* imgSumSqPtr(const int32 x, const int32 y) const
  { return (int64*)(_imgSqBase + y*_imgSqRowUpdate + x); }

  uint32* imgSumSqPtrLow(const int32 x, const int32 y) const
  { return (uint32*)(_imgSqBase + y*_imgSqRowUpdate + x + 1); }

  uint32* imgSumSqPtrFast(const int32 x, const int32 y) const
  { return (uint32*)(_imgSqBase + y*_imgSqRowUpdate + x); }

  void genSumSumSq1D(const rcRect& loc, float& sum, float& sumSq) const;
  void genSumSumSq2D(const rcRect& loc, float& sum, float& sumSq) const;
  void genSumSumSq2DFast(const rcRect& loc, float& sum, float& sumSq) const;

  void vProject1D(const rcRect& loc, rcWindow& project) const;
  void vProject2D(const rcRect& loc, rcWindow& project) const;
  void vProject2DFast(const rcRect& loc, rcWindow& project) const;

  void hProject1D(const rcRect& loc, rcWindow& project) const;
  void hProject2D(const rcRect& loc, rcWindow& project) const;
  void hProject2DFast(const rcRect& loc, rcWindow& project) const;

  const integralType _type;
  const momGenFct    _momGen;
  const projectFct   _vProject, _hProject;
  rcWindow           _imgSq;
  uint32*          _imgSqBase;
  int32            _imgSqRowUpdate;
};

/* rcAutoCorrelation - Calculates autocorrelation spaces.  Class is
 * optimized for performing high speed calculations in certain cases
 * by reading precalculated values from a set of integral images.
 * Current implementation is optimized for 3x3 and 5x5 cases.
 */
class rcAutoCorrelation
{
  typedef void (*gen2DIntegralsFct)(rcWindow image, rcWindow model,
				    rcWindow integral);

 public:

  enum resultSpace {
    eCross, // Generate integrals for horizontal/vertical locations only
    eFull   // Generate integrals for all locations
  };

  /* ctor - If fast is true, setup to support fast autocorrelation of
   * small "models" (<= 256 pixels), otherwise setup for support of
   * slower, but arbitrary size "models".
   */
  rcAutoCorrelation();

  virtual ~rcAutoCorrelation();
		 
  /* momGen - Allow clients direct access to moment generating class
   * for purposes of generating sum and sum square information.
   */
  const rcMomentGenerator& momGen() const
  { return _momSS; }
	
  /* gen3by3Point - Perform fast autocorrelation for the "model" at
   * the center of the given search space and return the result in
   * res. Correlation score will be generated using integral images
   * created by call to update.
   *
   * See rsCorr3by3Point for a description of the result structure.
   * The size of the model the operation is being performed upon
   * always has dimensions srchSpace.width()-2 by
   * srchSpace.height()-2.
   *
   * Note: if the object was created with fast == true, then the model
   * size must be <= 256.
   *
   * The number of correlations performed is determined by the
   * arguments passed to update(). See description of update()
   * for details.
   *
   * minVarTimesSz is a measure of variance. To be exact, it is
   * VARIANCE*N*(N-1), where N is the number of pixels in the
   * model. This choice was made to avoid the cost of performing the
   * division required to calculate the true variance.
   *
   * Since the correlation of a model with itself is always 1, the
   * calculated value of the variance measure for the model is stored
   * at the center location of the result structure (score[1][1]).
   * When this value is less than minVarTimesSz, none of the
   * correlation scores are calculated and the values stored in the
   * rest of the result structure are all undefined. Setting
   * minVarTimeSz to -1.0 guarantees that the correlation values are
   * always returned.
   *
   * Note: update() must be called at least once before this fct gets
   * called. The search space must refer to an rcRect that fits within
   * the last rcWindow passed to update() and be at minimum, 3 by 3.
   */
  void gen3by3Point(const rcRect& srchSpace, rsCorr3by3Point& res, resultSpace space = eFull) const;

  /* gen5by5Point - 5 by 5 equivalent of gen3by3Point(). Same
   * description as above, except for the following differences.
   *
   * The result is stored in an rsCorr5by5Point structure.  See
   * rsCorr5by5Point for a description of the result structure.  The
   * size of the model the operation is being performed upon always
   * has dimensions srchSpace.width()-4 by srchSpace.height()-4.
   *
   * The center location is at res.score[2][2].
   *
   * On preceeding call to update() acRadius argument must have been
   * >= 2. The search space must refer to an rcRect that is at
   * minimum, 5 by 5.
   */
  void gen5by5Point(const rcRect& srchSpace, rsCorr5by5Point& res,  resultSpace space = eFull) const;


  /* genPoint - Perform autocorrelation for the "model" at the center
   * of the given search space and return the result in res. Does not
   * depend upon integral images to generate results, but will use any
   * existing integral images to speed up processing.
   *
   * The result is a 2D vector of correlation scores. The inner/outer
   * indices specify vertical/horizontal locations in the image
   * coordinate system, respectively. In other words, assuming a 5 by
   * 5 result was requested, res[0][4] is the far upper-far right
   * correlation score array and res[2][0] is the middle-far left
   * correlation score array.
   *
   * acRadius determines the size of the resulting autocorrelation
   * space. It will be N x N where N = (1 + acRadius*2). res
   * must be at least large enough to hold this.
   *
   * The size of the model the operation is being performed upon
   * always has dimensions srchSpace.width-(acRadius*2) by
   * srchSpace.height()-(acRadius*2). acRadius must be > 0.
   *
   * space determines whether all or only the horizontal/vertical
   * results will be generated.
   *
   * The center location is at res[acRadius][acRadius]. The "variance
   * times size" value is stored here.  See description of
   * gen3by3Point for explanation of both this and the use of
   * minVarTimesSz.
   *
   * Note: update() must be called at least once before this function
   * gets called.  If this function is to be used to generate spaces
   * for which the original source frame will be required (stored
   * integral information is insufficient) update must have been
   * called with keepFrame == true.
   */
	void genPoint(const rcRect& srchSpace,	 vector<vector<rcCorr> >& res, const int32 acRadius,  resultSpace space ) const;
	
  /* update - Based upon frame, generate a new set of integral images.
   *
   * space determines whether all or only the horizontal/vertical
   * integral data will be created.
   *
   * style determines whether or not cache space required by line
   * processing functions will be allocated.
   *
   * acRadius determines whether 3 by 3 (if set to 1) or 5 by 5 ( if
   * set > 1) integral data will be created. If acRadius == 0, the
   * function returns immediately after checking keepFrame.
   *
   * If keepFrame is true, a copy of the window referred to by frame
   * is maintained internally. This may be used in case of calls to
   * genPoint().
   */
  void update(const rcWindow& frame, int32 acRadius, resultSpace space = eFull, bool keepFrame = false);

 private:
  void genCache();
  void clearCache();
  bool repInvariant();
  const rcPixel _updatePixelDepth;
	

  void genSumVarXSzData(uint32* startBL, uint32* startTL,
			uint32* startBR, uint32* startTR,
			int32 count, float* sizeP,
			float* destSum, float* destVarTimesSz);

  void genNorthScores(uint32* northBL, uint32* northTL,
		      uint32* northBR, uint32* northTR,
		      float* sumNorthP, float* varXSzNorthP,
		      float* sumMdlP, float* varXSzMdlP,
		      int32 count, float* sizeP,
		      float* northScore);

  void genSouthWestScores(uint32* southBL, uint32* southTL,
			  uint32* southBR, uint32* southTR,
			  uint32* westBL, uint32* westTL,
			  uint32* westBR, uint32* westTR,
			  float* sumWestMdlP, float* varXSzWestMdlP,
			  float* sumSouthP, float* varXSzSouthP,
			  int32 count, float* sizeP,
			  int32 offset,
			  float* bottomScore, float* leftScore);

  void genEastWestScores(uint32* eastBL, uint32* eastTL,
			 uint32* eastBR, uint32* eastTR,
			 uint32* westBL, uint32* westTL,
			 uint32* westBR, uint32* westTR,
			 float* sumImgP, float* varXSzImgP,
			 float* sumMdlP, float* varXSzMdlP,
			 int32 count, float* sizeP,
			 int32 offset, 
			 float* eastScore, float* westScore);

 void genSumVarXSzDataFast(uint32* startBL, uint32* startTL,
			   uint32* startBR, uint32* startTR,
			   int32 count, uint16* sizeP,
			   uint32* destSum,
			   float* destVarTimesSz);
 
  void genBottomLeftScoresFast(uint32* sbBL, uint32* sbTL,
			       uint32* sbBR, uint32* sbTR,
			       uint32* slBL, uint32* slTL,
			       uint32* slBR, uint32* slTR,
			       uint32* sumCtrP, float* varXSzCtrP,
			       uint32* sumBtmP, float* varXSzBtmP,
			       int32 count, uint16* sizeP,
			       float* bottomScore, float* leftScore);

  int64* nwIMSumPtr(const int32 x, const int32 y) const;
  int64* neIMSumPtr(const int32 x, const int32 y) const;
  int64* nIMSumPtr(const int32 x, const int32 y) const;
  int64* wIMSumPtr(const int32 x, const int32 y) const;
  int64* fnwIMSumPtr(const int32 x, const int32 y) const;
  int64* fneIMSumPtr(const int32 x, const int32 y) const;
  int64* fnIMSumPtr(const int32 x, const int32 y) const;
  int64* fwIMSumPtr(const int32 x, const int32 y) const;
  int64* nnwIMSumPtr(const int32 x, const int32 y) const;
  int64* wnwIMSumPtr(const int32 x, const int32 y) const;
  int64* wswIMSumPtr(const int32 x, const int32 y) const;
  int64* sswIMSumPtr(const int32 x, const int32 y) const;

  uint32* nwIMSumPtr32(const int32 x, const int32 y) const;
  uint32* neIMSumPtr32(const int32 x, const int32 y) const;
  uint32* nIMSumPtr32(const int32 x, const int32 y) const;
  uint32* wIMSumPtr32(const int32 x, const int32 y) const;
  uint32* fnwIMSumPtr32(const int32 x, const int32 y) const;
  uint32* fneIMSumPtr32(const int32 x, const int32 y) const;
  uint32* fnIMSumPtr32(const int32 x, const int32 y) const;
  uint32* fwIMSumPtr32(const int32 x, const int32 y) const;
  uint32* nnwIMSumPtr32(const int32 x, const int32 y) const;
  uint32* wnwIMSumPtr32(const int32 x, const int32 y) const;
  uint32* wswIMSumPtr32(const int32 x, const int32 y) const;
  uint32* sswIMSumPtr32(const int32 x, const int32 y) const;

  uint32* nwIM32SumPtr(const int32 x, const int32 y) const;
  uint32* neIM32SumPtr(const int32 x, const int32 y) const;
  uint32* nIM32SumPtr(const int32 x, const int32 y) const;
  uint32* wIM32SumPtr(const int32 x, const int32 y) const;
  uint32* fnwIM32SumPtr(const int32 x, const int32 y) const;
  uint32* fneIM32SumPtr(const int32 x, const int32 y) const;
  uint32* fnIM32SumPtr(const int32 x, const int32 y) const;
  uint32* fwIM32SumPtr(const int32 x, const int32 y) const;
  uint32* nnwIM32SumPtr(const int32 x, const int32 y) const;
  uint32* wnwIM32SumPtr(const int32 x, const int32 y) const;
  uint32* wswIM32SumPtr(const int32 x, const int32 y) const;
  uint32* sswIM32SumPtr(const int32 x, const int32 y) const;

  rcMomentGenerator _momSS;
  rcWindow _updateFrame;

  /* IM integral data for 3x3 case.
   */
  rcWindow _nw, _ne, _w, _n;
  int64 *_nwBase, *_neBase, *_wBase, *_nBase;
  int32 _nwRowUpdate, _neRowUpdate, _wRowUpdate, _nRowUpdate;
  
  /* Additional IM integral data for 5x5 case.
   */
  rcWindow _fnw, _fne, _fw, _fn;
  rcWindow _nnw, _wnw, _wsw, _ssw;
  int64 *_fnwBase, *_fneBase, *_fwBase, *_fnBase;
  int64 *_nnwBase, *_wnwBase, *_wswBase, *_sswBase;
  int32 _fnwRowUpdate, _fneRowUpdate, _fwRowUpdate, _fnRowUpdate;
  int32 _nnwRowUpdate, _wnwRowUpdate, _wswRowUpdate, _sswRowUpdate;

  /* Caching space for for final and intermediate results of a 3x3
   * correlation.
   */
  char* _baseP;    // Ptr to unaligned start of data allocated for use in below
  float* _size;    // Where pixel count is stored
  float* _sumP[3]; // Integrated sums of last two 2 lines
  float* _vxsP[3]; // "Variance times size" (N*sumII - sumI*sumI) of last 2 lines
  float* _wP;      // W and E correlation scores
  float* _nP[2];   // N and S correlation scores
  float* _nwP[2];  // NW and SE correlation scores
  float* _neP[2];  // NE and SW correlation scores

  /* Additional caching space for for final and intermediate results
   * of a 5x5 correlation.
   */
  float* _fwP;       // Far W and Far E correlation scores
  float* _wnwP[2];   // WNW and ESE correlation scores
  float* _wswP[2];   // WSW and ENE correlation scores
  float* _fnP[3];    // Far N and Far S correlation scores
  float* _fnwP[3];   // Far NW and Far SE correlation scores
  float* _fneP[3];   // Far NE and Far SW correlation scores
  float* _nnwP[3];   // NNW and SSE correlation scores
  float* _sswP[3];   // SSW and NNE correlation scores

  bool    _crossOnly;
  int32 _radius;
  int32 _topLine;
  rcIPair _srchSize;
  int32 _width;
  int32 _allocWidth;

  int sumVarXSzPrefetch;
  int btmLeftPrefetch1, btmLeftPrefetch2;
};

// Effect: horizontal and vertical image integrals
// Requires: row and column integrals to be same size as image
void rfImage8Integrals (const rcWindow& image, rcWindow& rproj, rcWindow& cproj);

void rfCorr5by5PointMomentFit (rsCorr5by5Point& s, float& x, float&y);
void rfCorr5by5LineMomentFit (rsCorr5by5Line& s, float& x, float&y);

#endif
