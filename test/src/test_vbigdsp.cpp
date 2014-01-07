/*
  Disclaimer: IMPORTANT: This Apple software is supplied to you by
  Apple Computer, Inc.  ("Apple") in consideration of your agreement
  to the following terms, and your use, installation, modification or
  redistribution of this Apple software constitutes acceptance of
  these terms.  If you do not agree with these terms, please do not
  use, install, modify or redistribute this Apple software.

  In consideration of your agreement to abide by the following terms,
  and subject to these terms, Apple grants you a personal,
  non-exclusive license, under Apple’s copyrights in this original
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

#ifdef __ppc__

#include <math.h>
#include <stdio.h>

#include "vbigdsp.h"

#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif


#pragma mark -

/* Calculates the root-mean-squared error between two real signals
 * x[n] and y[n].
 */
static double RMSE_real(float *x, float *y, int n)
{
  double t, err=0.0;
  int    i;
	
  for (i=0; i<n; i++) {
    t = x[i] - y[i];
    err += (t * t);
  }
	
  return(sqrt(err/n));
}


/* Calculates the root-mean-squared error between two complex signals
 * x[n] and y[n].
 */
static double RMSE_complex(float *x, float *y, int n)
{
  double t, err=0.0;
  int    i;
	
  for (i=0; i<n; i++) {
    t = x[2*i] - y[2*i];
    err += t * t;
    t = x[2*i+1] - y[2*i+1];
    err += t * t;
  }
	
  return(sqrt(err/n));
}

#pragma mark -

/* Performs a literal cyclic convolution on x and y, placing the
 * result in z. This is an O(n^2) algorithm useful
 * for accuracy testing.
 */
static void ConvolveRealLiteral(float *x, float *y, float *z, int n)
{
  int s, i, q;

  for (s = 0; s < n; s++) {
    z[s] = 0;
    for (q = 0; q < n; q++) {
      i = (s-q)%n;
      if(i<0) i+= n;
      z[s] += y[i] * x[q];
    }
  }
}

/* Performs a literal cyclic convolution on x and y, placing the
 * result in z. This is an O(n^2) algorithm useful
 * for accuracy testing.
 */
static void ConvolveComplexLiteral(float *x, float *y, float *z, int n)
{
  int s, i, q;

  for (s = 0; s < n; s++) {
    z[2*s] = z[2*s+1] = 0;
    for (q = 0; q < n; q++) {
      i = (s-q)%n;
      if(i<0) i+= n;
      z[2*s] += y[2*i] * x[2*q] - y[2*i+1] * x[2*q+1];
      z[2*s+1] += y[2*i+1] * x[2*q] + y[2*i] * x[2*q+1];
    }
  }
}

/* Literal cyclic convolution in two dimensions, for testing purposes. The
 * convolution of x[] and y[] is output in z[].
 */
static void Convolve2DRealLiteral(float *x, float *y, float *z,	int w, int h)
{
  int k, j, q, p, kk, jj;
	
  for (k=0; k<h; k++) {
    for (j=0; j<w; j++) {
      z[j + k*w] = 0;
      for (q = 0; q < h; q++) {
	kk = (k-q<0)?k-q+h:k-q;
	for (p = 0; p < w; p++) {
	  jj = (j-p<0)?j-p+w:j-p;
	  z[j + k*w] += x[p + q*w] * y[jj + kk*w];
	}
      }
    }
  }
}

#pragma mark -

#define COMPLEX_TEST_MIN_POWER	0
#define COMPLEX_TEST_MAX_POWER	18

