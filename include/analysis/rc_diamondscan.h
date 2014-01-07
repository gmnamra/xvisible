/*
 *
 *$Header $
 *$Id $
 *$Log: $
 *
 *
 * Copyright (c) 2006 Reify Corp. All rights reserved.
 */
#ifndef __RC_DIAMONDSCAN_H
#define __RC_DIAMONDSCAN_H

#include <vector>
#include <stdio.h>
#include <rc_analysis.h>
#include <rc_types.h>
#include <rc_pair.h>
#include <rc_window.h>
#include <rc_fit.h>
#include <rc_ip.h>
#include <rc_peak.h>
#include <rc_timinginfo.h>



void rfImageFind (const rcWindow& moving, const rcWindow& fixed, 
		  rcFindList& locations, rcWindow& space, bool exhaustive = true);

void rfImageFind (const rcWindow& moving, const rcWindow& fixed, const rcWindow& mask, 
			  rcFindList& locations, rcWindow& space, bool exhaustive = true);

#define mQuantz 65000

#if 0
class rcPointScan
{
private:
  static const uint16 mNotProcessed = (uint16) mQuantz+1;

 public:
  rcPointScan (const rcWindow& moving, const rcWindow& fixed, 
	       const rcIPair& expectedPos, int32 maxTravel = 4);

  bool setMask (const rcWindow& mask);
  bool peakAndInterpolate ();
  bool climb ();
  bool scan (const vector<rcIPair>&);
  const rcPeak<uint16>& maxPeak () const { return mMaxPeak;}

  static int32 notProcessedLabel () { return mNotProcessed; }
  int32 maxSpan () const;

  // Output Functions
  friend ostream& operator<< (ostream& o, const rcPointScan& p)
  {
    o << p.maxPeak () << endl << "Counters (cd+p): " << 
      p.mCorrCount << "," << p.mDiamCount << "," <<  
      p.mCrossCount << "," <<  p.mPeakCount << endl;
    p.dump ();
    return o;
  }

  void dump () const 
  {
    for (int32 i__temp = 0; i__temp < mTrace.height(); i__temp++)
      {  
	fprintf (stderr, "\n");					    
	uint16* vp = (uint16*)mTrace.rowPointer(i__temp);		    
	for (int32 j__temp = 0; j__temp < mTrace.width(); j__temp++)   
	  {
	    if (vp[j__temp] == rcPointScan::notProcessedLabel ()) fprintf (stderr, "  +  "); 
	    else 
	      fprintf (stderr, "%1.3f", vp[j__temp] == rcPointScan::notProcessedLabel () ? -1.0 : vp[j__temp] / 65000.0f); 
	  }
	fprintf (stderr, "\n");					    
      }
  }

 private:
  rcIPair mMovingPeak, mFrameStart, mFramePeak, mTb2Frame, mMad;
  const rcWindow& mMoving, mFixed;
  rsCorrParams mCorrParams;
  rcPeak<uint16> mMaxPeak;
  rcWindow mTrace, mMask;
  rcIRect mFrame;
  float mMaxScore;
  bool mMaskValid;
  int32 mMaskN;

  int32 mCorrCount, mDiamCount, mCrossCount, mPeakCount;
  bool checkOffset (rcIPair& point)
  {
    return mFrame.contains (point);
  }

};
#endif

template <class T>
void rfPrintCorrSpace (rcWindow& space, T precision, float quantization)
{
  int32 i;
  cerr << ">>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  cerr.setf (ios::fixed);
  cerr << setprecision (4);
  for (int32 j = 0; j < space.height(); j++)
    {
      for (i = 0; i < space.width() - 1; i++)
	cerr << setw(3) << ((*((T *) (space.pelPointer(i, j)))) * precision) / quantization  << "-";
      cerr << setw(3) << ((*((T *) (space.pelPointer(i, j)))) * precision) / quantization  << endl;
    }
  cerr << "<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
}  


void rfPrintFindList (const rcFindList& l, ostream& os);

#endif /* __RC_DIAMONDSCAN_H */
