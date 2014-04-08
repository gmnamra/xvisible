/*
 *  rc_rlewindow.cpp
 *  
 *  $Id: rc_rlewindow.cpp 7297 2011-03-07 00:17:55Z arman $
 *  Created by Arman Garakani on Tue Jun 04 2002.  Copyright (c) 2002
 *  Reify Corp. . All rights reserved.
 *
 */

#include <rc_rlewindow.h>
#include "rc_rle.h"
#include <rc_histstats.h>
#include <numeric>

// Includes and definitions specific to polygon conversion
//
#include <rc_polygon.h>

enum gpCompassDir {
  gpEast = 0,
  gpNorth,
  gpWest,
  gpSouth,
  gpNorthEast,
  gpSouthEast,
  gpNorthWest,
  gpSouthWest,
  gpCompassDirCnt
};

enum gpRelativeDir {
  gpLeft = 0,
  gpRight,
  gpUp,
  gpUpLeft,
  gpUpRight,
  gpDnLeft,
  gpDnRight,
  gpCenter,
  gpRelativeDirCnt
};

typedef struct gpOffset
{
  rcIPair off[gpRelativeDirCnt];
} gpOffset;

const gpOffset xlat[gpCompassDirCnt] =
  {
    { { rcIPair(0, -1), rcIPair(0, 1), rcIPair(1, 0), rcIPair(1, -1), 
	rcIPair(1, 1), rcIPair(-1, -1), rcIPair(-1, 1), rcIPair(0, 0) } }, // E

    { { rcIPair(-1, 0), rcIPair(1, 0), rcIPair(0, -1), rcIPair(-1, -1), 
	rcIPair(1, -1), rcIPair(-1, 1), rcIPair(1, 1), rcIPair(0, 0) } }, // N

    { { rcIPair(0, 1), rcIPair(0, -1), rcIPair(-1, 0), rcIPair(-1, 1), 
	rcIPair(-1, -1), rcIPair(1, 1), rcIPair(1, -1), rcIPair(0, 0) } }, // W

    { { rcIPair(1, 0), rcIPair(-1, 0), rcIPair(0, 1), rcIPair(1, 1), 
	rcIPair(-1, 1), rcIPair(1, -1), rcIPair(-1, -1), rcIPair(0, 0) } }, // S

    { { rcIPair(-1, -1), rcIPair(1, 1), rcIPair(1, -1), rcIPair(0, -1), 
	rcIPair(1, 0), rcIPair(-1, 0), rcIPair(0, 1), rcIPair(0, 0) } }, // NE

    { { rcIPair(1, -1), rcIPair(-1, 1), rcIPair(1, 1), rcIPair(1, 0), 
	rcIPair(0, 1), rcIPair(0, -1), rcIPair(-1, 0), rcIPair(0, 0) } }, // SE

    { { rcIPair(-1, 1), rcIPair(1, -1), rcIPair(-1, -1), rcIPair(-1, 0), 
	rcIPair(0, -1), rcIPair(0, 1), rcIPair(1, 0), rcIPair(0, 0) } }, // NW

    { { rcIPair(1, 1), rcIPair(-1, -1), rcIPair(-1, 1), rcIPair(0, 1), 
	rcIPair(-1, 0), rcIPair(1, 0), rcIPair(0, -1), rcIPair(0, 0) } } // SW
  };

const gpCompassDir dirMap[3][3] =
  {
    { gpNorthWest, gpWest, gpSouthWest },

    { gpNorth, gpCompassDirCnt, gpSouth },

    { gpNorthEast, gpEast, gpSouthEast }
  };

#define GPSRCHSEQSZ 6

static gpRelativeDir orthoSearchSeq[GPSRCHSEQSZ] =
  { gpRight, gpUp, gpUpRight, gpLeft, gpUpLeft, gpDnLeft };

static gpRelativeDir diagSearchSeq[GPSRCHSEQSZ] =
  { gpRight, gpUpRight, gpUpLeft, gpUp, gpDnLeft, gpLeft };

static gpRelativeDir* searchSeq[gpCompassDirCnt] =
  { orthoSearchSeq, orthoSearchSeq, orthoSearchSeq, orthoSearchSeq,
    diagSearchSeq, diagSearchSeq, diagSearchSeq, diagSearchSeq };

static gpRelativeDir originCheck[gpCompassDirCnt] =
  { gpUp, gpUp, gpUp, gpUp, gpUpLeft, gpUpLeft, gpUpLeft, gpUpLeft };

static gpRelativeDir orthoFillSeq[4] =
  { gpUpLeft, gpLeft, gpDnLeft, gpRelativeDirCnt };

static gpRelativeDir diagFillSeq[3] =
  { gpUpLeft, gpDnLeft, gpRelativeDirCnt };

static gpRelativeDir* fillSeq[gpCompassDirCnt] =
  { orthoFillSeq, orthoFillSeq, orthoFillSeq, orthoFillSeq,
    diagFillSeq, diagFillSeq, diagFillSeq, diagFillSeq };

// Debugging support for polygon conversion
//
#ifdef DYNAMIC_DEBUG
int __polygonDebug = 0;
#define GPDEBUG1 __polygonDebug
#define GPDEBUG2 __polygonDebug
#else
#define GPDEBUG1 0
#define GPDEBUG2 0
#endif

static char* compassStr[gpCompassDirCnt] =
  { "East", "North", "West", "South", "NorthEast",
    "SouthEast", "NorthWest", "SouthWest"
  };

static char* relDirStr[gpRelativeDirCnt] =
  { "Left", "Right", "Up", "UpLeft", "UpRight",
    "DnLeft", "DnRight", "Center"
  };

