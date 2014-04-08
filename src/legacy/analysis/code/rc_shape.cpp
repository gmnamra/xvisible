
#include <rc_shape.h>

#include <algorithm>
#include <rc_def_generators.h>
#include <rc_cubic_generators.h>
#include <rc_stats.h>
#include <rc_1dcorr.h>
#include <rc_filter1d.h>
#include <rc_draw.h>
#include <rc_moments.h>
#include <rc_imageprocessing.h>
#include <iterator>
#include <rc_similarity.h>
#include <rc_macro.h>

//#define SHAPEDEBUG


#ifdef SHAPEDEBUG
#include <rc_gnuplot.h>
#endif

rcShape::rcShape () : mExtendSize (0) {}


 // Destructor
rcShape::~rcShape() {}

bool rcShape:: isValid (enum Valid what) const
{
  return mValidSet[what];
}

bool rcShape:: validate (enum Valid what)
{
  mValidSet[what] = true;
  return mValidSet[what];
}

bool rcShape:: inValidate (enum Valid what)
{
  mValidSet[what] = false;
  return mValidSet[what];
}

bool rcShape::isBound (const enum geometry g)
{
  if (g == eAffine) return isValid(eIsValid);
  if (g == eImage) return (isValid(eIsValid) && isValid (eValidFrame));
  return false;
}


rcShape::rcShape (const rcWindow& frame) 
{
  reset (eAffine);
  reset (eImage);


  if (frame.isBound())
    {
      mFrame = frame;
      mMask = rcWindow ();
      validate (eValidFrame);
      inValidate (eValidPoly);
      validate (eIsValid);
    }
  else
    {
      rmExceptionMacro (<<" rcShape Ctor " << frame.isBound());
    }

}

rcShape::rcShape (const rcPolygon& poly) 
{
  reset (eAffine);
  reset (eImage);

  if (poly.isValid())
    {
      mPoly = poly;
      inValidate (eValidFrame);
      validate (eValidPoly);
      validate (eIsValid);
    }
  else
    {
      rmExceptionMacro (<<" rcShape Ctor " << poly.isValid());
    }

}

rcShape::rcShape (const rcWindow& frame, const rcPolygon& poly) 
{

  reset (eAffine);
  reset (eImage);

  if (poly.isValid() && frame.isBound())
    {
      mPoly = poly;
      mFrame = frame;
      mMask = rcWindow ();
      validate (eValidFrame);
      validate (eValidPoly);
      validate (eIsValid);
    }
  else
    {
      rmExceptionMacro (<<" rcShape Ctor " << poly.isValid() << frame.isBound());
    }
}

void rcShape::reset (geometry g)
{
  inValidate (eIsValid);
  mExtendSize = 0;
  mBg = uint8 (0);
  mFg = uint8 (0xff);
  inValidate (eValidDimensionConstraints);

  if (g == eAffine)
    {
      inValidate (eValidArea);
      inValidate (eValidPerimeter);
      inValidate (eValidCircularity);
      inValidate (eValidEllipseRatio);
    }

  if (g == eImage)
    {
      inValidate (eValidFrame );
      inValidate (eValidMinor );
      inValidate (eValidMajor );
      inValidate (eValidMajorProfile );
      inValidate (eValidMinorProfile );
      inValidate (eValidCom );
      inValidate (eValidAffineCom );
      inValidate (eValidMoments );
      inValidate (eValidAffineRect );
      inValidate (eValidAffineWindow );
    }

  mMajorD.reserve (1024);
  mMinorD.reserve (1024);

}


int32 rcShape::extendProfile (int32 s)
{
  mExtendSize = s;return mExtendSize;
}

const int32 rcShape::extendProfile () const
{
  return mExtendSize;
}

bool rcShape::majorEndConstraints (rcDPair& oneEnd, rcDPair& otherEnd)
{
  // One Size: from 0 on
  if (oneEnd.x() < 0 || oneEnd.y() < 0 || 
      (oneEnd.x()+oneEnd.y()) > (mMajorD.size() - 1))
    return false;

  // Other Size: from End (1/2 step) inward.
  if (otherEnd.x() < 0 || otherEnd.y() < 0 || 
      (otherEnd.x() >  (mMajorD.size() - 1)) ||
      ((otherEnd.x() - otherEnd.y())) > (mMajorD.size() - 1))
    return false;
  
  mMajorOneEndConstraints = oneEnd;
  mMajorOtherEndConstraints = otherEnd;
  validate (eValidDimensionConstraints);
  return   isValid (eValidDimensionConstraints);
}

