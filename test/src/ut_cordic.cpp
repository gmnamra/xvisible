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
  testUnits ();
  return mErrors;
}


UT_cordic::~UT_cordic ()
{
  printSuccessMessage( "rcCordic test", mErrors );  
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
  if (!real_equal ((double) p16.real(), d, (double) 1/255.))
    ++eCount;

  d += 2.0;
  p16 += 2.0;
  if (!real_equal ((double) p16.real(), d, (double) 1/255.))
    ++eCount;

  rcUTCheck (eCount == 0);

  radian = rcDegree(1.0);
  rcUTCheck (real_equal(rcDegree(1.0),rcDegree(radian) ,rcDegree(1e-15)));

  // Test rfSigmoid
  rcUTCheck (real_equal (rfSigmoid (0.0), 0.5));
  rcUTCheck (real_equal (rfSigmoid (1.0), 0.731, 0.0001));
}

