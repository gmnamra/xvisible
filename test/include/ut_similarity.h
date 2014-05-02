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

#include "rc_unittest.h"
#include "rc_similarity.h"
#include "rc_latticesimilarity.h"
#include "rc_similarity_producer.h"

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

    void testProducer (const std::string& fqfn);


};



class UT_similarity_producer : public rcUnitTest {
public:
    
    UT_similarity_producer( const std::string& rfyInputMovie, const std::string& QTInputMovie );
    ~UT_similarity_producer();
    
  
    virtual uint32 run();
    void signal_movie_loaded () { movie_loaded = true; }
    void signal_frame_loaded (int& findex, double& timestamp)
    { 
        frame_indices.push_back (findex);
        frame_times.push_back (timestamp);
        std::cout << "[" << findex << "] " << timestamp << std::endl;
    }    
    
    
private:
    std::vector<int> frame_indices;
    std::vector<double> frame_times;
    
    bool movie_loaded;    
    void clear_movie_loaded () { movie_loaded = false; }
    bool is_movie_loaded () { return movie_loaded; }

    bool test (const std::string& fqfn);
    
    std::string mRfyInputMovie;
    std::string mQTInputMovie;
};


#endif /* __UT_SIMILARITY_H */
