/*
 *  rc_qvideograbber.h
 *
 *  Created by Peter Roberts on Thu Oct 17 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

/* Support for real time video acquisition */

#ifndef _rcQVIDEOGRABBER_H_
#define _rcQVIDEOGRABBER_H_

#include <rc_types.h>
#include <rc_framebuf.h>
#include <rc_atomic.h>
#include <rc_gen_capture.h>
#include <rc_framegrabber.h>
#include <vector>

class rcExecWithShmem;

using namespace std;

class rcCameraGrabber : public rcFrameGrabber {
  public:
    // Get acquire state info
    virtual rcFrameGrabberStatus getAcqInfo(rcAcqInfo& acqInfo, 
                                            bool isBlocking,
                                            bool releaseMemory = TRUE) = 0;

    // Set acquire control info
    virtual rcFrameGrabberStatus setAcqControl(rcAcqControl& acqCtrl,
                                               bool isBlocking,
                                               bool releaseMemory = TRUE) = 0;

    // Camera grabbers don't have a cache.
    virtual int32 cacheSize() { return 0; }

    // Set the number of frames to make available
    virtual void setFrameCount(int32 frameCount) = 0;

    // Add to set of frame buffers to use during acquire.
    virtual void addBuffer(rcSharedFrameBufPtr& buf) = 0;
};

//
// Class to grab frames from shared memory
//

class rcQVideoGrabber : public rcCameraGrabber {
  public:
    // ctor
    rcQVideoGrabber(char* progName, int arg1, char* arg2, uint32 sz,
                    uint8 childControlsBufferFirst = TRUE);
    // virtual dtor
    virtual ~rcQVideoGrabber();

    //
    // Implementation of base class rcCameraGrabber API
    //
    
    // Get acquire state info
    virtual rcFrameGrabberStatus getAcqInfo(rcAcqInfo& acqInfo, 
                                            bool isBlocking,
                                            bool releaseMemory = TRUE);

    // Set acquire control info
    virtual rcFrameGrabberStatus setAcqControl(rcAcqControl& acqCtrl,
                                               bool isBlocking,
                                               bool releaseMemory = TRUE);

    // Set the number of frames to make available
    virtual void setFrameCount(int32 frameCount);

    // Add to set of frame buffers to use during acquire
    virtual void addBuffer(rcSharedFrameBufPtr& buf);

    //
    // Implementation of base class rcFrameGrabber API
    //
    
    // Returns instance validity
    virtual bool isValid() const { return _isValid; }

    // Get last error
    virtual rcFrameGrabberError getLastError() const;

    // Start grabbing
    virtual bool start();

    // Stop grabbing
    virtual bool stop();
    
    // Returns the number of frames available
    virtual int32 frameCount();

    // Get next frame, assign the frame to ptr
    virtual rcFrameGrabberStatus getNextFrame( rcSharedFrameBufPtr& ptr,
                                               bool isBlocking );

    // Get name of input source, ie. file name, camera name etc.
    virtual const std::string getInputSourceName();
    
  private:
    rcExecWithShmem*        _shmemCtrlP; // Process/shmem creation object
    rcFrameGrabberError     _lastError;
    bool                    _isValid;
    int32                 _frameCount;
    vector<rcSharedFrameBufPtr> _buffers;
};

#endif // _rcQVIDEOGRABBER_H_

