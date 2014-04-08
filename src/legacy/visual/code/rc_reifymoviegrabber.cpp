/*
 *  rc_reifymoviegrabber.cpp
 *
 *  Created by Peter Roberts on Thu Nov 14 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */
#include <rc_types.h>
#include <rc_reifymoviegrabber.h>

rcReifyMovieGrabber::rcReifyMovieGrabber(rcVideoCache& cache) 
  : rcFileGrabber(0), _framesLeft(0), _curFrame(0), _started(false),
    _cache(cache)
{
  if (!_cache.isValid())
    setLastError(errorNoTranslate(_cache.getFatalError()));
}

rcReifyMovieGrabber::~rcReifyMovieGrabber()
{
}

bool rcReifyMovieGrabber::start()
{
  if (!isValid())
    return false;

  if (_started == false) {
    _framesLeft = _cache.frameCount();
    _curFrame = 0;
    _started = true;
  }

  return true;
}

bool rcReifyMovieGrabber::stop()
{
  if (!isValid())
    return false;

  _started = false;

  return true;
}
    
rcFrameGrabberStatus rcReifyMovieGrabber::getNextFrame(rcSharedFrameBufPtr& ptr,
						       bool isBlocking)
{
  if (!isValid())
    return eFrameStatusError;

  if (!isBlocking) {
    setLastError(eFrameErrorNotImplemented);
    return eFrameStatusError;
  }
  
  if (!_started && !start())
    return eFrameStatusError;

  if (_framesLeft == 0)
    return eFrameStatusEOF;

  _framesLeft--;

  /*
   * Get reference to frame, but don't lock it so it won't be forced
   * into memory before it is needed.
   */
  rcVideoCacheError error;
  rcVideoCacheStatus status = _cache.getFrame(_curFrame++, ptr, &error, false);

  if (status != eVideoCacheStatusOK) {
    setLastError(errorNoTranslate(error));
    return eFrameStatusError;
  }

  return eFrameStatusOK;
}

rcFrameGrabberError
rcReifyMovieGrabber::errorNoTranslate(rcVideoCacheError error)
{
  switch (error) {
  case eVideoCacheErrorFileInit:
    return eFrameErrorFileInit;
  case eVideoCacheErrorFileSeek:
  case eVideoCacheErrorFileRead:
    return eFrameErrorFileRead;
  case eVideoCacheErrorFileClose:
    return eFrameErrorFileClose;
  case eVideoCacheErrorFileFormat:
    return eFrameErrorFileFormat;
  case eVideoCacheErrorFileUnsupported:
    return eFrameErrorFileUnsupported;
  case eVideoCacheErrorFileRevUnsupported:
    return eFrameErrorFileRevUnsupported;
  case eVideoCacheErrorSystemResources:
    return eFrameErrorSystemResources;
  case eVideoCacheErrorNoSuchFrame:
  case eVideoCacheErrorCacheInvalid:
    return eFrameErrorInternal;
  case eVideoCacheErrorOK:
    return eFrameErrorOK;
  case eVideoCacheErrorBomUnsupported:
    return eFrameErrorFileUnsupported;
  case eVideoCacheErrorDepthUnsupported:
    return eFrameErrorUnsupportedDepth;
  }

  return eFrameErrorUnknown;
}
