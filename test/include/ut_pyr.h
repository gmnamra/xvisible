/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2004/07/12 19:49:55  arman
 *added kinetic pyramid tests
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_PYR_H
#define __UT_PYR_H

#include <rc_unittest.h>
#include <rc_kinepyr.h>

class UT_pyramid : public rcUnitTest {
public:

   UT_pyramid();
   ~UT_pyramid();

   virtual uint32 run();

  private:

   void test ();
   
};



#endif /* __UT_PYR_H */
