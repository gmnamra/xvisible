/* @file
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.37  2005/12/27 15:49:29  arman
 *fixed crlf. x.translate post increments so crlf is -w,1
 *
 *Revision 1.36  2005/11/15 23:26:04  arman
 *motionpath redesign
 *
 *Revision 1.35  2005/08/30 21:08:50  arman
 *Cell Lineage
 *
 *Revision 1.35  2005/08/05 21:53:08  arman
 *position issue update
 *
 *Revision 1.34  2005/07/01 21:05:39  arman
 *version2.0p
 *
 *Revision 1.34  2005/05/15 00:09:52  arman
 **** empty log message ***
 *
 *Revision 1.33  2004/12/16 21:59:53  arman
 *removed rcFeature
 *
 *Revision 1.32  2004/12/12 16:03:38  arman
 *fixed bug in rcFeature ctor where unit() was called without checking isNull
 *
 *Revision 1.31  2004/12/09 21:40:45  arman
 *added @file
 *
 *Revision 1.30  2004/11/22 01:54:18  arman
 *added setting mDxDt to move (FVector) overload
 *
 *Revision 1.29  2004/11/19 22:19:57  arman
 *incremental
 *
 *Revision 1.28  2004/08/26 22:41:11  arman
 *position() no longer returns reference
 *
 *Revision 1.27  2004/08/25 03:16:45  arman
 *changes corresponding to kinetoscope changes
 *
 *Revision 1.26  2004/08/23 10:36:45  arman
 **** empty log message ***
 *
 *Revision 1.25  2004/08/19 21:17:32  arman
 *removed unneeded function
 *
 *Revision 1.24  2004/07/19 11:40:03  arman
 *added atStepOne
 *
 *Revision 1.23  2004/07/11 07:09:06  arman
 *handled case where rcFeature is null and therefore can not be asked for its unit ()
 *
 *Revision 1.22  2004/04/19 22:07:23  arman
 *added temporal tracking improvements
 *
 *Revision 1.21  2004/02/26 23:47:23  arman
 *added vector operations for rcFeature
 *
 *Revision 1.20  2004/02/16 21:48:35  arman
 *redesigned rcFeature to represent a unit vector and a length
 *
 *Revision 1.19  2004/02/11 22:45:21  arman
 *added minor changes
 *
 *Revision 1.18  2004/02/03 15:01:47  arman
 *added DxDt
 *
 *Revision 1.17  2004/01/21 21:40:35  arman
 *added a ctor
 *
 *Revision 1.16  2004/01/18 18:05:26  arman
 *improving api
 *
 *Revision 1.15  2004/01/14 20:28:59  arman
 *Major changes
 *
 *Revision 1.14  2003/12/05 19:40:46  arman
 *added dot product. Returns quality only when valid
 *
 *Revision 1.13  2003/11/27 02:53:10  arman
 *added * operator
 *
 *Revision 1.12  2003/11/25 02:00:17  arman
 *interim
 *
 *Revision 1.11  2003/11/12 23:35:26  sami
 *Bug fix: hasPolar() mD component test uses or instead of and
 *
 *Revision 1.10  2003/06/29 11:50:29  arman
 *Incremental ci
 *
 *Revision 1.9  2003/06/28 02:05:20  arman
 *removed mPeak
 *
 *Revision 1.8  2003/06/11 10:02:20  arman
 *added a new form of the ctor
 *
 *Revision 1.7  2003/06/04 21:51:57  sami
 *Added a bit of constness
 *
 *Revision 1.6  2003/06/02 11:53:06  arman
 *corrected geometry
 *
 *Revision 1.5  2003/05/11 18:02:02  arman
 *Inremental
 *
 *Revision 1.4  2003/05/11 12:27:52  arman
 *Added a cheap polar accessor
 *
 *Revision 1.3  2003/04/30 16:00:10  arman
 *fixed initialization
 *
 *Revision 1.2  2003/04/30 15:50:49  arman
 *fixed tangling initialization
 *
 *Revision 1.1  2003/04/18 21:22:13  arman
 *new cell tracking functionality
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_MOTIONPATH_H
#define __RC_MOTIONPATH_H

#include <rc_window.h>
#include <rc_vector2d.h>
#include <rc_math.h>
#include <rc_xforms.h>




/* Motion Paths
 *
 * MotionPaths are the underlying class for tracking
 * MotionPaths are initiated at an anchor in a frame
 * TBD: Integer start position (for now we start from 
 * an integer so rounding is ok given that we track
 * integer motions of the tracking window
 *
 */

