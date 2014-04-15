
#include <rc_rowfunc.h>
#include <static.hpp>


// A class to encapsulate a table of pre-computed squares
template<typename T>
class sSqrTable
{
    
public:
    // ctor
    sSqrTable()
    {
        for (uint32 i = 0; i < mSize; i++)
            mTable[i] = i * i;
    };
    
    // Array access operator
    uint32 operator [] (int index) const { rmAssertDebug( index < mSize ); return mTable[index]; };
    uint32* lut_ptr () { return mTable; }
    
    
    // Array size
    uint32 size() const { return (2*numeric_limits<T>::max()) - 1; };
    
private:
    static const int mSize = 2*numeric_limits<T>::max() - 1;
    uint32 mTable[ mSize ];
};

SINGLETON_FCN(sSqrTable<uint8>, square_table);



//
// rcRowFuncTwoSource class implementation
//
template <>
bool rcRowFuncTwoSource<uint8>::checkAssigned (const rcWindow& srcA, const rcWindow& srcB) 
{
  return (srcA.depth() == rcPixel8 && srcB.depth() == rcPixel8);
}

template <>
bool rcRowFuncTwoSource<uint16>::checkAssigned (const rcWindow& srcA, const rcWindow& srcB) 
{
  return (srcA.depth() == rcPixel8 && srcB.depth() == rcPixel16);
}

template <>
bool rcRowFuncTwoSource<uint32>::checkAssigned (const rcWindow& srcA, const rcWindow& srcB) 
{
  return (srcA.depth() == rcPixel8 && srcB.depth() == rcPixel32S);
}

template <>
bool rcRowFuncTwoSource<double>::checkAssigned (const rcWindow& srcA, const rcWindow& srcB) 
{
  return (srcA.depth() == rcPixelDouble && srcB.depth() == rcPixelDouble);
}

template <>
bool rcRowFuncTwoSource<float>::checkAssigned (const rcWindow& srcA, const rcWindow& srcB) 
{
  return (srcA.depth() == rcPixelFloat && srcB.depth() == rcPixelFloat && 
					srcA.frameBuf()->isD32Float() && 	  srcB.frameBuf()->isD32Float() );
}


// One Source One Destination 
template <>
bool rcRowFuncOneSourceOneDestination<uint8>::checkAssigned (const rcWindow& srcA, const rcWindow& srcB) 
{
  return (srcA.depth() == rcPixel8 && srcB.depth() == rcPixel8);
}
template <>
bool rcRowFuncOneSourceOneDestination<uint16>::checkAssigned (const rcWindow& srcA, const rcWindow& srcB) 
{
  return (srcA.depth() == rcPixel8 && srcB.depth() == rcPixel16);
}
template <>
bool rcRowFuncOneSourceOneDestination<uint32>::checkAssigned (const rcWindow& srcA, const rcWindow& srcB) 
{
  return (srcA.depth() == rcPixel8 && srcB.depth() == rcPixel32S);
}
template <>
bool rcRowFuncOneSourceOneDestination<double>::checkAssigned (const rcWindow& srcA, const rcWindow& srcB) 
{
  return (srcA.depth() == rcPixelDouble && srcB.depth() == rcPixelDouble);
}