static char* pxlClass(uint16 bgPix, uint16 fgPix, uint16 ptPix,
		      uint16 pix)
{
  if (pix == bgPix)
    return "BG";
  else if (pix == fgPix)
    return "FG";
  else
    rmAssert(pix == ptPix);

  return "PT";
}

// public

rcRleWindow::~rcRleWindow ()
{
    deleteRep();
}

rcRleWindow::rcRleWindow(const rcRleWindow& rhs) :
        mRep( 0 ), mMaxPixelValue( 0 )
{
    copy(rhs);
}

rcRleWindow& rcRleWindow::operator= (const rcRleWindow& rhs)
{
    if (this != &rhs)
    {
        copy(rhs);
    }
    return *this;
}

void rcRleWindow::image(rcWindow& frame) const
{
    rmAssert (!frame.isBound());

    rcPixel depth = maxPixelDepth();
    
    frame = rcWindow(width(), height(), depth);

    if ( depth == rcPixel8 ) {
        image8(frame);
    } else if ( depth == rcPixel16 ) {
        image16(frame);
    } else if ( depth == rcPixel32 ) {
        image32(frame);
    }
}

// Produce contour image (run end points only)
void rcRleWindow::contourImage(rcWindow& frame) const
{
    rmAssert (!frame.isBound());
     
    rcPixel depth = maxPixelDepth();
    
    frame = rcWindow(width(), height(), depth);

    frame.setAllPixels( 0 );
    
    if ( depth == rcPixel8 ) {
        contourImage8(frame);
    } else if ( depth == rcPixel16 ) {
        contourImage16(frame);
    } else if ( depth == rcPixel32 ) {
        contourImage32(frame);
    }
}

uint32 rcRleWindow::setN() const
{
  uint32 pCnt = 0;

  for (int32 y = 0; y < height(); y++) {
    for (const rcRleRun* rp = pointToRow(y); rp->length != 0; rp++)
      if (rp->value != 0)
	pCnt += rp->length;
  }

  return pCnt;
}

#define GP_GETPT(W,PT) ((uint16)(W).getPixel((PT).x(), (PT).y()))

void rcRleWindow::polygon(rcPolygon& p) const
{
  if (!internalPolygon(p, false)) {
    bool success = internalPolygon(p, true);
    rmAssert(success);
  }
}

