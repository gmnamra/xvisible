// Copyright 2002 Reify, Inc.

#ifndef _rcUT_FRAMEBUF_H_
#define _rcUT_FRAMEBUF_H_

#include <rc_unittest.h>
#include <rc_framebuf.h>

class UT_FrameBuf : public rcUnitTest {
public:

    UT_FrameBuf();
    ~UT_FrameBuf();

    virtual uint32 run();

private:
    void testFrameBuffer( int32 width, int32 height,rcPixel d );
    void testRowPointers( int32 width, int32 height, rcPixel d );
    void testPixel( int32 x, int32 y, rcFrame& buf );
};

#endif // _rcUT_FRAMEBUF_H_
