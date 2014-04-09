
#ifndef __rcHISTOSTAT_H__
#define __rcHISTOSTAT_H__

#include <rc_types.h>
#include <rc_window.h>

#include <vector>
using namespace std;

// Mainly for 8 bit image support
/*
 number of samples
 mean
 median
 mode
 minimum, maximum
 inverse cumulative
 standard deviation
 variance

 A histogram is represented by a stl vector
 */

class rcHistoStats
{
public:

  rcHistoStats();

  rcHistoStats(const rcWindow& image, int32 bins = 256);
  /*
    requires an allocated image. Supports 8bit and 16bit images only
    
  */

  rcHistoStats(vector<uint32>& histogram,bool xown=false);
  /*
    requires   histogram.size() > 0
    effect     Constructs a ccHistoStats using the given histogram.  All
               statistics will be computed using the histogram hist.
  */

  /* default copy ctor, assignment, dtor ok */
  /*
    effect     This ccHistoStats becomes a copy of rhs.
  */

  uint32 n() const { return n_; }
  /*
    effect     Returns the total number of samples in the histogram supplied
               at construction.
    requires   This ccHistoStats was not default constructed.
  */

  uint32 bins () const { return bins_; }
  /*
    effect     Returns the total number of bins in the histogram supplied
               at construction. This is mostly for 16 bit images since almost always
	       the capture image depth is less than 12 bits. 
    requires   This ccHistoStats was not default constructed.
  */

  double mean() const;
  /*
    effect     Returns the average value in the histogram supplied at
               construction.  Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  int32 mode() const;
  /*
    effect     Returns the most frequent value in the histogram supplied
               at construction.  Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  int32 median() const;
  /*
    effect     Returns the median value in the histogram supplied at
               construction.  Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  float interpolatedMedian() const;
  /*
    effect     Returns the interpolated median value in the histogram supplied at
               construction.  Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  int32 min(const int32 discardPels = 0) const;
  int32 max(const int32 discardPels = 0) const;
  /*
    effect     Returns the minimum(maximum) value in the histogram supplied
               at construction.  Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  int32 inverseCum(int percent) const;
  /*
    effect     Returns the value below which a specified percentage of values
               in the histogram fall.  Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  double sDev() const;
  /*
    effect     Returns the standard deviation of the values in the histogram
               supplied at construction.  Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  double entropy () const;

  /*
    effect     Returns Entropy of this histogram
               Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  double var() const;
  /*
    effect     Returns the variance of the values in the histogram supplied
               at construction.  Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  void stretchMapUs (vector<uint16>& map, const int32 leftDicard, 
		     const int32 rightDiscard) const;

  /*
    effect     Returns a stretch map for this histogram
    new_i = 65535 * (old_i - lowVal) / val;

               at construction.  Returns zero if nSamp() is zero.
    requires   This ccHistoStats was not default constructed.
  */

  const vector<uint32> &histogram() const { return histogram_;}
  /*
    returns    Histogram given at construction
  */

  void pdf (vector<float>& p) const;
  /*
    returns    Histogram given at construction
  */

  void integral (vector<uint32>& hist) const;
  /*
    returns    The integral image
  */

  void integralPdf (vector<float>& ph) const;
  /*
    returns    The integral image / total sample size
  */


  void equalizationMap (vector<uint8>& hist) const;
  /*
    returns   an equalization map for this histogram
  */

  friend ostream& operator<< (ostream& o, const rcHistoStats& p);

private:

  void computeNsamp();
  long computeInverseCum(int percent);  // compute and make valid the ic,
                                        //   return ic_[percent]
  void computeMoments();                // compute mean,sDev,var,energry

  vector<uint32> histogram_;         // histogram given at construction

  /* flags for already computed answers */
  uint8 computedMoments_;
  uint8 computedIC_;
  uint8 computedMode_;

  /* already computed answers */
  uint32 n_;     // always valid
  long mode_;          // valid if computeMode_
  double mean_;        // valid if computeMoments_
  double sDev_;        // valid if computeMoments_
  double var_;         // valid if computeMoments_
  vector<long> ic_;   // valid if computeIC_
  uint32 bins_; // always valid
};


class rcGrayCoPeak
{
public:
  rcGrayCoPeak() {};
  rcGrayCoPeak(uint32 v, uint32 r) : value (v), radius (r) {}

  uint32 value;
  uint32 radius;
  bool operator<(const rcGrayCoPeak& rhs) const
  {return value < rhs.value;}
  bool operator==(const rcGrayCoPeak& rhs) const
    {return value == rhs.value;}
  bool operator!=(const rcGrayCoPeak& rhs) const;
  bool operator>(const rcGrayCoPeak& rhs) const;

  friend ostream& operator<< (ostream& o, const rcGrayCoPeak& p)
  {
    o << "[" << p.radius << "]: " << p.value << " ";
    return o;
  }
  
};


int32 rf2dGrayCoHistogram(const rcWindow& image, rcWindow& hist);
void rfGenLutFromCoGray (const rcWindow& co, vector<uint8>& lut);

  
#endif // __rcHISTOSTAT_H__
