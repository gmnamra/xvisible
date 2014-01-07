/*
 *  @file
 *  
 *
 *	$Id: rc_visualcell.h 7297 2011-03-07 00:17:55Z arman $
 *
 *  
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */


#ifndef __RC_CELL_H
#define __RC_CELL_H


#include <rc_types.h>
#include <rc_rectangle.h>
#include <rc_window.h>
#include <rc_motionpath.h>
#include <rc_line.h>
#include <rc_vector2d.h>
#include <rc_similarity.h>
#include <rc_shape.h>
#include <rc_line.h>
#include <rc_macro.h>
#include <rc_mathmodel.h>
#include <rc_exception.h>

#include <vector>
#include <bitset>

// Note that default contructed xform is an identity transform

/*
 * @class rcVisualFunction rc_visualfunction.h "analysis/include/rc_visualfunction.h"
 * @brief
 * A VisualFunction is created from a collection of locomotive 
 * motion vectors. 
 * A VisualFunction is created by a region label passed by 
 * connectivity analysis done on motion vectors. 
 * The label image is then processed by a RLE processors that
 * returns the set of runs that 
 */

class rcVisualFunction;

bool rfCellNotViable (const rcVisualFunction&);

class rcVisualFunction : public rcBaseMotionPath
{			       
  friend class rcKinetoscope;
  static const float gInValidOrigin =rcFLT_MIN;
  static const float gInValidDispersion =rcFLT_MIN;
  static const float gNotAvailable = -1.0f;
  static const float gMinAdaptBias = 2.0f;
  static const float gEstSideShortening = 0.08f; // 4 percent
  static const int32 gAffineExtend = 5;
  static const float gGoodFit = 500.0f; // 0.5 * 1000 
  rcDPair gNotAvailablePair;
  static const double gInValidAcceleration =rcDBL_MIN;
  static const uint32 gPreHistoric = rcUINT32_MAX;

 public:

  rcVisualFunction ();

  rcVisualFunction (rcKinetoscope* kin, const rc2Fvector& anchor,
		    const rcPolygon& poly, bool isChild = false);
  rcVisualFunction (rcKinetoscope* kin, const rcPolygon& poly, const vector<rc2Fvector>& ends, const float rad);

  /*! 
    @function Constructor
    @discussion :Return date of birth relative to kinetoscope start
    @discussion : Attaches this visualFunction to this kinetoscope
    @discussion : Registers this visual function in time. This time is a 
    @discussion : relative time from the start of the kinestosope
    @discussion : minIntersecting and minEnclosng rectangles are initialized
    @discussion : to the Kinetoscopes frame
    @discussion : Anchor positions are with respect to the kintoscope.
    @discussion : Default assignment, copy constructor, destructor are OK
    @discussion : Second form is for construction from user input for cardiomyocytes

   */


  virtual ~rcVisualFunction () {}

  // Compiler Generated assignment and copy are ok (for now)
 /*  rcVisualFunction (const rcVisualFunction& rhs); */
/*   rcVisualFunction& operator= (const rcVisualFunction& rhs); */

  //@enum Haves are reset at the beginning of each measure cycle
  //
  enum Haves
  {
    eMotion = 0,
    eCorrespondence,
    eScale,
    eRLEs,
    eDispersion,
    eDimensions,
    eTemporal,
    eVelocity,
    eSpeed,
    eAcceleration,
    ePersistence,
    eBeatInfo,
    ePacingInfo,
    eCoarseSS,
    ePeriodicity,
    eGeometry,
    eContraction,
    eDistance,
    eNumHaves 
  };


  bool have (enum Haves what) const;
  bool mark (enum Haves what);
  bool clear (enum Haves what);
  void reset ();


  //@enum States persist between different stages of analysis

  enum State
  {
    eIsInitialized=0,
    eIsUnInitialized,
    eIsUnknown,
    eIsMoving,
    eIsDirected,
    eIsJustDivided,
    eIsGeneralModel,
    eIsDividing,
    eIsDivided,
    eIsFragmented,
    eIsOutSide,
    eIsBeating,
    eIsParent,
    eIsChild,
    eIsRounding,
    eIsRounded,
    eIsPeriodic,
    eHasFullyExtended,
    eHasSimilarator,
    eIsAffineXdirected,
    eNumStates
  };

