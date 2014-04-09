
/*
 *  pmcorr.cpp
 *  correlation
 *  $Log$
 *  Revision 1.27  2005/11/15 23:26:05  arman
 *  motionpath redesign
 *
 *  Revision 1.26  2005/11/04 22:04:25  arman
 *  cell lineage iv
 *
 *  Revision 1.25  2003/12/30 23:13:21  proberts
 *  add masked operation support
 *
 *  Revision 1.24  2003/04/12 01:47:05  arman
 *  moderate pipelining in correlation innerloops
 *
 *  Revision 1.23  2003/04/03 22:56:14  sami
 *  Correlation sum caching support added
 *
 *  Revision 1.22  2003/04/02 22:20:59  sami
 *  Removed redundant res.compute() calls
 *
 *  Revision 1.21  2003/04/01 02:50:11  arman
 *  re-implemented correlations on top of rowfuncs
 * 
 *  $Id: rc_pmcorr.cpp 7297 2011-03-07 00:17:55Z arman $
 *  Created by Arman Garakani on Tue Jun 04 2002.
 *  Copyright (c) 2002 Reify Corp. . All rights reserved.
 *
 */

#include <rc_analysis.h>

#include <rc_rowfunc.h>
#include <rc_correlationwindow.h>
#include <rc_imageprocessing.h>

#include <vector>
using namespace std;

static const  rsCorrParams sDefaultCorrParams;


// TODO: all this globally mutable data should be object instance state instead
#ifdef __ppc__


static void bunaligned_productsUS ( const unsigned short *input1, const unsigned short *input2, rcCorr& res, unsigned int m8, 
																	 vector unsigned char, vector unsigned char);
static void baligned_productsUS ( const unsigned short *input1, const unsigned short *input2, rcCorr& res, unsigned int m8);

static void bunaligned_products ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
																 vector unsigned char, vector unsigned char);
static void baligned_products ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16);
static void bunaligned_products_sumI ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
																			vector unsigned char, vector unsigned char);
static void baligned_products_sumI ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16);
static void bunaligned_products_sumM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
																			vector unsigned char, vector unsigned char);
static void baligned_products_sumM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16);
static void bunaligned_products_sumIM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
																			 vector unsigned char, vector unsigned char);


static void baligned_products_sumIM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16);

#endif

static void correlate16a32 (const rcWindow& sourceA, const rcWindow& sourceB, rcCorr& res);


// Explicit template instantation
// gcc cannot automatically instantiate templates that are not in header files
template void rfCorrelateWindow(rcCorrelationWindow<uint16>& sourceA, rcCorrelationWindow<uint16>& sourceB, const rsCorrParams& params, rcCorr& res);
template void rfCorrelateWindow(rcCorrelationWindow<uint32>& sourceA, rcCorrelationWindow<uint32>& sourceB, const rsCorrParams& params, rcCorr& res);
template void rfCorrelateWindow(rcCorrelationWindow<float>& sourceA, rcCorrelationWindow<float>& sourceB, const rsCorrParams& params, rcCorr& res);

#ifdef __ppc__
void altiVecCorrelate(const uint8* baseA, const uint8* baseB, uint32 rowbytesA, uint32 rowbytesB, 
                      uint32 width, uint32 height, rcCorr& res)
{
	rcAltiVecCorrRowFunc<uint8> avc (baseA, baseB, rowbytesA, rowbytesB, width, height);
	
	avc.prolog ();
	avc.areaFunc();
	avc.epilog (res);
}

void altiVecCorrelate(const rcWindow& sourceA, const rcWindow& sourceB, rcCorr& res)
{
	altiVecCorrelate (sourceA.rowPointer(0), sourceB.rowPointer(0), sourceA.rowUpdate(), sourceB.rowUpdate(),
										sourceA.width(), sourceA.height(), res);
}
#endif


static void correlate16a32 (const rcWindow& sourceA, const rcWindow& sourceB, rcCorr& res)
{
	// rcBasicCorrRowFunc ctor checks the depth so we do not repeat it
	if (sourceA.depth() == rcPixel32)
	{
		uint32 dummy;
		rcBasicCorrRowFunc<uint32> bc (sourceA, sourceB, dummy);
		
		bc.prolog ();
		bc.areaFunc();
		bc.epilog (res);
	}
	else if (sourceA.depth() == rcPixel16)
	{
		uint16 dummy;
		rcBasicCorrRowFunc<uint16> bc (sourceA, sourceB, dummy);
		
		bc.prolog ();
		bc.areaFunc();
		bc.epilog (res);
	}
}


