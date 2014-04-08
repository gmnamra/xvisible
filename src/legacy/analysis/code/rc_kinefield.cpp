
#include <rc_kinetoscope.h>
#include <rc_platform.h>
#include <rc_moments.h>
#include <rc_analysis.h>
#include <rc_fit.h>
#include <rc_histstats.h>
#include <rc_edge.h>
#include <iomanip>
#include <rc_ipconvert.h>

static const rcIPair sOne (1,1);
static const rcIPair sTwo = sOne + sOne;





/*	*************************
	*                       *
	*     Field Operations  *
	*                       *
	*************************
*/


/*
 * Find Peaks of local AutoCorrelation estimates.
 * Weiner-Lee Theorem
 */



static bool withinPreSegmentedBodies (vector<rcPolygon>& sml, rcIPair& offset, rcIPair loc)
{
  vector<rcPolygon>::iterator sit = sml.begin();
  rcRectangle<int32> reg (loc - offset, loc + offset);
  rmAssert (loc.x() >= offset.x());
  rmAssert (loc.y() >= offset.y());
  rcPolygon poly;

  // Push in the CCW
  rmAssert (poly.pushUniqueVertex (rc2Dvector (reg.ul())));
  rmAssert (poly.pushUniqueVertex (rc2Dvector (reg.ul() + rcIPair (reg.width(), 0))));
  rmAssert (poly.pushUniqueVertex (rc2Dvector (reg.lr())));
  rmAssert (poly.pushUniqueVertex (rc2Dvector (reg.lr() - rcIPair (reg.width(), 0))));
  rmAssert (poly.isValid());
  rmAssert (poly.isConvex());

  bool within (false);
  for (;sit != sml.end(); sit++)
    {
      if (sit->intersects (poly))
	{
	  within = true;
	  break;
	}
    }

  return within;
}

void rcKinetoscope::selfSimilarMotionField ()
{
#if 0
  // Calculate Registeration between fixed and moving.
  rmAssert (registeredFrameAnchors ().size() == 2);
  vector<rcWindow> fixedandmoving (2);
  rcIPair speed (maxRadialSpeed(), maxRadialSpeed());

  //@note this is a guess at good inner rectangle to use. We use hillclimbing to
  // register the frames. To do that:
  // 1. be large enough to allow registration among large number of cases
  // 2. the pad does not include the only unique features in the frame
  static float pad = 2.5f/100.0f; // percent shrinkage
  int32 tpad = (int32) (std::min (iframe().size().x(), iframe().size().y()) * pad);
  rcRectangle<int32> landmark = iframe().trim (tpad);
  fixedandmoving[0] = rcWindow (use4fixed (), landmark, tpad);
  fixedandmoving[1] = rcWindow (use4moving (), landmark, tpad);
  typedef vector<rcWindow>::iterator iterator;
  rcImageSetRegister<iterator> reg (fixedandmoving.begin (), fixedandmoving.end());
  reg.setRegister (fixedandmoving.begin (), speed, registeredFrameAnchors());
  mMovingFrameTranslation.x() = (int32) registeredFrameAnchors()[1].x() - speed.x();
  mMovingFrameTranslation.y() = (int32) registeredFrameAnchors()[1].y() - speed.y();
#endif

  // Appraise sites for motion vector calculation
  selfSimilarMotionField_AttentiveCapture ();
}



//      +------------------------+
//      |                                            |  searchSize (5,5)
//      |                                            |
//      |     +-----------------+    |   target() * 2 + 1
//      |     |                                |     |
//      |     |                                |     |
//      |     |                                |     |
//      |     |                                |     |
//      |     |               ^               |     |
//      |     |                                |     |
//      |     |                                |     |
//      |     |                                |     |
//      |     +-----------------+    |
//      |                                            |
//      |                                            |
//      +------------------------+
//