  friend ostream& operator<< (ostream&, const rcVisualFunction::State&);

  bool isState (const State st) const {return mState[st]; }
  bool isNotState (const State st) const {return !mState[st]; }
  bool setState (const State st) {mState.set(st); return isState (st); }
  bool setState (const State st, const bool b) {mState[st]=b; return isState (st); }
  bool clearState (const State st) {mState.reset (st);return isState (st); }
  State state () const;
  State fineState () const;

  enum DOF
  {
    eMotionCenter,
    eMotionCenterANDTP,
    eNone
  };

  enum parameters
  {
    eMinFramesPerPeriod4Stat = 12,
    eMaxHalfShorteningPct = 10,
    eMinMyoOtherDimensionInPixels = 6
  };

  uint32 id() const {return mId; }
  /*
    effect: returns the id of this cell
  */

  uint32 parentId() const {return mParentId; }
  /*
    effect: returns the id of this cell's parent, if generation () is greater than 0
  */

  uint32 generation () const {return mGeneration; }
  /*
    effect: returns the generation of this cell
  */

  void info (rcOrganismInfo::OrganismType c = rcOrganismInfo::eDefault,
	     rcOrganismInfo::OrganismName n = rcOrganismInfo::eUnknown);

  rcOrganismInfo::OrganismType organism () const
  { return mOtype;}

  rcOrganismInfo::OrganismName organismName () const
  { return mOname;}

  bool isOfOrganismType (rcOrganismInfo::OrganismType ot) const
  { return mOtype == ot; }

  bool isOfOrganismName (rcOrganismInfo::OrganismName onm) const
  { return mOname == onm; }

  bool isOfOrganismTypeAndName (rcOrganismInfo::OrganismType ot, rcOrganismInfo::OrganismName onm) const
  { return (mOtype == ot && mOname == onm); }

  bool isCardiacCell () const
  {
    return (organism() == rcOrganismInfo::eCell && 
	    (organismName() == rcOrganismInfo::eCardioMyocyte || 
	     organismName() == rcOrganismInfo::eCardioMyocyteSelected || 
	     organismName() == rcOrganismInfo::eCardioMyocyteCalcium));
  }

  bool isLabelProtein () const
  {
    return (organism() == rcOrganismInfo::eFluorescenceCluster && 
	    (organismName() == rcOrganismInfo::eProtein || organismName() == rcOrganismInfo::eProteinP53));
  }

    
  // TBD: Units: all in calibarated units 

  // Accessors

  float time () const
  {
    return sharedKin()->elapsedTime();
  }

  /*
   * Time now for this measurement on this cell from the capture start
   * Requires a valid visual Function
   */

  float age () const
  {
    return (time() - dob());
  }
  /*
   * Age: Time from creation of this visual function
   * Requires a valid visual Function
   */

  float area () const
  {
    return mArea;
    //    TODO: map through transformation
  }
  /*
    effect: return area of this well
    Requires initial calculation
  */

  float mass () const
  {
    return mMass;
  }
  /*
    effect: returns sum of raw pixel values inside the area of this cell. 
    Requires initial calculation
  */

  float perimeter () const
  {
    const rc2Xform& xform = sharedKin()->xform();
    return mPerimeter * xform.scale ();
  }
  /*
    effect: return perimeter of this well
    Requires initial calculation
    TODO: map through transformation
  */

  float circularity () const
  {
    return mCircularity;
  }
  /*
    effect: return circularity of this well
    Requires initial calculation
  */

  float ellipseRatio () const
  {
    return mEllipseRatio;
  }

  float rmsQuality () const
  {
    if (mSumQuality.n())
      return mSumQuality.rms ();
    else
      return gNotAvailable;
  }
  /*
    effect: return rms of Quality
    Requires initial calculation
  */