void rcShape::setMask () 
{
  rcWindow mask(mFrame.width(), mFrame.height());
  mask.setAllPixels(mBg);

  rcFillPolygon(mPoly, mask, mFg);

  if (mask.isBound())
    {
      mMask = mask;
    }
  else
    {
      rmExceptionMacro (<<" rcShape setMask " << mask.isBound());
    }
}


double rcShape::area () const
{
  return const_cast<rcShape *>(this)->needArea ();
}

double rcShape::needArea ()
{
  if (isValid (eIsValid) && !isValid (eValidArea))
    {
      mArea = mPoly.area ();
      validate (eValidArea);
      return mArea;
    }

  if (isValid (eIsValid) && isValid (eValidArea))
    return mArea;

  rmExceptionMacro (<< "rcShape::NotValid");
}


double rcShape::perimeter () const
{
  return const_cast<rcShape *>(this)->needPerimeter ();
}

double rcShape::needPerimeter () 
{
  if (isValid (eIsValid) && !isValid (eValidPerimeter))
    {
      mPerimeter = mPoly.perimeter ();
      validate (eValidPerimeter);
      return mPerimeter;
    }

  if (isValid (eIsValid) && isValid (eValidPerimeter))  
    return mPerimeter;

  rmExceptionMacro (<< "rcShape::NotValid");
}

double rcShape::circularity () const
{
  return const_cast<rcShape *>(this)->needCircularity ();
}

double rcShape::needCircularity ()
{
  if (isValid (eIsValid) && !isValid (eValidCircularity))
    {
      mCircularity = mPoly.circularity ();
      validate (eValidCircularity);
      return mCircularity;
    }

  if (isValid (eIsValid) && isValid (eValidCircularity))
    return mCircularity;

  rmExceptionMacro (<< "rcShape::NotValid");
}

double rcShape::ellipseRatio () const
 {
  return const_cast<rcShape *>(this)->needEllipseRatio ();
 }

double rcShape::needEllipseRatio () 
{
  if (isValid (eIsValid) && !isValid (eValidEllipseRatio))
    {
      mEllipseRatio = mPoly.ellipseRatio ();
      validate (eValidEllipseRatio);
      return mEllipseRatio;
    }

  if (isValid (eIsValid) && isValid (eValidEllipseRatio))
    return mEllipseRatio;

  rmExceptionMacro (<< "rcShape::NotValid");
}

const rcAffineRectangle& rcShape::affineRect () const
{
  return const_cast<rcShape *>(this)->needAffineRect ();
}


const rcAffineRectangle& rcShape::needAffineRect ()
 {
   if (isValid (eIsValid) && isValid (eValidPoly) &&  !isValid (eValidAffineRect))
    {
      rcAffineRectangle ar = mPoly.minimumAreaEnclosingRect ();
      bool cOr = (ar.cannonicalSize().x() >= ar.cannonicalSize().y()) ? true : false;
      rcIPair steps (0, 0);
      steps.x() = cOr ? mExtendSize : 0;
      steps.y() = cOr ? 0 : mExtendSize;
      mAffineRect = ar.expand (steps);
      
      mAffineSize.x(rmMax (mAffineRect.xyScale().x(), mAffineRect.xyScale().y()));
      mAffineSize.y(rmMin (mAffineRect.xyScale().x(), mAffineRect.xyScale().y()));
      validate (eValidAffineRect);
      return mAffineRect;
    }

   if (isValid (eIsValid) && isValid (eValidPoly) && isValid (eValidAffineRect))
    return mAffineRect;

  rmExceptionMacro (<< "rcShape::NotValid");

 }

rcIRect rcShape::orthogonalBoundingBox() const
{
  if ( isValid (eValidPoly))
    return affineRect().boundingBox ();
  else if ( isValid (eValidFrame))
    return mFrame.rectangle ();
  else return rcIRect ();
}

