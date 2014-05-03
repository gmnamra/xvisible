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
   void testPerformance( uint32 size, vector<rcWindow>& images );

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
        rcUTCheck (equal (timestamp, exected_movie_times[findex], 0.0001 ) );
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
    
    
    double exected_movie_times [57] = {0, 0.033333, 0.066666, 0.099999, 0.133332, 0.166665, 0.199998, 0.233331, 0.266664, 0.299997, 0.33333, 0.366663, 0.399996, 0.433329, 0.466662, 0.499995, 0.533328, 0.566661, 0.599994, 0.633327, 0.66666, 0.699993, 0.733326, 0.766659, 0.799992, 0.833325, 0.866658, 0.899991, 0.933324, 0.966657, 0.99999, 1.03332, 1.06666, 1.09999, 1.13332, 1.16665, 1.19999, 1.23332, 1.26665, 1.29999, 1.33332, 1.36665, 1.39999, 1.43332, 1.46665, 1.49998, 1.53332, 1.56665, 1.59998, 1.63332, 1.66665, 1.69998, 1.73332, 1.76665, 1.79998, 1.83332, 1.86665 };
    
};


#endif /* __UT_SIMILARITY_H */
