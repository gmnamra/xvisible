/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: ut_systeminfo.h 4391 2006-05-02 18:40:03Z armanmg $
 *
 ******************************************************************************/

#ifndef _rcUT_SYSTEMINFO_H_
#define _rcUT_SYSTEMINFO_H_

#include "rc_unittest.h"

class UT_Systeminfo : public rcUnitTest {
public:

    UT_Systeminfo();
    ~UT_Systeminfo();

    virtual uint32 run();
};

#endif // _rcUT_SYSTEMINFO_H_
