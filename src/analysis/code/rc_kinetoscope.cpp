/* @file
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.157  2006/01/11 17:48:27  arman
 *removed silly debug graphics code
 *
 *Revision 1.156  2006/01/10 23:42:59  arman
 *pre-rel2
 *
 *Revision 1.155  2006/01/01 21:30:58  arman
 *kinetoscope ut
 *
 *Revision 1.154  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.153  2005/11/20 03:52:15  arman
 *motion 2 fixed fixed
 *
 *Revision 1.152  2005/11/18 22:01:25  arman
 *incremental
 *
 *Revision 1.151  2005/11/17 23:19:06  arman
 *kinetoscope resync
 *
 *Revision 1.150  2005/11/13 19:25:40  arman
 *debug map dump
 *
 *Revision 1.149  2005/11/07 17:32:08  arman
 *cell lineage iv and bug fix
 *
 *Revision 1.148  2005/11/04 21:59:24  arman
 *epi is now also based on motion bases segmentation
 *
 *Revision 1.147  2005/11/02 21:03:59  arman
 *added prototype work on level setting
 *
 *Revision 1.146  2005/11/01 22:59:41  arman
 *mutualInfo level setting
 *
 *Revision 1.145  2005/10/31 22:47:51  arman
 *added level segmentation support
 *
 *Revision 1.144  2005/10/31 11:49:04  arman
 *Cell Lineage III
 *
 *Revision 1.143  2005/09/29 20:43:51  arman
 *fixed minSize specification bug
 *
 *Revision 1.142  2005/09/28 22:06:06  arman
 *fixing issue surrounding getting blood flow working again
 *
 *Revision 1.141  2005/09/27 21:32:27  arman
 *added prefill for directed flow
 *
 *Revision 1.140  2005/09/13 23:23:29  arman
 *Bringing up Cardio
 *
 *Revision 1.139  2005/09/09 20:45:21  arman
 *2.0 Pre
 *
 *Revision 1.138  2005/09/01 22:03:03  arman
 *Cell Lineage Cleanup
 *
 *Revision 1.137  2005/08/31 23:59:54  arman
 **** empty log message ***
 *
 *Revision 1.136  2005/08/30 20:57:16  arman
 *Cell Lineage
 *
 *Revision 1.159  2005/08/24 02:40:38  arman
 *Cell Lineage II
 *
 *Revision 1.158  2005/08/23 23:32:31  arman
 *Cell Lineage II
 *
 *Revision 1.157  2005/08/22 15:32:38  arman
 *default ctor is now useful for testing
 *
 *Revision 1.156  2005/08/18 02:36:27  arman
 *Cell Lineage II
 *
 *Revision 1.155  2005/08/17 12:45:22  arman
 *Cell Lineage II
 *
 *Revision 1.154  2005/08/15 20:29:38  arman
 *Cell Lineage II
 *
 *Revision 1.153  2005/08/15 12:53:55  arman
 *Cell Lineage II
 *
 *Revision 1.152  2005/08/13 23:57:48  arman
 *removed debugging output
 *
 *Revision 1.151  2005/08/12 20:37:53  arman
 *inc cell lineage plus
 *
 *Revision 1.150  2005/08/05 21:53:08  arman
 *position issue update
 *
 *Revision 1.149  2005/08/03 21:47:36  arman
 *added profiling probes
 *
 *Revision 1.148  2005/08/01 21:18:43  arman
 *cell lineage
 *
 *Revision 1.147  2005/08/01 12:20:11  arman
 *cell lineage
 *
 *Revision 1.146  2005/08/01 02:23:31  arman
 *removed circle filtering for now
 *
 *Revision 1.145  2005/08/01 01:47:25  arman
 *cell lineage addition
 *
 *Revision 1.144  2005/07/31 06:39:16  arman
 *cell lineage incremental
 *
 *Revision 1.143  2005/07/30 19:39:52  arman
 *epi is eFixed
 *
 *Revision 1.142  2005/07/27 23:40:37  arman
 *Moved rc_mathmodel.h to include dir
 *
 *Revision 1.141  2005/07/26 20:03:19  arman
 *epi has pre-advance setup now. Todo: pre-advance motion vector option
 *
 *Revision 1.140  2005/07/26 00:54:09  arman
 *reverse pipeline. after step the data is at the same temporal place as
 *kinetoscope.
 *
 *Revision 1.139  2005/07/25 01:45:41  arman
 *removed debugging displays
 *
 *Revision 1.138  2005/07/23 02:20:47  arman
 *removed debugging flag
 *
 *Revision 1.137  2005/07/22 15:01:44  arman
 *divided visualcell
 *
 *Revision 1.136  2005/07/21 22:09:00  arman
 *incremental
 *
 *Revision 1.135  1970/01/01 17:35:42  arman
 *incremental
 *
 *Revision 1.133  2005/07/16 02:26:17  arman
 *incremental
 *
 *Revision 1.132  2005/07/11 09:53:27  arman
 *incremental
 *
 *Revision 1.131  2005/07/01 21:05:39  arman
 *version2.0p
 *
 *Revision 1.138  2005/06/27 23:45:47  arman
 *debugScreen time placement (test needed)
 *
 *Revision 1.137  2005/06/27 15:27:32  arman
 *added daughter processing
 *
 *Revision 1.136  2005/06/25 20:30:15  arman
 *reconcile only functions at the start of a kinetoscope
 *reconcile. Avoiding children
 *
 *Revision 1.135  2005/06/24 21:02:58  arman
 *cleanup time
 *
 *Revision 1.134  2005/06/09 21:52:53  arman
 *incremental
 *
 *Revision 1.133  2005/06/05 06:59:30  arman
 *changed nature of the debug screen content
 *
 *Revision 1.132  2005/06/02 01:00:06  arman
 *added accessor to the multiplier
 *
 *Revision 1.131  2005/05/31 01:28:21  arman
 *fixed handling of directed flow
 *
 *Revision 1.130  2005/05/10 01:04:14  arman
 *adopted to the latest rcVisualFunction changes.
 *
 *Revision 1.129  2005/04/25 01:46:15  arman
 *additional api for organism name and type
 *
 *Revision 1.128  2005/04/12 22:21:24  arman
 *added framePerSecond
 *
 *Revision 1.127  2005/03/31 23:25:00  arman
 *fixed a bug in createFunctionSpecificObjects and various smaller fixes
 *
 *Revision 1.126  2005/03/10 20:51:29  arman
 *commented out section for division output
 *
 *Revision 1.125  2005/03/03 18:49:26  arman
 *removed earlier cardio fluor design
 *
 *Revision 1.124  2005/03/03 14:44:29  arman
 *added cardiomycocytecalcium
 *
 *Revision 1.123  2005/02/28 22:58:48  arman
 *connect the sim processors
 *
 *Revision 1.122  2005/02/28 01:28:46  arman
 *adding flu support
 *
 *Revision 1.121  2005/02/22 18:42:40  arman
 *added population results
 *
 *Revision 1.120  2005/02/06 09:00:46  arman
 *fixed mCount increment bug
 *
 *Revision 1.119  2005/02/03 14:49:32  arman
 *flu morphometry
 *
 *Revision 1.118  2005/01/27 08:46:44  arman
 *fixed a bug in default detail () setting
 *
 *Revision 1.117  2005/01/26 11:45:49  arman
 *added additional flow graphics. adjustment of hyst for sourceMotion
 *
 *Revision 1.116  2005/01/26 00:19:28  arman
 *incremental
 *
 *Revision 1.115  2005/01/25 09:41:00  arman
 *added Directedflow sourceMotion
 *
 *Revision 1.114  2005/01/24 16:41:27  arman
 *updated debug display for new development
 *
 *Revision 1.113  2005/01/21 18:23:04  arman
 *added opacity debug image for spatio-temporal segmentation
 *
 *Revision 1.112  2005/01/19 22:53:57  arman
 *modified reichardt
 *
 *Revision 1.111  2005/01/18 02:46:35  arman
 *removed the ungle debug() code.
 *
 *Revision 1.110  2005/01/17 14:49:13  arman
 *mSimPipe is instatenous
 *
 *Revision 1.109  2005/01/14 21:29:23  arman
 *added latticeSimilarator
 *
 *Revision 1.108  2005/01/07 16:28:02  arman
 *fixed major bug in pipline handling
 *
 *Revision 1.107  2004/12/21 22:59:02  arman
 *resize mMomentMap
 *
 *Revision 1.106  2004/12/21 22:49:05  arman
 *re-arranged advance. Incorporated momentGeneration.
 *
 *Revision 1.105  2004/12/20 22:47:45  arman
 *fixed the case when detail is efine.
 *
 *Revision 1.104  2004/12/19 18:24:27  arman
 *changed call to kinePyramid generation. Should be rolled in to a organism specification class
 *
 *Revision 1.103  2004/12/16 21:46:51  arman
 *minor fixes
 *
 *Revision 1.102  2004/12/15 22:09:31  arman
 *added copy of resolution transformation to the global xform
 *
 *Revision 1.101  2004/12/15 21:15:31  arman
 *first working version of pyramid operation
 *
 *Revision 1.100  2004/12/15 11:44:46  arman
 *incremental toward compeletion of detail() incorporation
 *
 *Revision 1.99  2004/12/07 22:35:32  arman
 *questionable motion vector
 *
 *Revision 1.98  2004/11/16 21:31:25  arman
 *added pframe ()
 *
 *Revision 1.97  2004/09/21 14:29:50  arman
 *debug graphics uses pixelmap the smart way
 *
 *Revision 1.96  2004/09/19 22:18:28  arman
 *fixed bugs in advance
 *
 *Revision 1.95  2004/09/15 01:24:09  arman
 *removed velocityFields for now
 *
 *Revision 1.94  2004/09/13 20:43:13  arman
 *added accessor for radialSpeed
 *
 *Revision 1.93  2004/08/26 22:37:41  arman
 *fixed bug in frame. added accessors for velocityFields
 *
 *Revision 1.92  2004/08/25 03:19:53  arman
 *further cleanup
 *
 *Revision 1.91  2004/08/24 21:35:28  arman
 **** empty log message ***
 *
 *Revision 1.90  2004/08/23 10:37:35  arman
 **** empty log message ***
 *
 *Revision 1.89  2004/08/19 21:16:38  arman
 *added new exceptions
 *
 *Revision 1.88  2004/08/17 17:27:23  arman
 *fixed a bug in using point corr (same images were not used)
 *
 *Revision 1.87  2004/08/09 13:23:42  arman
 *removed unneccesary operations.
 *
 *Revision 1.86  2004/07/17 12:43:45  arman
 *switched to Gaussian smoothing for contractile motion
 *
 *Revision 1.85  2004/07/12 19:48:01  arman
 *updated wrt move of ip functions
 *
 *Revision 1.84  2004/07/01 11:16:03  arman
 *gradProcess is now a memeber function. temp median
 *
 *Revision 1.83  2004/06/15 21:56:14  arman
 *added separate file support
 *
 *Revision 1.82  2004/06/10 21:30:02  arman
 *changed output of dumpCSV
 *
 *Revision 1.81  2004/06/07 22:40:27  arman
 *added dumpCSV support
 *
 *Revision 1.80  2004/06/07 13:57:43  arman
 *corrected output format
 *
 *Revision 1.79  2004/06/06 01:46:21  arman
 *incremental
 *
 *Revision 1.78  2004/06/06 00:50:08  arman
 *altered output
 *
 *Revision 1.77  2004/06/04 15:49:51  arman
 *added ostream << for rcKinetoscope
 *
 *Revision 1.76  2004/04/20 23:26:19  arman
 **** empty log message ***
 *
 *Revision 1.75  2004/04/20 19:37:54  arman
 *fixed a bug in frame() where mCount was incremented incorrectly
 *
 *Revision 1.74  2004/04/20 15:39:38  arman
 *added implementation of setMask for presegmented bodies
 *
 *Revision 1.73  2004/04/19 21:36:19  arman
 *added temporal tracking for beat processing
 *
 *Revision 1.72  2004/04/05 14:52:14  arman
 *added period motion api
 *
 *Revision 1.71  2004/04/02 17:26:57  arman
 *reduced unnecessary steps in contractile motion
 *
 *Revision 1.70  2004/03/19 12:37:44  arman
 *adding contractile motion source
 *
 *Revision 1.69  2004/03/16 21:24:17  arman
 *added api to set organism info
 *
 *Revision 1.68  2004/02/24 21:52:14  arman
 *POLYs are alive for use in self-similar motion fields
 *
 *
 *Revision 1.4  2003/03/11 00:11:46  sami
 *const correctness improved
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */


