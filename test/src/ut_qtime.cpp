
#include <stdlib.h>

#include "ut_qtime.h"
#include "qtime_cache.h"
#include "rc_windowhist.h"
#include "timing.hpp"
#include "rc_math.h"


static std::string fileName;

/*************************************************************************/
/*                                                                       */
/*                Support class for thread-safe tests                    */
/*                                                                       */
/*************************************************************************/
rcAtomicValue<int> utQtimeCacheThread::startTest(0);

utQtimeCacheThread::utQtimeCacheThread(QtimeCache& cache) : _cache(cache)
{
    for (uint32 i = 0; i < FRAME_COUNT; i++)
        _frameTouch[i] = 0;
    for (uint32 i = 0; i < (QtimeCache_THREAD_CNT*5); i++)
        _ref_count[i] = 0;
    _myErrors = 0;
    _prefetches = 0;
}

void utQtimeCacheThread::run()
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
        QtimeCache::frame_ref_t bp;
        eQtimeCacheError error;
        eQtimeCacheStatus status;
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
        
        if (status == eQtimeCacheStatusOK) {
            {
                rcWindow win(bp);
                uint32 ref_count = bp.refCount();
                if ((ref_count < 3) || (ref_count >= (QtimeCache_THREAD_CNT*5))) {
                    fprintf(stderr, "Error on ref_count check %d\n", ref_count);
                    _myErrors++;
                }
                else {
                    _ref_count[ref_count]++;
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
                    uint32 ref_count = bp.refCount();
                    if ((ref_count < 3) || (ref_count >= (QtimeCache_THREAD_CNT*5))) {
                        fprintf(stderr, "Error on ref_count check %d\n", ref_count);
                        _myErrors++;
                    }
                    else {
                        _ref_count[ref_count]++;
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

UT_QtimeCache::UT_QtimeCache(std::string movieFileName)
{
    fileName = movieFileName;
}

UT_QtimeCache::~UT_QtimeCache()
{
    printSuccessMessage("QtimeCache test", mErrors);
}

uint32 UT_QtimeCache::run()
{
    ctorTest();
    mappingTest();
    simpleAllocTest();
//    prefetchTest();
//    prefetchThreadTest();
    cacheFullTest();
    frameBufTest();
    dtorTest();
 //   threadSafeTest();

    
    return mErrors;
}

void UT_QtimeCache::ctorTest()
{
    const uint32 frameCount = 56;
    
    QtimeCache* cacheP = QtimeCache::QtimeCacheCtor(fileName, 5);
    
    rcUNITTEST_ASSERT(cacheP != 0);
    rcUNITTEST_ASSERT(cacheP->isValid());
    rcUNITTEST_ASSERT(cacheP->getFatalError() == eQtimeCacheErrorOK);
    rcUNITTEST_ASSERT(cacheP->frameCount() == frameCount);
    
    QtimeCache::QtimeCacheDtor(cacheP);
    
    cacheP = QtimeCache::QtimeCacheCtor(fileName, 5, false, true);
    
    rcUNITTEST_ASSERT(cacheP != 0);
    rcUNITTEST_ASSERT(cacheP->isValid());
    rcUNITTEST_ASSERT(cacheP->getFatalError() == eQtimeCacheErrorOK);
    rcUNITTEST_ASSERT(cacheP->frameCount() == frameCount);
    
    QtimeCache::QtimeCacheDtor(cacheP);
    
    cacheP = QtimeCache::QtimeCacheCtor(fileName, 10, false, false);
    
    rcUNITTEST_ASSERT(cacheP != 0);
    rcUNITTEST_ASSERT(cacheP->isValid());
    rcUNITTEST_ASSERT(cacheP->getFatalError() == eQtimeCacheErrorOK);
    rcUNITTEST_ASSERT(cacheP->frameCount() == frameCount);
    
    std::string nullFile;
    
    QtimeCache* nullCacheP = QtimeCache::QtimeCacheCtor(nullFile, 10, false,   false);
    
    rcUNITTEST_ASSERT(nullCacheP != 0);
    rcUNITTEST_ASSERT(nullCacheP->isValid() == false);
    rcUNITTEST_ASSERT(nullCacheP->getFatalError()== eQtimeCacheErrorFileInit);
    
    std::string badFile("xyzzy.dat");
    
    QtimeCache* badCacheP = QtimeCache::QtimeCacheCtor(badFile, 10, false, false);
    
    rcUNITTEST_ASSERT(badCacheP->isValid() == false);
    rcUNITTEST_ASSERT(badCacheP->getFatalError() == eQtimeCacheErrorFileInit);
    
    QtimeCache::QtimeCacheDtor(cacheP);
    QtimeCache::QtimeCacheDtor(nullCacheP);
    QtimeCache::QtimeCacheDtor(badCacheP);
    
    /* A set of tests to verify that the ctor properly calculates the
     * number of cache entries to be made available based upon the
     * maximum memory and cache size parameters. These numbers assume
     * the test movie contains 10 1K images.
     */
#define PIXELCNT 256
#define FRAMECNT 56
    vector<uint32> maxMemTests;
    maxMemTests.push_back(PIXELCNT/2);
    maxMemTests.push_back((FRAMECNT/2)*PIXELCNT);
    maxMemTests.push_back((FRAMECNT-1)*PIXELCNT);
    maxMemTests.push_back(FRAMECNT*PIXELCNT);
    maxMemTests.push_back((FRAMECNT+1)*PIXELCNT);
    
    vector<uint32> frameCntTests;
    frameCntTests.push_back(0);
    frameCntTests.push_back((FRAMECNT-8));
    frameCntTests.push_back((FRAMECNT/2));
    frameCntTests.push_back((FRAMECNT-3));
    frameCntTests.push_back((FRAMECNT-1));
    frameCntTests.push_back(FRAMECNT);
    frameCntTests.push_back((FRAMECNT+2));
    
    vector<vector<uint32> >expResults(5);
    expResults[0].push_back(0);
    expResults[0].push_back(0);
    expResults[0].push_back(0);
    expResults[0].push_back(0);
    expResults[0].push_back(0);
    expResults[0].push_back(0);
    expResults[0].push_back(0);
    
    expResults[1].push_back((FRAMECNT/2));
    expResults[1].push_back((FRAMECNT/2));
    expResults[1].push_back((FRAMECNT/2));
    expResults[1].push_back((FRAMECNT/2));
    expResults[1].push_back((FRAMECNT/2));
    expResults[1].push_back((FRAMECNT/2));
    expResults[1].push_back((FRAMECNT/2));
    
    expResults[2].push_back((FRAMECNT-1));
    expResults[2].push_back((FRAMECNT-8));
    expResults[2].push_back((FRAMECNT/2));
    expResults[2].push_back((FRAMECNT-3));
    expResults[2].push_back((FRAMECNT-1));
    expResults[2].push_back((FRAMECNT-1));
    expResults[2].push_back((FRAMECNT-1));
    
    expResults[3].push_back(FRAMECNT);
    expResults[3].push_back((FRAMECNT-8));
    expResults[3].push_back((FRAMECNT/2));
    expResults[3].push_back((FRAMECNT-3));
    expResults[3].push_back((FRAMECNT-1));
    expResults[3].push_back(FRAMECNT);
    expResults[3].push_back(FRAMECNT);
    
    expResults[4].push_back(FRAMECNT);
    expResults[4].push_back((FRAMECNT-8));
    expResults[4].push_back((FRAMECNT/2));
    expResults[4].push_back((FRAMECNT-3));
    expResults[4].push_back((FRAMECNT-1));
    expResults[4].push_back(FRAMECNT);
    expResults[4].push_back(FRAMECNT);
    
    
    for (uint32 memIndex = 0; memIndex < maxMemTests.size(); memIndex++)
        for (uint32 fcIndex = 0; fcIndex < frameCntTests.size(); fcIndex++) {
            QtimeCache* tcP =
            QtimeCache::QtimeCacheCtor(fileName, frameCntTests[fcIndex],
                                       false, false, maxMemTests[memIndex]);
            
            if (expResults[memIndex][fcIndex] == 0) {
                rcUNITTEST_ASSERT(tcP->isValid() == false);
                rcUNITTEST_ASSERT(tcP->getFatalError() ==
                                  eQtimeCacheErrorSystemResources);
            }
            else {
                rcUNITTEST_ASSERT(tcP->isValid() == true);
                rcUNITTEST_ASSERT(tcP->cacheSize() == expResults[memIndex][fcIndex]);
                //       std::cout << memIndex << " -- " << fcIndex << "  " << tcP->cacheSize() << " ? " << expResults[memIndex][fcIndex] << std::endl;
            }
            
            QtimeCache::QtimeCacheDtor(tcP);
        }
    
    
}

void UT_QtimeCache::mappingTest()
{
    const uint32 frameCount = 56;
    const uint32 extraCasesCount = 5;
    const uint32 totalCases = frameCount + extraCasesCount;
    
    double times[totalCases] = {
        0.0,
        0.033333
        ,0.066666
        ,0.098333
        ,0.13
        ,0.165
        ,0.198333
        ,0.231666
        ,0.261666
        ,0.295
        ,0.33
        ,0.363333
        ,0.396666
        ,0.428333
        ,0.461666
        ,0.495
        ,0.526666
        ,0.561666
        ,0.591666
        ,0.625
        ,0.66
        ,0.691666
        ,0.726666
        ,0.756666
        ,0.791666
        ,0.823333
        ,0.858333
        ,0.89
        ,0.923333
        ,0.955
        ,0.99
        ,1.02167
        ,1.055
        ,1.08833
        ,1.12167
        ,1.15333
        ,1.18833
        ,1.22167
        ,1.25167
        ,1.285
        ,1.32
        ,1.35333
        ,1.385
        ,1.41667
        ,1.45167
        ,1.485
        ,1.51667
        ,1.55
        ,1.58333
        ,1.61667
        ,1.64833
        ,1.68167
        ,1.71667
        ,1.74833
        ,1.78
        ,1.815,
        0., 6., 8., 2000000000., 9000000000. };
    
    QtimeCache* cacheP = QtimeCache::QtimeCacheCtor(fileName, 5, true,
                                                    false);
    rmAssert(frameCount == cacheP->frameCount());
    
    rcTimestamp actual;
    eQtimeCacheError error;
    eQtimeCacheStatus status;
    
    /* Test for functions frameIndexToTimestamp() and
     * timestampToFrameIndex().
     */
    const double eps = 1e-05;  // Needed !!
    for (uint32 i = 0; i < frameCount; i++) {
        status = cacheP->frameIndexToTimestamp(i, actual, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        if (status == eQtimeCacheStatusOK)
            rcUNITTEST_ASSERT(equal(actual.secs(), times[i], eps));
        
        uint32 actualIndex;
        status = cacheP->timestampToFrameIndex(actual, actualIndex, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        if (status == eQtimeCacheStatusOK)
            rcUNITTEST_ASSERT(actualIndex == i);
    }
    
    /* Test for functions firstTimestamp() and lastTimestamp().
     */
    status = cacheP->firstTimestamp(actual, &error);
    rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
    if (status == eQtimeCacheStatusOK)
        rcUNITTEST_ASSERT(actual.secs() == times[0]);
    
    status = cacheP->lastTimestamp(actual, &error);
    rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
    if (status == eQtimeCacheStatusOK)
        rcUNITTEST_ASSERT(actual.secs() == times[frameCount-1]);
    
    /* Test for function closestTimestamp()
     */
    {
        double expected[totalCases] = {
            0.0,
            0.033333
            ,0.066666
            ,0.098333
            ,0.13
            ,0.165
            ,0.198333
            ,0.231666
            ,0.261666
            ,0.295
            ,0.33
            ,0.363333
            ,0.396666
            ,0.428333
            ,0.461666
            ,0.495
            ,0.526666
            ,0.561666
            ,0.591666
            ,0.625
            ,0.66
            ,0.691666
            ,0.726666
            ,0.756666
            ,0.791666
            ,0.823333
            ,0.858333
            ,0.89
            ,0.923333
            ,0.955
            ,0.99
            ,1.02167
            ,1.055
            ,1.08833
            ,1.12167
            ,1.15333
            ,1.18833
            ,1.22167
            ,1.25167
            ,1.285
            ,1.32
            ,1.35333
            ,1.385
            ,1.41667
            ,1.45167
            ,1.485
            ,1.51667
            ,1.55
            ,1.58333
            ,1.61667
            ,1.64833
            ,1.68167
            ,1.71667
            ,1.74833
            ,1.78
            ,1.815, 1.815, 1.815, 1.815, 1.815, 1.815 };
        
        for (uint32 i = 0; i < totalCases; i++)
        {
            status = cacheP->closestTimestamp(rcTimestamp::from_seconds(times[i]), actual, &error);
            //    std::cout << "[" << i << "]:" << times[i] << " + : " << actual.secs() << " e " << (int) error << " s " << (int) status << std::endl;
            rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
            if (status == eQtimeCacheStatusOK && i < frameCount)
            {
                if (! equal(actual.secs(), expected[i], eps))
                    std::cout << "[" << i << "]:" << times[i] << " + : " << actual.secs() << " e " << (int) error << " s " << (int) status << std::endl;
                
                rcUNITTEST_ASSERT(equal(actual.secs(), expected[i], eps));
            }
            
        }
    }
    
    /* Test for function prevTimestamp()
     */
    {
        
        for (uint32 i = 0; i < totalCases; i++) {
            error = eQtimeCacheErrorFileInit;
            status = cacheP->prevTimestamp(rcTimestamp::from_seconds (times[i]), actual, &error);
            if (i == 0 || i == 56)
            {
                rcUNITTEST_ASSERT(status == eQtimeCacheStatusError);
                rcUNITTEST_ASSERT(error == eQtimeCacheErrorNoSuchFrame);
            }
            else
            {
                rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
                rcUNITTEST_ASSERT(cacheP->getFatalError() == eQtimeCacheErrorOK);
            }
        }
    }
    
#if ISSUE_TBD
    /* Test for function nextTimestamp() */
    {
        const double next_eps = 1e-04;  // Needed !!
        
        for (uint32 i = 0; i < totalCases; i++)
        {
            error = eQtimeCacheErrorFileInit;
            status = cacheP->nextTimestamp(rcTimestamp::from_seconds (times[i]), actual, &error);
            std::cout << "[" << i << "]:" << times[i] << "<->" << times[i+1] << " + : " << actual.secs() << " e " << (int) error << " s " << (int) status << std::endl;
            
            //        if (status != eQtimeCacheStatusOK)
            //            rcUNITTEST_ASSERT(error == eQtimeCacheErrorNoSuchFrame);
            //      else
            //      {
            //
            //          rcUNITTEST_ASSERT(equal(actual.secs(), times[i+1], next_eps));
            //      }
        }
        
    }
#endif
    
    QtimeCache::QtimeCacheDtor(cacheP);
    
}

void UT_QtimeCache::simpleAllocTest()
{
    /* Make simple test of getFrame functions. Note that the cache is
     * created with only 1 cache entry to verify that cache elements are
     * getting freed.
     */
    QtimeCache* cacheP = QtimeCache::QtimeCacheCtor(fileName, 1, true,
                                                    false, false);
    rc256BinHist hist(256);
    
    for (uint32 i = 0; i < cacheP->frameCount(); i++) {
        eQtimeCacheError error;
        eQtimeCacheStatus status;
        QtimeCache::frame_ref_t buf;
        rmAssert(buf.refCount() == 0);
        
        status = cacheP->getFrame(i, buf, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        if (status == eQtimeCacheStatusOK) {
            rcUNITTEST_ASSERT(buf.refCount() == 2);
            rcUNITTEST_ASSERT(buf->width() == 16);
            rcUNITTEST_ASSERT(buf->height() == 16);
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
            
#ifdef KNOWN_IMAGE
            rfGenDepth8Histogram(win, hist);
            rcUNITTEST_ASSERT((hist[0]+hist[138]) == win.pixelCount());
#endif
            
        }
    }
    
    for (uint32 i = 0; i < cacheP->frameCount(); i++) {
        eQtimeCacheError error;
        eQtimeCacheStatus status;
        QtimeCache::frame_ref_t buf;
        rmAssert(buf.refCount() == 0);
        
        rcTimestamp time;
        status = cacheP->frameIndexToTimestamp(i, time, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        status = cacheP->getFrame(time, buf, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        if (status == eQtimeCacheStatusOK) {
            rcUNITTEST_ASSERT(buf.refCount() == 2);
            rcUNITTEST_ASSERT(buf->width() == 16);
            rcUNITTEST_ASSERT(buf->height() == 16);
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
#ifdef KNOWN_IMAGE
            rfGenDepth8Histogram(win, hist);
            rcUNITTEST_ASSERT((hist[0]+hist[138]) == win.pixelCount());
#endif
        }
    }
    
    QtimeCache::QtimeCacheDtor(cacheP);
}

// Test for prefetch thread operation
void UT_QtimeCache::prefetchThreadTest()
{
    fprintf( stderr, "%-38s", "QtimeCache prefetch thread ctor");
    // Call ctor and dtor rapdily to test prefetch thread race conditions
    for ( uint32 i = 0;  i < 4096; ++i ) {
        QtimeCache* cacheP =
        QtimeCache::QtimeCacheCtor(fileName, 0, true, false);
        // Warning: exeution may deadlock here if prefetch thread
        // hasn't been properly initialized when dtor is called
        QtimeCache::QtimeCacheDtor(cacheP);
    }
    fprintf( stderr, " OK\n" );
}

void UT_QtimeCache::prefetchTest()
{
    const uint32 frameCount = 56;
    
    rc256BinHist hist(256);
    
    QtimeCache* cacheP =
    QtimeCache::QtimeCacheCtor(fileName, 0, true, true);
    
    QtimeCache::frame_ref_t frameBuf[frameCount];
    eQtimeCacheError error;
    eQtimeCacheStatus status;
    
    rcUNITTEST_ASSERT(cacheP->frameCount() == frameCount);
    
    for (uint32 i = 0; i < frameCount; i++) {
        status = cacheP->getFrame(i, frameBuf[i], &error, false);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        if (status != eQtimeCacheStatusOK)
        {
            std::cout << i << std::endl;
            return; // No point continuing if we've already screwed up
        }
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
        rcUNITTEST_ASSERT((hist[0]+hist[138]) == win.pixelCount());
    }
    
    
    
    QtimeCache::QtimeCacheDtor(cacheP);
    double noPrefetchTime;
    
//    {
//        QtimeCache::frame_ref_t frameBuf[frameCount];
        
        cacheP = QtimeCache::QtimeCacheCtor(fileName, 0, true, false);
        
        for (uint32 i = 0; i < frameCount; i++) {
            status = cacheP->getFrame(i, frameBuf[i], &error, false);
            rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
            if (status != eQtimeCacheStatusOK)
                return; // No point continuing if we've already screwed up
        }
        
        
        timer.reset ();
        for (uint32 i = 0; i < frameCount; i++)
            frameBuf[i].lock();
        noPrefetchTime = timer.getTime ();
        
        for (uint32 i = 0; i < frameCount; i++) {
            rcWindow win(frameBuf[i]);
            rfGenDepth8Histogram(win, hist);
            rcUNITTEST_ASSERT((hist[0]+hist[138]) == win.pixelCount());
        }
        
        
        
        QtimeCache::QtimeCacheDtor(cacheP);
//    }
    
    cout << "Performance: Cache Prefetch " << prefetchTime
    << " us, Time Per Frame " << (prefetchTime / frameCount)
    << " us" << endl;
    
    cout << "Performance: No Cache Prefetch " << noPrefetchTime
    << " us, Time Per Frame " << (noPrefetchTime / frameCount)
    << " us" << endl;
}

void UT_QtimeCache::cacheFullTest()
{
    /* Test cases around where cache gets filled.
     */
    const uint32 frameCount = 56;
    const uint32 cacheSize = frameCount/2, overflowSize = 2;
    const uint32 totalCases = cacheSize + overflowSize;
    
    QtimeCache* cacheP =
    QtimeCache::QtimeCacheCtor(fileName, cacheSize, true, false);
    rc256BinHist hist(256);
    QtimeCache::frame_ref_t bp[totalCases];
    eQtimeCacheError error;
    eQtimeCacheStatus status;
    
    /* Fill up cache with locked entries.
     */
    for (uint32 i = 0; i < cacheSize; i++) {
        status = cacheP->getFrame(i, bp[i], &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        if (status != eQtimeCacheStatusOK)
            return; // No point continuing if we've already screwed up
        rcUNITTEST_ASSERT(bp[i].refCount() == 2);
        rcWindow win(bp[i]);
        rcUNITTEST_ASSERT(bp[i].refCount() == 3);
        
        rfGenDepth8Histogram(win, hist);
        rcUNITTEST_ASSERT((hist[0]+hist[138]) == win.pixelCount());
    }
    
    /* Verify overflow entries are available.
     */
    for (uint32 i = cacheSize; i < totalCases; i++) {
        status = cacheP->getFrame(i, bp[i], &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        if (status == eQtimeCacheStatusOK) {
            rcUNITTEST_ASSERT(bp[i].refCount() == 2);
            rcWindow win(bp[i]);
            rcUNITTEST_ASSERT(bp[i].refCount() == 3);
            
            rfGenDepth8Histogram(win, hist);
            rcUNITTEST_ASSERT((hist[0]+hist[138]) == win.pixelCount());
        }
    }
    
    /* Unlock some elements and verify that new frames can be allocated.
     */
    for (uint32 i = cacheSize; i < totalCases; i++) {
        bp[i - cacheSize].unlock();
        status = cacheP->getFrame(i, bp[i], &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        if (status == eQtimeCacheStatusOK) {
            rcUNITTEST_ASSERT(bp[i].refCount() == 2);
            rcWindow win(bp[i]);
            rcUNITTEST_ASSERT(bp[i].refCount() == 3);
            
            rfGenDepth8Histogram(win, hist);
            rcUNITTEST_ASSERT((hist[0]+hist[138]) == win.pixelCount());
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
            rcUNITTEST_ASSERT((hist[0]+hist[138]) == win.pixelCount());
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
            rcUNITTEST_ASSERT((hist[0]+hist[138]) == win.pixelCount());
        }
    }
    
  //  QtimeCache::QtimeCacheDtor(cacheP);
}

void UT_QtimeCache::frameBufTest()
{
    /* Test manipulating QtimeCache::frame_ref_ts using both cached and
     * uncached frames.
     */
    const uint32 uncachedPixelValue = 0xA;
    const uint32 cachedPixelValue = 0x0;
    const uint32 cacheSize = 5;
    QtimeCache* cacheP =
    QtimeCache::QtimeCacheCtor(fileName, cacheSize, true, false);
    QtimeCache::frame_ref_t cachedBp, uncachedBp, changingBp;
    eQtimeCacheError error;
    eQtimeCacheStatus status;
    
    rcUNITTEST_ASSERT(cachedBp.refCount() == 0);
    rcUNITTEST_ASSERT(uncachedBp.refCount() == 0);
    rcUNITTEST_ASSERT(changingBp.refCount() == 0);
    
    /* First test: Set up cachedBp and uncachedBp. Set changingBp to
     *             point towards uncached frame.
     */
    status = cacheP->getFrame(0, cachedBp, &error);
    
    rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
    if (status != eQtimeCacheStatusOK)
        return; // No point in continuing test if this doesn't work.
    
    rcUNITTEST_ASSERT(cachedBp.refCount() == 2);
    rcUNITTEST_ASSERT(cachedBp->getPixel(0, 0) == cachedPixelValue);
    
    uncachedBp = new rcFrame(16, 16, rcPixel8);
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
     *             QtimeCache::frame_ref_t using a cached rcFrame* works
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
        
        QtimeCache::frame_ref_t cachedToo(frame1);
        rcUNITTEST_ASSERT(cachedToo->getPixel(0, 0) == cachedPixelValue);
        rcUNITTEST_ASSERT(cachedBp.refCount() == 4);
    }
    
    /* Sixth test: Check that ctor works correctly in all cases:
     *  - No arg
     *  - Null rcFrame*
     *  - non-null rcFrame*
     *  - non-null rcFrame* cached
     *  - Null QtimeCache::frame_ref_t
     *  - Non-null uncached QtimeCache::frame_ref_t
     *  - Non-null cached, locked QtimeCache::frame_ref_t
     *  - Non-null cached, unlocked QtimeCache::frame_ref_t
     */
    {
        QtimeCache::frame_ref_t null;
        rcUNITTEST_ASSERT(null == 0);
        rcUNITTEST_ASSERT(null.refCount() == 0);
        
        QtimeCache::frame_ref_t null2(null);
        rcUNITTEST_ASSERT(null2 == 0);
        rcUNITTEST_ASSERT(null2.refCount() == 0);
    }
    {
        rcFrame* nullFrame = 0;
        QtimeCache::frame_ref_t null(nullFrame);
        rcUNITTEST_ASSERT(null == 0);
        rcUNITTEST_ASSERT(null.refCount() == 0);
    }
    {
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t nonnull(nonnullFrame);
        rcUNITTEST_ASSERT(nonnullFrame == nonnull);
        rcUNITTEST_ASSERT(nonnull.refCount() == 1);
    }
    {
        QtimeCache::frame_ref_t nonnull;
        status = cacheP->getFrame(1, nonnull, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* cachedFrame = nonnull;
        QtimeCache::frame_ref_t nonnull2(cachedFrame);
        rcUNITTEST_ASSERT(nonnull2 == nonnull);
        rcUNITTEST_ASSERT(nonnull2.refCount() == 3);
    }
    {
        QtimeCache::frame_ref_t nonnull(new rcFrame(16, 16 , rcPixel8));
        QtimeCache::frame_ref_t nonnull2(nonnull);
        rcUNITTEST_ASSERT(nonnull2 == nonnull);
        rcUNITTEST_ASSERT(nonnull2.refCount() == 2);
    }
    {
        QtimeCache::frame_ref_t nonnull;
        status = cacheP->getFrame(1, nonnull, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        QtimeCache::frame_ref_t nonnull2(nonnull);
        rcUNITTEST_ASSERT(nonnull2 == nonnull);
        rcUNITTEST_ASSERT(nonnull2.refCount() == 3);
    }
    {
        QtimeCache::frame_ref_t nonnull;
        status = cacheP->getFrame(1, nonnull, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        QtimeCache::frame_ref_t nonnullcopy(nonnull);
        nonnull.unlock();
        QtimeCache::frame_ref_t nonnull2(nonnull);
        rcUNITTEST_ASSERT(nonnullcopy.refCount() == 2);
        rcUNITTEST_ASSERT(nonnull2 == nonnull);
        rcUNITTEST_ASSERT(nonnull2.refCount() == 3);
    }
    
    /* Seventh test - Check that assignment works correctly in all cases:
     *  - Null rcFrame*
     *  - Non-null rcFrame*
     *  - Non-null rcFrame* cached
     *  - Null QtimeCache::frame_ref_t
     *  - Non-null uncached QtimeCache::frame_ref_t
     *  - Non-null cached, locked QtimeCache::frame_ref_t
     *  - Non-null cached, unlocked QtimeCache::frame_ref_t
     *
     * All of the above cases must be tested as source in permutation
     * with all QtimeCache::frame_ref_t cases above tested as destination.
     */
    {
        QtimeCache::frame_ref_t destNull;
        
        rcFrame* nullFrame = 0;
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t srcNull;
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        
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
        rcFrame* frame1 = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t destNonnullUnc(frame1);
        rcUNITTEST_ASSERT(frame1 == destNonnullUnc);
        
        rcFrame* nullFrame = 0;
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t srcNull;
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        
        destNonnullUnc = nonnullFrame;
        rcUNITTEST_ASSERT(destNonnullUnc.refCount() == 1);
        rcUNITTEST_ASSERT(nonnullFrame == destNonnullUnc);
        
        destNonnullUnc = nullFrame;
        rcUNITTEST_ASSERT(destNonnullUnc == 0);
        rcUNITTEST_ASSERT(destNonnullUnc.refCount() == 0);
        frame1 = new rcFrame(16, 16, rcPixel8);
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
        frame1 = new rcFrame(16, 16, rcPixel8);
        destNonnullUnc = frame1;
        rcUNITTEST_ASSERT(frame1 == destNonnullUnc);
        rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);
        
        destNonnullUnc = nonnullC;
        rcUNITTEST_ASSERT(destNonnullUnc == nonnullC);
        rcUNITTEST_ASSERT(destNonnullUnc.refCount() == 3);
        frame1 = new rcFrame(16, 16, rcPixel8);
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
        QtimeCache::frame_ref_t destNonnullC;
        status = cacheP->getFrame(2, destNonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
        QtimeCache::frame_ref_t frame2Copy(destNonnullC);
        rcUNITTEST_ASSERT(destNonnullC.refCount() == 3);
        
        rcFrame* nullFrame = 0;
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t srcNull;
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        
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
        QtimeCache::frame_ref_t destNonnullCunlocked;
        status = cacheP->getFrame(2, destNonnullCunlocked, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
        QtimeCache::frame_ref_t frame2Copy(destNonnullCunlocked);
        rcUNITTEST_ASSERT(destNonnullCunlocked.refCount() == 3);
        
        rcFrame* nullFrame = 0;
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t srcNull;
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        
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
     *  - Null QtimeCache::frame_ref_t
     *  - Non-null uncached QtimeCache::frame_ref_t
     *  - Non-null cached, locked QtimeCache::frame_ref_t
     *  - Non-null cached, unlocked QtimeCache::frame_ref_t
     *
     * All permutations of above cases must be tested for both left-hand
     * and right-hand sides of comparison.
     */
    {
        rcFrame* nullFrame = 0;
        
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t rightNull;
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
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
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        
        rcFrame* nullFrame = 0;
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t rightNull;
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
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
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        
        rcFrame* nullFrame = 0;
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t rightNull;
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
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
        QtimeCache::frame_ref_t leftNull;
        
        rcFrame* nullFrame = 0;
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t rightNull;
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
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
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        
        rcFrame* nullFrame = 0;
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t rightNull;
        QtimeCache::frame_ref_t nonnullUncRight(new rcFrame(16, 16, rcPixel8));
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
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
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
        rcFrame* nullFrame = 0;
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t rightNull;
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        QtimeCache::frame_ref_t nonnullCright;
        status = cacheP->getFrame(2, nonnullCright, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
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
        QtimeCache::frame_ref_t nonnullC;
        status = cacheP->getFrame(1, nonnullC, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        nonnullC.unlock();
        
        rcFrame* nullFrame = 0;
        rcFrame* nonnullFrame = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t forFramePtr;
        status = cacheP->getFrame(3, forFramePtr, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        rcFrame* nonnullFrameC = forFramePtr;
        QtimeCache::frame_ref_t rightNull;
        QtimeCache::frame_ref_t nonnullUnc(new rcFrame(16, 16, rcPixel8));
        QtimeCache::frame_ref_t nonnullCright;
        status = cacheP->getFrame(2, nonnullCright, &error);
        rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
        
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
        QtimeCache::frame_ref_t nullUnc;
        rcFrame* frame1 = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t nonnullUnc(frame1);
        
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
        QtimeCache::frame_ref_t nullUnc;
        rcFrame* frame1 = new rcFrame(16, 16, rcPixel8);
        QtimeCache::frame_ref_t nonnullUnc(frame1);
        
        nullUnc.prefetch();
        rcUNITTEST_ASSERT(nullUnc == 0);
        rcUNITTEST_ASSERT(nullUnc.refCount() == 0);
        
        nonnullUnc.prefetch();
        rcUNITTEST_ASSERT(frame1 == nonnullUnc);
        rcUNITTEST_ASSERT(nonnullUnc.refCount() == 1);
    }
    
    // QtimeCache::QtimeCacheDtor(cacheP);
}

void UT_QtimeCache::dtorTest()
{
    /* Test that calling dtor doesn't cause referenced frame to go away.
     */
    const uint32 expPixelValue = 0x0;
    QtimeCache* cacheP = QtimeCache::QtimeCacheCtor(fileName, 5, true, false);
    
    rcUNITTEST_ASSERT(cacheP != 0);
    
    QtimeCache::frame_ref_t bp;
    eQtimeCacheError error;
    eQtimeCacheStatus status = cacheP->getFrame(0, bp, &error);
    rcUNITTEST_ASSERT(status == eQtimeCacheStatusOK);
    rcUNITTEST_ASSERT(bp.refCount() == 2);
    rcUNITTEST_ASSERT(bp->getPixel(0, 0) == expPixelValue);
    
    QtimeCache::QtimeCacheDtor(cacheP);
    
    rcUNITTEST_ASSERT(bp.refCount() == 1);
    rcUNITTEST_ASSERT(bp->getPixel(0, 0) == expPixelValue);
}

void UT_QtimeCache::threadSafeTest()
{
    /* Verify that cache member fcts are all thread safe.
     */
    QtimeCache* cacheP =
    QtimeCache::QtimeCacheCtor(fileName, QtimeCache_SZ, true, false,
                               false);
    
    rcUNITTEST_ASSERT(cacheP->isValid());
    if (!cacheP->isValid())
        return; // No point in continuing if this doesn't work
    
    /* If start flag is already set, something fishy is going on.
     */
    int temp;
    utQtimeCacheThread::startTest.getValue(temp);
    rmAssert(temp == 0);
    
    /* Create all the worker tasks.
     */
    vector<utQtimeCacheThread*> vidUser;
    vector<boost::thread*> vidUserThread;
    for (uint32 i = 0; i < QtimeCache_THREAD_CNT; i++) {
        vidUser.push_back(new utQtimeCacheThread(*cacheP));
        vidUserThread.push_back(new boost::thread(boost::bind (&utQtimeCacheThread::run, vidUser[i])));
    }
    
    /* Start all the worker tasks.
     */
    // for (uint32 i = 0; i < QtimeCache_THREAD_CNT; i++)
    //    vidUserThread[i]->start();
    
    /* Tell the worker tasks to have at it.
     */
    utQtimeCacheThread::startTest.setValue(1);
    
    /* Wait for the worker tasks to complete,
     */
    for (uint32 i = 0; i < QtimeCache_THREAD_CNT; i++)
        vidUserThread[i]->join();
    
    /* Tally up results.
     */
    for (uint32 i = 0; i < QtimeCache_THREAD_CNT; i++) {
        mErrors += vidUser[i]->_myErrors;
    }
    
#ifdef VALIDATING_TESTS
    /* Display coverage statistics.
     */
    for (uint32 i = 0; i < QtimeCache_THREAD_CNT; i++) {
        printf("vidUser[%d]: errors %d prefetches %d\n", i,
               vidUser[i]->_myErrors, vidUser[i]->_prefetches);
        
        uint32 extraCount = 0;
        for (uint32 j = 14; j < QtimeCache_THREAD_CNT*5; j++)
            extraCount += vidUser[i]->_ref_count[j];
        printf("Ref Count 0: %d 1: %d 2: %d >=14: %d\n", vidUser[i]->_ref_count[0],
               vidUser[i]->_ref_count[1], vidUser[i]->_ref_count[2], extraCount);
        printf("Ref Count 3: %d 4: %d 5: %d 6: %d 7: %d 8: %d 9: %d 10: %d "
               "11: %d 12: %d 13: %d\n", vidUser[i]->_ref_count[3],
               vidUser[i]->_ref_count[4], vidUser[i]->_ref_count[5],
               vidUser[i]->_ref_count[6], vidUser[i]->_ref_count[7],
               vidUser[i]->_ref_count[8], vidUser[i]->_ref_count[9],
               vidUser[i]->_ref_count[10], vidUser[i]->_ref_count[11],
               vidUser[i]->_ref_count[12], vidUser[i]->_ref_count[13]);
        
        printf("Frame Touch 0 %d 1 %d 2 %d 3 %d 4 %d 5 %d 6 %d 7 %d\n\n",
               vidUser[i]->_frameTouch[0], vidUser[i]->_frameTouch[1],
               vidUser[i]->_frameTouch[2], vidUser[i]->_frameTouch[3],
               vidUser[i]->_frameTouch[4], vidUser[i]->_frameTouch[5],
               vidUser[i]->_frameTouch[6], vidUser[i]->_frameTouch[7]);
    }
#endif
    
    QtimeCache::QtimeCacheDtor(cacheP);
}