bool rcRleWindow::internalPolygon(rcPolygon& p, bool fixup) const
{
  p = rcPolygon(); // Start by clearing result

  uint16 fgPix = 0;

  for (int32 y = 0; y < height(); y++) {
    const rcRleRun* rp = pointToRow(y);
    rmAssert(rp);
    while (rp->length != 0 && rp->value == 0)
      rp++;
    if (rp->value != 0) {
	fgPix = rp->value;
	break;
    }
  }

  if (fgPix == 0) {
    if (GPDEBUG1) cout << "No foreground pixels" << endl;
    return true;
  }

  const uint32 pSum = setN();

  if (GPDEBUG1) cout << "Set Pixels " <<  pSum << endl;

  /* Set up a bunch of constants and temporary images.
   */
  const uint16 bgPix = 0;
  const uint16 ptPix = (fgPix == 0xFF) ? 0xFE : 0xFF;

  rcWindow temp(image());

  rcWindow work(temp.width() + 4, temp.height() + 4, temp.depth());
  work.setAllPixels(bgPix);
  rcWindow workWin(work, 2, 2, temp.width(), temp.height());

  workWin.copyPixelsFromWindow(temp);

  /* Step 1 - Find first set pixel.
   */
  rcIPair startPt;

  bool searching = true;
  for (int32 y = 2; (y < work.height() - 2) && searching; y++)
    for (int32 x = 2; (x < work.width() - 2) && searching; x++)
      if (work.getPixel(x,y) == fgPix) {
	startPt = rcIPair(x,y);
	searching = false;
      }

  rmAssert(!searching);

  /* Handle 1 and 2 point blobs as special case.
   */
  if (pSum < 3) {
    p.pushVertex(rc2Dvector(startPt.x() - 2, startPt.y() - 2));    
    if (pSum == 2) {
      for (int32 yo = -1; yo < 2; yo++)
	for (int32 xo = -1; xo < 2; xo++) {
	  if ((xo == 0) && (yo == 0))
	    continue;

	  rcIPair ptLoc = startPt + rcIPair(xo,yo);
	  uint16 pixVal = GP_GETPT(work, ptLoc);
	  if (pixVal == fgPix) {
	    p.pushVertex(rc2Dvector(ptLoc.x() - 2, ptLoc.y() - 2));
	    return true;
	  }
	}
      rmAssert(0);
    }
    return true;
  }

  /* Set up initial conditions.
   */
  int32 polySz = 0;
  rcIPair curPt = startPt;
  gpCompassDir curDir = gpSouth;
  gpRelativeDir* srchSeqP = searchSeq[curDir];
  gpRelativeDir originCheckDir = originCheck[curDir];
  rcIPair curSlope = rcIPair(0,0);
  
  {
    rcIPair sAbove = startPt + rcIPair(0, -1);
    rcIPair sLeft = startPt + rcIPair(-1, 0);

    work.setPixel(sAbove.x(), sAbove.y(), ptPix);
    work.setPixel(sLeft.x(), sLeft.y(), ptPix);

    if (!fixup) {
      rcIPair sAboveRight = startPt + rcIPair(1, -1);
      work.setPixel(sAboveRight.x(), sAboveRight.y(), ptPix);
    }
  }

  if (GPDEBUG1) cout << "Init: @ " << curPt - rcIPair(2,2) << endl;
  
  uint16 pxl[gpRelativeDirCnt];

  for (;;) {
    if (GPDEBUG2) cout << endl << "New Loop" << endl
		       << "curPt " << curPt  - rcIPair(2,2)
		       << " curDir " << compassStr[curDir]
		       << " curSlope " << curSlope << endl;

    rcIPair upLeftPt = curPt + xlat[curDir].off[gpUpLeft];
    rcIPair upCenterPt = curPt + xlat[curDir].off[gpUp];
    rcIPair upRightPt = curPt + xlat[curDir].off[gpUpRight];
    rcIPair leftPt = curPt + xlat[curDir].off[gpLeft];
    rcIPair centerPt = curPt + xlat[curDir].off[gpCenter];
    rcIPair rightPt = curPt + xlat[curDir].off[gpRight];
    rcIPair dnLeftPt = curPt + xlat[curDir].off[gpDnLeft];
    rcIPair dnRightPt = curPt + xlat[curDir].off[gpDnRight];
      
    pxl[gpUpLeft] = GP_GETPT(work, upLeftPt);
    pxl[gpUp] = GP_GETPT(work, upCenterPt);
    pxl[gpUpRight] = GP_GETPT(work, upRightPt);
    pxl[gpLeft] = GP_GETPT(work, leftPt);
    pxl[gpCenter] = GP_GETPT(work, centerPt);
    pxl[gpRight] = GP_GETPT(work, rightPt);
    pxl[gpDnLeft] = GP_GETPT(work, dnLeftPt);
    pxl[gpDnRight] = GP_GETPT(work, dnRightPt);

    if (GPDEBUG2) cout << "Locations:          "
		       << " L " << upLeftPt - rcIPair(2,2)
		       << " C " << upCenterPt - rcIPair(2,2)
		       << " R " << upRightPt - rcIPair(2,2)
		       << endl << "                    "
		       << " L " << leftPt - rcIPair(2,2)
		       << " C " << centerPt - rcIPair(2,2)
		       << " R " << rightPt - rcIPair(2,2)
		       << endl << "                    "
		       << "DL " << dnLeftPt - rcIPair(2,2) << "      "
		       << "DR " << dnRightPt - rcIPair(2,2)
		       << endl
		       << "Values:             "
		       << " L " << pxlClass(bgPix, fgPix, ptPix, pxl[gpUpLeft])
		       << " C " << pxlClass(bgPix, fgPix, ptPix, pxl[gpUp])
		       << " R " << pxlClass(bgPix, fgPix, ptPix, pxl[gpUpRight])
		       << endl <<  "                    "
		       << " L " << pxlClass(bgPix, fgPix, ptPix, pxl[gpLeft])
		       << " C " << pxlClass(bgPix, fgPix, ptPix, pxl[gpCenter])
		       << " R " << pxlClass(bgPix, fgPix, ptPix, pxl[gpRight])
		       << endl << "                    "
		       << "DL " << pxlClass(bgPix, fgPix, ptPix, pxl[gpDnLeft])
		       << "     "
		       << "DR " << pxlClass(bgPix, fgPix, ptPix, pxl[gpDnRight])
		       << endl;

    gpRelativeDir newRelDir;
    uint32 newDirIndex;
    for (newDirIndex = 0; newDirIndex < GPSRCHSEQSZ; newDirIndex++ ) {
      newRelDir = srchSeqP[newDirIndex];
      
      rcIPair testPt = curPt + xlat[curDir].off[newRelDir];
      if (testPt == startPt) {
	if (GPDEBUG1) cout << "Done case:";

	/* The algorithm tries to follow the boundary in orthogonal
	 * directions first. This can sometimes lead to the end of the
	 * polygon being found prematurely. The following test checks
	 * to see if there is a foreground pixel to be found
	 * diagonally and outside of the start pixel. If so, follow it
	 * instead.
	 */	
	if (newRelDir == originCheckDir) {
	  gpRelativeDir tempRelDir = srchSeqP[newDirIndex + 1];
	  if (pxl[tempRelDir] == fgPix) {
	    if (GPDEBUG1) cout << " aborted" << endl;
	    newDirIndex++;
	    newRelDir = tempRelDir;
	    testPt = curPt + xlat[curDir].off[newRelDir];
	    break;
	  }
	}

	rcIPair newSlope = xlat[curDir].off[newRelDir];
	
	if (curSlope != newSlope) {
	  if (GPDEBUG1) cout << " Pushing " << curPt - rcIPair(2,2);
	  p.insertVertex(rc2Dvector(curPt.x() - 2, curPt.y() - 2), polySz);
	  polySz++;
	}
	if (GPDEBUG1) cout << endl;
	return true;
      }
      else if (pxl[newRelDir] == fgPix)
	break;
    }

    if (newDirIndex != GPSRCHSEQSZ) {
      rcIPair newSlope = xlat[curDir].off[newRelDir];

      if (GPDEBUG1) cout << "newRelDir " << relDirStr[newRelDir] << " curDir "
			 << compassStr[curDir] << " newSlope "
			 << newSlope << endl;

      if (curSlope != newSlope) {
	p.insertVertex(rc2Dvector(curPt.x() - 2, curPt.y() - 2), polySz);
	polySz++;
	curSlope = newSlope;
	curDir = dirMap[newSlope.x() + 1][newSlope.y() + 1];
	if (GPDEBUG1) cout << " Pushing " << curPt - rcIPair(2,2)
			   << " newDir " << compassStr[curDir] << endl;;
      }

      gpRelativeDir* fillSeqP = fillSeq[curDir];
	
      if (GPDEBUG2) {
	cout << "Fill Pts:";
	for (uint32 fillIndex = 0; fillSeqP[fillIndex] != gpRelativeDirCnt;
	     fillIndex++) {
	  gpRelativeDir fillDir = fillSeqP[fillIndex];
	  rcIPair fillPt = curPt + xlat[curDir].off[fillDir];
	  uint16 fillPxl = GP_GETPT(work, fillPt);
	  
	  cout << " (d " << relDirStr[fillDir] << " l "
	       << fillPt - rcIPair(2,2)
	       << " v " << pxlClass(bgPix, fgPix, ptPix, fillPxl) << ")";
	}
	cout << endl;
      }

      for (uint32 fillIndex = 0; fillSeqP[fillIndex] != gpRelativeDirCnt;
	   fillIndex++) {
	gpRelativeDir fillDir = fillSeqP[fillIndex];
	rcIPair fillPt = curPt + xlat[curDir].off[fillDir];
	uint16 fillPxl = GP_GETPT(work, fillPt);
	
	if (fillPxl == ptPix)
	  break;
	if (fillPxl == fgPix)
	  continue;
	if (GPDEBUG1) cout << "FILL@ " << fillPt - rcIPair(2,2) << " O: "
			   << pxlClass(bgPix, fgPix, ptPix, fillPxl)
			   << " N: FG" << endl;
	work.setPixel(fillPt.x(), fillPt.y(), fgPix);
      }
    }
    else // Origin not found
      return false;

    if (GPDEBUG1) cout << "PT@ " << curPt - rcIPair(2,2) << " O: "
		       << pxlClass(bgPix, fgPix, ptPix,
				   work.getPixel(curPt.x(), curPt.y()))
		       << " N: PT" << endl;
    work.setPixel(curPt.x(), curPt.y(), ptPix);

    curPt = curPt + curSlope;
    srchSeqP = searchSeq[curDir];
    originCheckDir = originCheck[curDir];
  } // End of: for(;;)

  return false;
}

