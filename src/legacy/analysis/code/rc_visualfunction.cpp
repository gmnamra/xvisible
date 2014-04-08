/*
 *@file
 *$Id $
 *$Log$
 *Revision 1.6  2006/01/15 22:56:51  arman
 *selective myo
 *
 *Revision 1.5  2006/01/01 21:30:58  arman
 *kinetoscope ut
 *
 *Revision 1.4  2005/11/17 23:19:06  arman
 *kinetoscope resync
 *
 *Revision 1.3  2005/11/08 20:34:29  arman
 *cell lineage iv and bug fixes
 *
 *Revision 1.2  2005/10/31 11:49:04  arman
 *Cell Lineage III
 *
 *Revision 1.1  2005/08/30 21:02:22  arman
 **** empty log message ***
 *
 *Revision 1.20  2005/08/23 23:32:32  arman
 *Cell Lineage II
 *
 *Revision 1.19  2005/08/22 23:54:35  arman
 *Cell Lineage II
 *
 *Revision 1.18  2005/08/19 21:44:16  arman
 *Cell Lineage II
 *
 *Revision 1.17  2005/08/18 02:36:27  arman
 *Cell Lineage II
 *
 *Revision 1.16  2005/08/17 12:45:22  arman
 *Cell Lineage II
 *
 *Revision 1.15  2005/08/15 12:53:56  arman
 *Cell Lineage II
 *
 *Revision 1.14  2005/08/05 21:53:08  arman
 *position issue update
 *
 *Revision 1.13  2005/08/02 13:44:53  arman
 *cell lineage
 *
 *Revision 1.12  2005/08/01 21:18:43  arman
 *cell lineage
 *
 *Revision 1.11  2005/08/01 01:47:25  arman
 *cell lineage addition
 *
 *Revision 1.10  2005/07/31 14:32:21  arman
 *removed deugging info
 *
 *Revision 1.9  2005/07/31 09:56:37  arman
 *cosmetic clearing
 *
 *Revision 1.8  2005/07/31 06:39:16  arman
 *cell lineage incremental
 *
 *Revision 1.7  2005/07/30 23:45:27  arman
 *stateName function and partial implementation
 *
 *Revision 1.6  2005/07/29 21:41:05  arman
 *cell lineage incremental
 *
 *Revision 1.5  2005/07/27 23:40:37  arman
 *Moved rc_mathmodel.h to include dir
 *
 *Revision 1.4  2005/07/22 16:38:13  arman
 *misc
 *
 *Revision 1.3  2005/07/22 15:01:18  arman
 *divided visualcell
 *
 *Revision 1.2  2005/07/22 11:03:17  arman
 *added isoMorphic`
 *
 *Revision 1.1  2005/07/21 22:10:21  arman
 *division functions here
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_analysis.h>
#include <rc_kinetoscope.h>
#include <rc_math.h>
#include <iomanip.h>
#include <algorithm>
#include <rc_lsfit.h>
#include <rc_xforms.h>
#include <rc_affinewindow.h>
#include <rc_shape.h>
#include <rc_lsfit.h>

#include <rc_mathmodel.h>
#include <rc_peak.h>
#include <rc_1dcorr.h>
#include <rc_stats.h>
#include <rc_filter1d.h>

static const int32 sDummy (0);

/// @{/// 
///
/// @class rcVisualFunction
/// @function makeVecMultiple 
///
/// @return rcIRect
///


void rcVisualFunction::initFunction ()
{
  rmAssert (!mHaves.any());
  rmAssert (!mState.any());

  mLastAngle = rcRadian (0.0);
  mId = 0;
  mSize = 0;
  mArea = 0;
  mLabel =  mExpDividedFrameIndex = mEllipseIndex = 0;
  mAffineSize = rcFPair (0.0f, 0.0f);
  mCosSums = 0.0f;
  mSumQuality.reset ();
  mRdiffuse = mPerimeter = mEllipseRatio = mCircularity = 0.0f;
  mDistance = 0.0f;
  mMotionPattern = 0.0f;
  mBeatNumber = 0.0f;
  mDividingTicks = mAnaphase = mNsQ = 0;
//  gNotAvailablePair = rcDPair (gNotAvailable, gNotAvailable);
  mBeatShortenning = 0.0f;
  mMajorEndPoints.resize (2);
}
  
rcVisualFunction::rcVisualFunction (rcKinetoscope* kin, 
				    const rc2Fvector& anch,
				    const rcPolygon& poly, bool isChild)
  : mKine (kin)
{
  rmAssert (!mHaves.any());
  rmAssert (!mState.any());

  initFunction ();
  // Motion is detected using this frame and the previous frame
  // Map using kinetoscopes time unit mapper 
  mGenesis = sharedKin()->secondsToUnits().mapVector (kin->startingAbsoluteTime().secs());
  info (sharedKin()->organism(), sharedKin()->organismName ());

  mEstimatedShortening = gEstSideShortening; // an estimate to start things off
  framesPerPeriod (kin->periodicMotion());
 
  // Cell's registration point is with respect to the orthogonal rect
  // Initial position
  rcFRect fr;
  poly.orthogonalEnclosingRect(fr);
  mMotionCtr = anch;
  anchor (mMotionCtr);
  mRectSnaps.push_back (fr);
  const rc2Xform& xform = sharedKin()->xform();
  mCurrentPosition = xform.mapPoint (anch);

  // Keep a copy of the polygon
  mPOLYs.push_back (poly);

  if (isLabelProtein ())
    createIrradianceModel ();

  setState(eIsChild, isChild);

  // In production sharedKin is ready. In testing this may not be true
  if (sharedKin()->canStep () && (sharedKin()->have (rcKinetoscope::eLineage)))
    {
      initializeSimilarator (mRectSnaps.back());
    } 
  setState (rcVisualFunction::eIsInitialized);

}
  
rcVisualFunction::rcVisualFunction (rcKinetoscope* kin, const rcPolygon& poly,
				    const vector<rc2Fvector>& ends,  const float rad)
  : mKine (kin)
{
  rmAssert (!mHaves.any());
  rmAssert (!mState.any());
  rmAssert (ends.size() == 2);
  mRad = (int32) rad;

  initFunction ();
  // Motion is detected using this frame and the previous frame
  // Map using kinetoscopes time unit mapper 
  mGenesis = sharedKin()->secondsToUnits().mapVector (kin->startingAbsoluteTime().secs());
  info (kin->organism(), kin->organismName ());

  mEstimatedShortening = gEstSideShortening; // an estimate to start things off
  framesPerPeriod (kin->periodicMotion());
 
  // Create a polygon using the ellipse formed by the ends.
  // Rotate and move it to the center of the line contecting the ends. 
  rc2Fvector m = (ends[0] + ends[1]) / 2.0f;
  rc2Fvector d = ends[1] - ends[0];
  if (d.isNull ())
    {
      rmExceptionMacro(<<"Length too short");
    }
  rc2Xform xf;
  static const rcDPair noScale (1.0, 1.0);
  rcMatrix_2d mx (d.angle(), noScale);
  xf.matrix (mx);
  xf.trans (m);

  float a =d.len (); a = a / 2.0f;
  rcPolygon elip (a, mRad, 12);
  elip.transform (xf);

  mMajorEnds = rcFPair (0.0f, a + a);
  for (uint32 i = 0; i < 2; i++) mMajorEndPoints[i] = ends[i];
  setState (eHasFullyExtended);
  mMidPoint = m;
  

  // Now find the union poly of the two 
  // Cell's registration point is with respect to the orthogonal rect
  // Initial position
  rcFRect fr;
  elip.orthogonalEnclosingRect(fr);
  mMotionCtr = m;
  anchor (mMotionCtr);
  mRectSnaps.push_back (fr);
  const rc2Xform& xform = sharedKin()->xform();
  mCurrentPosition = xform.mapPoint (m);

  // Keep a copy of the polygon
  mPOLYs.push_back (elip);
  mAffys.push_back (elip.minimumPerimeterEnclosingRect());

  // In production sharedKin is ready. In testing this may not be true
  // @note: should this be on for cardiac ? 
  if (sharedKin()->canStep () && (sharedKin()->have (rcKinetoscope::eLineage)))
    {
      initializeSimilarator (mRectSnaps.back());
    }      
  setState (rcVisualFunction::eIsInitialized);

}

void rcVisualFunction::createIrradianceModel ()
{
  // Create a Gauss Model. Currently only for catching daughters after mitosis
  // Default is a gaussian with std of 1.0
  static const int32 dummy (0);
  int32 d = rfRound (sqrt (mRectSnaps.back().area() / rkPI), dummy);
  d = rmMax (16, d);
  rcPixel depth (rcPixel8);
  rcWindow tmp (d, d, depth);
  mIradModel.gauss (tmp, rcMathematicalImage::eHiPeak);
  mGaussModel = rcCorrelationWindow<uint8> (tmp);
}

void rcVisualFunction::initializeSimilarator (const rcFRect& rect)
{
  uint32 temporalSimilarityWindowSize (3);
  if (isCardiacCell ())
    (uint32) (sharedKin()->periodicMotion() / 2.0f);
  ssWindowSize ( temporalSimilarityWindowSize);
  mCoarseSimRef = boost::shared_ptr<rcSimilarator> (new rcSimilarator (rcSimilarator::eExhaustive,
																																			 rcPixel8, temporalSimilarityWindowSize , 0));
  mCurrentSimRect = rect;
  setState (eHasSimilarator);
  updateSims (use4fixed(), mCurrentSimRect);
}
  
/*  @bug Does not handle cell drift and size change
    @todo 
    @Description: Updates Similarators
    Rectangle processing for Updating Sims:
    Best registered and identical size rect. 
*/

