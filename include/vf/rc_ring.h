// Copyright 2002 Reify, Inc.

#ifndef _rcRING_H_
#define _rcRING_H_

#include "assert.h"
#include <vector>
#include "rc_atomic.h"
#include "rc_types.h"

typedef uint8* (*sMemAllocator)(long);

/* rcRingBuffer - Thread-safe implementation of ring buffer. Thread-safety
 *                is based upon the use of atomicValue type integers for
 *                head and tail indices. A ring of pointers to T is
 *                created. The following restrictions apply:
 *
 *  1) There should be exactly one consumer thread (thread calling
 *     get()) and exactly one producer thread (thread calling put()).
 *
 * The interface consists of the following public functions:
 *
 *   rcRingBuffer<T>(uint32 sz)  - Construct empty ring buffer with sz free slots
 *
 *   T* getValue()                 - Get next value from ring buffer
 *
 *   bool putValue(T*)             - Store next value in ring buffer
 *
 *   size_t size()                 - Number of slots in ring
 *
 *   uint32 availValues()        - Number of values available to be read
 *
 *   uint32 availFreeSlots()     - Number of free slots available to put values in
 */
template <class T>
class rcRingBuffer
{
public:

  rcRingBuffer(uint32 sz) : _sz(sz+1), _specAlloc(0), _head(0), _tail(0)
  {
    /* Note: It was necessary to allocate an extra element in the
     * ring, since the code assumes (head == tail) means the ring is
     * empty and we don't want to maintain a counter to distinguish
     * between the two cases.
     */
    assert(_sz != 0); // Paranoia reigns supreme
    _ring = (T** volatile)malloc(_sz*sizeof(T*));
    assert(_ring != 0);
  }

  rcRingBuffer(uint32 sz, sMemAllocator allocFct ) : _sz(sz+1), _specAlloc(1), _head(0), _tail(0)
  {
    /* Note: It was necessary to allocate an extra element in the
     * ring, since the code assumes (head == tail) means the ring is
     * empty and we don't want to maintain a counter to distinguish
     * between the two cases.
     */
    assert(_sz != 0); // Paranoia reigns supreme
    assert(allocFct);

    _ring = (T** volatile) allocFct (_sz*sizeof(T*));
    assert(_ring != 0);
  }

  ~rcRingBuffer()
  {
    assert(_ring);

    if (!_specAlloc)
      free((void*)_ring);

    _ring = 0;
  }

  T* getValue(void)
  {
    uint32 readIndex, temp;

    if (_head.getValue(readIndex) == _tail.getValue(temp))
      return 0;

    // Read value from ring
    //
    T* retVal = _ring[readIndex];

    // Then update head pointer
    //
    if (++readIndex == _sz)
      readIndex = 0;

    _head.setValue(readIndex);

    return retVal;
  }

  bool putValue(T* tp)
  {
    uint32 writeIndex, headIndex, newIndex;

    _head.getValue(headIndex);
    newIndex = _tail.getValue(writeIndex) + 1;

    if (newIndex == _sz)
      newIndex = 0;

    if (newIndex == headIndex)
      return false;

    // Write value to ring
    //
    _ring[writeIndex] = tp;

    // Then update tail pointer
    //
    _tail.setValue(newIndex);

    return true;
  }

  uint32 availValues()
  {
    uint32 headIndex, tailIndex;

    if (_head.getValue(headIndex) <= _tail.getValue(tailIndex))
      return tailIndex - headIndex;

    return _sz + tailIndex - headIndex;
  }

  uint32 availFreeSlots()
  {
    uint32 headIndex, tailIndex;

    /* Note: "+ 1" accounts for the fact that (head == tail) ==> empty ring
     */
    if (_head.getValue(headIndex) >= (_tail.getValue(tailIndex) + 1))
      return headIndex - (tailIndex + 1);

    return _sz + headIndex - (tailIndex + 1);
  }

  size_t size()
  {
    return _sz - 1;
  }

private:

