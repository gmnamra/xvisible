/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.2  2003/10/02 02:24:57  arman
 *header for ut and function declaration
 *
 *Revision 1.1  2003/05/12 02:12:46  arman
 *Frequency tools UT
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_FREQ_H
#define __UT_FREQ_H

#include <rc_unittest.h>
#include <vector>

extern void rf1Dfft (vector<float>& signal, vector<float>& imaginary, int32 dir);
class UT_freq : public rcUnitTest {
public:

   UT_freq();
   ~UT_freq();

   virtual uint32 run();

  private:
   void testBasics();

};

#endif /* __UT_FREQ_H */