static void TestComplexFFT()
{
  float  *real_data;
  float  *vector_data;
  int    i, j;
  long   currentLength;
  int    result;
  double rmse_vector;

  currentLength = 1 << COMPLEX_TEST_MAX_POWER;
  real_data = (float*)valloc(2*currentLength*sizeof(float));
  vector_data = (float*)valloc(2*currentLength*sizeof(float));
  if (real_data == 0 || vector_data == 0) {
    printf("error allocating space for complex test\n");
  } else {

    for (i = COMPLEX_TEST_MIN_POWER; i<=COMPLEX_TEST_MAX_POWER; i++) {
      currentLength = 1 << i;
		
      /////////////////////////////////////////////////
      // initialize signal
      /////////////////////////////////////////////////

      for (j=0; j<currentLength; j++) {
	real_data[2*j] = vector_data[2*j] =
	  cos(PI*j*j/currentLength);
	real_data[2*j+1] = vector_data[2*j+1] =
	  sin(PI*j*j/currentLength);
      }
			
      /////////////////////////////////////////////////
      // perform forward and inverse FFT and compare
      // result with original signal
      /////////////////////////////////////////////////
			
      result = FFTComplex(vector_data, currentLength, -1);
      if (result != 0) {
	printf("error in forward complex FFT\n");
      }

      result = FFTComplex(vector_data, currentLength, 1);
      if (result != 0) {
	printf("error in inverse complex FFT\n");
      }

      rmse_vector = RMSE_complex(real_data, vector_data, currentLength);
			
      printf(" complex length %ld FFT rmse: %g\n", currentLength, rmse_vector);
    }
  }
	
  if (real_data) free(real_data);
  if (vector_data) free(vector_data);
}


#define REAL_TEST_MIN_POWER	0
#define REAL_TEST_MAX_POWER	18
int TestRealFFT()
{
  float  *real_data;
  float  *vector_data;
  int    i, j;
  double negativeOneToI = -1;
  double rmse_vector;
  long   currentLength;
  int    result;
	
  currentLength = 1 << REAL_TEST_MAX_POWER;
  real_data = (float*)valloc(currentLength*sizeof(float));
  vector_data = (float*)valloc(currentLength*sizeof(float));
	
  if (real_data == 0 || vector_data == 0) {
    printf("error allocating space for complex test\n");
  } else {
	
    for (i=REAL_TEST_MIN_POWER; i<= REAL_TEST_MAX_POWER; i++) {
      currentLength = 1 << i;

      /////////////////////////////////////////////////
      // initialize signal
      /////////////////////////////////////////////////

      for (j=0; j<currentLength; j++) {
	negativeOneToI = -negativeOneToI;
				
	real_data[j] = vector_data[j] = 
	  negativeOneToI + sin(3*PI*j/currentLength);
      }

      /////////////////////////////////////////////////
      // perform forward and inverse FFT and compare
      // result with original signal
      /////////////////////////////////////////////////

      result = FFTRealForward(vector_data, currentLength);
      if (result != 0) {
	if (real_data) free(real_data);
	if (vector_data) free(vector_data);
	return result;
      }
			
      result = FFTRealInverse(vector_data, currentLength);
      if (result != 0) {
	if (real_data) free(real_data);
	if (vector_data) free(vector_data);
	return result;
      }

      rmse_vector = RMSE_real(real_data, vector_data, currentLength);

      //printf(" real length %ld FFT rmse: %g\n", currentLength, rmse_vector);
      /* OK, I don't know what is really an acceptable error, so I picked
       * the largest value printed out.
       */
      if (rmse_vector > 4.4262e-06) {
	if (real_data) free(real_data);
	if (vector_data) free(vector_data);
	return -1000000;
      }
    }
  }		

  if (real_data) free(real_data);
  if (vector_data) free(vector_data);

  return 0;
}


