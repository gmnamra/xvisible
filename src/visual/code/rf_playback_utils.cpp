/*
 *  rf_playback_utils.cpp
 *
 *  Copyright (c) 2003 Reify Corp. All Rights reserved.
 */

// util

#include <rf_playback_utils.h>
#include <rc_sparsehist.h>
#include <rc_timestamp.h>

bool rfGenerateTimeline(vector<uint32>& timeline, double& msPerFrame,
			rcVideoCache& cache)
{
  /* A movie must contain at least two frames,
   */
  if (cache.frameCount() < 2) {
    cerr << "Minimum movie size is 2 frames." << endl;
    return false;
  }

  rcVideoCacheStatus status;
  rcVideoCacheError error;

  /* Step 1a - Generate a histogram of time deltas between consecutive
   * frames.
   */
  rcTimestamp firstTime, lastTime, nextTime;

  status = cache.lastTimestamp(lastTime, &error);
  if (status != eVideoCacheStatusOK) {
    cerr << "Couldn't read last timestamp. Error: " 
	 << rcVideoCache::getErrorString(error) << endl;
    return false;
  }

  status = cache.firstTimestamp(firstTime, &error);
  if (status != eVideoCacheStatusOK) {
    cerr << "Couldn't read first timestamp. Error: " 
	 << rcVideoCache::getErrorString(error) << endl;
    return false;
  }

  rcSparseHistogram timeDelta;

  rcTimestamp curTime = firstTime;
  do {
    status = cache.nextTimestamp(curTime, nextTime, 0);
    rmAssert(status == eVideoCacheStatusOK);

    rcTimestamp deltaTime(nextTime - curTime);
    double deltaMS = deltaTime.secs() * 1000;
    uint32 iDeltaMS = (uint32)floor(deltaMS);
    if ((deltaMS - iDeltaMS) > 1.0) {
      cerr << "Interframe delta exceeded maximum supported: " 
	   << deltaMS << endl;
      return false;
    }
    timeDelta.add(iDeltaMS);

    curTime = nextTime;
  } while (curTime != lastTime);

  rmAssert(timeDelta.isValid());

  /* Step 1b - Calculate the dominate time delta. This will be stored
   * in maxBin and will be used as the nominal time delta for the
   * purposes of screening out missed frames from the final time delta
   * calculation.
   */
  uint32 maxBinCount = 0;
  uint32 maxBin = 0;
  const rcSparseHistogram::sparseArray& deltaInfo = timeDelta.getArray();

  rcSparseHistogram::sparseArray::const_iterator start = deltaInfo.begin();
  for ( ; start != deltaInfo.end(); start++) {
    if (start->second > maxBinCount) {
      maxBinCount = start->second;
      maxBin = start->first;
    }
  }
  rmAssert(maxBinCount);
  rmAssert(maxBin);

  /* Step 1c - Calculate the average time delta for those deltas at
   * maxBin and those in the larger bin +/- 1 of maxBin. This will be
   * returned to caller for use as the nominal interframe delay.
   */
  uint32 belowCount = 0;
  uint32 aboveCount = 0;
  rcSparseHistogram::sparseArray::const_iterator loc = deltaInfo.find(maxBin-1);
  if (loc != deltaInfo.end())
    belowCount = loc->second;
  loc = deltaInfo.find(maxBin+1);
  if (loc != deltaInfo.end())
    aboveCount = loc->second;

  if (maxBinCount == 1) {
    msPerFrame = timeDelta.average();
  }
  else if (belowCount > aboveCount) {
    double numerator = maxBinCount*maxBin + belowCount*(maxBin-1);
    double denominator = maxBinCount + belowCount;
    msPerFrame = numerator/denominator;
  }
  else {
    double numerator = maxBinCount*maxBin + aboveCount*(maxBin+1);
    double denominator = maxBinCount + aboveCount;
    msPerFrame = numerator/denominator;
  }

  /* Step 2 - Use the just calculated average delta in creating a
   * timeline.  The timeline consists of an array describing the
   * closest integral number of average delta times between
   * consecutive video frames. Entry N in the timeline describes the
   * delta between frames N and N + 1.
   *
   * Note that this loop doesn't store a value for the last frame.  It
   * is assumed the last frame always lasts 1 average delta time.
   */
  timeline.resize(0);
  curTime = firstTime;
  uint32 curFrame = 0;
  do {
    rmAssert(curFrame < cache.frameCount());
    status = cache.nextTimestamp(curTime, nextTime, 0);
    rmAssert(status == eVideoCacheStatusOK);

    rcTimestamp deltaTime(nextTime - curTime);
    double deltaMS = deltaTime.secs() * 1000;
    uint32 deltaCount = (uint32)(deltaMS/msPerFrame + .5);
    if (!deltaCount)
      deltaCount = 1;
    while (deltaCount--)
      timeline.push_back(curFrame);

    curFrame++;
    curTime = nextTime;
  } while (curTime != lastTime);

  rmAssert(curFrame == (cache.frameCount() - 1));
  timeline.push_back(curFrame);

  rmAssert(cache.frameCount() <= timeline.size());
  
  return true;
}