rc2Fvector rcShape::orthogonalBoundingBoxCenter() const
{
  rcIRect bb = orthogonalBoundingBox ();
  return rc2Fvector ((float) bb.ul().x() + bb.width () / 2.0f,
	      (float) bb.ul().y() + bb.height () / 2.0f);
}

float rcShape::orthogonalBoundingBoxAspect() const
{
  rcIRect bb = orthogonalBoundingBox ();
  rmAssert (bb.height());
  return ((float) bb.width()) / ((float) bb.height());
}


const rcAffineWindow& rcShape::affineWindow () const
 {
  return const_cast<rcShape *>(this)->needAffineWindow ();
 }

const rcWindow& rcShape::affineImage () const
 {
   return mAffinedImage;
 }

const rcAffineWindow& rcShape::needAffineWindow ()
{
   if (isValid (eIsValid) && isValid (eValidAffineRect) &&  isValid (eValidFrame))
     {
       mAffineWindow = rcAffineWindow (mFrame, affineRect ());
       validate (eValidAffineWindow);
       return mAffineWindow;
     }

   if (isValid (eValidAffineWindow))
     return mAffineWindow;

  rmExceptionMacro (<< "rcShape::NotValid");
}
  
void rcShape::profiles () 
{
  if (!isValid (eIsValid))
    rmExceptionMacro (<< "rcShape::NotValid");

  if (isValid (eValidMinorProfile) && isValid (eValidMajorProfile))
    return;

  // For now use mask as an indication.
  if (isValid (eIsValid) && isValid (eValidPoly) &&  isValid (eValidFrame) &&
       !mMask.isBound())
    {
      rmWarningMacro ("Profile Generated\n");

      const rcAffineRectangle& ar = affineRect ();
      const rcAffineWindow& af = affineWindow ();
      bool cOr = (ar.cannonicalSize().x() >= ar.cannonicalSize().y()) ? true : false;

      mAffinedImage = rcWindow (ar.cannonicalSize());

      af.genImage (mAffinedImage, mAffinedImage.rectangle());
      
      if (cOr)
	project (mAffinedImage, mMask, mMajorD, mMinorD);
      else
	project (mAffinedImage, mMask, mMinorD, mMajorD);
    }
  else if (isValid (eIsValid) && isValid (eValidFrame) )
    {
      project (mFrame, mMask, mMajorD, mMinorD);
    }
  else 
    {
      rmExceptionMacro (<< "rcShape::NotValid" << isValid (eIsValid) << " " << isValid (eValidPoly) << " " << isValid (eValidFrame));
    }  
    
  validate (eValidMinorProfile);
  validate (eValidMajorProfile);
}


void rcShape::project (const rcWindow& src, const rcWindow&mask, vector<float>& widthProj, vector<float>& heightProj)
{
  rcWindow vproj (src.width(), 1, rcPixel32);
  rcWindow hproj (src.height(), 1, rcPixel32);
  rcIRect rectangle (0, 0, src.width(), src.height());

  if (!mask.isBound())
    {
      rcMomentGenerator mg (rcMomentGenerator::eMoment2D);
      mg.update (src);
      mg.vProject (rectangle, vproj);
      mg.hProject (rectangle, hproj);
    }
  else
    {
      rcWindow newFrame (src.width(), src.height());
      rfAndImage (src, mask, newFrame);

      rcMomentGenerator mg (rcMomentGenerator::eMoment2D);
      mg.update (newFrame);
      mg.vProject (newFrame.rectangle(), vproj);
      mg.hProject (newFrame.rectangle(), hproj);
    }

  // Copy in to the float vectors (this is a bit silly!)
  widthProj.resize (vproj.width());
  uint32 *projPtr = (uint32 *) vproj.rowPointer (0);
  vector<float>::iterator mItr = widthProj.begin();
  for (int32 i = 0; i < vproj.width(); i++) *mItr++ = *projPtr++;

  heightProj.resize (hproj.width());
  projPtr = (uint32 *) hproj.rowPointer (0);
  mItr = heightProj.begin();
  for (int32 i = 0; i < hproj.width(); i++) *mItr++ = *projPtr++;
      
}


