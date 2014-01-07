/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.3  2004/04/27 20:29:36  arman
 *removed FLine
 *
 *Revision 1.2  2003/12/05 19:43:03  arman
 *first working version.
 *
 *Revision 1.1  2003/11/25 01:52:40  arman
 *visual structures
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcVISUALSTRUCTURE_H
#define __rcVISUALSTRUCTURE_H



#include <rc_types.h>
#include <rc_rectangle.h>
#include <rc_window.h>
#include <vector>
#include <rc_motionpath.h>
#include <rc_rlewindow.h>
#include <rc_line.h>
#include <rc_vector2d.h>
#include <rc_platform.h>

/*
 * A VisualStructure is created from a single target location 
 */

class RFY_API  rcVisualTarget : public rcBaseMotionPath
{
  static const float inValidOrigin = rcFLT_MIN;
  static const float inValidDispersion = rcFLT_MIN;

 public:

  rcVisualTarget ();

  rcVisualTarget (const rc2Fvector& anchor, const rcFRect& bBox);

  // Attaches this motion vector path to this kinetoscope
  // Registers this motion path in time. This time is a 
  // relative time from the start of the kinestosope
  // minIntersecting and minEnclosng rectangles are initialized
  // to the Kinetoscopes frame
  // Default assignment, copy constructor, destructor are OK

  virtual ~rcVisualTarget () {}

  //  rcVisualTarget(const rcVisualTarget& rhs);
  //  rcVisualTarget& operator=(const rcVisualTarget& rhs);


  uint32 id() const {return mId; }
  /*
    effect: returns the id of this cell
  */

  // Note: Units: all in calibarated units 
  // Accessors

  float area () const
  {
    return mArea;
  }
  /*
    effect: return area of this well
    Requires initial calculation
  */

  float rmsQuality () const
  {
    return mQuality;
  }
  /*
    effect: return rms of Quality
    Requires initial calculation
  */

  float distance () const
  {
    return mDistance;
  }
  /*
    effect: returns Distance in pixels
    Requires initial calculation
  */

  const rc2Fvector& velocity () const
  {
    return mV;
  }
  /*
    effect: return the horizontal and vertical velocity
    Requires whole cell tracking
  */
  float speed () const;
  /*
    effect: return speed of the motion center of this cell
    Requires velocity
  */

  float persistence () const;
  /*
    effect: returns persistence of this cell.
    Requires all and can happen after 2 rounds
  */

   /*
    effect: returns persistence of cell movements towards me
    Requires all and can happen after 2 rounds
  */

  float minSqDisplacement () const
  {
    return mRdiffuse;
  }
  /*
    effect: return speed of the motion center of this cell
    Requires velocity
  */

 
  enum Lines
  {
    eVectors,
    eBoundingBox,
    eBoundingPoly,
    eCurl,
    eRelief
  };

    
  enum State
  {
    eUnInitialized=0,
    eIsMoving,
    eIsDirected,
    eIsRandom,
    eIsOutSide
  };

  enum Measure
  {
    eHasTemporal = 0,
    eVelocity,
    eSpeed,
    edPersistence
  };

  enum ChainDirection
  {
    eMinusY,			// up
    ePlusY,			// down
    eMinusX,			// left
    ePlusX=0,			// right
    eEnd
  };

  /*
   * effect: calculates and returns the measurement indicated
   */

  State state () const {return mState; }
  /*
    effect: returns the current state of this cell
  */

  const int32 history () const {return mSize; }
  /*
    effect: returns archival length. This is equal to the 
    size of all deques returning images,rect, or RLEs
  */

  const deque<rcWindow>& snaps () const {return mSnaps; } 
  /*
    effect: returns the rcWindow representing this cell
    at this time. The first rcWindow in the vector is the oldest.
    TBD: pointer, reference, or
  */

  const deque<rcIRect>& rectSnaps() const {return mRectSnaps; }
  /*
    effect: returns the vector of rcWindows representing this cell
    over time. The first rcWindow in the vector is the oldest.
    TBD: pointer, reference, or
  */

  const rcIRect& minTemporalEnclosingRect () const {return mMinTemporalEncRect; }
  /*
    effect: returns the rect that so far encloses this cells whereabouts.
  */


  const rcIRect& minTemporalIntersectingRect () const {return mMinTemporalIntRect; }
  /*
    effect: returns the rect that is the intersection of all rects of this cell
  */

  virtual void measure (); 
  virtual void reconcile  (); 
	
  virtual void update (const rc2Fvector&, const double& time, const float& quality );
  /*
   * updates tracking information
   */

  

  enum Haves
  {
    eHaveNothing = 0,
    ekeepCurrentOnly,
    eKeepImages
  };

  friend ostream& operator<< (ostream&, const rcVisualTarget&);

  // required for STL
  bool operator<(const rcVisualTarget& rhs) const { return mId < rhs.mId; };
  bool operator==(const rcVisualTarget& rhs) const { return mId == rhs.mId; };

 private:
  State mState;
  uint32 mId;
  int32 mSize;
  float mArea;

  float mCosSums;
  float mQuality;
  float mLastQuality;
  rcRadian mLastAngle;
  float mRdiffuse;

  deque<rcWindow> mSnaps;
  deque<rcIRect> mRectSnaps;
  rcIRect mMinTemporalEncRect;
  rcIRect mMinTemporalIntRect;
  double mDistance;
  rcFRect mFixed;
  float mMotionPattern;

  // These quantities should be cached
  rc2Fvector mV;
  
};




#endif /* __rcVISUALSTRUCTURE_H */