#define COMPLEX_CONVOLVE_MIN_POWER	2
#define COMPLEX_CONVOLVE_MAX_POWER	13
static void TestComplexConvolve()
{
  float  *data1;
  float  *data2;
  float  *literalConvolveData;
  int    i, j;
  //  double negativeOneToI = -1;
  double rmse_vector;
  long   currentLength;
  int    result;
	
  currentLength = 1 << COMPLEX_CONVOLVE_MAX_POWER;
  data1 = (float*)valloc(2*currentLength*sizeof(float));
  data2 = (float*)valloc(2*currentLength*sizeof(float));
  literalConvolveData =	(float*)valloc(2*currentLength*sizeof(float));
	
  if (data1 == 0 || data2 == 0 || literalConvolveData  == 0) {
    printf("error allocating space for complex convolve test\n");
  } else {

    for (i=COMPLEX_CONVOLVE_MIN_POWER; i <= COMPLEX_CONVOLVE_MAX_POWER; i++) {
      currentLength = 1 << i;
		
      /////////////////////////////////////////////////
      // initialize signals
      /////////////////////////////////////////////////

      for (j=0; j<currentLength; j++) {
	data1[2*j]   =  cos(PI*j*j/currentLength);
	data1[2*j+1] =  sin(PI*j*j/currentLength);
	data2[2*j]   =  2*sin(3.3*PI*j*j/currentLength);
	data2[2*j+1] =  -cos(1.4*PI*j*j/currentLength);
      }

      /////////////////////////////////////////////////
      // perform literal convolution, and compare
      // with convolution performed through FFT
      /////////////////////////////////////////////////

      ConvolveComplexLiteral(data1, data2, literalConvolveData, currentLength);

      result = ConvolveComplexAltivec(data1, data2, currentLength);
			
      if (result != 0) {
	printf("error in convolve complex\n");
      }

      rmse_vector = RMSE_complex(data2, literalConvolveData, currentLength);

      printf(" complex length %ld convolve rmse: %g\n", currentLength, rmse_vector);
    }
  }
	
  if (data1) free(data1);
  if (data2) free(data2);
  if (literalConvolveData) free(literalConvolveData);
}



#define REAL_CONVOLVE_MIN_POWER	0
#define REAL_CONVOLVE_MAX_POWER	14
static void TestRealConvolve()
{
  float *data1;
  float *data2;
  float *literalConvolveData;
  int    i, j;
  double negativeOneToI = -1;
  double rmse_vector;
  long   currentLength;
  int    result;
	
  currentLength = 1 << REAL_CONVOLVE_MAX_POWER;
  data1 = (float*)valloc(currentLength*sizeof(float));
  data2 = (float*)valloc(currentLength*sizeof(float));
  literalConvolveData =	(float*)valloc(currentLength*sizeof(float));
	
  if (data1 == 0 || data2 == 0 || literalConvolveData  == 0) {
    printf("error allocating space for real convolve test\n");
  } else {

    for (i=REAL_CONVOLVE_MIN_POWER; i <= REAL_CONVOLVE_MAX_POWER; i++) {
      currentLength = 1 << i;
		
      /////////////////////////////////////////////////
      // initialize signals
      /////////////////////////////////////////////////

      for (j=0; j<currentLength; j++) {
	negativeOneToI = -negativeOneToI;
				
	data1[j] = negativeOneToI + sin(3*PI*j/currentLength);
					
	data2[j] = 3 * negativeOneToI + sin(4.4*PI*j/currentLength);
				
      }

      /////////////////////////////////////////////////
      // perform literal convolution, and compare
      // with convolution performed through FFT
      /////////////////////////////////////////////////

      ConvolveRealLiteral(data1, data2, literalConvolveData, currentLength);

      result = ConvolveRealAltivec(data1, data2, currentLength);
			
      if (result != 0) {
	printf("error in convolve real\n");
      }

      rmse_vector = RMSE_real(data2, literalConvolveData, currentLength);

      printf(" real length %ld convolve rmse: %g\n", currentLength, rmse_vector);
    }		
  }
	
  if (data1) free(data1);
  if (data2) free(data2);
  if (literalConvolveData) free(literalConvolveData);
}


#define COMPLEX_FFT2D_MIN_POWER_X	1
#define COMPLEX_FFT2D_MAX_POWER_X	8
#define COMPLEX_FFT2D_MIN_POWER_Y	1
#define COMPLEX_FFT2D_MAX_POWER_Y	8

