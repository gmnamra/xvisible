/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.22  2003/04/15 22:23:10  sami
 *Fix merge error, add call to .Double()
 *
 *Revision 1.21  2003/04/15 22:02:48  sami
 *Visible update, performance tweaks
 *
 *Revision 1.20  2003/04/10 15:21:49  sami
 *Fixed rcRadian compilation errors
 *
 *Revision 1.19  2003/03/11 00:11:46  sami
 *const correctness improved
 *
 *Revision 1.18  2003/03/06 23:14:09  sami
 *Use two maps in rfMotionFiler, added rcFeature::target() accessor
 *
 *Revision 1.17  2003/03/04 22:57:19  arman
 *started adding reduced size rcFeature
 *
 *Revision 1.16  2003/03/04 20:06:04  arman
 *fixed a booboo
 *
 *Revision 1.15  2003/03/04 19:45:34  arman
 *added maxSpeed control
 *
 *Revision 1.14  2003/03/03 20:33:49  sami
 *Addec onst accessors, silenced compiler warnings
 *
 *Revision 1.13  2003/02/28 18:24:33  arman
 *incremental
 *
 *Revision 1.12  2003/02/27 19:41:00  arman
 *Updated before internal demo
 *
 *Revision 1.11  2003/02/26 12:35:18  arman
 *rcMotionMap support
 *
 *Revision 1.10  2003/02/21 16:16:00  arman
 *Incremental
 *
 *Revision 1.9  2003/02/18 21:08:17  arman
 *added << support
 *
 *Revision 1.8  2003/02/17 21:50:45  arman
 *Changed default mode
 *
 *Revision 1.7  2003/02/17 14:48:50  arman
 *Added rcFeature
 *
 *Revision 1.6  2003/01/28 15:32:14  arman
 *Added alignment mode
 *
 *Revision 1.5  2003/01/15 15:34:26  arman
 *Added check function
 *
 *Revision 1.4  2003/01/08 02:11:43  arman
 *removed unused stuff and added definitions
 *
 *Revision 1.3  2002/11/25 14:01:10  arman
 *Removed 1DCorrelation
 *
 *Revision 1.2  2002/11/24 13:49:59  arman
 *Incremental
 *
 *Revision 1.1  2002/11/23 20:37:36  arman
 *Incremental Checkin
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */


#ifndef __RC_DBDT_H
#define __RC_DBDT_H

#include <rc_window.h>
#include <rc_vector2d.h>
#include <rc_math.h>
#include <deque>
#include <map>

#define rmMinTarget 5
#define rmStdSpeed 1


class rcDbDtParams
{
 public:

  rcDbDtParams () : mSample (1,1), mSearchSize (9,9), mTargetSize (5,5), mDebug (false), mMethod (e2D2D) {}

  // default copy, assignment, dtor ok

  enum method
    {
      e2D2D,
      e1D2D,
      e1D1D,
      eEnd
    };

  // TBD: tightenup 
  // Note: Rectangle contains is not appropriate (src can be a window)
  bool validate (const rcWindow& src) const
  {
    bool flag = true;
    rcUIRect s (0, 0, uint32 (mSearchSize.x()), uint32 (mSearchSize.y()));
    rcUIRect t (0, 0, uint32 (mTargetSize.x()), uint32 (mTargetSize.y()));
    flag = s.contains (t);
    flag &= (src.rectangle().width() > uint32 (mSearchSize.x()) && src.rectangle().height() > uint32 (mSearchSize.y()));
    flag &= (mSample.x()  > 0 && mSample.y() > 0);
    flag &= (src.rectangle().width() > uint32 (mSample.x()) && src.rectangle().height() > uint32 (mSample.y()));
    return flag;
  };

  const rcIPair& sample () const {return mSample; }
  void sample (rcIPair& sam) {mSample = sam;}

  const int32 maxSpeed () const {return (mSearchSize.x()-mTargetSize.x())/2; }
  void maxSpeed (int32 s) {mSearchSize = mTargetSize + rcIPair (s,s) + rcIPair (s,s);}