rcWindow rcRleWindow::image() const
{
    rcWindow frame;
    if (mRep == 0) return frame;

    image( frame );
  
    return frame;
}


int32 rcRleWindow::width() const
{
    return mRep->width_;
}

int32 rcRleWindow::height() const
{
    return mRep->runTable_.size();
}

const rcRleWindow::Run* rcRleWindow::pointToRow(int32 rowNum) const
{
    return mRep->runTable_[rowNum];
}

const rcRleWindow::Run* const * rcRleWindow::pointToRat() const
{
    return &mRep->runTable_[0];
}

uint16 rcRleWindow::getPel(int32 x, int32 y) const
{
    assert (y  < height());
    assert (x  < mRep->width_);

    uint32 xr = x;
    const Run* runs = mRep->runTable_[y];
    while (xr)
    {
        assert(runs->length);
        if (xr < runs->length) break;
        xr -= runs->length;
        ++runs;
    }

    return runs->value;
}

void rcRleWindow::encode (const rcWindow& image, uint32 maxPixelValue,
			  const uint32 fgPixelValue)
{
    rmAssert(image.width() > 0);
    rmAssert(image.height() > 0);
    rmAssert( maxPixelValue <= rcUINT16_MAX );
    
    mMaxPixelValue = maxPixelValue;
    
    rcPixel depth = image.depth();
    mRep = new rcRleBuf;

    mRep->width_ = image.width();
    int32 height = image.height();
    mRep->runTable_.resize(height);
    int32 runsLeft = sizeRuns();
    Run* runs = allocRuns(runsLeft);

    for (int32 y = 0, imageY = 0 ; y < height ; ++y, ++imageY)
    {
        /* make sure we've got enough runs for this row, worst case */
        if (runsLeft < (mRep->width_ + 1))
        {
            /* nope, get a bigger chunk (wasting whatever was remaining) */
            uint32 count = sizeRuns();
            runs = allocRuns(count);
            runsLeft = count;
        }
        
        // do a row
        mRep->runTable_[y] = runs;
        int w = mRep->width_;
        int lastLength;
        uint32 lastValue;

        if ( depth == rcPixel8 ) {
            const uint8* pel = image.rowPointer (imageY);
            const uint8* runStart = pel;
	    if (!fgPixelValue || fgPixelValue > rcUINT8_MAX)
	    {
	        lastValue = *pel;
		// inner loop
		while (w--)
	        {
		    if (*pel != lastValue)
		    {
		        lastLength = pel - runStart;
			OUT_RUN(runs);
			lastValue = *pel;
			runStart = pel;
		    }
		    ++pel;
		}
	    }
	    else
	    {
	        const uint8 fgVal = (uint8)fgPixelValue;
	        lastValue = (*pel == fgVal) ? fgVal : 0;
		// inner loop
		while (w--)
	        {
		    const uint8 curVal = (*pel == fgVal) ? fgVal : 0;
		    if (curVal != lastValue)
		    {
		        lastLength = pel - runStart;
			OUT_RUN(runs);
			lastValue = curVal;
			runStart = pel;
		    }
		    ++pel;
		}
	    }

            lastLength = pel - runStart;
            OUT_RUN(runs);
        } else if ( depth == rcPixel16 ) {
            const uint16* pel =  (uint16 *) image.rowPointer (imageY);
            const uint16* runStart = pel;

	    if (!fgPixelValue || fgPixelValue > rcUINT16_MAX)
	    {
	        lastValue = *pel;
		// inner loop
		while (w--)
		{
		    if (*pel != lastValue)
		    {
		        lastLength = pel - runStart;
			OUT_RUN(runs);
			lastValue = *pel;
			runStart = pel;
		    }
		    ++pel;
		}
	    }
	    else
	    {
	        const uint16 fgVal = (uint16)fgPixelValue;
	        lastValue = (*pel == fgVal) ? fgVal : 0;
		// inner loop
		while (w--)
	        {
		    const uint16 curVal = (*pel == fgVal) ? fgVal : 0;
		    if (curVal != lastValue)
		    {
		        lastLength = pel - runStart;
			OUT_RUN(runs);
			lastValue = curVal;
			runStart = pel;
		    }
		    ++pel;
		}
	    }

            lastLength = pel - runStart;
            OUT_RUN(runs);
        } else if ( depth == rcPixel32 ) {
            const uint32* pel =  (uint32 *) image.rowPointer (imageY);
            const uint32* runStart = pel;

	    if (!fgPixelValue || fgPixelValue > rcUINT16_MAX)
	    {
	        lastValue = *pel;
		// inner loop
		while (w--)
                {
		    if (*pel != lastValue)
		    {
		        lastLength = pel - runStart;
			OUT_RUN(runs);
			lastValue = *pel;
			runStart = pel;
		    }
		    ++pel;
		}
	    }
	    else
	    {
	        const uint32 fgVal = fgPixelValue;
	        lastValue = (*pel == fgVal) ? fgVal : 0;
		// inner loop
		while (w--)
	        {
		    const uint32 curVal = (*pel == fgVal) ? fgVal : 0;
		    if (curVal != lastValue)
		    {
		        lastLength = pel - runStart;
			OUT_RUN(runs);
			lastValue = curVal;
			runStart = pel;
		    }
		    ++pel;
		}
	    }

            lastLength = pel - runStart;
            OUT_RUN(runs);
        }
        
        // make EOR marker
        runs->value = 0;
        runs->length = 0;
        ++runs;

        const int runsThisRow = runs - mRep->runTable_[y];
        runsLeft -= runsThisRow;
    }

    // Compute Mass 
    // Mass can be 0 (all pixels == 0)
    vector<uint32> proj;
    projectHoriz (proj);
    rmAssert (proj.size() > 0);
    mMass = accumulate (proj.begin(), proj.end(), 0);

    /* Horn method (1D 1st moment)
     * Center is given by .5(proj[0] + 3 proj[1] + 5 proj[2] + ........) / A
     *
     * if A = a + b + c + d + e + ...
     *   then a + 3b + 5c + 7d + ....
     *      = A + 2((A-a) + (A - a - b) + (A - a - b - c) + ....
     *
     * == > Center is .5(A + 2[(A - a) + (A - a - b) + (A - a - b - c) + ...]) / A
     *       == .5 + [(A - a) + (A - a - b) + (A - a - b -c) + .. ] / A 
     */

    double waterLine = mMass;
    double acc  = 0.0;

    for (uint32 i = 0; i < proj.size() - 1; i++)
      {
	waterLine -= proj[i];
	acc += waterLine;
      }
  
    double cx  = 0.5 + acc / mMass;
    projectVert (proj);
    rmAssert (proj.size() > 0);

    waterLine = mMass;
    acc  = 0.0;

    for (uint32 i = 0; i < proj.size() - 1; i++)
      {
	waterLine -= proj[i];
	acc += waterLine;
      }
    double cy  = 0.5 + acc / mMass;

    mCom = rc2Fvector ((float) cx, (float) cy);
}


