/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.2  2004/08/10 15:39:24  arman
 *added support for sagvol and convlv
 *
 *Revision 1.1  2003/09/22 00:54:25  arman
 *NR
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#ifndef __UT_NR_H
#define __UT_NR_H
#include <rc_unittest.h>

class UT_nr : public rcUnitTest {
public:

   UT_nr();
   ~UT_nr();

   virtual uint32 run();

  private:
   void testMrq();
   void testMrq2();
   void testSVD();
  void testconvlv ();
  void testsavgol ();
};


#endif /* __UT_NR_H */