#include <rc_kinetoscope.h>
#include <algorithm>
#include <functional>
#include <rc_edge.h>
#include <rc_histstats.h>
#include <iomanip>
#include <rc_qtime.h>
#include <rc_sparsehist.h>
#include <rc_platform.h>
#include <rc_imageprocessing.h>
#include <rc_time.h>
#include <rc_draw.h>
#include <rc_macro.h>
#include <rc_ipconvert.h>
#include <rc_mathmodel.h>
#include <rc_timinginfo.h>
#include <rc_level.h>

// UnComment for debgging printouts
//#include <rc_utdrawutils.h>

rcKinetoscope::rcKinetoscope ()
{
  mSample = 1;
  mPhase = 1;
  mRect = mUserRect = rcIRect ();
  mConnectionMap.resize (2);
  mFrameData.resize (2);
  mChannelData.resize (2);     
  mMomentMap.resize (2);
  detail (eFine);
  rcTimingInfo::turnOffPrint ();
  clear ();
  mDebug = false;
}

#define COMMONCTOR \
mSample = rmMax (sample, 1); \
mPhase = phase; \
mRect = mUserRect = rect; \
mConnectionMap.resize (2); \
mFrameData.resize (2); \
mChannelData.resize (2); \
mMomentMap.resize (2); \
detail (d); \
visualFunctionInfo (c, n);\
rcTimingInfo::turnOffPrint (); \
minMobSizeInPixels (minMobSize); \
debug (debuging); \
start ()