  float distance () const
  {
    const rc2Xform& xform = sharedKin()->xform();
    if (xform.matrix().isSingular())
      return mDistance;
    else
      return mDistance * xform.scale ();
  }
  /*
    effect: returns Distance in pixels
    Requires initial calculation
    TODO: cache initial position
  */

  const rc2Fvector& position () const
  {
    //@todo figure something cleaner about this
    //    if (steps () && !have (eMotion) && !isState(eIsDivided))
    //      rmExceptionMacro(<<"Does not have motion info");
    return mCurrentPosition;
  }

  /*
    effect: Overload of position in the base class
            It adds registration window ul()
    Requires moving cell
  */

  const rc2Fvector velocity () const
  {
    return sharedKin()->xform().mapPoint (mVt);
  }

  /*
    effect: return the horizontal and vertical velocity
    In Pixels per Msec
    Requires whole cell tracking
    TODO: map through transformation
  */

  const rc2Fvector instantVelocity () const
  {
    return sharedKin()->xform().mapPoint (mV);
  }
  /*
    effect: return the horizontal and vertical velocity
    Requires whole cell tracking
    TODO: map through transformation
  */

  const rc2Xform instantTransformation () const
  {
    return mXform;
  }

  /*
    effect: returns instant rigid motion transformation 
    Requires velocity
    TODO: map through transformation
  */

  const rc2Fvector translation () const
  {
    return rc2Fvector ((float) mXform.trans().x(), 
		       (float) mXform.trans().x());
  }

  const rcFPair scale () const
  {
    return rcFPair (100.0f * mXform.matrix().xScale(), 
		    100.0f * mXform.matrix().yScale());
  }

  const float shortenning () const
  {
    return mBeatShortenning;
  }

  const rcFPair dimensions () const
  {
    if (!have (eDimensions))
      rmExceptionMacro (<<"Does not have dimensional information");

    const rc2Xform& xform = sharedKin()->xform();

    if (isCardiacCell ())
      {
	return rcFPair (mAffineSize.x() * xform.scale(), 
			mAffineSize.y() * xform.scale ());
      }
    else
      {
	return rcFPair (mRectSnaps.back().size().x() * xform.scale(), 
			mRectSnaps.back().size().y() * xform.scale());
      }

  }
    
  /*
    effect: .x() is the major dimension, .y() is the minor
    Requires mHaveDimensions and isCardiac
    @note majorEnds are wrt bounding affine rectangle  (image coordinates)
    @note majorEndPoints are wrt frame (image coordinates)
  */

  const rcFPair& majorEnds () const { return mMajorEnds; }
  const vector<rc2Fvector>& majorEndPoints () const { return mMajorEndPoints; }
  const deque<rc2Fvector>& majorEndPoses () const {return mTemplatePoses;}

  /*
    effect: .x() is one end, .y() is the other, on the cardiac axis
    Requires mHaveDimensions and isCardiac
  */


  float speed () const;
  /*
    effect: return speed of the motion center of this cell
    Requires velocity
    TODO: map through transformation
  */

  float acceleration () const
  {
    return mAcceleration;
  }

  /*
    effect: return acceleration of this cell as computed using Self-Similarity
    Note: this is an approximation of acceleration and not the exact
    geometric quantity.
  */


  float persistence () const;
  /*
    effect: returns persistence of this cell.
    Requires all and can happen after 2 rounds
  */

  float framesPerPeriod () const
  {
    if (isState (eIsPeriodic) && mFramesPerPeriod >= 0.0)
      return mFramesPerPeriod;
    else 
    return gNotAvailable;
  }

  int32 integralFramesPerPeriod () const
  {
    if (isState (eIsPeriodic) && mFramesPerPeriod >= 0.0)
      return mIntegralFramesPerPeriod;
    else 
     return (int32) gNotAvailable;
  }

  int32 framesPerPeriodPowerOf2 () const
  {
    if (isState (eIsPeriodic) && mFramesPerPeriod >= 0.0)
      return mFramesPerPeriodPowerOf2;
    else 
      return (int32) gNotAvailable;
  }


  /*
    effect: returns periodicity
    Requires mHavePeriodicity to be True
  */