  T* volatile *_ring;
  uint32 _sz;
  uint32 _specAlloc;
  rcAtomicValue<uint32> _head;
  rcAtomicValue<uint32> _tail;

// Disallow default copy ctor and assignment operaators.
  rcRingBuffer(rcRingBuffer<T>&);
  rcRingBuffer(const rcRingBuffer<T>&);
  rcRingBuffer<T>& operator=(const rcRingBuffer<T>&);
  rcRingBuffer<T>& operator=(rcRingBuffer<T>&);
};



/* rcBidirectionalRing - A pair of ring buffers that allows threads to share
 *                       recylcable resources (like frame buffers) using the
 *                       following model:
 *                     
 *                         1) "producer thread" adds resource to ring.
 *
 *                         2) "consumer thread" takes the resource from the
 *                            ring and uses it during its processing.
 *
 *                         3) "consumer thread" releases resource back to ring.
 *
 *                         4) "producer thread" recovers the resource for later
 *                            reuse.
 *
 *                       This is implemented by using a pair of
 *                       rcRingBuffer class template objects. All
 *                       operations are thread-safe.  A bidirectional
 *                       ring of pointers to T is created. The
 *                       following restrictions apply:
 *
 * 1) For each of the 4 functions: giveResource(), takeResource(),
 *    releaseResource(), and recoverResource(), one and only one
 *    thread can call that function. That is, for example, if thread A
 *    is calling takeResource(), thread B can never. Thread B could call
 *    releaseResource(), but only if thread A (or for that matter, any
 *    other thread) will never call it.
 *
 * The interface consists of the following public functions:
 *
 *   rcBidirectionalRing<T>(uint32 sz)  - Construct empty pair of ring buffers,
 *                                          each with sz free slots
 *
 *   bool giveResource(T*)                - Add a resource to the consumer's
 *                                          "available resources ring"
 *
 *   T* takeResource()                    - Take the next available resource from
 *                                          the "available resources ring"
 *
 *   bool releaseResource(T*)             - Add a resource to the producer's
 *                                          "freed resources ring"
 *
 *   T* recoverResource()                 - Take the next freed resource from
 *                                          the "freed resources ring"
 *
 *   size_t size()                        - Number of slots in ring
 *
 *   uint32 availNewResources()         - Number of resources available in the
 *                                          "available resources ring"
 *
 *   uint32 availNewSlots()             - Number of free slots available in the
 *                                          "available resources ring" for new
 *                                          resources
 *
 *   uint32 availReleasedResources()    - Number of resources available in the
 *                                          "freed resources ring"
 *
 *   uint32 availReleasedSlots()        - Number of free slots available in the
 *                                          "freed resources ring" for freed
 *                                          resources
 *
 * Note: The implementation of this class is trivial. Does this mean
 * it shouldn't exist? I like the fact that it provides a template for
 * how to use rcRingBuffers to share resources among different threads,
 * so I'll leave it in for now.
 */
template <class T>
class rcBidirectionalRing
{
public:

  rcBidirectionalRing(uint32 sz) : _available(sz), _freed(sz)
  {
  }

  rcBidirectionalRing(uint32 sz, sMemAllocator allocFct) :
    _available(sz, allocFct), _freed(sz, allocFct)
  {
  }

  bool giveResource(T* tp)
  {
    return _available.putValue(tp);
  }

  T* takeResource(void)
  {
    return _available.getValue();
  }

  bool releaseResource(T* tp)
  {
    return _freed.putValue(tp);
  }

  T* recoverResource(void)
  {
    return _freed.getValue();
  }

  uint32 availNewResources()
  {
    return _available.availValues();
  }

  uint32 availNewSlots()
  {
    return _available.availFreeSlots();
  }

  uint32 availReleasedResources()
  {
    return _freed.availValues();
  }

  uint32 availReleasedSlots()
  {
    return _freed.availFreeSlots();
  }

  size_t size()
  {
      return _freed.size(); // Could have used _available too.
  }

private:

  rcRingBuffer<T> _available;
  rcRingBuffer<T> _freed;

// Disallow default copy ctor and assignment operaators.
  rcBidirectionalRing(rcBidirectionalRing<T>&);
  rcBidirectionalRing(const rcBidirectionalRing<T>&);
  rcBidirectionalRing<T>& operator=(const rcBidirectionalRing<T>&);
  rcBidirectionalRing<T>& operator=(rcBidirectionalRing<T>&);
};

#endif // _rcRING_H_
