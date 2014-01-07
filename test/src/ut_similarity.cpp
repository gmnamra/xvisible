/*
 *
 *$Id $
 *$Log$
 *Revision 1.16  2005/01/25 09:39:19  arman
 *added wrt latest latticeSimilarity changes
 *
 *Revision 1.15  2005/01/17 14:51:00  arman
 *updated wrt lattice changes
 *
 *Revision 1.14  2005/01/16 03:36:14  arman
 *added ut for longTermEntropy additions to ut_similarity
 *
 *Revision 1.13  2005/01/14 21:34:31  arman
 *added lattice test
 *
 *Revision 1.12  2005/01/14 16:25:50  arman
 *incremental
 *
 *Revision 1.11  2004/08/12 19:16:42  arman
 **** empty log message ***
 *
 *Revision 1.10  2004/08/09 13:20:23  arman
 *added test for Ref
 *
 *Revision 1.9  2004/03/15 03:16:31  arman
 *added correlationDefinition
 *
 *Revision 1.8  2003/09/21 22:53:34  arman
 *fixed according to default behaviour being 1 - visualEntropy. rcOptoKinetic produces visualEntropy
 *
 *Revision 1.7  2003/04/15 18:20:19  proberts
 *New rcSimilarator interface and implementation
 *
 *Revision 1.6  2003/04/04 20:47:21  sami
 *Use rfCorrelateWindow instead of rfCorrelate for pixel sums caching
 *
 *Revision 1.5  2003/03/06 22:34:15  sami
 *Added missing return value
 *
 *Revision 1.4  2003/03/03 20:20:03  arman
 *similarity ut
 *
 *Revision 1.3  2003/02/04 22:56:58  arman
 *incremental
 *
 *Revision 1.2  2003/02/04 22:07:20  arman
 *added 1/2 pixel shift
 *
 *Revision 1.1  2003/02/03 05:04:23  arman
 *unit test for similarity
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_similarity.h"
#include <rc_math.h>
#include <rc_time.h>
#include <rc_utdrawutils.h>
#include <rc_ip.h>
#include <rc_ipconvert.h>
#include <rc_fileutils.h>

UT_similarity::UT_similarity()
{
}

UT_similarity::~UT_similarity()
{
    printSuccessMessage( "rcSimilarator test", mErrors );
}



void UT_similarity::testLattice()
{
  vector<rc2Dvector> ctrs (2);
  vector<int32> dias (2);
  vector<rc2Dvector> ds (2);
  int32 width (128), height (128);
  dias [0] = 20 + uint8 (random ()) / 2;
  dias [1] = 50 + uint8 (random ()) / 2;
  ctrs[0] = rc2Dvector (width / 4, height / 4);
  ctrs[1] = rc2Dvector ((3 * width) / 4., (3 * height) / 4.);
  rcWindow tmp (width, height);
  rcWindow tmp2 (width, height);


  vector<rcWindow> movie;

  for (int32 j = -1; j < 2; j++)
    for (int32 i = -1; i < 2; i++)
      {
	tmp.setAllPixels (128);

	for (int32 c = 0; c < 2; c++)
	  rfDrawCircle (ctrs[c] + rc2Dvector (i, j), 
			dias[c] / 2, tmp, 32);

	rfGaussianConv (tmp, tmp2, 17);
	rcWindow celld = rfPixelSample (tmp2, 3, 3);
	movie.push_back (celld);
    }

  rcLatticeSimilarator lat (movie.size(), 5);

  vector<rcWindow>::iterator rItr = movie.begin();
  vector<rcWindow>::iterator eItr = rItr;
  for (int32 i = 0; i < 3; i++) eItr++;

  lat.fill (rItr, movie.end());
  rcWindow ttc, ttd;
  lat.ttc (ttc, ttd);
  rcWindow ttc8 = rfImageConvertFloat8 (ttc, 1.0f, 0.0f);
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
    rcUNITTEST_ASSERT(rfRealEq(ent[i], 0.0, 1.e-9));


  for (uint32 i = 0; i < images.size(); i++) {
    rcWindow tmp(640, 480, sm.depth());
    tmp.randomFill(i);
    images[i] = tmp;
  }

  rsCorrParams cp;
  vector<double> fr(images.size());
  rfOptoKineticEnergy(images, fr, cp);

  rcUTCheck (sm.longTermCache() == false);
  rcUTCheck (sm.longTermCache (true) == true);
  rcUTCheck (sm.longTermCache() == true);

  fRet = sm.fill(images);
  rcUNITTEST_ASSERT(fRet);

  eRet = sm.entropies(ent, rcSimilarator::eVisualEntropy);
  rcUNITTEST_ASSERT(fRet);

  rcUNITTEST_ASSERT(ent.size() == images.size());
  rcUNITTEST_ASSERT(ent.size() == fr.size());

  for (uint32 i = 0; i < ent.size(); i++)
    rcUNITTEST_ASSERT(rfRealEq(ent[i], fr[i], 1.e-9));

  for (uint32 i = 0; i < ent.size(); i++)
    rcUNITTEST_ASSERT(rfRealEq((double)sm.longTermEntropy()[i], 
				fr[i], 1.e-9));

  deque<deque<double> > matrix;
  sm.selfSimilarityMatrix(matrix);
  rfDumpMatrix (matrix);

  // Test RefSimilarator
	rcSimilaratorRef simi (new rcSimilarator (rcSimilarator::eExhaustive,rcPixel8,7, 0));
  rcUTCheck (simi.use_count() == 1);
  rcSimilaratorRef simi2 (simi);
  rcUTCheck (simi.use_count() == 2);

  // Test Base Filer
  vector<double> signal (32);
  simi->filter (signal);
}

void UT_similarity::testUpdate()
{
  rcSimilarator::rcMatrixGeneration matType[3] = {
    rcSimilarator::eExhaustive,
    rcSimilarator::eApproxNoMatrix,
    rcSimilarator::eApproximate
  };

  // Test LongTerm Correlation only for Exhaustive. 

  for (uint32 matGen = 0; matGen < 3; matGen++) {
    for (rcPixel depth = rcPixel8; depth != rcPixelDouble;
	 depth = (rcPixel)((uint32)depth << 1)) {
      uint32 icnt = 15;
      uint32 winSz = 2;
      vector<rcWindow> srcvector(icnt);
      for (uint32 i = 0; i < srcvector.size(); ++i) {
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
	    rcUTCheck (rfRealEq((double) simu.longTermEntropy().back(), entu.back (), 1.e-5));
	  }

	if (entu.size() != entf.size())
	  rcUNITTEST_ASSERT(entu.size() == entf.size());
	else {
	  for (uint32 j = 0; j < entu.size(); j++)
	    rcUNITTEST_ASSERT(entu[j] == entf[j]);
	}
      }
    } // End of: for (... ; depth != rcPixelDouble; ... ) {
  } // End of: for (uint32 matGen = 0; matGen < 3; matGen++) {
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
   
  rcTime timer;
  uint32 repeats = 1;

  rcSimilarator::rcMatrixGeneration type = rcSimilarator::eApproximate;
  if (useExh)
    type = rcSimilarator::eExhaustive;

  rcSimilarator sim(type, images[0].depth(), size, 0);

  timer.start();
  for (uint32 i = 0; i < repeats; ++i) {
    deque<double> sr;
    sim.fill(images);
    sim.entropies(sr);
    rmAssertDebug(sr.size() == imagevector.size());
  }
  timer.end();
        
  double dMilliSeconds = timer.milliseconds() / repeats;
  double dMicroSeconds = timer.microseconds() / repeats;
        
  // Per Byte in Useconds
  double perByte = dMicroSeconds /
    (imagevector[0].width() * imagevector[0].height() * imagevector[0].depth());
        
  fprintf(stderr,
	  "Performance: %s rcSimilarator[%s<%-.3lu>] "
	  "correlation [%i x %i %i]: %.3f ms, %.6f us/8bit pixel "
	  "%.2f MB/s, %.2f fps\n",
	  useAltivec ? "AltiVec" : "       ",
	  useExh ? "exhaustive" : "approximat",
	  imagevector.size(), imagevector[0].width(), imagevector[0].height(),
	  imagevector[0].depth() * 8, dMilliSeconds, perByte, 1/perByte,
	  1000/dMilliSeconds);
}

uint32 UT_similarity::run()
{
  // Basic tests
  testBasics();
  testUpdate();

  // Performance tests
  const uint32 min = 2;
  const uint32 max = 16;

#if defined (PERFORMANCE)	
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
      testPerformance( true, true, i, imagevector ); // Altivec
  }
  fprintf(stderr, "\n" );
  for ( uint32 i = min; i <= max; i *=2 ) {
      // deque ctor
      testPerformance( false, false, i, imagevector );
      testPerformance( true, false, i, imagevector ); // Altivec
  }

#endif  // Performace
	
  testLattice ();

  return mErrors;
}