void rfCorrelate(const uint8* baseA, const uint8* baseB, uint32 rupA, uint32 rupB, uint32 width, uint32 height,
                 rcCorr& res)
{
#ifdef __ppc__	
	if (rfHasSIMD ())
	{
		// Use AltiVec for processing
		altiVecCorrelate (baseA, baseB, rupA, rupB, width, height, res);
	} 
	else
#endif
	{
		rcBasicCorrRowFunc<uint8> bc (baseA, baseB, rupA, rupB, width, height);
		
		bc.prolog ();
		bc.areaFunc();
		bc.epilog (res);
	}
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
    else if (img.depth() == rcPixel32)
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


// Correlate two sources. We can not cache sums since we do not have a model representation
// Use Square table technique [reference Moravoc paper in the 80s]

void rfCorrelate(const rcWindow& sourceA, const rcWindow& sourceB, const rsCorrParams& params, rcCorr& res)
{
	rmAssert (sourceA.depth() == sourceB.depth());
		rfCorrelate (sourceA.rowPointer(0),sourceB.rowPointer(0),
								 sourceA.rowUpdate(), sourceB.rowUpdate(),
								 sourceA.width() * sourceA.depth(), 
								 sourceA.height(), 
								 res);
}

void rfCorrelate(const rcWindow& sourceA, const rcWindow& sourceB, rcCorr& res)
{
  rfCorrelate(sourceA, sourceB, sDefaultCorrParams, res);
}

// Specialization for 8-bit depth
template<>
void rfCorrelateWindow(rcCorrelationWindow<uint8>& sourceA, rcCorrelationWindow<uint8>& sourceB, const rsCorrParams& params, rcCorr& res)
{
	rmUnused( params );
	rmAssert (sourceA.depth() == sourceB.depth());
#ifdef __ppc__
	
	if (rfHasSIMD ()) {
		// Altivec enabled
		rcAltiVecCorrRowFunc<uint8> bc( sourceA, sourceB );
		
		bc.prolog ();
		bc.areaFunc();
		bc.epilog (res);
	} else
		
#endif
	{
		// Altivec disabled
		rcBasicCorrRowFunc<uint8> bc( sourceA, sourceB );
		
		bc.prolog ();
		bc.areaFunc();
		bc.epilog (res);
	} 
	
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
	
		// Bytewise correlation
		// Note: cannot construct rcCorrelationWindow because of window depth difference
		rcBasicCorrRowFunc<uint8> bc( sourceA.rowPointer(0), sourceB.rowPointer(0),
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

#ifdef __ppc__

// Normalized Correlation Products
// Note: The following code is written for readability and efficiency. 
//       Code duplication is worth paying for critical loops.
//       6 functions cover:   aligned              unaligned
//                            no sum cached        no sum cached
//                            one sum cached       one sum cached
//                            both sum cached      both sum cached

void altivec8bitPelProducts ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16,
														 vector unsigned char perm1, vector unsigned char perm2)
{
	if (!((uint32) input1 & 0xf) && !((uint32) input2 & 0xf))
	{
		baligned_products (input1, input2,  res, m16);
	}
	else
	{
		bunaligned_products (input1, input2,  res, m16, perm1, perm2);
	}
}

void altivec16bitPelProducts ( const unsigned short *input1, const unsigned short *input2, rcCorr& res, unsigned int m8,
                              vector unsigned char perm1, vector unsigned char perm2)
{
	if (!((uint32) input1 & 0xf) && !((uint32) input2 & 0xf))
	{
		baligned_productsUS (input1, input2,  res, m8);
	}
	else
	{
		bunaligned_productsUS (input1, input2,  res, m8, perm1, perm2);
	}
}


void altivec8bitPelProductsSumI ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
																 vector unsigned char perm1, vector unsigned char perm2)
{
	if (!((uint32) input1 & 0xf) && !((uint32) input2 & 0xf))
	{
		baligned_products_sumI (input1, input2,  res, m16);
	}
	else
	{
		bunaligned_products_sumI (input1, input2,  res, m16, perm1, perm2);
	}
}

void altivec8bitPelProductsSumM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
																 vector unsigned char perm1, vector unsigned char perm2)
{
	if (!((uint32) input1 & 0xf) && !((uint32) input2 & 0xf))
	{
		baligned_products_sumM (input1, input2,  res, m16);
	}
	else
	{
		bunaligned_products_sumM (input1, input2,  res, m16, perm1, perm2);
	}
}

