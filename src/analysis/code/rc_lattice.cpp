/*
 *  rc_lattice.cpp
 *  analysis
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */

#include <rc_diamondscan.h>
#include <rc_numeric.h>



#define rmPrintImage(a){					    \
for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {  \
fprintf (stderr, "\n");					    \
uint16* vp = (uint16*)(a).row_pointer(i__temp);		    \
for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)   \
{\
if (vp[j__temp] == rcPointScan::notProcessedLabel ()) fprintf (stderr, "  +  "); \
else \
fprintf (stderr, "%1.3f+", vp[j__temp] == rcPointScan::notProcessedLabel () ? -1.0 : vp[j__temp] / 65000.0f); \
}\
fprintf (stderr, "\n");					    \
}}

	//#define FINDEBUG

/* +---------------+---------------+ */
/* |               |               | */
/* |               |               | */
/* |               |               | */
/* |      *========|======*========| */
/* |      !-1,-1   |      !1,-1    | */
/* |      !        |      !        | */
/* |      !        |      !        | */
/* +---------------+---------------+ */
/* |      !  (0,0) |      !        | */
/* |      !        |      !        | */
/* |      !        |      !        | */
/* |      *========|======*========| */
/* |      !-1,1    |      !1,1     | */
/* |      !        |      !        | */
/* |      !        |      !        | */
/* +---------------+---------------+ */

static rcIPair sDiamond[9] = {rcIPair (0,0), rcIPair(2,0), rcIPair (1,1), rcIPair (0,2),rcIPair (-1,1),
	rcIPair (-2,0), rcIPair (-1,-1), rcIPair (0,-2), rcIPair (1,-1)};
static rcIPair sCross[4] = {rcIPair (-1,0), rcIPair (0,-1), rcIPair (1,0), rcIPair (0,1)};

static const rcIPair sZeroIPair (rcNumericTraits<int>::Zero, rcNumericTraits<int>::Zero);
static const rcIPair sOneIPair (rcNumericTraits<int>::One, rcNumericTraits<int>::One);
static const rcIPair sTwoIPair = sOneIPair + sOneIPair;

/*! \class rcDiamondSteps
 *  \brief scanning steps
 *
 */

class rcDiamondSteps
{
public:
		// ctor
	rcDiamondSteps()
	{
		mDiamond.resize (9);
		for (uint32 i = 0; i < 9; i++)
			mDiamond[i] = sDiamond[i];
	};

	const vector<rcIPair>& steps () const { return mDiamond; }

private:
    vector<rcIPair> mDiamond;
};

/*! \class rcCrossSteps
 *  \brief scanning steps
 *
 */

class rcCrossSteps
{
public:
		// ctor
	rcCrossSteps()
	{
		mCross.resize (4);
		for (uint32 i = 0; i < 4; i++)
			mCross[i] = sCross[i];
	};

	const vector<rcIPair>& steps () const { return mCross; }
private:
    vector<rcIPair> mCross;
};

rcDiamondSteps sDiamondSteps;
rcCrossSteps sCrossSteps;

static void _exhaustiveImageFind (const rcWindow& moving, const rcWindow& fixed, const rcWindow& mask,
								  rcFindList& locations, rcWindow& space);

	//@Function
	//@todo move it to a better place

void rfImageFind (const rcWindow& moving, const rcWindow& fixed,
				  rcFindList& locations, rcWindow& space)
{
	rcWindow noMask;

	_exhaustiveImageFind (moving, fixed, noMask, locations, space);

}


void rfImageFind (const rcWindow& moving, const rcWindow& fixed, const rcWindow& mask,
				  rcFindList& locations, rcWindow& space, bool exhaustive)
{

	if (exhaustive)
		return _exhaustiveImageFind (moving, fixed, mask, locations, space);

	{
	  rmAssert (0);
	}
}


