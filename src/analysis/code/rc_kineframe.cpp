/*
 * @file
 *$Id $
 *$Log$
 *Revision 1.5  2006/01/11 17:46:17  arman
 *removed debugging output
 *
 *Revision 1.4  2006/01/10 23:42:59  arman
 *pre-rel2
 *
 *Revision 1.3  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.2  2005/11/20 03:52:15  arman
 *motion 2 fixed fixed
 *
 *Revision 1.1  2005/11/18 17:43:59  arman
 *frame and grabbing functions
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_kinetoscope.h>
#include <rc_ipconvert.h>

void rcKinetoscope::advance ()
{

  rcTimingInfo profile (10, 1);
  profile.nextPass (true);

  // Debugging display
  profile.touch (0, std::string ("Start"));
  if (isCardiacCell ())
    {
      debugScreen (mFrameData[eFixed].base (detail ()));
    }
  else
    {
      if (organism() !=  rcOrganismInfo::eModel || organismName () != rcOrganismInfo::eDanioRerio)
	{
	  if (channelAvailable ())
	    debugScreen (mChannelData[eFixed].base (detail ()));
	  else
	    debugScreen (connect());
	}
    }

 // swap is constant time for containers. We keep a copy of the previous POLYS. 
  profile.touch (1, std::string ("Debug"));
  mFixedPOLYs = mPOLYs;
      
  // Rotate Frame Collections
  profile.touch (2, std::string ("Poly Shuffle"));
  rotate (mFrameData.begin(), mFrameData.end() - 1, mFrameData.end());

  // Rotate Frame Collections
  profile.touch (3, std::string ("Frame Data Shuffle"));
  if (mChannelData.size())
    rotate (mChannelData.begin(), mChannelData.end() - 1, mChannelData.end());

  // Rotate Connection Maps
  profile.touch (4, std::string ("Channel Data"));
  rotate (mConnectionMap.begin(), mConnectionMap.end() - 1, mConnectionMap.end());

  // Rotate Moment Maps
  profile.touch (5, std::string ("Connection Map Shuffle"));
  rotate (mMomentMap.begin(), mMomentMap.end() - 1, mMomentMap.end());

  // @note use motion based or field based segmentation flags for this
  profile.touch (5, std::string ("Moment Map Shuffle"));
  if (sourceMotion() != eContractile)
    {
      // Clear Temporal
      rcWindow all (connect().frameBuf());
      all.setAllPixels (0);
    }

  // Other Clearing if we have to
  //  velocityXfield().setAllPixels (0);
  //  velocityYfield().setAllPixels (0);

  profile.touch (5, std::string ("Temporal Cleanup"));

  profile.printInfo (1);

}


uint32 rcKinetoscope::startFrame() const
{
    switch ( mSource ) {
        case eVideoCache:
            return mPhase;
        case eFileGrabber:
            return mPhase+1;
        default:
            rmAssert( 0 );
            return mPhase;
    }
}

int32 rcKinetoscope::count() const
{
  return mCount;
}

int32 rcKinetoscope::frameCount () const
{
  int32 count (0);

  if (mSource == eVideoCache)
    {
      rmAssert (mVc);
      count = mVc->frameCount ();
    }
  else if (mSource == eFileGrabber)
    {
      rmAssert (mFg);
      count = mFg->frameCount ();
    }
  else
    {
      rmAssert (1);
    }

  return count;
}

bool rcKinetoscope::frame (uint32 deltaFrames, const Time when)
{
  rcSharedFrameBufPtr buf;
  if (mSource == eVideoCache)
    {
      uint32 c = mCount+deltaFrames;
      mVcStatus = mVc->getFrame (c, buf, 0);
      if (mVcStatus != eVideoCacheStatusOK)
          return false;
      rmAssert (mVc);
    }
  else if (mSource == eFileGrabber)
    {
      for (uint32 i = 0; i < deltaFrames; i++)
	{
	  mFgStatus = mFg->getNextFrame (buf, 1);
	  if (mFgStatus == eFrameStatusEOF)
          return false;
	  rmAssert (mFgStatus == eFrameStatusOK);
	}
    }
  else
    {
      rmAssert (0);
    }

  
  // A blank rectangle implies entire
  // Else if the rectangle is within the frame use it.
  if (!count()) // First Frame
    {
      rcIRect thisBuf (0, 0, int32 (buf->width()),
		       int32 (buf->height()));

      if (mUserRect == rcIRect (rcIPair (0,0), rcIPair (0,0)))
	mUserRect = thisBuf;
      else if (!thisBuf.contains (mUserRect))
	rmExceptionMacro(<< "Kinetoscope: invalid window");

      mStartAbsTimeStamp = buf-> timestamp ();

    }

  // Record The Current Time: in Msec
  // Note: do this before any processing
  mCurrentAbsTimeStamp = buf-> timestamp ();
  mLastElapsedTimeSecs = mElapsedTimeSecs;
  mElapsedTimeSecs = mCurrentAbsTimeStamp.secs() - startingAbsoluteTime ().secs();

  if (count() > 1)
    rmAssert (mElapsedTimeSecs > mLastElapsedTimeSecs);

  // @note Upto hear there are no assumption on the kind of frame
  // we have (number channels and so on. Frames should be 8 bit
  // with the exception of:
  // 1. fileOrgType movieOriginGrayIsVisibleAlphaIsFlu

  bool combinedPhaseAndFluor = (fileOriginType().origin () == movieOriginGrayIsVisibleAlphaIsFlu);
  rmAssert ((buf->depth() == rcPixel8) == (!combinedPhaseAndFluor));

  rcWindow tmp, channel;
  if (combinedPhaseAndFluor)
    {
      rmAssert (buf->depth() == rcPixel32);
      vector<rcWindow> planes;
      rcWindow argb (buf);
      rfImageConvertARGBto8888 (argb, planes);
      rmAssert(planes.size() == 4);
      channel = rcWindow (planes[0], uframe().origin().x(), uframe().origin().y(),
			  uframe().width(),uframe().height());
      channel.atTime (argb);
      tmp = rcWindow (planes[1], uframe().origin().x(), uframe().origin().y(),
			  uframe().width(),uframe().height());
      mark (eMutualChannel);
      tmp.atTime (argb);
    }
  else
    {
      tmp = rcWindow (buf, uframe().origin().x(), uframe().origin().y(),
		uframe().width(),uframe().height());
    }

  //@note: fix this!
  // double h = (sourceMotion () == eDirectedFlow || 
// 	      sourceMotion () ==  eEpiFluorescence) ? 0.0 : 
//     laminarSuppression ();

  //@note this needs to be fixed
  mFrameData[when] = rc2LevelSpatialPyramid (tmp, 2, 5, 2,
					     0.0 , false);
  if (channelAvailable () && channel.isBound ())
    {
      mChannelData[when] = rc2LevelSpatialPyramid (channel, 
						   2, 5, 2,
						   0.0 , false);
    }      

  // @todo we need two Xforms but for now this is ok. 
  if (detail () == eCoarse)
    mGlobalXform = rc2Xform (static_cast<rc2Xform> (mFrameData[when]));
  mRect = mFrameData[when].imageRect (detail());

  mFpFrame =  rcFRect ((float) mRect.ul().x(), (float) mRect.ul().y(), 
		       (float) mRect.width(), (float) mRect.height());

  //@note if we need to use motion field
  if (!isCardiacCell())
    {
      // update moment generators
      mMomentMap[when]->update(use4when(when));

      // update point corr
      if (when == eMoving && count() >= 1)
	{
	  /* @note Within the context of kinetoscope's correspondence of fixed to
	   * moving where fixed is before moving in time, "fixed" corresponds to 
	   * mdlWin and "moving" correspondes to imgWin. 
	   */
	//  mPointCorr.update (use4moving(), use4fixed(), *mMomentMap[eMoving], *mMomentMap[eFixed] );
	}
    }

  mProcessRect = mFrameData[when].region ();

  //  cerr << hex << "[" << (uint32) use4fixed().frameBuf()->rowPointer (0);
  //  if (mCount > 0) 
  //    cerr << hex << ", " << (uint32) use4moving().frameBuf()->rowPointer (0);
  //  cerr << "]" << dec << endl;


  // @note: count from the start frame.
  mCount ++; 

  return true;

}




