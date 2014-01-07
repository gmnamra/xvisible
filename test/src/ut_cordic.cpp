/*
 *
 *$Id $
 *$Log$
 *Revision 1.10  2005/08/30 21:08:51  arman
 *Cell Lineage
 *
 *Revision 1.10  2005/08/13 23:52:52  arman
 *added ut for sigmoid
 *
 *Revision 1.9  2003/08/26 09:50:42  arman
 *fixed warnings
 *
 *Revision 1.8  2003/06/19 17:39:30  arman
 *modified performance to test exactly what we are using (cartesian to polar) conversion
 *
 *Revision 1.7  2003/06/11 02:31:04  arman
 *adjusted cordic ut for improved accuracy
 *
 *Revision 1.6  2003/04/03 19:59:25  arman
 *adjusted to the reduction in precision of rcfixed16
 *
 *Revision 1.5  2003/03/20 22:49:39  arman
 *Position to fixed
 *
 *Revision 1.4  2003/03/11 15:41:05  arman
 *added unit tests for units
 *
 *Revision 1.3  2002/12/27 16:24:20  arman
 *Added Benchmarks
 *
 *Revision 1.2  2002/12/24 19:15:22  arman
 *First version
 *
 *Revision 1.1  2002/12/24 17:15:19  arman
 *Fixed Point Cordic Unit Tests
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_cordic.h"
#include <rc_math.h>
#include <stdio.h>
#include <stdlib.h>
#include <rc_time.h>
#include <rc_vector2d.h>

typedef int32 TFixed;	/* f * 2^16 */



UT_cordic::UT_cordic ()
{}

uint32 UT_cordic::run ()
{
  test();
  testUnits ();
  return mErrors;
}


UT_cordic::~UT_cordic ()
{
  printSuccessMessage( "rcCordic test", mErrors );  
}

void UT_cordic::test()
{
  double dx, dy;
  TFixed x, y, rx, ry, r, t;

	
  dx = dy = 1;
  x = TFixed (dx * (1L << 16));
  y = TFixed (dy * (1L << 16));
  r = x;
  t = y;
  rfFxPolarize(&r, &t);
  rcUNITTEST_ASSERT(rfRealEq (double (r/65536.0), 1.414, 0.001));
  rcUNITTEST_ASSERT(rfRealEq (double (t/65536.0), 44.9998, 0.001));
  //  printf("(%.4f, %.4f) --> %.4f @ %.4f Degrees \n",x/65536.0, y/65536.0, r/65536.0, t/65536.0);

	/* ArcTan2 */
  rx = TFixed (0.3333 * (1L << 16));
  ry = TFixed (0.6667 * (1L << 16));
  t = rfFxAtan2 (rx, ry);
  rcUNITTEST_ASSERT(rfRealEq (double (t/65536.0), 63.4383, 0.001));
  //  printf("(%.4f, %.4f) --> %.4f \n",rx/65536.0, ry/65536.0, t/65536.0);


  // Performance Benchmark
  rcTime timer;
  int32 count (10000000);
  rc2Dvector p;
  timer.start ();
  for (int32 i = 0; i < count; i++)
    {
      p[0] = (double) i;
      p[1] = (double) (i+1);
      p.len();
      p.angle();
    }
  timer.end ();


  double dMilliSeconds = timer.milliseconds () / count;
  double dMicroSeconds = timer.microseconds () / count;
 
   // Per Byte in Useconds
  double perOp = dMicroSeconds;

   fprintf(stderr,
           "Performance: Cartesian to Polar\t(floating point):\t%.6f us  %.2f Op/s \n", perOp, 1000000 / perOp);

   int32 px, py;
  timer.start ();
  for (int32 i = 0; i < count; i++)
    {
      px = i;
      py = 65536 - i;
      rfFxPolarize (&px, &py);
    }
  timer.end ();


  dMilliSeconds = timer.milliseconds () / count;
  dMicroSeconds = timer.microseconds () / count;
 
   // Per Byte in Useconds
  perOp = dMicroSeconds;

  fprintf(stderr,
           "Performance: Cartesian to Polar\t(fixed point   ):\t%.6f us  %.2f Op/s \n", perOp, 1000000 / perOp);

}


