/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/05/08 13:15:51  arman
 *initial version
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_SIGNALFOLD_H
#define __RC_SIGNALFOLD_H

#include "rc_sample.h>
#include "rc_fixedarray.h>
#include "rc_listhistogram.h>

/*!
    @class rcSignalFold
    @Periodic Signal Recognition Class
    @discussion 
    @throws exception when not initialized
*/

class rcSignalFold
{
public:

  typedef rcListSample<rc2Fmeasurement> samples;
  typedef float binType;
  typedef rcListSampleToHistogramGenerator< samples, binType> histo;

  enum unit
  {
    eIndex,
    eLength,
    eDefault = eIndex
  };

/*! 
  @Default Constructor
*/
  rcSignalFold ();

/*! 
  @function Constructor
  @discussion Initializes from a vector of values. Default "x-axis" is array indices.
  @discussion: The 2nd form supplies "x-axis" of time stamps, etc.
*/
  rcSignalFold ();
  template<class M>
  rcSignalFold (vector<M>&);
  rcSignalFold (vector<M>&, vector<M>&);

  rcSignalFold (deque<M>&);
  rcSignalFold (deque<M>&, deque<M>&);

/*! 
  @function periodicity
  @discussion Returns periodicity of the signal in two forms:
  @discussion one is fraction of the signal length ("duration"),
  @discussion and length or duration of the period
*/
  double period (unit) const;
  double periodicity (unit) const;

/*! 
  @function phase
  @discussion Returns phase of the periodic signal in length 
*/
  double phase (unit) const;

private:
  samples::Pointer mSamplesPtr;
  void size(uint32);
};

rcSignalFold::size (rcUInt32 sz)
{
  mSamplesPtr = samples::New ();
  mSamplesPtr->Resize (sz);
}

template<class M>
rcSignalFold::rcSignalFold (vector<M>& signal)
{
  rmAssert (signal.size());
  size (signal.size());

  for (rcUInt32 i = 0; i < signal.size(); i++)
    {
      rc2Fmeasurement mv;
      mv[0] = (float) i; mv[1] = (float) signal[i];
      mSamplesPtr->PushBack (mv);
    }
}

template<class M>
rcSignalFold::rcSignalFold (deque<M>& signal)
{
  rmAssert (signal.size());
  size (signal.size());

  for (rcUInt32 i = 0; i < signal.size(); i++)
    {
      rc2Fmeasurement mv;
      mv[0] = (float) i; mv[1] = (float) signal[i];
      mSamplesPtr->PushBack (mv);
    }
}

template<class M>
rcSignalFold::rcSignalFold (vector<M>& signal, vector<M>& stamp)
{
  rmAssert (signal.size());
  rmAssert (signal.size() == stamp.size());
  size (signal.size());

  for (rcUInt32 i = 0; i < signal.size(); i++)
    {
      rc2Fmeasurement mv;
      mv[0] = (float) stamp[i]; mv[1] = (float) signal[i];
      mSamplesPtr->PushBack (mv);
    }
}

template<class M>
rcSignalFold::rcSignalFold (deque<M>& signal, deque<M>& stamp)
{
  rmAssert (signal.size());
  rmAssert (signal.size() == stamp.size());
  size (signal.size());

  for (rcUInt32 i = 0; i < signal.size(); i++)
    {
      rc2Fmeasurement mv;
      mv[0] = (float) stamp[i]; mv[1] = (float) signal[i];
      mSamplesPtr->PushBack (mv);
    }
}

#endif /* __RC_SIGNALFOLD_H */


