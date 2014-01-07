/* @file
 *
 *$Id $
 *$Log$
 *Revision 1.13  2005/09/28 23:17:48  arman
 *incremental
 *
 *Revision 1.12  2005/01/26 00:20:38  arman
 *added zfish
 *
 *Revision 1.11  2005/01/25 09:40:33  arman
 *added two window output
 *
 *Revision 1.10  2005/01/24 22:49:19  arman
 *added better calculation of dt
 *
 *Revision 1.9  2005/01/24 16:40:35  arman
 *added angular velocity
 *
 *Revision 1.8  2005/01/21 18:29:09  arman
 *added raw comparison of whole aci to regions acis.
 *
 *Revision 1.7  2005/01/19 22:50:20  arman
 *correlation coherence method
 *
 *Revision 1.6  2005/01/17 14:49:50  arman
 *added pipeline length
 *
 *Revision 1.5  2005/01/14 19:37:42  arman
 *fixed a bug in ttc where image used was not 32bit
 *
 *Revision 1.4  2005/01/14 18:16:00  arman
 *first basically working version
 *
 *Revision 1.3  2005/01/14 16:26:12  arman
 *incremental
 *
 *Revision 1.2  2005/01/14 14:59:39  arman
 *incremental
 *
 *Revision 1.1  2005/01/13 23:07:49  arman
 *lattice similarator
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_latticesimilarity.h>
#include <rc_1dcorr.h>
#include <rc_line.h>

static int32 log2max(int32 n);
static void rfDrawCircle(rc2Dvector center, int32 r, rcWindow& dst, uint8 circleColor);

rcLatticeSimilarator::rcLatticeSimilarator(int32 tpipe, int32 lrad)
  : mTemporal (tpipe), mLrad (lrad)
{
  if (!mLrad)
    rmExceptionMacro (<< "Lrad can not be zero");
}

void rcLatticeSimilarator::ttc (rcWindow& horz, rcWindow& vert)
{
  rcWindow hscreen = horz;
  rcWindow vscreen = vert;

  if (!hscreen.isBound() || hscreen.size() != mImageSize || 
      hscreen.depth() != rcPixel8)
    {
      hscreen = rcWindow (mImageSize, rcPixel8);
      horz = hscreen;
    }

  hscreen.setAllPixels (0);

  if (!vscreen.isBound() || vscreen.size() != mImageSize || 
      vscreen.depth() != rcPixel8)
    {
      vscreen = rcWindow (mImageSize, rcPixel8);
      vert = vscreen;
    }

  vscreen.setAllPixels (0);

  const vector<float>& dreal =  mWholeSim->longTermEntropy ();
  if (!(mWholeSim->longTermCache() && dreal.size()))
   rmExceptionMacro (<< "Lattice Similarity Cache Off");	    

  for (int32 j = 0; j < mSize.y(); j++)
    {
      for (int32 i = 0; i < mSize.x(); i++)
	{
	  const vector<float>& real = mSims[j][i]->longTermEntropy ();
	  if (!(mSims[j][i]->longTermCache() && dreal.size()) && real.size() != dreal.size())
	    rmExceptionMacro (<< "Lattice Similarity Cache Off");	    

	  if (dreal.size() <= 7)
	    continue;

	  rcWindow bb (hscreen, mTiles[j][i]);
	  rcWindow cc (vscreen, mTiles[j][i]);
	  double mTr (0.0), rTp (0.0), pe;

	  // Dt / Dy estimate by y+ - y-
	  if (j > 0 && j < (mSize.y() - 1))
	    {
	      const vector<float>& jmreal = mSims[j-1][i]->longTermEntropy ();
	      if (!(mSims[j-1][i]->longTermCache() && jmreal.size()) && jmreal.size() != real.size())
		rmExceptionMacro (<< "Lattice Similarity Cache Off"); 

	      // jm is at re registration with respect to real
	      mTr = rf1DRegister (jmreal.begin(), jmreal.end(), real.begin(), real.end(), 1, pe);

	      const vector<float>& jpreal = mSims[j+1][i]->longTermEntropy ();
	      if (!(mSims[j+1][i]->longTermCache() && jpreal.size()) && jpreal.size() != real.size())
		rmExceptionMacro (<< "Lattice Similarity Cache Off"); 

	      // real is at re registration with respect to jp
	      // m to p is positive: if mTr and rTp agree (if both negative or positive)
	      // composing these two motions is the sum of mTr and rTp.
	      // returns are within range of - 0.5 to + 0.5. => sum - 1.0 to 1.0
	      mTr += rf1DRegister (jpreal.begin(), jpreal.end(), real.begin(), real.end(),1, pe);
	    }


	  if (i > 0 && i < (mSize.x() - 1))
	    {
	      const vector<float>& jmreal = mSims[j][i-1]->longTermEntropy ();
	      if (!(mSims[j][i-1]->longTermCache() && jmreal.size()) && jmreal.size() != real.size())
		rmExceptionMacro (<< "Lattice Similarity Cache Off"); 

	      // jm is at re registration with respect to real
	      rTp = rf1DRegister (jmreal.begin(), jmreal.end(), real.begin(), real.end(), 1, pe);
	      
	      const vector<float>& jpreal = mSims[j][i+1]->longTermEntropy ();
	      if (!(mSims[j][i+1]->longTermCache() && jpreal.size()) && jpreal.size() != dreal.size())
		rmExceptionMacro (<< "Lattice Similarity Cache Off"); 

	      // real is at re registration with respect to jp
	      // m to p is positive: if mTr and rTp agree (if both negative or positive)
	      // composing these two motions is the sum of mTr and rTp.
	      // returns are within range of - 0.5 to + 0.5 ==> Sum - 1.0 to 1.0
	      rTp += rf1DRegister (jpreal.begin(), jpreal.end(), real.begin(), real.end(), 1, pe);
	    }

	  rc2Dvector dt (rTp, mTr);
	  if (dt.isNull ()) 
	    continue;
	  rcRadian ad = dt.angle ();
	  rcAngle8 ab (ad);
	  bb.setAllPixels (ab.basic ());
	  cc.setAllPixels (127.0 * dt.l2 ());
	}
      //      cerr << endl;
    }

  //  cerr << "PipeLine: " << real.size() << endl;

}


////////////////////////////////////////////////////////////////////////////////
//
//	log2max
//
//	This function computes the ceiling of log2(n).  For example:
//
//	log2max(7) = 3
//	log2max(8) = 3
//	log2max(9) = 4
//
////////////////////////////////////////////////////////////////////////////////
static int32 log2max(int32 n)
{
  int32 power = 1;
  int32 k = 1;
  
  if (n==1) {
    return 0;
  }
	
  while ((k <<= 1) < n) {
    power++;
  }
	
  return power;
}

static void rfDrawCircle(rc2Dvector center, int32 r, rcWindow& dst, uint8 circleColor)
{
   for(int32 i=0;i< dst.width();i++)
   {
      for(int32 j=0;j< dst.height(); j++)
      {
         rc2Dvector pos(i+0.5,j+0.5);
         if (center.distance (pos) <= r) dst.setPixel (i,j,circleColor);
      }
   }
}