const rc2Fvector rcRleWindow::medianCenter () const
{
  rc2Fvector Mctr;
  vector<uint32> proj;
  projectHoriz (proj);
  rmAssert (proj.size() > 0);
  rcHistoStats hh (proj);
  Mctr.x(hh.interpolatedMedian());
  projectVert (proj);
  rmAssert (proj.size() > 0);
  rcHistoStats vv (proj);
  Mctr.y(vv.interpolatedMedian());
  return Mctr;
}

uint32 rcRleWindow::n() const
{
    int numRuns = 0;
  
    for (int32 y = 0 ; y < height() ; ++y)
    {
        const Run* runs = pointToRowRel(y);
        while(runs++->length)
            ++numRuns;
    }
    return numRuns;
}

double rcRleWindow::meanLength() const
{
    int32 numRuns = 0;
    int32 totLength = 0;
    for (int32 y = 0 ; y < height() ; ++y)
    {
        // do a row
        const Run* srcRuns = pointToRowRel(y);
        while (1)
        {
	  if (srcRuns->length == 0) break;
	  ++numRuns;
	  totLength += srcRuns->length;
	  ++srcRuns;
        }
    }
  
    return (numRuns ? totLength / (double) numRuns : 0);
}

uint32 rcRleWindow::n(uint16 val) const
{

    int32 answer = 0;
    for (int32 y = 0 ; y < height() ; ++y)
    {
        // do a row
        const Run* srcRuns = pointToRowRel(y);
        while (1)
        {
            if (srcRuns->length == 0) break;
            if (srcRuns->value == val) ++answer;
            ++srcRuns;
        }
    }

    return answer;
}

double rcRleWindow::meanLength(uint16 val) const
{

    int32 numRuns = 0;
    int32 totLength = 0;
    for (int32 y = 0 ; y < height() ; ++y)
    {
        // do a row
        const Run* srcRuns = pointToRowRel(y);
        while (1)
        {
            if (srcRuns->length == 0) break;
            if (srcRuns->value == val)
            {
                ++numRuns;
                totLength += srcRuns->length;
            }
            ++srcRuns;
        }
    }
  
    return (numRuns ? totLength / (double) numRuns : 0);
}

double rcRleWindow::meanLengthNot(uint16 val) const
{

    int32 numRuns = 0;
    int32 totLength = 0;
    for (int32 y = 0 ; y < height() ; ++y)
    {
        // do a row
        const Run* srcRuns = pointToRowRel(y);
        while (1)
        {
            if (srcRuns->length == 0) break;
            if (srcRuns->value != val)
            {
                ++numRuns;
                totLength += srcRuns->length;
            }
            ++srcRuns;
        }
    }
  
    return (numRuns ? totLength / (double) numRuns : 0);
}

