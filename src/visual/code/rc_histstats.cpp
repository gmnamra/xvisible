
#include <rc_histstats.h>
#include <math.h>
#include <rc_math.h>
#include <rc_windowhist.h>
#include <Carbon/Carbon.h>


ostream& operator<< (ostream& o, const rcHistoStats& p)
{
    o << "{" << endl << "Mean: " << p.mean() << ",Median  " << p.median() <<
      "Min:  " << p.min() << "Max: " << p.max() << "Bins " << p.bins () << endl;

    uint32 i = 0;
    if (p.n())
      {
	while (i < p.bins ())
	  {
	    uint32 pd (p.histogram()[i]);
	    if (pd)
	      {
		cerr << "[" << i << "]: " << pd << "," << (float) pd / (float) p.n() << endl;
	      }
	    i++;
	  }
      }
    return o;
}


int32 rcHistoStats::inverseCum(int percent) const
{
  if( computedIC_ ) return ic_[percent];
  else return ((rcHistoStats* const)this)->computeInverseCum(percent);
}

rcHistoStats::rcHistoStats()
{
}

rcHistoStats::rcHistoStats( const rcWindow& src, int32 bins)
  : computedMoments_(0), computedIC_(0), computedMode_(0), n_(0),
    mode_(0), mean_(0), sDev_(0), var_(0), ic_(101), bins_(bins)
    
{
  if (src.depth() == rcPixel8)
    {
      rmAssert(src.depth() == rcPixel8);
      histogram_.resize(bins);
      rfGenDepth8Histogram (src, histogram_);
    }
  else if (src.depth() == rcPixel16)
    {
      rmAssert(src.depth() == rcPixel16);
      static uint32 zu (0);
      histogram_.resize (bins, zu);

      uint32 lastRow = src.height() - 1, row = 0;
      const uint32 opsPerLoop = 8;
      uint32 unrollCnt = src.width() / opsPerLoop;
      uint32 unrollRem = src.width() % opsPerLoop;
    
      for ( ; row <= lastRow; row++)
	{
	  const uint16* pixelPtr = (uint16 *) src.rowPointer(row);
                
	  for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
	    {
	      histogram_[ (*pixelPtr)]++; pixelPtr++;
	      histogram_[ (*pixelPtr)]++; pixelPtr++;
	      histogram_[ (*pixelPtr)]++; pixelPtr++;
	      histogram_[ (*pixelPtr)]++; pixelPtr++;
	      histogram_[ (*pixelPtr)]++; pixelPtr++;
	      histogram_[ (*pixelPtr)]++; pixelPtr++;
	      histogram_[ (*pixelPtr)]++; pixelPtr++;
	      histogram_[ (*pixelPtr)]++; pixelPtr++;
	    }
                
	  for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++)
	    {
	      histogram_[ (*pixelPtr)]++; pixelPtr++;
	    }

	}
    
    }


  computeNsamp();
}

void rcHistoStats::integral (vector<uint32>& ahist) const
{
  uint32 tooHiSub = histogram_.size();
  assert (tooHiSub && tooHiSub == ahist.size());

  uint32 sum  = 0;
  for (uint32 i = 0; i < tooHiSub; i++)
    {
      sum += histogram_[i];
      ahist[i] = sum;
    }
}

void rcHistoStats::integralPdf (vector<float>& ph) const
{
  uint32 tooHiSub = histogram_.size();
  assert (tooHiSub && tooHiSub == ph.size());

  uint32 sum  = 0;
  for (uint32 i = 0; i < tooHiSub; i++)
    {
      sum += histogram_[i];
      ph[i] = (float) sum / (float) n();
    }
}

void rcHistoStats::pdf (vector<float>& ph) const
{
  uint32 tooHiSub = histogram_.size();
  assert (tooHiSub && tooHiSub == ph.size());

  for (uint32 i = 0; i < tooHiSub; i++)
    {
      ph[i] = (float)  histogram_[i]/ (float) n();
    }
}

rcHistoStats::rcHistoStats( vector<uint32>& histogram,bool xown)
  : computedMoments_(0), computedIC_(0), computedMode_(0), n_(0),
    mode_(0), mean_(0), sDev_(0), var_(0), ic_(101)
{
  if( xown )  // take ownership
    {
      histogram_.swap(histogram);
      histogram.resize(0);
    }
  else  // make copy
    histogram_ = histogram;

  computeNsamp();
}

/* default copy ctor, assignment, dtor ok */

double rcHistoStats::mean() const
{
  if( computedMoments_ ) return mean_;
  ((rcHistoStats* const)this)->computeMoments();
  return mean_;
}

int32 rcHistoStats::mode() const
{
  if( computedMode_ ) return mode_;

  // compute mode, make valid, return mode_

  long startIndex;
  long lastIndex;
  if( computedIC_ )
  {
    startIndex = ic_[0];
    lastIndex = ic_[100];
  }
  else
  {
    lastIndex = histogram_.size() - 1;
  }

  long i, indexOfMax;
  uint32 maxCountSoFar;
  vector<uint32>::const_iterator pBinI;
  for( i = 1,
       indexOfMax = 0,
       pBinI = histogram_.begin(),
       maxCountSoFar = *pBinI;
       i<=lastIndex;
       i++ )
    if( *++pBinI > maxCountSoFar ) maxCountSoFar = *pBinI, indexOfMax = i;

  ((rcHistoStats* const)this)->computedMode_ = 1;
  return ((rcHistoStats* const)this)->mode_ = indexOfMax;
}

