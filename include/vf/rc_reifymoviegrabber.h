/*
 *  rc_reifymoviegrabber.h
 *
 *  Created by Peter Roberts on Thu Nov 14 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcREIFYMOVIEGRABBER_H_
#define _rcREIFYMOVIEGRABBER_H_

#include "rc_types.h"
#include "rc_filegrabber.h"
#include "rc_moviefileformat.h"
#include "rc_videocache.h"
//
// Class to grab frames from a movie file
//

class rcReifyMovieGrabber : public rcFileGrabber {
 public:
  // ctor
  rcReifyMovieGrabber(rcVideoCache& cache);
  // virtual dtor
  virtual ~rcReifyMovieGrabber();

  //
  // rcFrameGrabber API
  //

  // Start grabbing
  virtual bool start();

  // Stop grabbing
  virtual bool stop();
    
  // Returns the number of frames available
  virtual int32 frameCount() { return _cache.frameCount(); }

  // Returns the size of the cache, in frames.
  virtual int32 cacheSize() { return _cache.cacheSize(); }

  // Get next frame, assign the frame to ptr
  virtual rcFrameGrabberStatus getNextFrame(rcSharedFrameBufPtr& ptr,
					    bool isBlocking);

  // Get name of input source, ie. file name, camera name etc.
  const std::string getInputSourceName()
  { return _cache.getInputSourceName(); }

 private:
  static rcFrameGrabberError errorNoTranslate(rcVideoCacheError error);

  int32             _framesLeft;
  int32             _curFrame;
  bool                _started;
  rcVideoCache&       _cache;
};

#endif // _rcREIFYMOVIEGRABBER_H_