static void _exhaustiveImageFind (const rcWindow& moving, const rcWindow& fixed, const rcWindow& mask,
								  rcFindList& locations, rcWindow& space)
{
	int32 maxX (-1), maxY(-1);
	rc2Fvector interp;
	vector<float> score (9);
	rcPair<bool> edge;
	double maxScore (0.0);
	rsCorrParams cp;
	rcIPair step (1,0);
	rcTimingInfo profile (11, 2);

		// Size and create search space
	const rcIPair searchSpace (moving.width() - fixed.width() + 1,
							   moving.height() - fixed.height() + 1);
		// rcCorrelationWindow for the model
	rcWindow movingWin (moving, 0, 0, fixed.width(), fixed.height());

	profile.nextPass (true);
	profile.touch (0);

		// Correlation space. Pad by 2 since we do a 5x5 peak detection
		//@ note that crlf takes into account that space is padded
	static int32 cpad (2);
	rcWindow basespace (searchSpace.x()+cpad+cpad, searchSpace.y()+cpad+cpad, rcPixel16);
	basespace.set (0);
	rcWindow cspace (basespace, cpad, cpad, searchSpace.x(), searchSpace.y(), rcPixel16);
	profile.touch (1);

	const rcIPair colUpdate (1, 0);
	rcIPair row_update (-searchSpace.x(), 1);

	if (mask.isBound() && mask.size () == fixed.size ())
	{
          rmAssert (0);
	}
	else
	{
		uint32 curRow = 0;
		do {
			uint32 curCol = 0;
			do {


				rcCorr res;
				profile.touch (2);
				rmAssertDebug (moving.pelPointer (curCol, curRow) == movingWin.pelPointer (0,0));
				rfCorrelate (fixed, movingWin,res);
				profile.touch (3);
				const float r = res.r();
				cspace.setPixel (curCol, curRow, (uint16) (mQuantz * r));
				if (r > maxScore)
				{
					maxX = curCol, maxY = curRow, maxScore = r;
				}
			} while (movingWin.translate(colUpdate) && ++curCol && curCol < searchSpace.x());
			row_update[0] = -curCol;
			curRow++;
		} while(movingWin.translate(row_update) && curRow < searchSpace.y());
		rmAssert(curRow == (searchSpace.y()));
		space = cspace;
	}

#ifdef FINDEBUG
	rmPrintImage (space);
#endif
		// Locate peaks in the correlation space
	rcFindList peaks;

		// If result is sized for one, then jus return the best (locally bettered)
		// we add the offset to basespace to match with what we get from the
		// peakDetect
	profile.touch (4);
	if (locations.size() == 1)
	{
		peaks.resize (1);
		rcFindList::iterator tmp = peaks.begin();
		tmp->location().x(maxX + cpad);
		tmp->location().y( maxY + cpad);
		tmp->score ( (uint16) (mQuantz * maxScore) );
	}
	else
	{
		uint16 accept (100);
		rfDetectPeaks (basespace, accept, peaks);
		locations.resize (peaks.size());
	}
	profile.touch (5);

		//@note: peaks are wrt to space passed in to rfDetectPeaks. In this case basespace
	rcFindList::iterator lTr = peaks.begin();

	for (uint32 i = 0; i < peaks.size(); i++, lTr++)
	{
			// Correct to be wrt cspace
			// 0 --> basespace.width() -1 ==> 2 --> basesace.width() - 3  ==> 0 --> cspace.width() - 1
	  int32 maxX = lTr->interpolated().x() - cpad;
	  int32 maxY = lTr->interpolated().y() - cpad;
	  lTr->interpolated().x(maxX);
	  lTr->interpolated().y(maxY);

		if (maxX < 0 || maxY <0 || maxX > (cspace.width() - 1)  || maxY > (cspace.height() - 1))
		{
		  lTr->score (0);
			continue;
		}

			// With zero padding the neighbours could get interpolated, but we might be able
			// to peak outside!
		edge.y() = !(maxY > 0 && maxY < (cspace.height() - 1));
		edge.x() = !(maxX > 0 && maxX < (cspace.width() - 1));
		//		lTr->edge = edge;

		if (!edge.x() && !edge.y())
		{
			score[0] = cspace.getPixel (maxX-1,maxY-1)/ (float) mQuantz;
			score[1] = cspace.getPixel(maxX, maxY-1)/ (float) mQuantz;
			score[2] = cspace.getPixel(maxX+1, maxY-1)/ (float) mQuantz;
			score[3] = cspace.getPixel(maxX-1, maxY)/ (float) mQuantz;
			score[4] = cspace.getPixel(maxX, maxY)/ (float) mQuantz;
			score[5] = cspace.getPixel(maxX+1, maxY)/ (float) mQuantz;
			score[6] = cspace.getPixel(maxX-1, maxY+1)/ (float) mQuantz;
			score[7] = cspace.getPixel(maxX, maxY+1)/ (float) mQuantz;
			score[8] = cspace.getPixel(maxX+1, maxY+1)/ (float) mQuantz;

#ifdef FINDEBUG
			rfPRINT_STL_ELEMENTS (score);
#endif

			float xInterpolatedPeakValue (0.0f), yInterpolatedPeakValue (0.0f);
			interp = rc2Fvector (parabolicFit (score [3], score [4], score [5], &xInterpolatedPeakValue),
								 parabolicFit (score[1], score [4], score [7], &yInterpolatedPeakValue));
			lTr->score (std::max (xInterpolatedPeakValue, yInterpolatedPeakValue));
			lTr->interpolated() = interp + rc2Fvector (maxX, maxY) ;
		}
		else
		{
				// If the peak is on the edge of the match space, just return it
		  lTr->score ( maxScore);
		  lTr->interpolated() = rc2Fvector (maxX, maxY);
		}
	}
	profile.touch (9);

		// Resort. Some peaks might have improved
		// Sort and put the highest first
	peaks.sort();
	peaks.reverse();
	rcFindList::iterator rTr = locations.begin();
	lTr = peaks.begin();
	for (uint32 i = 0; i < peaks.size(); i++, lTr++, rTr++)
	{
		*rTr = *lTr;
	}
}


