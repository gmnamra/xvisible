/*
 *  rc_reifymoviegrabber.cpp
 *
 *  Created by Peter Roberts on Thu Nov 14 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */
#include "rc_types.h"
#include "rc_reifymoviegrabber.h"


template<> rcFrameGrabberError
rcReifyMovieGrabberT<rcVideoCache,rcVideoCacheError>::errorNoTranslate(rcVideoCacheError error)
{
    switch (error) {
        case rcVideoCacheError::FileInit:
            return rcFrameGrabberError::eFrameErrorFileInit;
        case rcVideoCacheError::FileSeek:
        case rcVideoCacheError::FileRead:
            return rcFrameGrabberError::eFrameErrorFileRead;
        case rcVideoCacheError::FileClose:
            return rcFrameGrabberError::eFrameErrorFileClose;
        case rcVideoCacheError::FileFormat:
            return rcFrameGrabberError::eFrameErrorFileFormat;
        case rcVideoCacheError::FileUnsupported:
            return rcFrameGrabberError::eFrameErrorFileUnsupported;
        case rcVideoCacheError::FileRevUnsupported:
            return rcFrameGrabberError::eFrameErrorFileRevUnsupported;
        case rcVideoCacheError::SystemResources:
            return rcFrameGrabberError::eFrameErrorSystemResources;
        case rcVideoCacheError::NoSuchFrame:
        case rcVideoCacheError::CacheInvalid:
            return rcFrameGrabberError::eFrameErrorInternal;
        case rcVideoCacheError::OK:
            return rcFrameGrabberError::eFrameErrorOK;
        case rcVideoCacheError::BomUnsupported:
            return rcFrameGrabberError::eFrameErrorFileUnsupported;
        case rcVideoCacheError::DepthUnsupported:
            return rcFrameGrabberError::eFrameErrorUnsupportedDepth;
    }
    
    return rcFrameGrabberError::eFrameErrorUnknown;
}


template<>
rcReifyMovieGrabberT<rcVideoCache,rcVideoCacheError>::rcReifyMovieGrabberT(rcVideoCache& cache)
  : rcFileGrabber(0), _framesLeft(0), _curFrame(0), _started(false),
    _cache(cache)
{
  if (!_cache.isValid())
    setLastError(errorNoTranslate(_cache.getFatalError()));
}

template <>
rcReifyMovieGrabberT<rcVideoCache,rcVideoCacheError>::~rcReifyMovieGrabberT()
{
}


template<>
bool rcReifyMovieGrabberT<rcVideoCache,rcVideoCacheError>::start()
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

template<>
bool rcReifyMovieGrabberT<rcVideoCache,rcVideoCacheError>::stop()
{
  if (!isValid())
    return false;

  _started = false;

  return true;
}


template<>
rcFrameGrabberStatus rcReifyMovieGrabberT<rcVideoCache,rcVideoCacheError>::getNextFrame(rcFrameRef& ptr,
						       bool isBlocking)
{
  if (!isValid())
    return eFrameStatusError;

  if (!isBlocking) {
    setLastError(rcFrameGrabberError::eFrameErrorNotImplemented);
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

