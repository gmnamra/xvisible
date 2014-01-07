// Copyright (c) 2002 Reify Corp. All rights reserved.

#ifndef _rcUT_QTIME_H_
#define _rcUT_QTIME_H_

#include <rc_unittest.h>
#include <rc_qtime.h>

class UT_Qtime : public rcUnitTest {
public:

    UT_Qtime();
    ~UT_Qtime();

    virtual uint32 run();

private:

    uint32 testRfFillColormap();
};

#endif // _rcUT_QTIME_H_