bool rcRleWindow::operator== (const rcRleWindow& rhs) const
{

    if (width() != rhs.width() || height() != rhs.height()) 
        return false;

    for (int32 y = 0 ; y < height() ; ++y)
    {
        // do a row
        const Run* srcRuns = pointToRowRel(y);
        const Run* compareRuns = rhs.pointToRowRel(y);
        while (1)
        {
            if (srcRuns->length == 0)
            {
                assert(compareRuns->length == 0);
                return true;
            }
            if ((srcRuns->value != compareRuns->value) ||
                (srcRuns->length != compareRuns->length)) return false;

            ++srcRuns;
            ++compareRuns;
        }
    }
    assert(0);			/* should not get here */
    return false;			/* keep compiler happy */
}



/* For each row, we must calculate the sum of 

run->value * run->length

over all of the runs in the row.  We use a no-multiply approach
that works as follows:

We declare an accumulator table, and accumulate as follows:

accum[ run->value ] += run->length

for all of the runs in the row.  The accum table now looks like:

index   accumulated
value   length
-----   -----
0       a
1       b
2       c
3       d
etc     etc

Then, we perform a sum across the table in reverse order.  For
simplicity, ignore indices greater than 3:

index
value   summed entry
-----   ------------
0       d + c + b + a
1       d + c + b
2       d + c
3       d

Then, the values in bins 1, 2, and 3 are summed, yielding the
following:

0*a + 1*b + 2*c + 3*d

which is the desired sum of run->value * run->length over all
runs in the row.

*/     


void rcRleWindow::projectHoriz(vector<uint32>& projection) const
{
    int32 h = height();
    projection.resize(h);

    Run **runs = &mRep->runTable_[0];
    int32 y;

#ifdef notyet  // Not used yet
    if(mRep->binary_)
    {
        for(y = 0; y < h ; ++y)
        {
            const Run* srcRuns = *runs++;
            uint32 sum[2] = {0};
            while(srcRuns->length)
                sum[srcRuns++->value] += srcRuns->length;
            projection[y] = sum[1];
        }
    }
    else
#endif        
    {
      uint32 zero (0);
      vector<uint32> accum (rcUINT16_MAX, zero);
      for (y = 0 ; y < h; ++y)
      {
          // do a row
          uint16 maxVal = 0;		// maximum value encountered in row
          const Run* srcRuns = *runs++;
          while (srcRuns->length)
          {
              uint16 val = srcRuns->value;
              if (val > maxVal) maxVal = val;
              accum[val] += srcRuns++->length;
          }
	
          /* post-process accum to obtain desired projection sum for this
             row */
          for (int32 n1 = maxVal ; n1 > 1 ; n1--) accum[n1-1] += accum[n1];
          uint32 projSum = 0;
          for (int32 n2 = maxVal ; n2 >= 1 ; n2--)
          {
              projSum += accum[n2];
              accum[n2] = 0;		// clean up for next row
          }
          projection[y] = projSum;
      }
    }
}

void rcRleWindow::projectVert(vector<uint32>& projection) const
{
    int32 w = width(),h = height();
    projection.resize(w);

    uint32 zero (0);
    vector<uint32> accum (w+1, zero);
    Run **runs = &mRep->runTable_[0];

    // Horn Page 60.

    for (int32 y = 0 ; y < h; ++y)
    {
        // do a row
        uint32 xPos = 0;
        const Run* srcRuns = *runs++;
        while (srcRuns->length)
        {
            accum[xPos] += srcRuns->value;
            accum[xPos + srcRuns->length] -= srcRuns->value;
            xPos += srcRuns++->length;
        }
    }

    /* post-process accum to obtain desired projection */
    uint32 sum =  projection[0] = accum[0];
    if(w > 1)
    {
        uint32 *projPtr = &projection[1];
        for (int32 n = 1 ; n < w ; ++n)
        {
            sum += accum[n];
            *projPtr++ = sum;
        }
    }
}

void rcRleWindow::projectDiag(vector<uint32>& projection) const
{
    projection.resize(width() + height() - 1, 0);

    vector<uint32> accum;
    accum.resize(width() + height(),0);

    // Horn again

    for (int32 y = 0 ; y < height() ; ++y)
    {
        // do a row
        uint32 xPos = 0;
        const Run* srcRuns = pointToRowRel(y);
        while (1)
        {
            if (srcRuns->length == 0) break;
            accum[y + xPos] += srcRuns->value;
            accum[y + xPos + srcRuns->length] -= srcRuns->value;
            xPos += srcRuns->length;
            ++srcRuns;
        }
    }

    /* post-process accum to obtain desired projection */
    projection[0] = accum[0];
    for (int32 n = 1 ; n < width()+height()-1 ; ++n)
    {
        projection[n] = projection[n-1] + accum[n];
    }

}

// Debugging: returns number of run structure allocations
uint32 rcRleWindow::allocCount() const
{
    return (mRep ? mRep->allocs_.size() : 0);
}

// Debugging: returns approximate number of bytes allocated by this instance
uint32 rcRleWindow::sizeOf() const
{
    uint32 size = 0;

    if ( mRep ) {
        uint32 runs = mRep->allocs_.size();
        uint32 runSize = sizeRuns();

        // Run structure size
        size = runSize * runs * sizeof(rcRleWindow::Run);
        
        // rcRleBuf size
        size += sizeof(*mRep);
        // Approximate runTable_ size
        size += mRep->runTable_.size() * sizeof( rcRleWindow::Run* );
        // Approximate allocs_ size
        size += runs * sizeof( rcRleWindow::Run* );
    }
    
    // Rest of the class
    size += sizeof( *this );
    
    return size;
}