#if 0

rcPointScan::rcPointScan (const rcWindow& moving, const rcWindow& fixed,
						  const rcIPair& expectedPos, int32 maxTravel) :
mMovingPeak (expectedPos), mFramePeak (rcIPair (0,0)), mFrame (moving.frame()),
mMoving (moving), mFixed (fixed), mMaskValid (false), mMaskN (-1)
{
	if (!moving.isBound () || !fixed.isBound ())
		rmExceptionMacro (<< "Invalid Scan Arguments");

		// Calculate size of trace buffer we need.
		// MaxTravel of -1 or larger than possible trim, indicates using max possible trim
	int32 maxTrim = maxSpan ();
	if (maxTravel != -1 && maxTravel > maxTrim)
		maxTrim = maxTravel;

		// Peak is on the edge. run a local search here.
		// Start position in the root
	mFrameStart = moving.position();
	mFrameStart += (expectedPos.x() == -1 || expectedPos.y() == -1) ? (moving.size() - fixed.size()) / sTwoIPair : expectedPos;
	mFramePeak = mFrameStart;

		// Now create the tracebuffer. Tracebuffer does not extend
		// outside of the framebuf.
	mTrace = rcWindow (2 * maxTrim + 1, 2 * maxTrim + 1, rcPixel16);
	mTrace.set (uint16 (mNotProcessed));
	mMad = rcIPair (maxTrim, maxTrim);
	rmAssert (mMaxPeak.value == 0);
	mTb2Frame = mFrameStart - mMad;
	mCorrCount = mDiamCount = mCrossCount = mPeakCount = 0;
}

bool rcPointScan::peakAndInterpolate ()
{
	rcIPair off2tbTop = mFramePeak - mTb2Frame;

	float mCenter =  mTrace.getPixel (off2tbTop.x(), off2tbTop.y());
	float mRight =  mTrace.getPixel (off2tbTop.x()+1, off2tbTop.y());
	float mLeft =  mTrace.getPixel (off2tbTop.x()-1, off2tbTop.y());
	float mTop =  mTrace.getPixel (off2tbTop.x(), off2tbTop.y() -1);
	float mBottom =  mTrace.getPixel (off2tbTop.x(), off2tbTop.y() + 1);

	mMaxPeak.location = mFramePeak - mMoving.position ();
	bool ix (false), iy (false);
	if (ix = (mCenter >= mLeft && mCenter >= mRight))
    {
		mMaxPeak.interp.x(parabolicFit (mLeft, mCenter, mRight));
    }

	if (iy = (mCenter >= mTop && mCenter >= mBottom))
    {
		mMaxPeak.interp.y(parabolicFit (mTop, mCenter, mBottom));
    }

	mMaxPeak.interp += rc2Fvector (mMaxPeak.location.x(),
								   mMaxPeak.location.y());

	return (ix && iy);
}

int32 rcPointScan::maxSpan () const
{
	rcIPair span = mMoving.size() - mFixed.size();
	return std::min (span.x(), span.y());
}

bool rcPointScan::setMask(const rcWindow& mask)
{
	mMaskValid = false;
	if (mask.size() != mMoving.size()) return false;

	uint32 allOnes = 0xFF;
	int32 maskN (0);

	for (int32 y = 0; y < mask.height(); y++)
		for (int32 x = 0; x < mask.width(); x++) {
			uint32 val = mask.getPixel(x, y);
			if (val == allOnes)
				maskN++;
			else
				if (val != 0) return false;
		}

	mMask = mask;
	mMaskN = maskN;
	mMaskValid = true;
	return mMaskValid;
}

