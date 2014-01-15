/*
 *
 *$Id $
 *$Log$
 *Revision 1.68  2006/01/15 22:56:51  arman
 *selective myo
 *
 *Revision 1.67  2006/01/11 17:45:24  arman
 *removed debugging output
 *
 *Revision 1.66  2006/01/10 23:42:59  arman
 *pre-rel2
 *
 *Revision 1.65  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.64  2005/11/13 19:24:37  arman
 *added support for lineage option handling
 *
 *Revision 1.63  2005/11/07 23:27:48  arman
 *cell lineage iv and bug fixes
 *
 *Revision 1.62  2005/11/07 17:32:08  arman
 *cell lineage iv and bug fix
 *
 *Revision 1.61  2005/10/31 10:24:30  arman
 *fixed warnings
 *
 *Revision 1.60  2005/09/09 20:45:21  arman
 *2.0 Pre
 *
 *Revision 1.59  2005/09/01 22:03:03  arman
 *Cell Lineage Cleanup
 *
 *Revision 1.58  2005/08/30 20:57:16  arman
 *Cell Lineage
 *
 *Revision 1.86  2005/08/24 02:40:39  arman
 *Cell Lineage II
 *
 *Revision 1.85  2005/08/23 23:32:31  arman
 *Cell Lineage II
 *
 *Revision 1.84  2005/08/23 04:17:16  arman
 *fixed division bug disrupting tracking after divide
 *
 *Revision 1.83  2005/08/22 23:54:35  arman
 *Cell Lineage II
 *
 *Revision 1.82  2005/08/22 14:01:16  arman
 *fixed updating dividing status
 *
 *Revision 1.81  2005/08/19 21:44:16  arman
 *Cell Lineage II
 *
 *Revision 1.80  2005/08/18 11:13:17  arman
 *Cell Lineage II
 *
 *Revision 1.79  2005/08/18 02:36:27  arman
 *Cell Lineage II
 *
 *Revision 1.78  2005/08/17 12:45:22  arman
 *Cell Lineage II
 *
 *Revision 1.77  2005/08/16 22:23:39  arman
 *Cell Lineage II
 *
 *Revision 1.76  2005/08/16 16:00:18  arman
 *fixed a bug in generational numbering
 *
 *Revision 1.75  2005/08/16 03:30:40  arman
 *Cell Lineage II
 *
 *Revision 1.74  2005/08/15 20:29:38  arman
 *Cell Lineage II
 *
 *Revision 1.73  2005/08/15 12:53:55  arman
 *Cell Lineage II
 *
 *Revision 1.72  2005/08/12 20:37:54  arman
 *inc cell lineage plus
 *
 *Revision 1.71  2005/08/10 21:53:45  arman
 *added mathematica format dump of correspondence map
 *
 *Revision 1.70  2005/08/05 21:53:08  arman
 *position issue update
 *
 *Revision 1.69  2005/08/01 02:24:23  arman
 *removed using discBuffer for now
 *
 *Revision 1.68  2005/08/01 01:47:25  arman
 *cell lineage addition
 *
 *Revision 1.67  2005/07/31 14:37:34  arman
 *a big reminder of const reference handling potential issues
 *
 *Revision 1.66  2005/07/31 09:55:42  arman
 *fixed a bug in image selection
 *
 *Revision 1.65  2005/07/31 06:39:16  arman
 *cell lineage incremental
 *
 *Revision 1.64  2005/07/30 19:49:41  arman
 *exapnded logic for image selection
 *
 *Revision 1.63  2005/07/27 23:40:37  arman
 *Moved rc_mathmodel.h to include dir
 *
 *Revision 1.62  2005/07/27 01:34:30  arman
 *removed debugging info in the email.
 *
 *Revision 1.61  2005/07/26 00:55:51  arman
 *added time arg to connectFunctions (under review)
 *
 *Revision 1.60  2005/07/23 21:47:21  arman
 *incremental
 *
 *Revision 1.59  2005/07/21 22:09:11  arman
 *incremental
 *
 *Revision 1.58  2005/07/21 03:18:49  arman
 *fixed a bug where a divided cell was still being used.
 *
 *Revision 1.57  1970/01/01 22:44:43  arman
 *assume cells are unattached.
 *
 *Revision 1.56  1970/01/01 17:36:19  arman
 *incremental
 *
 *Revision 1.54  2005/07/11 09:53:42  arman
 *incremental
 *
 *Revision 1.53  2005/07/01 21:05:39  arman
 *version2.0p
 *
 *Revision 1.60  2005/06/27 23:23:49  arman
 *added handling of divided cells
 *
 *Revision 1.59  2005/06/27 15:25:17  arman
 *removed debugging macro and switched to center of mass instead of
 *median
 *
 *Revision 1.58  2005/06/24 21:01:17  arman
 *switched to median point and added generation
 *
 *Revision 1.57  2005/06/22 00:12:37  arman
 *re-organized.
 *
 *Revision 1.56  2005/06/03 18:30:47  arman
 *removed buggy diagnostic
 *
 *Revision 1.55  2005/06/02 01:01:46  arman
 *updated to rcShapeRef
 *
 *Revision 1.54  2005/05/31 10:44:04  arman
 *moved similarator ctor to its ctor
 *
 *Revision 1.53  2005/05/15 00:15:42  arman
 *updated wrt changes in rcVisualFunction
 *
 *Revision 1.52  2005/05/03 21:05:16  arman
 *fixed the infinite similarity bug
 *
 *Revision 1.51  2005/03/31 23:26:33  arman
 *incorporate channelSelection
 *
 *Revision 1.50  2005/02/03 14:48:50  arman
 *flu morphometry
 *
 *Revision 1.49  2004/12/07 22:14:46  arman
 *parameterized preProcessing of connect
 *
 *Revision 1.48  2004/11/22 15:18:24  arman
 *VisualFunction origin is set by MotionCenter
 *
 *Revision 1.47  2004/11/16 21:32:51  arman
 *incremental
 *
 *Revision 1.46  2004/09/24 21:05:45  arman
 *switched logic due to apparent compiler bug
 *
 *Revision 1.45  2004/08/26 22:38:53  arman
 *moved debugging code to what it is
 *
 *Revision 1.44  2004/08/25 03:15:53  arman
 *added commented out code to compute com from other images
 *
 *Revision 1.43  2004/08/23 10:37:55  arman
 **** empty log message ***
 *
 *Revision 1.42  2004/08/20 19:12:37  arman
 *passing polygons to visual functions instead of rects
 *
 *Revision 1.41  2004/08/09 13:27:03  arman
 *added similarator initialization for cells
 *
 *Revision 1.40  2004/05/28 13:12:10  arman
 *added a direct vector api to associate
 *
 *Revision 1.39  2004/04/05 14:03:04  arman
 *added periodic api
 *
 *Revision 1.38  2004/03/19 17:18:02  arman
 *More Contractile Motion
 *
 *Revision 1.37  2004/03/19 12:37:44  arman
 *adding contractile motion source
 *
 *Revision 1.36  2004/03/16 21:29:06  arman
 *added reconcile
 *
 *Revision 1.35  2004/02/26 23:48:25  arman
 *updated to new motionmap key scheme
 *
 *Revision 1.34  2004/02/22 17:58:06  arman
 *corrected errors in global alignment as well as association
 *
 *Revision 1.33  2004/02/13 22:06:50  arman
 *fixed another bug in prePOLYs
 *
 *Revision 1.32  2004/02/13 04:04:57  arman
 *corrected preSegmentation logic
 *
 *Revision 1.31  2004/02/11 22:46:38  arman
 *fixed a bug in association
 *
 *Revision 1.30  2004/02/05 02:36:44  arman
 *fixed a bug in control logic.
 *corrected a /2 difference in min size
 *
 *Revision 1.29  2004/02/04 10:33:32  arman
 *added new polygon & PreSegmented Bodies interface
 *
 *Revision 1.28  2004/02/04 04:35:28  arman
 *fixed assertion failure in association
 *
 *Revision 1.27  2004/02/03 14:58:37  arman
 *fixed bugs in globalestimation
 *
 *Revision 1.26  2004/01/21 17:01:09  arman
 *added support for scaling
 *
 *Revision 1.25  2004/01/18 17:51:38  arman
 *redesign of rle to function
 *
 *Revision 1.24  2004/01/14 20:39:26  arman
 *major changes
 *
 *Revision 1.23  2003/12/09 03:11:20  arman
 *fixed min size bug
 *
 *Revision 1.22  2003/12/08 18:06:19  sami
 *Commented RLEs debug display
 *
 *Revision 1.21  2003/12/08 01:35:41  arman
 *fixed a bug in registrating boundaries to motion vector field
 *
 *Revision 1.20  2003/12/05 20:15:08  arman
 *added bead detection
 *
 *Revision 1.19  2003/12/05 20:08:59  arman
 *added bead tracking and temporal capacitence
 *
 *Revision 1.18  2003/11/25 23:58:42  sami
 *Disabled connection image output in rcKinetoscope::connectedFunctions()
 *
 *Revision 1.17  2003/11/25 01:55:31  arman
 *interim checkin
 *
 *Revision 1.16  2003/11/03 22:25:39  arman
 *added new motion center support
 *
 *Revision 1.15  2003/08/25 12:12:12  arman
 *fixed warnings
 *
 *Revision 1.14  2003/07/01 12:16:43  arman
 *origin is the median point of the RLE
 *
 *Revision 1.13  2003/07/01 02:32:32  arman
 *switched to projection
 *
 *Revision 1.12  2003/06/29 11:42:13  arman
 *added support for the new RLE interface.
 *
 *Revision 1.11  2003/06/27 17:37:16  arman
 *added deformation
 *
 *Revision 1.10  2003/06/21 15:29:06  arman
 *added impl for displacement rms
 *
 *Revision 1.9  2003/06/20 16:23:01  arman
 *incremental ci
 *
 *Revision 1.8  2003/06/13 20:54:09  sami
 *Removed combineRegions()
 *
 *Revision 1.7  2003/06/12 20:47:34  sami
 *Fixed bug in combineRegions()
 *
 *Revision 1.6  2003/06/10 14:55:06  sami
 *RLE vectorization changes
 *
 *Revision 1.5  2003/06/04 21:53:14  sami
 *Added RLE debugging code
 *
 *Revision 1.4  2003/06/02 22:24:53  sami
 *Removed gRects argument from rfGetRLEs(). rcRleWindow::rectangle() method
 *can be used to access RLE bounding rect.
 *
 *Revision 1.3  2003/06/02 21:12:16  sami
 *Use rfGetRects() for mRegions updates
 *
 *Revision 1.2  2003/05/28 08:57:51  arman
 *Fixed a bug in region processing partly responsible for kinetoscope window bug
 *
 *Revision 1.1  2003/05/28 08:34:33  arman
 *Kinetoscope Tracking Functionality
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include <rc_kinetoscope.h>
#include <rc_pmcorr.h>
#include <rc_ip.h>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <rc_peak.h>
#include <rc_analysis.h>
#include <rc_lsfit.h>
#include <rc_associate.h>
#include <rc_similarity.h>
#include <rc_macro.h>
#include <rc_polygongroup.h>


//#define TRACKDEBUG

#ifdef PLOTDEBUG
#include <rc_gnuplot.h>
#endif

#define rmPrintBinaryImageGTEV(a,v){				    \
  for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {    \
    for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)	    \
      fprintf(stderr, "%1c", ((a).getPixel(j__temp,i__temp)>=(v)) ? '+' : '-'); \
    fprintf (stderr, "\n");					    \
  }}


void rfOfCellsAndPolys (list<rcVisualFunction>& cells, vector<rcPolygon>& fixed, vector<rcPolygon>& moving);


/*	*************************
	*                       *
	*     Localization      *
	*                       *
	*************************
*/

