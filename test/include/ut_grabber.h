/*
 *  ut_grabber.h
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */

#ifndef _UT_GRABBER_H_
#define _UT_GRABBER_H_

#include <rc_framegrabber.h>
#include <rc_framebuf.h>

class utGrabber : public rcFrameGrabber {
  public:
    // ctor
    utGrabber(vector<rcSharedFrameBufPtr>& buffers)
      : _buffers(buffers), _isValid (buffers.size()), _currentIndex(0),
        _lastError(eFrameErrorOK)
    { }

    // virtual dtor
    virtual ~utGrabber() { };

    //
    // rcFrameGrabber API
    //
    
    // Returns instance validity
    virtual bool isValid() const { return _isValid; }

    // Get last error
    virtual rcFrameGrabberError getLastError() const { return _lastError; }

    // Start grabbing
    virtual bool start() { return true; }

    // Stop grabbing
    virtual bool stop() { return true; }
    
    // Returns the number of frames available
    virtual int32 frameCount()
    {
      return _buffers.size();
    }

    // UT grabbers don't have a cache.
    virtual int32 cacheSize() { return 0; }

    // Get next frame, assign the frame to ptr
    virtual rcFrameGrabberStatus getNextFrame(rcSharedFrameBufPtr& ptr, bool)
    {
      if (_currentIndex < _buffers.size()) {
	ptr = _buffers[_currentIndex++];
	return eFrameStatusOK;
      }
      else {
	ptr = 0;
	return eFrameStatusEOF;
      }
    }

    // Get name of input source, ie. file name, camera name etc.
    virtual const std::string getInputSourceName() { return "Unit Test Grabber"; }
    
  private:
    vector<rcSharedFrameBufPtr>& _buffers;
    bool                           _isValid;
    uint32                       _currentIndex; // Index to next image
    rcFrameGrabberError            _lastError;
};


#endif
