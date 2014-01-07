/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      ut_polygon.cpp
 *   Creation Date  09/09/2003
 *   Author         Peter Roberts
 *
 ***************************************************************************/

#include <ut_polygon.h>

UT_Polygon::UT_Polygon()
{
}

UT_Polygon::~UT_Polygon()
{
  printSuccessMessage("Polygon tests", mErrors);
}

uint32 UT_Polygon::run()
{
  testBasic();
  testGeomProperties();

  testConvexHullConvert();
  testIntersects();
  testContains();
  testPolygonCombine();

  testMinEncRect();
  testOrthEncRect();
  testDiscBuffer ();
  testPolygonGroup ();
  return mErrors;
}


void UT_Polygon::testBasic()
{
  /* Do some simple tests that add and remove points from a polygon.
   * Check that the points can be read back at the correct location
   * and that the polygon state has been set correctly.
   */
  {
    rc2Dvector p0(1, 1), p1(2,2), p2(2,4);
    rcPolygon p;

    rcUNITTEST_ASSERT(p.vertexCnt() == 0);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.pushVertex(p0);

    rcUNITTEST_ASSERT(p.vertexCnt() == 1);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.pushVertex(p1);

    rcUNITTEST_ASSERT(p.vertexCnt() == 2);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.pushVertex(p2);

    rcUNITTEST_ASSERT(p.vertexCnt() == 3);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p1);
    rcUNITTEST_ASSERT(p.vertexAt(2) == p2);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.popVertex();

    rcUNITTEST_ASSERT(p.vertexCnt() == 2);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.insertVertex(p2, 0);

    rcUNITTEST_ASSERT(p.vertexCnt() == 3);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p2);
    rcUNITTEST_ASSERT(p.vertexAt(2) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == false);

    p.deleteVertex(1);

    rcUNITTEST_ASSERT(p.vertexCnt() == 2);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.insertVertex(p2, 1);

    rcUNITTEST_ASSERT(p.vertexCnt() == 3);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p1);
    rcUNITTEST_ASSERT(p.vertexAt(2) == p2);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.deleteVertex(2);

    rcUNITTEST_ASSERT(p.vertexCnt() == 2);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.insertVertex(p2, 2);

    rcUNITTEST_ASSERT(p.vertexCnt() == 3);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p2);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(2) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.deleteVertex(3);

    rcUNITTEST_ASSERT(p.vertexCnt() == 2);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.deleteVertex(0);

    rcUNITTEST_ASSERT(p.vertexCnt() == 1);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    bool retval = p.pushUniqueVertex(p1);

    rcUNITTEST_ASSERT(retval == false);
    rcUNITTEST_ASSERT(p.vertexCnt() == 1);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    retval = p.pushUniqueVertex(p2);

    rcUNITTEST_ASSERT(retval == true);
    rcUNITTEST_ASSERT(p.vertexCnt() == 2);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p1);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p2);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    retval = p.pushUniqueVertex(p1);

    rcUNITTEST_ASSERT(retval == true);
    rcUNITTEST_ASSERT(p.vertexCnt() == 3);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p1);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p2);
    rcUNITTEST_ASSERT(p.vertexAt(2) == p1);
    rcUNITTEST_ASSERT(p.isValid() == false);
    rcUNITTEST_ASSERT(p.isConvex() == false);

    p.popVertex();

    rcUNITTEST_ASSERT(p.vertexCnt() == 2);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p1);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p2);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.popVertex();

    rcUNITTEST_ASSERT(p.vertexCnt() == 1);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.popVertex();

    rcUNITTEST_ASSERT(p.vertexCnt() == 0);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    retval = p.pushUniqueVertex(p1);

    rcUNITTEST_ASSERT(retval == true);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p1);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);
  }

  /* Some more rigorous tests of the isValid() and isConvex()
   * functions.
   */
  {
    rc2Dvector v0(5, 1), v1(8, 4), v2(8, 7), v3(5, 6), v4(4, 5);
    rc2Dvector v5(2, 3), v6(4, 10), v7(5, 10);

    {
      rcPolygon p;
      p.pushVertex(v5); p.pushVertex(v0); p.pushVertex(v3); p.pushVertex(v1);
      rcUNITTEST_ASSERT(p.isValid() == false);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v5); p.pushVertex(v0); p.pushVertex(v1); p.pushVertex(v3);
      rcUNITTEST_ASSERT(p.isValid() == true);
      rcUNITTEST_ASSERT(p.isConvex() == true);
    }

    {
      rcPolygon p;
      p.pushVertex(v5); p.pushVertex(v0); p.pushVertex(v4); p.pushVertex(v1);
      p.pushVertex(v3);
      rcUNITTEST_ASSERT(p.isValid() == false);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v5); p.pushVertex(v0); p.pushVertex(v1); p.pushVertex(v3);
      p.pushVertex(v4);
      rcUNITTEST_ASSERT(p.isValid() == true);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v5); p.pushVertex(v0); p.pushVertex(v1); p.pushVertex(v2);
      p.pushVertex(v6); p.pushVertex(v3);
      rcUNITTEST_ASSERT(p.isValid() == true);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v1); p.pushVertex(v3); p.pushVertex(v4); p.pushVertex(v5);
      rcUNITTEST_ASSERT(p.isValid() == true);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v0); p.pushVertex(v5); p.pushVertex(v1); p.pushVertex(v2);
      p.pushVertex(v3);
      rcUNITTEST_ASSERT(p.isValid() == false);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v5); p.pushVertex(v0); p.pushVertex(v0);
      rcUNITTEST_ASSERT(p.isValid() == false);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v5); p.pushVertex(v0); p.pushVertex(v5);
      rcUNITTEST_ASSERT(p.isValid() == false);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v0); p.pushVertex(v1); p.pushVertex(v2); p.pushVertex(v3);
      p.pushVertex(v2); p.pushVertex(v6); p.pushVertex(v5);
      rcUNITTEST_ASSERT(p.isValid() == false);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v0); p.pushVertex(v1); p.pushVertex(v2); p.pushVertex(v2);
      p.pushVertex(v2); p.pushVertex(v6); p.pushVertex(v5);
      rcUNITTEST_ASSERT(p.isValid() == false);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }

    {
      rcPolygon p;
      p.pushVertex(v0); p.pushVertex(v7); p.pushVertex(v6); p.pushVertex(v3);
      p.pushVertex(v4);
      rcUNITTEST_ASSERT(p.isValid() == false);
      rcUNITTEST_ASSERT(p.isConvex() == false);
    }
  }

  /* Test equality operators
   */
  {
    rc2Dvector v0(5, 1), v1(8, 4), v2(8, 7), v3(5, 6), v4(9, 14);

    {
      rcPolygon p1, p2;
      rcUNITTEST_ASSERT(p1 == p2);
      rcUNITTEST_ASSERT(p2 == p1);

      p1.pushVertex(v0);
      rcUNITTEST_ASSERT(p1 != p2);
      rcUNITTEST_ASSERT(p2 != p1);

      p2.pushVertex(v1);
      rcUNITTEST_ASSERT(p1 != p2);
      rcUNITTEST_ASSERT(p2 != p1);
    }

    {
      rcPolygon p1, p2;
      p1.pushVertex(v0);
      p2.pushVertex(v0);
      rcUNITTEST_ASSERT(p1 == p2);
      rcUNITTEST_ASSERT(p2 == p1);

      p1.pushVertex(v1);
      rcUNITTEST_ASSERT(p1 != p2);
      rcUNITTEST_ASSERT(p2 != p1);
    }

    {
      rcPolygon p1, p2;
      p1.pushVertex(v0); p1.pushVertex(v1); p1.pushVertex(v2); p1.pushVertex(v3);
      p2.pushVertex(v1); p2.pushVertex(v2); p2.pushVertex(v3); p2.pushVertex(v0);
      rcUNITTEST_ASSERT(p1 == p2);
      rcUNITTEST_ASSERT(p2 == p1);

      p2.pushVertex(v4);
      rcUNITTEST_ASSERT(p1 != p2);
      rcUNITTEST_ASSERT(p2 != p1);
    }

    {
      rcPolygon p1, p2;
      p1.pushVertex(v0); p1.pushVertex(v1); p1.pushVertex(v2); p1.pushVertex(v3);
      p2.pushVertex(v2); p2.pushVertex(v3); p2.pushVertex(v0); p2.pushVertex(v1);
      rcUNITTEST_ASSERT(p1 == p2);
      rcUNITTEST_ASSERT(p2 == p1);

      p2.pushVertex(v4);
      rcUNITTEST_ASSERT(p1 != p2);
      rcUNITTEST_ASSERT(p2 != p1);
    }

    {
      rcPolygon p1, p2;
      p1.pushVertex(v0); p1.pushVertex(v1); p1.pushVertex(v2); p1.pushVertex(v3);
      p2.pushVertex(v3); p2.pushVertex(v0); p2.pushVertex(v1); p2.pushVertex(v2);
      rcUNITTEST_ASSERT(p1 == p2);
      rcUNITTEST_ASSERT(p2 == p1);

      p2.pushVertex(v4);
      rcUNITTEST_ASSERT(p1 != p2);
      rcUNITTEST_ASSERT(p2 != p1);
    }

    {
      rcPolygon p1, p2;
      p1.pushVertex(v0); p1.pushVertex(v1); p1.pushVertex(v2); p1.pushVertex(v3);
      p2.pushVertex(v0); p2.pushVertex(v1); p2.pushVertex(v2); p2.pushVertex(v3);
      rcUNITTEST_ASSERT(p1 == p2);
      rcUNITTEST_ASSERT(p2 == p1);

      p2.pushVertex(v4);
      rcUNITTEST_ASSERT(p1 != p2);
      rcUNITTEST_ASSERT(p2 != p1);
    }

    {
      rcPolygon p1, p2;
      p1.pushVertex(v0); p1.pushVertex(v1); p1.pushVertex(v3); p1.pushVertex(v2);
      p2.pushVertex(v0); p2.pushVertex(v1); p2.pushVertex(v2); p2.pushVertex(v3);
      rcUNITTEST_ASSERT(p1 != p2);
      rcUNITTEST_ASSERT(p2 != p1);
    }

    {
      rcPolygon p1, p2;
      p1.pushVertex(v0); p1.pushVertex(v1); p1.pushVertex(v2); p1.pushVertex(v2);
      p2.pushVertex(v0); p2.pushVertex(v1); p2.pushVertex(v2); p2.pushVertex(v3);
      rcUNITTEST_ASSERT(p1 != p2);
      rcUNITTEST_ASSERT(p2 != p1);
    }
  }

  /* Test translate fct.
   */
  {
    rc2Dvector v0(5, 1), v1(8, 4), v2(8, 7), v3(5, 6), v4(9, 14);
    rc2Dvector delta(2, 5);
    rc2Dvector w0(7, 6), w1(10, 9), w2(10, 12), w3(7, 11), w4(11, 19);

    {
      rcPolygon p, pd;
      
      pd.translate(delta);

      rcUNITTEST_ASSERT(p == pd);
      rcUNITTEST_ASSERT(pd == p);
    }

    {
      rcPolygon p, pd;
      
      p.pushVertex(v0);
      pd.pushVertex(w0);
      
      p.translate(delta);

      rcUNITTEST_ASSERT(p == pd);
      rcUNITTEST_ASSERT(pd == p);
    }

    {
      rcPolygon p, pd;
      
      p.pushVertex(v0); p.pushVertex(v1);
      pd.pushVertex(w0); pd.pushVertex(w1);
      
      p.translate(delta);

      rcUNITTEST_ASSERT(p == pd);
      rcUNITTEST_ASSERT(pd == p);
    }

    {
      rcPolygon p, pd;
      
      p.pushVertex(v0); p.pushVertex(v1); p.pushVertex(v2);
      pd.pushVertex(w0); pd.pushVertex(w1); pd.pushVertex(w2);
      
      p.translate(delta);

      rcUNITTEST_ASSERT(p == pd);
      rcUNITTEST_ASSERT(pd == p);
    }

    {
      rcPolygon p, pd;
      
      p.pushVertex(v0); p.pushVertex(v1); p.pushVertex(v2); p.pushVertex(v3);
      p.pushVertex(v4);
      pd.pushVertex(w0); pd.pushVertex(w1); pd.pushVertex(w2); pd.pushVertex(w3);
      pd.pushVertex(w4);
      
      p.translate(delta);

      rcUNITTEST_ASSERT(p == pd);
      rcUNITTEST_ASSERT(pd == p);
    }

    {
      rcPolygon pp(10.0f, 10);
      rcUTCheck (pp.isValid());
      rcUTCheck (pp.isConvex());
      rcUTCheck (pp.vertexCnt() == 10);
    }
  }
}

void UT_Polygon::testGeomProperties()
{
  rc2Dvector p0(3,3), p1(11,3), p2(11,12), p3(4,15), p4(1,10), p5(2,6), p6(6,6);

  {
    rcPolygon p;
    p.pushVertex(p0); p.pushVertex(p1); p.pushVertex(p2);
    p.pushVertex(p3); p.pushVertex(p4); p.pushVertex(p5); p.pushVertex(p6);

    rcUNITTEST_ASSERT(rfRealEq(p.area(), 89.5));
    rcUNITTEST_ASSERT(rfRealEq(p.perimeter(), 42.8124713, 1e-7));
  }

  {
    rcPolygon p;
    p.pushVertex(p0); p.pushVertex(p1); p.pushVertex(p2);
    p.pushVertex(p3); p.pushVertex(p4); p.pushVertex(p5);

    rcUNITTEST_ASSERT(rfRealEq(p.area(), 95.5));
    rcUNITTEST_ASSERT(rfRealEq(p.perimeter(), 37.7321083, 1e-7));
  }

  {
    rcPolygon p;
    p.pushVertex(p2); p.pushVertex(p3); p.pushVertex(p5);

    rcUNITTEST_ASSERT(rfRealEq(p.area(), 34.5));
    rcUNITTEST_ASSERT(rfRealEq(p.perimeter(), 27.6519714, 1e-7));
  }

  {
    rcPolygon p;

    rcUNITTEST_ASSERT(rfRealEq(p.area(), 0));
    rcUNITTEST_ASSERT(rfRealEq(p.perimeter(), 0));
  }

  {
    rcPolygon p;
    p.pushVertex(p0);

    rcUNITTEST_ASSERT(rfRealEq(p.area(), 0));
    rcUNITTEST_ASSERT(rfRealEq(p.perimeter(), 0));
  }

  {
    rcPolygon p;
    p.pushVertex(p0); p.pushVertex(p1);

    rcUNITTEST_ASSERT(rfRealEq(p.area(), 0));
    rcUNITTEST_ASSERT(rfRealEq(p.perimeter(), 16));
  }
}

