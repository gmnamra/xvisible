/*
 *
 *$Id $
 *$Log$
 *Revision 1.3  2004/01/18 22:27:12  proberts
 *new muscle tracker and support code
 *
 *Revision 1.2  2003/10/02 02:24:26  arman
 *added ut for 1Dfft with answers from Mathematica
 *
 *Revision 1.1  2003/05/12 02:12:14  arman
 *Requires VecLib
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_freq.h"
#include <vbigdsp.h>

UT_freq::UT_freq()
{
}

UT_freq::~UT_freq()
{
    printSuccessMessage( "rcFrequency test", mErrors );
}

uint32 UT_freq::run()
{
  // Basic tests
  testBasics();
  return mErrors;
}


// Fourier[{-1, -1, -1, -1, 1, 1, 1, 1}, FourierParameters -> {-1, 1}]

// {0. + 0. , -0.25 - 0.603553 , 
//   0. + 0. , -0.25 - 0.103553 , 
//   0. + 0. , -0.25 + 0.103553 , 
//   0. + 0. , -0.25 + 0.603553 }
// In[26]:=
// Abs[%]
// Fourier[{1.0, 0.0, -1.0, 0.0, 1.0, 0.0 , -1.0, 0.0}, 
//   FourierParameters -> {-1, 1}]
// Out[26]=
// {0., 0.653281, 0., 0.270598, 0., 0.270598, 0., 0.653281}
// Out[27]=
// {0. + 0. , 
//   0. + 0. , 
//   0.5 + 0. , 
//   0. + 0. , 
//   0. + 0. , 
//   0. + 0. , 
//   0.5 + 0. , 
//   0. + 0. }
// In[28]:=
// Abs[%]
// Out[28]=
// {0., 0., 0.5, 0., 0., 0., 0.5, 0.}
// In[29]:=
// Fourier[{0.0, 1.0, 0.0, 1.0, 0.0 , 1.0, 0.0, 1.0}, 
//   FourierParameters -> {-1, 1}]
// Out[29]=
// {0.5 + 0. , 
//   0. + 0. , 
//   0. + 0. , 
//   0. + 0. , -0.5 + 0. , 
//   0. + 0. , 
//   0. + 0. , 
//   0. + 0. }
// In[30]:=
// Abs[%]
// Out[30]=
// {0.5, 0., 0., 0., 0.5, 0., 0., 0.}



void UT_freq::testBasics()
{
  float zf (0.0f);
  vector<float> imag (32, zf);

  {
    cout << "Use Step" << endl;
    vector<float> data(32);
    for (int32 i = 0; i < 16; i++) data[i] = -1.0f;
    for (int32 i = 16; i < 32; i++) data[i] = 1.0f;

    rf1Dfft (data, imag, 1);
  
    for (int32 i = 0; i < 32; i++)
      cout << "[" << data[i] << "+" << imag[i] << "ii] " << sqrt (rmSquare(data[i]) + rmSquare (imag[i])) << endl;

  }

  {
    cout << "Use Constant" << endl;
    float cf(3.14f);
    vector<float> data(8, cf);
    rf1Dfft (data, imag, 1);
  
    for (int32 i = 0; i < 8; i++)
      cout << "[" << data[i] << "+" << imag[i] << "ii] " << sqrt (rmSquare(data[i]) + rmSquare (imag[i])) << endl;
  }  

  {
    cout << "Use Cos" << endl;
    float fdata[8] = {1.0, 0.0, -1.0, 0.0, 1.0, 0.0 , -1.0, 0.0};
    vector<float> data(8);
    for (int32 i = 0; i < 8; i++) data[i] = fdata[i];    
    rf1Dfft (data, imag, 1);
  
    for (int32 i = 0; i < 8; i++)
      cout << "[" << data[i] << "+" << imag[i] << "ii] " << sqrt (rmSquare(data[i]) + rmSquare (imag[i])) << endl;
  }  

  {

    cout << "Use T = 2 Periodicity" << endl;
    float fdata[8] = {0.0, 1.0, 0.0, 1.0, 0.0 , 1.0, 0.0, 1.0};
    vector<float> data(8);
    for (int32 i = 0; i < 8; i++) data[i] = fdata[i];    
    rf1Dfft (data, imag, 1);
  
    for (int32 i = 0; i < 8; i++)
      cout << "[" << data[i] << "+" << imag[i] << "ii] " << sqrt (rmSquare(data[i]) + rmSquare (imag[i])) << endl;
  }  

  {
    char space[1024];
    float* data = (float*)(((int)space + 15) & 0xFFFFFFF0);
    printf("space 0x%X data 0x%X\n", (int)space, (int)data);

    for (int32 i = 0; i < 16; i++) data[i] = -1.0f;
    for (int32 i = 16; i < 32; i++) data[i] = 1.0f;
    for (int32 i = 32; i < 64; i++) data[i] = 1e10;

    int32 retval = FFTRealForward(data, 32);
  
    cout << "FFTRealForward - retval: " << retval << endl;
    for (int32 i = 0; i < 64; i += 2)
      cout << "[" << data[i] << "+" << data[i+1] << "ii] "
	   << sqrt (rmSquare(data[i]) + rmSquare (data[i+1]))/32 << endl;

    for (int32 i = 0; i < 32; ) { data[i++] = -1.0f; data[i++] = 0; }
    for (int32 i = 32; i < 64; ) { data[i++] = 1.0f; data[i++] = 0; }
    
#ifdef __ppc__
    retval = FFTComplex(data, 32, IFLAG_FFT_FORWARD);
  
    cout << "FFTCompplex - retval: " << retval << endl;
    for (int32 i = 0; i < 64; i +=2)
      cout << "[" << data[i] << "+" << data[i+1] << "ii] "
	   << sqrt (rmSquare(data[i]) + rmSquare (data[i+1]))/32 << endl;
#endif	
  }
}