// Collecting Co-locomotive Bodies
// The Output vectors and areas are same size and one larger than the number of found regions
// Background is counted and considered the first entry
// Returns number of non-zero locomotive regions
// TBD: switch to runlength encoding

// #define TRACKDEBUG


bool rcKinetoscope::connectFunction (const rcIRect& r,
				     const rcPolygon& p,
				     rc2Fvector& moc,
				     vector<rc2Fvector>& anc,
				     vector<rc2Fvector>& mot)
{
  rcMotionPathMap::iterator dm;
  rcMotionCenter com;

  for (int32 j = r.ul().y(); j < r.ll().y(); j++)
    {
      for (int32 i = r.ul().x(); i < r.ur().x(); i++)
	{
	  rcIPair ij (i, j);
	  rc2Dvector dij (ij);
	  if (!p.contains (dij)) continue;

	  // Get the key for this ij and this time
	  int32 index = (((i)<<16)+(j));
	  dm = mPathMap.find (index);
	  if (dm == mPathMap.end()) continue;

	  // Accumulate info from this motion path
	  rc2Fvector dM = dm->second.position () - dm->second.anchor ();
	  com.add (dm->second.position (), dM);

	  // Register the motion vectors for this cell
	  anc.push_back (dm->second.anchor());
	  mot.push_back (dm->second.position());
	}
    }

  return com.center (moc);
}


