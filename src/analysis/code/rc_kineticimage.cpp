 /****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_kineticimage.cpp
 *   Creation Date  09/23/2003
 *   Author         Peter Roberts
 *
 * rcOptoKineticImage is a generator of images describing intensity
 * changes within a scene through a series of frames.
 *
 ***************************************************************************/

#include <rc_kineticimage.h>
#include <math.h>
#include <rc_analysis.h>
#include <rc_similarity.h>
#include <rc_moments.h>
#include <rc_1dcorr.h>
#include <rc_stats.h>
#include <rc_lsfit.h>
#include <rc_filter1d.h>
#include <rc_ip.h>

#define rmPrintImage(a){					    \
  for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {    \
    fprintf (stderr, "\n");					    \
    for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)	    \
      fprintf (stderr, "%3d ", (a).getPixel (j__temp, i__temp));    \
    fprintf (stderr, "\n");					    \
  }}

static inline double shannon (double r) { return (-1.0 * r * log2 (r)); }

float segPoint(const rcWindow& measured, float minBinVal, float binSz,
	       int32 binCnt, bool getPeak) 
{
  rmAssert(binSz > 0);
  rmAssert(binCnt > 0);
  rmAssert(rcPixel32 == measured.depth());

  const int32 width = measured.width(), height = measured.height();
  
  /* Initialize histogram.
   */
  vector<int32> hist(binCnt);
  for (int32 i = 0; i < binCnt; i++)
    hist[i] = 0;
  int32 overflow = 0;

  /* Fill histogram.
   */
  const float cutoff = minBinVal - binSz + 0.0000001;
  const float binSzInv = 1/binSz;
  const double dBinCnt = binCnt;

  if (0) printf("cutoff %f binSzInv %f dBinCnt %f\n", cutoff, binSzInv, dBinCnt);

  for (int32 y = 0; y < height; y++) {
    float* valP = (float*)measured.rowPointer(y);
    
    for (int32 x = 0; x < width; x++) {
      int32 index = 0;
      float val = *valP++;
      if (val > cutoff) {
	double dIndex = (val - cutoff)*binSzInv; // Double avoids overflows
	if (dIndex >= dBinCnt) {
	  overflow++;
	  continue;
	}
	index = (int32)dIndex;
      }
      rmAssert(index >= 0 && index < binCnt);
      hist[index]++;
    }
  }

  /* Convert to cumulative histogram.
   */
  for (int32 si = 1; si < binCnt; si++)
    hist[si] += hist[si-1];

  /* Create the correlation matrix.
   */
  rmAssert(width*height == hist[binCnt-1]+overflow);

  double** corrMatrix;

  corrMatrix = (double**)malloc(binCnt*sizeof(double*));
  rmAssert(corrMatrix);
  
  double* matrixBase = (double*)malloc(binCnt*binCnt*sizeof(double));
  rmAssert(matrixBase);
  for (int32 i = 0; i < binCnt; i++)
    corrMatrix[i] = matrixBase + (i*binCnt);

  /* Fill in correlation matrix.
   */
  for (int32 i = 0; i < binCnt; i++)
    corrMatrix[i][i] = 1.0;

  const double N = width*height;
  for (int32 m = 0; m < binCnt; m++) {
    const double mCnt = N - hist[m];
    const double em = N*mCnt - mCnt*mCnt;
    if (em == 0) {
      for (int32 i = m+1; i < binCnt; i++)
	corrMatrix[i][m] = corrMatrix[m][i] = 1.0;
    }
    else {
      for (int32 i = m+1; i < binCnt; i++) {
	const double iCnt = N - hist[i];
	const double eiXem = (N - iCnt)*iCnt*em;
	if (eiXem == 0) {
	  corrMatrix[i][m] = corrMatrix[m][i] = 1.0;
	  continue;
	}

	rmAssert(mCnt >= iCnt);
	const double cross = (N - mCnt)*iCnt;
	const double corr = corrMatrix[m][i] = cross*cross/eiXem;
	corrMatrix[i][m] = (corr < 1.0) ? corr : 1.0;
      }
    }
  }
  
  /* Calculate entropy signal.
   */
  vector<double> sums(binCnt);
  vector<double> entropies(binCnt);

  for (int32 i = 0; i < binCnt; i++) {
    sums[i] = corrMatrix[i][i];
    entropies[i] = 0.0;
  }

  for (int32 i = 0; i < (binCnt-1); i++)
    for (int32 j = (i+1); j < binCnt; j++) {
      sums[i] += corrMatrix[i][j];
      sums[j] += corrMatrix[i][j];
    }

  const double log2N = log2(N);

  for (int32  i = 0; i < binCnt; i++) {
    for (int32 j = i; j < binCnt; j++) {
      double rr =
	corrMatrix[i][j]/sums[i]; // Normalize for total energy in samples
      entropies[i] += shannon(rr);
      
      if (i != j) {
	rr = corrMatrix[i][j]/sums[j]; //Normalize for total energy in samples
	entropies[j] += shannon(rr);
      }
    }
    entropies[i] = entropies[i]/log2N;// Normalize for count of samples
  }

  /* Done with correlation matrix - free it.
   */
  free(matrixBase);
  free(corrMatrix);

  if (0) {
    for (int32 i = 0; i < binCnt; i++)
      printf("%d: %f\n", i, entropies[i]);
  }

  /* Find segmentation point. If getPeak is true, just return the point
   * corresponding to the maximum entropy location.
   */
  if (getPeak) {
    int32 maxBin = 0;
    double maxVal = entropies[0];

    for (int32 bin = 1; bin < binCnt; bin++) {
      if (entropies[bin] > maxVal) {
	maxBin = bin;
	maxVal = entropies[bin];
      }
    }
    
    return minBinVal + maxBin*binSz;
  }

  /* Find segmentation point by breaking the source image into two
   * regions: inside and outside. In our case, this will typically
   * mean a cell, but that's not required. The two requirements are:
   *
   *   1) the measured quality is greater on the inside than the outside.
   *
   *   2) the measured quality becomes relatively stable within the inner
   *      region (at least up to the values represented in the last bin
   *      of the cumulative histogram).
   *
   * Given that these are true, then the entropy signal should take on
   * the following characteristics. At the high end (higher indices),
   * there should be a relatively flat parabola. This is the region of
   * values that indicate inside pixels. Past this there will be a
   * relatively quick drop off. Possibly some additional rises and
   * fall will occur as you move towards the low end, but these will
   * be ignored. All these values are considered to be outside pixels.
   *
   * The following finds the location of the first bin at the lower
   * end of the parabola that has a higher entropy signal value than
   * that at the high end. This bin index is then used to calculate
   * the segementation point.
   */
  double segCutOff = entropies[binCnt-1];

  int32 curBin = -1;
  double minVal = 1e99;

  for (int32 bin = 0; bin < binCnt; bin++) {
    if (entropies[bin] < minVal) {
      curBin = bin;
      minVal = entropies[bin];
    }
  }
  rmAssert(curBin >= 0);

  for ( ; curBin < binCnt; curBin++) {
    if (entropies[curBin] > segCutOff)
      break;
  }

  /* Low end has been found. Convert bin index to its corresponding
   * measurement value.
   */
  return minBinVal + curBin*binSz;
}

static void gaussRing(vector<uint8>& src, vector<uint8>& dest)
{
  const uint32 sz = src.size();
  rmAssert(sz == dest.size());

  if (sz >= 3) {
    uint32 left = src[sz-2], center = src[sz-1], right = src[0];
    uint32 sum = (left + center + center + right + 2) >> 2;
    
    dest[sz-1] = (uint8)sum;
    
    for (uint32 i = 0; i < sz-1; i++) {
      left = center;
      center = right;
      right = src[i+1];
      sum = (left + center + center + right + 2) >> 2;
      dest[i] = (uint8)sum;
    }
  }
  else {
    for (uint32 i = 0; i < sz; i++)
      dest[i] = src[i];
  }
}

void genSynthSignal(uint32 period, uint32 pulseWidth,
		    uint8 fg, uint8 bg, uint8 smooth, 
		    vector<uint8>& signal)
{
  rmAssert(pulseWidth <= period);

  signal.resize(period);
  if (!period)
    return;

  vector<uint8> signal2(period);

  for (uint32 i = 0; i < pulseWidth; i++)
    signal[i] = fg;

  for (uint32 i = pulseWidth; i < period; i++)
    signal[i] = bg;
  
  for (uint32 i = 0; i < smooth; i++) {
    if (i & 1)
      gaussRing(signal2, signal);
    else
      gaussRing(signal, signal2);
  }

  if (smooth & 1) {
    for (uint32 i = 0; i < period; i++)
      signal[i] = signal2[i];
  }
}

void genSpatialSDImg(const rcWindow& src, rcWindow& dest, int32 radius)
{
  rmAssert(radius >= 0);
  rmAssert(src.depth() == rcPixel8);
  rmAssert(dest.depth() == rcPixel32);
  rmAssert(src.width() == dest.width() + 2*(radius+1));
  rmAssert(src.height() == dest.height() + 2*(radius+1));
#if 0
  rcAutoCorrelation ac;
  rsCorr3by3Line result;

  rcWindow smooth (src.width(), src.height());
  rfGaussianConv (src, smooth, uint32 (5));
  
  ac.update(smooth, 1, rcAutoCorrelation::eFull, rcAutoCorrelation::eLine);
  rcIPair searchSize((radius+1)*2 + 1, (radius+1)*2 + 1);

  const int32 height = dest.height();
  const int32 width = dest.width();

  // @note use vsqrtd () in Accelerate.framework/Frameworks/vecLib.framework/Headers/vfp.h

  double szNorm = searchSize.x()*(searchSize.x() - 1);
  for (int32 y = 0; y < height; y++) {
    ac.gen3by3Line(searchSize, y, result);

    rmAssert((int32)result.count == width);

    float* destP = (float*)dest.rowPointer(y);

    for (uint32 x = 0; x < result.count; x++)
      destP[x] = (float)sqrt(*(result.score[1][1] + x)/szNorm);
  }
#endif	
}

