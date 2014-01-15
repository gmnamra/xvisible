/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.21  2006/01/10 23:43:23  arman
 *pre-rel2
 *
 *Revision 1.20  2005/10/31 11:44:22  arman
 *added checking against nan
 *
 *Revision 1.19  2005/07/01 21:05:40  arman
 *version2.0p
 *
 *Revision 1.19  2005/05/25 20:16:52  arman
 *added rmSigmoid
 *
 *Revision 1.18  2005/03/08 22:11:59  arman
 *added double specialization
 *
 *Revision 1.17  2005/03/01 15:07:46  arman
 *another null option
 *
 *Revision 1.16  2005/03/01 06:26:49  arman
 *added default null pointer arg for parabolic functions
 *
 *Revision 1.15  2004/09/18 01:16:18  arman
 *fixed incorrect y axis in 2dr
 *
 *Revision 1.14  2004/09/13 15:05:03  arman
 *removed unnecessary inclusion
 *
 *Revision 1.13  2004/08/27 22:17:47  arman
 *if parabolic fit returns outside, return no results
 *
 *Revision 1.12  2004/08/26 13:36:12  arman
 *added direct par fit function
 *
 *Revision 1.11  2004/08/24 21:39:11  arman
 *no mass no fit
 *
 *Revision 1.10  2004/08/23 10:39:06  arman
 **** empty log message ***
 *
 *Revision 1.9  2004/08/16 10:43:39  arman
 *templatized rfMoment
 *
 *Revision 1.8  2004/08/13 20:55:54  arman
 *added rfInterpolatePeak
 *
 *Revision 1.7  2004/08/09 15:21:17  arman
 *locals changed to double
 *
 *Revision 1.6  2004/07/08 13:32:56  arman
 *added nonPeak version of parabolicPeak
 *
 *Revision 1.5  2004/01/20 16:01:55  arman
 *added quadratic support
 *
 *Revision 1.4  2004/01/14 19:44:29  arman
 *added float overload
 *
 *Revision 1.3  2003/12/31 14:11:47  arman
 *added rfMomentFit. changed par api to vector
 *
 *Revision 1.2  2003/12/30 21:54:43  arman
 *fixed a bug in y coordinate (it is assumed to be in TV coords)
 *
 *Revision 1.1  2003/10/24 16:21:01  arman
 *fiting support. 1D and 2D radius of 3 parabolic
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcFIT_H
#define __rcFIT_H

#include <rc_vector2d.h>
#include<vector>

#define rmSigmoid(a) (1.0/(1.0+exp(-(double) a)))

// Solve a quadratic at unit distance and solve for position of max.
// Coordinate System assumption is left -> right is positive 

template<class T>
inline T parabolicFit ( const T left, const T center, const T right, T *maxp = NULL)
{
  const bool cl (center == left);
  const bool cr (center == right);

  if ((!(center > left || cl ) && !(center > right || cr )) &&
      (!(center < left || cl ) && !(center < right || cr )))
    rmExceptionMacro (<<" Invalid Data " << left << "," << 
		      center << "," << right);

  const T s = left + right;
  const T d = right - left;
  const T c = center + center;

  // case: center is at max
  if (c == s)
    {
      if (maxp) *maxp = center;
      return 0;
    }

  // ymax for the quadratic
  if (maxp) *maxp = (4 * c * c - 4 * c * s + d * d) / (8 * (c - s));

  // position of the max
  return d / (2 * (c - s));
   
}

template<> 
inline double parabolicFit (const double left, const double center, const double right, double *maxp)
{
  const bool cl (real_equal (center - left, (double) (1.e-15)));
  const bool cr (real_equal (center - right, (double) (1.e-15)));

  if ((!(center > left || cl ) && !(center > right || cr )) &&
      (!(center < left || cl ) && !(center < right || cr )))
    rmExceptionMacro (<<" Invalid Data " << left << "," << 
		      center << "," << right);

  const double s = left + right;
  const double d = right - left;
  const double c = center + center;

  // case: center is at max
  if (c == s)
    {
      if (maxp) *maxp = center;
      return 0;
    }

  // ymax for the quadratic
  if (maxp) *maxp = (4 * c * c - 4 * c * s + d * d) / (8 * (c - s));

  // position of the max
  return d / (2 * (c - s));
   
}