/*	****************************************************
	*                                                  *
	*     To Segment or To PreSegment                  *
	*                                                  *
	****************************************************
*/
#define FILLFUNCTION \
  vf.mMinTemporalIntRect = *rectIter;\
  vf.mMinTemporalEncRect = *rectIter;\
  vf.mId = regionLabel;		     \
  vf.frameIndex (videoFrameIndex);\
  vf.mParentId = rcVisualFunction::gPreHistoric;\
  vf.mGeneration = 0;\
  vf.label (regionLabel-1);\
  vf.mMass = mass;\
  mFunctionMap.push_back (vf)


void rcKinetoscope::connectedFunctions (Time when, vector<rcPolygon>& POLYs)
{
  vector<rcIRect> cRects;

  // @note:
  // mPrePOLYs empty && POLYs empty ==> no presegmentation information: cannonical operation
  // mPrePOLYs NOTempty && POLYs empty ==> presegmented, first time through: use presegmented
  // mPrePOLYs NOTempty && POLYs NOTempty ==> presegmented, after first time through: generate new polygons

  if (!mPrePOLYs.empty() && isCardiacCell ())
    {    
      POLYs = mPrePOLYs;
      for (uint32 i = 0; i < POLYs.size(); i++)
	cRects.push_back (POLYs[i].orthogonalEnclosingRect());
    }
  else if (!mPrePOLYs.empty() && mCount <= startFrame())
    {    
      POLYs = mPrePOLYs;
      for (uint32 i = 0; i < POLYs.size(); i++)
	cRects.push_back (POLYs[i].orthogonalEnclosingRect());
    }
  else 
    {
      if (when == eFixed)
	{
	  if (preProcessConnect ())
	    {
	      rcWindow buf =  rfPixelMin3By3 (connected());
	      rfPixelMax3By3 (buf, connected());
	    }
	  rfComponentPolygons (connected(), mMinParea/2, POLYs, cRects, false);
	}
      else
	{
	  if (preProcessConnect ())
	    {
	      rcWindow buf =  rfPixelMin3By3 (connect());
	      rfPixelMax3By3 (buf, connect());
	    }
	  rfComponentPolygons (connect(), mMinParea/2, POLYs, cRects, false);
	}
    }

  rmAssert (cRects.size() == POLYs.size());

#ifdef TRACKDEBUG
  rfOfCellsAndPolys (visualBodies(), mFixedPOLYs, mPOLYs);
#endif
  
  // rfPolygonGroupMutualMerge (mFixedPOLYs, mPOLYs,  mMergedPOLYs, 5.0);

  // Free motion vectos space if any
  // We will have cell number of vectors for 
  // copying the motion vectors. If we have no 
  // motion vectors the little waste is ok

  if (!mPathMap.empty() && !isLabelProtein ())
    {
      for (uint32 i = 0; i < mVanchor.size(); i++)
	mVanchor[i].clear();
      for (uint32 i = 0; i < mVmoved.size(); i++)
	mVmoved[i].clear();
      mVanchor.clear();
      mVmoved.clear();

      mVanchor.resize (POLYs.size());
      mVmoved.resize (POLYs.size());
    }

  uint32 regionLabel = 1;
  bool wasEmpty = mFunctionMap.empty();

  // Cells are associate with locomotive regions by an index to the
  // regions that is stored in label data member of the cells. Note
  // each cell is through a pointer handle connected with kinetoscope
  // that owns regions
  // First time. Identify cells based on locomotive regions
  // @note Get the right frame index. Motion Based Segmentation is always
  // called with Moving time. 
  rcWindow src =use4fixed();
  if (when != eFixed) src = use4moving();
  int32 videoFrameIndex = src.frameBuf()->frameIndex ();

  if (wasEmpty)
    {
      vector<rcPolygon>::iterator polyIter = POLYs.begin();
      vector<rcIRect>::iterator rectIter = cRects.begin();
      for (; polyIter != POLYs.end(); polyIter++, regionLabel++, rectIter++)
	{
	  rc2Dvector cop;
	  rc2Fvector motionCtr;
	  double mass;

	  if (!polyIter->centerOf (cop)) continue;

	  // Validate the area by estimating its Hessian
	  // Use motion center for origin
	  if (!mPathMap.empty() && !isLabelProtein ())
	    {
	      rmAssert (regionLabel != 0);
	      rc2Fvector df (-1.0f, -1.0f);
	      if (!validate (*rectIter, df) || df.x() < 1.0)
		continue;
	      connectFunction (*rectIter, *polyIter, motionCtr, 
			       mVanchor[regionLabel-1], mVmoved[regionLabel-1]);
	    }
	  else // Use Irradiance
	    {
	      rcWindow tmp (src, *rectIter);
	      rcShapeRef shaper = boost::shared_ptr<rcShape> (new rcShape (tmp));
	      motionCtr = shaper->centerOfmass (rcShape::eImage);
	      motionCtr += rc2Fvector (rectIter->ul());	  
	    }
	
	  // Now create the cell 
	  rmAssert (regionLabel!= 0);
	  if (isCardiacCell () && have (eSelectedMyo))
	    {
	      vector< vector<rc2Fvector> >::iterator endsIter = mBodyEnds.begin();
	      std::advance (endsIter, regionLabel - 1);
	      rcVisualFunction vf ( this ,*polyIter, *endsIter, 
				   mBodyRads[regionLabel - 1]);
	      FILLFUNCTION;
	    }
	  else
	    {
	      rcVisualFunction vf ( this, motionCtr, *polyIter);
	      FILLFUNCTION;
	    }
	}
    } // if empty (that is the first time


  
  // New Generation ? 
  // The detected off-springs at last moving are now accepted when moving-- is the 
  // fixed. 
  // If any cells @ fixed time are expecting, add them to the daughter list
  // @note daughters are set in moving at this time
  // In a cell lineage experiment, we are detecting the case where the cell is in the
  // final phases of mitosis where moving contains two newly separated daughter cells
  // After corrspondence, make sure we have cleared offsprings
  if (have(rcKinetoscope::eLineage))
    {
      reconcileNewGeneration ();
      rmAssert (mDaughterFunctionMap.empty());
      newGeneration (mFunctionMap, POLYs);
    }

  
  // Associate to moving
  if (!wasEmpty) 
    {
      associate (mFunctionMap, POLYs);
    }

}

