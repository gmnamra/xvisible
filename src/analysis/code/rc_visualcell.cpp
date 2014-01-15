/* @file
 *
 *$Id $
 *$Log$
 *Revision 1.149  2006/01/10 23:38:08  arman
 *pre rel2
 *
 *Revision 1.148  2006/01/01 21:30:58  arman
 *kinetoscope ut
 *
 *Revision 1.147  2005/09/10 19:39:26  arman
 *2.0 Pre
 *
 *Revision 1.146  2005/08/30 20:57:16  arman
 *Cell Lineage
 *
 *Revision 1.170  2005/08/23 23:32:32  arman
 *Cell Lineage II
 *
 *Revision 1.169  2005/08/22 14:01:49  arman
 *added PolyCell cross matrix. Updated setting divided state
 *
 *Revision 1.168  2005/08/19 21:44:16  arman
 *Cell Lineage II
 *
 *Revision 1.167  2005/08/18 02:36:27  arman
 *Cell Lineage II
 *
 *Revision 1.166  2005/08/16 22:23:39  arman
 *Cell Lineage II
 *
 *Revision 1.165  2005/08/15 12:53:55  arman
 *Cell Lineage II
 *
 *Revision 1.164  2005/08/12 20:37:54  arman
 *inc cell lineage plus
 *
 *Revision 1.163  2005/08/05 21:53:08  arman
 *position issue update
 *
 *Revision 1.162  2005/08/04 02:33:10  arman
 *switched to framewise com calculation. There was 60 ms profile cost to
 *the call. It is now down to 1 msec!
 *
 *Revision 1.161  2005/08/01 21:18:43  arman
 *cell lineage
 *
 *Revision 1.160  2005/08/01 12:20:11  arman
 *cell lineage
 *
 *Revision 1.159  2005/08/01 04:12:14  arman
 *cell lineage
 *
 *Revision 1.158  2005/08/01 04:01:37  arman
 *cell lineage
 *
 *Revision 1.157  2005/08/01 01:47:25  arman
 *cell lineage addition
 *
 *Revision 1.156  2005/07/31 14:33:23  arman
 *first working version of cell lineage.
 *
 *Revision 1.155  2005/07/31 12:46:38  arman
 *fixed the position and hopping bugs
 *
 *Revision 1.154  2005/07/31 10:07:38  arman
 *commented out debuggin information
 *
 *Revision 1.153  2005/07/31 06:39:16  arman
 *cell lineage incremental
 *
 *Revision 1.152  2005/07/29 21:41:05  arman
 *cell lineage incremental
 *
 *Revision 1.151  2005/07/27 23:40:37  arman
 *Moved rc_mathmodel.h to include dir
 *
 *Revision 1.150  2005/07/23 21:46:43  arman
 *incremental
 *
 *Revision 1.149  2005/07/22 22:04:39  arman
 *incremental
 *
 *Revision 1.148  2005/07/22 16:38:41  arman
 *moved predict here.
 *
 *Revision 1.147  2005/07/22 15:00:35  arman
 *divided visualcell
 *
 *Revision 1.146  2005/07/21 22:09:50  arman
 *incremental
 *
 *
 *Revision 1.38  2003/11/25 01:54:27  arman
 *Major rework:
 *. new correspondence / measure
 *. new reconcile / dispersion
 *. floating point regions
 *
 *Revision 1.37  2003/11/12 19:22:27  arman
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
#include <rc_macro.h>
#include <rc_timinginfo.h>

//#define TRACKDEBUG
//#define MINDEBUG

#ifdef TRACKDEBUG
#include <rc_gnuplot.h>
#endif


/*	******************************
	*                            *
	*     predicteTolerance      *
	*                            *
	******************************

    @Description: Returns tolerance predicted from past quality
    @todo: switch to use kalman filtering
*/

const rcFPair rcVisualFunction::predictedTolerance (rc2Fvector& dD) const
{
  float quality = cQuality ();
 // quality = (quality == gNotAvailable) ? 0.5f : quality;
  float ab = adaptBias ();
  const rcFPair minTolerance (ab / mRectSnaps.back().width(), ab / mRectSnaps.back().height());

  // Do not use below 0.5 quality (unlikely from correspondence)
  float adapt rmMax ((1.0f / quality ), 1.0f);
  rcFPair tolerance = isAtStep (1) ? minTolerance :  rcFPair (adapt, adapt) * minTolerance;
  tolerance.x() = rmMax (rmABS(dD.x()) / mRectSnaps.back().width(), tolerance.x());
  tolerance.y() = rmMax (rmABS(dD.y()) / mRectSnaps.back().height(), tolerance.y());
  return minTolerance;
}