void rcKinetoscope::selfSimilarMotionField_AttentiveCapture ()
{
#if 0
  int32 index (0);
  rcAutoCorrelation ac;
  rsCorr5by5Line result;
  const rcIPair& iOrigin = mOrigin;
  const rcIPair dimension = connectionSize ();
  const rcIPair targetSize = targetRect().size();

  // @note this corresponds to requirement by gen5x5line that the model
  // it uses is -4,-4 fo the searchSize passed in.
  const rcIPair searchSpace (5, 5);
  rcIPair target4moments = iOrigin * sTwo + rcIPair ((int32) (mOrigin.x()+mOrigin.x()),
						    (int32) (mOrigin.y()+mOrigin.y()));

  rmAssert (dimension >= target4moments);
  rmAssert (searchSpace.x() == searchSpace.y());
  rmAssert (searchSpace.x() == 5);

  // @note this is the property of kinetoscope: 8bit is supported by SIMD on PPC.
  bool simd_ppc_or_pixel8 = rfHasSIMD () && use4fixed().depth() == rcPixel8;
  if (simd_ppc_or_pixel8)
    ac.update(use4fixed (), 2, rcAutoCorrelation::eFull, rcAutoCorrelation::eLine);
  else
    ac.update(use4fixed (), 2, rcAutoCorrelation::eFull, rcAutoCorrelation::eLine, false, targetSize);

  // @note this is the property of kinetoscope:
  float cutOffVarianceTimesSize = mCutOff;
  cutOffVarianceTimesSize *= ((targetSize.x()* targetSize.y() - 1));
  cutOffVarianceTimesSize *= (targetSize.x()*targetSize.y());

  const rcWindow& mAi = fixedDirection ();
  rmAssert (mAi.depth() == rcPixel8);
  rcRectangle<int32> localPframe (pframe ());
  translateFromFrame (localPframe);

  for (int32 j = iOrigin.y(); j < (dimension.y() + iOrigin.y()); j++)
    {
      if (!localPframe.contains (rcIPair (iOrigin.x(), j) )) continue;

      // Fetch the row pointer for grad max.
      const uint8 *dirPtr = (const uint8 *) mAi.pelPointer (iOrigin.x(), j);

      if (eScanDir > 1 && !(j%eScanDir)) continue;

      if (simd_ppc_or_pixel8)
	{
	  ac.gen5by5Line(target4moments, j - iOrigin.y(), result);
	  rmAssert ((int32) result.count == dimension.x());
	}
      else
	{
	  ac.genLine(targetSize, j - iOrigin.y(), result);
	  rmAssert ((int32) result.count == (dimension.x()));
	}

      for (int32 i = iOrigin.x(); i < (int32) (result.count+iOrigin.x()); i++, dirPtr++)
	{
	  if (eScanDir > 1 && !(i % eScanDir)) continue;

	  if (!localPframe.contains (rcIPair (i, j) )) continue;

	  const uint32 ii (i - iOrigin.x());

	  if (sourceMotion () == eLongExposure && ! simd_ppc_or_pixel8 &&
	      (*(result.score[2][2] + ii ) ) < 1)
	    continue;
	  else if (*(result.score[2][2] + ii ) < cutOffVarianceTimesSize)
	    continue;

	  static const rcAngle8 prepend8 (uint8 (64));
	  rcAngle8 tt (*dirPtr);
	  tt += prepend8;
	  rcRadian rr (tt);
	  rc2Fvector disp (1.0f, rr);
	  const rc2Fvector anchor (i + mOrigin.x(), j + mOrigin.y());
	  rcMotionVectorPath mvp (rcSharedKinetoscope (this), anchor);
	  index = (i<<16)+j;
	  const pair<int32, rcMotionVectorPath> dmv (index, mvp);
	  mPathMap.insert (dmv);
	}
    }
#endif

}