class rcBaseMotionPath
{

 public:

  rcBaseMotionPath (): mGenesis (0.0f), mSteps(0) {}

  virtual ~rcBaseMotionPath () {}

  virtual void measure () = 0;
  virtual void update () = 0;
  virtual void reconcile () = 0;

  /*
   * Accessors
   */

  bool isWithin (rcFPair&) const;

  /*
   * Effect: checks if a point is contained within this motionpath
   */

  const float& dob () const
  {
    return mGenesis;
  }

  /*! 
    @function dob
    @discussion :Return date of birth relative to kinetoscope start
   */

  const rc2Fvector position () const
  {
    return mSumPos + mStartPosition;
  }
  /*! 
    @function position
    @discussion Position with respect to the image from it was created from
  */

  rcIPair iAnchor () const
  {
    return rcIPair (int32 (mStartPosition.x()),
		    int32 (mStartPosition.y()));
  }

  const rc2Fvector& travelled () const { return mSumPos; }

  const rc2Fvector& motion () const { return mDxDt; }


  const rc2Fvector& anchor () const { return mStartPosition; }

  void move (const rc2Fvector& t) 
  { 
    mDxDt = t;
    mSumPos += t;
  }
  /*! 
    @function move (vector)
    @discussion Move assuming the (t) -> (t+1)
    @discussion for motion vectors we assume translation 
  */

  void move (const rc2Xform& xm) 
  { 
    rc2Fvector now = mSumPos;
    mSumPos = xm.invMapPoint (now);
    mDxDt = mSumPos - now;
  }
  /*! 
    @function move (xform)
    @discussion Move assuming the (t) -> (t+1)
    @discussion for motion vectors we assume translation 
    @bug: not tested.
  */

  const rc2Fvector& anchor (const rc2Fvector& a) 
  { 
    mStartPosition = a;
    return mStartPosition;
  }

  void step () {mSteps++; return;}

  uint32 steps () const { return mSteps; }

  bool isAtStep (uint32 step) const 
  {
    return mSteps == step;
  }

  // Starting & Current Relative position:
  // Assumes that camera position has not changed (Important)

 protected:
  float mGenesis;
  rc2Fvector mStartPosition;
  rc2Fvector mDxDt;
  rc2Fvector mSumPos;
  uint32 mSteps;
};


/*	*********************
	*                   *
	*  Feature Info     *
	*                   *
	*********************

 rcFeature is 2D vector plus Q as length and dx and dy as length of the unit vector
 
*/

class rcFeature
{
 public:

  // Default copy/dtor/assignment ok

  enum { notAvailable = -3};
  static const float rcFeatureMinLength = 1.0f / (1 << rcFxPrecision);
  static const float oneOverMultConst = 1.0f / (1 << (rcFixed16Precision+rcFixed16Precision));

  rcFeature () : mQ (notAvailable)
  {
    mD.x() = rcFixed16(0.0);
    mD.y() = rcFixed16(0.0);
  }

  // Create from Polar Coordinates
  template <class T>
 rcFeature (const T length, rcRadian d)
  { 
    mD.x() = rcFixed16 (sin (d));
    mD.y() = rcFixed16 (cos (d));
    mQ = rcFixed16 (length);
  }

  template <class T>
  rcFeature (const rc2dVector<T>& v)
  { 
    if (!v.isLenNull()) 
      {
	mQ = rcFixed16 (v.len());
	mD.x() = rcFixed16(v.unit().x());
	mD.y() = rcFixed16(v.unit().y());
      }
    else
      mQ = notAvailable;
  }

  template <class T>
    rcFeature (const T x, const T y)
  { 
    rc2dVector<T> v (x, y);
    if (v.isNull())
      {
	mD.x() = mD.y() = rcFixed16 (0);
	mQ = notAvailable;	
      }
    else
      {
	mD.x() = rcFixed16(v.unit().x());
	mD.y() = rcFixed16(v.unit().y());
	if (!v.isLenNull()) 
	  mQ = rcFixed16 (v.len());
	else
	  mQ = notAvailable;
      }
  }


  // Create Unit Vectors
  rcFeature (rcRadian d)
  { 
    mD.x() = rcFixed16 (sin (d));
    mD.y() = rcFixed16 (cos (d));
    mQ = rcFixed16 (1.0f);
  }

