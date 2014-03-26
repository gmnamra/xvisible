/* @file
 *
 *$header $
 *$Id $
 *$Log$
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_KINETOSCOPE_H
#define __RC_KINETOSCOPE_H

/*
 * rcvisualstructure.h and rc_visualcell.h are included at the bottom
 */

#include <rc_ip.h>
#include <rc_analysis.h>
#include <rc_histstats.h>
#include <rc_rectangle.h>
#include <rc_graphics.h>
#include <rc_framegrabber.h>
#include <rc_videocache.h>
#include <rc_stats.h>
#include <rc_timestamp.h>
#include <rc_xforms.h>
#include <rc_similarity.h>
#include <rc_timinginfo.h>
#include <rc_moments.h>
#include <rc_point_corr.h>
#include <rc_kinepyr.h>
#include <rc_polygon.h>
#include <rc_latticesimilarity.h>
#include <rc_moviefileformat.h>
#include <map>
#include <list>
#include <bitset>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>



// Forward declarations
class rcMotionVectorPath;
typedef map<int32, rcMotionVectorPath> rcMotionPathMap;
class   rcVisualFunction;

class rcOrganismInfo 
{
 public:
  enum OrganismType
  {
    eBacteria,
    eCell,
    eFluorescenceCluster,
    eModel,
    eDefault = eCell
  };

  enum OrganismName
  {
    eProteinP53 = eFluorescenceCluster + 100,
    eProtein,
    eSkov,
    eCardioMyocyte = eCell + 200,
    eCardioMyocyteCalcium,
    eCardioMyocyteSelected,
    eDanioRerio = eModel + 300,
    eUnknown,
  };
};


class rcKinetoscope : public boost::enable_shared_from_this<rcKinetoscope>
{
  friend class rcMotionVectorPath;
  friend class rcVisualFunction;

 public:

  enum Source
  {
    eVideoCache = 0,
    eFileGrabber,
    eUnknownSource
  };

  //   Contractile Motion implies there is no drift and spatiotemporal  
  //   motion energy is largely in periodic motion clusters in the 
  //   in the scene. We can detect this condition. 

  enum SourceMotion
  {
    eAttentiveCapture = 0,
    eOverSampled,
    eUnderSampled,
    eContractile,
    eCanonical,
    eEpiFluorescence,
    eDirectedFlow,
    eUnknownMotion
  };


  enum Defaults
    {
      eMinMobSizeInPixels = 25,
      eMinSpeedInPixels = 2,
      eMaxSpeedInPixels = 10,
      eSplatSize=3,
      eSplatArea=eSplatSize*eSplatSize,
      eEndDefault
    };

  enum Time
    {
      eFixed = 0,
      eMoving = 1
    };

  enum Detail
    {
      eFine = 0,
      eCoarse = 1
    };

  enum Option
  {
    eBasic=0,
    eLineage,
    eMutualChannel,
    eCanStep,
    eSelectedMyo,
    eNumOptions
  };

  bool have (enum Option what) const;
  bool mark (enum Option what);
  bool clear (enum Option what);
  void reset ();

/*	*********************************
	*                               *
	*     Constructor / Destructors *
	*                               *
	*********************************
*/
  
	typedef boost::shared_ptr<rcKinetoscope> rcSharedKinetoscope;
	
  rcKinetoscope ();

  /* @Brief: Defult Constructor
   * For test use only
   */

  rcKinetoscope (shared_video_cache& fg, const rcIRect rect, uint32 phase = 0,
		 uint32 sample = 1, rcKinetoscope::Detail d = eFine,
		 rcMovieFileOrgExt org = rcMovieFileOrgExt (),  rcOrganismInfo::OrganismType c = rcOrganismInfo::eDefault,
		 rcOrganismInfo::OrganismName n = rcOrganismInfo::eUnknown, int32 minMobSize =  eMinMobSizeInPixels, bool debug = false );

