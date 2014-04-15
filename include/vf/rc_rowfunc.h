/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.6  2004/08/17 18:09:26  arman
 *added rcAltiVecCorrRowFunc.epilog ()
 *
 *Revision 1.5  2003/04/03 22:56:13  sami
 *Correlation sum caching support added
 *
 *Revision 1.4  2003/04/02 22:19:54  sami
 *Minor optimizations
 *
 *Revision 1.3  2003/04/01 02:50:58  arman
 *added basicCorr and square table implementation for 8bit case
 *
 *Revision 1.2  2003/03/31 03:15:50  arman
 *rearranged.
 *
 *Revision 1.1  2003/03/30 20:43:58  arman
 *Header for row func template class
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcROWFUNC_H
#define __rcROWFUNC_H

#include <rc_ncs.h>
#include <rc_pair.h>

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


enum rcCorrMomentsCached {
	eNone = 0,      // Nothing cached 
	eImageI,        // First image sums cached
	eImageM,        // Second image sums cached
	eImageIM        // Both image sums cached
};

template <class T>
class rcRowFuncTwoSource
{
	
public:
  rcRowFuncTwoSource () {}
  virtual ~rcRowFuncTwoSource () {}
	
  virtual void prolog () = 0;
  virtual void rowFunc () = 0;
  virtual void areaFunc () = 0;
  
protected:
  static bool checkAssigned (const rcWindow&, const rcWindow&);
  const T* mFirst;
  const T* mSecond;
  uint32 mWidth;
  uint32 mHeight;  
};

template <class T>
class rcRowFuncOneSourceOneDestination
{
	
public:
  rcRowFuncOneSourceOneDestination () {}
  virtual ~rcRowFuncOneSourceOneDestination () {}
	
  virtual void prolog () = 0;
  virtual void rowFunc () = 0;
  virtual void areaFunc () = 0;
  
protected:
  static bool checkAssigned (const rcWindow&, const rcWindow&);
  const T* mFirst;
  T* mSecond;
  uint32 mWidth;
  uint32 mHeight;  
};


template<class T>
class rcBasicPixelMap : public rcRowFuncOneSourceOneDestination<T>
{
public:
  rcBasicPixelMap (const rcWindow& srcA, const rcWindow& dst, const vector<T>& lut);
	
  virtual void prolog ();
  virtual void rowFunc ();
  virtual void areaFunc ();
	
  virtual ~rcBasicPixelMap ();
  
private:
  const T* mLut;
  rcUIPair mRUP;
};


template <class T>
class rcBasicCorrRowFunc : public rcRowFuncTwoSource<T>
{
public:
	rcBasicCorrRowFunc (const rcWindow& srcA, const rcWindow& srcB, T& dummy);
	rcBasicCorrRowFunc (rcCorrelationWindow<T>& srcA, rcCorrelationWindow<T>& srcB);
	
	rcBasicCorrRowFunc (const T* baseA, const T* baseB, uint32 rupA, uint32 rupB, 
											uint32 width, uint32 height);
	
	virtual void prolog ();
	virtual void rowFunc ();
	virtual void areaFunc ();
	void epilog (rcCorr&);
	
	virtual ~rcBasicCorrRowFunc ();
	
private:
	void setCache(const rcCorrelationWindow<T>& srcA, const rcCorrelationWindow<T>& srcB);
	void rowFuncNoneCached();
	void rowFuncICached();
	void rowFuncMCached();
	void rowFuncIMCached();
	
	rcCorr   mRes;
	rcUIPair mRUP;
	rcCorrMomentsCached mCacheState;
};


#ifdef __ppc__

// TBD: pass in a rcCorr to avoid a copy at the end
template <class T>
class rcAltiVecCorrRowFunc : public rcRowFuncTwoSource<T>
{
public:
  rcAltiVecCorrRowFunc (const rcWindow& srcA, const rcWindow& srcB, T dummy);
	
  rcAltiVecCorrRowFunc (const T* baseA, const T* baseB, uint32 rupA, uint32 rupB, 
												uint32 width, uint32 height);
	
