
#include "ut_similarity.h"
#include "rc_math.h"
#include "rc_fileutils.h"
#include "rc_similarity_producer.h"
#include <boost/bind/bind.hpp>
#include "timing.hpp"

using namespace boost;

UT_similarity::UT_similarity()
{
}

UT_similarity::~UT_similarity()
{
    printSuccessMessage( "rcSimilarator test", mErrors );
}


UT_similarity_producer::UT_similarity_producer( const std::string& rfyInputMovie, const std::string& QTInputMovie ) :
   mRfyInputMovie( rfyInputMovie ), mQTInputMovie( QTInputMovie )
{
    rcUTCheck( rfFileExists (rfyInputMovie  ) );
    rcUTCheck( rfFileExists (QTInputMovie) );
}


UT_similarity_producer::~UT_similarity_producer()
{
    printSuccessMessage( "rcSimilarator Producer test", mErrors );
}



uint32 UT_similarity_producer::run ()
{
    int errors = test (mRfyInputMovie) ? 0 : 1;
    errors += test (mQTInputMovie) ? 0 : 1;
    return errors;
}


bool UT_similarity_producer::test (const std::string& fqfn)
{
    clear_movie_loaded();
    boost::function<void (void)> movie_loaded_cb = boost::bind (&UT_similarity_producer::signal_movie_loaded, this); 
    boost::function<void (int&,double&)> frame_loaded_cb = boost::bind (&UT_similarity_producer::signal_frame_loaded, this, _1, _2);   
    
    boost::shared_ptr<SimilarityProducer> sp ( new SimilarityProducer () );
    boost::signals2::connection ml_connection = sp->registerCallback(movie_loaded_cb);
    boost::signals2::connection fl_connection = sp->registerCallback(frame_loaded_cb);    
    //    bool provides_movie_loaded_signal = sp->providesCallback<movie_loaded_cb> ();
    
    sp->load_content_file(fqfn);
    
    sp->operator()(0, 0);
    
    return is_movie_loaded () && sp->has_content();
    
}


void UT_similarity::testBasics()
{
  vector<rcWindow> images(4);
  double tiny = 1e-10;

  rcSimilarator sm(rcSimilarator::eExhaustive,
		   rcPixel8,
		   images.size(),
		   0, rcSimilarator::eNorm,
		   false,
		   0,
		   tiny); 

  rcUNITTEST_ASSERT(sm.matGenType() == rcSimilarator::eExhaustive);
  rcUNITTEST_ASSERT(sm.corrDefinition () == rcSimilarator::eNorm);
  rcUNITTEST_ASSERT(sm.depth() == rcPixel8);
  rcUNITTEST_ASSERT(sm.matrixSz() == images.size());
  rcUNITTEST_ASSERT(sm.cacheSz() == 0);
  rcUNITTEST_ASSERT(sm.aborted() == false);

  rcWindow tmp (640, 480, sm.depth());
  tmp.randomFill(0);

  for (uint32 i = 0; i < images.size(); i++) {
    images[i] = tmp;
  }

  deque<double> ent;

  bool fRet = sm.fill(images);
  rcUNITTEST_ASSERT(fRet);

  bool eRet = sm.entropies(ent);
  rcUNITTEST_ASSERT(fRet);

  rcUNITTEST_ASSERT(ent.size() == images.size());

  for (uint32 i = 0; i < ent.size(); i++)
    rcUNITTEST_ASSERT(real_equal(ent[i], 0.0, 1.e-9));


  for (uint32 i = 0; i < images.size(); i++) {
    rcWindow tmp(640, 480, sm.depth());
    tmp.randomFill(i);
    images[i] = tmp;
  }


  rcUTCheck (sm.longTermCache() == false);
  rcUTCheck (sm.longTermCache (true) == true);
  rcUTCheck (sm.longTermCache() == true);

  fRet = sm.fill(images);
  rcUNITTEST_ASSERT(fRet);

  eRet = sm.entropies(ent, rcSimilarator::eVisualEntropy);
  rcUNITTEST_ASSERT(fRet);

  rcUNITTEST_ASSERT(ent.size() == images.size());
  rcUNITTEST_ASSERT(fRet);


  deque<deque<double> > matrix;
  sm.selfSimilarityMatrix(matrix);
  rfDumpMatrix (matrix);

  // Test RefSimilarator
	rcSimilaratorRef simi (new rcSimilarator (rcSimilarator::eExhaustive,rcPixel8,7, 0));
  rcUTCheck (simi.use_count() == 1);
  rcSimilaratorRef simi2 (simi);
  rcUTCheck (simi.use_count() == 2);

  // Test Base Filer
  // vector<double> signal (32);
  // simi->filter (signal);
}

