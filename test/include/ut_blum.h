/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.4  2003/06/16 02:53:58  arman
 *added function decleration
 *
 *Revision 1.3  2003/06/16 02:32:06  arman
 *added unit test for the faster distance map algorithm
 *
 *Revision 1.2  2003/05/23 21:21:38  arman
 *first ut checks
 *
 *Revision 1.1  2003/05/18 17:32:54  arman
 *Blum UT
 *

 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#ifndef __UT_BLUM_H
#define __UT_BLUM_H

#include <rc_unittest.h>
#include <rc_analysis.h>


extern vector<float> rfCBLDistance (vector<rc2Fvector>& edges);
extern void rfGrassFire(vector<rc2Dvector>& edges,
                   rcWindow &featureImage,
			rcWindow& distanceImage);
extern int distmap_4ssedp( int *map, int n[2] );
extern int distmap_saito2d(int *map, int size[2]);

class UT_Blum : public rcUnitTest {
public:

   UT_Blum();
   ~UT_Blum();

   virtual uint32 run();

  private:
   void testCBL ();
   void testBlum ();
   void testSeedp ();
   int diff(int *im1, int *im2, int size[2]);
   void draw(int *im, int size[2]);
   void testPerformance( bool useAltivec, uint32 size);
};

#endif /* __UT_BLUM_H */