//
// rcBasicCorrRowFunc class implementation
//
template <>
inline void rcBasicCorrRowFunc<uint8>::rowFuncNoneCached ()
{
	uint32 Si (0), Sm (0), Sim (0), Smm (0), Sii (0);
	const uint8 *pFirst (mFirst);
	const uint8 *pSecond (mSecond);
	static uint32* sqr_lut = square_table().lut_ptr();
    
	for (const uint8 *pEnd = mFirst + mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
	{
		const uint8 i = *pFirst;
		const uint8 j = *pSecond;
		
		Si  += i;
		Sm  += j;
		Sii += sqr_lut[i];
		Smm += sqr_lut[j];
		Sim += sqr_lut[i + j];
	}
	mRes.accumulate (Sim, Sii, Smm, Si, Sm);
	
	mFirst += mRUP.x();
	mSecond += mRUP.y();
}
template <>
inline void rcBasicCorrRowFunc<uint8>::rowFuncICached ()
{
	uint32 Sm (0), Sim (0), Smm (0);
	const uint8 *pFirst (mFirst);
	const uint8 *pSecond (mSecond);
	static uint32* sqr_lut = square_table().lut_ptr();
    
	for (const uint8 *pEnd = mFirst + mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
	{
		const uint8 i = *pFirst;
		const uint8 j = *pSecond;
		
		Sm  += j;
		Smm += sqr_lut[j];
		Sim += sqr_lut[i + j];
	}
	mRes.accumulateM (Sim, Smm, Sm);
	
	mFirst += mRUP.x();
	mSecond += mRUP.y();
}
template <>
inline void rcBasicCorrRowFunc<uint8>::rowFuncMCached ()
{
	uint32 Si (0), Sim (0), Sii (0);
	const uint8 *pFirst (mFirst);
	const uint8 *pSecond (mSecond);
	static uint32* sqr_lut = square_table().lut_ptr();    
	
	for (const uint8 *pEnd = mFirst + mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
	{
		const uint8 i = *pFirst;
		const uint8 j = *pSecond;
		
		Si  += i;
		Sii += sqr_lut[i];
		Sim += sqr_lut[i + j];
	}
	mRes.accumulate (Sim, Sii, Si);
	
	mFirst += mRUP.x();
	mSecond += mRUP.y();
}
template <>
inline void rcBasicCorrRowFunc<uint8>::rowFuncIMCached ()
{
	uint32 Sim (0);
	const uint8 *pFirst (mFirst);
	const uint8 *pSecond (mSecond);
	static uint32* sqr_lut = square_table().lut_ptr(); 
    
	for (const uint8 *pEnd = mFirst + mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
	{
		const uint8 i = *pFirst;
		const uint8 j = *pSecond;
		
		Sim += sqr_lut[i + j];
	}
	mRes.accumulate (Sim);
	
	mFirst += mRUP.x();
	mSecond += mRUP.y();
}
template <>
inline void rcBasicCorrRowFunc<uint8>::rowFunc ()
{
	switch ( mCacheState ) {
		case eNone:
			rowFuncNoneCached();
			break;
		case eImageI:
			rowFuncICached();
			break;
		case eImageM:
			rowFuncMCached();
			break;
		case eImageIM:
			rowFuncIMCached();
			break;
	}
}


// uint8 version uses the Square Table technique
// Correlate two sources. We can not cache sums since we do not have a model representation
// Use Square table technique [reference Moravoc paper in the 80s]
template <>
inline void rcBasicCorrRowFunc<uint8>::epilog (rcCorr& res)
{
	mRes.Sim ((mRes.Sim() - mRes.Sii() - mRes.Smm())/2.);
	mRes.n (mWidth * mHeight);
	mRes.compute ();
	res = mRes;
}


template <>
inline void rcBasicPixelMap<uint8>::rowFunc ()
{
	const uint8 *pFirst (mFirst);
	uint8 *pSecond (mSecond);
	
	for (const uint8 *pEnd = mFirst + mWidth; pFirst < pEnd; )
	{
		*pSecond++ = mLut[*pFirst++];
	}
	mFirst += mRUP.x();
	mSecond += mRUP.y();
}
template <>
inline void rcBasicPixelMap<uint16>::rowFunc ()
{
	const uint16 *pFirst (mFirst);
	uint16 *pSecond (mSecond);
	
	for (const uint16 *pEnd = mFirst + mWidth; pFirst < pEnd; )
	{
		*pSecond++ = mLut[*pFirst++];
	}
	mFirst += mRUP.x();
	mSecond += mRUP.y();
}

//
// rcAltiVecCorrRowFunc class implementation
//
#ifdef __ppc__
static const uint32 rcAltiVecBytesInVector = 16;
static const uint32 rcAltiVecShortsInVector = 8;
static const uint32 rcAltiVecVectorsInStrip = 16;
static const uint32 rcAltiVecDefaultStripCount = rcAltiVecVectorsInStrip * rcAltiVecBytesInVector;
static const uint32 rcAltiVecShortsStripCount = rcAltiVecVectorsInStrip * rcAltiVecShortsInVector;


// Nothing to expuge for now
template <>
rcAltiVecCorrRowFunc<uint8>::~rcAltiVecCorrRowFunc<uint8> () {}

// TBD: add check that we have been initialized
template <>
inline void rcAltiVecCorrRowFunc<uint8>::prolog ()
{
	mBlocks16 = mWidth / rcAltiVecBytesInVector;
	mRemPelBlocks = mWidth % rcAltiVecBytesInVector;
	mBlocks = mBlocks16 / rcAltiVecVectorsInStrip;
	mRem16Blocks = mBlocks16 % rcAltiVecVectorsInStrip;
	assert (mWidth == (mRemPelBlocks + 
										 rcAltiVecBytesInVector * mRem16Blocks + 
										 mBlocks * rcAltiVecVectorsInStrip * rcAltiVecBytesInVector));
}
template <>
inline void rcAltiVecCorrRowFunc<uint8>::epilog (rcCorr& res)
{
  mRes.n (mWidth * mHeight);
  mRes.compute ();
  res = mRes;
}
template <>
inline void rcAltiVecCorrRowFunc<uint8>::rowFuncNoneCached()
{
  const uint8 *pFirst (mFirst);
  const uint8 *pSecond (mSecond);
	
  // Do as many strips
  for (int32 j = 0; j < mBlocks; ++j, pFirst +=  rcAltiVecDefaultStripCount , pSecond+=rcAltiVecDefaultStripCount)
	{
		altivec8bitPelProductsSumIM (pFirst, pSecond, mRes, rcAltiVecVectorsInStrip, mPermFirst, mPermSecond);
	}
	
  // Do as many 16 wide strips left over
  if (mRem16Blocks)
	{
		altivec8bitPelProductsSumIM (pFirst, pSecond, mRes, mRem16Blocks, mPermFirst, mPermSecond);
		pFirst += mRem16Blocks * rcAltiVecBytesInVector;
		pSecond += mRem16Blocks * rcAltiVecBytesInVector;
	}
	
  // Ok fewer than 16 pels left over. Copy them in to a 16 byte cleared buffer
  // @note optimize better
  if (mRemPelBlocks)
	{
		uint8 ib[16] __attribute__ ((aligned (16)));
		uint8 mb[16] __attribute__ ((aligned (16)));
		
		for (int32 j = 0; j < mRemPelBlocks; ++j) ib[j] = pFirst[j], mb[j] = pSecond[j];
		for (int32 j = mRemPelBlocks; j < int32 (rcAltiVecBytesInVector); ++j) ib[j] = 0, mb[j] = 0;
		
		altivec8bitPelProductsSumIM (ib, mb, mRes, 1, mPermFirst, mPermSecond);
	}
  mFirst += mRUP.x();
  mSecond += mRUP.y();
}
template <>
inline void rcAltiVecCorrRowFunc<uint8>::rowFuncICached()
{
  const uint8 *pFirst (mFirst);
  const uint8 *pSecond (mSecond);
	
  // Do as many strips
  for (int32 j = 0; j < mBlocks; ++j, pFirst +=  rcAltiVecDefaultStripCount , pSecond+=rcAltiVecDefaultStripCount)
	{
		altivec8bitPelProductsSumM (pFirst, pSecond, mRes, rcAltiVecVectorsInStrip, mPermFirst, mPermSecond);
	}
	
  // Do as many 16 wide strips left over
  if (mRem16Blocks)
	{
		altivec8bitPelProductsSumM (pFirst, pSecond, mRes, mRem16Blocks, mPermFirst, mPermSecond);
		pFirst += mRem16Blocks * rcAltiVecBytesInVector;
		pSecond += mRem16Blocks * rcAltiVecBytesInVector;
	}
	
  // Ok fewer than 16 pels left over. Copy them in to a 16 byte cleared buffer
  if (mRemPelBlocks)
	{
		uint8 ib[16] __attribute__ ((aligned (16)));
		uint8 mb[16] __attribute__ ((aligned (16)));	
	  
		for (int32 j = 0; j < mRemPelBlocks; ++j) ib[j] = pFirst[j], mb[j] = pSecond[j];
		for (int32 j = mRemPelBlocks; j < int32 (rcAltiVecBytesInVector); ++j) ib[j] = 0, mb[j] = 0;
		
		altivec8bitPelProductsSumM (ib, mb, mRes, 1, mPermFirst, mPermSecond);
	}
  mFirst += mRUP.x();
  mSecond += mRUP.y();
}
template <>
inline void rcAltiVecCorrRowFunc<uint8>::rowFuncMCached()
{
  const uint8 *pFirst (mFirst);
  const uint8 *pSecond (mSecond);
	
  // Do as many strips
  for (int32 j = 0; j < mBlocks; ++j, pFirst +=  rcAltiVecDefaultStripCount , pSecond+=rcAltiVecDefaultStripCount)
	{
		altivec8bitPelProductsSumI(pFirst, pSecond, mRes, rcAltiVecVectorsInStrip, mPermFirst, mPermSecond);
	}
	
  // Do as many 16 wide strips left over
  if (mRem16Blocks)
	{
		altivec8bitPelProductsSumI (pFirst, pSecond, mRes, mRem16Blocks, mPermFirst, mPermSecond);
		pFirst += mRem16Blocks * rcAltiVecBytesInVector;
		pSecond += mRem16Blocks * rcAltiVecBytesInVector;
	}
	
  // Ok fewer than 16 pels left over. Copy them in to a 16 byte cleared buffer
  if (mRemPelBlocks)
	{
		uint8 ib[16] __attribute__ ((aligned (16)));
		uint8 mb[16] __attribute__ ((aligned (16)));	
	  
		for (int32 j = 0; j < mRemPelBlocks; ++j) ib[j] = pFirst[j], mb[j] = pSecond[j];
		for (int32 j = mRemPelBlocks; j < int32 (rcAltiVecBytesInVector); ++j) ib[j] = 0, mb[j] = 0;
		
		altivec8bitPelProductsSumI (ib, mb, mRes, 1, mPermFirst, mPermSecond);
	}
  mFirst += mRUP.x();
  mSecond += mRUP.y();
}
template <>
inline void rcAltiVecCorrRowFunc<uint8>::rowFuncIMCached()
{
	const uint8 *pFirst (mFirst);
	const uint8 *pSecond (mSecond);
	
  // Do as many strips
  for (int32 j = 0; j < mBlocks; ++j, pFirst +=  rcAltiVecDefaultStripCount , pSecond+=rcAltiVecDefaultStripCount)
	{
		altivec8bitPelProducts(pFirst, pSecond, mRes, rcAltiVecVectorsInStrip, mPermFirst, mPermSecond);
	}
	
  // Do as many 16 wide strips left over
  if (mRem16Blocks)
	{
		altivec8bitPelProducts(pFirst, pSecond, mRes, mRem16Blocks, mPermFirst, mPermSecond);
		pFirst += mRem16Blocks * rcAltiVecBytesInVector;
		pSecond += mRem16Blocks * rcAltiVecBytesInVector;
	}
	
  // Ok fewer than 16 pels left over. Copy them in to a 16 byte cleared buffer
  if (mRemPelBlocks)
	{
		uint8 ib[16] __attribute__ ((aligned (16)));
		uint8 mb[16] __attribute__ ((aligned (16)));
	  
		for (int32 j = 0; j < mRemPelBlocks; ++j) ib[j] = pFirst[j], mb[j] = pSecond[j];
		for (int32 j = mRemPelBlocks; j < int32 (rcAltiVecBytesInVector); ++j) ib[j] = 0, mb[j] = 0;
		
		altivec8bitPelProducts(ib, mb, mRes, 1, mPermFirst, mPermSecond);
	}
  mFirst += mRUP.x();
  mSecond += mRUP.y();
}
template <>
inline void rcAltiVecCorrRowFunc<uint8>::areaFunc()
{
	uint32 height = mHeight;
	
#ifdef DEBUG_LOG
	switch ( mCacheState ) {
		case eNone:
			cerr << "rowFuncNoneCached" << endl;
			break;
		case eImageI:
			cerr << "rowFuncICached" << endl;
			break;
		case eImageM:
			cerr << "rowFuncMCached" << endl;
			break;
		case eImageIM:
			cerr << "rowFuncIMCached" << endl;
			break;
	}
#endif
	
	switch ( mCacheState ) {
		case eNone:
			do { rowFuncNoneCached(); } while (--height);
			break;
		case eImageI:
			do { rowFuncICached(); } while (--height);
			break;
		case eImageM:
			do { rowFuncMCached(); } while (--height);
			break;
		case eImageIM:
			do { rowFuncIMCached(); } while (--height);
			break;
	}
}
template <>
inline void rcAltiVecCorrRowFunc<uint8>::rowFunc()
{
	switch ( mCacheState ) {
		case eNone:
			rowFuncNoneCached();
			break;
		case eImageI:
			rowFuncICached();
			break;
		case eImageM:
			rowFuncMCached();
			break;
		case eImageIM:
			rowFuncIMCached();
			break;
	}
}


///////// 16 bit /////////////////


// Nothing to expuge for now
template <>
rcAltiVecCorrRowFunc<uint16>::~rcAltiVecCorrRowFunc<uint16> () {}

// TBD: add check that we have been initialized
template <>
inline void rcAltiVecCorrRowFunc<uint16>::prolog ()
{
	mBlocks16 = mWidth / rcAltiVecShortsInVector;
	mRemPelBlocks = mWidth % rcAltiVecShortsInVector;
	mBlocks = mBlocks16 / rcAltiVecVectorsInStrip;
	mRem16Blocks = mBlocks16 % rcAltiVecVectorsInStrip;
	assert (mWidth == (mRemPelBlocks + 
										 rcAltiVecShortsInVector * mRem16Blocks + 
										 mBlocks * rcAltiVecVectorsInStrip * rcAltiVecShortsInVector));
}
template <>
inline void rcAltiVecCorrRowFunc<uint16>::epilog (rcCorr& res)
{
  mRes.n (mWidth * mHeight);
  mRes.compute ();
  res = mRes;
}
template <>
inline void rcAltiVecCorrRowFunc<uint16>::rowFuncNoneCached()
{
  const uint16 *pFirst (mFirst);
  const uint16 *pSecond (mSecond);
	
  // Do as many strips
  for (int32 j = 0; j < mBlocks; ++j, pFirst +=  rcAltiVecShortsStripCount , pSecond+=rcAltiVecShortsStripCount)
	{
		altivec16bitPelProducts (pFirst, pSecond, mRes, rcAltiVecVectorsInStrip, mPermFirst, mPermSecond);
	}
	
  // Do as many 16 wide strips left over
  if (mRem16Blocks)
	{
		altivec16bitPelProducts (pFirst, pSecond, mRes, mRem16Blocks, mPermFirst, mPermSecond);
		pFirst += mRem16Blocks * rcAltiVecShortsInVector;
		pSecond += mRem16Blocks * rcAltiVecShortsInVector;
	}
	
  // Ok fewer than 16 pels left over. Copy them in to a 16 byte cleared buffer
  // @note optimize better
  if (mRemPelBlocks)
	{
		uint16 ib[16], mb[16];
	  
		for (int32 j = 0; j < mRemPelBlocks; ++j) ib[j] = pFirst[j], mb[j] = pSecond[j];
		for (int32 j = mRemPelBlocks; j < int32 (rcAltiVecShortsInVector); ++j) ib[j] = 0, mb[j] = 0;
		
		altivec16bitPelProducts (ib, mb, mRes, 1, mPermFirst, mPermSecond);
	}
  mFirst += mRUP.x();
  mSecond += mRUP.y();
}
template <>
inline void rcAltiVecCorrRowFunc<uint16>::areaFunc()
{
	uint32 height = mHeight;
	
	switch ( mCacheState ) {
		case eNone:
		case eImageI:
		case eImageM:
		case eImageIM:
			do { rowFuncNoneCached(); } while (--height);
			break;
	}
}
template <>
inline void rcAltiVecCorrRowFunc<uint16>::rowFunc()
{
	switch ( mCacheState ) {
		case eNone:
		case eImageI:
		case eImageM:
		case eImageIM:
			rowFuncNoneCached();
			break;
	}
}

#endif

// Instantiate : Keep this at the bottom
#ifdef __ppc__
template class rcAltiVecCorrRowFunc<uint8>;
template class rcAltiVecCorrRowFunc<uint16>;
#endif
template class rcBasicPixelMap<uint8>;
template class rcBasicPixelMap<uint16>;