  rcKinetoscope (boost::shared_ptr<rcFrameGrabber>& fg, const rcIRect rect,  uint32 phase = 0,
		 uint32 sample = 1, rcKinetoscope::Detail d = eFine, 
		 rcMovieFileOrgExt org = rcMovieFileOrgExt (), rcOrganismInfo::OrganismType c = rcOrganismInfo::eDefault,
		 rcOrganismInfo::OrganismName n = rcOrganismInfo::eUnknown, int32 minMobSize =  eMinMobSizeInPixels, bool debug = false );

  /* @Brief: Constructs a Kinetoscope
   * Requires a valid videoCache or frameGrabber. 
   * Grabs the first frame
   */

  virtual ~rcKinetoscope ();
  
  /* @Brief: Destructs a Kinetoscope. Stops the grabber. Prints errors to cout
   * Requires a started grabber. 
   */

	rcSharedKinetoscope shared ()
	{
		 return shared_from_this();
	}
	
/*	****************************************************
	*                                                  *
	*     Mutators & Accessors for Control Parameters  *
	*                                                  *
	****************************************************

	A kinetoscope is in one of 3 modes of operation:
	1. General (cannonical cell migration)
	2. General + Beads 
	3. PreSegmented (Muscle Cells / CardiomyoCytes


*/
  void visualFunctionInfo (rcOrganismInfo::OrganismType c = rcOrganismInfo::eDefault,
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
	     organismName() == rcOrganismInfo::eCardioMyocyteCalcium || 
	     organismName() == rcOrganismInfo::eCardioMyocyteSelected));
  }

  bool isLabelProtein () const
  {
    return (organism() == rcOrganismInfo::eFluorescenceCluster && 
	    (organismName() == rcOrganismInfo::eProtein || organismName() == rcOrganismInfo::eProteinP53));
  }

  void fileOriginType (rcMovieFileOrgExt& orgx)
  { mOrgExt = orgx;}

  const rcMovieFileOrgExt& fileOriginType () const
  { return mOrgExt;}

  const int32 frames () const
  { return mFc; }

  const bool channelAvailable () const
  {
    return have (eMutualChannel);
  }

  int32 sourceMotion () const
  {
    return mSourceMotion;
  }

  int32 sourceMotion (SourceMotion sm) 
  {
    return (mSourceMotion = sm);
  }

  void preSegmentedBodies (const vector<rcPolygon>& bodies);
  void preSegmentedBodies (const vector< vector <rc2Fvector> >& bodyEnds, 
			   vector<float>& rads);

  /* @Brief: Polygons representing pre-segmented bodies.
   *         Polygons are required to be within "rect" described above
   */

  uint32 sample () const;
  uint32 phase () const;

  void periodicMotion (float);
  float periodicMotion () const;

  void averageFramesPerSecond (double);
  float averageFramesPerSecond () const;

  /* @Brief: indicating periodic motion.
   *         @note used by muscleSegmentation (run outside of this) to indicate 
             frames during a period. 
   */

  inline int32 detail () const
  {
    return (int32) mDetail;
  }

  inline int32 detail (const Detail d)
  {
    return mDetail = d;
  }

  /* @Brief: Sets / Gets temporal sampling. 
   * Requires a started grabber. 
   */
  
  void target (int32 sam);
  rcIPair target () const;
  void maxRadialSpeed (int32 sam);
  int32  maxRadialSpeed () const;
  rcIPair capture () const;
  float sizeMultiplier () const;
  void sizeMultiplier (float);

  /* @Brief: Sets Motion estimatio target size and max speed
   * Requires a started grabber. 
   */

  const rcIRect& pframe () const;
  /* @Brief: Gets the active image size for this kinetoscope
   * Requires a started grabber. 
   */

  const rcIRect& iframe () const;
  /* @Brief: Gets the image size for this kinetoscope
   * Requires a started grabber. 
   */

  void laminarSuppression (double over);
  double laminarSuppression () const;
  /* Suppress Laminar 
   * Cut off 0.0 - 1.0. Higher Cut Off will suppress higher level of 
   * noise in acquisition and quantization. Default: 0.7
   */

  void minMobSizeInPixels (int32 ms);
  int32 minMobSizeInPixels (); 

  /* Minimum Moving Object Size in Pixels. Can not be less than 5.
     Negative values are invalid. 
   */

  const rc1Xform& imageFromPhysical () const;
  const rc1Xform& physicalFromImage () const;
  const rc1Xform& secondsToUnits () const
  {
    return mTemporal;
  }

  /*
    @brief: setting transformations between real temporal/spatial units and discrete ones.
  */


  void temporalScale (float ts)
  {
    mTemporal.scale (ts);
  }

  /* Reporting Scale used for temporal measurements 
   * Example: 1. (Seconds), 1000. (Milliseconds)
   * Default MilliSeconds
   */

  inline const rc2Xform& xform () const
  {
    return mGlobalXform;
  }
 