/** 
 *     R e c o n c i l e      
 *                            
 *     
 * 
 */
void rcVisualFunction::reconcile  ()
{
	mMotionPattern = 0.0; // gInValidDispersion;
  clear (eDispersion);

  if (rfCellNotViable (*this)) return;

  // Beat Processing For CardioMyocytes
  // starts after phase # of frames
  if (isCardiacCell () && have (eDimensions))
    {
      beatProcess ();
    }
  else if (sharedKin()->have (rcKinetoscope::eLineage))
    {
     if (isState(eIsDividing))
       {
	 bool divided = isState(eIsJustDivided);
	 setState (eIsDivided, divided);
	 setState (eIsDividing, !divided);
	 if (!divided)
	   {
	     mDividingTicks--;
	     setState (eIsDividing, mDividingTicks > 0);
	   }

	 if(divided)
	   {
	     mExpDividedFrameIndex = sharedKin()->moving().frameBuf()->frameIndex();
	   }
       }
    }
  else
    {
      populationMigration ();
    }
}


void rcVisualFunction::populationMigration  ()
{
  clear (eDispersion);
  if (sharedKin()->mFunctionMap.size() < 2) return;
  
  // else measure population migration 
  list<rcVisualFunction>::const_iterator cell;
  vector<float> cellDiffusion;
  vector<float> distances;

  distances.reserve (sharedKin()->mFunctionMap.size() - 1);
  cellDiffusion.reserve (sharedKin()->mFunctionMap.size() - 1);

  for(cell = sharedKin()->mFunctionMap.begin(); cell != sharedKin()->mFunctionMap.end(); cell++)
    {
      // Skip me
      if (cell->id() == id()) continue;

      // Vector from me to another cell
      rc2Dvector c2c (position().x() - cell->position().x(), 
		      position().y() - cell->position().y());

      // Distance from me to another cell      
      float f = position().distance (cell->position());
      distances.push_back (f);

      // My velocity towards the other cell (dot product)
      rc2Dvector vd (velocity().x(), velocity().y());
      cellDiffusion.push_back (c2c * vd);
    }

  // Now get min and max distances
  vector<float>::const_iterator mind = min_element (distances.begin(),
						    distances.end());
  vector<float>::const_iterator maxd = max_element (distances.begin(),
						    distances.end());
  // If all cells are attached to me, we are undetermined
  if (real_equal (*maxd, 0.0f)) return;

  // Find min and max distances; if there is no range, pick max
  float scale (*maxd - *mind);
  if (real_equal (scale, 0.0f)) scale = *maxd;

  // Sum persistence towards me or away from me scale pro closest to me.
  // Use a linear map to use distance
  float dispersion (0.0f);
  for (uint32 i = 0; i < distances.size(); i++)
    dispersion += (distances[i] * cellDiffusion[i]) / scale;
  
  mMotionPattern = dispersion;
  mark (eDispersion);
}


// Persistence Measurement:
// Accumulated is rms of angular differences between two successive paths
// Persistence is maximum when the rms is at horizontal axis and minimized when it 
// close to the vertical. We use abs (sin) to measure this from rms.

float rcVisualFunction::persistence () const
{
  double p = mCosSums / ((float) (rmMax (1, steps())));
  return (p);
}


/*	****************************************************
	*                                                  *
	*     LeastSquare Fitting of Motion Vectors        *
	*                                                  *
	****************************************************
	@note: Fitting return transformation from moved to 
	fixed (t+1 -> t. Given the temporal ordering (t -> t+1), we
	use the inverse to get (t -> t+1)


	@bug 

	@todo: cache results

*/