rcKinetoscope::rcKinetoscope (shared_video_cache& fg, const rcIRect rect, 
			      uint32 phase, uint32 sample, rcKinetoscope::Detail d, 
			      rcMovieFileOrgExt org, rcOrganismInfo::OrganismType c,
			      rcOrganismInfo::OrganismName n, int32 minMobSize, bool debuging)
: mSource (eVideoCache), mVc (fg), mOrgExt (org)
  {
    if (!mVc)
      rmExceptionMacro(<<"Kinetoscope: inValid Video Cache");      
    COMMONCTOR;
  }

rcKinetoscope::rcKinetoscope (boost::shared_ptr<rcFrameGrabber>& fg, const rcIRect rect,
			      uint32 phase, uint32 sample, rcKinetoscope::Detail d,
			      rcMovieFileOrgExt org, rcOrganismInfo::OrganismType c,
			      rcOrganismInfo::OrganismName n,  int32 minMobSize, bool debuging)
: mSource (eFileGrabber), mFg (fg), mOrgExt (org)
  {
    if (! mFg || !mFg->start ())
      rmExceptionMacro(<<"Kinetoscope: Can start frame grabber"); 
    COMMONCTOR;
  }

  /* Effect: Destructs a Kinetoscope. 
   */
rcKinetoscope::~rcKinetoscope ()
  {
    
  }

