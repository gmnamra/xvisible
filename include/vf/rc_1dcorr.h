/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.11  2006/01/01 21:30:58  arman
 *kinetoscope ut
 *
 *Revision 1.10  2005/08/30 20:57:15  arman
 *Cell Lineage
 *
 *Revision 1.10  2005/07/31 04:46:24  arman
 *fixed the warnings
 *
 *Revision 1.9  2005/04/13 02:39:56  arman
 *corrected AutoCorr length
 *
 *Revision 1.8  2005/04/12 22:19:25  arman
 *added deque<double> API to AutoCorr. Also fixed a bug with end iterator
 *
 *Revision 1.7  2005/03/15 01:29:16  arman
 *fixed a bug in AutoCorr
 *
 *Revision 1.6  2005/03/03 18:18:13  arman
 *added AutoCorr and AutoCross
 *
 *Revision 1.5  2005/01/24 16:39:41  arman
 *added 1dRegister
 *
 *Revision 1.4  2004/01/18 22:27:11  proberts
 *new muscle tracker and support code
 *
 *Revision 1.3  2004/01/11 15:17:19  arman
 *added ray processing
 *
 *Revision 1.2  2003/08/21 18:09:07  arman
 *added 1dSignal Intersection
 *
 *Revision 1.1  2002/11/23 20:39:28  arman
 *1D Correlation
 *Incremental Checkin
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rc1DCORR_H
#define __rc1DCORR_H

#include "rc_window.h"
#include "rc_vector2d.h"
#include "rc_pair.h"

#include  "rc_ncs.h"




#include "rc_fit.h"
#include "rc_stats.h"

// 1D correlation: Normalized Correlation
//
template <class Iterator>
double rf1DNormalizedCorr (Iterator Ib, Iterator Ie, Iterator Mb, Iterator Me)
{
  double sii(0.0), si(0.0), smm(0.0), sm(0.0), sim (0.0);
  int32 n;
  double rsq;

  n = Ie - Ib;
  rmAssert (n);
  rmAssert ((Me - Mb) == n);

  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  Iterator ip = Ib;
  Iterator mp = Mb;

  while (ip < Ie)
    {
      double iv (*ip);
      double mv (*mp);
      si += iv;
      sm += mv;
      sii += (iv * iv);
      smm += (mv * mv);
      sim += (iv * mv);

      ip++;
      mp++;
    }

  double cross = ((n * sim) - (si * sm));
  double energyA = ((n * sii) - (si * si));
  double energyB = ((n * smm) - (sm * sm));

  energyA = energyA * energyB;
  rsq = 0.;
  if (energyA != 0.)
    rsq = (cross * cross) / energyA;

  return rsq;
}


template <class Iterator>
double rf1DCrossCorr (Iterator Ib, Iterator Ie, Iterator Mb, Iterator Me)
{
  double sim (0.0);
  int32 n;

  n = Ie - Ib;
  rmAssert (n);
  rmAssert ((Me - Mb) == n);

  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  Iterator ip = Ib;
  Iterator mp = Mb;

  while (ip < Ie)
    {
      value_type iv (*ip);
      value_type mv (*mp);
      sim += (iv * mv);
      ip++;
      mp++;
    }

  return sim;
}


// 1D auto-correlation: Normalized Correlation
//
template <class Iterator>
void rf1DAutoCorr (Iterator Ib, Iterator Ie, Iterator acb, bool op = true)
{
  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  int32 n = distance (Ib, Ie);
  vector<value_type> ring (n + n);
  Iterator ip = Ib;
  int32 i;
  for (i = 0; i < n; i++) ring[i] = *ip++;
  ip = Ib;
  for (; i < (int32) ring.size(); i++) ring[i] = *ip++;

  ip = ring.begin();
  Iterator cp = acb;

  if (op)
    {
      for (int32 i = 0; i < n; i++, ip++)
	*cp++ = rf1DNormalizedCorr (Ib, Ie, ip, ip + n);
    }
  else
    {
      for (int32 i = 0; i < n; i++, ip++)
	*cp++ = rf1DCrossCorr (Ib, Ie, ip, ip + n);
    }    
}