void UT_Polygon::testConvexHullConvert()
{
  rc2Dvector p0(0,0), p1(1,-2), p2(0,1), p3(3,-2), p4(4,2), p5(5,1);
  rc2Dvector p6(7,4), p7(6,5), p8(3,3), p9(3,5), p10(-2,2), p11(2,5);
  rc2Dvector p12(0,5), p13(-3,4), p14(-5,2), p15(-3,2), p16(-5,1);
  rc2Dvector p17(-5,-1), p18(-3,-2);

  {
    rcPolygon p;

    p.convexHullConvert();

    rcUNITTEST_ASSERT(p.vertexCnt() == 0);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    rcPolygon hcp;
    
    p.convexHull(hcp);

    rcUNITTEST_ASSERT(p == hcp);
  }

  {
    rcPolygon p;
    
    p.pushVertex(p4);

    p.convexHullConvert();

    rcUNITTEST_ASSERT(p.vertexCnt() == 1);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p4);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.pushVertex(p17);

    p.convexHullConvert();

    rcUNITTEST_ASSERT(p.vertexCnt() == 2);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p4);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p17);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);


    p.pushVertex(p18);
    rcUNITTEST_ASSERT(p.isConvex() == true);
    
    p.convexHullConvert();

    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    rcPolygon hp;
    hp.pushVertex(p4); hp.pushVertex(p17); hp.pushVertex(p18);
    
    rcUNITTEST_ASSERT(p == hp);

    rcPolygon hcp;
    
    p.convexHull(hcp);

    rcUNITTEST_ASSERT(p == hcp);
  }

  {
    rcPolygon p;
    
    p.pushVertex(p4); p.pushVertex(p18); p.pushVertex(p17);
    rcUNITTEST_ASSERT(p.isConvex() == false);
    
    p.convexHullConvert();

    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    rcPolygon hp;
    hp.pushVertex(p4); hp.pushVertex(p17); hp.pushVertex(p18);
    
    rcUNITTEST_ASSERT(p == hp);

    rcPolygon hcp;
    
    p.convexHull(hcp);

    rcUNITTEST_ASSERT(p == hcp);
  }

  {
    rcPolygon p;

    p.pushVertex(p0); p.pushVertex(p1); p.pushVertex(p2); p.pushVertex(p4);
    p.pushVertex(p8); p.pushVertex(p9); p.pushVertex(p11); p.pushVertex(p12);
    p.pushVertex(p10);
    
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == false);
    
    p.convexHullConvert();
  
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    rcPolygon hp;
    hp.pushVertex(p1); hp.pushVertex(p4); hp.pushVertex(p9); hp.pushVertex(p12);
    hp.pushVertex(p10);
    
    rcUNITTEST_ASSERT(p == hp);

    rcPolygon hcp;
    
    p.convexHull(hcp);

    rcUNITTEST_ASSERT(p == hcp);
  }

  {
    rcPolygon p;

    p.pushVertex(p0); p.pushVertex(p1); p.pushVertex(p2); p.pushVertex(p3);
    p.pushVertex(p4); p.pushVertex(p5); p.pushVertex(p6); p.pushVertex(p7);
    p.pushVertex(p8); p.pushVertex(p9); p.pushVertex(p10); p.pushVertex(p11);
    p.pushVertex(p12); p.pushVertex(p13); p.pushVertex(p14); p.pushVertex(p15);
    p.pushVertex(p16); p.pushVertex(p17); p.pushVertex(p18);
    
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == false);
    
    p.convexHullConvert();
  
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    rcPolygon hp;
    hp.pushVertex(p3); hp.pushVertex(p6); hp.pushVertex(p7); hp.pushVertex(p12);
    hp.pushVertex(p13); hp.pushVertex(p14); hp.pushVertex(p17); hp.pushVertex(p18);
    
    rcUNITTEST_ASSERT(p == hp);

    rcPolygon hcp;
    
    p.convexHull(hcp);

    rcUNITTEST_ASSERT(p == hcp);
  }
}