/*
 * Various accessor and mutators only used
 * by this calls or its friends
 * Max speed is currently an integer value. We add one to this since
 * we need an extra row/column of correlation value to produce
 * an interpolated value. 
 */
void rcKinetoscope::clear ()
{
  detail (eFine);
  mStepProcessingTime = -1.0;
  mElapsedTimeSecs = 0;
  mLastElapsedTimeSecs = 0;
  mPeriodicity = 0.0;
  mOrigin = rcFPair (0.0f, 0.0f);
  mCount = 0;
  target (defaultTargetRadius);
  maxRadialSpeed (defaultMaxRadSpeed);
  mSourceMotion = eAttentiveCapture;
  mSimPipe = 3;
//  mLsm = new rcLatticeSimilarator (mSimPipe, 2);
  mSizeMultiplier = 0.0f;
  rmAssert (mGlobalXform.isIdentity());
  rmAssert (mImageToPhysical.isIdentity());
  rmAssert (mPhysicalToImage.isIdentity());
}

rcTimestamp rcKinetoscope::resultTime() const
{ 
  rcTimestamp now = (isCardiacCell ()) ?  fixed ().frameBuf()->timestamp() :
    (count() > 1) ? moving().frameBuf()->timestamp() : 
    fixed ().frameBuf()->timestamp();

  return now;
  }