void rcKinetoscope::newGeneration (list<rcVisualFunction>& cells, vector<rcPolygon>& regions)
{
   // For each cell (plus a disc buffer ) that is dividing, check to see if it overlaps two polygons. 
   // If it does create daughters from the polys 

   list<rcVisualFunction>::iterator aliveCell = cells.begin();

   for (; aliveCell != cells.end(); aliveCell++)
     {
       // Get the current (fixed) polygon representing the cell
       rcPolygon cpg = aliveCell->polygons().back();

       // How many of the moving polygons intersect with it?
       // If I am dividing these could be the daughters
       vector<rcPolygon> groupIntersects;
       if (aliveCell->isState (rcVisualFunction::eIsDividing) && cpg.intersects (regions, groupIntersects, 0.1f) && 
	   groupIntersects.size() == 2 &&  real_equal (aliveCell->cQuality (), 1.0f, 0.01f))
	 {
	   // Do these polys belong to any other cells ? 
	   // Create daughter cells and set the cell to divided
	   aliveCell->grow (groupIntersects, mDaughterFunctionMap);
	   aliveCell->setState (rcVisualFunction::eIsJustDivided, true);
#ifdef TRACKDEBUG
	   cerr << "Cell " << aliveCell->id () << "Intersects with 2 polys" << endl;
#endif
	 }
     }
}