void rcKinetoscope::epiMotionField ()
{
#if 0
  int32 index (0);
  rcAutoCorrelation ac;
  rsCorr5by5Line result;
  const rcIPair& iOrigin = connectionPhase ();
  static const rcIPair unit (1,1);
  static const rcIPair two (2,2);
  rcIPair dimension = iframe().size();
  rcIPair targetSize = target() * two + unit;

  // @note this corresponds to requirement by gen5x5line that the model
  // it uses is -4,-4 fo the searchSize passed in.
  const rcIPair searchSpace (5, 5);
  rcIPair target4moments = targetSize + two + two;
  dimension -= target4moments; // dilated by half search size on both sides
  dimension += unit;
  rmAssert (dimension >= target4moments);
  rmAssert (searchSpace.x() == searchSpace.y());
  rmAssert (searchSpace.x() == 5);

  // @note this is the property of kinetoscope: 8bit is supported by SIMD on PPC.
  bool simd_ppc_or_pixel8 = rfHasSIMD () && use4fixed().depth() == rcPixel8;
  if (simd_ppc_or_pixel8)
    ac.update(use4fixed (), 2, rcAutoCorrelation::eFull, rcAutoCorrelation::eLine);
  else
    ac.update(use4fixed (), 2);

  // @note this is the property of kinetoscope:
  float cutOffVarianceTimesSize = mCutOff;
  cutOffVarianceTimesSize *= ((targetSize.x()* targetSize.y() - 1));
  cutOffVarianceTimesSize *= (targetSize.x()*targetSize.y());
  rcHistoStats h (use4fixed());

  const rcWindow& mAi = mFrameData[eMoving].direction (detail());
  rmAssert (mAi.depth() == rcPixel8);


  for (int32 j = iOrigin.y(); j < (dimension.y() + iOrigin.y()); j++)
    {
      const uint8 *gPtr = (const uint8 *) use4fixed().pelPointer (iOrigin.x(), j);
      const uint8 *dirPtr = (const uint8 *) mAi.pelPointer (iOrigin.x(), j);

      if (!(j%eScanDir)) continue;

      if (simd_ppc_or_pixel8)
	ac.gen5by5Line(target4moments, j - iOrigin.y(), result);
      else
	ac.genLine(targetSize, j - iOrigin.y(), result);

      rmAssert ((int32) result.count == dimension.x());

      for (int32 i = iOrigin.x(); i < (int32) (result.count+iOrigin.x()); i++, gPtr++, dirPtr++)
	{
	  if (*gPtr <= h.median ()) continue;
	  if (!(i % eScanDir)) continue;
	  const uint32 ii (i - iOrigin.x());

	  if (*(result.score[2][2] + ii ) < cutOffVarianceTimesSize)
	    continue;

	  static const rcAngle8 prepend8 (uint8 (64));
	  rcAngle8 tt (*dirPtr);
	  tt += prepend8;
	  rcRadian rr (tt);
	  rc2Fvector disp (1.0f, rr);
	  const rc2Fvector anchor (i + mOrigin.x(), j + mOrigin.y());
	  rcMotionVectorPath mvp (rcSharedKinetoscope (this), anchor, disp);
	  index = (i<<16)+j;
	  const pair<int32, rcMotionVectorPath> dmv (index, mvp);
	  mPathMap.insert (dmv);
	}
    }
#endif

}




void rcKinetoscope::canonicalMotionField (const rcWindow& mEi)
{
#if 0
  const rcIPair unit (1,1);
  const rcIPair two (2,2);
  const rcIPair& iOrigin = mOrigin;
  rcIPair dimension = iframe().size();
  rcIPair targetSize = target() * two + unit;

  rcIPair target4moments = targetSize + two + two;
  dimension -= target4moments; // dilated by half search size on both sides
  dimension += unit;
  const rcIPair searchSpace (target4moments - targetSize + unit);
  rmAssert (searchSpace > unit);
  rmAssert (dimension >= target4moments);
  rmAssert (searchSpace.x() == searchSpace.y());
  rmAssert (searchSpace.x() == 5);

  int32 index (0);
  rc2Fvector disp;

  // @note Skip over zero bytes. Perhaps the best way is to run length!
  for (int32 j = iOrigin.y(); j < (dimension.y() + iOrigin.y()); j++)
    {
      const uint8 *gPtr = (uint8 *) mEi.pelPointer (iOrigin.x(), j);

      int32 d (1);
      for (int32 i = iOrigin.x(); i < (dimension.x() + iOrigin.x()); i+=d, gPtr+=d, d = 1)
	{
	  if (!(*(uint32 *) gPtr))
	    {
	      d = sizeof (uint32);
	      continue;
	    }

	  if (!(*(uint16 *) gPtr))
	    {
	      d = sizeof (uint16);
	      continue;
	    }

	  if (!(*(uint8 *) gPtr))
	    {
	      d = sizeof (uint8);
	      continue;
	    }


	  const rc2Fvector anchor (i + mOrigin.x(), j + mOrigin.y());
	  rcMotionVectorPath mvp (rcSharedKinetoscope (this), anchor, disp);
	  index = (i<<16)+j;
	  const pair<int32, rcMotionVectorPath> dmv (index, mvp);
	  mPathMap.insert (dmv);
	}
    }
#endif

}



#if 0
void rcKinetoscope::fieldMap (rcWindow& cn) const
{
  if (mPathMap.size())
    {
      cn = rcWindow (const_cast<rcKinetoscope *>(this)->connect().width(),
		     const_cast<rcKinetoscope *>(this)->connect().height(),
		     const_cast<rcKinetoscope *>(this)->connect().depth());
      cn.copyPixelsFromWindow (const_cast<rcKinetoscope *>(this)->connect());
    }
}

void rcKinetoscope::dumpMap (ostream& o)
{
  o << mPathMap.size() << endl;
  rcMotionPathMap::iterator dm;

  int32 w = const_cast<rcKinetoscope *>(this)->connect().width();
  int32 h = const_cast<rcKinetoscope *>(this)->connect().height();

  for (int32 j = 0; j < h; j++)
    {
      o << endl;
      for (int32 i = 0; i < w; i++)
	{
	  int32 dispair ((i<<16)+j);
	  dm = mPathMap.find (dispair);
	  if (dm == mPathMap.end())
	    {
	      o << "+";
	    }
	  else
	    {
	      o << ".";
	    }
	}
    }
}
#endif


