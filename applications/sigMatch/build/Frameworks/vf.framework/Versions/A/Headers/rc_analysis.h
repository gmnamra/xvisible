
/*
 * 	$Id: rc_analysis.h 7297 2011-03-07 00:17:55Z arman $     
 *
 *      Copyright (c) 2002 Reify Corp. All rights reserved.
 *      $Log$
 *      Revision 1.65  2005/11/15 23:26:04  arman
 *      motionpath redesign
 *
 *      Revision 1.64  2005/11/04 22:04:24  arman
 *      cell lineage iv
 *
 *      Revision 1.63  2005/08/30 21:08:50  arman
 *      Cell Lineage
 *
 *      Revision 1.63  2005/08/01 01:47:25  arman
 *      cell lineage addition
 *
 *      Revision 1.62  2005/07/01 21:05:39  arman
 *      version2.0p
 *
 *      Revision 1.62  2005/06/28 23:24:49  arman
 *      minor changes to peak print
 *
 *      Revision 1.61  2005/04/23 17:46:49  arman
 *      added more support for rfImageFind
 *
 *      Revision 1.60  2005/04/04 14:17:47  arman
 *      added a 2nd form of ctor for rcWindow as space.
 *
 *      Revision 1.59  2005/04/01 18:12:44  arman
 *      added changes to rfImageFind
 *
 *      Revision 1.58  2005/03/31 23:30:15  arman
 *      updated rfImageFind
 *
 *      Revision 1.57  2005/03/10 20:32:13  arman
 *      imageFind returns multiple peaks
 *
 *      Revision 1.56  2005/03/04 16:24:50  arman
 *      added new imageFid definition
 *
 *      Revision 1.55  2005/03/03 18:52:24  arman
 *      added rcPointScan for hillclimbing towards correlation peaks
 *
 *      Revision 1.54  2005/02/03 14:51:13  arman
 *      flu morphometry
 *
 *      Revision 1.53  2004/08/13 20:53:05  arman
 *      removed rf1DPeak
 *
 *      Revision 1.52  2004/03/15 20:04:00  arman
 *      added relative correlation
 *
 *      Revision 1.51  2004/02/22 17:57:21  arman
 *      updated declaration for getPoygons
 *
 *      Revision 1.50  2004/02/04 10:34:34  arman
 *      added rfComponentPolygons
 *
 *      Revision 1.49  2004/01/29 19:53:47  proberts
 *      Speed up of muscle cell segmentation - renamed rcMCT to rcMuscleSegmenter Fixed rcPolygon bug
 *
 *      Revision 1.48  2004/01/14 20:23:44  arman
 *      added another overload for grass fire
 *
 *      Revision 1.47  2003/12/30 23:13:21  proberts
 *      add masked operation support
 *
 *      Revision 1.46  2003/11/18 20:56:47  proberts
 *      Fix rfGetRLEs to generate only a single blobs runs in each RLE
 *
 *      Revision 1.45  2003/06/13 20:54:46  sami
 *      Addeed bool arg to rfGetRects() and rfGetRLES()
 *
 *      Revision 1.44  2003/06/10 14:55:06  sami
 *      RLE vectorization changes
 *
 *      Revision 1.43  2003/06/04 14:29:43  proberts
 *      Split out moments related definitions from rc_analysis.h
 *
 *      Revision 1.42  2003/06/02 22:24:53  sami
 *      Removed gRects argument from rfGetRLEs(). rcRleWindow::rectangle() method
 *      can be used to access RLE bounding rect.
 *
 *      Revision 1.41  2003/06/02 21:06:58  sami
 *      Added rfGetRects()
 *
 *      Revision 1.40  2003/05/30 22:50:02  proberts
 *      Update moments generating code
 *
 *      Revision 1.39  2003/05/30 20:02:25  sami
 *      Added rfGetRLEs
 *
 *      Revision 1.38  2003/05/18 19:40:37  proberts
 *      moment generating code
 *
 *      Revision 1.37  2003/05/13 02:41:52  arman
 *      Correct standard deviation accessors
 *
 *      Revision 1.36  2003/05/12 21:28:20  arman
 *      added methods for standard deviations
 *
 *      Revision 1.35  2003/05/11 18:11:31  proberts
 *      add new integral moments generating fcts. chg some ip unit tests to use 1280x960 imgs
 *
 *      Revision 1.34  2003/04/18 17:57:27  sami
 *      Removed rcAnalysisProgressIndicator
 *
 *      Revision 1.33  2003/04/03 22:56:13  sami
 *      Correlation sum caching support added
 *
 *      Revision 1.32  2003/04/01 12:28:01  arman
 *      added new accumulate functions
 *
 *      Revision 1.31  2003/03/29 17:47:30  arman
 *      removed (old) tile correlation and sad correlation
 *
 *      Revision 1.30  2003/03/17 15:10:29  arman
 *      added accumulate function
 *
 *      Revision 1.29  2003/03/15 14:20:20  arman
 *      added direct api to correlation
 *
 *      Revision 1.28  2003/03/11 00:11:46  sami
 *      const correctness improved
 *
 *      Revision 1.27  2003/02/23 22:28:26  arman
 *      Removed medialPost
 *
 *      Revision 1.26  2003/02/18 21:13:16  arman
 *      added rfOptoKineticVariance
 *      Removed rfTemporalGradient
 *
 */