  rcFeature (rcAngle8 d)
  { 
    static const int16 p (1<<rcFixed16Precision);
    static const int16 dxTable[8] = {  p,  p,  0, -p, -p, -p,  0,  p};
    static const int16 dyTable[8] = {  0,  p,  p,  p,  0, -p, -p, -p};
    int32 ti = ((d.basic() + 16) / 32 ) % 8;
    mD.x() = rcFixed16 (dxTable[ti]);
    mD.y() = rcFixed16 (dyTable[ti]);
    mQ = rcFixed16 (1.0f);
  }

  // default copy, assignment, dtor ok


  bool hasPolar (void) const 
  { 
    return (mQ != notAvailable || mD.x() != 0 || mD.y() != 0);
  }

  // returns	true if vector has known direction (implies it has length also)
  // valid mQ or valid mQ and dx != 0 or valid mQ and dy != 0 or 
  // valid mQ and dx != 0 and dy != 0 or 

  rc2Fvector pos () const 
  {
    return rc2Fvector (mD.x().basic() * mQ.basic(), mD.y().basic() * mQ.basic()) 
      * oneOverMultConst;
  }

  /*
   * effect: returns the motion pose
   * note: this function will create a 2D vector <double>
   */

  const rcPair<rcFixed16>& pos16 () const {return mD;}
  /*
   * effect: returns a reference to the motion pose
   */

  void polar (rcFixed16& len, rcRadian& angle) const
  {

    int32 r (mD.x().basic());
    int32 t (mD.y().basic());

    rfFxPolarize (&r, &t);

    angle = rcRadian (rcDegree ((double) (t * rcFeatureMinLength)));
    len = rcFixed16 (r);
  }
    
  /* Note: polar conversion uses Cordic. 
   * Return the motion vector in polar coordinates
   */

  rcAngle8 octants (int32& dX, int32& dY) const
  {
    static const int32 dxTable[8] = {  1,  1,  0, -1, -1, -1,  0,  1};
    static const int32 dyTable[8] = {  0,  1,  1,  1,  0, -1, -1, -1};

    int32 r (mD.x().basic());
    int32 t (mD.y().basic());

    rfFxPolarize (&r, &t);

    rcAngle8 a8 (rcDegree ((double) (t * rcFeatureMinLength)));
    int32 octant = ((a8.basic() + 16) / 32) % 8;
    dX = dxTable[octant];
    dY = dyTable[octant];
    return a8;
  }
  /* 
   * Return the octant neighbours delta indices.
   */
  

  void polar (double& len, rcRadian& angle) const
  {
    rcFixed16 len16;
    polar (len16, angle);
    len = len16.real();
    len = rmMax (rcFeatureMinLength, len);

  }
    
  rcRadian angle () const
  {
    rcFixed16 len;
    rcRadian angle;

    polar (len, angle);
    return angle;
  }
  /* Note: polar conversion uses Cordic
   * Return the motion vector angle
   */
  float len () const
  {
    return length();
  }
  /* Note: polar conversion uses Cordic
   * Return the motion vector angle
   */

  // Arithmatic and vector operations
  rcFeature operator*(float fm) const;
  friend rcFeature operator*(float fm, const rcFeature& f)
  {
    return f * fm;
  }
    
  rcFeature& operator*=(float fm);

  float operator*(const rcFeature& other) const;
  rcFeature project(const rcFeature& other) const;

  float length () const
  {
    if (mQ >=0)
      return mQ.real ();
    else return 0;
  }

  /*
   * Good enough resolution for graphics
   * TBD
   */
  bool operator== (const rcFeature& r) const {return (*this == r); }
  bool operator!= (const rcFeature& r) const { return !(*this == r); }
  bool operator< (const rcFeature& r) const { return !(mD.x() < r.pos16().x() && mD.y() < r.pos16().y()); }

  friend ostream& operator<< (ostream& o, const rcFeature& f)
  {
    if (f.hasPolar())
      {
	rcRadian d (f.angle());
	float l = f.length();
	o << d.Double() << " Radians, Length:  " << f.len() << " Q: " << l << endl;
      }
    return o;
  }
  
 private:
  rcPair<rcFixed16> mD;
  rcFixed16 mQ;

};

// @todo motionCenter calculation and least square fitting share a lot. Merge

class rcMotionCenter
{
 public:
  rcMotionCenter () : mSumUU (0), mSumVV (0), mSumUV (0), mSumUR (0), mSumVR (0), mValid (false) {}