bool rcVisualFunction::vXform (DOF dofoptions, rc2Xform& xform)
{
  // return identity if anything goes wron
  xform = rc2Xform ();

  if (dofoptions == eNone)
    return true;

  int32 i (0);
  rcLsFit<float> lsm;
  rcMotionCenter rcm;

  vector<rc2Fvector>::const_iterator anc = sharedKin()->mVanchor[mLabel].begin();
  vector<rc2Fvector>::const_iterator mov = sharedKin()->mVmoved[mLabel].begin();
  vector<rc2Fvector>::const_iterator ancend = sharedKin()->mVanchor[mLabel].end();
  vector<rc2Fvector>::const_iterator movend = sharedKin()->mVmoved[mLabel].end();

  const rc2Fvector vfix = rc2Fvector (mRectSnaps.back().ul().x(), mRectSnaps.back().ul().y());  
  // Motion Vector are wrt kinetoscope frames. translate to the ul of fixed
  for (i = 0; anc != ancend && mov != movend; anc++, mov++, i++)
    {
      rc2Fvector dM = *mov - *anc;
      rc2Fvector af = *anc - vfix;
      rcm.add (af, dM);
      if (dofoptions == eMotionCenterANDTP)
	{
	  rc2Fvector mf = *mov - vfix;
	  lsm.update (af, mf);
	}
    }

  // Computer Motion Center

  bool mHaveMotionCenter = rcm.center (mMotionCtr);
  
  if (dofoptions == eMotionCenter)
    {
      if (mHaveMotionCenter)
	{
	  mark (eMotion);
	  return true;
	}
    }
  
  // Update lsfit api directly to xform
  bool success;
  rc2Xform xf;
  
  success = lsm.solvetp (xf);
  
  success = mHaveMotionCenter && success;

  if (success)
    {
      xform = xf.inverse (); // see note above
      mark (eMotion);
      mark (eScale);
    }

  return success;

}




/*	******************************
	*                            *
	*     M e a s u r e          * 
	*                            *
	******************************
    @bug Handle case when motion vector registration fails and polygon
         center is used instead. In particular what registration point 
	 it is compared against.

    @todo 
    @Description: Returns if there are no RLEs. Measures motion.
*/

void rcVisualFunction::measure ()
{
  step();

  // Invalidate all measurements before we begin
  reset ();
  
  if (sharedKin()->movingPolygons().size() < 1) return;
  //  sharedKin()->dumpPolys (cerr);

  //@note allow unknown. 
  if (rfCellNotViable (*this)) return;
  static const rc2Fvector tinyMove (2.0f, 2.0f);
 
  // Get the associated Segmented Poly and most recent poly. Use wc as variable
  vector<rcPolygon>::const_iterator pc = polygons().end(); std::advance (pc, -1);
  vector<rcPolygon>::const_iterator vc = sharedKin()->movingPolygons().begin() + label();
  vector<rcPolygon>::const_iterator wc;

  // If we have a reasonable correspondence use it
  // Otherwise use the updated rect from last time
  rcTimingInfo profile (10, sharedKin()->frameCount() + 1);
  profile.nextPass (true);

  if (vc != sharedKin()->movingPolygons().end() && vc->isValid())
    {
      // We do not need this one around
      //@note we use two different ShapeRefs here. For calculating center of mass we use the
      //  the orthogonal window. For shape we use the segmented polygon. 
      rcFRect fr;rcIRect ir;
      vc->orthogonalEnclosingRect(fr);
      ir = roundRectangle (fr);
      rcWindow frame (use4moving(), ir);
      rcShapeRef frameRef = boost::shared_ptr<rcShape> (new rcShape (frame));
      rc2Fvector dD = frameRef->centerOfmass (rcShape::eImage);

      //@note this is poorely placed here. Change. 
      mMass = (float) frameRef->mass ();

      dD += rc2Fvector (fr.ul());
      profile.touch (1);
      if (!isCardiacCell () && !isLabelProtein ())
	{
	  // @note: if we have motion vectors, compute transformation
	  if (vXform (eMotionCenterANDTP, mMotionXform))
	    {
	      dD = mMotionXform.transf ();
	    }

	  rcFPair tolerance = predictedTolerance (dD);
	  correspond (dD, tolerance);
	  mPOLYs.push_back (*vc); 
	  mark (eMotion);
	  mark (eDimensions);	 
	}
      else if (isCardiacCell ())
	{
	  cardiacMeasure (vc);
	}	  
      else if (isLabelProtein ())
	{
	  // @note
	  // dD is com of the most correponding region. 
	  // mCurrentPosition is known position before this measure. 
	  // Immediately after division, there might not be separate 
	  // regions corresponding to each child. 
	  // So a gaussian model is used to track the daughters until 
	  // they form a isomorphic (one to one) correspondence with a region.
	  // The first time they do, daughters are updated to the 
	  // region and  the cell is moved  to the com  of the region. 

	  float distance = dD.distance (mCurrentPosition);
	  float radius = sqrt (area() / rkPI) * 1.5f;

	  if (real_equal (cQuality (), 1.0f, 0.001f) && 
	      real_equal (circularity (), (float) vc->circularity(), 0.067f) &&
	      distance < radius)
	    {
	      direct (dD);	      
	      // @note we are accepting the perfect correspondence
	      wc = vc;
	    }
	  else
	    {
	      dD = rc2Fvector (adaptBias (), adaptBias ());
	      rcFPair tolerance = predictedTolerance (dD);
	      profile.touch (0);
	      if (sharedKin ()->channelAvailable ())
		correspondMutualChannel (dD, tolerance);
	      else
		correspond (dD, tolerance);
	      // @note we are moving the last poly forward
	//			pc->transform (instantTransformation ()); 
	      wc = pc;
	      profile.touch(1);
	    }


	  mPOLYs.push_back (*wc);
	  mark (eMotion);
	  mark (eDimensions);
	}

      else
	{
	  rmAssert (0);
	}
    }
  vector<std::string> lables;
  lables.push_back (std::string ("Correspondence"));
  profile.printInfo (2, lables);
}