/*! \fn bool rcPointScan::scan (vector<rcIPair>& steps)
 *  \brief A member function.
 *  \param steps is a reference to a vector of rcIPair (moves)
 *  \return a bool indicating if a peak has been located
 */

bool rcPointScan::scan (const vector<rcIPair>& steps)
{
	rcIPair peak2TbTop = mFramePeak - mTb2Frame;
	rcIPair startingPos = mFramePeak;
	float scanMaxScore = 0.0f;
	int32 scanMaxIndex = -1;

		// Increment Angle each turn, increase diameter every other.
	if (steps == sCrossSteps.steps()) cerr << " Cross Steps";
	else cerr << " Diamond Steps";
	cerr << " Current Peak @ " << mFramePeak << ": " << mMaxPeak.score << endl;

	for (uint32 d = 0; d < steps.size(); d++)
    {
			// Update Spiral counter
		rcIPair off2tbtop =  peak2TbTop + steps[d];

		cerr << "Currently @ " << off2tbtop + mTb2Frame << "Step: " << steps[d] << endl;

			// Location with respect to top left of the trace buffer
		uint16 *fp = (uint16 *) mTrace.pelPointer (off2tbtop.x(), off2tbtop.y());

			// If it is already processed move on
		float score (0.0f);
		if (*fp == mNotProcessed)
		{
			rcIPair toFrameTop = mTb2Frame + off2tbtop;
			cerr << "Assesing " << toFrameTop << endl;

				// If any neighbor is outside the frame this is not a peak
			if (!checkOffset (toFrameTop))
				return false;

			rcWindow movingWin (mMoving.root(),
								 toFrameTop.x(), toFrameTop.y(),
								 mFixed.width(), mFixed.height());
			rcCorr res;
			if (mMaskValid)
			{
				rcWindow maskWin (mMask.root(),
								   toFrameTop.x(), toFrameTop.y(),
								   mFixed.width(), mFixed.height());
				rfCorrelate (mFixed, movingWin, maskWin, res, mMaskN);
			}
			else
				rfCorrelate (mFixed, movingWin, res);
			score = res.r();
			*fp = (uint16) (score * mQuantz);
			mCorrCount++;
		}
		else
			score = *fp / (float) mQuantz;

		if (score > scanMaxScore)
		{
			scanMaxScore = score;
			scanMaxIndex = d;
			mPeakCount++;
		}
    }

		// If we have had peaks and the starting center of the diamond is a peak
		// 1. cases where the center is a peak and that all diamond positions have already been evaluated.
		// 2. cases where the all diamond positions are done but the center is not.

		// Update peak score if any
	if (scanMaxIndex >= 0)
    {
		mFramePeak = peak2TbTop + steps[scanMaxIndex] + mTb2Frame;
		mMaxPeak.score = scanMaxScore;
    }

		// Return if center is a peak
	return (scanMaxIndex == 0);
}

	// start at best peak
	// check if a 5x5 region around the peak fits (actually the space is setup with borders that way anyway
	// laydown the diamond, assess at 9 edges of diamond (at any point tb is checked for previous op)
	// if this peak is a peak of the diamond positions, then do cross and return done
	// else lay diamond at new best peak.
	// to make sure that we are not running an infinite loop.

bool rcPointScan::climb ()
{
	while (1)
    {
		bool center = scan (sDiamondSteps.steps());
		if (center)
		{
			if (scan (sCrossSteps.steps()))
				break;
		}
		mDiamCount++;
		cerr << *this << endl;
    }

	return (peakAndInterpolate ());

}


void rfPrintFindList (const rcFindList& l, ostream& os)
{
	if (!l.size())
    {
		os << " Empty List " << endl;
		return;
    }

	rcFindList::const_iterator p;
	int32 i (0);
	for (p = l.begin(); p != l.end(); p++, i++)
		os << "[" << i << "]: " << *p << endl;
}

void rfImageACF (const rcWindow& moving, rcIPair& range, rcWindow& space)
{
	rcIPair play ( 2*range.x() + 1, 2*range.y() + 1);
	rcWindow base (moving, range.x(), range.y(), moving.width() - play.x(),
					moving.height() - play.y());
	cerr << endl;
	for (int32 j = 0; j < play.y(); j++)
    {
		for (int32 i = 0; i < play.x(); i++)
		{
			rcCorr res;
			rcIPair ij (i, j);
			rcWindow tmp (moving, i, j, base.width(), base.height());
			rfCorrelate(base, tmp, res);
			cerr << setw(4) << (int32) (res.r() * 1000);
			if (i != range.x()) cerr << ".";
		}
		cerr << endl;
    }
}

#endif
