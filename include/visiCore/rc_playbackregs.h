// Copyright (c) 2003 Reify Corp. All Rights reserved.

#ifndef _rcPLAYBACKREGS_H_
#define _rcPLAYBACKREGS_H_

// util
#include <rc_types.h>
#include <rc_atomic.h>

enum rcMoviePlaybackState {
    eMoviePlaybackState_Uninitialized = 0
  , eMoviePlaybackState_Initialized
  , eMoviePlaybackState_Playback
  , eMoviePlaybackState_Exit
};

/* rcMoviePlaybackCtrl and rcMoviePlaybackCtrlReader provide the
 * register set required to allow control of video playback.
 *
 * The movie playback client should instantiate an rcMoviePlaybackCtrl
 * class object and make a reference to the rcMoviePlaybackCtrlReader
 * base class available to the video playback object.
 */
class rcMoviePlaybackCtrlReader
{
 public:
  rcMoviePlaybackCtrlReader() : _ctrlInfo(0)
  { }

  virtual ~rcMoviePlaybackCtrlReader()
  { }

  int getCtrlInfo() const
  { int x; return _ctrlInfo.getValue(x); }

 protected:
  rcAtomicValue<int> _ctrlInfo;
};

class rcMoviePlaybackCtrl : public rcMoviePlaybackCtrlReader
{
 public:
  rcMoviePlaybackCtrl() : rcMoviePlaybackCtrlReader()
  { }

  void ctrlInfo(int ctrlInfo)
  { _ctrlInfo.setValue(ctrlInfo); }
};

/* rcMoviePlaybackStatus and rcMoviePlaybackStatusReader provide the
 * register set required to allow monitoring of the current state
 * of the video playback object.
 *
 * The video playback object should instantiate a rcMoviePlaybackStatus
 * class object and make a reference to the rcMoviePlaybackStatusReader
 * base class available to the video playback client.
 */
class rcMoviePlaybackStatusReader
{
 public:
  rcMoviePlaybackStatusReader()
    : _playbackState(eMoviePlaybackState_Uninitialized)
  { }

  virtual ~rcMoviePlaybackStatusReader()
  { }

  rcMoviePlaybackState getState() const
  { rcMoviePlaybackState x; return _playbackState.getValue(x); }

 protected:
  rcAtomicValue<rcMoviePlaybackState> _playbackState;
};


class rcMoviePlaybackStatus : public rcMoviePlaybackStatusReader
{
 public:
  rcMoviePlaybackStatus() : rcMoviePlaybackStatusReader()
  { }

  void state(rcMoviePlaybackState playbackState)
  { _playbackState.setValue(playbackState); }
};

#endif
