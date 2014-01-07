// Copyright 2003 Reify, Inc.

#include <math.h>

// visual
#include <rf_playback_utils.h>

#include <ut_playback_utils.h>

UT_PlaybackUtils::UT_PlaybackUtils()
{
}

UT_PlaybackUtils::~UT_PlaybackUtils()
{
  printSuccessMessage("Playback Utils test", mErrors);
}

uint32 UT_PlaybackUtils::run()
{
  testGenerateTimeline();

  return mErrors;
}

#define genValue(MASK) ((rand() >> 4) & MASK)

void UT_PlaybackUtils::testGenerateTimeline()
{
  const double secPerFrame = 1.0;
  const int delayMask = 0x7;
  const int delayTimeMask = 0x1FF;
  const double maxDelayTime = delayTimeMask + 1;
  const double maxDelayMs = 400;
  const int skipMask = 0x1F;
  const int maxConsecutiveSkips = 5;
  const int jitterMask = 0x7;
  const int jitterBase = 2;
  const int totalTests = 10000;
  const int frameCountBase = 2;
  const int frameCountMask = 0x3FF;

  srand(0);

  double minAvg = 1e99;
  double maxAvg = -1;
  double cumAvg = 0.0;

  for (int testRun = 0; testRun < totalTests; testRun++) {
    /* Generate a test vector of times and its corresponding expected
     * results vector.
     */
    vector<rcTimestamp> testVector;
    vector<uint32> expResult;
    int frameCount = genValue(frameCountMask) + frameCountBase;
    double curTime = 0.0;
    int curConsecutiveSkips = 0;
    int skipCount = 0;
    int frameIndex = 0;
    int curSlot = 0;
    int jitterCount = genValue(jitterMask) + jitterBase;
    while (frameIndex < frameCount) {
      /* Check for frame skip. Never skip 1st frame and don't exceed
       * maximum consecutive count.
       */
      if (frameIndex && (curConsecutiveSkips < maxConsecutiveSkips) &&
	  (genValue(skipMask) == 0)) {
	skipCount++;
	curConsecutiveSkips++;
	expResult.push_back(frameIndex-1);
      }
      else {
	curConsecutiveSkips = 0;
	double delayTime = 0.0;
	if (genValue(delayMask) == 0)
	  delayTime =
	    ((genValue(delayTimeMask) + 1)*maxDelayMs)/(maxDelayTime*1000);
	if (curSlot && ((curSlot % jitterCount) == 0))
	  curTime += .001;

	testVector.push_back(rcTimestamp(curTime + delayTime));
	expResult.push_back(frameIndex);
	frameIndex++;
      }
      curSlot++;
      curTime += secPerFrame;
    } // End of: while (frameIndex < frameCount) {

    rmAssert((testVector.size() + (long)skipCount) == expResult.size());

    /* Create a test video cache using the test vector and use this as
     * input to the fct to be tested. Verify that the video cache
     * seems to be operating properly before continuing on to actual
     * test.
     */
    rcVideoCache* cacheP = rcVideoCache::rcVideoCacheUTCtor(testVector);
    if (testVector.size() != cacheP->frameCount()) {
      rcUNITTEST_ASSERT(testVector.size() == cacheP->frameCount());
      return;
    }
    for (uint32 i = 0; i < testVector.size(); i++) {
      rcVideoCacheError error;
      rcVideoCacheStatus status;
      rcTimestamp actual;
      status = cacheP->frameIndexToTimestamp(i, actual, &error);
      if ((status != eVideoCacheStatusOK) ||
	  (actual != testVector[i])) {
	rcUNITTEST_ASSERT(status == eVideoCacheStatusOK);
	rcUNITTEST_ASSERT(actual == testVector[i]);
	return;
      }
    }

    vector<uint32> actResult;
    double msPerFrame;
    bool success = rfGenerateTimeline(actResult, msPerFrame, *cacheP);
    
    if (success) {
      if (msPerFrame > maxAvg)
	maxAvg = msPerFrame;
      if (msPerFrame < minAvg)
	minAvg = msPerFrame;
      cumAvg += msPerFrame;

      uint32 slotCount = actResult.size();
      if (slotCount != expResult.size()) {
	fprintf(stderr, "as %ld es %ld\n", actResult.size(), expResult.size());
	rcUNITTEST_ASSERT(slotCount == expResult.size());
	if (slotCount > expResult.size())
	  slotCount = expResult.size();
      }
      for(uint32 i = 0; i < slotCount; i++)
	rcUNITTEST_ASSERT(actResult[i] == expResult[i]);
    }
    else
      rcUNITTEST_ASSERT(success);

    rcVideoCache::rcVideoCacheDtor(cacheP);
  } // End of: for (int testRun = 0; testRun < totalTests; testRun++) {
#if 0
  fprintf(stderr, "Min Avg %f Max Avg %f Avg Avg %f\n", minAvg, maxAvg,
	  cumAvg/totalTests);
#endif
}
