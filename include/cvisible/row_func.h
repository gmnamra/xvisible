#ifndef __ROWFUNC_H
#define __ROWFUNC_H

#include "roi_window.h"
#include  "image_correlation.h"
#include "simple_pair.h"

/*
 * Base class for two sources (and a scaler or a non-image
 * result out ).
 *
 * Usage: 
 * drive from the base class
 * implement the row func
 *
 * A processing function is a template function of a derived
 * class of this and if image class is a templatized 
 * 
 */


enum moments_cached {
	eNone = 0,      // Nothing cached 
	eImageI,        // First image sums cached
	eImageM,        // Second image sums cached
	eImageIM        // Both image sums cached
};

template <class T>
class row_func_TwoSource
{
	
public:
  row_func_TwoSource () {}
  virtual ~row_func_TwoSource () {}
	
  virtual void prolog () = 0;
  virtual void rowFunc () = 0;
  virtual void areaFunc () = 0;
  
protected:
  static bool checkAssigned (const roi_window&, const roi_window&);
  const T* mFirst;
  const T* mSecond;
  uint32 mWidth;
  uint32 mHeight;  
};

template <class T>
class row_func_OneSourceOneDestination
{
	
public:
  row_func_OneSourceOneDestination () {}
  virtual ~row_func_OneSourceOneDestination () {}
	
  virtual void prolog () = 0;
  virtual void rowFunc () = 0;
  virtual void areaFunc () = 0;
  
protected:
  static bool checkAssigned (const roi_window&, const roi_window&);
  const T* mFirst;
  T* mSecond;
  uint32 mWidth;
  uint32 mHeight;  
};


template<class T>
class pixel_map : public row_func_OneSourceOneDestination<T>
{
public:
  pixel_map (const roi_window& srcA, const roi_window& dst, const vector<T>& lut);
	
  virtual void prolog ();
  virtual void rowFunc ();
  virtual void areaFunc ();
	
  virtual ~pixel_map ();
  
private:
  const T* mLut;
  rcUIPair mRUP;
};


template <class T>
class basic_corr_RowFunc : public row_func_TwoSource<T>
{
public:
	basic_corr_RowFunc (const roi_window& srcA, const roi_window& srcB, T& dummy);
	basic_corr_RowFunc (roi_window_t<T>& srcA, roi_window_t<T>& srcB);
	
	basic_corr_RowFunc (const T* baseA, const T* baseB, uint32 rupA, uint32 rupB, 
											uint32 width, uint32 height);
	
	virtual void prolog ();
	virtual void rowFunc ();
	virtual void areaFunc ();
	void epilog (rcCorr&);
	
	virtual ~basic_corr_RowFunc ();
	
private:
	void setCache(const roi_window_t<T>& srcA, const roi_window_t<T>& srcB);
	void rowFuncNoneCached();
	void rowFuncICached();
	void rowFuncMCached();
	void rowFuncIMCached();
	
	rcCorr   mRes;
	rcUIPair mRUP;
	moments_cached mCacheState;
};


// public

template <class T>
basic_corr_RowFunc<T>::basic_corr_RowFunc(roi_window_t<T>& winA, roi_window_t<T>& winB ) :
mRUP( winA.rowUpdate () / sizeof (T), winB.rowUpdate () / sizeof (T) )
{
	
	row_func_TwoSource<T>::checkAssigned (winB, winA);
	rmAssertDebug (winA.size() == winB.size());
	row_func_TwoSource<T>::mWidth = winA.width();
	row_func_TwoSource<T>::mHeight = winB.height();
	row_func_TwoSource<T>::mFirst = (T *) winA.rowPointer(0);
	row_func_TwoSource<T>::mSecond = (T *) winB.rowPointer(0);
	
	setCache( winA, winB );
}

template <class T>
basic_corr_RowFunc<T>::basic_corr_RowFunc (const T* baseA, const T* baseB, uint32 rowPelsA, uint32 rowPelsB, 
                                           uint32 width, uint32 height) :
mRUP( rowPelsA / sizeof (T), rowPelsB / sizeof (T) ),
mCacheState( eNone )
{
	row_func_TwoSource<T>::mWidth = width;
	row_func_TwoSource<T>::mHeight = height;
	row_func_TwoSource<T>::mFirst = baseA;
	row_func_TwoSource<T>::mSecond = baseB;
}

template <class T>
basic_corr_RowFunc<T>::basic_corr_RowFunc (const roi_window& srcA,  const roi_window& srcB, T& dummy) :
mRUP( srcA.rowUpdate () / sizeof (T), srcB.rowUpdate () / sizeof (T) ),
mCacheState( eNone )
{
  rmUnused (dummy);
  row_func_TwoSource<T>::checkAssigned (srcB, srcA);
  rmAssertDebug (srcA.size() == srcB.size());
  row_func_TwoSource<T>::mWidth = srcA.width();
  row_func_TwoSource<T>::mHeight = srcB.height();
  row_func_TwoSource<T>::mFirst = (T *) srcA.rowPointer(0);
  row_func_TwoSource<T>::mSecond = (T *) srcB.rowPointer(0);
}

template <class T>
void basic_corr_RowFunc<T>::rowFunc ()
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

