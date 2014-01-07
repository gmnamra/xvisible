/*
 *
 *$Id: ut_stats.cpp 6124 2008-09-26 20:46:18Z arman $
 *
 * Copyright (c) 2003 Reify Corp. All rights reserved.
 */

#include "ut_stats.h"
#include <rc_lsfit.h>

UT_stats::UT_stats ()
{
}

uint32 UT_stats::run ()
{
    test();
    testfit();
    return mErrors;
}


UT_stats::~UT_stats ()
{
    printSuccessMessage( "Stats test", mErrors );  
}

void UT_stats::testfit()
{
  float x (-3.0f), y (-3.0f);
  rc2Fvector xy (x, y);
  float data[9] = {0.2f, 0.9f, 0.9f, 0.2f, 1.0f, 0.8f, 0.5f, 0.8f, 0.8f};

  vector<float> first (9);
  for (int32 i = 0; i < 9; i++) first[i] = data [i];
  rfParfit2DR3 (first, xy);
  rcUTCheck (rfRealEq (xy.x(), 0.3275f, 0.001f));
  rcUTCheck (rfRealEq (xy.y(), 0.4827f, 0.001f));

  float xx (-3.0f), yy (-3.0f);
  rc2Fvector vv (xx, yy);
  rfParfit2DR3 (first[0], first[1], first[2], first[3], first[4], first[5], first[6],
		first[7], first[8], vv);
  rcUTCheck (rfRealEq (vv.x(), xy.x(), 0.001f));
  rcUTCheck (rfRealEq (vv.y(), xy.y(), 0.001f));
  
  vector<double> space (3);
  space[0] = first[3]; space[1] = first[4]; space[2] = first[5];
  rcUTCheck (rfRealEq (rfParabolicFit (space), 0.3, 0.001));

  space[0] = first[1]; space[1] = first[4]; space[2] = first[7];
  rcUTCheck (rfRealEq (rfParabolicFit (space), (double) -0.1666, (double) 0.001));


  float sx (0), sy (0);
  sx += -first[0] + first[2] - first[3] + first[5] - first[6] + first[8];
  sy += -first[0] + first[6] - first[1] + first[7] - first[2] + first[8];

  float dx (0), dy (0);
  rfMomentFit(first, dx, dy);
  rcUTCheck (rfRealEq (dy, sy/9.0f, 0.001f));
  rcUTCheck (rfRealEq (dx, sx/9.0f, 0.001f));

  {
    rcLsFit<float> fit3;
    vector<rc2Fvector> fixed (4);
    vector<rc2Fvector> moving (4);

    fixed[0] = rc2Fvector (0.0f, 0.0f);
    fixed[1] = rc2Fvector (0.0f, 1.0f);
    fixed[2] = rc2Fvector (1.0f, 1.0f);
    fixed[3] = rc2Fvector (1.0f, 0.0f);

    moving[0] = rc2Fvector (0.0f, -0.7f);
    moving[1] = rc2Fvector (-0.7f, 0.0f);
    moving[2] = rc2Fvector (0.0f, 0.7f);
    moving[3] = rc2Fvector (0.7f, 0.0f);

    for (uint32 i = 0; i < 4; i++)
      fit3.update (fixed[i], moving[i]);

    rc2Fvector trans;
    rcRadian rad;
    bool success = fit3.solve3p (trans, rad);

    rcUTCheck (success == true);
    rcUTCheck (rfRealEq (trans.x(), -0.5f, 0.001f));
    rcUTCheck (rfRealEq (trans.y(), -0.5f, 0.001f));
    rcUTCheck (rfRealEq (rad.Double(), 0.0, 0.001));
  }

#if 0 	// Currently buggy
  {
    rcLsFit<float> fit5;
    vector<rc2Fvector> fixed (4);
    vector<rc2Fvector> moving (4);

    fixed[0] = rc2Fvector (0.0f, 0.0f);
    fixed[1] = rc2Fvector (0.0f, 1.0f);
    fixed[2] = rc2Fvector (1.0f, 1.0f);
    fixed[3] = rc2Fvector (1.0f, 0.0f);

    moving[0] = rc2Fvector (0.0f, -0.7f);
    moving[1] = rc2Fvector (-0.7f, 0.0f);
    moving[2] = rc2Fvector (0.0f, 0.7f);
    moving[3] = rc2Fvector (0.7f, 0.0f);

    for (uint32 i = 0; i < 4; i++)
      fit5.update (fixed[i], moving[i]);

    rc2Xform xform;

    bool success = fit5.solve5p (xform);

    rc2Fvector trans (xform.transf());
    rcMatrix<2> mat = xform.matrix();

    float XX (0.0f);
    for (uint32 i = 0; i < 4; i++)
      {
	XX += rmSquare (moving[i] - (fixed[i] * mat.inverse() + trans));
	fit5.rms (fixed[i], moving[i], trans, mat) == sqrt ((float) XX / i);
      }

    rcUTCheck (success == true);
    rcUTCheck (rfRealEq ((float) mat.element(0,0), 0.7f, 0.001f));    
    rcUTCheck (rfRealEq ((float) mat.element(0,1), -0.7f, 0.001f));    
    rcUTCheck (rfRealEq ((float) mat.element(1,0), 0.7f, 0.001f));    
    rcUTCheck (rfRealEq ((float) mat.element(1,1), 0.7f, 0.001f));    
    rcUTCheck (rfRealEq (trans.x(), 0.0f, 0.001f));
    rcUTCheck (rfRealEq (trans.y(), -0.7f, 0.01f));
  }
#endif
}

