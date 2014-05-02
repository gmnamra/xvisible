// Copyright 2002 Reify, Inc.

#ifndef _rcUT_RING_H_
#define _rcUT_RING_H_

#include "rc_unittest.h"

class UT_RingBuffer : public rcUnitTest {
public:

    UT_RingBuffer();
    ~UT_RingBuffer();

    virtual uint32 run();

private:
    void ringBufferTest();
};

class UT_BidirectionalRing : public rcUnitTest {
public:

    UT_BidirectionalRing();
    ~UT_BidirectionalRing();

    virtual uint32 run();

private:
    void bidirectionalRingTest();
};

#endif // _rcUT_RING_H_