void altivec8bitPelProductsSumIM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
																	vector unsigned char perm1, vector unsigned char perm2)
{
	if (!((uint32) input1 & 0xf) && !((uint32) input2 & 0xf))
	{
		baligned_products_sumIM (input1, input2,  res, m16);
	}
	else
	{
		bunaligned_products_sumIM (input1, input2,  res, m16, perm1, perm2);
	}
}


static void baligned_products_sumIM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16)
{
	vector unsigned char *current, *previous;
	vector unsigned char t1, t2;
	vector unsigned int zero, mom01, mom02, sumim;
	vector unsigned int cross, mom11, mom22;
	unsigned int result, sii, smm, si, sm;
	
	
	current = (vector unsigned char *) input1;
	previous = (vector unsigned char *) input2;
	
	// Sum IM
	/* initialization */
	INITCOMMON;
	INITMOM1;
	INITMOM2;
	
	// m16 is 16 multiples. Subtrack 1 to match numbering of product loop
	assert (m16);
	m16 -= 1;
	
	// Prime the pipe
	t1  = current[0];                /* align current vector  */     
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	ALCROSSSUMIM(0);
	ALCROSSSUMIM(1);
	ALCROSSSUMIM(2);
	ALCROSSSUMIM(3);
	ALCROSSSUMIM(4);
	ALCROSSSUMIM(5);
	ALCROSSSUMIM(6);
	ALCROSSSUMIM(7);
	ALCROSSSUMIM(8);
	ALCROSSSUMIM(9);
	ALCROSSSUMIM(10);
	ALCROSSSUMIM(11);
	ALCROSSSUMIM(12);
	ALCROSSSUMIM(13);
	ALCROSSSUMIM(14);
	ALCROSSSUMIM(15);
	
accum:
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = (vector unsigned int) (vec_sums((vector signed int) cross, (vector signed int) zero));
	ACUMMOM1;
	ACUMMOM2;
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	REPLMOM1;
	REPLMOM2;
	
	// Store the result in vector at an address
	vec_ste( sumim, 0, &result );
	STRMOM1;
	STRMOM2;
	
	res.accumulate (result,sii,smm,si,sm);
	
}


static void baligned_products ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16)
{
	vector unsigned char *current, *previous;
	vector unsigned char t1, t2;
	vector unsigned int zero, sumim;
	vector unsigned int cross;
	unsigned int result;
	
	
	current = (vector unsigned char *) input1;
	previous = (vector unsigned char *) input2;
	
	// Sum
	/* initialization */
	INITCOMMON;
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	
	switch (m16)
	{
		case 16:
			ALCROSS;
		case 15:
			ALCROSS;
		case 14:
			ALCROSS;
		case 13:
			ALCROSS;
		case 12:
			ALCROSS;
		case 11:
			ALCROSS;
		case 10:
			ALCROSS;
		case 9:
			ALCROSS;
		case 8:
			ALCROSS;
		case 7:
			ALCROSS;
		case 6:
			ALCROSS;
		case 5:
			ALCROSS;
		case 4:
			ALCROSS;
		case 3:
			ALCROSS;
		case 2:
			ALCROSS;
		case 1:
			ALCROSS;
			break;
	}
	
	
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = (vector unsigned int) (vec_sums((vector signed int) cross, (vector signed int) zero));
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	
	// Store the result in vector at an address
	vec_ste( sumim, 0, &result );
	
	res.accumulate (result);
	
}




static void baligned_products_sumI ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16)
{
	vector unsigned char *current, *previous;
	vector unsigned char t1, t2;
	vector unsigned int zero, sumim;
	vector unsigned int mom01, mom11, cross;
	unsigned int result, sii, si;
	
	
	current = (vector unsigned char *) input1;
	previous = (vector unsigned char *) input2;
	
	// Sum I
	/* initialization */
	INITCOMMON;
	INITMOM1;
	
	// m16 is 16 multiples. Subtrack 1 to match numbering of product loop
	assert (m16);
	m16 -= 1;
	
	// Prime the pipe
	t1  = current[0];                /* align current vector  */     
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	ALCROSSSUMI(0);
	ALCROSSSUMI(1);
	ALCROSSSUMI(2);
	ALCROSSSUMI(3);
	ALCROSSSUMI(4);
	ALCROSSSUMI(5);
	ALCROSSSUMI(6);
	ALCROSSSUMI(7);
	ALCROSSSUMI(8);
	ALCROSSSUMI(9);
	ALCROSSSUMI(10);
	ALCROSSSUMI(11);
	ALCROSSSUMI(12);
	ALCROSSSUMI(13);
	ALCROSSSUMI(14);
	ALCROSSSUMI(15);
	
accum:
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = (vector unsigned int) (vec_sums((vector signed int) cross, (vector signed int) zero));
	ACUMMOM1;
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	REPLMOM1;
	
	// Store the result in vector at an address
	vec_ste( sumim, 0, &result );
	STRMOM1;
	
	res.accumulate (result,sii,si);
	
}