void rcShape::projectMI (const rcWindow& src, const rcWindow&mask, vector<float>& widthProj, vector<float>& heightProj)
{
  static double tiny = 1e-10;

  rcSimilarator sm(rcSimilarator::eExhaustive,
		     rcPixel8,
		     src.width(),
		     0, rcSimilarator::eNorm, false, 0, tiny); 

  rcSimilarator zm(rcSimilarator::eExhaustive,
		   rcPixel8,
		   src.height(),
		   0, rcSimilarator::eNorm, false, 0, tiny); 
  bool havez = zm.fill (src, false);
  bool haves = sm.fill (src);

  deque<double> sig;
  deque<double> zig;
  if (havez) havez = zm.entropies (zig);
  if (haves) haves = sm.entropies (sig);

  rmAssert (havez && haves);

  if (sig.size() == src.width() && zig.size() == src.height())
    {
      widthProj.resize (sig.size ());
      for (uint32 i = 0; i < sig.size(); i++)
	widthProj[i] = sig[i];

      heightProj.resize (zig.size ());
      for (uint32 i = 0; i < zig.size(); i++)
	    heightProj[i] = zig[i];
    }
  else
    {
      widthProj.resize (zig.size ());
      for (uint32 i = 0; i < zig.size(); i++)
	widthProj[i] = zig[i];

      heightProj.resize (sig.size ());
      for (uint32 i = 0; i < sig.size(); i++)
	    heightProj[i] = sig[i];
    }
}


double rcShape::mass () const
 {
   return const_cast<rcShape *>(this)->needMass ();
 }

rcRadian rcShape::angle () const
 {
   const rcAffineRectangle& ar = affineRect ();
   return  ar.angle();
 }



static void hornMoments(vector<float>& profile, float& zero, float& first, float& second)
{
   // we shift the interval [left tail, right tail] to [0, right - left]
   float m0 = 0.0, m1 = 0.0, m2 = 0.0;

   // Based on Horn
   // Traversing from the end
   vector<float>::const_iterator endp = profile.end();   
   endp--;
   for(vector<float>::const_iterator p = endp;
       p > profile.begin();
       p--){
      m0 += *p;
      m1 += m0;
      m2 += m1;
   }

   // since we assume left tail == 0, we do not have to add i * hist[i] to m1
   zero = m0 + profile[0];     // number of sample
   first = m1;                  // mean * number of sample
   second = m2 * 2 - m1;        // var = second_/zero_ - (first_/zero_)^2
}

double rcShape::needMass () 
{
  if (!isValid (eIsValid) &&  !isValid (eValidFrame))
    rmExceptionMacro (<< "rcShape::NotValid");

  if (isValid (eValidMoments))
    return mMass;

  profiles ();

  // get Center of Mass, need moments
  float massX (0.0f), massY (0.0f);
  mX = mXX = mY = mYY = 0.0f;
  hornMoments (mMajorD, massX, mX, mXX);
  hornMoments (mMinorD, massY, mY, mYY);

  // record mass
  mMass = (massX + massY) / 2.0;
  validate (eValidMoments);
  return mMass;
}

rc2Fvector rcShape::centerOfmass (geometry g) const
{
  return const_cast<rcShape *>(this)->needCenterOfmass (g);
}

rc2Fvector rcShape::needCenterOfmass (geometry g)
{
  if (!isValid (eIsValid))
    rmExceptionMacro (<< "rcShape::NotValid");

  if (g == eAffine && isValid (eValidAffineCom))
    {
      return rc2Fvector ((float) mAffineCom.x(), (float) mAffineCom.y());
    }

  if (!isValid (eValidAffineCom) && g == eAffine)
    {
      rc2Dvector pctr;
      mPoly.centerOf (pctr);
      mAffineCom.x((float) pctr.x() );
      mAffineCom.y((float) pctr.y() );
      validate (eValidAffineCom);
      return rc2Fvector ((float) mAffineCom.x(), (float) mAffineCom.y());
    }


  if (isValid (eValidCom) && isValid (eValidFrame) && g == eImage)
    return rc2Fvector ((float) mCom.x(), 
		       (float) mCom.y());

      
  // Need moments
  double mas = mass ();

  // Calculate in pixels and in the affineWindow
  mCom.x((float) mX / mas);
  mCom.y((float) mY / mas);

  if (isValid (eIsValid) && isValid (eValidPoly) &&  isValid (eValidFrame)  && 
      !mMask.isBound())
    {
      // Normalize to affine size in pixels
      mCom.x(mCom.x() / mAffineSize.x());
      mCom.y(mCom.y() / mAffineSize.y());

      rmAssert (mCom.x() >= 0.0 && mCom.x() <= 1.0);
      rmAssert (mCom.y() >= 0.0 && mCom.y() <= 1.0);

      // Now map it to image coordinates
      mCom = affineWindow().ar().affineToImage (mCom);
      validate (eValidCom);
      return rc2Fvector ((float) mCom.x(), (float) mCom.y());
    }
  else if (isValid (eIsValid) && isValid (eValidFrame))
    {
      validate (eValidCom);
      return rc2Fvector ((float) mCom.x(), (float) mCom.y());
    }
  else 
    {
      rmExceptionMacro (<< "rcShape::NotValid" << isValid (eIsValid) << " " << isValid (eValidPoly) << " " << isValid (eValidFrame));
    }  
}


