/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.2  2004/10/07 18:18:53  arman
 *added general and list histogram
 *
 *Revision 1.1  2004/08/20 01:30:39  arman
 *fixed array and other histogram statistics containers
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_HISTOGRAM_H
#define __UT_HISTOGRAM_H

#include <rc_unittest.h>
#include <rc_fixedarray.h>
#include <rc_generalhistogram.h>
#include <rc_listhistogram.h>


class UT_histogram : public rcUnitTest {
public:

    UT_histogram();
    ~UT_histogram();

    virtual uint32 run();

private:
  void testGeneralHistogram ();
  void testListHistogram ();
  void testFixedArray ();
};


#endif /* __UT_HISTOGRAM_H */