double genSpatialSDSum(const rcWindow& src, int32 radius)
{
  rmAssert(radius >= 0);
  rmAssert(src.depth() == rcPixel8);
  rcAutoCorrelation ac;
  rsCorr3by3Line result;
#if 0
  ac.update(src, 1, rcAutoCorrelation::eFull, rcAutoCorrelation::eLine);
  rcIPair searchSize((radius+1)*2 + 1, (radius+1)*2 + 1);

  const int32 height = src.height() - 2*(radius+1);
  const int32 width = src.width() - 2*(radius+1);

  double retVal = 0.0;
  for (int32 y = 0; y < height; y++) {
    ac.gen3by3Line(searchSize, y, result);

    rmAssert((int32)result.count == width);

    for (uint32 x = 0; x < result.count; x++)
      retVal += *(result.score[1][1] + x);
  }

  return retVal;
#endif	
}

static void binarize(const rcWindow& src, const rcWindow& dest, float limit,
		     const uint8 fg, const uint8 bg)
{
  int32 height = src.height(), width = src.width();
  rmAssert(height == dest.height());
  rmAssert(width == dest.width());
  rmAssert(src.depth() == rcPixel32);
  rmAssert(dest.depth() == rcPixel8);

  for (int32 y = 0; y < height; y++) {
    const float* srcRow = (const float*)src.rowPointer(y);
    uint8* destRow = (uint8*)dest.rowPointer(y);
    for (int32 x = 0; x < width; x++) {
      const float val = *srcRow++;
      if (val > limit)
	*destRow++ = fg;
      else
	*destRow++ = bg;
    }
  }
}

extern void rfPixel8Max3By3(const rcWindow& srcImg, rcWindow& dstImg);
extern void rfPixel8Min3By3(const rcWindow& srcImg, rcWindow& dstImg);

#define SPBE_PRT1 0
#define SPBE_PRT2 0

float segPointByEnergy(const rcWindow& measured, float minVal, float maxVal,
		       float binSz, float threshPct, vector<rcWindow>& movie)
{
  rmAssert(measured.depth() == rcPixel32);
  rmAssert(maxVal > minVal);
  rmAssert(binSz > 0.0);
  rmAssert(threshPct >= 0.0);
  rmAssert(threshPct <= 1.0);
  uint32 mLth = movie.size();
  rmAssert(mLth > 4);

  double baseDelta;

  {
    rcSimilarator energy(rcSimilarator::eExhaustive,
			 rcPixel8, 5, 0);

    vector<rcWindow> eMovie;
    eMovie.push_back(movie[0]);
    eMovie.push_back(movie[1]);
    eMovie.push_back(movie[2]);
    eMovie.push_back(movie[3]);
    eMovie.push_back(movie[4]);

    energy.fill(eMovie);
    deque<double> tempSig;
    bool rval = energy.entropies(tempSig, rcSimilarator::eACI);
    rmAssert(rval);

    double minSig = tempSig[2], maxSig = tempSig[2];

    for (uint32 i = 5; i < mLth; i++) {
      rval = energy.update(movie[i]);
      rmAssert(rval);
      
      energy.entropies(tempSig, rcSimilarator::eACI);
      if (tempSig[1] < minSig)
	minSig = tempSig[2];
      if (tempSig[1] > maxSig)
	maxSig = tempSig[2];
    }

    baseDelta = maxSig - minSig;
  }

  rcWindow mask(measured.width(), measured.height());

  if (SPBE_PRT1) printf("Init State: min %f max %f m sz %d bin sz %f"
			" base delta %f\n",
			minVal, maxVal, mLth, binSz, baseDelta);

  float curVal = minVal;
  float segVal = minVal, segDelta = 1e38;
 
  for (;;) {

    binarize(measured, mask, curVal, 0x00, 0xFF);
    rcWindow temp(mask.width()-2, mask.height()-2);
    rcWindow mWin(mask, 1, 1,
		  mask.width()-2, mask.height()-2);
    rfPixel8Min3By3(mask, temp);
    mWin.copyPixelsFromWindow(temp);
    rfPixel8Max3By3(mask, temp);
    mWin.copyPixelsFromWindow(temp);

    rcSimilarator energy(rcSimilarator::eExhaustive,
			 rcPixel8, 5, 0);
    energy.setMask(mask);

    vector<rcWindow> eMovie;
    eMovie.push_back(movie[0]);
    eMovie.push_back(movie[1]);
    eMovie.push_back(movie[2]);
    eMovie.push_back(movie[3]);
    eMovie.push_back(movie[4]);

    energy.fill(eMovie);
    deque<double> tempSig;
    bool rval = energy.entropies(tempSig, rcSimilarator::eACI);
    rmAssert(rval);

    double minSig = tempSig[2], maxSig = tempSig[2];

    for (uint32 i = 5; i < mLth; i++) {
      rval = energy.update(movie[i]);
      rmAssert(rval);
      
      energy.entropies(tempSig, rcSimilarator::eACI);
      if (tempSig[1] < minSig)
	minSig = tempSig[2];
      if (tempSig[1] > maxSig)
	maxSig = tempSig[2];
    }

    double curDelta = maxSig - minSig;

    if (curDelta < segDelta) {
     segDelta = curDelta;
     segVal = curVal;
    }

    if (SPBE_PRT2)
      printf("curVal %f curDelta %f segVal %f segDelta %f\n",
	     curVal, curDelta, segVal, segDelta);

    if (curVal >= maxVal) {
      if (SPBE_PRT1) printf("Returning %f\n", segVal);
      return segVal;
    }

    curVal += 0.1;
  }

  rmAssert(0);
  return 1e38;
}

#define CEMCF_PRT1 0
#define CEMCF_PRT2 0

