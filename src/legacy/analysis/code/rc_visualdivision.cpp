/*
 *
 *$Id $
 *$Log$
 *Revision 1.4  2006/01/01 21:30:58  arman
 *kinetoscope ut
 *
 *Revision 1.3  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.2  2005/11/08 20:34:29  arman
 *cell lineage iv and bug fixes
 *
 *Revision 1.1  2005/08/30 21:02:22  arman
 **** empty log message ***
 *
 *Revision 1.19  2005/08/18 11:13:17  arman
 *Cell Lineage II
 *
 *Revision 1.18  2005/08/18 02:36:27  arman
 *Cell Lineage II
 *
 *Revision 1.17  2005/08/17 12:45:22  arman
 *Cell Lineage II
 *
 *Revision 1.16  2005/08/16 22:23:39  arman
 *Cell Lineage II
 *
 *Revision 1.15  2005/08/15 12:53:56  arman
 *Cell Lineage II
 *
 *Revision 1.14  2005/08/05 21:53:08  arman
 *position issue update
 *
 *Revision 1.13  2005/08/01 21:18:43  arman
 *cell lineage
 *
 *Revision 1.12  2005/08/01 17:51:45  arman
 *inccremental
 *
 *Revision 1.11  2005/08/01 12:20:11  arman
 *cell lineage
 *
 *Revision 1.10  2005/08/01 04:01:37  arman
 *cell lineage
 *
 *Revision 1.9  2005/08/01 01:47:25  arman
 *cell lineage addition
 *
 *Revision 1.8  2005/07/31 10:09:03  arman
 *commend out debugging information
 *
 *Revision 1.7  2005/07/31 06:39:16  arman
 *cell lineage incremental
 *
 *Revision 1.6  2005/07/29 21:41:05  arman
 *cell lineage incremental
 *
 *Revision 1.5  2005/07/28 00:13:17  arman
 *pre-release ci for lineage additions
 *
 *Revision 1.3  2005/07/23 21:48:52  arman
 *incremental
 *
 *Revision 1.2  2005/07/22 22:05:49  arman
 *added debugging statements
 *
 *Revision 1.1  2005/07/22 15:00:48  arman
 *divided visualcell
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_analysis.h>
#include <rc_kinetoscope.h>
#include <rc_math.h>

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


uint32 rcVisualFunction:: intersectSet (vector<rcPolygon*>&corrs)
{
  if (!corrs.empty())
    corrs.clear ();

  vector<rcPolygon>::iterator polys = sharedKin()->mPOLYs.begin();
  vector<rcPolygon>::iterator polyend = sharedKin()->mPOLYs.end();
  rcPolygon dispoly (polygons().back());
   for (; polys != polyend; polys++)
     {
       if (polys->intersects (dispoly))
	     {
	       corrs.push_back (&(*polys));
	     }
     }
   return corrs.size();
}

bool rcVisualFunction:: isIsoMorphic ()
{
  vector<rcPolygon*>corr;

  // How many region polys intersect with me?
  intersectSet (corr);
  if (!corr.size() == 1)
    {
      return false;
    }

  // So only one region poly  intersects with this cell. 
  // How many cells intersect with this region poly.
  list<rcVisualFunction>::const_iterator  cell = sharedKin()->visualBodies ().begin();
  list<rcVisualFunction>::const_iterator  cellend = sharedKin()->visualBodies ().end();
  for (; cell != cellend; cell++)
    {
      if (cell->isState(eIsOutSide)) continue;
      if (cell->isState(eIsDivided)) continue;

      if (cell->id() == id()) continue; // skip me
      rcPolygon dispoly (cell->polygons().back());
      if (corr[0]->intersects (dispoly))
	{
	  return false;
	}
    }
  return true;
}

/*	******************************
	*    Create Visual Functions from Polygon Regions
	*     
	*                            
	******************************
	@note Grow. In biological sense is multiplying
*/

bool rcVisualFunction::grow(vector<rcPolygon> groupIntersects,  list<rcVisualFunction>& functionList)
{
  if (groupIntersects.size() != 2) return false;
  vector<rcPolygon>::iterator gp = groupIntersects.begin();
  vector<rcVisualFunction> two;

// Find which one of the RLEs in connect() correspond to the Daughters
  for (int32 i = 0; i < (int32) groupIntersects.size(); i++, gp++)
    {
      rc2Fvector motionCtr;
      rcIRect ir =   gp->orthogonalEnclosingRect();

      // Compute Center Of Mass Using Moving Time (by definition for growing)
      rcWindow src =sharedKin()->moving();
      if (sharedKin()->channelAvailable())
	{
	  src = sharedKin()->channelMoving();
	}

      rcWindow tmp (src, ir);
      rcShapeRef shaper = boost::shared_ptr<rcShape> (new rcShape (tmp));
      motionCtr = shaper->centerOfmass (rcShape::eImage);
      motionCtr += rc2Fvector (ir.ul());	  

      rcVisualFunction vf (sharedKin ().get (), motionCtr, *gp, true);
      vf.mMinTemporalIntRect = ir;
      vf.mMinTemporalEncRect = ir;
      vf.mId = (uint32) i;
      vf.mParentId = id();
      vf.mGeneration = generation() + 1;
      vf.info (organism(), organismName ());
      vf.setState(eIsChild);
      vf.frameIndex(sharedKin()->moving().frameBuf()->frameIndex ());
      functionList.push_back (vf);
    }

}


