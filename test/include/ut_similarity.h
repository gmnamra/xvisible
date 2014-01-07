/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.4  2005/01/14 16:29:39  arman
 *added lattice test
 *
 *Revision 1.3  2003/04/15 18:20:19  proberts
 *New rcSimilarator interface and implementation
 *
 *Revision 1.2  2003/04/04 20:47:21  sami
 *Use rfCorrelateWindow instead of rfCorrelate for pixel sums caching
 *
 *Revision 1.1  2003/02/03 05:05:10  arman
 *unit test for similarity
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#ifndef __UT_SIMILARITY_H
#define __UT_SIMILARITY_H

#include <rc_unittest.h>
#include <rc_similarity.h>
#include <rc_latticesimilarity.h>

class UT_similarity : public rcUnitTest {
public:

   UT_similarity();
   ~UT_similarity();

   virtual uint32 run();

  private:
   void testBasics();

   // Test that update produces same results as fill
   void testUpdate();

   // Test performance with different vector sizes
   void testPerformance( bool useAltivec, bool useVector, uint32 size,
                         vector<rcWindow>& images );

   void testLattice();


};

#endif /* __UT_SIMILARITY_H */
