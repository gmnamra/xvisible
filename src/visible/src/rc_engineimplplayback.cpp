/******************************************************************************
 *   Copyright (c) 2003 Reify Corp. All Rights reserved.
 *
 *	rc_engineimplplayback.cpp
 *
 *	This file contains the engine video playback support.
 *
 ******************************************************************************/

#include <math.h>
#include <stdio.h>

// visual
#include <rc_window.h>

#include "rc_engineimpl.h"
#include "rc_engineimplplayback.h"

rcEngineMoviePlayback::rcEngineMoviePlayback(rcEngineImpl* engine,
					     const rcMoviePlaybackCtrlReader& playbackCtrl,
					     rcEngineObserver* observer,
					     double msPerUpdate,
					     bool noSkip)
  : _engine(engine), _playbackCtrl(playbackCtrl), _observer(observer),
    _firstTime(-1.0), _prefetchCnt(0), _msPerUpdate(msPerUpdate),
    _msPerFrame(0), _noSkip(noSkip), _cacheP(0)
{
  rmAssert(_engine);
  rmAssert(_msPerUpdate >= 1.0);
}

void rcEngineMoviePlayback::setupTimeCtrlPlayback(rcVideoCache& cache,
						  rcTimestamp startTime,
						  int32 rateMultiplier,
						  uint32 prefetchCnt)
{
  rmAssert(_playbackStatus.getState() != eMoviePlaybackState_Playback);
  rmAssert(rateMultiplier);
  _rateMultiplier = rateMultiplier;

  if (!rfGenerateTimeline(_timeline, _msPerFrame, cache))
    return;

  rcVideoCacheStatus status;
  rcVideoCacheError error;

  status = cache.firstTimestamp(_firstTime, &error);
  if (status != eVideoCacheStatusOK) {
    cerr << "Couldn't read first timestamp. Error: " 
	 << rcVideoCache::getErrorString(error) << endl;
    return;
  }

  _startTimeInMS = startTime.secs() * 1000;

  _prefetchCnt = prefetchCnt ? prefetchCnt : 1;
  _cacheP = &cache;
  _playbackStatus.state(eMoviePlaybackState_Initialized);
}

//#define DEBUG_PLAYBACK
#include <rc_timinginfo.h>

