/*
 *
 *$Id $
 *$Log$
 *Revision 1.4  2006/01/15 22:56:51  arman
 *selective myo
 *
 *Revision 1.3  2006/01/10 23:42:17  arman
 *updated
 *
 *Revision 1.2  2006/01/01 21:30:58  arman
 *kinetoscope ut
 *
 *Revision 1.1  2005/08/30 21:03:24  arman
 **** empty log message ***
 *
 *Revision 1.1  2005/07/22 15:01:02  arman
 *divided visualcell
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

/*
 * Cell specific measure functions
 * @note: Input a polygon 
 *            Output: updating transforms and geometric info
 *            Does not set the state.
  // Constraint Processing. Ends of the fully extended cell are recorded and then used for
  // bracketing future end detection. At the moment it only continues as long as the constraint
  // fits the affineWindow size. It could always be with respect to the last first frame of a beat.
  // Setting Up For template Processing:
  // There are 3 templates for cardiac: the whole cell, and one for each end. 
  // Use the whole cell registration to setup for the end cells, enlarge the end cells to
  // have a capture range for shortening. In fact we can just use the 2 ends for everything!

 */

void rcVisualFunction::cardiacMeasure (vector<rcPolygon>::const_iterator& vc)  
{
  // @note Construction of a shape calculator is almost zero cost (everything is calculated when you need it and cached).
  // 
  mXform = rc2Xform ();
  rc2Fvector current;

  // First Frame: use pre-segmented information and use profiles to generate first Ends detection
  if (isAtStep (1) &&  !isState (eHasFullyExtended)) // Run end detector to determine the ends
    {
      rcPolygon refined (*vc);
      mShapeRef = boost::shared_ptr<rcShape> (new rcShape (use4fixed(), *vc) );
      rcAffineRectangle ar = mShapeRef->affineRect ();

      // @note not sure this belongs here
      mAffys.push_back (mShapeRef->affineRect ());
      bool side = ar.cannonicalSize().x() >  ar.cannonicalSize().y();
      setState (eIsAffineXdirected, side);
      int32 d = side ? ar.cannonicalSize().x() : ar.cannonicalSize().y();

      //@note todo: use projection on the other end to get a better than halfway placement
      //@note remove redundant calculations
      rcDPair endpair = mShapeRef->majorEnds ();
      mMajorEnds.x() =  (float) endpair.x();
      mMajorEnds.y() =  (float) endpair.y();
      rcFPair maj (mMajorEnds.x() / (double) d, mMajorEnds.y() / (double) d);
      mRad= d / 5;
      rmAssert (mRad >= 8);
      for (uint32 i = 0; i < 2; i++)
	{
	  mMajorEndPoints[i] = ar.affineToImage (rc2Fvector (side ? maj[i]: 0.5f, side ? 0.5f: maj[i]));
	}

      cardiacUpdate ();
    }
  else if (isAtStep (1) && isState (eHasFullyExtended))
    {
      //@note setup from the end points provided. 
      cardiacUpdate ();      
    }
  else 
    {
      rmAssert (!isAtStep (1) &&  isState (eHasFullyExtended));
      rcAffineRectangle ar = affys().back();
      int32 sEst = rmMax (1, rfRound (sideShorteningEstimate () * mFirstBeatLength, sDummy));
      rmAssert(sEst > 0);

      mTemplatePoses.clear();
      mTemplatePoses.resize (2);
      mSearchRects.clear();
      mSearchRects.resize (2);

      vector<rc2Fvector> ends (2);
      rcDPair newEnds;

      // First Locate the ends in this frame
      for (uint32 i = 0; i < mTemplates.size(); i++)
	{
	  // Get the template box and widen it according to the estimate
	  rcFRect fr = mTemplateRects[i];
	  int32 play (sEst);

	  rcIRect fixed = roundRectangle (fr);
	  rcWindow fixedw (use4fixed(), fixed, true);
	  while (play && !fixedw.trim (-play)) play--;	  
	  mSearchRects[i] = rcIRect (fixedw.rectangle().ul(), fixedw.rectangle().lr());
#if 0
	  std::string f ("/Users/arman/Desktop/fixed.tif");
	  fixedw.tiff (f);
	  std::string m  ("/Users/arman/Desktop/template.tif");
	  mTemplates[i].window().tiff (m);
#endif
	  double q = match (fixedw, mTemplates[i], mTemplatePoses[i]);
	  rmUnused (q);

	  //	  cerr << hex <<  (int32) use4fixed().frameBuf()->rowPointer (0) << " [" << i << "]: " << 
	  //	    mTemplatePoses[i] << "  " << int32 (q * 1000) << " (p: " << play << " )";
      
	  // End template wrt frame = position of template TL wrt fixedw + position fixedw in frame
	  mTemplatePoses[i] += rc2Fvector (fixedw.position ());

	  //	  cerr << "[" << i << "]: " << mTemplatePoses[i] << endl;
	}


      // Update TemplateRects: we start where we found the end the last time:
      for (uint32 rr = 0; rr < mTemplates.size(); rr++)
	{
	  mTemplateRects[rr] = rcFRect (mTemplatePoses[rr].x(), mTemplatePoses[rr].y(), mTemplateRects[rr].width(), 
				       mTemplateRects[rr].height());
	}

      // Add Template2end to Poses 
      for (uint32 tp = 0; tp < mTemplates.size(); tp++)
	{
	  mTemplatePoses[tp] = mTemplatePoses[tp] + mTemplateOrigins[tp];
	}

      // Using Center Position Translation, find out how much the cell has moved
      rc2Fvector newMidPoint ((mTemplatePoses[0] + mTemplatePoses[1]) / 2.0);
      mXform.trans (mMidPoint - newMidPoint);
      mMidPoint = newMidPoint;


      //      rcPolygon refined (*vc);
      //      refined.transform (mXform);
      //      mShapeRef = new rcShape (use4fixed(), refined);

      // How much has it moved from the origin (along the cardiac axis)
      for (uint32 ff = 0; ff < mTemplates.size(); ff++)
	{
	  rc2Dvector tmp ((double) mTemplatePoses[ff].x(), (double) mTemplatePoses[ff].y());
	  //	  tmp = mCardiacAxis.project (tmp);
	  tmp = ar.imageToAffine (tmp);
	  newEnds[ff] = isState (eIsAffineXdirected) ? tmp.x() : tmp.y();
	}

      // Get the affine dimension used as major
      int32 d = isState (eIsAffineXdirected)  ? ar.cannonicalSize().x() : ar.cannonicalSize().y();
      mMajorEnds.x() = newEnds.x() * d;
      mMajorEnds.y() = newEnds.y() * d;
      current = rc2Fvector (mTemplatePoses[0].distance (mTemplatePoses[1]), 0.0f);
  
      // We can now mark that we have had a contraction
      mV = current - mAffineSize;
      mAffineSize = current;
      moving (mRectSnaps.back());  

      mark(eMotion);
      mark(eContraction);
      mark(eDimensions);
    }
}