bool rcVisualFunction::updateSims (const rcWindow& win, const rcFRect& rect)
{
  rmAssert (isState (eHasSimilarator));
  clear (eCoarseSS);

  rcFRect pm, fm; rcIRect bm, im;
  static const rcFPair stationary (0.0f, 0.0f);
  static const bool doClip (true);

  // Get the current and estimated rect positons, window the est and setup mask if size has changed
  // Note that the current rect might have been outside itself. We check the inteded size with estiamted
  // window size, and act on clipping done or not.
  // rcFPair ds = rect.size() - mCurrentSimRect.size(); 
  //rcFPair dp = rect.ul() - mCurrentSimRect.ul();

  updateRects (rect, stationary, fm, im);
  updateRects (mCurrentSimRect, stationary, pm, bm);
  rcWindow tmp (win, im, doClip);

  if (tmp.size () == bm.size() && tmp.size() == im.size() && im.size() == mCoarseSimRef->fillImageSize() && 
      (mCoarseSimRef->update (tmp) || steps() < mCoarseSimRef->matrixSz()))
    {
      mark(eCoarseSS);
      mCurrentSimRect = fm;
      return true;
    }
  else
    {

      rcWindow content (win, im.ul().x(), im.ul().y(), rmMin (tmp.width(), bm.width()),  rmMin (tmp.height(), bm.height()));
      rcWindow extended (bm.width(), bm.height());
      rcWindow masked (bm.width(), bm.height());
      extended.setAllPixels (0);
      masked.setAllPixels (255);
      rcWindow exw (extended, content.width(), content.height());
      rcWindow mw (extended, content.width(), content.height());
      exw.copyPixelsFromWindow (content);
      mw.setAllPixels (0);

      if (extended.size() == bm.size() && masked.size() == bm.size())
	{
	  mCoarseSimRef->clearMask ();
	  mCoarseSimRef->setMask (masked);
	  if (mCoarseSimRef->update (extended) || steps() < mCoarseSimRef->matrixSz())
	    {
	      mark(eCoarseSS);
	      mCurrentSimRect = fm;
	      return true;
	    }
	}
    }

  // @note for now only one similarator
  return false;
}

