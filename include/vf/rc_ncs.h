
#ifndef _RC_NCS_H_
#define _RC_NCS_H_

#include <vector>
#include <deque>
using namespace std;

#include <rc_types.h>
#include <rc_pair.h>
#include <rc_window.h>
#include <rc_correlationwindow.h>


// Correlation results. For Sum of Absolute differences. Sim is the sum. r is Average intensity difference normalized

// Note that 16bit and 32bit pixel depth correlations are implemented as multiple 8bit correlations. That is a 16bit image
// is correlated as an 8bit image with twice as many pixels

class rcCorr
{
public:
    rcCorr () : mR(0.0), mSi(0), mSm (0), mSii (0), mSmm (0), mSim (0), mN(0), mRp(0), mEim (0), mCosine (0) {}

   // default copy, assignment, dtor ok

  inline double compute ()
    {
        rmAssertDebug( mSi >= 0.0 );
        rmAssertDebug( mSm >= 0.0 );
        
        const double ei = ((mN * mSii) - (mSi * mSi));
        const double em = ((mN * mSmm) - (mSm * mSm));
        const double mEim = ei * em;

        // Modified Normalized Correlation. If both image and model have zero standard deviation and equal moments then
        // they are identical. This case produces 0 in regression based correlation because you can not fit a line 
        // through a single point. However, from image matching point of view it is a perfect match.
        
        mRp = (mEim == 0.) && (mSmm == mSii) && (mSi == mSm);
        
        mCosine = ((mN * mSim) - (mSi * mSm));
        mCosine = mCosine < 0 ? 0 : mCosine;
        mR = (mCosine * mCosine ) / mEim;
        return mR;
    }

    inline double cosine ()
    {
        return acos (mCosine / std::sqrt (mEim));
    }

  inline double relative ()
    {
        rmAssertDebug( mSi >= 0.0 );
        rmAssertDebug( mSm >= 0.0 );
        return std::sqrt (r () );
    }

///// Add partial sums to rcCorr


  inline void accumulate (unsigned int& sim,
		    unsigned int& sii,
		    unsigned int& smm, 
		    unsigned int& si, 
		    unsigned int& sm)
   {
     mSim += sim;
     mSii += sii;
     mSmm += smm;
     mSi += si;
     mSm += sm;
   }

  inline void accumulate (unsigned int& sim,
		    unsigned int& sii,
		    unsigned int& si)
   {
     mSim += sim;
     mSii += sii;
     mSi += si;
   }

  inline void accumulateM (unsigned int& sim,
		    unsigned int& smm,
		    unsigned int& sm)
   {
     mSim += sim;
     mSmm += smm;
     mSm += sm;
   }
   
  inline void accumulate (unsigned int& sim)
   {
     mSim += sim;
   }


  inline void accumulate (double& sim,
		    double& sii,
		    double& smm, 
		    double& si, 
		    double& sm)
   {
     mSim += sim;
     mSii += sii;
     mSmm += smm;
     mSi += si;
     mSm += sm;
   }

  inline void accumulate (double& sim,
		    double& sii,
		    double& si)
   {
     mSim += sim;
     mSii += sii;
     mSi += si;
   }

  inline void accumulateM (double& sim,
		    double& smm,
		    double& sm)
   {
     mSim += sim;
     mSmm += smm;
     mSm += sm;
   }
    
  inline void accumulate (double& sim)
   {
     mSim += sim;
   }


  inline double iStd ()
  {
    double dN (mN);
    return sqrt (((mN * mSii) - (mSi * mSi)) / (dN * (dN - 1.)));
  }

  inline double mStd ()
  {
    double dN (mN);
    return sqrt (((mN * mSmm) - (mSm * mSm)) / (dN * (dN - 1.)));
  }


inline   double r () const { return mR; }

inline   double r (double r)  {return mR = r;}

inline   bool singular () const { return mRp; }

inline   int32 n () const { return mN; }

inline   int32 n (int32 n) {return mN = n;}

inline   double Si () const { return mSi; }

inline   double Si (double Si) {return mSi = Si;}

inline   double Sm () const { return mSm; }

inline   double Sm (double Sm)  {return mSm = Sm;}

inline   double Sii () const { return mSii; }

inline   double Sii (double Sii) {return mSii = Sii;}

inline   double Smm () const { return mSmm; }

inline   double Smm (double Smm)  {return mSmm = Smm;}

inline   double Sim () const { return mSim; }

inline   double Sim (double Sim) {return mSim = Sim;}

inline bool operator ==(const rcCorr& o) const
  {
    return (mR == o.mR && mRp == o.mRp && mN == o.mN && mSi == o.mSi && mSm == o.mSm &&
	    mSii == o.mSii && mSmm == o.mSmm && mSim == o.mSim);
  }

inline bool operator !=(const rcCorr& o) const { return !(this->operator==(o)); }

private:
   double mR, mCosine, mEim;
   double mSi, mSm;
   double mSii, mSmm, mSim;
   int32 mN;
   bool mRp;
};

// Stream output operator
ostream& operator << ( ostream& os, const rcCorr& corr );

enum RC_MatchMethod {
    NormalizedCorr = 0,           //One match position correlation
};

enum RC_EnergyMethod {
    Mean = 0,      // Mean
    SumPlogPNorm = 1    // Shannon's Entropy, fully normalized
};

enum RC_PixelInterp {
    Whole = 0,        // Use the whole 32 bit or 16. Width = number of pixels
    ByteWise = 1      // Use Byte wise. Width = number of bytes
};



struct rsCorrParams
{
  RC_MatchMethod match;
  RC_EnergyMethod em;
  RC_PixelInterp pd;
    int maskN;

    rsCorrParams () : match (NormalizedCorr), em (SumPlogPNorm), pd (ByteWise), maskN (-1) {}

    // default copy, assignment, dtor ok
};

 // Correlation Functions:
// 
void rfCorrelate (const rcWindow& sourceA, const rcWindow& sourceB, const rsCorrParams& params, rcCorr& res);
void rfCorrelate(const rcWindow& sourceA, const rcWindow& sourceB, rcCorr& res);
void rfCorrelate (const rcWindow& sourceA, const rcWindow& sourceB, const rcWindow& mask, const rsCorrParams& params, rcCorr& res, int32 maskN = -1);
void rfCorrelate(const uint8* baseA, const uint8* baseB, uint32 rupA, uint32 rupB, uint32 width, uint32 height, rcCorr& res);

// T is the image pixel storage type, e.g. uint8, uint16, rcUin32 etc.
template <class T>
void rfCorrelateWindow(rcCorrelationWindow<T>& sourceA, rcCorrelationWindow<T>& sourceB, const rsCorrParams& params, rcCorr& res);




#endif // _RC_ANALYSIS_H_
