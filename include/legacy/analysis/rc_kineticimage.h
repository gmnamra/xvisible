/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_kineticimage.h
 *   Creation Date  09/19/2003
 *   Author         Peter Roberts
 *
 * rcOptoKineticImage is a generator of images describing intensity
 * changes within a scene through a series of frames.
 *
 ***************************************************************************/

#ifndef _RC_KINETICIMAGE_H_
#define _RC_KINETICIMAGE_H_

#include <rc_types.h>
#include <rc_window.h>

extern float segPoint(const rcWindow& measured, float minBinVal,
		      float binSz, int32 binCnt, bool getPeak);

extern void genSynthSignal(uint32 period, uint32 pulseWidth,
			   uint8 fg, uint8 bg, uint8 smooth, 
			   vector<uint8>& signal);

extern void genSpatialSDImg(const rcWindow& src, rcWindow& dest, int32 radius);

extern double genSpatialSDSum(const rcWindow& src, int32 radius);

extern void genLogRMS(rcWindow freqImg);

extern float segPointByEnergy(const rcWindow& measured, float minVal,
			      float maxVal, float binSz, float threshPct,
			      vector<rcWindow>& movie);

extern int32 calcExtendedMuscleCellFrame(const deque<double>& signal,
					   const vector<rcWindow>& movie,
					   int32 movieOffset);

extern float calcMusclePeriod(const vector<rcWindow>& movie,
			      uint32 startFrame, uint32 endFrame,
			      uint32 mdlIndex, float maxFramesPerPeriod);

class rcOptoKineticImage
{
 public:

  enum kineType {
    eKineTypeDummy = 0,
    eKineTypeVariance,
    eKineTypeStdDev,
    eKineTypeVelEntropy,
    eKineTypeAccelEntropy,
    eKineTypeThetaEntropy,
    eKineTypeLogRMS,
    eKineTypeInvalid
  };

  rcOptoKineticImage(kineType type);
  
  /* push - Add images to queue of frames to process. Note that all
   * image must have the same depth and dimensions.
   */
  void push(rcWindow img);
  void push(vector<rcWindow>& sequence);
  void push(deque<rcWindow>& sequence);
  
  /* pop - Pop cnt elements from front of movie. Returns remaining
   * number of elements.
   */
  uint32 pop(uint32 cnt = 1);
  
  void update(rcWindow img)
  { pop(); push(img); }

  void update(vector<rcWindow>& sequence)
  { pop(sequence.size()); push(sequence); }

  void update(deque<rcWindow>& sequence)
  { pop(sequence.size()); push(sequence); }
  
  /* genKineticImg - Generate a kinetic image, composed of floats,
   * based on the algorithm specified in the ctor. Note that at least
   * 2 images must be available.
   */
  void genKineticImg(rcWindow kineticImg);

  void genKineticStats(const rcWindow& kineticImg, float& minVal,
		       float& maxVal, float& mean, float& stdDev);

  void genDFT(rcWindow freqImg, rcWindow phaseImg, rcWindow amplitudeImg);

  void genEightBitDFT(rcWindow freqImg, rcWindow freqEBImg,
		      rcWindow phaseImg, rcWindow phaseEBImg,
		      rcWindow amplitudeImg, rcWindow amplitudeEBImg,
		      uint32 expectedFreq);

  void genFreqImg(rcWindow freqImg, float fpp, const rcWindow* maskImg = 0);

  void gen1DCorrKineticImgs(uint32 period, uint32 pulseWidth,
			    rcWindow ampImg, rcWindow phaseImg,
			    rcWindow corrPeaks, rcWindow phasePeaks);

  void genEightBitFreq(rcWindow freqImg, rcWindow freqEBImg,
		       const float expectedFreq);

  /* genEightBitImg - Convert kinetic image, composed of floats, into
   * an unsigned eight bit image. In the 1st case, the destination
   * pixels will be centered around the kinetic image's mean, with
   * sdClamp specifying the number of standard deviations to map out
   * to the min (pixel value 0) and max (pixel value 255) values. If
   * sdClamp is < 0.0, then the kinetic image's entire dynamic range
   * is used. Note that, in this case, the kinetic image must contain
   * at least 2 pixels.
   *
   * In the 2nd case, the caller specifies an arbitrary mapping. The
   * values in map should be in strictly ascending order. For each
   * kinetic pixel value P, (map[N] <= P < map[N+1]) ==> P maps to N+1
   * (as you would expect, (P < map[0]) ==> P maps to 0 and (P >=
   * map[LAST]) ==> P maps to LAST + 1). Up to the 1st 255 elements of
   * the map are used.
   */
  void genEightBitImg(const rcWindow& kineticImg, rcWindow ebImg,
		      float sdClamp, const vector<uint8>* map);
  void genEightBitImg(const rcWindow& kineticImg, rcWindow ebImg, 
		      vector<float>& map);
  void genEightBitImg(const rcWindow& kineticImg, rcWindow& ebImg);

  /* For debugging eKineTypeVelEntropy and eKineTypeAccelEntropy
   * cases.
   */
  uint32 underflow() const { return _uCnt; }
  uint32 overflow() const { return _oCnt; }

 private:

  typedef void (rcOptoKineticImage::*IMGHDLR)(const rcWindow& img);
  typedef void (rcOptoKineticImage::*GENHDLR)(rcWindow kineticImg);

  void genVarImg(rcWindow stdDevImg);
  void genStdDevImg(rcWindow stdDevImg);
  void genVelEntropyImg(rcWindow entropyImg);
  void genAccelEntropyImg(rcWindow entropyImg);
  void genThetaEntropyImg(rcWindow entropyImg);
  void genLogRMS (rcWindow);
  void genDummyImg(rcWindow)
  { }

  void decrementU8(const rcWindow& img);
  void incrementU8(const rcWindow& img);

  void decVarImg(const rcWindow& img)
  { decrementU8(img); }
  void incVarImg(const rcWindow& img)
  { incrementU8(img); }
  void decStdDevImg(const rcWindow& img)
  { decrementU8(img); }
  void incStdDevImg(const rcWindow& img)
  { incrementU8(img); }
  void decVelEntropyImg(const rcWindow& img)
  { decrementU8(img);}
  void incVelEntropyImg(const rcWindow& img)
  { incrementU8(img);}
  void decLogRMSImg(const rcWindow& img)
  { decrementU8(img); }
  void incLogRMSImg(const rcWindow& img)
  { incrementU8(img); }

  void decAccelEntropyImg(const rcWindow&)
  { }
  void incAccelEntropyImg(const rcWindow&)
  { }
  void decThetaEntropyImg(const rcWindow&)
  { }
  void incThetaEntropyImg(const rcWindow&)
  { }

  void decDummyImg(const rcWindow&)
  { }
  void incDummyImg(const rcWindow&)
  { }

  void initTheta();

  kineType         _kType;
  IMGHDLR          _incHdlr, _decHdlr;
  GENHDLR          _genHdlr;
  deque<rcWindow>  _movie;
  rcWindow         _sum, _sumSq;
  uint32         _uCnt, _oCnt;
  vector<float>    _theta;
};

#endif
