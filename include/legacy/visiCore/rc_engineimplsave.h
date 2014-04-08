// Copyright (c) 2002 Reify Corp. All Rights reserved.

#ifndef _rcENGINEIMPLSAVE_H_
#define _rcENGINEIMPLSAVE_H_

#include <stdio.h>

// util
#include <rc_types.h>
#include <rc_thread.h>
#include <rc_atomic.h>
#include <rc_ring.h>
#include <rc_sparsehist.h>

// visual
#include <rc_framebuf.h>

#include "rc_saveregs.h"

/******************************************************************************
*	Threaded movie saver implementation class
******************************************************************************/

#define rcMOVIE_WARN_MSG_SZ 1024

class rcEngineImpl;
class rcEngineObserver;

class rcEngineMovieSaver : public rcRunnable {
 public:
  rcEngineMovieSaver(rcEngineImpl* engine,
                     const rcMovieSaveCtrlReader& saveCtrl,
                     const rcAcqInfo& acqInfo,
                     rcEngineObserver* observer,
                     rcBidirectionalRing<rcSharedFrameBufPtr>& saveRing,
                     rcConditionVariable& buffersInTransmit);
    
  virtual ~rcEngineMovieSaver() {}

  /* Runnable implementation
   */
  virtual void run();

  /* Accessor fcts
   */

  const rcMovieSaveStatusReader& saveStatus() const { return _saveStatus; }

  /* Mutator fcts
   */

  /* Resets object back to initial state.
   *
   * Note: Must be called when object is in state eMovieFileState_Done.
   */
  void reset();

  // Update acquisition info (camera control settings etc.)
  void updateAcqInfo( const rcAcqInfo& acqInfo );

private:
  void error_cleanup(rcSharedFrameBufPtr* sbp);
    
  /* State handling fcts.
   */

  /* uninitializedHdlr - Called at initialization time. Its one
   * function is to make sure all time critical handlers have been
   * called in an effort to make sure their code has all been swapped
   * in.
   */
  rcMovieFileState uninitializedHdlr(rcMovieFileState prevState);

  /* waitHdlr - Spin loops on usleep until user starts movie save.
   */
  rcMovieFileState waitHdlr(rcMovieFileState prevState);

  /* delayHdlr - Called if user specified delay before save. Spin
   * loops on usleep until either delay period expires or the user
   * cancels the movie save.
   */
  rcMovieFileState delayHdlr(rcMovieFileState prevState);

  /* openHdlr - Opens/Creates the movie file and writes a dummy
   * header.
   */
  rcMovieFileState openHdlr(rcMovieFileState prevState);

  /* saveHdlr - Saves frames to the movie file and generates most
   * values for the final movie header.
   *
   * Note: This is the one function where realtime constraints must be
   * stringently enforced. No slow or unpredictable processing should
   * be performed here that doesn't directly relate to the capture and
   * storage of a movie.
   */
  rcMovieFileState saveHdlr(rcMovieFileState prevState);

  /* errorHdlr - Does some randmow cleanup and sets state to done.
   */
  rcMovieFileState errorHdlr(rcMovieFileState prevState);

  /* closeHdlr - Completes setting up the movie header and stores
   * it to disk. Closes the movie and generates any necessary
   * warning info. Also entry point for the save of any XML data
   * associated with this movie save.
   */
  rcMovieFileState closeHdlr(rcMovieFileState prevState);

  /* doneHdlr - Make sure engine is in the stopped state and then
   * wait for capture thread to decide whether or not to restart
   * the save state machine.
   */
  rcMovieFileState doneHdlr(rcMovieFileState prevState);

  /* Private state - Used to share data between state handling
   * fcts. No access control required.
   */
  FILE*                                 _saveStream;
  rcSparseHistogram                     _hist;
  uint32                              _histOverflow;
  rcMovieFileFormat2                    _movieHdrRev2;
  rcTimestamp                           _prevTime;
  vector<int64>                       _toc;
  
  /* Shared state - Either accessible through accessor fcts or an
   * external object provided by an object running in a different
   * thread. Access control issues must be considered.
   */
  rcEngineImpl*                         _engine;
  const rcMovieSaveCtrlReader&          _saveCtrl;
  rcAcqInfo                             _acqInfo; // Mutable copy 
  rcMovieSaveStatus                     _saveStatus;
  rcEngineObserver*                     _observer;
  rcBidirectionalRing<rcSharedFrameBufPtr>& _saveRing;
  rcConditionVariable&                  _buffersInTransmit;
  rcAtomicValue<uint32>               _curResetValue;
  uint32                              _prevResetValue;
  std::string                              _saveFileName;
  char                                  _warningMsg[rcMOVIE_WARN_MSG_SZ];
};

#endif // _rcENGINEIMPLSAVE_H_
