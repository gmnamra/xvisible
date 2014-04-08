/*
 *
 *$Id $
 *$Log$
 *Revision 1.8  2006/01/15 22:56:51  arman
 *selective myo
 *
 *Revision 1.7  2006/01/10 23:42:59  arman
 *pre-rel2
 *
 *Revision 1.6  2006/01/08 01:18:01  arman
 *removed advance from contractile step
 *
 *Revision 1.5  2005/12/01 23:58:17  arman
 *processing loop calls advance
 *
 *Revision 1.4  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.3  2005/11/20 03:52:15  arman
 *motion 2 fixed fixed
 *
 *Revision 1.2  2005/11/18 22:01:25  arman
 *incremental
 *
 *Revision 1.1  2005/11/17 23:19:06  arman
 *kinetoscope resync
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_kinetoscope.h>
#include <rc_draw.h>


/*	******************************
	*                                          *
	*     Step                               *
	*                                          *
	******************************
*/

int32 rcKinetoscope::step (uint32 steps, bool keepgoing)
{
  // Create any objects / models we need for what we are doing
  // If they have not been create already
  createFunctionSpecificObjects ();


  if (isCardiacCell ())
    return stepContractile (steps, keepgoing);

  if (organism() ==  rcOrganismInfo::eModel && organismName () == rcOrganismInfo::eDanioRerio)
   return stepDirectedFlow (steps, keepgoing);

  if ((organism() == rcOrganismInfo::eFluorescenceCluster &&
      ( organismName() == rcOrganismInfo::eProteinP53 ||
	organismName() == rcOrganismInfo::eProtein)))
    return stepEpi (steps, keepgoing);


  if (!keepgoing || !steps) return 0;

  // Invalidate per frame statistics
  mSnapStats.reset ();

  // Fetch next. First throttle to the next one
  uint32 sampledSteps = steps * sample ();
  if ( !frame (sampledSteps, eMoving) )
      return 0;

  rmAssert (isBound());

  if (!mPathMap.empty()) mPathMap.clear();

  selfSimilarMotionField ();

  // Clear Temporal Polygons
  // Temporal Information is used by selfSimilarMotionField
  mPOLYs.clear ();

  // And measure motion of fixed() in moving
  measure (mPathMap);

  // And correspond from fixed() to moving
  // We are not calling update. Measure register the motion. Saving the function call update (mPathMap);

  // And register from fixed() to moving
  reconcile (mPathMap);

  // Motion based Segmentation
  connectedFunctions (eMoving, mPOLYs);

  // Measure their motion
  measure (mFunctionMap);

  // Update 
  update (mFunctionMap);

  // Reconcile 
  reconcile (mFunctionMap);

  updateProcessingTime ();

  return 1;
      
}    

void rcKinetoscope::preSegmentedBodies (const vector<rcPolygon>& bodies)
{
  createFunctionSpecificObjects ();

  if (mCount <= startFrame())
    {
      mPrePOLYs = bodies;
      const uint8 bg = 0, fg = 0xff;
      mMaskCellwFF.setAllPixels(bg);
      for (uint32 ci = 0; ci < bodies.size(); ci++)
	rcFillPolygon(bodies[ci], mMaskCellwFF, fg);
    }    
}

void rcKinetoscope::preSegmentedBodies (const vector< vector <rc2Fvector> >& bodyEnds, vector<float>& rads)
{
  if (bodyEnds.size() == 0 || bodyEnds.size() != rads.size())
    {
      rmExceptionMacro(<<"Inconsistent Selection Data");
    }

  createFunctionSpecificObjects ();

  if (mCount <= startFrame())
    {
      for (uint32 i = 0; i < bodyEnds.size(); i++)
	{
	  vector<rc2Fvector> ends = bodyEnds[i];
	  // Create a polygon using the ellipse formed by the ends.
	  // Rotate and move it to the center of the line contecting the ends. 
	  rc2Fvector m = (ends[0] + ends[1]) / 2.0f;
	  rc2Fvector d = ends[1] - ends[0];
	  if (d.isNull ())
	    {
	      rmExceptionMacro(<<"Length too short");
	    }

	  float a =d.len ();
	  a = a / 2.0f;
	  rcPolygon poly (a, rads[i], 12);
	  static const rcDPair noscale (1.0, 1.0);
	  rc2Xform xt (rcMatrix_2d (d.angle (), noscale), m);
	  poly.transform (xt);  
	  mPrePOLYs.push_back (poly);
	  mBodyEnds.push_back (ends);
	  mBodyRads.push_back(rads[i]);
	}
      mark (eSelectedMyo);
    }    
}


/*	******************************
	*                            *
	*     StepContractile        *
	*                            *
	******************************
*/