int32 rcVisualFunction::ssWindowSize (int32 ws)
{
  mSSwindowSize = ws;
  return mSSwindowSize;
}

void rcVisualFunction::info (rcOrganismInfo::OrganismType c, rcOrganismInfo::OrganismName n)
{
  mOtype = c;
  mOname = n;
}

float rcVisualFunction::framesPerPeriod (float p)
{
  clearState (eIsPeriodic);
  mFramesPerPeriod = p;
  if (p > 0.0) setState (eIsPeriodic);
  mIntegralFramesPerPeriod = rfRoundPlus (framesPerPeriod (), sDummy);

  // Use a window (up adjusted to power of 2) of two periods to estimate contraction frequency
  frexp (p+p, &mFramesPerPeriodPowerOf2);
  mFramesPerPeriodPowerOf2 = (int32) ldexp (1.0f, mFramesPerPeriodPowerOf2);
  mBeatAC.resize (mFramesPerPeriodPowerOf2);
  return p;
}

bool rcVisualFunction::have (enum Haves what) const
{
  return mHaves[what];
}

bool rcVisualFunction::mark (enum Haves what) 
{
  mHaves.set(what);
  return have (what);
}

bool rcVisualFunction::clear (enum Haves what) 
{
  mHaves.reset(what);
  return have (what);
}

