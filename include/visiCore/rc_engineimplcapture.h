// Copyright (c) 2002 Reify Corp. All Rights reserved.

#ifndef _rcENGINEIMPLCAPTURE_H_
#define _rcENGINEIMPLCAPTURE_H_

// util
#include <rc_types.h>
#include <rc_thread.h>
#include <rc_atomic.h>
#include <rc_ring.h>

#include "rc_captureregs.h"
#include "rc_saveregs.h"

enum rcVideoCaptureState {
    eVideoCaptureState_Uninitialized = 0
  , eVideoCaptureState_MovieWait
  , eVideoCaptureState_MovieSave
  , eVideoCaptureState_MovieClose
};

class rcEngineImpl;
class rcEngineObserver;

class rcEngineMovieCapture : public rcRunnable {
 public:
  rcEngineMovieCapture(rcEngineImpl* engine,
		       rcVideoCaptureCtrl& captureCtrl,
		       const rcMovieSaveCtrlReader& saveCtrl,
		       rcEngineObserver* observer);
    
  virtual ~rcEngineMovieCapture() {}

  /* Accessor fcts.
   */

  const rcVideoCaptureStatusReader& captureStatus() const
  { return _captureStatus; }

  /* Pass through to rcEngineMovieSaver::delayLeft(). If there isn't
   * currently a movie saver, 0 is returned.
   */
  uint32 delayLeft();

  /* Pass through to rcEngineMovieSaver::state(). If there isn't
   * currently a movie saver, eMovieFileState_Uninitialized is returned.
   */
  rcMovieFileState saveState();

  /* Pass through to rcEngineMovieSaver::reset(). If there isn't
   * currently a movie saver, or the movie saver isn't in the state
   * eMovieFileState_Done, the fct is a nop.
   */
  void saveReset();

  /* Runnable implementation
   */
  virtual void run();

  static const uint32 _bufRingSz;

 private:
  /* Camera initialization/control helper fcts.
   */
  rcCameraGrabber* createGrabber();
  bool initializeCamera(rcCameraGrabber& grabber, char*& errorString,
			bool& allocError);
  bool getCameraState(rcCameraGrabber& grabber);
  bool updateCameraSettings(rcCameraGrabber& grabber);

  /* Movie save related helper fcts.
   */
  void sendShutdown(rcCameraGrabber& grabber);
  void startSaveThread();
  void closeSaveFile();
  void saveComplete();
  void killSaveThread();

  // Movie save specific data
  vector<rcSharedFrameBufPtr>              _frameBufPtrs;
  rcBidirectionalRing<rcSharedFrameBufPtr> _saveRing;
  rcConditionVariable                       _buffersInTransmit;
  rcAtomicValue<rcMovieFileErr>             _saveStatus;
  rcThread*		                    _saveThread;
  
  rcMutex                                   _saverMutex;
  rcEngineMovieSaver*                       _movieSaver;

  // General data
  rcEngineImpl*                             _engine;
  rcVideoCaptureCtrl&                       _captureCtrl;
  rcVideoCaptureStatus                      _captureStatus;
  const rcMovieSaveCtrlReader&              _saveCtrl;
  rcEngineObserver*                         _observer;
  rcAcqInfo                                 _acqInfo;
  rcAcqControl                              _acqControl;
  rcAcqControl                              _acqSavedControl;
};

#endif // _rcENGINEIMPLCAPTURE_H_
