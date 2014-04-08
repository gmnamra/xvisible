/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_filter1d.cpp
 *   Creation Date  07/17/2003
 *   Author         Peter Roberts
 *
 * Some simple 1D filtering capability
 *
 *****************************************************************************/

#include <rc_filter1d.h>

rcEdgeFilter1D::rcEdgeFilter1D() : _filter(1), _stdDev(0.0), _min(0), _max(0)
{
  _filter[0] = 1;
}

rcEdgeFilter1D::~rcEdgeFilter1D()
{
}

void rcEdgeFilter1D::filter(const vector<int32>& filter)
{
  rmAssert(filter.size());
  _filter = filter;
}

void rcEdgeFilter1D::genPeaks(const rcWindow& img,
			      vector<int32>* filteredImg,
			      vector<rcEdgeFilter1DPeak>& peak,
			      int32 filterOffset,
			      double absMinPeak, double relMinPeak)
{
  const uint32 iSz = img.width();
  const uint32 fSz = _filter.size();
  rmAssert(iSz >= fSz);
  const uint32 fiSz = iSz - fSz + 1;
  
  vector<int32>& fImg = filteredImg ? *filteredImg : _filteredImg;

  /* Note: Possible performance hack would be to test for "<", but this
   * way the caller knows exactly how large the filtered image is.
   */
  if (fImg.size() != fiSz)
    fImg.resize(fiSz);

  /* Generate the filtered image. Along the way calculate 3 statistics
   * for the filtered image: std dev, min value, max value.
   */
  _min = 0xFFFFFFFF;
  _max = 0;
  double sumY = 0, sumYY = 0;
  
  for (uint32 fi = 0; fi < fiSz; fi++) {
    int32 curVal = 0;
    for (uint32 f = 0; f < fSz; f++)
      curVal += (int32)img.getPixel(fi+f, 0) * _filter[f];

    fImg[fi] = curVal;
    double absVal = fabs((double)fImg[fi]);
    sumY += absVal;
    sumYY += absVal*absVal;
    if (absVal < _min)
      _min = (uint32)absVal;
    if (absVal > _max)
      _max = (uint32)absVal;
  }

  double n = fiSz;
  _stdDev = sqrt((n*sumYY - sumY*sumY)/(n*(n-1)));
  
  /* Now search the filtered image for peaks and store them.
   */
  int32 rMin = 0;
  if (relMinPeak > 0.0)
    rMin = (int32)(relMinPeak*_stdDev + 0.5);

  int32 aMin = 0;
  if (absMinPeak > 0.0)
    aMin = (int32)(absMinPeak + 0.5);

  int32 minPeakValuePos = rMin > aMin ? rMin : aMin;
  int32 minPeakValueNeg = -minPeakValuePos;

  peak.resize(0);
  rcEdgeFilter1DPeak result;
  
  for (uint32 fi = 0; fi < fiSz; fi++) {
    if (fImg[fi] < 0) {
      if (fImg[fi] <= minPeakValueNeg) {
	result._value = fImg[fi];
	result._location = fi + filterOffset;
	peak.push_back(result);
      }
    }
    else if (fImg[fi] >= minPeakValuePos) {
      result._value = fImg[fi];
      result._location = fi + filterOffset;
      peak.push_back(result);
    }
  }
}

void rcEdgeFilter1D::genFilter(vector<int32>& filter, const uint32 oneCnt,
			       const uint32 zeroCnt)
{
  rmAssert(oneCnt);
  filter.resize(oneCnt*2 + zeroCnt);

  uint32 i = 0;

  while (i < oneCnt)
    filter[i++] = -1;

  if (zeroCnt)
    while (i < oneCnt + zeroCnt)
      filter[i++] = 0;

  while (i < oneCnt*2 + zeroCnt)
    filter[i++] = 1;
}


// Unfortunate code duplication for now

void rcEdgeFilter1D::genPeaks(const vector<float>& signal,
			      vector<int32>* filteredImg,
			      vector<rcEdgeFilter1DPeak>& peak,
			      int32 filterOffset,
			      double absMinPeak, double relMinPeak)
{
  const uint32 iSz = signal.size();
  const uint32 fSz = _filter.size();
  rmAssert(iSz >= fSz);
  const uint32 fiSz = iSz - fSz + 1;
  
  vector<int32>& fImg = filteredImg ? *filteredImg : _filteredImg;

  /* Note: Possible performance hack would be to test for "<", but this
   * way the caller knows exactly how large the filtered image is.
   */
  if (fImg.size() != fiSz)
    fImg.resize(fiSz);

  /* Generate the filtered image. Along the way calculate 3 statistics
   * for the filtered image: std dev, min value, max value.
   */
  _min = 0xFFFFFFFF;
  _max = 0;
  double sumY = 0, sumYY = 0;
  
  for (uint32 fi = 0; fi < fiSz; fi++) {
    int32 curVal = 0;
    for (uint32 f = 0; f < fSz; f++)
      curVal += (int32)signal[fi+f] * _filter[f];

    fImg[fi] = curVal;
    double absVal = fabs((double)fImg[fi]);
    sumY += absVal;
    sumYY += absVal*absVal;
    if (absVal < _min)
      _min = (uint32)absVal;
    if (absVal > _max)
      _max = (uint32)absVal;
  }

  double n = fiSz;
  _stdDev = sqrt((n*sumYY - sumY*sumY)/(n*(n-1)));
  
  /* Now search the filtered image for peaks and store them.
   */
  int32 rMin = 0;
  if (relMinPeak > 0.0)
    rMin = (int32)(relMinPeak*_stdDev + 0.5);

  int32 aMin = 0;
  if (absMinPeak > 0.0)
    aMin = (int32)(absMinPeak + 0.5);

  int32 minPeakValuePos = rMin > aMin ? rMin : aMin;
  int32 minPeakValueNeg = -minPeakValuePos;

  peak.resize(0);
  rcEdgeFilter1DPeak result;
  
  for (uint32 fi = 0; fi < fiSz; fi++) {
    if (fImg[fi] < 0) {
      if (fImg[fi] <= minPeakValueNeg) {
	result._value = fImg[fi];
	result._location = fi + filterOffset;
	peak.push_back(result);
      }
    }
    else if (fImg[fi] >= minPeakValuePos) {
      result._value = fImg[fi];
      result._location = fi + filterOffset;
      peak.push_back(result);
    }
  }
}