void rcEngineMoviePlayback::run()
{
  _engine->start();

  rmAssert(_playbackStatus.getState() == eMoviePlaybackState_Initialized);
  rmAssert(_cacheP);
  _playbackStatus.state(eMoviePlaybackState_Playback);

  /* If the requested update rate is slower than the maximum allowed,
   * adjust the actual update rate to match the requested update rate.
   */
  double rateMult = abs(_rateMultiplier);
  double msPerUpdate = 
    (_msPerUpdate > _msPerFrame/rateMult) ? _msPerUpdate : _msPerFrame/rateMult;

  /* Calculate which frames to display and when.
   */
  vector<pbRec> movieData;
  genMovieData(movieData, msPerUpdate);
  rmAssert(!movieData.empty());

#if 0
  fprintf(stderr, "Start Time %f ms MS per Update B: %f A: %f MS per Frame %f\n",
	  _startTimeInMS, _msPerUpdate, msPerUpdate, _msPerFrame);
  fprintf(stderr, "\nTimeline:\n");
  for (uint32 i = 0; i < _timeline.size(); i++) {
    if ((i & 0x7) == 0)
      fprintf(stderr, "\n%02d:", i);

    fprintf(stderr, " %04d", _timeline[i]);
  }
  fprintf(stderr, "\n");

  uint32 totUpdates = 0;
  fprintf(stderr, "\nMovie Data:\n");
  for (uint32 i = 0; i < movieData.size(); i++) {
    if ((i & 0x7) == 0)
      fprintf(stderr, "\n%02d:", i);

    fprintf(stderr, " (%04d,%02d)", movieData[i].frameIndex,
	    movieData[i].nextUpdateCnt);
    totUpdates += movieData[i].nextUpdateCnt;
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "Tot Updates %d Exp Time %f\n", totUpdates,
	  totUpdates*msPerUpdate);
  playbackCleanup(eVideoCacheErrorOK, 99);
  return;
#endif

  /* Start out by prefetching the first few frames.
   */
  rcVideoCacheStatus status;
  rcVideoCacheError error;

  uint32 prefetchCnt = (_prefetchCnt <= (int32)movieData.size()) ?
    _prefetchCnt : movieData.size();
  uint32 curPrefetch = 0;
  for ( ; curPrefetch < prefetchCnt; curPrefetch++) {
    rcSharedFrameBufPtr buf;
    status = _cacheP->getFrame(movieData[curPrefetch].frameIndex, buf,
			       &error, true);
    if (status != eVideoCacheStatusOK) {
      playbackCleanup(error, 0);
      return;
    }
  } // End of: for ( ; curPrefetch < prefetchCnt; curPrefetch++) {

  /* Main playback loop.
   */
  uint32 delayCnt = 0, droppedCnt = 0; // Debugging info
  rcTimingInfo profile(7, movieData.size() + 1); // Debugging info
  for (uint32 curDisplay = 0; 
       (curDisplay < movieData.size()) && !seppukuRequested();
       curDisplay++) {
    profile.nextPass(false);

    /* Read the count of unprocessed time events. If _noSkip is true,
     * wait for the count to go to 0.
     */
    int32 backupCnt = _observer->timeEventCount();
    while (backupCnt && _noSkip) {
      delayCnt++;
      rcThread::sleep((uint32)(msPerUpdate + .5));
      backupCnt = _observer->timeEventCount();
    }
    profile.touch(0);

    /* Prefetch the next frame.
     */
    if (curPrefetch < movieData.size()) {
      rcSharedFrameBufPtr pBuf;
      status = _cacheP->getFrame(movieData[curPrefetch].frameIndex, pBuf,
				 &error, false);
      if (status != eVideoCacheStatusOK) {
	playbackCleanup(error, 1);
	return;
      }
      profile.touch(1);
      pBuf.prefetch();
      profile.touch(2);
      curPrefetch++;
    } // End of: if (curPrefetch < movieData.size()) {
    
    /* If display side isn't backed up, send new time event to cause
     * next frame to display. Note that backupCnt will only be true if
     * _noSkip is false.
     */
    if (backupCnt)
      droppedCnt++;
    else {
      rcSharedFrameBufPtr buf;
      profile.touch(3);
      status = _cacheP->getFrame(movieData[curDisplay].frameIndex, buf,
				 &error, true);
      profile.touch(4);
      if (status != eVideoCacheStatusOK) {
	playbackCleanup(error, 2);
	return;
      }

      /* Among other things, this causes monitor to update to new frame.
       */
      _observer->notifyTime(buf->timestamp() - _firstTime);
      profile.touch(5);
    } // End of: if (!backupCnt) {
    
    /* Let frame display for the appropriate amount of time.
     */
    profile.touch(6);
    uint32 msToSleep =
      (uint32)(msPerUpdate * movieData[curDisplay].nextUpdateCnt + .5);
    rcThread::sleep(msToSleep);
  }
  fprintf(stderr, "dc %d dt %d sc %d\n", delayCnt,
	  delayCnt*(uint32)(msPerUpdate + .5), droppedCnt);
  //  profile.printInfo(movieData.size() + 1);

  playbackCleanup(eVideoCacheErrorOK, 3);
}

void rcEngineMoviePlayback::genMovieData(vector<pbRec>& movieData,
					 double msPerUpdate)
{
  uint32 updateCnt = 1;
  uint32 curFrame = 0xFFFFFFFF;
  movieData.resize(0);
  double curTime = (_startTimeInMS < 0.0) ? 0.0 : _startTimeInMS;
  uint32 slotIndex = (uint32)(curTime/_msPerFrame + .5);
  if (slotIndex >= _timeline.size()) {
    slotIndex = _timeline.size() - 1;
    curTime = slotIndex * _msPerFrame;
  }

  while (slotIndex < _timeline.size()) {
    if (curFrame != _timeline[slotIndex]) {
      if (!movieData.empty()) {
	rmAssert(movieData.back().frameIndex == curFrame);
	movieData.back().nextUpdateCnt = updateCnt;
      }
      updateCnt = 0;
      curFrame = _timeline[slotIndex];
      movieData.push_back(pbRec(curFrame, 1));
    }
    updateCnt++;
    curTime += msPerUpdate * _rateMultiplier;
    slotIndex = (uint32)(curTime/_msPerFrame + .5);
  }
}

void rcEngineMoviePlayback::playbackCleanup(rcVideoCacheError error,
					    uint32 eIndex)
{
  if (error != eVideoCacheErrorOK)
    cerr << "Error reading frame during playback at: " << eIndex << ". Error: " 
	 << rcVideoCache::getErrorString(error) << endl;
  _timeline.resize(0);
  _playbackStatus.state(eMoviePlaybackState_Exit);
  _engine->stop();
  _observer->notifyStatus("Ready");
}
