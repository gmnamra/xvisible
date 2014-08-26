

#include "image_correlation.h"
#include "row_func.h"
#include "roi_window.h"


#include <vector>
using namespace std;

static const  rsCorrParams sDefaultCorrParams;

namespace
{
    
    static void AndImage(const roi_window& srcWin, const roi_window& maskWin,
                                roi_window& destWin)
    {
        const int32 width = srcWin.width(), height = srcWin.height();
        
        for (int32 y = 0; y < height; y++)
            for (int32 x = 0; x < width; x++)
                destWin.setPixel(x, y, (maskWin.getPixel(x, y) & srcWin.getPixel(x, y)));
    }

}
// Stream output operator
ostream& operator << ( ostream& os, const rcCorr& corr )
{
    os << "R " << corr.r() << " N " <<  corr.n() << " Rp " <<  corr.singular();
    os << " Si " <<  corr.Si() << " Sii " <<  corr.Sii() << " Sm " <<  corr.Sm() << " Smm " <<  corr.Smm() << " Sim " <<  corr.Sim() << endl;
    return os;
}

// Explicit template instantation
// gcc cannot automatically instantiate templates that are not in header files
template void rfCorrelateWindow(roi_window_t<uint16>& sourceA, roi_window_t<uint16>& sourceB, const rsCorrParams& params, rcCorr& res);
template void rfCorrelateWindow(roi_window_t<uint32>& sourceA, roi_window_t<uint32>& sourceB, const rsCorrParams& params, rcCorr& res);
template void rfCorrelateWindow(roi_window_t<float>& sourceA, roi_window_t<float>& sourceB, const rsCorrParams& params, rcCorr& res);

// Pure Byte wise correlation
void rfCorrelate(const uint8* baseA, const uint8* baseB, uint32 rupA, uint32 rupB, uint32 width, uint32 height,
                 rcCorr& res)
{
	basic_corr_RowFunc<uint8> bc (baseA, baseB, rupA, rupB, width, height);
	
	bc.prolog ();
	bc.areaFunc();
	bc.epilog (res);
}

void rfCorrelate(const roi_window& sourceA, const roi_window& sourceB, const rsCorrParams& params, rcCorr& res)
{
	assert(sourceA.depth() == sourceB.depth());
  //  int width_bytes = (params.pd == ByteWise) ? sourceA.width() * get_bytes().count (sourceA.depth()) : sourceA.width();
    rfCorrelate (sourceA.rowPointer(0),sourceB.rowPointer(0),
                 sourceA.rowUpdate(), sourceB.rowUpdate(),
                 sourceA.rowUpdate(),
                 sourceA.height(), 
                 res);
}


// Perform masked version of correlate.
// Only pixel locations that are set in the corresponding masked image are
// used in the calculation.
// All images must be the same size and depth and pixels in the mask image
// must all be either 0 or all ones.
void rfCorrelate(const roi_window& img, const roi_window& mdl,
                 const roi_window& mask, const rsCorrParams& params, rcCorr& res,
                 int32 maskN)
{
    roi_window iMasked(img.width(), img.height(), img.depth());
    roi_window mMasked(img.width(), img.height(), img.depth());
	
    AndImage(img, mask, iMasked);
    AndImage(mdl, mask, mMasked);
    rfCorrelate(iMasked, mMasked, params, res);
	
    if (maskN < 0) {
        uint32 allOnes = 0xFF;
        if (img.depth() == rpixel16)
            allOnes = 0xFFFF;
        else if (img.depth() == rpixel32)
            allOnes = 0xFFFFFFFF;
        else
            assert(img.depth() == rpixel8);
		
        maskN = 0;
        for (int32 y = 0; y < mask.height(); y++)
            for (int32 x = 0; x < mask.width(); x++) {
				uint32 val = mask.getPixel(x, y);
				if (val == allOnes)
					maskN++;
				else
					assert(val == 0);
            }
    }
    res.n(maskN);
    res.compute();
}


//Correlate two same depth roi_windows -- convenient function using default settings
void rfCorrelate(const roi_window& sourceA, const roi_window& sourceB, rcCorr& res)
{
    rfCorrelate(sourceA, sourceB, sDefaultCorrParams, res);
}


// Specialization for CorrelationWindow 

// Specialization for 8-bit depth
template<>
void rfCorrelateWindow(roi_window_t<uint8>& sourceA, roi_window_t<uint8>& sourceB, const rsCorrParams& params, rcCorr& res)
{
    UnusedParameter ( params );
	assert(sourceA.depth() == sourceB.depth());
	
    basic_corr_RowFunc<uint8> bc( sourceA, sourceB );
    
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
void rfCorrelateWindow(roi_window_t<T>& sourceA, roi_window_t<T>& sourceB, const rsCorrParams& params, rcCorr& res)
{
	assert(sourceA.depth() == sourceB.depth());
    assert(params.pd == ByteWise);
    
    // Bytewise correlation
    basic_corr_RowFunc<uint8> bc( sourceA.rowPointer(0), sourceB.rowPointer(0),
                                 sourceA.rowUpdate(), sourceB.rowUpdate(),
                                 sourceA.width() * sourceA.depth(), 
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

