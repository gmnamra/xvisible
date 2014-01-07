/*
 *
 *$Id $
 *$Log$
 *Revision 1.3  2005/04/06 02:20:10  arman
 *first working version
 *
 *Revision 1.2  2005/04/05 22:32:22  arman
 *more
 *
 *Revision 1.1  2005/04/04 20:57:38  arman
 *ut for Kalman filter Support
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include "ut_kalman.h"

UT_kalman::UT_kalman () {}

UT_kalman::~UT_kalman ()
{
        printSuccessMessage( "Kalman Filtering Tests", mErrors );
}
  
uint32 UT_kalman::run ()
{
  rcKalmanFilter<1, 1> foo;
  
  rcVector<double, 1> mean (0.0);
  rc_Matrix<double, 1, 1> cov (10000);
  rc_Matrix<double, 1, 1> h (1.0);
  rc_Matrix<double, 1, 1> r (9.0);
  rc_Matrix<double, 1, 1> q (4.0);

  foo.H(h); // measurement matrix with covariance of 1.0
  foo.A(h); // movement matrix with covariance of 1.0
  foo.R(r); // measurement noise
  foo.Q(q); // movement noise

  foo.init (mean, cov); // initial mean and cov of the state

  cerr << foo.x()[0] << endl << foo.P().a(0,0) << endl << foo.K().a(0,0) << endl;

  rcVector<double, 1> m0 (84.0);  
  foo.update (m0);

  cerr << foo.x()[0] << endl << foo.P().a(0,0) << endl << foo.K().a(0,0) << endl;

  rcVector<double, 1> m1 (83.0);  
  foo.update (m1);

  cerr << foo.x()[0] << endl << foo.P().a(0,0) << endl << foo.K().a(0,0) << endl;

  rcVector<double, 1> m2 (88.0);  
  foo.update (m2);

  cerr << foo.x()[0] << endl << foo.P().a(0,0) << endl << foo.K().a(0,0) << endl;

}


