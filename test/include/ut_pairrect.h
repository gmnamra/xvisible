/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2002/12/08 22:55:03  arman
 *pair and rect unit test
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_PAIRRECT_H
#define __UT_PAIRRECT_H

#include "rc_unittest.h"

class UT_pair : public rcUnitTest {
public:

    UT_pair();
    ~UT_pair();

    virtual uint32 run();

private:
    void test ();
};


class UT_rect : public rcUnitTest {
public:

    UT_rect();
    ~UT_rect();

    virtual uint32 run();

private:
    void test ();
};


#endif /* __UT_PAIRRECT_H */