  float contractionFrequency () const
  {
    if (have (ePacingInfo) && mFramesPerPeriod >= 0.0)
      return mContractionFrequency;
    else 
      return gNotAvailable;
  }

  float pacingFrequency () const
  {
    if (have (ePacingInfo) && mFramesPerPeriod >= 0.0)
      return sharedKin()->averageFramesPerSecond () / framesPerPeriod();
    else 
      return gNotAvailable;
  }

  /*
    effect: returns paced frequency
    Requires mHavePeriodicity to be True
  */
  

  float minSqDisplacement () const
  {
    if (steps())
      return mRdiffuse;

  }
  /*
    effect: return RMS of Displacements
    Requires velocity
  */

  float directedCellPersistence () const
  {
    return mMotionPattern;
  }
  /*
    effect: return RMS of Displacements
    Requires velocity
  */

  const vector<float>& similarity () const
  {
    return mSS;
  }


  int32 label () const
  {
    return mLabel;
  }

  int32 startFrameIndex () const
  {
    return mExpStartFrameIndex;
  }

  int32 dividedFrameIndex () const
  {
    return mExpDividedFrameIndex;
  }

  // Mutators
  void frameIndex (int32 fi)
  {
    rmAssert (fi >= 0);
    mExpStartFrameIndex = fi;
  }

  /*
   * @Desc actual frame index of the videoCache or Movie when this function was created
   *
   */

  int32 label (int32 l) 
  {
    rmAssert (l >= 0);
    mLabel = l;
    mark (eCorrespondence);
    return mLabel;
  }

  /*
    effect: label is the key to motionPathss belonging to
            this cell
    Note: At the moment this is connectivity label 
  */

  float cQuality () const
  { 
    return mCorrespondenceQuality;
  }   

  void  cQuality (const float _arg)
  { 
    if (mCorrespondenceQuality != _arg) 
      { 
	mCorrespondenceQuality = _arg;
      }
  } 

  enum Lines
  {
    eVectors,
    eBoundingBox,
    eBoundingPoly,
    eCurl,
    eRelief
  };

  const int32 history () const {return mSize; }
  /*
    effect: returns archival length. This is equal to the 
    size of all deques returning images,rect, or RLEs
  */

  const deque<rcCorrelationWindow<uint8> >& snaps () const {return mSnaps; } 
  /*
    effect: returns the rcWindow representing this cell
    at this time. The first rcWindow in the vector is the oldest.
    TBD: pointer, reference, or
    TODO: map through transformation
  */

  const deque<rcFRect>& rectSnaps() const {return mRectSnaps; }
  const deque<rcFRect>& rectTemplates() const {return mTemplateRects; }
  const deque<rcIRect>& rectSearches() const {return mSearchRects; }
  /*
    effect: returns the vector of rcWindows representing this cell
    over time. The first rcWindow in the vector is the oldest.
    TBD: pointer, reference, or
  */

  const vector<rcPolygon>& polygons() const {return mPOLYs; }
  /*
    effect: returns the vector of polygons representing this cell
    over time. The first polygon in the vector is the oldest.
    TODO: map through transformation
  */

  const vector<rcAffineRectangle>& affys() const {return mAffys; }
  /*
    effect: returns the vector of polygons representing this cell
    over time. The first polygon in the vector is the oldest.
    TODO: map through transformation
  */

  const rcIRect& minTemporalEnclosingRect () const {return mMinTemporalEncRect; }
  /*
    effect: returns the rect that so far encloses this cells whereabouts.
    TODO: map through transformation
  */


  const rcIRect& minTemporalIntersectingRect () const {return mMinTemporalIntRect; }
  /*
    effect: returns the rect that is the intersection of all rects of this cell
    TODO: map through transformation
  */

  const rcShapeRef& shape () const {return mShapeRef; }


  // Tracking Functions

  virtual void measure ();
  /*
   * measure () performs disparity measurement for this cell.
   */

  virtual void update ();
  /*
   * updates tracking information
   */

  virtual void reconcile ();
  /*
   * Reconciles state information about this cell
   * 
   */

  const boost::shared_ptr<rcKinetoscope> sharedKin() const 
  {
    return mKine; 
  }