int32 calcExtendedMuscleCellFrame(const deque<double>& signal,
				    const vector<rcWindow>& movie,
				    int32 movieOffset)
{
  if (CEMCF_PRT1)
    printf("cEMCF: signal sz %d offset %d\n", (int)signal.size(), movieOffset);

  rmAssert(signal.size());
  rmAssert(movieOffset >= 0);
  rmAssert(((int32)movie.size() - movieOffset) >= (int32)signal.size());

  rcStatistics stats;

  for (uint32 ii = 0; ii < signal.size(); ii++)
    stats.add(signal[ii]);

  double hiThreshSD = stats.mean() + 2*stats.stdDev();
  double hiThreshMM =
    stats.max() - (stats.max() - stats.min())*0.05;
  double loThreshSD = stats.mean() - 2*stats.stdDev();
  double loThreshMM =
    stats.min() + (stats.max() - stats.min())*0.05;
  double hiThresh =
    hiThreshSD < hiThreshMM ? hiThreshSD : hiThreshMM;
  double loThresh =
    loThreshSD > loThreshMM ? loThreshSD : loThreshMM;
		    
  if (CEMCF_PRT1) printf("cEMCF: hi %f lo %f mean %f sd %f min %f max %f\n",
			 hiThresh, loThresh, stats.mean(),
			 stats.stdDev(),  stats.min(), stats.max());
  
  deque<double> tempSig(signal.size());

  double peakVal = -1.0;
  int32 peakFrameIndex = -1;
  int32 curRunLth = 0, curRunIndex = -1;
  int32 peakRunLth = 0, peakRunIndex = -1;

  if (CEMCF_PRT2) printf("cEMCF: input signal:\n");

  for (uint32 ii = 0; ii < signal.size(); ii++) {
    uint32 val;
    if (signal[ii] > hiThresh) {
      val = 2;
      if (curRunIndex == -1) {
	curRunIndex = ii;
	curRunLth = 0;
      }
      curRunLth++;
    }
    else {
      val = (signal[ii] < loThresh) ? 0 : 1;
      if (curRunIndex != -1) {
	if (peakRunLth < curRunLth) {
	  peakRunLth = curRunLth;
	  peakRunIndex = curRunIndex;
	}
	curRunIndex = -1;
      }
    }

    if (CEMCF_PRT2) printf("  %03d: %d  %f\n", ii, val, signal[ii]);
    tempSig[ii] = val;
    if (signal[ii] > peakVal) {
      peakVal = signal[ii];
      peakFrameIndex = ii;
    }
  }

  if ((curRunIndex != -1) && (peakRunLth < curRunLth)) {
    peakRunLth = curRunLth;
    peakRunIndex = curRunIndex;
  }

  if (CEMCF_PRT2) printf("\n");

  /* Search for an extended muscle cell. The following strategy will
   * be used:
   *
   * 1 - See if any run of 3 or more frames was found that all exceed
   *     the high threshold. If so, pick the peak value from this set.
   *
   * 2 - If 1) fails the assumption will be that we are in the case
   *     where the extended resting phase never lasts more than 1
   *     frame. In this case, pick two consecutive peaks under the
   *     assumption that 1 will be a fully extended frame and the
   *     other is a fully contracted frame. The one that has the
   *     higher amount of edge information will be picked as the
   *     extended frame.
   */

  if (peakRunLth > 2) {
    peakVal = -1.0;
    peakFrameIndex = -1;
    for (int32 ii = peakRunIndex; ii < peakRunIndex + peakRunLth; ii++)
      if (signal[ii] > peakVal) {
	peakVal = signal[ii];
	peakFrameIndex = ii;
      }
    if (CEMCF_PRT1)
      printf("cEMCF: peakRunIndex %d peakRunLth (%d) > 2, peak @ %d\n", 
	     peakRunIndex, peakRunLth, peakFrameIndex);
  }
  else {
    if (CEMCF_PRT1)
      printf("cEMCF: peakRunLth (%d) < 3\n", peakRunLth);

    /* Search both forward and backward for a second peak. Use the
     * second peak that is closest. This is to try and limit the cases
     * where the second peak is actually from a different pulse of the
     * cell.
     *
     * Note: During search, the end of the current peak must be found
     * first. This is the purpose of foundEdge.
     */
    int32 peakFrameIndexFwd;
    bool foundEdge = false;
    for (peakFrameIndexFwd = peakFrameIndex + 1; 
	 peakFrameIndexFwd < (int32)tempSig.size();
	 peakFrameIndexFwd++) {
      if (tempSig[peakFrameIndexFwd] == 2) {
	if (foundEdge)
	  break;
      }
      else
	foundEdge = true;
    }

    int32 peakFrameIndexRev;
    foundEdge = false;
    for (peakFrameIndexRev = peakFrameIndex - 1; 
	 peakFrameIndexRev >= 0;
	 peakFrameIndexRev--) {
      if (tempSig[peakFrameIndexRev] == 2) {
	if (foundEdge)
	  break;
      }
      else
	foundEdge = true;
    }

    int32 peakFrameIndex2 = -1;
    if (peakFrameIndexFwd == (int32)tempSig.size())
      peakFrameIndex2 = peakFrameIndexRev; // Either valid or -1
    else if (peakFrameIndexRev == -1)
      peakFrameIndex2 = peakFrameIndexFwd;
    else if ((peakFrameIndex - peakFrameIndexRev) <
	     (peakFrameIndexFwd - peakFrameIndex))
      peakFrameIndex2 = peakFrameIndexRev;
    else
      peakFrameIndex2 = peakFrameIndexFwd;

    if (peakFrameIndex2 != -1) {
      double sum = genSpatialSDSum(movie[movieOffset+peakFrameIndex], 1);
      double sum2 = genSpatialSDSum(movie[movieOffset+peakFrameIndex2], 1);

      if (CEMCF_PRT1)
	printf("cEMCF: Dual peaks, values %f @ %d and %f @ %d", 
	       sum, peakFrameIndex, sum2, peakFrameIndex2);

      if (sum2 > sum)
	peakFrameIndex = peakFrameIndex2;

      if (CEMCF_PRT1) printf(", peak @ %d\n", peakFrameIndex);
    }
    else if (CEMCF_PRT1)
      printf("cEMCF: Single peak @ %d\n", peakFrameIndex);
  }

  rmAssert(peakFrameIndex > -1);
  return peakFrameIndex;
}

#define CMP_PRT1 0
#define CMP_PRT2 0

float calcMusclePeriod(const vector<rcWindow>& movie, uint32 startFrame,
		       uint32 endFrame, uint32 mdlIndex,
		       float maxFramesPerPeriod)
{
  rmAssert(endFrame < movie.size());
  rmAssert(mdlIndex >= startFrame && mdlIndex < endFrame);
  rmAssert(maxFramesPerPeriod > 0.0);

  /* Calculate the average number of frames per muscle contraction
   * using the following method:
   *
   * - Calculate a 1D correlation signal based on comparing the image
   *   at mdlIndex with all the others in the sampling interval.
   *
   * - Perform an FFT analysis of this signal.
   *
   * - Peak detect the resulting frequency space.
   *
   * - Determine the dominant frequency from the list of peaks. The
   *   following special rules will be applied:
   *
   *   - Discount any frequencies below a minimum value as determined
   *     by maxFramesPerPeriod.
   *
   *   - Try to detect the true frequency of narrow peaked input
   *     signals by looking for the resultant flat frequency space.
   */
  const uint32 sigSz = endFrame-startFrame;
  deque<double> sig(sigSz);
  rsCorrParams params;
  rcCorr res;
  if (CMP_PRT1) fprintf(stderr, "Input signal\n");

#if 0
  vector<rcWindow> fillers(3);
  rcSimilarator selfsim (rcSimilarator::eExhaustive,
			 rcPixel8, 3, 0);
  uint32 i; 
  for (i = startFrame; i < (startFrame+3); i++)
    {
      fillers[i - startFrame] = movie[i];
    }
  selfsim.fill (fillers);

  deque<double> tsig;
  bool rval = selfsim.entropies (tsig,rcSimilarator::eACI);
  sig[0] = tsig[1];

  for (uint32 j = 1; i < endFrame; i++, j++)
    {
      rval = selfsim.update (movie[i]);
      rmAssert (rval);
      selfsim.entropies(tsig, rcSimilarator::eACI); 
      sig[j] = tsig[1];
  }
#else

  uint32 curMdlIndex = mdlIndex, prevMdlIndex = mdlIndex;
  if (mdlIndex > startFrame) {
    uint32 i = mdlIndex;
    do {
      i--;
      rfCorrelate(movie[i], movie[curMdlIndex], params, res);
      sig[i-startFrame] = res.r();
      if (sig[i-startFrame] >= 0.95) {
	rfCorrelate(movie[i], movie[prevMdlIndex], params, res);
	if (res.r() >= 0.95) {
	  prevMdlIndex = curMdlIndex;
	  curMdlIndex = i;
	  if (CMP_PRT2)
	    fprintf(stderr, "Score %f (%f) New mdl index %d\n",
		    sig[i-startFrame], res.r(), curMdlIndex);
	}
      }
    } while ( i != startFrame);
  }

  curMdlIndex = prevMdlIndex = mdlIndex;
  for (uint32 i = mdlIndex+1; i < endFrame; i++) {
    rfCorrelate(movie[i], movie[curMdlIndex], params, res);
    sig[i-startFrame] = res.r();
    if (sig[i-startFrame] >= 0.95) {
	rfCorrelate(movie[i], movie[prevMdlIndex], params, res);
	if (res.r() >= 0.95) {
	  prevMdlIndex = curMdlIndex;
	  curMdlIndex = i;
	  if (CMP_PRT2)
	    fprintf(stderr, "Score %f (%f) New mdl index %d\n",
		    sig[i-startFrame], res.r(), curMdlIndex);
	}
    }
  }

  if (mdlIndex == startFrame) {
    if (sig[1] >= 0.95)
      sig[0] = sig[1];
    else
      sig[0] = 1.0;
  }
  else if (mdlIndex == (endFrame-1)) {
    if (sig[sigSz-2] >= 0.95)
      sig[sigSz-1] = sig[sigSz-2];
    else
      sig[sigSz-1] = 1.0;
  }
  else {
    double cand = sig[mdlIndex-1-startFrame];
    if (cand < sig[mdlIndex+1-startFrame])
      cand = sig[mdlIndex+1-startFrame];

    if (cand >= 0.95)
      sig[mdlIndex-startFrame] = cand;
    else
      sig[mdlIndex-startFrame] = 1.0;
  }

  if (CMP_PRT1) {
    for (uint32 i = 0; i < sigSz; i++)
      fprintf(stderr, "%d: %f\n", i, sig[i]);
  }
#endif

  rs1DFourierResult rslt;
  rf1DFourierAnalysis(sig, rslt, rc1DFourierForceFFT |rc1DFourierRaisedCosine);

  vector<uint32> peakLocs;
  vector<float> interLocs, interVals;
  rf1DPeakDetect(rslt.amplitude, peakLocs, interLocs,
		 interVals, 0.10f);

  float minFrequency = rslt.workingSz/maxFramesPerPeriod - 0.5;

  /* Find the the frequency, greater than minFrequency, with the
   * greatest amplitude.
   */
  uint32 maxFreqIndex = 0;
  float maxAmplitude = -1.0;
  for (uint32 ii = 0; ii < peakLocs.size(); ii++) {
    if ((interLocs[ii] > minFrequency) && (interVals[ii] > maxAmplitude)) {
      maxFreqIndex = ii;
      maxAmplitude = interVals[ii];
    }
    if (1)
      fprintf(stderr, " (%d %f) inter: (%f, %f)\n",
	      peakLocs[ii], rslt.amplitude[peakLocs[ii]],
	      interLocs[ii], interVals[ii]);
  }
  
  if (1)
    fprintf(stderr, "maxAmplitude %f maxFreqIndex %d minFrequency %f\n",
	    maxAmplitude, maxFreqIndex, minFrequency);

  if (maxAmplitude == -1.0)
    return 0.0;

  /* Now check to see if the frequency space is "flat". My definition
   * will be frequencies within 10% of the peak amplitude
   */
  for (uint32 ii = 0; ii < maxFreqIndex; ii++) {
    if ((interLocs[ii] > minFrequency) &&
	(interVals[ii] > maxAmplitude*0.9)) {
      if (1)
	fprintf(stderr, "Flat peak detected - @ %d: loc %f amp %f\n",
		ii, interLocs[ii], interVals[ii]);

      return rslt.workingSz/interLocs[ii];
    }
  }

  if (1)
    fprintf(stderr, "Return max peak - @ %d: loc %f amp %f\n",
	    maxFreqIndex, interLocs[maxFreqIndex],
	    interVals[maxFreqIndex]);

  return rslt.workingSz/interLocs[maxFreqIndex];
}