void UT_Polygon::testIntersects()
{
  /* Use n and o to test case where the number of vertices in the polgons
   * is < 3.
   */
  {
    rc2Dvector o0(0,0), o1(3,0), o2(1,2), o3(1,1), o4(1,0), o5(5,5);
    rc2Dvector o6(5,0), o7(2,2), o8(6,0), o9(6,6);

    rcPolygon o, n;

    /* Test case of both polygons being empty.
     */
    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!o.intersects(o));

    /* Test cases with 1 polygon empty and 1 polygon having 1 vertex.
     */
    o.pushVertex(o1); // O = { o1 }

    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!n.intersects(o));

    o.popVertex(); o.pushVertex(o0); // O = { o0 }

    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!n.intersects(o));

    /* Test cases with 1 polygon empty and 1 polygon having 2 vertices.
     */
    o.pushVertex(o1); // O = { o0 o1 }

    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!n.intersects(o));

    /* Test cases with both polygons having 1 vertex.
     */
    o.popVertex(); // O = { o0 }
    n.pushVertex(o1); // N = { o1 }

    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!n.intersects(o));

    n.popVertex(); n.pushVertex(o0); // N = { o0 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    /* Test cases with 1 polygon having 1 vertex and 1 polygon
     * having >= 2 vertices.
     */
    o.pushVertex(o1); // O = { o0 o1 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    n.popVertex(); n.pushVertex(o1); // N = { o1 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    n.popVertex(); n.pushVertex(o4); // N = { o4 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    o.pushVertex(o2); // O = { o0 o1 o2 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    n.popVertex(); n.pushVertex(o5); // N = { o5 }

    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!n.intersects(o));

    /* Test cases with both polygons having 2 vertices.
     */
    o.popVertex(); // O = { o0 o1 }
    n.popVertex(); // N = { }
    n.pushVertex(o4); n.pushVertex(o6); // N = { o4 o6 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    o.popVertex(); o.pushVertex(o5); // O = { o0 o5 }
    n.popVertex(); n.pushVertex(o2); // N = { o4 o2 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    o.popVertex(); o.pushVertex(o2); // O = { o0 o2 }
    n.popVertex(); n.pushVertex(o3); // N = { o4 o3 }

    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!n.intersects(o));

    n.popVertex(); n.pushVertex(o0); // N = { o4 o0 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    o.popVertex(); o.pushVertex(o6); // O = { o0 o6 }
    n.popVertex(); n.pushVertex(o1); // N = { o4 o1 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    o.popVertex(); o.pushVertex(o4); // O = { o0 o4 }
    n.deleteVertex(0); n.pushVertex(o6); // N = { o1 o6 }

    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!n.intersects(o));

    n.popVertex(); n.popVertex(); // N = { }
    n.pushVertex(o2); n.pushVertex(o7); // N = { o2 o7 }

    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!n.intersects(o));

    n.popVertex(); n.popVertex(); // N = { }
    n.pushVertex(o4); n.pushVertex(o0); // N = { o4 o0 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    n.popVertex(); n.popVertex(); // N = { }
    n.pushVertex(o0); n.pushVertex(o4); // N = { o0 o4 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));
    rcUNITTEST_ASSERT(o.intersects(o));

    /* Test cases where one polygon has 2 vertices and the other > 2
     * vertices.
     */
    o.popVertex(); o.pushVertex(o2); // O = { o0 o2 }
    o.pushVertex(o5); o.pushVertex(o6); // O = { o0 o2 o5 o6 }
    n.deleteVertex(0); n.pushVertex(o2); // N = { o0 o2 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    n.popVertex(); n.pushVertex(o5); // N = { o0 o5 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    n.popVertex(); n.popVertex(); // N = { }
    n.pushVertex(o4); n.pushVertex(o1); // N = { o4 o1 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    n.popVertex(); n.insertVertex(o3, n.vertexCnt()); // N = { o3 o4 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    n.popVertex(); n.pushVertex(o7); // N = { o3 o7 }

    rcUNITTEST_ASSERT(o.intersects(n));
    rcUNITTEST_ASSERT(n.intersects(o));

    n.popVertex(); n.popVertex(); // N = { }
    n.pushVertex(o8); n.pushVertex(o9); // N = { o8 o9 }

    rcUNITTEST_ASSERT(!o.intersects(n));
    rcUNITTEST_ASSERT(!n.intersects(o));
  }

  rc2Dvector p0(4,-6), p1(10,-4), p2(10,4), p3(4,6), p4(-2,6);
  rc2Dvector p5(-4,2), p6(-6,-4), p7(-4,-6), p7p(-4,-4);

  rc2Dvector c0(-40,-40), c1(60,-40), c2(60, 70), c3(-40, 70);

  rc2Dvector b01((p0.x() + (p1.x() - p0.x())/2), (p0.y() + (p1.y() - p0.y())/2));
  rc2Dvector b12((p1.x() + (p2.x() - p1.x())/2), (p1.y() + (p2.y() - p1.y())/2));
  rc2Dvector b45((p4.x() + (p5.x() - p4.x())/2), (p4.y() + (p5.y() - p4.y())/2));
  rc2Dvector b56((p5.x() + (p6.x() - p5.x())/2), (p5.y() + (p6.y() - p5.y())/2));
  rc2Dvector b67p((p6.x() + (p7p.x() - p6.x())/2),
		  (p6.y() + (p7p.y() - p6.y())/2));
  rc2Dvector b7p0((p7p.x() + (p0.x() - p7p.x())/2),
		  (p7p.y() + (p0.y() - p7p.y())/2));

  rc2Dvector q0(16,6), q1(22,8), q2(18,12), q3(12,10);

  rcPolygon convex;
  convex.pushVertex(p0); convex.pushVertex(p1); convex.pushVertex(p2);
  convex.pushVertex(p3); convex.pushVertex(p4); convex.pushVertex(p5);
  convex.pushVertex(p6); convex.pushVertex(p7);

  rcPolygon valid;
  valid.pushVertex(p0); valid.pushVertex(p1); valid.pushVertex(p2);
  valid.pushVertex(p3); valid.pushVertex(p4); valid.pushVertex(p5);
  valid.pushVertex(p6); valid.pushVertex(p7p);

  rcPolygon contains;
  contains.pushVertex(c0); contains.pushVertex(c1);
  contains.pushVertex(c2); contains.pushVertex(c3);

  for (int32 ci = 0; ci < convex.vertexCnt(); ci++) {
    rc2Dvector t = convex.vertexAt(0); // Rotate polygon by 1 position
    convex.deleteVertex(0);
    convex.pushVertex(t);
    rcUNITTEST_ASSERT(convex.isConvex());

    t = valid.vertexAt(0); // Rotate polygon by 1 position
    valid.deleteVertex(0);
    valid.pushVertex(t);
    rcUNITTEST_ASSERT(valid.isValid());
    rcUNITTEST_ASSERT(!valid.isConvex());

    rcPolygon p;
    p.pushVertex(q0); p.pushVertex(q1); p.pushVertex(q2); p.pushVertex(q3); 

    for (int32 i = 0; i < p.vertexCnt(); i++) {
      rc2Dvector temp = p.vertexAt(0);
      p.deleteVertex(0);
      p.pushVertex(temp);
      rcUNITTEST_ASSERT(p.isConvex());

      {
	rcPolygon outside(p);

	rcUNITTEST_ASSERT(!convex.intersects(outside));
	rcUNITTEST_ASSERT(!valid.intersects(outside));
      }

      {
	rcPolygon outside(p);
	outside.translate(rc2Dvector(-16,2));

	rcUNITTEST_ASSERT(!convex.intersects(outside));
	rcUNITTEST_ASSERT(!valid.intersects(outside));
      }

      {
	rcPolygon outside(p);
	outside.translate(rc2Dvector(-30,0));

	rcUNITTEST_ASSERT(!convex.intersects(outside));
	rcUNITTEST_ASSERT(!valid.intersects(outside));
      }

      {
	rcPolygon outside(p);
	outside.translate(rc2Dvector(-34,-10));

	rcUNITTEST_ASSERT(!convex.intersects(outside));
	rcUNITTEST_ASSERT(!valid.intersects(outside));
      }

      {
	rcPolygon outside(p);
	outside.translate(rc2Dvector(-22,-28));

	rcUNITTEST_ASSERT(!convex.intersects(outside));
	rcUNITTEST_ASSERT(!valid.intersects(outside));
      }

      {
	rcPolygon outside(p);
	outside.translate(rc2Dvector(-20,-16));

	rcUNITTEST_ASSERT(convex.intersects(outside));
	rcUNITTEST_ASSERT(valid.intersects(outside));
      }

      {
	rcPolygon outside(p);
	outside.translate(rc2Dvector(-20,-4));

	rcUNITTEST_ASSERT(convex.intersects(outside));
	rcUNITTEST_ASSERT(valid.intersects(outside));
      }

      {
	rcPolygon outside(p);
	outside.translate(rc2Dvector(6,-10));

	rcUNITTEST_ASSERT(!convex.intersects(outside));
	rcUNITTEST_ASSERT(!valid.intersects(outside));
      }

      {
	rcPolygon outside(p);
	outside.translate(rc2Dvector(-10,-14));

	rcUNITTEST_ASSERT(convex.intersects(outside));
	rcUNITTEST_ASSERT(valid.intersects(outside));
      }

      {
	rcPolygon outside(p);
	outside.translate(rc2Dvector(-8,-16));

	rcUNITTEST_ASSERT(convex.intersects(outside));
	rcUNITTEST_ASSERT(valid.intersects(outside));
      }

      {
	rcPolygon inside(p);
	inside.translate(rc2Dvector(-14,-10));

	rcUNITTEST_ASSERT(convex.intersects(inside));
	rcUNITTEST_ASSERT(valid.intersects(inside));
      }

      {
	rcPolygon inside(p);
	inside.translate(rc2Dvector(-16,-12));

	rcUNITTEST_ASSERT(convex.intersects(inside));
	rcUNITTEST_ASSERT(valid.intersects(inside));
      }

      {
	rcPolygon inside(p);
	inside.translate(rc2Dvector(-12,-12));

	rcUNITTEST_ASSERT(convex.intersects(inside));
	rcUNITTEST_ASSERT(valid.intersects(inside));
      }
    } // End of: for (int32 i = 0; i < p.vertexCnt(); i++) {

    {
      rcUNITTEST_ASSERT(convex.intersects(convex));
      rcUNITTEST_ASSERT(convex.intersects(valid));
      rcUNITTEST_ASSERT(valid.intersects(convex));
      rcUNITTEST_ASSERT(valid.intersects(valid));
    }

    for (int32 i = 0; i < contains.vertexCnt(); i++) {
      rc2Dvector temp = contains.vertexAt(0); // Rotate polygon by 1 position
      contains.deleteVertex(0);
      contains.pushVertex(temp);
      rcUNITTEST_ASSERT(contains.isConvex());

      {
	rcUNITTEST_ASSERT(valid.intersects(contains));
	rcUNITTEST_ASSERT(convex.intersects(contains));
      }
    }

    rcPolygon inEdge;
    inEdge.pushVertex(p1); inEdge.pushVertex(p2); inEdge.pushVertex(p4);
    inEdge.pushVertex(p5);

    for (int32 i = 0; i < inEdge.vertexCnt(); i++) {
      rc2Dvector temp = inEdge.vertexAt(0); // Rotate polygon by 1 position
      inEdge.deleteVertex(0);
      inEdge.pushVertex(temp);
      rcUNITTEST_ASSERT(inEdge.isConvex());

      {
	rcUNITTEST_ASSERT(valid.intersects(inEdge));
	rcUNITTEST_ASSERT(convex.intersects(inEdge));
      }
    }

    rcPolygon inEdge2, inEdge3, inEdge4;
    inEdge2.pushVertex(b12); inEdge2.pushVertex(p2);
    inEdge2.pushVertex(b45); inEdge2.pushVertex(p5);
    inEdge3.pushVertex(b12); inEdge3.pushVertex(b45);
    inEdge3.pushVertex(p5); inEdge3.pushVertex(p1);
    inEdge4.pushVertex(b01); inEdge4.pushVertex(b12);
    inEdge4.pushVertex(b45); inEdge4.pushVertex(b56);

    for (int32 i = 0; i < inEdge2.vertexCnt(); i++) {
      rc2Dvector temp = inEdge2.vertexAt(0); // Rotate polygon by 1 position
      inEdge2.deleteVertex(0);
      inEdge2.pushVertex(temp);
      rcUNITTEST_ASSERT(inEdge2.isConvex());
      
      temp = inEdge3.vertexAt(0); // Rotate polygon by 1 position
      inEdge3.deleteVertex(0);
      inEdge3.pushVertex(temp);
      rcUNITTEST_ASSERT(inEdge3.isConvex());
      
      temp = inEdge4.vertexAt(0); // Rotate polygon by 1 position
      inEdge4.deleteVertex(0);
      inEdge4.pushVertex(temp);
      rcUNITTEST_ASSERT(inEdge4.isConvex());
      
      {
	rcUNITTEST_ASSERT(valid.intersects(inEdge2));
	rcUNITTEST_ASSERT(convex.intersects(inEdge2));
	rcUNITTEST_ASSERT(valid.intersects(inEdge3));
	rcUNITTEST_ASSERT(convex.intersects(inEdge3));
	rcUNITTEST_ASSERT(valid.intersects(inEdge4));
	rcUNITTEST_ASSERT(convex.intersects(inEdge4));
      }
    }

    rcPolygon outEdge, outEdge2;
    outEdge.pushVertex(p1); outEdge.pushVertex(p2); outEdge.pushVertex(p5);
    outEdge.pushVertex(p6); outEdge.pushVertex(p0);
    outEdge2.pushVertex(p6); outEdge2.pushVertex(p0);
    outEdge2.pushVertex(b01); outEdge2.pushVertex(b56);

    for (int32 i = 0; i < outEdge.vertexCnt(); i++) {
      rc2Dvector temp = outEdge.vertexAt(0); // Rotate polygon by 1 position
      outEdge.deleteVertex(0);
      outEdge.pushVertex(temp);
      rcUNITTEST_ASSERT(outEdge.isValid());
      
      temp = outEdge2.vertexAt(0); // Rotate polygon by 1 position
      outEdge2.deleteVertex(0);
      outEdge2.pushVertex(temp);
      rcUNITTEST_ASSERT(outEdge2.isValid());
      
      {
	rcUNITTEST_ASSERT(valid.intersects(outEdge));
	rcUNITTEST_ASSERT(valid.intersects(outEdge2));
      }
    }

    rcPolygon outEdge3, outEdge4;
    outEdge3.pushVertex(b67p); outEdge3.pushVertex(b7p0);
    outEdge3.pushVertex(p7p);
    outEdge4.pushVertex(p6); outEdge4.pushVertex(p0); outEdge4.pushVertex(p7p);
      
    for (int32 i = 0; i < outEdge3.vertexCnt(); i++) {
      rc2Dvector temp = outEdge3.vertexAt(0); // Rotate polygon by 1 position
      outEdge3.deleteVertex(0);
      outEdge3.pushVertex(temp);
      rcUNITTEST_ASSERT(outEdge3.isValid());

      temp = outEdge4.vertexAt(0); // Rotate polygon by 1 position
      outEdge4.deleteVertex(0);
      outEdge4.pushVertex(temp);
      rcUNITTEST_ASSERT(outEdge4.isValid());
      
      {
	rcUNITTEST_ASSERT(valid.intersects(outEdge3));
	rcUNITTEST_ASSERT(valid.intersects(outEdge4));
      }
    }
  } // for (int32 ci = 0; ci < convex.vertexCnt(); ci++) {

  {
    rc2Dvector atc0(0,0), atc1(10,0), atc2(10,10), atc3(0,10);
    rc2Dvector atp0(1,1), atp1(10,-10), atp2(10,10), atp3(-10,10);
    rc2Dvector ai0(10,8), ai1(5,8), ai2(5,5), ai3(10,5);

    rcPolygon amyTC;
    amyTC.pushVertex(atc0); amyTC.pushVertex(atc1); amyTC.pushVertex(atc2);
    amyTC.pushVertex(atc3);

    rcPolygon amyTP;
    amyTP.pushVertex(atp0); amyTP.pushVertex(atp1); amyTP.pushVertex(atp2);
    amyTP.pushVertex(atp3);

    rcPolygon amyI;
    amyI.pushVertex(ai0); amyI.pushVertex(ai1); amyI.pushVertex(ai2);
    amyI.pushVertex(ai3);
      
    for (int32 ci = 0; ci < amyTC.vertexCnt(); ci++) {
      rc2Dvector t = amyTC.vertexAt(0); // Rotate polygon by 1 position
      amyTC.deleteVertex(0);
      amyTC.pushVertex(t);
      rcUNITTEST_ASSERT(amyTC.isConvex());

      t = amyTP.vertexAt(0); // Rotate polygon by 1 position
      amyTP.deleteVertex(0);
      amyTP.pushVertex(t);
      rcUNITTEST_ASSERT(amyTP.isValid());
      rcUNITTEST_ASSERT(!amyTP.isConvex());

      for (int32 i = 0; i < amyI.vertexCnt(); i++) {
	rc2Dvector temp = amyI.vertexAt(0); // Rotate polygon by 1 position
	amyI.deleteVertex(0);
	amyI.pushVertex(temp);
	rcUNITTEST_ASSERT(amyI.isConvex());

	{
	  rcUNITTEST_ASSERT(amyTC.intersects(amyI));
	  rcUNITTEST_ASSERT(amyTP.intersects(amyI));
	}
      }
    }
  }

  {
    rc2Dvector stc0(0,0), stc1(10,0), stc2(10,10), stc3(0,10);
    rc2Dvector stp0(1,1), stp1(10,-10), stp2(10,10), stp3(-10,10);
    rc2Dvector so0(5,5), so1(15,5), so2(15,15), so3(5,15);

    rcPolygon sqsqTC;
    sqsqTC.pushVertex(stc0); sqsqTC.pushVertex(stc1); sqsqTC.pushVertex(stc2);
    sqsqTC.pushVertex(stc3);

    rcPolygon sqsqTP;
    sqsqTP.pushVertex(stp0); sqsqTP.pushVertex(stp1); sqsqTP.pushVertex(stp2);
    sqsqTP.pushVertex(stp3);

    rcPolygon sqsqO;
    sqsqO.pushVertex(so0); sqsqO.pushVertex(so1); sqsqO.pushVertex(so2);
    sqsqO.pushVertex(so3);
      
    for (int32 ci = 0; ci < sqsqTC.vertexCnt(); ci++) {
      rc2Dvector t = sqsqTC.vertexAt(0); // Rotate polygon by 1 position
      sqsqTC.deleteVertex(0);
      sqsqTC.pushVertex(t);
      rcUNITTEST_ASSERT(sqsqTC.isConvex());

      t = sqsqTP.vertexAt(0); // Rotate polygon by 1 position
      sqsqTP.deleteVertex(0);
      sqsqTP.pushVertex(t);
      rcUNITTEST_ASSERT(sqsqTP.isValid());
      rcUNITTEST_ASSERT(!sqsqTP.isConvex());

      for (int32 i = 0; i < sqsqO.vertexCnt(); i++) {
	rc2Dvector temp = sqsqO.vertexAt(0); // Rotate polygon by 1 position
	sqsqO.deleteVertex(0);
	sqsqO.pushVertex(temp);
	rcUNITTEST_ASSERT(sqsqO.isConvex());

	{
	  rcUNITTEST_ASSERT(sqsqTC.intersects(sqsqO));
	  rcUNITTEST_ASSERT(sqsqTP.intersects(sqsqO));
	}
      }
    }
  }

  {
    rc2Dvector htc0(0,0), htc1(20,0), htc2(20,10), htc3(10,20), htc4(0, 10);
    rc2Dvector htp0(10,4), htp1(20,-10), htp2(20,10), htp3(10,20), htp4(0,10);
    rc2Dvector ho0(10,10), ho1(30,10), ho2(30,20), ho3(10,20);

    rcPolygon houseTC;
    houseTC.pushVertex(htc0); houseTC.pushVertex(htc1); houseTC.pushVertex(htc2);
    houseTC.pushVertex(htc3); houseTC.pushVertex(htc4);

    rcPolygon houseTP;
    houseTP.pushVertex(htp0); houseTP.pushVertex(htp1); houseTP.pushVertex(htp2);
    houseTP.pushVertex(htp3); houseTP.pushVertex(htp4);

    rcPolygon houseO;
    houseO.pushVertex(ho0); houseO.pushVertex(ho1); houseO.pushVertex(ho2);
    houseO.pushVertex(ho3);
      
    for (int32 ci = 0; ci < houseTC.vertexCnt(); ci++) {
      rc2Dvector t = houseTC.vertexAt(0); // Rotate polygon by 1 position
      houseTC.deleteVertex(0);
      houseTC.pushVertex(t);
      rcUNITTEST_ASSERT(houseTC.isConvex());

      t = houseTP.vertexAt(0); // Rotate polygon by 1 position
      houseTP.deleteVertex(0);
      houseTP.pushVertex(t);
      rcUNITTEST_ASSERT(houseTP.isValid());
      rcUNITTEST_ASSERT(!houseTP.isConvex());

      for (int32 i = 0; i < houseO.vertexCnt(); i++) {
	rc2Dvector temp = houseO.vertexAt(0); // Rotate polygon by 1 position
	houseO.deleteVertex(0);
	houseO.pushVertex(temp);
	rcUNITTEST_ASSERT(houseO.isConvex());

	{
	  rcUNITTEST_ASSERT(houseTC.intersects(houseO));
	  rcUNITTEST_ASSERT(houseTP.intersects(houseO));
	}
      }
    }
  }

  {
    rc2Dvector tt0(0,16), tt1(5,8), tt2(13,0), tt3(19,2), tt4(24,10);
    rc2Dvector tt5(24,26), tt6(19,29), tt7(13,32), tt8(7,32), tt9(3,29);
    rc2Dvector to0(16,-3), to1(28,16), to2(16,32), to3(0,22), to4(3,10);

    rcPolygon twoT;
    twoT.pushVertex(tt0); twoT.pushVertex(tt1); twoT.pushVertex(tt2);
    twoT.pushVertex(tt3); twoT.pushVertex(tt4); twoT.pushVertex(tt5);
    twoT.pushVertex(tt6); twoT.pushVertex(tt7); twoT.pushVertex(tt8);
    twoT.pushVertex(tt9);

    rcPolygon twoO;
    twoO.pushVertex(to0); twoO.pushVertex(to1); twoO.pushVertex(to2);
    twoO.pushVertex(to3); twoO.pushVertex(to4);
      
    for (int32 ci = 0; ci < twoT.vertexCnt(); ci++) {
      rc2Dvector t = twoT.vertexAt(0); // Rotate polygon by 1 position
      twoT.deleteVertex(0);
      twoT.pushVertex(t);
      rcUNITTEST_ASSERT(twoT.isConvex()); 

      for (int32 i = 0; i < twoO.vertexCnt(); i++) {
	rc2Dvector temp = twoO.vertexAt(0); // Rotate polygon by 1 position
	twoO.deleteVertex(0);
	twoO.pushVertex(temp);
	rcUNITTEST_ASSERT(twoO.isConvex());

	{
	  rcUNITTEST_ASSERT(twoT.intersects(twoO));
	  rcUNITTEST_ASSERT(twoO.intersects(twoT));
	}
      }
    }
  }
}

