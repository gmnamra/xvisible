// Copyright 2003 Reify, Inc.

#include <stdlib.h>

#include "ut_reifymoviegrabber.h"

#include <rc_windowhist.h>

static std::string fileName;

UT_ReifyMovieGrabber::UT_ReifyMovieGrabber(std::string movieFileName)
{
  fileName = movieFileName;
}

UT_ReifyMovieGrabber::~UT_ReifyMovieGrabber()
{
  printSuccessMessage("rcReifyMovieGrabber test", mErrors);
}

uint32 UT_ReifyMovieGrabber::run()
{
  simpleTest();

  return mErrors;
}

void UT_ReifyMovieGrabber::simpleTest()
{
  const uint32 frameCount = 10;

  double expectedTimes[frameCount] = {
    2., 3., 4., 10., 100., 1000000000., 4294967296., 8589934592., 8589934593., 8589934594.
  };

  rcVideoCache* cacheP = rcVideoCache::rcVideoCacheCtor(fileName, 10, true,
							false, false);

  rcUNITTEST_ASSERT(cacheP->isValid());

  if (!cacheP->isValid())
    return; // No point in continuing if cache is not valid.

  rcReifyMovieGrabber grabber(*cacheP);

  rcUNITTEST_ASSERT(grabber.isValid());
  if (!grabber.isValid())
    return; // No point in continuing if grabber is not valid.

  rcUNITTEST_ASSERT(grabber.getLastError() == eFrameErrorOK);
  rcUNITTEST_ASSERT(grabber.frameCount() == (uint32)frameCount);
  rcUNITTEST_ASSERT(grabber.getInputSourceName() == fileName);

  bool startOK = grabber.start();
  rcUNITTEST_ASSERT(startOK);

  uint32 frameIndex = 0;
  rcSharedFrameBufPtr ptr;
  rcFrameGrabberStatus status;
  rc256BinHist hist(256);

  while ((status = grabber.getNextFrame(ptr, true))) {
    rcUNITTEST_ASSERT(ptr->timestamp() == expectedTimes[frameIndex]);

    rcWindow win(ptr);
    rfGenDepth8Histogram(win, hist);
    rcUNITTEST_ASSERT(hist[frameIndex] == win.pixelCount());
    frameIndex++;
  }

  rcUNITTEST_ASSERT(frameIndex == frameCount);
  rcUNITTEST_ASSERT(status == eFrameStatusEOF);
  rcUNITTEST_ASSERT(grabber.isValid());
  rcUNITTEST_ASSERT(grabber.getLastError() == eFrameErrorOK);

  bool stopOK = grabber.stop();
  rcUNITTEST_ASSERT(stopOK);

  frameIndex = 0;
  while ((status = grabber.getNextFrame(ptr, true))) {
    rcWindow win(ptr);

    rfGenDepth8Histogram(win, hist);
    rcUNITTEST_ASSERT(hist[frameIndex] == win.pixelCount());
    frameIndex++;
  }
  ptr = 0;

  rcVideoCache::rcVideoCacheDtor(cacheP);

  std::string badFile("xyzzy.dat");

  rcVideoCache* badCacheP =
    rcVideoCache::rcVideoCacheCtor(badFile, 10, true, false, false);

  rcUNITTEST_ASSERT(badCacheP->isValid() == false);
  rcReifyMovieGrabber badGrabber(*badCacheP);

  rcUNITTEST_ASSERT(badGrabber.isValid() == false);
  rcUNITTEST_ASSERT(badGrabber.getLastError() == eFrameErrorFileInit);

  rcVideoCache::rcVideoCacheDtor(badCacheP);
}