static void baligned_products_sumM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16)
{
	vector unsigned char *current, *previous;
	vector unsigned char t1, t2;
	vector unsigned int zero, sumim;
	vector unsigned int mom02, mom22, cross;
	unsigned int result, smm, sm;
	
	
	current = (vector unsigned char *) input1;
	previous = (vector unsigned char *) input2;
	
	// Sum M
	/* initialization */
	INITCOMMON;
	INITMOM2;
	
	// m16 is 16 multiples. Subtrack 1 to match numbering of product loop
	assert (m16);
	m16 -= 1;
	
	// Prime the pipe
	t2  = previous[0];                /* align current vector  */     
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	ALCROSSSUMM(0);
	ALCROSSSUMM(1);
	ALCROSSSUMM(2);
	ALCROSSSUMM(3);
	ALCROSSSUMM(4);
	ALCROSSSUMM(5);
	ALCROSSSUMM(6);
	ALCROSSSUMM(7);
	ALCROSSSUMM(8);
	ALCROSSSUMM(9);
	ALCROSSSUMM(10);
	ALCROSSSUMM(11);
	ALCROSSSUMM(12);
	ALCROSSSUMM(13);
	ALCROSSSUMM(14);
	ALCROSSSUMM(15);
	
accum:
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = (vector unsigned int) (vec_sums((vector signed int) cross, (vector signed int) zero));
	ACUMMOM2;
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	REPLMOM2;
	
	// Store the result in vector at an address
	vec_ste( sumim, 0, &result );
	STRMOM2;
	
	res.accumulateM (result,smm,sm);
	
}

static void bunaligned_products_sumIM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16,
																			 vector unsigned char perm1, vector unsigned char perm2)
{
	vector unsigned char *current, *previous;
	vector unsigned char t1, t2;
	vector unsigned int zero, mom01, mom02, sumim;
	vector unsigned int cross, mom11, mom22;
	unsigned int result, sii, smm, si, sm;
	
	
	current = (vector unsigned char *) input1;
	previous = (vector unsigned char *) input2;
	
	// Sum IM
	/* initialization */
	INITCOMMON;
	INITMOM1;
	INITMOM2;
	
	// m16 is 16 multiples. Subtrack 1 to match numbering of product loop
	assert (m16);
	m16 -= 1;
	
	// Prime the pipe
	t1  = vec_perm(current[0], current[1], perm1 );  /* align current vector  */ \
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	UUCROSSSUMIM(0);
	UUCROSSSUMIM(1);
	UUCROSSSUMIM(2);
	UUCROSSSUMIM(3);
	UUCROSSSUMIM(4);
	UUCROSSSUMIM(5);
	UUCROSSSUMIM(6);
	UUCROSSSUMIM(7);
	UUCROSSSUMIM(8);
	UUCROSSSUMIM(9);
	UUCROSSSUMIM(10);
	UUCROSSSUMIM(11);
	UUCROSSSUMIM(12);
	UUCROSSSUMIM(13);
	UUCROSSSUMIM(14);
	UUCROSSSUMIM(15);
	
uaccum:
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = (vector unsigned int) (vec_sums((vector signed int) cross, (vector signed int) zero));
	ACUMMOM1;
	ACUMMOM2;
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	REPLMOM1;
	REPLMOM2;
	
	// Store the result in vector at an address
	vec_ste( sumim, 0, &result );
	STRMOM1;
	STRMOM2;
	
	res.accumulate (result,sii,smm,si,sm);
	
}


