
#ifndef _UT_QTCACHE_H_
#define _UT_QTCACHE_H_

#include "rc_unittest.h"
#include "qtime_cache.h"
#include "rc_thread.h"
#include "rc_atomic.h"

typedef struct qutExpMapResult {
  eQtimeCacheError  expError;
  eQtimeCacheStatus expStatus;
  double             expTime;
} qutExpMapResult;

class UT_QtimeCache : public rcUnitTest {
 public:
  UT_QtimeCache(std::string movieFileName);
  ~UT_QtimeCache();

  virtual uint32 run();

 private:
  void ctorTest();
  void mappingTest();
  void simpleAllocTest();
  void prefetchTest();
  void prefetchThreadTest();
  void cacheFullTest();
  void frameBufTest();
  void dtorTest();
  void threadSafeTest();
};

// Thread-safe test specific stuff

#define FRAME_COUNT           8
#define QtimeCache_THREAD_CNT 5
#define QtimeCache_SZ         (QtimeCache_THREAD_CNT - 1)

class utQtimeCacheThread : public rcRunnable
{
public:

  utQtimeCacheThread(QtimeCache& cache);
  
  void run();

  uint32      _refCount[QtimeCache_THREAD_CNT*5];
  uint32      _frameTouch[FRAME_COUNT];
  uint32      _prefetches;
  uint32      _myErrors;
  QtimeCache& _cache;

  static rcAtomicValue<int> startTest;
};



#endif // _UT_QtimeCache_H_