template<> 
inline float parabolicFit (const float left, const float center, const float right, float *maxp)
{
  const bool cl (real_equal (center - left, (float) (1.e-10)));
  const bool cr (real_equal (center - right, (float) (1.e-10)));

  if ((!(center > left || cl ) && !(center > right || cr )) &&
      (!(center < left || cl ) && !(center < right || cr )))
    rmExceptionMacro (<<" Invalid Data " << left << "," << 
		      center << "," << right);

  const float s = left + right;
  const float d = right - left;
  const float c = center + center;

  // case: center is at max
  if (c == s)
    {
      if (maxp) *maxp = center;
      return 0;
    }

  // ymax for the quadratic
  if (maxp) *maxp = (4 * c * c - 4 * c * s + d * d) / (8 * (c - s));

  // position of the max
  return d / (2 * (c - s));
   
}
  
template<class T>
inline T rfParabolicFitNonPeak ( const T left, const T center, const T right, T *maxp = NULL )
{
  const T s = left + right;
  const T d = right - left;
  const T c = center + center;

  // case: center is at max
  if (c == s)
    {
      if (maxp) *maxp = center;
      return 0;
    }

  // ymax for the quadratic
  if (maxp) *maxp = (4 * c * c - 4 * c * s + d * d) / (8 * (c - s));

  // position of the max
  return d / (2 * (c - s));
   
}

template<class T, class M>
inline void parfit2DR3 (const vector<T>& score, rc2dVector<M>& v)
{

  M MULTA (0.408248);
  M MULTB (0.816497);

  M a = MULTA * (MULTA * (M) (score[6]) - MULTB * (M) (score[7]) + MULTA * (M) (score[8]) + MULTA * (M) (score[3]) - 
          MULTB * (M) (score[4]) + MULTA * (M) (score[5]) + MULTA * (M) (score[0]) - MULTB * (M) (score[1]) + 
		  MULTA * (M) (score[2]));
  M b = MULTA * (MULTA * (M) (score[6]) + MULTA * (M) (score[7]) + MULTA * (M) (score[8]) - MULTB * (M) (score[3]) - 
          MULTB * (M) (score[4]) - MULTB  * (M) (score[5]) + MULTA * (M) (score[0]) + MULTA * (M) (score[1]) + 
			MULTA * (M) (score[2]));
  M c = 0.5 * (-0.5 * (M) (score[6]) + 0.5 * (M) (score[8]) + 0.5 * (M) (score[0]) - 0.5 * (M) (score[2]));
  M d = MULTA * (-MULTA * (M) (score[6]) + MULTA * (M) (score[8]) - MULTA * (M) (score[3]) + MULTA * (M) (score[5]) - 
		  MULTA * (M) (score[0]) + MULTA * (M) (score[2]));
  M e = MULTA * (MULTA * (M) (score[6]) + MULTA * (M) (score[7]) + MULTA * (M) (score[8]) - MULTA * (M) (score[0]) - 
		  MULTA * (M) (score[1]) - MULTA * (M) (score[2]));

  M f = c * c - 4 * a * b;
  if (f == M(0))
    {
      v = rc2dVector<M> ();
      return;
    }

  v.x((2 * b * d - c * e) / f);
  v.y((2 * a * e - c * d) / f);

  if (v.x() > 1.0 || v.x() < -1.0 || v.y() > 1.0 || v.y() < -1.0)
    v = rc2dVector<M> ();
}


template<class T, class M>
  inline void parfit2DR3 (const T& score11,
			  const T& score12,
			  const T& score13,
			  const T& score21,
			  const T& score22,
			  const T& score23,
			  const T& score31,
			  const T& score32,
			  const T& score33, 
			  rc2dVector<M>& v)
{

  M MULTA (0.408248);
  M MULTB (0.816497);

  M a = MULTA * (MULTA * score31 - MULTB * score32 + MULTA * score33 + MULTA * score21 - 
          MULTB * score22 + MULTA * score23 + MULTA * score11 - MULTB * score12 + 
		  MULTA * score13);
  M b = MULTA * (MULTA * score31 + MULTA * score32 + MULTA * score33 - MULTB * score21 - 
          MULTB * score22 - MULTB  * score23 + MULTA * score11 + MULTA * score12 + 
			MULTA * score13);
  M c = 0.5 * (-0.5 * score31 + 0.5 * score33 + 0.5 * score11 - 0.5 * score13);
  M d = MULTA * (-MULTA * score31 + MULTA * score33 - MULTA * score21 + MULTA * score23 - 
		  MULTA * score11 + MULTA * score13);
  M e = MULTA * (MULTA * score31 + MULTA * score32 + MULTA * score33 - MULTA * score11 - 
		  MULTA * score12 - MULTA * score13);

  M f = c * c - 4 * a * b;
  if (f == M(0))
    {
      v = rc2dVector<M> ();
      return;
    }

  v.x((2 * b * d - c * e) / f);
  v.y((2 * a * e - c * d) / f);

  if (v.x() > 1.0 || v.x() < -1.0 || v.y() > 1.0 || v.y() < -1.0)
    v = rc2dVector<M> ();
}