static void bunaligned_products_sumI ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16,
																			vector unsigned char perm1, vector unsigned char perm2)
{
	vector unsigned char *current, *previous;
	vector unsigned char t1, t2;
	vector unsigned int zero, mom01, sumim;
	vector unsigned int cross, mom11;
	unsigned int result, sii, si;
	
	
	current = (vector unsigned char *) input1;
	previous = (vector unsigned char *) input2;
	
	// Sum I
	/* initialization */
	INITCOMMON;
	INITMOM1;
	
	// m16 is 16 multiples. Subtrack 1 to match numbering of product loop
	assert (m16);
	m16 -= 1;
	
	// Prime the pipe
	t1  = vec_perm(current[0], current[1], perm1 );  /* align current vector  */ \
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	UUCROSSSUMI(0);
	UUCROSSSUMI(1);
	UUCROSSSUMI(2);
	UUCROSSSUMI(3);
	UUCROSSSUMI(4);
	UUCROSSSUMI(5);
	UUCROSSSUMI(6);
	UUCROSSSUMI(7);
	UUCROSSSUMI(8);
	UUCROSSSUMI(9);
	UUCROSSSUMI(10);
	UUCROSSSUMI(11);
	UUCROSSSUMI(12);
	UUCROSSSUMI(13);
	UUCROSSSUMI(14);
	UUCROSSSUMI(15);
	
uaccum:
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = (vector unsigned int) (vec_sums((vector signed int) cross, (vector signed int) zero));
	ACUMMOM1;
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	REPLMOM1;
	
	// Store the result in vector at an address
	vec_ste( sumim, 0, &result);
	STRMOM1;
	
	res.accumulate (result, sii, si );
	
}

static void bunaligned_products_sumM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16,
																			vector unsigned char perm1, vector unsigned char perm2)
{
	vector unsigned char *current, *previous;
	vector unsigned char t1, t2;
	vector unsigned int zero, mom02, sumim;
	vector unsigned int cross, mom22;
	unsigned int result, smm, sm;
	
	
	current = (vector unsigned char *) input1;
	previous = (vector unsigned char *) input2;
	
	// Sum M
	/* initialization */
	INITCOMMON;
	INITMOM2;
	
	// m16 is 16 multiples. Subtrack 1 to match numbering of product loop
	assert (m16);
	m16 -= 1;
	
	// Prime the pipe
	t2  = vec_perm(previous[0], previous[1], perm2 );/* align previous vector */    
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	UUCROSSSUMM(0);
	UUCROSSSUMM(1);
	UUCROSSSUMM(2);
	UUCROSSSUMM(3);
	UUCROSSSUMM(4);
	UUCROSSSUMM(5);
	UUCROSSSUMM(6);
	UUCROSSSUMM(7);
	UUCROSSSUMM(8);
	UUCROSSSUMM(9);
	UUCROSSSUMM(10);
	UUCROSSSUMM(11);
	UUCROSSSUMM(12);
	UUCROSSSUMM(13);
	UUCROSSSUMM(14);
	UUCROSSSUMM(15);
	
uaccum:
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = (vector unsigned int) (vec_sums((vector signed int) cross, (vector signed int) zero));
	ACUMMOM2;
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	REPLMOM2;
	
	// Store the result in vector at an address
	vec_ste( sumim, 0, &result);
	STRMOM2;
	
	res.accumulateM(result, smm, sm );
	
}

static void bunaligned_products ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16,
																 vector unsigned char perm1, vector unsigned char perm2)
{
	vector unsigned char *current, *previous;
	vector unsigned char t1, t2;
	vector unsigned int zero, sumim;
	vector unsigned int cross;
	unsigned int result;
	
	
	current = (vector unsigned char *) input1;
	previous = (vector unsigned char *) input2;
	
	// Sum 
	/* initialization */
	INITCOMMON;
	
	// m16 is 16 multiples. Subtrack 1 to match numbering of product loop
	assert (m16);
	m16 -= 1;
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	UUCROSS(0);
	UUCROSS(1);
	UUCROSS(2);
	UUCROSS(3);
	UUCROSS(4);
	UUCROSS(5);
	UUCROSS(6);
	UUCROSS(7);
	UUCROSS(8);
	UUCROSS(9);
	UUCROSS(10);
	UUCROSS(11);
	UUCROSS(12);
	UUCROSS(13);
	UUCROSS(14);
	UUCROSS(15);
	
uaccum:
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = (vector unsigned int) (vec_sums((vector signed int) cross, (vector signed int) zero));
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	
	// Store the result in vector at an address
	vec_ste( sumim, 0, &result );
	
	res.accumulate (result);
	
}