void rcKinetoscope::start ()
{

    rmAssert (mSource == eFileGrabber || mSource == eVideoCache);
    rmAssert (mFrameData.size () == 2);
    rmAssert (mChannelData.size () == 2);

    mFc = frameCount();

    clear ();
    rmAssert (count() == 0);

    if (mFc < int32 (2 * (mPhase + mSample)))
      rmExceptionMacro(<< "Kinetoscope: sequence too short");

    // Connect moment generators
    mMomentMap[eFixed] = &mMomGen0;    
    mMomentMap[eMoving] = &mMomGen1;

    // Fetch the first frame
    frame (startFrame(), eFixed);

    if (mProcessRect.width() <= 0 || mProcessRect.height() <= 0)
      rmExceptionMacro (<< "Kinetoscope: Region Too Small");

    mov ();
  //  mCutOff = defaultCutOff;

    /* 
     * Record The Start Time In MilliSeconds
     */
    temporalScale (1000.0f);

    // Create the connected component image
    // @todo: Update for MultiResolution
    // @note can use shallow copy in some cases. 
    mConnectionMap[eMoving] = rcWindow (iframe().size(), rcPixel8);
    connect().setAllPixels (uint32 (0));
    mConnectionMap[eFixed] = rcWindow (iframe().size(), rcPixel8);
    connected().setAllPixels (uint32 (0));

    // 
    // Create the potentialMap
    //   mXvelocityField = rcWindow (iframe().size(), rcPixel16);
    //     mYvelocityField = rcWindow (iframe().size(), rcPixel16);
    //     mXvelocityField.setAllPixels (uint32 (0));
    //     mYvelocityField.setAllPixels (uint32 (0));

    // Claim
    mark (eCanStep);


  if (organism() ==  rcOrganismInfo::eModel && organismName () == rcOrganismInfo::eDanioRerio)
    {
      stepDirectedFlowPreFill ();
    }

  // Debugging display
  if (isDebugOn ())
    {
  if (organism() !=  rcOrganismInfo::eModel || organismName () != rcOrganismInfo::eDanioRerio)
    {
      if ((organism() == rcOrganismInfo::eFluorescenceCluster &&
	   ( organismName() == rcOrganismInfo::eProteinP53 ||
	     organismName() == rcOrganismInfo::eProtein)))
	{
	  if (channelAvailable ())
	    debugScreen (mChannelData[eFixed].base (detail ()));
	  else
	    {
	      debugScreen (mFrameData[eFixed].base(detail ()));
	    }
	}
      else
	debugScreen (mFrameData[eFixed].base(detail ()));
    }
    }
  
  updateProcessingTime ();
}


