// Copyright 2002 Reify, Inc.

#ifndef _rcUT_ATOMIC_H_
#define _rcUT_ATOMIC_H_

#include "rc_unittest.h"

class UT_AtomicValue : public rcUnitTest {
public:

    UT_AtomicValue();
    ~UT_AtomicValue();

    virtual uint32 run();

private:
    void genTestCases();
    void raceTestNoGuard();
    void liveThreadsTest();
};

#endif // _rcUT_ATOMIC_H_