/* @file
*
*$Id $
*$Log$
*Revision 1.61  2006/01/01 21:30:58  arman
*kinetoscope ut
*
*Revision 1.60  2005/11/17 23:19:06  arman
*kinetoscope resync
*
*Revision 1.59  2005/11/13 18:56:15  arman
*removed distance transform from dumpMap
*
*Revision 1.58  2005/11/09 20:26:21  arman
*use 2 1d interpolator for autocorr peak. Should use jacobian
*
*Revision 1.57  2005/11/08 20:34:29  arman
*cell lineage iv and bug fixes
*
*Revision 1.56  2005/11/04 22:04:24  arman
*cell lineage iv
*
*Revision 1.55  2005/10/31 11:49:04  arman
*Cell Lineage III
*
*Revision 1.54  2005/09/14 22:49:01  arman
*fixed min object size bug & removed square of cutOff
*
*Revision 1.53  2005/09/09 20:45:21  arman
*2.0 Pre
*
*Revision 1.52  2005/08/30 20:57:15  arman
*Cell Lineage
*
*Revision 1.53  2005/08/12 20:37:53  arman
*inc cell lineage plus
*
*Revision 1.52  2005/07/21 22:08:15  arman
*incremental
*
*Revision 1.51  2005/03/31 23:27:24  arman
*removed blitz includes. Added channelSelection
*
*Revision 1.50  2005/03/03 18:51:15  arman
*back to normal way before cardio fluo
*
*Revision 1.49  2005/03/03 14:46:02  arman
*added CardioCalcium
*
*Revision 1.48  2005/02/03 14:48:27  arman
*flu morphometry
*
*Revision 1.47  2005/01/07 16:26:59  arman
*addition of epiMotionField
*
*Revision 1.46  2004/12/19 18:23:05  arman
*found and fixed a temporal discrepency in time of autoCorrelation and motion comparison
*
*Revision 1.45  2004/12/16 21:45:58  arman
*updated wrt rcFeature removal
*
*Revision 1.44  2004/12/16 20:35:56  arman
*added @file and notes
*
*Revision 1.43  2004/09/21 18:15:10  arman
*ac aperture is not based on expected speed
*
*Revision 1.42  2004/09/17 21:27:30  arman
*added isNull()
*
*Revision 1.41  2004/09/15 01:22:29  arman
*corrected logic
*
*Revision 1.40  2004/09/13 20:42:40  arman
*incremental
*
*Revision 1.39  2004/08/27 22:16:54  arman
*in progress
*
*Revision 1.38  2004/08/26 13:35:07  arman
*switched to calling a more direct fit function
*
*Revision 1.37  2004/08/26 02:12:19  arman
*added blitz stencils
*
*Revision 1.36  2004/08/25 03:18:37  arman
*changes corresponding to motionVectorPath changes
*
*Revision 1.35  2004/08/24 21:35:14  arman
**** empty log message ***
*
*Revision 1.34  2004/03/17 01:25:38  arman
*added attentiveCapture field capture
*
*Revision 1.33  2004/03/16 21:36:08  arman
*supports motionSource
*
*Revision 1.32  2004/02/24 21:51:27  arman
*added spatial selective motion field computation
*
*Revision 1.31  2004/02/16 00:38:34  arman
*corrected selective motion vector processing
*
*Revision 1.30  2004/02/11 22:44:34  arman
*added processed rect support
*
*Revision 1.29  2004/02/03 14:57:26  arman
*misc
*
*Revision 1.28  2004/01/21 21:39:51  arman
*removed stats
*
*Revision 1.27  2004/01/18 23:23:46  arman
*removed declaration no longer needed.
*
*Revision 1.26  2004/01/18 20:23:12  arman
*removed include of stlplus
*
*Revision 1.25  2004/01/18 19:22:50  arman
*adde integrated sub-pixel interpolation support
*
*Revision 1.23  2004/01/14 20:38:00  arman
*Major changes
*
*Revision 1.22  2004/01/09 21:20:33  arman
*tmp ci
*
*Revision 1.21  2003/12/05 20:11:02  arman
*added adaptive handling of laminar supression
*
*Revision 1.20  2003/11/26 21:43:31  arman
*fixed bug in uniqueness checking
*
*Revision 1.19  2003/11/25 01:56:50  arman
*interim checnkin
*
*Revision 1.18  2003/11/10 01:18:42  arman
*consolidated ACF in to kinetoscope methods
*
*
* Copyright (c) 2002 Reify Corp. All rights reserved.
*/