void UT_Polygon::testContains()
{

  /* Test point contains fct.
   */
  {
    rc2Dvector p0(2,-3), p1(4,-1), p2(7,-1), p3(9,-3), p3a(9,-1), p3b(11,-1);
    rc2Dvector p4(9,4), p5(6,5), p6(-4,5), p7(-7,2), p8(-7,-1), p9(-10,-5);
    rc2Dvector p10(-5,-4), p11(-3,-1);

    /* Use o to test case where the number of vertices in the polgon
     * is < 3.
     */
    {
      rcPolygon o;
      rcUNITTEST_ASSERT(!o.contains(p2));
      rcUNITTEST_ASSERT(!o.contains(p3b));
      
      o.pushVertex(p2);
      rcUNITTEST_ASSERT(o.contains(p2));
      rcUNITTEST_ASSERT(!o.contains(p3b));
      
      o.pushVertex(p3b);
      rcUNITTEST_ASSERT(o.contains(p2));
      rcUNITTEST_ASSERT(o.contains(p3a));
      rcUNITTEST_ASSERT(o.contains(p3b));
      rcUNITTEST_ASSERT(!o.contains(p1));
      rcUNITTEST_ASSERT(!o.contains(p0));
    }

    rcPolygon p;
    
    p.pushVertex(p0); p.pushVertex(p1); p.pushVertex(p2); p.pushVertex(p3);
    p.pushVertex(p3a);  p.pushVertex(p3b); p.pushVertex(p4); p.pushVertex(p5);
    p.pushVertex(p6); p.pushVertex(p7); p.pushVertex(p8); p.pushVertex(p9);
    p.pushVertex(p10); p.pushVertex(p11);

    rc2Dvector b0((p0.x() + (p1.x() - p0.x())/2), (p0.y() + (p1.y() - p0.y())/2));
    rc2Dvector b1((p1.x() + (p2.x() - p1.x())/2), (p1.y() + (p2.y() - p1.y())/2));
    rc2Dvector b2((p2.x() + (p3.x() - p2.x())/2), (p2.y() + (p3.y() - p2.y())/2));
    rc2Dvector b3((p3.x() + (p3a.x() - p3.x())/2), (p3.y() + (p3a.y() - p3.y())/2));
    rc2Dvector b3a((p3a.x() + (p3b.x() - p3a.x())/2),
		   (p3a.y() + (p3b.y() - p3a.y())/2));
    rc2Dvector b3b((p3b.x() + (p4.x() - p3b.x())/2),
		   (p3b.y() + (p4.y() - p3b.y())/2));
    rc2Dvector b4((p4.x() + (p5.x() - p4.x())/2), (p4.y() + (p5.y() - p4.y())/2));
    rc2Dvector b5((p5.x() + (p6.x() - p5.x())/2), (p5.y() + (p6.y() - p5.y())/2));
    rc2Dvector b6((p6.x() + (p7.x() - p6.x())/2), (p6.y() + (p7.y() - p6.y())/2));
    rc2Dvector b7((p7.x() + (p8.x() - p7.x())/2), (p7.y() + (p8.y() - p7.y())/2));
    rc2Dvector b8((p8.x() + (p9.x() - p8.x())/2), (p8.y() + (p9.y() - p8.y())/2));
    rc2Dvector b9((p9.x() + (p10.x() - p9.x())/2), (p9.y() + (p10.y() - p9.y())/2));
    rc2Dvector b10((p10.x() + (p11.x() - p10.x())/2),
		   (p10.y() + (p11.y() - p10.y())/2));
    rc2Dvector b11((p11.x() + (p0.x() - p11.x())/2),
		   (p11.y() + (p0.y() - p11.y())/2));

    rc2Dvector i0(-8,-4), i1(-6,2), i2(-1,2), i3(1,4), i4(7,4);
    rc2Dvector i5(8,2), i6(1,-2), i7(8,-1), i8(8.5,2.0), i9(-5,-1);
    rc2Dvector i10(0, -1);

    rc2Dvector o0(-9,10), o1(2,6), o2(8,5), o3(14,10), o4(10,2);
    rc2Dvector o5(12,-8), o6(6,-2), o7(-3,-3), o8(-5,-5), o9(-12,-7);
    rc2Dvector o10(-11,-1), o11(-8,1), o12(-6,4);

    rcUNITTEST_ASSERT(p.contains(p0));
    rcUNITTEST_ASSERT(p.contains(p1));
    rcUNITTEST_ASSERT(p.contains(p2));
    rcUNITTEST_ASSERT(p.contains(p3));
    rcUNITTEST_ASSERT(p.contains(p3a));
    rcUNITTEST_ASSERT(p.contains(p3b));
    rcUNITTEST_ASSERT(p.contains(p4));
    rcUNITTEST_ASSERT(p.contains(p5));
    rcUNITTEST_ASSERT(p.contains(p6));
    rcUNITTEST_ASSERT(p.contains(p7));
    rcUNITTEST_ASSERT(p.contains(p8));
    rcUNITTEST_ASSERT(p.contains(p9));
    rcUNITTEST_ASSERT(p.contains(p10));
    rcUNITTEST_ASSERT(p.contains(p11));

    rcUNITTEST_ASSERT(p.contains(b0));
    rcUNITTEST_ASSERT(p.contains(b1));
    rcUNITTEST_ASSERT(p.contains(b2));
    rcUNITTEST_ASSERT(p.contains(b3));
    rcUNITTEST_ASSERT(p.contains(b3a));
    rcUNITTEST_ASSERT(p.contains(b3b));
    rcUNITTEST_ASSERT(p.contains(b4));
    rcUNITTEST_ASSERT(p.contains(b5));
    rcUNITTEST_ASSERT(p.contains(b6));
    rcUNITTEST_ASSERT(p.contains(b7));
    rcUNITTEST_ASSERT(p.contains(b8));
    rcUNITTEST_ASSERT(p.contains(b9));
    rcUNITTEST_ASSERT(p.contains(b10));
    rcUNITTEST_ASSERT(p.contains(b11));

    rcUNITTEST_ASSERT(p.contains(i0));
    rcUNITTEST_ASSERT(p.contains(i1));
    rcUNITTEST_ASSERT(p.contains(i2));
    rcUNITTEST_ASSERT(p.contains(i3));
    rcUNITTEST_ASSERT(p.contains(i4));
    rcUNITTEST_ASSERT(p.contains(i5));
    rcUNITTEST_ASSERT(p.contains(i6));
    rcUNITTEST_ASSERT(p.contains(i7));
    rcUNITTEST_ASSERT(p.contains(i8));
    rcUNITTEST_ASSERT(p.contains(i9));
    rcUNITTEST_ASSERT(p.contains(i10));

    rcUNITTEST_ASSERT(!p.contains(o0));
    rcUNITTEST_ASSERT(!p.contains(o1));
    rcUNITTEST_ASSERT(!p.contains(o2));
    rcUNITTEST_ASSERT(!p.contains(o3));
    rcUNITTEST_ASSERT(!p.contains(o4));
    rcUNITTEST_ASSERT(!p.contains(o5));
    rcUNITTEST_ASSERT(!p.contains(o6));
    rcUNITTEST_ASSERT(!p.contains(o7));
    rcUNITTEST_ASSERT(!p.contains(o8));
    rcUNITTEST_ASSERT(!p.contains(o9));
    rcUNITTEST_ASSERT(!p.contains(o10));
    rcUNITTEST_ASSERT(!p.contains(o11));
    rcUNITTEST_ASSERT(!p.contains(o12));
  }

  /* Test polygon contain fct
   */
  {
    rc2Dvector p0(4,-6), p1(10,-4), p2(10,4), p3(4,6), p4(-2,6);
    rc2Dvector p5(-4,2), p6(-6,-4), p7(-4,-6), p7p(-4,-4);

    rc2Dvector c0(-40,-40), c1(60,-40), c2(60, 70), c3(-40, 70);

    rc2Dvector b01((p0.x() + (p1.x() - p0.x())/2), (p0.y() + (p1.y() - p0.y())/2));
    rc2Dvector b12((p1.x() + (p2.x() - p1.x())/2), (p1.y() + (p2.y() - p1.y())/2));
    rc2Dvector b45((p4.x() + (p5.x() - p4.x())/2), (p4.y() + (p5.y() - p4.y())/2));
    rc2Dvector b56((p5.x() + (p6.x() - p5.x())/2), (p5.y() + (p6.y() - p5.y())/2));
    rc2Dvector b67p((p6.x() + (p7p.x() - p6.x())/2),
		    (p6.y() + (p7p.y() - p6.y())/2));
    rc2Dvector b7p0((p7p.x() + (p0.x() - p7p.x())/2),
		    (p7p.y() + (p0.y() - p7p.y())/2));

    rc2Dvector q0(16,6), q1(22,8), q2(18,12), q3(12,10);

    /* Use n and o to test case where the number of vertices in the polgons
     * is < 3.
     */
    {
      rc2Dvector o0(0,0), o1(3,0), o2(1,2), o3(1,1), o4(1,0), o5(5,5);
      rc2Dvector o6(5,0), o7(2,2), o8(6,0), o9(6,6);

      rcPolygon o, n;

      /* Test case of both polygons being empty.
       */
      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(o.contains(o));

      /* Test cases with 1 polygon empty and 1 polygon having 1 vertex.
       */
      o.pushVertex(o1); // O = { o1 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      o.popVertex(); o.pushVertex(o0); // O = { o0 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      /* Test cases with 1 polygon empty and 1 polygon having 2 vertices.
       */
      o.pushVertex(o1); // O = { o0 o1 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      /* Test cases with both polygons having 1 vertex.
       */
      o.popVertex(); // O = { o0 }
      n.pushVertex(o1); // N = { o1 }

      rcUNITTEST_ASSERT(!o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.pushVertex(o0); // N = { o0 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(n.contains(o));

      /* Test cases with 1 polygon having 1 vertex and 1 polygon
       * having >= 2 vertices.
       */
      o.pushVertex(o1); // O = { o0 o1 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.pushVertex(o1); // N = { o1 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.pushVertex(o4); // N = { o4 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      o.pushVertex(o2); // O = { o0 o1 o2 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.pushVertex(o5); // N = { o5 }

      rcUNITTEST_ASSERT(!o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      /* Test cases with both polygons having 2 vertices.
       */
      o.popVertex(); // O = { o0 o1 }
      n.popVertex(); // N = { }
      n.pushVertex(o4); n.pushVertex(o6); // N = { o4 o6 }

      rcUNITTEST_ASSERT(!o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      o.popVertex(); o.pushVertex(o5); // O = { o0 o5 }
      n.popVertex(); n.pushVertex(o2); // N = { o4 o2 }

      rcUNITTEST_ASSERT(!o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      o.popVertex(); o.pushVertex(o2); // O = { o0 o2 }
      n.popVertex(); n.pushVertex(o3); // N = { o4 o3 }

      rcUNITTEST_ASSERT(!o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.pushVertex(o0); // N = { o4 o0 }

      rcUNITTEST_ASSERT(!o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      o.popVertex(); o.pushVertex(o6); // O = { o0 o6 }
      n.popVertex(); n.pushVertex(o1); // N = { o4 o1 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      o.popVertex(); o.pushVertex(o4); // O = { o0 o4 }
      n.deleteVertex(0); n.pushVertex(o6); // N = { o1 o6 }

      rcUNITTEST_ASSERT(!o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o2); n.pushVertex(o7); // N = { o2 o7 }

      rcUNITTEST_ASSERT(!o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o4); n.pushVertex(o0); // N = { o4 o0 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(n.contains(o));

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o0); n.pushVertex(o4); // N = { o0 o4 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(n.contains(o));
      rcUNITTEST_ASSERT(o.contains(o));

      /* Test cases where one polygon has 2 vertices and the other > 2
       * vertices.
       */
      o.popVertex(); o.pushVertex(o2); // O = { o0 o2 }
      o.pushVertex(o5); o.pushVertex(o6); // O = { o0 o2 o5 o6 }
      n.deleteVertex(0); n.pushVertex(o2); // N = { o0 o2 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.pushVertex(o5); // N = { o0 o5 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o4); n.pushVertex(o1); // N = { o4 o1 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.insertVertex(o3, n.vertexCnt()); // N = { o3 o4 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.pushVertex(o7); // N = { o3 o7 }

      rcUNITTEST_ASSERT(o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o8); n.pushVertex(o9); // N = { o8 o9 }

      rcUNITTEST_ASSERT(!o.contains(n));
      rcUNITTEST_ASSERT(!n.contains(o));
    }

    rcPolygon convex;
    convex.pushVertex(p0); convex.pushVertex(p1); convex.pushVertex(p2);
    convex.pushVertex(p3); convex.pushVertex(p4); convex.pushVertex(p5);
    convex.pushVertex(p6); convex.pushVertex(p7);

    rcPolygon valid;
    valid.pushVertex(p0); valid.pushVertex(p1); valid.pushVertex(p2);
    valid.pushVertex(p3); valid.pushVertex(p4); valid.pushVertex(p5);
    valid.pushVertex(p6); valid.pushVertex(p7p);

    rcPolygon contains;
    contains.pushVertex(c0); contains.pushVertex(c1);
    contains.pushVertex(c2); contains.pushVertex(c3);

    for (int32 ci = 0; ci < convex.vertexCnt(); ci++) {
      rc2Dvector t = convex.vertexAt(0); // Rotate polygon by 1 position
      convex.deleteVertex(0);
      convex.pushVertex(t);
      rcUNITTEST_ASSERT(convex.isConvex());

      t = valid.vertexAt(0); // Rotate polygon by 1 position
      valid.deleteVertex(0);
      valid.pushVertex(t);
      rcUNITTEST_ASSERT(valid.isValid());
      rcUNITTEST_ASSERT(!valid.isConvex());

      rcPolygon p;
      p.pushVertex(q0); p.pushVertex(q1); p.pushVertex(q2); p.pushVertex(q3); 

      for (int32 i = 0; i < p.vertexCnt(); i++) {
	rc2Dvector temp = p.vertexAt(0);
	p.deleteVertex(0);
	p.pushVertex(temp);
	rcUNITTEST_ASSERT(p.isConvex());

	{
	  rcPolygon outside(p);

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon outside(p);
	  outside.translate(rc2Dvector(-16,2));

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon outside(p);
	  outside.translate(rc2Dvector(-30,0));

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon outside(p);
	  outside.translate(rc2Dvector(-34,-10));

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon outside(p);
	  outside.translate(rc2Dvector(-22,-28));

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon outside(p);
	  outside.translate(rc2Dvector(-20,-16));

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon outside(p);
	  outside.translate(rc2Dvector(-20,-4));

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon outside(p);
	  outside.translate(rc2Dvector(6,-10));

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon outside(p);
	  outside.translate(rc2Dvector(-10,-14));

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon outside(p);
	  outside.translate(rc2Dvector(-8,-16));

	  rcUNITTEST_ASSERT(!convex.contains(outside));
	  rcUNITTEST_ASSERT(!valid.contains(outside));
	}

	{
	  rcPolygon inside(p);
	  inside.translate(rc2Dvector(-14,-10));

	  rcUNITTEST_ASSERT(convex.contains(inside));
	  rcUNITTEST_ASSERT(valid.contains(inside));
	}

	{
	  rcPolygon inside(p);
	  inside.translate(rc2Dvector(-16,-12));

	  rcUNITTEST_ASSERT(convex.contains(inside));
	  rcUNITTEST_ASSERT(!valid.contains(inside));
	}

	{
	  rcPolygon inside(p);
	  inside.translate(rc2Dvector(-12,-12));

	  rcUNITTEST_ASSERT(convex.contains(inside));
	  rcUNITTEST_ASSERT(valid.contains(inside));
	}
      } // End of: for (int32 i = 0; i < p.vertexCnt(); i++) {

      {
	rcUNITTEST_ASSERT(convex.contains(convex));
	rcUNITTEST_ASSERT(convex.contains(valid));
	rcUNITTEST_ASSERT(!valid.contains(convex));
	rcUNITTEST_ASSERT(valid.contains(valid));
      }

      for (int32 i = 0; i < contains.vertexCnt(); i++) {
	rc2Dvector temp = contains.vertexAt(0); // Rotate polygon by 1 position
	contains.deleteVertex(0);
	contains.pushVertex(temp);
	rcUNITTEST_ASSERT(contains.isConvex());

	{
	  rcUNITTEST_ASSERT(!valid.contains(contains));
	  rcUNITTEST_ASSERT(!convex.contains(contains));
	}
      }

      rcPolygon inEdge;
      inEdge.pushVertex(p1); inEdge.pushVertex(p2); inEdge.pushVertex(p4);
      inEdge.pushVertex(p5);

      for (int32 i = 0; i < inEdge.vertexCnt(); i++) {
	rc2Dvector temp = inEdge.vertexAt(0); // Rotate polygon by 1 position
	inEdge.deleteVertex(0);
	inEdge.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge.isConvex());

	{
	  rcUNITTEST_ASSERT(valid.contains(inEdge));
	  rcUNITTEST_ASSERT(convex.contains(inEdge));
	}
      }

      rcPolygon inEdge2, inEdge3, inEdge4;
      inEdge2.pushVertex(b12); inEdge2.pushVertex(p2);
      inEdge2.pushVertex(b45); inEdge2.pushVertex(p5);
      inEdge3.pushVertex(b12); inEdge3.pushVertex(b45);
      inEdge3.pushVertex(p5); inEdge3.pushVertex(p1);
      inEdge4.pushVertex(b01); inEdge4.pushVertex(b12);
      inEdge4.pushVertex(b45); inEdge4.pushVertex(b56);

      for (int32 i = 0; i < inEdge2.vertexCnt(); i++) {
	rc2Dvector temp = inEdge2.vertexAt(0); // Rotate polygon by 1 position
	inEdge2.deleteVertex(0);
	inEdge2.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge2.isConvex());
      
	temp = inEdge3.vertexAt(0); // Rotate polygon by 1 position
	inEdge3.deleteVertex(0);
	inEdge3.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge3.isConvex());
      
	temp = inEdge4.vertexAt(0); // Rotate polygon by 1 position
	inEdge4.deleteVertex(0);
	inEdge4.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge4.isConvex());
      
	{
	  rcUNITTEST_ASSERT(valid.contains(inEdge2));
	  rcUNITTEST_ASSERT(convex.contains(inEdge2));
	  rcUNITTEST_ASSERT(valid.contains(inEdge3));
	  rcUNITTEST_ASSERT(convex.contains(inEdge3));
	  rcUNITTEST_ASSERT(valid.contains(inEdge4));
	  rcUNITTEST_ASSERT(convex.contains(inEdge4));
	}
      }

      rcPolygon outEdge, outEdge2;
      outEdge.pushVertex(p1); outEdge.pushVertex(p2); outEdge.pushVertex(p5);
      outEdge.pushVertex(p6); outEdge.pushVertex(p0);
      outEdge2.pushVertex(p6); outEdge2.pushVertex(p0);
      outEdge2.pushVertex(b01); outEdge2.pushVertex(b56);

      for (int32 i = 0; i < outEdge.vertexCnt(); i++) {
	rc2Dvector temp = outEdge.vertexAt(0); // Rotate polygon by 1 position
	outEdge.deleteVertex(0);
	outEdge.pushVertex(temp);
	rcUNITTEST_ASSERT(outEdge.isValid());
      
	temp = outEdge2.vertexAt(0); // Rotate polygon by 1 position
	outEdge2.deleteVertex(0);
	outEdge2.pushVertex(temp);
	rcUNITTEST_ASSERT(outEdge2.isValid());
      
	{
	  rcUNITTEST_ASSERT(!valid.contains(outEdge));
	  rcUNITTEST_ASSERT(!valid.contains(outEdge2));
	}
      }

      rcPolygon outEdge3, outEdge4;
      outEdge3.pushVertex(b67p); outEdge3.pushVertex(b7p0);
      outEdge3.pushVertex(p7p);
      outEdge4.pushVertex(p6); outEdge4.pushVertex(p0); outEdge4.pushVertex(p7p);
      
      for (int32 i = 0; i < outEdge3.vertexCnt(); i++) {
	rc2Dvector temp = outEdge3.vertexAt(0); // Rotate polygon by 1 position
	outEdge3.deleteVertex(0);
	outEdge3.pushVertex(temp);
	rcUNITTEST_ASSERT(outEdge3.isValid());

	temp = outEdge4.vertexAt(0); // Rotate polygon by 1 position
	outEdge4.deleteVertex(0);
	outEdge4.pushVertex(temp);
	rcUNITTEST_ASSERT(outEdge4.isValid());
      
	{
	  rcUNITTEST_ASSERT(!valid.contains(outEdge3));
	  rcUNITTEST_ASSERT(!valid.contains(outEdge4));
	}
      }
    } // for (int32 ci = 0; ci < convex.vertexCnt(); ci++) {

    {
      rc2Dvector atc0(0,0), atc1(10,0), atc2(10,10), atc3(0,10);
      rc2Dvector atp0(1,1), atp1(10,-10), atp2(10,10), atp3(-10,10);
      rc2Dvector ai0(10,8), ai1(5,8), ai2(5,5), ai3(10,5);

      rcPolygon amyTC;
      amyTC.pushVertex(atc0); amyTC.pushVertex(atc1); amyTC.pushVertex(atc2);
      amyTC.pushVertex(atc3);

      rcPolygon amyTP;
      amyTP.pushVertex(atp0); amyTP.pushVertex(atp1); amyTP.pushVertex(atp2);
      amyTP.pushVertex(atp3);

      rcPolygon amyI;
      amyI.pushVertex(ai0); amyI.pushVertex(ai1); amyI.pushVertex(ai2);
      amyI.pushVertex(ai3);
      
      for (int32 ci = 0; ci < amyTC.vertexCnt(); ci++) {
	rc2Dvector t = amyTC.vertexAt(0); // Rotate polygon by 1 position
	amyTC.deleteVertex(0);
	amyTC.pushVertex(t);
	rcUNITTEST_ASSERT(amyTC.isConvex());

	t = amyTP.vertexAt(0); // Rotate polygon by 1 position
	amyTP.deleteVertex(0);
	amyTP.pushVertex(t);
	rcUNITTEST_ASSERT(amyTP.isValid());
	rcUNITTEST_ASSERT(!amyTP.isConvex());

	for (int32 i = 0; i < amyI.vertexCnt(); i++) {
	  rc2Dvector temp = amyI.vertexAt(0); // Rotate polygon by 1 position
	  amyI.deleteVertex(0);
	  amyI.pushVertex(temp);
	  rcUNITTEST_ASSERT(amyI.isConvex());

	  {
	    rcUNITTEST_ASSERT(amyTC.contains(amyI));
	    rcUNITTEST_ASSERT(amyTP.contains(amyI));
	  }
	}
      }
    }

    {
      rc2Dvector stc0(0,0), stc1(10,0), stc2(10,10), stc3(0,10);
      rc2Dvector stp0(1,1), stp1(10,-10), stp2(10,10), stp3(-10,10);
      rc2Dvector so0(5,5), so1(15,5), so2(15,15), so3(5,15);

      rcPolygon sqsqTC;
      sqsqTC.pushVertex(stc0); sqsqTC.pushVertex(stc1); sqsqTC.pushVertex(stc2);
      sqsqTC.pushVertex(stc3);

      rcPolygon sqsqTP;
      sqsqTP.pushVertex(stp0); sqsqTP.pushVertex(stp1); sqsqTP.pushVertex(stp2);
      sqsqTP.pushVertex(stp3);

      rcPolygon sqsqO;
      sqsqO.pushVertex(so0); sqsqO.pushVertex(so1); sqsqO.pushVertex(so2);
      sqsqO.pushVertex(so3);
      
      for (int32 ci = 0; ci < sqsqTC.vertexCnt(); ci++) {
	rc2Dvector t = sqsqTC.vertexAt(0); // Rotate polygon by 1 position
	sqsqTC.deleteVertex(0);
	sqsqTC.pushVertex(t);
	rcUNITTEST_ASSERT(sqsqTC.isConvex());

	t = sqsqTP.vertexAt(0); // Rotate polygon by 1 position
	sqsqTP.deleteVertex(0);
	sqsqTP.pushVertex(t);
	rcUNITTEST_ASSERT(sqsqTP.isValid());
	rcUNITTEST_ASSERT(!sqsqTP.isConvex());

	for (int32 i = 0; i < sqsqO.vertexCnt(); i++) {
	  rc2Dvector temp = sqsqO.vertexAt(0); // Rotate polygon by 1 position
	  sqsqO.deleteVertex(0);
	  sqsqO.pushVertex(temp);
	  rcUNITTEST_ASSERT(sqsqO.isConvex());

	  {
	    rcUNITTEST_ASSERT(!sqsqTC.contains(sqsqO));
	    rcUNITTEST_ASSERT(!sqsqTP.contains(sqsqO));
	  }
	}
      }
    }

    {
      rc2Dvector htc0(0,0), htc1(20,0), htc2(20,10), htc3(10,20), htc4(0, 10);
      rc2Dvector htp0(10,4), htp1(20,-10), htp2(20,10), htp3(10,20), htp4(0,10);
      rc2Dvector ho0(10,10), ho1(30,10), ho2(30,20), ho3(10,20);

      rcPolygon houseTC;
      houseTC.pushVertex(htc0); houseTC.pushVertex(htc1); houseTC.pushVertex(htc2);
      houseTC.pushVertex(htc3); houseTC.pushVertex(htc4);

      rcPolygon houseTP;
      houseTP.pushVertex(htp0); houseTP.pushVertex(htp1); houseTP.pushVertex(htp2);
      houseTP.pushVertex(htp3); houseTP.pushVertex(htp4);

      rcPolygon houseO;
      houseO.pushVertex(ho0); houseO.pushVertex(ho1); houseO.pushVertex(ho2);
      houseO.pushVertex(ho3);
      
      for (int32 ci = 0; ci < houseTC.vertexCnt(); ci++) {
	rc2Dvector t = houseTC.vertexAt(0); // Rotate polygon by 1 position
	houseTC.deleteVertex(0);
	houseTC.pushVertex(t);
	rcUNITTEST_ASSERT(houseTC.isConvex());

	t = houseTP.vertexAt(0); // Rotate polygon by 1 position
	houseTP.deleteVertex(0);
	houseTP.pushVertex(t);
	rcUNITTEST_ASSERT(houseTP.isValid());
	rcUNITTEST_ASSERT(!houseTP.isConvex());

	for (int32 i = 0; i < houseO.vertexCnt(); i++) {
	  rc2Dvector temp = houseO.vertexAt(0); // Rotate polygon by 1 position
	  houseO.deleteVertex(0);
	  houseO.pushVertex(temp);
	  rcUNITTEST_ASSERT(houseO.isConvex());

	  {
	    rcUNITTEST_ASSERT(!houseTC.contains(houseO));
	    rcUNITTEST_ASSERT(!houseTP.contains(houseO));
	  }
	}
      }
    }

    {
      rc2Dvector tt0(0,16), tt1(5,8), tt2(13,0), tt3(19,2), tt4(24,10);
      rc2Dvector tt5(24,26), tt6(19,29), tt7(13,32), tt8(7,32), tt9(3,29);
      rc2Dvector to0(16,-3), to1(28,16), to2(16,32), to3(0,22), to4(3,10);

      rcPolygon twoT;
      twoT.pushVertex(tt0); twoT.pushVertex(tt1); twoT.pushVertex(tt2);
      twoT.pushVertex(tt3); twoT.pushVertex(tt4); twoT.pushVertex(tt5);
      twoT.pushVertex(tt6); twoT.pushVertex(tt7); twoT.pushVertex(tt8);
      twoT.pushVertex(tt9);

      rcPolygon twoO;
      twoO.pushVertex(to0); twoO.pushVertex(to1); twoO.pushVertex(to2);
      twoO.pushVertex(to3); twoO.pushVertex(to4);
      
      for (int32 ci = 0; ci < twoT.vertexCnt(); ci++) {
	rc2Dvector t = twoT.vertexAt(0); // Rotate polygon by 1 position
	twoT.deleteVertex(0);
	twoT.pushVertex(t);
	rcUNITTEST_ASSERT(twoT.isConvex()); 

	for (int32 i = 0; i < twoO.vertexCnt(); i++) {
	  rc2Dvector temp = twoO.vertexAt(0); // Rotate polygon by 1 position
	  twoO.deleteVertex(0);
	  twoO.pushVertex(temp);
	  rcUNITTEST_ASSERT(twoO.isConvex());

	  {
	    rcUNITTEST_ASSERT(!twoT.contains(twoO));
	  }
	}
      }
    }
  }
}

void UT_Polygon::testPolygonCombine()
{
  /* Test generate intersection function.
   */
  {
    /* Use n and o to test case where the number of vertices in the polgons
     * is < 3.
     */
    {
      rc2Dvector o0(0,0), o1(3,0), o2(1,2), o3(1,1), o4(1,0), o5(5,5);
      rc2Dvector o6(5,0), o7(2,2), o8(6,0), o9(6,6);

      rcPolygon o, n, polyInt;

      /* Test case of both polygons being empty.
       */
      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);

      o.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);

      /* Test cases with 1 polygon empty and 1 polygon having 1 vertex.
       */
      o.pushVertex(o0); // O = { o0 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);

      /* Test cases with 1 polygon empty and 1 polygon having 2 vertices.
       */
      o.pushVertex(o1); // O = { o0 o1 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);

      /* Test cases with both polygons having 1 vertex.
       */
      o.popVertex(); // O = { o0 }
      n.pushVertex(o1); // N = { o1 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);

      n.popVertex(); n.pushVertex(o0); // N = { o0 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == o);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      /* Test cases with 1 polygon having 1 vertex and 1 polygon
       * having >= 2 vertices.
       */
      o.pushVertex(o1); // O = { o0 o1 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      n.popVertex(); n.pushVertex(o1); // N = { o1 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      n.popVertex(); n.pushVertex(o4); // N = { o4 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      o.pushVertex(o2); // O = { o0 o1 o2 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      n.popVertex(); n.pushVertex(o5); // N = { o5 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);

      /* Test cases with both polygons having 2 vertices.
       */
      o.popVertex(); // O = { o0 o1 }
      n.popVertex(); // N = { }
      n.pushVertex(o4); n.pushVertex(o6); // N = { o4 o6 }

      {
	rcPolygon e; e.pushVertex(o1); e.pushVertex(o4);

	o.convexIntersection(n, polyInt);
	rcUNITTEST_ASSERT(polyInt == e);
	n.convexIntersection(o, polyInt);
	rcUNITTEST_ASSERT(polyInt == e);
      }

      o.popVertex(); o.pushVertex(o5); // O = { o0 o5 }
      n.popVertex(); n.pushVertex(o2); // N = { o4 o2 }

      {
	rcPolygon e; e.pushVertex(o3);

	o.convexIntersection(n, polyInt);
	rcUNITTEST_ASSERT(polyInt == e);
	n.convexIntersection(o, polyInt);
	rcUNITTEST_ASSERT(polyInt == e);
      }

      o.popVertex(); o.pushVertex(o2); // O = { o0 o2 }
      n.popVertex(); n.pushVertex(o3); // N = { o4 o3 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);

      n.popVertex(); n.pushVertex(o0); // N = { o4 o0 }

      {
	rcPolygon e; e.pushVertex(o0);

	o.convexIntersection(n, polyInt);
	rcUNITTEST_ASSERT(polyInt == e);
	n.convexIntersection(o, polyInt);
	rcUNITTEST_ASSERT(polyInt == e);
      }

      o.popVertex(); o.pushVertex(o6); // O = { o0 o6 }
      n.popVertex(); n.pushVertex(o1); // N = { o4 o1 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      o.popVertex(); o.pushVertex(o4); // O = { o0 o4 }
      n.deleteVertex(0); n.pushVertex(o6); // N = { o1 o6 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o2); n.pushVertex(o7); // N = { o2 o7 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o4); n.pushVertex(o0); // N = { o4 o0 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == o);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o0); n.pushVertex(o4); // N = { o0 o4 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == o);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      o.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == o);

      /* Test cases where one polygon has 2 vertices and the other > 2
       * vertices.
       */
      o.popVertex(); o.pushVertex(o2); // O = { o0 o2 }
      o.pushVertex(o5); o.pushVertex(o6); // O = { o0 o2 o5 o6 }
      n.deleteVertex(0); n.pushVertex(o2); // N = { o0 o2 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      n.popVertex(); n.pushVertex(o5); // N = { o0 o5 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o4); n.pushVertex(o1); // N = { o4 o1 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      n.popVertex(); n.insertVertex(o3, n.vertexCnt()); // N = { o3 o4 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      n.popVertex(); n.pushVertex(o7); // N = { o3 o7 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt == n);

      n.popVertex(); n.popVertex(); // N = { }
      n.pushVertex(o8); n.pushVertex(o9); // N = { o8 o9 }

      o.convexIntersection(n, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);
      n.convexIntersection(o, polyInt);
      rcUNITTEST_ASSERT(polyInt.vertexCnt() == 0);
    }

    rc2Dvector p0(4,-6), p1(10,-4), p2(10,4), p3(4,6), p4(-2,6);
    rc2Dvector p5(-4,2), p6(-6,-4), p7(-4,-6);

    rc2Dvector c0(-40,-40), c1(60,-40), c2(60, 70), c3(-40, 70);

    rc2Dvector b01((p0.x() + (p1.x() - p0.x())/2), (p0.y() + (p1.y() - p0.y())/2));
    rc2Dvector b12((p1.x() + (p2.x() - p1.x())/2), (p1.y() + (p2.y() - p1.y())/2));
    rc2Dvector b45((p4.x() + (p5.x() - p4.x())/2), (p4.y() + (p5.y() - p4.y())/2));
    rc2Dvector b56((p5.x() + (p6.x() - p5.x())/2), (p5.y() + (p6.y() - p5.y())/2));

    rc2Dvector q0(16,6), q1(22,8), q2(18,12), q3(12,10);

    rcPolygon convex;
    convex.pushVertex(p0); convex.pushVertex(p1); convex.pushVertex(p2);
    convex.pushVertex(p3); convex.pushVertex(p4); convex.pushVertex(p5);
    convex.pushVertex(p6); convex.pushVertex(p7);

    rcPolygon contains;
    contains.pushVertex(c0); contains.pushVertex(c1);
    contains.pushVertex(c2); contains.pushVertex(c3);

    for (int32 ci = 0; ci < convex.vertexCnt(); ci++) {
      rc2Dvector t = convex.vertexAt(0); // Rotate polygon by 1 position
      convex.deleteVertex(0);
      convex.pushVertex(t);
      rcUNITTEST_ASSERT(convex.isConvex());

      rcPolygon p;
      p.pushVertex(q0); p.pushVertex(q1); p.pushVertex(q2); p.pushVertex(q3); 
      
      for (int32 i = 0; i < p.vertexCnt(); i++) {
	rc2Dvector temp = p.vertexAt(0); // Rotate polygon by 1 position
	p.deleteVertex(0);
	p.pushVertex(temp);
	rcUNITTEST_ASSERT(p.isConvex());

	{
	  rcPolygon  polyInt, expected;

	  rcPolygon outside(p);
	  convex.convexIntersection(outside, polyInt);
	  rcUNITTEST_ASSERT(polyInt == expected);

	  rcPolygon outside1(p); outside1.translate(rc2Dvector(-16,2));
	  convex.convexIntersection(outside1, polyInt);
	  rcUNITTEST_ASSERT(polyInt == expected);

	  rcPolygon outside2(p); outside2.translate(rc2Dvector(-30,0));
	  convex.convexIntersection(outside2, polyInt);
	  rcUNITTEST_ASSERT(polyInt == expected);

	  rcPolygon outside3(p); outside3.translate(rc2Dvector(-34,-10));
	  convex.convexIntersection(outside3, polyInt);
	  rcUNITTEST_ASSERT(polyInt == expected);

	  rcPolygon outside4(p); outside4.translate(rc2Dvector(-22,-28));
	  convex.convexIntersection(outside4, polyInt);
	  rcUNITTEST_ASSERT(polyInt == expected);

	  rcPolygon outside5(p); outside5.translate(rc2Dvector(-20,-16));
	  convex.convexIntersection(outside5, polyInt);
	  {
	    rc2Dvector e0(-4,-6), e1(0,-6), e2(-2,-4), e3(-5,-5);
	    rcPolygon ePoly;
	    ePoly.pushVertex(e0); ePoly.pushVertex(e1);
	    ePoly.pushVertex(e2); ePoly.pushVertex(e3);
	    rcUNITTEST_ASSERT(polyInt == ePoly);
	  }

	  rcPolygon outside6(p); outside6.translate(rc2Dvector(-20,-4));
	  convex.convexIntersection(outside6, polyInt);
	  {
	    rc2Dvector e0(-2,6), e1(-4,2), e2(2,4), e3(0,6);
	    rcPolygon ePoly;
	    ePoly.pushVertex(e0); ePoly.pushVertex(e1);
	    ePoly.pushVertex(e2); ePoly.pushVertex(e3);
	    rcUNITTEST_ASSERT(polyInt == ePoly);
	  }

	  rcPolygon outside7(p); outside7.translate(rc2Dvector(6,-10));
	  convex.convexIntersection(outside7, polyInt);
	  rcUNITTEST_ASSERT(polyInt == expected);

	  rcPolygon outside8(p); outside8.translate(rc2Dvector(-10,-14));
	  convex.convexIntersection(outside8, polyInt);
	  {
	    rc2Dvector e0(4,-6), e1(10,-4), e2(8,-2), e3(2,-4);
	    rcPolygon ePoly;
	    ePoly.pushVertex(e0); ePoly.pushVertex(e1);
	    ePoly.pushVertex(e2); ePoly.pushVertex(e3);
	    rcUNITTEST_ASSERT(polyInt == ePoly);
	  }

	  rcPolygon outside9(p); outside9.translate(rc2Dvector(-8,-16));
	  convex.convexIntersection(outside9, polyInt);
	  {
	    rc2Dvector e0(4,-6), e1(10,-4);
	    rcPolygon ePoly;
	    ePoly.pushVertex(e0); ePoly.pushVertex(e1);
	    rcUNITTEST_ASSERT(polyInt == ePoly);
	  }

	  rcPolygon inside(p); inside.translate(rc2Dvector(-14,-10));
	  convex.convexIntersection(inside, polyInt);
	  rcUNITTEST_ASSERT(polyInt == inside);

	  rcPolygon inside1(p); inside1.translate(rc2Dvector(-16,-12));
	  convex.convexIntersection(inside1, polyInt);
	  rcUNITTEST_ASSERT(polyInt == inside1);

	  rcPolygon inside2(p); inside2.translate(rc2Dvector(-12,-12));
	  convex.convexIntersection(inside2, polyInt);
	  rcUNITTEST_ASSERT(polyInt == inside2);
	}
      } // End of: for (int32 i = 0; i < p.vertexCnt(); i++) {

      rcPolygon polyInt;

      {
	convex.convexIntersection(convex, polyInt);
	rcUNITTEST_ASSERT(polyInt == convex);
      }

      for (int32 i = 0; i < contains.vertexCnt(); i++) {
	rc2Dvector temp = contains.vertexAt(0); // Rotate polygon by 1 position
	contains.deleteVertex(0);
	contains.pushVertex(temp);
	rcUNITTEST_ASSERT(contains.isConvex());

	{
	  convex.convexIntersection(contains, polyInt);
	  rcUNITTEST_ASSERT(polyInt == convex);
	}
      }

      rcPolygon inEdge, inEdge2, inEdge3, inEdge4, inEdge5;
      inEdge.pushVertex(p1); inEdge.pushVertex(p2);
      inEdge.pushVertex(p4); inEdge.pushVertex(p5);
      inEdge2.pushVertex(b12); inEdge2.pushVertex(p2);
      inEdge2.pushVertex(b45); inEdge2.pushVertex(p5);
      inEdge3.pushVertex(b12); inEdge3.pushVertex(b45);
      inEdge3.pushVertex(p5); inEdge3.pushVertex(p1);
      inEdge4.pushVertex(b01); inEdge4.pushVertex(b12);
      inEdge4.pushVertex(b45); inEdge4.pushVertex(b56);
      inEdge5.pushVertex(p6); inEdge5.pushVertex(p0);
      inEdge5.pushVertex(b01); inEdge5.pushVertex(b56);

      for (int32 i = 0; i < inEdge.vertexCnt(); i++) {
	rc2Dvector temp = inEdge.vertexAt(0); // Rotate polygon by 1 position
	inEdge.deleteVertex(0);
	inEdge.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge.isConvex());

	temp = inEdge2.vertexAt(0); // Rotate polygon by 1 position
	inEdge2.deleteVertex(0);
	inEdge2.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge2.isConvex());
      
	temp = inEdge3.vertexAt(0); // Rotate polygon by 1 position
	inEdge3.deleteVertex(0);
	inEdge3.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge3.isConvex());
      
	temp = inEdge4.vertexAt(0); // Rotate polygon by 1 position
	inEdge4.deleteVertex(0);
	inEdge4.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge4.isConvex());
      
	temp = inEdge5.vertexAt(0); // Rotate polygon by 1 position
	inEdge5.deleteVertex(0);
	inEdge5.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge5.isConvex());
      
	{
	  convex.convexIntersection(inEdge, polyInt);
	  rcUNITTEST_ASSERT(polyInt == inEdge);
	  convex.convexIntersection(inEdge2, polyInt);
	  rcUNITTEST_ASSERT(polyInt == inEdge2);
	  convex.convexIntersection(inEdge3, polyInt);
	  rcUNITTEST_ASSERT(polyInt == inEdge3);
	  convex.convexIntersection(inEdge4, polyInt);
	  rcUNITTEST_ASSERT(polyInt == inEdge4);
	  convex.convexIntersection(inEdge5, polyInt);
	  rcUNITTEST_ASSERT(polyInt == inEdge5);
	}
      }

      rcPolygon inEdge6;
      inEdge6.pushVertex(p1); inEdge6.pushVertex(p2); inEdge6.pushVertex(p5);
      inEdge6.pushVertex(p6); inEdge6.pushVertex(p0);

      for (int32 i = 0; i < inEdge6.vertexCnt(); i++) {
	rc2Dvector temp = inEdge6.vertexAt(0); // Rotate polygon by 1 position
	inEdge6.deleteVertex(0);
	inEdge6.pushVertex(temp);
	rcUNITTEST_ASSERT(inEdge6.isConvex());

	{
	  convex.convexIntersection(inEdge6, polyInt);
	  rcUNITTEST_ASSERT(polyInt == inEdge6);
	}
      }
    } // End of: for (int32 ci = 0; ci < convex.vertexCnt(); ci++) {

    {
      rc2Dvector atc0(0,0), atc1(10,0), atc2(10,10), atc3(0,10);
      rc2Dvector ai0(10,8), ai1(5,8), ai2(5,5), ai3(10,5);

      rcPolygon amyTC;
      amyTC.pushVertex(atc0); amyTC.pushVertex(atc1); amyTC.pushVertex(atc2);
      amyTC.pushVertex(atc3);

      rcPolygon amyI;
      amyI.pushVertex(ai0); amyI.pushVertex(ai1); amyI.pushVertex(ai2);
      amyI.pushVertex(ai3);
      
      for (int32 ci = 0; ci < amyTC.vertexCnt(); ci++) {
	rc2Dvector t = amyTC.vertexAt(0); // Rotate polygon by 1 position
	amyTC.deleteVertex(0);
	amyTC.pushVertex(t);
	rcUNITTEST_ASSERT(amyTC.isConvex());

	for (int32 i = 0; i < amyI.vertexCnt(); i++) {
	  rc2Dvector temp = amyI.vertexAt(0); // Rotate polygon by 1 position
	  amyI.deleteVertex(0);
	  amyI.pushVertex(temp);
	  rcUNITTEST_ASSERT(amyI.isConvex());

	  {
	    rcPolygon polyInt;
	    amyTC.convexIntersection(amyI, polyInt);
	    rcUNITTEST_ASSERT(polyInt == amyI);
	  }
	}
      }
    }

    {
      rc2Dvector stc0(0,0), stc1(10,0), stc2(10,10), stc3(0,10);
      rc2Dvector so0(5,5), so1(15,5), so2(15,15), so3(5,15);

      rc2Dvector e0(5,5), e1(10,5), e2(10,10), e3(5,10);

      rcPolygon sqsqTC;
      sqsqTC.pushVertex(stc0); sqsqTC.pushVertex(stc1);
      sqsqTC.pushVertex(stc2); sqsqTC.pushVertex(stc3);

      rcPolygon sqsqO;
      sqsqO.pushVertex(so0); sqsqO.pushVertex(so1);
      sqsqO.pushVertex(so2); sqsqO.pushVertex(so3);
      
      rcPolygon expected;
      expected.pushVertex(e0); expected.pushVertex(e1);
      expected.pushVertex(e2); expected.pushVertex(e3);

      for (int32 ci = 0; ci < sqsqTC.vertexCnt(); ci++) {
	rc2Dvector t = sqsqTC.vertexAt(0); // Rotate polygon by 1 position
	sqsqTC.deleteVertex(0);
	sqsqTC.pushVertex(t);
	rcUNITTEST_ASSERT(sqsqTC.isConvex());

	for (int32 i = 0; i < sqsqO.vertexCnt(); i++) {
	  rc2Dvector temp = sqsqO.vertexAt(0); // Rotate polygon by 1 position
	  sqsqO.deleteVertex(0);
	  sqsqO.pushVertex(temp);
	  rcUNITTEST_ASSERT(sqsqO.isConvex());

	  {
	    rcPolygon polyInt;
	    sqsqTC.convexIntersection(sqsqO, polyInt);
	    rcUNITTEST_ASSERT(polyInt == expected);
	  }
	}
      }
    }

    {
      rc2Dvector htc0(0,0), htc1(20,0), htc2(20,10), htc3(10,20), htc4(0, 10);
      rc2Dvector ho0(10,10), ho1(30,10), ho2(30,20), ho3(10,20);

      rcPolygon houseTC;
      houseTC.pushVertex(htc0); houseTC.pushVertex(htc1); houseTC.pushVertex(htc2);
      houseTC.pushVertex(htc3); houseTC.pushVertex(htc4);

      rcPolygon houseO;
      houseO.pushVertex(ho0); houseO.pushVertex(ho1); houseO.pushVertex(ho2);
      houseO.pushVertex(ho3);
      
      rcPolygon expected;
      expected.pushVertex(ho0); expected.pushVertex(htc2);
      expected.pushVertex(ho3);

      for (int32 ci = 0; ci < houseTC.vertexCnt(); ci++) {
	rc2Dvector t = houseTC.vertexAt(0); // Rotate polygon by 1 position
	houseTC.deleteVertex(0);
	houseTC.pushVertex(t);
	rcUNITTEST_ASSERT(houseTC.isConvex());

	for (int32 i = 0; i < houseO.vertexCnt(); i++) {
	  rc2Dvector temp = houseO.vertexAt(0); // Rotate polygon by 1 position
	  houseO.deleteVertex(0);
	  houseO.pushVertex(temp);
	  rcUNITTEST_ASSERT(houseO.isConvex());

	  {
	    rcPolygon polyInt;
	    houseTC.convexIntersection(houseO, polyInt);
	    rcUNITTEST_ASSERT(polyInt == expected);
	  }
	}
      }
    }

    {
      rc2Dvector twoOne0(0,16), twoOne1(5,8), twoOne2(13,0), twoOne3(19,2);
      rc2Dvector twoOne4(24,10), twoOne5(24,26), twoOne6(19,29);
      rc2Dvector twoOne7(13,32), twoOne8(7,32), twoOne9(3,29);
      rc2Dvector twoTwo0(16,-3), twoTwo1(28,16), twoTwo2(16,32);
      rc2Dvector twoTwo3(0,22), twoTwo4(3,10);

      rc2Dvector exp0(5,8), exp1(13,0), exp2(19,2), exp3(24,10);
      rc2Dvector exp4(24.0,21.333), exp5(17.8,29.6), exp6(14.667, 31.167);
      rc2Dvector exp7(1.618, 23.011), exp8(0.72,19.12), exp9(2.5,12.0);
 
      rcPolygon expected;
      expected.pushVertex(exp0); expected.pushVertex(exp1);
      expected.pushVertex(exp2); expected.pushVertex(exp3);
      expected.pushVertex(exp4); expected.pushVertex(exp5);
      expected.pushVertex(exp6); expected.pushVertex(exp7);
      expected.pushVertex(exp8); expected.pushVertex(exp9);
      rcUNITTEST_ASSERT(expected.isConvex());

      rcPolygon poly1;
      poly1.pushVertex(twoOne0); poly1.pushVertex(twoOne1);
      poly1.pushVertex(twoOne2); poly1.pushVertex(twoOne3);
      poly1.pushVertex(twoOne4); poly1.pushVertex(twoOne5);
      poly1.pushVertex(twoOne6); poly1.pushVertex(twoOne7);
      poly1.pushVertex(twoOne8); poly1.pushVertex(twoOne9);

      rcPolygon poly2;
      poly2.pushVertex(twoTwo0); poly2.pushVertex(twoTwo1);
      poly2.pushVertex(twoTwo2); poly2.pushVertex(twoTwo3);
      poly2.pushVertex(twoTwo4);

      for (int32 ci = 0; ci < poly1.vertexCnt(); ci++) {
	rc2Dvector t = poly1.vertexAt(0); // Rotate polygon by 1 position
	poly1.deleteVertex(0);
	poly1.pushVertex(t);
	rcUNITTEST_ASSERT(poly1.isConvex()); 

	for (int32 i = 0; i < poly2.vertexCnt(); i++) {
	  rc2Dvector temp = poly2.vertexAt(0); // Rotate polygon by 1 position
	  poly2.deleteVertex(0);
	  poly2.pushVertex(temp);
	  rcUNITTEST_ASSERT(poly2.isConvex());

	  {
	    rcPolygon polyInt;
	    
	    poly1.convexIntersection(poly2, polyInt);
	    
	    rcUNITTEST_ASSERT(polyInt == expected);
	  }
	}
      }
    }

    {
      rc2Dvector pl0(4,12), pl1(12,4), pl2(8,12), pl3(12,20);
      rc2Dvector pu0(10,12), pu1(12,12), pu2(14,8), pu3(14,16);
      rc2Dvector e0(10,12), e1(12,10), e2(12,14);

      rcPolygon pLeft, pUp, expected;
      pLeft.pushVertex(pl0); pLeft.pushVertex(pl1);
      pLeft.pushVertex(pl2); pLeft.pushVertex(pl3);
      pUp.pushVertex(pu0); pUp.pushVertex(pu1);
      pUp.pushVertex(pu2); pUp.pushVertex(pu3);
      expected.pushVertex(e0); expected.pushVertex(e1);
      expected.pushVertex(e2);

      for (int32 ci = 0; ci < pLeft.vertexCnt(); ci++) {
	rc2Dvector t = pLeft.vertexAt(0); // Rotate polygon by 1 position
	pLeft.deleteVertex(0);
	pLeft.pushVertex(t);
	rcUNITTEST_ASSERT(pLeft.isValid()); 

	for (int32 i = 0; i < pUp.vertexCnt(); i++) {
	  rc2Dvector temp = pUp.vertexAt(0); // Rotate polygon by 1 position
	  pUp.deleteVertex(0);
	  pUp.pushVertex(temp);
	  rcUNITTEST_ASSERT(pUp.isValid());

	  {
	    rcPolygon polyInt;
	    
	    pLeft.convexIntersection(pUp, polyInt);
	    
	    rcUNITTEST_ASSERT(polyInt == expected);
	  }
	}
      }
    }
  }

  /* Test generate union function.
   */
  {
    rc2Dvector p0(2,1), p1(7,1), p2(2,5), p3(7,5);
    rc2Dvector q0(12,1), q1(12,5);
    rc2Dvector r0(10,7), r1(15,7), r2(15,11), r3(10,11);
    rc2Dvector s0(13, 9);

    rcPolygon p, q, r, s;
    p.pushVertex(p0); p.pushVertex(p1);
    p.pushVertex(p3); p.pushVertex(p2);
    q.pushVertex(p3); q.pushVertex(p1);
    q.pushVertex(q0); q.pushVertex(q1);
    r.pushVertex(r0); r.pushVertex(r1);
    r.pushVertex(r2); r.pushVertex(r3);
    s.pushVertex(r0); s.pushVertex(r1);
    s.pushVertex(r2); s.pushVertex(r3);
    s.pushVertex(s0);

    rcPolygon polyUnion;

    {
      p.convexUnion(p, polyUnion);
      rcUNITTEST_ASSERT(polyUnion == p);
    }

    {
      rcPolygon expected;
      expected.pushVertex(p2); expected.pushVertex(p0);
      expected.pushVertex(q0); expected.pushVertex(q1);

      p.convexUnion(q, polyUnion);
      rcUNITTEST_ASSERT(polyUnion == expected);
    }

    {
      rcPolygon expected;
      expected.pushVertex(p2); expected.pushVertex(p0);
      expected.pushVertex(p1); expected.pushVertex(r1);
      expected.pushVertex(r2); expected.pushVertex(r3);

      p.convexUnion(r, polyUnion);
      rcUNITTEST_ASSERT(polyUnion == expected);

      p.convexUnion(s, polyUnion);
      rcUNITTEST_ASSERT(polyUnion == expected);
    }
  }
}