  rcAltiVecCorrRowFunc (rcCorrelationWindow<T>& srcA, rcCorrelationWindow<T>& srcB );
	
  virtual void prolog ();
  virtual void rowFunc ();
  virtual void areaFunc ();
  void epilog (rcCorr&);
	
  virtual ~rcAltiVecCorrRowFunc ();
  
private:
  void setCache(const rcCorrelationWindow<T>& srcA, const rcCorrelationWindow<T>& srcB);
  void rowFuncNoneCached();
  void rowFuncICached();
  void rowFuncMCached();
  void rowFuncIMCached();
  
  vector unsigned char mPermFirst;
  vector unsigned char mPermSecond;
  int32 mAligned;
  int32 mBlocks16;
  int32 mRemPelBlocks;
  int32 mBlocks;
  int32 mRem16Blocks;  
  rcCorr   mRes;
  rcUIPair mRUP;
  rcCorrMomentsCached mCacheState;
};


template <class T>
rcAltiVecCorrRowFunc<T>::rcAltiVecCorrRowFunc (const rcWindow& srcA,  const rcWindow& srcB, T dummy) :
mRUP( srcA.rowUpdate () / sizeof (T), srcB.rowUpdate () / sizeof (T) ),
mCacheState( eNone )
{
  rmUnused (dummy);
  rcRowFuncTwoSource<T>::checkAssigned (srcB, srcA);
  rmAssertDebug (srcA.size() == srcB.size());
  rcRowFuncTwoSource<T>::mWidth = srcA.width();
  rcRowFuncTwoSource<T>::mHeight = srcB.height();
  rcRowFuncTwoSource<T>::mFirst = (T *) srcA.rowPointer(0);
  rcRowFuncTwoSource<T>::mSecond = (T *) srcB.rowPointer(0);
  mPermFirst = vec_lvsl(0, rcRowFuncTwoSource<T>::mFirst);
  mPermSecond = vec_lvsl(0, rcRowFuncTwoSource<T>::mSecond);
}

template <class T>
rcAltiVecCorrRowFunc<T>::rcAltiVecCorrRowFunc (const T* baseA, const T* baseB, uint32 rowPelsA, uint32 rowPelsB, 
                                               uint32 width, uint32 height) :
mRUP( rowPelsA / sizeof (T), rowPelsB / sizeof (T) ),
mCacheState( eNone )
{
  rcRowFuncTwoSource<T>::mWidth = width;
  rcRowFuncTwoSource<T>::mHeight = height;
  rcRowFuncTwoSource<T>::mFirst = baseA;
  rcRowFuncTwoSource<T>::mSecond = baseB;
  mPermFirst = vec_lvsl(0, rcRowFuncTwoSource<T>::mFirst);
  mPermSecond = vec_lvsl(0, rcRowFuncTwoSource<T>::mSecond);
}

template <class T>
rcAltiVecCorrRowFunc<T>::rcAltiVecCorrRowFunc(rcCorrelationWindow<T>& srcA, rcCorrelationWindow<T>& srcB ) :
mRUP( srcA.rowUpdate () / sizeof (T), srcB.rowUpdate () / sizeof (T) )
{
	const rcWindow& winA = srcA.window();
	const rcWindow& winB = srcB.window();
	
	rcRowFuncTwoSource<T>::checkAssigned (winB, winA);
	rmAssertDebug(winA.size() == winB.size());
	rcRowFuncTwoSource<T>::mWidth = winA.width();
	rcRowFuncTwoSource<T>::mHeight = winB.height();
	rcRowFuncTwoSource<T>::mFirst = (T *) winA.rowPointer(0);
	rcRowFuncTwoSource<T>::mSecond = (T *) winB.rowPointer(0);
	mPermFirst = vec_lvsl(0, rcRowFuncTwoSource<T>::mFirst);
	mPermSecond = vec_lvsl(0, rcRowFuncTwoSource<T>::mSecond);
	
	setCache( srcA, srcB );
}