/*
 *     Affine Rectangle Dimesions
*      o  ______________________________________________ X-axis
*        |
*        |  Y-axis
*        |                     (0,1)
*        |                              / \
*        |              major.y()  /   \
*        |                            /     \    
*        |                           /    \  \ 
*        |                          /        /   (1,1) 
*        |                         /        /
*        |                        /        /
*        |                       /  \    /
*        |                      /    \  /
*        |                     /      \/ 
*        |                    /       /
*        |                   /       /   
*        |   (0,0)        /       /
*        |                  \  \   /
*        |                   \    /  major.x()
*        |                    \  /
*        |                     \/  (1,0)
*/  

void rcVisualFunction::cardiacUpdate ()
{
  if (isAtStep (1))
    {
      mTemplates.clear ();
      mTemplateRects.clear ();
      mTemplateOrigins.clear ();
      rcAffineRectangle ar = affys().back();
      bool side = ar.cannonicalSize().x() >  ar.cannonicalSize().y();
      setState (eIsAffineXdirected, side);
      int32 d = side ? ar.cannonicalSize().x() : ar.cannonicalSize().y();


      clearState (eHasFullyExtended);
      rc2Fvector csum;
      for (uint32 i = 0; i < 2; i++)
	{
	  rcIPair ic ((int32) mMajorEndPoints[i].x(), (int32) mMajorEndPoints[i].y());
	  csum += mMajorEndPoints[i];
	  int32 umin =  rmMin (ic.x(), ic.y());
	  if (umin < mRad) mRad =umin - 1;
	  rmAssert (mRad >= (int32)  eMinMyoOtherDimensionInPixels);
	  umin = use4fixed().width() - (1 + ic.x()+mRad); // -1 moved to - side
	  umin -= 1; // border of 1 from the edge of the image
	  if (umin < 0) mRad += umin; // reduce to fit
	  rmAssert (mRad >= (int32)  eMinMyoOtherDimensionInPixels);
	  umin = use4fixed().height() - (1 + ic.y()+mRad); // -1 moved to - side
	  umin -= 1; // border of 1 from the edge of the image
	  if (umin < 0) mRad += umin; // reduce to fit
	  rmAssert (mRad >= (int32)  eMinMyoOtherDimensionInPixels);
	  rcPolygon pg (mRad /2.0f, 8);
	  rc2Xform xt;
	  xt.trans (mMajorEndPoints[i]);
	  pg.transform (xt);
	  rcFRect fr;      rcIRect ir;
	  pg.orthogonalEnclosingRect (fr);
	  ir = roundRectangle (fr);	  
	  
	  rcWindow tmp (use4fixed (), ir, true);
	  rcWindow fixedCopy (ir.size());
	  fixedCopy.copyPixelsFromWindow (tmp);
	  rcCorrelationWindow<uint8> fixedw (fixedCopy);
	  mTemplates.push_back (fixedw);
	  mTemplateRects.push_back (fr);

	  mTemplateOrigins.push_back (rc2Fvector(mMajorEndPoints[i].x() - fr.ul().x(), mMajorEndPoints[i].y() - fr.ul().y()));
	  //	  cerr << hex << (int32) use4fixed().frameBuf()->rowPointer (0) << " [" << i << "]: " << 
	  //	    "Template [" << mTemplates.size() << "] made: " << fr << " i " << ir << endl;
	}

      // Construct a line from one end to the other end
      csum /= 2.0; // center in affine coordinates
      mMidPoint = rc2Fvector ((float) csum.x(), (float) csum.y());
      mAffineSize = rc2Fvector (mMajorEndPoints[0].distance (mMajorEndPoints[1]), 0.0f);
      mFirstBeatLength = mAffineSize.x();

      setState (eHasFullyExtended);
    }
}