template <class T>
void basic_corr_RowFunc<T>::areaFunc ()
{
	uint32 height = row_func_TwoSource<T>::mHeight;
	
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

template <class T>
void basic_corr_RowFunc<T>::epilog (rcCorr& res)
{
	mRes.n (row_func_TwoSource<T>::mWidth * row_func_TwoSource<T>::mHeight);
	mRes.compute ();
	res = mRes;
}

template <class T>
void basic_corr_RowFunc<T>::prolog ()
{
}

template <class T>  
basic_corr_RowFunc<T>::~basic_corr_RowFunc<T> () {}

// private

template <class T>
void basic_corr_RowFunc<T>::setCache(const roi_window_t<T>& srcA, const roi_window_t<T>& srcB )
{
	// Set cache state and sums
	if ( srcA.sumValid() ) {
		if ( srcB.sumValid() ) {
			mCacheState = eImageIM;
			mRes.Sii( srcA.sumSquares() );
			mRes.Si( srcA.sum() );
			mRes.Smm( srcB.sumSquares() );
			mRes.Sm( srcB.sum() );
		}
		else {
			mCacheState = eImageI;
			mRes.Sii( srcA.sumSquares() );
			mRes.Si( srcA.sum() );
		}
	} else if ( srcB.sumValid() ) {
		mCacheState = eImageM;
		mRes.Smm( srcB.sumSquares() );
		mRes.Sm( srcB.sum() );
	} else {
		mCacheState = eNone;
	}
}

template <class T>
void basic_corr_RowFunc<T>::rowFuncNoneCached ()
{
  double Si(0), Sm(0), Sii(0), Smm(0), Sim(0);
  const T *pFirst (row_func_TwoSource<T>::mFirst);
  const T *pSecond (row_func_TwoSource<T>::mSecond);
	
  for (const T *pEnd = row_func_TwoSource<T>::mFirst + row_func_TwoSource<T>::mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
	{
		const T i = *pFirst;
		const T j = *pSecond;
		
		Si  += i;
		Sm  += j;
		Sii += i * i;
		Smm += j * j;
		Sim += i * j;
  }
  mRes.accumulate (Sim, Sii, Smm, Si, Sm);
	
  row_func_TwoSource<T>::mFirst += mRUP.x();
  row_func_TwoSource<T>::mSecond += mRUP.y();
}

template <class T>
void basic_corr_RowFunc<T>::rowFuncICached ()
{
  double Sm(0), Smm(0), Sim(0);
  const T *pFirst (row_func_TwoSource<T>::mFirst);
  const T *pSecond (row_func_TwoSource<T>::mSecond);
	
  for (const T *pEnd = row_func_TwoSource<T>::mFirst + row_func_TwoSource<T>::mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
  {
		const T i = *pFirst;
		const T j = *pSecond;
		
		Sm  += j;
		Smm += j * j;
		Sim += i * j;
  }
  mRes.accumulateM (Sim, Smm, Sm);
	
  row_func_TwoSource<T>::mFirst += mRUP.x();
  row_func_TwoSource<T>::mSecond += mRUP.y();
}

template <class T>
void basic_corr_RowFunc<T>::rowFuncMCached ()
{
  double Si(0), Sii(0), Sim(0);
  const T *pFirst (row_func_TwoSource<T>::mFirst);
  const T *pSecond (row_func_TwoSource<T>::mSecond);
	
  for (const T *pEnd = row_func_TwoSource<T>::mFirst + row_func_TwoSource<T>::mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
  {
		const T i = *pFirst;
		const T j = *pSecond;
		
		Si  += i;
		Sii += i * i;
		Sim += i * j;
  }
  mRes.accumulate (Sim, Sii, Si);
	
  row_func_TwoSource<T>::mFirst += mRUP.x();
  row_func_TwoSource<T>::mSecond += mRUP.y();
}

template <class T>
void basic_corr_RowFunc<T>::rowFuncIMCached ()
{
  double Sim(0);
  const T *pFirst (row_func_TwoSource<T>::mFirst);
  const T *pSecond (row_func_TwoSource<T>::mSecond);
	
  for (const T *pEnd = row_func_TwoSource<T>::mFirst + row_func_TwoSource<T>::mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
  {
		const T i = *pFirst;
		const T j = *pSecond;
		
		Sim += i * j;
  }
  mRes.accumulate (Sim);
	
  row_func_TwoSource<T>::mFirst += mRUP.x();
  row_func_TwoSource<T>::mSecond += mRUP.y();
}


// Pixel Map row functions

template <class T>
pixel_map<T>::pixel_map (const roi_window& srcA,  const roi_window& srcB,
																		 const vector<T>& lut) :
mRUP( srcA.rowUpdate () / sizeof (T), srcB.rowUpdate () / sizeof (T) )
{
  row_func_OneSourceOneDestination <T>::checkAssigned (srcB, srcA);
  rmAssertDebug (srcA.size() == srcB.size());
  row_func_OneSourceOneDestination<T>::mWidth = srcA.width();
  row_func_OneSourceOneDestination<T>::mHeight = srcB.height();
  row_func_OneSourceOneDestination<T>::mFirst = (T *) srcA.rowPointer(0);
  row_func_OneSourceOneDestination<T>::mSecond = (T *) srcB.rowPointer(0);
  mLut = (T *) (& lut[0]);
}


template <class T>
void pixel_map<T>::areaFunc ()
{
	uint32 height = row_func_OneSourceOneDestination<T>::mHeight;
	do { rowFunc(); } while (--height);
}

template <class T>  
pixel_map<T>::~pixel_map<T> () {}
template <class T>  
void pixel_map<T>::prolog () {}


#endif /* __ROWFUNC_H */