template <class T>
void rcAltiVecCorrRowFunc<T>::setCache(const rcCorrelationWindow<T>& srcA, const rcCorrelationWindow<T>& srcB )
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
void rcAltiVecCorrRowFunc<T>::epilog (rcCorr& res)
{
	mRes.n (rcRowFuncTwoSource<T>::mWidth * rcRowFuncTwoSource<T>::mHeight);
	mRes.compute ();
	res = mRes;
}


#endif

// public

template <class T>
rcBasicCorrRowFunc<T>::rcBasicCorrRowFunc(rcCorrelationWindow<T>& srcA, rcCorrelationWindow<T>& srcB ) :
mRUP( srcA.rowUpdate () / sizeof (T), srcB.rowUpdate () / sizeof (T) )
{
	const rcWindow& winA = srcA.window();
	const rcWindow& winB = srcB.window();
	
	rcRowFuncTwoSource<T>::checkAssigned (winB, winA);
	rmAssertDebug (winA.size() == winB.size());
	rcRowFuncTwoSource<T>::mWidth = winA.width();
	rcRowFuncTwoSource<T>::mHeight = winB.height();
	rcRowFuncTwoSource<T>::mFirst = (T *) winA.rowPointer(0);
	rcRowFuncTwoSource<T>::mSecond = (T *) winB.rowPointer(0);
	
	setCache( srcA, srcB );
}

template <class T>
rcBasicCorrRowFunc<T>::rcBasicCorrRowFunc (const T* baseA, const T* baseB, uint32 rowPelsA, uint32 rowPelsB, 
                                           uint32 width, uint32 height) :
mRUP( rowPelsA / sizeof (T), rowPelsB / sizeof (T) ),
mCacheState( eNone )
{
	rcRowFuncTwoSource<T>::mWidth = width;
	rcRowFuncTwoSource<T>::mHeight = height;
	rcRowFuncTwoSource<T>::mFirst = baseA;
	rcRowFuncTwoSource<T>::mSecond = baseB;
}

template <class T>
rcBasicCorrRowFunc<T>::rcBasicCorrRowFunc (const rcWindow& srcA,  const rcWindow& srcB, T& dummy) :
mRUP( srcA.rowUpdate () / sizeof (T), srcB.rowUpdate () / sizeof (T) ),
mCacheState( eNone )
{
  rmUnused (dummy);
  rcRowFuncTwoSource<T>::checkAssigned (srcB, srcA);
  rmAssertDebug (srcA.size() == srcB.size());
  rcRowFuncTwoSource<T>::mWidth = srcA.width();
  rcRowFuncTwoSource<T>::mHeight = srcB.height();
  rcRowFuncTwoSource<T>::mFirst = (T *) srcA.rowPointer(0);
  rcRowFuncTwoSource<T>::mSecond = (T *) srcB.rowPointer(0);
}