/*	****************************************************
	*                                                  *
	*     Mutators Control Methods                     *
	*                                                  *
	****************************************************
*/
 
  int32 step (uint32 steps = 1, bool keepgoing = 1);
  /*
   @note started
             current iterator have an iterator at -1 index
   @brief:   runs measure in all motionpaths 
             TBD: should it take number of steps or
	          potentially have an "auto" step
  */

  int32 stepContractile (uint32 steps = 1, bool keepgoing = 1);
  /*
   @note started
             current iterator have an iterator at -1 index
   @brief:   runs measure in all motionpaths 
             TBD: should it take number of steps or
	          potentially have an "auto" step
  */

  int32 stepDirectedFlow (uint32 steps = 1, bool keepgoing = 1);
  /*
   @note started
             current iterator have an iterator at -1 index
   @brief:   runs measure in all motionpaths 
             TBD: should it take number of steps or
	          potentially have an "auto" step
  */

  int32 stepEpi (uint32 steps = 1, bool keepgoing = 1);
  /*
   @note started
             current iterator have an iterator at -1 index
   @brief:   runs measure in all motionpaths 
             TBD: should it take number of steps or
	          potentially have an "auto" step
  */

  int32 stepDirectedFlowPreFill ();
  /*
   @note Pre Fill Array of Similarators
  */

  void advance ();

  /*  
   * Rotate the FrameData and Connection Maps:
   * 1 -> 0, 0 -> 1 ready for new data at one
   *
   */

  void reconcileNewGeneration ();
 /*
   * @note: add recent arrivals to the function list.
   *
   */ 

  void newGeneration (list<rcVisualFunction>& cells, 
				vector<rcPolygon>& regions);
 /*
   * @note: add recent arrivals to the function list.
   *
   */ 