//@note percentage based on current length and periodicity
// quadratic: y = -4x^2 + 4x, maxima at 0.5, 1.0, anchor at 0.0, 0.0 and 1.0, 0.0
// mod (frameNumber, FramesPerPeriod) / FramesPerPeriod 0 << >> 1

float rcVisualFunction::sideShorteningEstimate () const
 {
   float f = fmod ((double) steps(), (double) framesPerPeriod()) / framesPerPeriod();
	 f = -4*f*(f - 1); // gEstSideShortening;
   return f;
 }
 




void rcVisualFunction::beatProcess  ()
 {
   // As long as the size is less than frames per period
   // built up the time series
   if ((int32) mDimensionTs.size() < ((int32)framesPerPeriod ()))
     {
       mDimensionTs.push_back (age());
       mDimensions.push_back (mAffineSize.x());
       rmAssert (mDimensionTs.size() == mDimensions.size());
     }

   if (mDimensionTs.size() >= (uint32) integralFramesPerPeriod ())
     {
       rmAssert (mDimensionTs.size() == mDimensions.size());
       clear (eBeatInfo);

       // What is the median dimension
       // Must be after the first length measurement. 
       // @note: if we have less than 12 frames per 
       if (mDimensionTs.size() < ((uint32)eMinFramesPerPeriod4Stat))
	 mBeatMedian = mFirstBeatLength;
       else
	 mBeatMedian = rfMedian (mDimensions);
       
       // What is the minimum and when is it reached
       vector<float>::iterator minL = min_element (mDimensions.begin(),
						   mDimensions.end());
       uint32 index = minL - mDimensions.begin();

       // interpolate and get min length and time
       // Report 
       // Interpolation is on Median - Length (t - 1), 
       //                     Median - Length (t), 
       //                     Median - Length (t + 1)

       float minIntL (0), minIntT (0), minIntNormT (0);
       if (index > 0 && index < (mDimensions.size() - 1))
	 {
	   minIntNormT = minIntT = parabolicFit (mBeatMedian - mDimensions[index-1],
						 mBeatMedian - *minL,
						 mBeatMedian - mDimensions[index+1], 
						 &minIntL);
	 }

       // Peak return is mBeatMedian - minimumLength. 
       // Scale Time according to the actual
       if (minIntT > 0.0) minIntT *= (mDimensionTs[index+1] - mDimensionTs[index]);
       if (minIntT < 0.0) minIntT *= (mDimensionTs[index] - mDimensionTs[index-1]);
       minIntL = mBeatMedian - minIntL;

       // If Interpolated is too close or inconclusive/degenerate use the measured minimum
       if (real_equal (minIntL, 0.0f) || real_equal (minIntT, 0.0f) || real_equal (*minL, minIntL, 1.0f))
	 minIntL = *minL;
       mBeatShortenning = 1.0f - minIntL / mBeatMedian;

       // Construct Contraction and Relaxation immediate angles

       // Actual time stamp at the minimum is sub-frame time + time at the center.

       minIntT += mDimensionTs[index];
       rcLineSegment<float> cLine (rc2Fvector (minIntNormT , minIntL), rc2Fvector(-1.0f,
										  mDimensions[index-1]));
       rcLineSegment<float> rLine (rc2Fvector (minIntNormT , minIntL), rc2Fvector(+1.0f,
										  mDimensions[index+1]));	  
       // Put all the LTs out including the interpolated value, in order of increasing time. 
       // A little inefficient for now
       uint32 fi;
       for (fi = 0; fi <= index; fi++)
	 {
	   mInterpolatedMin.push_back (rcFPair (mDimensionTs[fi], mDimensions[fi]));
	 }

       if (minIntL != *minL)
	 mInterpolatedMin.push_back (rcFPair (minIntT , minIntL ));

       // Assert that iL is within +/- 1 from minL

       for (; fi < mDimensions.size(); fi++)
	 {
	   mInterpolatedMin.push_back (rcFPair (mDimensionTs[fi], mDimensions[fi]));
	 }
	  
       // Transform Angles to our definition:
       // ContractionAngle = - (90 + (- cLine.angle())
       // RelaxationAngle = 90 - rLine.angle()
       rcRadian contraction (cLine.angle());
       rcRadian relaxation (rLine.angle());
       contraction.normSigned();
       relaxation.normSigned ();
       contraction = - (rkPI/2.0 + (- contraction.Double()));
       relaxation = rkPI/2.0 - relaxation.Double();
       mPeakAngles.push_back (rcPair<rcRadian> (contraction, relaxation));

       // Clear temporal buffer 
       mDimensionTs.clear ();
       mDimensions.clear ();
       mark (eBeatInfo);
       mBeatNumber += 1;
     }

   // If we have at least 2 beats worth of frames
   if (mBeatNumber > 1 && similarity().size() > mBeatAC.size())
     {
       clear (ePacingInfo);
       vector<float>::const_iterator Begin = similarity().end();
       advance (Begin, - (int32) mBeatAC.size());
       rf1DAutoCorr (Begin, similarity().end(), mBeatAC);
       rs1DFourierResult rslt;
       rf1DFourierAnalysis(mBeatAC, rslt, rc1DFourierForceFFT);
       mContractionFrequency = (float) mBeatAC.size() / rslt.peakFrequency;
       mark (ePacingInfo);       
     }
 }