void rcKinetoscope::reconcileNewGeneration ()
{
  if (mDaughterFunctionMap.empty()) return;

  // First find the highest Id used thusfar
  list<rcVisualFunction>::iterator vfunc = mFunctionMap.begin();
  uint32 numId (0);
  for (; vfunc != mFunctionMap.end(); vfunc++)
    {
      numId = rmMax (numId, vfunc->mId);
    }  

#ifdef TRACKDEBUG
  cerr << "Reconcile [" << count() << "]: ";
#endif

  // If there are any daughters from last frame add them
  // This frames Id start at the next available id. 
  numId++;

#ifdef TRACKDEBUG
  cerr << "New Generation Cell #s start @ " << numId << endl;
#endif

  vfunc = mDaughterFunctionMap.begin();
  for (; vfunc != mDaughterFunctionMap.end(); vfunc++, numId++)
    {
      vfunc->mId = numId;
      mFunctionMap.push_back (*vfunc);
#ifdef TRACKDEBUG
      cerr << "{" << endl << *vfunc << "}" << endl;
#endif
    }

  mDaughterFunctionMap.clear ();

}	  

void rfOfCellsAndPolys (list<rcVisualFunction>& cells, vector<rcPolygon>& fixed, vector<rcPolygon>& moving)
{
  cerr << fixed.size () << " Fixed Poly(s),   " << cells.size() << " Cell(s),   " << 
    moving.size() << " Moving Cell(s) " << endl;

   list<rcVisualFunction>::iterator aliveCell = cells.begin();

   for (; aliveCell != cells.end(); aliveCell++)
     {
       cerr << "Cell " << aliveCell->id() << "\t";

       if (rfCellNotViable (*aliveCell))
	 {
	   cerr << aliveCell->state () << endl;
	   continue;
	 }

       rcPolygon cpg = aliveCell->polygons().back();
       vector<rcPolygon>::iterator fp = fixed.begin();
       vector<int32> fflags, mflags;
       for (;fp != fixed.end(); fp++)
	 {
	   int32 intersect = cpg.intersects (*fp);
	   fflags.push_back (intersect);
	 }

       fp = moving.begin();
       for (;fp != moving.end(); fp++)
	 {
	   int32 intersect = cpg.intersects (*fp);
	   mflags.push_back (intersect);
	 }

       for (uint32 fi = 0; fi < (fflags.size() + mflags.size()) ; fi++)
	 {
	   if (fi < fflags.size() && fflags[fi]) 
	     cerr << "f+\t";
	   else if (fi < fflags.size() && fflags[fi] == 0) 	   
	     cerr << "f-\t";

	   if (fi >= fflags.size())
	     {
	       uint32 mi = fi - fflags.size();
	       if (mi < mflags.size() && mflags[mi]) 
		 cerr << "m+\t";
	       else if (mi < mflags.size() && mflags[mi] == 0) 	   
		 cerr << "m-\t";
	     }
	 }

       cerr << endl;
     }
}