// private

rcRleWindow::Run* rcRleWindow::allocRuns(int32 howMany)
{
    assert(howMany > 0);
    const int32 size = mRep->allocs_.size();
    
#ifdef rcDEBUG_ALLOC
    if ( size )
        cerr << this << " allocRuns " << width() << " x " << height() << ": " << size << " " << howMany << endl;
#endif
    mRep->allocs_.resize( size+1 );

    Run* buf = new Run[howMany];
    assert(buf);
    mRep->allocs_[size] = buf;
    return buf;
}


int32 rcRleWindow::sizeRuns() const
{
    const int32 w = width();
    const int32 h = height();
    int32 basis = 0;
    
    // Handle common special cases
    if ( w == 1 ) {
        // Each row has exactly two runs
        basis = h * 2;
    } else {
        if ( w < 7 || h < 7 ) {
            // Allocate worst case so no realloc is ever needed
            basis = h * (w+1);
        } else {
            basis = h + w + w  + 1;
#if 0 // Need to investigate allocation patterns more
            basis = h * (2 + w/2) + 1;
            if ( basis > 8192 )
                basis = 8192;
#endif            
        }
    }

    return basis;
}

void rcRleWindow::copy(const rcRleWindow& rhs)
{
    if (!rhs.mRep ) return;

    deleteRep();
    rmAssert (!isBound());

    mRect = rhs.mRect;
    mOrigin = rhs.mOrigin;
    mCom = rhs.mCom;
    mMaxPixelValue = rhs.mMaxPixelValue;
    
    mRep = new rcRleBuf;
    mRep->width_ = rhs.mRep->width_;
    int32 height = rhs.height();

    Run* dest = new Run[rhs.n() + height];

    mRep->allocs_.resize(1);
    mRep->allocs_[0] = dest;

    mRep->runTable_.resize(height);
    for (int32 y = 0 ; y < height ; ++y)
    {
        mRep->runTable_[y] = dest;
        const Run* src = rhs.pointToRowRel(y);
        while (1)
        {
            *dest++ = *src;
            if (src->length == 0) break;
            ++src;
        }
    }

}

const rcRleWindow::Run* rcRleWindow::pointToRowRel(int32 rowNumRel) const
{
    return mRep->runTable_[rowNumRel];
}


void rcRleWindow::deleteRep()
{
    if (mRep) {
        for (uint32 n = 0 ; n < mRep->allocs_.size() ; ++n)
            delete[] mRep->allocs_[n];
        
        delete mRep;
        mRep = 0;
    }
}

void rcRleWindow::setRow8( rcWindow& frame, int32 y ) const
{
     const Run* runs = pointToRowRel(y);
     uint8* pel = frame.rowPointer (y);
        
     uint16 len;
     while ((len = runs->length) != 0)
     {
         const uint16 val = runs->value;
         int32 loop = (len & ~0x3) >> 2;
         while (loop--)
         {
             pel[0] = val;
             pel[1] = val;
             pel[2] = val;
             pel[3] = val;
             pel += 4;
         }
         if (len & 2)
         {
             pel[0] = val;
             pel[1] = val;
             pel += 2;
         }
         if (len & 1)
         {
             pel[0] = val;
             ++pel;
         }
         ++runs;
     }
}

void rcRleWindow::setRow16( rcWindow& frame, int32 y ) const
{
     const Run* runs = pointToRowRel(y);
     uint16* pel = (uint16*)frame.rowPointer (y);
        
     uint16 len;
     while ((len = runs->length) != 0)
     {
         const uint16 val = runs->value;
         int32 loop = (len & ~0x3) >> 2;
         while (loop--)
         {
             pel[0] = val;
             pel[1] = val;
             pel[2] = val;
             pel[3] = val;
             pel += 4;
         }
         if (len & 2)
         {
             pel[0] = val;
             pel[1] = val;
             pel += 2;
         }
         if (len & 1)
         {
             pel[0] = val;
             ++pel;
         }
         ++runs;
     }
}

void rcRleWindow::setRow32( rcWindow& frame, int32 y ) const
{
     const Run* runs = pointToRowRel(y);
     uint32* pel = (uint32*)frame.rowPointer (y);
        
     uint16 len;
     while ((len = runs->length) != 0)
     {
         const uint16 val = runs->value;
         int32 loop = (len & ~0x3) >> 2;
         while (loop--)
         {
             pel[0] = val;
             pel[1] = val;
             pel[2] = val;
             pel[3] = val;
             pel += 4;
         }
         if (len & 2)
         {
             pel[0] = val;
             pel[1] = val;
             pel += 2;
         }
         if (len & 1)
         {
             pel[0] = val;
             ++pel;
         }
         ++runs;
     }
}

void rcRleWindow::image8(rcWindow& frame) const
{
    assert (frame.isBound());
    assert (width() >= frame.width());
    assert (height() >= frame.height());  
    rmAssert( frame.depth() == rcPixel8 );
        
    for (int32 y = 0 ; y < height(); ++y) {
        setRow8( frame, y );
    }
}

void rcRleWindow::image16(rcWindow& frame) const
{
    assert (frame.isBound());
    assert (width() >= frame.width());
    assert (height() >= frame.height());  
    rmAssert( frame.depth() == rcPixel16 );
        
    for (int32 y = 0 ; y < height(); ++y) {
        setRow16( frame, y );
    }
}

void rcRleWindow::image32(rcWindow& frame) const
{
    assert (frame.isBound());
    assert (width() >= frame.width());
    assert (height() >= frame.height());  
    rmAssert( frame.depth() == rcPixel32 );
    
    for (int32 y = 0 ; y < height(); ++y)
    {
        setRow32( frame, y );
    }
}

