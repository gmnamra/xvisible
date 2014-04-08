/*
 *
 *$Id $
 *$Log$
 *Revision 1.18  2006/01/01 21:30:58  arman
 *kinetoscope ut
 *
 *Revision 1.17  2005/11/04 22:04:25  arman
 *cell lineage iv
 *
 *Revision 1.16  2005/08/30 20:57:16  arman
 *Cell Lineage
 *
 *Revision 1.16  2005/08/15 12:53:55  arman
 *Cell Lineage II
 *
 *Revision 1.15  2005/03/31 23:31:41  arman
 *possibly temporary edge detection change (smooth first)
 *
 *Revision 1.14  2005/01/07 16:27:26  arman
 *added imageRect accessor
 *
 *Revision 1.13  2004/12/21 22:50:51  arman
 *fixed (not tested) a bug with threshold inclusion
 *
 *Revision 1.12  2004/12/19 18:20:12  arman
 *added finer control of gradient processing
 *
 *Revision 1.11  2004/12/15 21:11:28  arman
 *pyramid intensity images are in temporal agreement with their base.
 *
 *Revision 1.10  2004/12/15 11:40:39  arman
 *added implementation for imageRect
 *
 *Revision 1.9  2004/08/24 21:34:53  arman
 **** empty log message ***
 *
 *Revision 1.8  2004/08/24 16:10:10  arman
 *added more correct unbound testing of resulting images
 *
 *Revision 1.7  2004/08/19 19:22:34  arman
 *switched to using the new exception macro
 *
 *Revision 1.6  2004/08/19 17:01:11  arman
 *fixed hystersis test and 0's the borders
 *
 *Revision 1.5  2004/08/19 16:06:52  arman
 *added assertion to make sure that single level xform is identity
 *
 *Revision 1.4  2004/08/19 15:58:21  arman
 *filters all deal with borders and do not offset the result (I think)
 *
 *Revision 1.3  2004/07/13 21:06:53  arman
 *added base
 *
 *Revision 1.2  2004/07/12 19:41:59  arman
 *first working version
 *
 *Revision 1.1  2004/07/12 02:15:08  arman
 *pyramid processing class
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include <rc_kinepyr.h>
#include <rc_histstats.h>
#include <rc_stats.h>
#include <rc_macro.h>

/************************
 *                      *
 * Pyramid Processing   *
 *                      *
 ************************/  

// TODO: implement regions

rcSpatialPyramid<2>::rcSpatialPyramid ()
{
}


rcSpatialPyramid<2>::rcSpatialPyramid 
(rcWindow& image, int32 sample, 
		    int32 baseSmooth, float edgeDetect,
					  float hyst, bool zc, bool highAccDir) : 
  mEdgeCount (0), mLeft (0), mZc (zc), m16bitDir (highAccDir)
{
  rmAssert (image.isBound());
    mSample = sample;
    mGranularity = baseSmooth;
    mEdgeDetect = edgeDetect;
    mHysteresis = hyst;

    if (granularity() > (rmMin (image.width(), image.height())) / 2)
      rmExceptionMacro(<< "Invalid Granularity");

    mLevels.resize (2);
    mLevelDirectionHists.resize (2);
    mEdgeCount.resize (2);
    mLeft.resize (2);
    intensityProcess (image, 0);
    intensityProcess (image, 1);

    //@note create histograms that are twice as wide for a wrap around. 
    // To match direction histograms we correlate the cannoical size in the ring 
    // of the other histogram
    mLevelDirectionHists[0] = vector<uint32> ((1 << ((8+1) * image.depth())));
    
    if (edgeDetect != 0.0f)
      gradientProcess ();
    else
      {
	for (uint32 fi = 0; fi < mLevels.size(); fi++)
	  {
	    mLevels[fi].push_back (mUnbound);
	    mLevels[fi].push_back (mUnbound);
	    mLevels[fi].push_back (mUnbound);
	  }
      }
    
  }

void rcSpatialPyramid<2>::intensityProcess (rcWindow& image, 
					 int32 level)
{
  rmAssert (level < (int32) (mLevels.size()));
  mLevels[level].clear ();

  // We smooth by a 5x5 Gaussian for every sample by 2. 
  // Sampling is done with Aspect ratio untouched

  if (level == 0)
    {
      mLevels[level].push_back (image);
  
      rcWindow mLi (image.width(), image.height(), image.depth());
      rfGaussianConv (image, mLi, mGranularity);
      mLi.atTime (image);
      mLevels[level].push_back (mLi);
      rmAssert (isIdentity ());
      return;
    }

  rmAssert (mSample > 1 && level == 1);

  rcWindow tmp (image.width(), image.height(), image.depth());
  int32 smoothKernelWidth = (mSample * 5) - 1;
  rfGaussianConv (image, tmp, smoothKernelWidth);
  rcWindow sampled = rfPixelSample (tmp, mSample, mSample);
  sampled.atTime (image);
  mLevels[level].push_back (sampled);  

  rcWindow mLi (sampled.width(), sampled.height(), sampled.depth());
  rfGaussianConv (sampled, mLi, mGranularity);
  mLi.atTime (image);
  mLevels[level].push_back (mLi);  
  const rcRadian zr (0.0);
  const bool is_odd = mSample % 2;  
 rc2Dvector translation (!is_odd ? -0.5 : 0.0, !is_odd ? -0.5 : 0.0);
  matrix (rcMatrix_2d (zr, rcDPair ((double) mSample, 
				  (double) mSample)));
  trans (translation);
}

