// Copyright (c) 2002 Reify Corp. All Rights reserved.

#ifndef _rcSAVEREGS_H_
#define _rcSAVEREGS_H_

#include <rc_types.h>
#include <rc_atomic.h>
#include <rc_thread.h>

// Maximum number of frames that can be captured in one session
const uint32 cMaxSaveFrameCount = 524288;

enum rcMovieFileErr {
    eMovieFileErr_NoErr = 0
  , eMovieFileErr_Aborted
  , eMovieFileErr_OpenFailed
  , eMovieFileErr_WriteFailed
  , eMovieFileErr_NullInput
  , eMovieFileErr_UnsupportedType
  , eMovieFileErr_NotHomogenous
  , eMovieFileErr_SeekFailed
  , eMovieFileErr_CloseFailed
  , eMovieFileErr_VariableFrameRate
  , eMovieFileErr_MaxFramesExceeded
};

enum rcMovieFileState {
    eMovieFileState_Uninitialized = 0
  , eMovieFileState_Wait
  , eMovieFileState_Delay
  , eMovieFileState_Open
  , eMovieFileState_Save
  , eMovieFileState_Close
  , eMovieFileState_Error
  , eMovieFileState_Done
  , eMovieFileState_Exit
};

/* rcMovieSaveCtrl and rcMovieSaveCtrlReader provide the register set
 * required to allow control of movie saving.
 *
 * The movie saver client should instantiate a rcMovieSaveCtrl class
 * object and make a reference to the rcMovieSaveCtrlReader base class
 * available to the movie saver object.
 */
class rcMovieSaveCtrlReader
{
 public:
  rcMovieSaveCtrlReader()
    : _frameCaptureTime(0), _frameCaptureDelayTime(0), _outputFileName(""),
    _movieFileName("")
  { }

  virtual ~rcMovieSaveCtrlReader()
  { }

  /* Returns capture time in units of minutes. Value of 0 ==> infinite
   * capture time.
   */
  uint32 getFrameCaptureTime() const
  {
    uint32 x;
    return _frameCaptureTime.getValue(x);
  }
  
  /* Returns capture delay time in units of minutes. Value of 0 ==> no
   * delay.
   */
  uint32 getFrameCaptureDelayTime() const
  {
    uint32 x;
    return _frameCaptureDelayTime.getValue(x);
  }
  
  std::string getOutputFileName() const
  {
    rcMutex* mutex = const_cast<rcMutex*>(&_mutex);

    rcLock lock(*mutex);

    std::string retVal = _outputFileName;

    return retVal;
  }

  std::string getMovieFileName() const
  {
    rcMutex* mutex = const_cast<rcMutex*>(&_mutex);

    rcLock lock(*mutex);

    std::string retVal = _movieFileName;

    return retVal;
  }

 protected:
  /* Control information contained in atomic variables.
   */
  rcAtomicValue<uint32> _frameCaptureTime;
  rcAtomicValue<uint32> _frameCaptureDelayTime;

  /* Mutex synchronized control information.
   */
  rcMutex                 _mutex;
  std::string                _outputFileName;
  std::string                _movieFileName;
};

class rcMovieSaveCtrl : public rcMovieSaveCtrlReader
{
 public:
  rcMovieSaveCtrl() : rcMovieSaveCtrlReader()
  { }

  void frameCaptureTime(uint32 frameCaptureTime)
  {
    _frameCaptureTime.setValue(frameCaptureTime);
  }

  void frameCaptureDelayTime(uint32 frameCaptureDelayTime)
  {
    _frameCaptureDelayTime.setValue(frameCaptureDelayTime);
  }

  void outputFileName(const std::string& outputFileName)
  {
    rcLock lock(_mutex);
    _outputFileName = outputFileName;
  }

  void movieFileName(const std::string& movieFileName)
  {
    rcLock lock(_mutex);
    _movieFileName = movieFileName;
  }
};

/* rcMovieSaveStatus and rcMovieSaveStatusReader provide the register
 * set required to allow monitoring of the current state of the movie
 * save object.
 *
 * The movie save object should instantiate a rcMovieSaveStatus class
 * object and make a reference to the rcMovieSaveStatusReader base
 * class available to the movie save client.
 */
class rcMovieSaveStatusReader
{
 public:
  rcMovieSaveStatusReader() : _saveStatus(eMovieFileErr_NoErr),
    _saveState(eMovieFileState_Uninitialized), _delayLeft(0), _captureTime(0)
  { }

  virtual ~rcMovieSaveStatusReader()
  { }

  rcMovieFileErr getStatus() const
  { rcMovieFileErr x; return _saveStatus.getValue(x); }

  rcMovieFileState getState() const
  { rcMovieFileState x; return _saveState.getValue(x); }

  uint32 getDelayLeft() const
  { uint32 x; return _delayLeft.getValue(x); }

  uint32 getCaptureTime() const
  { uint32 x; return _captureTime.getValue(x); }

 protected:
  rcAtomicValue<rcMovieFileErr>   _saveStatus;
  rcAtomicValue<rcMovieFileState> _saveState;
  rcAtomicValue<uint32>         _delayLeft;
  rcAtomicValue<uint32>         _captureTime;
};

class rcMovieSaveStatus : public rcMovieSaveStatusReader
{
 public:
  rcMovieSaveStatus() : rcMovieSaveStatusReader()
  { }

  void status(rcMovieFileErr saveStatus)
  { _saveStatus.setValue(saveStatus); }

  void state(rcMovieFileState saveState)
  { _saveState.setValue(saveState); }

  void delayLeft(uint32 delayLeft)
  { _delayLeft.setValue(delayLeft); }

  void captureTime(uint32 captureTime)
  { _captureTime.setValue(captureTime); }
};

#endif