/*	******************************
	*                            *
	*     U p d a t e            *
	*                            *
	******************************
    @bug Rectangle update causing assertion failure in line 885
    @todo 
    @Description: Returns if there are no RLEs. Updates Motion
*/

void rcVisualFunction::update ()
{
  const rcFRect& fpframe = sharedKin()->fpframe ();

  // If we are dead or mostly dead skip
  if (rfCellNotViable (*this)) return;
  if (! have (eDimensions) || ! have (eMotion) ) return;

  // Check on what measure left us with
  rcFRect lastPolyRect, last = mRectSnaps.back();
  if (isState (eHasSimilarator) && ! fpframe.contains (last)) return;

  rcPolygon lastPoly = mPOLYs.back ();
  lastPoly.orthogonalEnclosingRect(lastPolyRect);
  if (! fpframe.contains (lastPolyRect)) return;  

 // Set our state based of what we have been through
  setState (eIsMoving, have (eMotion) && have(eDimensions));

  // 1. Update Similarators
  if ((isCardiacCell () || isLabelProtein ()) && (isState (eHasSimilarator)))
    {
      updateSims (use4fixed(), last);
    }

  if (isState (eHasSimilarator) && sharedKin()->have (rcKinetoscope::eLineage))
    {
      computeAcceleration ();
      if (acceleration () > 1.0)
	{
	  setState (eIsDividing);
	  // @note: do we need a limit of how long we wait for mitosis.
	  // Also, how does this interact with checking for apoptosis
	  mDividingTicks =10;
	}
    }

  // Update Work on where we are
  // @note: redesign. also expensive polygon copy
  mArea = lastPoly.area ();
  mPerimeter = lastPoly.perimeter ();
  mCircularity = lastPoly.circularity ();
  mEllipseRatio = lastPoly.ellipseRatio ();
  mark (eGeometry);
 


  // @todo centralize and use time
  mNsQ += rmSquare (steps());

  mRectSnaps.push_back (moving());

  // Get the just moved to pose (translation only for now)
  // @description: This is how much fixed is moved to match moving
  rc2Fvector pos = mXform.transf();

  /*
   * If we have moved in space and time
   * -> make sure we have moved a minimum since last frame
   *    and that we are at least a minim passed dob.
   */
  const double minTimeDelta = sharedKin()->secondsToUnits().invMapPoint (1.0f);

  if (have (eMotion) && sharedKin()->deltaTime () >= minTimeDelta &&
      (sharedKin()->currentTime() - dob()) >= minTimeDelta)
    {
      rcRadian angle (mLastAngle);
      double len (0.0);

      move (pos); // now move to where we are. 
      mRdiffuse += (pos.x() * pos.x() + pos.y() * pos.y());
      if (!motion().isNull ())
	{
	  len = motion().len ();
	  angle = motion().angle();
	}
      mDistance += len;
      
      // Measure rms of direction changes
      rcRadian bsd = angle - mLastAngle;
      mCosSums += cos (bsd);
      mLastAngle = angle;

      // Velocity is instant velocityß
      mV = motion() / age ();
      mVt = travelled() / (float) sharedKin()->elapsedTime();

      // Compute current position
      const rc2Xform& xform = sharedKin()->xform();
      mCurrentPosition = xform.mapPoint (anchor() + travelled ());

      if (!isAtStep (1))
	{
	  mLastAngle = angle;
	  mark (ePersistence);
	}

      mark (eSpeed);
      mark (eVelocity);
      mark (eDistance);
    }

#ifdef MINDEBUG
  cout << *this << endl;
#endif

}