inline void rfParfit2DR3 (const vector<double>& score, rc2Dvector& v)
{
  return parfit2DR3 (score, v);
}


inline void rfParfit2DR3 (const vector<float>& score, rc2Fvector& v)
{
  return parfit2DR3 (score, v);
}

inline void rfParfit2DR3 (const float& score11,
			  const float& score12,
			  const float& score13,
			  const float& score21,
			  const float& score22,
			  const float& score23,
			  const float& score31,
			  const float& score32,
			  const float& score33, 
			  rc2Fvector& v)
{
  return parfit2DR3 ( score11,
			   score12,
			   score13,
			   score21,
			   score22,
			   score23,
			   score31,
			   score32,
		      score33, v);
}


inline double rfParabolicFit (const vector<double>& space)
{
  double *d (0);
  return parabolicFit ( space[0], space[1], space[2], d);
}


inline double rfParabolicFit (const vector<double>& space, double *dmax)
{
  return parabolicFit ( space[0], space[1], space[2], dmax);
}


inline float rfParabolicFit (const vector<float>& space)
{
  static float *f(0);
  return parabolicFit ( space[0], space[1], space[2], f);
}

template<class T>
inline void rfMomentFit (const vector<T>& first, T& x, T& y)
{
  T sx (0), sy (0);
  rmAssert (first.size() == 9);
  sx += -first[0] + first[2] - first[3] + first[5] - first[6] + first[8];
  sy += -first[0] + first[6] - first[1] + first[7] - first[2] + first[8];
  x = sx / first.size();
  y = sy / first.size();
}

  /*! 
    @function rcMomentFit
    @discussion :Return interpolated positions wrt center of 
    @discussion :in the 1D case and top of the search space 
    @discussion :in case of the 2D.
   */

template<class T>
inline void rfMomentFit (const vector< vector<T> >& space, T& x, T& y)
{
  T sx (0), sy (0), mass (0);

  // make sure we are equal sized and non-zero
  uint32 s = space[0].size();
  rmAssert (s);
  for (uint32 j = 0; j < space.size(); j++)
    rmAssert (space[j].size() == s);

  for (uint32 j = 0; j < space.size(); j++)  
    for (uint32 i = 0; i < s; i++)
      {
	T val = space[j][i];
	mass += val;
	sx += i * val; sy +=  j * val;
      }

  if (mass > T(0))
    x = sx / mass; y = sy / mass;
}



template<class P>
float rfInterpolatePeak(const vector<P>& signal, uint32 index, P* ampP = NULL)
{
  float delta, damp, *dampPtr (NULL);
  if (ampP) dampPtr = &damp;

  if (index == (signal.size()-1))  { // Peak is pegged at end
    delta = parabolicFit((float) signal[index-1], 
			 (float) signal[index], 0.0f, dampPtr);
    if (delta > 0.0)
      rmExceptionMacro (<<" Invalid Data " << (float) signal[index-1] << "," << 
			(float) signal[index]);


  }
  else if (index == 0) { // Peak is pegged at start
    delta = parabolicFit(0.0f, (float) signal[0], (float) signal[1], dampPtr);
    if (delta < 0.0)
      rmExceptionMacro (<<" Invalid Data " <<  (float) signal[0] << "," << 
			(float) signal[1]);

  }
  else { // Interpolate peak location
    delta = parabolicFit((float) signal[ index-1], (float) signal[index], 
			 (float) signal[index+1], dampPtr);
  }

  if (ampP)
    {
      P amp;
      amp = damp;
      * ((P *) ampP) = amp;
    }

  return index + delta;
}

      

// Statistical Distribution Support from NR (in rc_units.h)
double rfGasDev(int32 &idum);
double rfRanOne(int32 &idum);

    

// Motion Vector Classification
//
#if 0

template <class T>
class rcMvCluster
{
 public:

  rcMvCluster (uint32 size);

  // Returns rms error
  T update (rc2dvector<T>& xy, rc2dvector<T>& uv)
    {
      T& x(xy.x()), y(xy.y()), u(uv.x()), v(uv.y());
      
    }

 private:

};



vector<float> rfCBLDistance (vector<rc2Fvector>& edges)
{
   //
   const float zero (0.0);
   int32 n (edges.size());
   vector<float> dmap (n, zero);

   for (int32 j = 0; j < n; j++)
   {
      for (int32 i = j + 1; i < n; i++)
      {
	float dist = edges[i].distance (edges[j]);
	dist *= dist;
	dmap[i] += log (dist);
	dmap[j] += log (dist);
      }
   }

   return dmap;
}

#endif

#endif /* __rcFIT_H */