void UT_Polygon::testMinEncRect()
{
  {
    rc2Dvector p0(1,0), p1(4,0), p2(4,3);

    rcPolygon p;

    rcAffineRectangle rect = p.minimumPerimeterEnclosingRect();

    rcUNITTEST_ASSERT(rect.origin() == rc2Dvector(-1000000, -1000000));
    rcUNITTEST_ASSERT((rect.cannonicalSize().x() == 1) &&
		       (rect.cannonicalSize().y() == 1));
    rcUNITTEST_ASSERT(rfRealEq(rect.angle().Double(),
				rcDegree(0.0).Double(), 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				rect.xyScale().x(), 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				rect.xyScale().y(), 1e-7));

    p.pushVertex(p1);

    rect = p.minimumPerimeterEnclosingRect();

    rcUNITTEST_ASSERT(rect.origin() == p1);
    rcUNITTEST_ASSERT((rect.cannonicalSize().x() == 1) &&
		       (rect.cannonicalSize().y() == 1));
    rcUNITTEST_ASSERT(rfRealEq(rect.angle().Double(),
				rcDegree(0.0).Double(), 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				rect.xyScale().x(), 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				rect.xyScale().y(), 1e-7));

    p.pushVertex(p2);

    rect = p.minimumPerimeterEnclosingRect();

    rcUNITTEST_ASSERT(rect.origin() == p1);
    rcUNITTEST_ASSERT((rect.cannonicalSize().x() == 3) &&
		       (rect.cannonicalSize().y() == 1));
    rcUNITTEST_ASSERT(rfRealEq(rect.angle().norm().Double(), rkPI/2, 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				rect.xyScale().x(), 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				rect.xyScale().y(), 1e-7));

    p.popVertex(); p.pushVertex(p0);

    rect = p.minimumPerimeterEnclosingRect();

    rcUNITTEST_ASSERT(rect.origin() == p1);
    rcUNITTEST_ASSERT((rect.cannonicalSize().x() == 3) &&
		       (rect.cannonicalSize().y() == 1));
    rcUNITTEST_ASSERT(rfRealEq(rect.angle().norm().Double(), rkPI, 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				rect.xyScale().x(), 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				rect.xyScale().y(), 1e-7));

    p.popVertex(); p.popVertex();
    p.pushVertex(p0); p.pushVertex(p2);

    rect = p.minimumPerimeterEnclosingRect();

    rcUNITTEST_ASSERT(rect.origin() == p0);
    rcUNITTEST_ASSERT((rect.cannonicalSize().x() == (int32)sqrt(9.+9.)) &&
		       (rect.cannonicalSize().y() == 1));
    rcUNITTEST_ASSERT(rfRealEq(rect.angle().norm().Double(), rkPI/4, 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				rect.xyScale().x(), 1e-7));
    rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				rect.xyScale().y(), 1e-7));
  }

  {
    rc2Dvector p0(7,3), p1(10,4), p2(12,7), p3(12,11), p4(9,11);
    rc2Dvector p5(5,7), p6(6,4);
    
    rcPolygon p;
    p.pushVertex(p0); p.pushVertex(p1); p.pushVertex(p2);
    p.pushVertex(p3); p.pushVertex(p4); p.pushVertex(p5);
    p.pushVertex(p6);
    
    for (int32 i = 0; i < p.vertexCnt(); i++) {
      rc2Dvector t = p.vertexAt(0); // Rotate polygon by 1 position
      p.deleteVertex(0);
      p.pushVertex(t);
      rcUNITTEST_ASSERT(p.isConvex()); 
	
      rcAffineRectangle rect = p.minimumPerimeterEnclosingRect();
	
      /* Not easy to know the origin of the resulting rect, but 
       * we do know what its dimensions should be.
       */
      rcUNITTEST_ASSERT(((rect.cannonicalSize().x() == 10) &&
			  (rect.cannonicalSize().y() == 7)) ||
			 ((rect.cannonicalSize().x() == 7) &&
			  (rect.cannonicalSize().y() == 10)));

      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				  rect.xyScale().x(), 0.5));
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				  rect.xyScale().y(), 0.5));
	
      rect = p.minimumAreaEnclosingRect();
	
      rcUNITTEST_ASSERT(((rect.cannonicalSize().x() == 10) &&
			  (rect.cannonicalSize().y() == 7)) ||
			 ((rect.cannonicalSize().x() == 7) &&
			  (rect.cannonicalSize().y() == 10)));
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				  rect.xyScale().x(), 0.5));
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				  rect.xyScale().y(), 0.5));
    }
  }
      
  {
    rc2Dvector s0(0,0), s1(10,0), s2(10,5), s3(0,5);
      
    rcPolygon p;
    p.pushVertex(s0); p.pushVertex(s1);
    p.pushVertex(s2); p.pushVertex(s3);

    for (int32 i = 0; i < p.vertexCnt(); i++) {
      rc2Dvector t = p.vertexAt(0); // Rotate polygon by 1 position
      p.deleteVertex(0);
      p.pushVertex(t);
      rcUNITTEST_ASSERT(p.isConvex()); 
	
      rcAffineRectangle rect = p.minimumPerimeterEnclosingRect();

      /* Here we know that the origin should be one of the points from
       * the polygon. Check both the origin and the dimensions of the
       * resulting rect.
       */
      int32 index;
      for (index = 0; index < p.vertexCnt(); index++)
	if (p.vertexAt(index) == rect.origin())
	  break;
      rcUNITTEST_ASSERT(index != p.vertexCnt());

      rcUNITTEST_ASSERT(((rect.cannonicalSize().x() == 11) &&
			  (rect.cannonicalSize().y() == 6)) ||
			 ((rect.cannonicalSize().x() == 6) &&
			  (rect.cannonicalSize().y() == 11)));
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				  rect.xyScale().x(), 1e-7));
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				  rect.xyScale().y(), 1e-7));

      rect = p.minimumAreaEnclosingRect();

      for (index = 0; index < p.vertexCnt(); index++)
	if (p.vertexAt(index) == rect.origin())
	  break;
      rcUNITTEST_ASSERT(index != p.vertexCnt());

      rcUNITTEST_ASSERT(((rect.cannonicalSize().x() == 11) &&
			  (rect.cannonicalSize().y() == 6)) ||
			 ((rect.cannonicalSize().x() == 6) &&
			  (rect.cannonicalSize().y() == 11)));
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				  rect.xyScale().x(), 1e-7));
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				  rect.xyScale().y(), 1e-7));
    }
  }

  {
    rc2Dvector d0(5,0), d1(10,5), d2(5,10), d3(0,5);

    rcPolygon p;
    p.pushVertex(d0); p.pushVertex(d1);
    p.pushVertex(d2); p.pushVertex(d3);

    for (int32 i = 0; i < p.vertexCnt(); i++) {
      rc2Dvector t = p.vertexAt(0); // Rotate polygon by 1 position
      p.deleteVertex(0);
      p.pushVertex(t);
      rcUNITTEST_ASSERT(p.isConvex()); 
	
      rcAffineRectangle rect = p.minimumPerimeterEnclosingRect();
	
      /* Here we know that the origin should be one of the points from
       * the polygon. Check both the origin and the dimensions of the
       * resulting rect.
       */
      int32 index;
      for (index = 0; index < p.vertexCnt(); index++)
	if (p.vertexAt(index) == rect.origin())
	  break;
      rcUNITTEST_ASSERT(index != p.vertexCnt());

      rcUNITTEST_ASSERT(rect.cannonicalSize().x() == 8);
      rcUNITTEST_ASSERT(rect.cannonicalSize().y() == 8);
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				  rect.xyScale().x(), 0.5));
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				  rect.xyScale().y(), 0.5));

      rect = p.minimumAreaEnclosingRect();
	
      for (index = 0; index < p.vertexCnt(); index++)
	if (p.vertexAt(index) == rect.origin())
	  break;
      rcUNITTEST_ASSERT(index != p.vertexCnt());

      rcUNITTEST_ASSERT(rect.cannonicalSize().x() == 8);
      rcUNITTEST_ASSERT(rect.cannonicalSize().y() == 8);
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().x(),
				  rect.xyScale().x(), 0.5));
      rcUNITTEST_ASSERT(rfRealEq(rect.cannonicalSize().y(),
				  rect.xyScale().y(), 0.5));
    }
  }
}