/*  @note This function does not alter any state variables
    @Description: setup moving rect frome fixed rectangle
*/

void rcVisualFunction::updateRects (const rcFRect& fixed, const rcFPair& tol, 
				    rcFRect& movingf, rcIRect& moving) const
{
   rcFRect fMoving (fixed);

   fMoving = fMoving.trim (-tol.x() * fMoving.width(),
		 -tol.x() * fMoving.width(),
		 -tol.y() * fMoving.height(),
		 -tol.y() * fMoving.height());

   // Adjust to make sure it fits in the processing frame
   // @todo accessor to a float rect for the kinetoscope rects
   // @todo multi-resolution implementation needs to project rects
   // @todo from low res to high res
   const rcIRect& pframe = sharedKin()->iframe ();
   const rcFRect& fpframe = sharedKin()->fpframe ();

   // @todo move only in the direction needed
   rcFPair ud, d;
   ud.x() = rmMax (fpframe.ul().x(), fMoving.ul().x());
   ud.y() = rmMax (fpframe.ul().y(), fMoving.ul().y());
   
   // @todo adjust size only in the direction needed
   // @note trim is move out the ul and move in the lr
   d.x() = rmMin (fpframe.lr().x(), fMoving.lr().x());
   d.y() = rmMin (fpframe.lr().y(), fMoving.lr().y());
   movingf = rcFRect (ud, d);

   // Check if we are within.
   if (!fpframe.contains (movingf))
     {
    //   setState (eIsOutSide);
       return;
       //       rmExceptionMacro (<<"VisualFunction Correspondence");
     }
   
   // Produce the rounded one
   moving = roundRectangle (movingf);
   rmAssert (pframe.contains (moving));
}


/*  
 *   @Description: directly correspond a cell with a region
 *   @ updates: mXform, mSumQuality, 
 *                    States: isOutSide, eMotion
 *                    Calls/sets: moving rectangle
 *
 */
  
 void rcVisualFunction::direct (const rc2Fvector& pose)
 {
   const rcFRect& fpframe = sharedKin()->fpframe ();

   // Declare no translation
   rc2Fvector tr = pose - mCurrentPosition;

   // Are we registring to the immediate past 
   rcFRect fMoving, fFixed = mRectSnaps.back ();      
   fMoving = fFixed;

   // Direct is perfect!
   mSumQuality.add (1.0);
     
   /* Motion of fixed wrt moving:
    * Moving + fixed_location (integer+interpolated offset) @ now
    *          - fixed_location @ last
    * Adjust pose if the rect is positioned outside
    */
   fMoving.translate (rcFPair (tr.x(), tr.y()));

   // @todo move only in the direction needed
   rcFPair ud, d;
   ud.x() = rmMax (fpframe.ul().x(), fMoving.ul().x());
   ud.y() = rmMax (fpframe.ul().y(), fMoving.ul().y());
   
   // @todo adjust size only in the direction needed
   // @note trim is move out the ul and move in the lr
   d.x() = rmMin (fpframe.lr().x(), fMoving.lr().x());
   d.y() = rmMin (fpframe.lr().y(), fMoving.lr().y());
   fMoving = rcFRect (ud, d);
   tr = rc2Fvector (ud - fFixed.ul());
   moving (fMoving);
   mXform.trans (tr);
   mark (eMotion);
 }

/*  
 *   @Description: Correspond Fixed in Moving. 
 *   @ updates: mXform, mSumQuality, 
 *                    States: isOutSide, eMotion
 *                    Calls/sets: moving rectangle
 *
 */
  