rc2Fvector rcShape::median (geometry g) const
{
  return const_cast<rcShape *>(this)->needMedian (g);
}

 rc2Fvector rcShape::needMedian (geometry g)
{
  if (!isValid (eIsValid))
    rmExceptionMacro(<< "rcShape::NotValid");

  double mas = mass ();
  rmUnused (mas);

  profiles ();

  // The assumption is that the projection is done in a most straight and unbiased way
  // hence no correction for aspect, bin size etc. 

  rc2Fvector majMed;
  double soFar(0.0);
  const double majority (0.5 * mass ());

  for(uint32 i = 0; i < mMajorD.size(); i++, soFar += mMajorD[i])
  {
    if (!i) rmAssertDebug (soFar == 0.0);

    if (soFar >= majority)
    {
      majMed.x((i - (majority - soFar) / mMajorD[i]));
      break;
    }
  }

  soFar = 0.0;
  for(uint32 i = 0; i < mMinorD.size(); i++, soFar += mMinorD[i])
  {
    if (!i) rmAssertDebug (soFar == 0.0);

    if (soFar >= majority)
    {
      majMed.y((i - (majority - soFar) / mMinorD[i]));
      break;
    }
  }

  mMedian = rc2Dvector (majMed.x() , majMed.y());

  if (g == eImage)
    return rc2Fvector ((float) mMedian.x(), (float) mMedian.y());

  rc2Dvector mm = rc2Dvector (majMed.x() / mMajorD.size(), majMed.y() / mMinorD.size());

  // Now map it to image coordinates
  mAffineMedian = affineWindow().ar().affineToImage (mm);

  return rc2Fvector ((float) mAffineMedian.x(), (float) mAffineMedian.y());

}

float rcShape::majorDimension () const
{
  return const_cast<rcShape *>(this)->needMajorDimension ();
 }


float rcShape::needMajorDimension () 
{
  if (!isValid (eIsValid))
    {
      rmExceptionMacro (<< "rcShape::NotValid");
    }


  if (isValid (eValidMajor))
    {
      return mMajord;
    }

  // Need Projections
  profiles ();

  // detect the ends
  peakDetect1d (mMajorD, mMajorEnds);
  mMajord = mMajorEnds.y() - mMajorEnds.x();

  validate (eValidMajor);
  return mMajord;
}


float rcShape::minorDimension () const
{
  return const_cast<rcShape *>(this)->needMinorDimension ();
}

float rcShape::needMinorDimension () 
{
  if (!isValid (eIsValid))
    rmExceptionMacro (<< "rcShape::NotValid");

  if (isValid (eValidMinor))
    return mMinord;

  // Need Projections
  profiles ();

  // detect the ends
  peakDetect1d (mMinorD, mMinorEnds);
  mMinord = mMinorEnds.y() - mMinorEnds.x();
  validate (eValidMinor);
  return mMinord;
}


const vector<float>& rcShape::majorProfile () const
{
  const_cast<rcShape *>(this)->needMajorDimension ();
  return mMajorD;
}


const vector<float>& rcShape::minorProfile () const
{
  const_cast<rcShape *>(this)->needMinorDimension ();
  return mMinorD;
}

rcDPair rcShape::majorEnds () const
{
  const_cast<rcShape *>(this)->needMajorDimension ();
  return mMajorEnds; 
}