/*	****************************
	*                          *
	*  A s s o c i a t i o n   *
	*                          *
	****************************
*/



void rcKinetoscope::associate (list<rcVisualFunction>& cells, vector<rcPolygon>& regions, Time when)
 {
   uint32 dividedCells (0), allCells (cells.size());

   // For each (moving) polygon see how many cells intersect with it
   list<rcVisualFunction>::iterator cell = cells.begin();

   static int32 zi (0);
   vector<int32> mflags (cells.size(), zi);
   vector<int32>::iterator mf = mflags.begin();
   vector<int32> fflags (mFixedPOLYs.size(), zi);
   vector<int32>::iterator ff = fflags.begin();
   
   for (; cell != cells.end(); cell++, mf++)
     {
       if (uint32 b = (uint32) rfCellNotViable (*cell))
	 {
	   dividedCells += b;
	   continue;
	 }

       rcPolygon cpg = cell->polygons().back();
       *mf = cpg.intersects (regions);
     }

   if (when == eMoving)
     {
       vector<rcPolygon>::iterator fp = mFixedPOLYs.begin();

       for (;fp != mFixedPOLYs.end(); fp++, ff++)
	 {
	   *ff = fp->intersects (regions);
	 }
     }

#ifdef TRACKDEBUG
   cerr << "Associate: " << "Total Registered Cells " << cells.size() << " Dead Cells: " << dividedCells << endl;
   rfPRINT_STL_ELEMENTS (mflags, "moving (cells) Flags");
   rfPRINT_STL_ELEMENTS (fflags, "moving (fixed) Flags");
#endif

   //@note handle case where everyone has died!
   //@description LUT is a lookup table between living cell and their index among all cells
   // that is if we have LUT[i] = j, then ith living cell has j index among all cells
   uint32 liveCells = allCells - dividedCells;
   static const int32 inValid (-1);
   vector<int32> LUT (liveCells, inValid);
   vector<rc2Dvector> notDeadcells, regionCtrs;
   uint32 i (0);
   for(cell = cells.begin(); cell != cells.end(); ++cell, ++i )
     {
       if (rfCellNotViable (*cell)) continue;
       rc2Dvector p (cell->position().x(), cell->position().y());

       // remember indices are 0 based
       uint32 next (notDeadcells.size());
       notDeadcells.push_back (p);
       LUT[next] = (int32) i;
     }

   rmAssert (notDeadcells.size() == liveCells);

   rfPolygonGroupCenters (regions, regionCtrs);
 
   // Correspond Cells and Regions
   // @ what happens if you have more or less polys than cells ?
   vector<int32> labels (liveCells);
   vector<float> scores (liveCells);
   rfAssociate(notDeadcells, regionCtrs, labels, scores);

   for (uint32 i = 0; i < LUT.size(); i++)
     {
       int32 anyCellIndex = LUT[i];
       rmAssert (anyCellIndex >= 0 && anyCellIndex < (int32) (cells.size()));
       cell = cells.begin(); 
       std::advance (cell, anyCellIndex);
       cell->label (labels[i]);
       cell->cQuality (scores[i]);
     }
 }

