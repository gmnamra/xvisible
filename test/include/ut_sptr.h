/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2004/03/16 19:01:39  arman
 *added minimal unit test for smart pointer
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_SPTR_H
#define __UT_SPTR_H

#include <rc_unittest.h>


class UT_smartPtr : public rcUnitTest {
 public:
  UT_smartPtr();

  ~UT_smartPtr();

  virtual uint32 run();
    
 private:

};

#endif /* __UT_SPTR_H */