void rcKinetoscope::debugScreen (const rcWindow& image)
{
  if (!isDebugOn ()) return;
  mDebugScreen = rcWindow (iframe().width(), iframe().height(), image.depth());
  mDebugScreen.copyPixelsFromWindow (image);
}

uint32 rcKinetoscope::sample () const { return mSample ; }
uint32 rcKinetoscope::phase () const { return mPhase; }


  /* Effect: Sets / Gets temporal sampling
   * Requires a started grabber. 
   */

const rcIRect& rcKinetoscope::iframe () const
{
  return mRect;
}

const rcIRect& rcKinetoscope::pframe () const
{
  return mProcessRect;
}

void rcKinetoscope::laminarSuppression (double over)
  {
    if (over < 1.0) mCutOff = 1. / (1.0 - over);
  }

double rcKinetoscope::laminarSuppression () const
  {
    return 1.0 - 1./mCutOff;
  }

bool rcKinetoscope::have (enum Option what) const
{
  return mOptions[what];
}

bool rcKinetoscope::mark (enum Option what) 
{
  mOptions.set(what);
  return have (what);
}

bool rcKinetoscope::clear (enum Option what) 
{
  mOptions.reset(what);
  return have (what);
}

void rcKinetoscope::reset ()
{
  mOptions.reset ();
}

const rcWindow& rcKinetoscope::use4moving () const 
{
  if (isLabelProtein ())
    {
      if (channelAvailable ())
	return channelMovingPast();	    
      else
	return movingPast();
    }
  else
    return moving();
}

const rcWindow& rcKinetoscope::use4fixed () const 
{
  if (isLabelProtein ())
    {
      if (channelAvailable ())
	return channelFixedPast();
      else
	return fixedPast();
    }
  else
    return fixed();
}

const rcWindow& rcKinetoscope::use4when (const Time when) const
{
  if (when == eFixed) return use4fixed();
  return use4moving ();
}

void rcKinetoscope::keepPopulationMeasure ()
{
  const list<rcVisualFunction>& vbs = visualBodies();

  list<rcVisualFunction>::const_iterator cell;
  vector<float> ameasure;

  for( cell = vbs.begin(); cell != vbs.end(); ++cell )
    {
      if (cell->have (rcVisualFunction::eBeatInfo))
	ameasure.push_back (cell->shortenning ());
    }

  if (ameasure.size())
    mPopulationStats.add (rfMedian (ameasure));

}

const rcStatistics& rcKinetoscope::populationMeasures () const
{
  return mPopulationStats;
}




void rcKinetoscope::visualFunctionInfo (rcOrganismInfo::OrganismType c, rcOrganismInfo::OrganismName n)
{
  mOtype = c;
  mOname = n;
}

void rcKinetoscope::updateProcessingTime ()
{
  rcTimestamp rct (getCurrentTimestamp ());
  if (mStepProcessingTime < 0.0)
    mStepProcessingTime = rct.secs ();
  else
    mStepProcessingTime = rct.secs () - mStepProcessingTime;

  mStepProcessingTime /= mCount;
}



bool rcKinetoscope::validate(const rcIRect& rect, rc2Fvector& df) const
{
  // Check to see If this is a valid Site
  // calculate hessian for this area
  int32 isInside;
  const rcWindow& mGi = mFrameData[eMoving].gradient (detail());
  rcWindow rw (mGi, rect, isInside);
  if (!isInside) return false;
  float meh = rfSurfaceHessian (rw);  
  df.x(meh); df.y(meh);
  return true;
}  


bool rcKinetoscope::isWithin () const
{
  if (mCount <= startFrame()) return true;
  return false;
}


bool rcKinetoscope::isWithin (const rcIRect& v) const
{
  // make a copy of v and translate it in to mRect
  rcIRect vcp = v;
  vcp.translate (mRect.ul());
  return (mRect.contains (vcp));
}