void rcVisualFunction::correspond (const rc2Fvector& pose, const rcFPair& tolerance)
 {
   rmUnused (pose);
   int32 dummy;

   // Declare no translation
   rc2Fvector tr;

   // Are we registring to the immediate past 
   rcFRect registerTo = mRectSnaps.back ();      

   // Set up moving: fixed plus tolerances
   rcFRect fMoving;
   rcIRect iMoving;
   rcIRect fixed = roundRectangle (registerTo);
   if (!use4fixed().contains (fixed))
     {
       mXform.trans (tr); // default ctor of vector is 0,0
       clear (eMotion);
       return;
     }

   updateRects (registerTo, tolerance, fMoving, iMoving);
   if (isState (eIsOutSide))
     {
       mXform.trans (tr); // default ctor of vector is 0,0
       clear (eMotion);
       return;
     }
   
   // If after triming we have not really expanded. Assume nothing. 
   rcIPair play = iMoving.size () - fixed.size ();
   if (play.x() < 2 || play.y() < 2)
     {
       mXform.trans (tr); // default ctor of vector is 0,0
       clear (eMotion);
       rmAssert (setState (eIsOutSide));
       return;
     }

   // Note using the fixedRaw(), moving() pair 
   // @note moving and fixed buffers belong to a ring buffer that 
   // @note is rotated every acquisition. Beware of rcWindows in to these
   // @note buffers. They will not contain original content after a ring
   // @note rotation. I believe this is handled by rcWindow mutex implementation
   // @note so this copy is unnecessary but it does relax requiring framebuf it 
   // belongs to stay around
   // Create a model at the last frame.
   // @note For Functions that are shape perseving we used a model from the first one

   rcWindow movingWin (use4moving(), iMoving, dummy);
  
   rcWindow fixedw (use4fixed(), fixed, true);
   rcWindow fixedCopy (fixed.size());
   fixedCopy.copyPixelsFromWindow (fixedw);
   rcCorrelationWindow<uint8> fixedWin (fixedCopy);
   // This is the translation of fixed in moving. 
   // @note registrating to the previous frame
   mSumQuality.add (match (movingWin, fixedw, tr));
     
   /* Motion of fixed wrt moving:
    * Moving + fixed_location (integer+interpolated offset) @ now
    *          - fixed_location @ last
    */
   fMoving.translate (rcFPair (tr.x(), tr.y()));
   fMoving.size (registerTo.size());
   tr = rc2Fvector (fMoving.ul().x(), fMoving.ul().y()) - 
     rc2Fvector (registerTo.ul().x(), registerTo.ul().y());

   moving (fMoving);
   mXform.trans (tr);
   mark (eMotion);

}



/*  
 *   @Description: Correspond Fixed in Moving in Mutual Channels
 *   
 *
 */
  
void rcVisualFunction::correspondMutualChannel (const rc2Fvector& pose, const rcFPair& tolerance)
 {
   rmUnused (pose);
   int32 dummy;

   // Declare no translation
   rc2Fvector tr, cr;

   // Are we registring to the immediate past or to the first frame?
   rcFRect registerTo = mRectSnaps.back ();      

   // Set up moving: fixed plus tolerances
   rcFRect fMoving;
   rcIRect iMoving;
   rcIRect fixed = roundRectangle (registerTo);
   updateRects (registerTo, tolerance, fMoving, iMoving);

   // If after triming we have not really expanded. Assume nothing. 
   rcIPair play = iMoving.size () - fixed.size ();

   if (play.x() < 2 || play.y() < 2)
     {
       rmAssertDebug (tr.x() == 0.0f && tr.y() == 0.0f);
       mXform.trans (tr); // default ctor of vector is 0,0
       clear (eMotion);
       rmAssert (setState (eIsOutSide));
       return;
     }

   if (isAtStep (1))
     {
       {
	 rcIRect fixed = roundRectangle (mRectSnaps.back ());	  
	 rcWindow fixedw (use4fixed (), fixed, true);
	 rcWindow fixedCopy (fixed.size());
	 rfGaussianConv (fixedw, fixedCopy, 5);
	 rcCorrelationWindow<uint8> fixedWin (fixedCopy);
	 mSnaps.push_back (fixedWin);
       }

       {
	 rcIRect fixed = roundRectangle (mRectSnaps.back ());	  
	 rcWindow fixedw (sharedKin()->fixed (), fixed, true);
	 rcWindow fixedCopy (fixed.size());
	 rfGaussianConv (fixedw, fixedCopy, 5);
	 rcCorrelationWindow<uint8> fixedWin (fixedCopy);
	 mSnaps.push_back (fixedWin);
       }
       
     }

   
   /* @note See note in corresponde about ring buffers
    * Window in moving and fixed in channel
    * match in both channels keep the best. 
    * note: we are not conductng 2 different tracking
    */
   double tq (-1.0); // , cq (-1.0);
   {
     rcWindow movingWin (use4moving(), iMoving, dummy);
     rcWindow fixedw (use4fixed(), fixed, true);
     rcWindow fixedCopy (fixed.size());
     fixedCopy.copyPixelsFromWindow (fixedw);
     rcCorrelationWindow<uint8> fixedWin (fixedCopy);

     // This is the translation of fixed in moving. 
     // @note registrating to the previous frame
     tq = match (movingWin, mSnaps[0], tr);
   }

#if 0
   {
     rcWindow movingWin (sharedKin()->moving(), iMoving, dummy);
     rcWindow fixedw (sharedKin()->fixed (), fixed, true);
     rcWindow fixedCopy (fixed.size());
     fixedCopy.copyPixelsFromWindow (fixedw);
     rcCorrelationWindow<uint8> fixedWin (fixedCopy);

     // This is the translation of fixed in moving. 
     // @note registrating to the previous frame
     cq = match (movingWin, mSnaps[1], cr);
   }

   if (cq > 0.0 && cq > tq)
     {
       tq = cq; tr = cr;
     }
#endif

   /* Motion of fixed wrt moving:
    * Moving + fixed_location (integer+interpolated offset) @ now
    *          - fixed_location @ last
    */
   fMoving.translate (rcFPair (tr.x(), tr.y()));
   fMoving.size (registerTo.size());
   tr = rc2Fvector (fMoving.ul().x(), fMoving.ul().y()) - 
     rc2Fvector (registerTo.ul().x(), registerTo.ul().y());

   moving (fMoving);
   mXform.trans (tr);
   mark (eMotion);
   mSumQuality.add (tq);

   //@todo work out model allocations
   //@todo we failed to match to the next frame using the oldest model
   //@todo Time to update the model
   //@todo Where is the current position?
   //@todo Get the enclosing box of the current poly
   //@todo remember the offset between enclosing box and position
   //@todo add vector overloads for rcRectangle
   //@todo how do we index to the rect that model was made from ? 
}