static void TestFFT2DComplex()
{
  float  *data1;
  float  *data2;
  int    xsize, ysize, j;
  //  double negativeOneToI = -1;
  double rmse_vector;
  long   currentLength;
  int    result;
	
  currentLength = (1 << COMPLEX_FFT2D_MAX_POWER_X) *
    (1 << COMPLEX_FFT2D_MAX_POWER_Y);
  data1 = (float*)valloc(2*currentLength*sizeof(float));
  data2 = (float*)valloc(2*currentLength*sizeof(float));
			
  if (data1 == 0 || data2 == 0 ) {
    printf("error allocating space for FFT 2D test\n");
  } else {
			
    for (xsize=COMPLEX_FFT2D_MIN_POWER_X; xsize <= COMPLEX_FFT2D_MAX_POWER_X;
	 xsize++) {
		
      for (ysize = COMPLEX_FFT2D_MIN_POWER_Y; ysize <= COMPLEX_FFT2D_MAX_POWER_Y;
	   ysize++) {
			
	currentLength = (1 << xsize) * (1 << ysize);
	
	/////////////////////////////////////////////////
	// initialize signal
	/////////////////////////////////////////////////

	for (j=0; j<currentLength; j++) {
	  data1[2*j] = data2[2*j] =
	    cos(PI*j*j/currentLength);
	  data1[2*j+1] = data2[2*j+1] =
	    sin(PI*j*j/currentLength);
	}

	/////////////////////////////////////////////////
	// perform forward and inverse FFT and compare
	// result with original signal
	/////////////////////////////////////////////////

	result = FFT2DComplex(data1, 1 << xsize, 1 << ysize, -1);
	if (result != 0) {
	  printf("error in fft 2d complex\n");
	}
				
	result = FFT2DComplex(data1, 1 << xsize, 1 << ysize, 1);
	if (result != 0) {
	  printf("error in fft 2d complex\n");
	}
				
	rmse_vector = RMSE_complex(data1, data2, currentLength);
				
	printf(" complex 2D %d X %d FFT rmse: %g\n",
	       1 << xsize, 1 << ysize, rmse_vector);
      }
    }
  }
	
  if (data1) free(data1);
  if (data2) free(data2);
}


#define REAL_FFT2D_MIN_POWER_X	3
#define REAL_FFT2D_MAX_POWER_X	9
#define REAL_FFT2D_MIN_POWER_Y	2
#define REAL_FFT2D_MAX_POWER_Y	9
static void TestFFT2DReal()
{
  float  *data1;
  float  *data2;
  int    xsize, ysize, j;
  double negativeOneToI = -1;
  double rmse_vector;
  long   currentLength;
  int    result;
	
  currentLength = (1 << REAL_FFT2D_MAX_POWER_X) * (1 << REAL_FFT2D_MAX_POWER_Y);
  data1 = (float*)valloc(2*currentLength*sizeof(float));
  data2 = (float*)valloc(2*currentLength*sizeof(float));

  if (data1 == 0 || data2 == 0 ) {
    printf("error allocating space for FFT 2D test\n");
  } else {

    for (xsize=REAL_FFT2D_MIN_POWER_X; xsize <=	REAL_FFT2D_MAX_POWER_X; xsize++) {
		
      for (ysize = REAL_FFT2D_MIN_POWER_Y; ysize <= REAL_FFT2D_MAX_POWER_Y;
	   ysize++) {
			
	currentLength = (1 << xsize) * (1 << ysize);
			
	/////////////////////////////////////////////////
	// initialize signal
	/////////////////////////////////////////////////

	for (j=0; j<currentLength; j++) {
	  negativeOneToI = -negativeOneToI;
					
	  data1[j] = data2[j] = 
	    negativeOneToI + sin(3*PI*j/currentLength);
	}

	/////////////////////////////////////////////////
	// perform forward and inverse FFT and compare
	// result with original signal
	/////////////////////////////////////////////////

	result = FFT2DRealForward(data1, 1 << xsize, 1 << ysize);
	if (result != 0) {
	  printf("error in fft 2d real forward\n");
	}
				
	result = FFT2DRealInverse(data1, 1 << xsize, 1 << ysize);
	if (result != 0) {
	  printf("error in fft 2d real inverse\n");
	}
				
	rmse_vector = RMSE_real(data1, data2, currentLength);
				
	printf(" real 2D %d X %d FFT rmse: %g\n",
	       1 << xsize, 1 << ysize, rmse_vector);
      }
    }
  }
	
  if (data1) free(data1);
  if (data2) free(data2);
}