template <class T>
void rcBasicCorrRowFunc<T>::rowFunc ()
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
void rcBasicCorrRowFunc<T>::areaFunc ()
{
	uint32 height = rcRowFuncTwoSource<T>::mHeight;
	
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
void rcBasicCorrRowFunc<T>::epilog (rcCorr& res)
{
	mRes.n (rcRowFuncTwoSource<T>::mWidth * rcRowFuncTwoSource<T>::mHeight);
	mRes.compute ();
	res = mRes;
}

template <class T>
void rcBasicCorrRowFunc<T>::prolog ()
{
}

template <class T>  
rcBasicCorrRowFunc<T>::~rcBasicCorrRowFunc<T> () {}

// private

template <class T>
void rcBasicCorrRowFunc<T>::setCache(const rcCorrelationWindow<T>& srcA, const rcCorrelationWindow<T>& srcB )
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
void rcBasicCorrRowFunc<T>::rowFuncNoneCached ()
{
  double Si(0), Sm(0), Sii(0), Smm(0), Sim(0);
  const T *pFirst (rcRowFuncTwoSource<T>::mFirst);
  const T *pSecond (rcRowFuncTwoSource<T>::mSecond);
	
  for (const T *pEnd = rcRowFuncTwoSource<T>::mFirst + rcRowFuncTwoSource<T>::mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
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
	
  rcRowFuncTwoSource<T>::mFirst += mRUP.x();
  rcRowFuncTwoSource<T>::mSecond += mRUP.y();
}

template <class T>
void rcBasicCorrRowFunc<T>::rowFuncICached ()
{
  double Sm(0), Smm(0), Sim(0);
  const T *pFirst (rcRowFuncTwoSource<T>::mFirst);
  const T *pSecond (rcRowFuncTwoSource<T>::mSecond);
	
  for (const T *pEnd = rcRowFuncTwoSource<T>::mFirst + rcRowFuncTwoSource<T>::mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
  {
		const T i = *pFirst;
		const T j = *pSecond;
		
		Sm  += j;
		Smm += j * j;
		Sim += i * j;
  }
  mRes.accumulateM (Sim, Smm, Sm);
	
  rcRowFuncTwoSource<T>::mFirst += mRUP.x();
  rcRowFuncTwoSource<T>::mSecond += mRUP.y();
}

template <class T>
void rcBasicCorrRowFunc<T>::rowFuncMCached ()
{
  double Si(0), Sii(0), Sim(0);
  const T *pFirst (rcRowFuncTwoSource<T>::mFirst);
  const T *pSecond (rcRowFuncTwoSource<T>::mSecond);
	
  for (const T *pEnd = rcRowFuncTwoSource<T>::mFirst + rcRowFuncTwoSource<T>::mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
  {
		const T i = *pFirst;
		const T j = *pSecond;
		
		Si  += i;
		Sii += i * i;
		Sim += i * j;
  }
  mRes.accumulate (Sim, Sii, Si);
	
  rcRowFuncTwoSource<T>::mFirst += mRUP.x();
  rcRowFuncTwoSource<T>::mSecond += mRUP.y();
}

template <class T>
void rcBasicCorrRowFunc<T>::rowFuncIMCached ()
{
  double Sim(0);
  const T *pFirst (rcRowFuncTwoSource<T>::mFirst);
  const T *pSecond (rcRowFuncTwoSource<T>::mSecond);
	
  for (const T *pEnd = rcRowFuncTwoSource<T>::mFirst + rcRowFuncTwoSource<T>::mWidth; pFirst < pEnd; ++pFirst, ++pSecond)
  {
		const T i = *pFirst;
		const T j = *pSecond;
		
		Sim += i * j;
  }
  mRes.accumulate (Sim);
	
  rcRowFuncTwoSource<T>::mFirst += mRUP.x();
  rcRowFuncTwoSource<T>::mSecond += mRUP.y();
}


// Pixel Map row functions

template <class T>
rcBasicPixelMap<T>::rcBasicPixelMap (const rcWindow& srcA,  const rcWindow& srcB,
																		 const vector<T>& lut) :
mRUP( srcA.rowUpdate () / sizeof (T), srcB.rowUpdate () / sizeof (T) )
{
  rcRowFuncOneSourceOneDestination <T>::checkAssigned (srcB, srcA);
  rmAssertDebug (srcA.size() == srcB.size());
  rcRowFuncOneSourceOneDestination<T>::mWidth = srcA.width();
  rcRowFuncOneSourceOneDestination<T>::mHeight = srcB.height();
  rcRowFuncOneSourceOneDestination<T>::mFirst = (T *) srcA.rowPointer(0);
  rcRowFuncOneSourceOneDestination<T>::mSecond = (T *) srcB.rowPointer(0);
  mLut = (T *) (& lut[0]);
}


template <class T>
void rcBasicPixelMap<T>::areaFunc ()
{
	uint32 height = rcRowFuncOneSourceOneDestination<T>::mHeight;
	do { rowFunc(); } while (--height);
}

template <class T>  
rcBasicPixelMap<T>::~rcBasicPixelMap<T> () {}
template <class T>  
void rcBasicPixelMap<T>::prolog () {}


#endif /* __rcROWFUNC_H */
