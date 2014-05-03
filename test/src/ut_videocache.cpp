// Copyright 2002 Reify, Inc.

#include <stdlib.h>

#include "ut_videocache.h"
#include "rc_videocache.h"
#include "rc_windowhist.h"
#include "timing.hpp"


static std::string fileName;

/*************************************************************************/
/*                                                                       */
/*                Support class for thread-safe tests                    */
/*                                                                       */
/*************************************************************************/
rcAtomicValue<int> utVideoCacheThread::startTest(0);

utVideoCacheThread::utVideoCacheThread(rcVideoCache& cache) : _cache(cache)
{
  for (uint32 i = 0; i < FRAME_COUNT; i++)
    _frameTouch[i] = 0;
  for (uint32 i = 0; i < (VIDEOCACHE_THREAD_CNT*5); i++)
    _refCount[i] = 0;
  _myErrors = 0;
  _prefetches = 0;
}

void utVideoCacheThread::run()
{
  int temp;
  
  while (startTest.getValue(temp) == 0)
    usleep(100);

  rc256BinHist hist(256);

  for (uint32 loopCnt = 0; loopCnt < 125000; loopCnt++) {
    int preAllocSleep = (rand() >> 4) & 0xff;
    int prefetchSleep = (rand() >> 4) & 0xff;
    int unlockedSleep = (rand() >> 4) & 0xff;
    int postAllocSleep = (rand() >> 4) & 0xff;

    int r = rand() >> 4;
    uint32 frameToAlloc = (uint32)(r & 0x7); r >>= 3;
    rmAssert(frameToAlloc < FRAME_COUNT);

    if (preAllocSleep)
      usleep(preAllocSleep); // Time without a buffer lock attempt
    rcSharedFrameBufPtr bp;
    rcVideoCacheError error;
    rcVideoCacheStatus status;
    bool prefetch = ((r & 1) == 1); r >>= 1;

    if (prefetch) {
      _prefetches++;
      status = _cache.getFrame(frameToAlloc, bp, &error, false);
      bp.prefetch();
      if (prefetchSleep)
	usleep(prefetchSleep);
    }
    else 
      status = _cache.getFrame(frameToAlloc, bp, &error, true);

    if (status == eVideoCacheStatusOK) {
      {
	rcWindow win(bp);
	uint32 refCount = bp.refCount();
	if ((refCount < 3) || (refCount >= (VIDEOCACHE_THREAD_CNT*5))) {
	  fprintf(stderr, "Error on refCount check %d\n", refCount);
	  _myErrors++;
	}
	else {
	  _refCount[refCount]++;
	  _frameTouch[frameToAlloc]++;
	}

	rfGenDepth8Histogram(win, hist);
	if (hist[frameToAlloc] != win.pixelCount()) {
	  fprintf(stderr, "Error on pixel check %d %d\n",
		  hist[frameToAlloc], win.pixelCount());
	  _myErrors++;
	}

	uint32 frameIndex = bp->frameIndex();
	if (frameIndex != frameToAlloc) {
	  fprintf(stderr, "Expected frame index: %d  actual: %d\n",
		 frameToAlloc, frameIndex);
	  _myErrors++;
	}
      }
      {
	bp.unlock();
	if (unlockedSleep)
	  usleep(unlockedSleep); // Unlock time
	if (bp) {
	  rcWindow win(bp);
	  uint32 refCount = bp.refCount();
	  if ((refCount < 3) || (refCount >= (VIDEOCACHE_THREAD_CNT*5))) {
	    fprintf(stderr, "Error on refCount check %d\n", refCount);
	    _myErrors++;
	  }
	  else {
	    _refCount[refCount]++;
	    _frameTouch[frameToAlloc]++;
	  }

	  rfGenDepth8Histogram(win, hist);
	  if (hist[frameToAlloc] != win.pixelCount()) {
	    fprintf(stderr, "Error on pixel check %d %d\n",
		    hist[frameToAlloc], win.pixelCount());
	    _myErrors++;
	  }

	  uint32 frameIndex = bp->frameIndex();
	  if (frameIndex != frameToAlloc) {
	    fprintf(stderr, "Expected frame index: %d  actual: %d\n",
		    frameToAlloc, frameIndex);
	    _myErrors++;
	  }
	}
	else {
	  fprintf(stderr, "Expected valid frame buffer pointer\n");
	  _myErrors++;
	}
      }
      
      if (postAllocSleep)
	usleep(postAllocSleep); // Time with buffer locked
    }
    else {
      _myErrors++;
      fprintf(stderr, "Error, unexpected error %d\n", error);
    }
  }
}

/*************************************************************************/
/*                                                                       */
/*              End of support class for thread-safe tests               */
/*                                                                       */
/*************************************************************************/

UT_VideoCache::UT_VideoCache(std::string movieFileName)
{
  fileName = movieFileName;
}

UT_VideoCache::~UT_VideoCache()
{
  printSuccessMessage("rcVideoCache test", mErrors);
}

uint32 UT_VideoCache::run()
{
  ctorTest();
  mappingTest();
  simpleAllocTest();
  prefetchTest();
  prefetchThreadTest();
  cacheFullTest();
  frameBufTest();
  dtorTest();
  threadSafeTest();

  return mErrors;
}

