/* @file
 *
 *$Id $
 *$Log$
 *Revision 1.84  2006/01/01 21:30:58  arman
 *kinetoscope ut
 *
 *Revision 1.83  2005/12/27 15:45:48  arman
 *fixed crlf. x translate post increments. So crlf is -w,1.
 *
 *Revision 1.82  2005/12/21 21:32:02  arman
 *added debugging code (conditionally compiled)
 *
 *Revision 1.81  2005/12/01 23:56:17  arman
 *removing debugging output
 *
 *Revision 1.80  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.78  2005/11/18 22:01:25  arman
 *incremental
 *
 *Revision 1.77  2005/11/17 23:19:06  arman
 *kinetoscope resync
 *
 *Revision 1.76  2005/11/15 23:26:05  arman
 *motionpath redesign
 *
 *Revision 1.75  2005/11/13 19:25:07  arman
 *in progress. handling of splat
 *
 *Revision 1.74  2005/11/08 20:34:29  arman
 *cell lineage iv and bug fixes
 *
 *Revision 1.73  2005/11/07 23:48:22  arman
 *fixed a bug in using translation and correlation
 *
 *Revision 1.72  2005/11/07 22:50:39  arman
 *fixed a bug in new splat processing
 *
 *Revision 1.71  2005/11/07 17:32:09  arman
 *cell lineage iv and bug fix
 *
 *Revision 1.70  2005/11/04 21:59:46  arman
 *in progress.
 *
 *Revision 1.69  2005/09/13 21:59:08  arman
 *16+32 bit support
 *
 *Revision 1.68  2005/07/01 21:05:39  arman
 *version2.0p
 *
 *Revision 1.68  2005/05/15 00:19:17  arman
 *incremental
 *
 *Revision 1.67  2005/02/28 23:00:24  arman
 *minor warning fixes
 *
 *Revision 1.66  2005/01/07 16:28:27  arman
 *removed rcFeature. Simplified
 *
 *Revision 1.65  2004/12/21 22:48:29  arman
 *removed rcFeature from the class
 *
 *Revision 1.64  2004/12/17 05:56:36  arman
 *streamlined and removed check against global motion estimation
 *
 *Revision 1.63  2004/12/16 21:46:21  arman
 *removed rcFeature from API
 *
 *Revision 1.62  2004/12/09 21:47:08  arman
 *use class rcMotionCenter instead of rf
 *
 *Revision 1.61  2004/12/07 21:59:07  arman
 *fixed bug in motion center (not checking for 0 length) and improved reconcile condition
 *
 *Revision 1.60  2004/09/21 20:19:55  arman
 *expanded reconcile to take in to account the target size. incremental.
 *
 *Revision 1.59  2004/09/19 22:19:43  arman
 *removed edge processing
 *added check of correlation space with global motion estimation
 *fixed definition of motion vector and its registration
 *added new debug printouts
 *
 *Revision 1.58  2004/09/18 02:05:13  arman
 *fixed in point corr portion registration issues
 *
 *Revision 1.57  2004/09/17 20:54:58  arman
 *removed detail level processing
 *
 *Revision 1.56  2004/09/13 20:43:57  arman
 *motionpath processing mostly now reserved for a multi-resolution implementation
 *
 *Revision 1.55  2004/09/13 15:51:51  arman
 *updated
 *
 *Revision 1.54  2004/08/25 03:17:35  arman
 *changes in ctor to indicate starting position and displacement.
 *
 *Revision 1.53  2004/08/24 21:36:02  arman
 **** empty log message ***
 *
 *Revision 1.52  2004/08/23 10:38:42  arman
 **** empty log message ***
 *
 *Revision 1.51  2004/08/17 21:15:50  arman
 *fixed motion space bugs and inconsitenties
 *
 *Revision 1.50  2004/08/17 17:29:45  arman
 *turned point corr back on
 *
 *Revision 1.49  2004/08/09 13:22:43  arman
 *turning of fast point corr until the bug is fixed
 *
 *Revision 1.48  2004/08/08 23:49:02  arman
 *converted window functions to no clip
 *
 *Revision 1.47  2004/07/11 07:07:34  arman
 *fixed bug in parabolicDisparity where rcCorrelationWindow was incorrectly constructed
 *
 *Revision 1.46  2004/03/19 12:37:44  arman
 *adding contractile motion source
 *
 *Revision 1.45  2004/03/16 21:36:46  arman
 *added reconcile
 *
 *Revision 1.44  2004/02/27 00:01:02  arman
 *incremental
 *
 *Revision 1.43  2004/02/24 22:46:07  arman
 *added partial sum caching (correlationWindow)
 *
 *Revision 1.42  2004/02/16 21:50:25  arman
 *updated according to rcFeature changes
 *
 *Revision 1.41  2004/02/11 21:40:56  arman
 *major cleanup. rework of bound checking
 *
 *Revision 1.40  2004/02/03 21:33:50  arman
 *fixed parentwindow overload issue
 *
 *Revision 1.39  2004/02/03 15:02:40  arman
 *incremental
 *
 *Revision 1.38  2004/01/26 21:04:46  arman
 *cleaning
 *
 *Revision 1.37  2004/01/18 18:04:28  arman
 *added least square fitting
 *
 *Revision 1.36  2004/01/15 00:32:43  arman
 *major changes
 *
 *Revision 1.35  2004/01/08 22:45:51  arman
 *temporary ci to fix build problems
 *
 *Revision 1.34  2004/01/06 18:52:43  arman
 *yet another correction
 *
 *Revision 1.33  2004/01/06 18:51:46  arman
 *updated use of rfParfit2DR3
 *
 *Revision 1.32  2003/12/09 03:38:36  arman
 *initialize label to 0.
 *
 *Revision 1.31  2003/12/05 19:25:03  arman
 *interim checkin
 *
 *Revision 1.30  2003/11/26 21:44:06  arman
 *fixed a bug in temporal integration
 *
 *Revision 1.29  2003/11/25 01:59:10  arman
 *interim
 *
 *Revision 1.28  2003/11/03 22:28:27  arman
 *added parabolic2D interpolation support
 *
 *Revision 1.27  2003/08/28 21:59:53  proberts
 *Convert window geometry to signed int fixups
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_kinetoscope.h>
#include <rc_fit.h>
#include <iomanip.h>
#include <rc_altivecutil.h>
#include <rc_analysis.h>


// UnComment for debgging printouts
//#include <rc_utdrawutils.h>
//#define MATHPRINT
//       cerr << fixedw << endl;
//       cerr << movingw << endl;
//       cerr << endl << "sm = {";
//       for (uint32 j = 0; j < mSpace.size(); j++)
// 	{
// 	  cerr << endl;
// 	  for (uint32 i = 0; i < searchSpace.x(); i++)
// 	    {
// 	      cerr << setw (4);
// 	      cerr << (int32) (mSpace[j][i] * 1000); 
// 	      if (i < searchSpace.x() - 1) 
// 		cerr << "+";
// 	    }
// 	}


rcFeature rcFeature::operator*(float fm) const
{
    return rcFeature (mD.x().real() * fm, mD.y().real() * fm);
}


rcFeature& rcFeature::operator*=(float fm) 
{
    return *this = *this * fm;
}


float rcFeature::operator*(const rcFeature& other) const
{
	// dot (inner) product
	return (mD.x().real() * other.mD.x().real() +
			mD.y().real() * other.mD.y().real());
}

rcFeature rcFeature::project(const rcFeature& other) const
{
    return ((other * *this) / (*this * *this)) * *this;
}


rcMotionVectorPath::rcMotionVectorPath () 
{
	mSplat.resize (rcKinetoscope::eSplatArea);
}

rcMotionVectorPath::rcMotionVectorPath (rcKinetoscope* kin,const rc2Fvector& anch) :
mKine ( kin )
{
	anchor (anch);
	mSplat.resize (rcKinetoscope::eSplatArea);
}

rcMotionVectorPath::rcMotionVectorPath (rcKinetoscope* kin, const rc2Fvector& anch, const rc2Fvector& disp)
: mKine ( kin )
{
	anchor(anch);
	mSplat.resize (rcKinetoscope::eSplatArea);
	mACF = disp;
}


// Measure disparity:
// Anchor corresponds to the center of the target template.


//                |       |       |       |       |
//                |       |       |       |       |
//                |       |       |       |       |
//         -------+-------+-------+-------+-------+
//                |       |       |       |       |
//                |       |       |       |       |
//                |       |     //+\\     |       |
//         -------+-------+-------+-------+-------+ TopLeft of the Anchor Pixel
//                |       |     \\+//     |       |
//                |       |       | /\    |       |
//                |       |       | \/    |       | Current Anchor
//         -------+-------+-------+-------+-------+
//                |       |       |       |       |
//                |       |       |       |       |
//                |       |       |       |       |
//         -------+-------+-------+-------+-------+
//                |       |       |       |       |

void rcMotionVectorPath::measure ()
{
	// Anchor is the center of the origin pixel. This is
	// Independent of target sizes and gradient filter sizes
	// MovingSize is 1 + radius * 2 of the search area -- hence divide by 2
	// 
	
	rcIPair anc (iAnchor ());
	rcIPair span = sharedKin()->movingSize ();
	rcIPair target = sharedKin()->target ();
	
	rcIRect fixed (anc.x() - target.x(), anc.y() - target.y(), 
				   target.x()+target.x()+1, target.y()+target.y()+1);
	
	rcIRect moving (anc.x() - span.x(), anc.y() - span.y(), 
					span.x()+span.x()+1, span.y()+span.y()+1);
	
	if (sharedKin()->isWithin(fixed) &&  sharedKin()->isWithin(moving))
		parabolicDisparity (fixed, moving);
	
}


// Upddate Statistics:

void rcMotionVectorPath::update ()
{
	//  move (mNewFeature.pos());
}

void rcMotionVectorPath::parabolicDisparity (const rcRect& fixed, const rcRect& moving)
{
	float maxScore (0.0);
	rcPair<bool> edge;  
	rc2Fvector interp;
	rcCorr maxCorr;
	vector<vector<float> > mSpace;
	
	// Size and create search space
	const rcIPair searchSpace (moving.width() - fixed.width() + 1,
							   moving.height() - fixed.height() + 1);
	
	rc2Fvector origin (searchSpace);
	rcIPair maxPos = searchSpace;
	vector<float> score (rcKinetoscope::eSplatArea);
	origin /= 2.0f;
	
	
	bool fastCorr = sharedKin()->use4fixed().depth() == rcPixel8 && 
    searchSpace.y() == 5 && searchSpace.x() == 5 &&
    sharedKin()->isPointCorrOptimizationAvailable ();
	
	    {
		rcWindow fixedw (sharedKin()->use4fixed(), fixed);
		rcWindow movingw (sharedKin()->use4moving(), moving);
		rmAssert (fixedw.depth() == movingw.depth());
		maxScore = match (fixedw, movingw, searchSpace, maxCorr, maxPos, mSpace);
		
		
		edge.y() = !(maxPos.y() > 0 && maxPos.y() < (searchSpace.y() - 1));
		edge.x() = !(maxPos.x() > 0 && maxPos.x() < (searchSpace.x() - 1));
		
		if (!edge.x() && !edge.y())
		{
			score[0] = mSpace[maxPos.y()-1][maxPos.x()-1];
			score[1] = mSpace[maxPos.y()-1][maxPos.x()];
			score[2] = mSpace[maxPos.y()-1][maxPos.x()+1];
			score[3] = mSpace[maxPos.y()][maxPos.x()-1];
			score[4] = mSpace[maxPos.y()][maxPos.x()];
			score[5] = mSpace[maxPos.y()][maxPos.x()+1];
			score[6] = mSpace[maxPos.y()+1][maxPos.x()-1];
			score[7] = mSpace[maxPos.y()+1][maxPos.x()];
			score[8] = mSpace[maxPos.y()+1][maxPos.x()+1];
		}

	
	if (!edge.x() && !edge.y())  
    {
		static float * dnull (0);
		interp = rc2Fvector (parabolicFit (score [3], score [4], score [5], dnull), 
							 parabolicFit (score[1], score [4], score [7], dnull));
		for (uint32 i = 0; i < score.size();i++)
		{
			mSplat[i] = uint8 (255 * score[i]);
		}
		
		interp += rc2Fvector (maxPos.x()+0.5f, maxPos.y()+0.5f);
		interp -= origin;
    }
	
	// @note We do this here to avoid calling update 
	move (interp);
	
	    }
}

ostream& operator<< (ostream& o, const rcMotionVectorPath& p)
{
	
	o << "--------------------------" << endl;
	o << "Trace    :" << p.motion() << endl;
	o << "Anchor   :" << p.anchor() << endl;
	//  o << "Siblings :" << p.sharedKin()->refCount () << endl;
	o << "--------------------------" << endl;
	return o;
}


// +------------+------------+------------+
// |            |            |            |
// |            |            |            |
// |     +      |     +      |     +      |
// |            |            |            |
// +------------+------------+------------+
// |            |         *  |            |
// |            |            |            |
// |     +      |     +      |     +      |
// |            |            |            |
// +------------+------------+------------+
// |            |            |            |
// |            |            |            |
// |     +      |     +      |     +      |
// |            |            |            |
// +------------+------------+------------+

void rcMotionVectorPath::reconcile ()
{
	// When we implement pyramid processing, this
	// will be added the logic to take the low resolution
	// motion vectors and check them at a higher
	// resolution
	// For now it is assumed that we only have one
	// level pyramid
	// See how prependicular are ACF and motion. We want them as prependicular as possible
	// Which means we want to dot product between the two vectors to be as close to 0 as possible
	// Adding Velocity Fields
	//  rcFeature d16 (motion ());
	//   rcIPair ip = iAnchor ();
	//   mKine->velocityXfield().setPixel (ip.x(), ip.y(), (uint32) (d16.pos16().x().real()));
	//   mKine->velocityYfield().setPixel (ip.x(), ip.y(), (uint32) (d16.pos16().y().real()));
	
	if (motion() == rc2Fvector()) return;
	
	float l2 = motion().l2();
	
	// @note default setting
	if (l2 < 0.0025) return;
	
	rcWindow& connect = mKine->connect(); 
	const rcWindow& moving = mKine->use4moving(); 
	rcIPair pt2 ((int32) (anchor().x() + motion().x()), 
				 (int32) (anchor().y() + motion().y()));
	rcIPair target (rcKinetoscope::eSplatSize / 2);
	vector<uint8>::const_iterator sp = mSplat.begin();
	
	
	for (int32 j =  - target.y(); j <= target.y(); j++)
		for (int32 i =  - target.x(); i <= target.x(); i++)
		{
			rcIPair pt = pt2 + rcIPair (i, j);
			
			rmAssertDebug (connect.isWithin(pt));
			uint8 *pp = (uint8 *) connect.pelPointer (pt.x(), pt.y());
			uint8 *mm = (uint8 *) moving.pelPointer (pt.x(), pt.y());
			*pp = (*sp++ * *mm++) >> 8;
		}
	
	
}

// Macro defining depth specific match function. 
// @todo redesign this ugly bit

#define defineMatchFunction(name,pelType,pelTypePtr)				\
float rcMotionVectorPath::name (const rcWindow& fixed, rcWindow& moving, const rcIPair& searchSpace, \
rcCorr& maxCorr, rcIPair& maxPos, vector<vector<float> >& mSpace) \
{\
static const rsCorrParams cp;\
mSpace.resize(searchSpace.y());\
for (int32 i = 0; i < searchSpace.y(); i++)\
mSpace[i].resize (searchSpace.x());\
float maxScore (0.0f);\
rcCorrelationWindow<pelType> fixedWin (fixed);\
rcCorrelationWindow<pelType> movingWin (rcWindow (moving, 0, 0, fixed.width(), fixed.height()));\
const rcIPair colUpdate (1, 0);\
rcIPair rowUpdate (-searchSpace.x(), 1);\
int32 curRow = 0;\
do {\
int32 curCol = 0;\
do {\
rcCorr res;\
rmAssertDebug ((pelTypePtr) moving.pelPointer (curCol, curRow) == (pelTypePtr) movingWin.pelPointer (0,0)); \
rfCorrelateWindow (fixedWin, movingWin,cp, res);\
const float r = res.r();\
mSpace[curRow][curCol] = r;\
if (r > maxScore)\
{\
maxPos.x() = curCol; maxPos.y() = curRow;\
maxScore = r;\
maxCorr = res;\
}\
} while (movingWin.translate(colUpdate) && ++curCol && curCol < searchSpace.x()); \
rowUpdate[0] = -curCol;\
curRow++; \
} while(movingWin.translate(rowUpdate) && curRow < searchSpace.y()); \
rmAssert(curRow == searchSpace.y());		\
return maxScore;\
}


// Avoid code duplication

defineMatchFunction(_match_u8,uint8,uint8*);
defineMatchFunction(_match_u16,uint16,uint16*);
defineMatchFunction(_match_u32,uint32,uint32*);