template <class Iterator>
void rf1DAutoCorr (Iterator Ib, Iterator Ie, deque<double>& acb, bool op = true)
{
  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  int32 n = distance (Ib, Ie);
  vector<value_type> ring (n + n);
  Iterator ip = Ib;
  uint32 i;
  for (i = 0; i < (uint32) n; i++) ring[i] = *ip++;
  ip = Ib;
  for (; i < ring.size(); i++) ring[i] = *ip++;

  ip = ring.begin();
  acb.resize(n);
  deque<double>::iterator cp = acb.begin();

  if (op)
    {
      for (uint32 i = 0; i < (uint32) n; i++, ip++)
	*cp++ = rf1DNormalizedCorr (Ib, Ie, ip, ip + n);
    }
  else
    {
      for (uint32 i = 0; i < (uint32) n; i++, ip++)
	*cp++ = rf1DCrossCorr (Ib, Ie, ip, ip + n);
    }    
}
  
// 1D Signal Registration:
// @function rf1DRegister
// @description return best registration point of sliding model represented by Mb/Me on Ib/Ie
// Slide has to be greater or equal to 1. Mb is lined up with (Ib+slide) with 0 passed in for
// slide number of bins on the other end (and reduced count in calculation of correlation). Similarly
// (Mb+slide) is matched with Ib with first slide model bins treated as 0s and reduction of count accordingly
// @todo: constant time implementation

template <class Iterator>
double rf1D(Iterator Ib, Iterator Ie, Iterator Mb, Iterator Me, uint32 slide, double& pose)
{

  rmAssert (slide >= 1);
  vector<double> space (slide + slide + 1);
  vector<double>::iterator orgItr = space.begin() + slide;
  vector<double>::iterator slidItr;

  // Sliding I to the right
  slidItr = orgItr;
  for (uint32 i = 0; i < (slide + 1); i++)
    {
      *slidItr-- = rf1DNormalizedCorr (Ib + i, Ie, Mb, Me - i);
    }

  // Sliding I to the left (orgin already done)
  slidItr = orgItr+1;
  for (uint32 i = 1; i < (slide + 1); i++)
    {
      *slidItr++ = rf1DNormalizedCorr (Ib, Ie - i, Mb + i, Me);
    }

  vector<double>::iterator endd = space.end();
  advance (endd, -1); // The last guy

  vector<double>::iterator maxd = max_element (space.begin(), space.end());
  pose = *maxd;
  double alignment = (double) distance  (space.begin(), maxd);

  // If the middle is a peak, interpolate around it.
  if (!(space.size() == 1 || maxd == space.begin() || maxd == endd))
    {
      double left (*(maxd-1)), centre (*maxd), right (*(maxd+1));
      if (centre >= left && centre >= right)
	{
	  alignment += parabolicFit (left, centre, right, &pose);
	}
    }
  else
    {
      //      cerr << "Edge Condition" << endl;
      //      rfPRINT_STL_ELEMENTS (space);
    }

  return alignment;
  
}


template <class Iterator>
double rf1D(Iterator Ib, Iterator Ie, Iterator Mb, Iterator Me, uint32 slide, double& pose, double& acc)
{

	rmAssert (slide >= 1);
	vector<double> space (slide + slide + 1);
	vector<double>::iterator orgItr = space.begin() + slide;
	vector<double>::iterator slidItr;

// Sliding I to the right
	slidItr = orgItr;
	for (uint32 i = 0; i < (slide + 1); i++)
	{
		*slidItr-- = rf1DNormalizedCorr (Ib + i, Ie, Mb, Me - i);
	}

// Sliding I to the left (orgin already done)
	slidItr = orgItr+1;
	for (uint32 i = 1; i < (slide + 1); i++)
	{
		*slidItr++ = rf1DNormalizedCorr (Ib, Ie - i, Mb + i, Me);
	}

	vector<double>::iterator endd = space.end();
	advance (endd, -1); // The last guy

	vector<double>::iterator maxd = max_element (space.begin(), space.end());
	pose = *maxd;
	double alignment = (double) distance  (space.begin(), maxd);
	rcStatistics stat;
	vector<double>::iterator tmpIt = space.begin ();
	for (;tmpIt < space.end (); tmpIt++)
	{
		if (tmpIt != maxd) stat.add (*tmpIt);
	}
//	cerr << space << stat << endl;
	
// If the middle is a peak, interpolate around it.
	if (!(space.size() == 1 || maxd == space.begin() || maxd == endd))
	{
		double left (*(maxd-1)), centre (*maxd), right (*(maxd+1));
		acc = stat.mean ();
		if (centre >= left && centre >= right)
		{
			alignment += parabolicFit (left, centre, right, &pose);
		}
	}
	else
	{
	  //		cerr << "Edge Condition" << endl;
	  //		rfPRINT_STL_ELEMENTS (space);
	}

	return alignment;

}