#define COMPLEX_CONVOLVE2D_MIN_POWER_X	2
#define COMPLEX_CONVOLVE2D_MAX_POWER_X	5
#define COMPLEX_CONVOLVE2D_MIN_POWER_Y	2
#define COMPLEX_CONVOLVE2D_MAX_POWER_Y	5


#define REAL_CONVOLVE2D_MIN_POWER_X	3
#define REAL_CONVOLVE2D_MAX_POWER_X	8
#define REAL_CONVOLVE2D_MIN_POWER_Y	2
#define REAL_CONVOLVE2D_MAX_POWER_Y	8

static void TestConvolve2DReal()
{
  float  *data1;
  float  *data2;
  float  *literalConvolve;
  int    xsize, ysize, j;
  double negativeOneToI = -1;
  double rmse_vector;
  long   currentLength;
  int    result;
	
  currentLength = (1 << REAL_CONVOLVE2D_MAX_POWER_X) *
    (1 << REAL_CONVOLVE2D_MAX_POWER_Y);
  data1 = (float*)valloc(currentLength*sizeof(float));
  data2 = (float*)valloc(currentLength*sizeof(float));
  literalConvolve = (float*)valloc(currentLength*sizeof(float));
	
  if (data1 == 0 || data2 == 0 || literalConvolve == 0) {

    printf("error allocating space for convolve 2D real test\n");

  } else {

    for (xsize=REAL_CONVOLVE2D_MIN_POWER_X; xsize <= REAL_CONVOLVE2D_MAX_POWER_X;
	 xsize++) {
		
      for (ysize = REAL_CONVOLVE2D_MIN_POWER_Y;
	   ysize <=REAL_CONVOLVE2D_MAX_POWER_Y; ysize++) {
			
	currentLength = (1 << xsize) * (1 << ysize);
			
	/////////////////////////////////////////////////
	// initialize signals
	/////////////////////////////////////////////////

	for (j=0; j<currentLength; j++) {
	  negativeOneToI = -negativeOneToI;
					
	  data1[j] = negativeOneToI + sin(0.05*PI*j/currentLength);
	  data2[j] = -1.3  * cos(1.3*PI*j/currentLength);
	}

	/////////////////////////////////////////////////
	// perform literal convolution, and compare
	// with convolution performed through FFT
	/////////////////////////////////////////////////

	Convolve2DRealLiteral(data1, data2, literalConvolve,
			      1 << xsize, 1 << ysize);

	result = ConvolveRealAltivec2D(data1, data2, 1 << xsize, 1 << ysize);
	if (result != 0) {
	  printf("error in fft 2d real\n");
	}
				
	rmse_vector = RMSE_real(data2, literalConvolve, currentLength);
				
	printf(" real 2D %d X %d convolve rmse: %g\n",
	       1 << xsize, 1 << ysize, rmse_vector);
      }
    }
  }
  
  if (data1) free(data1);
  if (data2) free(data2);
  if (literalConvolve) free(literalConvolve);
}


#pragma mark -

void test_vbigdsp()
{
  
  /////////////////////////////////////////////////////////////////////////
	
  printf("\n\nTesting complex FFT:\n");
	
  TestComplexFFT(); 

  /////////////////////////////////////////////////////////////////////////

  printf("\n\nTesting real FFT:\n");

  TestRealFFT();

  /////////////////////////////////////////////////////////////////////////

  printf("\n\nTesting 2D complex FFT:\n");

  TestFFT2DComplex();

  /////////////////////////////////////////////////////////////////////////

  printf("\n\nTesting 2D real FFT:\n");

  TestFFT2DReal();

  /////////////////////////////////////////////////////////////////////////

  printf("\n\nTesting complex convolution:\n");

  TestComplexConvolve();

  /////////////////////////////////////////////////////////////////////////

  printf("\n\nTesting real convolution:\n");

  TestRealConvolve();

  /////////////////////////////////////////////////////////////////////////

  printf("\n\nTesting 2D real convolution:\n");

  TestConvolve2DReal();
}

#endif