/*	****************************************************
	*                                                  *
	*     Accessor Methods                             *
	*                                                  *
	****************************************************
*/

  rcTimestamp resultTime() const;


  /* Absolute time stamp of first result
   */  
 
  double deltaTime () const
  {
    return mTemporal.mapVector (mElapsedTimeSecs - mLastElapsedTimeSecs);
  }

  /*
   * Time since the last frame
   * In Milliseconds.
   */
  double elapsedTime () const
  {    
    return mTemporal.mapVector (mElapsedTimeSecs);
  }

  /*
   * Time since the last frame
   * In Milliseconds.
   */
  double currentTime () const
  {    
    return mTemporal.mapVector (mCurrentAbsTimeStamp.secs());
  }

  /*
   * How long has we been running ?
   * In Miliseconds.   
   */

  const rcTimestamp& startingAbsoluteTime() const { return mStartAbsTimeStamp; }

  /* Absolute time stamp of first fetched frame
   */  

  const rcWindow& use4fixed () const;
  const rcWindow& use4moving () const;
  const rcWindow& use4when (const Time when) const;
  const rcWindow& fixed () const { return mFrameData[eFixed].base(detail()); }
  const rcWindow& fixedPast () const { return mFrameData[eFixed].past (detail()); }
  const rcWindow& movingPast () const { return mFrameData[eMoving].past (detail()); }
  const rcWindow& moving () const {return mFrameData[eMoving].base(detail()); }
  const rcWindow& movingDirection () const {return mFrameData[eMoving].direction (detail()); }
  const rcWindow& fixedDirection () const { return mFrameData[eFixed].direction(detail()); }

  const rcWindow& channelFixed () const { return mChannelData[eFixed].base(detail()); }
  const rcWindow& channelFixedPast () const { return mChannelData[eFixed].past (detail()); }
  const rcWindow& channelMovingPast () const { return mChannelData[eMoving].past (detail()); }
  const rcWindow& channelMoving () const {return mChannelData[eMoving].base(detail()); }
  const rcWindow& channelMovingDirection () const {return mChannelData[eMoving].direction (detail()); }
  const rcWindow& channelFixedDirection () const { return mChannelData[eFixed].direction(detail()); }

  //@note debug flag can only be changed before second frame. 
  const rcWindow debug () const;
  bool debug (bool db) { if (count() < 2) mDebug = db; return mDebug;}
  bool isDebugOn () const { return mDebug; }

  // For testing 
  void fieldMap (rcWindow&) const;

  /*
   * access to temporal images. 
   * @note: this can be much better designed.
   * @note started
   * @description moving is t+1 frame (raw)
   *              fixedRaw is t frame 
   *
   */

  vector<rcPolygon>& fixedPolygons () { return mFixedPOLYs; }
  vector<rcPolygon>& movingPolygons () { return mPOLYs; }

  bool canStep () const { return have (eCanStep); }
  /*
   @Brief: return true if the well is ready to be stepping
  */

  const rcStatistics& populationMeasures () const;
  /*
    return Statistics of temporal mean of population median meaasures
  */
  
  float directedCellToCellMigration ();
  /*
    @brief: return average migration of cells towards other cells
    returns notAvailble if there are no cells
  */

  float avgMeanSqCellDisplacements ();
  /*
    @brief: return measure of diffusion: average min square of cell displacements
  */
  
  void rawSpeedStats (rcStatistics& stats);
  /*
   * returns unsegmented raw motion speeds for the just
   * completed run
   */
  void rawDirectStats (rcHistoStats&);
  /*
   * returns unsegmented raw direction histogram for the just
   */

  list<rcVisualFunction>& visualBodies ()
  {
    return mFunctionMap;
  }

  /*
   * Output Temporal Information
   *
   */

  ostream& temporalResults (ostream&,ostream&);  
  friend ostream& operator<< (ostream&, rcKinetoscope&);


/*	***************************
	*                         *
	*     Graphics Accessors  *
	*                         *
	***************************
*/

  void visualGraphics (rcVisualSegmentCollection& segments, const rcVisualStyle& baseStyle);
  void visualFunctionMap (rcVisualSegmentCollection& segments, const rcVisualStyle& baseStyle);
  void visualFunctionHistory (rcVisualSegmentCollection& segments, const rcVisualStyle& baseStyle);
  void visualDebugGraphics (rcVisualSegmentCollection& segments, const rcVisualStyle& baseStyle);
 
  void visualGraphics (rcVisualGraphicsCollection& graphics);
  void visualFunctionMap (rcVisualGraphicsCollection& graphics);
  void visualFunctionHistory (rcVisualGraphicsCollection& graphics);
  void visualDebugGraphics (rcVisualGraphicsCollection& graphics);

/*	***************************
	*                                      *
	*     Frame Accessors         *
	*                                      *
	***************************
*/
  int32 count () const;
   

