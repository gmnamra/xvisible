/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.4  2005/02/07 13:53:19  arman
 *added testFilter
 *
 *Revision 1.3  2004/08/20 19:13:53  arman
 *added moment tests
 *
 *Revision 1.2  2004/03/12 22:16:33  arman
 *added more tests
 *
 *Revision 1.1  2004/03/05 18:00:50  arman
 *initial
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_SHAPE_H
#define __UT_SHAPE_H


#include <rc_unittest.h>
#include <rc_shape.h>
#include <rc_filter1d.h>

class UT_shape : public rcUnitTest {
public:

   UT_shape();
   ~UT_shape();

   virtual uint32 run();

private:
   void testBasic ();
   void testMoment ();
   void testAffine ();
   void testFilter ();
};


#endif /* __UT_SHAPE_H */