rcOptoKineticImage::rcOptoKineticImage(kineType type) : _kType(type)
{
  switch (_kType)
  {
  case eKineTypeDummy:
    _incHdlr = &rcOptoKineticImage::incDummyImg;
    _decHdlr = &rcOptoKineticImage::decDummyImg;
    _genHdlr = &rcOptoKineticImage::genDummyImg;
    break;

  case eKineTypeVariance:
    _incHdlr = &rcOptoKineticImage::incVarImg;
    _decHdlr = &rcOptoKineticImage::decVarImg;
    _genHdlr = &rcOptoKineticImage::genVarImg;
    break;

  case eKineTypeStdDev:
    _incHdlr = &rcOptoKineticImage::incStdDevImg;
    _decHdlr = &rcOptoKineticImage::decStdDevImg;
    _genHdlr = &rcOptoKineticImage::genStdDevImg;
    break;

  case eKineTypeVelEntropy:
    _incHdlr = &rcOptoKineticImage::incVelEntropyImg;
    _decHdlr = &rcOptoKineticImage::decVelEntropyImg;
    _genHdlr = &rcOptoKineticImage::genVelEntropyImg;
    break;

  case eKineTypeAccelEntropy:
    _incHdlr = &rcOptoKineticImage::incAccelEntropyImg;
    _decHdlr = &rcOptoKineticImage::decAccelEntropyImg;
    _genHdlr = &rcOptoKineticImage::genAccelEntropyImg;
    break;

  case eKineTypeThetaEntropy:
    _incHdlr = &rcOptoKineticImage::incThetaEntropyImg;
    _decHdlr = &rcOptoKineticImage::decThetaEntropyImg;
    _genHdlr = &rcOptoKineticImage::genThetaEntropyImg;
    break;

  case eKineTypeLogRMS:
    _incHdlr = &rcOptoKineticImage::incLogRMSImg;
    _decHdlr = &rcOptoKineticImage::decLogRMSImg;
    _genHdlr = &rcOptoKineticImage::genLogRMS;
    break;

  default:
    rmAssert(0);
    break;
  }
}

void rcOptoKineticImage::push(rcWindow img)
{
  rmAssert(img.depth() == rcPixel8);
  if (!_movie.empty()) {
    rmAssert(img.width() == _movie[0].width());
    rmAssert(img.height() == _movie[0].height());
  }
  else
    rmAssert(img.width()*img.height() > 1);

  (this->*_incHdlr)(img);
  _movie.push_back(img);
}

void rcOptoKineticImage::push(vector<rcWindow>& sequence)
{
  for (uint32 i = 0; i < sequence.size(); i++) {
    rmAssert(sequence[i].depth() == rcPixel8);
    if (!_movie.empty()) {
      rmAssert(sequence[i].width() == _movie[0].width());
      rmAssert(sequence[i].height() == _movie[0].height());
    }
    else
      rmAssert(sequence[i].width()*sequence[i].height() > 1);
    (this->*_incHdlr)(sequence[i]);
    _movie.push_back(sequence[i]);
  }
}

void rcOptoKineticImage::push(deque<rcWindow>& sequence)
{
  for (uint32 i = 0; i < sequence.size(); i++) {
    rmAssert(sequence[i].depth() == rcPixel8);
    if (!_movie.empty()) {
      rmAssert(sequence[i].width() == _movie[0].width());
      rmAssert(sequence[i].height() == _movie[0].height());
    }
    else
      rmAssert(sequence[i].width()*sequence[i].height() > 1);
    (this->*_incHdlr)(sequence[i]);
    _movie.push_back(sequence[i]);
  }
}

uint32 rcOptoKineticImage::pop(uint32 cnt)
{

  uint32 popCnt = _movie.size();
  uint32 retVal = 0;

  if (popCnt > cnt) {
    retVal = popCnt - cnt;
    popCnt = cnt;
  }

  while (popCnt--) {
    (this->*_decHdlr)(_movie.front());
    _movie.pop_front();
  }

  return retVal;
}

void rcOptoKineticImage::genKineticStats(const rcWindow& kineticImg, 
					 float& minVal, float& maxVal,
					 float& mean, float& stdDev)
{
  rmAssert(kineticImg.depth() == rcPixel32);
  const int32 width = kineticImg.width(), height = kineticImg.height();
  const double N = width*height;
  rmAssert(N > 1);

  double sum = 0.0, sumSq = 0.0;
  minVal = rcFLT_MAX;
  maxVal = rcFLT_MIN;

  for (int32 y = 0; y < height; y++) {
    const float* srcRow = (const float*)kineticImg.rowPointer(y);
    for (int32 x = 0; x < width; x++) {
      const float val = *srcRow++;
      if (val < minVal)
	minVal = val;
      if (val > maxVal) 
	maxVal = val;
      sum += val;
      sumSq += ((double)val)*val;
    }
  }
  
  mean = (float)(sum/N);
  stdDev = (float)sqrt((sumSq*N - sum*sum)/(N*N - N));
  rmAssert(stdDev > 0.0);
}

void rcOptoKineticImage::genKineticImg(rcWindow kineticImg)
{
  (this->*_genHdlr)(kineticImg);
}

#if 0
static int xyzzy_cnt = 0;
#include <vbigdsp.h>
#endif

void rcOptoKineticImage::genDFT(rcWindow freqImg, rcWindow phaseImg,
				rcWindow amplitudeImg)
{
  const uint32 movieLth = _movie.size();
  rmAssert(movieLth);

  const int32 width = _movie[0].width(), height = _movie[0].height();
  rmAssert(width == freqImg.width());
  rmAssert(height == freqImg.height());
  rmAssert(rcPixel32 == freqImg.depth());
  rmAssert(width == phaseImg.width());
  rmAssert(height == phaseImg.height());
  rmAssert(rcPixel32 == phaseImg.depth());
  rmAssert(width == amplitudeImg.width());
  rmAssert(height == amplitudeImg.height());
  rmAssert(rcPixel32 == amplitudeImg.depth());

  vector<float> imag(movieLth);
  vector<float> real(movieLth);

  for (int32 y = 0; y < height; y++) {
    if (0) printf("Processing line %d\n", y);
    uint32* fp = (uint32*)freqImg.rowPointer(y);
    float* pp = (float*)phaseImg.rowPointer(y);
    float* ap = (float*)amplitudeImg.rowPointer(y);

    for (int32 x = 0; x < width; x++) {
      /* At each location, perform the following steps:
       *
       *   1) Store 1D signal in real and imag vectors.
       *
       *   2) Run dft on signal.
       *
       *   3) Search for peak amplitude.
       *
       *   4) Store peak amplitude, corresponding phase and magnitude
       *      in frqImg, phaseImg and magImg.
       */
      for (uint32 frmNum = 0; frmNum < movieLth; frmNum++) {
	real[frmNum] = (float)_movie[frmNum].getPixel(x,y);
	imag[frmNum] = 0.0;
      }

#if 0
      printf("%d %d:", x, y);
      for (uint32 g = 0; g < real.size(); g++)
	printf(" (%f %f)", real[g], imag[g]);
      printf("\n     :");

      if (xyzzy_cnt) {
	float* temp = (float*)malloc(movieLth*sizeof(float));
	rmAssert(temp);
	for (uint32 i = 0; i < movieLth; i++)
	  temp[i] = real[i];

	rf1Ddft(real, imag, 1);

	int32 retval = FFTRealForward(temp, (int32)movieLth);
	if (retval != 0)
	  fprintf(stderr, "Retval %d != 0\n", retval);
	rmAssert(retval == 0);

	for (uint32 i = 0; i < movieLth; i++) {
	  float dftval = sqrt(real[i]*real[i] + imag[i]*imag[i]);
	  if (fabs(temp[i]-dftval) > 0.0001)
	    printf("Cnt %d: pos %d: dft %f fft %f diff %f\n",
		   xyzzy_cnt, i, dftval, temp[i], fabs(temp[i]-dftval));
	}
	xyzzy_cnt--;
      }
      else
#endif
#if 0
	rf1Ddft(real, imag, 1);
#else
	rf1Dfft(real, imag, 1);
#endif

      float maxVal = -1.0;
      uint32 maxIndex = 1;

      for (uint32 i = 1; i < movieLth/2; i++) {
	float val = sqrt(real[i]*real[i] + imag[i]*imag[i]);
	//	printf(" (%f)", val);
	if (val > maxVal) {
	  maxVal = val;
	  maxIndex = i;
	}
      }
      //      printf("\n");

      float phase = atan2(imag[maxIndex], real[maxIndex]);
      *fp++ = maxIndex;
      *pp++ = phase;
      *ap++ = maxVal;
    }
  }
}

