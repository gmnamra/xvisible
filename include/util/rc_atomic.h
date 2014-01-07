// Copyright 2002 Reify, Inc.

#ifndef _rcATOMIC_H_
#define _rcATOMIC_H_

#if defined (_USE_NATIVE_ATOMIC_)
#include "assert.h"
#include "string.h"
#include <rc_assemblyfcts.h>
#else
#include <boost/atomic.hpp>
#endif

#if defined (_USE_NATIVE_ATOMIC_)

// Defines allow unit test code to insert
#ifdef UT_INSERT_WRITE
#define UT_GET_DATA_PTR       UT_HELP_GET_DATA_PTR
#define UT_INSERT_RACE        UT_HELP_INSERT_RACE
#define UT_INSERT_RACE_N_CODE UT_HELP_INSERT_RACE_N_CODE
#else
#define UT_GET_DATA_PTR
#define UT_INSERT_RACE
#define UT_INSERT_RACE_N_CODE
#endif

/*
 *     copy/comparison operations are required (and the likelyhood
 *     of a read/write collision grows as well).
 *
 * The interface consists of the following public functions:
 *
 *   rcAtomicValue<T>(const T& v)                 - Init object to value of v
 *
 *   rcAtomicValue<T>(const rcAtomicValue<T>& at) - Init object from another
 *                                                  rcAtomicValue<T>
 *
 *   T& rcAtomicValue<T>::getValue(T& v) const    - Store the current value in v
 *                                                  and return a reference to v
 *
 *   void rcAtomicValue<T>::setValue(const T& v)  - Set the value of the object
 *                                                  to v
 */

/* rcAtomicValue - Allows the creation of data types that can safely be
 *                 read "atomically" by one or more "consumer threads"
 *                 without concern that a "producer thread" is
 *                 simultaneously modifying the same data. Since no
 *                 mutexes are used, the caller is guaranteed never to
 *                 be blocked.
 *
 * The following restrictions apply:
 *
 *  1) T must work properly with the default copy constructor and
 *     assignment operator. See 3) for a feel of why this requried.
 *
 *  2) There must be only one "producer thread".
 *
 *  3) T should not be "too large". What's too large? I'll describe a
 *     little of how rcAtomicValue works and let you decide for yourself:
 *
 *     rcAtomicValue based objects store 4 copies of the T object
 *     internally.  When a call to getValue() is made, all four values
 *     are read into local stack variables and then compared with each
 *     other. If two copies are found that match, and some other
 *     conditions are met, a valid copy of the underlying T object is
 *     deemed to have been found. Otherwise, the process repeats. A
 *     final copy is done to pass a version of the current value of
 *     the T object back to the caller.
 *
 *     As you can see, as the size of T grows, a large number of
 *     copy/comparison operations are required (and the likelyhood
 *     of a read/write collision grows as well).
 *
 * The interface consists of the following public functions:
 *
 *   rcAtomicValue<T>(const T& v)                 - Init object to value of v
 *
 *   rcAtomicValue<T>(const rcAtomicValue<T>& at) - Init object from another
 *                                                  rcAtomicValue<T>
 *
 *   T& rcAtomicValue<T>::getValue(T& v) const    - Store the current value in v
 *                                                  and return a reference to v
 *
 *   void rcAtomicValue<T>::setValue(const T& v)  - Set the value of the object
 *                                                  to v
 */
template <class T>
class rcAtomicValue
{
public:
	enum {is_boost = 0 };
	
  rcAtomicValue(const T& v) : _counter (0)
  {
    setValue(v); // Initializes all member data
  }
	
  rcAtomicValue(const rcAtomicValue<T>& av) : _counter (0)
  {
    T temp;
		
    setValue(av.getValue(temp));
  }
	