void UT_VideoCache::ctorTest()
{
  const uint32 frameCount = 10;

  rcVideoCache* cacheP = rcVideoCache::rcVideoCacheCtor(fileName, 5, true,
							false, false);

  rcUNITTEST_ASSERT(cacheP != 0);
  rcUNITTEST_ASSERT(cacheP->isValid());
  rcUNITTEST_ASSERT(cacheP->getFatalError() == eVideoCacheErrorOK);
  rcUNITTEST_ASSERT(cacheP->frameCount() == frameCount);

  rcVideoCache::rcVideoCacheDtor(cacheP);

  cacheP = rcVideoCache::rcVideoCacheCtor(fileName, 5, true, false, true);

  rcUNITTEST_ASSERT(cacheP != 0);
  rcUNITTEST_ASSERT(cacheP->isValid());
  rcUNITTEST_ASSERT(cacheP->getFatalError() == eVideoCacheErrorOK);
  rcUNITTEST_ASSERT(cacheP->frameCount() == frameCount);

  rcVideoCache::rcVideoCacheDtor(cacheP);

  cacheP = rcVideoCache::rcVideoCacheCtor(fileName, 10, true, false,
					  false);

  rcUNITTEST_ASSERT(cacheP != 0);
  rcUNITTEST_ASSERT(cacheP->isValid());
  rcUNITTEST_ASSERT(cacheP->getFatalError() == eVideoCacheErrorOK);
  rcUNITTEST_ASSERT(cacheP->frameCount() == frameCount);

  std::string nullFile;

  rcVideoCache* nullCacheP = rcVideoCache::rcVideoCacheCtor(nullFile, 10,
							    true, false, 
							    false);

  rcUNITTEST_ASSERT(nullCacheP != 0);
  rcUNITTEST_ASSERT(nullCacheP->isValid() == false);
  rcUNITTEST_ASSERT(nullCacheP->getFatalError()== eVideoCacheErrorFileInit);

  std::string badFile("xyzzy.dat");

  rcVideoCache* badCacheP = rcVideoCache::rcVideoCacheCtor(badFile, 10,
							   true, false,
							   false);

  rcUNITTEST_ASSERT(badCacheP->isValid() == false);
  rcUNITTEST_ASSERT(badCacheP->getFatalError() == eVideoCacheErrorFileInit);

  rcVideoCache::rcVideoCacheDtor(cacheP);
  rcVideoCache::rcVideoCacheDtor(nullCacheP);
  rcVideoCache::rcVideoCacheDtor(badCacheP);

  /* A set of tests to verify that the ctor properly calculates the
   * number of cache entries to be made available based upon the
   * maximum memory and cache size parameters. These numbers assume
   * the test movie contains 10 1K images.
   */
  vector<uint32> maxMemTests;
  maxMemTests.push_back(512);
  maxMemTests.push_back(5*1024);
  maxMemTests.push_back(9*1024);
  maxMemTests.push_back(10*1024);
  maxMemTests.push_back(11*1024);

  vector<uint32> frameCntTests;
  frameCntTests.push_back(0);
  frameCntTests.push_back(2);
  frameCntTests.push_back(5);
  frameCntTests.push_back(7);
  frameCntTests.push_back(9);
  frameCntTests.push_back(10);
  frameCntTests.push_back(12);

  vector<vector<uint32> >expResults(5);
  expResults[0].push_back(0);
  expResults[0].push_back(0);
  expResults[0].push_back(0);
  expResults[0].push_back(0);
  expResults[0].push_back(0);
  expResults[0].push_back(0);
  expResults[0].push_back(0);

  expResults[1].push_back(5);
  expResults[1].push_back(2);
  expResults[1].push_back(5);
  expResults[1].push_back(5);
  expResults[1].push_back(5);
  expResults[1].push_back(5);
  expResults[1].push_back(5);

  expResults[2].push_back(9);
  expResults[2].push_back(2);
  expResults[2].push_back(5);
  expResults[2].push_back(7);
  expResults[2].push_back(9);
  expResults[2].push_back(9);
  expResults[2].push_back(9);

  expResults[3].push_back(10);
  expResults[3].push_back(2);
  expResults[3].push_back(5);
  expResults[3].push_back(7);
  expResults[3].push_back(9);
  expResults[3].push_back(10);
  expResults[3].push_back(10);

  expResults[4].push_back(10);
  expResults[4].push_back(2);
  expResults[4].push_back(5);
  expResults[4].push_back(7);
  expResults[4].push_back(9);
  expResults[4].push_back(10);
  expResults[4].push_back(10);

  for (uint32 memIndex = 0; memIndex < maxMemTests.size(); memIndex++)
    for (uint32 fcIndex = 0; fcIndex < frameCntTests.size(); fcIndex++) {
      rcVideoCache* tcP = 
	rcVideoCache::rcVideoCacheCtor(fileName, frameCntTests[fcIndex],
				       false, false, false,
				       maxMemTests[memIndex]);

      if (expResults[memIndex][fcIndex] == 0) {
	rcUNITTEST_ASSERT(tcP->isValid() == false);
	rcUNITTEST_ASSERT(tcP->getFatalError() ==
			   eVideoCacheErrorSystemResources);
      }
      else {
	rcUNITTEST_ASSERT(tcP->isValid() == true);
	rcUNITTEST_ASSERT(tcP->cacheSize() ==
			   expResults[memIndex][fcIndex]);
      }

      rcVideoCache::rcVideoCacheDtor(tcP);
    }
}

