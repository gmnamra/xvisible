/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.2  2003/03/11 15:41:45  arman
 *added units ut
 *
 *Revision 1.1  2002/12/24 17:13:59  arman
 *Unit test for fixed point cordic implentation
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_CORDIC_H
#define __UT_CORDIC_H


#include <rc_unittest.h>

class UT_cordic : public rcUnitTest {
public:

    UT_cordic();
    ~UT_cordic();

    virtual uint32 run();

private:
    void test ();
    void testUnits ();

};



#endif /* __UT_CORDIC_H */