  const rc2Fvector& cop() const 
  {
    return mMotionCtr;
  }

  const rcFRect& moving () const
  {
    return mMoving;
  }

  void moving (const rcFRect& fm)
  {
    mMoving = fm;
  }

  // TBD: mark with haveTemporal
  const deque<rcFPair>& interpolatedMin () const
  {
    return mInterpolatedMin;
  }

  const deque<rcPair<rcRadian> >& peakAngles () const
  {
    return mPeakAngles;
  }

 const vector<rc2Fvector>& children () const
  {
    return mChildren;
  }

  const rcIRect& dividingArea () const
  {
    return mDividingWatchWindow;
  }

const rcFRect& similarityArea () const
  {
    return mCurrentSimRect;
  }

  template<class P>
  rcIRect roundRectangle (const rcRectangle<P>& fr, bool makeVecMultiple = false) const;

  /*
   * @Description uniform rounding for cell morphometry
   */

  const rcStatistics& stats () const
  {
    return mSumQuality;
  }

  /*
   * @Description return a const reference to the Statistics calculator
   */

  
  friend ostream& operator<< (ostream&, const rcVisualFunction&);

  // required for STL
  bool operator<(const rcVisualFunction& rhs) const { return mId < rhs.mId; };
  bool operator==(const rcVisualFunction& rhs) const { return mId == rhs.mId; };

 private:

  // Organism Info
  rcOrganismInfo::OrganismType mOtype;
  rcOrganismInfo::OrganismName mOname;

  
  // Info markers
  bitset<eNumHaves> mHaves;
  bitset<eNumStates> mState;

  // Workers
  void initFunction ();
  void direct (const rc2Fvector&);
  void correspond (const rc2Fvector&, const rcFPair&);
  void correspondMutualChannel (const rc2Fvector&, const rcFPair&);

  bool vXform (DOF, rc2Xform&);

  inline rc2Fvector& motionCenter ()
  {
    return mMotionCtr;
  }

  float framesPerPeriod (float p);
  const int32 ssWindowSize () const {return mSSwindowSize; }
  inline const float adaptBias () const {return rmMax (gMinAdaptBias, sharedKin()->maxRadialSpeed()); }
  inline const rcFPair predictedTolerance (rc2Fvector&) const;

  //@note: Cell/Organism specific Functions

  // General Cell 
  void populationMigration ();

  // Cardiac Cells
  void cardiacUpdate ();
  void cardiacMeasure (vector<rcPolygon>::const_iterator&);
  void beatProcess ();
  float sideShorteningEstimate () const;
  float sideShorteningActual () const;
  deque<rc2Fvector>& ends () const;

  // Label'd (?) and Dividing cells
  void dividingMeasure (vector<rcPolygon>::const_iterator&);  
  bool dividingProcess();
  bool dividedProcess();
  bool grow (const  vector<rcIRect>&);
  bool grow(vector<rcPolygon> groupIntersects, 
	  list<rcVisualFunction>& functionList);

  uint32 intersectSet (vector<rcPolygon*>&);
  bool isIsoMorphic ();
  void createIrradianceModel ();

  // Running accelerator
  bool updateSims (const rcWindow&, const rcFRect&);
  void updateRects (const rcFRect&, const rcFPair& , rcFRect& , rcIRect& ) const;
  int32 ssWindowSize (int32 ws);
  double computeAcceleration ();
  void initializeSimilarator (const rcFRect& simRect);

  // Correspondence
  double match (rcWindow& image,
	      rcWindow& model,
	      rc2Fvector& trans);

  double match (rcWindow& image,
		rcCorrelationWindow<uint8>& model,
		rc2Fvector& trans);


  // @note these two functions are set to the appropriate 
  // (function wise) images / channels. We could use 
  // a design so that they would get called only at 
  // initialization of the function. But now we set them 
  // at measure () since it is the first call at every 
  // Kinetoscope step.
  // @note no check is done to see if the selected images / channels
  // are an appropriate set. It is your responsibility to check that

  const rcWindow& use4moving () const;
  const rcWindow& use4fixed () const;