void UT_VideoCache::mappingTest()
{
  const uint32 frameCount = 10;
  const uint32 extraCasesCount = 5;
  const uint32 totalCases = frameCount + extraCasesCount;
  
  double times[totalCases] = {
    2., 3., 4., 10., 100.,
    1000000000., 4294967296., 8589934592., 8589934593., 8589934594.,
    0., 6., 8., 2000000000., 9000000000.
  };

  rcVideoCache* cacheP = rcVideoCache::rcVideoCacheCtor(fileName, 5, true,
							false, false);
  rmAssert(frameCount == cacheP->frameCount());

  rcTimestamp actual;
  rcVideoCacheError error;
  rcVideoCacheStatus status;

  /* Test for functions frameIndexToTimestamp() and
   * timestampToFrameIndex().
   */
  for (uint32 i = 0; i < frameCount; i++) {
    status = cacheP->frameIndexToTimestamp(i, actual, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    if (status == eVideoCacheStatusOK)
      rcUNITTEST_ASSERT(actual.secs() == times[i]);

    uint32 actualIndex;
    status = cacheP->timestampToFrameIndex(actual, actualIndex, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    if (status == eVideoCacheStatusOK)
      rcUNITTEST_ASSERT(actualIndex == i);
  }

  /* Test for functions firstTimestamp() and lastTimestamp().
   */
  status = cacheP->firstTimestamp(actual, &error);
  rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
  if (status == eVideoCacheStatusOK)
    rcUNITTEST_ASSERT(actual.secs() == times[0]);
  
  status = cacheP->lastTimestamp(actual, &error);
  rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
  if (status == eVideoCacheStatusOK)
    rcUNITTEST_ASSERT(actual.secs() == times[frameCount-1]);
  
  /* Test for function closestTimestamp()
   */
  {
    double expected[totalCases] = {
      /* 00 */ 2.,
      /* 01 */ 3.,
      /* 02 */ 4.,
      /* 03 */ 10.,
      /* 04 */ 100.,
      /* 05 */ 1000000000.,
      /* 06 */ 4294967296., 
      /* 07 */ 8589934592.,
      /* 08 */ 8589934593.,
      /* 09 */ 8589934594.,
      /* 10 */ 2.,
      /* 11 */ 4.,
      /* 12 */ 10.,
      /* 13 */ 1000000000.,
      /* 14 */ 8589934594.
    };

    for (uint32 i = 0; i < totalCases; i++) {
      status =
        cacheP->closestTimestamp(rcTimestamp::from_seconds(times[i]), actual, &error);

      rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
      if (status == eVideoCacheStatusOK)
      {
          rcUNITTEST_ASSERT(actual == rcTimestamp::from_seconds(expected[i]));
      }
      
    }
  }
  
  /* Test for function prevTimestamp()
   */
  {
    utExpMapResult expected[totalCases] = {
      /* 00 */ { eVideoCacheErrorNoSuchFrame, eVideoCacheStatusError, 0 },
      /* 01 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 2. },
      /* 02 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 3. },
      /* 03 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 4. },
      /* 04 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 10. },
      /* 05 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 100. },
      /* 06 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 1000000000. },
      /* 07 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 4294967296. },
      /* 08 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 8589934592. },
      /* 09 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 8589934593. },
      /* 10 */ { eVideoCacheErrorNoSuchFrame, eVideoCacheStatusError, 0. },
      /* 11 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 4. },
      /* 12 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 4. },
      /* 13 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 1000000000. },
      /* 14 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 8589934594. }
    };

    for (uint32 i = 0; i < totalCases; i++) {
      error = eVideoCacheErrorFileInit;
      status =
        cacheP->prevTimestamp(rcTimestamp::from_seconds (times[i]), actual, &error);

      rcUNITTEST_ASSERT(status == expected[i].expStatus);
      if (expected[i].expStatus == eVideoCacheStatusError)
	rcUNITTEST_ASSERT(error == expected[i].expError);
      else
          rcUNITTEST_ASSERT(actual == rcTimestamp::from_seconds (expected[i].expTime));
    }
  }
  
  /* Test for function nextTimestamp()
   */
  {
    utExpMapResult expected[totalCases] = {
      /* 00 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 3. },
      /* 01 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 4. },
      /* 02 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 10. },
      /* 03 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 100. },
      /* 04 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 1000000000. },
      /* 05 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 4294967296. },
      /* 06 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 8589934592. },
      /* 07 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 8589934593. },
      /* 08 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 8589934594. },
      /* 09 */ { eVideoCacheErrorNoSuchFrame, eVideoCacheStatusError, 0. },
      /* 10 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 2. },
      /* 11 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 10. },
      /* 12 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 10. },
      /* 13 */ { eVideoCacheErrorOK, eVideoCacheStatusOK, 4294967296. },
      /* 14 */ { eVideoCacheErrorNoSuchFrame, eVideoCacheStatusError, 0. }
    };

    for (uint32 i = 0; i < totalCases; i++) {
      error = eVideoCacheErrorFileInit;
      status =
	cacheP->nextTimestamp(rcTimestamp::from_seconds (times[i]), actual, &error);

      rcUNITTEST_ASSERT(status == expected[i].expStatus);
      if (expected[i].expStatus == eVideoCacheStatusError)
	rcUNITTEST_ASSERT(error == expected[i].expError);
      else
          rcUNITTEST_ASSERT(actual == rcTimestamp::from_seconds (expected[i].expTime));
    }
  }

  rcVideoCache::rcVideoCacheDtor(cacheP);
}

void UT_VideoCache::simpleAllocTest()
{
  /* Make simple test of getFrame functions. Note that the cache is
   * created with only 1 cache entry to verify that cache elements are
   * getting freed.
   */
  rcVideoCache* cacheP = rcVideoCache::rcVideoCacheCtor(fileName, 1, true,
							false, false);
  rc256BinHist hist(256);

  for (uint32 i = 0; i < cacheP->frameCount(); i++) {
    rcVideoCacheError error;
    rcVideoCacheStatus status;
    rcSharedFrameBufPtr buf;
    rmAssert(buf.refCount() == 0);
    
    status = cacheP->getFrame(i, buf, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    if (status == eVideoCacheStatusOK) {
      rcUNITTEST_ASSERT(buf.refCount() == 2);
      rcUNITTEST_ASSERT(buf->width() == 32);
      rcUNITTEST_ASSERT(buf->height() == 32);
      rcUNITTEST_ASSERT(buf->depth() == rcPixel8);
        
#ifdef LEGACY_COLORMAP        
      rcUNITTEST_ASSERT(buf->colorMapSize() == 256);
        
      if (buf->colorMapSize() == 256) {
	const uint32* colorMap = buf->colorMap();
	rmAssert(colorMap);
	for (uint32 c = 0; c < 256; c++)
	  rcUNITTEST_ASSERT(colorMap[c] == rfRgb(c,c,c));
      }
#endif
        
      rcWindow win(buf);
      rcUNITTEST_ASSERT(buf.refCount() == 3);

      rfGenDepth8Histogram(win, hist);
      rcUNITTEST_ASSERT(hist[i] == win.pixelCount());
    }
  }
  
  for (uint32 i = 0; i < cacheP->frameCount(); i++) {
    rcVideoCacheError error;
    rcVideoCacheStatus status;
    rcSharedFrameBufPtr buf;
    rmAssert(buf.refCount() == 0);

    rcTimestamp time;
    status = cacheP->frameIndexToTimestamp(i, time, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    status = cacheP->getFrame(time, buf, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    if (status == eVideoCacheStatusOK) {
      rcUNITTEST_ASSERT(buf.refCount() == 2);
      rcUNITTEST_ASSERT(buf->width() == 32);
      rcUNITTEST_ASSERT(buf->height() == 32);
      rcUNITTEST_ASSERT(buf->depth() == rcPixel8);
     
#ifdef LEGACY_COLORMAP           
      rcUNITTEST_ASSERT(buf->colorMapSize() == 256);
      if (buf->colorMapSize() == 256) {
	const uint32* colorMap = buf->colorMap();
	rmAssert(colorMap);
	for (uint32 c = 0; c < 256; c++)
	  rcUNITTEST_ASSERT(colorMap[c] == rfRgb(c,c,c));
      }
#endif        
        
      rcWindow win(buf);
      rcUNITTEST_ASSERT(buf.refCount() == 3);

      rfGenDepth8Histogram(win, hist);
      rcUNITTEST_ASSERT(hist[i] == win.pixelCount());
    }
  }

  rcVideoCache::rcVideoCacheDtor(cacheP);
}

// Test for prefetch thread operation
void UT_VideoCache::prefetchThreadTest()
{
    fprintf( stderr, "%-38s", "rcVideoCache prefetch thread ctor");
    // Call ctor and dtor rapdily to test prefetch thread race conditions
    for ( uint32 i = 0;  i < 4096; ++i ) {
        rcVideoCache* cacheP =
            rcVideoCache::rcVideoCacheCtor(fileName, 0, true, false, true, 0);
        // Warning: exeution may deadlock here if prefetch thread
        // hasn't been properly initialized when dtor is called
        rcVideoCache::rcVideoCacheDtor(cacheP);
    }
    fprintf( stderr, " OK\n" );
}

void UT_VideoCache::prefetchTest()
{
  const uint32 frameCount = 10;

  rc256BinHist hist(256);

  rcVideoCache* cacheP =
    rcVideoCache::rcVideoCacheCtor(fileName, 0, true, false, true);
  
  rcSharedFrameBufPtr frameBuf[frameCount];
  rcVideoCacheError error;
  rcVideoCacheStatus status;

  rcUNITTEST_ASSERT(cacheP->frameCount() == frameCount);

  for (uint32 i = 0; i < frameCount; i++) {
    status = cacheP->getFrame(i, frameBuf[i], &error, false);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    if (status != eVideoCacheStatusOK)
      return; // No point continuing if we've already screwed up
  }

  for (uint32 i = 0; i < frameCount; i++)
    frameBuf[i].prefetch();
  
  usleep(200000);
    chronometer timer;
  for (uint32 i = 0; i < frameCount; i++)
    frameBuf[i].lock();

    double prefetchTime = timer.getTime ();    

  for (uint32 i = 0; i < frameCount; i++) {
    rcWindow win(frameBuf[i]);
    rfGenDepth8Histogram(win, hist);
    rcUNITTEST_ASSERT(hist[i] == win.pixelCount());
  }



  rcVideoCache::rcVideoCacheDtor(cacheP);

  cacheP = rcVideoCache::rcVideoCacheCtor(fileName, 0, true, false, true);

  for (uint32 i = 0; i < frameCount; i++) {
    status = cacheP->getFrame(i, frameBuf[i], &error, false);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    if (status != eVideoCacheStatusOK)
      return; // No point continuing if we've already screwed up
  }

   
    timer.reset ();
  for (uint32 i = 0; i < frameCount; i++)
    frameBuf[i].lock();
    double noPrefetchTime = timer.getTime ();

  for (uint32 i = 0; i < frameCount; i++) {
    rcWindow win(frameBuf[i]);
    rfGenDepth8Histogram(win, hist);
    rcUNITTEST_ASSERT(hist[i] == win.pixelCount());
  }



  rcVideoCache::rcVideoCacheDtor(cacheP);

  cout << "Performance: Cache Prefetch " << prefetchTime
       << " us, Time Per Frame " << (prefetchTime / frameCount)
       << " us" << endl;

  cout << "Performance: No Cache Prefetch " << noPrefetchTime
       << " us, Time Per Frame " << (noPrefetchTime / frameCount)
       << " us" << endl;
}

void UT_VideoCache::cacheFullTest()
{
  /* Test cases around where cache gets filled. 
   */
  const uint32 cacheSize = 5, overflowSize = 2;
  const uint32 totalCases = cacheSize + overflowSize;
  
  rcVideoCache* cacheP =
    rcVideoCache::rcVideoCacheCtor(fileName, cacheSize, true, false,
				   false);
  rc256BinHist hist(256);
  rcSharedFrameBufPtr bp[totalCases];
  rcVideoCacheError error;
  rcVideoCacheStatus status;

  /* Fill up cache with locked entries.
   */
  for (uint32 i = 0; i < cacheSize; i++) {
    status = cacheP->getFrame(i, bp[i], &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    if (status != eVideoCacheStatusOK)
      return; // No point continuing if we've already screwed up
    rcUNITTEST_ASSERT(bp[i].refCount() == 2);
    rcWindow win(bp[i]);
    rcUNITTEST_ASSERT(bp[i].refCount() == 3);
    
    rfGenDepth8Histogram(win, hist);
    rcUNITTEST_ASSERT(hist[i] == win.pixelCount());
  }

  /* Verify overflow entries are available.
   */
  for (uint32 i = cacheSize; i < totalCases; i++) {
    status = cacheP->getFrame(i, bp[i], &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    if (status == eVideoCacheStatusOK) {
      rcUNITTEST_ASSERT(bp[i].refCount() == 2);
      rcWindow win(bp[i]);
      rcUNITTEST_ASSERT(bp[i].refCount() == 3);
    
      rfGenDepth8Histogram(win, hist);
      rcUNITTEST_ASSERT(hist[i] == win.pixelCount());
    }
  }

  /* Unlock some elements and verify that new frames can be allocated.
   */
  for (uint32 i = cacheSize; i < totalCases; i++) {
    bp[i - cacheSize].unlock();
    status = cacheP->getFrame(i, bp[i], &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    if (status == eVideoCacheStatusOK) {
      rcUNITTEST_ASSERT(bp[i].refCount() == 2);
      rcWindow win(bp[i]);
      rcUNITTEST_ASSERT(bp[i].refCount() == 3);
    
      rfGenDepth8Histogram(win, hist);
      rcUNITTEST_ASSERT(hist[i] == win.pixelCount());
    }
  }

  /* Try to lock previously unlocked elements and verify that the
   * frames can be accessed.
   */
  for (uint32 i = 0; i < overflowSize; i++) {
    bp[i].lock();
    rcUNITTEST_ASSERT(bp[i].refCount() == 2);
    if (bp[i].refCount() == 2) {
      rcWindow win(bp[i]);
      rcUNITTEST_ASSERT(bp[i].refCount() == 3);
    
      rfGenDepth8Histogram(win, hist);
      rcUNITTEST_ASSERT(hist[i] == win.pixelCount());
    }
  }

  /* Now unlock some frames and verify that locks will now work.
   */
  for (uint32 i = 0; i < overflowSize; i++) {
    bp[i + cacheSize].unlock();
    bp[i].lock();
    rcUNITTEST_ASSERT(bp[i].refCount() == 2);
    if (bp[i].refCount() == 2) {
      rcWindow win(bp[i]);
      rcUNITTEST_ASSERT(bp[i].refCount() == 3);
    
      rfGenDepth8Histogram(win, hist);
      rcUNITTEST_ASSERT(hist[i] == win.pixelCount());
    }
  }

  rcVideoCache::rcVideoCacheDtor(cacheP);
}

void UT_VideoCache::frameBufTest()
{
  /* Test manipulating rcSharedFrameBufPtrs using both cached and
   * uncached frames.
   */
  const uint32 uncachedPixelValue = 0xA;
  const uint32 cachedPixelValue = 0x0;
  const uint32 cacheSize = 5;
  rcVideoCache* cacheP =
    rcVideoCache::rcVideoCacheCtor(fileName, cacheSize, true, false,
				   false);
  rcSharedFrameBufPtr cachedBp, uncachedBp, changingBp;
  rcVideoCacheError error;
  rcVideoCacheStatus status;

  rcUNITTEST_ASSERT(cachedBp.refCount() == 0);
  rcUNITTEST_ASSERT(uncachedBp.refCount() == 0);
  rcUNITTEST_ASSERT(changingBp.refCount() == 0);

  /* First test: Set up cachedBp and uncachedBp. Set changingBp to
   *             point towards uncached frame.
   */
  status = cacheP->getFrame(0, cachedBp, &error);

  rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
  if (status != eVideoCacheStatusOK)
    return; // No point in continuing test if this doesn't work.

  rcUNITTEST_ASSERT(cachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(cachedBp->getPixel(0, 0) == cachedPixelValue);
  
  uncachedBp = new rcFrame(32, 32, rcPixel8);
  rcUNITTEST_ASSERT(uncachedBp.refCount() == 1);
  rcUNITTEST_ASSERT(uncachedBp != cachedBp);

  rcWindow win(uncachedBp);
  rcUNITTEST_ASSERT(uncachedBp.refCount() == 2);

  win.setAllPixels(uncachedPixelValue);
  rcUNITTEST_ASSERT(uncachedBp->getPixel(0, 0) == uncachedPixelValue);

  rcUNITTEST_ASSERT(uncachedBp != changingBp);
  changingBp = uncachedBp;
  rcUNITTEST_ASSERT(changingBp.refCount() == 3);
  rcUNITTEST_ASSERT(uncachedBp.refCount() == 3);
  rcUNITTEST_ASSERT(uncachedBp == changingBp);
  rcUNITTEST_ASSERT(changingBp->getPixel(0, 0) == uncachedPixelValue);

  /* Second test: Point changingBp towards cached frame.
   */
  changingBp = cachedBp;
  rcUNITTEST_ASSERT(cachedBp.refCount() == 3);
  rcUNITTEST_ASSERT(changingBp.refCount() == 3);
  rcUNITTEST_ASSERT(uncachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(cachedBp->getPixel(0, 0) == cachedPixelValue);
  rcUNITTEST_ASSERT(changingBp->getPixel(0, 0) == cachedPixelValue);
  rcUNITTEST_ASSERT(uncachedBp->getPixel(0, 0) == uncachedPixelValue);
  rcUNITTEST_ASSERT(cachedBp == changingBp);
  rcUNITTEST_ASSERT(uncachedBp != changingBp);

  /* Third test: Point changingBp back towards uncached frame.
   */
  changingBp = uncachedBp;
  rcUNITTEST_ASSERT(cachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(changingBp.refCount() == 3);
  rcUNITTEST_ASSERT(uncachedBp.refCount() == 3);
  rcUNITTEST_ASSERT(cachedBp->getPixel(0, 0) == cachedPixelValue);
  rcUNITTEST_ASSERT(changingBp->getPixel(0, 0) == uncachedPixelValue);
  rcUNITTEST_ASSERT(uncachedBp->getPixel(0, 0) == uncachedPixelValue);
  rcUNITTEST_ASSERT(cachedBp != changingBp);
  rcUNITTEST_ASSERT(uncachedBp == changingBp);

  /* Fourth test: Point changingBp towards cached frame and then set
   *              it to NULL. Verify that lock/unlock doesn't change
   *              anything.
   */
  changingBp = cachedBp;
  changingBp = 0;
  rcUNITTEST_ASSERT(cachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(changingBp.refCount() == 0);
  rcUNITTEST_ASSERT(uncachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(cachedBp->getPixel(0, 0) == cachedPixelValue);
  rcUNITTEST_ASSERT(uncachedBp->getPixel(0, 0) == uncachedPixelValue);
  rcUNITTEST_ASSERT(changingBp == 0);
  rcUNITTEST_ASSERT(cachedBp != changingBp);
  rcUNITTEST_ASSERT(uncachedBp != changingBp);
  
  changingBp.lock();
  rcUNITTEST_ASSERT(cachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(changingBp.refCount() == 0);
  rcUNITTEST_ASSERT(uncachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(cachedBp->getPixel(0, 0) == cachedPixelValue);
  rcUNITTEST_ASSERT(uncachedBp->getPixel(0, 0) == uncachedPixelValue);
  rcUNITTEST_ASSERT(changingBp == 0);
  rcUNITTEST_ASSERT(cachedBp != changingBp);
  rcUNITTEST_ASSERT(uncachedBp != changingBp);
  
  changingBp.unlock();
  rcUNITTEST_ASSERT(cachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(changingBp.refCount() == 0);
  rcUNITTEST_ASSERT(uncachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(cachedBp->getPixel(0, 0) == cachedPixelValue);
  rcUNITTEST_ASSERT(uncachedBp->getPixel(0, 0) == uncachedPixelValue);
  rcUNITTEST_ASSERT(changingBp == 0);
  rcUNITTEST_ASSERT(cachedBp != changingBp);
  rcUNITTEST_ASSERT(uncachedBp != changingBp);

  /* Fifth test: Point changingBp towards cached frame and then verify
   *             that unlock causes reference count to decrement, but
   *             using the reference object causes the count to increment
   *             back up.
   *
   *             Also, check that assigning/constructing a
   *             rcSharedFrameBufPtr using a cached rcFrame* works
   *             (though anyone actually doing this should burn in
   *             hell).
   */
  changingBp = cachedBp;
  rcUNITTEST_ASSERT(cachedBp.refCount() == 3);
  rcUNITTEST_ASSERT(changingBp.refCount() == 3);

  changingBp.unlock();
  rcUNITTEST_ASSERT(cachedBp.refCount() == 2);
  rcUNITTEST_ASSERT(changingBp.refCount() == 3);
  rcUNITTEST_ASSERT(cachedBp.refCount() == 3);

  {
    changingBp = uncachedBp;
    rcUNITTEST_ASSERT(changingBp->getPixel(0, 0) == uncachedPixelValue);
    rcUNITTEST_ASSERT(cachedBp.refCount() == 2);
    rcFrame* frame1 = cachedBp;
    rcUNITTEST_ASSERT(frame1->getPixel(0, 0) == cachedPixelValue);
    changingBp = frame1;
    rcUNITTEST_ASSERT(changingBp->getPixel(0, 0) == cachedPixelValue);
    rcUNITTEST_ASSERT(cachedBp.refCount() == 3);

    rcSharedFrameBufPtr cachedToo(frame1);
    rcUNITTEST_ASSERT(cachedToo->getPixel(0, 0) == cachedPixelValue);
    rcUNITTEST_ASSERT(cachedBp.refCount() == 4);
  }

  /* Sixth test: Check that ctor works correctly in all cases:
   *  - No arg
   *  - Null rcFrame*
   *  - non-null rcFrame*
   *  - non-null rcFrame* cached
   *  - Null rcSharedFrameBufPtr
   *  - Non-null uncached rcSharedFrameBufPtr
   *  - Non-null cached, locked rcSharedFrameBufPtr
   *  - Non-null cached, unlocked rcSharedFrameBufPtr
   */
  {
    rcSharedFrameBufPtr null;
    rcUNITTEST_ASSERT(null == 0);
    rcUNITTEST_ASSERT(null.refCount() == 0);

    rcSharedFrameBufPtr null2(null);
    rcUNITTEST_ASSERT(null2 == 0);
    rcUNITTEST_ASSERT(null2.refCount() == 0);
  }
  {
    rcFrame* nullFrame = 0;
    rcSharedFrameBufPtr null(nullFrame);
    rcUNITTEST_ASSERT(null == 0);
    rcUNITTEST_ASSERT(null.refCount() == 0);
  }
  {
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr nonnull(nonnullFrame);
    rcUNITTEST_ASSERT(nonnullFrame == nonnull);
    rcUNITTEST_ASSERT(nonnull.refCount() == 1);
  }
  {
    rcSharedFrameBufPtr nonnull;
    status = cacheP->getFrame(1, nonnull, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* cachedFrame = nonnull;
    rcSharedFrameBufPtr nonnull2(cachedFrame);
    rcUNITTEST_ASSERT(nonnull2 == nonnull);
    rcUNITTEST_ASSERT(nonnull2.refCount() == 3);
  }
  {
    rcSharedFrameBufPtr nonnull(new rcFrame(32, 32, rcPixel8));
    rcSharedFrameBufPtr nonnull2(nonnull);
    rcUNITTEST_ASSERT(nonnull2 == nonnull);
    rcUNITTEST_ASSERT(nonnull2.refCount() == 2);
  }
  {
    rcSharedFrameBufPtr nonnull;
    status = cacheP->getFrame(1, nonnull, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcSharedFrameBufPtr nonnull2(nonnull);
    rcUNITTEST_ASSERT(nonnull2 == nonnull);
    rcUNITTEST_ASSERT(nonnull2.refCount() == 3);
  }
  {
    rcSharedFrameBufPtr nonnull;
    status = cacheP->getFrame(1, nonnull, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcSharedFrameBufPtr nonnullcopy(nonnull);
    nonnull.unlock();
    rcSharedFrameBufPtr nonnull2(nonnull);
    rcUNITTEST_ASSERT(nonnullcopy.refCount() == 2);
    rcUNITTEST_ASSERT(nonnull2 == nonnull);
    rcUNITTEST_ASSERT(nonnull2.refCount() == 3);
  }

  /* Seventh test - Check that assignment works correctly in all cases:
   *  - Null rcFrame*
   *  - Non-null rcFrame*
   *  - Non-null rcFrame* cached
   *  - Null rcSharedFrameBufPtr
   *  - Non-null uncached rcSharedFrameBufPtr
   *  - Non-null cached, locked rcSharedFrameBufPtr
   *  - Non-null cached, unlocked rcSharedFrameBufPtr
   *
   * All of the above cases must be tested as source in permutation
   * with all rcSharedFrameBufPtr cases above tested as destination.
   */
  {
    rcSharedFrameBufPtr destNull;

    rcFrame* nullFrame = 0;
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr srcNull;
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));

    destNull = nonnullFrame;
    rcUNITTEST_ASSERT(nonnullFrame == destNull);
    rcUNITTEST_ASSERT(destNull.refCount() == 1);

    destNull = nullFrame;
    rcUNITTEST_ASSERT(destNull == 0);
    rcUNITTEST_ASSERT(destNull.refCount() == 0);

    destNull = nonnullFrameC;
    rcUNITTEST_ASSERT(nonnullFrameC == destNull);
    rcUNITTEST_ASSERT(destNull.refCount() == 3);
    
    destNull = 0;
    rcUNITTEST_ASSERT(destNull == 0);
    rcUNITTEST_ASSERT(forFramePtr.refCount() == 2);
    
    destNull = srcNull;
    rcUNITTEST_ASSERT(destNull == 0);
    rcUNITTEST_ASSERT(destNull.refCount() == 0);

    destNull = nonnullUnc;
    rcUNITTEST_ASSERT(destNull == nonnullUnc);
    rcUNITTEST_ASSERT(destNull.refCount() == 2);
    
    destNull = 0;
    rcUNITTEST_ASSERT(destNull == 0);
    rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);
    
    destNull = nonnullC;
    rcUNITTEST_ASSERT(destNull == nonnullC);
    rcUNITTEST_ASSERT(destNull.refCount() == 3);
    
    destNull = 0;
    rcUNITTEST_ASSERT(destNull == 0);
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);
    
    nonnullC.unlock();
    destNull = nonnullC;
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);
    rcUNITTEST_ASSERT(destNull == nonnullC);
    rcUNITTEST_ASSERT(destNull.refCount() == 3);
  }

  {
    rcFrame* frame1 = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr destNonnullUnc(frame1);
    rcUNITTEST_ASSERT(frame1 == destNonnullUnc);
    
    rcFrame* nullFrame = 0;
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr srcNull;
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));

    destNonnullUnc = nonnullFrame;
    rcUNITTEST_ASSERT(destNonnullUnc.refCount() == 1);
    rcUNITTEST_ASSERT(nonnullFrame == destNonnullUnc);

    destNonnullUnc = nullFrame;
    rcUNITTEST_ASSERT(destNonnullUnc == 0);
    rcUNITTEST_ASSERT(destNonnullUnc.refCount() == 0);
    frame1 = new rcFrame(32, 32, rcPixel8);
    destNonnullUnc = frame1;
    rcUNITTEST_ASSERT(frame1 == destNonnullUnc);

    destNonnullUnc = nonnullFrameC;
    rcUNITTEST_ASSERT(nonnullFrameC == destNonnullUnc);
    rcUNITTEST_ASSERT(destNonnullUnc.refCount() == 3);
    
    destNonnullUnc = 0;
    rcUNITTEST_ASSERT(destNonnullUnc == 0);
    rcUNITTEST_ASSERT(forFramePtr.refCount() == 2);
    
    destNonnullUnc = nonnullUnc;
    rcUNITTEST_ASSERT(destNonnullUnc == nonnullUnc);
    rcUNITTEST_ASSERT(destNonnullUnc.refCount() == 2);
    frame1 = new rcFrame(32, 32, rcPixel8);
    destNonnullUnc = frame1;
    rcUNITTEST_ASSERT(frame1 == destNonnullUnc);
    rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);

    destNonnullUnc = nonnullC;
    rcUNITTEST_ASSERT(destNonnullUnc == nonnullC);
    rcUNITTEST_ASSERT(destNonnullUnc.refCount() == 3);
    frame1 = new rcFrame(32, 32, rcPixel8);
    destNonnullUnc = frame1;
    rcUNITTEST_ASSERT(frame1 == destNonnullUnc);
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);

    nonnullC.unlock();
    destNonnullUnc = nonnullC;
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);
    rcUNITTEST_ASSERT(destNonnullUnc == nonnullC);
    rcUNITTEST_ASSERT(destNonnullUnc.refCount() == 3);
  }

  {
    rcSharedFrameBufPtr destNonnullC;
    status = cacheP->getFrame(2, destNonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    
    rcSharedFrameBufPtr frame2Copy(destNonnullC);
    rcUNITTEST_ASSERT(destNonnullC.refCount() == 3);

    rcFrame* nullFrame = 0;
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr srcNull;
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));

    destNonnullC = nullFrame;
    rcUNITTEST_ASSERT(destNonnullC == 0);
    rcUNITTEST_ASSERT(destNonnullC.refCount() == 0);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullC = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullC == frame2Copy);

    destNonnullC = nonnullFrame;
    rcUNITTEST_ASSERT(destNonnullC.refCount() == 1);
    rcUNITTEST_ASSERT(nonnullFrame == destNonnullC);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullC = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullC == frame2Copy);

    destNonnullC = nonnullFrameC;
    rcUNITTEST_ASSERT(nonnullFrameC == destNonnullC);
    rcUNITTEST_ASSERT(destNonnullC.refCount() == 3);
    
    destNonnullC = 0;
    rcUNITTEST_ASSERT(destNonnullC == 0);
    rcUNITTEST_ASSERT(forFramePtr.refCount() == 2);
    
    destNonnullC = nonnullUnc;
    rcUNITTEST_ASSERT(destNonnullC.refCount() == 2);
    rcUNITTEST_ASSERT(destNonnullC == nonnullUnc);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullC = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullC == frame2Copy);
    rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);

    destNonnullC = nonnullC;
    rcUNITTEST_ASSERT(destNonnullC.refCount() == 3);
    rcUNITTEST_ASSERT(destNonnullC == nonnullC);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullC = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullC == frame2Copy);
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);

    nonnullC.unlock();
    destNonnullC = nonnullC;
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);
    rcUNITTEST_ASSERT(destNonnullC == nonnullC);
    rcUNITTEST_ASSERT(destNonnullC.refCount() == 3);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullC = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullC == frame2Copy);
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);
  }

  {
    rcSharedFrameBufPtr destNonnullCunlocked;
    status = cacheP->getFrame(2, destNonnullCunlocked, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    
    rcSharedFrameBufPtr frame2Copy(destNonnullCunlocked);
    rcUNITTEST_ASSERT(destNonnullCunlocked.refCount() == 3);

    rcFrame* nullFrame = 0;
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr srcNull;
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));

    destNonnullCunlocked.unlock();
    destNonnullCunlocked = nullFrame;
    rcUNITTEST_ASSERT(destNonnullCunlocked == 0);
    rcUNITTEST_ASSERT(destNonnullCunlocked.refCount() == 0);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullCunlocked = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullCunlocked == frame2Copy);

    destNonnullCunlocked.unlock();
    destNonnullCunlocked = nonnullFrame;
    rcUNITTEST_ASSERT(destNonnullCunlocked.refCount() == 1);
    rcUNITTEST_ASSERT(nonnullFrame == destNonnullCunlocked);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullCunlocked = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullCunlocked == frame2Copy);

    destNonnullCunlocked.unlock();
    destNonnullCunlocked = nonnullFrameC;
    rcUNITTEST_ASSERT(nonnullFrameC == destNonnullCunlocked);
    rcUNITTEST_ASSERT(destNonnullCunlocked.refCount() == 3);
    
    destNonnullCunlocked.unlock();
    destNonnullCunlocked = 0;
    rcUNITTEST_ASSERT(destNonnullCunlocked == 0);
    rcUNITTEST_ASSERT(forFramePtr.refCount() == 2);
    
    destNonnullCunlocked.unlock();
    destNonnullCunlocked = nonnullUnc;
    rcUNITTEST_ASSERT(destNonnullCunlocked.refCount() == 2);
    rcUNITTEST_ASSERT(destNonnullCunlocked == nonnullUnc);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullCunlocked = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullCunlocked == frame2Copy);
    rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);

    destNonnullCunlocked.unlock();
    destNonnullCunlocked = nonnullC;
    rcUNITTEST_ASSERT(destNonnullCunlocked.refCount() == 3);
    rcUNITTEST_ASSERT(destNonnullCunlocked == nonnullC);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullCunlocked = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullCunlocked == frame2Copy);
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);

    destNonnullCunlocked.unlock();
    nonnullC.unlock();
    destNonnullCunlocked = nonnullC;
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);
    rcUNITTEST_ASSERT(destNonnullCunlocked == nonnullC);
    rcUNITTEST_ASSERT(destNonnullCunlocked.refCount() == 3);
    rcUNITTEST_ASSERT(frame2Copy.refCount() == 2);
    destNonnullCunlocked = frame2Copy;
    rcUNITTEST_ASSERT(destNonnullCunlocked == frame2Copy);
    rcUNITTEST_ASSERT(nonnullC.refCount() == 2);
  }

  /* Eighth test - Check that comparison works correctly in all cases:
   *  - Null rcFrame*
   *  - Non-null rcFrame*
   *  - Non-null rcFrame* cached
   *  - Null rcSharedFrameBufPtr
   *  - Non-null uncached rcSharedFrameBufPtr
   *  - Non-null cached, locked rcSharedFrameBufPtr
   *  - Non-null cached, unlocked rcSharedFrameBufPtr
   *
   * All permutations of above cases must be tested for both left-hand
   * and right-hand sides of comparison.
   */
  {
    rcFrame* nullFrame = 0;

    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr rightNull;
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);

    rcUNITTEST_ASSERT(nullFrame == nullFrame);
    rcUNITTEST_ASSERT((nullFrame != nullFrame) == false);

    rcUNITTEST_ASSERT(nullFrame != nonnullFrame);
    rcUNITTEST_ASSERT((nullFrame == nonnullFrame) == false);

    rcUNITTEST_ASSERT(nullFrame != nonnullFrameC);
    rcUNITTEST_ASSERT((nullFrame == nonnullFrameC) == false);

    rcUNITTEST_ASSERT(nullFrame == rightNull);
    rcUNITTEST_ASSERT((nullFrame != rightNull) == false);

    rcUNITTEST_ASSERT(nullFrame != nonnullUnc);
    rcUNITTEST_ASSERT((nullFrame == nonnullUnc) == false);
    
    rcUNITTEST_ASSERT(nullFrame != nonnullC);
    rcUNITTEST_ASSERT((nullFrame == nonnullC) == false);

    nonnullC.unlock();
    rcUNITTEST_ASSERT(nullFrame != nonnullC);
    rcUNITTEST_ASSERT((nullFrame == nonnullC) == false);

    delete nonnullFrame;
  }

  {
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);

    rcFrame* nullFrame = 0;
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr rightNull;
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);

    rcUNITTEST_ASSERT(nonnullFrame != nullFrame);
    rcUNITTEST_ASSERT((nonnullFrame == nullFrame) == false);

    rcUNITTEST_ASSERT(nonnullFrame != nonnullFrameC);
    rcUNITTEST_ASSERT((nonnullFrame == nonnullFrameC) == false);

    rcUNITTEST_ASSERT(nonnullFrame == nonnullFrame);
    rcUNITTEST_ASSERT((nonnullFrame != nonnullFrame) == false);

    rcUNITTEST_ASSERT(nonnullFrame != rightNull);
    rcUNITTEST_ASSERT((nonnullFrame == rightNull) == false);

    rcUNITTEST_ASSERT(nonnullFrame != nonnullUnc);
    rcUNITTEST_ASSERT((nonnullFrame == nonnullUnc) == false);
    
    rcUNITTEST_ASSERT(nonnullFrame != nonnullC);
    rcUNITTEST_ASSERT((nonnullFrame == nonnullC) == false);

    nonnullC.unlock();
    rcUNITTEST_ASSERT(nonnullFrame != nonnullC);
    rcUNITTEST_ASSERT((nonnullFrame == nonnullC) == false);

    delete nonnullFrame;
  }

  {
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;

    rcFrame* nullFrame = 0;
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr rightNull;
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);

    rcUNITTEST_ASSERT(nonnullFrameC != nullFrame);
    rcUNITTEST_ASSERT((nonnullFrameC == nullFrame) == false);

    rcUNITTEST_ASSERT(nonnullFrameC != nonnullFrame);
    rcUNITTEST_ASSERT((nonnullFrameC == nonnullFrame) == false);

    rcUNITTEST_ASSERT(nonnullFrameC == nonnullFrameC);
    rcUNITTEST_ASSERT((nonnullFrameC != nonnullFrameC) == false);

    rcUNITTEST_ASSERT(nonnullFrameC != rightNull);
    rcUNITTEST_ASSERT((nonnullFrameC == rightNull) == false);

    rcUNITTEST_ASSERT(nonnullFrameC != nonnullUnc);
    rcUNITTEST_ASSERT((nonnullFrameC == nonnullUnc) == false);
    
    rcUNITTEST_ASSERT(nonnullFrameC != nonnullC);
    rcUNITTEST_ASSERT((nonnullFrameC == nonnullC) == false);

    nonnullC.unlock();
    rcUNITTEST_ASSERT(nonnullFrameC != nonnullC);
    rcUNITTEST_ASSERT((nonnullFrameC == nonnullC) == false);

    delete nonnullFrame;
  }

  {
    rcSharedFrameBufPtr leftNull;

    rcFrame* nullFrame = 0;
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr rightNull;
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);

    rcUNITTEST_ASSERT(leftNull.operator==(nullFrame));
    rcUNITTEST_ASSERT((leftNull.operator!=(nullFrame)) == false);

    rcUNITTEST_ASSERT(leftNull.operator!=(nonnullFrame));
    rcUNITTEST_ASSERT((leftNull.operator==(nonnullFrame)) == false);

    rcUNITTEST_ASSERT(leftNull.operator!=(nonnullFrameC));
    rcUNITTEST_ASSERT((leftNull.operator==(nonnullFrameC)) == false);

    rcUNITTEST_ASSERT(leftNull == rightNull);
    rcUNITTEST_ASSERT((leftNull != rightNull) == false);

    rcUNITTEST_ASSERT(leftNull != nonnullUnc);
    rcUNITTEST_ASSERT((leftNull == nonnullUnc) == false);
    
    rcUNITTEST_ASSERT(leftNull != nonnullC);
    rcUNITTEST_ASSERT((leftNull == nonnullC) == false);

    nonnullC.unlock();
    rcUNITTEST_ASSERT(leftNull != nonnullC);
    rcUNITTEST_ASSERT((leftNull == nonnullC) == false);

    delete nonnullFrame;
  }

  {
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));

    rcFrame* nullFrame = 0;
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr rightNull;
    rcSharedFrameBufPtr nonnullUncRight(new rcFrame(32, 32, rcPixel8));
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);

    rcUNITTEST_ASSERT(nonnullUnc.operator!=(nullFrame));
    rcUNITTEST_ASSERT((nonnullUnc.operator==(nullFrame)) == false);

    rcUNITTEST_ASSERT(nonnullUnc.operator!=(nonnullFrame));
    rcUNITTEST_ASSERT((nonnullUnc.operator==(nonnullFrame)) == false);

    rcUNITTEST_ASSERT(nonnullUnc.operator!=(nonnullFrameC));
    rcUNITTEST_ASSERT((nonnullUnc.operator==(nonnullFrameC)) == false);

    rcUNITTEST_ASSERT(nonnullUnc != rightNull);
    rcUNITTEST_ASSERT((nonnullUnc == rightNull) == false);

    rcUNITTEST_ASSERT(nonnullUnc == nonnullUnc);
    rcUNITTEST_ASSERT((nonnullUnc != nonnullUnc) == false);
    
    rcUNITTEST_ASSERT(nonnullUnc != nonnullUncRight);
    rcUNITTEST_ASSERT((nonnullUnc == nonnullUncRight) == false);
    
    rcUNITTEST_ASSERT(nonnullUnc != nonnullC);
    rcUNITTEST_ASSERT((nonnullUnc == nonnullC) == false);

    nonnullC.unlock();
    rcUNITTEST_ASSERT(nonnullUnc != nonnullC);
    rcUNITTEST_ASSERT((nonnullUnc == nonnullC) == false);

    delete nonnullFrame;
  }

  {
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);

    rcFrame* nullFrame = 0;
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr rightNull;
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));
    rcSharedFrameBufPtr nonnullCright;
    status = cacheP->getFrame(2, nonnullCright, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);

    rcUNITTEST_ASSERT(nonnullC.operator!=(nullFrame));
    rcUNITTEST_ASSERT((nonnullC.operator==(nullFrame)) == false);

    rcUNITTEST_ASSERT(nonnullC.operator!=(nonnullFrame));
    rcUNITTEST_ASSERT((nonnullC.operator==(nonnullFrame)) == false);

    rcUNITTEST_ASSERT(nonnullC.operator!=(nonnullFrameC));
    rcUNITTEST_ASSERT((nonnullC.operator==(nonnullFrameC)) == false);

    rcUNITTEST_ASSERT(nonnullC != rightNull);
    rcUNITTEST_ASSERT((nonnullC == rightNull) == false);

    rcUNITTEST_ASSERT(nonnullC != nonnullUnc);
    rcUNITTEST_ASSERT((nonnullC == nonnullUnc) == false);
    
    rcUNITTEST_ASSERT(nonnullC != nonnullCright);
    rcUNITTEST_ASSERT((nonnullC == nonnullCright) == false);
    
    rcUNITTEST_ASSERT(nonnullC == nonnullC);
    rcUNITTEST_ASSERT((nonnullC != nonnullC) == false);

    nonnullCright.unlock();
    rcUNITTEST_ASSERT(nonnullC != nonnullCright);
    rcUNITTEST_ASSERT((nonnullC == nonnullCright) == false);

    delete nonnullFrame;
  }

  {
    rcSharedFrameBufPtr nonnullC;
    status = cacheP->getFrame(1, nonnullC, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    nonnullC.unlock();

    rcFrame* nullFrame = 0;
    rcFrame* nonnullFrame = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr forFramePtr;
    status = cacheP->getFrame(3, forFramePtr, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
    rcFrame* nonnullFrameC = forFramePtr;
    rcSharedFrameBufPtr rightNull;
    rcSharedFrameBufPtr nonnullUnc(new rcFrame(32, 32, rcPixel8));
    rcSharedFrameBufPtr nonnullCright;
    status = cacheP->getFrame(2, nonnullCright, &error);
    rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);

    rcUNITTEST_ASSERT(nonnullC.operator!=(nullFrame));
    rcUNITTEST_ASSERT((nonnullC.operator==(nullFrame)) == false);

    rcUNITTEST_ASSERT(nonnullC.operator!=(nonnullFrame));
    rcUNITTEST_ASSERT((nonnullC.operator==(nonnullFrame)) == false);

    rcUNITTEST_ASSERT(nonnullC.operator!=(nonnullFrameC));
    rcUNITTEST_ASSERT((nonnullC.operator==(nonnullFrameC)) == false);

    rcUNITTEST_ASSERT(nonnullC != rightNull);
    rcUNITTEST_ASSERT((nonnullC == rightNull) == false);

    rcUNITTEST_ASSERT(nonnullC != nonnullUnc);
    rcUNITTEST_ASSERT((nonnullC == nonnullUnc) == false);
    
    rcUNITTEST_ASSERT(nonnullC != nonnullCright);
    rcUNITTEST_ASSERT((nonnullC == nonnullCright) == false);
    
    rcUNITTEST_ASSERT(nonnullC == nonnullC);
    rcUNITTEST_ASSERT((nonnullC != nonnullC) == false);

    nonnullCright.unlock();
    rcUNITTEST_ASSERT(nonnullC != nonnullCright);
    rcUNITTEST_ASSERT((nonnullC == nonnullCright) == false);

    delete nonnullFrame;
  }

  /* Ninth test - Check that locks and unlocks of uncached frames act
   * as noops.
   */
  {
    rcSharedFrameBufPtr nullUnc;
    rcFrame* frame1 = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr nonnullUnc(frame1);

    nullUnc.lock();
    rcUNITTEST_ASSERT(nullUnc == 0);
    rcUNITTEST_ASSERT(nullUnc.refCount() == 0);

    nullUnc.unlock();
    rcUNITTEST_ASSERT(nullUnc == 0);
    rcUNITTEST_ASSERT(nullUnc.refCount() == 0);

    nullUnc.lock();
    rcUNITTEST_ASSERT(!nullUnc);
    rcUNITTEST_ASSERT(nullUnc.refCount() == 0);

    nonnullUnc.lock();
    rcUNITTEST_ASSERT(frame1 == nonnullUnc);
    rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);

    nonnullUnc.unlock();
    rcUNITTEST_ASSERT(frame1 == nonnullUnc);
    rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);

    nonnullUnc.lock();
    rcUNITTEST_ASSERT(!nonnullUnc == false);
    rcUNITTEST_ASSERT(frame1 == nonnullUnc);
    rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);
  }


  /* Tenth test - Check that prefetches of uncached frames act as
   * noops.
   */
  {
    rcSharedFrameBufPtr nullUnc;
    rcFrame* frame1 = new rcFrame(32, 32, rcPixel8);
    rcSharedFrameBufPtr nonnullUnc(frame1);

    nullUnc.prefetch();
    rcUNITTEST_ASSERT(nullUnc == 0);
    rcUNITTEST_ASSERT(nullUnc.refCount() == 0);

    nonnullUnc.prefetch();
    rcUNITTEST_ASSERT(frame1 == nonnullUnc);
    rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);
  }

  rcVideoCache::rcVideoCacheDtor(cacheP);
}

