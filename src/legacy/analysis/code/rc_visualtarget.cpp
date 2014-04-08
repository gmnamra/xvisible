/*
 *
 *$Id $
 *$Log$
 *Revision 1.10  2004/09/13 15:51:06  arman
 *updated
 *
 *Revision 1.9  2004/04/19 22:13:29  arman
 *updates
 *
 *Revision 1.8  2004/02/16 21:50:54  arman
 *updated according to rcFeature changes
 *
 *Revision 1.7  2004/01/18 22:18:13  arman
 *uptodate wrt changes in api
 *
 *Revision 1.6  2004/01/14 21:45:49  arman
 *incremental
 *
 *Revision 1.5  2004/01/14 21:30:29  arman
 *silenced warnings
 *
 *Revision 1.4  2004/01/09 00:05:23  arman
 *tmp ci
 *
 *Revision 1.3  2003/12/09 03:37:49  arman
 *fixed a bug in sizing distances
 *
 *Revision 1.2  2003/12/05 20:14:24  arman
 *fixed a bug in translate. Bead regions will not change size.
 *
 *Revision 1.1  2003/11/25 01:54:56  arman
 *visual structures
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */


//#define TRACKDEBUG
#include <rc_kinetoscope.h>
#include <rc_math.h>
#include <rc_fit.h>
#include <iomanip>
#include <rc_visualtarget.h>

  
rcVisualTarget::rcVisualTarget ( const rc2Fvector& anchor,    const rcFRect& bBox)

{
	mDistance = 0.0;
  mSumPos.x(0.0);
  mSumPos.y(0.0);
  mStartPosition.x(anchor.x());
  mStartPosition.y(anchor.y());
  mCosSums = 0.0f;
  mQuality = 1.0f;

  mRectSnaps.push_back (rcIRect ((int) bBox.ul().x(), (int)bBox.ul().y(), (int) bBox.width(), (int) bBox.height()));
  mRdiffuse = 0.0f;
  mState = eUnInitialized;
  mFixed = bBox;

}

  
  
// Persistence Measurement:
// Accumulated is rms of angular differences between two successive paths
// Persistence is maximum when the rms is at horizontal axis and minimized when it 
// close to the vertical. We use abs (sin) to measure this from rms.






float rcVisualTarget::persistence () const
{
  double p = mCosSums / ((float) (rmMax (1, mSteps)));
  return (p);
}

void rcVisualTarget::measure () 
{
	rmAssert (0);
}

void rcVisualTarget::reconcile () 
{
	rmAssert (0);
}


void rcVisualTarget::update (const rc2Fvector& pos, const double& time, const float& quality)
{
  rcRadian angle;
  double len;

  rcIRect last = mRectSnaps.back();

  if (mState == eIsOutSide) return;

/* Note on Coordinate Transformation:
 * A 2x2 matrix is defined as follows in terms of these variables:
 *   |dX|   |-1  0  |   |Translation in X|
 *   |dY| = |0   -1 | * |Translation in Y|   
 *
 */
//  const rc2Fvector& pos = feature().pos();
  mSumPos -= pos;
  mRdiffuse += (pos.x() * pos.x() + pos.y() * pos.y());

#if 0
  if (feature().hasPolar())
    feature().polar (len, angle);
  else
    {
      len = 0.0;
      angle = mLastAngle;
    }
  mDistance += len;
#endif
 
  // Velocity is instant velocity
//  mV.x (pos.x() / mKine->deltaTime());
//  mV.y (pos.y() / mKine->deltaTime());

  // Calculate dR/dS
  // Adjust region size proportional to correlation match. 
  // Notice that we shrink if correlation is improvign, and expand when 
  // it is not. A consistenly scoring object will oscilate on the edge 
  // of shrink and expand but of a very small amount. 
  float drds = 1. / (log(rmMax (rmABS (mLastQuality - quality), 0.001)));
  drds = (mLastQuality > quality) ? - drds : drds;
  mLastQuality = quality;

  // Translate according to our velocity
  // Note triming a rect by negative expands the rect
  const rcFPair trans (-pos.x(), -pos.y());
  mFixed.translate (trans);
  //  mFixed = mFixed.trim (drds, drds, drds, drds);

  static const int32 dummy (0);

  // Round up ul -- round down lr
  rcIPair ul (rfRoundNeg (mFixed.origin().x(), dummy), 
	      rfRoundNeg (mFixed.origin().y(), dummy));
  rcIPair lr (rfRoundPlus (mFixed.origin().x() + mFixed.width(), dummy),
	      rfRoundPlus (mFixed.origin().y() + mFixed.height(), dummy));
  
   mRectSnaps.push_back (last);

#ifdef TRACKDEBUG
  cout << "Update: " << endl;
  cout << *this << endl;
#endif

}



float rcVisualTarget::speed() const
{
  return sqrt (rmSquare(mV.x()) + rmSquare(mV.y()));
}


ostream& operator<< (ostream& o, const rcVisualTarget& p)
{
  o << "====> Target   : " << p.mId << "<=======" << endl;

  o << "Trace        :" << p.anchor() << endl;
  o << "Area         :" << p.area() << endl;
  o << "Distance     :" << p.distance() << endl;
  o << "Persistance  :" << p.persistence() << endl;
  o << "Anchor       :" << p.anchor() << endl;
  o << "Position     :" << p.position () << endl;
  o << "Velocity     :" << p.velocity() << endl;
  o << "Speed        :" << p.speed() << endl;
  o << "Mean Quality : " << p.rmsQuality() << endl;
#if 0
  deque<rcIRect>::const_iterator rq = p.rectSnaps().begin();
  int q = 0;
  for (; rq != p.rectSnaps().end(); rq++, q++)
    cout << "[" << q << ":" << *rq << "]";
#endif
  return o;
}