  // General Accumulators and instant markers
  uint32 mId, mParentId, mGeneration;
  int32 mSize, mNsQ, mEllipseIndex, mLabel, mRad;
  int32 mIntegralFramesPerPeriod, mFramesPerPeriodPowerOf2;
  float mArea, mMass, mPerimeter, mCircularity, mEllipseRatio, mRdiffuse, mCosSums, mMotionPattern, mCorrespondenceQuality, mAcceleration;
  rc2Fvector mV, mVt, mMotionCtr, mAffineSize, mCurrentPosition;
  rcIRect mMinTemporalEncRect, mMinTemporalIntRect;
  rc2Xform mXform, mMotionXform; 
  rcFRect mMoving, mCurrentSimRect;
  rcStatistics mSumQuality;
  rcRadian mLastAngle;
  double mDistance;

  // Referenced copy access to the Kinestoscope
  boost::shared_ptr<rcKinetoscope> mKine;
  int32 mExpStartFrameIndex;
  int32 mExpDividedFrameIndex;

  // Cardiac Processing
  rcInfLine mCardiacAxis;
  rc2Fvector mMidPoint;
  vector<rc2Fvector> mMajorEndPoints;
  rcFPair mMajorEnds;
  float mFirstBeatLength, mBeatMedian, mBeatShortenning, mBeatNumber, mContractionFrequency;
  float mEstimatedShortening, mActualShortening, mFramesPerPeriod;

  // Match Space
  vector<vector<double> > mSpace;
  
  // Shape Calculator
  rcShapeRef mShapeRef;

  // Reference to a rcSimilarator Object
  rcSimilaratorRef mCoarseSimRef;


  // Temporal Data Holders
  deque<rcCorrelationWindow<uint8> > mSnaps;
  deque<rcCorrelationWindow<uint8> > mTemplates;
  deque<rcFRect> mTemplateRects;
  deque<rcIRect> mSearchRects;
  deque<rc2Fvector> mTemplateOrigins;
  deque<rc2Fvector> mTemplatePoses;

  rcDPair mTemplateOriginEnds;
  deque<rcFRect> mRectSnaps;
  vector<rcPolygon> mPOLYs;
  vector<rcAffineRectangle> mAffys;
  vector<float> mSS;
  vector<float> mAcl;
  rcCorrelationWindow<uint8>  mGaussModel;
  vector<rc2Fvector> mChildren;
  vector<float>mChildrenL2;
  
  // One dimenstional temporal value buffer for Beat Processing
  vector<float> mDimensions;
  vector<float> mDimensionTs;
  deque<double> mBeatAC;
  deque<rcFPair> mInterpolatedMin;
  deque<rcPair<rcRadian> > mPeakAngles;

  // Division processing Attributes
  int32 mSSwindowSize;
  int32 mDividingTicks, mAnaphase;
  rcIRect mDividingWatchWindow;
  rcMathematicalImage mIradModel;
};


/*  @bug 
    @todo 
    @Description: Round Rectangle: Round up ul -- round down lr
*/

 template<class P>
 rcIRect rcVisualFunction::roundRectangle (const rcRectangle<P>& fr, bool makeVecMultiple) const
{
  static const int32 sDummy (0);
  rcIPair ul (rfRound (fr.origin().x(), sDummy), 
	      rfRound (fr.origin().y(), sDummy));
  rcIPair lr (rfRound (fr.origin().x() + fr.width(), sDummy),
	      rfRound (fr.origin().y() + fr.height(), sDummy));
  if (!makeVecMultiple)
    return rcIRect (ul, lr);

  rcIRect ir (ul, lr);
  int32 p16 = ir.width() % 16;
  p16 = p16 ? 16 - p16 : p16; // how much to grow
  return rcIRect (ir.ul().x(), ir.ul().y(), ir.width() + p16, ir.height());
  
}

void rfVisualFunctionStateName( rcVisualFunction::State state,
				char* valueBuf, uint32 bufSize );

void rfDivisionReport (list<rcVisualFunction>& visualBodies, ostream& output);



#endif /* __RC_CELL_H */
