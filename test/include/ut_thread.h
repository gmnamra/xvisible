// Copyright 2002 Reify, Inc.

#ifndef _rcUT_THREAD_H_
#define _rcUT_THREAD_H_

#include <rc_unittest.h>

class UT_Thread : public rcUnitTest {
public:

    UT_Thread();
    ~UT_Thread();

    virtual uint32 run();

private:
    void mutexLiveThreadsTest();
    void mutexStaticTest();
    void priorityInversionTest();
    void priorityTest();
};

#endif // _rcUT_THREAD_H_
