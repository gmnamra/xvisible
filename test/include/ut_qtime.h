
#ifndef _UT_QTCACHE_H_
#define _UT_QTCACHE_H_

#include "basic_ut.hpp"
#include "qtime_cache.h"
#include <atomic>

typedef struct qutExpMapResult {
  QtimeCacheError  expError;
  QtimeCacheStatus expStatus;
  double             expTime;
} qutExpMapResult;

class UT_QtimeCache : public rcUnitTest {
 public:
  UT_QtimeCache(std::string movieFileName);
  ~UT_QtimeCache();

    virtual uint32 run();
 //   int get_which () { return which; }
 //   void set_which (int wh = 0) { which = wh; }
    

 private:
  void ctorTest();
  void mappingTest();
  void simpleAllocTest();
  void prefetchTest();
  void prefetchThreadTest();
  void cacheFullTest();
  void frameBufTest();
  void dtorTest();
 // void threadSafeTest();
  //  void threadSafe (bool);
  //  int which;
};

// Thread-safe test specific stuff


#define QtimeCache_THREAD_CNT 5
#define QtimeCache_SZ         (QtimeCache_THREAD_CNT - 1)

class utQtimeCacheThread
{
public:
    utQtimeCacheThread (QtimeCache& cache);
  
    void run ();
  
  uint32      _ref_count[QtimeCache_THREAD_CNT*5];
  std::vector<uint32>    _frameTouch;
  uint32      _prefetches;
  uint32      _myErrors;
  uint32 _frame_count;
  QtimeCache&  _cache;
    static std::atomic<int> startTest;
};



#endif // _UT_QtimeCache_H_