void UT_Polygon::testOrthEncRect()
{
  {
    rcPolygon p;
    rcUNITTEST_ASSERT(p.isValid());

    rcIRect actual = p.orthogonalEnclosingRect();

    rcIRect expected(0, 0, 0, 0);

    rcUNITTEST_ASSERT(expected == actual);
  }

  {
    rcPolygon q;

    rc2Dvector q0(3,3);
    q.pushVertex(q0);
    rcUNITTEST_ASSERT(q.isValid());

    rcIRect actual = q.orthogonalEnclosingRect();

    rcIRect expected(3, 3, 0, 0);
    rcUNITTEST_ASSERT(expected == actual);
  }

  {
    rcPolygon r;

    rc2Dvector r0(5,2), r1(7,3);
    r.pushVertex(r0); r.pushVertex(r1);

    for (int32 i = 0; i < r.vertexCnt(); i++) {
      rc2Dvector temp = r.vertexAt(0); // Rotate polygon by 1 position
      r.deleteVertex(0);
      r.pushVertex(temp);
      rcUNITTEST_ASSERT(r.isValid());

      rcIRect actual = r.orthogonalEnclosingRect();
      
      rcIRect expected(5, 2, 2, 1);
      rcUNITTEST_ASSERT(expected == actual);
    }
  }

  {
    rcPolygon s;

    rc2Dvector s0(9,2), s1(9,3);
    s.pushVertex(s0); s.pushVertex(s1);

    for (int32 i = 0; i < s.vertexCnt(); i++) {
      rc2Dvector temp = s.vertexAt(0); // Rotate polygon by 1 position
      s.deleteVertex(0);
      s.pushVertex(temp);
      rcUNITTEST_ASSERT(s.isValid());
      
      rcIRect actual = s.orthogonalEnclosingRect();
      
      rcIRect expected(9, 2, 0, 1);
      rcUNITTEST_ASSERT(expected == actual);
    }
  }

  {
    rcPolygon t;

    rc2Dvector t0(7,1), t1(9,1);
    t.pushVertex(t0); t.pushVertex(t1);

    for (int32 i = 0; i < t.vertexCnt(); i++) {
      rc2Dvector temp = t.vertexAt(0); // Rotate polygon by 1 position
      t.deleteVertex(0);
      t.pushVertex(temp);
      rcUNITTEST_ASSERT(t.isValid());
      
      rcIRect actual = t.orthogonalEnclosingRect();
      
      rcIRect expected(7, 1, 2, 0);
      rcUNITTEST_ASSERT(expected == actual);
    }
  }

  {
    rcPolygon u;

    rc2Dvector u0(0,0), u1(10,0), u2(10,5), u3(0,5);
    u.pushVertex(u0); u.pushVertex(u1);
    u.pushVertex(u2); u.pushVertex(u3);

    for (int32 i = 0; i < u.vertexCnt(); i++) {
      rc2Dvector temp = u.vertexAt(0); // Rotate polygon by 1 position
      u.deleteVertex(0);
      u.pushVertex(temp);
      rcUNITTEST_ASSERT(u.isValid());
      
      rcIRect actual = u.orthogonalEnclosingRect();
      
      rcIRect expected(0, 0, 10, 5);
      rcUNITTEST_ASSERT(expected == actual);
    }
  }

  {
    rcPolygon v;

    rc2Dvector v0(3,-3), v1(3,3), v2(-3,3), v3(-3,-3);
    v.pushVertex(v0); v.pushVertex(v1);
    v.pushVertex(v2); v.pushVertex(v3);

    for (int32 i = 0; i < v.vertexCnt(); i++) {
      rc2Dvector temp = v.vertexAt(0); // Rotate polygon by 1 position
      v.deleteVertex(0);
      v.pushVertex(temp);
      rcUNITTEST_ASSERT(v.isValid());

      rcIRect actual = v.orthogonalEnclosingRect();
      
      rcIRect expected(-3, -3, 6, 6);
      rcUNITTEST_ASSERT(expected == actual);
    }
  }

  {
    rcPolygon w;

    rc2Dvector w0(7,-1), w1(13,6), w2(10,8), w3(10,10), w4(3,7);
    w.pushVertex(w0); w.pushVertex(w1);
    w.pushVertex(w2); w.pushVertex(w3);
    w.pushVertex(w4);

    for (int32 i = 0; i < w.vertexCnt(); i++) {
      rc2Dvector temp = w.vertexAt(0); // Rotate polygon by 1 position
      w.deleteVertex(0);
      w.pushVertex(temp);
      rcUNITTEST_ASSERT(w.isValid());

      rcIRect actual = w.orthogonalEnclosingRect();
      
      rcIRect expected(3, -1, 10, 11);
      rcUNITTEST_ASSERT(expected == actual);
    }
  }
}

