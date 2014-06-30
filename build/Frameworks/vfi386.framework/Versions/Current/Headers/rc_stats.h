
#ifndef __rcSTATS_H
#define __rcSTATS_H


#include "rc_types.h"
#include "rc_limits.h"
#include <algorithm>
#include <vector>
#include <deque>
#include <numeric>

using namespace std;

/*! \fn 
  \brief compute median of a list. If length is odd it returns the center element in the sorted list, otherwise (i.e.
  * even length) it returns the average of the two center elements in the list
  \note the definition is identical to Mathematica
 * Partial_sort_copy copies the smallest Nelements from the range [first, last) to the range [result_first, result_first + N) 
  * where Nis the smaller of last - first and result_last - result_first . 
  * The elements in [result_first, result_first + N) will be in ascending order.
  */

template <class T>
double rfMedian (const vector<T>& a )
 {
   const int32 length =  a.size();
   if ( length == 0 )
       return 0.0;
   
   const bool is_odd = length % 2;
   const int array_length = (length / 2) + 1;
   vector<T> array (array_length);


 
   partial_sort_copy(a.begin(), a.end(), array.begin(), array.end());
 
   if (is_odd)
     return double(array [array_length-1]);
   else
     return double( (array [array_length - 2] + array [array_length-1]) / T(2) );
} 

template <class T>
double rfMedian (const deque<T>& a )
 {
   const int32 length =  a.size();
   if ( length == 0 )
       return 0.0;
   
   const bool is_odd = length % 2;
   const int array_length = (length / 2) + 1;
   deque<T> array (array_length);


 
   partial_sort_copy(a.begin(), a.end(), array.begin(), array.end());
 
   if (is_odd)
     return double(array [array_length-1]);
   else
     return double( (array [array_length - 2] + array [array_length-1]) / T(2) );
} 

template <class T>
double rfSum (const vector<T>& a )
 {
   const int32 length =  a.size();
   if ( length == 0 )
       return 0.0;
   return (double) accumulate (a.begin(), a.end(), T(0));
} 


template <class T>
double rfSum (const deque <T>& a )
{
    const int32 length =  a.size();
    if ( length == 0 )
        return 0.0;
    return (double) accumulate (a.begin(), a.end(), T(0));
}


template <class T>
double rfMean (const vector<T>& a )
 {
   const int32 length =  a.size();
   if ( length == 0 )
       return 0.0;

   double s = rfSum (a);
   return s / (double) a.size();
 } 



template <class T>
T rfRMS (const vector<T>& a)
{
  rmAssert (a.size() > 1);

  // These typename definitions silence gcc compiler warnings
  typedef typename vector<T>::iterator iterator;
  typedef typename vector<T>::const_iterator const_iterator;

  const_iterator accPtr = a.begin();

  T sofar ( (T) 0);
  int32 i (0);
  for (; accPtr < a.end(); accPtr++, i++)
    sofar += rmSquare (*accPtr);
  return (T) (sqrt (sofar / (T) i));
}

template <class T>
void rfRMS (const vector<T>& a, vector<T>& b)
{
  if (a.size() < 1) return;
  rmAssert (a.size() == b.size());

  // These typename definitions silence gcc compiler warnings
  typedef typename vector<T>::iterator iterator;
  typedef typename vector<T>::const_iterator const_iterator;

  iterator fs = b.begin();
  const_iterator accPtr = a.begin();

  T sofar ( (T) 0);
  int32 i (1);
  for (; accPtr < a.end(); fs++, accPtr++, i++)
    {
      sofar += rmSquare (*accPtr);
      *fs = sqrt (sofar / (T) i);
    }
}

template <class T>
void rfcSum (const deque<T>& a, deque <T>& b)
{
    if (a.size() < 1) return;
    rmAssert (a.size() == b.size());

        // These typename definitions silence gcc compiler warnings
    typedef typename deque<T>::iterator iterator;
    typedef typename deque<T>::const_iterator const_iterator;

    iterator fs = b.begin();
    const_iterator accPtr = a.begin();

    T sofar ( (T) 0);
    int32 i (1);
    for (; accPtr < a.end(); fs++, accPtr++, i++)
    {
        sofar += (*accPtr);
        *fs = sofar;
    }
}


template <class T>
void rfProductOfrValues (const vector<T>& a, vector<T>& b)
{
 if (a.size() < 1) return;
  rmAssert (a.size() == b.size());

  // These typename definitions silence gcc compiler warnings
  typedef typename vector<T>::iterator iterator;
  typedef typename vector<T>::const_iterator const_iterator;

  iterator fs = b.begin();
  const_iterator accPtr = a.begin();

  T sofar ( (T) 1);
  int32 i (1);
  for (; accPtr < a.end(); fs++, accPtr++, i++)
    {
      sofar *= (*accPtr);
      *fs = sofar;
    }
}


template <class T>
void rfRevRMS (const vector<T>& a, vector<T>& b)
{
  if (a.size() < 1) return;
  rmAssert (a.size() == b.size());

  // These typename definitions silence gcc compiler warnings
  typedef typename vector<T>::iterator iterator;
  typedef typename vector<T>::const_reverse_iterator const_iterator;

  iterator fs = b.begin();
  const_iterator accPtr = a.rbegin();

  T sofar ( (T) 0);
  int32 i (1);
  for (; accPtr < a.rend(); fs++, accPtr++, i++)
    {
      sofar += rmSquare (*accPtr);
      *fs = sqrt (sofar / (T) i);
    }
}
  

class rcStatistics
{

public:


   rcStatistics ()
   {
     reset ();
   }

   // effect	Construct a rcStatistics object.



   ~rcStatistics () {};



   // Default copy and assignment are ok.



   void reset ()
   {
     mN = 0;
     mSum = 0;
     mSumsq = 0;
     mLast = 0;
     mMin = rcDBL_MAX;
     mMax = rcDBL_MIN;
   }

   // effect	Removes all values from the collection.



   void add (double val)
   {
     mN++;
     mLast = val;
     mSum += val;
     mSumsq += val * val;
     if (val < mMin) mMin = val;
     if (val > mMax) mMax = val;
   }

   // effect	Add a new value to the collection.



   uint32 n () const { return mN; };

   // effect	Return the number of data items added so far.



   double last () const { return mLast; };

   // effect	Return the last value added.



   double mean () const { return (mN) ? (mSum / mN) : 0.0; };

   // effect 	Return the mean.



   double min () const { assert(mN); return mMin; };

   // effect	Return the mininum value added.

   // requires  mN > 0



   double max () const { assert(mN); return mMax; };

   // effect	Return the maximum value added.

   // requires  mN > 0



   double variance () const

     { return (mN > 1) ? (mSumsq - mSum * mean()) / (mN-1) : 0; };

   // effect	Return the variance for the sample population.

   double rms () const

  { return (mN > 0) ? sqrt (mSumsq / mN) : 0; };

   // effect	Return the variance for the sample population.


   double stdDev () const

     { return sqrt(variance()); };

   // effect	Return the standard deviation for the sample population.

   friend ostream& operator<< (ostream& ous, const rcStatistics& rc)
   {
       ous << "Min (" << rc.min() << ")" << endl;
       ous << "Max (" << rc.max() <<  ")" << endl;
       ous << "Mean (" << rc.mean() << ")" << endl;
       ous << "Stdev (" << rc.stdDev() << ")" << endl;

       return ous;
   }



private:

   uint32 mN;   	// number of items in collection

   double mLast;	// last value added

   double mSum;		// sum of values

   double mSumsq;	// sum of squared values

   double mMin;		// min

   double mMax;		// max

};


#endif /* __rcSTATS_H */