void rcVisualFunction::reset ()
{
  mHaves.reset ();
}

const rcWindow& rcVisualFunction::use4moving () const 
{
  return sharedKin()->use4moving();
}

const rcWindow& rcVisualFunction::use4fixed () const 
{
  return sharedKin()->use4fixed();
}

double rcVisualFunction::computeAcceleration () 
{
	mAcceleration = 0.0; //gInValidAcceleration;
  clear (eAcceleration);
  if (have (eCoarseSS))
    {
      deque<double> signal;
      bool hasSignal = mCoarseSimRef->entropies (signal);
      
      if (hasSignal && signal.size())
	{
	  rmAssert (signal.size () == mCoarseSimRef->matrixSz ());
	  mSS.push_back (signal[mCoarseSimRef->matrixSz () / 2]);

	  if (mSS.size() > (uint32) ssWindowSize ())
	    {
	      mAcceleration = log10(mSS[mSS.size() - 1] / mSS[mSS.size() - 2]);
	      mAcl.push_back ((float) acceleration ());
	      mark (eAcceleration);
	    }
	}
    }
  return mAcceleration;
}


float rcVisualFunction::speed() const
{
  const rc2Xform& xform = sharedKin()->xform();
  rc2Fvector s = xform.mapPoint (mVt);
  if (!s.isNull ()) return s.len ();
}

rcVisualFunction::State rcVisualFunction::state () const
{
  if(isState(eIsUnInitialized)) return   eIsUnInitialized;
  if(isState(eIsUnknown)) return   eIsUnknown;
  if(isState(eIsOutSide)) return   eIsOutSide;
  if(isState(eIsBeating)) return   eIsBeating;
  if(isState(eIsMoving)) return   eIsMoving;
  if(isState(eIsDividing)) return   eIsDividing;
  if(isState(eIsDivided)) return   eIsDivided;
}

rcVisualFunction::State rcVisualFunction::fineState () const
{
  if(isState(eIsDirected)) return   eIsDirected;
  if(isState(eIsJustDivided)) return   eIsJustDivided;
  if(isState(eIsFragmented)) return   eIsFragmented;
  if(isState(eIsParent)) return   eIsParent;
  if(isState(eIsChild)) return   eIsChild;
  if(isState(eIsRounding)) return   eIsRounding;
  if(isState(eIsRounded)) return   eIsRounded;
}

bool rfCellNotViable (const rcVisualFunction& vf)
{
  return (vf.isState (rcVisualFunction::eIsDivided) || vf.isState(rcVisualFunction::eIsOutSide) 
	  || vf.isState (rcVisualFunction::eIsUnInitialized));
}

// Get cell state name string
//@todo clean this up!