void UT_Polygon::testDiscBuffer ()
{
  {
    rc2Dvector p0 (100, 100), p1 (150, 150), p2 (50, 150);
    rcPolygon p;

    rcUNITTEST_ASSERT(p.vertexCnt() == 0);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.pushVertex(p0);
    p.pushVertex(p1);
    p.pushVertex(p2);

    rcUNITTEST_ASSERT(p.vertexCnt() == 3);
    rcUNITTEST_ASSERT(p.vertexAt(0) == p0);
    rcUNITTEST_ASSERT(p.vertexAt(1) == p1);
    rcUNITTEST_ASSERT(p.vertexAt(2) == p2);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    rcPolygon b = p.discBuffer (5.0);
    rcUNITTEST_ASSERT(b.vertexCnt() == 6);
    rcUNITTEST_ASSERT(b.isValid() == true);
    rcUNITTEST_ASSERT(b.isConvex() == true);

    rcUTCheck (rfRealEq (b.vertexAt(0).x(), 96.0));
    rcUTCheck (rfRealEq (b.vertexAt(0).y(), 96.0));
    rcUTCheck (rfRealEq (b.vertexAt(1).x(), 104.0));
    rcUTCheck (rfRealEq (b.vertexAt(1).y(), 96.0));
    rcUTCheck (rfRealEq (b.vertexAt(2).x(), 155.0));
    rcUTCheck (rfRealEq (b.vertexAt(2).y(), 150.0));
    rcUTCheck (rfRealEq (b.vertexAt(3).x(), 154.0));
    rcUTCheck (rfRealEq (b.vertexAt(3).y(), 154.0));
    rcUTCheck (rfRealEq (b.vertexAt(4).x(), 46.0));
    rcUTCheck (rfRealEq (b.vertexAt(4).y(), 154.0));
    rcUTCheck (rfRealEq (b.vertexAt(5).x(), 45.0));
    rcUTCheck (rfRealEq (b.vertexAt(5).y(), 150.0));
  }
}

void UT_Polygon::testPolygonGroup ()
{
	rc2Dvector p0 (100, 100), p1 (150, 150), p2 (50, 150);
    rcPolygon p;

    rcUNITTEST_ASSERT(p.vertexCnt() == 0);
    rcUNITTEST_ASSERT(p.isValid() == true);
    rcUNITTEST_ASSERT(p.isConvex() == true);

    p.pushVertex(p0);
    p.pushVertex(p1);
    p.pushVertex(p2);
	
	rcSharedPolygonGroupPtr c0 = new rcPolygonGroup;
	rcPolygonGroupRef cref (c0);
	rcUTCheck (cref.size () == 0);
	cref.push_back (p);
	rcUTCheck (cref.size () == 1);
	rcUTCheck (cref[0] == p);
	
}