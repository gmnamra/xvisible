/*
 *
 *$Id: ut_stats.h 4391 2006-05-02 18:40:03Z armanmg $
 *
 * Copyright (c) 2003 Reify Corp. All rights reserved.
 */
#ifndef _UT_STATS_H_
#define _UT_STATS_H_

#include "rc_unittest.h"
#include "rc_stats.h"
#include "rc_fit.h"
#include "rc_lsfit.h"

class UT_stats : public rcUnitTest {
public:

    UT_stats();
    ~UT_stats();

    virtual uint32 run();

private:
    void test ();
    void testfit ();
};

#endif /* _UT_STATS_H_ */
