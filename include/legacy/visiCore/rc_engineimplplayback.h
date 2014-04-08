// Copyright (c) 2003 Reify Corp. All Rights reserved.

#ifndef _rcENGINEIMPLPLAYBACK_H_
#define _rcENGINEIMPLPLAYBACK_H_

// util
#include <rc_types.h>
#include <rc_thread.h>
#include <rc_atomic.h>
#include <rc_timestamp.h>

// visual
#include <rc_videocache.h>
#include <rf_playback_utils.h>

#include "rc_playbackregs.h"

class rcEngineImpl;
class rcEngineObserver;

class rcEngineMoviePlayback : public rcRunnable {
 public:
  rcEngineMoviePlayback(rcEngineImpl* engine,
			const rcMoviePlaybackCtrlReader& playbackCtrl,
			rcEngineObserver* observer, double msPerUpdate,
			bool noSkip);
    
  virtual ~rcEngineMoviePlayback() {}

  /* Mutator fcts.
   */
  void setupTimeCtrlPlayback(rcVideoCache& cache, rcTimestamp startTime,
			     int32 rateMultiplier, uint32 prefetchCnt);

  void reset() {
    if (_playbackStatus.getState() == eMoviePlaybackState_Exit)
      _playbackStatus.state(eMoviePlaybackState_Uninitialized);
  }

  /* Accessor fcts.
   */
  const rcMoviePlaybackStatusReader& playbackStatus() const
  { return _playbackStatus; }

  /* Runnable implementation
   */
  virtual void run();

 private:

  class pbRec
  {
  public:
    pbRec(uint32 fi = 0, uint32 updateCnt = 1)
      : frameIndex(fi), nextUpdateCnt(updateCnt)
    {
    }

    uint32 frameIndex;    // Frame's cache index
    uint32 nextUpdateCnt; // Update time units to wait before new frame
  };

  /* genMovieData - Generate vector of playback information from
   * _timeline and client provided parameters _firstTime, _msPerUpdate
   * and _rateMultiplier.
   */
  void genMovieData(vector<pbRec>& movieData, double msPerUpdate);

  void playbackCleanup(rcVideoCacheError error, uint32 eIndex);

  rcEngineImpl*                    _engine;
  const rcMoviePlaybackCtrlReader& _playbackCtrl;
  rcMoviePlaybackStatus            _playbackStatus;
  rcEngineObserver*                _observer;
  rcTimestamp                      _firstTime;
  int32                          _prefetchCnt;
  int32                          _rateMultiplier;
  vector<uint32>                 _timeline;
  double                           _startTimeInMS;
  double                           _msPerUpdate;
  double                           _msPerFrame;
  const bool                       _noSkip;
  rcVideoCache*                    _cacheP;
};

#endif
