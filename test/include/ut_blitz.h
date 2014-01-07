/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: ut_blitz.h 4391 2006-05-02 18:40:03Z armanmg $
 *
 ******************************************************************************/

#ifndef _rcUT_BLITZ_H_
#define _rcUT_BLITZ_H_

#include <rc_unittest.h>

class UT_Blitz : public rcUnitTest {
public:

    UT_Blitz();
    ~UT_Blitz();

    virtual uint32 run();

  private:
    uint32 testThreads( int32 nThreads );
};

#endif // _rcUT_BLITZ_H_