float rcKinetoscope::directedCellToCellMigration ()
{
  
  if (mFunctionMap.empty())
    {
      return 0;
    }

  list<rcVisualFunction>::const_iterator sit = mFunctionMap.begin();
  float sp (0.0f);
  for (; sit != mFunctionMap.end(); sit++)
    {
      sp += sit->directedCellPersistence();
    }
  sp /= mFunctionMap.size();
  return sp;
}  


float rcKinetoscope::avgMeanSqCellDisplacements ()
{
  list<rcVisualFunction>::iterator cTr = mFunctionMap.begin();
  float answer (0.0f);
  for (; cTr != mFunctionMap.end(); cTr++)
    {
      answer += cTr->minSqDisplacement ();
    }
  if (mFunctionMap.size())
    answer = answer / mFunctionMap.size();
  return answer;
}

/*
 * step() clear the statistics.
 * Between calls to step, the first time rawSpeedStats is called
 * it will accumulate the statistics.
 * All subsequent calls untill the next step call return the most
 * recent statistics
 */

void rcKinetoscope::rawSpeedStats (rcStatistics& stats)
{

  if (mSnapStats.n())
    {
      stats = mSnapStats;
      return;
    }

  rcMotionPathMap::iterator pos;

  for (pos = mPathMap.begin(); pos != mPathMap.end(); pos++)
    {
      mSnapStats.add (pos->second.motion().len ());
    }

  stats = mSnapStats;

}  

void rcKinetoscope::minMobSizeInPixels (int32 ms)
{
  if (ms < 5) 
    rmExceptionMacro(<< "Kinetoscope: invalid organism size");

  mMinParea = ms;
}

int32 rcKinetoscope::minMobSizeInPixels ()
{
  return mMinParea;
}


void rcKinetoscope::periodicMotion (float p)
{
  mPeriodicity = p;
}

float rcKinetoscope::periodicMotion () const
{
  return mPeriodicity;
}

void rcKinetoscope::averageFramesPerSecond (double p)
{
  mFramesPerSecond = p;
}

float rcKinetoscope::averageFramesPerSecond () const
{
  return   mFramesPerSecond;
}

void rcKinetoscope::target (int32 sam)
{
  mTargetRadius = rcIPair (sam, sam); mov();
}

rcIPair rcKinetoscope::target () const
{
  return mTargetRadius;
}

void rcKinetoscope::maxRadialSpeed (int32 sam)
{ 
  rmAssert (sam >= 2);
  mVmaxRadial = rcIPair (sam, sam); mov ();
}

int32 rcKinetoscope::maxRadialSpeed () const
{
  return mVmaxRadial.x();
}


void rcKinetoscope::mov ()
{
  mMovingSize = mTargetRadius + mVmaxRadial;
}    

const rcIPair& rcKinetoscope::movingSize () const
{
  return mMovingSize;
}

float rcKinetoscope::sizeMultiplier () const
{
  return mSizeMultiplier;
}

void rcKinetoscope::sizeMultiplier (float ms) 
{
  mSizeMultiplier = ms;
}


bool rcKinetoscope::isBound ()
{
  return (fixedPast ().isBound() && moving ().isBound() && fixed ().isBound() && movingPast ().isBound());
}

void rcKinetoscope::createFunctionSpecificObjects ()
{
  if (organism() ==  rcOrganismInfo::eModel && organismName () == rcOrganismInfo::eDanioRerio)
    {
      if (!mFullImage.isBound())
	{
	  mFullImage = rcWindow (iframe().width(), iframe().height());
	  mFullImage.setAllPixels (255);
	}

      if (!mClearImage.isBound())
	{
	  mClearImage = rcWindow (iframe().width(), iframe().height());
	  mClearImage.setAllPixels (0);
	}
    }


  if (organismName() == rcOrganismInfo::eCardioMyocyte || 
      organismName() == rcOrganismInfo::eCardioMyocyteCalcium)
    {
      if (!mMaskCellwFF.isBound())
	mMaskCellwFF = rcWindow (iframe().width(), iframe().height());
    }

}