int32 rcHistoStats::median() const
{
  return inverseCum(50);
}

float rcHistoStats::interpolatedMedian() const
{
  float med = (float) histogram_ [inverseCum(50)];
  
  float sum = (float) histogram_ [inverseCum(51)] +
    (float) histogram_ [inverseCum(49)] + med;
  float interp = (((float) inverseCum(51)) * histogram_ [inverseCum(51)]) +
    (((float) inverseCum(49)) * histogram_ [inverseCum(49)]) +
    (((float) inverseCum(50)) * histogram_ [inverseCum(50)]);
  return interp/sum;
}

int32 rcHistoStats::min(const int32 discardPels) const
{
  if (!discardPels)
    return inverseCum(0);
  int32 sum (0);
  long i = 0;
  long endI = histogram_.size();

  if (n_)
    while (i < endI && sum < discardPels)
      {
	sum += histogram_[i++];
      }
  return i;
}

int32 rcHistoStats::max(const int32 discardPels) const
{
  if (!discardPels)
    return inverseCum(100);
  int32 sum (0);
  long i = histogram_.size() - 1;
  if (n_)
    while (i >= 0 && sum < discardPels)
      {
	sum += histogram_[i--];
      }
  return i;
  
}

double rcHistoStats::sDev() const
{
  if( computedMoments_ ) return sDev_;
  ((rcHistoStats* const)this)->computeMoments();
  return sDev_;
}

double rcHistoStats::var() const
{
  if( computedMoments_ ) return var_;
  ((rcHistoStats* const)this)->computeMoments();
  return var_;
}


long rcHistoStats::computeInverseCum(int p)
{
  // fill in ic_

  double one_percent = (double)n_ / 100.;
  double percent;
  uint32 sum = 0;
  long nextIndex = 0;

  long i = 0;
  long endI = histogram_.size();
  long j;

  if (n_)   // if no samples, leave ic_ all zeros
    while( i < endI )
    {
      if( histogram_[i] )
      {
        ic_[100] = i;
        sum += histogram_[i];
        percent = (double)sum / one_percent;

        if (percent > nextIndex)
        {
          for( j=nextIndex; j<percent; j++ ) ic_[j] = i;
          nextIndex = j;
        }
      }
      i++;
    }

  // validate inverseCum...
  computedIC_ = 1;
  return ic_[p];
}

void rcHistoStats::computeMoments()
{
  if(n_)
  {
    long endI = histogram_.size();
    long i;
    uint32 sum = 0;
    double temp;
    
    // Accumulate the sum = SUM( i*hist[i] )
    for( i=0; i<endI; i++ )
      sum += histogram_[i] * i;
    
    // Calculate mean
    mean_ = (double)sum / n_;
      
    // Calculate variance
    if(n_ > 1)
    {
      for( i=0; i<endI; i++ )
        if( histogram_[i] )
        {
          temp = i - mean_;
          var_ += temp * temp * histogram_[i];
        }
      var_ /= (double)(n_-1);
    }
    else
      var_ = 0.0;

    // Calculate sDev
    sDev_ = sqrt(var_);
  }
  else
    mean_ = var_ = sDev_ = 0.0;
  computedMoments_ = 1;
}
        
void rcHistoStats::computeNsamp()
{
    // compute nSamp
    long tooHiSub = histogram_.size();
    long i;
    for(i=0; i<tooHiSub; i++ )
        n_ += histogram_[i];

    for(i = 0; i < 101; i++)
        ic_[i] = 0;
}

double rcHistoStats::entropy () const
{
  double Sum (n_);

  double Entropy = 0.0;

  long tooHiSub = histogram_.size();
  long i;
  for(i=0; i<tooHiSub; i++ )
    {
      const double probability = histogram_[i] / Sum;

      if( probability > 1e-16 )
	{
	  Entropy += - probability * log( probability ) / log( 2.0 );
	}
    }

  return Entropy;
}

void rcHistoStats::equalizationMap (vector<uint8>& map) const
{
  uint32 tooHiSub = histogram_.size();

  uint32 sum  = 0;
  for (uint32 i = 0; i < tooHiSub; i++)
    {
      sum += histogram_[i];
      map [i] = (sum * 255) / n_;
    }
}

void rcHistoStats::stretchMapUs (vector<uint16>& map, const int32 leftDiscard, const int32 rightDiscard) const
{
  int32 minVal = min (leftDiscard);
  int32 maxVal = max (rightDiscard);
  int32 modeVal = mode ();
  int32 val = rmMax ((maxVal - modeVal), (modeVal - minVal));
  rmAssert (val >= 0 & val < bins());
  
  static uint32 zus (0);
  map.resize (bins(), zus);
  uint16 *mPtr = &map[minVal];
  for (uint32 i = minVal; i < maxVal; i++)
    {
      int32 m = rmABS (m - i);
      m = (m * 255) / val;
      *mPtr++ = (uint16) m;
    }
}


