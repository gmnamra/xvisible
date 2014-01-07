/*
  File:		vBigDSP.c
  
  Contains:	AltiVec-based Implementation of DSP routines (real &
                complex FFT, convolution)

  Version:	1.0

  Copyright:    1999 by Apple Computer, Inc., all rights reserved.

  Change History (most recent first):
                03/22/03	GEA		Updated for Linux and Darwin
		10/12/99	JK		Created

*/

/////////////////////////////////////////////////////////////////////////////////
//      File Name: vBigDSP.c                                                   //
// 				    					       //
//    This library provides a set of DSP routines, implemented using the       //
//    AltiVec instruction set. 						       //
//									       //
//                                                                             //
//    Copyright 1999 Apple Computer, Inc.  All rights reserved.                //
//                                                                             //
//      Version 1.0                                                            //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

/*
  Disclaimer: IMPORTANT: This Apple software is supplied to you by
  Apple Computer, Inc.  ("Apple") in consideration of your agreement
  to the following terms, and your use, installation, modification or
  redistribution of this Apple software constitutes acceptance of
  these terms.  If you do not agree with these terms, please do not
  use, install, modify or redistribute this Apple software.
	
  In consideration of your agreement to abide by the following terms,
  and subject to these terms, Apple grants you a personal,
  non-exclusive license, under Apple's copyrights in this original
  Apple software (the "Apple Software"), to use, reproduce, modify and
  redistribute the Apple Software, with or without modifications, in
  source and/or binary forms; provided that if you redistribute the
  Apple Software in its entirety and without modifications, you must
  retain this notice and the following text and disclaimers in all
  such redistributions of the Apple Software.  Neither the name,
  trademarks, service marks or logos of Apple Computer, Inc. may be
  used to endorse or promote products derived from the Apple Software
  without specific prior written permission from Apple.  Except as
  expressly stated in this notice, no other rights or licenses,
  express or implied, are granted by Apple herein, including but not
  limited to any patent rights that may be infringed by your
  derivative works or by other works in which the Apple Software may
  be incorporated.
	
  The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
  MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT
  LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
  APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH
  YOUR PRODUCTS.
	
  IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT,
  INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
  USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE
  SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
  (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE
  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <errno.h>
#include <math.h>
#include "vbigdsp.h"
#include <Accelerate/Accelerate.h>

#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif


// This hideous little sequence is the key to portable code.
#if defined(OS_LINUXPPC) && (__GNUC__==3)
// the new gcc-3.2 way
#include <altivec.h>
#define Altivec_Const4(type,e0,e1,e2,e3)	\
  ((type){e0,e1,e2,e3})
#define Altivec_Const8(type,e0,e1,e2,e3,e4,e5,e6,e7)	\
  ((type){e0,e1,e2,e3,e4,e5,e6,e7})
#define Altivec_Const16(type,e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,ea,eb,ec,ed,ee,ef) \
  ((type){e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,ea,eb,ec,ed,ee,ef})
#else
#define Altivec_Const4(type,e0,e1,e2,e3)	\
  ((type)(e0,e1,e2,e3))
#define Altivec_Const8(type,e0,e1,e2,e3,e4,e5,e6,e7)	\
  ((type)(e0,e1,e2,e3,e4,e5,e6,e7))
#define Altivec_Const16(type,e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,ea,eb,ec,ed,ee,ef) \
  ((type)(e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,ea,eb,ec,ed,ee,ef))
#ifdef OS_LINUXPPC 
#define vector __vector
#endif
#endif


////////////////////////////////////////////////////////////////////////////////
//	temporary workspace buffer for FFT implementations 
////////////////////////////////////////////////////////////////////////////////
static float*    gTempBufferPtr = 0;

////////////////////////////////////////////////////////////////////////////////
//	current buffer length for above pointer
////////////////////////////////////////////////////////////////////////////////
static int32   gTempBufferSize = 0;

////////////////////////////////////////////////////////////////////////////////
//	table of pre-calculated sin & cos values for use by ping pong FFT
////////////////////////////////////////////////////////////////////////////////
static float*    gSinCosTablePtr = 0;

////////////////////////////////////////////////////////////////////////////////
//	length of currently allocated sin & cos table
////////////////////////////////////////////////////////////////////////////////
static int32   gSinCosTableSize = 0;

////////////////////////////////////////////////////////////////////////////////
//
//	log2max
//
//	This function computes the ceiling of log2(n).  For example:
//
//	log2max(7) = 3
//	log2max(8) = 3
//	log2max(9) = 4
//
////////////////////////////////////////////////////////////////////////////////
static int32 log2max(int32 n)
{
  int32 power = 1;
  int32 k = 1;
  
  if (n==1) {
    return 0;
  }
	
  while ((k <<= 1) < n) {
    power++;
  }
	
  return power;
}


////////////////////////////////////////////////////////////////////////////////
//
//	EnsureStaticBufferSize
//
// This function is used to ensure that the global pointer gTempBufferPtr
// is large enough to hold complexCount complex floats.  It compares the
// currently allocated length (if there is one) with the desired length,
// and, if necessary, allocates a new, larger pointer.
// It will NOT shrink the pointer, because a function higher in the calling
// chain may need the larger size.
////////////////////////////////////////////////////////////////////////////////
static int EnsureStaticBufferSize(int32 complexCount)
{
  int32 result = 0;
	
  if (gTempBufferSize < complexCount) {
    gTempBufferSize = complexCount;
	
    if (gTempBufferPtr) {
      free(gTempBufferPtr);
      gTempBufferPtr = 0;
    }
		
    gTempBufferPtr = (float*) valloc(2*complexCount*sizeof(float));
		
    if (!gTempBufferPtr) result = ENOMEM;
    if (result != 0) {
      gTempBufferPtr = 0;
      gTempBufferSize = 0;
    }
  }
  
  return result;
}


/////////////////////////////////////////////////////////////////////////////////
// InitFFTSinCos
// Initialize cos & sin lookup table.
/////////////////////////////////////////////////////////////////////////////////
static int InitFFTSinCos(int32 len) {
  int32 result = 0;
  int32 i;
  float   angle;
  float	  *sinCosTable;
  float	  curSin;
  float	  curCos;
  int32 pointerSize;
	
  ////////////////////////////////////////////////////////////////////////////////
  // set length indicator to new length
  ////////////////////////////////////////////////////////////////////////////////
  gSinCosTableSize = len;
	
  ////////////////////////////////////////////////////////////////////////////////
  // deallocate old pointer if there is one
  ////////////////////////////////////////////////////////////////////////////////
  if(gSinCosTablePtr) free(gSinCosTablePtr);
  
  ////////////////////////////////////////////////////////////////////////////////
  // calculate appropriate pointer size for table.  Ensure a minimum length for
  //  a one-entry (two-value: cos & sin) table
  ////////////////////////////////////////////////////////////////////////////////
  pointerSize = sizeof(float) * len;
  
  if (!pointerSize) pointerSize = 2 * sizeof(float);
	
  ////////////////////////////////////////////////////////////////////////////////
  // alloc new pointer for sin & cos table
  ////////////////////////////////////////////////////////////////////////////////
  gSinCosTablePtr = (float*) valloc(pointerSize);

  if (!gSinCosTablePtr) result = ENOMEM;
	
  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////////
    // get pointer to start of sin cos table
    // to reference as array
    /////////////////////////////////////////////////////////////////////////////
    sinCosTable = (float*)gSinCosTablePtr;

    if (len < 4) {
      //////////////////////////////////////////////////////////////////////////
      // always create first entries in table
      //////////////////////////////////////////////////////////////////////////
      sinCosTable[0] = 1;
      sinCosTable[1] = 0;
			
      /////////////////////////////////////////////////////////////////////////
      // if len is 2, then add second pair to table
      /////////////////////////////////////////////////////////////////////////
      if (len == 2) {
	sinCosTable[2] = -1;
	sinCosTable[3] = 0;
      }
    } else {
      /////////////////////////////////////////////////////////////////////////
      // calculate cos & sin for range of 
      // [0, pi].
      /////////////////////////////////////////////////////////////////////////
      for(i=0; i < len/4; i++) {
	angle = 2.0 * PI * i/len;
	
	curCos = cos(angle);
	curSin = sin(angle);
	
	sinCosTable[2*i] = curCos;
	sinCosTable[2*i+1] = curSin;
	
	sinCosTable[len/2 + 2*i] = -curSin;
	sinCosTable[len/2 + 2*i + 1] = curCos;
      }
    }
  }
  
  /////////////////////////////////////////////////////////////////////////////
  // if there was an error, then dealloc
  // anything we allocated, and reset
  // length indicator to zero.
  /////////////////////////////////////////////////////////////////////////////
  if (result != 0) {
    if (gSinCosTablePtr != 0) {
      free(gSinCosTablePtr);
      gSinCosTablePtr = 0;
    }		
    
    gSinCosTableSize = 0;
  }
  
  return result;
  
}


///////////////////////////////////////////////////////////////////////////////
// ShutdownFFT
//
// deallocates any memory curently allocated for fft buffers.
///////////////////////////////////////////////////////////////////////////////
void ShutdownFFT()
{
  if (gSinCosTablePtr != 0) {
    free(gSinCosTablePtr);
    gSinCosTablePtr = 0;
  }

  gSinCosTableSize = 0;		

  if (gTempBufferPtr != 0) {
    free(gTempBufferPtr);
    gTempBufferPtr = 0;	
  }
  
  gTempBufferSize = 0;
}


#ifndef __ppc__

/////////////////////////////////////////////////////////////////////////////
// scalar implementation of altivec forward real fft above
/////////////////////////////////////////////////////////////////////////////
static int32 fft_real_forward_scalar(float *data, int32 len)
{
  int32 result = 0;
  double  updateA, updateB;

  double  currentCos, currentSin, tempCos1;
	
  float   realLo, realHi, imLo, imHi;
	
  float   realResultLo, realResultHi, imResultLo, imResultHi;
	
  float   realDiff, imDiff, realSum, imSum;
		
  float   *pInLo, *pInHi;
	
  int32 i;
	
  ///////////////////////////////////////////////////////////////////////////
  // for length of 0 or 1 we do nothing
  ///////////////////////////////////////////////////////////////////////////
  if (len < 2) return 0;
		
  ///////////////////////////////////////////////////////////////////////////
  // perform a forward complex fft on our
  // real signal data, treating it as a
  // half-length complex signal.
  ///////////////////////////////////////////////////////////////////////////
  result = FFTComplex(data, len/2, -1);

  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////
    //
    //	Given c = cos(w) and s = sin(w), if we
    //	want to find the cos and sin of w plus
    //	some small angle d, then we can do so
    //	by defining:
    //	
    //	a = 2 * ((sin(d/2)) ^ 2)
    //	b = sin(d)
    //	
    //	Then, we can calculate the cos and sin
    //	of the updated angle w+d as:
    //	
    //	cos(w+d) = c - ac - bs
    //	sin(w+d) = s - as + bc
    //	
    //	We will need to incrementally calculate
    //	cos(w) and sin(w), so we define the
    //	following scalar values.  We use scalar
    //	because we need the precision of doubles,
    //	and vectors are floats.	
    //
    /////////////////////////////////////////////////////////////////////////
				
    updateA = sin(PI/len);
    updateB = sin(2*PI/len);
    updateA *= updateA*2;

    /////////////////////////////////////////////////////////////////////////
    // start with sin(0) and cos(0)
    /////////////////////////////////////////////////////////////////////////

    currentCos = 1;
    currentSin = 0;	
	
    /////////////////////////////////////////////////////////////////////////
    // first, calculate re[0], and re[n/2]. we don't bother to do the
    // sin, cos multiplies, because we know that they are zero and one.
    /////////////////////////////////////////////////////////////////////////
				
    realResultLo = data[0]+data[1];
    realResultHi = data[0]-data[1];
		
    /////////////////////////////////////////////////////////////////////////
    // store re[0].  We don't calculate im[0] because we know that it is 0.
    /////////////////////////////////////////////////////////////////////////
    data[0] = realResultLo;

    /////////////////////////////////////////////////////////////////////////
    // store re[n/2].  We don't calculate im[n/2] because we know it
    // is 0.  We store re[n/2] in the position of im[0] because we
    // don't want to take up any more space than the source data, and
    // if we were to store it in the n/2 position, it would be past
    // the array of source data, which ranges from 0 to (n/2)-1.
    /////////////////////////////////////////////////////////////////////////
    data[1] = realResultHi;
		
    /////////////////////////////////////////////////////////////////////////
    // Start our source pointers to point at re[1] and re[(n/2)-1].
    /////////////////////////////////////////////////////////////////////////
    pInLo = &data[2];
    pInHi = &data[len-2];

    /////////////////////////////////////////////////////////////////////////
    // for each loop iteration, we calculate two complex results (four
    // floats), so we iterate through the loop len/4 times
    /////////////////////////////////////////////////////////////////////////
    for (i = 0; i < len/4; i++) {
      ///////////////////////////////////////////////////////////////////////
      // calculate the new sin and cos values
      ///////////////////////////////////////////////////////////////////////
		
      tempCos1   = currentCos - (updateA*currentCos  + updateB * currentSin);
      currentSin = currentSin - (updateA*currentSin  - updateB * currentCos);
      currentCos = tempCos1;

      ///////////////////////////////////////////////////////////////////////
      // load re[i+1], im[i+1]
      ///////////////////////////////////////////////////////////////////////
      realLo      = *pInLo;
      imLo        = *(pInLo+1);

      ///////////////////////////////////////////////////////////////////////
      // load re[(n/2)-1-i], im[(n/2)-1-i] 
      ///////////////////////////////////////////////////////////////////////
      realHi      = *pInHi;
      imHi        = *(pInHi+1);
			
      ///////////////////////////////////////////////////////////////////////
      // calculate:
      // realDiff	= re[i+1]-re[(n/2)-1-i]
      // realSum	= re[i+1]+re[(n/2)-1-i]
      ///////////////////////////////////////////////////////////////////////
      realDiff    = realLo-realHi;
      realSum     = realLo+realHi;

      ///////////////////////////////////////////////////////////////////////
      // calculate:
      // imDiff	= im[i+1]-im[(n/2)-1-i]
      // imSum	= im[i+1]+im[(n/2)-1-i]
      ///////////////////////////////////////////////////////////////////////
      imDiff      = imLo-imHi;
      imSum       = imLo+imHi;
			
      ///////////////////////////////////////////////////////////////////////
      // calculate:
      //
      // realResultLo = 	
      //	re[i+1]+re[(n/2)-1-i] + 
      // 	(im[i+1]+im[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
      //	(re[i+1]-re[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
      //
      // imResultLo = 	
      //	im[i+1]-im[(n/2)-1-i] - 
      // 	(re[i+1]-re[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
      //	(im[i+1]+im[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
      //
      ///////////////////////////////////////////////////////////////////////
			
      realResultLo = realSum + (imSum*currentCos) - (realDiff*currentSin);
      imResultLo   = imDiff - (realDiff * currentCos) - (imSum*currentSin);
			
      ///////////////////////////////////////////////////////////////////////
      // calculate:
      //
      // realResultHi = 	
      //	re[(n/2)-1-i]+re[i+1] +
      // 	(im[(n/2)-1-i]+im[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
      //	(re[(n/2)-1-i]-re[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
      //
      // imResultHi = 	
      //	im[(n/2)-1-i]-im[i+1] - 
      // 	(re[(n/2)-1-i]-re[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
      //	(im[(n/2)-1-i]+im[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
      //
      // taking advantage of the following:
      //
      // sin(  (i+1) * 2pi / n ) =  sin ( ((n/2)-i-1) * 2pi / n )
      // cos(  (i+1) * 2pi / n ) = -cos ( ((n/2)-i-1) * 2pi / n )
      // re[(n/2)-1-i]-re[i+1] 	 = -(re[i+1]-re[(n/2)-1-i])
      // re[(n/2)-1-i]-re[i+1] 	 = -(re[i+1]-re[(n/2)-1-i])
      ///////////////////////////////////////////////////////////////////////
			
      realResultHi = realSum - (imSum*currentCos) + (realDiff*currentSin);
      imResultHi = -imDiff - (realDiff * currentCos) - (imSum*currentSin);

      ///////////////////////////////////////////////////////////////////////
      // store results, advance low pointer forward to next complex element, 
      // and high pointer backward to previous complex element
      ///////////////////////////////////////////////////////////////////////
			
      *pInLo++ = realResultLo/2;		
      *pInLo++ = imResultLo/2;		
      *(pInHi+1) = imResultHi/2;		
      *pInHi = realResultHi/2;
					
      pInHi -= 2;
    }
  }
		
  return result;	
}

/////////////////////////////////////////////////////////////////////////////
// scalar implementation of altivec inverse real fft above
/////////////////////////////////////////////////////////////////////////////
static int32 fft_real_inverse_scalar(float *data, int32 len)
{
  int32 result = 0;
  double  updateA, updateB;

  double  currentCos, currentSin, tempCos1;
	
  float   realLo, realHi, imLo, imHi;
	
  float   realResultLo, realResultHi, imResultLo, imResultHi;
	
  float   realDiff, imDiff, realSum, imSum;
		
  float   *pInLo, *pInHi;
	
  int32 i;

  ///////////////////////////////////////////////////////////////////////////
  // for length of 0 or 1 we do nothing
  ///////////////////////////////////////////////////////////////////////////
  if (len < 2) return 0;

  ///////////////////////////////////////////////////////////////////////////
  // Given c = cos(w) and s = sin(w), if we want to find the cos and
  // sin of w plus some small angle d, then we can do so by defining:
  //	
  // a = 2 * ((sin(d/2)) ^ 2)
  // b = sin(d)
  // 
  // Then, we can calculate the cos and sin of the updated angle w+d
  // as:
  // 
  // cos(w+d) = c - ac - bs
  // sin(w+d) = s - as + bc
  // 
  // We will need to incrementally calculate cos(w) and sin(w), so we
  // define the following scalar values.  We use scalar because we
  // need the precision of doubles, and vectors are floats.
  ///////////////////////////////////////////////////////////////////////////
			
  updateA = sin(PI/len);
  updateB = sin(2*PI/len);
  updateA *= updateA*2;

  ///////////////////////////////////////////////////////////////////////////
  // start with sin(0) and cos(0)
  ///////////////////////////////////////////////////////////////////////////

  currentCos = 1;
  currentSin = 0;	

  ///////////////////////////////////////////////////////////////////////////
  // load re[0] and im[0].  Only re[0] is stored -- im[0] is
  // understood to be 0.
  ///////////////////////////////////////////////////////////////////////////
  realLo   = data[0];
  imLo     = 0;

  ///////////////////////////////////////////////////////////////////////////
  // load re[n/2] and im[n/2].  
  // Only re[n/2] is stored -- im[n/2] is understood to be 0.
  ///////////////////////////////////////////////////////////////////////////
  realHi    = data[1];
  imHi      = 0;				

  ///////////////////////////////////////////////////////////////////////////
  // calculat sum and difference of real & imaginary values
  ///////////////////////////////////////////////////////////////////////////

  realDiff  = realLo-realHi;
  realSum   = realLo+realHi;

  imDiff    = imLo-imHi;
  imSum     = imLo+imHi;

  ///////////////////////////////////////////////////////////////////////////
  // calculate re[0], and im[0]. we don't bother to do the sin, cos
  // multiplies, because we know that they are zero and one.
  ///////////////////////////////////////////////////////////////////////////

  realResultLo = realSum - imSum;
  imResultLo = imDiff + realDiff;

  data[0] = realResultLo/2;
  data[1] = imResultLo/2;

  ///////////////////////////////////////////////////////////////////////////
  // Start our source pointers to point at re[1] and re[(n/2)-1].
  ///////////////////////////////////////////////////////////////////////////
  pInLo = &data[2];
  pInHi = &data[len-2];
	
  ///////////////////////////////////////////////////////////////////////////
  // for each loop iteration, we calculate two complex results (four
  // floats), so we iterate through the loop len/4 times
  ///////////////////////////////////////////////////////////////////////////
  for (i = 0; i < len/4; i++) {
    /////////////////////////////////////////////////////////////////////////
    // calculate the new sin and cos values
    /////////////////////////////////////////////////////////////////////////
    tempCos1 	= currentCos - (updateA*currentCos  + updateB * currentSin);
    currentSin 	= currentSin - (updateA*currentSin  - updateB * currentCos);
    currentCos 	= tempCos1;

    /////////////////////////////////////////////////////////////////////////
    // load re[i+1], im[i+1]
    /////////////////////////////////////////////////////////////////////////
    realLo	= *pInLo;
    imLo 	= *(pInLo+1);

    /////////////////////////////////////////////////////////////////////////
    // load re[(n/2)-1-i], im[(n/2)-1-i] 
    /////////////////////////////////////////////////////////////////////////
    realHi 	= *pInHi;
    imHi	= *(pInHi+1);
		
    /////////////////////////////////////////////////////////////////////////
    // calculate:
    // realDiff	= re[i+1]-re[(n/2)-1-i]
    // realSum	= re[i+1]+re[(n/2)-1-i]
    /////////////////////////////////////////////////////////////////////////
    realDiff 	= realLo-realHi;
    realSum 	= realLo+realHi;

    /////////////////////////////////////////////////////////////////////////
    // calculate:
    // imDiff	= im[i+1]-im[(n/2)-1-i]
    // imSum	= im[i+1]+im[(n/2)-1-i]
    /////////////////////////////////////////////////////////////////////////
    imDiff 	= imLo-imHi;
    imSum 	= imLo+imHi;
		
    /////////////////////////////////////////////////////////////////////////
    // calculate:
    //
    // realResultLo = 	
    //		re[i+1]+re[(n/2)-1-i] - 
    // 		(im[i+1]+im[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
    //		(re[i+1]-re[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
    //
    // imResultLo = 	
    //		im[i+1]-im[(n/2)-1-i] + 
    // 		(re[i+1]-re[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
    //		(im[i+1]+im[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
    //
    /////////////////////////////////////////////////////////////////////////
    realResultLo = realSum - (imSum*currentCos) - (realDiff*currentSin);
    imResultLo = imDiff + (realDiff * currentCos) - (imSum*currentSin);

    /////////////////////////////////////////////////////////////////////////
    // calculate:
    //
    // realResultHi = 	
    //		re[(n/2)-1-i]+re[i+1] -
    // 		(im[(n/2)-1-i]+im[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
    //		(re[(n/2)-1-i]-re[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
    //
    // imResultHi = 	
    //		im[(n/2)-1-i]-im[i+1] + 
    // 		(re[(n/2)-1-i]-re[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
    //		(im[(n/2)-1-i]+im[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
    //
    // taking advantage of the following:
    //
    // sin(  (i+1) * 2pi / n )	=  sin ( ((n/2)-i-1) * 2pi / n )
    // cos(  (i+1) * 2pi / n )	= -cos ( ((n/2)-i-1) * 2pi / n )
    // re[(n/2)-1-i]-re[i+1] 	= -(re[i+1]-re[(n/2)-1-i])
    // re[(n/2)-1-i]-re[i+1] 	= -(re[i+1]-re[(n/2)-1-i])
    /////////////////////////////////////////////////////////////////////////

    realResultHi = realSum + (imSum*currentCos) + (realDiff*currentSin);
    imResultHi = -imDiff + (realDiff * currentCos) - (imSum*currentSin);

    /////////////////////////////////////////////////////////////////////////
    // store results, advance low pointer forward to next complex
    // element, and high pointer backward to previous complex element
    /////////////////////////////////////////////////////////////////////////

    *pInLo++ = realResultLo/2;		
    *pInLo++ = imResultLo/2;		
    *(pInHi+1) = imResultHi/2;		
    *pInHi = realResultHi/2;		

    pInHi -= 2;
  }

  /////////////////////////////////////////////////////////////////////////
  // perform an inverse complex fft on the data.  The resulting data
  // is the real-only inverse fft.
  /////////////////////////////////////////////////////////////////////////
  result = FFTComplex(data, len/2, 1);

  return result;	
}

/////////////////////////////////////////////////////////////////////////////
// FFTRealForward
//
// Performs a real forward FFT on the data.  A wrapper function for
// the scalar and AltiVec versions of the forward real FFT, since
// there is a minimum length for the AltiVec implementation.
/////////////////////////////////////////////////////////////////////////////
int32 FFTRealForward(float *data, int32 len)
{
   return fft_real_forward_scalar(data, len);
}

/////////////////////////////////////////////////////////////////////////////
// FFTRealInverse
//
// Performs a real inverse FFT on the data.  A wrapper function for
// the scalar and AltiVec versions of the inverse real FFT, since
// there is a minimum length for the AltiVec implementation.
/////////////////////////////////////////////////////////////////////////////
int32 FFTRealInverse(float *data, int32 len)
{
   return fft_real_inverse_scalar(data, len);
}

/////////////////////////////////////////////////////////////////////////////
// fft_scalar
//
// Ping-pong Stockham FFT
//
//	Performs a forward or inverse FFT on the complex signal data
// pointed to by pData. pTempBuffer must point to a buffer that is of
// equal length as the signal pointed to by pData, and which will be
// overwritten by the routine.  If isign == -1, then a forward FFT is
// performed.  Otherwise an inverse FFT is performed.
//
// requirements:
//
//	- length must be an exact power of 2 
//
/////////////////////////////////////////////////////////////////////////////
static int32 fft_scalar(float *pData, float *tempbuff, int32 len,
			  int32 isign)
{
  int32  j, i;
  float    c, s, tre, tim, *srcDataPtr, *dstDataPtr, *tmp;
  double   inverseDivideMultiplier;
  int32  pow, root, trig;
  int32  result = 0;
  float	   *sinCosTable;

  ///////////////////////////////////////////////////////////////////////////
  // calculate log2(len)
  ///////////////////////////////////////////////////////////////////////////
	
  pow = log2max(len);
	
  ///////////////////////////////////////////////////////////////////////////
  // length must be an exact power of 2	
  ///////////////////////////////////////////////////////////////////////////
	
  if ((1 << pow) != len) return EINVAL;

  ///////////////////////////////////////////////////////////////////////////
  // make sure that the sin cos lookup table is for the current fft length
  ///////////////////////////////////////////////////////////////////////////
  if (len != gSinCosTableSize) result = InitFFTSinCos(len);

  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////
    // initialize "trig" step for destination index
    /////////////////////////////////////////////////////////////////////////

    trig = 1;
		
    /////////////////////////////////////////////////////////////////////////
    // Start with data as source, and temp buffer as destination
    /////////////////////////////////////////////////////////////////////////

    srcDataPtr = pData;
    dstDataPtr = tempbuff;

    /////////////////////////////////////////////////////////////////////////
    // get pointer to start of sin/cos table pointer for array reference
    /////////////////////////////////////////////////////////////////////////
		
    sinCosTable = (float*)gSinCosTablePtr;

    for(i = pow-1; i > 0; i--) {

      root = 0;
      while(root < len/2) {
	/////////////////////////////////////////////////////////////////////
	// load cos and sin values for current root value.
	/////////////////////////////////////////////////////////////////////

	c = sinCosTable[2*root];
	s = isign*sinCosTable[2*root+1];

	for(j = trig; j > 0; j--) {
	  ///////////////////////////////////////////////////////////////////
	  // calculate butterflies.  For inputs:
	  // [re0, im0] [re1, im1]
	  //
	  // we calculate:
	  //
	  // out0 = [re0+re1, im0+im1]
	  //
	  // out1 = [ cos * (re0-re1) - sin * (im0-im1),
	  //			sin * (re0-re1) + cos * (im0-im1) ]
	  //
	  ///////////////////////////////////////////////////////////////////
					
	  tre = srcDataPtr[0] - srcDataPtr[len];
	  tim = srcDataPtr[1] - srcDataPtr[len + 1];
	  dstDataPtr[0] = srcDataPtr[0] + srcDataPtr[len];
	  dstDataPtr[1] = srcDataPtr[1] + srcDataPtr[len+1];
	  dstDataPtr[2*trig] = c*tre - s*tim;
	  dstDataPtr[2*trig + 1] = s*tre + c*tim;
	  srcDataPtr += 2; dstDataPtr += 2;
	}

	///////////////////////////////////////////////////////////////////
	// update dest pointer and root value
	///////////////////////////////////////////////////////////////////
				
	dstDataPtr += 2*trig;
	root += trig;
      }

      /////////////////////////////////////////////////////////////////////
      // update trig value and ping pong source and dest pointers
      /////////////////////////////////////////////////////////////////////

      trig *= 2; srcDataPtr -= len; dstDataPtr -= 2*len;
      tmp = srcDataPtr; srcDataPtr = dstDataPtr; dstDataPtr = tmp; 

    }

    ///////////////////////////////////////////////////////////////////////
    // For last iteration, we are fortunate in that the source indices
    // for the butterfly calculations are the same as the destination
    // indices. This is not true for other loop iterations, which is
    // why we cannot simply store our results directly back to the
    // data -- we would overwrite source data that had not yet been
    // used for the current iteration of the loop. If the power of two
    // of the length is odd, then, on the second to last iteration,
    // the data will end up ping-ponged back to the source buffer.  If
    // this is the case, then we set source and dest to be the
    // same. Otherwise, if the power of two is even, then we will be
    // reading from temp data, and writing back to our original
    // buffer.
    ///////////////////////////////////////////////////////////////////////

    if (pow & 1) {
      dstDataPtr = srcDataPtr;
    }
	
    root = 0;
		
    if (isign > 0) {
      /////////////////////////////////////////////////////////////////////
      // we are performing an inverse FFT.  We need to divide all
      // elements of the final result by len.  Since the float
      // multiply operation is faster than the divide operation, we
      // multiply by the reciprocal.
      /////////////////////////////////////////////////////////////////////
      inverseDivideMultiplier = (double)1/len;
			
      while(root < len/2) {

	///////////////////////////////////////////////////////////////////
	// load cos and sin values for current root value.
	///////////////////////////////////////////////////////////////////

	c = sinCosTable[2*root];
	s = isign*sinCosTable[2*root+1];

	for(j = trig; j > 0; j--) {
	  /////////////////////////////////////////////////////////////////
	  // calculate butterflies.  For inputs:
	  // [re0, im0] [re1, im1]
	  //
	  // we calculate:
	  //
	  // out0 = [re0+re1, im0+im1]
	  //
	  // out1 = [ cos * (re0-re1) - sin * (im0-im1),
	  //			sin * (re0-re1) + cos * (im0-im1) ]
	  //
	  //
	  // We also divide each element by the length of the signal
	  // (this is done by multiplying by the reciprocal).
	  /////////////////////////////////////////////////////////////////
					
	  tre = srcDataPtr[0] - srcDataPtr[len];
	  tim = srcDataPtr[1] - srcDataPtr[len + 1];
	  dstDataPtr[0] = (srcDataPtr[0] + srcDataPtr[len])*inverseDivideMultiplier;
	  dstDataPtr[1]=(srcDataPtr[1] + srcDataPtr[len+1])*inverseDivideMultiplier;
	  dstDataPtr[2*trig] = (c*tre - s*tim)*inverseDivideMultiplier;
	  dstDataPtr[2*trig + 1] = (s*tre + c*tim)*inverseDivideMultiplier;
	  srcDataPtr += 2; dstDataPtr += 2;
	}

	///////////////////////////////////////////////////////////////////
	// update dest pointer and root value
	///////////////////////////////////////////////////////////////////
				
	dstDataPtr += 2*trig;
	root += trig;

      }
    } else {
      while(root < len/2) {
	///////////////////////////////////////////////////////////////////
	// load cos and sin values for current root value.
	///////////////////////////////////////////////////////////////////

	c = sinCosTable[2*root];
	s = isign*sinCosTable[2*root+1];

	for(j = trig; j > 0; j--) {
	  /////////////////////////////////////////////////////////////////
	  // calculate butterflies.  For inputs:
	  // [re0, im0] [re1, im1]
	  //
	  // we calculate:
	  //
	  // out0 = [re0+re1, im0+im1]
	  //
	  // out1 = [ cos * (re0-re1) - sin * (im0-im1),
	  //			sin * (re0-re1) + cos * (im0-im1) ]
	  //
	  /////////////////////////////////////////////////////////////////
					
	  tre = srcDataPtr[0] - srcDataPtr[len];
	  tim = srcDataPtr[1] - srcDataPtr[len + 1];
	  dstDataPtr[0] = srcDataPtr[0] + srcDataPtr[len];
	  dstDataPtr[1] = srcDataPtr[1] + srcDataPtr[len+1];
	  dstDataPtr[2*trig] = c*tre - s*tim;
	  dstDataPtr[2*trig + 1] = s*tre + c*tim;
	  srcDataPtr += 2; dstDataPtr += 2;
	}

	///////////////////////////////////////////////////////////////////
	// update dest pointer and root value
	///////////////////////////////////////////////////////////////////
				
	dstDataPtr += 2*trig;
	root += trig;
      }
    }
  }

  return result;    
}

/////////////////////////////////////////////////////////////////////////////
//
//	fft_pingpong
//
// This function calls either the scalar or vector implementation of
// the pingpong fft, based on the length of the data.  The vector
// implementation only works above certain lengths because of
// implementation details.
/////////////////////////////////////////////////////////////////////////////
static int32 fft_pingpong(float *data, int32 len, int32 isign)
{
  int32 result = 0;

  ///////////////////////////////////////////////////////////////////////////
  // make sure that our temp buffer is sufficiently large to hold a copy of
  // the data.
  ///////////////////////////////////////////////////////////////////////////
  result = EnsureStaticBufferSize(len);
	
  if (result == 0) {
      ///////////////////////////////////////////////////////////////////////
      // call scalar version
      ///////////////////////////////////////////////////////////////////////
      result = fft_scalar(data, (float*)gTempBufferPtr, len, isign);
    } 

  return result;
}



////////////////////////////////////////////////////////////////////////////
//	FFTComplex
//
//	Performs a forward or inverse complex FFT on the data.  This
// is a wrapper function for three different FFT routines.  If the
// length is below the breakover to call the recursive FFT, then it
// calls the pingpong FFT.  If it's above the point at which the
// recursive FFT is faster, then it calls one of the recursive FFTs.
// If this is the case, then it chooses between two forms of
// recursion.  For even powers of two, it calls the matrix FFT (which
// can only be performed on even powers of two because it allows for a
// square matrix, which can be easily transposed).  If the length is
// an odd power of two, then a one-step recursive FFT is called.
/////////////////////////////////////////////////////////////////////////////
int32 FFTComplex(float *data, int32 length, int32 isign)
{
  int32 result = 0;
	
  if (length <= PINGPONG_FFT_MAXLEN) {
    /////////////////////////////////////////////////////////////////////////
    // length is in the range where pingpong FFT is fastest
    /////////////////////////////////////////////////////////////////////////
    result = fft_pingpong(data, length, isign);	
  } 
 else {
	rmAssert (0);
  }
  
  return result;
}



#else

#pragma mark -
#pragma mark R E C U R S I V E   C O M P L E X

////////////////////////////////////////////////////////////////////////////////
//	SquareComplexTransposeVector performs a transpose on a square matrix of
// 	complex floats with side length of rowLength.  Data is assumed to be
//	arranged lexicographically, i.e., with side length n:
//
//	re[0] im[0]	re[1] im[1] ... re[n-1] im[n-1]
//	re[n] im[n]		...			re[2n-1] im[2n-1]
//	.
//	.
//	re[(n-1)*n] im[(n-1)*n]	...	re[n^2-1] im[n^2-1]
//	
//	Data becomes:
//
//	re[0] im[0] re[n] im[n] re[2n] im[2n]...
//	re[1] im[1] re[n+1] im[n+1] ...
//	re[2] im[2]	...
//	.
//	.
//	.
//	re[n-1] im[n-1]	...		re[n^2-1] im[n^2-1]
//
//
////////////////////////////////////////////////////////////////////////////////

static void SquareComplexTransposeVector(float *data, int32 rowLength)
{
  int32               i, j;
  vector float          *pInLeft1, *pInLeft2;
  vector float          *pInTop1, *pInTop2;
  vector float          vInLeft1, vInLeft2;
  vector float          vInTop1, vInTop2;
  vector float          vOutLeft1, vOutLeft2;
  vector float          vOutTop1, vOutTop2;
  vector unsigned char  vMergeHiPairPerm;
  vector unsigned char  vMergeLoPairPerm;		

  ////////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x1 y0 y1
  ////////////////////////////////////////////////////////////////////////////
  vMergeHiPairPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5,
				     6, 7, 16, 17, 18, 19, 20, 21, 22, 23);

  ////////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x2 x3 y2 y3
  ////////////////////////////////////////////////////////////////////////////
  vMergeLoPairPerm = Altivec_Const16(vector unsigned char, 8, 9, 10, 11, 12,
				     13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);

  for (i = 0; i < rowLength/2; i++) {
    /////////////////////////////////////////////////////////////////////////
    // set pointers to point to the beginning of row 2i and 2i+1
    /////////////////////////////////////////////////////////////////////////
    pInLeft1 = ((vector float*)data) + i * rowLength;
    pInLeft2 = pInLeft1 + rowLength / 2;
		
    /////////////////////////////////////////////////////////////////////////
    // set pointers to point to the first elements of columns 2i and 2i+1
    /////////////////////////////////////////////////////////////////////////
    pInTop1 = ((vector float*)data) + i;
    pInTop2 = pInTop1 + rowLength / 2;
    
    for (j = 0; j < i; j++) {
      //////////////////////////////////////////////////////////////////////
      // read in two vectors that contain a 2x2 square of matrix elements
      // from the left side of the diagonal
      //////////////////////////////////////////////////////////////////////
					
      vInLeft1 = *pInLeft1;
      vInLeft2 = *pInLeft2;

      //////////////////////////////////////////////////////////////////////
      // read in two vectors that contain a 2x2 square of matrix elements
      // from the top side of the diagonal
      //////////////////////////////////////////////////////////////////////

      vInTop1 = *pInTop1;
      vInTop2 = *pInTop2;

      ///////////////////////////////////////////////////////////////////////
      // We now transpose our two 2x2 sub-matrices, and swap their positions.
      // The transpose is done with vector permute operations.
      ///////////////////////////////////////////////////////////////////////
			
      vOutLeft1 = vec_perm(vInTop1, vInTop2, vMergeHiPairPerm);
      vOutLeft2 = vec_perm(vInTop1, vInTop2, vMergeLoPairPerm);

      vOutTop1 = vec_perm(vInLeft1, vInLeft2, vMergeHiPairPerm);
      vOutTop2 = vec_perm(vInLeft1, vInLeft2, vMergeLoPairPerm);

      ///////////////////////////////////////////////////////////////////////
      // store the transposed matrices in their swapped positions
      ///////////////////////////////////////////////////////////////////////
      
      *pInLeft1 = vOutLeft1;
      *pInLeft2 = vOutLeft2;
      
      *pInTop1 = vOutTop1;
      *pInTop2 = vOutTop2;

      ///////////////////////////////////////////////////////////////////////
      // advance pointers to next elements in the current columns & rows
      ///////////////////////////////////////////////////////////////////////
						
      pInLeft1 += 1;
      pInLeft2 += 1;
      
      pInTop1 += rowLength;
      pInTop2 += rowLength;
    }

    /////////////////////////////////////////////////////////////////////////
    // when the above loop is finished, our input pointers point to the 2x2
    // sub-matrix that is on the diagonal of our matrix to be transposed.
    // We read in this sub-matrix into two vectors, transpose the sub-matrix
    // (using vector transposes) and store the sub-matrix.
    /////////////////////////////////////////////////////////////////////////
		
    vInLeft1 = *pInLeft1;
    vInLeft2 = *pInLeft2;
		
    vOutLeft1 = vec_perm(vInLeft1, vInLeft2, vMergeHiPairPerm);
    vOutLeft2 = vec_perm(vInLeft1, vInLeft2, vMergeLoPairPerm);

    *pInLeft1 = vOutLeft1;
    *pInLeft2 = vOutLeft2;
  }
}

////////////////////////////////////////////////////////////////////////////////
//	SquareComplexTransposeTwist performs a transpose on a square matrix of
// 	complex floats with side length of rowLength.  Data is assumed to be
//	arranged lexicographically, i.e., with side length n.
//
//
//
//	re[0] im[0]	re[1] im[1] ... re[n-1] im[n-1]
//	re[n] im[n]		...			re[2n-1] im[2n-1]
//	.
//	.
//	re[(n-1)*n] im[(n-1)*n]	...	re[n^2-1] im[n^2-1]
//	
//
//	In addition to performing the transpose, each element X(j, k) is
// 	multiplied by the twist factor e^(+/- 2 pi i j k / (n^2) ). 
//
////////////////////////////////////////////////////////////////////////////////

static void
SquareComplexTransposeTwist(float *data, int32 sideLength, int32 isInverse)
{
  int32               i, j;
  vector float          *pInLeft1, *pInLeft2;
  vector float          *pInTop1, *pInTop2;
  vector float          vInLeft1, vInLeft2;
  vector float          vInTop1, vInTop2;
  vector float          vOutLeft1, vOutLeft2;
  vector float          vOutTop1, vOutTop2;
  vector float          vTwistedTop1, vTwistedTop2;
  vector float          vPermutedLeft1, vPermutedLeft2;
  
  vector unsigned char  vMergeHiPairPerm;
  vector unsigned char  vMergeLoPairPerm;		
  vector unsigned char  vSwappedPerm;
  
  vector float          vCosTwistA0;
  vector float          vCosTwistA1;

  vector float          vCosTemp0, vCosTemp1;

  vector float          vSinTwistA0;
  vector float          vSinTwistA1;

  vector float          vA;
  vector float          vB;

  vector float          vTransition;
  vector float          vSinSignMultiplier;
  vector float          vZero = Altivec_Const4(vector float, 0, 0, 0, 0);
	
  double                fTemp;
  double                baseAngle1;
  double                baseAngle2;

  double                fSinBaseAngle1;
  double                fSinBaseAngle2;

  //////////////////////////////////////////////////////////////////////////////
  // initialize multiplier for sin vector, for whether we are multiplying
  // by e^(2 pi i l m / (n^2) ) or e^(-2 pi i l m / (n^2) ).
  //////////////////////////////////////////////////////////////////////////////
	
  if (isInverse) {
    vSinSignMultiplier = Altivec_Const4(vector float, -1, 1, -1, 1);
  } else {
    vSinSignMultiplier = Altivec_Const4(vector float, 1, -1, 1, -1);	
  }

  /////////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x1 y0 y1
  /////////////////////////////////////////////////////////////////////////////
  vMergeHiPairPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5, 6,
				     7, 16, 17, 18, 19, 20, 21, 22, 23);

  /////////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x2 x3 y2 y3
  /////////////////////////////////////////////////////////////////////////////
  vMergeLoPairPerm = Altivec_Const16(vector unsigned char, 8, 9, 10, 11, 12, 13,
				     14, 15, 24, 25, 26, 27, 28, 29, 30, 31);

  /////////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x1 x0 x3 y2
  /////////////////////////////////////////////////////////////////////////////
  vSwappedPerm  = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 0, 1, 2,
				  3, 12, 13, 14, 15, 8, 9, 10, 11);

  /////////////////////////////////////////////////////////////////////////////
  //
  // special-case upper-left 2x2 sub-matrix
  //
  /////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////
  //	set pointers to point at first and second rows	
  /////////////////////////////////////////////////////////////////////////////

  pInLeft1 = ((vector float*)data);
  pInLeft2 = pInLeft1 + sideLength / 2;

  /////////////////////////////////////////////////////////////////////////////
  //	create twist multiplier cos & sin vectors
  //
  //	the vTransition vector is used to move elements from the scalar domain
  // 	to the vector domain.
  /////////////////////////////////////////////////////////////////////////////
	
  baseAngle1 = 0;
  baseAngle2 = (2*PI)/(sideLength*sideLength);

  fSinBaseAngle1 = sin(baseAngle1);
  fSinBaseAngle2 = sin(baseAngle2);

  ((float*)&vTransition)[0] = 1;
	
  ((float*)&vTransition)[2] = cos(baseAngle1);
  ((float*)&vTransition)[3] = cos(baseAngle2);
	
  vCosTwistA0 = vec_splat(vTransition, 0);
  vCosTwistA1 = vec_mergel(vTransition, vTransition);
	
  vSinTwistA0 = vZero;
	
  ((float*)&vTransition)[2] = fSinBaseAngle1;
  ((float*)&vTransition)[3] = fSinBaseAngle2;
							
  vSinTwistA1 = vec_mergel(vTransition, vTransition);
  vSinTwistA1 = vec_madd(vSinTwistA1, vSinSignMultiplier, vZero);

  /////////////////////////////////////////////////////////////////////////////
  // twist multiply & permute 2x2 square on diagonal.
  /////////////////////////////////////////////////////////////////////////////
	
  vInLeft1 = *pInLeft1;
  vInLeft2 = *pInLeft2;
				
  vPermutedLeft1 = vec_perm(vInLeft1, vInLeft2, vMergeHiPairPerm);
  vPermutedLeft2 = vec_perm(vInLeft1, vInLeft2, vMergeLoPairPerm);
	
  vOutLeft1 = vec_madd(vPermutedLeft1, vCosTwistA0, vZero);
  vOutLeft1 = vec_madd(vec_perm(vPermutedLeft1, vPermutedLeft1, vSwappedPerm),
		       vSinTwistA0, vOutLeft1);

  vOutLeft2 = vec_madd(vPermutedLeft2, vCosTwistA1, vZero);
  vOutLeft2 = vec_madd(vec_perm(vPermutedLeft2, vPermutedLeft2, vSwappedPerm),
		       vSinTwistA1, vOutLeft2);

  *pInLeft1 = vOutLeft1;
  *pInLeft2 = vOutLeft2;

  /////////////////////////////////////////////////////////////////////////////
  // loop through all remaining rows/columns 2 at a time
  /////////////////////////////////////////////////////////////////////////////

  for (i = 1; i < sideLength/2; i++) {
    ///////////////////////////////////////////////////////////////////////////
    // set pointers to point to the beginning of row 2i and 2i+1
    ///////////////////////////////////////////////////////////////////////////
    pInLeft1 = ((vector float*)data) + i * sideLength;
    pInLeft2 = pInLeft1 + sideLength / 2;
		
    ///////////////////////////////////////////////////////////////////////////
    // set pointers to point to the first elements of columns 2i and 2i+1
    ///////////////////////////////////////////////////////////////////////////
    pInTop1 = ((vector float*)data) + i;
    pInTop2 = pInTop1 + sideLength / 2;
	
    ///////////////////////////////////////////////////////////////////////////
    // set up vectors for incremental updating of cos & sin vectors
    //
    // Given c = cos(w) and s = sin(w), if we want to find the cos and
    // sin of w plus some small angle d, then we can do so by
    // defining:
    //	
    // a = 2 * ((sin(d/2)) ^ 2)
    // b = sin(d)
    // 
    // Then, we can calculate the cos and sin of the updated angle w+d as:
    // 
    // cos(w+d) = c - ac - bs
    // sin(w+d) = s - as + bc
    // 
    // the vectors vA and vB contain these increment factors a and
    // b. Note that each outer loop iteration requires different
    // increment angles. Looking at the matrix, the angle multipliers
    // that we use for our cos & sin twist multipliers look something
    // like:
    //
    //  0/N	0/N	0/N	0/N	0/N	0/N	0/N	...	0/N
    //	0/N	1/N	2/N	3/N	4/N	5/N	6/N		
    //	0/N	2/N	4/N	6/N	8/N	10/N	12/N	.
    //	0/N	3/N	6/N	9/N	12/N	15/N	18/N	.	
    //	0/N	4/N	8/N	12/N	16/N	20/N	24/N
    //		.		.
    //		.		.
    //	0/N					...			(N-1)(N-1)
    //
    // Since we're doing two rows/columns at a time, the vA and vB
    // vectors need to have two different sets of increment values.
    // For example, for the third and fourth columns (handled in the
    // second time through the outer loop) each row increments by 2/N
    // and 3/N.  Since we actually have two sets of twist multiplier
    // vectors, we skip a row for each, so the increment angle is
    // doubled.  This helps reduce calculation dependencies and cuts
    // the number of increment iterations in half, which helps reduce
    // errors in the single-precision calculations used.
    //		
    ///////////////////////////////////////////////////////////////////////////

    baseAngle1 = 4*i*PI/(sideLength*sideLength);
    baseAngle2 = (2*(2*i+1)*PI)/(sideLength*sideLength);

    fSinBaseAngle1 = sin(baseAngle1);
    fTemp = 2*fSinBaseAngle1*fSinBaseAngle1;
		
    ((float*)&vTransition)[0] = fTemp;
    ((float*)&vTransition)[2] = sin(2*baseAngle1);

    fSinBaseAngle2 = sin(baseAngle2);
    fTemp = 2*fSinBaseAngle2*fSinBaseAngle2;

    ((float*)&vTransition)[1] = fTemp;
    ((float*)&vTransition)[3] = sin(2*baseAngle2);

    vA = vec_mergeh(vTransition, vTransition);
    vB = vec_mergel(vTransition, vTransition);
		
    vB = vec_madd(vB, vSinSignMultiplier, vZero);

    ///////////////////////////////////////////////////////////////////////////
    // Set up the initial twist multiplier vectors for the beginning
    // of this row/column pair.  Values are calculated in the scalar
    // domain, and then transferred to the vector domain via the
    // vTransition vector.
    ///////////////////////////////////////////////////////////////////////////
		
    ((float*)&vTransition)[0] = 1;
		
    ((float*)&vTransition)[2] = cos(baseAngle1);
    ((float*)&vTransition)[3] = cos(baseAngle2);
		
    vCosTwistA0 = vec_splat(vTransition, 0);
    vCosTwistA1 = vec_mergel(vTransition, vTransition);
		
    vSinTwistA0 = vZero;
		
    ((float*)&vTransition)[2] = fSinBaseAngle1;
    ((float*)&vTransition)[3] = fSinBaseAngle2;
								
    vSinTwistA1 = vec_mergel(vTransition, vTransition);
    vSinTwistA1 = vec_madd(vSinTwistA1, vSinSignMultiplier, vZero);

    ///////////////////////////////////////////////////////////////////////////
    // load in the leftmost and topmost 2x2 matrices for our row/column pair
    ///////////////////////////////////////////////////////////////////////////

    vInLeft1 = *pInLeft1;
    vInLeft2 = *pInLeft2;

    vInTop1 = *pInTop1;
    vInTop2 = *pInTop2;

    ///////////////////////////////////////////////////////////////////////////
    // twist multiply in top vectors and use permute to transpose before
    // storing to position in new row.
    ///////////////////////////////////////////////////////////////////////////
		
    vTwistedTop1 = vInTop1;
		
    vTwistedTop2 = vec_madd(vInTop2, vCosTwistA1, vZero);
    vTwistedTop2 = vec_madd(vec_perm(vInTop2, vInTop2, vSwappedPerm),
			    vSinTwistA1, vTwistedTop2);

    vOutLeft1 = vec_perm(vTwistedTop1, vTwistedTop2, vMergeHiPairPerm);
    vOutLeft2 = vec_perm(vTwistedTop1, vTwistedTop2, vMergeLoPairPerm);

    ///////////////////////////////////////////////////////////////////////////
    // use permute to transpose 2x2 matrix from current row, and then twist
    // multiply.
    ///////////////////////////////////////////////////////////////////////////
		
    vPermutedLeft1 = vec_perm(vInLeft1, vInLeft2, vMergeHiPairPerm);
    vPermutedLeft2 = vec_perm(vInLeft1, vInLeft2, vMergeLoPairPerm);
		
    vOutTop1 = vPermutedLeft1;

    vOutTop2 = vec_madd(vPermutedLeft2, vCosTwistA1, vZero);
    vOutTop2 = vec_madd(vec_perm(vPermutedLeft2, vPermutedLeft2, vSwappedPerm),
			vSinTwistA1, vOutTop2);

    ///////////////////////////////////////////////////////////////////////////
    // update sin & cos twist multiplier vectors
    ///////////////////////////////////////////////////////////////////////////

    vCosTemp0 = vec_nmsub(vCosTwistA0, vA, vCosTwistA0);
    vCosTemp0 = vec_nmsub(vSinTwistA0, vB, vCosTemp0);

    vSinTwistA0 = vec_nmsub(vSinTwistA0, vA, vSinTwistA0);
    vSinTwistA0 = vec_madd (vCosTwistA0, vB, vSinTwistA0);

    vCosTwistA0 = vCosTemp0;

    vCosTemp1 = vec_nmsub(vCosTwistA1, vA, vCosTwistA1);
    vCosTemp1 = vec_nmsub(vSinTwistA1, vB, vCosTemp1);

    vSinTwistA1 = vec_nmsub(vSinTwistA1, vA, vSinTwistA1);
    vSinTwistA1 = vec_madd (vCosTwistA1, vB, vSinTwistA1);

    vCosTwistA1 = vCosTemp1;

    ///////////////////////////////////////////////////////////////////////////
    // store twist multiplied and transposed matrices.
    ///////////////////////////////////////////////////////////////////////////
			
    *pInLeft1 = vOutLeft1;
    *pInLeft2 = vOutLeft2;

    *pInTop1 = vOutTop1;
    *pInTop2 = vOutTop2;
		
    ///////////////////////////////////////////////////////////////////////////
    // update pointers to move down in current columns and right in
    // current rows
    ///////////////////////////////////////////////////////////////////////////
		
    pInLeft1 += 1;
    pInLeft2 += 1;
		
    pInTop1 += sideLength;
    pInTop2 += sideLength;

    ///////////////////////////////////////////////////////////////////////////
    // loop through remaining 2x2 sub-matrices in current row/column
    // pair until we reach the diagonal of the matrix.
    ///////////////////////////////////////////////////////////////////////////
			
    for (j = 1; j < i; j++) {
      /////////////////////////////////////////////////////////////////////////
      // load in 2x2 matrices for our row/column pair
      /////////////////////////////////////////////////////////////////////////
			
      vInLeft1 = *pInLeft1;
      vInLeft2 = *pInLeft2;

      vInTop1 = *pInTop1;
      vInTop2 = *pInTop2;

      /////////////////////////////////////////////////////////////////////////
      // twist multiply in top vectors and use permute to transpose
      // before storing to position in new row.
      /////////////////////////////////////////////////////////////////////////
			
			
      vTwistedTop1 = vec_madd(vInTop1, vCosTwistA0, vZero);
      vTwistedTop1 = vec_madd(vec_perm(vInTop1, vInTop1, vSwappedPerm),
			      vSinTwistA0, vTwistedTop1);
      
      vTwistedTop2 = vec_madd(vInTop2, vCosTwistA1, vZero);
      vTwistedTop2 = vec_madd(vec_perm(vInTop2, vInTop2, vSwappedPerm),
			      vSinTwistA1, vTwistedTop2);

      vOutLeft1 = vec_perm(vTwistedTop1, vTwistedTop2, vMergeHiPairPerm);
      vOutLeft2 = vec_perm(vTwistedTop1, vTwistedTop2, vMergeLoPairPerm);

      /////////////////////////////////////////////////////////////////////////
      // use permute to transpose 2x2 matrix from current row, and
      // then twist multiply.
      /////////////////////////////////////////////////////////////////////////
			
      vPermutedLeft1 = vec_perm(vInLeft1, vInLeft2, vMergeHiPairPerm);
      vPermutedLeft2 = vec_perm(vInLeft1, vInLeft2, vMergeLoPairPerm);
			
      vOutTop1 = vec_madd(vPermutedLeft1, vCosTwistA0, vZero);
      vOutTop1 = vec_madd(vec_perm(vPermutedLeft1, vPermutedLeft1, vSwappedPerm),
			  vSinTwistA0, vOutTop1);

      vOutTop2 = vec_madd(vPermutedLeft2, vCosTwistA1, vZero);
      vOutTop2 = vec_madd(vec_perm(vPermutedLeft2, vPermutedLeft2, vSwappedPerm),
			  vSinTwistA1, vOutTop2);

      /////////////////////////////////////////////////////////////////////////
      // update sin & cos twist multiplier vectors
      /////////////////////////////////////////////////////////////////////////

      vCosTemp0 = vec_nmsub(vCosTwistA0, vA, vCosTwistA0);
      vCosTemp0 = vec_nmsub(vSinTwistA0, vB, vCosTemp0);

      vSinTwistA0 = vec_nmsub(vSinTwistA0, vA, vSinTwistA0);
      vSinTwistA0 = vec_madd (vCosTwistA0, vB, vSinTwistA0);

      vCosTwistA0 = vCosTemp0;

      vCosTemp1 = vec_nmsub(vCosTwistA1, vA, vCosTwistA1);
      vCosTemp1 = vec_nmsub(vSinTwistA1, vB, vCosTemp1);

      vSinTwistA1 = vec_nmsub(vSinTwistA1, vA, vSinTwistA1);
      vSinTwistA1 = vec_madd (vCosTwistA1, vB, vSinTwistA1);

      vCosTwistA1 = vCosTemp1;

      /////////////////////////////////////////////////////////////////////////
      // store twist multiplied and transposed matrices.
      /////////////////////////////////////////////////////////////////////////
				
      *pInLeft1 = vOutLeft1;
      *pInLeft2 = vOutLeft2;

      *pInTop1 = vOutTop1;
      *pInTop2 = vOutTop2;

      /////////////////////////////////////////////////////////////////////////
      // update pointers to move down in current columns and right in current
      // rows
      /////////////////////////////////////////////////////////////////////////
			
      pInLeft1 += 1;
      pInLeft2 += 1;
			
      pInTop1 += sideLength;
      pInTop2 += sideLength;
    }
		
    ///////////////////////////////////////////////////////////////////////////
    // At this point, the above loop has left the pointers at the 2x2
    // sub-matrix on the diagonal.  We read in this 2x2 matrix,
    // transpose it, twist multiply it, and store it back.
    ///////////////////////////////////////////////////////////////////////////
		
    vInLeft1 = *pInLeft1;
    vInLeft2 = *pInLeft2;
					
    vPermutedLeft1 = vec_perm(vInLeft1, vInLeft2, vMergeHiPairPerm);
    vPermutedLeft2 = vec_perm(vInLeft1, vInLeft2, vMergeLoPairPerm);
		
    vOutLeft1 = vec_madd(vPermutedLeft1, vCosTwistA0, vZero);
    vOutLeft1 = vec_madd(vec_perm(vPermutedLeft1, vPermutedLeft1, vSwappedPerm),
			 vSinTwistA0, vOutLeft1);

    vOutLeft2 = vec_madd(vPermutedLeft2, vCosTwistA1, vZero);
    vOutLeft2 = vec_madd(vec_perm(vPermutedLeft2, vPermutedLeft2, vSwappedPerm),
			 vSinTwistA1, vOutLeft2);

    *pInLeft1 = vOutLeft1;
    *pInLeft2 = vOutLeft2;
  }
}

/////////////////////////////////////////////////////////////////////////////
//	fft_recursive
//
//	Performs a recursive (N/2 by 2) forward or inverse FFT on the
// signal pointed to by pData. This is done in the following manner.
// First, the signal X is divided into two parts, namely X1 = X(0,
// N/2-1) and X2 = X(N/2, N-1). These two sub-signals are then turned
// into the sum and difference, respectively, of these two
// signals. This is in effect an FFT of length 2 performed between
// each pair of elements taken from X1 and X2.  This produces two new
// subsignals X1' and X2'.
//
// Next, a twist operation is performed on X2', where X2'(j) is
// multiplied by e^(2 pi ij / N).
//
// Next, a length N/2 complex FFT is performed on each of X1' and X2',
// yielding X1" and X2".
//
// Finally, the elements of X1" and X2" are merged, so that the
// resulting length-N signal Y is given by:
//
//	Y = X1"(0) X2"(0) X1"(1) X2"(1) ... X1"(N/2-1) X2"(N/2-1)
//
//	This resulting signal Y is the FFT of the original signal X
//
/////////////////////////////////////////////////////////////////////////////
static int fft_recursive(float *pData, int32 len, int32 isign)
{
  int32               result = 0;
  int32               i;

  vector float          *pInLoBottom, *pInLoTop, *pInHiBottom, *pInHiTop;
  vector float          *pOutLoBottom, *pOutLoTop, *pOutHiBottom, *pOutHiTop;

  vector float          vLoBottom, vLoTop, vHiBottom, vHiTop;
	
  vector float		vZero, vSinLo, vCosLo, vSinHi, vCosHi;

  vector float          vFTransition, vFNegTransition;

  double                updateA, updateB, newCos1, newCos2;
  double                newSin1, newSin2, tempCos1, tempCos2;
	
  double                startSinMul;

  vector unsigned char  vCosLoPerm;
  vector unsigned char  vSinPerm;
  vector unsigned char  vSwapPairPerm;
  vector unsigned char  vMergeLoPairPerm;
  vector unsigned char  vMergeHiPairPerm;
  vector unsigned int   vHiPairSelect;
	
  vector float          vDiffBottom, vDiffTop;	
	
  vector float          vOneHalf;
	
  vector float          vTwistBottom, vTwistTop;
	
  vector float          vSwappedDiffBottom, vSwappedDiffTop;
	
  ///////////////////////////////////////////////////////////////////////////
  // make sure that work buffer is large enough to hold half of entire data
  // length for merge operation at end of routine.
  ///////////////////////////////////////////////////////////////////////////
  result = EnsureStaticBufferSize(len/2);
	
  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x1 x0 x3 x2
    /////////////////////////////////////////////////////////////////////////
    vSwapPairPerm = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 0, 1,
				    2, 3, 12, 13, 14, 15, 8, 9, 10, 11);

    /////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x0 x0 y3 y3
    /////////////////////////////////////////////////////////////////////////
    vCosLoPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 0, 1, 2,
				 3, 28, 29, 30, 31, 28, 29, 30, 31);

    /////////////////////////////////////////////////////////////////////////
    // initialize a select vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x0 x1 y2 y3
    /////////////////////////////////////////////////////////////////////////
    vHiPairSelect = (vector unsigned int)(Altivec_Const4(vector signed int,
							 0, 0, -1, -1));
    
    if (isign == -1) {
      startSinMul = 1;

      ///////////////////////////////////////////////////////////////////////
      // initialize a permute vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = x0 y0 x1 y1
      ///////////////////////////////////////////////////////////////////////
      vSinPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 16, 17,
				 18, 19, 4, 5, 6, 7, 20, 21, 22, 23);
    } else {
      startSinMul = -1;

      ///////////////////////////////////////////////////////////////////////
      // initialize a permute vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = y0 x0 y1 x1
      ///////////////////////////////////////////////////////////////////////
      vSinPerm = Altivec_Const16(vector unsigned char, 16, 17, 18, 19, 0, 1,
				 2, 3, 20, 21, 22, 23, 4, 5, 6, 7);		
    }

    /////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x0 x1 y0 y1
    //////////////////////////////////////////////////////////////////////////
    vMergeHiPairPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5,
				       6, 7, 16, 17, 18, 19, 20, 21, 22, 23);

    //////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x2 x3 y2 y3
    //////////////////////////////////////////////////////////////////////////
    vMergeLoPairPerm = Altivec_Const16(vector unsigned char, 8, 9, 10, 11, 12,
				       13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);
		
    vZero = Altivec_Const4(vector float, 0, 0, 0, 0);

		
    /////////////////////////////////////////////////////////////////////////
    // start source pointers at start and end of high and low halves of the
    // data
    /////////////////////////////////////////////////////////////////////////
    pInLoBottom = (vector float*)pData;
    pInHiBottom = pInLoBottom + len/4;
    pInLoTop = pInHiBottom - 1;
    pInHiTop = pInLoBottom + (len/2) - 1;

    /////////////////////////////////////////////////////////////////////////
    // start dest pointers at start and end of high and low halves of the
    // data
    /////////////////////////////////////////////////////////////////////////
    pOutHiBottom = pInHiBottom;
    pOutHiTop = pInHiTop;

    pOutLoBottom = pInLoBottom;
    pOutLoTop = (pOutLoBottom + len/4) - 1;

    ////////////////////////////////////////////////////////////////////////
    //
    //	Given c = cos(w) and s = sin(w), if we
    //	want to find the cos and sin of w plus
    //	some small angle d, then we can do so
    //	by defining:
    //	
    //	a = 2 * ((sin(d/2)) ^ 2)
    //	b = sin(d)
    //	
    //	Then, we can calculate the cos and sin
    //	of the updated angle w+d as:
    //	
    //	cos(w+d) = c - ac - bs
    //	sin(w+d) = s - as + bc
    //	
    //	We will need to incrementally calculate
    //	cos(w) and sin(w), so we define the
    //	following scalar values.  We use scalar
    //	because we need the precision of doubles,
    //	and vectors are floats.	
    //
    ////////////////////////////////////////////////////////////////////////

    updateA = sin(2*PI/len);
    updateA *= updateA*2;

    updateB = sin(4*PI/len);

    ////////////////////////////////////////////////////////////////////////
    //
    // We want to have four cos and sin vectors that are updated
    // incrementally, using the doubles that we have used to calculate
    // our new values.  To do this we have a "transition" vector that
    // allows us to get our scalar values into the vector domain.
    //
    // Scalar values are stored into the elements of the transition
    // vector, and then our end cos and sin vectors are created by
    // permuting values out of our transition vector and other
    // previously created sin and cos vectors.
    // 
    ////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////
    // set up vSinLo as
    // vSinLo = -sin(0) sin(0) -sin(pi/len) sin(pi/len)
    ////////////////////////////////////////////////////////////////////////
    ((float*)&vFTransition)[0] = 0;
    ((float*)&vFTransition)[1] = 0;

    newSin1 = sin(2*PI/len);
			
    ((float*)&vFTransition)[2] = newSin1*startSinMul;
    ((float*)&vFTransition)[3] = -newSin1*startSinMul;

    vSinLo = vFTransition;

    ////////////////////////////////////////////////////////////////////////
    // set up vSinHi as
    // vSinHi = -sin((len-2)*pi/len) sin((len-2)*pi/len) -
    //           sin((len-1)*pi/len) sin((len-1)*pi/len)
    ////////////////////////////////////////////////////////////////////////
    newSin2 = sin(4*PI/len);
		
    ((float*)&vFTransition)[0] = newSin2*startSinMul;	
    ((float*)&vFTransition)[1] = -newSin2*startSinMul;
		
    vSinHi = vFTransition;

    ////////////////////////////////////////////////////////////////////////
    // set up vCosLo as
    // vCosLo = -cos(0) cos(0) -cos(pi/len) cos(pi/len)
    ////////////////////////////////////////////////////////////////////////
    ((float*)&vFTransition)[0] = 1;
    ((float*)&vFTransition)[1] = 1;

    newCos1 = cos(2*PI/len);
			
    ((float*)&vFTransition)[2] = newCos1;
    ((float*)&vFTransition)[3] = newCos1;

    vCosLo = vFTransition;

    ////////////////////////////////////////////////////////////////////////
    // set up vCosHi as
    // vCosHi = -cos((len-2)*pi/len) cos((len-2)*pi/len) -
    //           cos((len-1)*pi/len) cos((len-1)*pi/len)
    ////////////////////////////////////////////////////////////////////////
    newCos2 = cos(4*PI/len);
		
    ((float*)&vFTransition)[0] = newCos2;	
    ((float*)&vFTransition)[1] = newCos2;
		
    vCosHi = vec_sub(vZero, vFTransition);	

    if (isign == -1) {
      //////////////////////////////////////////////////////////////////////
      // we are doing a forward FFT
      //////////////////////////////////////////////////////////////////////
		
      for (i = 0; i < len/16; i++) {
	////////////////////////////////////////////////////////////////////
	// calculate new sin, cos for next time through loop, using
	// our incremental updating algorithm
	////////////////////////////////////////////////////////////////////

	tempCos1 	= newCos1 - (updateA*newCos1  + updateB * newSin1);
	newSin1 	= newSin1 - (updateA*newSin1  - updateB * newCos1);
	newCos1 	= tempCos1;

	tempCos2 	= newCos2 - (updateA*newCos2  + updateB * newSin2);
	newSin2 	= newSin2 - (updateA*newSin2  - updateB * newCos2);
	newCos2 	= tempCos2;

	////////////////////////////////////////////////////////////////////
	// load input vectors
	////////////////////////////////////////////////////////////////////

	vLoBottom 	= *pInLoBottom++;
	vHiBottom 	= *pInHiBottom++;
	vLoTop 		= *pInLoTop--;
	vHiTop	 	= *pInHiTop--;

	////////////////////////////////////////////////////////////////////
	// store new cos & sin to transition vector
	////////////////////////////////////////////////////////////////////

	((float*)&vFTransition)[0] = newSin2;
	((float*)&vFTransition)[1] = newSin1;
	((float*)&vFTransition)[2] = newCos2;
	((float*)&vFTransition)[3] = newCos1;

	vFNegTransition	= vec_sub(vZero, vFTransition);

	////////////////////////////////////////////////////////////////////
	// calc sum of input data
	////////////////////////////////////////////////////////////////////

	*pOutLoBottom++ 	= vec_add(vLoBottom, vHiBottom);		
	*pOutLoTop-- 		= vec_add(vLoTop, vHiTop);		

	////////////////////////////////////////////////////////////////////
	// calc difference of input data
	////////////////////////////////////////////////////////////////////
				
	vDiffBottom 	= vec_sub(vLoBottom, vHiBottom);
	vDiffTop 		= vec_sub(vLoTop, vHiTop);

	////////////////////////////////////////////////////////////////////
	// multiply difference by twist factor
	////////////////////////////////////////////////////////////////////
				
	vTwistBottom       = vec_madd(vCosLo, vDiffBottom, vZero);		
	vTwistTop	   = vec_madd(vCosHi, vDiffTop, vZero);		

	vSwappedDiffTop    = vec_perm(vDiffTop, vDiffTop, vSwapPairPerm);	
	vSwappedDiffBottom = vec_perm(vDiffBottom, vDiffBottom, vSwapPairPerm);	
				
	vTwistBottom       = vec_madd(vSinLo, vSwappedDiffBottom, vTwistBottom);
	vTwistTop          = vec_madd(vSinHi, vSwappedDiffTop, vTwistTop);

	//////////////////////////////////////////////////////////////////////
	// generate new cos & sin vectors from transition vector and
	// previously calculated steps.
	//////////////////////////////////////////////////////////////////////

	vCosLo = vec_sub(vZero, vec_perm(vCosHi, vFNegTransition, vCosLoPerm));
	vCosHi = vec_mergel(vFNegTransition, vFNegTransition);
	vSinLo = vec_perm(vFTransition, vFNegTransition, vSinPerm);
	vSinLo = vec_sel(vSinHi, vSinLo, vHiPairSelect);
	vSinHi = vec_perm(vFTransition, vFNegTransition, vSinPerm);

	//////////////////////////////////////////////////////////////////////
	// load input vectors for next sum & difference calculation
	//////////////////////////////////////////////////////////////////////

	vLoBottom 	= *pInLoBottom++;
	vHiBottom 	= *pInHiBottom++;
	vLoTop 		= *pInLoTop--;
	vHiTop	 	= *pInHiTop--;

	//////////////////////////////////////////////////////////////////////
	// store twist-multiplied difference.
	//////////////////////////////////////////////////////////////////////
				
	*pOutHiBottom++ = vTwistBottom;
	*pOutHiTop--    = vTwistTop;

	//////////////////////////////////////////////////////////////////////
	// calculate new sin, cos for next time through loop, using
	// our incremental updating algorithm
	//////////////////////////////////////////////////////////////////////

	tempCos1 	= newCos1 - (updateA*newCos1  + updateB * newSin1);
	newSin1 	= newSin1 - (updateA*newSin1  - updateB * newCos1);
	newCos1 	= tempCos1;

	tempCos2 	= newCos2 - (updateA*newCos2  + updateB * newSin2);
	newSin2 	= newSin2 - (updateA*newSin2  - updateB * newCos2);
	newCos2 	= tempCos2;

	//////////////////////////////////////////////////////////////////////
	// store new cos & sin to transition vector
	//////////////////////////////////////////////////////////////////////

	((float*)&vFTransition)[0] = newSin2;
	((float*)&vFTransition)[1] = newSin1;
	((float*)&vFTransition)[2] = newCos2;
	((float*)&vFTransition)[3] = newCos1;

	vFNegTransition	= vec_sub(vZero, vFTransition);

	//////////////////////////////////////////////////////////////////////
	// calc sum of input data
	//////////////////////////////////////////////////////////////////////

	*pOutLoBottom++   = vec_add(vLoBottom, vHiBottom);		
	*pOutLoTop--      = vec_add(vLoTop, vHiTop);		

	//////////////////////////////////////////////////////////////////////
	// calc difference of input data
	//////////////////////////////////////////////////////////////////////
				
	vDiffBottom       = vec_sub(vLoBottom, vHiBottom);
	vDiffTop          = vec_sub(vLoTop, vHiTop);

	//////////////////////////////////////////////////////////////////////
	// multiply difference by twist factor
	//////////////////////////////////////////////////////////////////////
				
	vTwistBottom      = vec_madd(vCosLo, vDiffBottom, vZero);		
	vTwistTop         = vec_madd(vCosHi, vDiffTop, vZero);		

	vSwappedDiffTop   = vec_perm(vDiffTop, vDiffTop, vSwapPairPerm);	
	vSwappedDiffBottom= vec_perm(vDiffBottom, vDiffBottom, vSwapPairPerm);	
				
	vTwistBottom      = vec_madd(vSinLo, vSwappedDiffBottom, vTwistBottom);
	vTwistTop         = vec_madd(vSinHi, vSwappedDiffTop, vTwistTop);

	//////////////////////////////////////////////////////////////////////
	// generate new cos & sin vectors from transition vector and
	// previously calculated steps.
	//////////////////////////////////////////////////////////////////////

	vCosLo = vec_sub(vZero, vec_perm(vCosHi, vFNegTransition, vCosLoPerm));
	vCosHi = vec_mergel(vFNegTransition, vFNegTransition);
	vSinLo = vec_perm(vFTransition, vFNegTransition, vSinPerm);
	vSinLo = vec_sel(vSinHi, vSinLo, vHiPairSelect);
	vSinHi = vec_perm(vFTransition, vFNegTransition, vSinPerm);
				
	//////////////////////////////////////////////////////////////////////
	// store twist-multiplied difference.
	//////////////////////////////////////////////////////////////////////
				
	*pOutHiBottom++ = vTwistBottom;
	*pOutHiTop--    = vTwistTop;
			
      }	
    } else {
      //////////////////////////////////////////////////////////////////////
      // We are doing an inverse FFT, so we must adjust output by
      // dividing by 2.
      //////////////////////////////////////////////////////////////////////

      vOneHalf          = Altivec_Const4(vector float, .5, .5, .5, .5);

      for (i = 0; i < len/16; i++) {
	////////////////////////////////////////////////////////////////////
	// calculate new sin, cos for next time through loop, using
	// our incremental updating algorithm
	////////////////////////////////////////////////////////////////////

	tempCos1 	= newCos1 - (updateA*newCos1  + updateB * newSin1);
	newSin1 	= newSin1 - (updateA*newSin1  - updateB * newCos1);
	newCos1 	= tempCos1;

	tempCos2 	= newCos2 - (updateA*newCos2  + updateB * newSin2);
	newSin2 	= newSin2 - (updateA*newSin2  - updateB * newCos2);
	newCos2 	= tempCos2;

	////////////////////////////////////////////////////////////////////
	// load input vectors
	////////////////////////////////////////////////////////////////////

	vLoBottom 	= *pInLoBottom++;
	vHiBottom 	= *pInHiBottom++;
	vLoTop 		= *pInLoTop--;
	vHiTop	 	= *pInHiTop--;

	////////////////////////////////////////////////////////////////////
	// do inverse adjusting by dividing all elements by two. 
	////////////////////////////////////////////////////////////////////

	vLoBottom      = vec_madd(vLoBottom, vOneHalf, vZero);
	vHiBottom      = vec_madd(vHiBottom, vOneHalf, vZero);
	vLoTop         = vec_madd(vLoTop, vOneHalf, vZero);
	vHiTop         = vec_madd(vHiTop, vOneHalf, vZero);

	////////////////////////////////////////////////////////////////////
	// store new cos & sin to transition vector
	////////////////////////////////////////////////////////////////////

	((float*)&vFTransition)[0] = newSin2;
	((float*)&vFTransition)[1] = newSin1;
	((float*)&vFTransition)[2] = newCos2;
	((float*)&vFTransition)[3] = newCos1;

	vFNegTransition = vec_sub(vZero, vFTransition);

	////////////////////////////////////////////////////////////////////
	// calc sum of input data
	////////////////////////////////////////////////////////////////////

	*pOutLoBottom++ = vec_add(vLoBottom, vHiBottom);		
	*pOutLoTop--    = vec_add(vLoTop, vHiTop);		

	////////////////////////////////////////////////////////////////////
	// calc difference of input data
	////////////////////////////////////////////////////////////////////
				
	vDiffBottom 	= vec_sub(vLoBottom, vHiBottom);
	vDiffTop        = vec_sub(vLoTop, vHiTop);

	////////////////////////////////////////////////////////////////////
	// multiply difference by twist factor
	////////////////////////////////////////////////////////////////////
				
	vTwistBottom    = vec_madd(vCosLo, vDiffBottom, vZero);		
	vTwistTop       = vec_madd(vCosHi, vDiffTop, vZero);		

	vSwappedDiffTop = vec_perm(vDiffTop, vDiffTop, vSwapPairPerm);	
	vSwappedDiffBottom= vec_perm(vDiffBottom, vDiffBottom, vSwapPairPerm);	
				
	vTwistBottom    = vec_madd(vSinLo, vSwappedDiffBottom, vTwistBottom);
	vTwistTop       = vec_madd(vSinHi, vSwappedDiffTop, vTwistTop);

	////////////////////////////////////////////////////////////////////
	// generate new cos & sin vectors from transition vector and
	// previously calculated steps.
	////////////////////////////////////////////////////////////////////

	vCosLo = vec_sub(vZero, vec_perm(vCosHi, vFNegTransition, vCosLoPerm));
	vCosHi = vec_mergel(vFNegTransition, vFNegTransition);
	vSinLo = vec_perm(vFTransition, vFNegTransition, vSinPerm);
	vSinLo = vec_sel(vSinHi, vSinLo, vHiPairSelect);
	vSinHi = vec_perm(vFTransition, vFNegTransition, vSinPerm);
								
	////////////////////////////////////////////////////////////////////
	// store twist-multiplied difference.
	////////////////////////////////////////////////////////////////////
				
	*pOutHiBottom++ = vTwistBottom;
	*pOutHiTop--    = vTwistTop;

	////////////////////////////////////////////////////////////////////
	// calculate new sin, cos for next time through loop, using
	// our incremental updating algorithm
	////////////////////////////////////////////////////////////////////

	tempCos1 	= newCos1 - (updateA*newCos1  + updateB * newSin1);
	newSin1 	= newSin1 - (updateA*newSin1  - updateB * newCos1);
	newCos1 	= tempCos1;

	tempCos2 	= newCos2 - (updateA*newCos2  + updateB * newSin2);
	newSin2 	= newSin2 - (updateA*newSin2  - updateB * newCos2);
	newCos2 	= tempCos2;


	////////////////////////////////////////////////////////////////////
	// load input vectors
	////////////////////////////////////////////////////////////////////

	vLoBottom 	= *pInLoBottom++;
	vHiBottom 	= *pInHiBottom++;
	vLoTop 		= *pInLoTop--;
	vHiTop	 	= *pInHiTop--;

	////////////////////////////////////////////////////////////////////
	// do inverse adjusting by dividing all elements by two. 
	////////////////////////////////////////////////////////////////////

	vLoBottom = vec_madd(vLoBottom, vOneHalf, vZero);
	vHiBottom = vec_madd(vHiBottom, vOneHalf, vZero);
	vLoTop = vec_madd(vLoTop, vOneHalf, vZero);
	vHiTop = vec_madd(vHiTop, vOneHalf, vZero);

	////////////////////////////////////////////////////////////////////
	// store new cos & sin to transition vector
	////////////////////////////////////////////////////////////////////

	((float*)&vFTransition)[0] = newSin2;
	((float*)&vFTransition)[1] = newSin1;
	((float*)&vFTransition)[2] = newCos2;
	((float*)&vFTransition)[3] = newCos1;

	vFNegTransition	= vec_sub(vZero, vFTransition);

	////////////////////////////////////////////////////////////////////
	// calc sum of input data
	////////////////////////////////////////////////////////////////////

	*pOutLoBottom++   = vec_add(vLoBottom, vHiBottom);		
	*pOutLoTop--      = vec_add(vLoTop, vHiTop);		

	////////////////////////////////////////////////////////////////////
	// calc difference of input data
	////////////////////////////////////////////////////////////////////
				
	vDiffBottom       = vec_sub(vLoBottom, vHiBottom);
	vDiffTop          = vec_sub(vLoTop, vHiTop);

	////////////////////////////////////////////////////////////////////
	// multiply difference by twist factor
	////////////////////////////////////////////////////////////////////
				
	vTwistBottom      = vec_madd(vCosLo, vDiffBottom, vZero);		
	vTwistTop         = vec_madd(vCosHi, vDiffTop, vZero);		

	vSwappedDiffTop   = vec_perm(vDiffTop, vDiffTop, vSwapPairPerm);	
	vSwappedDiffBottom= vec_perm(vDiffBottom, vDiffBottom, vSwapPairPerm);	
				
	vTwistBottom      = vec_madd(vSinLo, vSwappedDiffBottom, vTwistBottom);
	vTwistTop         = vec_madd(vSinHi, vSwappedDiffTop, vTwistTop);

	////////////////////////////////////////////////////////////////////
	// generate new cos & sin vectors from transition vector and
	// previously calculated steps.
	////////////////////////////////////////////////////////////////////

	vCosLo = vec_sub(vZero, vec_perm(vCosHi, vFNegTransition, vCosLoPerm));
	vCosHi = vec_mergel(vFNegTransition, vFNegTransition);
	vSinLo = vec_perm(vFTransition, vFNegTransition, vSinPerm);
	vSinLo = vec_sel(vSinHi, vSinLo, vHiPairSelect);
	vSinHi = vec_perm(vFTransition, vFNegTransition, vSinPerm);
				
	////////////////////////////////////////////////////////////////////
	// store twist-multiplied difference.
	////////////////////////////////////////////////////////////////////
				
	*pOutHiBottom++   = vTwistBottom;
	*pOutHiTop--      = vTwistTop;
      }	
    }

    ////////////////////////////////////////////////////////////////////////
    // perform N/2-length FFT on high half of data
    ////////////////////////////////////////////////////////////////////////
    result = FFTComplex(pData+len, len/2, isign);
		
    if (result == 0) {
      //////////////////////////////////////////////////////////////////////
      // perform N/2-length FFT on low half of data
      //////////////////////////////////////////////////////////////////////
      result = FFTComplex(pData, len/2, isign);
			
      if (result == 0) {
	////////////////////////////////////////////////////////////////////
	// Move low half of data to temporary buffer, which will leave
	// room in main data space to merge elements of low and high
	// data in to the original buffer space.
	////////////////////////////////////////////////////////////////////
	{								
	  vector float *pSource = (vector float*)pData;
	  vector float *pDest = (vector float*)gTempBufferPtr;
					
	  vector float v1, v2, v3, v4;
					
	  for (i = 0; i < len/16; i++) {
	    v1 = *pSource++;
	    v2 = *pSource++;
	    v3 = *pSource++;
	    v4 = *pSource++;
	    
	    *pDest++ = v1;
	    *pDest++ = v2;
	    *pDest++ = v3;
	    *pDest++ = v4;
	  }
	}

	////////////////////////////////////////////////////////////////////
	// Merge low and high halves of data back into original data
	// buffer.  Elements are merged so that, given high and low
	// signals X1 and X2, resulting merged signal is X1(0), X2(0),
	// X1(1), X2(1), ... , X1(N/2-1), X2(N/2-1)
	////////////////////////////////////////////////////////////////////
	pInHiBottom = (vector float*)gTempBufferPtr;
	pInHiTop = (vector float*)(pData+len);
	pOutLoBottom = (vector float*)pData;	

	for (i = 0; i < len/8; i++) {
	  vector float	vInEven1, vInOdd1;
	  vector float	vInEven2, vInOdd2;
	  vector float	vOut1, vOut2, vOut3, vOut4;
							
	  vInEven1 = *pInHiBottom++;
	  vInOdd1 = *pInHiTop++;
	  vInEven2 = *pInHiBottom++;
	  vInOdd2 = *pInHiTop++;
					
	  vOut1 = vec_perm(vInEven1, vInOdd1, vMergeHiPairPerm);
	  vOut2 = vec_perm(vInEven1, vInOdd1, vMergeLoPairPerm);
	  vOut3 = vec_perm(vInEven2, vInOdd2, vMergeHiPairPerm);
	  vOut4 = vec_perm(vInEven2, vInOdd2, vMergeLoPairPerm);
					
	  *pOutLoBottom++ = vOut1;
	  *pOutLoBottom++ = vOut2;
	  *pOutLoBottom++ = vOut3;
	  *pOutLoBottom++ = vOut4;
	}
      }
    }
  }
		
  return result;
}


////////////////////////////////////////////////////////////////////////////
//	fft_matrix_forward_columnwise
//
//	Performs a recursive 2D matrix FFT on the data pointed to by
// pData. It leaves data in columnwise order.
//
//	There are three steps to the 2D matrix FFT.  First, a FFT is
// performed on each column of the matrix.  Second, each element in
// the matrix is multiplied by a twist factor.  In a length N signal,
// element X(j, k) is multiplied by e^(+/- 2 pi i j k / N ). Finally,
// an FFT is performed on each row of the matrix.  The resulting data
// is the FFT of the original signal, stored in columnwise order.
//	
//
////////////////////////////////////////////////////////////////////////////
static int fft_matrix_forward_columnwise(float *pData, int32 length)
{
  int32  i, j;
  int32  pow;
  float    *pRow;
  int32  rowCount;
  int32  colCount;
  int32  rowPower;
  int32  colPower;
  void*    rowBufferPointer = 0;
  int32  result = 0;
			
  pow = log2max(length);
    
  /////////////////////////////////////////////////////////////////////////
  // length must be an exact power of 2
  /////////////////////////////////////////////////////////////////////////
  if ((1 << pow) != length) {
    return EINVAL;
  }
    
  /////////////////////////////////////////////////////////////////////////
  // data must be 16-byte aligned because of vector load/store requirements
  /////////////////////////////////////////////////////////////////////////
  if (((int32)pData) & 0x0F) {
    return EINVAL;
  }
    
  /////////////////////////////////////////////////////////////////////////
  // calculate dimensions of matrix.  If matrix can not be square, then the
  // column count will be 2X the row count    
  /////////////////////////////////////////////////////////////////////////

  rowPower = pow / 2;
    
  colPower = rowPower;
    
  if (pow & 1) colPower++;

  rowCount = 1<<rowPower;
  colCount = 1<<colPower;

  /////////////////////////////////////////////////////////////////////////
  // allocate a buffer that will be used for column ferrying, and can store
  // two columns at a time.
  /////////////////////////////////////////////////////////////////////////

  rowBufferPointer = (float*) valloc(2*2*sizeof(float)*rowCount);
	
  if (!rowBufferPointer) result = ENOMEM;
	
  if (result == 0) {
    vector float         *pCurrentColumn;
    vector float         vIn1, vIn2, vOut1, vOut2;
    vector unsigned char vMergeHiPairPerm;
    vector unsigned char vMergeLoPairPerm;
    vector float         *pSplit1, *pSplit2;
    float                *pFFT1, *pFFT2;
    vector unsigned char vSwappedPerm;
    vector float         vSinSignMultiplier, vCosTwistA0, vCosTwistA1;

    vector float         vCosTemp0, vCosTemp1, vSinTwistA0, vSinTwistA1;

    vector float         vA, vB, vTransition;
    vector float         vZero = Altivec_Const4(vector float, 0, 0, 0, 0);
		
    double               fTemp,	baseAngle1, baseAngle2;

    double               fSinBaseAngle1, fSinBaseAngle2;
		
    ///////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x0 x1 y0 y1
    ///////////////////////////////////////////////////////////////////////
    vMergeHiPairPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4,
				       5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23);

    ///////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x2 x3 y2 y3
    ///////////////////////////////////////////////////////////////////////
    vMergeLoPairPerm = Altivec_Const16(vector unsigned char, 8, 9, 10, 11,
				       12, 13, 14, 15, 24, 25, 26, 27, 28,
				       29, 30, 31);
		
    ///////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x1 x0 x3 y2
    ///////////////////////////////////////////////////////////////////////
    vSwappedPerm  = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 0, 1,
				    2, 3, 12, 13, 14, 15, 8, 9, 10, 11);
		
    ///////////////////////////////////////////////////////////////////////
    // vSinSignMultiplier is a vector used to create a vector in the form
    // sin(x) -sin(x) sin(y) -sin(y)
    ///////////////////////////////////////////////////////////////////////

    vSinSignMultiplier = Altivec_Const4(vector float, 1, -1, 1, -1);	
		
    ///////////////////////////////////////////////////////////////////////
    // we now loop through all columns, two columns at a time, and
    // perform column ferrying FFTs on the columns.  We first copy the
    // columns to a separate buffer, then perform the FFTs, and
    // finally copy them back to their original column positions
    ///////////////////////////////////////////////////////////////////////

    for (i = 0; i < colCount/2; i++) {
      ///////////////////////////////////////////////////////////////////////
      // set up vectors for incremental updating of cos & sin vectors
      //
      // Given c = cos(w) and s = sin(w), if we want to find the cos
      // and sin of w plus some small angle d, then we can do so by
      // defining:
      //	
      // a = 2 * ((sin(d/2)) ^ 2)
      // b = sin(d)
      // 
      // Then, we can calculate the cos and sin of the updated angle
      // w+d as:
      // 
      // cos(w+d) = c - ac - bs
      // sin(w+d) = s - as + bc
      // 
      // the vectors vA and vB contain these increment factors a and
      // b.  Note that each outer loop iteration requires different
      // increment angles.  Looking at the matrix, the angle
      // multipliers that we use for our cos & sin twist multipliers
      // look something like:
      //
      // 0/N	0/N	0/N	0/N	0/N	0/N	0/N	...	0/N
      // 0/N	1/N	2/N	3/N	4/N	5/N	6/N
      // 0/N	2/N	4/N	6/N	8/N    10/N	12/N	.
      // 0/N	3/N	6/N	9/NF   12/N    15/N	18/N	.	
      // 0/N	4/N	8/N    12/N    16/N    20/N	24/N
      //	.	.
      //	.	.
      // 0/N					...			(N-1)(N-1)
      //
      // Since we're doing two columns at a time, the vA and vB
      // vectors need to have two different sets of increment values.
      // For example, for the third and fourth columns (handled in the
      // second time through the outer loop) each row increments by
      // 2/N and 3/N.  Since we actually have two sets of twist
      // multiplier vectors, we skip a row for each, so the increment
      // angle is doubled.  This helps reduce calculation dependencies
      // and cuts the number of increment iterations in half, which
      // helps reduce errors in the single-precision calculations
      // used.
      //		
      ///////////////////////////////////////////////////////////////////////

      baseAngle1 = 4*i*PI/(colCount*rowCount);
      baseAngle2 = (2*(2*i+1)*PI)/(colCount*rowCount);

      fSinBaseAngle1 = sin(baseAngle1);
      fTemp = 2*fSinBaseAngle1*fSinBaseAngle1;
			
      ((float*)&vTransition)[0] = fTemp;
      ((float*)&vTransition)[2] = sin(2*baseAngle1);

      fSinBaseAngle2 = sin(baseAngle2);
      fTemp = 2*fSinBaseAngle2*fSinBaseAngle2;

      ((float*)&vTransition)[1] = fTemp;
      ((float*)&vTransition)[3] = sin(2*baseAngle2);

      vA = vec_mergeh(vTransition, vTransition);
      vB = vec_mergel(vTransition, vTransition);
			
      vB = vec_madd(vB, vSinSignMultiplier, vZero);

      ///////////////////////////////////////////////////////////////////////
      // Set up the initial twist multiplier vectors for the beginning
      // of this column.  Values are calculated in the scalar domain,
      // and then transferred to the vector domain via the vTransition
      // vector.
      ///////////////////////////////////////////////////////////////////////
			
      ((float*)&vTransition)[0] = 1;
			
      ((float*)&vTransition)[2] = cos(baseAngle1);
      ((float*)&vTransition)[3] = cos(baseAngle2);
			
      vCosTwistA0 = vec_splat(vTransition, 0);
      vCosTwistA1 = vec_mergel(vTransition, vTransition);
			
      vSinTwistA0 = vZero;
			
      ((float*)&vTransition)[2] = fSinBaseAngle1;
      ((float*)&vTransition)[3] = fSinBaseAngle2;
									
      vSinTwistA1 = vec_mergel(vTransition, vTransition);
      vSinTwistA1 = vec_madd(vSinTwistA1, vSinSignMultiplier, vZero);

      ///////////////////////////////////////////////////////////////////////
      // initialize pointers in temporary buffer for storing copied columns.
      ///////////////////////////////////////////////////////////////////////
		
      pSplit1 = (vector float*)rowBufferPointer;
      pSplit2 = pSplit1 + rowCount / 2;
			
      ///////////////////////////////////////////////////////////////////////
      // point at top of columns to be copied
      ///////////////////////////////////////////////////////////////////////

      pCurrentColumn = ((vector float*)pData) + i;
		
      ///////////////////////////////////////////////////////////////////////
      // loop through all rows in the current column, copying elements
      // to the temporary buffer.  We load two rows at a time, and
      // permute the vectors to create a single vector that contains
      // the appropriate column elements from both rows.
      ///////////////////////////////////////////////////////////////////////

      for (j = 0; j < rowCount/2; j++) {
	///////////////////////////////////////////////////////////////////////
	// read in two rows worth of vectors (two rows X two columns)
	///////////////////////////////////////////////////////////////////////

	vIn1 = *pCurrentColumn;
	pCurrentColumn += colCount/2;
	vIn2 = *pCurrentColumn;
	pCurrentColumn += colCount/2;
				
	///////////////////////////////////////////////////////////////////////
	// permute the vectors so that the two left column entries are
	// in one vector and the two right column entries are in one
	// vector
	///////////////////////////////////////////////////////////////////////

	vOut1 = vec_perm(vIn1, vIn2, vMergeHiPairPerm);
	vOut2 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
				
	///////////////////////////////////////////////////////////////////////
	// store the split columns to the appropriate buffers
	///////////////////////////////////////////////////////////////////////

	*pSplit1++ = vOut1;
	*pSplit2++ = vOut2;
      }

      /////////////////////////////////////////////////////////////////////////
      // perform FFTs on the two columns of data that we have copied
      // to the temp buffer
      /////////////////////////////////////////////////////////////////////////
		
      pFFT1 = (float *)rowBufferPointer;
      pFFT2 = pFFT1 + 2*rowCount;
			
      result = FFTComplex(pFFT1, rowCount, -1);
      if (result != 0) break;
			
      result = FFTComplex(pFFT2, rowCount, -1);
      if (result != 0) break;
				
      /////////////////////////////////////////////////////////////////////////
      // point at the beginning of the two copied column buffers, and
      // at the top of the columns in the matrix where we will store
      // them back.
      /////////////////////////////////////////////////////////////////////////

      pSplit1 = (vector float*)rowBufferPointer;
      pSplit2 = pSplit1 + rowCount / 2;
      pCurrentColumn = ((vector float*)pData) + i;

      /////////////////////////////////////////////////////////////////////////
      // loop through all of the column entries that are stored in the
      // temp buffer, and merge them to be stored back into the
      // columns of the matrix.  At the same time, we perform the
      // twist multiply on the entries.
      /////////////////////////////////////////////////////////////////////////

      for (j=0; j < rowCount/2; j++) {
	vector float vNew1, vNew2;
			
	///////////////////////////////////////////////////////////////////////
	// get two vectors of column data
	///////////////////////////////////////////////////////////////////////

	vIn1 = *pSplit1++;
	vIn2 = *pSplit2++;
				
	///////////////////////////////////////////////////////////////////////
	// turn them into row vectors
	///////////////////////////////////////////////////////////////////////
				
	vNew1 = vec_perm(vIn1, vIn2, vMergeHiPairPerm);
	vNew2 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
				
	///////////////////////////////////////////////////////////////////////
	// do twist multiplication on both row vectors
	///////////////////////////////////////////////////////////////////////

	vOut1 = vec_madd(vCosTwistA0, vNew1, vZero);
	vOut1 = vec_madd(vSinTwistA0, vec_perm(vNew1, vNew1, vSwappedPerm), vOut1);

	vOut2 = vec_madd(vCosTwistA1, vNew2, vZero);
	vOut2 = vec_madd(vSinTwistA1, vec_perm(vNew2, vNew2, vSwappedPerm), vOut2);
				
	///////////////////////////////////////////////////////////////////////
	// store row vectors back into the matrix
	///////////////////////////////////////////////////////////////////////
				
	*pCurrentColumn = vOut1;
	pCurrentColumn += colCount / 2;
	*pCurrentColumn = vOut2;				
	pCurrentColumn += colCount / 2;
								
	///////////////////////////////////////////////////////////////////////
	// update sin & cos vectors for twist multiply
	///////////////////////////////////////////////////////////////////////

	vCosTemp0 = vec_nmsub(vCosTwistA0, vA, vCosTwistA0);
	vCosTemp0 = vec_nmsub(vSinTwistA0, vB, vCosTemp0);

	vSinTwistA0 = vec_nmsub(vSinTwistA0, vA, vSinTwistA0);
	vSinTwistA0 = vec_madd (vCosTwistA0, vB, vSinTwistA0);

	vCosTwistA0 = vCosTemp0;

	vCosTemp1 = vec_nmsub(vCosTwistA1, vA, vCosTwistA1);
	vCosTemp1 = vec_nmsub(vSinTwistA1, vB, vCosTemp1);

	vSinTwistA1 = vec_nmsub(vSinTwistA1, vA, vSinTwistA1);
	vSinTwistA1 = vec_madd (vCosTwistA1, vB, vSinTwistA1);

	vCosTwistA1 = vCosTemp1;
      }
						
		
    }
		
    if (result == 0) {
      ///////////////////////////////////////////////////////////////////////
      // finally, loop through all rows of matrix, performing an FFT
      // on each row
      ///////////////////////////////////////////////////////////////////////

      pRow = pData;
			
      for (i=rowCount-1; i>=0; i--) {
	result = FFTComplex(pRow+i*(colCount*2), colCount, -1);
	if (result != 0) break;
      }
    }
  }

  if (rowBufferPointer) {
    free(rowBufferPointer);
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////
//	fft_matrix_inverse_columnwise
//
//	Performs a recursive 2D matrix inverse FFT on the data pointed
// to by pData. It assumes that the data is in columnwise order, and
// returns the resulting data in "normal" linear (lexicographic)
// order.
//
//	There are three steps to the 2D matrix inverse FFT.  First, a
// FFT is performed on each row of the matrix.  Second, each element
// in the matrix is multiplied by a twist factor. In a length N
// signal, element X(j, k) is multiplied by e^(+/- 2 pi i j k / N
// ). Finally, an FFT is performed on each column of the matrix.  The
// resulting data is the recovered signal from the inverse FFT, in
// linear order.
//
////////////////////////////////////////////////////////////////////////////
static int fft_matrix_inverse_columnwise(float *pData, int32 length)
{
  int32  i, j;
  int32  pow;
  float    *pRow;
  int32  rowCount;
  int32  colCount;
  int32  rowPower;
  int32  colPower;
  float*   rowBufferPointer = 0;
  int32  result = 0;
			
  pow = log2max(length);
    
  //////////////////////////////////////////////////////////////////////////
  // length must be an exact power of 2
  //////////////////////////////////////////////////////////////////////////
  if ((1 << pow) != length) {
    return EINVAL;
  }
    
  //////////////////////////////////////////////////////////////////////////
  // data must be 16-byte aligned because of vector load/store requirements
  //////////////////////////////////////////////////////////////////////////
  if (((int32)pData) & 0x0F) {
    return EINVAL;
  }
 
 
  //////////////////////////////////////////////////////////////////////////
  // calculate dimensions of matrix.  If matrix can not be square, then the
  // column count will be 2X the row count    
  //////////////////////////////////////////////////////////////////////////

  rowPower = pow / 2;

  colPower = rowPower;

  if (pow & 1) colPower++;

  rowCount = 1<<rowPower;
  colCount = 1<<colPower;

  //////////////////////////////////////////////////////////////////////////
  // allocate a buffer that will be used for column ferrying, and can store
  // two columns at a time.
  //////////////////////////////////////////////////////////////////////////

  rowBufferPointer = (float*) valloc(2*2*sizeof(float)*rowCount);

  if (!rowBufferPointer) result = ENOMEM;
	
  if (result == 0) {
    vector float         *pCurrentColumn;
    vector float         vIn1, vIn2;
    vector float         vOut1, vOut2;
    vector unsigned char vMergeHiPairPerm;
    vector unsigned char vMergeLoPairPerm;
    vector float         *pSplit1, *pSplit2;
    float                *pFFT1, *pFFT2;
    vector unsigned char vSwappedPerm;
    vector float         vSinSignMultiplier;
    vector float         vCosTwistA0, vCosTwistA1, vCosTemp0, vCosTemp1;

    vector float         vSinTwistA0, vSinTwistA1;

    vector float         vA, vB, vTransition;
    vector float         vZero = Altivec_Const4(vector float, 0, 0, 0, 0);
		
    double               fTemp, baseAngle1, baseAngle2;

    double               fSinBaseAngle1, fSinBaseAngle2;
		
    ////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x0 x1 y0 y1
    ////////////////////////////////////////////////////////////////////////
    vMergeHiPairPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5,
				       6, 7, 16, 17, 18, 19, 20, 21, 22, 23);

    ////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x2 x3 y2 y3
    ////////////////////////////////////////////////////////////////////////
    vMergeLoPairPerm = Altivec_Const16(vector unsigned char, 8, 9, 10, 11,
				       12, 13, 14, 15, 24, 25, 26, 27, 28,
				       29, 30, 31);
		
    ////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x1 x0 x3 y2
    ////////////////////////////////////////////////////////////////////////
    vSwappedPerm  = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 0, 1,
				    2, 3, 12, 13, 14, 15, 8, 9, 10, 11);
		
    ////////////////////////////////////////////////////////////////////////
    // vSinSignMultiplier is a vector used to create a vector in the form
    // -sin(x) sin(x) -sin(y) sin(y)
    ////////////////////////////////////////////////////////////////////////
    vSinSignMultiplier = Altivec_Const4(vector float, -1, 1, -1, 1);
		
    ////////////////////////////////////////////////////////////////////////
    // perform a FFT on every row of the matrix
    ////////////////////////////////////////////////////////////////////////
    pRow = pData;
		
    for (i = rowCount-1; i >= 0; i--) {
      result = FFTComplex(pRow+i*(colCount*2), colCount, 1);
      if (result != 0) break;
    }

    if (result == 0) {
      ////////////////////////////////////////////////////////////////////////
      // we now loop through all columns, two columns at a time, and
      // perform column ferrying FFTs on the columns.  We must also
      // perform a twist multiply on all entries, so we do this after
      // we load in the columns, and before we ferry them over to the
      // temporary buffer
      ////////////////////////////////////////////////////////////////////////

      for (i = 0; i < colCount/2; i++) {
	//////////////////////////////////////////////////////////////////////
	// set up vectors for incremental updating of cos & sin
	// vectors
	//
	// Given c = cos(w) and s = sin(w), if we want to find the cos
	// and sin of w plus some small angle d, then we can do so by
	// defining:
	//	
	// a = 2 * ((sin(d/2)) ^ 2)
	// b = sin(d)
	// 
	// Then, we can calculate the cos and sin of the updated angle
	// w+d as:
	// 
	// cos(w+d) = c - ac - bs
	// sin(w+d) = s - as + bc
	// 
	// the vectors vA and vB contain these increment factors a and
	// b.  Note that each outer loop iteration requires different
	// increment angles.  Looking at the matrix, the angle
	// multipliers that we use for our cos & sin twist multipliers
	// look something like:
	//
	//  0/N	   0/N	 0/N	0/N	0/N	0/N	0/N	...	0/N
	//  0/N	   1/N	 2/N	3/N	4/N	5/N	6/N		
	//  0/N	   2/N	 4/N	6/N	8/N    10/N    12/N	.
	//  0/N	   3/N	 6/N	9/N    12/N    15/N    18/N	.	
	//  0/N	   4/N	 8/N   12/N    16/N    20/N    24/N
	//		.		.
	//		.		.
	//  0/N				...			     (N-1)(N-1)
	//
	// Since we're doing two columns at a time, the vA and vB
	// vectors need to have two different sets of increment
	// values.  For example, for the third and fourth columns
	// (handled in the second time through the outer loop) each
	// row increments by 2/N and 3/N.  Since we actually have two
	// sets of twist multiplier vectors, we skip a row for each,
	// so the increment angle is doubled.  This helps reduce
	// calculation dependencies and cuts the number of increment
	// iterations in half, which helps reduce errors in the
	// single-precision calculations used.
	//		
	//////////////////////////////////////////////////////////////////////

	baseAngle1 = 4*i*PI/(colCount*rowCount);
	baseAngle2 = (2*(2*i+1)*PI)/(colCount*rowCount);

	fSinBaseAngle1 = sin(baseAngle1);
	fTemp = 2*fSinBaseAngle1*fSinBaseAngle1;
				
	((float*)&vTransition)[0] = fTemp;
	((float*)&vTransition)[2] = sin(2*baseAngle1);

	fSinBaseAngle2 = sin(baseAngle2);
	fTemp = 2*fSinBaseAngle2*fSinBaseAngle2;

	((float*)&vTransition)[1] = fTemp;
	((float*)&vTransition)[3] = sin(2*baseAngle2);

	vA = vec_mergeh(vTransition, vTransition);
	vB = vec_mergel(vTransition, vTransition);
				
	vB = vec_madd(vB, vSinSignMultiplier, vZero);

	//////////////////////////////////////////////////////////////////////
	// Set up the initial twist multiplier vectors for the
	// beginning of this column.  Values are calculated in the
	// scalar domain, and then transferred to the vector domain
	// via the vTransition vector.
	//////////////////////////////////////////////////////////////////////
				
	((float*)&vTransition)[0] = 1;
				
	((float*)&vTransition)[2] = cos(baseAngle1);
	((float*)&vTransition)[3] = cos(baseAngle2);
				
	vCosTwistA0 = vec_splat(vTransition, 0);
	vCosTwistA1 = vec_mergel(vTransition, vTransition);
				
	vSinTwistA0 = vZero;
				
	((float*)&vTransition)[2] = fSinBaseAngle1;
	((float*)&vTransition)[3] = fSinBaseAngle2;
										
	vSinTwistA1 = vec_mergel(vTransition, vTransition);
	vSinTwistA1 = vec_madd(vSinTwistA1, vSinSignMultiplier, vZero);

	//////////////////////////////////////////////////////////////////////
	// initialize pointers in temporary buffer for storing copied columns.
	//////////////////////////////////////////////////////////////////////
			
	pSplit1 = (vector float*)rowBufferPointer;
	pSplit2 = pSplit1 + rowCount / 2;
				
	//////////////////////////////////////////////////////////////////////
	// point at top of columns to be copied
	//////////////////////////////////////////////////////////////////////

	pCurrentColumn = ((vector float*)pData) + i;
			
	//////////////////////////////////////////////////////////////////////
	// loop through all rows in the current column, copying
	// elements to the temporary buffer.  We load two rows at a
	// time, and permute the vectors to create a single vector
	// that contains the appropriate column elements from both
	// rows.
	//////////////////////////////////////////////////////////////////////

	for (j = 0; j < rowCount/2; j++) {
	  vector float	vTwistedIn1, vTwistedIn2;

	  ////////////////////////////////////////////////////////////////////
	  // read in two rows worth of vectors (two rows X two columns)
	  ////////////////////////////////////////////////////////////////////

	  vIn1 = *pCurrentColumn;
	  pCurrentColumn += colCount/2;
	  vIn2 = *pCurrentColumn;
	  pCurrentColumn += colCount/2;

	  ////////////////////////////////////////////////////////////////////
	  // twist multiply input vectors
	  ////////////////////////////////////////////////////////////////////

	  vTwistedIn1 = vec_madd(vCosTwistA0, vIn1, vZero);
	  vTwistedIn1 = vec_madd(vSinTwistA0, vec_perm(vIn1, vIn1, vSwappedPerm),
				 vTwistedIn1);

	  vTwistedIn2 = vec_madd(vCosTwistA1, vIn2, vZero);
	  vTwistedIn2 = vec_madd(vSinTwistA1, vec_perm(vIn2, vIn2, vSwappedPerm),
				 vTwistedIn2);

	  ////////////////////////////////////////////////////////////////////
	  // permute the vectors so that the two left column entries
	  // are in one vector and the two right column entries are in
	  // one vector
	  ////////////////////////////////////////////////////////////////////
									
	  vOut1 = vec_perm(vTwistedIn1, vTwistedIn2, vMergeHiPairPerm);
	  vOut2 = vec_perm(vTwistedIn1, vTwistedIn2, vMergeLoPairPerm);
					
	  ////////////////////////////////////////////////////////////////////
	  // store the split columns to the appropriate buffers
	  ////////////////////////////////////////////////////////////////////

	  *pSplit1++ = vOut1;
	  *pSplit2++ = vOut2;

	  ////////////////////////////////////////////////////////////////////
	  // update sin & cos vectors for twist multiply
	  ////////////////////////////////////////////////////////////////////

	  vCosTemp0 = vec_nmsub(vCosTwistA0, vA, vCosTwistA0);
	  vCosTemp0 = vec_nmsub(vSinTwistA0, vB, vCosTemp0);

	  vSinTwistA0 = vec_nmsub(vSinTwistA0, vA, vSinTwistA0);
	  vSinTwistA0 = vec_madd (vCosTwistA0, vB, vSinTwistA0);

	  vCosTwistA0 = vCosTemp0;

	  vCosTemp1 = vec_nmsub(vCosTwistA1, vA, vCosTwistA1);
	  vCosTemp1 = vec_nmsub(vSinTwistA1, vB, vCosTemp1);

	  vSinTwistA1 = vec_nmsub(vSinTwistA1, vA, vSinTwistA1);
	  vSinTwistA1 = vec_madd (vCosTwistA1, vB, vSinTwistA1);

	  vCosTwistA1 = vCosTemp1;
	}

	////////////////////////////////////////////////////////////////////
	// perform FFTs on the two columns of data that we have copied
	// to the temp buffer
	////////////////////////////////////////////////////////////////////

	pFFT1 = (float *)rowBufferPointer;
	pFFT2 = pFFT1 + 2*rowCount;
				
	result = FFTComplex(pFFT1, rowCount, 1);
	if (result != 0) break;
				
	result = FFTComplex(pFFT2, rowCount, 1);
	if (result != 0) break;

	////////////////////////////////////////////////////////////////////
	// point at the beginning of the two copied column buffers,
	// and at the top of the columns in the matrix where we will
	// store them back.
	////////////////////////////////////////////////////////////////////

	pSplit1 = (vector float*)rowBufferPointer;
	pSplit2 = pSplit1 + rowCount / 2;
	pCurrentColumn = ((vector float*)pData) + i;

	////////////////////////////////////////////////////////////////////
	// loop through all of the column entries that are stored in
	// the temp buffer, and merge them to be stored back into the
	// columns of the matrix.
	////////////////////////////////////////////////////////////////////

	for (j = 0; j < rowCount/2; j++) {
	  ////////////////////////////////////////////////////////////////////
	  // get two vectors of column data
	  ////////////////////////////////////////////////////////////////////

	  vIn1 = *pSplit1++;
	  vIn2 = *pSplit2++;
					
	  ////////////////////////////////////////////////////////////////////
	  // turn them into row vectors
	  ////////////////////////////////////////////////////////////////////

	  vOut1 = vec_perm(vIn1, vIn2, vMergeHiPairPerm);
	  vOut2 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
									
	  ////////////////////////////////////////////////////////////////////
	  // store row vectors back into the matrix
	  ////////////////////////////////////////////////////////////////////

	  *pCurrentColumn = vOut1;
	  pCurrentColumn += colCount / 2;
	  *pCurrentColumn = vOut2;				
	  pCurrentColumn += colCount / 2;
	}
      }
    }
  }

  if (rowBufferPointer) {
    free(rowBufferPointer);
  }

  return result;
}



/////////////////////////////////////////////////////////////////////////////
//	fft_square_matrix
//
//	Performs a forward or inverse FFT on the data pointed to by
// pData.  If isign == -1, then a forward FFT is performed, otherwise
// an inverse FFT is performed.  The FFT is performed by doing a
// recursive matrix FFT.  The length of the data must be an even power
// of two, to ensure that the matrix is a square, which allows for a
// trivial transpose of the matrix to ensure that output data is in
// lexicographic order (or to transform input data into columnwise
// order, in the case of the inverse FFT).
/////////////////////////////////////////////////////////////////////////////
static int fft_square_matrix(float *pData, int32 length, int32 isign)
{
  int32  pow;
  int32  result = 0;
			
  pow = log2max(length);
    
  ///////////////////////////////////////////////////////////////////////////
  // length must be an exact power of 2
  ///////////////////////////////////////////////////////////////////////////
  if ((1 << pow) != length) {
    return EINVAL;
  }
    
  ///////////////////////////////////////////////////////////////////////////
  // length must be an even power of 2
  ///////////////////////////////////////////////////////////////////////////
  if (pow & 1) {
    return EINVAL;
  }


  if (isign == -1) {
    /////////////////////////////////////////////////////////////////////////
    // we are performing a forward FFT, so do a normal forward matrix
    // FFT, which will leave the data in columnwise order
    /////////////////////////////////////////////////////////////////////////

    result = fft_matrix_forward_columnwise(pData, length);

    if (result == 0) {
      ///////////////////////////////////////////////////////////////////////
      // transpose the square matrix, so that data is no longer in
      // columnwise order
      ///////////////////////////////////////////////////////////////////////

      SquareComplexTransposeVector(pData, 1<<(pow/2));
    }

  } else {
    /////////////////////////////////////////////////////////////////////////
    // transpose the square matrix of input data, to transform it into
    // columnwise order.
    /////////////////////////////////////////////////////////////////////////
    SquareComplexTransposeVector(pData, 1<<(pow/2));

    /////////////////////////////////////////////////////////////////////////
    // perform an inverse matrix FFT on the data, which expects the
    // data to be in columnwise order, and leaves the result in
    // lexicographic order
    /////////////////////////////////////////////////////////////////////////

    result = fft_matrix_inverse_columnwise(pData, length);	
  }

  return result;
}

#pragma mark -
#pragma mark C O M P L E X   F F T

/////////////////////////////////////////////////////////////////////////////
// fft_altivec
//
//	Performs a forward or inverse FFT on the complex signal data
// pointed to by pData. pTempBuffer must point to a buffer that is of
// equal length as the signal pointed to by pData, and which will be
// overwritten by the routine.  If isign == -1, then a forward FFT is
// performed.  Otherwise an inverse FFT is performed.
//
// requirements:
//
//	- length must be an exact power of 2 
//	- pData and pTempBuffer must be 16-byte aligned
//	- signal length must be at least 16, because of vector sizes
//	  and implementation details.
//
/////////////////////////////////////////////////////////////////////////////

static int32 fft_altivec(float *pData, float *pTempBuffer, int32 len,
			   int32 isign)
{
  int32               j, i;
  float                 *srcPtr, *dstPtr, *tmp;
  int32               pow, root, trig;
  float                 *sinCosTable;
  int32               result = 0;   

  vector float          vZero;
  vector unsigned char  vSwappedPerm;
  vector unsigned char  vMergeHiPairPerm;
  vector unsigned char  vMergeLoPairPerm;
  vector unsigned char  vCosPermute;
  vector signed int     vSinNegSinSelect;
  vector unsigned char  vSinPermute;

  vector float          vCSLoad1, vCSLoad2, vNegCSLoad1, vNegCSLoad2;

  vector float          vCosA1, vSinA1, vCosA2, vSinA2;

  vector float          vIn1, vIn2, vIn3, vIn4;
	
  vector float          vDiffA1, vDiffA2, vSumA1, vSumA2;
	
  vector float          vSwappedDiffA1, vSwappedDiffA2;
	
  vector float          vButterflyA1, vButterflyA2;

  vector float          vInLoB1, vInHiB1, vInLoB2, vInHiB2;
  vector float          vSinB1, vSinB2, vCosB1, vCosB2;

  vector float          vDiffB1, vDiffB2, vSwappedDiffB1, vSwappedDiffB2;
	
  vector float          vResultLoB1, vResultLoB2, vResultHiB1, vResultHiB2;

  vector float          vCSLoadB1, vNegSinB1, vCSLoadB2, vNegSinB2;

  vector float          *pInVec1, *pInVec2, *pInVec3, *pInVec4;
  vector float          *pOutVec1, *pOutVec2, *pOutVec3, *pOutVec4;

  vector float          vInLo1, vInLo2, vInHi1, vInHi2;
  vector float          vResultLo1, vResultLo2, vResultHi1, vResultHi2;

  vector float          vSinSignMultiplier;

  vector float          vTransitionFloatVector, vInverseDivideMultiplier;

	
  ///////////////////////////////////////////////////////////////////////////
  // calculate log2(len)
  ///////////////////////////////////////////////////////////////////////////	
  pow = log2max(len);
    
  ///////////////////////////////////////////////////////////////////////////
  // length must be an exact power of 2
  ///////////////////////////////////////////////////////////////////////////
  if ((1 << pow) != len) {
    return EINVAL;
  }
    
  ///////////////////////////////////////////////////////////////////////////
  // data must be 16-byte aligned because of vector load/store requirements
  ///////////////////////////////////////////////////////////////////////////
  if (((int32)pData) & 0x0F) {
    return EINVAL;
  }

  ///////////////////////////////////////////////////////////////////////////
  // temp buffer must be 16-byte aligned because of vector load/store
  // requirements
  ///////////////////////////////////////////////////////////////////////////
  if (((int32)pTempBuffer) & 0x0F) {
    return EINVAL;
  }


  ///////////////////////////////////////////////////////////////////////////
  // make sure that the sin cos lookup table is for the current fft length
  ///////////////////////////////////////////////////////////////////////////
  if (len != gSinCosTableSize) result = InitFFTSinCos(len);
    
  if (result == 0) {

    /////////////////////////////////////////////////////////////////////////
    // initialize a zero vector
    /////////////////////////////////////////////////////////////////////////
    vZero = Altivec_Const4(vector float, 0, 0, 0, 0);

    /////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x1 x0 x3 y2
    /////////////////////////////////////////////////////////////////////////
    vSwappedPerm  = Altivec_Const16(vector unsigned char,4, 5, 6, 7, 0, 1, 2,
				    3, 12, 13, 14, 15, 8, 9, 10, 11);

    /////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x0 x1 y0 y1
    /////////////////////////////////////////////////////////////////////////
    vMergeHiPairPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5,
				       6, 7, 16, 17, 18, 19, 20, 21, 22, 23);

    /////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x2 x3 y2 y3
    /////////////////////////////////////////////////////////////////////////
    vMergeLoPairPerm = Altivec_Const16(vector unsigned char, 8, 9, 10, 11,
				       12, 13, 14, 15, 24, 25, 26, 27, 28,
				       29, 30, 31);

    /////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x0 x0 x2 x2
    /////////////////////////////////////////////////////////////////////////
    vCosPermute = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 0, 1, 2,
				  3, 8, 9, 10, 11, 8, 9, 10, 11);

    /////////////////////////////////////////////////////////////////////////
    // initialize permute, select, and 
    // multiplier vectors based on whether
    // we are doing a forward or inverse
    // fft.
    /////////////////////////////////////////////////////////////////////////
    if (isign < 0) {
      /////////////////////////////////////////////////////////////////////////
      // initialize a multiplier vector that will
      // negate the second and fourth float
      // elements.
      /////////////////////////////////////////////////////////////////////////
      vSinSignMultiplier = Altivec_Const4(vector float, 1, -1, 1, -1);

      /////////////////////////////////////////////////////////////////////////
      // initialize a select vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = x0 y1 x2 y3
      /////////////////////////////////////////////////////////////////////////
      vSinNegSinSelect = Altivec_Const4(vector signed int, 0, -1, 0, -1);
			
      /////////////////////////////////////////////////////////////////////////
      // initialize a permute vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = x1 y1 x3 y3
      /////////////////////////////////////////////////////////////////////////
      vSinPermute = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 20, 21,
				    22, 23, 12, 13, 14, 15, 28, 29, 30, 31);
			
    } else {
      /////////////////////////////////////////////////////////////////////////
      // initialize a multiplier vector that will
      // negate the first and third float
      // elements.
      /////////////////////////////////////////////////////////////////////////
      vSinSignMultiplier = Altivec_Const4(vector float, -1, 1, -1, 1);

      /////////////////////////////////////////////////////////////////////////
      // initialize a select vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = y0 x1 y2 x3
      /////////////////////////////////////////////////////////////////////////
      vSinNegSinSelect = Altivec_Const4(vector signed int, -1, 0, -1, 0);    	

      /////////////////////////////////////////////////////////////////////////
      // initialize a permute vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = y1 x1 y3 x3
      /////////////////////////////////////////////////////////////////////////
      vSinPermute = Altivec_Const16(vector unsigned char, 20, 21, 22, 23, 4, 5,
				    6, 7, 28, 29, 30, 31, 12, 13, 14, 15);
    }

    ///////////////////////////////////////////////////////////////////////////
    // init trig counter, for indexing output vectors 
    ///////////////////////////////////////////////////////////////////////////
    trig = 1;
	    	    
    ///////////////////////////////////////////////////////////////////////////
    // start with data as source and temp buffer as destination
    ///////////////////////////////////////////////////////////////////////////
    srcPtr = pData;
    dstPtr = pTempBuffer;

    ///////////////////////////////////////////////////////////////////////////
    // create pointer to start of cos/sin table for array indexing
    ////////////////////////////////////////////////////////////////////////////////
    sinCosTable = (float*)gSinCosTablePtr;

    ///////////////////////////////////////////////////////////////////////////
    // initialize root to zero.  Root is used as an index into the sin
    // & cos table
    ///////////////////////////////////////////////////////////////////////////
    root = 0;

    do {
      /////////////////////////////////////////////////////////////////////////
      // load sin & cos
      /////////////////////////////////////////////////////////////////////////
      vCSLoad1 = *(vector float*)&sinCosTable[root];
      vCSLoad2 = *(vector float*)&sinCosTable[(len/2)+root];
			
      /////////////////////////////////////////////////////////////////////////
      // create negative sin & cos
      /////////////////////////////////////////////////////////////////////////
      vNegCSLoad1 = vec_sub(vZero, vCSLoad1);
      vNegCSLoad2 = vec_sub(vZero, vCSLoad2);

      /////////////////////////////////////////////////////////////////////////
      // create vector
      //
      // vCosA1 = cos(root*pi/len) cos(root*pi/len)
      //          cos((root+2)*pi/len) cos((root+2)*pi/len)
      /////////////////////////////////////////////////////////////////////////
      vCosA1 = vec_perm(vCSLoad1, vCSLoad1, vCosPermute);

      /////////////////////////////////////////////////////////////////////////
      // create vector
      //
      // 	vSinA1 = sin(root*pi/len) -sin(root*pi/len)
      //                 sin((root+2)*pi/len) -sin((root+2)*pi/len)
      //
      // or
      //
      // 	vSinA1 = -sin(root*pi/len) sin(root*pi/len)
      //                 -sin((root+2)*pi/len) sin((root+2)*pi/len)
      //
      // depending on whether we are doing a forward or inverse fft
      /////////////////////////////////////////////////////////////////////////
      vSinA1 = vec_perm(vCSLoad1, vNegCSLoad1, vSinPermute);

      /////////////////////////////////////////////////////////////////////////
      // create vector
      //
      //    vCosA2 = cos(pi/2 + root*pi/len) cos(pi/2 + root*pi/len)
      //             cos(pi/2 + (root+2)*pi/len) cos(pi/2 + (root+2)*pi/len)
      /////////////////////////////////////////////////////////////////////////
      vCosA2 = vec_perm(vCSLoad2, vCSLoad2, vCosPermute);

      /////////////////////////////////////////////////////////////////////////
      // create vector
      //
      //    vSinA1 = sin(pi/2 + root*pi/len) -sin(pi/2 + root*pi/len)
      //             sin(pi/2 + (root+2)*pi/len) -sin(pi/2 + (root+2)*pi/len)
      //
      // or
      //
      //    vSinA1 = -sin(pi/2 + root*pi/len) sin(pi/2 + root*pi/len)
      //             -sin(pi/2 + (root+2)*pi/len) sin(pi/2 + (root+2)*pi/len)
      //
      // depending on whether we are doing a forward or inverse fft
      /////////////////////////////////////////////////////////////////////////
      vSinA2 = vec_perm(vCSLoad2, vNegCSLoad2, vSinPermute);

      /////////////////////////////////////////////////////////////////////////
      // load four input vectors for calculating butterflies
      /////////////////////////////////////////////////////////////////////////
      vIn1 = *(vector float*)srcPtr;
      vIn2 = *(vector float*)&srcPtr[len];
      vIn3 = *(vector float*)&srcPtr[len/2];
      vIn4 = *(vector float*)&srcPtr[len + len/2];

      /////////////////////////////////////////////////////////////////////////
      // calculate four butterflies of input vectors (two from vIn1
      // and vIn2, two from vIn3 and vIn4).
      /////////////////////////////////////////////////////////////////////////
			
      vDiffA1 = vec_sub(vIn1, vIn2);
      vDiffA2 = vec_sub(vIn3, vIn4);

      vSumA1 = vec_add(vIn1, vIn2);
      vSumA2 = vec_add(vIn3, vIn4);
			
      vSwappedDiffA1 = vec_perm(vDiffA1, vDiffA1, vSwappedPerm);
      vSwappedDiffA2 = vec_perm(vDiffA2, vDiffA2, vSwappedPerm);
			
      vButterflyA1 = vec_madd(vDiffA1, vCosA1, vZero);
      vButterflyA2 = vec_madd(vDiffA2, vCosA2, vZero);

      vButterflyA1 = vec_madd(vSwappedDiffA1, vSinA1, vButterflyA1);
      vButterflyA2 = vec_madd(vSwappedDiffA2, vSinA2, vButterflyA2);

      /////////////////////////////////////////////////////////////////////////
      // results of butterflies from first stage are input to butterflies
      // of second stage
      /////////////////////////////////////////////////////////////////////////

      vInLoB1 = vec_perm(vSumA1, vButterflyA1, vMergeHiPairPerm);
      vInLoB2 = vec_perm(vSumA1, vButterflyA1, vMergeLoPairPerm);
			
      vInHiB1 = vec_perm(vSumA2, vButterflyA2, vMergeHiPairPerm);
      vInHiB2 = vec_perm(vSumA2, vButterflyA2, vMergeLoPairPerm);

      /////////////////////////////////////////////////////////////////////////
      // load sin & cos and generate sin, cos vectors for second-stage
      // butterfly
      /////////////////////////////////////////////////////////////////////////

      vCSLoadB1 = *(vector float*)&sinCosTable[2*root];
      vSinB1 = vec_splat(vCSLoadB1, 1);
      vCosB1 = vec_splat(vCSLoadB1, 0);
      vNegSinB1 = vec_sub(vZero, vSinB1);

      vSinB1 = vec_sel(vSinB1, vNegSinB1, (vector unsigned int)vSinNegSinSelect);

      vCSLoadB2 = *(vector float*)&sinCosTable[2*root+4];
      vSinB2 = vec_splat(vCSLoadB2, 1);
      vCosB2 = vec_splat(vCSLoadB2, 0);
      vNegSinB2 = vec_sub(vZero, vSinB2);

      vSinB2 = vec_sel(vSinB2, vNegSinB2, (vector unsigned int)vSinNegSinSelect);
			
      vDiffB1 = vec_sub(vInLoB1, vInHiB1);
      vDiffB2 = vec_sub(vInLoB2, vInHiB2);
			
      vSwappedDiffB1 = vec_perm(vDiffB1, vDiffB1, vSwappedPerm);
      vSwappedDiffB2 = vec_perm(vDiffB2, vDiffB2, vSwappedPerm);
			
      vResultLoB1 = vec_add(vInLoB1, vInHiB1);
      vResultLoB2 = vec_add(vInLoB2, vInHiB2);

      vResultHiB1 = vec_madd(vCosB1, vDiffB1, vZero);
      vResultHiB1 = vec_madd(vSinB1, vSwappedDiffB1, vResultHiB1);

      vResultHiB2 = vec_madd(vCosB2, vDiffB2, vZero);
      vResultHiB2 = vec_madd(vSinB2, vSwappedDiffB2, vResultHiB2);
					
      /////////////////////////////////////////////////////////////////////////
      // store results of second stage butterfly
      /////////////////////////////////////////////////////////////////////////

      *(vector float*)dstPtr 			= vResultLoB1;
      *(vector float*)(dstPtr+4) 		= vResultHiB1;
      *(vector float*)(dstPtr+8)		= vResultLoB2;
      *(vector float*)(dstPtr+12) 	= vResultHiB2;

      /////////////////////////////////////////////////////////////////////////
      // update pointers and sin/cos root index for next time through loop
      /////////////////////////////////////////////////////////////////////////
      srcPtr += 4; 		
      dstPtr += 16;
      root += 4;		
			
    } while (root < len/2);
		
		
    /////////////////////////////////////////////////////////////////////////
    // for second step, source is temp buffer, and destination is original
    // data buffer
    /////////////////////////////////////////////////////////////////////////
    srcPtr = pTempBuffer;
    dstPtr = pData;

    trig *= 4;
		
    /////////////////////////////////////////////////////////////////////////
    // In the ping-pong FFT, for each power of two, butterflies are
    // calculated using all elements in the data.  To eliminate
    // load/store operations, we perform a "double butterfly",
    // essentially doing two steps of butterflies for each pass
    // through the data.  So, we need to do pow/2 passes through the
    // data.  We've already done the first pass, so we must do
    // (pow-2/2) loops through the data at this point.
    /////////////////////////////////////////////////////////////////////////
    for(i = (pow-2)/2; i > 0; i--) {
      ///////////////////////////////////////////////////////////////////////
      // initialize source pointers
      ///////////////////////////////////////////////////////////////////////
      pInVec1 = (vector float*)srcPtr;
      pInVec2 = (vector float*)&srcPtr[len];
      pInVec3 = (vector float*)&srcPtr[len/2];	
      pInVec4 = (vector float*)&srcPtr[len + len/2];		
								
      root = 0;

      ///////////////////////////////////////////////////////////////////////
      // if this is the last time through the loop, and the length is
      // an even power of two, but an odd power of four, then the
      // source data is the input buffer, and the dest data would be
      // the temp buffer.  However, for the last step, the input data
      // and output data indices for the butterflies are the same, so
      // we can write directly back into the source data.  This
      // eliminates the need to copy from the temp buffer back to the
      // original buffer to return the fft result data.
      ///////////////////////////////////////////////////////////////////////
      if (i == 1) {
			
	/////////////////////////////////////////////////////////////////////
	// if the length is an even power of two, then this is the
	// last iteration that we will go through.  Otherwise we'll do
	// an additional single butterfly pass through the data.
	/////////////////////////////////////////////////////////////////////
	if (!(pow & 1)) {
	  ///////////////////////////////////////////////////////////////////
	  // we're copying from original buffer back into original buffer
	  ///////////////////////////////////////////////////////////////////
	  if ((pow & 3) == 2) {
	    dstPtr = srcPtr;
	  }
				
	  ///////////////////////////////////////////////////////////////////
	  // load sin & cos for first butterfly, and generate sin &
	  // cos vectors
	  ///////////////////////////////////////////////////////////////////
	  vCSLoad1 = *(vector float*)&sinCosTable[root];
					
	  vCosA1 = vec_splat(vCSLoad1, 0);
	  vCosA2 = vec_splat(vCSLoad1, 1);

	  vSinA1 = vec_madd(vCosA2, vSinSignMultiplier, vZero);
					
	  vCosA2 = vec_sub(vZero, vCosA2);
					
	  vSinA2 = vec_madd(vCosA1, vSinSignMultiplier, vZero);

	  ///////////////////////////////////////////////////////////////////
	  // set up output data pointers
	  ///////////////////////////////////////////////////////////////////

	  pOutVec1 = ((vector float*)dstPtr)+root;
	  pOutVec2 = ((vector float*)(dstPtr + trig*4))+root;
	  pOutVec3 = ((vector float*)(dstPtr + trig*2))+root;
	  pOutVec4 = ((vector float*)(dstPtr + trig*6))+root;

	  if (isign > 0) {
	    /////////////////////////////////////////////////////////////////
	    // we are performing an inverse FFT, so we need to divide
	    // the final results by the length of the FFT input data.
	    // Since there is no vector divide, we create a float
	    // vector of 1/N, and then do a multiply of the data
	    // before we store it back.
	    //
	    // We use a transition vector to go from the scalar to
	    // vector domain.  We don't store directly to our
	    // multiplier vector, so that the compiler can more easily
	    // optimize the multiplier vector to be register-based
	    // rather than stack-based.
	    /////////////////////////////////////////////////////////////////
	    ((float*)&vTransitionFloatVector)[0] = (double)1/(double)len;
						
	    vInverseDivideMultiplier = vec_splat(vTransitionFloatVector, 0);
						
	    for(j = trig/4; j > 0; j--) {

	      ///////////////////////////////////////////////////////////////
	      // load in four input vectors
	      ///////////////////////////////////////////////////////////////
	      vIn1 = *pInVec1++;
	      vIn2 = *pInVec2++;
	      vIn3 = *pInVec3++;
	      vIn4 = *pInVec4++;
							
	      ///////////////////////////////////////////////////////////////
	      // calculate butterflies for first stage
	      ///////////////////////////////////////////////////////////////
							
	      vDiffA1 = vec_sub(vIn1, vIn2);
	      vDiffA2 = vec_sub(vIn3, vIn4);

	      vSumA1 = vec_add(vIn1, vIn2);
	      vSumA2 = vec_add(vIn3, vIn4);
							
	      vSwappedDiffA1 = vec_perm(vDiffA1, vDiffA1, vSwappedPerm);
	      vSwappedDiffA2 = vec_perm(vDiffA2, vDiffA2, vSwappedPerm);
							
	      vButterflyA1 = vec_madd(vDiffA1, vCosA1, vZero);
	      vButterflyA1 = vec_madd(vSwappedDiffA1, vSinA1, vButterflyA1);

	      vButterflyA2 = vec_madd(vDiffA2, vCosA2, vZero);
	      vButterflyA2 = vec_madd(vSwappedDiffA2, vSinA2, vButterflyA2);
							
	      ///////////////////////////////////////////////////////////////
	      // calculate butterflies for second stage
	      ///////////////////////////////////////////////////////////////

	      vDiffB1 = vec_sub(vSumA1, vSumA2);
	      vDiffB2 = vec_sub(vButterflyA1, vButterflyA2);
							
	      vResultLoB1 = vec_add(vSumA1, vSumA2);
	      vResultLoB2 = vec_add(vButterflyA1, vButterflyA2);

	      vSwappedDiffB1 = vec_perm(vDiffB1, vDiffB1, vSwappedPerm);
	      vSwappedDiffB2 = vec_perm(vDiffB2, vDiffB2, vSwappedPerm);

	      vResultHiB1 = vec_madd(vCosA1, vDiffB1, vZero);
	      vResultHiB1 = vec_madd(vSinA1, vSwappedDiffB1, vResultHiB1);

	      vResultHiB2 = vec_madd(vCosA1, vDiffB2, vZero);
	      vResultHiB2 = vec_madd(vSinA1, vSwappedDiffB2, vResultHiB2);

	      ///////////////////////////////////////////////////////////////
	      // since we're performing an inverse FFT, we now divide
	      // each element by len before we store it back.
	      ///////////////////////////////////////////////////////////////
						
	      vResultLoB1 = vec_madd(vResultLoB1, vInverseDivideMultiplier, vZero);
	      vResultLoB2 = vec_madd(vResultLoB2, vInverseDivideMultiplier, vZero);
	      vResultHiB1 = vec_madd(vResultHiB1, vInverseDivideMultiplier, vZero);
	      vResultHiB2 = vec_madd(vResultHiB2, vInverseDivideMultiplier, vZero);

	      ///////////////////////////////////////////////////////////////
	      // For speed, we unroll the loop once. This allows the
	      // compiler to better optimize the object code.  Because
	      // the compiler doesn't move loads and stores around
	      // each other, the code is faster if we explicitly move
	      // the second set of input vector loads above the first
	      // set of output vector stores, which allows the
	      // compiler to better optimize the dispatch of the input
	      // loads.
	      ///////////////////////////////////////////////////////////////

	      ///////////////////////////////////////////////////////////////
	      // load input vectors for second half of unrolled loop
	      ///////////////////////////////////////////////////////////////

	      vIn1 = *pInVec1++;
	      vIn2 = *pInVec2++;
	      vIn3 = *pInVec3++;
	      vIn4 = *pInVec4++;

	      ///////////////////////////////////////////////////////////////
	      // store second stage butterfly output of first half of
	      // unrolled loop to dest buffer
	      ///////////////////////////////////////////////////////////////

	      *pOutVec1++ = vResultLoB1;
	      *pOutVec2++ = vResultHiB1;
	      *pOutVec3++ = vResultLoB2;
	      *pOutVec4++ = vResultHiB2;
						
	      ///////////////////////////////////////////////////////////////
	      // calculate butterflies for first stage
	      ///////////////////////////////////////////////////////////////
							
	      vDiffA1 = vec_sub(vIn1, vIn2);
	      vDiffA2 = vec_sub(vIn3, vIn4);

	      vSumA1 = vec_add(vIn1, vIn2);
	      vSumA2 = vec_add(vIn3, vIn4);
							
	      vSwappedDiffA1 = vec_perm(vDiffA1, vDiffA1, vSwappedPerm);
	      vSwappedDiffA2 = vec_perm(vDiffA2, vDiffA2, vSwappedPerm);
							
	      vButterflyA1 = vec_madd(vDiffA1, vCosA1, vZero);
	      vButterflyA1 = vec_madd(vSwappedDiffA1, vSinA1, vButterflyA1);

	      vButterflyA2 = vec_madd(vDiffA2, vCosA2, vZero);
	      vButterflyA2 = vec_madd(vSwappedDiffA2, vSinA2, vButterflyA2);
							
	      ///////////////////////////////////////////////////////////////
	      // calculate butterflies for second stage
	      ///////////////////////////////////////////////////////////////
							
	      vDiffB1 = vec_sub(vSumA1, vSumA2);
	      vDiffB2 = vec_sub(vButterflyA1, vButterflyA2);
							
	      vResultLoB1 = vec_add(vSumA1, vSumA2);
	      vResultLoB2 = vec_add(vButterflyA1, vButterflyA2);

	      vSwappedDiffB1 = vec_perm(vDiffB1, vDiffB1, vSwappedPerm);
	      vSwappedDiffB2 = vec_perm(vDiffB2, vDiffB2, vSwappedPerm);

	      vResultHiB1 = vec_madd(vCosA1, vDiffB1, vZero);
	      vResultHiB1 = vec_madd(vSinA1, vSwappedDiffB1, vResultHiB1);

	      vResultHiB2 = vec_madd(vCosA1, vDiffB2, vZero);
	      vResultHiB2 = vec_madd(vSinA1, vSwappedDiffB2, vResultHiB2);

	      ///////////////////////////////////////////////////////////////
	      // since we're performing an inverse FFT, we now divide
	      // each element by len before we store it back.
	      ///////////////////////////////////////////////////////////////
						
	      vResultLoB1 = vec_madd(vResultLoB1, vInverseDivideMultiplier, vZero);
	      vResultLoB2 = vec_madd(vResultLoB2, vInverseDivideMultiplier, vZero);
	      vResultHiB1 = vec_madd(vResultHiB1, vInverseDivideMultiplier, vZero);
	      vResultHiB2 = vec_madd(vResultHiB2, vInverseDivideMultiplier, vZero);

	      ///////////////////////////////////////////////////////////////
	      // store second stage butterfly output of second half of
	      // unrolled loop to dest buffer
	      ///////////////////////////////////////////////////////////////

	      *pOutVec1++ = vResultLoB1;
	      *pOutVec2++ = vResultHiB1;
	      *pOutVec3++ = vResultLoB2;
	      *pOutVec4++ = vResultHiB2;				
	    }
	  } else {
	    /////////////////////////////////////////////////////////////////
	    // perform normal butterfly calculations for forward FFT
	    /////////////////////////////////////////////////////////////////

	    for(j = trig/4; j > 0; j--) {
	      ///////////////////////////////////////////////////////////////
	      // load in four input vectors
	      ///////////////////////////////////////////////////////////////
	      vIn1 = *pInVec1++;
	      vIn2 = *pInVec2++;
	      vIn3 = *pInVec3++;
	      vIn4 = *pInVec4++;
							
	      ///////////////////////////////////////////////////////////////
	      // calculate butterflies for first stage
	      ///////////////////////////////////////////////////////////////
							
	      vDiffA1 = vec_sub(vIn1, vIn2);
	      vDiffA2 = vec_sub(vIn3, vIn4);

	      vSumA1 = vec_add(vIn1, vIn2);
	      vSumA2 = vec_add(vIn3, vIn4);
							
	      vSwappedDiffA1 = vec_perm(vDiffA1, vDiffA1, vSwappedPerm);
	      vSwappedDiffA2 = vec_perm(vDiffA2, vDiffA2, vSwappedPerm);
							
	      vButterflyA1 = vec_madd(vDiffA1, vCosA1, vZero);
	      vButterflyA1 = vec_madd(vSwappedDiffA1, vSinA1, vButterflyA1);

	      vButterflyA2 = vec_madd(vDiffA2, vCosA2, vZero);
	      vButterflyA2 = vec_madd(vSwappedDiffA2, vSinA2, vButterflyA2);
							
	      ///////////////////////////////////////////////////////////////
	      // calculate butterflies for second stage
	      ///////////////////////////////////////////////////////////////

	      vDiffB1 = vec_sub(vSumA1, vSumA2);
	      vDiffB2 = vec_sub(vButterflyA1, vButterflyA2);
							
	      vResultLoB1 = vec_add(vSumA1, vSumA2);
	      vResultLoB2 = vec_add(vButterflyA1, vButterflyA2);

	      vSwappedDiffB1 = vec_perm(vDiffB1, vDiffB1, vSwappedPerm);
	      vSwappedDiffB2 = vec_perm(vDiffB2, vDiffB2, vSwappedPerm);

	      vResultHiB1 = vec_madd(vCosA1, vDiffB1, vZero);
	      vResultHiB1 = vec_madd(vSinA1, vSwappedDiffB1, vResultHiB1);

	      vResultHiB2 = vec_madd(vCosA1, vDiffB2, vZero);
	      vResultHiB2 = vec_madd(vSinA1, vSwappedDiffB2, vResultHiB2);

	      ///////////////////////////////////////////////////////////////
	      // For speed, we unroll the loop once. This allows the
	      // compiler to better optimize the object code.  Because
	      // the compiler doesn't move loads and stores around
	      // each other, the code is faster if we explicitly move
	      // the second set of input vector loads above the first
	      // set of output vector stores, which allows the
	      // compiler to better optimize the dispatch of the input
	      // loads.
	      ///////////////////////////////////////////////////////////////

	      ///////////////////////////////////////////////////////////////
	      // load input vectors for second half of
	      // unrolled loop
	      ///////////////////////////////////////////////////////////////

	      vIn1 = *pInVec1++;
	      vIn2 = *pInVec2++;
	      vIn3 = *pInVec3++;
	      vIn4 = *pInVec4++;

	      ///////////////////////////////////////////////////////////////
	      // store second stage butterfly output of first half of
	      // unrolled loop to dest buffer
	      ///////////////////////////////////////////////////////////////

	      *pOutVec1++ = vResultLoB1;
	      *pOutVec2++ = vResultHiB1;
	      *pOutVec3++ = vResultLoB2;
	      *pOutVec4++ = vResultHiB2;
						
	      ///////////////////////////////////////////////////////////////
	      // calculate butterflies for first stage
	      ///////////////////////////////////////////////////////////////
							
	      vDiffA1 = vec_sub(vIn1, vIn2);
	      vDiffA2 = vec_sub(vIn3, vIn4);

	      vSumA1 = vec_add(vIn1, vIn2);
	      vSumA2 = vec_add(vIn3, vIn4);
							
	      vSwappedDiffA1 = vec_perm(vDiffA1, vDiffA1, vSwappedPerm);
	      vSwappedDiffA2 = vec_perm(vDiffA2, vDiffA2, vSwappedPerm);
							
	      vButterflyA1 = vec_madd(vDiffA1, vCosA1, vZero);
	      vButterflyA1 = vec_madd(vSwappedDiffA1, vSinA1, vButterflyA1);

	      vButterflyA2 = vec_madd(vDiffA2, vCosA2, vZero);
	      vButterflyA2 = vec_madd(vSwappedDiffA2, vSinA2, vButterflyA2);
							
	      ///////////////////////////////////////////////////////////////
	      // calculate butterflies for second stage
	      ///////////////////////////////////////////////////////////////
							
	      vDiffB1 = vec_sub(vSumA1, vSumA2);
	      vDiffB2 = vec_sub(vButterflyA1, vButterflyA2);
							
	      vResultLoB1 = vec_add(vSumA1, vSumA2);
	      vResultLoB2 = vec_add(vButterflyA1, vButterflyA2);

	      vSwappedDiffB1 = vec_perm(vDiffB1, vDiffB1, vSwappedPerm);
	      vSwappedDiffB2 = vec_perm(vDiffB2, vDiffB2, vSwappedPerm);

	      vResultHiB1 = vec_madd(vCosA1, vDiffB1, vZero);
	      vResultHiB1 = vec_madd(vSinA1, vSwappedDiffB1, vResultHiB1);

	      vResultHiB2 = vec_madd(vCosA1, vDiffB2, vZero);
	      vResultHiB2 = vec_madd(vSinA1, vSwappedDiffB2, vResultHiB2);

	      ///////////////////////////////////////////////////////////////
	      // store second stage butterfly output of second half of
	      // unrolled loop to dest buffer
	      ///////////////////////////////////////////////////////////////

	      *pOutVec1++ = vResultLoB1;
	      *pOutVec2++ = vResultHiB1;
	      *pOutVec3++ = vResultLoB2;
	      *pOutVec4++ = vResultHiB2;				
	    }
	  }

	  return result;
	}
      }

      ///////////////////////////////////////////////////////////////////////
      // a special case for what would be the first iteration through
      // the "while (root < len/2)" loop below, for which root = 0.
      // When root is 0, the sin cos table loads for root are the same
      // as the sin cos table loads for 2*root, so we special-case
      // this instance to avoid the extra load and permutes to setup
      // the sin & cos vectors.
      ///////////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////////
      // load sin & cos for first butterfly, and generate sin & cos vectors
      ///////////////////////////////////////////////////////////////////////
      vCSLoad1 = *(vector float*)&sinCosTable[root];
			
      vCosA1 = vec_splat(vCSLoad1, 0);
      vCosA2 = vec_splat(vCSLoad1, 1);

      vSinA1 = vec_madd(vCosA2, vSinSignMultiplier, vZero);
			
      vCosA2 = vec_sub(vZero, vCosA2);
			
      vSinA2 = vec_madd(vCosA1, vSinSignMultiplier, vZero);

      ///////////////////////////////////////////////////////////////////////
      // set up output data pointers
      ///////////////////////////////////////////////////////////////////////

      pOutVec1 = ((vector float*)dstPtr)+root;
      pOutVec2 = ((vector float*)(dstPtr + trig*4))+root;
      pOutVec3 = ((vector float*)(dstPtr + trig*2))+root;
      pOutVec4 = ((vector float*)(dstPtr + trig*6))+root;
			
      for(j = trig/4; j > 0; j--) {

	/////////////////////////////////////////////////////////////////////
	// load in four input vectors
	/////////////////////////////////////////////////////////////////////
				
	vIn1 = *pInVec1++;
	vIn2 = *pInVec2++;
	vIn3 = *pInVec3++;
	vIn4 = *pInVec4++;
				
	/////////////////////////////////////////////////////////////////////
	// calculate butterflies for first stage
	/////////////////////////////////////////////////////////////////////
				
	vDiffA1 = vec_sub(vIn1, vIn2);
	vDiffA2 = vec_sub(vIn3, vIn4);

	vSumA1 = vec_add(vIn1, vIn2);
	vSumA2 = vec_add(vIn3, vIn4);
				
	vSwappedDiffA1 = vec_perm(vDiffA1, vDiffA1, vSwappedPerm);
	vSwappedDiffA2 = vec_perm(vDiffA2, vDiffA2, vSwappedPerm);
				
	vButterflyA1 = vec_madd(vDiffA1, vCosA1, vZero);
	vButterflyA1 = vec_madd(vSwappedDiffA1, vSinA1, vButterflyA1);

	vButterflyA2 = vec_madd(vDiffA2, vCosA2, vZero);
	vButterflyA2 = vec_madd(vSwappedDiffA2, vSinA2, vButterflyA2);
				
	/////////////////////////////////////////////////////////////////////
	// calculate butterflies for second stage
	/////////////////////////////////////////////////////////////////////

	vDiffB1 = vec_sub(vSumA1, vSumA2);
	vDiffB2 = vec_sub(vButterflyA1, vButterflyA2);
				
	vResultLoB1 = vec_add(vSumA1, vSumA2);
	vResultLoB2 = vec_add(vButterflyA1, vButterflyA2);

	vSwappedDiffB1 = vec_perm(vDiffB1, vDiffB1, vSwappedPerm);
	vSwappedDiffB2 = vec_perm(vDiffB2, vDiffB2, vSwappedPerm);

	vResultHiB1 = vec_madd(vCosA1, vDiffB1, vZero);
	vResultHiB1 = vec_madd(vSinA1, vSwappedDiffB1, vResultHiB1);

	vResultHiB2 = vec_madd(vCosA1, vDiffB2, vZero);
	vResultHiB2 = vec_madd(vSinA1, vSwappedDiffB2, vResultHiB2);

	/////////////////////////////////////////////////////////////////////
	// For speed, we unroll the loop once. This allows the
	// compiler to better optimize the object code.  Because the
	// compiler doesn't move loads and stores around each other,
	// the code is faster if we explicitly move the second set of
	// input vector loads above the first set of output vector
	// stores, which allows the compiler to better optimize the
	// dispatch of the input loads.
	/////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////
	// load input vectors for second half of unrolled loop
	/////////////////////////////////////////////////////////////////////

	vIn1 = *pInVec1++;
	vIn2 = *pInVec2++;
	vIn3 = *pInVec3++;
	vIn4 = *pInVec4++;

	/////////////////////////////////////////////////////////////////////
	// store second stage butterfly output of first half of
	// unrolled loop to dest buffer
	/////////////////////////////////////////////////////////////////////

	*pOutVec1++ = vResultLoB1;
	*pOutVec2++ = vResultHiB1;
	*pOutVec3++ = vResultLoB2;
	*pOutVec4++ = vResultHiB2;
			
	/////////////////////////////////////////////////////////////////////
	// calculate butterflies for first stage
	/////////////////////////////////////////////////////////////////////
				
	vDiffA1 = vec_sub(vIn1, vIn2);
	vDiffA2 = vec_sub(vIn3, vIn4);

	vSumA1 = vec_add(vIn1, vIn2);
	vSumA2 = vec_add(vIn3, vIn4);
				
	vSwappedDiffA1 = vec_perm(vDiffA1, vDiffA1, vSwappedPerm);
	vSwappedDiffA2 = vec_perm(vDiffA2, vDiffA2, vSwappedPerm);
				
	vButterflyA1 = vec_madd(vDiffA1, vCosA1, vZero);
	vButterflyA1 = vec_madd(vSwappedDiffA1, vSinA1, vButterflyA1);

	vButterflyA2 = vec_madd(vDiffA2, vCosA2, vZero);
	vButterflyA2 = vec_madd(vSwappedDiffA2, vSinA2, vButterflyA2);
				
	/////////////////////////////////////////////////////////////////////
	// calculate butterflies for second stage
	/////////////////////////////////////////////////////////////////////
				
	vDiffB1 = vec_sub(vSumA1, vSumA2);
	vDiffB2 = vec_sub(vButterflyA1, vButterflyA2);
				
	vResultLoB1 = vec_add(vSumA1, vSumA2);
	vResultLoB2 = vec_add(vButterflyA1, vButterflyA2);

	vSwappedDiffB1 = vec_perm(vDiffB1, vDiffB1, vSwappedPerm);
	vSwappedDiffB2 = vec_perm(vDiffB2, vDiffB2, vSwappedPerm);

	vResultHiB1 = vec_madd(vCosA1, vDiffB1, vZero);
	vResultHiB1 = vec_madd(vSinA1, vSwappedDiffB1, vResultHiB1);

	vResultHiB2 = vec_madd(vCosA1, vDiffB2, vZero);
	vResultHiB2 = vec_madd(vSinA1, vSwappedDiffB2, vResultHiB2);

	/////////////////////////////////////////////////////////////////////
	// store second stage butterfly output of second half of
	// unrolled loop to dest buffer
	/////////////////////////////////////////////////////////////////////

	*pOutVec1++ = vResultLoB1;
	*pOutVec2++ = vResultHiB1;
	*pOutVec3++ = vResultLoB2;
	*pOutVec4++ = vResultHiB2;				
				
      }

      /////////////////////////////////////////////////////////////////////
      // update root value for sin & cos load
      /////////////////////////////////////////////////////////////////////
      root += 2*trig;

			
      while (root < len/2) {      
	/////////////////////////////////////////////////////////////////////
	// load sin & cos
	/////////////////////////////////////////////////////////////////////
	vCSLoad1 = *(vector float*)&sinCosTable[root];
				
	/////////////////////////////////////////////////////////////////////
	// create vector
	//
	// 	vCosA1 = cos(root*pi/len) cos(root*pi/len)
	//               cos((root+2)*pi/len) cos((root+2)*pi/len)
	/////////////////////////////////////////////////////////////////////
	vCosA1 = vec_splat(vCSLoad1, 0);
				
				
	/////////////////////////////////////////////////////////////////////
	// create vector
	//
	// vCosA2 = -cos(pi/2 + root*pi/len) -cos(pi/2 + root*pi/len)
	//          -cos(pi/2 + (root+2)*pi/len) -cos(pi/2 + (root+2)*pi/len)
	/////////////////////////////////////////////////////////////////////
	vCosA2 = vec_splat(vCSLoad1, 1);

	/////////////////////////////////////////////////////////////////////
	// create vector
	//
	// vSinA1 = sin(root*pi/len) -sin(root*pi/len)
	//          sin((root+2)*pi/len) -sin((root+2)*pi/len)
	//
	// or
	//
	// vSinA1 = -sin(root*pi/len) sin(root*pi/len)
	//          -sin((root+2)*pi/len) sin((root+2)*pi/len)
	//
	// depending on whether we are doing a forward or inverse fft
	/////////////////////////////////////////////////////////////////////
	vSinA1 = vec_madd(vCosA2, vSinSignMultiplier, vZero);
				
	/////////////////////////////////////////////////////////////////////
	// turn negative cos to positive cos
	/////////////////////////////////////////////////////////////////////
	vCosA2 = vec_sub(vZero, vCosA2);
				
	/////////////////////////////////////////////////////////////////////
	// create vector
	//
	// vSinA1 = sin(pi/2 + root*pi/len) -sin(pi/2 + root*pi/len)
	//          sin(pi/2 + (root+2)*pi/len) -sin(pi/2 + (root+2)*pi/len)
	//
	// or
	//
	// vSinA1 = -sin(pi/2 + root*pi/len) sin(pi/2 + root*pi/len)
	//          -sin(pi/2 + (root+2)*pi/len) sin(pi/2 + (root+2)*pi/len)
	//
	// depending on whether we are doing a forward or inverse fft
	/////////////////////////////////////////////////////////////////////
	vSinA2 = vec_madd(vCosA1, vSinSignMultiplier, vZero);

	/////////////////////////////////////////////////////////////////////
	// load sin & cos and generate sin, cos vectors for
	// second-stage butterfly
	/////////////////////////////////////////////////////////////////////
	vCSLoadB1 = *(vector float*)&sinCosTable[2*root];
	vSinB1 = vec_splat(vCSLoadB1, 1);
	vCosB1 = vec_splat(vCSLoadB1, 0);

	vNegSinB1 = vec_sub(vZero, vSinB1);

	vSinB1 = vec_sel(vSinB1, vNegSinB1, (vector unsigned int)vSinNegSinSelect);

	/////////////////////////////////////////////////////////////////////
	// set up output data pointers
	/////////////////////////////////////////////////////////////////////

	pOutVec1 = ((vector float*)dstPtr)+root;
	pOutVec2 = ((vector float*)(dstPtr + trig*4))+root;
	pOutVec3 = ((vector float*)(dstPtr + trig*2))+root;
	pOutVec4 = ((vector float*)(dstPtr + trig*6))+root;
				

	for(j = trig/4; j > 0; j--) {
	  ///////////////////////////////////////////////////////////////////
	  // load in four input vectors
	  ///////////////////////////////////////////////////////////////////
	  vIn1 = *pInVec1++;
	  vIn2 = *pInVec2++;
	  vIn3 = *pInVec3++;
	  vIn4 = *pInVec4++;

	  ///////////////////////////////////////////////////////////////////
	  // calculate butterflies for first stage
	  ///////////////////////////////////////////////////////////////////

	  vDiffA1 = vec_sub(vIn1, vIn2);
	  vDiffA2 = vec_sub(vIn3, vIn4);

	  vSumA1 = vec_add(vIn1, vIn2);
	  vSumA2 = vec_add(vIn3, vIn4);

	  vSwappedDiffA1 = vec_perm(vDiffA1, vDiffA1, vSwappedPerm);
	  vSwappedDiffA2 = vec_perm(vDiffA2, vDiffA2, vSwappedPerm);

	  vButterflyA1 = vec_madd(vDiffA1, vCosA1, vZero);
	  vButterflyA1 = vec_madd(vSwappedDiffA1, vSinA1, vButterflyA1);

	  vButterflyA2 = vec_madd(vDiffA2, vCosA2, vZero);
	  vButterflyA2 = vec_madd(vSwappedDiffA2, vSinA2, vButterflyA2);

	  ///////////////////////////////////////////////////////////////////
	  // calculate butterflies for second stage
	  ///////////////////////////////////////////////////////////////////

	  vDiffB1 = vec_sub(vSumA1, vSumA2);
	  vDiffB2 = vec_sub(vButterflyA1, vButterflyA2);

	  vResultLoB1 = vec_add(vSumA1, vSumA2);
	  vResultLoB2 = vec_add(vButterflyA1, vButterflyA2);

	  vSwappedDiffB1 = vec_perm(vDiffB1, vDiffB1, vSwappedPerm);
	  vSwappedDiffB2 = vec_perm(vDiffB2, vDiffB2, vSwappedPerm);

	  vResultHiB1 = vec_madd(vCosB1, vDiffB1, vZero);
	  vResultHiB1 = vec_madd(vSinB1, vSwappedDiffB1, vResultHiB1);

	  vResultHiB2 = vec_madd(vCosB1, vDiffB2, vZero);
	  vResultHiB2 = vec_madd(vSinB1, vSwappedDiffB2, vResultHiB2);

	  ///////////////////////////////////////////////////////////////////
	  // For speed, we unroll the loop once. This allows the
	  // compiler to better optimize the object code.  Because the
	  // compiler doesn't move loads and stores around each other,
	  // the code is faster if we explicitly move the second set
	  // of input vector loads above the first set of output
	  // vector stores, which allows the compiler to better
	  // optimize the dispatch of the input loads.
	  ///////////////////////////////////////////////////////////////////

	  ///////////////////////////////////////////////////////////////////
	  // load input vectors for second half of
	  // unrolled loop
	  ///////////////////////////////////////////////////////////////////

	  vIn1 = *pInVec1++;
	  vIn2 = *pInVec2++;
	  vIn3 = *pInVec3++;
	  vIn4 = *pInVec4++;

	  ///////////////////////////////////////////////////////////////////
	  // store second stage butterfly output of first half of
	  // unrolled loop to dest buffer
	  ///////////////////////////////////////////////////////////////////

	  *pOutVec1++ = vResultLoB1;
	  *pOutVec2++ = vResultHiB1;
	  *pOutVec3++ = vResultLoB2;
	  *pOutVec4++ = vResultHiB2;

	  ///////////////////////////////////////////////////////////////////
	  // calculate butterflies for first stage
	  ///////////////////////////////////////////////////////////////////

	  vDiffA1 = vec_sub(vIn1, vIn2);
	  vDiffA2 = vec_sub(vIn3, vIn4);

	  vSumA1 = vec_add(vIn1, vIn2);
	  vSumA2 = vec_add(vIn3, vIn4);

	  vSwappedDiffA1 = vec_perm(vDiffA1, vDiffA1, vSwappedPerm);
	  vSwappedDiffA2 = vec_perm(vDiffA2, vDiffA2, vSwappedPerm);

	  vButterflyA1 = vec_madd(vDiffA1, vCosA1, vZero);
	  vButterflyA1 = vec_madd(vSwappedDiffA1, vSinA1, vButterflyA1);

	  vButterflyA2 = vec_madd(vDiffA2, vCosA2, vZero);
	  vButterflyA2 = vec_madd(vSwappedDiffA2, vSinA2, vButterflyA2);

	  ///////////////////////////////////////////////////////////////////
	  // calculate butterflies for second stage
	  ///////////////////////////////////////////////////////////////////

	  vDiffB1 = vec_sub(vSumA1, vSumA2);
	  vDiffB2 = vec_sub(vButterflyA1, vButterflyA2);

	  vResultLoB1 = vec_add(vSumA1, vSumA2);
	  vResultLoB2 = vec_add(vButterflyA1, vButterflyA2);

	  vSwappedDiffB1 = vec_perm(vDiffB1, vDiffB1, vSwappedPerm);
	  vSwappedDiffB2 = vec_perm(vDiffB2, vDiffB2, vSwappedPerm);

	  vResultHiB1 = vec_madd(vCosB1, vDiffB1, vZero);
	  vResultHiB1 = vec_madd(vSinB1, vSwappedDiffB1, vResultHiB1);

	  vResultHiB2 = vec_madd(vCosB1, vDiffB2, vZero);
	  vResultHiB2 = vec_madd(vSinB1, vSwappedDiffB2, vResultHiB2);

	  ///////////////////////////////////////////////////////////////////
	  // store second stage butterfly output of second half of
	  // unrolled loop to dest buffer
	  ///////////////////////////////////////////////////////////////////

	  *pOutVec1++ = vResultLoB1;
	  *pOutVec2++ = vResultHiB1;
	  *pOutVec3++ = vResultLoB2;
	  *pOutVec4++ = vResultHiB2;
	}

	root += 2*trig;
      }

      trig *= 4;
      tmp = dstPtr; dstPtr = srcPtr; srcPtr = tmp;
    }

    /////////////////////////////////////////////////////////////////////////
    // if the length is an odd power of two, then we need to do one
    // more single-butterfly pass through the data.  Since we know
    // that the sin and cos values for this last pass are 0 and 1, we
    // can simplify the butterfly calculations to adds and subtracts.
    /////////////////////////////////////////////////////////////////////////

    if (pow & 1) {
      ///////////////////////////////////////////////////////////////////////
      // output always goes back to original input data buffer		
      ///////////////////////////////////////////////////////////////////////
						
      pOutVec1 = (vector float*)pData;
			
      ///////////////////////////////////////////////////////////////////////
      // set source pointer to load from whatever the last step's
      // output data was
      ///////////////////////////////////////////////////////////////////////

      if (pow & 2) {
	pInVec1 = (vector float*)pTempBuffer;
      } else {
	pInVec1 = (vector float*)pData;
      }
					
      ///////////////////////////////////////////////////////////////////////
      // set second input and output vectors to point half-way 
      ///////////////////////////////////////////////////////////////////////
      pInVec2 = pInVec1+(len/4);	
      pOutVec2 = pOutVec1+(len/4);

      if (isign > 0) {
	/////////////////////////////////////////////////////////////////////
	// we are performing an inverse FFT, so we need to divide the
	// final results by the length of the FFT input data.  Since
	// there is no vector divide, we create a float vector of 1/N,
	// and then do a multiply of the data before we store it back.
	//
	// We use a transition vector to go from the scalar to vector
	// domain.  We don't store directly to our multiplier vector,
	// so that the compiler can more easily optimize the
	// multiplier vector to be register-based rather than
	// stack-based.
	/////////////////////////////////////////////////////////////////////
	((float*)&vTransitionFloatVector)[0] = (double)1/(double)len;
				
	vInverseDivideMultiplier = vec_splat(vTransitionFloatVector, 0);

	for(j = trig/8; j > 0; j--) {
	  ///////////////////////////////////////////////////////////////////
	  // load four input vectors
	  ///////////////////////////////////////////////////////////////////
	  vInLo1 = *pInVec1++;
	  vInHi1 = *pInVec2++;
	  vInLo2 = *pInVec1++;
	  vInHi2 = *pInVec2++;
					
	  ///////////////////////////////////////////////////////////////////
	  // calc simplified butterflies, multiplying by inverse
	  // correction factor.
	  ///////////////////////////////////////////////////////////////////

	  vInLo1 = vec_madd(vInLo1, vInverseDivideMultiplier, vZero);
					
	  vResultHi1 = vec_nmsub(vInHi1, vInverseDivideMultiplier, vInLo1);
	  vResultLo1 = vec_madd(vInHi1, vInverseDivideMultiplier, vInLo1);

	  vInLo2 = vec_madd(vInLo2, vInverseDivideMultiplier, vZero);
					
	  vResultHi2 = vec_nmsub(vInHi2, vInverseDivideMultiplier, vInLo2);
	  vResultLo2 = vec_madd(vInHi2, vInverseDivideMultiplier, vInLo2);

	  ///////////////////////////////////////////////////////////////////
	  // load input vectors for second half of unrolled loop
	  ///////////////////////////////////////////////////////////////////
				
	  vInLo1 = *pInVec1++;
	  vInHi1 = *pInVec2++;
	  vInLo2 = *pInVec1++;
	  vInHi2 = *pInVec2++;

	  ///////////////////////////////////////////////////////////////////
	  // store results from first half of unrolled loop
	  ///////////////////////////////////////////////////////////////////
			
	  *pOutVec1++ = vResultLo1;
	  *pOutVec2++ = vResultHi1;
	  *pOutVec1++ = vResultLo2;
	  *pOutVec2++ = vResultHi2;

	  ///////////////////////////////////////////////////////////////////
	  // calc simplified butterflies, multiplying by inverse
	  // multiplier factor.
	  ///////////////////////////////////////////////////////////////////

	  vInLo1 = vec_madd(vInLo1, vInverseDivideMultiplier, vZero);
					
	  vResultHi1 = vec_nmsub(vInHi1, vInverseDivideMultiplier, vInLo1);
	  vResultLo1 = vec_madd(vInHi1, vInverseDivideMultiplier, vInLo1);

	  vInLo2 = vec_madd(vInLo2, vInverseDivideMultiplier, vZero);
					
	  vResultHi2 = vec_nmsub(vInHi2, vInverseDivideMultiplier, vInLo2);
	  vResultLo2 = vec_madd(vInHi2, vInverseDivideMultiplier, vInLo2);

	  ///////////////////////////////////////////////////////////////////
	  // store results from second half of unrolled loop
	  ///////////////////////////////////////////////////////////////////
			
	  *pOutVec1++ = vResultLo1;
	  *pOutVec2++ = vResultHi1;
	  *pOutVec1++ = vResultLo2;
	  *pOutVec2++ = vResultHi2;
	}
      } else {
	for(j = trig/8; j > 0; j--) {
	  ///////////////////////////////////////////////////////////////////
	  // load four input vectors
	  ///////////////////////////////////////////////////////////////////
	  vInLo1 = *pInVec1++;
	  vInHi1 = *pInVec2++;
	  vInLo2 = *pInVec1++;
	  vInHi2 = *pInVec2++;
					
	  ///////////////////////////////////////////////////////////////////
	  // calc simplified butterflies 
	  ///////////////////////////////////////////////////////////////////
	  vResultHi1 = vec_sub(vInLo1, vInHi1);
	  vResultLo1 = vec_add(vInLo1, vInHi1);
							
	  vResultHi2 = vec_sub(vInLo2, vInHi2);				
	  vResultLo2 = vec_add(vInLo2, vInHi2);

	  ///////////////////////////////////////////////////////////////////
	  // load input vectors for second half of unrolled loop
	  ///////////////////////////////////////////////////////////////////
				
	  vInLo1 = *pInVec1++;
	  vInHi1 = *pInVec2++;
	  vInLo2 = *pInVec1++;
	  vInHi2 = *pInVec2++;

	  ///////////////////////////////////////////////////////////////////
	  // store results from first half of unrolled loop
	  ///////////////////////////////////////////////////////////////////
			
	  *pOutVec1++ = vResultLo1;
	  *pOutVec2++ = vResultHi1;
	  *pOutVec1++ = vResultLo2;
	  *pOutVec2++ = vResultHi2;


	  ///////////////////////////////////////////////////////////////////
	  // calc simplified butterflies 
	  ///////////////////////////////////////////////////////////////////

	  vResultHi1 = vec_sub(vInLo1, vInHi1);
	  vResultLo1 = vec_add(vInLo1, vInHi1);
			
	  vResultHi2 = vec_sub(vInLo2, vInHi2);				
	  vResultLo2 = vec_add(vInLo2, vInHi2);

	  ///////////////////////////////////////////////////////////////////
	  // store results from second half of unrolled loop
	  ///////////////////////////////////////////////////////////////////
			
	  *pOutVec1++ = vResultLo1;
	  *pOutVec2++ = vResultHi1;
	  *pOutVec1++ = vResultLo2;
	  *pOutVec2++ = vResultHi2;
	}
      }
    }
  }
	
  return result;
}


/////////////////////////////////////////////////////////////////////////////
// fft_scalar
//
// Ping-pong Stockham FFT
//
//	Performs a forward or inverse FFT on the complex signal data
// pointed to by pData. pTempBuffer must point to a buffer that is of
// equal length as the signal pointed to by pData, and which will be
// overwritten by the routine.  If isign == -1, then a forward FFT is
// performed.  Otherwise an inverse FFT is performed.
//
// requirements:
//
//	- length must be an exact power of 2 
//
/////////////////////////////////////////////////////////////////////////////
static int32 fft_scalar(float *pData, float *tempbuff, int32 len,
			  int32 isign)
{
  int32  j, i;
  float    c, s, tre, tim, *srcDataPtr, *dstDataPtr, *tmp;
  double   inverseDivideMultiplier;
  int32  pow, root, trig;
  int32  result = 0;
  float	   *sinCosTable;

  ///////////////////////////////////////////////////////////////////////////
  // calculate log2(len)
  ///////////////////////////////////////////////////////////////////////////
	
  pow = log2max(len);
	
  ///////////////////////////////////////////////////////////////////////////
  // length must be an exact power of 2	
  ///////////////////////////////////////////////////////////////////////////
	
  if ((1 << pow) != len) return EINVAL;

  ///////////////////////////////////////////////////////////////////////////
  // make sure that the sin cos lookup table is for the current fft length
  ///////////////////////////////////////////////////////////////////////////
  if (len != gSinCosTableSize) result = InitFFTSinCos(len);

  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////
    // initialize "trig" step for destination index
    /////////////////////////////////////////////////////////////////////////

    trig = 1;
		
    /////////////////////////////////////////////////////////////////////////
    // Start with data as source, and temp buffer as destination
    /////////////////////////////////////////////////////////////////////////

    srcDataPtr = pData;
    dstDataPtr = tempbuff;

    /////////////////////////////////////////////////////////////////////////
    // get pointer to start of sin/cos table pointer for array reference
    /////////////////////////////////////////////////////////////////////////
		
    sinCosTable = (float*)gSinCosTablePtr;

    for(i = pow-1; i > 0; i--) {

      root = 0;
      while(root < len/2) {
	/////////////////////////////////////////////////////////////////////
	// load cos and sin values for current root value.
	/////////////////////////////////////////////////////////////////////

	c = sinCosTable[2*root];
	s = isign*sinCosTable[2*root+1];

	for(j = trig; j > 0; j--) {
	  ///////////////////////////////////////////////////////////////////
	  // calculate butterflies.  For inputs:
	  // [re0, im0] [re1, im1]
	  //
	  // we calculate:
	  //
	  // out0 = [re0+re1, im0+im1]
	  //
	  // out1 = [ cos * (re0-re1) - sin * (im0-im1),
	  //			sin * (re0-re1) + cos * (im0-im1) ]
	  //
	  ///////////////////////////////////////////////////////////////////
					
	  tre = srcDataPtr[0] - srcDataPtr[len];
	  tim = srcDataPtr[1] - srcDataPtr[len + 1];
	  dstDataPtr[0] = srcDataPtr[0] + srcDataPtr[len];
	  dstDataPtr[1] = srcDataPtr[1] + srcDataPtr[len+1];
	  dstDataPtr[2*trig] = c*tre - s*tim;
	  dstDataPtr[2*trig + 1] = s*tre + c*tim;
	  srcDataPtr += 2; dstDataPtr += 2;
	}

	///////////////////////////////////////////////////////////////////
	// update dest pointer and root value
	///////////////////////////////////////////////////////////////////
				
	dstDataPtr += 2*trig;
	root += trig;
      }

      /////////////////////////////////////////////////////////////////////
      // update trig value and ping pong source and dest pointers
      /////////////////////////////////////////////////////////////////////

      trig *= 2; srcDataPtr -= len; dstDataPtr -= 2*len;
      tmp = srcDataPtr; srcDataPtr = dstDataPtr; dstDataPtr = tmp; 

    }

    ///////////////////////////////////////////////////////////////////////
    // For last iteration, we are fortunate in that the source indices
    // for the butterfly calculations are the same as the destination
    // indices. This is not true for other loop iterations, which is
    // why we cannot simply store our results directly back to the
    // data -- we would overwrite source data that had not yet been
    // used for the current iteration of the loop. If the power of two
    // of the length is odd, then, on the second to last iteration,
    // the data will end up ping-ponged back to the source buffer.  If
    // this is the case, then we set source and dest to be the
    // same. Otherwise, if the power of two is even, then we will be
    // reading from temp data, and writing back to our original
    // buffer.
    ///////////////////////////////////////////////////////////////////////

    if (pow & 1) {
      dstDataPtr = srcDataPtr;
    }
	
    root = 0;
		
    if (isign > 0) {
      /////////////////////////////////////////////////////////////////////
      // we are performing an inverse FFT.  We need to divide all
      // elements of the final result by len.  Since the float
      // multiply operation is faster than the divide operation, we
      // multiply by the reciprocal.
      /////////////////////////////////////////////////////////////////////
      inverseDivideMultiplier = (double)1/len;
			
      while(root < len/2) {

	///////////////////////////////////////////////////////////////////
	// load cos and sin values for current root value.
	///////////////////////////////////////////////////////////////////

	c = sinCosTable[2*root];
	s = isign*sinCosTable[2*root+1];

	for(j = trig; j > 0; j--) {
	  /////////////////////////////////////////////////////////////////
	  // calculate butterflies.  For inputs:
	  // [re0, im0] [re1, im1]
	  //
	  // we calculate:
	  //
	  // out0 = [re0+re1, im0+im1]
	  //
	  // out1 = [ cos * (re0-re1) - sin * (im0-im1),
	  //			sin * (re0-re1) + cos * (im0-im1) ]
	  //
	  //
	  // We also divide each element by the length of the signal
	  // (this is done by multiplying by the reciprocal).
	  /////////////////////////////////////////////////////////////////
					
	  tre = srcDataPtr[0] - srcDataPtr[len];
	  tim = srcDataPtr[1] - srcDataPtr[len + 1];
	  dstDataPtr[0] = (srcDataPtr[0] + srcDataPtr[len])*inverseDivideMultiplier;
	  dstDataPtr[1]=(srcDataPtr[1] + srcDataPtr[len+1])*inverseDivideMultiplier;
	  dstDataPtr[2*trig] = (c*tre - s*tim)*inverseDivideMultiplier;
	  dstDataPtr[2*trig + 1] = (s*tre + c*tim)*inverseDivideMultiplier;
	  srcDataPtr += 2; dstDataPtr += 2;
	}

	///////////////////////////////////////////////////////////////////
	// update dest pointer and root value
	///////////////////////////////////////////////////////////////////
				
	dstDataPtr += 2*trig;
	root += trig;

      }
    } else {
      while(root < len/2) {
	///////////////////////////////////////////////////////////////////
	// load cos and sin values for current root value.
	///////////////////////////////////////////////////////////////////

	c = sinCosTable[2*root];
	s = isign*sinCosTable[2*root+1];

	for(j = trig; j > 0; j--) {
	  /////////////////////////////////////////////////////////////////
	  // calculate butterflies.  For inputs:
	  // [re0, im0] [re1, im1]
	  //
	  // we calculate:
	  //
	  // out0 = [re0+re1, im0+im1]
	  //
	  // out1 = [ cos * (re0-re1) - sin * (im0-im1),
	  //			sin * (re0-re1) + cos * (im0-im1) ]
	  //
	  /////////////////////////////////////////////////////////////////
					
	  tre = srcDataPtr[0] - srcDataPtr[len];
	  tim = srcDataPtr[1] - srcDataPtr[len + 1];
	  dstDataPtr[0] = srcDataPtr[0] + srcDataPtr[len];
	  dstDataPtr[1] = srcDataPtr[1] + srcDataPtr[len+1];
	  dstDataPtr[2*trig] = c*tre - s*tim;
	  dstDataPtr[2*trig + 1] = s*tre + c*tim;
	  srcDataPtr += 2; dstDataPtr += 2;
	}

	///////////////////////////////////////////////////////////////////
	// update dest pointer and root value
	///////////////////////////////////////////////////////////////////
				
	dstDataPtr += 2*trig;
	root += trig;
      }
    }
  }

  return result;    
}

/////////////////////////////////////////////////////////////////////////////
//
//	fft_pingpong
//
// This function calls either the scalar or vector implementation of
// the pingpong fft, based on the length of the data.  The vector
// implementation only works above certain lengths because of
// implementation details.
/////////////////////////////////////////////////////////////////////////////
static int32 fft_pingpong(float *data, int32 len, int32 isign)
{
  int32 result = 0;

  ///////////////////////////////////////////////////////////////////////////
  // make sure that our temp buffer is sufficiently large to hold a copy of
  // the data.
  ///////////////////////////////////////////////////////////////////////////
  result = EnsureStaticBufferSize(len);
	
  if (result == 0) {
    if (len < ALTIVEC_COMPLEX_MIN_LENGTH) {
      ///////////////////////////////////////////////////////////////////////
      // call scalar version
      ///////////////////////////////////////////////////////////////////////
      result = fft_scalar(data, (float*)gTempBufferPtr, len, isign);
    } else {
      ///////////////////////////////////////////////////////////////////////
      // call AltiVec version
      ///////////////////////////////////////////////////////////////////////
      result = fft_altivec(data, (float*)gTempBufferPtr, len, isign);
    }
  }	

  return result;
}


/////////////////////////////////////////////////////////////////////////////
//	FFTComplex
//
//	Performs a forward or inverse complex FFT on the data.  This
// is a wrapper function for three different FFT routines.  If the
// length is below the breakover to call the recursive FFT, then it
// calls the pingpong FFT.  If it's above the point at which the
// recursive FFT is faster, then it calls one of the recursive FFTs.
// If this is the case, then it chooses between two forms of
// recursion.  For even powers of two, it calls the matrix FFT (which
// can only be performed on even powers of two because it allows for a
// square matrix, which can be easily transposed).  If the length is
// an odd power of two, then a one-step recursive FFT is called.
/////////////////////////////////////////////////////////////////////////////
int32 FFTComplex(float *data, int32 length, int32 isign)
{
  int32 result = 0;
	
  if (length <= PINGPONG_FFT_MAXLEN) {
    /////////////////////////////////////////////////////////////////////////
    // length is in the range where pingpong FFT is fastest
    /////////////////////////////////////////////////////////////////////////
    result = fft_pingpong(data, length, isign);	
  } else {
    int32 pow = log2max(length);
    
    if (!(pow & 1)) {
      ///////////////////////////////////////////////////////////////////////
      // data can be represented in a square matrix, so call matrix FFT
      ///////////////////////////////////////////////////////////////////////
      result = fft_square_matrix(data, length, isign);
    } else {
      ///////////////////////////////////////////////////////////////////////
      // data length is odd power of two, so call recursive FFT
      ///////////////////////////////////////////////////////////////////////
      result = fft_recursive(data, length, isign);
    }
  }
  
  return result;
}


#pragma mark -
#pragma mark R E A L   F F T



/////////////////////////////////////////////////////////////////////////////
//	fft_real_forward_altivec
//
//	Given a real signal X = x_0 ... x_(n-1), one can calculate a
//	real-signal fft as follows:
//	
//	First, the real signal is treated as complex data (of length
//	n/2), and a complex FFT is performed on X to yield complex
//	data U, with U = u_0...u_(n/2-1).
//	
//	Next, the signal U is used to define e_k and o_k (k in [0, n/2],
//	with e_n/2 = e_0), where
//	
//	e_k = (u_k + u_(n/2 - k)*) / 2
//
//	o_k = (u_k + u_(n/2 - k)*) / 2i
//	
//	Given e_k and o_k, we define Y as
//	
//	Y_k = e_k + ( e^(-2*pi*i*k/N) * o_k ) 
//	
//	for k in [0, n/2].  Then, the signal Y is the real-signal FFT.
//
//	Result is stored in hermitian order, so that it may occupy the
//      exact same space as the original data.  Given the
//      complex-signal FFT result Y, this result is stored in the
//      order:
//
//	Y(0)r Y(N/2)r Y(1)r Y(1)i Y(2)r Y(2)i ... Y(N/2-1)r Y(N/2-1)i
/////////////////////////////////////////////////////////////////////////////
static int32 fft_real_forward_altivec(float *data, int32 length)
{
  int32              result = 0;
  vector float         *pInVecLo, *pInVecHi;
  vector float         *pOutVecLo, *pOutVecHi;
  vector float         vInLoNext, vInHiPrev;
	
  vector float         vUHi1, vUHi2, vULo1, vULo2;
  vector float         vZero;
  vector float         vDiffLo, vDiffHi, vSumLo, vSumHi;
	
  vector unsigned int  vHiLoSelect, vAlternateSelect;

  vector unsigned char vNewSinLoPerm, vNewCosLoPerm;
	
  vector unsigned char vAdderPerm, vFirstMulPerm, vSecondMulPerm;

  vector float         vFirstMul, vSecondMul;

  vector float         vSinLo, vCosLo;

  vector float         vResultLo, vResultHi;

  vector float         vSinHi, vCosHi;

  vector float         vOneHalf;

  vector float         vAlternateNegate, vAlternateNegate2;
	
  int32              i;

  vector float         vFTransition;

  double               updateA, updateB;

  double               newCos1, newCos2, newSin1, newSin2;

  double               tempCos1, tempCos2;

  float                real0, im0;
	
  ///////////////////////////////////////////////////////////////////////////
  // perform a forward complex fft on our real signal data, treating
  // it as a half-length complex signal.
  ///////////////////////////////////////////////////////////////////////////
	
  result = FFTComplex(data, length/2, -1);

  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////
    // initialize zero vector
    /////////////////////////////////////////////////////////////////////////
    vZero = Altivec_Const4(vector float, 0, 0, 0, 0);

    /////////////////////////////////////////////////////////////////////////
    // create a select vector that will select the first two elements
    // of vector one, and second two elements of vector two
    /////////////////////////////////////////////////////////////////////////
    vHiLoSelect = (vector unsigned int)(Altivec_Const4(vector signed int, -1,
						       -1, 0, 0));

    /////////////////////////////////////////////////////////////////////////
    // create a select vector that will select alternate elements from
    // first and second input vectors
    /////////////////////////////////////////////////////////////////////////
    vAlternateSelect = (vector unsigned int)(Altivec_Const4(vector signed int,
							    0, -1, 0, -1));

    /////////////////////////////////////////////////////////////////////////
    // create a permute vector that, given input vectors 
    //
    // X = x0 x1 x2 x3
    // Y = y0 y1 y2 y3
    //
    // will create the output vector
    // Z = x0 x0 y3 y3
    /////////////////////////////////////////////////////////////////////////
    vNewSinLoPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 0, 1,
				    2, 3, 28, 29, 30, 31, 28, 29, 30, 31);

    /////////////////////////////////////////////////////////////////////////
    // create a permute vector that, given input vectors 
    //
    // X = x0 x1 x2 x3
    // Y = y0 y1 y2 y3
    //
    // will create the output vector
    // Z = x0 x0 y1 y1
    /////////////////////////////////////////////////////////////////////////
    vNewCosLoPerm = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 4, 5,
				    6, 7, 20, 21, 22, 23, 20, 21, 22, 23);
		
    /////////////////////////////////////////////////////////////////////////
    // create a permute vector that, given input vectors 
    //
    // X = x0 x1 x2 x3
    // Y = y0 y1 y2 y3
    //
    // will create the output vector
    // Z = x0 y1 x2 y3
    /////////////////////////////////////////////////////////////////////////
    vAdderPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 20, 21,
				 22, 23, 8, 9, 10, 11, 28, 29, 30, 31);
		
    /////////////////////////////////////////////////////////////////////////
    // create a permute vector that, given input vectors 
    //
    // X = x0 x1 x2 x3
    // Y = y0 y1 y2 y3
    //
    // will create the output vector
    // Z = x1 y0 x3 y2
    /////////////////////////////////////////////////////////////////////////
    vFirstMulPerm = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 16, 17,
				    18, 19, 12, 13, 14, 15, 24, 25, 26, 27);

    /////////////////////////////////////////////////////////////////////////
    // create a permute vector that, given input vectors 
    //
    // X = x0 x1 x2 x3
    // Y = y0 y1 y2 y3
    //
    // will create the output vector
    // Z = y0 x1 y2 x3
    /////////////////////////////////////////////////////////////////////////
    vSecondMulPerm = Altivec_Const16(vector unsigned char, 16, 17, 18, 19, 4,
				     5, 6, 7, 24, 25, 26, 27, 12, 13, 14, 15);

    /////////////////////////////////////////////////////////////////////////
    // create float vector of 0.5
    /////////////////////////////////////////////////////////////////////////
    vOneHalf = Altivec_Const4(vector float,0.5,0.5,0.5,0.5);

    /////////////////////////////////////////////////////////////////////////
    // create a multiply vector that will negate second and fourth elements
    /////////////////////////////////////////////////////////////////////////
    vAlternateNegate = Altivec_Const4(vector float, 1, -1, 1, -1);

    /////////////////////////////////////////////////////////////////////////
    // create a multiply vector that will negate first and third elements
    /////////////////////////////////////////////////////////////////////////
    vAlternateNegate2 = Altivec_Const4(vector float, -1, 1, -1, 1);

    /////////////////////////////////////////////////////////////////////////
    //
    //	Given c = cos(w) and s = sin(w), if we
    //	want to find the cos and sin of w plus
    //	some small angle d, then we can do so
    //	by defining:
    //	
    //	a = 2 * ((sin(d/2)) ^ 2)
    //	b = sin(d)
    //	
    //	Then, we can calculate the cos and sin
    //	of the updated angle w+d as:
    //	
    //	cos(w+d) = c - ac - bs
    //	sin(w+d) = s - as + bc
    //	
    //	We will need to incrementally calculate
    //	cos(w) and sin(w), so we define the
    //	following scalar values.  We use scalar
    //	because we need the precision of doubles,
    //	and vectors are floats.	
    //
    /////////////////////////////////////////////////////////////////////////

    updateA = sin(2*PI/length);
    updateA *= updateA*2;

    updateB = sin(4*PI/length);
		
    /////////////////////////////////////////////////////////////////////////
    //
    // We want to have four cos and sin vectors that are updated
    // incrementally, using the doubles that we have used to calculate
    // our new values.  To do this we have a "transition" vector that
    // allows us to get our scalar values into the vector domain.
    //
    // Scalar values are stored into the elements of the transition
    // vector, and then our end cos and sin vectors are created by
    // permuting values out of our transition vector and other
    // previously created sin and cos vectors.
    // 
    // We also take advantage of the fact that
    // sin(pi - d) = sin(0 + d) 
    // and
    // cos(pi - d) = -cos(0 + d) 
    // 
    /////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    // We store values into vFTransiton so that we can create:
    // 
    // vCosLo = cos(0) cos(0) cos(2pi/length) cos(2pi/length)	
    //
    /////////////////////////////////////////////////////////////////////////
		
    ((float*)&vFTransition)[0] = 1;
    ((float*)&vFTransition)[1] = 1;

    newCos1 = cos(2*PI/length);
    ((float*)&vFTransition)[2] = newCos1;
    ((float*)&vFTransition)[3] = newCos1;

    vCosLo = vFTransition;

    /////////////////////////////////////////////////////////////////////////
    // We store values into vFTransiton so that we can create:
    // 
    // vCosHi = cos(2*2pi/length) cos(2*2pi/length)
    //           cos(2pi/length) cos(2pi/length)	
    //
    /////////////////////////////////////////////////////////////////////////

    newCos2 = cos(4*PI/length);
    ((float*)&vFTransition)[0] = newCos2;
    ((float*)&vFTransition)[1] = newCos2;
		
    vCosHi = vFTransition;
		
    /////////////////////////////////////////////////////////////////////////
    // We store values into vFTransiton so that we can create:
    // 
    // vSinLo = sin(0) sin(0) sin(2pi/length) sin(2pi/length)	
    //
    /////////////////////////////////////////////////////////////////////////
    ((float*)&vFTransition)[0] = 0;
    ((float*)&vFTransition)[1] = 0;
		
    newSin1 = sin(2*PI/length);
    ((float*)&vFTransition)[2] = newSin1;
    ((float*)&vFTransition)[3] = newSin1;

    vSinLo = vFTransition;
				
    /////////////////////////////////////////////////////////////////////////
    // We store values into vFTransiton so that we can create:
    // 
    // vSinHi = sin(2*2pi/length) sin(2*2pi/length)
    //          sin(2pi/length) sin(2pi/length)	
    //
    /////////////////////////////////////////////////////////////////////////
    newSin2 = sin(4*PI/length);
    ((float*)&vFTransition)[0] = newSin2;
    ((float*)&vFTransition)[1] = newSin2;

    vSinHi = vFTransition;

    /////////////////////////////////////////////////////////////////////////
    // save real[0] and im[0] for later use
    /////////////////////////////////////////////////////////////////////////
						
    real0 = data[0];
    im0 = data[1];

    /////////////////////////////////////////////////////////////////////////
    // set up hi,lo pointers for input from data to point at first and
    // last vectors, and do the same for output vectors, since we
    // overwrite our results in place.
    /////////////////////////////////////////////////////////////////////////
    pInVecLo = (vector float*)data;
    pInVecHi = pInVecLo + (length/4) - 1;

    pOutVecLo = pInVecLo;
    pOutVecHi = pInVecHi;

    /////////////////////////////////////////////////////////////////////////
    // get 0 element for wraparound, since U_{n/2} = U_0.
    /////////////////////////////////////////////////////////////////////////
    vInLoNext = *pInVecLo++;
    vInHiPrev = vInLoNext;
		
    /////////////////////////////////////////////////////////////////////////
    // Loop through all elements.  Since each vector is four elements,
    // and we are calculating two vectors per iteration through the
    // loop, we calculate 8 elements per loop, and so we iterate
    // through the loop (length/8) times.
    /////////////////////////////////////////////////////////////////////////

    for (i = 0; i < length/8; i++) {
      ///////////////////////////////////////////////////////////////////////
      // negate alternating elements of vCosLo and vCosHi vectors.
      ///////////////////////////////////////////////////////////////////////
      vCosLo = vec_madd(vCosLo, vAlternateNegate, vZero);
      vCosHi = vec_madd(vCosHi, vAlternateNegate2, vZero);
		 
      ///////////////////////////////////////////////////////////////////////
      // calculate new sin, cos for next time through loop, using our
      // incremental updating algorithm
      ///////////////////////////////////////////////////////////////////////
      tempCos1 	= newCos1 - (updateA*newCos1  + updateB * newSin1);
      newSin1 	= newSin1 - (updateA*newSin1  - updateB * newCos1);
      newCos1 	= tempCos1;

      tempCos2 	= newCos2 - (updateA*newCos2  + updateB * newSin2);
      newSin2 	= newSin2 - (updateA*newSin2  - updateB * newCos2);
      newCos2 	= tempCos2;
			
      ///////////////////////////////////////////////////////////////////////
      // Using vectors we load in, and vectors from previous time
      // through loop, we generate the vectors:
      //
      // vULo1 = re[k] 		im[k] 		re[k+1]		im[k+1]
      // vULo2 = re[n/2-k]	im[n/2-k]	re[n/2-k-1]	im[n/2-k-1]
      //
      // vUHi1 = re[n/2-k-1]	im[n/2-k-1]	re[n/2-k-2]	im[n/2-k-2]
      // vUHi2 = re[k+1] 	im[k+1] 	re[k+2]		im[k+2]
      //
      // with k = 2*i (where i is the loop iterator)
      ///////////////////////////////////////////////////////////////////////
					
      vULo1 = vInLoNext;
      vUHi1 = *pInVecHi--;
					
      vULo2 = vec_sel(vUHi1, vInHiPrev, vHiLoSelect);
			
      vInHiPrev = vUHi1;
			
      vInLoNext = *pInVecLo++;
			
      vUHi2 = vec_sel(vULo1, vInLoNext, vHiLoSelect);

      ///////////////////////////////////////////////////////////////////////
      //	Calculate sum and difference of vULo1 and vULo2, which are:
      //
      // vSumLo = re[k]+re[n/2-k], im[k]+im[n/2-k],
      //          re[k+1]+re[n/2-k-1], im[k+1]+im[n/2-k-1]
      // vDiffLo = re[k]-re[n/2-k], im[k]-im[n/2-k],
      //           re[k+1]-re[n/2-k-1], im[k+1]-im[n/2-k-1]
      ///////////////////////////////////////////////////////////////////////

      vSumLo = vec_add(vULo1, vULo2);		
      vDiffLo = vec_sub(vULo1, vULo2);		

      ///////////////////////////////////////////////////////////////////////
      //	We start by generating a result vector that is:
      //
      // vResultLo = re[k]+re[n/2-k]   im[k]-im[n/2-k]
      //             re[k+1]+re[n/2-k-1]   im[k+1]-im[n/2-k-1]
      ///////////////////////////////////////////////////////////////////////
			
      vResultLo = vec_perm(vSumLo, vDiffLo, vAdderPerm);

      ///////////////////////////////////////////////////////////////////////
      // Next we create a multiplier vector that is:
      //
      // vFirstMul = im[k]+im[n/2-k]   re[k]-re[n/2-k]
      //   	     im[k+1]+im[n/2-k-1]   re[k+1]-re[n/2-k-1]
      ///////////////////////////////////////////////////////////////////////

      vFirstMul = vec_perm(vSumLo, vDiffLo, vFirstMulPerm);			

      ///////////////////////////////////////////////////////////////////////
      // Next we create a multiplier vector that is:
      //
      // vSecondMul = re[k]-re[n/2-k]   im[k]+im[n/2-k]
      //              re[k+1]-re[n/2-k-1]   im[k+1]+im[n/2-k-1]
      ///////////////////////////////////////////////////////////////////////

      vSecondMul = vec_perm(vSumLo, vDiffLo, vSecondMulPerm);		
		
      ///////////////////////////////////////////////////////////////////////
      // We now calculate:
      //
      // vResultLo = 
      //
      //   re[k]+re[n/2-k] + (im[k]+im[n/2-k]) * cos(k*2pi/length),
      //   im[k]-im[n/2-k] + (re[k]-re[n/2-k]) * -cos(k*2pi/length),
      //   re[k+1]+re[n/2-k-1] + (im[k+1]+im[n/2-k-1]) * cos((k+1)*2pi/length),
      //   im[k+1]-im[n/2-k-1] + (re[k+1]-re[n/2-k-1]) * -cos((k+1)*2pi/length)
      //		
      ///////////////////////////////////////////////////////////////////////

      vResultLo = vec_madd(vCosLo, vFirstMul, vResultLo);

      ///////////////////////////////////////////////////////////////////////
      // with a negative multiply-subtract, we calculate
      //
      // vResultLo = 
      //
      //  re[k]+re[n/2-k] + (im[k]+im[n/2-k]) * cos(k*2pi/length) +
      //   (re[k]-re[n/2-k]) * -sin(k*2pi/length),
      //
      //  im[k]-im[n/2-k] + (re[k]-re[n/2-k]) * -cos(k*2pi/length) +
      //   (im[k]+im[n/2-k]) * -sin(k*2pi/length),
      //
      //  re[k+1]+re[n/2-k-1] + (im[k+1]+im[n/2-k-1]) * cos((k+1)*2pi/length) +
      //   (re[k+1]-re[n/2-k-1]) * -sin((k+1)*2pi/length),
      //
      //  im[k+1]-im[n/2-k-1] + (re[k+1]-re[n/2-k-1]) * -cos((k+1)*2pi/length) +
      //   (im[k+1]+im[n/2-k-1]) * -sin((k+1)*2pi/length)
      //		
      ///////////////////////////////////////////////////////////////////////

      vResultLo = vec_nmsub(vSinLo, vSecondMul, vResultLo);
			
      ///////////////////////////////////////////////////////////////////////
      // finally, divide all elements by two (multiply by one half).
      // With this step finished, we have calculated:
      //  e_k + (e^(-2pi*i*k/N) * o_k), e_(k+1) + (e^(-2pi*i*(k+1)/N) *
      //  o_(k+1))
      ///////////////////////////////////////////////////////////////////////

      vResultLo = vec_madd(vResultLo, vOneHalf, vZero);
			
      ///////////////////////////////////////////////////////////////////////
      // store this vector back, overwriting source data. 
      ///////////////////////////////////////////////////////////////////////

      *pOutVecLo++ = vResultLo;
			
      ///////////////////////////////////////////////////////////////////////
      // store our new angles to the transition vector, and generate
      // new cos,sin vectors from them.
      ///////////////////////////////////////////////////////////////////////
			
      ((float*)&vFTransition)[0] = newCos2;
      ((float*)&vFTransition)[1] = newCos1;
      ((float*)&vFTransition)[2] = newSin2;
      ((float*)&vFTransition)[3] = newSin1;

      vCosLo = vec_perm(vCosHi, vFTransition, vNewCosLoPerm);
					
      vSinLo = vec_perm(vSinHi, vFTransition, vNewSinLoPerm);
					
      ///////////////////////////////////////////////////////////////////////
      // Calculate 
      // e_k + (e^(-2pi*i*k/N) * o_k), e_(k+1) + (e^(-2pi*i*(k+1)/N) * o_(k+1))
      // for high elements in array
      ///////////////////////////////////////////////////////////////////////

      vSumHi = vec_add(vUHi1, vUHi2);		
      vDiffHi = vec_sub(vUHi1, vUHi2);		
							
      vFirstMul = vec_perm(vSumHi, vDiffHi, vFirstMulPerm);
      vSecondMul = vec_perm(vSumHi, vDiffHi, vSecondMulPerm);
			
      vResultHi = vec_perm(vSumHi, vDiffHi, vAdderPerm);

      vResultHi = vec_madd(vCosHi, vFirstMul, vResultHi);
      vResultHi = vec_nmsub(vSinHi, vSecondMul, vResultHi);
			
      vResultHi = vec_madd(vResultHi, vOneHalf, vZero);

      ///////////////////////////////////////////////////////////////////////
      // create new angle vectors from our transition vectors for next time
      // through loop
      ///////////////////////////////////////////////////////////////////////
      vCosHi = vec_mergeh(vFTransition, vFTransition);		
      vSinHi = vec_mergel(vFTransition, vFTransition);
			
      ///////////////////////////////////////////////////////////////////////
      // store high result, overwriting source
      ///////////////////////////////////////////////////////////////////////
      *pOutVecHi-- = vResultHi;
							
    }

    ///////////////////////////////////////////////////////////////////////
    // finally, store re[n/2]
    ///////////////////////////////////////////////////////////////////////
    data[1] = real0-im0;
  }
	
  return result;
}

///////////////////////////////////////////////////////////////////////////
//	fft_real_inverse_altivec
//
//	Given Y, a complex-result FFT of a real signal, the inverse
// real FFT can be taken with the following steps:
//
//	First, define the signals e_k and o_k:
//
//	e_k = 1/2 * (Y_k + (Y_N/2-k)*)
//
//	o_k = 1/2 * (Y_k - (Y_N/2-k)*) * e_^(2pi i k / N)
//
//	where N is the (real) signal length, and Y* is Y conjugate,
//      for k=(0, N/2)
//
//	Next, from this, generate a signal X:
//
//	X_k = e_k + i*o_k 		(k = (0, N/2)
//
// Then, perform the inverse complex FFT in this length-N/2 signal.
// The result is the real, in-order inverse FFT.
//
///////////////////////////////////////////////////////////////////////////
static int32 fft_real_inverse_altivec(float *data, int32 length)
{
  int32               result = 0;

  vector float          *pInVecLo, *pInVecHi;
  vector float          *pOutVecLo, *pOutVecHi;
	
  vector float          vInLoNext, vInHiPrev;
	
  vector float          vUHi1, vUHi2, vULo1, vULo2;
  vector float          vZero;
  vector float          vDiffLo, vDiffHi;
  vector float          vSumLo, vSumHi;
	
  vector unsigned int   vHiLoSelect;

  vector unsigned char  vNewSinLoPerm;
  vector unsigned char  vNewCosLoPerm;

  vector unsigned char  vAdderPerm;
  vector unsigned char  vFirstMulPerm;
  vector unsigned char  vSecondMulPerm;

  vector float          vFirstMul, vSecondMul;

  vector float          vSinLo, vCosLo;

  vector float          vResultLo, vResultHi;

  vector float          vSinHi, vCosHi;

  vector float          vOneHalf;

  vector float          vAlternateNegate, vAlternateNegate2;

  vector unsigned int   vClearSecondElementSelect;
  vector unsigned char  vTwoToOnePermute;

  int32               i;

  vector float          vFTransition;

  double                updateA, updateB;

  double                newCos1, newCos2, newSin1, newSin2;

  double                tempCos1, tempCos2;

  /////////////////////////////////////////////////////////////////////////
  // initialize zero vector
  /////////////////////////////////////////////////////////////////////////
  vZero = Altivec_Const4(vector float, 0, 0, 0, 0);

  /////////////////////////////////////////////////////////////////////////
  // create a select vector that will select the first two elements of
  // vector one, and second two elements of vector two
  /////////////////////////////////////////////////////////////////////////
  vHiLoSelect = (vector unsigned int)(Altivec_Const4(vector signed int,
						     -1, -1, 0, 0));

  /////////////////////////////////////////////////////////////////////////
  // create a permute vector that, given input vectors 
  //
  // X = x0 x1 x2 x3
  // Y = y0 y1 y2 y3
  //
  // will create the output vector
  // Z = x0 x0 y3 y3
  /////////////////////////////////////////////////////////////////////////
  vNewSinLoPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 0, 1,
				  2, 3, 28, 29, 30, 31, 28, 29, 30, 31);

  /////////////////////////////////////////////////////////////////////////
  // create a permute vector that, given input vectors 
  //
  // X = x0 x1 x2 x3
  // Y = y0 y1 y2 y3
  //
  // will create the output vector
  // Z = x0 x0 y1 y1
  /////////////////////////////////////////////////////////////////////////
  vNewCosLoPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 0, 1,
				  2, 3, 20, 21, 22, 23, 20, 21, 22, 23);

  /////////////////////////////////////////////////////////////////////////
  // create a permute vector that, given input vectors 
  //
  // X = x0 x1 x2 x3
  // Y = y0 y1 y2 y3
  //
  // will create the output vector
  // Z = x0 y1 x2 y3
  /////////////////////////////////////////////////////////////////////////
  vAdderPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 20, 21,
			       22, 23, 8, 9, 10, 11, 28, 29, 30, 31);

  /////////////////////////////////////////////////////////////////////////
  // create a permute vector that, given input vectors 
  //
  // X = x0 x1 x2 x3
  // Y = y0 y1 y2 y3
  //
  // will create the output vector
  // Z = x1 y0 x3 y2
  /////////////////////////////////////////////////////////////////////////
  vFirstMulPerm = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 16, 17,
				  18, 19, 12, 13, 14, 15, 24, 25, 26, 27);

  /////////////////////////////////////////////////////////////////////////
  // create a permute vector that, given input vectors 
  //
  // X = x0 x1 x2 x3
  // Y = y0 y1 y2 y3
  //
  // will create the output vector
  // Z = y0 x1 y2 x3
  /////////////////////////////////////////////////////////////////////////
  vSecondMulPerm = Altivec_Const16(vector unsigned char, 16, 17, 18, 19, 4,
				   5, 6, 7, 24, 25, 26, 27, 12, 13, 14, 15);

  /////////////////////////////////////////////////////////////////////////
  // create float vector of 0.5
  /////////////////////////////////////////////////////////////////////////
  vOneHalf = Altivec_Const4(vector float, .5, .5, .5, .5);

  /////////////////////////////////////////////////////////////////////////
  // create a multiply vector that will negate second and fourth elements
  /////////////////////////////////////////////////////////////////////////
  vAlternateNegate = Altivec_Const4(vector float, 1, -1, 1, -1);

  /////////////////////////////////////////////////////////////////////////
  // create a multiply vector that will negate first and third elements
  /////////////////////////////////////////////////////////////////////////
  vAlternateNegate2 = Altivec_Const4(vector float, -1, 1, -1, 1);

  /////////////////////////////////////////////////////////////////////////
  // create a select vector that, given the input vectors
  // 
  // Y = 0  0  0  0
  // X = x0 x1 x2 x3
  //
  // will create the output vector
  // x0 0 x2 x3
  /////////////////////////////////////////////////////////////////////////
  vClearSecondElementSelect = Altivec_Const4(vector unsigned int, 0xffffffff,
					     0x00000000, 0xffffffff, 0xffffffff);
	
  /////////////////////////////////////////////////////////////////////////
  // create a permute vector that, given the input vectors
  // 
  // X = x0 x1 x2 x3
  // Y = 0  0  0  0
  //
  // will create the output vector
  // x1 0 x2 x3
  /////////////////////////////////////////////////////////////////////////
  vTwoToOnePermute = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 16,
				     16, 16, 16, 8, 9, 10, 11, 12, 13, 14, 15);

  /////////////////////////////////////////////////////////////////////////
  //
  //	Given c = cos(w) and s = sin(w), if we
  //	want to find the cos and sin of w plus
  //	some small angle d, then we can do so
  //	by defining:
  //	
  //	a = 2 * ((sin(d/2)) ^ 2)
  //	b = sin(d)
  //	
  //	Then, we can calculate the cos and sin
  //	of the updated angle w+d as:
  //	
  //	cos(w+d) = c - ac - bs
  //	sin(w+d) = s - as + bc
  //	
  //	We will need to incrementally calculate
  //	cos(w) and sin(w), so we define the
  //	following scalar values.  We use scalar
  //	because we need the precision of doubles,
  //	and vectors are floats.	
  //
  /////////////////////////////////////////////////////////////////////////

  updateA = sin(2*PI/length);
  updateA *= updateA*2;

  updateB = sin(4*PI/length);
	
  /////////////////////////////////////////////////////////////////////////
  //
  // We want to have four cos and sin vectors that are updated
  // incrementally, using the doubles that we have used to calculate
  // our new values.  To do this we have a "transition" vector that
  // allows us to get our scalar values into the vector domain.
  //
  // Scalar values are stored into the elements of the transition
  // vector, and then our end cos and sin vectors are created by
  // permuting values out of our transition vector and other
  // previously created sin and cos vectors.
  // 
  // We also take advantage of the fact that
  // sin(pi - d) = sin(0 + d) 
  // and
  // cos(pi - d) = -cos(0 + d) 
  // 
  /////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////
  // We store values into vFTransiton, creating:
  // 
  // vCosLo = cos(0) cos(0) cos(2pi/length) cos(2pi/length)	
  //
  /////////////////////////////////////////////////////////////////////////
	
  ((float*)&vFTransition)[0] = 1;
  ((float*)&vFTransition)[1] = 1;

  newCos1 = cos(2*PI/length);
  ((float*)&vFTransition)[2] = newCos1;
  ((float*)&vFTransition)[3] = newCos1;

  vCosLo = vFTransition;

  /////////////////////////////////////////////////////////////////////////
  // We store values into vFTransiton, creating:
  // 
  // vCosHi = -cos(pi-(2*2pi/length)) -cos(pi-(2*2pi/length))
  //          -cos(pi-(2pi/length)) -cos(pi-(2pi/length))	
  //
  // We only need to store elements 0 and 1, since elements 2 and 3
  // already contain cos(2pi/length), and cos(pi-(2pi/length)) =
  // -cos(2pi/length)
  /////////////////////////////////////////////////////////////////////////

  newCos2 = cos(4*PI/length);
  ((float*)&vFTransition)[0] = newCos2;
  ((float*)&vFTransition)[1] = newCos2;
	
  vCosHi = vFTransition;
	
  /////////////////////////////////////////////////////////////////////////
  // We store values into vFTransiton, creating:
  // 
  // vSinLo = sin(0) sin(0) sin(2pi/length) sin(2pi/length)	
  //
  /////////////////////////////////////////////////////////////////////////

  ((float*)&vFTransition)[0] = 0;
  ((float*)&vFTransition)[1] = 0;
	
  newSin1 = sin(2*PI/length);
  ((float*)&vFTransition)[2] = newSin1;
  ((float*)&vFTransition)[3] = newSin1;

  vSinLo = vFTransition;
			
  /////////////////////////////////////////////////////////////////////////
  // We store values into vFTransiton, creating:
  // 
  // vSinHi = sin(2*2pi/length) sin(2*2pi/length)
  //          sin(2pi/length) sin(2pi/length)	
  //
  // We only need to store elements 0 and 1, since elements 2 and 3
  // already contain sin(2pi/length), and sin(pi-(2pi/length)) =
  // sin(2pi/length)
  /////////////////////////////////////////////////////////////////////////

  newSin2 = sin(4*PI/length);
  ((float*)&vFTransition)[0] = newSin2;
  ((float*)&vFTransition)[1] = newSin2;

  vSinHi = vFTransition;

  /////////////////////////////////////////////////////////////////////////
  // set up hi,lo pointers for input from data to point at first and last
  // vectors, and do the same for output vectors, since we overwrite our
  // results in place.
  /////////////////////////////////////////////////////////////////////////
  pInVecLo = (vector float*)data;
  pInVecHi = pInVecLo + (length/4) - 1;

  pOutVecLo = pInVecLo;
  pOutVecHi = pInVecHi;

  /////////////////////////////////////////////////////////////////////////
  // get 0 element for wraparound, since U_(n/2) = U_0.
  /////////////////////////////////////////////////////////////////////////

  vInLoNext = *pInVecLo++;
	
  /////////////////////////////////////////////////////////////////////////
  // Source data is assumed to be stored in
  // the order:
  //
  // re[0] re[n/2] re[1] im[1] re[2] im[2] re[3]...re[n/2-1] im[n/2-1]
  // 
  // Since we want the first high vector to be in the form 
  // re[n/2-1] im[n/2-1] re[n/2 im[n/2] and we know that im[n/2] = 0,
  // we want to generate a "previous" high vector in the form
  //
  // re[n/2] im[n/2] x x
  //
  // so that, in the loop, we can generate the desired output vector of
  //
  // re[n/2-1] im[n/2-1] re[n/2 im[n/2] 
  //
  // using our "previous" vector and the permute instruction.
  // So, we create the vector
  //
  // vInHiPrev = re[n/2] 0 x x
  /////////////////////////////////////////////////////////////////////////

  vInHiPrev = vec_perm(vInLoNext, vZero, vTwoToOnePermute);

  /////////////////////////////////////////////////////////////////////////
  // Since we want the first input vector to be in the form
  //  re[0] im[0] re[1] im[1]
  // and we know that im[0] is zero, we simply zero the second element
  // in our first input vector, which is re[0] re[n/2] re[1] im[1] to
  // form vInLoNext = re[0] 0 re[1] im[1]
  /////////////////////////////////////////////////////////////////////////

  vInLoNext = vec_sel(vZero, vInLoNext, vClearSecondElementSelect);
	
  /////////////////////////////////////////////////////////////////////////
  // Loop through all elements.  Since each vector is four elements,
  // and we are calculating two vectors per iteration through the
  // loop, we calculate 8 elements per loop, and so we iterate through
  // the loop (length/8) times.
  /////////////////////////////////////////////////////////////////////////
  for (i = 0; i < length/8; i++) {
    ///////////////////////////////////////////////////////////////////////
    // negate alternating elements of vCosLo and vCosHi vectors.
    ///////////////////////////////////////////////////////////////////////
    vCosLo = vec_madd(vCosLo, vAlternateNegate2, vZero);
    vCosHi = vec_madd(vCosHi, vAlternateNegate, vZero);
	
    ///////////////////////////////////////////////////////////////////////
    // calculate new sin, cos for next time through loop, using our
    // incremental updating algorithm
    ///////////////////////////////////////////////////////////////////////
    tempCos1 	= newCos1 - (updateA*newCos1  + updateB * newSin1);
    newSin1 	= newSin1 - (updateA*newSin1  - updateB * newCos1);
    newCos1 	= tempCos1;

    tempCos2 	= newCos2 - (updateA*newCos2  + updateB * newSin2);
    newSin2 	= newSin2 - (updateA*newSin2  - updateB * newCos2);
    newCos2 	= tempCos2;

    ///////////////////////////////////////////////////////////////////////
    // Using vectors we load in, and vectors from previous time
    // through loop, we generate the vectors:
    //
    // vULo1 = 	re[k] 		im[k] 		re[k+1]		im[k+1]
    // vULo2 =	re[n/2-k]	im[n/2-k]	re[n/2-k-1]	im[n/2-k-1]
    //
    // vUHi1 = 	re[n/2-k-1]	im[n/2-k-1]	re[n/2-k-2]	im[n/2-k-2]
    // vUHi2 =	re[k+1] 	im[k+1] 	re[k+2]		im[k+2]
    //
    // with k = 2*i (where i is the loop iterator)
    ///////////////////////////////////////////////////////////////////////

    vULo1 = vInLoNext;
    vUHi1 = *pInVecHi--;
				
    vULo2 = vec_sel(vUHi1, vInHiPrev, vHiLoSelect);
		
    vInHiPrev = vUHi1;
		
    vInLoNext = *pInVecLo++;
		
    vUHi2 = vec_sel(vULo1, vInLoNext, vHiLoSelect);

    ///////////////////////////////////////////////////////////////////////
    //	Calculate sum and difference of vULo1 and vULo2, which are:
    //
    // vSumLo = re[k]+re[n/2-k], im[k]+im[n/2-k],
    //          re[k+1]+re[n/2-k-1], im[k+1]+im[n/2-k-1]
    // vDiffLo = re[k]-re[n/2-k], im[k]-im[n/2-k],
    //           re[k+1]-re[n/2-k-1], im[k+1]-im[n/2-k-1]
    ///////////////////////////////////////////////////////////////////////

    vSumLo = vec_add(vULo1, vULo2);		
    vDiffLo = vec_sub(vULo1, vULo2);		

    ///////////////////////////////////////////////////////////////////////
    // Next we create a multiplier vector that is:
    //
    // vFirstMul = im[k]+im[n/2-k]   re[k]-re[n/2-k]
    //             im[k+1]+im[n/2-k-1]   re[k+1]-re[n/2-k-1]
    ///////////////////////////////////////////////////////////////////////

    vFirstMul = vec_perm(vSumLo, vDiffLo, vFirstMulPerm);
		
    ///////////////////////////////////////////////////////////////////////
    // Next we create a multiplier vector that is:
    //
    // vSecondMul = re[k]-re[n/2-k]   im[k]+im[n/2-k]
    //              re[k+1]-re[n/2-k-1]   im[k+1]+im[n/2-k-1]
    ///////////////////////////////////////////////////////////////////////
		
    vSecondMul = vec_perm(vSumLo, vDiffLo, vSecondMulPerm);
		
    ///////////////////////////////////////////////////////////////////////
    //	We start by generating a result vector
    // that is:
    //
    // vResultLo = re[k]+re[n/2-k]   im[k]-im[n/2-k]
    //             re[k+1]+re[n/2-k-1]   im[k+1]-im[n/2-k-1]
    ///////////////////////////////////////////////////////////////////////
		
    vResultLo = vec_perm(vSumLo, vDiffLo, vAdderPerm);

    ///////////////////////////////////////////////////////////////////////
    // We now calculate:
    //
    // vResultLo = 
    //
    //	 re[k]+re[n/2-k] + (im[k]+im[n/2-k]) * -cos(k*2pi/length),
    //	 im[k]-im[n/2-k] + (re[k]-re[n/2-k]) * cos(k*2pi/length),
    //	 re[k+1]+re[n/2-k-1] + (im[k+1]+im[n/2-k-1]) * -cos((k+1)*2pi/length),
    //	 im[k+1]-im[n/2-k-1] + (re[k+1]-re[n/2-k-1]) * cos((k+1)*2pi/length)
    //		
    ///////////////////////////////////////////////////////////////////////

    vResultLo = vec_madd(vCosLo, vFirstMul, vResultLo);

    ///////////////////////////////////////////////////////////////////////
    // with a negative multiply-subtract, we calculate
    //
    // vResultLo = 
    //
    //	 re[k]+re[n/2-k] + (im[k]+im[n/2-k]) * -cos(k*2pi/length) +
    //   (re[k]-re[n/2-k]) * -sin(k*2pi/length),
    //
    //   im[k]-im[n/2-k] + (re[k]-re[n/2-k]) * cos(k*2pi/length) +
    //   (im[k]+im[n/2-k]) * -sin(k*2pi/length),
    //
    //   re[k+1]+re[n/2-k-1] + (im[k+1]+im[n/2-k-1]) * -cos((k+1)*2pi/length) +
    //   (re[k+1]-re[n/2-k-1]) * -sin((k+1)*2pi/length),
    //
    //   im[k+1]-im[n/2-k-1] + (re[k+1]-re[n/2-k-1]) * cos((k+1)*2pi/length) +
    //   (im[k+1]+im[n/2-k-1]) * -sin((k+1)*2pi/length)
    //		
    ///////////////////////////////////////////////////////////////////////

    vResultLo = vec_nmsub(vSinLo, vSecondMul, vResultLo);

    ///////////////////////////////////////////////////////////////////////
    // finally, divide all elements by two multiply by one half).
    // With this step finished, we have calculated:
    //
    // e_k + (e^(2pi*i*k/N) * o_k * i), e_(k+1) +
    // (e^(2pi*i*(k+1)/N) * o_(k+1) * i)
    ///////////////////////////////////////////////////////////////////////

    vResultLo = vec_madd(vResultLo, vOneHalf, vZero);
		
    ///////////////////////////////////////////////////////////////////////
    // store this vector back, overwriting source data. 
    ///////////////////////////////////////////////////////////////////////

    *pOutVecLo++ = vResultLo;

    ///////////////////////////////////////////////////////////////////////
    // store our new angles to the transition vector, and generate new
    // cos,sin vectors, using both newly calculated sin&cos, and those
    // already contained in vCosHi, vSinHi.
    ///////////////////////////////////////////////////////////////////////
		
    ((float*)&vFTransition)[0] = newCos2;
    ((float*)&vFTransition)[1] = newCos1;
    ((float*)&vFTransition)[2] = newSin2;
    ((float*)&vFTransition)[3] = newSin1;

    vCosLo = vec_perm(vCosHi, vFTransition, vNewCosLoPerm);
    vSinLo = vec_perm(vSinHi, vFTransition, vNewSinLoPerm);
				
    ///////////////////////////////////////////////////////////////////////
    // calculate sum and difference of vUHi1 and vUHi2
    ///////////////////////////////////////////////////////////////////////
		
    vSumHi = vec_add(vUHi1, vUHi2);		
    vDiffHi = vec_sub(vUHi1, vUHi2);		
						
    ///////////////////////////////////////////////////////////////////////
    // Calculate:
    //
    // e_k + (e^(2pi*i*k/N) * o_k * i), e_(k+1) +
    // (e^(2pi*i*(k+1)/N) * o_(k+1) * i) 
    //
    // for high elements in array
    ///////////////////////////////////////////////////////////////////////
						
    vFirstMul = vec_perm(vSumHi, vDiffHi, vFirstMulPerm);
    vSecondMul = vec_perm(vSumHi, vDiffHi, vSecondMulPerm);
		
    vResultHi = vec_perm(vSumHi, vDiffHi, vAdderPerm);

    vResultHi = vec_madd(vCosHi, vFirstMul, vResultHi);
    vResultHi = vec_nmsub(vSinHi, vSecondMul, vResultHi);
		
    vResultHi = vec_madd(vResultHi, vOneHalf, vZero);

    ///////////////////////////////////////////////////////////////////////
    // create new angle vectors from our transition vectors for next time
    // through loop
    ///////////////////////////////////////////////////////////////////////

    vCosHi = vec_mergeh(vFTransition, vFTransition);		
    vSinHi = vec_mergel(vFTransition, vFTransition);
		
    ///////////////////////////////////////////////////////////////////////
    // store high result, overwriting source
    ///////////////////////////////////////////////////////////////////////
		
    *pOutVecHi-- = vResultHi;
  }

  /////////////////////////////////////////////////////////////////////////
  // perform an inverse complex fft on data,
  // treating it as half-length complex signal.
  /////////////////////////////////////////////////////////////////////////
  result = FFTComplex(data, length/2, 1);

  return result;
}


/////////////////////////////////////////////////////////////////////////////
// scalar implementation of altivec forward real fft above
/////////////////////////////////////////////////////////////////////////////
static int32 fft_real_forward_scalar(float *data, int32 len)
{
  int32 result = 0;
  double  updateA, updateB;

  double  currentCos, currentSin, tempCos1;
	
  float   realLo, realHi, imLo, imHi;
	
  float   realResultLo, realResultHi, imResultLo, imResultHi;
	
  float   realDiff, imDiff, realSum, imSum;
		
  float   *pInLo, *pInHi;
	
  int32 i;
	
  ///////////////////////////////////////////////////////////////////////////
  // for length of 0 or 1 we do nothing
  ///////////////////////////////////////////////////////////////////////////
  if (len < 2) return 0;
		
  ///////////////////////////////////////////////////////////////////////////
  // perform a forward complex fft on our
  // real signal data, treating it as a
  // half-length complex signal.
  ///////////////////////////////////////////////////////////////////////////
  result = FFTComplex(data, len/2, -1);

  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////
    //
    //	Given c = cos(w) and s = sin(w), if we
    //	want to find the cos and sin of w plus
    //	some small angle d, then we can do so
    //	by defining:
    //	
    //	a = 2 * ((sin(d/2)) ^ 2)
    //	b = sin(d)
    //	
    //	Then, we can calculate the cos and sin
    //	of the updated angle w+d as:
    //	
    //	cos(w+d) = c - ac - bs
    //	sin(w+d) = s - as + bc
    //	
    //	We will need to incrementally calculate
    //	cos(w) and sin(w), so we define the
    //	following scalar values.  We use scalar
    //	because we need the precision of doubles,
    //	and vectors are floats.	
    //
    /////////////////////////////////////////////////////////////////////////
				
    updateA = sin(PI/len);
    updateB = sin(2*PI/len);
    updateA *= updateA*2;

    /////////////////////////////////////////////////////////////////////////
    // start with sin(0) and cos(0)
    /////////////////////////////////////////////////////////////////////////

    currentCos = 1;
    currentSin = 0;	
	
    /////////////////////////////////////////////////////////////////////////
    // first, calculate re[0], and re[n/2]. we don't bother to do the
    // sin, cos multiplies, because we know that they are zero and one.
    /////////////////////////////////////////////////////////////////////////
				
    realResultLo = data[0]+data[1];
    realResultHi = data[0]-data[1];
		
    /////////////////////////////////////////////////////////////////////////
    // store re[0].  We don't calculate im[0] because we know that it is 0.
    /////////////////////////////////////////////////////////////////////////
    data[0] = realResultLo;

    /////////////////////////////////////////////////////////////////////////
    // store re[n/2].  We don't calculate im[n/2] because we know it
    // is 0.  We store re[n/2] in the position of im[0] because we
    // don't want to take up any more space than the source data, and
    // if we were to store it in the n/2 position, it would be past
    // the array of source data, which ranges from 0 to (n/2)-1.
    /////////////////////////////////////////////////////////////////////////
    data[1] = realResultHi;
		
    /////////////////////////////////////////////////////////////////////////
    // Start our source pointers to point at re[1] and re[(n/2)-1].
    /////////////////////////////////////////////////////////////////////////
    pInLo = &data[2];
    pInHi = &data[len-2];

    /////////////////////////////////////////////////////////////////////////
    // for each loop iteration, we calculate two complex results (four
    // floats), so we iterate through the loop len/4 times
    /////////////////////////////////////////////////////////////////////////
    for (i = 0; i < len/4; i++) {
      ///////////////////////////////////////////////////////////////////////
      // calculate the new sin and cos values
      ///////////////////////////////////////////////////////////////////////
		
      tempCos1   = currentCos - (updateA*currentCos  + updateB * currentSin);
      currentSin = currentSin - (updateA*currentSin  - updateB * currentCos);
      currentCos = tempCos1;

      ///////////////////////////////////////////////////////////////////////
      // load re[i+1], im[i+1]
      ///////////////////////////////////////////////////////////////////////
      realLo      = *pInLo;
      imLo        = *(pInLo+1);

      ///////////////////////////////////////////////////////////////////////
      // load re[(n/2)-1-i], im[(n/2)-1-i] 
      ///////////////////////////////////////////////////////////////////////
      realHi      = *pInHi;
      imHi        = *(pInHi+1);
			
      ///////////////////////////////////////////////////////////////////////
      // calculate:
      // realDiff	= re[i+1]-re[(n/2)-1-i]
      // realSum	= re[i+1]+re[(n/2)-1-i]
      ///////////////////////////////////////////////////////////////////////
      realDiff    = realLo-realHi;
      realSum     = realLo+realHi;

      ///////////////////////////////////////////////////////////////////////
      // calculate:
      // imDiff	= im[i+1]-im[(n/2)-1-i]
      // imSum	= im[i+1]+im[(n/2)-1-i]
      ///////////////////////////////////////////////////////////////////////
      imDiff      = imLo-imHi;
      imSum       = imLo+imHi;
			
      ///////////////////////////////////////////////////////////////////////
      // calculate:
      //
      // realResultLo = 	
      //	re[i+1]+re[(n/2)-1-i] + 
      // 	(im[i+1]+im[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
      //	(re[i+1]-re[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
      //
      // imResultLo = 	
      //	im[i+1]-im[(n/2)-1-i] - 
      // 	(re[i+1]-re[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
      //	(im[i+1]+im[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
      //
      ///////////////////////////////////////////////////////////////////////
			
      realResultLo = realSum + (imSum*currentCos) - (realDiff*currentSin);
      imResultLo   = imDiff - (realDiff * currentCos) - (imSum*currentSin);
			
      ///////////////////////////////////////////////////////////////////////
      // calculate:
      //
      // realResultHi = 	
      //	re[(n/2)-1-i]+re[i+1] +
      // 	(im[(n/2)-1-i]+im[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
      //	(re[(n/2)-1-i]-re[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
      //
      // imResultHi = 	
      //	im[(n/2)-1-i]-im[i+1] - 
      // 	(re[(n/2)-1-i]-re[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
      //	(im[(n/2)-1-i]+im[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
      //
      // taking advantage of the following:
      //
      // sin(  (i+1) * 2pi / n ) =  sin ( ((n/2)-i-1) * 2pi / n )
      // cos(  (i+1) * 2pi / n ) = -cos ( ((n/2)-i-1) * 2pi / n )
      // re[(n/2)-1-i]-re[i+1] 	 = -(re[i+1]-re[(n/2)-1-i])
      // re[(n/2)-1-i]-re[i+1] 	 = -(re[i+1]-re[(n/2)-1-i])
      ///////////////////////////////////////////////////////////////////////
			
      realResultHi = realSum - (imSum*currentCos) + (realDiff*currentSin);
      imResultHi = -imDiff - (realDiff * currentCos) - (imSum*currentSin);

      ///////////////////////////////////////////////////////////////////////
      // store results, advance low pointer forward to next complex element, 
      // and high pointer backward to previous complex element
      ///////////////////////////////////////////////////////////////////////
			
      *pInLo++ = realResultLo/2;		
      *pInLo++ = imResultLo/2;		
      *(pInHi+1) = imResultHi/2;		
      *pInHi = realResultHi/2;
					
      pInHi -= 2;
    }
  }
		
  return result;	
}

/////////////////////////////////////////////////////////////////////////////
// scalar implementation of altivec inverse real fft above
/////////////////////////////////////////////////////////////////////////////
static int32 fft_real_inverse_scalar(float *data, int32 len)
{
  int32 result = 0;
  double  updateA, updateB;

  double  currentCos, currentSin, tempCos1;
	
  float   realLo, realHi, imLo, imHi;
	
  float   realResultLo, realResultHi, imResultLo, imResultHi;
	
  float   realDiff, imDiff, realSum, imSum;
		
  float   *pInLo, *pInHi;
	
  int32 i;

  ///////////////////////////////////////////////////////////////////////////
  // for length of 0 or 1 we do nothing
  ///////////////////////////////////////////////////////////////////////////
  if (len < 2) return 0;

  ///////////////////////////////////////////////////////////////////////////
  // Given c = cos(w) and s = sin(w), if we want to find the cos and
  // sin of w plus some small angle d, then we can do so by defining:
  //	
  // a = 2 * ((sin(d/2)) ^ 2)
  // b = sin(d)
  // 
  // Then, we can calculate the cos and sin of the updated angle w+d
  // as:
  // 
  // cos(w+d) = c - ac - bs
  // sin(w+d) = s - as + bc
  // 
  // We will need to incrementally calculate cos(w) and sin(w), so we
  // define the following scalar values.  We use scalar because we
  // need the precision of doubles, and vectors are floats.
  ///////////////////////////////////////////////////////////////////////////
			
  updateA = sin(PI/len);
  updateB = sin(2*PI/len);
  updateA *= updateA*2;

  ///////////////////////////////////////////////////////////////////////////
  // start with sin(0) and cos(0)
  ///////////////////////////////////////////////////////////////////////////

  currentCos = 1;
  currentSin = 0;	

  ///////////////////////////////////////////////////////////////////////////
  // load re[0] and im[0].  Only re[0] is stored -- im[0] is
  // understood to be 0.
  ///////////////////////////////////////////////////////////////////////////
  realLo   = data[0];
  imLo     = 0;

  ///////////////////////////////////////////////////////////////////////////
  // load re[n/2] and im[n/2].  
  // Only re[n/2] is stored -- im[n/2] is understood to be 0.
  ///////////////////////////////////////////////////////////////////////////
  realHi    = data[1];
  imHi      = 0;				

  ///////////////////////////////////////////////////////////////////////////
  // calculat sum and difference of real & imaginary values
  ///////////////////////////////////////////////////////////////////////////

  realDiff  = realLo-realHi;
  realSum   = realLo+realHi;

  imDiff    = imLo-imHi;
  imSum     = imLo+imHi;

  ///////////////////////////////////////////////////////////////////////////
  // calculate re[0], and im[0]. we don't bother to do the sin, cos
  // multiplies, because we know that they are zero and one.
  ///////////////////////////////////////////////////////////////////////////

  realResultLo = realSum - imSum;
  imResultLo = imDiff + realDiff;

  data[0] = realResultLo/2;
  data[1] = imResultLo/2;

  ///////////////////////////////////////////////////////////////////////////
  // Start our source pointers to point at re[1] and re[(n/2)-1].
  ///////////////////////////////////////////////////////////////////////////
  pInLo = &data[2];
  pInHi = &data[len-2];
	
  ///////////////////////////////////////////////////////////////////////////
  // for each loop iteration, we calculate two complex results (four
  // floats), so we iterate through the loop len/4 times
  ///////////////////////////////////////////////////////////////////////////
  for (i = 0; i < len/4; i++) {
    /////////////////////////////////////////////////////////////////////////
    // calculate the new sin and cos values
    /////////////////////////////////////////////////////////////////////////
    tempCos1 	= currentCos - (updateA*currentCos  + updateB * currentSin);
    currentSin 	= currentSin - (updateA*currentSin  - updateB * currentCos);
    currentCos 	= tempCos1;

    /////////////////////////////////////////////////////////////////////////
    // load re[i+1], im[i+1]
    /////////////////////////////////////////////////////////////////////////
    realLo	= *pInLo;
    imLo 	= *(pInLo+1);

    /////////////////////////////////////////////////////////////////////////
    // load re[(n/2)-1-i], im[(n/2)-1-i] 
    /////////////////////////////////////////////////////////////////////////
    realHi 	= *pInHi;
    imHi	= *(pInHi+1);
		
    /////////////////////////////////////////////////////////////////////////
    // calculate:
    // realDiff	= re[i+1]-re[(n/2)-1-i]
    // realSum	= re[i+1]+re[(n/2)-1-i]
    /////////////////////////////////////////////////////////////////////////
    realDiff 	= realLo-realHi;
    realSum 	= realLo+realHi;

    /////////////////////////////////////////////////////////////////////////
    // calculate:
    // imDiff	= im[i+1]-im[(n/2)-1-i]
    // imSum	= im[i+1]+im[(n/2)-1-i]
    /////////////////////////////////////////////////////////////////////////
    imDiff 	= imLo-imHi;
    imSum 	= imLo+imHi;
		
    /////////////////////////////////////////////////////////////////////////
    // calculate:
    //
    // realResultLo = 	
    //		re[i+1]+re[(n/2)-1-i] - 
    // 		(im[i+1]+im[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
    //		(re[i+1]-re[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
    //
    // imResultLo = 	
    //		im[i+1]-im[(n/2)-1-i] + 
    // 		(re[i+1]-re[(n/2)-1-i]) * cos( (i+1) * 2pi / n ) -
    //		(im[i+1]+im[(n/2)-1-i]) * sin( (i+1) * 2pi / n )
    //
    /////////////////////////////////////////////////////////////////////////
    realResultLo = realSum - (imSum*currentCos) - (realDiff*currentSin);
    imResultLo = imDiff + (realDiff * currentCos) - (imSum*currentSin);

    /////////////////////////////////////////////////////////////////////////
    // calculate:
    //
    // realResultHi = 	
    //		re[(n/2)-1-i]+re[i+1] -
    // 		(im[(n/2)-1-i]+im[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
    //		(re[(n/2)-1-i]-re[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
    //
    // imResultHi = 	
    //		im[(n/2)-1-i]-im[i+1] + 
    // 		(re[(n/2)-1-i]-re[i+1]) * cos( ((n/2)-i-1) * 2pi / n ) -
    //		(im[(n/2)-1-i]+im[i+1]) * sin( ((n/2)-i-1) * 2pi / n )
    //
    // taking advantage of the following:
    //
    // sin(  (i+1) * 2pi / n )	=  sin ( ((n/2)-i-1) * 2pi / n )
    // cos(  (i+1) * 2pi / n )	= -cos ( ((n/2)-i-1) * 2pi / n )
    // re[(n/2)-1-i]-re[i+1] 	= -(re[i+1]-re[(n/2)-1-i])
    // re[(n/2)-1-i]-re[i+1] 	= -(re[i+1]-re[(n/2)-1-i])
    /////////////////////////////////////////////////////////////////////////

    realResultHi = realSum + (imSum*currentCos) + (realDiff*currentSin);
    imResultHi = -imDiff + (realDiff * currentCos) - (imSum*currentSin);

    /////////////////////////////////////////////////////////////////////////
    // store results, advance low pointer forward to next complex
    // element, and high pointer backward to previous complex element
    /////////////////////////////////////////////////////////////////////////

    *pInLo++ = realResultLo/2;		
    *pInLo++ = imResultLo/2;		
    *(pInHi+1) = imResultHi/2;		
    *pInHi = realResultHi/2;		

    pInHi -= 2;
  }

  /////////////////////////////////////////////////////////////////////////
  // perform an inverse complex fft on the data.  The resulting data
  // is the real-only inverse fft.
  /////////////////////////////////////////////////////////////////////////
  result = FFTComplex(data, len/2, 1);

  return result;	
}

/////////////////////////////////////////////////////////////////////////////
// FFTRealForward
//
// Performs a real forward FFT on the data.  A wrapper function for
// the scalar and AltiVec versions of the forward real FFT, since
// there is a minimum length for the AltiVec implementation.
/////////////////////////////////////////////////////////////////////////////
int32 FFTRealForward(float *data, int32 len)
{
  if (len < ALTIVEC_REAL_MIN_LENGTH) {
    return fft_real_forward_scalar(data, len);
  } else {
    return fft_real_forward_altivec(data, len);
  }
}

/////////////////////////////////////////////////////////////////////////////
// FFTRealInverse
//
// Performs a real inverse FFT on the data.  A wrapper function for
// the scalar and AltiVec versions of the inverse real FFT, since
// there is a minimum length for the AltiVec implementation.
/////////////////////////////////////////////////////////////////////////////
int32 FFTRealInverse(float *data, int32 len)
{
  if (len < ALTIVEC_REAL_MIN_LENGTH) {
    return fft_real_inverse_scalar(data, len);
  } else {
    return fft_real_inverse_altivec(data, len);
  }
}


#pragma mark -
#pragma mark 1 D   C O N V O L U T I O N


/////////////////////////////////////////////////////////////////////////////
//	mul_dyadic_real_altivec
//
//	Performs a dyadic multiply on the hermitian-order data from a
// real FFT.  This means that, for a length-N real signal, the complex
// data that we will be performing the dyadic mul on is expected to be
// in the order:
//
// X = Xr(0) Xr(N/2) Xr(1) Xi(1) Xr(2) Xi(2) ... Xr(N/2-1) Xi(N/2-1)
//
//	Signal2 is replaced by Signal2 X Signal1.  Signal1 is unmodified.
//
//	
/////////////////////////////////////////////////////////////////////////////
static void mul_dyadic_real_altivec(float *signal1, float *signal2, int32 len)
{
  vector unsigned char  vSwappedPerm;
  vector float          vSignal1In, vSignal2In;
  vector float          *pSignal1, *pSignal2;
  vector float          vSignal2Out;
  int32               i;	
  vector float          vMul1, vMul2, vMul3;
  vector unsigned char  vRealsPerm;
  vector unsigned char  vImsPerm;
  vector unsigned char  vSwappedNegatePerm;
  vector float          vNegSignal2;
  vector float          vZero = Altivec_Const4(vector float, 0, 0, 0, 0);
	
  float                 tempreal, tempim, real1, im1, real2, im2;
				
  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x1 x0 x3 y2
  ///////////////////////////////////////////////////////////////////////////
  vSwappedPerm  = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 0, 1, 2,
				  3, 12, 13, 14, 15, 8, 9, 10, 11);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x0 x2 x2
  ///////////////////////////////////////////////////////////////////////////
  vRealsPerm  = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 0, 1, 2,
				3, 8, 9, 10, 11, 8, 9, 10, 11);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x1 x1 x3 x3
  ///////////////////////////////////////////////////////////////////////////
  vImsPerm  = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 4, 5, 6, 7,
			      12, 13, 14, 15, 12, 13, 14, 15);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = y1 x0 y3 x2
  ///////////////////////////////////////////////////////////////////////////
  vSwappedNegatePerm  = Altivec_Const16(vector unsigned char, 20, 21, 22, 23,
					0, 1, 2, 3, 28, 29, 30, 31, 8, 9, 10, 11);
	
  ///////////////////////////////////////////////////////////////////////////
  // do simple multiply of real elements Xr(0) and Xr(N/2)
  ///////////////////////////////////////////////////////////////////////////

  signal2[0] 	*= signal1[0];
  signal2[1] 	*= signal1[1];
	
  ///////////////////////////////////////////////////////////////////////////
  // do scalar complex multiply of X(1) with Y(1)
  ///////////////////////////////////////////////////////////////////////////

  real1		= signal1[2];
  real2 	= signal2[2];
  im1 		= signal1[3];
  im2 		= signal2[3];
	
  tempreal	= (real1*real2) - (im1*im2);
  tempim 	= (real1*im2) + (real2*im1);
	
  signal2[2]	= tempreal;
  signal2[3] 	= tempim;

  ///////////////////////////////////////////////////////////////////////////
  // the rest of the data is complex, and aligned on a vector
  // boundary, so we will perform all subsequent multiplies in the
  // vector domain.  We start by setting two pointers to point at the
  // start of the remaining signal
  ///////////////////////////////////////////////////////////////////////////

  pSignal1 = ((vector float*)signal1)+1;
  pSignal2 = ((vector float*)signal2)+1;
	
  ///////////////////////////////////////////////////////////////////////////
  // we loop through all remaining complex elements, performing a
  // complex multiply
  ///////////////////////////////////////////////////////////////////////////

  for (i = 1; i < len/4; i++) {
    /////////////////////////////////////////////////////////////////////////
    // load in a vector from each of signal1 and signal2
    /////////////////////////////////////////////////////////////////////////

    vSignal1In = *pSignal1++;
    vSignal2In = *pSignal2;
		
    /////////////////////////////////////////////////////////////////////////
    // negate signal 2, so we have negative values necessary for our
    // multiplies
    /////////////////////////////////////////////////////////////////////////

    vNegSignal2 = vec_sub(vZero, vSignal2In);

    /////////////////////////////////////////////////////////////////////////
    // set up multiplicand vectors.  Since we are multiplying complex
    // numbers, we need to do more than just call a multiply routine.
    // Given two complex numbers x = (xr, xi) and y = (yr, yi), then
    // the product z = (zr, zi) = x*y is defined as:
    // 
    //	zr = xr*yr - xi*yi
    //	zi = xr*yi + xi*yr
    //
    // to perform this multiplication, we generate four multiplicand
    // vectors. If we consider our input vectors X and Y, they each
    // contain two complex numbers:
    //
    //	X = (x1r, x1i, x2r, x2i)
    //	Y = (y1r, y1i, y2r, y2i)
    //
    // then we want to generate a new output Z vector:
    //
    //	Z = (x1r*y1r - x1i*y1i, x1r*y1i + x1i*y1r,
    //       x2r*y2r - x2i*y2i, x2r*y2i + x2i*y2r)
    //
    //	So, we need to have multiplicand vectors:
    //
    //	M1 = 	x1r		x1r		x2r		x2r
    //	M2 = 	x1i		x1i		x2i 	x2i
    //	M3 =	-y1i	y1r		-y2i	y2r
    //	M4 = 	y1r		y1i		y2r		y2i
    //
    //	We can then generate our Z vector with a vector mul and a
    //	vector mul-add.
    /////////////////////////////////////////////////////////////////////////
		
    vMul1 = vec_perm(vSignal1In, vSignal1In, vRealsPerm);
    vMul2 = vec_perm(vSignal1In, vSignal1In, vImsPerm);
    vMul3 = vec_perm(vSignal2In, vNegSignal2, vSwappedNegatePerm);
		
    vSignal2Out = vec_madd(vMul1, vSignal2In, vZero);
    vSignal2Out = vec_madd(vMul2, vMul3, vSignal2Out);
		
    /////////////////////////////////////////////////////////////////////////
    // store product vector back to current position in signal 2
    /////////////////////////////////////////////////////////////////////////

    *pSignal2++ = vSignal2Out;
  }
}


/////////////////////////////////////////////////////////////////////////////
//	mul_dyadic_complex_altivec
//
// Performs a dyadic multiply on two complex signals. 
//	
//	Signal2 is replaced by Signal2 X Signal1.  Signal1 is unmodified.
//
/////////////////////////////////////////////////////////////////////////////
static void mul_dyadic_complex_altivec(float *signal1, float *signal2, int32 len)
{
  vector unsigned char  vSwappedPerm;
  vector float          vSignal1In, vSignal2In;
  vector float          *pSignal1, *pSignal2;
  vector float          vSignal2Out;
  int32               i;	
  vector float          vMul1, vMul2, vMul3;
  vector unsigned char  vRealsPerm;
  vector unsigned char  vImsPerm;
  vector unsigned char  vSwappedNegatePerm;
  vector float          vNegSignal2;
  vector float          vZero = Altivec_Const4(vector float, 0, 0, 0, 0);
				
  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x1 x0 x3 y2
  ///////////////////////////////////////////////////////////////////////////
  vSwappedPerm  = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 0, 1, 2,
				  3, 12, 13, 14, 15, 8, 9, 10, 11);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x0 x2 x2
  ///////////////////////////////////////////////////////////////////////////
  vRealsPerm  = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 0, 1, 2,
				3, 8, 9, 10, 11, 8, 9, 10, 11);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x1 x1 x3 x3
  ///////////////////////////////////////////////////////////////////////////
  vImsPerm  = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 4, 5, 6, 7,
			      12, 13, 14, 15, 12, 13, 14, 15);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = y1 x0 y3 x2
  ///////////////////////////////////////////////////////////////////////////
  vSwappedNegatePerm  = Altivec_Const16(vector unsigned char, 20, 21, 22, 23,
					0, 1, 2, 3, 28, 29, 30, 31, 8, 9, 10, 11);
	
  ///////////////////////////////////////////////////////////////////////////
  // initialize pointers to start at beginning of both signals
  ///////////////////////////////////////////////////////////////////////////

  pSignal1 = (vector float*)signal1;
  pSignal2 = (vector float*)signal2;
	
  ///////////////////////////////////////////////////////////////////////////
  // loop through all elements one vector (two complex entries) at a time.
  ///////////////////////////////////////////////////////////////////////////


  for (i = 0; i < len/2; i++) {
    /////////////////////////////////////////////////////////////////////////
    // load in a vector from each of signal1 and signal2
    /////////////////////////////////////////////////////////////////////////

    vSignal1In = *pSignal1++;
    vSignal2In = *pSignal2;
				
    /////////////////////////////////////////////////////////////////////////
    // negate signal 2, so we have negative values necessary for our
    // multiplies
    /////////////////////////////////////////////////////////////////////////

    vNegSignal2 = vec_sub(vZero, vSignal2In);
		
    /////////////////////////////////////////////////////////////////////////
    // set up multiplicand vectors.  Since we are multiplying complex
    // numbers, we need to do more than just call a multiply routine.
    // Given two complex numbers x = (xr, xi) and y = (yr, yi), then
    // the product z = (zr, zi) = x*y is defined as:
    // 
    //	zr = xr*yr - xi*yi
    //	zi = xr*yi + xi*yr
    //
    // to perform this multiplication, we generate four multiplicand
    // vectors. If we consider our input vectors X and Y, they each
    // contain two complex numbers:
    //
    //	X = (x1r, x1i, x2r, x2i)
    //	Y = (y1r, y1i, y2r, y2i)
    //
    // then we want to generate a new output Z vector:
    //
    //	Z = (x1r*y1r - x1i*y1i, x1r*y1i + x1i*y1r,
    //       x2r*y2r - x2i*y2i, x2r*y2i + x2i*y2r)
    //
    //	So, we need to have multiplicand vectors:
    //
    //	M1 = 	x1r	x1r	x2r	x2r
    //	M2 = 	x1i	x1i	x2i 	x2i
    //	M3 =	-y1i	y1r	-y2i	y2r
    //	M4 = 	y1r	y1i	y2r	y2i
    //
    //	We can then generate our Z vector with a vector mul and a
    //	vector mul-add.
    /////////////////////////////////////////////////////////////////////////

    vMul1 = vec_perm(vSignal1In, vSignal1In, vRealsPerm);
    vMul2 = vec_perm(vSignal1In, vSignal1In, vImsPerm);
    vMul3 = vec_perm(vSignal2In, vNegSignal2, vSwappedNegatePerm);
		
    vSignal2Out = vec_madd(vMul1, vSignal2In, vZero);
    vSignal2Out = vec_madd(vMul2, vMul3, vSignal2Out);
		
    /////////////////////////////////////////////////////////////////////////
    // store product vector back to current position in signal 2
    /////////////////////////////////////////////////////////////////////////

    *pSignal2++ = vSignal2Out;
  }
}

/////////////////////////////////////////////////////////////////////////////
//	ConvolveComplexAltivec
//
//	calculates the convolution of signal1 and signal2, both of
// length n.  This is done by performing a forward FFT on both
// signals, then calculating the dyadic mul of the two resulting
// signals.  Finally, an inverse FFT is performed on the result of the
// dyadic mul.
//
//	signal2 is replaced by the convolution of signal1 and
// signal2. signal1 is replaced by the FFT of signal1.
//
/////////////////////////////////////////////////////////////////////////////
int32 ConvolveComplexAltivec(float *pSignal1, float *pSignal2, int32 n)
{
  int32 result = 0;
	
  ///////////////////////////////////////////////////////////////////////////
  // if the length is great enough, then we want to perform a
  // columnwise matrix FFT instead of a standard complex FFT. One
  // leaves the data in normal order, the other leaves it in
  // columnwise order. (The motivation is that the columnwise FFT is
  // considerably faster for longer run lengths.)
	
  // We will then be peforming a dyadic mul (for which order doesn't
  // matter).  Finally, we will perform an inverse FFT, and if we used
  // the columnwise forward FFT (which left data in columnwise order)
  // then we will use the columnwise inverse FFT, which will expect
  // the input data in columnwise order.
  ///////////////////////////////////////////////////////////////////////////
	
  if (n >= ALTIVEC_COLUMNWISE_FFT_BREAKOVER) {
    /////////////////////////////////////////////////////////////////////////
    // perform matrix fft on data which will leave data in columnwise order.
    /////////////////////////////////////////////////////////////////////////
		
    result = fft_matrix_forward_columnwise(pSignal1, n);

  } else {
    /////////////////////////////////////////////////////////////////////////
    // perform standard complex fft on data
    /////////////////////////////////////////////////////////////////////////

    result = FFTComplex(pSignal1, n, -1);
  }
	
  if (result == 0) {

    if (n >= ALTIVEC_COLUMNWISE_FFT_BREAKOVER) {
      ///////////////////////////////////////////////////////////////////////
      // perform matrix fft on data which will leave data in
      // columnwise order.
      ///////////////////////////////////////////////////////////////////////

      result = fft_matrix_forward_columnwise(pSignal2, n);

    } else {
      ///////////////////////////////////////////////////////////////////////
      // perform standard complex fft on data
      ///////////////////////////////////////////////////////////////////////

      result = FFTComplex(pSignal2, n, -1);
    }

    if (result == 0) {
      ///////////////////////////////////////////////////////////////////////
      // perform dyadic multiply on result of both FFTs, replacing
      // signal2 with the result
      ///////////////////////////////////////////////////////////////////////

      mul_dyadic_complex_altivec(pSignal1, pSignal2, n);
			
      ///////////////////////////////////////////////////////////////////////
      // perform inverse FFT on signal2, which will yield a result
      // that is the convolution of signal1 and signal2.
      ///////////////////////////////////////////////////////////////////////

      if (n >= ALTIVEC_COLUMNWISE_FFT_BREAKOVER) {
	/////////////////////////////////////////////////////////////////////
	// perform inverse matrix fft on data which expects input data
	// to be in columnwise order.
	/////////////////////////////////////////////////////////////////////

	result = fft_matrix_inverse_columnwise(pSignal2, n);
				
      } else {
	/////////////////////////////////////////////////////////////////////
	// perform standard inverse complex fft on data
	/////////////////////////////////////////////////////////////////////

	result = FFTComplex(pSignal2, n, 1);
      }
    }
  }

  return result; 
}



/////////////////////////////////////////////////////////////////////////////
//	ConvolveRealAltivec
//
//	calculates the convolution of signal1 and signal2, both of
// length n.  This is done by performing a forward FFT on both
// signals, then calculating the dyadic mul of the two resulting
// signals.  Finally, an inverse FFT is performed on the result of the
// dyadic mul.
//
//	signal2 is replaced by the convolution of signal1 and
// signal2. signal1 is replaced by the FFT of signal1.
//
/////////////////////////////////////////////////////////////////////////////
int32 ConvolveRealAltivec(float *pSignal1, float *pSignal2, int32 length)
{
  int32 result = 0;
	
  ///////////////////////////////////////////////////////////////////////////
  // perform real FFT on signal1
  ///////////////////////////////////////////////////////////////////////////

  result = FFTRealForward(pSignal1, length);
    
  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////
    // perform real FFT on signal2
    /////////////////////////////////////////////////////////////////////////

    result = FFTRealForward(pSignal2, length);
		
    if (result == 0) {
      ///////////////////////////////////////////////////////////////////////
      // perform dyadic mul on result of both FFTs, which are stored
      // in hermitian order.
      ///////////////////////////////////////////////////////////////////////

      mul_dyadic_real_altivec(pSignal1, pSignal2, length);

      ///////////////////////////////////////////////////////////////////////
      // perform inverse real FFT on result of dyadic mul, which
      // produces the convolution of signal1 and signal2.
      ///////////////////////////////////////////////////////////////////////

      result = FFTRealInverse(pSignal2, length);
    }
  }
	
  return result;
}


#pragma mark -
#pragma mark 2 D   F F T

/////////////////////////////////////////////////////////////////////////////
//	FFT2DComplex
//
//	performs a 2-dimensional FFT on a complex 2D (width X height)
// signal. If iflag is -1, a forward FFT is performed, otherwise an
// inverse FFT is performed.
//
//	**NOTE** that implementation details demand that width >= 2
//	and height >=2
//
/////////////////////////////////////////////////////////////////////////////

int32 FFT2DComplex(float *pData, int32 width, int32 height,
		     int32 iflag)
{
  int32  i, j;
  float    *pRow;
  int32  rowPower;
  int32  colPower;
  int32  result = 0;
	
  ///////////////////////////////////////////////////////////////////////////
  // allocate a buffer for column ferrying of two columns.
  ///////////////////////////////////////////////////////////////////////////
  vector float rowBufferPointer[height];

  ///////////////////////////////////////////////////////////////////////////
  // ensure that width & height are at least 2 (required for algorithm
  // impl.)
  ///////////////////////////////////////////////////////////////////////////
  if ((width < 2) || (height < 2)) {
    return EINVAL;	
  }

  ///////////////////////////////////////////////////////////////////////////
  // data must be 16-byte aligned because of vector load/store requirements
  ///////////////////////////////////////////////////////////////////////////
  if (((int32)pData) & 0x0F) {
    return EINVAL;
  }
			       
  rowPower = log2max(width);
  colPower = log2max(height);

  ///////////////////////////////////////////////////////////////////////////
  // width must be an exact power of 2
  ///////////////////////////////////////////////////////////////////////////
  if ((1 << rowPower) != width) {
    return EINVAL;
  }

  ///////////////////////////////////////////////////////////////////////////
  // height must be an exact power of 2
  ///////////////////////////////////////////////////////////////////////////
  if ((1 << colPower) != height) {
    return EINVAL;
  }
    
  if (result == 0) {
    vector float          *pCurrentColumn;
    vector float          vIn1, vIn2;
    vector float          vOut1, vOut2;
    vector unsigned char  vMergeHiPairPerm;
    vector unsigned char  vMergeLoPairPerm;
    vector float          *pSplit1, *pSplit2;
    float                 *pFFT1, *pFFT2;
		
    /////////////////////////////////////////////////////////////////////////
    // perform an FFT on every row of the 2D array
    /////////////////////////////////////////////////////////////////////////
    pRow = pData;
		
    for (i = 0; i < height; i++) {
      result = FFTComplex(pRow+i*(width*2), width, iflag);
      if (result != 0) break;
    }
		
    if (result == 0) {		
      ///////////////////////////////////////////////////////////////////////
      // initialize a permute vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = x0 x1 y0 y1
      ///////////////////////////////////////////////////////////////////////
      vMergeHiPairPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4,
					 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 23);

      ///////////////////////////////////////////////////////////////////////
      // initialize a permute vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = x2 x3 y2 y3
      ///////////////////////////////////////////////////////////////////////
      vMergeLoPairPerm = Altivec_Const16(vector unsigned char, 8, 9, 10, 11,
					 12, 13, 14, 15, 24, 25, 26, 27, 28,
					 29, 30, 31);
					
      ///////////////////////////////////////////////////////////////////////
      // we now loop through all columns, two columns at a time, and
      // perform column ferrying FFTs on the columns.  We first copy
      // the columns to a separate buffer, then perform the FFTs, and
      // finally copy them back to their original column positions.
      ///////////////////////////////////////////////////////////////////////

      for (i = 0; i < width/2; i++) {
	/////////////////////////////////////////////////////////////////////
	// point at top of current columns
	/////////////////////////////////////////////////////////////////////
	pCurrentColumn = ((vector float*)pData) + i;		

	/////////////////////////////////////////////////////////////////////
	// initialize pointers in temporary buffer for storing copied
	// columns.
	/////////////////////////////////////////////////////////////////////
	pSplit1 = (vector float*)rowBufferPointer;
	pSplit2 = pSplit1 + height / 2;
					
	/////////////////////////////////////////////////////////////////////
	// loop through all rows in the current column, copying
	// elements to the temporary buffer.  We load two rows at a
	// time, and permute the vectors to create a single vector
	// that contains the appropriate column elements from both
	// rows.
	/////////////////////////////////////////////////////////////////////

	for (j = 0; j < height/2; j++) {
	  ///////////////////////////////////////////////////////////////////
	  // read in two rows worth of vectors (two rows X two columns)
	  ///////////////////////////////////////////////////////////////////

	  vIn1 = *pCurrentColumn;
	  pCurrentColumn += width/2;
	  vIn2 = *pCurrentColumn;
	  pCurrentColumn += width/2;
					
	  ///////////////////////////////////////////////////////////////////
	  // permute the vectors so that the two left column entries
	  // are in one vector and the two right column entries are in
	  // one vector
	  ///////////////////////////////////////////////////////////////////

	  vOut1 = vec_perm(vIn1, vIn2, vMergeHiPairPerm);
	  vOut2 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
					
	  ///////////////////////////////////////////////////////////////////
	  // store the split columns to the appropriate buffers
	  ///////////////////////////////////////////////////////////////////

	  *pSplit1++ = vOut1;
	  *pSplit2++ = vOut2;
	}

	/////////////////////////////////////////////////////////////////////
	// perform FFTs on the two columns of data that we have copied
	// to the temp buffer
	/////////////////////////////////////////////////////////////////////

	pFFT1 = (float *)rowBufferPointer;
	pFFT2 = pFFT1 + 2*height;
				
	result = FFTComplex(pFFT1, height, iflag);
	if (result != 0) break;
				
	result = FFTComplex(pFFT2, height, iflag);
	if (result != 0) break;
					
	/////////////////////////////////////////////////////////////////////
	// point at the beginning of the two copied column buffers,
	// and at the top of the columns in the matrix where we will
	// store them back.
	/////////////////////////////////////////////////////////////////////

	pSplit1 = (vector float*)rowBufferPointer;
	pSplit2 = pSplit1 + height / 2;
	pCurrentColumn = ((vector float*)pData) + i;

	/////////////////////////////////////////////////////////////////////
	// loop through all of the column entries that are stored in
	// the temp buffer, and merge them to be stored back into the
	// columns of the matrix.
	/////////////////////////////////////////////////////////////////////

	for (j = 0; j < height/2; j++) {
	  ///////////////////////////////////////////////////////////////////
	  // get two vectors of column data
	  ///////////////////////////////////////////////////////////////////

	  vIn1 = *pSplit1++;
	  vIn2 = *pSplit2++;
					
	  ///////////////////////////////////////////////////////////////////
	  // turn them into row vectors
	  ///////////////////////////////////////////////////////////////////

	  vOut1 = vec_perm(vIn1, vIn2, vMergeHiPairPerm);
	  vOut2 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
									
	  ///////////////////////////////////////////////////////////////////
	  // store row vectors back into the matrix
	  ///////////////////////////////////////////////////////////////////

	  *pCurrentColumn = vOut1;
	  pCurrentColumn += width / 2;
	  *pCurrentColumn = vOut2;			
	  pCurrentColumn += width / 2;
	}
      }
    }	
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
//	FFT2DRealForward
//
// performs a 2-dimensional FFT on a real 2D (width X height) signal. 
// Resulting data is stored in a matrix in the following arrangement:
//
// X = 
//		
// Xr(0,0)   Xr(0, W/2)  Xr(0,1) Xi(0,1) Xr(0,2) Xi(0,2)...Xr(0,W/2-1) Xi(0,W/2-1)
// Xr(H/2,0) Xr(H/2,W/2) Xr(1,1) Xi(1,1) Xr(1,2) Xi(1,2)...Xr(1,W/2-1) Xi(1,W/2-1)
// Xr(1,0)   Xr(1,W/2)   Xr(2,1) Xi(2,1) Xr(2,2) Xi(2,2)...Xr(2,W/2-1) Xi(2,W/2-1)
// Xi(1,0)   Xi(1,W/2)   Xr(3,1) Xi(3,1) Xr(3,2) Xi(3,2)...Xr(3,W/2-1) Xi(3,W/2-1)
//	.
//	.
//	.
// Xr(H/2-1,0) Xr(H/2-1,W/2) Xr(H-2,1) Xi(H-2,1)  ...  Xr(H-2,W/2-1) Xi(H-2,W/2-1)
// Xi(H/2-1,0) Xi(H/2-1,W/2) Xr(H-1,1) Xi(H-1,1)  ...  Xr(H-1,W/2-1) Xi(H-1,W/2-1)
//
// We perform the 2D fft by taking the following steps.  First, we
// perform a forward real fft on each row. This results in the first
// two entries of each row being the real elements of X[0] and X[m/2],
// given m columns.  The remaining entries in the row are the complex
// elements of X[1] ... X[m-1].  Next we perform a real FFT on the
// first two columns of the matrix (which are real data).  finally, we
// perform a complex FFT on the remaining columns of complex data.
//
// **NOTE** that implementation details demand that width >= 8 and height >=4
//
//////////////////////////////////////////////////////////////////////////////
int32 FFT2DRealForward(float *data, int32 width, int32 height)
{
  int32  i, j;
  float    *pRow;
  int32  rowPower;
  int32  colPower;
  int32  result = 0;
	
  ///////////////////////////////////////////////////////////////////////////
  // allocate a buffer for column ferrying of two columns.
  ///////////////////////////////////////////////////////////////////////////
  vector float rowBufferPointer[height];
			       
  rowPower = log2max(width);
  colPower = log2max(height);

  ///////////////////////////////////////////////////////////////////////////
  // ensure that width & height are at least 4 (required for algorithm impl)
  ///////////////////////////////////////////////////////////////////////////
  if ((width < 8) || (height < 4)) {
    return EINVAL;	
  }
    
  ///////////////////////////////////////////////////////////////////////////
  // width must be an exact power of 2
  ///////////////////////////////////////////////////////////////////////////

  if ((1 << rowPower) != width) {
    return EINVAL;
  }

  ///////////////////////////////////////////////////////////////////////////
  // height must be an exact power of 2
  ///////////////////////////////////////////////////////////////////////////

  if ((1 << colPower) != height) {
    return EINVAL;
  }
    
  if (result == 0) {
    vector float          *pCurrentColumn;
    vector float          vIn1, vIn2, vIn3, vIn4;
    vector float          vOut1, vOut2, vOut3, vOut4;
    vector unsigned char  vMergeHiPairPerm;
    vector unsigned char  vMergeLoPairPerm;
    vector float          *pSplit1, *pSplit2, *pSplit3;
    float                 *pFFT1, *pFFT2;
    vector float          vTemp1, vTemp2;
		
    /////////////////////////////////////////////////////////////////////////
    // perform standard forward real fft on every row
    /////////////////////////////////////////////////////////////////////////

    pRow = data;
		
    for (i=0; i<height; i++) {
      result = FFTRealForward(pRow+i*(width), width);
      if (result != 0) break;
    }

    if (result == 0) {
      ///////////////////////////////////////////////////////////////////////
      // initialize a permute vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = x0 x1 y0 y1
      ///////////////////////////////////////////////////////////////////////
      vMergeHiPairPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5,
					 6, 7, 16, 17, 18, 19, 20, 21, 22, 23);

      ///////////////////////////////////////////////////////////////////////
      // initialize a permute vector that, given input vectors:
      //
      //	X = x0 x1 x2 x3
      //	Y = y0 y1 y2 y3
      //
      // will generate
      //	Z = x2 x3 y2 y3
      ///////////////////////////////////////////////////////////////////////
      vMergeLoPairPerm = Altivec_Const16(vector unsigned char, 8, 9, 10, 11,
					 12, 13, 14, 15, 24, 25, 26, 27, 28,
					 29, 30, 31);

      ///////////////////////////////////////////////////////////////////////
      // point at top of current columns in matrix
      ///////////////////////////////////////////////////////////////////////

      pCurrentColumn = (vector float*)data;		

      ///////////////////////////////////////////////////////////////////////
      // prepare pointers into column buffer to make copies of first
      // two columns of data (which are real data) and third column,
      // which is complex data.
      ///////////////////////////////////////////////////////////////////////

      pSplit1 = (vector float*)rowBufferPointer;
      pSplit2 = pSplit1 + height / 4;
      pSplit3 = pSplit1 + height / 2;
				
      ///////////////////////////////////////////////////////////////////////
      // loop through all rows (four at a time), making copies of the
      // two real columns and one complex column.
      ///////////////////////////////////////////////////////////////////////

      for (j = 0; j < height/4; j++) {
	/////////////////////////////////////////////////////////////////////
	// read in four rows of data from the columns.
	/////////////////////////////////////////////////////////////////////

	vIn1 = *pCurrentColumn;
	pCurrentColumn += width/4;
	vIn2 = *pCurrentColumn;
	pCurrentColumn += width/4;
	vIn3 = *pCurrentColumn;
	pCurrentColumn += width/4;
	vIn4 = *pCurrentColumn;
	pCurrentColumn += width/4;
				
	/////////////////////////////////////////////////////////////////////
	// create temp vectors that contain real data from first two columns
	/////////////////////////////////////////////////////////////////////

	vTemp1 = vec_mergeh(vIn1, vIn2);
	vTemp2 = vec_mergeh(vIn3, vIn4);
				
	/////////////////////////////////////////////////////////////////////
	// create a vector that contains four entries from first real column
	/////////////////////////////////////////////////////////////////////
				
	vOut1 = vec_perm(vTemp1, vTemp2, vMergeHiPairPerm);

	/////////////////////////////////////////////////////////////////////
	// create a vector that contains four entries from second real column
	/////////////////////////////////////////////////////////////////////

	vOut2 = vec_perm(vTemp1, vTemp2, vMergeLoPairPerm);

	/////////////////////////////////////////////////////////////////////
	// create two vectors that contains four entries from first
	// complex column
	/////////////////////////////////////////////////////////////////////

	vOut3 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
	vOut4 = vec_perm(vIn3, vIn4, vMergeLoPairPerm);

	/////////////////////////////////////////////////////////////////////
	// store our vectors to appropriate temporary buffer location
	/////////////////////////////////////////////////////////////////////
							
	*pSplit1++ = vOut1;
	*pSplit2++ = vOut2;
	*pSplit3++ = vOut3;
	*pSplit3++ = vOut4;
      }

      ///////////////////////////////////////////////////////////////////////
      // perform real fft on copies of first two columns
      ///////////////////////////////////////////////////////////////////////

      pFFT1 = (float *)rowBufferPointer;
      pFFT2 = pFFT1 + height;
			
      result =  FFTRealForward(pFFT1, height);
      if (result == 0) {
	result = FFTRealForward(pFFT2, height);
      }

      ///////////////////////////////////////////////////////////////////////
      // perform complex fft on copy of first complex column
      ///////////////////////////////////////////////////////////////////////

      pFFT2 = pFFT1 + 2*height;

      if (result == 0) {
	result = FFTComplex(pFFT2, height, -1);
      }
			
      if (result == 0) {
	/////////////////////////////////////////////////////////////////////
	// point at beginning of copy buffers to prepare to copy data
	// back into columns of matrix after having performed FFTs.
	/////////////////////////////////////////////////////////////////////
					
	pSplit1 = (vector float*)rowBufferPointer;
	pSplit2 = pSplit1 + height / 4;
	pSplit3 = pSplit1 + height / 2;

	/////////////////////////////////////////////////////////////////////
	// point to top of columns that we will be copying back into
	/////////////////////////////////////////////////////////////////////
	pCurrentColumn = (vector float*)data;


	/////////////////////////////////////////////////////////////////////
	// loop through all rows (four at a time), copying data from
	// temporary buffer back into appropriate row positions in the
	// current columns
	/////////////////////////////////////////////////////////////////////
	for (j=0; j < height/4; j++) {
	  ///////////////////////////////////////////////////////////////////
	  // get one vector from result of first real fft
	  ///////////////////////////////////////////////////////////////////

	  vIn1 = *pSplit1++;

	  ///////////////////////////////////////////////////////////////////
	  // get one vector from result of second real fft
	  ///////////////////////////////////////////////////////////////////
					
	  vIn2 = *pSplit2++;

	  ///////////////////////////////////////////////////////////////////
	  // get two vector from result of first complex fft
	  ///////////////////////////////////////////////////////////////////

	  vIn3 = *pSplit3++;
	  vIn4 = *pSplit3++;
				
	  ///////////////////////////////////////////////////////////////////
	  // permute all data back into columnwise form to be stored
	  // as vectors in the original matrix
	  ///////////////////////////////////////////////////////////////////

	  vTemp1 = vec_mergeh(vIn1, vIn2);	
	  vTemp2 = vec_mergel(vIn1, vIn2);	
					
	  vOut1 = vec_perm(vTemp1, vIn3, vMergeHiPairPerm);
	  vOut2 = vec_perm(vTemp1, vIn3, vMergeLoPairPerm);
	  vOut3 = vec_perm(vTemp2, vIn4, vMergeHiPairPerm);
	  vOut4 = vec_perm(vTemp2, vIn4, vMergeLoPairPerm);

	  ///////////////////////////////////////////////////////////////////
	  // store four vectors of data back into the current columns
	  ///////////////////////////////////////////////////////////////////
									
	  *pCurrentColumn = vOut1;
	  pCurrentColumn += width / 4;
	  *pCurrentColumn = vOut2;
	  pCurrentColumn += width / 4;
	  *pCurrentColumn = vOut3;
	  pCurrentColumn += width / 4;
	  *pCurrentColumn = vOut4;
	  pCurrentColumn += width / 4;
	}

	/////////////////////////////////////////////////////////////////////
	// we now loop through all remaining columns, two columns at a
	// time, and perform column ferrying FFTs on the columns.  We
	// first copy the columns to a separate buffer, then perform
	// the FFTs, and finally copy them back to their original
	// column positions.
	/////////////////////////////////////////////////////////////////////

	for (i = 1; i < width/4; i++) {
	  ///////////////////////////////////////////////////////////////////
	  // point at top of current columns
	  ///////////////////////////////////////////////////////////////////
				
	  pCurrentColumn = ((vector float*)data) + i;		

	  ///////////////////////////////////////////////////////////////////
	  // initialize pointers in temporary buffer for storing
	  // copied columns.
	  ///////////////////////////////////////////////////////////////////

	  pSplit1 = (vector float*)rowBufferPointer;
	  pSplit2 = pSplit1 + height / 2;
						
	  ///////////////////////////////////////////////////////////////////
	  // loop through all of the column entries that are stored in
	  // the temp buffer, and merge them to be stored back into
	  // the columns of the matrix.
	  ///////////////////////////////////////////////////////////////////

	  for (j = 0; j < height/2; j++) {
	    /////////////////////////////////////////////////////////////////
	    // read in two rows worth of vectors (two rows X two columns)
	    /////////////////////////////////////////////////////////////////
	    vIn1 = *pCurrentColumn;
	    pCurrentColumn += width/4;
	    vIn2 = *pCurrentColumn;
	    pCurrentColumn += width/4;
						
	    /////////////////////////////////////////////////////////////////
	    // permute the vectors so that the two left column entries
	    // are in one vector and the two right column entries are
	    // in one vector
	    /////////////////////////////////////////////////////////////////
	    vOut1 = vec_perm(vIn1, vIn2, vMergeHiPairPerm);
	    vOut2 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
						
	    /////////////////////////////////////////////////////////////////
	    // store the split columns to the appropriate buffers
	    /////////////////////////////////////////////////////////////////
	    *pSplit1++ = vOut1;
	    *pSplit2++ = vOut2;
	  }

	  ///////////////////////////////////////////////////////////////////
	  // perform FFTs on the two columns of data that we have
	  // copied to the temp buffer
	  ///////////////////////////////////////////////////////////////////

	  pFFT1 = (float *)rowBufferPointer;
	  pFFT2 = pFFT1 + 2*height;
					
	  result = FFTComplex(pFFT1, height, -1);
	  if (result != 0) break;
					
	  result = FFTComplex(pFFT2, height, -1);
	  if (result != 0) break;
						
	  ///////////////////////////////////////////////////////////////////
	  // point at the beginning of the two copied column buffers,
	  // and at the top of the columns in the matrix where we will
	  // store them back.
	  ///////////////////////////////////////////////////////////////////

	  pSplit1 = (vector float*)rowBufferPointer;
	  pSplit2 = pSplit1 + height / 2;
	  pCurrentColumn = ((vector float*)data) + i;

	  ///////////////////////////////////////////////////////////////////
	  // loop through all of the column entries that are stored in
	  // the temp buffer, and merge them to be stored back into
	  // the columns of the matrix.
	  ///////////////////////////////////////////////////////////////////
	  for (j = 0; j < height/2; j++) {
	    /////////////////////////////////////////////////////////////////
	    // get two vectors of column data
	    /////////////////////////////////////////////////////////////////

	    vIn1 = *pSplit1++;
	    vIn2 = *pSplit2++;
						
	    /////////////////////////////////////////////////////////////////
	    // turn them into row vectors
	    /////////////////////////////////////////////////////////////////

	    vOut1 = vec_perm(vIn1, vIn2, vMergeHiPairPerm);
	    vOut2 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
										
	    /////////////////////////////////////////////////////////////////
	    // store row vectors back into the matrix
	    /////////////////////////////////////////////////////////////////

	    *pCurrentColumn = vOut1;
	    pCurrentColumn += width / 4;
	    *pCurrentColumn = vOut2;				
	    pCurrentColumn += width / 4;
	  }		
	}
      }				
    }
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////
//	FFT2DRealInverse
//
// performs an inverse 2-dimensional FFT on a real 2D (width X height)
// signal.  Routine expects input data to be in the following
// 2D-hermitian form:
//
// X = 
//		
// Xr(0,0)   Xr(0, W/2)  Xr(0,1) Xi(0,1) Xr(0,2) Xi(0,2) ... Xr(0,W/2-1) Xi(0,W/2-1)
// Xr(H/2,0) Xr(H/2,W/2) Xr(1,1) Xi(1,1) Xr(1,2) Xi(1,2) ... Xr(1,W/2-1) Xi(1,W/2-1)
// Xr(1,0)   Xr(1,W/2)   Xr(2,1) Xi(2,1) Xr(2,2) Xi(2,2) ... Xr(2,W/2-1) Xi(2,W/2-1)
// Xi(1,0)   Xi(1,W/2)   Xr(3,1) Xi(3,1) Xr(3,2) Xi(3,2) ... Xr(3,W/2-1) Xi(3,W/2-1)
//	.
//	.
//	.
// Xr(H/2-1,0) Xr(H/2-1,W/2) Xr(H-2,1) Xi(H-2,1)  ...  Xr(H-2,W/2-1) Xi(H-2,W/2-1)
// Xi(H/2-1,0) Xi(H/2-1,W/2) Xr(H-1,1) Xi(H-1,1)  ...  Xr(H-1,W/2-1) Xi(H-1,W/2-1)
//
// The inverse real 2D FFT is performed in the following steps.
// First, an inverse real FFT is performed on the first two columns.
// Then, an inverse complex FFT is performed on the remaining columns
// of complex numbers.  Finally, an inverse real FFT is performed on
// each row of the matrix, resulting in a width X height all-real
// signal.
//
// **NOTE** that implementation details demand that width >= 8 and height >=4
//
/////////////////////////////////////////////////////////////////////////////

int32 FFT2DRealInverse(float *data, int32 width, int32 height)
{
  int32  i, j;
  float    *pRow;
  int32  rowPower;
  int32  colPower;
  int32  result = 0;

  ///////////////////////////////////////////////////////////////////////////
  // allocate a buffer for column ferrying of two columns.
  ///////////////////////////////////////////////////////////////////////////
  vector float rowBufferPointer[height];
	
  ///////////////////////////////////////////////////////////////////////////
  // ensure that width & height are at least 4 (required for algorithm impl)
  ///////////////////////////////////////////////////////////////////////////
  if ((width < 8) || (height < 4)) {
    return EINVAL;	
  }    
			       
  rowPower = log2max(width);
  colPower = log2max(height);
    
  ///////////////////////////////////////////////////////////////////////////
  // width must be an exact power of 2
  ///////////////////////////////////////////////////////////////////////////
  if ((1 << rowPower) != width) {
    return EINVAL;
  }

  ///////////////////////////////////////////////////////////////////////////
  // height must be an exact power of 2
  ///////////////////////////////////////////////////////////////////////////
  if ((1 << colPower) != height) {
    return EINVAL;
  }
    
  if (result == 0) {
    vector float          *pCurrentColumn;
    vector float          vIn1, vIn2, vIn3, vIn4;
    vector float          vOut1, vOut2, vOut3, vOut4;
    vector unsigned char  vMergeHiPairPerm;
    vector unsigned char  vMergeLoPairPerm;
    vector float          *pSplit1, *pSplit2, *pSplit3;
    float                 *pFFT1, *pFFT2;
    vector float          vTemp1, vTemp2;		
		
    /////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x0 x1 y0 y1
    /////////////////////////////////////////////////////////////////////////
    vMergeHiPairPerm = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5,
				       6, 7, 16, 17, 18, 19, 20, 21, 22, 23);

    /////////////////////////////////////////////////////////////////////////
    // initialize a permute vector that, given input vectors:
    //
    //	X = x0 x1 x2 x3
    //	Y = y0 y1 y2 y3
    //
    // will generate
    //	Z = x2 x3 y2 y3
    /////////////////////////////////////////////////////////////////////////
    vMergeLoPairPerm = Altivec_Const16(vector unsigned char, 8, 9, 10, 11, 12,
				       13, 14, 15, 24, 25, 26, 27, 28, 29, 30, 31);

    /////////////////////////////////////////////////////////////////////////
    // point at top of current columns in matrix
    /////////////////////////////////////////////////////////////////////////

    pCurrentColumn = (vector float*)data;		

    /////////////////////////////////////////////////////////////////////////
    // prepare pointers into column buffer to make copies of first two
    // columns of data (which are complex data stored vertically) and
    // third column, (which is complex data stored two-wide).
    /////////////////////////////////////////////////////////////////////////

    pSplit1 = (vector float*)rowBufferPointer;
    pSplit2 = pSplit1 + height / 4;
    pSplit3 = pSplit1 + height / 2;
			
    /////////////////////////////////////////////////////////////////////////
    // loop through all rows (four at a time), making copies of the
    // two narrow columns and one wide column.
    /////////////////////////////////////////////////////////////////////////

    for (j = 0; j < height/4; j++) {
      ///////////////////////////////////////////////////////////////////////
      // read in four rows of data from the columns.
      ///////////////////////////////////////////////////////////////////////
      vIn1 = *pCurrentColumn;
      pCurrentColumn += width/4;
      vIn2 = *pCurrentColumn;
      pCurrentColumn += width/4;
      vIn3 = *pCurrentColumn;
      pCurrentColumn += width/4;
      vIn4 = *pCurrentColumn;
      pCurrentColumn += width/4;
			
      ///////////////////////////////////////////////////////////////////////
      // permute input data so that we have one vector that contains
      // all data of first column, one vector that contains all data
      // of second column, and two vectors that contain all data of
      // third column.
      ///////////////////////////////////////////////////////////////////////

      vTemp1 = vec_mergeh(vIn1, vIn2);
      vTemp2 = vec_mergeh(vIn3, vIn4);
			
      vOut1 = vec_perm(vTemp1, vTemp2, vMergeHiPairPerm);
      vOut2 = vec_perm(vTemp1, vTemp2, vMergeLoPairPerm);
      vOut3 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
      vOut4 = vec_perm(vIn3, vIn4, vMergeLoPairPerm);

      ///////////////////////////////////////////////////////////////////////
      // store our column data vectors to the appropriate buffers
      ///////////////////////////////////////////////////////////////////////
						
      *pSplit1++ = vOut1;
      *pSplit2++ = vOut2;
      *pSplit3++ = vOut3;
      *pSplit3++ = vOut4;
    }

    ///////////////////////////////////////////////////////////////////////
    // perform real inverse FFTs on first two columns.
    ///////////////////////////////////////////////////////////////////////

    pFFT1 = (float *)rowBufferPointer;
    pFFT2 = pFFT1 + height;
		
    result = FFTRealInverse(pFFT1, height);
    if (result == 0) {
      result = FFTRealInverse(pFFT2, height);
    }

    ///////////////////////////////////////////////////////////////////////
    // perform complex inverse FFT on first complex column
    ///////////////////////////////////////////////////////////////////////
    pFFT2 = pFFT1 + 2*height;

    if (result == 0) {
      result = FFTComplex(pFFT2, height, 1);
    }
		
    if (result == 0) {	
      /////////////////////////////////////////////////////////////////////
      // point at beginning of copy buffers to prepare to copy data
      // back into columns of matrix after having performed inverse
      // FFTs.
      /////////////////////////////////////////////////////////////////////
      pSplit1 = (vector float*)rowBufferPointer;
      pSplit2 = pSplit1 + height / 4;
      pSplit3 = pSplit1 + height / 2;

      /////////////////////////////////////////////////////////////////////
      // point to top of columns that we will be copying back into
      /////////////////////////////////////////////////////////////////////
      pCurrentColumn = (vector float*)data;

      /////////////////////////////////////////////////////////////////////
      // loop through all rows (four at a time), copying data from
      // temporary buffer back into appropriate row positions in the
      // current columns
      /////////////////////////////////////////////////////////////////////
      for (j = 0; j < height/4; j++) {
	///////////////////////////////////////////////////////////////////
	// get one vector of real data from first column
	///////////////////////////////////////////////////////////////////
			
	vIn1 = *pSplit1++;

	///////////////////////////////////////////////////////////////////
	// get one vector of real data from second column
	///////////////////////////////////////////////////////////////////

	vIn2 = *pSplit2++;

	///////////////////////////////////////////////////////////////////
	// get two vectors of real data from second column
	///////////////////////////////////////////////////////////////////

	vIn3 = *pSplit3++;
	vIn4 = *pSplit3++;
			
	///////////////////////////////////////////////////////////////////
	// permute all data back into columnwise form to be stored as
	// vectors in the original matrix
	///////////////////////////////////////////////////////////////////

	vTemp1 = vec_mergeh(vIn1, vIn2);	
	vTemp2 = vec_mergel(vIn1, vIn2);	
				
	vOut1 = vec_perm(vTemp1, vIn3, vMergeHiPairPerm);
	vOut2 = vec_perm(vTemp1, vIn3, vMergeLoPairPerm);
	vOut3 = vec_perm(vTemp2, vIn4, vMergeHiPairPerm);
	vOut4 = vec_perm(vTemp2, vIn4, vMergeLoPairPerm);
								
	///////////////////////////////////////////////////////////////////
	// store four vectors of data back into the current columns
	///////////////////////////////////////////////////////////////////

	*pCurrentColumn = vOut1;
	pCurrentColumn += width / 4;
	*pCurrentColumn = vOut2;
	pCurrentColumn += width / 4;
	*pCurrentColumn = vOut3;
	pCurrentColumn += width / 4;
	*pCurrentColumn = vOut4;
	pCurrentColumn += width / 4;
      }
										
      /////////////////////////////////////////////////////////////////////
      // we now loop through all remaining columns, two columns at a
      //  time, and perform column ferrying inverse FFTs on the
      //  columns.  We first copy the columns to a separate buffer,
      //  then perform the FFTs, and finally copy them back to their
      //  original column positions.
      /////////////////////////////////////////////////////////////////////
      for (i = 1; i < width/4; i++) {
	///////////////////////////////////////////////////////////////////
	// point at top of current columns
	///////////////////////////////////////////////////////////////////

	pCurrentColumn = ((vector float*)data) + i;		

	///////////////////////////////////////////////////////////////////
	// initialize pointers in temporary buffer for storing copied
	// columns.
	///////////////////////////////////////////////////////////////////

	pSplit1 = (vector float*)rowBufferPointer;
	pSplit2 = pSplit1 + height / 2;
					
	///////////////////////////////////////////////////////////////////
	// loop through all of the column entries that are stored in
	// the temp buffer, and merge them to be stored back into the
	// columns of the matrix.
	///////////////////////////////////////////////////////////////////

	for (j = 0; j < height/2; j++) {
	  /////////////////////////////////////////////////////////////////
	  // read in two rows worth of vectors (two rows X two columns)
	  /////////////////////////////////////////////////////////////////
	  vIn1 = *pCurrentColumn;
	  pCurrentColumn += width/4;
	  vIn2 = *pCurrentColumn;
	  pCurrentColumn += width/4;
					
	  /////////////////////////////////////////////////////////////////
	  // permute the vectors so that the two left column entries
	  // are in one vector and the two right column entries are in
	  // one vector
	  /////////////////////////////////////////////////////////////////
	  vOut1 = vec_perm(vIn1, vIn2, vMergeHiPairPerm);
	  vOut2 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
					
	  /////////////////////////////////////////////////////////////////
	  // store the split columns to the appropriate buffers
	  /////////////////////////////////////////////////////////////////
	  *pSplit1++ = vOut1;
	  *pSplit2++ = vOut2;
	}

	///////////////////////////////////////////////////////////////////
	// perform inverse FFTs on the two columns of data that we
	// have copied to the temp buffer
	///////////////////////////////////////////////////////////////////

	pFFT1 = (float *)rowBufferPointer;
	pFFT2 = pFFT1 + 2*height;
				
	result = FFTComplex(pFFT1, height, 1);
	if (result != 0) break;

	result = FFTComplex(pFFT2, height, 1);
	if (result != 0) break;
									
	///////////////////////////////////////////////////////////////////
	// point at the beginning of the two copied column buffers,
	// and at the top of the columns in the matrix where we will
	// store them back.
	///////////////////////////////////////////////////////////////////

	pSplit1 = (vector float*)rowBufferPointer;
	pSplit2 = pSplit1 + height / 2;
	pCurrentColumn = ((vector float*)data) + i;

	///////////////////////////////////////////////////////////////////
	// loop through all of the column entries that are stored in
	// the temp buffer, and merge them to be stored back into the
	// columns of the matrix.
	///////////////////////////////////////////////////////////////////

	for (j = 0; j < height/2; j++) {
	  /////////////////////////////////////////////////////////////////
	  // get two vectors of column data
	  /////////////////////////////////////////////////////////////////

	  vIn1 = *pSplit1++;
	  vIn2 = *pSplit2++;
					
	  /////////////////////////////////////////////////////////////////
	  // turn them into row vectors
	  /////////////////////////////////////////////////////////////////

	  vOut1 = vec_perm(vIn1, vIn2, vMergeHiPairPerm);
	  vOut2 = vec_perm(vIn1, vIn2, vMergeLoPairPerm);
									
	  /////////////////////////////////////////////////////////////////
	  // store row vectors back into the matrix
	  /////////////////////////////////////////////////////////////////

	  *pCurrentColumn = vOut1;
	  pCurrentColumn += width / 4;
	  *pCurrentColumn = vOut2;
	  pCurrentColumn += width / 4;
	}		
      }

      if (result == 0) {
	///////////////////////////////////////////////////////////////////
	// perform an inverse real FFT on every row of the matrix
	///////////////////////////////////////////////////////////////////
			
	pRow = data;
				
	for (i = 0; i < height; i++) {
	  result = FFTRealInverse(pRow+i*(width), width);
	  if (result != 0) break;
	}
      }
    }				
  }

  return result;
}

#pragma mark -
#pragma mark 2 D   C O N V O L U T I O N

/////////////////////////////////////////////////////////////////////////////
//	ConvolveComplexAltivec2D
//
//	calculates the 2D convolution of signal1 and signal2.  This is
// done by performing an FFT on signal1 and signal2, calculating the
// dyadic product of the two results, and then performing an inverse
// FFT on the result of the dyadic mul.
//
//	signal2 is replaced by the convolution of signal1 and
// signal2. signal1 is replaced by the FFT of signal1.
//
/////////////////////////////////////////////////////////////////////////////
int32 ConvolveComplexAltivec2D(float *pSignal1, float *pSignal2,
				 int32 width, int32 height)
{	
  int32 result = 0;
	
  ///////////////////////////////////////////////////////////////////////////
  // perform 2D FFT on signal 1
  ///////////////////////////////////////////////////////////////////////////

  result = FFT2DComplex(pSignal1, width, height, -1);
	
  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////
    // perform 2D FFT on signal 1
    /////////////////////////////////////////////////////////////////////////
		
    result = FFT2DComplex(pSignal2, width, height, -1);
		
    if (result == 0) {
      ///////////////////////////////////////////////////////////////////////
      // perform dyadic mul on two signals.  The fact that they are 2D
      // is irrelevant for the sake of the dyadic mul, so we call the
      // standard dyadic mul for the entire width*length signal.
      ///////////////////////////////////////////////////////////////////////

      mul_dyadic_complex_altivec(pSignal1, pSignal2, width*height);	
    }
	
    /////////////////////////////////////////////////////////////////////////
    // perform inverse 2D fft on result in signal2, producing the
    // convolution of signal1 and signal2
    /////////////////////////////////////////////////////////////////////////

    result = FFT2DComplex(pSignal2, width, height, 1);
  }
	
  return result;
}


////////////////////////////////////////////////////////////////////////////////
//	mul_dyadic_2D_hermitian_altivec
//
// Performs a dyadic multiply on the 2D hermitian-order data from a
// real 2D FFT.  The data for both signals is expected to be in the
// following 2D hermitian order:
//
// X = 
//		
// Xr(0,0)   Xr(0, W/2)  Xr(0,1) Xi(0,1) Xr(0,2) Xi(0,2)...Xr(0,W/2-1) Xi(0,W/2-1)
// Xr(H/2,0) Xr(H/2,W/2) Xr(1,1) Xi(1,1) Xr(1,2) Xi(1,2)...Xr(1,W/2-1) Xi(1,W/2-1)
// Xr(1,0)   Xr(1,W/2)   Xr(2,1) Xi(2,1) Xr(2,2) Xi(2,2)...Xr(2,W/2-1) Xi(2,W/2-1)
// Xi(1,0)   Xi(1,W/2)   Xr(3,1) Xi(3,1) Xr(3,2) Xi(3,2)...Xr(3,W/2-1) Xi(3,W/2-1)
//	.
//	.
//	.
// Xr(H/2-1,0) Xr(H/2-1,W/2) Xr(H-2,1) Xi(H-2,1)  ...  Xr(H-2,W/2-1) Xi(H-2,W/2-1)
// Xi(H/2-1,0) Xi(H/2-1,W/2) Xr(H-1,1) Xi(H-1,1)  ...  Xr(H-1,W/2-1) Xi(H-1,W/2-1)
//
//	Signal2 is replaced by Signal2 X Signal1.  Signal1 is unmodified.
//
// **NOTE** that implementation details demand that width >= 8 and height >=4
//	
/////////////////////////////////////////////////////////////////////////////
static void mul_dyadic_2D_hermitian_altivec(float *pSignal1, float *pSignal2,
					    int32 width, int32 height)
{
  vector unsigned char  vSwappedPerm;
  vector float          vSignal1InTop, vSignal2InTop;
  vector float          vSignal1InBottom, vSignal2InBottom;
  vector float          *pSignal1Top, *pSignal2Top;
  vector float          *pSignal1Bottom, *pSignal2Bottom;
  vector float          vSignal2OutTop, vSignal2OutBottom;
  int32               i, j;	
  vector float          vMul1, vMul2, vMul3, vMul4, vMul5, vMul6;
  vector unsigned char  vRealsPerm;
  vector unsigned char  vImsPerm;
  vector unsigned char  vSwappedNegatePerm;
  vector float          vNegSignal2Top, vNegSignal2Bottom;
  vector float          vZero = Altivec_Const4(vector float, 0, 0, 0, 0);
  vector unsigned char  vMulPerm1, vMulPerm2, vMulPerm3, vMulPerm4, vMulPerm6;
  vector float          vNegSignalMultiplier;
  float                 real1, real2, im1, im2, tempreal, tempim;

  ///////////////////////////////////////////////////////////////////////////
  // initialize a multiplier vector that negates elements 1, 2, and 4
  // in a float vector
  ///////////////////////////////////////////////////////////////////////////
  vNegSignalMultiplier = Altivec_Const4(vector float, -1, -1, 1, -1);
	
  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x1 x2 x2
  ///////////////////////////////////////////////////////////////////////////
  vMulPerm1  = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5, 6, 7,
			       8, 9, 10, 11, 8, 9, 10, 11);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x1 y3 y3
  ///////////////////////////////////////////////////////////////////////////
  vMulPerm2  = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5, 6, 7,
			       28, 29, 30, 31, 28, 29, 30, 31);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x1 y3 y2
  ///////////////////////////////////////////////////////////////////////////
  vMulPerm3  = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5, 6, 7,
			       28, 29, 30, 31, 24, 25, 26, 27);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x1 y2 y2
  ///////////////////////////////////////////////////////////////////////////
  vMulPerm4  = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5, 6, 7,
			       24, 25, 26, 27, 24, 25, 26, 27);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x1 x3 x3
  ///////////////////////////////////////////////////////////////////////////
  vMulPerm6  = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 4, 5, 6, 7,
			       12, 13, 14, 15, 12, 13, 14, 15);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x1 x0 x3 y2
  ///////////////////////////////////////////////////////////////////////////
  vSwappedPerm  = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 0, 1, 2,
				  3, 12, 13, 14, 15, 8, 9, 10, 11);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x0 x0 x2 x2
  ///////////////////////////////////////////////////////////////////////////
  vRealsPerm  = Altivec_Const16(vector unsigned char, 0, 1, 2, 3, 0, 1, 2,
				3, 8, 9, 10, 11, 8, 9, 10, 11);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = x1 x1 x3 x3
  ///////////////////////////////////////////////////////////////////////////
  vImsPerm  = Altivec_Const16(vector unsigned char, 4, 5, 6, 7, 4, 5, 6, 7,
			      12, 13, 14, 15, 12, 13, 14, 15);

  ///////////////////////////////////////////////////////////////////////////
  // initialize a permute vector that, given input vectors:
  //
  //	X = x0 x1 x2 x3
  //	Y = y0 y1 y2 y3
  //
  // will generate
  //	Z = y1 x0 y3 x2
  ///////////////////////////////////////////////////////////////////////////
  vSwappedNegatePerm  = Altivec_Const16(vector unsigned char, 20, 21, 22, 23, 0,
					1, 2, 3, 28, 29, 30, 31, 8, 9, 10, 11);

  ///////////////////////////////////////////////////////////////////////////
  // handle the upper left quad, which are real values (unlike the
  // rest of the 2D array, which is all complex data).
  ///////////////////////////////////////////////////////////////////////////
	
  pSignal2[0] *= pSignal1[0];
  pSignal2[1] *= pSignal1[1];
  pSignal2[width] *= pSignal1[width];
  pSignal2[width+1] *= pSignal1[width+1];

  ///////////////////////////////////////////////////////////////////////////
  // calculate first complex of first and second rows, since these
  // elements are not vector aligned.
  ///////////////////////////////////////////////////////////////////////////
	
  real1       = pSignal1[2];
  real2       = pSignal2[2];
  im1         = pSignal1[3];
  im2         = pSignal2[3];
	
  tempreal    = (real1*real2) - (im1*im2);
  tempim      = (real1*im2) + (real2*im1);
	
  pSignal2[2] = tempreal;
  pSignal2[3] = tempim;

  real1       = pSignal1[width+2];
  real2       = pSignal2[width+2];
  im1         = pSignal1[width+3];
  im2         = pSignal2[width+3];
	
  tempreal    = (real1*real2) - (im1*im2);
  tempim      = (real1*im2) + (real2*im1);
	
  pSignal2[width+2] = tempreal;
  pSignal2[width+3] = tempim;

  ///////////////////////////////////////////////////////////////////////////
  // do all of leftmost three columns below upper-left quad.  All
  // columns are complex, but the first two are single-width, with
  // alternating real and complex values on each row.  The third
  // column (and all others after it) is double- width, so that each
  // row contains a real and imaginary value (so the row is two floats
  // wide).  We have to permute these vectors to create the
  // appropriate multiplicand vectors for mul-add operations.
  //
  //	as we read them in, the vectors are in the following format:
  //
  //	vSignal1InTop = 	(X1r X2r X3r X3i)
  //	vSignal1InBottom = 	(X1i X2i X4r X4i)
  //
  //	vSignal2InTop = 	(Y1r Y2r Y3r Y3i)
  //	vSignal2InBottom = 	(Y1i Y2i Y4r Y4i)
  //
  //
  //	We need to generate output vectors in the form:
  //
  //	vSignal2OutTop = 	(X1r*Y1r - X1i*Y1i, X2r*Y2r - X2i*Y2i,
  //                             X3r*Y3r - X3i*Y3i, X3r*Y3i + X3i * Y3r);
  //	vSignal2OutBottom =	(X1r*Y1i + X1i*Y1r, X2r*Y2i + X2i*Y2r,
  //                             X4r*Y4r - X4i*Y4i, X4r*Y4i + X4i * Y4r);
  //
  ///////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////
  // point to top two rows of signal 1
  ///////////////////////////////////////////////////////////////////////////
  pSignal1Top = (vector float*)pSignal1+width/2;
  pSignal1Bottom = pSignal1Top + width/4;

  ///////////////////////////////////////////////////////////////////////////
  // point to top two rows of signal 2
  ///////////////////////////////////////////////////////////////////////////
  pSignal2Top = (vector float*)pSignal2 + width/2;
  pSignal2Bottom = pSignal2Top + width/4;

  ///////////////////////////////////////////////////////////////////////////
  // loop through all rows, two rows at a time
  ///////////////////////////////////////////////////////////////////////////

  for (i = 1; i < height/2; i++) {
    /////////////////////////////////////////////////////////////////////////
    // load two vectors from next two rows of signal 1
    /////////////////////////////////////////////////////////////////////////

    vSignal1InTop = *pSignal1Top;
    vSignal1InBottom = *pSignal1Bottom;

    /////////////////////////////////////////////////////////////////////////
    // load two vectors from next two rows of signal 2
    /////////////////////////////////////////////////////////////////////////

    vSignal2InTop = *pSignal2Top;
    vSignal2InBottom = *pSignal2Bottom;
		
    /////////////////////////////////////////////////////////////////////////
    // make negative copies of signal 2 vectors, so that we can use
    // them as permute arguments to create multiplicand vectors that
    // can be used in subsequent mul-add vector ops with the correct
    // signs
    /////////////////////////////////////////////////////////////////////////
		
    vNegSignal2Top = vec_madd(vSignal2InTop, vNegSignalMultiplier, vZero);
    vNegSignal2Bottom = vec_madd(vSignal2InBottom, vNegSignalMultiplier, vZero);

    /////////////////////////////////////////////////////////////////////////
    // 	create multiplicand vectors for mul-add operations
    /////////////////////////////////////////////////////////////////////////

    vMul1 = vec_perm(vSignal1InTop, vSignal1InTop, vMulPerm1);
    vMul2 = vec_perm(vSignal1InBottom, vSignal1InTop, vMulPerm2);
    vMul3 = vec_perm(vNegSignal2Bottom, vNegSignal2Top, vMulPerm3);

    vMul4 = vec_perm(vSignal1InTop, vSignal1InBottom, vMulPerm4);
    vMul5 = vec_perm(vSignal2InTop, vNegSignal2Bottom, vMulPerm3);
    vMul6 = vec_perm(vSignal1InBottom, vSignal2InBottom, vMulPerm6);

    /////////////////////////////////////////////////////////////////////////
    //	calculate dyadic product of two sets of vectors
    /////////////////////////////////////////////////////////////////////////

    vSignal2OutTop = vec_madd(vMul1, vSignal2InTop, vZero);
    vSignal2OutTop = vec_madd(vMul2, vMul3, vSignal2OutTop);

    vSignal2OutBottom = vec_madd(vMul4, vSignal2InBottom, vZero);
    vSignal2OutBottom = vec_madd(vMul5, vMul6, vSignal2OutBottom);
		
    /////////////////////////////////////////////////////////////////////////
    // store dyadic product results to current location in signal2
    /////////////////////////////////////////////////////////////////////////

    *pSignal2Top = vSignal2OutTop;
    *pSignal2Bottom = vSignal2OutBottom;

    /////////////////////////////////////////////////////////////////////////
    // advance pointers to next two rows of signal1 and signal2
    /////////////////////////////////////////////////////////////////////////

    pSignal1Top	+= width/2;
    pSignal1Bottom	+= width/2;
    pSignal2Top	+= width/2;
    pSignal2Bottom	+= width/2;
  }

  ///////////////////////////////////////////////////////////////////////////
  // calculate dyadic product for remaining columns.  Remaining
  // columns are two- wide (real an imaginary floats adjacent in
  // memory).
  ///////////////////////////////////////////////////////////////////////////
	
  for (j = 1; j < width/4; j++) {  
    /////////////////////////////////////////////////////////////////////////
    // point to top of current columns in signal1 and signal2
    /////////////////////////////////////////////////////////////////////////
		
    pSignal1Top = ((vector float*)pSignal1) + j;
    pSignal2Top = ((vector float*)pSignal2) + j;

    /////////////////////////////////////////////////////////////////////////
    // loop through rows, calculating dyadic product of elements in
    // current columns
    /////////////////////////////////////////////////////////////////////////

    for (i = 0; i < height; i++) {
      ///////////////////////////////////////////////////////////////////////
      // load two complex entries from signal1
      ///////////////////////////////////////////////////////////////////////

      vSignal1InTop = *pSignal1Top;

      ///////////////////////////////////////////////////////////////////////
      // load two complex entries from signal2
      ///////////////////////////////////////////////////////////////////////

      vSignal2InTop = *pSignal2Top;
					
      ///////////////////////////////////////////////////////////////////////
      // negate signal 2, so we have negative values necessary for our
      // multiplies
      ///////////////////////////////////////////////////////////////////////

      vNegSignal2Top = vec_sub(vZero, vSignal2InTop);
			
      ///////////////////////////////////////////////////////////////////////
      // set up multiplicand vectors.  Since we are multiplying
      // complex numbers, we need to do more than just call a multiply
      // routine.  Given two complex numbers x = (xr, xi) and y = (yr,
      // yi), then the product z = (zr, zi) = x*y is defined as:
      // 
      //	zr = xr*yr - xi*yi
      //	zi = xr*yi + xi*yr
      //
      // to perform this multiplication, we generate four multiplicand
      // vectors. If we consider our input vectors X and Y, they each
      // contain two complex numbers:
      //
      //	X = (x1r, x1i, x2r, x2i)
      //	Y = (y1r, y1i, y2r, y2i)
      //
      // then we want to generate a new output Z vector:
      //
      //	Z = (x1r*y1r - x1i*y1i, x1r*y1i + x1i*y1r,
      //             x2r*y2r - x2i*y2i, x2r*y2i + x2i*y2r)
      //
      //	So, we need to have multiplicand vectors:
      //
      //	M1 = 	x1r	x1r	x2r	x2r
      //	M2 = 	x1i	x1i	x2i 	x2i
      //	M3 =	-y1i	y1r	-y2i	y2r
      //	M4 = 	y1r	y1i	y2r	y2i
      //
      //	We can then generate our Z vector with a vector mul
      //	and a vector mul-add.
      ///////////////////////////////////////////////////////////////////////
      vMul1 = vec_perm(vSignal1InTop, vSignal1InTop, vRealsPerm);
      vMul2 = vec_perm(vSignal1InTop, vSignal1InTop, vImsPerm);
      vMul3 = vec_perm(vSignal2InTop, vNegSignal2Top, vSwappedNegatePerm);
			
      vSignal2OutTop = vec_madd(vMul1, vSignal2InTop, vZero);
      vSignal2OutTop = vec_madd(vMul2, vMul3, vSignal2OutTop);
			
      ///////////////////////////////////////////////////////////////////////
      // store product vector back to current position in signal 2
      ///////////////////////////////////////////////////////////////////////

      *pSignal2Top = vSignal2OutTop;
			
      ///////////////////////////////////////////////////////////////////////
      // advance pointers to next rows
      ///////////////////////////////////////////////////////////////////////
			
      pSignal1Top += width/4;
      pSignal2Top += width/4;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
//	ConvolveRealAltivec2D
//
//	calculates the convolution of signal1 and signal2, both of
// which are 2D signals that are width*height in size.
//
// This is done by performing a forward 2D FFT on both signals, then
// calculating the dyadic product of the two resulting signals.
// Finally, an inverse 2D FFT is performed on the result of the dyadic
// mul.
//
//	signal2 is replaced by the convolution of signal1 and
// signal2. signal1 is replaced by the 2D FFT of signal1.
//
// **NOTE** that implementation details demand that width >= 8 and height >=4
//
/////////////////////////////////////////////////////////////////////////////
int32 ConvolveRealAltivec2D(float *pSignal1, float *pSignal2,
			      int32 width, int32 height)
{
  int32 result = 0;
	
  ///////////////////////////////////////////////////////////////////////////
  // because of implementation details, we have these restrictions
  ///////////////////////////////////////////////////////////////////////////
  if ((width < 8) || (height < 4)) {
    return EINVAL;
  }

  ///////////////////////////////////////////////////////////////////////////
  // perform 2D forward FFt on signal1
  ///////////////////////////////////////////////////////////////////////////
		
  result = FFT2DRealForward(pSignal1, width, height);	
  if (result == 0) {
    /////////////////////////////////////////////////////////////////////////
    // perform 2D forward FFt on signal2
    /////////////////////////////////////////////////////////////////////////
	
    result = FFT2DRealForward(pSignal2, width, height);	
		
    if (result == 0) {
      /////////////////////////////////////////////////////////////////////////
      // perform 2D forward FFT on signal2
      /////////////////////////////////////////////////////////////////////////
			
      mul_dyadic_2D_hermitian_altivec(pSignal1, pSignal2, width, height);
			
      result = FFT2DRealInverse(pSignal2, width, height);	
    }
  } 

  return result;
}

#endif