void
rcOptoKineticImage::genEightBitDFT(rcWindow freqImg, rcWindow freqEBImg,
				   rcWindow phaseImg, rcWindow phaseEBImg,
				   rcWindow amplitudeImg, rcWindow amplitudeEBImg,
				   uint32 expectedFreq)
{
  const int32 width = freqImg.width(), height = freqImg.height();
  rmAssert(rcPixel32 == freqImg.depth());
  rmAssert(width == phaseImg.width());
  rmAssert(height == phaseImg.height());
  rmAssert(rcPixel32 == phaseImg.depth());
  rmAssert(width == amplitudeImg.width());
  rmAssert(height == amplitudeImg.height());
  rmAssert(rcPixel32 == amplitudeImg.depth());
  rmAssert(width == freqEBImg.width());
  rmAssert(height == freqEBImg.height());
  rmAssert(rcPixel8 == freqEBImg.depth());
  rmAssert(width == phaseEBImg.width());
  rmAssert(height == phaseEBImg.height());
  rmAssert(rcPixel8 == phaseEBImg.depth());
  rmAssert(width == amplitudeEBImg.width());
  rmAssert(height == amplitudeEBImg.height());
  rmAssert(rcPixel8 == amplitudeEBImg.depth());

  vector<uint8> freqMap(_movie.size()/2);
  for (uint32 i = 0; i < freqMap.size(); i++)
    freqMap[i] = 0;
  rmAssert(expectedFreq != 0 && expectedFreq < freqMap.size());
  freqMap[expectedFreq] = 255;
  if (expectedFreq > 1)
    freqMap[expectedFreq-1] = 160;
  if (expectedFreq > 2)
    freqMap[expectedFreq-2] = 96;
  if (expectedFreq < freqMap.size() - 1)
    freqMap[expectedFreq+1] = 160;
  if (expectedFreq < freqMap.size() - 2)
    freqMap[expectedFreq+2] = 96;
  

  for (int32 y = 0; y < height; y++) {
    uint32* xp = (uint32*)freqImg.rowPointer(y);
    for (int32 x = 0; x <width; x++, xp++) {
      rmAssert(*xp < freqMap.size());
      freqEBImg.setPixel(x, y, freqMap[*xp]);
    }
  }

  for (int32 y = 0; y < height; y++) {
    float* xp = (float*)phaseImg.rowPointer(y);
    for (int32 x = 0; x <width; x++, xp++) {
      uint32 phase = (uint32)((*xp * 180)/rkPI);
      phaseEBImg.setPixel(x, y, (phase % 360)/2);
    }
  }

  float minVal, maxVal, mean, stdDev;
  genKineticStats(amplitudeImg, minVal, maxVal, mean, stdDev);

  float scale = (maxVal-minVal) ? 255/(maxVal-minVal) : 0;
  if (scale == 0)
    printf("WARNING: Scale == 0\n");
  printf("minVal %f maxVal %f mean %f stdDev %f\n", minVal, maxVal, mean, stdDev);

  for (int32 y = 0; y < height; y++) {
    float* xp = (float*)amplitudeImg.rowPointer(y);
    for (int32 x = 0; x <width; x++, xp++) {
      uint32 val = (uint32)((*xp - minVal)*scale);
      rmAssert(val < 256);
      amplitudeEBImg.setPixel(x, y, val);
    }
  }
}

#define FAST_PROCESSING

void rcOptoKineticImage::genFreqImg(rcWindow freqImg, float fpp,
				    const rcWindow* maskImg)
{
  const uint32 movieLth = _movie.size();
  rmAssert(movieLth);
  rmAssert(fpp > 0.0);

  const int32 width = _movie[0].width(), height = _movie[0].height();
  rmAssert(width == freqImg.width());
  rmAssert(height == freqImg.height());
  rmAssert(rcPixel32 == freqImg.depth());
  if (maskImg) {
    rmAssert(width == maskImg->width());
    rmAssert(height == maskImg->height());
    rmAssert(rcPixel8 == maskImg->depth());
  }

  vector<uint32> dummy;
  vector<float> peakLoc, peakVal;
  rs1DFourierResult result;
  float nomOffset = -1.0;

#ifdef FAST_PROCESSING
  rsAnalysisVoxels info;

  info.weightsSz = 0;
  info.weights = 0;
  info.workSigSz = 0;
  info.workSig = 0;
  info.workSigBase = 0;
  uint8* signal = (uint8*)malloc(movieLth);
  rmAssert(signal);
#else
  deque<double> signal(movieLth);
#endif

  for (int32 y = 0; y < height; y++) {
    if (0) printf("Processing line %d\n", y);
    float* fp = (float*)freqImg.rowPointer(y);
    uint8* mip = maskImg ? (uint8*)maskImg->rowPointer(y) : 0;
    for (int32 x = 0; x < width; x++) {
      if (mip && (*mip++ == 0)) {
	*fp++ = 0;
	continue;
      }

      for (uint32 f = 0; f < movieLth; f++)
	signal[f] = _movie[f].getPixel(x, y);

#ifdef FAST_PROCESSING
      rf1DFourierAnalysisVoxels(signal, movieLth, info, result,
				rc1DFourierForceFFT | rc1DFourierRaisedCosine);
#else
      rf1DFourierAnalysis(signal, result,
			  rc1DFourierForceFFT | rc1DFourierRaisedCosine);
#endif

      if (nomOffset == -1.0)
	nomOffset = result.workingSz/fpp;

      rf1DPeakDetect(result.amplitude, dummy, peakLoc, peakVal, 0.10f);

      float maxVal = rcFLT_MAX;
      float sigSum = 0.0, freqSum = 0.0;
      float expLoc = nomOffset;

      for (uint32 p = 0; p < peakLoc.size(); p++) {
	sigSum += peakVal[p];
	if (fabs(peakLoc[p] - expLoc) <= 0.5) {
	  maxVal = peakVal[p] < maxVal ? peakVal[p] : maxVal;
	  freqSum += maxVal;
	  expLoc = peakLoc[p] + nomOffset; // maybe += nomOffset??? xyzzy
	}
	else if (peakLoc[p] > expLoc)
	  maxVal = 0;
      }

      *fp++ = (sigSum != 0.0) ? freqSum/sigSum : 0.0;
    }
  }

#ifdef FAST_PROCESSING
  free(signal);
  if (info.weights)
    free(info.weights);
  if (info.workSigBase)
    free(info.workSigBase);
#endif
}

