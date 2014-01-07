/*
 *@file
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/04/04 21:03:28  arman
 *kalman ut
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_KALMAN_H
#define __UT_KALMAN_H

#include <rc_unittest.h>
#include <kalman.h>

class UT_kalman : public rcUnitTest {
public:

   UT_kalman();
   ~UT_kalman();

   virtual uint32 run();

  private:

   void test ();
   
};


#endif /* __UT_KALMAN_H */
