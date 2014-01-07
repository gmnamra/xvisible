/*
 *  ut_windowhist.h
 *  framebuf
 *
 *  Created by Peter Roberts on Tue May 14 2002.
 *  Copyright (c) 2002 Reify, Inc. All rights reserved.
 *
 */

#ifndef __UT_WINDOWHIST_H__
#define __UT_WINDOWHIST_H__

#include "rc_unittest.h"

class UT_WindowHistogram : public rcUnitTest {
public:

    UT_WindowHistogram();
    ~UT_WindowHistogram();

    virtual uint32 run();

private:
    void hist8Test();
    void histTime(uint32 width, uint32 height);
};

#endif