void
rcOptoKineticImage::gen1DCorrKineticImgs(uint32 period, uint32 pulseWidth,
					 rcWindow ampImg, rcWindow phaseImg,
					 rcWindow corrPeaks, rcWindow phasePeaks)
{
  rmAssert(pulseWidth > 0);
  rmAssert(pulseWidth <= period);

  const uint32 movieLth = _movie.size();
  const uint32 mdlLth = period;
  rmAssert(mdlLth > 1);
  rmAssert(movieLth >= mdlLth);
  rmAssert(rcPixel8 == _movie[0].depth());

  const int32 width = _movie[0].width(), height = _movie[0].height();
  rmAssert(ampImg.width() == width);
  rmAssert(ampImg.height() == height);
  rmAssert(ampImg.depth() == rcPixel8);
  rmAssert(phaseImg.width() == width);
  rmAssert(phaseImg.height() == height);
  rmAssert(phaseImg.depth() == rcPixel8);
  rmAssert(corrPeaks.width() == width);
  rmAssert(corrPeaks.height() == height);
  rmAssert(corrPeaks.depth() == rcPixel32);
  rmAssert(phasePeaks.width() == width);
  rmAssert(phasePeaks.height() == height);
  rmAssert(phasePeaks.depth() == rcPixel32);

  if (0) printf("xyzzy period %d pulse width %d movie lth %d\n",
		period, pulseWidth, movieLth);

  vector<uint8> mdl;
  genSynthSignal(period, pulseWidth, 255, 0, 1, mdl);

  const uint32 resultSpcSz = movieLth - mdlLth + 1;
  vector<double> tempCorrSpc(resultSpcSz), cumCorrSpcLow(resultSpcSz),
    cumCorrSpcHigh(resultSpcSz);
  for (uint32 i = 0; i < resultSpcSz; i++)
    cumCorrSpcLow[i] = cumCorrSpcHigh[i] = 0;

  vector<uint8> img(movieLth);

  double globalMinCorrScore = 2.0, globalMaxCorrScore = -1.0;

  for (int32 y = 0; y < height; y++) {
    float* corrP = (float*)corrPeaks.rowPointer(y);
    float* phaseP = (float*)phasePeaks.rowPointer(y);

    for (int32 x = 0; x < width; x++) {

      for (uint32 i = 0; i < movieLth; i++)
	img[i] = _movie[i].getPixel(x, y);

      double maxVal = -1.0;
      uint32 maxValLoc = 0;
      vector<uint8>::iterator iStart = img.begin(), iEnd = iStart + mdlLth;
      for (uint32 i = 0; i < resultSpcSz; i++) {
	tempCorrSpc[i] = rf1DNormalizedCorr(iStart + i, iEnd + i,
					    mdl.begin(), mdl.end());
	if (tempCorrSpc[i] > maxVal) {
	  maxValLoc = i;
	  maxVal = tempCorrSpc[i];
	}

	if (tempCorrSpc[i] > 1e-6)
	  cumCorrSpcHigh[i] += tempCorrSpc[i]*tempCorrSpc[i];
	else
	  cumCorrSpcLow[i] += tempCorrSpc[i]*tempCorrSpc[i];
      }

      if (0) {
	printf("xyzzy space (%d,%d):", x, y);
	for (uint32 i = 0; i < tempCorrSpc.size(); i++)
	  printf(" %f", tempCorrSpc[i]);
	printf("\n");
      }

      double delta, maxScore;
  
      if (maxValLoc == (resultSpcSz-1))  { // Peak is pegged at end
	delta = parabolicFit(tempCorrSpc[maxValLoc-1], tempCorrSpc[maxValLoc],
			     (double)0.0, &maxScore);
	rmAssert(delta <= 0.0);
      }
      else if (maxValLoc == 0) { // Peak is pegged at start
	delta = parabolicFit((double)0.0, tempCorrSpc[0], tempCorrSpc[1],
			     &maxScore);
	rmAssert(delta >= 0.0);
      }
      else { // Interpolate peak location
	delta = parabolicFit(tempCorrSpc[maxValLoc-1],tempCorrSpc[maxValLoc],
			     tempCorrSpc[maxValLoc+1], &maxScore);
      }

      if (maxScore < globalMinCorrScore)
	globalMinCorrScore = maxScore;
      if (maxScore > globalMaxCorrScore)
	globalMaxCorrScore = maxScore;

      *phaseP++ = (float)(maxValLoc + delta);
      *corrP++ = (float)maxScore;
    }
  } // End of: for (int32 y = 0; y < height; y++) {

  double maxVal = -1.0;
  uint32 maxValLoc = 0;
      
  for (uint32 i = 0; i < resultSpcSz; i++) {
    cumCorrSpcHigh[i] += cumCorrSpcLow[i];

    if (maxVal < cumCorrSpcHigh[i]) {
      maxVal = cumCorrSpcHigh[i];
      maxValLoc = i;
    }
  }

  if (0) printf("xyzzy maxVal %f maxValLoc %d\n", maxVal, maxValLoc);

  double delta;
  
  if (maxValLoc == (resultSpcSz-1))  { // Peak is pegged at end
    delta = parabolicFit(cumCorrSpcHigh[maxValLoc-1], cumCorrSpcHigh[maxValLoc],
			 (double)0.0, (double*)0);
    rmAssert(delta <= 0.0);
  }
  else if (maxValLoc == 0) { // Peak is pegged at start
    delta = parabolicFit((double)0.0, cumCorrSpcHigh[0], cumCorrSpcHigh[1],
			 (double*)0);
    rmAssert(delta >= 0.0);
  }
  else { // Interpolate peak location
    delta = parabolicFit(cumCorrSpcHigh[maxValLoc-1],cumCorrSpcHigh[maxValLoc],
			 cumCorrSpcHigh[maxValLoc+1], (double*)0);
  }

  float phaseSpcSz = mdlLth;
  float phasePeak = (float)fmod((maxValLoc + delta), (double)mdlLth);
  float phaseMod = phaseSpcSz/2.0;
  float phaseScale = 255.0/phaseMod;

  float corrMin = (float)globalMinCorrScore;
  float corrScale = 255.0/(float)(globalMaxCorrScore - globalMinCorrScore);

  if (0) printf("xyzzy corr min %f max %f scale %f\n", corrMin,
		globalMaxCorrScore, corrScale);

  for (int32 y = 0; y < height; y++) {
      float* corrP = (float*)corrPeaks.rowPointer(y);
      float* phaseP = (float*)phasePeaks.rowPointer(y);
      uint8* ampImgP = (uint8*)ampImg.rowPointer(y);
      uint8* phaseImgP = (uint8*)phaseImg.rowPointer(y);
    for (int32 x = 0; x < width; x++) {
      float phaseDelta
	= fabs(phaseMod - fabs(fmod(*phaseP++, phaseSpcSz) - phasePeak));
      uint32 pVal = (uint32)(phaseDelta*phaseScale);
      rmAssert(pVal <= 255);
      *phaseImgP++ = (uint8)pVal;

      pVal = (uint32)((*corrP++ - corrMin)*corrScale);
      rmAssert(pVal <= 255);
      *ampImgP++ = (uint8)pVal;
    }
  }

  if (1)
    printf("xyzzy phase space sz %f peak %f mod %f scale %f\n",
	   phaseSpcSz, phasePeak, phaseMod, phaseScale);

  if (0) {
    for (int32 y = 0; y < phasePeaks.height(); y++) {
      float* pP = (float*)phasePeaks.rowPointer(y);
      printf("%02d:", y);
      for (int32 x = 0; x < phasePeaks.width(); x++)
	printf(" %f", *pP++);
      printf("\n");
    }
    printf("\n");
    for (int32 y = 0; y < phaseImg.height(); y++) {
      uint8* pP = (uint8*)phaseImg.rowPointer(y);
      printf("%02d:", y);
      for (int32 x = 0; x < phaseImg.width(); x++)
	printf(" %03d", *pP++);
      printf("\n");
    }
  }


}

void
rcOptoKineticImage::genEightBitFreq(rcWindow freqImg, rcWindow freqEBImg,
				    const float expectedFreq)
{
  const int32 width = freqImg.width(), height = freqImg.height();
  rmAssert(rcPixel32 == freqImg.depth());
  rmAssert(width == freqEBImg.width());
  rmAssert(height == freqEBImg.height());
  rmAssert(rcPixel8 == freqEBImg.depth());

  for (int32 y = 0; y < height; y++) {
    float* xp = (float*)freqImg.rowPointer(y);
    for (int32 x = 0; x < width; x++, xp++) {
      float fdiff = fabs((float)(expectedFreq - *xp));
      if (fdiff <= 0.5)
	freqEBImg.setPixel(x, y, 255);
      else if (fdiff <= 1.0)
	freqEBImg.setPixel(x, y, 128);
      else
	freqEBImg.setPixel(x, y, 0);
    }
  }
}

void
rcOptoKineticImage::genEightBitImg(const rcWindow& freqImg, rcWindow& freqEBImg)
{
  const int32 width = freqImg.width(), height = freqImg.height();
  rmAssert(rcPixel32 == freqImg.depth());
  rmAssert(width == freqEBImg.width());
  rmAssert(height == freqEBImg.height());
  rmAssert(rcPixel8 == freqEBImg.depth() || rcPixel32 == freqEBImg.depth ());


  if (freqEBImg.depth() == rcPixel8)
    {
      float pmax (0.0f), pmin (1.0f);
      for (int32 y = 0; y < height; y++)
	{
	  float* xp = (float*)freqImg.rowPointer(y);
	  for (int32 x = 0; x < width; x++, xp++)
	    {
	      float p (*xp);
	      if (p >= 1.0f) continue;
	      pmin = rmMin (p, pmin);
	      pmax = rmMax (p, pmax);
	    }
	}

      for (int32 y = 0; y < height; y++)
	{
	  float* xp = (float*)freqImg.rowPointer(y);
	  uint8* destRow = (uint8*)freqEBImg.rowPointer(y);
	  for (int32 x = 0; x < width; x++, xp++, destRow++)
	    {
	      float p (*xp);
	      if (p >= 1.0f) continue;
	      if (p <= 0.0)
		p = 128 - (p * 127.0) / pmin;
	      else
		p = 128 + (p * 127.0) / pmax;
	      int32 pv ((int32) p);
	      *destRow = (uint8) pv;
	    }
	}
    }
  else if (freqEBImg.depth() == rcPixel32)
    {
      float pmax (0.0f), pmin (1.0f);
      for (int32 y = 0; y < height; y++)
	{
	  float* xp = (float*)freqImg.rowPointer(y);
	  for (int32 x = 0; x < width; x++, xp++)
	    {
	      float p (*xp);
	      if (p >= 1.0f) continue;
	      pmin = rmMin (p, pmin);
	      pmax = rmMax (p, pmax);
	    }
	}

      for (int32 y = 0; y < height; y++)
	{
	  float* xp = (float*)freqImg.rowPointer(y);
	  uint32* destRow = (uint32 *)freqEBImg.rowPointer(y);
	  for (int32 x = 0; x < width; x++, xp++, destRow++)
	    {
	      float p (*xp);
	      uint32 r, g, b;
	      g = (uint32) (p * 255);
	      r = b = g;


	      if (p == 0.0f)
		*destRow = rfRgb (r, g, b);
	      else
		{
		  if (p < 0.0)
		    {
		      r = 255;
		      b = 0;
		    }
		  else
		    {
		      b = 255;
		      r = 0;
		    }

		  uint32 pv = rfRgb (r, g, b);
		  *destRow = pv;
		}
	    }
	}

    }
}