/*
 * Edge detection maps to 2 3x3 operation (one is the combined polar 
 * the other one is peak detection). Smoothing is 5x5. 
 */
  

void rcSpatialPyramid<2>::gradientProcess ()
{
  rmAssert (mLevels[0].size () == 2 && mLevels[1].size () == 2);

  for (uint32 fi = 0; fi < mLevels.size(); fi++)
    {
      rcWindow& image = mLevels[fi][1];

      // Allocate images and 0 the borders
      rcWindow mEi (image.width(), image.height(), image.depth());
      rcWindow mGi (image.width(), image.height(), image.depth());
      rcWindow mAi (image.width(), image.height(), m16bitDir ? rcPixel16 : image.depth());

      rfSetWindowBorder(mEi, uint8 (0));
      rfSetWindowBorder(mGi, uint8 (0));
      if (!m16bitDir)
	rfSetWindowBorder(mAi, uint8 (0));
      else
	rfSetWindowBorder(mAi, uint16 (0));

      rcWindow energy ((mEi), 1, 1, (mEi).width()-2, (mEi).height()-2);	
      rcWindow grad ((mGi), 1, 1, (mGi).width()-2, (mGi).height()-2); 
      rcWindow angle ((mAi), 1, 1, (mAi).width()-2, (mAi).height()-2); 
      rfSobelEdge (image, energy, angle);

      rcHistoStats h (energy);
      uint8 low = (uint8) mEdgeDetect;
      uint8 high = (uint8) mHysteresis; 
      vector<uint32> hist = h.histogram ();
      hist[0] = 0; rcHistoStats hsz (hist);
      low = rmMax (1, (uint8) hsz.median () / 2);
      high = rmMax (low, (uint8) hsz.median () * 2);

      if (mHysteresis == 0.0f)
	{
	  mEdgeCount[fi] = (int32) (rfSpatialEdge ((mEi), (mAi), grad, low));
	}
      else
	{
	  rcWindow buffer ((mEi).width(), (mEi).height()); 
	  rcWindow gradholder (buffer, 1, 1, (mEi).width() - 2, (mEi).height() - 2);

	  if (mZc)
	    {
	      if (m16bitDir)
		rmExceptionMacro (<<" Unimplemented Functionality");

	      mEdgeCount[fi] = (int32) (rfSpatialEdge ((mEi), (mAi), gradholder, low));
	      rfHysteresisThreshold (gradholder, grad, low, high, mLeft[fi], 
				     mEdgeCount[fi] + 1, 0, 
				     edgeLabel ());
	    }
	  else
	    {
	      const vector<uint32>& hist = h.histogram ();
	      mEdgeCount[fi] = (int32) rfSum (hist);
	      for (int32 i = 0; i <  low; i++) mEdgeCount[fi] -= hist[i];
	      rfHysteresisThreshold (energy, grad, low, high, mLeft[fi], mEdgeCount[fi] + 1, 0,
				     edgeLabel ());
	    }
	}
      
      mLevels[fi].push_back (mEi);
      mLevels[fi].push_back (mAi);
      mLevels[fi].push_back (mGi);
    }
}

bool rcSpatialPyramid<2>::isValid () const
{
  bool v = mLevels.size() == 2 &&
    mLevels[0].size () > 1 &&
    mLevels[1].size () > 1 &&
    mLevels[0][0].isBound() &&
    mLevels[0][1].isBound() &&
    mLevels[1][0].isBound() &&
    mLevels[1][1].isBound();

  if (mEdgeDetect == 0.0f)
    return v;

  v = v &&
    mLevels[0].size () == 5 &&
    mLevels[1].size () == 5 &&
    mLevels[0][2].isBound() &&
    mLevels[0][3].isBound() &&
    mLevels[0][4].isBound() &&
    mLevels[1][2].isBound() &&
    mLevels[1][3].isBound() &&
    mLevels[1][4].isBound();
  
  return v;
}


rcIRect rcSpatialPyramid<2>::imageRect (int32 level) const
{
  if (isValid ())
    return mLevels[level][0].rectangle ();
  else 
    return rcIRect ();
}

const rcWindow& rcSpatialPyramid<2>::base (int32 level) const
{
  if (isValid ())
    return mLevels[level][0];
  else 
    return mUnbound;
}

const rcWindow& rcSpatialPyramid<2>::past (int32 level) const
{
  if (isValid ())
    return mLevels[level][1];
  else 
    return mUnbound;
}


const rcWindow& rcSpatialPyramid<2>::gradient (int32 level) const
{
  if (isValid ())
    return mLevels[level][2];
  else 
    return mUnbound;
}

const rcWindow& rcSpatialPyramid<2>::direction (int32 level) const
{
  if (isValid ())
    return mLevels[level][3];
  else 
    return mUnbound;
}

const rcWindow& rcSpatialPyramid<2>::edge (int32 level) const
{
  if (isValid ())
    return mLevels[level][4];
  else 
    return mUnbound;
}