rcDPair rcShape::minorEnds () const
{
  const_cast<rcShape *>(this)->needMinorDimension ();
  return mMinorEnds; 
}



bool rcShape::peakDetect1d (const vector<float>& signal, rcDPair& extendPeaks)
{
  // assume nothing.
  extendPeaks.x() = 0.0; extendPeaks.y() = 0;

  // Take the highest positive and the highest negative.
  rcEdgeFilter1DPeak posMax, negMax;
  posMax._value = 0, posMax._location = -1, negMax._value = 0, negMax._location = -1;
  vector<rcEdgeFilter1DPeak>peaks, posPeaks, negPeaks;
  rcEdgeFilter1D pfinder;
  vector<int32> efilter, filtered;
  pfinder.genFilter (efilter, 2, 1);
  pfinder.filter (efilter);
  pfinder.genPeaks (signal, &filtered, peaks, 0, 1.0, 0.5);
  
  for (uint32 i = 0; i < peaks.size(); i++)
    {
      int32 left = (i == 0) ? 0 : peaks[i-1]._value;
      int32 right = (i == (peaks.size() -1) ) ? 0 : peaks[i+1]._value;
      int32 val = peaks[i]._value;

      if (val <= 0) 
	{
	  if (val <= left && val <= right)
	    {
	      negPeaks.push_back (peaks[i]);

	      if (val < negMax._value)
		{
		  negMax._value = val;
		  negMax._location = peaks[i]._location;
		}
	    }
	}
      else
	{
	  rmAssert (val > 0);

	  if (val >= left && val >= right)
	    {
	      posPeaks.push_back (peaks[i]);

	      if (val > posMax._value)
		{
		  posMax._value = val;
		  posMax._location = peaks[i]._location;
		}
	    }
	}
    }

  if (posMax._location < 0 || negMax._location < 0) return false;
  // In case we needed space for all the score vector <float> scores (posPeaks.size() * negPeaks.size(), zf);
  // Score every pos/neg pair based on spatial distance and peak strength. 
  float dMax (signal.size());
  float pnMax = posMax._value - negMax._value;
  float sdMax (0.0f);
  rcIPair sMax;

  for (uint32 i = 0; i < negPeaks.size(); i++)
    {
      for (uint32 j = 0; j < posPeaks.size(); j++)
	{
	  float d = rmABS (posPeaks[j]._location - negPeaks[i]._location) / dMax;
	  float s = (posPeaks[j]._value - negPeaks[i]._value) / pnMax;
	  s *= d;
	  if (s > sdMax)
	    {
	      sMax = rcIPair ((int32) i, (int32) j);
	      sdMax = s;
	    }
	}
    }

  // First things first
  // Given the filtering, peaks will never be the ends but right now be paranoid
  
  rcDPair extends;
  extends.x() = rfInterpolatePeak (filtered, negPeaks[sMax.x()]._location);
  extends.y() = rfInterpolatePeak (filtered, posPeaks[sMax.y()]._location);
  extendPeaks.x() = rmMin (extends.x(), extends.y());
  extendPeaks.y() = rmMax (extends.x(), extends.y());

// Debugging Print

#ifdef SHAPEDEBUG

  cerr << dec << "+++++++++++++++++++++++++" << endl;
  cerr << signal << endl;
  cerr << filtered << endl;
  cerr << efilter << endl;
  for (uint32 i = 0; i < peaks.size(); i++) cerr << "[" << i << "]" << peaks[i]._location << ":\t" << peaks[i]._value << endl;
  cerr << negPeaks[sMax.x()]._location << " ---- " << posPeaks[sMax.y()]._location  << endl;
  cerr << extendPeaks << endl;

  cerr << "+++++++++++++++++++++++++" << endl;

#endif

  return true;
}


bool rcShape::testPeak ()
{
  vector<float> tst (11);
  tst[0] = 4450.0f;
  tst[1] = 4451.0f;
  tst[2] = 4452.0f;
  tst[3] = 4453.0f;
  tst[4] = 4100.0f;
  tst[5] = 4212.0f;
  tst[6] = 4230.0f;
  tst[7] = 4449.0f;
  tst[8] = 4450.0f;
  tst[9] = 4449.0f;
  tst[10] = 4450.0f;

  rcDPair dp;
  peakDetect1d (tst, dp);
}

