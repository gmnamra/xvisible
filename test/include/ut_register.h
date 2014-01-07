/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.2  2005/12/04 19:41:27  arman
 *register unit test plus lattice bug fix
 *
 *Revision 1.1  2005/10/24 02:08:39  arman
 *ut for register
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_REGISTER_H
#define __UT_REGISTER_H

#include <rc_unittest.h>
#include <rc_register.h>


class UT_register : public rcUnitTest {
public:

   UT_register();
   ~UT_register();

   virtual uint32 run();

  private:
  void test (int32 w, int32 h, int32 integerPixelSpacing, bool opt);
};



#endif /* __UT_REGISTER_H */
