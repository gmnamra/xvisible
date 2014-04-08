
#ifndef _RC_ANALYSIS_H_
#define _RC_ANALYSIS_H_

#include <vector>
#include <deque>
using namespace std;

#include <rc_types.h>
#include <rc_pair.h>
#include <rc_window.h>
#include <rc_vector2d.h>
#include <rc_correlationwindow.h>
#include <rc_peak.h>
#include <rc_polygon.h>

// A class to encapsulate a table of pre-computed squares
class rcSquareTable {
    enum { RC_SQUARE_TABLE_SIZE = 511 };

public:
    // ctor
    rcSquareTable() {
        for (uint32 i = 0; i < RC_SQUARE_TABLE_SIZE; i++)
            mTable[i] = i * i;
    };
    
    // Array access operator
    uint32 operator [] (int index) const { rmAssertDebug( index < RC_SQUARE_TABLE_SIZE ); return mTable[index]; };

    // Array size
    uint32 size() const { return RC_SQUARE_TABLE_SIZE; };
    
private:
    uint32 mTable[RC_SQUARE_TABLE_SIZE];
};

// For correlation finding and motion vector we report integer correlation values

#define cmIntegerCorr 1000000

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
    RC_NormalizedCorr = 0,           //One match position correlation
    RC_IntFrequencyDist = 1         // 3x3 match space, max - min
};

enum RC_PreProcessMethod {
    RC_PreNone = 0,           //One match position correlation
    RC_PreSharpen = 1,         // Highpass before matching
    RC_PreSmooth = 2          // Lowpass before matching
};

enum RC_EnergyMethod {
    RC_SumSquare = 0,      // Square
    RC_SumPlogP = 1,       // Shannon's Entropy
    RC_SumPlogPNorm = 2    // Shannon's Entropy, fully normalized
};

enum RC_PixelInterp {
    RC_Whole = 0,        // Use the whole 32 bit or 16. Width = number of pixels
    RC_ByteWise = 1      // Use Byte wise. Width = number of bytes
};

enum RC_EnergyReport {
  RC_TimeDerivative = 0,  // Raw Energy is dE / dt
  RC_TimeIntegral = 1   // Intgral of E 
};

enum RC_ColorMethod {
    RC_ColorReduceAll32to8 = 0, // Reduce all 32-bit images to 8-bit gray scale images 
    RC_ColorReduceGray32to8,    // Reduce 32-bit gray scale images to 8-bit gray scale images, analyze all bytes of color images
    RC_ColorUse32               // Analyze all bytes in all 32-bit images
    // TODO: what to do with 8-bit color images?
};

struct rsCorrParams
{
  RC_MatchMethod match;
  RC_PreProcessMethod pp;
  RC_EnergyMethod em;
  RC_PixelInterp pd;
  RC_EnergyReport rp;
  RC_ColorMethod rc;

  rsCorrParams () : match (RC_NormalizedCorr), pp (RC_PreNone), em (RC_SumPlogPNorm), pd (RC_ByteWise), rp (RC_TimeDerivative),
        rc(RC_ColorReduceAll32to8) {}

    // default copy, assignment, dtor ok
};

 // Correlation Functions:
// 
void rfCorrelate (const rcWindow& sourceA, const rcWindow& sourceB, const rsCorrParams& params, rcCorr& res);
void rfCorrelate(const rcWindow& sourceA, const rcWindow& sourceB, rcCorr& res);
void rfCorrelate (const rcWindow& sourceA, const rcWindow& sourceB,
		  const rcWindow& mask, const rsCorrParams& params, rcCorr& res,
		  int32 maskN = -1);
void rfCorrelate(const uint8* baseA, const uint8* baseB, uint32 rupA, uint32 rupB, uint32 width, uint32 height, rcCorr& res);
// T is the image pixel storage type, e.g. uint8, uint16, rcUin32 etc.
template <class T>
void rfCorrelateWindow(rcCorrelationWindow<T>& sourceA, rcCorrelationWindow<T>& sourceB, const rsCorrParams& params, rcCorr& res);

// Return true if analysis finished, false if analysis was aborted
bool rfOptoKineticEnergy (vector<rcWindow>& images, vector<double>& energy, const rsCorrParams& params,
                          rcProgressIndicator* pIndicator = 0 );

void rfOptoKineticVariance (vector <rcWindow>& sequence, rcWindow& varImage);

void rfOptoMoments (vector <rcWindow>& sequence, rcWindow& sumImage, rcWindow& sumSqImage);

//
// Effect: labels connected regions and return the regions in lComponent and their number in region
// Requires: src to have been mapped to 0 and non-zero. lComponents to be a 32 bit image
void rfGetComponents (const rcWindow& src, uint32& regions, rcWindow& lComponents);

// Effect: get bounding rects from a label image
//         Rects smaller than minSize will not be returned.
//         If combineRects is true, rects enclosed by another rect will not be returned
void rfGetRects (const rcWindow& labels, uint32 nLabels, const rcIPair& minSize,
                 bool combineRects, vector<rcIRect>& gRects,
		 vector<uint32>& gRectLabel);

// Effect: builds Polygons from a label image
//         Rects smaller than minSize will not be returned
//         Polygons are merged according to overlap

void rfComponentPolygons(const rcWindow&, const int32, vector<rcPolygon>&, vector<rcIRect>&, bool doMerge = true);


void rfGrassFire (rcWindow& field, rcWindow&feature, rcWindow& distance);
void rfGrassFire(vector<rc2Dvector>& edges, rcWindow &featureImage, rcWindow& distanceImage);

//
// Effect: Intgrates the energy signal.
// Note: The passed signal contains the result 
void rfIntegrateEnergy (vector<double>& energy);

// rfGenLatticeDimensions - Helper fct to rfLatticeCorrelate that
// calculates the correct sizes for the scores and workspace 2D arrays.
//
// Note: Made a public fct to allow users of rfLatticeCorrelate to
// preallocate scores and workspace structures of the correct size.
//
void rfGenLatticeDimensions(const rcRect& geom, const rcIPair& latticeOff, 
			    const rcIPair& tileDim, rcUIPair& scoresDim,
			    rcUIPair& workSpaceDim);

// rfLatticeCorrelate - Generates a "lattice" of correlation results
// by creating tiled windows into winI and winM and correlating IM
// window pairs at the same relative positions. Tiling is controlled
// by the latticeOff and tileDim parameters.
//
// winI and winM are the two images to be carved up into lattices of
// tiles and correlated. latticeOff determines the amount to move the
// origin between consecutive tiles. tileDim define the width/height
// of the tiled windows. workSpaceP allows the caller to provide a
// scratch array of rcCorr structures that is used when an optimized
// version of the score generating is used. See comments for
// genLatticeScoresUsingPartialSums for a description of the
// conditions required.
//
// Note: The returned scores is a vector of vectors of rows of scores.
// In other words, each element of scores will be a vector of
// correlation scores for IM tile pairs whose origins are at the same
// y-offset with respect to winI/winM.
//
// Note: For simplicity's sake, the only matching method currently
// supported is normalized correlation.
//
void rfLatticeCorrelate(rcWindow& winI, rcWindow& winM, 
			const rcIPair& latticeOff, const rcIPair& tileDim,
			vector<vector<double> >& scores,
			vector<vector<rcCorr> >* workSpaceP,
			const rsCorrParams& params);


typedef list<rcPeak<int16> > rcFindList;

void rfImageFind (const rcWindow& moving, const rcWindow& fixed, 
		  rcFindList& locations, rcWindow& space);

#define mQuantz 65000




#endif // _RC_ANALYSIS_H_