
#include "row_func.h"
#include "static.hpp"


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
    uint32 operator [] (int index) const { assert( index < mSize ); return mTable[index]; };
    uint32* lut_ptr () { return mTable; }
    
    
    // Array size
    uint32 size() const { return (2*numeric_limits<T>::max()) - 1; };
    
private:
    static const int mSize = 2*numeric_limits<T>::max() - 1;
    uint32 mTable[ mSize ];
};

SINGLETON_FCN(sSqrTable<uint8>, square_table);



//
// row_func_TwoSource class implementation
//
template <>
bool row_func_TwoSource<uint8>::checkAssigned (const roi_window& srcA, const roi_window& srcB) 
{
  return (srcA.depth() == rpixel8 && srcB.depth() == rpixel8);
}

template <>
bool row_func_TwoSource<uint16>::checkAssigned (const roi_window& srcA, const roi_window& srcB) 
{
  return (srcA.depth() == rpixel8 && srcB.depth() == rpixel16);
}


// One Source One Destination 
template <>
bool row_func_OneSourceOneDestination<uint8>::checkAssigned (const roi_window& srcA, const roi_window& srcB) 
{
  return (srcA.depth() == rpixel8 && srcB.depth() == rpixel8);
}
template <>
bool row_func_OneSourceOneDestination<uint16>::checkAssigned (const roi_window& srcA, const roi_window& srcB) 
{
  return (srcA.depth() == rpixel8 && srcB.depth() == rpixel16);
}


//
// basic_corr_RowFunc class implementation
//
template <>
inline void basic_corr_RowFunc<uint8>::rowFuncNoneCached ()
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
	
	mFirst += mRUP.first;
	mSecond += mRUP.second;
}
template <>
inline void basic_corr_RowFunc<uint8>::rowFuncICached ()
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
	
	mFirst += mRUP.first;
	mSecond += mRUP.second;
}
template <>
inline void basic_corr_RowFunc<uint8>::rowFuncMCached ()
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
	
	mFirst += mRUP.first;
	mSecond += mRUP.second;
}
template <>
inline void basic_corr_RowFunc<uint8>::rowFuncIMCached ()
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
	
	mFirst += mRUP.first;
	mSecond += mRUP.second;
}
template <>
inline void basic_corr_RowFunc<uint8>::rowFunc ()
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
inline void basic_corr_RowFunc<uint8>::epilog (rcCorr& res)
{
	mRes.Sim ((mRes.Sim() - mRes.Sii() - mRes.Smm())/2.);
	mRes.n (mWidth * mHeight);
	mRes.compute ();
	res = mRes;
}


template <>
inline void pixel_map<uint8>::rowFunc ()
{
	const uint8 *pFirst (mFirst);
	uint8 *pSecond (mSecond);
	
	for (const uint8 *pEnd = mFirst + mWidth; pFirst < pEnd; )
	{
		*pSecond++ = mLut[*pFirst++];
	}
	mFirst += mRUP.first;
	mSecond += mRUP.second;
}
template <>
inline void pixel_map<uint16>::rowFunc ()
{
	const uint16 *pFirst (mFirst);
	uint16 *pSecond (mSecond);
	
	for (const uint16 *pEnd = mFirst + mWidth; pFirst < pEnd; )
	{
		*pSecond++ = mLut[*pFirst++];
	}
	mFirst += mRUP.first;
	mSecond += mRUP.second;
}

// Instantiate : Keep this at the bottom

template class pixel_map<uint8>;
template class pixel_map<uint16>;
