// Copyright 2002-2003 Reify, Inc.

#ifndef _rcUT_WINDOW_H_
#define _rcUT_WINDOW_H_

#include "rc_unittest.h"
#include "rc_window.h"
#include "rc_framebuf.h"

class UT_Window : public rcUnitTest {
public:

    UT_Window();
    ~UT_Window();

    virtual uint32 run();

private:
    void testWindow ( int32 x, int32 y, int32 width, int32 height, rcFrameRef& buf );
    void testSubWindows ( int32 x, int32 y, int32 width, int32 height, rcFrameRef& buf );
    void testRowPointers ( int32 x, int32 y, int32 width, int32 height, rcFrameRef& buf );
    // Compare two windows, return true if identical
    bool compare( const rcWindow& w1, const rcWindow& w2 );
};


class UT_WindowMutator : public rcUnitTest {
public:

    UT_WindowMutator();
    ~UT_WindowMutator();

    virtual uint32 run();

private:
    void setTest();
    void copyTest();
    void randomTest();
    void testRandomFill( rcWindow& w1, rcWindow& w2 );
    void mirrorTest();
    void testMirror( rcWindow& w );
};

#endif // _rcUT_WINDOW_H_
