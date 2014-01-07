/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/04/23 17:52:17  arman
 *initial
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_GSHAPE_H
#define __UT_GSHAPE_H


#include <rc_unittest.h>
#include <rc_ellipse.h>

class UT_ellipse : public rcUnitTest {
public:

    UT_ellipse();
    ~UT_ellipse();

    virtual uint32 run();

private:
    void test ();
};



#endif /* __UT_GSHAPE_H */
