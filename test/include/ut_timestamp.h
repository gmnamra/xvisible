// Copyright (c) 2002 Reify Corp. All rights reserved.

#ifndef _rcUT_TIMESTAMP_H_
#define _rcUT_TIMESTAMP_H_

#include "rc_unittest.h"

class UT_Timestamp : public rcUnitTest {
public:

    UT_Timestamp();
    ~UT_Timestamp();

    virtual uint32 run();

  private:
    // Basic timestamp tests
    void testBasic();
    // Test timestamp addition vs. multiplication
    void additionTest( int n, double inc );
#ifdef QT_VISIBLE_SUPPORT    
    // Test speed calculator
    void testFps();
    // Test speed calculator with a specific frame interval
    void testFpsInterval( double frameInterval );
#endif
    
};

#endif // _rcUT_TIMESTAMP_H_