void rcShape::printCache ()
{

  cerr << "mIsValid : "	         << boolalpha <<  isValid(eIsValid) << endl;
  cerr << "mValidArea : "	         << boolalpha <<  isValid(eValidArea  ) << endl;
  cerr << "mValidPerimeter : "      << boolalpha <<  isValid(eValidPerimeter   ) << endl;
  cerr << "mValidCircularity : "    << boolalpha <<  isValid(eValidCircularity ) << endl;
  cerr << "mValidEllipseRatio : "   << boolalpha <<  isValid(eValidEllipseRatio) << endl;
  cerr << "mValidFrame : "	         << boolalpha <<  isValid(eValidFrame ) << endl;
  cerr << "mValidMinor : "	         << boolalpha <<  isValid(eValidMinor ) << endl;
  cerr << "mValidMajor : "	         << boolalpha <<  isValid(eValidMajor  ) << endl;
  cerr << "mValidMajorProfile : "   << boolalpha <<  isValid(eValidMajorProfile) << endl;
  cerr << "mValidMinorProfile : "   << boolalpha <<  isValid(eValidMinorProfile) << endl;
  cerr << "mValidCom : "	         << boolalpha <<  isValid(eValidCom	     ) << endl;
  cerr << "mValidAffineCom : "      << boolalpha <<  isValid(eValidAffineCom   ) << endl;
  cerr << "mValidMoments : "        << boolalpha <<  isValid(eValidMoments     ) << endl;
  cerr << "mValidAffineRect : "     << boolalpha <<  isValid(eValidAffineRect  ) << endl;
  cerr << "mValidAffineWindow : "   << boolalpha <<  isValid(eValidAffineWindow ) << endl;


  cerr << "{" << endl;
  uint32 i(0);
  for (i = 0; i < mMajorD.size()-1; i++) cerr << "{" << i << "," << mMajorD[i] << "},";
  cerr << "{" << i << "," << mMajorD[i] << "}};";  
  cerr << endl;

  for (uint32 i = 0; i < mMinorD.size(); i++) cerr << "[" << mMinorD[i] << "]";
  cerr << endl;

  cerr << affineWindow () << endl;

}

void rcShape::copyFrom (const rcShape& other)
{

  mValidSet = other.mValidSet;

  mArea = other.mArea;
  mMass = other.mMass;
  mPerimeter = other.mPerimeter;
  mInertia = other.mInertia;
  mAngle = other.mAngle;
  mElongation = other.mElongation;
  mCircularity = other.mCircularity;
  mEllipseRatio = other.mEllipseRatio;
  mMedian = other.mMedian;
  mCom = other.mCom;
  mAffineCom = other.mAffineCom;
  mAffineMedian = other.mAffineMedian;
  mAffineSize = other.mAffineSize;
  mMajorEnds = other.mMajorEnds;
  mMinorEnds = other.mMinorEnds;
  mMajord = other.mMajord;
  mMinord = other.mMinord; 
  mX = other.mX;
  mY = other.mY; 
  mXX = other.mXX; 
  mYY = other.mYY;

  mFrame = other.mFrame;
  mPoly = other.mPoly;
//  mRegion = other.mRegion;
  mAffineRect = other.mAffineRect;
  mAffineWindow = other.mAffineWindow;
  mMajorD = other.mMajorD;
  mMinorD = other.mMinorD;
  mExtendSize = other.mExtendSize;
}

bool rcShape::operator==(const rcShape& other) const
{
  bool pieces = false;

  pieces = mValidSet == other.mValidSet &&
  mMinord == other.mMinord && 
  mX == other.mX &&
  mY == other.mY && 
  mXX == other.mXX && 
  mYY == other.mYY &&
    mFrame.position() == other.mFrame.position() &&
    mFrame.size() == other.mFrame.size() &&
    mFrame.frameBuf() == other.mFrame.frameBuf() &&
    mPoly == other.mPoly &&
    mMajorD.size() == other.mMajorD.size() &&
    mMinorD.size() == other.mMinorD.size() &&
    mExtendSize == other.mExtendSize;

  return pieces;
}
  