void UT_cordic::testUnits()
{
  rcDegree deg2;
  rcDegree deg4;
  rcDegree deg8;
  double foo;

  rcDegree deg1 (1.0);
  deg2 = 2. * rcDegree(1.0);
  deg4 = 4. * rcDegree(1.0);
  deg8 = 8. * rcDegree(1.0);
  foo = cos (1.);

  int32 eCount = 0;

  if (deg1 != rcDegree(1.0)) ++eCount;
  if (deg1 == deg2) ++eCount;
  if (deg2 == deg1) ++eCount;
  if (! (deg1 == rcDegree(1.0))) ++eCount;
  if (! (deg1 != deg2)) ++eCount;
  if (! (deg2 != deg1)) ++eCount;
  rcUTCheck (eCount == 0);
  
  eCount = 0;

  if (deg1 > deg2) ++eCount;
  if (deg1 > rcDegree(1.0)) ++eCount;
  if (! (deg2 > deg1)) ++eCount;
  rcUTCheck (eCount == 0);


  eCount = 0;

  if (deg2 < deg1) ++eCount;
  if (deg1 < rcDegree(1.0)) ++eCount;
  if (! (deg1 < deg2)) ++eCount;
  rcUTCheck (eCount == 0);


  eCount = 0;

  if (! (deg1 >= rcDegree(1.0))) ++eCount;
  if (deg1 >= deg2) ++eCount;
  if (! (deg2 >= deg1)) ++eCount;
  rcUTCheck (eCount == 0);

  eCount = 0;

  if (! (deg1 <= rcDegree(1.0))) ++eCount;
  if (! (deg1 <= deg2)) ++eCount;
  if (deg2 <= deg1) ++eCount;
  rcUTCheck (eCount == 0);

  eCount = 0;

  rcRadian radian;
  radian = rcRadian(1.0);

  if (radian != rcRadian(1.0))
    ++eCount;
  rcUTCheck (eCount == 0);

  eCount = 0;

  if (deg2 != deg1 + deg1)
    ++eCount;
  if (deg4 != deg2 + deg1 + deg1)
    ++eCount;
  rcUTCheck (eCount == 0);

  eCount = 0;

  if (deg1 != deg2 - deg1)
    ++eCount;
  if (deg2 != (deg4 - deg1) - deg1)
    ++eCount;
  rcUTCheck (eCount == 0);

  eCount = 0;

  if (deg2 * 2. != deg4)
    ++eCount;
  if (deg8 != deg2 * 4.)
    ++eCount;
  rcUTCheck (eCount == 0);

  eCount = 0;

  if (deg8 / 4.0 != deg2)
    ++eCount;
  if (deg4 / 2. != deg2)
    ++eCount;
  rcUTCheck (eCount == 0);


  eCount = 0;

  deg1 *= 6;
  deg1 /= 3;
  deg1 -= deg2;
  deg1 += rcDegree(1.0);

  if (deg1 != rcDegree(1.0))
    ++eCount;

  rcUTCheck (eCount == 0);

  eCount = 0;
  double d (3.3);
  rcFixed16 p16 (d);
  if (!rfRealEq ((double) p16.real(), d, (double) 1/255.))
    ++eCount;

  d += 2.0;
  p16 += 2.0;
  if (!rfRealEq ((double) p16.real(), d, (double) 1/255.))
    ++eCount;

  rcUTCheck (eCount == 0);

  radian = rcDegree(1.0);
  rcUTCheck (rfRealEq(rcDegree(1.0),rcDegree(radian) ,rcDegree(1e-15)));

  // Test rfSigmoid
  rcUTCheck (rfRealEq (rfSigmoid (0.0), 0.5));
  rcUTCheck (rfRealEq (rfSigmoid (1.0), 0.731, 0.0001));
}