void UT_similarity::testUpdate()
{
  rcSimilarator::rcMatrixGeneration matType[3] = {
    rcSimilarator::eExhaustive,
    rcSimilarator::eApproxNoMatrix,
    rcSimilarator::eApproximate
  };

  // Test LongTerm Correlation only for Exhaustive. 

   uint32 matGen = 0;
    
    int depths[3] = {get_ipl().id (rcPixel8), get_ipl().id(rcPixel16), get_ipl().id(rcPixel32S) };
    for (int i = 0; i < 3; i++)
    {
      rcPixel depth = get_pixel().em(depths[i]);
      uint32 icnt = 15;
      uint32 winSz = 2;
      vector<rcWindow> srcvector(icnt);
      for (uint32 i = 0; i < srcvector.size(); ++i)
      {
	rcWindow tmp (640, 480, depth);
	tmp.randomFill(i);
	srcvector[i] = tmp;
      }

      vector<rcWindow> imagevector;

      rcSimilarator simu(matType[matGen], depth, winSz, 0);

      rcUTCheck (simu.longTermCache() == false);
      rcUTCheck (simu.longTermCache (true) == true);
      rcUTCheck (simu.longTermCache() == true);

      deque<double> entu;
      bool iFill = simu.fill(imagevector); // Initialize with null vector
      bool iEnt = simu.entropies(entu, rcSimilarator::eVisualEntropy);
      if (iFill || iEnt) {
	rcUNITTEST_ASSERT(!iFill && !iEnt);
	return;
      }

      for (uint32 i = 0; i < icnt; i++) {
	rcSimilarator simf(matType[matGen], depth, winSz, 0);

	rcUTCheck (simf.longTermCache() == false);
	rcUTCheck (simf.longTermCache (true) == true);
	rcUTCheck (simf.longTermCache() == true);


	vector<rcWindow> imagevectorf;
	imagevectorf.insert(imagevectorf.begin(), srcvector.begin(),
			    srcvector.begin() + i + 1);

	deque<double> entf;
	bool fFill = simf.fill(imagevectorf);
	if (i < (winSz-1))
	  {
	    rcUNITTEST_ASSERT(!fFill);
	    if (!matGen)
	      rcUTCheck (simf.longTermEntropy().size() == simf.matrixSz ());
	  }
	else
	  {
	    rcUNITTEST_ASSERT(fFill);
	    if (!matGen)
	      rcUTCheck (simf.longTermEntropy().size() == winSz);
	  }

	bool fEnt = simf.entropies(entf, rcSimilarator::eVisualEntropy);
	rcUNITTEST_ASSERT(fEnt == fFill);

	bool update = simu.update(srcvector[i]);
	if (i < (winSz-1))
	  rcUNITTEST_ASSERT(!update);
	else 
	  rcUNITTEST_ASSERT(update);

	bool uEnt = simu.entropies(entu, rcSimilarator::eVisualEntropy);
	rcUNITTEST_ASSERT(update == uEnt);

	if (!matGen && i > (winSz-1) && update)
	  {
	    rcUTCheck (simu.longTermEntropy().size() == (entu.size()+i));
              //    rcUTCheck (real_equal((double) simu.longTermEntropy().back(), entu.back (), 1.e-5));
	  }

	if (entu.size() != entf.size())
	  rcUNITTEST_ASSERT(entu.size() == entf.size());
	else {
	  for (uint32 j = 0; j < entu.size(); j++)
	    rcUNITTEST_ASSERT(entu[j] == entf[j]);
	}
      }
    } // End of: for (... ; depth != rcPixelDouble; ... ) {
}


// Test performance with different vector sizes
void UT_similarity::testPerformance(bool useAltivec, bool useExh,
				    uint32 size, vector<rcWindow>& images)
{
  rmAssert(size && (size <= images.size()));

  // Create appropriately sized vector from input images
  vector<rcWindow> imagevector(size);
    
  for (uint32 i = 0; i < size; ++i)
    imagevector[i] = images[i];
     
  rfForceSIMD(useAltivec);
   

  uint32 repeats = 1;

  rcSimilarator::rcMatrixGeneration type = rcSimilarator::eApproximate;
  if (useExh)
    type = rcSimilarator::eExhaustive;

  rcSimilarator sim(type, images[0].depth(), size, 0);

    {
        chronometer timer;
        double starttime = timer.getTime();
  for (uint32 i = 0; i < repeats; ++i) {
    deque<double> sr;
    sim.fill(images);
    sim.entropies(sr);
    rmAssertDebug(sr.size() == imagevector.size());
  }
        
  double dMicroSeconds = timer.getTime() / repeats;
        
  // Per Byte in Useconds
  double perByte = dMicroSeconds /
    (imagevector[0].width() * imagevector[0].height() * imagevector[0].depth());
        
  fprintf(stderr,
	  "Performance: %s rcSimilarator[%s<%-.3lu>] "
	  "correlation [%i x %i %i]: %.3f ms, %.6f us/8bit pixel "
	  "%.2f MB/s \n",
	  useAltivec ? "AltiVec" : "       ",
	  useExh ? "exhaustive" : "approximat",
	  imagevector.size(), imagevector[0].width(), imagevector[0].height(),
          imagevector[0].depth() * 8, dMicroSeconds / 1000., perByte, 1/perByte);
    }
    
}


uint32 UT_similarity::run()
{
  // Basic tests
  testBasics();
  testUpdate();

  // Performance tests
  const uint32 min = 2;
  const uint32 max = 16;

  // Create a vector of random images
  vector<rcWindow> imagevector( max );
  for (uint32 i = 0; i < imagevector.size(); ++i) {
      rcWindow tmp (1280, 960);
      tmp.randomFill (i);
      imagevector[i] = tmp;
  }
  // Test perfomance with varying vector sizes
  for ( uint32 i = min; i <= max; i *=2 ) {
      // vector ctor
      testPerformance( false, true, i, imagevector ); 
      //    testPerformance( true, true, i, imagevector ); // Altivec
  }
  fprintf(stderr, "\n" );
  for ( uint32 i = min; i <= max; i *=2 ) {
      // deque ctor
      testPerformance( false, false, i, imagevector );
      //      testPerformance( true, false, i, imagevector ); // Altivec
  }


	
  return mErrors;
}