void rcKinetoscope::reichardt (Time when)
{
  if (count () <= mSimPipe)
    {
      mSimBuffer.push_back (mFrameData[when].base (detail ()));
      if (mSimBuffer.size() == (uint32) mSimPipe)
	{
	  rmAssert (mLsm->fill (mSimBuffer.begin(), mSimBuffer.end()));
	}
    }
  else if (count() > mSimPipe)
    {
      mLsm->update (&mFrameData[when].base (detail ()));
      rcWindow ttc, ttd;
      mLsm->ttc (ttc, ttd);

      // resultant_pixel_color = top_pixel_alpha/255 * top_pixel_color + (1 - top_pixel_alpha/255) * bottom_pixel_color

      vector<rcWindow> iargb (4);
      iargb[0] = ttc;
      iargb[1] = mFullImage;
      iargb[2] = mFrameData[when].base (detail ());
      iargb[3] = mFrameData[when].base (detail ());
      rcWindow grayargb = rfImageConvert8ToARGB (mFrameData[when].base (detail ()), mFullImage);
      rcWindow redmask = rcWindow (grayargb.width(), grayargb.height(), rcPixel32);
      rfImageConvert8888ToARGB (iargb, redmask);
      vImage_Buffer top, bot, dst;
      grayargb.vImage (bot);
      redmask.vImage (top);
      vImage_Error ve;
      rcWindow blend = rcWindow (grayargb.width(), grayargb.height(), rcPixel32);
      blend.vImage (dst);
      ve = vImageAlphaBlend_ARGB8888 (&top, &bot, &dst, kvImageNoFlags);
      iargb[0] = ttd;
      iargb[1] = mFrameData[when].base (detail ());
      iargb[2] = mFrameData[when].base (detail ());
      iargb[3] = mFullImage;
      rfImageConvert8888ToARGB (iargb, redmask);
      ve = vImageAlphaBlend_ARGB8888 (&top, &dst, &bot, kvImageNoFlags);
      debugScreen (grayargb);
    }
}

