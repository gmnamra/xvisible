/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2003/03/04 20:12:05  arman
 *added line ut
 *
 *Revision 1.1  2002/12/24 17:13:59  arman
 *Unit test for fixed point cordic implentation
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_LINE_H
#define __UT_LINE_H


#include "rc_unittest.h"

class UT_line : public rcUnitTest {
public:

    UT_line();
    ~UT_line();

    virtual uint32 run();

private:
    void test ();
};



#endif /* __UT_LINE_H */