int32 rcKinetoscope::stepContractile (uint32 steps, bool keepgoing)
{

  if (!keepgoing || !steps) return 0;

  // Invalidate per frame statistics
  mSnapStats.reset ();
  mDisVarError = 0;
  mDisGMEError = 0;
  mDisEdgeError = 0;
  mDisPolarError = 0;

  // Fetch next. First throttle to the next one
  uint32 sampledSteps = steps * sample ();
  if ( !frame (sampledSteps, eMoving) )
      return 0;

  rmAssert (isBound());

  if (!mPathMap.empty()) mPathMap.clear();
  mPOLYs.clear ();

  connectedFunctions (eFixed , mPOLYs);

  // Measure their motion
  measure (mFunctionMap);

  // Update 
  update (mFunctionMap);

  // Reconcile 
  reconcile (mFunctionMap);

  // Compute average shortening and store it as a population measure
  keepPopulationMeasure ();  

  updateProcessingTime ();

  return 1;

}    


int32 rcKinetoscope::stepDirectedFlowPreFill ()
{
  if (count () <= mSimPipe)
    {
      mSimBuffer.push_back (mFrameData[eFixed].base (detail ()));
    }
}
  

int32 rcKinetoscope::stepEpi (uint32 steps, bool keepgoing)
{

  if (!keepgoing || !steps) return 0;

  rcTimingInfo profile (10, frameCount () + 1);
  profile.nextPass (false);

  // Invalidate per frame statistics
  mSnapStats.reset ();

  // Fetch next. First throttle to the next one
  uint32 sampledSteps = steps * sample ();
  profile.touch (0);
  if ( !frame (sampledSteps, eMoving) )
      return 0;
  profile.touch (1);

  rmAssert (isBound());
  profile.touch (2);

  if (!mPathMap.empty()) mPathMap.clear();

  epiMotionField ();
  profile.touch (3);

  // Clear Temporal Polygons
  // Temporal Information is used by selfSimilarMotionField
  mPOLYs.clear ();

  // And measure motion of fixed() in moving
  measure (mPathMap);

  // And correspond from fixed() to moving
  // We are not calling update. Measure register the motion. Saving the function call update (mPathMap);
  
  // And register from fixed() to moving
  reconcile (mPathMap);
  connectedFunctions (eMoving, mPOLYs);
  profile.touch (4);

  // Measure their motion
  measure (mFunctionMap);
  profile.touch (5);

  // Update 
  update (mFunctionMap);
  profile.touch (6);
  profile.printInfo (4);

  // Reconcile 
  reconcile (mFunctionMap);
  profile.touch (7);

  updateProcessingTime ();

  //@note removed advance. It is now in the main loop in engineimpl

  return 1;

}    



/*	******************************
	*                            *
	*     StepDirectedFlow        *
	*                            *
	******************************
*/

int32 rcKinetoscope::stepDirectedFlow (uint32 steps, bool keepgoing)
{

  if (!keepgoing || !steps) return 0;

  // Invalidate per frame statistics
  mSnapStats.reset ();
  mDisVarError = 0;
  mDisGMEError = 0;
  mDisEdgeError = 0;
  mDisPolarError = 0;

  // Fetch next. First throttle to the next one
  uint32 sampledSteps = steps * sample ();
  if ( !frame (sampledSteps, eMoving) )
      return 0;

  rmAssert (isBound());


  // Update Sims
  reichardt (eMoving);

  updateProcessingTime ();

  //@note removed advance. It is now in the main loop in engineimpl
  //  advance ();

  return 1;

}    


#if 0
// @note division support
list<rcVisualFunction>::const_iterator cell;
 cerr << "--------------------------------------------" << endl;
 cerr << "Frame: " << count() -1 << endl;
 for( cell = visualBodies().begin(); cell != visualBodies().end(); ++cell )
  {
    cerr.setf (ios::fixed);
    cerr << setprecision (4);    

    cerr << "cell " << "[" << cell->id() << "," << cell->parentId () << "] @ " 
	   << cell->position() << " age: " << cell-> age ();
		
    std::string foo (cell->isState(rcVisualFunction::eIsMoving) ? " is " : "is not");
    cerr << foo << "Moving ";
    std::string bar (cell->isState(rcVisualFunction::eIsDividing) ? " is " : "is not");
    cerr << bar << "Dividing ";
    if (cell->isState(rcVisualFunction::eIsDivided))
      cerr  << " has Divided  ";

    cerr << endl << "A,M,C,A,cQ: " << cell->area () << "," << cell->mass () << "," 
	 << cell->circularity () << "," << cell->acceleration() << "," <<cell->cQuality() << endl;
  }

#endif