/*	******************************
	*                            *
	*     D i v i d i n g        *
	*                            *
	******************************

*/

bool rcVisualFunction::dividingProcess  ()
{
#if 0
  rcFRect fixed = mRectSnaps.back();
  mAnaphase--;

  // make one 3 time as wide and high
  rcFRect fMoving;
  rcFPair tol (0.67f, 0.67f);
  updateRects (fixed, tol, fMoving, mDividingWatchWindow);

  if (!(mDividingWatchWindow.width() > mGaussModel.width() && 
	mDividingWatchWindow.height() > mGaussModel.height()))
    return false;

 // make a window in fixed and moving and look for daughters
  vector<rcWindow> zone;
  zone.push_back (rcWindow (use4moving(), mDividingWatchWindow, true));
  zone.push_back (rcWindow (use4fixed(), mDividingWatchWindow, true));
  vector<rcIRect> bRects;
  vector<rc2Fvector> links;
  rcWindow gaussm = mGaussModel.window ();

  double mind (gaussm.width() * 0.90);

  for (uint32 i = 0; i < 2; i++)
    {
      bRects.clear ();
      list<rcPeak<int16> > locs;
      rcWindow space;
      rfImageFind (zone[i], gaussm, locs, space);
      
      // If we have two far away peaks better than 500
      // Peaks are in decreasing scores.
      list<rcPeak<int16> >::iterator rTr = locs.begin();
      if (locs.size() < 2) return false;
      float minScore (rTr->value);

      vector<rc2Fvector> ends;
      for (uint32 i = 0; i < 2; i++, rTr++)
	{
	  rcIRect tmp (rTr->_location.x() + mDividingWatchWindow.ul().x(), 
		       rTr->_location.y() + mDividingWatchWindow.ul().y(),
		       gaussm.width(), gaussm.height());
	  bRects.push_back (tmp);
	  minScore = rmMin (minScore, rTr->value);
	  ends.push_back (rTr->interp);
	  rc2Fvector vtmp (fMoving.ul()); vtmp += rTr->interp;
	  mChildren.push_back (vtmp);
	}

      if (minScore < gGoodFit) return false;
      ends[0] -= ends[1];
      if (ends[0].isLenNull() || ends[0].len () < mind) return false;
      links.push_back (ends[0]);
    }

  rcRadian play (rkPI / 4.0);
  if (!real_equal (links[0].angle(), links[1].angle(), play)) return false;

   // go ahead an multiply
  return grow (bRects);
#endif 
  return false;
}



bool rcVisualFunction::grow (const  vector<rcIRect>& bRects)
{
  vector<rcIRect>::const_iterator cItr = bRects.begin();
  vector<rcVisualFunction> two;

// Find which one of the RLEs in connect() correspond to the Daughters
  for (int32 i = 0; i < (int32) bRects.size(); i++, cItr++)
    {
      rcPolygon poly (cItr->width() / 2.0f, 8);
      rc2Fvector motionCtr;
      poly.translate (rc2Dvector (cItr->ul().x(), cItr->ul().y()));
      poly.centerOf (motionCtr);
      rcVisualFunction vf (sharedKin ().get() , motionCtr, poly, true);

      vf.mMinTemporalIntRect = *cItr;
      vf.mMinTemporalEncRect = *cItr;
      vf.mId = (uint32) i;
      vf.mParentId = id();
      vf.mGeneration = generation() + 1;
      vf.info (organism(), organismName ());
      vf.setState(eIsChild);
      vf.setState(eIsGeneralModel);
      vf.frameIndex(sharedKin()->moving().frameBuf()->frameIndex ());
      two.push_back (vf);
    }

  // Only add the children if both could be created
  if (two.size() == 2)
    {
      for (uint32 i = 0; i < 2; i++)
	{
	  sharedKin()->mDaughterFunctionMap.push_back (two[i]);
	}
    }

  return (two.size() == 2);
}


/*	**********************************
	*                                *
	*     Special Output Processing  *
	*                                *
	**********************************
*/

void rfDivisionReport (list<rcVisualFunction>& visualBodies, ostream& output)
{
  output.setf (ios::fixed);
  output << setprecision (5);
  list<rcVisualFunction>::const_iterator cell;

  output << " Id, parentId, generation, start_x, start_y, start_frame_index, end_frame_index" << endl;
  output << "Positions in pixels, top to bottom, left to right" << endl;
  output << "Time in hours (using capture time lapse of 15 minutes)" << endl;
  output << "[";

  list<rcVisualFunction>::const_iterator cellone2end = visualBodies.end();std::advance (cellone2end, -1);
  for( cell = visualBodies.begin(); cell != visualBodies.end(); ++cell )
    {
      rc2Fvector startpos (cell->rectSnaps()[0].ul().x(), cell->rectSnaps()[0].ul().y());
      output << "{" << (int32) cell->id() << "," << (int32) cell->parentId ()  << "," << 
	(int32) cell->generation() << "," << 
	cell->anchor().x() + startpos.x() << "," << 
	cell->anchor().y() + startpos.y() << ","  << 
	cell->startFrameIndex() * 0.25 << "," << cell->dividedFrameIndex()  * 0.25;
      if (cell == cellone2end)
	output << "}" << endl;
      else 
	output << "}," << endl;
    }
}