void rcOptoKineticImage::genEightBitImg(const rcWindow& kineticImg,
					rcWindow ebImg, float sdClamp,
					const vector<uint8>* map)
{
  const int32 width = kineticImg.width(), height = kineticImg.height();
  rmAssert(width == ebImg.width());
  rmAssert(height == ebImg.height());
  rmAssert(ebImg.depth() == rcPixel8);

  /* Step 1 - Calculate min, max, mean, and std dev.
   */
  float minVal, maxVal, stdDev, mean;
  genKineticStats(kineticImg, minVal, maxVal, mean, stdDev);

  if (0) printf("gebi: clamp %f map 0x%X min %f max %f sd %f mean %f\n",
		sdClamp, (int)map, minVal, maxVal, stdDev, mean);

  /* Step 2 - Figure out minLimit, maxLimit, center, delta, and binSzInv values.
   */
  float minLimit, maxLimit, center, delta, binSzInv;

  /* If clamp value was specified, center the translation around the
   * mean and extend out to sdClamp number of std dev.
   *
   * If not, use the entire dynamic range.
   */
  if (sdClamp > 0) {
    center = mean;
    delta = stdDev*sdClamp;
    minLimit = center - delta;
    maxLimit = center + delta;
    rmAssert(maxLimit - minLimit);
    binSzInv = 256/(maxLimit - minLimit);
  }
  else {
    center = (float)((maxVal - minVal)/2);
    delta = (float)(center - minVal);
    minLimit = (float)minVal;
    maxLimit = (float)maxVal;
    rmAssert(maxLimit - minLimit);
    binSzInv = (float)(256/(maxLimit - minLimit));
  }

  /* Step 3 - Map pixels.
   */
  if (map) {
    const vector<uint8>& mapR(*map);
    rmAssert(mapR.size() >= 256);
    for (int32 y = 0; y < height; y++) {
      const float* srcRow = (const float*)kineticImg.rowPointer(y);
      uint8* destRow = (uint8*)ebImg.rowPointer(y);
      for (int32 x = 0; x < width; x++) {
	const float val = *srcRow++;
	if (val <= minLimit)
	  *destRow++ = mapR[0];
	else if (val >= maxLimit)
	  *destRow++ = mapR[255];
	else {
	  const uint32 index = (uint32)((val - center + delta) * binSzInv);
	  if (index >= 256) { // xyzzy
	    printf("minVal %f maxVal %f\n", minVal, maxVal);
	    printf("index %d val %f center %f delta %f sdClamp %f stdDev %f\n",
		   index, val, center, delta, sdClamp, stdDev);
	    printf("minLimit %f maxLimit %f binSzInv %f\n",
		   minLimit, maxLimit, binSzInv);
	  }
	  rmAssert(index < 256);
	  *destRow++ = mapR[index];
	}
      }
    }
  }
  else {
     for (int32 y = 0; y < height; y++) {
      const float* srcRow = (const float*)kineticImg.rowPointer(y);
      uint8* destRow = (uint8*)ebImg.rowPointer(y);
      for (int32 x = 0; x < width; x++) {
	const float val = *srcRow++;
	if (val <= minLimit)
	  *destRow++ = 0;
	else if (val >= maxLimit)
	  *destRow++ = 255;
	else {
	  const uint8 pVal = (uint8)((val - center + delta) * binSzInv);
	  //	  rmAssert(pVal < 256); True by definition
	  *destRow++ = pVal;
	}
      }
    }
  }
}

void rcOptoKineticImage::genEightBitImg(const rcWindow& kineticImg,
					rcWindow ebImg, vector<float>& map)
{
  const int32 width = kineticImg.width(), height = kineticImg.height();
  
  rmAssert(width == ebImg.width());
  rmAssert(height == ebImg.height());
  rmAssert(kineticImg.depth() == rcPixel32);
  rmAssert(ebImg.depth() == rcPixel8);

  const uint32 mapSz = map.size() > 255 ? 255 : map.size();

  for (int32 y = 0; y < height; y++) {
    const float* srcRow = (const float*)kineticImg.rowPointer(y);
    uint8* destRow = (uint8*)ebImg.rowPointer(y);
    for (int32 x = 0; x < width; x++) {
      float sVal = *srcRow++;
      uint32 pVal;
      for (pVal = 0; pVal < mapSz; pVal++)
	if (sVal < map[pVal])
	  break;
      *destRow++ = pVal;
    }
  }
}

void rcOptoKineticImage::genVarImg(rcWindow stdDevImg)
{
  const uint32 n = _movie.size();
  rmAssert(n > 1);
  const float N = n;
  const int32 width = stdDevImg.width(), height = stdDevImg.height();
  rmAssert(width == _sum.width());
  rmAssert(height == _sum.height());
  rmAssert(stdDevImg.depth() == rcPixel32);
  
  if (_sum.depth() == rcPixel16) {
    for (int32 y = 0; y < height; y++) {
      float *varRow = (float*)stdDevImg.rowPointer(y);
      uint16 *sumRow = (uint16*)_sum.rowPointer(y);
      uint32 *sumSqRow = (uint32*)_sumSq.rowPointer(y);
      
      for (int32 x = 0; x < width; x++) {
	const float sumSq = *sumSqRow++;
	const float sum = *sumRow++;
	*varRow++ = (N*sumSq - sum*sum)/(N*N - N);
      }
    }
  }
  else if (_sum.depth() == rcPixel32) {
    for (int32 y = 0; y < height; y++) {
      float *varRow = (float*)stdDevImg.rowPointer(y);
      uint32 *sumRow = (uint32*)_sum.rowPointer(y);
      uint32 *sumSqRow = (uint32*)_sumSq.rowPointer(y);
      
      for (int32 x = 0; x < width; x++) {
	const float sumSq = *sumSqRow++;
	const float sum = *sumRow++;
	*varRow++ = (N*sumSq - sum*sum)/(N*N - N);
      }
    }
  }
  else
    rmAssert(0);
}

void rcOptoKineticImage::genStdDevImg(rcWindow stdDevImg)
{
  const uint32 n = _movie.size();
  rmAssert(n > 1);
  const float N = n;
  const int32 width = stdDevImg.width(), height = stdDevImg.height();
  rmAssert(width == _sum.width());
  rmAssert(height == _sum.height());
  rmAssert(stdDevImg.depth() == rcPixel32);
  
  if (_sum.depth() == rcPixel16) {
    for (int32 y = 0; y < height; y++) {
      float *sdRow = (float*)stdDevImg.rowPointer(y);
      uint16 *sumRow = (uint16*)_sum.rowPointer(y);
      uint32 *sumSqRow = (uint32*)_sumSq.rowPointer(y);
      
      for (int32 x = 0; x < width; x++) {
	const float sumSq = *sumSqRow++;
	const float sum = *sumRow++;
	*sdRow++ = sqrt((N*sumSq - sum*sum)/(N*N - N));
      }
    }
  }
  else if (_sum.depth() == rcPixel32) {
    for (int32 y = 0; y < height; y++) {
      float *sdRow = (float*)stdDevImg.rowPointer(y);
      uint32 *sumRow = (uint32*)_sum.rowPointer(y);
      uint32 *sumSqRow = (uint32*)_sumSq.rowPointer(y);
      
      for (int32 x = 0; x < width; x++) {
	const float sumSq = *sumSqRow++;
	const float sum = *sumRow++;
	*sdRow++ = sqrt((N*sumSq - sum*sum)/(N*N - N));
      }
    }
  }
  else
    rmAssert(0);
}

#define SHANNON(R) (-1.0 * R * log2(R))

void rcOptoKineticImage::genVelEntropyImg(rcWindow entropyImg)
{
  const uint32 n = _movie.size();
  rmAssert(n);
  const int32 width = entropyImg.width(), height = entropyImg.height();
  rmAssert(width == _sum.width());
  rmAssert(height == _sum.height());
  rmAssert(entropyImg.depth() == rcPixel32);
  vector<float> vel(n);
  const float N = n;
  const float log2Sz = log2(N);
  _uCnt = _oCnt = 0;

  if (_sum.depth() == rcPixel16) {
    for (int32 y = 0; y < height; y++) {
      float* entRow = (float*)entropyImg.rowPointer(y);
      uint16* sumRow = (uint16*)_sum.rowPointer(y);
      uint32* sumSqRow = (uint32*)_sumSq.rowPointer(y);
      
      for (int32 x = 0; x < width; x++) {
	/* Step 1 - Calculate the std dev and mean for this pixel
	 * location from the sum and sum square images.
	 */
	const float sumSq = *sumSqRow++;
	const float sum = *sumRow++;
	const float sd = sqrt((sumSq*N - sum*sum)/(N*N - N));
	const float mean = sum/N;
	const float maxSd = sd*4;
	const float minVal = rcFLT_MIN, maxVal = maxSd*2;

	/* Step 2 - Calculate the normalized "velocity" values. At the
	 * same time, calculate the sum of these normalized values.
	 */
	float velSum = 0.0;
	for (uint32 i = 0; i < n; i++) {
	  float val = _movie[i].getPixel(x, y) - mean + maxSd;
	  if (val < minVal) {
	    val = minVal;
	    _uCnt++;
	  }
	  else if (val > maxVal) {
	    val = maxVal;
	    _oCnt++;
	  }

	  velSum += val;
	  vel[i] = val;
	}

	/* Step 3 - Generate shannon's entropy for this pixel
	 * location.
	 */
	float entropies = 0.0;
	for (uint32 i = 0; i < n; i++) {
	  const float r = vel[i]/velSum;
	  entropies += SHANNON(r);
	}

	float entropy = entropies/log2Sz;
	*entRow++ = entropy > 1.0 ? 1.0 : entropy;
      }
    }
  }
  else if (_sum.depth() == rcPixel32) {
    for (int32 y = 0; y < height; y++) {
      float* entRow = (float*)entropyImg.rowPointer(y);
      uint32* sumRow = (uint32*)_sum.rowPointer(y);
      uint32* sumSqRow = (uint32*)_sumSq.rowPointer(y);
      
      for (int32 x = 0; x < width; x++) {
	/* Step 1 - Calculate the std dev and mean for this pixel
	 * location from the sum and sum square images.
	 */
	const float sumSq = *sumSqRow++;
	const float sum = *sumRow++;
	const float sd = sqrt((sumSq*N - sum*sum)/(N*N - N));
	const float mean = sum/N;
	const float maxSd = sd*4;
	const float minVal = rcFLT_MIN, maxVal = maxSd*2;

	/* Step 2 - Calculate the normalized "velocity" values. At the
	 * same time, calculate the sum of these normalized values.
	 */
	float velSum = 0.0;
	for (uint32 i = 0; i < n; i++) {
	  float val = _movie[i].getPixel(x, y) - mean + maxSd;
	  if (val < minVal) {
	    val = minVal;
	    _uCnt++;
	  }
	  else if (val > maxVal) {
	    val = maxVal;
	    _oCnt++;
	  }
	  velSum += val;
	  vel[i] = val;
	}

	/* Step 3 - Generate shannon's entropy for this pixel
	 * location.
	 */
	float entropies = 0.0;
	for (uint32 i = 0; i < n; i++) {
	  const float r = vel[i]/velSum;
	  entropies += SHANNON(r);
	}

	float entropy = entropies/log2Sz;
	*entRow++ = entropy > 1.0 ? 1.0 : entropy;
      }
    }
  }
  else
    rmAssert(0);
}

