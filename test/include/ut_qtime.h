
#ifndef _UT_QTCACHE_H_
#define _UT_QTCACHE_H_

#include "basic_ut.hpp"
#include "qtime_cache.h"
#include <atomic>
#include <boost/assign/std/vector.hpp> // for 'operator+=()'
#include <boost/assert.hpp>

using namespace boost::assign; // bring 'operator+=()' into scope

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
    
    
    std::vector<double> times;
    std::vector<double> expected;
    void fill_times ()
    {
        times +=
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
        0., 6., 8., 2000000000., 9000000000.;
    
        expected +=
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
        ,1.815, 1.815, 1.815, 1.815, 1.815, 1.815;
    }
    
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
