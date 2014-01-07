/*
 *
 *$Header $
 *$Id $
 *$Log: 
 *
 * 
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#ifndef __UT_MOREIP_H
#define __UT_MOREIP_H

#include <rc_unittest.h>
#include <rc_ip.h>

class UT_moreip : public rcUnitTest {
public:

   UT_moreip();
   ~UT_moreip();

   virtual uint32 run();

  private:
   void testmoreip();

   // Test one window
   void testExpand();

   // Test one window size with all origins
   void testSample();
  void testReflect (int32 size);
   void testShift();
   void testHyst();
   void testIntegral();
   void testGenMomentIntegrals();
  void testmorph ();
   void testGauss ();
   void testPixelMap ();
   void testPixelMap (uint32 width, uint32 height );
  void testGauss (uint32 width, uint32 height , rcPixel);
   void testTimes ( bool useAltivec, int32 width, int32 height );
   void testTimes2 ( bool useAltivec, int32 width, int32 height );

   
};


#endif /* __UT_MOREIP_H */
