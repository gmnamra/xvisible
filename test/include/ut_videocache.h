// Copyright 2003 Reify, Inc.

#ifndef _UT_VIDEOCACHE_H_
#define _UT_VIDEOCACHE_H_

#include <rc_unittest.h>
#include <rc_videocache.h>
#include <rc_thread.h>
#include <rc_atomic.h>

typedef struct utExpMapResult {
  rcVideoCacheError  expError;
  rcVideoCacheStatus expStatus;
  double             expTime;
} utExpMapResult;

class UT_VideoCache : public rcUnitTest {
 public:
  UT_VideoCache(std::string movieFileName);
  ~UT_VideoCache();

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
#define VIDEOCACHE_THREAD_CNT 5
#define VIDEOCACHE_SZ         (VIDEOCACHE_THREAD_CNT - 1)

class utVideoCacheThread : public rcRunnable
{
public:

  utVideoCacheThread(rcVideoCache& cache);
  
  void run();

  uint32      _refCount[VIDEOCACHE_THREAD_CNT*5];
  uint32      _frameTouch[FRAME_COUNT];
  uint32      _prefetches;
  uint32      _myErrors;
  rcVideoCache& _cache;

  static rcAtomicValue<int> startTest;
};



#endif // _UT_VIDEOCACHE_H_
