/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/03/25 20:49:20  arman
 *portaudio
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_PORTAUDIO_H
#define __UT_PORTAUDIO_H

#include <rc_unittest.h>

class UT_pa : public rcUnitTest {
public:

    UT_pa();
    ~UT_pa();

    virtual uint32 run();

private:
    void test ();
};




#endif /* __UT_PORTAUDIO_H */
