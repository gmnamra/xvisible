#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <rc_ring.h>
#include <ut_ring.h>

void UT_RingBuffer::ringBufferTest()
{
  const uint32 sz = 20;
  rcRingBuffer<double> ring(sz);
  vector<double> values(sz);

  rcUNITTEST_ASSERT(ring.size() == sz);

  for (uint32 i = 0; i < ring.size(); i++)
  {
    values[i] = 5.3 + i;
    
    rcUNITTEST_ASSERT(ring.putValue(&values[i]));

    rcUNITTEST_ASSERT(ring.availValues() == (i+1));

    rcUNITTEST_ASSERT(ring.availFreeSlots() == (sz-(i+1)));
  }

  rcUNITTEST_ASSERT(!ring.putValue(&values[0]));

  for (uint32 i = 0; i < ring.size(); i++)
  {
    double* dp;

    rcUNITTEST_ASSERT((dp = ring.getValue()) != 0);
    
    rcUNITTEST_ASSERT(ring.availValues() == (sz - (i+1)));

    rcUNITTEST_ASSERT(ring.availFreeSlots() == i+1);
  }

  rcUNITTEST_ASSERT(ring.getValue() == 0);
}

UT_RingBuffer::UT_RingBuffer()
{
}

UT_RingBuffer::~UT_RingBuffer()
{
    printSuccessMessage( "rcRingBuffer test", mErrors );
}

uint32 UT_RingBuffer::run()
{
  ringBufferTest();

  return mErrors;  
}

void UT_BidirectionalRing::bidirectionalRingTest()
{
  const uint32 sz = 20;
  rcBidirectionalRing<double> ring(sz);
  vector<double> values(sz);

  rcUNITTEST_ASSERT(ring.size() == sz);

  for (uint32 i = 0; i < ring.size(); i++)
  {
    values[i] = 5.3 + i;

    rcUNITTEST_ASSERT(ring.giveResource(&values[i]));

    rcUNITTEST_ASSERT(ring.availNewResources() == i+1);

    rcUNITTEST_ASSERT(ring.availNewSlots() == (sz - (i+1)));

    rcUNITTEST_ASSERT(ring.availReleasedResources() == 0);

    rcUNITTEST_ASSERT(ring.availReleasedSlots() == sz);
  }

  rcUNITTEST_ASSERT(!ring.giveResource(&values[0]));

  for (uint32 i = 0; i < ring.size(); i++)
  {
    double* dp;

    rcUNITTEST_ASSERT((dp = ring.takeResource()) != 0);

    rcUNITTEST_ASSERT(dp == &values[i]);

    rcUNITTEST_ASSERT(ring.availNewResources() == (sz - (i+1)));

    rcUNITTEST_ASSERT(ring.availNewSlots() == i+1);

    rcUNITTEST_ASSERT(ring.availReleasedResources() == 0);

    rcUNITTEST_ASSERT(ring.availReleasedSlots() == sz);
  }

  rcUNITTEST_ASSERT(ring.takeResource() == 0);

  for (uint32 i = 0; i < ring.size(); i++)
  {
    values[i] = 5.3 + i;

    rcUNITTEST_ASSERT(ring.releaseResource(&values[i]));

    rcUNITTEST_ASSERT(ring.availNewResources() == 0);

    rcUNITTEST_ASSERT(ring.availNewSlots() == sz);

    rcUNITTEST_ASSERT(ring.availReleasedResources() == i+1);

    rcUNITTEST_ASSERT(ring.availReleasedSlots() == (sz - (i+1)));
  }

  rcUNITTEST_ASSERT(!ring.releaseResource(&values[0]));

  for (uint32 i = 0; i < ring.size(); i++)
  {
    double* dp;

    rcUNITTEST_ASSERT((dp = ring.recoverResource()) != 0);

    rcUNITTEST_ASSERT(dp == &values[i]);

    rcUNITTEST_ASSERT(ring.availNewResources() == 0);

    rcUNITTEST_ASSERT(ring.availNewSlots() == sz);

    rcUNITTEST_ASSERT(ring.availReleasedResources() == (sz - (i+1)));

    rcUNITTEST_ASSERT(ring.availReleasedSlots() == i+1);
  }

  rcUNITTEST_ASSERT(ring.recoverResource() == 0);
}

UT_BidirectionalRing::UT_BidirectionalRing()
{
}

UT_BidirectionalRing::~UT_BidirectionalRing()
{
    printSuccessMessage( "rcBidirectionalRing test", mErrors );
}

uint32 UT_BidirectionalRing::run()
{
  bidirectionalRingTest();

  return mErrors;  
}