void rfVisualFunctionStateName( rcVisualFunction::State state,
                       char* valueBuf,
                       uint32 bufSize )
{
    switch ( state ) {
        case rcVisualFunction::eIsUnInitialized:
            snprintf( valueBuf, bufSize, "Uninitialized");
            return;
        case rcVisualFunction::eIsMoving:
            snprintf( valueBuf, bufSize, "Moving");
            return;
        case rcVisualFunction::eIsDirected:
            snprintf( valueBuf, bufSize, "Directed");
            return;
        case rcVisualFunction::eIsJustDivided:
	  snprintf( valueBuf, bufSize, "JustDivided");
            return;
        case rcVisualFunction::eIsDividing:
            snprintf( valueBuf, bufSize, "Dividing");
            return;
        case rcVisualFunction::eIsDivided:
            snprintf( valueBuf, bufSize, "Divided");
            return;
        case rcVisualFunction::eIsFragmented:
            snprintf( valueBuf, bufSize, "Fragmented");
            return;
        case rcVisualFunction::eIsOutSide:
            snprintf( valueBuf, bufSize, "Outside");
            return;
        case rcVisualFunction::eIsUnknown:
            snprintf( valueBuf, bufSize, "Unknown");
            return;
        case rcVisualFunction::eIsBeating:
            snprintf( valueBuf, bufSize, "Beating");
            return;
        case rcVisualFunction::eIsParent:
            snprintf( valueBuf, bufSize, "Parent");
            return;


    }
    // Default case
    snprintf( valueBuf, bufSize, "Unknown state %i", state );
}

ostream& operator<< (ostream& o, const rcVisualFunction::State& st)
{
  char buf[64];
  rfVisualFunctionStateName(st, buf, 64);
  o << buf;
  return o;
}

  
ostream& operator<< (ostream& o, const rcVisualFunction& p)
{
  string s (" States: \n");
  if(p.isState(rcVisualFunction::eIsUnInitialized)) s +=  string ("+eUnInitialized\n");
  if(p.isState(rcVisualFunction::eIsUnknown)) s +=  string ("+eIsUnknown\n");
  if(p.isState(rcVisualFunction::eIsMoving)) s +=  string ("+eIsMoving\n");
  if(p.isState(rcVisualFunction::eIsDirected)) s +=  string ("+eIsDirected\n");
  if(p.isState(rcVisualFunction::eIsJustDivided)) s +=  string ("+eIsJustDivided\n");
  if(p.isState(rcVisualFunction::eIsDividing)) s +=  string ("+eIsDividing\n");
  if(p.isState(rcVisualFunction::eIsDivided)) s +=  string ("+eIsDivided\n");
  if(p.isState(rcVisualFunction::eIsFragmented)) s +=  string ("+eIsFragmented\n");
  if(p.isState(rcVisualFunction::eIsOutSide)) s +=  string ("+eIsOutSide\n");
  if(p.isState(rcVisualFunction::eIsBeating)) s +=  string ("+eIsBeating\n");
  if(p.isState(rcVisualFunction::eIsParent)) s +=  string ("+eIsParent\n");
  if(p.isState(rcVisualFunction::eIsChild)) s +=  string ("+eIsChild\n");
  if(p.isState(rcVisualFunction::eIsRounding)) s +=  string ("+eIsRounding\n");
  if(p.isState(rcVisualFunction::eIsRounded)) s +=  string ("+eIsRounded\n");

  string c (" Type: \n");
  if (p.isCardiacCell()) c += string (" Cardiac \n");
  else if (p.isLabelProtein ()) c += string (" Label Protein \n");  
  else c += string (" General \n");

  o << endl;
  o << c << " Id: " << p.id() << " Parent: " << p.parentId() << endl;
  o << s << "[" << p.mSteps << "]: ";

  o << "Trace: " << p.motion() << "Area: " << p.area() << endl;
  o << "Dist:  " << p.distance() << "Persistance  : " << p.persistence() << "CEP : " << 
    p.circularity () <<  "," << p.ellipseRatio() << "," << p.perimeter () << endl;
  o << "Age :" << p.age() << "Anchor: " << p.anchor() << "Position: " << p.position () << endl;
  o << "Velocity: " << p.velocity() << "Speed: " << p.speed() << endl;

  if (p.isCardiacCell ())
    {
      if ( p.have( rcVisualFunction::eContraction ) ) 
	o << "Length: " << p.dimensions().x();

      if ( p.have( rcVisualFunction::ePacingInfo) )
	o << "Pacing Freq: " << p.contractionFrequency ();

      if ( p.have( rcVisualFunction::eBeatInfo) )
	o << "Shortening: " << p.shortenning();
    }

  if (p.mSS.size()) 
    {
 //     o << p.mSS << endl;
    }

  return o;
}