void rcOptoKineticImage::genAccelEntropyImg(rcWindow entropyImg)
{
  const uint32 n = _movie.size() - 1;
  rmAssert(_movie.size() > 1);
  const int32 width = entropyImg.width(), height = entropyImg.height();
  rmAssert(width == _movie[0].width());
  rmAssert(height == _movie[0].height());
  rmAssert(entropyImg.depth() == rcPixel32);
  vector<float> vel(n);
  const float N = n;
  const float log2Sz = log2(N);

  for (int32 y = 0; y < height; y++) {
    float* entRow = (float*)entropyImg.rowPointer(y);
      
    for (int32 x = 0; x < width; x++) {
      /* Step 1 - Calculate the change in adjacent pixels (velocity).
       * At the same time, calculate the sum of these velocities.
       */
      float velSum = 0.0;
      int32 p0 = (int32)_movie[0].getPixel(x, y);
      for (uint32 i = 0; i < n; i++) {
	int32 p1 = (int32)_movie[i+1].getPixel(x, y);
	vel[i] = p0 - p1 + 256;
	velSum += vel[i];
	p0 = p1;
      }

      /* Step 2 - Generate shannon's entropy for this pixel
       * location.
       */
      float entropies = 0.0;
      for (uint32 i = 0; i < n; i++) {
	const float r = vel[i]/velSum;
	entropies += SHANNON(r);
      }

      *entRow++ = entropies/log2Sz;
    }
  }
}

void rcOptoKineticImage::genThetaEntropyImg(rcWindow entropyImg)
{
  const uint32 n = _movie.size() - 1;
  rmAssert(_movie.size() > 1);
  const int32 width = entropyImg.width(), height = entropyImg.height();
  rmAssert(width == _movie[0].width());
  rmAssert(height == _movie[0].height());
  rmAssert(entropyImg.depth() == rcPixel32);
  vector<float> theta(n);
  const float N = n;
  const float log2Sz = log2(N);

  initTheta();
  for (int32 y = 0; y < height; y++) {
    float* entRow = (float*)entropyImg.rowPointer(y);
      
    for (int32 x = 0; x < width; x++) {
      /* Step 1 - Calculate the thetas of adjacent pixels. At the same
       * time, calculate the sum of these thetas.
       */
      float thetaSum = 0.0;
      int32 p0 = (int32)_movie[0].getPixel(x, y);
      for (uint32 i = 0; i < n; i++) {
	int32 p1 = (int32)_movie[i+1].getPixel(x, y);
	theta[i] = _theta[p0 - p1 + 255];
	thetaSum += theta[i];
	p0 = p1;
      }

      /* Step 2 - Generate shannon's entropy for this pixel
       * location.
       */
      float entropies = 0.0;
      for (uint32 i = 0; i < n; i++) {
	const float r = theta[i]/thetaSum;
	entropies += SHANNON(r);
      }

      *entRow++ = entropies/log2Sz;
    }
  }
}

void rcOptoKineticImage::decrementU8(const rcWindow& img)
{
  const int32 width = img.width(), height = img.height();
  rmAssert(width == _sum.width());
  rmAssert(height == _sum.height());

  if (_sum.depth() == rcPixel16) {
    for (int32 y = 0; y < height; y++) {
      const uint8* pelRow = (const uint8*)img.rowPointer(y);
      uint16* sumRow = (uint16*)_sum.rowPointer(y);
      uint32* sumSqRow = (uint32*)_sumSq.rowPointer(y);

      for (int32 x = 0; x < width; x++) {
	const uint16 pel = *pelRow++;
	*sumRow++ -= pel;
	*sumSqRow++ -= pel*pel;
      }
    }
  }
  else if (_sum.depth() == rcPixel32) {
    for (int32 y = 0; y < height; y++) {
      const uint8* pelRow = (const uint8*)img.rowPointer(y);
      uint32* sumRow = (uint32*)_sum.rowPointer(y);
      uint32* sumSqRow = (uint32*)_sumSq.rowPointer(y);

      for (int32 x = 0; x < width; x++) {
	const uint16 pel = *pelRow++;
	*sumRow++ -= pel;
	*sumSqRow++ -= pel*pel;
      }
    }
  }
  else
    rmAssert(0);
}


void rcOptoKineticImage::incrementU8(const rcWindow& img)
{
  const int32 width = img.width(), height = img.height();
  if ((width != _sum.width()) || (height != _sum.height())) {
    rmAssert(_movie.size() == 0);
    _sum = rcWindow(width, height, rcPixel16);
    _sumSq= rcWindow(width, height, rcPixel32);
    _sum.setAllPixels(0);
    _sumSq.setAllPixels(0);
  }

  /* Prevent counter overflows.
   */
  if ((_sum.depth() == rcPixel16) && (_movie.size() > 257)) {
    rcWindow newSum(_sum.width(), _sum.height(), rcPixel32);

    for (int32 y = 0; y < height; y++) {
      uint16* oldSumRow = (uint16*)_sum.rowPointer(y);
      uint32* newSumRow = (uint32*)newSum.rowPointer(y);

      for (int32 x = 0; x < width; x++)
	*newSumRow++ = *oldSumRow++;
    }
    _sum = newSum;
  }

  if (_sum.depth() == rcPixel16) {
    for (int32 y = 0; y < height; y++) {
      const uint8* pelRow = (const uint8*)img.rowPointer(y);
      uint16* sumRow = (uint16*)_sum.rowPointer(y);
      uint32* sumSqRow = (uint32*)_sumSq.rowPointer(y);

      for (int32 x = 0; x < width; x++) {
	const uint16 pel = *pelRow++;
	*sumRow++ += pel;
	*sumSqRow++ += pel*pel;
      }
    }
  }
  else if (_sum.depth() == rcPixel32) {
    for (int32 y = 0; y < height; y++) {
      const uint8* pelRow = (const uint8*)img.rowPointer(y);
      uint32* sumRow = (uint32*)_sum.rowPointer(y);
      uint32* sumSqRow = (uint32*)_sumSq.rowPointer(y);

      for (int32 x = 0; x < width; x++) {
	const uint16 pel = *pelRow++;
	*sumRow++ += pel;
	*sumSqRow++ += pel*pel;
      }
    }
  }
  else
    rmAssert(0);
}

void rcOptoKineticImage::initTheta()
{
  if (!_theta.empty())
    return;

  _theta.resize(511);

#if 1
  float offset = -atan((float)-255) + 0.1;
  for (int32 i = -255; i < 256; i++)
    _theta[255 + i] = atan((float)i) + offset;
#else
  for (int32 i = 0; i < 252; i++)
    _theta[i] = i*4;
  for (int32 i = 252; i < 259; i++)
    _theta[i] = 252*4 + i - 252;
  for (int32 i = 259; i < 511; i++)
    _theta[i] = 252*4 + 258 - 252 + (i-258)*4;

  for (int32 i = 0; i < 511; i++)
    printf("%d: %f\n", i, _theta[i]);
#endif
}


//////////
// 
/////////


void rcOptoKineticImage::genLogRMS(rcWindow freqImg)
{
  const uint32 movieLth = _movie.size();
  rmAssert(movieLth);

  const int32 width = _movie[0].width(), height = _movie[0].height();
  rmAssert(width == freqImg.width());
  rmAssert(height == freqImg.height());
  rmAssert(rcPixel32 == freqImg.depth());

  deque<float> signal(movieLth);

  for (int32 y = 0; y < height; y++)
    {
      float* fp = (float*)freqImg.rowPointer(y);
      for (int32 x = 0; x < width; x++)
	{
	  rcLsFit<float> ls;

	  // Basic: fit a line to I(t).
	  // RMS: fit a line to <I(t)>
	  // LogRMS: fit a line to Log (<I(t)>)

	  // Change this to call rfRMS
	  float pv = (float) _movie[0].getPixel(x, y);
	  float sumsq (rmSquare (pv));
	  for (uint32 f = 1; f < movieLth; f++)
	    {
	      signal[f-1] = sqrt (sumsq / (float) f);
	      float pv = (float) _movie[f].getPixel(x, y);
	      sumsq += rmSquare (pv);
	    }

	  for (uint32 f = 0; f < movieLth; f++)
	    {
	      rc2Fvector it ((float) f / (float) movieLth, 
			     signal[f] / 255.0f);
	      ls.update (it);
	    }

	  rcLineSegment<float> l = ls.fit ();

	  *fp++ = cos (l.angle());
	}
    }

}