/*	***************************
	*                         *
	*     Defaults
	*                         *
	***************************
*/

  static const int32 defaultTargetRadius = 3;
  static const int32 defaultMaxRadSpeed =  eMinSpeedInPixels;
  static const double defaultCutOff = 5.0;
  static const float notAvailable = -2.0;

 protected:
  
  
  void start ();

  /*  
   * Start the Function Localization
   * 
   */
  void connectedFunctions (Time when, vector<rcPolygon>& polys2fill);


  /*  
   * Measure Functions
   * 
   */
  void measure (rcMotionPathMap&);
  void measure (list<rcVisualFunction>&);

  /*
   * Update 
   *
   */
  void update (rcMotionPathMap&);
  void update (list<rcVisualFunction>&);

  /*
   * Reconcile
   *
   */

  void reconcile (rcMotionPathMap&);
  void reconcile (list<rcVisualFunction>&);

  /*  
   * Start the field of motionPaths:
   * 
   *  Maps of motionPaths
   */
  void selfSimilarMotionField ();
  void selfSimilarMotionField_AttentiveCapture ();
  void selfSimilarMotionField_UnderSampled ();
  void canonicalMotionField (const rcWindow&);
  void epiMotionField ();

  /*
   * Internal Functions that probably should be private.
   * TBD: Re-org
   */

  inline const rcIRect& uframe () const
  {
    return mUserRect;
  }

  inline const rcFRect& fpframe () const
  {
    return mFpFrame;
  }

  /* @Brief: Gets the image size for this kinetoscope
   * Requires a started grabber. This is at User resolution.
   */

  inline bool isPointCorrOptimizationAvailable () const
  {
    return (mMomentMap[eFixed] != 0 && mMomentMap[eMoving] != 0);
  }

  /*
    Return True if optimized point corr is possible
  */

  inline bool preProcessConnect () const
  {
    return false;
  }

 
  /*
   * Check if a spatial point is set in the temporal registry
   */ 
  bool isWithin (const rc2Fvector&) const;

  /*
   * Check if we are inside the phase period
   */ 
  bool isWithin () const;

  /*
   * Check if a rect is in the active region
   */ 
  bool isWithin (const rcIRect&) const;

  rcIPair kernelBorder () const;

  /*
   * @brief: Sets up flow analysis prototype
   */
  void reichardt (Time);


  bool validate (const rcIRect& rect, rc2Fvector&) const;
  /*
   * Validate a region's spatial intensity profile
   */

  const rcIPair& movingSize () const;

  bool isBound ();

  void clear ();
  /* @Brief: Sets Default settings
   * Requires a started grabber. 
   */

 private:
  rcKinetoscope (const rcKinetoscope& );
  rcKinetoscope& operator= (const rcKinetoscope& );
  /* Prohibit direct copying and assignment
   */

  void createFunctionSpecificObjects ();
  /* @note: create function specific objects;
   */

  void debugScreen (const rcWindow& image);

  rcWindow& connect ()  {return mConnectionMap[eMoving]; }
  rcWindow& connected ()  {return mConnectionMap[eFixed]; }
  rcWindow& velocityXfield () {return mXvelocityField;}
  rcWindow& velocityYfield () {return mYvelocityField;}

  /*
    @brief: connect area of vector field to a function
  */
  bool connectFunction (const rcIRect& r,
			const rcPolygon& p,
			rc2Fvector& moc,
			vector<rc2Fvector>& anc,
			vector<rc2Fvector>& mot);
  /*
    @brief: return reference to correlation space generating object
  */
  void associate (list<rcVisualFunction>& cells, 
		  vector<rcPolygon>& regions, Time when = eMoving);
  /*
    @brief: associate regions to objects
  */

  const rcPointCorrelation& pointCorr() const { return mPointCorr; }

  /*
   * Facility to store temporal mean of median of cell measures
   */
  void keepPopulationMeasure ();
  rcStatistics mPopulationStats;

  /*
   * Frame Grabbing Interface
   * API to both VideoCache and Grabber interfaces
   */
  bool frame (uint32 deltaFrames, const Time when);
  int32 frameCount () const;
  uint32 startFrame () const;

  /*
   * Control Parameters
   * Note: for now we support both interfaces to grabbing
   */
  Source mSource;
	shared_video_cache mVc;
  rcVideoCacheStatus mVcStatus;
	boost::shared_ptr<rcFrameGrabber> mFg;
  rcFrameGrabberStatus mFgStatus;
  rcMovieFileOrgExt mOrgExt;

  /*
   * Image Buffers
   * @note the right way of doing this is to have a vector 
   * of framedata or better off abstract the whole thing in
   * class. For now, we add mChannelData which gets filled 
   * if we have a combined channel indication from fileOrgExtension
   * (filled in from file information by engineimpl)
   */
  vector<rc2LevelSpatialPyramid> mFrameData;
  vector<rc2LevelSpatialPyramid> mChannelData;
  vector<rcWindow> mConnectionMap;
  vector<rcMomentGenerator *>mMomentMap;
  rcWindow mXvelocityField, mYvelocityField;
  vector<rcWindow> mSimBuffer;
  rcWindow mClearImage;
  rcWindow mFullImage;
  Detail mDetail;
  rcIRect mRect;
  rcIRect mProcessRect;
  rcIRect mUserRect;
  rcWindow mDebugScreen;
  rcWindow mMaskCellwFF;
  

  /*
   * Tracking Objects
   */
  rcMotionPathMap mPathMap; 
  list<rcVisualFunction> mFunctionMap;
  list<rcVisualFunction> mDaughterFunctionMap;
  vector<rcPolygon> mPOLYs, mFixedPOLYs, mMergedPOLYs;
  vector<vector <rc2Fvector> > mBodyEnds;
  vector<float> mBodyRads;
  vector<vector <rc2Fvector> > mVanchor;
  vector<vector <rc2Fvector> > mVmoved;
  vector<rcPolygon> mPrePOLYs;
  rcOrganismInfo::OrganismType mOtype;
  rcOrganismInfo::OrganismName mOname;
  rcLatticeSimilaratorRef mLsm;
  vector<float> mPopulationMeasure;

  /* 
   * Book Keepers
   */
  rcTimestamp mStartAbsTimeStamp;
  rcTimestamp mCurrentAbsTimeStamp;
  float mElapsedTimeSecs;
  float mLastElapsedTimeSecs;
  double mStepProcessingTime;
  rcStatistics mSnapStats;
  double mGlobalMotionEst;
  SourceMotion mSourceMotion;
  float mPeriodicity;
  float mFramesPerSecond;
  void updateProcessingTime ();
  int32 mSimPipe;

 // Option markers
  bitset<eNumOptions> mOptions;

  // transformations

  rc1Xform mImageToPhysical;
  rc1Xform mPhysicalToImage;
  rc1Xform mTemporal;
  rc2Xform mGlobalXform;
  
  // Tracking fixed and moving sizes
  void mov ();
  void dumpMap (ostream&);
  void dumpPolys (ostream&);
  void printPolys ()   __attribute__ ((noinline));
  void printMap ()  __attribute__ ((noinline));
  void printConnect()  __attribute__ ((noinline));
  void printConnected()  __attribute__ ((noinline));
 
  rsCorrParams mCorr;
  rcAutoCorrelation mAutoCorr;
  rcMomentGenerator mMomGen0, mMomGen1;
  rcPointCorrelation mPointCorr;
  int32 mFc;
  uint32 mCount;
  int32 mDisVarError, mDisGMEError,mDisEdgeError, mDisPolarError;
  uint32 mSample, mPhase;
  rcIPair mTargetRadius, mCaptureRadius, mVmaxRadial, mMovingSize;
  int32 mMinParea;
  rcFPair mOrigin;
  rcFRect mFpFrame;
  double mCutOff;
  bool mDebug;
  float mSizeMultiplier;
};




#include <rc_motionpath.h>
#include <rc_visualcell.h>




#endif /* __RC_KINETOSCOPE_H */
