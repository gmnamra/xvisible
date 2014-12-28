

#include "rc_ncs.h"
#include "rc_rowfunc.h"
#include "rc_correlationwindow.h"
#include "rc_imageprocessing.h"

#include <vector>
using namespace std;

static const  rsCorrParams sDefaultCorrParams;


// Stream output operator
ostream& operator << ( ostream& os, const rcCorr& corr )
{
    os << "R " << corr.r() << " N " <<  corr.n() << " Rp " <<  corr.singular();
    os << " Si " <<  corr.Si() << " Sii " <<  corr.Sii() << " Sm " <<  corr.Sm() << " Smm " <<  corr.Smm() << " Sim " <<  corr.Sim() << endl;
    return os;
}

// Explicit template instantation
// gcc cannot automatically instantiate templates that are not in header files
template void rfCorrelateWindow(rcCorrelationWindow<uint16>& sourceA, rcCorrelationWindow<uint16>& sourceB, const rsCorrParams& params, rcCorr& res);
template void rfCorrelateWindow(rcCorrelationWindow<uint32>& sourceA, rcCorrelationWindow<uint32>& sourceB, const rsCorrParams& params, rcCorr& res);
template void rfCorrelateWindow(rcCorrelationWindow<float>& sourceA, rcCorrelationWindow<float>& sourceB, const rsCorrParams& params, rcCorr& res);

// Pure Byte wise correlation
void rfCorrelate(const uint8* baseA, const uint8* baseB, uint32 rupA, uint32 rupB, uint32 width, uint32 height,
                 rcCorr& res)
{
	rcBasicCorrRowFunc<uint8> bc (baseA, baseB, rupA, rupB, width, height);
	
	bc.prolog ();
	bc.areaFunc();
	bc.epilog (res);
}

void rfCorrelate(const rcWindow& sourceA, const rcWindow& sourceB, const rsCorrParams& params, rcCorr& res)
{
	rmAssert (sourceA.depth() == sourceB.depth());
    int width_bytes = (params.pd == ByteWise) ? sourceA.width() * get_bytes().count (sourceA.depth()) : sourceA.width();
    rfCorrelate (sourceA.rowPointer(0),sourceB.rowPointer(0),
                 sourceA.rowUpdate(), sourceB.rowUpdate(),
                 width_bytes, 
                 sourceA.height(), 
                 res);
}


// Perform masked version of correlate.
// Only pixel locations that are set in the corresponding masked image are
// used in the calculation.
// All images must be the same size and depth and pixels in the mask image
// must all be either 0 or all ones.
void rfCorrelate(const rcWindow& img, const rcWindow& mdl,
                 const rcWindow& mask, const rsCorrParams& params, rcCorr& res,
                 int32 maskN)
{
    rcWindow iMasked(img.width(), img.height(), img.depth());
    rcWindow mMasked(img.width(), img.height(), img.depth());
	
    rfAndImage(img, mask, iMasked);
    rfAndImage(mdl, mask, mMasked);
    rfCorrelate(iMasked, mMasked, params, res);
	
    if (maskN < 0) {
        uint32 allOnes = 0xFF;
        if (img.depth() == rcPixel16)
            allOnes = 0xFFFF;
        else if (img.depth() == rcPixel32S)
            allOnes = 0xFFFFFFFF;
        else
            rmAssert(img.depth() == rcPixel8);
		
        maskN = 0;
        for (int32 y = 0; y < mask.height(); y++)
            for (int32 x = 0; x < mask.width(); x++) {
				uint32 val = mask.getPixel(x, y);
				if (val == allOnes)
					maskN++;
				else
					rmAssert(val == 0);
            }
    }
    res.n(maskN);
    res.compute();
}


//Correlate two same depth rcWindows -- convenient function using default settings
void rfCorrelate(const rcWindow& sourceA, const rcWindow& sourceB, rcCorr& res)
{
    rfCorrelate(sourceA, sourceB, sDefaultCorrParams, res);
}


// Specialization for CorrelationWindow 

// Specialization for 8-bit depth
template<>
void rfCorrelateWindow(rcCorrelationWindow<uint8>& sourceA, rcCorrelationWindow<uint8>& sourceB, const rsCorrParams& params, rcCorr& res)
{
	rmUnused( params );
	rmAssert (sourceA.depth() == sourceB.depth());
	
    rcBasicCorrRowFunc<uint8> bc( sourceA, sourceB );
    
    bc.prolog ();
    bc.areaFunc();
    bc.epilog (res);
	
	// Cache sums
	if ( !sourceA.sumValid() ) {
		sourceA.sum( res.Si() );
		sourceA.sumSquares( res.Sii() );
	}
	if ( !sourceB.sumValid() ) {
		sourceB.sum( res.Sm() );
		sourceB.sumSquares( res.Smm() );
	}
}

// For depths other than 8-bit
template <class T>
void rfCorrelateWindow(rcCorrelationWindow<T>& sourceA, rcCorrelationWindow<T>& sourceB, const rsCorrParams& params, rcCorr& res)
{
	rmAssert (sourceA.depth() == sourceB.depth());
    rmAssert (params.pd == ByteWise);
    
    // Bytewise correlation
    rcBasicCorrRowFunc<uint8> bc( sourceA.rowPointer(0), sourceB.rowPointer(0),
                                 sourceA.rowUpdate(), sourceB.rowUpdate(),
                                 sourceA.width() * get_bytes().count (sourceA.depth()), 
                                 sourceA.height() );
    bc.prolog ();
    bc.areaFunc();
    bc.epilog (res);
  	
	// Cache sums
	if ( !sourceA.sumValid() ) {
		sourceA.sum( res.Si() );
		sourceA.sumSquares( res.Sii() );
	}
	if ( !sourceB.sumValid() ) {
		sourceB.sum( res.Sm() );
		sourceB.sumSquares( res.Smm() );
	}
}