////////////// 16 bit Support ///////////////////

static void baligned_productsUS ( const unsigned short *input1, const unsigned short *input2, rcCorr& res, unsigned int m16)
{
	vector signed short *current, *previous;
	vector signed short t1, t2;
	vector signed int zero, mom01, mom02, sumim;
	vector signed int cross, mom11, mom22;
	unsigned int result, sii, smm, si, sm;
	
	
	current = (vector signed short *) input1;
	previous = (vector signed short *) input2;
	
	// Sum IM
	/* initialization */
	INITCOMMON16;
	
	// m16 is 16 multiples. Subtrack 1 to match numbering of product loop
	assert (m16);
	m16 -= 1;
	
	// Prime the pipe
	t1  = current[0];                /* align current vector  */     
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	ALCROSSSUMIM(0);
	ALCROSSSUMIM(1);
	ALCROSSSUMIM(2);
	ALCROSSSUMIM(3);
	ALCROSSSUMIM(4);
	ALCROSSSUMIM(5);
	ALCROSSSUMIM(6);
	ALCROSSSUMIM(7);
	ALCROSSSUMIM(8);
	ALCROSSSUMIM(9);
	ALCROSSSUMIM(10);
	ALCROSSSUMIM(11);
	ALCROSSSUMIM(12);
	ALCROSSSUMIM(13);
	ALCROSSSUMIM(14);
	ALCROSSSUMIM(15);
	
accum:
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = vec_sums((vector signed int) cross, (vector signed int) zero);
	ACUMMOM1S;
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	REPLMOMS;
	
	// Store the result in vector at an address
	vec_ste( (vector unsigned int) sumim, 0, &result );
	STRMOMS;
	
	res.accumulate (result,sii,smm,si,sm);
	
}


static void bunaligned_productsUS ( const unsigned short *input1, const unsigned short *input2, rcCorr& res, unsigned int m16,
																	 vector unsigned char perm1, vector unsigned char perm2)
{
	vector signed short *current, *previous;
	vector signed short t1, t2;
	vector signed int zero, mom01, mom02, sumim;
	vector signed int cross, mom11, mom22;
	unsigned int result, sii, smm, si, sm;
	
	
	current = (vector signed short *) input1;
	previous = (vector signed short *) input2;
	
	// Sum IM
	/* initialization */
	INITCOMMON16;
	
	// m16 is 16 multiples. Subtrack 1 to match numbering of product loop
	assert (m16);
	m16 -= 1;
	
	// Prime the pipe
	t1  = vec_perm(current[0], current[1], perm1 );  /* align current vector  */ \
	
	// Process 16 time 16 Pels. Each step consists of cross product as well as 0, and first moments
	// Results are written in to an int vector which has 4 partial sums in it
	UUCROSSSUMIM(0);
	UUCROSSSUMIM(1);
	UUCROSSSUMIM(2);
	UUCROSSSUMIM(3);
	UUCROSSSUMIM(4);
	UUCROSSSUMIM(5);
	UUCROSSSUMIM(6);
	UUCROSSSUMIM(7);
	UUCROSSSUMIM(8);
	UUCROSSSUMIM(9);
	UUCROSSSUMIM(10);
	UUCROSSSUMIM(11);
	UUCROSSSUMIM(12);
	UUCROSSSUMIM(13);
	UUCROSSSUMIM(14);
	UUCROSSSUMIM(15);
	
uaccum:
	// Sum 4 partial sums in a vector of int in to a single int of a int vector [s,s,s,s] = [-,-,-,S]
	sumim = vec_sums((vector signed int) cross, (vector signed int) zero);
	ACUMMOM1S;
  
	// replicate the partial sums in to the other ints in an int vector
	sumim = vec_splat( sumim, 3 );
	REPLMOMS;
	
	// Store the result in vector at an address
	vec_ste((vector unsigned int) sumim, 0, &result );
	STRMOMS;
	
	res.accumulate (result,sii,smm,si,sm);
	
}

#endif

// Stream output operator
ostream& operator << ( ostream& os, const rcCorr& corr )
{
  os << "R " << corr.r() << " N " <<  corr.n() << " Rp " <<  corr.singular();
  os << " Si " <<  corr.Si() << " Sii " <<  corr.Sii() << " Sm " <<  corr.Sm() << " Smm " <<  corr.Smm() << " Sim " <<  corr.Sim() << endl;
  return os;
}