void UT_stats::test()
{
    // A simple rfMedian test with a few different types
    {
        vector<double> v1;
        rcUNITTEST_ASSERT( rfMedian( v1 ) == 0.0 );
        v1.push_back( 1.0 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[0] );
        v1.push_back( 2.0 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == ((v1[0]+v1[1])/2) );
        v1.push_back( 3.0 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[1] );
    }
    {
        vector<float> v1;
        
        rcUNITTEST_ASSERT( rfMedian( v1 ) == 0.0 );
        v1.push_back( 1.0 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[0] );
        v1.push_back( 2.0 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == ((v1[0]+v1[1])/2) );
        v1.push_back( 3.0 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[1] );
    }
    {
        vector<uint32> v1;
        
        rcUNITTEST_ASSERT( rfMedian( v1 ) == 0.0 );
        v1.push_back( 1 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[0] );
        v1.push_back( 2 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == ((v1[0]+v1[1])/2) );
        v1.push_back( 3 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[1] );
    }
    {
        vector<int32> v1;
        
        rcUNITTEST_ASSERT( rfMedian( v1 ) == 0.0 );
        v1.push_back( 1 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[0] );
        v1.push_back( 2 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == ((v1[0]+v1[1])/2) );
        v1.push_back( 3 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[1] );
    }
    {
        vector<uint16> v1;
        
        rcUNITTEST_ASSERT( rfMedian( v1 ) == 0.0 );
        v1.push_back( 1 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[0] );
        v1.push_back( 2 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == ((v1[0]+v1[1])/2) );
        v1.push_back( 3 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[1] );
    }
    {
        vector<int16> v1;
        
        rcUNITTEST_ASSERT( rfMedian( v1 ) == 0.0 );
        v1.push_back( 1 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[0] );
        v1.push_back( 2 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == ((v1[0]+v1[1])/2) );
        v1.push_back( 3 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[1] );
    }
    {
        vector<uint8> v1;
        
        rcUNITTEST_ASSERT( rfMedian( v1 ) == 0.0 );
        v1.push_back( 1 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[0] );
        v1.push_back( 2 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == ((v1[0]+v1[1])/2) );
        v1.push_back( 3 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[1] );
    }
    {
        vector<int8> v1;
        
        rcUNITTEST_ASSERT( rfMedian( v1 ) == 0.0 );
        v1.push_back( 1 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[0] );
        v1.push_back( 2 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == ((v1[0]+v1[1])/2) );
        v1.push_back( 3 );
        rcUNITTEST_ASSERT( rfMedian( v1 ) == v1[1] );
    }


    // A simple rfMean and rfSum test with a few different types
    {
        vector<double> v1;
        rcUNITTEST_ASSERT( rfMean( v1 ) == 0.0 );
        v1.push_back( 1.0 );
        rcUNITTEST_ASSERT( rfMean( v1 ) == v1[0] );
        v1.push_back( 2.0 );
        rcUNITTEST_ASSERT( rfMean( v1 ) == ((v1[0]+v1[1])/2) );
        v1.push_back( 3.0 );
        rcUNITTEST_ASSERT( rfMean( v1 ) == ((v1[0]+v1[1]+v1[2])/3) );
    }
    {
        vector<double> v1;
        rcUNITTEST_ASSERT( rfSum( v1 ) == 0.0 );
        v1.push_back( 1.0 );
        rcUNITTEST_ASSERT( rfSum( v1 ) == v1[0] );
        v1.push_back( 2.0 );
        rcUNITTEST_ASSERT( rfSum( v1 ) == (v1[0]+v1[1]) );
        v1.push_back( 3.0 );
        rcUNITTEST_ASSERT( rfSum( v1 ) == (v1[0]+v1[1]+v1[2]) );
    }
    {
      double s5 (sqrt (5.0 / 2.0));
      double s14 (sqrt (14.0 / 3.0));

      vector<double> v1;
      vector<double> v2;
      v1.push_back( 1.0 );
      v2.push_back( 0.0 );
      rfRMS( v1, v2);
      rcUNITTEST_ASSERT(  v2.size() == 1 && v2[0] == 1 );
      v1.push_back( 2.0 );
      v2.push_back( 0.0 );v2[0] = 0;
      rfRMS( v1, v2);
      rcUNITTEST_ASSERT(  v2.size() == 2 && v2[0] == 1 
			   && 
			   rfRealEq (v2[1], s5, 0.01));
      v1.push_back( 3.0 );
      v2.push_back( 0.0 );v2[0] = 0;v2[1] = 0;
      rfRMS( v1, v2);
      rcUNITTEST_ASSERT(  v2.size() == 3 && v2[0] == 1 &&
			   rfRealEq (v2[1], s5, 0.01) &&
			   rfRealEq (v2[2], s14, 0.01));

    }

    {
      double s5 (sqrt (5.0 / 2.0));
      double s13 (sqrt (13.0 / 2.0));
      double s14 (sqrt (14.0 / 3.0));

      vector<double> v1;
      vector<double> v2;
      v1.push_back( 3.0 );
      v2.push_back( 0.0 );
      rfRevRMS( v1, v2);
      rcUNITTEST_ASSERT(  v2.size() == 1 && v2[0] == 3 );
      v1.push_back ( 2.0 );
      v2.push_back ( 0.0 );v2[0] = 0;
      rfRevRMS( v1, v2);
      rcUNITTEST_ASSERT(  v2.size() == 2 && v2[0] == 2
			   && 
			   rfRealEq (v2[1], s13, 0.01));
      v1.push_back( 1.0 );
      v2.push_back( 0.0 );v2[0] = 0;v2[1] = 0;
      rfRevRMS( v1, v2);
      rcUNITTEST_ASSERT(  v2.size() == 3 && v2[0] == 1 &&
			   rfRealEq (v2[1], s5, 0.01) &&
			   rfRealEq (v2[2], s14, 0.01));
      rcUTCheck (rfRealEq (rfRMS (v1), s14, 0.01));
    }
      
    // Tests for basic least square fitting
    {
      rcLsFit<float> ls;	  
      for (uint32 i = 0; i < 10; i++)
	{
	  rc2Fvector pt ((float) i, (float) i);
	  ls.update (pt);
	}

      rcLineSegment<float> l = ls.fit();
      rcUTCheck (rfRealEq ((float) l.angle().Double(), 0.785358f, 0.001f));
    }

    {
      rcLsFit<float> ls;	  
      for (uint32 i = 10; i ; i--)
	{
	  rc2Fvector pt ((float) (10 - i), (float) i);
	  ls.update (pt);
	}

      rcLineSegment<float> l = ls.fit();
      rcUTCheck (rfRealEq ((float) l.angle().normSigned().Double(), -0.785358f, 0.001f));
    }
	  
}