/*	******************************
	*                                          *
	*    Match      
	*                                          *
	******************************
    @Description: Interface to imageFind
    @todo add support for CorrelationWindow to rfImageFind

*/

double rcVisualFunction::match (rcWindow& moving,
			      rcCorrelationWindow<uint8>& fixed,
			      rc2Fvector& trans)
{
  rcWindow fixedw = fixed.window ();
  return match (moving, fixedw, trans);
}


double rcVisualFunction::match (rcWindow& moving,
				rcWindow& fixed,
				rc2Fvector& trans)
{
  rcFindList locs;
  rcWindow cspace;

  rcTimingInfo profile (2, 10);

  // We are looking for the top score
  locs.resize (1);

  profile.touch (0);
  rfImageFind (moving, fixed, locs, cspace);
  profile.touch (1);

  vector<std::string> lables;
  lables.push_back (std::string ("match"));
  lables.push_back (std::string ("Image Find"));

  profile.printInfo (10, lables);
  rcFindList::iterator tmp = locs.begin();
  //  trans = tmp->_interp;
  //return (tmp->value / 1000.0);
  return 0.0;
}




////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
// Debugging Snippets
////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

#if 0
       // Construct Power Law Curve fitting using Log
       // We fit a linear line through Log (L(t)) that is Log of lenght as a function of time. 
       // We know that in a power relationship: L(t) = a exp (bt). 
       // Taking the log of both side, we have: Log (L(t) = Log(a) + b * t.
       // In other words Log of the length is related linearly to the parameters of the power law fit. 
       // The fitted line has slope of b which is Sin / Cos of the line segment and
       // Distance of Log (a) which is the distance of lineSegment. 

       rcLsFit<float> cLs, rLs;
	  
       vector<float>::iterator bg = mDimensions.begin();
       vector<float>::iterator tbg = mDimensionTs.begin();
       for (; *bg < minIntL; bg++, tbg++)
	 {
	   rc2Fvector logPt (log (*bg), *tbg);
	   cLs.update (logPt);
	 }

       // We are moving forward in time. So the iterators should at the begining of the relaxation
       // point. 
       rmAssert (bg < mDimensions.end());
       rmAssert (tbg < mDimensionTs.end());

       for (; bg < mDimensions.end(); bg++, tbg++)
	 {
	   rc2Fvector logPt (log (*bg), *tbg);
	   rLs.update (logPt);
	 }

       rcLineSegment<float> cl = cLs.fit();
       rcLineSegment<float> rl = rLs.fit();
#endif
