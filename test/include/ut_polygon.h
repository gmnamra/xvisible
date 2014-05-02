// Copyright 2003 Reify, Inc.

#ifndef _UT_POLYGON_H_
#define _UT_POLYGON_H_

#include "rc_unittest.h"
#include "rc_polygon.h"
#include "rc_polygongroup.h"

class UT_Polygon : public rcUnitTest {
public:

    UT_Polygon();
    ~UT_Polygon();

    virtual uint32 run();

  private:

    void testBasic();
    void testGeomProperties();

    void testConvexHullConvert();

    void testIntersects();
    void testContains();
    void testPolygonCombine();

    void testMinEncRect();
    void testOrthEncRect();
    void testDiscBuffer ();
	void testPolygonGroup ();

};

#endif // _UT_POLYGON_H_