  T& getValue(T& v) const
  {
    unsigned char local_v0[2][sizeof(T)];
    unsigned char local_v1[2][sizeof(T)];
		
    UT_GET_DATA_PTR;
    /* Note: In the following comments the terms "stable" and
     * "matching" are used. By "stable", I mean: "the value, when read
     * in, is not partially the old value and partially the new
     * value". By "matching", I mean: "a binary comparison of the two
     * values are identical".
     */
    /* Keep looping until we get a stable, matching pair of
     * values. Note that all reads (and writes) are done through
     * function calls to guarantee that the compiler doesn't try to
     * reorder any of them.
     */
    for (;;)
    {
      UT_INSERT_RACE;
      unsigned long beforeCtr = getCounter();
			
      /* Read values in order opposite of that used to do writes in
       * setValue().
       */
      copyValue((char*)local_v0[0], (volatile char*)_v0[0]);
      copyValue((char*)local_v0[1], (volatile char*)_v0[1]);
      copyValue((char*)local_v1[0], (volatile char*)_v1[0]);
      copyValue((char*)local_v1[1], (volatile char*)_v1[1]);
			
      /* If the counter has changed, 2 or more calls to setValue() may
       * have occurred during this call. This is at least one more
       * than we can safely handle. Try again.
       */
      UT_INSERT_RACE;
      unsigned long afterCtr = getCounter();
      if (beforeCtr != afterCtr)
				continue;
			
      /* If the counter hasn't changed then at most 1 call to
       * setValue() has occurred during this call. If setValue()
       * hasn't been called during this call, one of two things has
       * happened:
       *
       *  1) Both _v0[] and _v1[] have stable, matching data.
       *
       *  2) Some thread was preempted in the middle of a call to
       *     setValue() and it is still waiting to run again. In
       *     this case, either _v0[] or _v1[] may contain unstable
       *     values, but not both.
       *
       * In either case, a stable, matching pair of values is
       * available.
       *
       * Now consider the case where a call to setValue() was
       * actively writing _v0[] and _v1[] during our reads of the
       * same values. Because these reads were done in the opposite
       * order as the writes were done in setValue(), at most one
       * of the values read in has an unstable value. As proof,
       * consider the following:
       *
       *  1) For a value to be unstable, it must be in the process
       *     of being read while writes are occuring.
       *
       *  2) If the first unstable read is to be _v0[0], then
       *     setValue() must have already written _v0[1], _v1[0],
       *     _v1[1]. The subsequent reads of these must yield stable
       *     values.
       *
       *  3) If the first unstable read is to be _v0[1], then
       *     setValue() must have already written _v1[0], _v1[1].
       *     Moreover, the write to _v0[0] cannot have occurred yet,
       *     so the read of that value must have been stable.
       *
       *  4) If the first unstable read is to be _v1[0], then
       *     setValue() must have already written _v1[1]. Moreover,
       *     the writes to _v0[0], _v0[1] cannot have occurred yet,
       *     so the reads of those values must have been stable.
       *
       *  5) If the first unstable read is to be _v1[1], then the
       *     writes to _v0[0], _v0[1], _v1[0] cannot have occurred
       *     yet, so the reads of those values must have been stable.
       *
       *  6) From inspection of 2) through 5) it is clear that one
       *     of _v0[] or _v1[] is a stable, matching pair.
       */
      if (equalValue((char*)local_v0[0], (char*)local_v0[1]))
      {
        copyValue((char*)&v, (char*)local_v0[0]);
				return v;
      }
      else if (equalValue((char*)local_v1[0], (char*)local_v1[1]))
      {
        copyValue((char*)&v, (char*)local_v1[0]);
				return v;
      }
			
      /* If, as claimed, a stable, matching pair is available
       * this code should never be reached.
       */
      assert(0);
    }
		
    return v; // Make compiler happy
  }
	
  void setValue(const T& v)
  {
    /* Note that all writes are done through function calls to
     * guarantee that the compiler doesn't try to reorder any of them.
     */
    /* Note: In the following comments the terms "stable" and
     * "matching" are used. By "stable", I mean: "the value, when read
     * in, is not partially the old value and partially the new
     * value". By "matching", I mean: "a binary comparison of the two
     * values are identical".
     */
    /* If the calling thread is preempted at some point during these
     * two writes, _v0[] still has stable, matching values.
     */
    copyValue((volatile char*)_v1[1], (char*)&v);
    genSync();
    copyValue((volatile char*)_v1[0], (char*)&v);
    genSync();
		
    /* If the calling thread is preempted at some point during these
     * two writes, _v1[] still has stable, matching values.
     */
    copyValue((volatile char*)_v0[1], (char*)&v);
    genSync();
    copyValue((volatile char*)_v0[0], (char*)&v);
    genSync();
		
    /* Tells getValue() this function has been called.
     */
    incCounter();
  }
	
private:
	
  volatile unsigned char _v0[2][sizeof(T)];
  volatile unsigned char _v1[2][sizeof(T)];
  volatile unsigned long _counter;
	
  static void copyValue(volatile char* vDest, const volatile char* vSrc)
  {
    UT_INSERT_RACE_N_CODE;
		
    if (sizeof(T) == sizeof(int))
      *(volatile int*)((void*)vDest) = *(volatile int*)((void*)vSrc);
    else if (sizeof(T) == sizeof(short))
      *(volatile short*)((void*)vDest) = *(volatile short*)((void*)vSrc);
    else if (sizeof(T) == sizeof(char))
      *(volatile char*)((void*)vDest) = *(volatile char*)((void*)vSrc);
    else
      memcpy((void*)vDest, (void*)vSrc, sizeof(T));
  }
	
  static bool equalValue(const char* vA, const char* vB)
  {
    if (sizeof(T) == sizeof(int))
      return *(int*)((void*)vA) == *(int*)((void*)vB);
    else if (sizeof(T) == sizeof(short))
      return *(short*)((void*)vA) == *(short*)((void*)vB);
    else if (sizeof(T) == sizeof(char))
      return *(char*)((void*)vA) == *(char*)((void*)vB);
		
    return memcmp((void*)vA, (void*)vB, sizeof(T)) == 0;
  }
	
  void incCounter()
  {
    _counter = _counter + 1;
    genSync();
  }
	
  unsigned long getCounter() const
  {  
    return _counter;
  }
};


#else // Use boost::atomic

using namespace boost;

template <typename T>
class rcAtomicValue : public boost::atomic<T>
{
public:
	
	enum {is_boost = 1 };
	
	typedef boost::atomic<T> super;
	
  explicit rcAtomicValue(const T& v)
  {
    setValue(v); // Initializes all member data
  }
	
	rcAtomicValue(const rcAtomicValue<T>& av)
  {
		T temp;
    setValue(av.getValue (temp));
  }

	rcAtomicValue& operator= (rcAtomicValue& other)
	{
		T temp;		
		setValue(other.getValue (temp) );
	}

	
  T& getValue(T& v) const
	{
		v = super::load ();
		return v;
  }
	
  void setValue(const T& v)
  {
		super::store (v);
  }
private:

};


#endif

#endif // _rcATOMIC_H_