#ifndef _RC_ANALYSIS_H_
#define _RC_ANALYSIS_H_

#include <vector>
#include <deque>
using namespace std;

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
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
    rcCorr () : mR(0.0), mSi(0), mSm (0), mSii (0), mSmm (0), mSim (0), mN(0), mRp(0) {}

   // default copy, assignment, dtor ok

  inline double compute ()
    {
        rmAssertDebug( mSi >= 0.0 );
        rmAssertDebug( mSm >= 0.0 );
        
        const double ei = ((mN * mSii) - (mSi * mSi));
        const double em = ((mN * mSmm) - (mSm * mSm));
        const double ex = ei * em;

        // Modified Normalized Correlation. If both image and model have zero standard deviation and equal moments then
        // they are identical. This case produces 0 in regression based correlation because you can not fit a line 
        // through a single point. However, from image matching point of view it is a perfect match.
        
        mRp = (ex == 0.) && (mSmm == mSii) && (mSi == mSm);
        
        if (ex != 0.) {
            const double cross = ((mN * mSim) - (mSi * mSm));
            mR = (cross * cross) / ex;
        }
        else
            mR = 0.;
        
        return mR;
    }

  inline double relative ()
    {
        rmAssertDebug( mSi >= 0.0 );
        rmAssertDebug( mSm >= 0.0 );
        
	double mstd = mStd ();
        
        if (mstd != 0.)
	  {
            const double cross = ((mN * mSim) - (mSi * mSm));
            mstd = (cross * cross) / (mN * mstd);
	  }

        return mstd;
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
   double mR;
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

void rf1Dfft(vector<float>& signal, vector<float>& imaginary, int32 dir);
void rf1Ddft(vector<float>& signal, vector<float>& imaginary, int32 dir);

typedef struct rsAnalysisVoxels {
  uint32 weightsSz;
  float* weights;
  uint32 workSigSz;
  float* workSig;
  char* workSigBase;
} rsAnalysisVoxels;

typedef struct rs1DFourierResult {
  vector<float> real;
  vector<float> imag;
  vector<float> phase;
  vector<float> amplitude;
  float peakFrequency;
  float peakPhase;
  float peakAmplitude;
  int32 workingSz;
} rs1DFourierResult;

/* rf1DFourierAnalysis - Takes the 1D input signal and performs a
 * fourier analysis of the data, leaving all the results in rslt. The
 * real and imag vectors in the result structure are both resized to
 * the size of the input signal. The phase and amplitude vectors only
 * contain the entries for frequencies that are within the nyqist
 * limit (approximately half the size of the input signal).
 *
 * In addition, the result amplitudes are searched. The location of
 * the frequency at which the peak amplitude occurred is returned as
 * the peak frequency. A parabolic fit is done to find the final vaues
 * for peak frequency and peak amplitude. The peak phase is computed
 * by computing a linear fit of the two phase angles closest to the
 * computed peak frequency.
 *
 * The bits in the control word works as follows:
 *
 *   rc1DFourierForceFFT - If set, the input signal will be rounded up
 *   in size to a power of 2. This "rounding" is done by adding zeros
 *   to the end of the signal. If rc1DFourierZeroFill is also set, the
 *   input signal will be rounded up to the nearest power of 2 plus 1,
 *   otherwise it will be rounded up to the nearest power of 2.
 *
 *   rc1DFourierZeroFill - If set, zero padding is added to the end of
 *   the signal. If rc1DFourierForceFFT is also set, then zero filling
 *   is as described above, otherwise signal size number of zeros are
 *   added.
 *
 *   rc1DFourierRaisedCosine - If set, a Hanning filter is applied to
 *   the input signal.
 *
 * Note: If the modified signal (with any zero filling added) has a
 * size that is a power of 2, then an FFT is performed, otherwise a
 * DFT is performed.
 *
 * Note: In all cases, the DC component is subtracted from the input
 * signal before FFT processing is done.
 *
 * Note: Choosing to support an input signal that is a deque of
 * doubles was chosen for compatibility with the rcSimilarity's output
 * signal.
 */
enum rc1DFourierControl {
  rc1DFourierForceFFT = 1,
  rc1DFourierZeroFill = 2,
  rc1DFourierRaisedCosine = 4,
  rc1DFourierGenPhase = 8
};

void rf1DFourierAnalysis(const deque<double>& signal,
			 rs1DFourierResult& rslt,
			 uint32 ctrl);

void rf1DFourierAnalysisVoxels(const uint8* sig, const uint32 sigSz,
			       rsAnalysisVoxels& analysisInfo,
			       rs1DFourierResult& rslt, const uint32 ctrl);

typedef list<rcPeak<int16> > rcFindList;

void rfImageFind (const rcWindow& moving, const rcWindow& fixed, 
		  rcFindList& locations, rcWindow& space);

#define mQuantz 65000

class rcPointScan
{
private:
  static const uint16 mNotProcessed = (uint16) mQuantz+1;

 public:
  rcPointScan (const rcWindow& moving, const rcWindow& fixed, rcIPair& peak, 
	       vector< vector<float> >& cspace, int32 maxTravel = 4);
  rcPointScan (const rcWindow& moving, const rcWindow& fixed, rcIPair& peak, 
	       rcWindow& space, int32 maxTravel = 4);

  /*
   * @class rcPointScan
   * fixed was found at edge (peak) of correlation space (cspace) 
   * follow this peak once per scan call and if it is a peak (4 way), 
   * interpolate and return frameBuf position + interpolate position
   */

  bool setMask (const rcWindow& mask);

  bool peakAndInterpolate (rc2Fvector&);
  bool scan ();
  float maxScore () { return mMaxScore; }
  int16 imaxScore () { return (uint16) (mMaxScore * 1000); }


  void dump ()
  {
    cerr << mTrace << endl;
  }

 private:
  rcIPair mMovingPeak;
  rcIPair mFrameStart;
  rcIPair mFramePeak;
  rcIRect mFrame;
  float mMaxScore;
  float mLeft, mRight, mCenter, mTop, mBottom;
  rcIPair mMad;
  const rcWindow& mMoving;
  const rcWindow& mFixed;
  rcWindow mTrace;
  rcWindow mMask;
  bool mMaskValid;
  int32 mMaskN;
  rsCorrParams mCorrParams;

  bool checkOffset (rcIPair& point)
  {
    return mFrame.contains (point);
  }

  void clear ()
  {
    mCenter = mRight = mLeft = mTop = mBottom = mNotProcessed;
    if (mTrace.isBound())
      mTrace.setAllPixels (uint16 (mNotProcessed));      
  }
};

template <class T>
void rfPrintCorrSpace (rcWindow& space, T precision)
{
  int32 i;
  cerr << ">>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
  for (int32 j = 0; j < space.height(); j++)
    {
      for (i = 0; i < space.width() - 1; i++)
	cerr << setw(3) << (*((T *) (space.pelPointer(i, j)))) * precision  << "-";
      cerr << setw(3) << (*((T *) (space.pelPointer(i, j)))) * precision  << endl;
    }
  cerr << "<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
}  


void rfPrintFindList (const rcFindList& l, ostream& os);

void rfImageACF (const rcWindow& moving, rcIPair& range, rcWindow& space);

#endif // _RC_ANALYSIS_H_