  void add (const rc2Fvector& pos, const rc2Fvector& motion)
  {
      mValid = false;
      double u = motion.x();
      double v = motion.y();
      double r = pos.x() * u + pos.y() * v;
      mSumUU += u * u;
      mSumVV += v * v;
      mSumUV += u * v;
      mSumUR += u * r;
      mSumVR += v * r;
    }

  bool center (rc2Fvector& center)
  {
    double d = mSumUU * mSumVV - mSumUV * mSumUV;
    mValid = d > 0;
    if (mValid == true)
      {
	center.x((mSumUR * mSumVV - mSumUV * mSumVR)/d);
	center.y((mSumVR * mSumUU - mSumUV * mSumUR)/d);
      }
    return mValid;
  }

 private:
  double mSumUU;
  double mSumVV;
  double mSumUV;
  double mSumUR;
  double mSumVR;
  bool mValid;
};


class rcMotionVectorPath
{
 public:
  rcMotionVectorPath();

  virtual ~rcMotionVectorPath() {}

  rcMotionVectorPath (rcKinetoscope* kin, const rc2Fvector& anchor);
  rcMotionVectorPath (rcKinetoscope* kin, const rc2Fvector& anchor, const rc2Fvector& displacement);

  // Attaches this motion vector path to this kinetoscope
  // Registers this motion path in time. This time is a 
  // relative time from the start of the kinestosope
  
  // Default assignment, copy constructor, destructor are OK

  const rc2Fvector position () const
  {
    return mDxDt + mStartPosition;
  }
  /*! 
    @function position
    @discussion Position with respect to the image from it was created from
  */

  const rc2Fvector& anchor () const { return mStartPosition; }

  const rc2Fvector& anchor (const rc2Fvector& a) 
  { 
    mStartPosition = a;
    return mStartPosition;
  }

  rcIPair iAnchor () const
  {
    return rcIPair (int32 (mStartPosition.x()),
		    int32 (mStartPosition.y()));
  }

  const rc2Fvector& motion () const { return mDxDt; }

  void move (const rc2Fvector& t) 
  { 
    mDxDt = t;
  }

  //TBD: should not need this for identifying we are tracking
  // collection of features and not cells

  virtual void measure ();
  /*
   * measure () performs disparity estimation for this path
   * Measure 
   */

  virtual void update ();
  /*
   * measure () performs disparity estimation for this path
   * Measure 
   */

  virtual void reconcile ();
  /*
   * measure () performs disparity estimation for this path
   * Measure 
   */

  const boost::shared_ptr<rcKinetoscope> sharedKin() const
  {
    return mKine;
  }
  /*
   * @Brief: return the shared pointer to the kinetoscope
   */

  friend ostream& operator<< (ostream&, const rcMotionVectorPath&);

  bool operator< (const rcMotionVectorPath& r) const;
   // note	These declarations are required by the standard template
   //		headers.  The functions are NOT implemented.

 
  private:
  boost::shared_ptr<rcKinetoscope> mKine;
  rc2Fvector mStartPosition;
  rc2Fvector mDxDt;
  rc2Fvector mACF;
  vector<uint8> mSplat;
  void parabolicDisparity (const rcRect&, const rcRect&);

  float match (const rcWindow& fixed, rcWindow& moving, const rcIPair& searchSpace, 
	       rcCorr& maxCorr, rcIPair& maxPos,   vector<vector<float> >& mSpace)
  {
    if (fixed.depth() == rcPixel8)
      return _match_u8(fixed, moving, searchSpace, maxCorr, maxPos, mSpace);
    if (fixed.depth() == rcPixel32)
      return _match_u32(fixed, moving, searchSpace, maxCorr, maxPos, mSpace);
    if (fixed.depth() == rcPixel16)
      return _match_u16(fixed, moving, searchSpace, maxCorr, maxPos, mSpace);
    rmAssert (0);
  }

  float _match_u8(const rcWindow& fixed, rcWindow& moving, const rcIPair& searchSpace, rcCorr& maxCorr, rcIPair& maxPos,   vector<vector<float> >& mSpace);
  float _match_u16(const rcWindow& fixed, rcWindow& moving, const rcIPair& searchSpace, rcCorr& maxCorr, rcIPair& maxPos,   vector<vector<float> >& mSpace);
  float _match_u32(const rcWindow& fixed, rcWindow& moving, const rcIPair& searchSpace, rcCorr& maxCorr, rcIPair& maxPos,   vector<vector<float> >& mSpace);
};



#endif /* __RC_MOTIONPATH_H */
