// Copyright (c) 2002-2004 Reify Corp. All Rights reserved.

#ifndef _rcCAPTUREREGS_H_
#define _rcCAPTUREREGS_H_

// util
#include <rc_types.h>
#include <rc_atomic.h>

// visual
#include <rc_rect.h>
#include <rc_framebuf.h>

// analysis
#include <rc_analyzer.h>

/* rcVideoCaptureCtrl and rcVideoCaptureCtrlReader provide the
 * register set required to allow control of video capture.
 *
 * The video capture client should instantiate an rcVideoCaptureCtrl
 * class object and make a reference to the rcVideoCaptureCtrlReader
 * base class available to the video capture object.
 */
class rcVideoCaptureCtrlReader
{
 public:
  rcVideoCaptureCtrlReader() : _decRate(1), _slidingWindowSize(3),
    _slidingWindowOrigin(eAnalyzerResultOriginCenter),
        _slidingWindowEnabled(true), _ACIEnabled(false), _entropyDefinition(rcSimilarator::eACI),
			       _gain(0), _shutter(0), _binning (1)
  { }

  virtual ~rcVideoCaptureCtrlReader()
  { }

  uint32 getDecimationRate(void) const
  { uint32 x; return _decRate.getValue(x); }

  int getSlidingWindowSize() const
  { int x; return _slidingWindowSize.getValue(x); }

  rcAnalyzerResultOrigin getSlidingWindowOrigin() const
  { rcAnalyzerResultOrigin x; return _slidingWindowOrigin.getValue(x); }

  bool getSlidingWindowEnabled() const
  { bool x; return _slidingWindowEnabled.getValue(x); }

  bool getACIEnabled() const
  { bool x; return _ACIEnabled.getValue(x); }

  rcSimilarator::rcEntropyDefinition getEntropyDefinition() const
  { rcSimilarator::rcEntropyDefinition x;  return _entropyDefinition.getValue(x); }

  int32 getGain(void) const
  { int32 x; return _gain.getValue(x); }

  int32 getShutter(void) const
  { int32 x; return _shutter.getValue(x); }

  int32 getBinning(void) const
  { int32 x; return _binning.getValue(x); }

     
 protected:
  rcAtomicValue<uint32>                _decRate;
  rcAtomicValue<int>                     _slidingWindowSize;
  rcAtomicValue<rcAnalyzerResultOrigin>  _slidingWindowOrigin;
  rcAtomicValue<bool>                    _slidingWindowEnabled;
  rcAtomicValue<bool>                    _ACIEnabled;
  rcAtomicValue<rcSimilarator::rcEntropyDefinition> _entropyDefinition;
  rcAtomicValue<int32>                _gain;
  rcAtomicValue<int32>                _shutter;
  rcAtomicValue<int32>                _binning;
};

class rcVideoCaptureCtrl : public rcVideoCaptureCtrlReader
{
 public:
  rcVideoCaptureCtrl() : rcVideoCaptureCtrlReader()
  { }

  void decimationRate(uint32 decimationRate)
  { _decRate.setValue(decimationRate); }

  void slidingWindowSize(int slidingWindowSize)
  { if ( slidingWindowSize < 1 ) slidingWindowSize = 1;
      _slidingWindowSize.setValue(slidingWindowSize); }

  void slidingWindowOrigin(rcAnalyzerResultOrigin slidingWindowOrigin)
  { _slidingWindowOrigin.setValue(slidingWindowOrigin); }

  void slidingWindowEnabled(bool slidingWindowEnabled)
  { _slidingWindowEnabled.setValue(slidingWindowEnabled); }

  void ACIEnabled(bool ACIEnabled)
  { _ACIEnabled.setValue(ACIEnabled); }

  void entropyDefinition( rcSimilarator::rcEntropyDefinition eDef )
  {  _entropyDefinition.setValue(eDef); }

  void gain(int32 g)
  { _gain.setValue(g); }

  void shutter(int32 g)
  { _shutter.setValue(g); }

 void binning(int32 g)
  { _binning.setValue(g); }
};

/* rcVideoCaptureStatus and rcVideoCaptureStatusReader provide the
 * register set required to allow monitoring of the current state
 * of the video capture object.
 *
 * The video capture object should instantiate a rcVideoCaptureStatus
 * class object and make a reference to the rcVideoCaptureStatusReader
 * base class available to the video capture client.
 */
class rcVideoCaptureStatusReader
{
 public:
    rcVideoCaptureStatusReader() : _captureRect(rcRect(0, 0, 640, 480)), _captureDepth(rcPixelUnknown), _maxFramesPerSecond(0.0)
  { }

  virtual ~rcVideoCaptureStatusReader()
  { }

  rcRect getCaptureRect() const
  { rcRect x; return _captureRect.getValue(x); }
  rcPixel getCaptureDepth() const
  { rcPixel x; return _captureDepth.getValue(x); }
  double getMaxFramesPerSecond() const
  { double x; return  _maxFramesPerSecond.getValue(x); }
  
 protected:
  rcAtomicValue<rcRect> _captureRect;
  rcAtomicValue<rcPixel> _captureDepth;
  rcAtomicValue<double> _maxFramesPerSecond;
};

class rcVideoCaptureStatus : public rcVideoCaptureStatusReader
{
 public:
  rcVideoCaptureStatus() : rcVideoCaptureStatusReader()
  { }

  void captureRect(const rcRect& captureRect)
  { _captureRect.setValue(captureRect); }
  void captureDepth(const rcPixel& captureDepth)
  { _captureDepth.setValue(captureDepth); }
  void maxFramesPerSecond( const double& f )
  {  _maxFramesPerSecond.setValue(f); }
};

// Display utilities

ostream& operator << ( ostream& os, const rcVideoCaptureCtrl& c );

#endif