// Produce contour image which has only run start/end points
// Note: it is a true contour iff the RLE has one connected area.
void rcRleWindow::contourImage8(rcWindow& frame) const
{
    int32 w = frame.width();
    int32 h = frame.height();
    rmAssert (frame.isBound());
    rmAssert (width() >= w);
    rmAssert (height() >= h);  
    rmAssert( frame.depth() == rcPixel8 );

    if ( h ) {
        // Set all pixels of first row
        setRow8( frame, 0 );
        if ( h > 1 ) {
            // Set all pixels of last row
            setRow8( frame, h-1 );
            
            for (int32 y = 1 ; y < h-1; ++y)
            {
                const Run* runs = pointToRowRel(y);
                uint8* pel = frame.rowPointer (y);
                uint8* pelStart = pel;
                
                uint16 len;
                while ((len = runs->length) != 0)
                {
                    uint16 val = runs->value;
                    // Set run start
                    pel[0] = val;
                    if ( len > 1 ) {
                        uint32 trueX = pel - pelStart;
                        // Set pixels in the run
                        for ( int32 x = 1; x < len - 1; ++x ) {
                            // Is contour pixel if pixel below or above has a zero value
                            if ( !getPel( trueX + x, y-1 ) ||
                                 !getPel( trueX + x, y+1 ) )
                                pel[x] = val;
                        }
                        // Set run end
                        pel[len-1] = val;
                    } 
                    pel += len;
                    ++runs;
                }
            }
        }
    }
}

// Produce contour image which has only run start/end points
// Note: it is a true contour iff the RLE has one connected area.
void rcRleWindow::contourImage16(rcWindow& frame) const
{
    int32 w = frame.width();
    int32 h = frame.height();
    rmAssert (frame.isBound());
    rmAssert (width() >= w);
    rmAssert (height() >= h);  
    rmAssert( frame.depth() == rcPixel16 );

    if ( h ) {
        // Set all pixels of first row
        setRow16( frame, 0 );
        if ( h > 1 ) {
            // Set all pixels of last row
            setRow16( frame, h-1 );
            
            for (int32 y = 1 ; y < h-1; ++y)
            {
                const Run* runs = pointToRowRel(y);
                uint16* pel = ( uint16*)frame.rowPointer (y);
                uint16* pelStart = pel;
                
                uint16 len;
                while ((len = runs->length) != 0)
                {
                    uint16 val = runs->value;
                    // Set run start
                    pel[0] = val;
                    if ( len > 1 ) {
                        uint32 trueX = pel - pelStart;
                        // Set pixels in the run
                        for ( int32 x = 1; x < len - 1; ++x ) {
                            // Is contour pixel if pixel below or above has a zero value
                            if ( !getPel( trueX + x, y-1 ) ||
                                 !getPel( trueX + x, y+1 ) )
                                pel[x] = val;
                        }
                        // Set run end
                        pel[len-1] = val;
                    } 
                    pel += len;
                    ++runs;
                }
            }
        }
    }
}

// Produce contour image which has only run start/end points
// Note: it is a true contour iff the RLE has one connected area.
void rcRleWindow::contourImage32(rcWindow& frame) const
{
    int32 w = frame.width();
    int32 h = frame.height();
    rmAssert (frame.isBound());
    rmAssert (width() >= w);
    rmAssert (height() >= h);  
    rmAssert( frame.depth() == rcPixel32 );

    if ( h ) {
        // Set all pixels of first row
        setRow32( frame, 0 );
        if ( h > 1 ) {
            // Set all pixels of last row
            setRow32( frame, h-1 );
            
            for (int32 y = 1 ; y < h-1; ++y)
            {
                const Run* runs = pointToRowRel(y);
                uint32* pel = (uint32*)frame.rowPointer (y);
                uint32* pelStart = pel;
                
                uint16 len;
                while ((len = runs->length) != 0)
                {
                    uint16 val = runs->value;
                    // Set run start
                    pel[0] = val;
                    if ( len > 1 ) {
                        uint32 trueX = pel - pelStart;
                        // Set pixels in the run
                        for ( int32 x = 1; x < len - 1; ++x ) {
                            // Is contour pixel if pixel below or above has a zero value
                            if ( !getPel( trueX + x, y-1 ) ||
                                 !getPel( trueX + x, y+1 ) )
                                pel[x] = val;
                        }
                        // Set run end
                        pel[len-1] = val;
                    } 
                    pel += len;
                    ++runs;
                }
            }
        }
    }
}

// Determine which image depth we need to accommodate all pixel values
rcPixel rcRleWindow::maxPixelDepth() const
{
    rcPixel depth = rcPixelUnknown;
    
    if ( mMaxPixelValue <= rcUINT8_MAX )
        depth = rcPixel8;
    else if ( mMaxPixelValue <= rcUINT16_MAX )
        depth = rcPixel16;
    else
        depth = rcPixel32;

    return depth;
}

// Return value of pixel
uint16 rcRleWindow::getPel(int32 x, const Run* runs) const
{
    while (x)
    {
        if (x < runs->length)
            break;
        x -= runs->length;
        ++runs;
    }

    return runs->value;
}


// Debug output
ostream& operator << ( ostream& os, const rcRleWindow& run )
{
    os << "RLE " << run.width() << " x " << run.height() << ", max " << run.maxPixelValue();
    os << ", runs " << run.n() << ", sizeof " << run.sizeOf();
    os << " Origin " << run.origin() << "Bounding Box " << run.rectangle();
    os << endl;
    
    return os;
}