// Debugging/development image
const rcWindow rcKinetoscope::debug () const
{
  return mDebugScreen;
}
  

/*	*************************
	*                       *
	*  Tracking Functions   *
	*                       *
	*************************
*/

/*
 * for_each should work but for now it does not
  for_each (mp.begin(),
	    mp.end(),
	    mem_fun_ref (rcMotionVectorPath::measure));

*/

void rcKinetoscope::measure (rcMotionPathMap& mp)
{
  rcMotionPathMap::iterator pos;

  for (pos = mp.begin(); pos != mp.end(); pos++)
    {
      pos->second.measure();
    }
}

void rcKinetoscope::update (rcMotionPathMap& mp)
{
  rcMotionPathMap::iterator pos;
  
  for (pos = mp.begin(); pos != mp.end(); pos++)
    {
      pos->second.update();
    }
}


void rcKinetoscope::reconcile (rcMotionPathMap& mp)
{
//  if ((organism() == rcOrganismInfo::eFluorescenceCluster &&
//       ( organismName() == rcOrganismInfo::eProteinP53 ||
// 	organismName() == rcOrganismInfo::eProtein)))
//    {
//      rcWindow tmp (connect().size(), connect().depth());
//      tmp.copyPixelsFromWindow (connect());
//      rfGaussianConv (tmp, connect(), 3);
//    }
//  else
   {
     rcMotionPathMap::iterator pos;
     for (pos = mp.begin(); pos != mp.end(); pos++)
       pos->second.reconcile ();
   }
}



void rcKinetoscope::measure (list<rcVisualFunction>& mp)
{
  list<rcVisualFunction>::iterator pos;
  rcTimingInfo profile (2, frameCount() + 1);
  profile.nextPass (0);


  for (pos = mp.begin(); pos != mp.end(); pos++)
    {
      profile.touch(0);
      try
	{
	  pos->measure();
	}
      catch (general_exception& x)
	{
	  pos->setState(rcVisualFunction::eIsOutSide);
	  fprintf( stderr, "In Measure Function Caught general_exception %s %s %d\n", 
		   x.what(), x.GetFile (), x.GetLine () );
	}
      profile.touch(1);
    }
  profile.printInfo (1);	

}

void rcKinetoscope::update (list<rcVisualFunction>& mp)
{
  list<rcVisualFunction>::iterator pos;
  rcTimingInfo profile (2, frameCount() + 1);
  profile.nextPass (0);


  for (pos = mp.begin(); pos != mp.end(); pos++)
    {
      profile.touch(0);
      try
	{
	  pos->update();
	}
      catch (general_exception& x)
	{
	  pos->setState(rcVisualFunction::eIsOutSide);
	  fprintf( stderr, "In Update Function Caught general_exception %s %s %d\n", 
		   x.what(), x.GetFile (), x.GetLine () );
	}
      profile.touch(1);
    }
  profile.printInfo (1);

}


void rcKinetoscope::reconcile (list<rcVisualFunction>& mp)
{
  list<rcVisualFunction>::iterator pos;
  uint32 i = 0, initialSize (mp.size());

  rcTimingInfo profile (2, frameCount() + 1);
  profile.nextPass (0);

  for (pos = mp.begin(); i < initialSize; pos++, i++)
    {
      profile.touch(0);

      try
	{
	  pos->reconcile();
	}
      catch (general_exception& x)
	{
	  pos->setState(rcVisualFunction::eIsOutSide);
	  fprintf( stderr, "In Reconcile Function Caught general_exception %s %s %d\n", 
		   x.what(), x.GetFile (), x.GetLine () );
	}
      profile.touch(1);
    }
  profile.printInfo (1);
}


void rcKinetoscope::printPolys ()
{
  //    dumpPolys (cerr);
}

void rcKinetoscope::printMap ()
{
  //    dumpMap (cerr);
}

void rcKinetoscope::printConnect()
{
  cerr << connect();
}

void rcKinetoscope::printConnected()
{
  cerr << connected();
}



