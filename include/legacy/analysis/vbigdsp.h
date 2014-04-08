/*
	File:		vBigDSP.h

	Contains:       AltiVec-based Implementation of DSP routines (real &
	                complex FFT, convolution)

	Version:	1.0

	Copyright:	1999 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

			10/12/99	JK		Created

*/

/////////////////////////////////////////////////////////////////////////////////
//      File Name: vBigDSP.h                                                   //
//	 								       //
//	This library provides a set of DSP routines, implemented using the     //
//	AltiVec instruction set.					       //
//									       //
//                                                                             //
//      Copyright  1999 Apple Computer, Inc.  All rights reserved.             //
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
   Apple software (the "Apple Software"), to use, reproduce, modify
   and redistribute the Apple Software, with or without modifications,
   in source and/or binary forms; provided that if you redistribute
   the Apple Software in its entirety and without modifications, you
   must retain this notice and the following text and disclaimers in
   all such redistributions of the Apple Software.  Neither the name,
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
   APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION
   WITH YOUR PRODUCTS.

   IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT,
   INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
   USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE
   SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
   (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF
   APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <rc_types.h>

//////////////////////////////////////////////////////////
// constants for foward/inverse flags for complex FFT	//
//////////////////////////////////////////////////////////
#define	IFLAG_FFT_FORWARD	(-1)
#define	IFLAG_FFT_INVERSE	(1)

//////////////////////////////////////////////////////////
//	length above which we no longer use 		//
//	the ping pong FFT (for speed reasons)		//
//////////////////////////////////////////////////////////
#define PINGPONG_FFT_MAXLEN	(1<<15)

//////////////////////////////////////////////////////////////
//	minimum length for calling AltiVec implementation   //
// 	of ping pong FFT	(because of implementation  //	
//	details, algorithm will not work on lengths	    //
// below this).						    //
//////////////////////////////////////////////////////////////
#define ALTIVEC_COMPLEX_MIN_LENGTH 16

//////////////////////////////////////////////////////////////
//	minimum length for calling AltiVec implementation   //
// 	of real FFT	(because of implementation          //	
//	details, algorithm will not work on lengths         //
// below this).						    //
//////////////////////////////////////////////////////////////
#define ALTIVEC_REAL_MIN_LENGTH	   32

//////////////////////////////////////////////////////////
// minimum length at which the columnwise FFT is faster	//
// than the standard ping pong FFT.                     //
//////////////////////////////////////////////////////////
#define ALTIVEC_COLUMNWISE_FFT_BREAKOVER (1<<15)


//////////////////////////////////////////////////////////
//	ShutdownFFT					//
//							//
// deallocate any temporary buffer and sin/cos table	//
// lookup buffers used by FFT code			//
//////////////////////////////////////////////////////////
void ShutdownFFT();

//////////////////////////////////////////////////////////
//	FFTComplex					//
//							//
//	forward/inverse FFT for complex signals		//
//							//
//	data	address of signal (16-byte aligned)	//
//	length	number of complex elements in signal    //
//	isign	one of 	IFLAG_FFT_FORWARD		//
//			IFLAG_FFT_INVERSE		//
//		for forward or inverse FFT		//
//////////////////////////////////////////////////////////
int32 FFTComplex(float *data, int32 length, int32 isign);

//////////////////////////////////////////////////////////
// FFTRealForward					//
//							//
// forward FFT for real signal				//
//							//
//	data	address of signal (16-byte aligned)	// 
//	len	number of complex elements in signal    //
//////////////////////////////////////////////////////////
int32 FFTRealForward(float *data, int32 len);

//////////////////////////////////////////////////////////
// FFTRealInverse					//
//							//
// inverse FFT for real signal				//
//							//
//	data	address of signal (16-byte aligned)	// 
//	len	number of complex elements in signal    //
//////////////////////////////////////////////////////////
int32 FFTRealInverse(float *data, int32 len);


//////////////////////////////////////////////////////////
// ConvolveComplexAltivec				//
//							//
// calculates convolution of two complex signals	//
//							//
//	x	address of first complex signal for	// 
//		complex convolution (is replaced 	// 
//		with FFT(x)	(16-byte aligned)	//
//	y	address of first complex signal for	// 
//		complex convolution (is replaced 	// 
//		with convolution of x, y 		//
//		(16-byte aligned)			//
//	n	number of complex elems in signals	//
//////////////////////////////////////////////////////////
int32 ConvolveComplexAltivec(float *x, float *y, int32 n);

//////////////////////////////////////////////////////////
// ConvolveRealAltivec					//
//							//
// calculates convolution of two real signals		//
//							//
//	x	address of first real signal for	// 
//		real convolution (is replaced	 	// 
//		with FFT(x) (16-byte aligned)		//
//	y	address of first real signal for	// 
//		real convolution (is replaced	 	// 
//		with convolution of x, y		//
//		(16-byte aligned)			//
//	n	number of real elems in signals		//
//////////////////////////////////////////////////////////
int32 ConvolveRealAltivec(float *x, float *y, int32 n);

//////////////////////////////////////////////////////////
// FFT2DComplex						//
//							//
// forward/inverse FFT for 2D complex signal		//
//							//
//	data	address of signal (16-byte aligned)	// 
//	width	width of 2D array of complex signal	//
//	height	height of 2D array of complex signal    //
//	isign	one of 	IFLAG_FFT_FORWARD		//
//		IFLAG_FFT_INVERSE			//
//		for forward or inverse FFT		//
//////////////////////////////////////////////////////////
int32 FFT2DComplex(float *data, int32 width,
		     int32 height, int32 iflag);

//////////////////////////////////////////////////////////
// FFT2DRealForward					//
//							//
// forward FFT for 2D real signal			//
//							//
//	data	address of signal (16-byte aligned)	// 
//	width	width of 2D array of real signal	//
//	height	height of 2D array of real signal	//
//////////////////////////////////////////////////////////
int32 FFT2DRealForward(float *data, int32 width, int32 height);

//////////////////////////////////////////////////////////
// FFT2DRealInverse					//
//							//
// forward FFT for 2D real signal			//
//							//
//	data	address of signal (16-byte aligned)	// 
//	width	width of 2D array of real signal	//
//	height	height of 2D array of real signal	//
//////////////////////////////////////////////////////////
int32 FFT2DRealInverse(float *data, int32 width, int32 height);

//////////////////////////////////////////////////////////
// ConvolveComplexAltivec2D				//
//							//
// 2D convolution for two complex signals		//
//							//
//	x	address of first complex signal for	// 
//		complex convolution (is replaced	// 
//		with FFT(x) (16-byte aligned)		//
//	y	address of first complex signal for	// 
//		complex convolution (is replaced	// 
//		with convolution of x, y		//
//		(16-byte aligned)			//
//	width	width of 2D array of complex signal	//
//	height	height of 2D array of complex signal    //
//////////////////////////////////////////////////////////
int32 ConvolveComplexAltivec2D(float *x, float *y,
				 int32 width, int32 height);

//////////////////////////////////////////////////////////
// ConvolveRealAltivec2D				//
//							//
// 2D convolution for two real signals			//
//							//
//	x	address of first real signal for	// 
//		real convolution (is replaced		// 
//		with FFT(x) (16-byte aligned)		//
//	y	address of first real signal for	// 
//		real convolution (is replaced		// 
//		with convolution of x, y		//
//		(16-byte aligned)			//
//	width	width of 2D array of real signal	//
//	height	height of 2D array of real signal	//
//////////////////////////////////////////////////////////
int32 ConvolveRealAltivec2D(float *x, float *y,
			      int32 width, int32 height);