void UT_VideoCache::dtorTest()
{
  /* Test that calling dtor doesn't cause referenced frame to go away.
   */
  const uint32 expPixelValue = 0x0;
  rcVideoCache* cacheP = rcVideoCache::rcVideoCacheCtor(fileName, 5, true,
							false, false);

  rcUNITTEST_ASSERT(cacheP != 0);

  rcSharedFrameBufPtr bp;
  rcVideoCacheError error;
  rcVideoCacheStatus status = cacheP->getFrame(0, bp, &error);
  rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
  rcUNITTEST_ASSERT(bp.refCount() == 2);
  rcUNITTEST_ASSERT(bp->getPixel(0, 0) == expPixelValue);

  rcVideoCache::rcVideoCacheDtor(cacheP);

  rcUNITTEST_ASSERT(bp.refCount() == 1);
  rcUNITTEST_ASSERT(bp->getPixel(0, 0) == expPixelValue);
}

void UT_VideoCache::threadSafeTest()
{
  /* Verify that cache member fcts are all thread safe.
   */
  rcVideoCache* cacheP =
    rcVideoCache::rcVideoCacheCtor(fileName, VIDEOCACHE_SZ, true, false,
				   false);
  
  rcUNITTEST_ASSERT(cacheP->isValid());
  if (!cacheP->isValid())
    return; // No point in continuing if this doesn't work

  /* If start flag is already set, something fishy is going on.
   */
  int temp;
  utVideoCacheThread::startTest.getValue(temp);
  rmAssert(temp == 0);

  /* Create all the worker tasks.
   */
  vector<utVideoCacheThread*> vidUser;
  vector<rcThread*> vidUserThread;
  for (uint32 i = 0; i < VIDEOCACHE_THREAD_CNT; i++) {
    vidUser.push_back(new utVideoCacheThread(*cacheP));
    vidUserThread.push_back(new rcThread(vidUser[i]));
  }

  /* Start all the worker tasks.
   */
  for (uint32 i = 0; i < VIDEOCACHE_THREAD_CNT; i++)
    vidUserThread[i]->start();

  /* Tell the worker tasks to have at it.
   */
  utVideoCacheThread::startTest.setValue(1);

  /* Wait for the worker tasks to complete,
   */
  for (uint32 i = 0; i < VIDEOCACHE_THREAD_CNT; i++)
    vidUserThread[i]->join();
  
  /* Tally up results.
   */
  for (uint32 i = 0; i < VIDEOCACHE_THREAD_CNT; i++) {
    mErrors += vidUser[i]->_myErrors;
  }

#ifdef VALIDATING_TESTS
  /* Display coverage statistics.
   */
  for (uint32 i = 0; i < VIDEOCACHE_THREAD_CNT; i++) {
    printf("vidUser[%d]: errors %d prefetches %d\n", i,
	   vidUser[i]->_myErrors, vidUser[i]->_prefetches);

    uint32 extraCount = 0;
    for (uint32 j = 14; j < VIDEOCACHE_THREAD_CNT*5; j++)
      extraCount += vidUser[i]->_refCount[j];
    printf("Ref Count 0: %d 1: %d 2: %d >=14: %d\n", vidUser[i]->_refCount[0],
	   vidUser[i]->_refCount[1], vidUser[i]->_refCount[2], extraCount);
    printf("Ref Count 3: %d 4: %d 5: %d 6: %d 7: %d 8: %d 9: %d 10: %d "
	   "11: %d 12: %d 13: %d\n", vidUser[i]->_refCount[3],
	   vidUser[i]->_refCount[4], vidUser[i]->_refCount[5],
	   vidUser[i]->_refCount[6], vidUser[i]->_refCount[7],
	   vidUser[i]->_refCount[8], vidUser[i]->_refCount[9],
	   vidUser[i]->_refCount[10], vidUser[i]->_refCount[11],
	   vidUser[i]->_refCount[12], vidUser[i]->_refCount[13]);

    printf("Frame Touch 0 %d 1 %d 2 %d 3 %d 4 %d 5 %d 6 %d 7 %d\n\n",
	   vidUser[i]->_frameTouch[0], vidUser[i]->_frameTouch[1],
	   vidUser[i]->_frameTouch[2], vidUser[i]->_frameTouch[3],
	   vidUser[i]->_frameTouch[4], vidUser[i]->_frameTouch[5],
	   vidUser[i]->_frameTouch[6], vidUser[i]->_frameTouch[7]);
  }
#endif

  rcVideoCache::rcVideoCacheDtor(cacheP);
}