  const rcIPair& search () const {return mSearchSize; }
  void search (rcIPair& sea) {mSearchSize = sea;}
  const rcIPair& target () const {return mTargetSize; }
  void target (rcIPair& sam) {mTargetSize = sam;}
  const bool debug () const { return mDebug; }
  void debug (bool dog) { mDebug = dog; }
  const double reckon () const { return mReckon; }
  void reckon (double reckon) { mReckon = reckon; }
  const int32 method () const { return mMethod; }
  void method (enum method m) { mMethod = m; }

  const rsCorrParams& corrParams () const {return mCorr; }

  friend ostream& operator<< (ostream&, const rcDbDtParams&);

 private:
  rsCorrParams mCorr;
  double mReckon;
  rcIPair mSample;
  rcIPair mSearchSize;
  rcIPair mTargetSize;
  bool mDebug;
  enum method mMethod;

};

// Design Considerations for rcFeature
// Data is for the last frame analyzed (t)
// 
//  t-1        t        t+1
//                 ^
//
// Disparity Information:
// Only the following data are essential:
// dX,dY    :Sub-pixel fractions in 1/16 integer increments
// iX,iY    :Whole pixel in signed shorts
// quality  :Quality in an unsigned short
// r        :Angle 
//
class rcFeature
{
 public:

  rcFeature () { init (); }

  rcFeature (const double& quality, const rc2Dvector& p, const rcDPair& ctr,
	     const rcIRect& target = mStdTargetSize, 
	     int32 speed = rmStdSpeed)
  { 
    mPos = p; mCtr = ctr;
    mTarget = target;
    mSpeed = speed;
    mQuality = quality;

    static int32 dummy;
    mI.x() = rfRound (ctr.x(), dummy);
    mI.y() = rfRound (ctr.y(), dummy);

  }

  // default copy, assignment, dtor ok

  // Returned track position with respect to the origin
  // TBD: fixed point results
  rc2Dvector& pos () {return mPos;}
  const rc2Dvector& pos () const {return mPos;}
  rcUIPair& polar () {return mRT;}
  const rcUIPair& polar () const {return mRT;}

  // Origin of the tracked region
  rcIPair& in () {return mI;}
  const rcIPair& in () const {return mI;}
  rcDPair& origin () {return mCtr;}
  const rcDPair& origin () const {return mCtr;}

  // Quality of the match
  double& quality () {return mQuality;}
  const double& quality () const {return mQuality;}

  // Target size of the region
  rcIRect& target () {return mTarget;}
  const rcIRect& target () const {return mTarget;}
  
  bool operator== (const rcFeature& r) const {return (*this == r); }
  bool operator!= (const rcFeature& r) const { return !(*this == r); }
  bool operator< (const rcFeature& r) const { return !(mI.x() < r.in().x() && mI.y() < r.in().y()); }

  friend ostream& operator<< (ostream&, const rcFeature&);
  
 private:
  static rcIRect mStdTargetSize;

  inline void init ()
  {
    mPos = rc2Dvector ();
    mRT = rcUIPair (0,0);
    mI = rcIPair (0, 0);
    mCtr = rcDPair (0., 0.);
    mTarget = mStdTargetSize;
    mSpeed = rmStdSpeed;
    mQuality = 0.0;
  }

  double mQuality;
  rcIRect mTarget;
  int32 mSpeed;
  rc2Dvector mPos;
  rcUIPair mRT;
  rcIPair  mI;
  rcDPair  mCtr;

  uint8 mDx;
  uint8 mDy;
  uint8 mDr;
  uint16 miX;
  uint16 miY;
  uint16 mQ;
};



// TBD: rewrite all to use fixed point
class rcMotionPath
{
public:

  enum state
    {
      eUnInitialized,
      eTracking,
      eNoFeature
    };

  enum mode
    {
      eLean,
      eKeepFeatures
    };

  rcMotionPath(mode m = eLean) : mMode (m)
  {
    init ();
  }

  rcMotionPath(const rcFeature& sp, mode m = eLean)
    : mMode (m), mStartPosition (sp.origin ())
  {
    init ();
    add (sp);
  }
  
  // Default assignment, copy constructor, destructor are OK

  // Add a track to the repository:

  void add (const rcFeature& feature)
  {
    mNstarted++;
    mNtracked++;
    const rc2Dvector& pos = feature.pos();

  // If the motion vector indicates no motion, the current decision is to add them
  // but not add to the statistics (TBD)
    if (pos.x() < 0.0001 && pos.y() < 0.0001)
      return;

    double angle = pos.angle().Double();
    mSumPos += pos;
    mSumAngle += angle;
    mSumSqAngle += angle * angle;
    mSumAngle += feature.quality();
    mSumSqQuality += feature.quality() * feature.quality();

    if (mNtracked)
      {
	double bsd = angle - mLastAngle;
	mSumAngledT += bsd;
	mSumSqAngledT += bsd*bsd;
	mLastAngle = angle;
      }

    // Finally if mode is not zero, add this feature to the features list
    // Note: a copy will made
    if (mode() == eKeepFeatures) features.push_front (feature);
    state (eTracking);
    mLastFeature = feature;

  }

  void add ()
  {
    mNstarted++;
    mNtracked = 0;
    state (eNoFeature);
  }
    
  int32& n() { return mNstarted; };
  const int32& n() const { return mNstarted; };
  int32& numTracked() { return mNtracked; };
  const int32& numTracked() const { return mNtracked; };
  rc2Dvector& sumPos() { return mSumPos; };
  const rc2Dvector& sumPos() const { return mSumPos; };
 
  rcDPair startPosition () const { return mStartPosition; }
  // Starting position:
  // Assumes that camera position has not changed (Important)

  enum mode mode () { return mMode; };
  state state () {return mState;};
  rcFeature& lastFeature() { return mLastFeature; };
  const rcFeature& lastFeature() const { return mLastFeature; };
  
  void state (enum state st) {mState = st;}
  //TBD: should not need this for identifying we are tracking
  // collection of features and not cells

  double& sumAngle() { return mSumAngle; };
  double& sumAngledT() { return mSumAngledT; };
  double& sumSqAngle() { return mSumSqAngle; };
  double& sumSqAngledT() { return mSumSqAngledT; };
  double& sumQuality() { return mSumQuality; };
  double& sumSqQuality() { return mSumSqQuality; };

  friend ostream& operator<< (ostream&, const rcMotionPath&);

  bool operator< (const rcMotionPath& r) const 
  { 
    return (startPosition () < r.startPosition());
  }
 
  private:
  enum state mState;
  // Accumulation of angles is in Radians and radians squared
  // TBD: fixed point support
  enum mode mMode;
  int32 mNtracked;
  int32 mNstarted;
  rcFeature mLastFeature;
  double mLastAngle;
  rc2Dvector mSumPos;
  rcDPair mStartPosition;
  double mSumAngle;   
  double mSumSqAngle; 
  double mSumAngledT;   
  double mSumSqAngledT; 
  double mSumQuality;   
  double mSumSqQuality; 

  void init ()
  {
    mNtracked = mNstarted = 0;
    mSumPos = rc2Dvector (0,0);
    mSumAngle = 0;
    mSumSqAngle = 0;
    mSumAngledT = 0;
    mSumSqAngledT = 0;
    mState = eUnInitialized;    
  }

  deque<rcFeature> features;
};


// Helper functions for getting and setting coordinates in a key

inline int32 rf2key (const rcIPair& p)
{
  return (((p.x())<<16)+(p.y()));
}

inline void rfkey2xy (const int32& key, rcIPair& p)
{
  p.y() = key & 0x0000ffff;
  p.x() = key >> 16;
}

typedef multimap<int32, rcMotionPath> rcMotionMap;



void rfACPeaks (const rcWindow& cur, rcMotionMap& mmap, const rcDbDtParams& params);
/*
 * rfACPeaks can be called in 2 scenarios:
 * 1. mmap is empty: i.e first call. Create lattice of motionpaths, detect motion targets, and update map
 * 2. (TBD) mmap is not empty: some number of motion paths have hit NoFeature states. New targets need to be created 
 */


uint32 rfMotionField (const rcWindow& cur, const rcWindow& next, rcMotionMap& vectorMap, rcMotionMap& bodyMap, const rcDbDtParams& params);
/*
 * rfMotionField can be called in 2 scenarios:
 * 1. vectorMap is empty: i.e first call. Create lattice of motionpaths, detect motion targets, and update map
 * 2. vectorMap is not empty: use tracked positions to create targets in cur and track in next
 */


#endif /* __RC_DBDT_H */
