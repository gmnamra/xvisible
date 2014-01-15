/*
 *
 *$Id $
 *$Log$
 *Revision 1.6  2004/04/28 02:48:09  arman
 *working version of the line ut
 *
 *Revision 1.5  2004/04/27 21:32:32  arman
 *incremental
 *
 *Revision 1.4  2003/06/19 18:14:01  arman
 *added a soft test for ctor initialization
 *
 *Revision 1.3  2003/03/05 22:22:33  sami
 *Do not expect infinite accuacy from matrix operations, added epsilon
 *
 *Revision 1.2  2003/03/04 20:15:24  arman
 **** empty log message ***
 *
 *Revision 1.1  2003/03/04 19:22:32  arman
 *minimal ut for rcLineSegment
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
#include "ut_line.h"
#include <rc_math.h>
#include <stdio.h>
#include <stdlib.h>
#include <rc_time.h>
#include <rc_line.h>

#define UTCHECKLINE(a,b,c) \
{ \
  rcUTCheck(real_equal((a).angle(),(b)));		  \
  rcUTCheck(real_equal((a).distance(),(c)));		  \
  rcUTCheck(real_equal((a).Sin(),sin((b))));		  \
  rcUTCheck(real_equal((a).Cos(),cos((b)))); }

#define UTCHECKVEC(a, b, c) {   \
    rcUTCheck (real_equal((a).x(), (b)));		\
    rcUTCheck (real_equal((a).y(), (c))); }

UT_line::UT_line ()
{}

uint32 UT_line::run ()
{
  test();

  return mErrors;
}


UT_line::~UT_line ()
{
  printSuccessMessage( "rcLine test", mErrors );  
}

void UT_line::test()
{
  {
  rcLineSegment<double> line;
  UTCHECKLINE(line,rcRadian(0),0);
  }

  {
  rcLineSegment<double> ll(rc2Dvector(1,2),rc2Dvector(1,1));
  UTCHECKLINE(ll,rcRadian(-rkPI/2.),1);
  rcLineSegment<double> lll(rc2Dvector(1,1),rc2Dvector(1,2));
  UTCHECKLINE(lll,rcRadian(-rkPI/2.),1);
  }

  {
  rcLineSegment<double> ll = rcLineSegment<double>(rc2Dvector(1,1),rc2Dvector(1,2));
  UTCHECKLINE(ll,rcRadian(-rkPI/2.),1);
  rcLineSegment<double> lll(ll);
  UTCHECKLINE(lll,rcRadian(-rkPI/2.),1);
  }

  rcLineSegment<double> line(rcRadian(rkPI/2.), 1.0);

  rc2Dvector pt = line.closestPoint(rc2Dvector(1,1));
  UTCHECKVEC(pt,-1,1);

  rcUTCheck (real_equal(line.distanceFrom(rc2Dvector(1,1)), 2));
  rcUTCheck (real_equal(line.distanceFrom(rc2Dvector(-2,1)), -1));
  rcUTCheck (real_equal(line.distanceAlong(rc2Dvector(1,1)), 1));

  vector<rc2Dvector> intpt = line.intersection(rcLineSegment<double>(rcRadian(rkPI/2.),0.0));
  rcUTCheck (intpt.size() == 0);
  intpt = line.intersection(rcLineSegment<double>(rcRadian(0),0));
  rcUTCheck (intpt.size() == 1);
  UTCHECKVEC(intpt[0],-1,0);

  {
  rcLineSegment<double> ll(rc2Dvector(1,2),rc2Dvector(1,1));
  rcLineSegment<double> lll(rc2Dvector(1,2),rc2Dvector(1,1));
  rcUTCheck (ll == lll);
  }    

  // Minimum testing of rcInfLine
  {
    rcInfLine v;
    rcUTCheck(v.dir().x() == 1.0 && v.dir().y() == 0.0);
    rcUTCheck(v.pos().x() == 0.0 && v.pos().y() == 0.0);    
  }
}
