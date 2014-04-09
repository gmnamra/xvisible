/*
 *
 *$Id $
 *$Log$
 *Revision 1.8  2005/11/07 17:32:09  arman
 *cell lineage iv and bug fix
 *
 *Revision 1.7  2005/07/12 02:10:41  arman
 *fixed typo
 *
 *Revision 1.6  2005/07/12 02:10:13  arman
 *added rfLog2Max
 *
 *Revision 1.5  2005/05/09 18:05:06  arman
 *added rfIsPowerOf2
 *
 *Revision 1.4  2004/09/13 15:54:53  arman
 *added inclusion of vector
 *
 *Revision 1.3  2003/07/21 15:20:13  arman
 *incremental ci
 *
 *Revision 1.2  2003/07/18 03:44:20  arman
 *added statistic distribution support
 *
 *Revision 1.1  2003/03/11 15:37:56  arman
 *Support for special units
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_math.h>
#include <iostream>
#include <vector>


bool rfIsPowerOf2(int32 n)
{
  if ( n == 0 ) return false;
  while( (n % 2) == 0 )
    {
      n >>= 1;
    }
  return (n == 1);
}

int32 rfLog2max(int32 n)
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

bool real_equal (rcRadian x, rcRadian y, rcRadian epsilon)
{ 
  return abs((x - y).normSigned()) <= epsilon;
}


rcRadian rcRadian::norm () const
{ 
  rcRadian r;
  r.val = fmod (val, 2 * rkPI);
  if (r.val < 0.) r.val += 2 * rkPI;
  return r;
}

rcDegree rcDegree::norm () const
{ 
  rcDegree r;
  r.val = fmod (val, 360.);
  if (r.val < 0.) r.val += 360.;
  return r;
}

/*
 * Signed unit normalization routines:
 *
 * The standard normalization routines force the result into the
 * In the range of [-N/2 - N/2] return the sign that produces
 * the minimum magnitude result. 
 */
rcRadian rcRadian::normSigned () const
{ 
  rcRadian r;
  r = norm();
  if (r.val < 2 * rkPI - r.val)
    return r;
  r.val = r.val - 2 * rkPI;
  return r;
}

rcDegree rcDegree::normSigned () const
{ 
  rcDegree r;
  r = norm();
  if (r.val < 360. - r.val)
    return r;
  r.val = r.val - 360.;
  return r;
}



double rfRanOne(int32 &idum)
{
  const int32 IA=16807,IM=2147483647,IQ=127773,IR=2836,NTAB=32;
  const int32 NDIV=(1+(IM-1)/NTAB);
  const double EPS=3.0e-16,AM=1.0/IM,RNMX=(1.0-EPS);
  static int32 iy=0;
	static std::vector<int32> iv(NTAB);
  int32 j,k;
  double temp;

  if (idum <= 0 || !iy) {
    if (-idum < 1) idum=1;
    else idum = -idum;
    for (j=NTAB+7;j>=0;j--) {
      k=idum/IQ;
      idum=IA*(idum-k*IQ)-IR*k;
      if (idum < 0) idum += IM;
      if (j < NTAB) iv[j] = idum;
    }
    iy=iv[0];
  }
  k=idum/IQ;
  idum=IA*(idum-k*IQ)-IR*k;
  if (idum < 0) idum += IM;
  j=iy/NDIV;
  iy=iv[j];
  iv[j] = idum;
  if ((temp=AM*iy) > RNMX) return RNMX;
  else return temp;
}

double rfGasDev(int32 &idum)
{
  static int32 iset=0;
  static double gset;
  double fac,rsq,v1,v2;

  if (idum < 0) iset=0;
  if (iset == 0) {
    do {
      v1=2.0*rfRanOne(idum)-1.0;
      v2=2.0*rfRanOne(idum)-1.0;
      rsq=v1*v1+v2*v2;
    } while (rsq >= 1.0 || rsq == 0.0);
    fac=sqrt(-2.0*log(rsq)/rsq);
    gset=v1*fac;
    iset=1;
    return v2*fac;
  } else {
    iset=0;
    return gset;
  }
}

int32 rfGCDeuclid(int32 a, int32 b)
 {
   /* Returns the greatest common divisor of u and v by Euclid's method.
    * I have experimented also with Stein's method, which involves only
    * subtraction and left/right shifting; Euclid is faster, both for
    * integers of size 0 - 1024 and also for random integers of a size
    * which fits in a long integer.  Stein's algorithm might be better
    * when the integers are HUGE, but for our purposes, Euclid is fine.
    *
    * Walter H. F. Smith, 25 Feb 1992, after D. E. Knuth, vol. II  */
 
   int32 u,v,r;
 
   u = rmMax (rmABS(a), rmABS(b));
   v = rmMin (rmABS(a), rmABS(b));
 
   while (v > 0) {
     r = u%v;    /* Knuth notes that u < 2v 40% of the time;  */
     u = v;      /* thus we could have tried a subtraction  */
     v = r;      /* followed by an if test to do r = u%v  */
   }
   return(u);
 }