template <class Iterator>
double rf1D(Iterator Ib, Iterator Ie, Iterator Mb, Iterator Me, 
		     uint32 slideLeft, uint32 slideRight, double& pose)
{
  uint32 slide = slideLeft + slideRight;
  rmAssert (slide >= 1);
  vector<double> space (slideLeft + slideRight + 1);
  vector<double>::iterator orgItr = space.begin() + slideRight;
  vector<double>::iterator slidItr;

  // Sliding I to the right
  slidItr = orgItr;
  for (uint32 i = 0; i < (slideRight + 1); i++)
    {
      *slidItr-- = rf1DNormalizedCorr (Ib + i, Ie, Mb, Me - i);
    }

  // Sliding I to the left (orgin already done)
  slidItr = orgItr+1;
  for (uint32 i = 1; i < (slideLeft + 1); i++)
    {
      *slidItr++ = rf1DNormalizedCorr (Ib, Ie - i, Mb + i, Me);
    }

  vector<double>::iterator endd = space.end();
  advance (endd, -1); // The last guy

  vector<double>::iterator maxd = max_element (space.begin(), space.end());
  pose = *maxd;
  double alignment = (double) distance  (space.begin(), maxd);

  // If the middle is a peak, interpolate around it.
  if (!(space.size() == 1 || maxd == space.begin() || maxd == endd))
    {
      double left (*(maxd-1)), centre (*maxd), right (*(maxd+1));
      if (centre >= left && centre >= right)
	{
	  alignment += parabolicFit (left, centre, right, &pose);
	}
    }
  else
    {
      //      cerr << "Edge Condition" << endl;
      //      rfPRINT_STL_ELEMENTS (space);
    }

  return alignment;
  
}



// 1D Histogram Intersection
//
template <class Iterator>
double rf1dSignalIntersection (Iterator Ib, Iterator Ie, Iterator Mb, Iterator Me)
{
  double sm(0.0), sim (0.0);
  int32 n;

  n = Ie - Ib;
  rmAssert (n);
  rmAssert ((Me - Mb) == n);

  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  Iterator ip = Ib;
  Iterator mp = Mb;

  while (ip < Ie)
    {
      value_type iv (*ip);
      value_type mv (*mp);
      sm += mv;
      sim += rmMin (iv, mv);
      ip++;
      mp++;
    }

  double cross = sim / sm;
  return cross;
}
  

// 1D Histogram Euclidean Distance
//
template <class Iterator>
double rf1dSignalNormEuclideanIntersection (Iterator Ib, Iterator Ie, Iterator Mb, Iterator Me)
{
  double sm(0.0), sim (0.0);
  int32 n;

  n = Ie - Ib;
  rmAssert (n);
  rmAssert ((Me - Mb) == n);

  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  Iterator ip = Ib;
  Iterator mp = Mb;

  while (ip < Ie)
    {
      value_type iv (*ip);
      value_type mv (*mp);
      sim += rmSquare (iv - mv);
      ip++;
      mp++;
    }

   return 1.0 - sim;
}

#ifdef NOTYET

class rcRayCorr
{
 public:
  rcRayCorr () : mIs (false) {}

  rcRayCorr (int32 dirQ, int32 r);
  // Initializes rays at dirQ (typically 8 or 16) directions

  int32 dirs () const;
  // Returns angle quanitization

  bool rayCorr (rcWindow& image, rcWindow& site, 
		rcFPair& org, int32 dir, rcIPair& start);
  // Runs a correlation around each ray point
  // Interpolates in the direction of the ray
  // Clears any existing results
  // Returns if it was successful
  // org is origin of the model as a fraction of width and height
  // interger pixel origin of the model corresponds to the 
  // ray positions. In other words model is traveling over the ray
  // anchored on its origin

  bool hasResults() const;
  // Returns True if there are currently valid results

  bool wasClipped () const;
  // Returns True if ray was clipped

  const rc2Fvector& position () const
  {
    return mBest;
  }
  // Returns a reference to the correlation max interpolated result

  const vector<float>& space () const
  {
    return mSpace;
  }
  // returns a reference to the correlation space

  const float &rayDiffusion () const
  {
    return mDiffusion;
  }
  // returns rms of correlation values

 private:
  float mDirs;
  float mR;
  int32 mIs;
  int32 mIsClipped;
  vector<float> mSpace;
  vector< vector<rcIPair> > mPathWays;
  rc2Fvector mBest;
  float mDiffusion;
};

#endif


#endif /* __rc1DCORR_H */
