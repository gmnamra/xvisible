/*
 *  rc_polygon.h
 *
 *  Created by Peter Roberts on Tue Oct 17 2003.
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 */

#ifndef _rcPOLYGON_H_
#define _rcPOLYGON_H_

#include <rc_types.h>
#include <rc_vector2d.h>
#include <rc_affine.h>
#include <rc_xforms.h>
#include <rc_polygon_edge.h>

#include <algorithm> // for reverse, unique
#include <iostream>
#include <string>
#include <vector>

#if 0
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>
#include <boost/geometry/multi/multi.hpp>


using namespace boost::geomtry;

class Polygon : polygon_2d
{
	/* ctor - 
     Create a polygon for a circle of this radius or
     an ellipse with a, b
	 * For our purposes, these are
	 * considered to be both valid and convex. 
	 * Use translate to move it where you want it
	 * @note should use rcCircle or rcEllipse
	 */
	
	rcPolygon (const float , const int32);
	rcPolygon (const float , const float, const int32);
	rcPolygon (const rcAffineRectangle& af);
	
		// void clear() same as base class
	
	/* centerOf - Returns the area circumscribed by this polygon. The
	 * polygon must be valid. This is an O(N) operation.
	 * centerOf is cached. All vertex operations as well as translate and transform
	 * invalidate the center.
	 */
	bool centerOf(point_2d& cent) const { centroid (*this, cent); }
	
	/* Area - Returns area this polygon. The
	 * polygon must be valid. This is an O(N) operation.
	 * Negative area indicates abnormal polygon. 
	 * @note bring back the const
	 */
	double area() const { return area(*this); }
	
	/* perimeter - Returns the length of the polygon's perimeter. This
	 * is an O(N) operation.
	 * Note: The polygon need not be valid.
	 */
	double perimeter() const  { return perimeter (*this); }
	
	/* Circularity - Returns "projection sphericity". It equals to 1 for 
	 * any disc and less than 1 for all other shapes. This is an O(N) operation.
	 * Note: The polygon need not be valid.
	 * Reference:Volodymyr Kindratenko (Simple Geometric Shapes) (Arman)
	 */
	double circularity () const;
	
	/* elipseRatio - Returns ratio of long and short semi axis of an ellipse
	 * with area and perimeter equal to this polygon. 
	 * This is an O(N) operation.
	 * Note: The polygon need not be valid.
	 * Reference:Volodymyr Kindratenko (Simple Geometric Shapes) (Arman)
	 */
	
	
	/* contains - Check that the point is contained within this
	 * polygon. The polygon must be valid. This is in O(N) operation.
	 */
	bool contains(const point_2d& point) const { return within (point, *this); }
	
	/* contains - Check that the specified polygon is contained
	 * within this polygon. Both polygons must be valid. If either
	 * polygon is not convex then the algorithm is O(N**2). Otherwise, a
	 * special convex intersection function is called that is O(N).
	 */
	bool contains(const rcPolygon& polygon) const;
	
	/* intersects - Check that the specified polygons intersects. Both
	 * polygons must be valid. If either polygon is not convex then the
	 * algorithm is O(N**2). Otherwise, a special convex intersection
	 * function is called that is O(N).
	 *
	 * The group overload return a vector of rcPolygon (copies) that intersect from a vector of polygons. 
	 * @note use a different design or make polygons a smart pointer class
	 */
	bool intersects(const rcPolygon& polygon) const;
	bool intersects (const vector<rcPolygon>& group, vector<rcPolygon>& groupIntersects, float areaRatio = 0.25f) const;
	int32 intersects (const vector<rcPolygon>& group, float areaRatio = 0.25f) const;
	
	/* operator== - Checks that the two polygons have exactly the same
	 * points stored in the same relative order. So, { (0,0) (1,1) (2,2) }
	 * matches { (2,2) (0,0) (1,1) }, but not { (0,0) (2,2) (1,1) }
	 */
	friend bool operator==(const rcPolygon& a, const rcPolygon& b);
	friend bool operator!=(const rcPolygon& a, const rcPolygon& b);
	
	
#endif
	

class rcPolygon;

/* rfPolygonToSegmentsCollection - Takes the source polygon p and puts a
 * displayable version of it into v.
 */
    //void rfPolygonToSegmentsCollection(const rcPolygon& p, rcVisualSegmentCollection& v, bool drawCOM = false);


    //void rfGenerateSmoothingPts(const rcPolygon& p,    vector<rc2Dvector>& smoothedPts,   double smoothingDistance);

class rcPolygon 
{
 public:

  typedef double FT;
  typedef rc2dVector<FT>  Point_2;
  typedef std::deque<Point_2> Container_P;
  typedef  Container_P::difference_type difference_type;
  typedef  Container_P::value_type value_type;
  typedef  Container_P::pointer pointer;
  typedef  Container_P::reference reference;
  typedef  Container_P::const_reference const_reference;
  typedef  Container_P::iterator       Vertex_iterator;
  typedef  Container_P::iterator       Vertex_const_iterator;
  typedef Polygon_2_edge_iterator<Container_P> Edge_const_iterator;

  /* ctor - Create an empty polygon. For our purposes, these are
   * considered to be both valid and convex.
   */
  rcPolygon();
  /*
   * Compiler Generated copy ctor, assignment, and dtor ok
   */ 

  /* ctor - 
     Create a polygon for a circle of this radius or
     an ellipse with a, b
   * For our purposes, these are
   * considered to be both valid and convex. 
   * Use translate to move it where you want it
   * @note should use rcCircle or rcEllipse
   */

  rcPolygon (const float , const int32);
  rcPolygon (const float , const float, const int32);
  rcPolygon (const rcAffineRectangle& af);

  /* MUTATORS
   */
  /* Functions to manipulate the vertices in a polygon. Note that the
   * internal state of the polygon goes to unknown (isValid and
   * isConvex states go to unknown) after a call to any of these
   * functions.
   *
   * Note: Though vertices are represented as pairs of doubles, the
   * input coordinates are rounded to their nearest integer value.
   *
   * Note: Whenever an index to a vertex is passed as an argument
   * (functions deleteVertex(), insertVertex() and vertexAt()), modulo
   * arithmetic (modulo vertex count) is done on the index before it
   * is used. This allows insertVertex() to be used to add a vertex to
   * the start, by specifying vertexCnt() as the index argument.
   *
   * Note: When either popVertex() or deleteVertex() is called, the
   * polygon must aleady have at least 1 vertex.
   *
   * Note: pushUniqueVertex() will only add the vertex if it does not
   * match the last one. Returns true if the push was successful,
   * false otherwise.
   */
  void pushVertex(const rc2Dvector& vertex);
  bool pushUniqueVertex(const rc2Dvector& vertex);
  void popVertex();
  void deleteVertex(int32 index);
  void insertVertex(const rc2Dvector& vertex, int32 after);

  void erase(Vertex_iterator pos)
  { _vert.erase(pos); }

  void erase(Vertex_iterator first, Vertex_iterator last)
  {
    _vert.erase(first, last);
  }

  void clear()
  {
    _vert.clear();
  }

  void reverse_orientation()
  {
    if (size() <= 1)
      return;
    Container_P::iterator i = _vert.begin();
    std::reverse(++i, _vert.end());
  }

  /* translate - Move the polygon over delta by adding delta to each
   * vertex in the polygon. Note that this doesn't invalidate any
   * previously cached "is valid" or "is convex" information.
   */
  void translate(const rc2Dvector& delta);
  void translate(const rc2Fvector& delta);

  /* transform - transform the polygon by mapping each
   * vertex in the polygon. Note that this doesn't invalidate any
   * previously cached "is valid" or "is convex" information.
   */
  void transform (const rc2Xform& xfm);

  /* convexHullConvert - Convert this polygon into its convex
   * hull. The polygon must be valid. This is an O(NLOGN) operation.
   *
   * Note: The polygon will be both valid and convex after this call.
   */
  void convexHullConvert()
  { createConvexHull(*this); }

  /* ACCESSORS
   */
  const rc2Dvector& vertexAt(int32 index) const;

  /* isValid - Checks that the polygon meets the definition of a
   * simple closed curve. Essentially it checks that no points on the
   * curve either touch or intersect. Note that isValid is an O(N**2)
   * algorithm. If polygons with large numbers of vertices are to be
   * used, then a faster algorithm may be required.
   */
  bool isValid() const;

  /* isConvex - Checks that this is a valid polygon, with all interior
   * angles less than 180 degrees. Note that the class at all times
   * assumes that vertices are stored in counter-clockwise order. An
   * otherwise convex polygon, that has its vertices stored in the
   * reverse direction will not be found to be convex. Assuming the
   * polygon is already known to be valid, this is an O(N) operation.
   */
  bool isConvex() const;

  int32 vertexCnt() const;
  int32 size () const;

  /* OPERATORS
   */

  /* contains - Check that the point is contained within this
   * polygon. The polygon must be valid. This is in O(N) operation.
   */
  bool contains(const rc2Dvector& point) const;

  /* contains - Check that the specified polygon is contained
   * within this polygon. Both polygons must be valid. If either
   * polygon is not convex then the algorithm is O(N**2). Otherwise, a
   * special convex intersection function is called that is O(N).
   */
  bool contains(const rcPolygon& polygon) const;

  /* intersects - Check that the specified polygons intersects. Both
   * polygons must be valid. If either polygon is not convex then the
   * algorithm is O(N**2). Otherwise, a special convex intersection
   * function is called that is O(N).
   *
   * The group overload return a vector of rcPolygon (copies) that intersect from a vector of polygons. 
   * @note use a different design or make polygons a smart pointer class
   */
  bool intersects(const rcPolygon& polygon) const;
  bool intersects (const vector<rcPolygon>& group, vector<rcPolygon>& groupIntersects, float areaRatio = 0.25f) const;
  int32 intersects (const vector<rcPolygon>& group, float areaRatio = 0.25f) const;

  /* operator== - Checks that the two polygons have exactly the same
   * points stored in the same relative order. So, { (0,0) (1,1) (2,2) }
   * matches { (2,2) (0,0) (1,1) }, but not { (0,0) (2,2) (1,1) }
   */
  friend bool operator==(const rcPolygon& a, const rcPolygon& b);
  friend bool operator!=(const rcPolygon& a, const rcPolygon& b);

  /* centerOf - Returns the area circumscribed by this polygon. The
   * polygon must be valid. This is an O(N) operation.
   * centerOf is cached. All vertex operations as well as translate and transform
   * invalidate the center.
   */
  bool centerOf(rc2Dvector&) const;
  bool centerOf(rc2Fvector&) const;

  /* Area - Returns area this polygon. The
   * polygon must be valid. This is an O(N) operation.
   * Negative area indicates abnormal polygon. 
   * @note bring back the const
   */
  double area() const;

  /* perimeter - Returns the length of the polygon's perimeter. This
   * is an O(N) operation.
   * Note: The polygon need not be valid.
   */
  double perimeter() const;

  /* Circularity - Returns "projection sphericity". It equals to 1 for 
   * any disc and less than 1 for all other shapes. This is an O(N) operation.
   * Note: The polygon need not be valid.
   * Reference:Volodymyr Kindratenko (Simple Geometric Shapes) (Arman)
   */
  double circularity () const;

  /* elipseRatio - Returns ratio of long and short semi axis of an ellipse
   * with area and perimeter equal to this polygon. 
   * This is an O(N) operation.
   * Note: The polygon need not be valid.
   * Reference:Volodymyr Kindratenko (Simple Geometric Shapes) (Arman)
   */
  double ellipseRatio () const;

  /* convexIntersection - Takes the convex hull's of both polygon and
   * this and stores the resulting intersection in polyInt. This is an
   * O(N) operation.
   *
   * Note: The coordinates for vertices generated creating
   * intersecting polygons are all rounded to their nearest integer
   * value.
   *
   * Note: Both this and polygon must be valid. If either is not
   * already a convex hull, the convex hull for it is first
   * generated. Conversion is an O(NLOGN) operation.
   *
   * Note: The final intersection, stored in polyInt, will be a convex
   * hull.
   */
  void convexIntersection(const rcPolygon& polygon, rcPolygon& polyInt) const;

  /* convexUnion - Takes the convex hull's of both polygon and this
   * and stores the convex hull of the resulting "union" in polyUnion.
   *
   * Note: Both this and polygon must be valid. If either is not
   * already a convex hull, the convex hull for it is first generated.
   * Conversion is an O(NLOGN) operation.
   *
   * Note: The final union, stored in polyUnion, will be a convex
   * hull.
   */
  void convexUnion(const rcPolygon& polygon, rcPolygon& polyUnion) const;

  /* convexHull - Takes the convex hull of this polygon and stores it
   * in polyHull. This is an O(NLOGN) operation.
   */
  void convexHull(rcPolygon& polyHull) const
  { createConvexHull(polyHull); }

  /* minimumAreaEnclosingRect/minimumPerimeterEnclosingRect - Returns
   * a minimum enclosing oriented rectangle. This is an O(N)
   * operation.
   *
   * Note: This must be valid. If it is not already a convex hull,
   * this is first generated. Conversion is an O(NLOGN) operation.
   */
  rcAffineRectangle minimumAreaEnclosingRect() const
  { return genMinEnclosingRect(true); }
  rcAffineRectangle minimumPerimeterEnclosingRect() const
  { return genMinEnclosingRect(false); }

  /* orthogonalEnclosingRect - Returns the minimum enclosing rectangle
   * that is orthogonal to the image plane.
   *
   * Note: This must be valid.
   */
  rcIRect orthogonalEnclosingRect() const;
  void orthogonalEnclosingRect(rcDRect&) const;
  void orthogonalEnclosingRect(rcFRect&) const;

  /* Returns a polygon that is a "buffer" to this polygon using distance as the 
   * radius of a ball that was "morphed" at all vertices. 
   * This is O(NN) operation.
   * Requires a convex hull 
   */
  rcPolygon discBuffer (double diameter) const;

  // Default copy ctor and assignment operators OK.

  friend ostream& operator<< (ostream&, const rcPolygon&);

  // Traversing a polygon
  Vertex_const_iterator vertices_begin() const
  { return const_cast<rcPolygon&>(*this)._vert.begin(); }

  Vertex_const_iterator vertices_end() const
  { return const_cast<rcPolygon&>(*this)._vert.end(); }

  Edge_const_iterator edges_begin() const
  { return Edge_const_iterator(&_vert, _vert.begin()); }

  Edge_const_iterator edges_end() const
  { return Edge_const_iterator(&_vert, _vert.end()); }

 private:

  enum eInState {
    eInUnknown = 0,
    ePIn,
    eQIn
  };

  enum eIntersection {
    eInterUnknown = 0,
    eNoIntersection,
    eProperIntersection,
    eVertexIntersection,
    eSharedCollinear
  };

  double triArea2(const rc2Dvector& a, const rc2Dvector& b,
		  const rc2Dvector& c) const;
    
  double polyArea2() const;

  inline bool leftTurn(const rc2Dvector& a, const rc2Dvector& b,
		const rc2Dvector& c) const;

  inline bool leftTurnOrOn(const rc2Dvector& a, const rc2Dvector& b,
		    const rc2Dvector& c) const;
  
  bool collinear(const rc2Dvector& a, const rc2Dvector& b,
		 const rc2Dvector& c) const;

  bool intersection(const rc2Dvector& a0, const rc2Dvector& a1,
		    const rc2Dvector& b0, const rc2Dvector& b1,
		    rc2Dvector* inter) const;

  void createConvexHull(rcPolygon& poly, bool mastBeValid = true) const;

  int32 areaSign(const rc2Dvector& a, const rc2Dvector& b,
		   const rc2Dvector& c) const;

  bool between(const rc2Dvector& a, const rc2Dvector& b,
	       const rc2Dvector& c) const;

  eIntersection parallelInter(const rc2Dvector& a0, const rc2Dvector& a1,
			      const rc2Dvector& b0, const rc2Dvector& b1,
			      rc2Dvector& inter1, rc2Dvector& inter2) const;

  eIntersection segmentInter(const rc2Dvector& a0, const rc2Dvector& a1,
			     const rc2Dvector& b0, const rc2Dvector& b1,
			     rc2Dvector& inter1, rc2Dvector& inter2) const;

  bool convexComparedToConvex(const rcPolygon& polygon,
			      const bool contains) const;
  
  int32 findPeak(const int32 startIndex,
		   const rc2Dvector& b0, const rc2Dvector& b1) const;

  rc2Dvector crossing(const rc2Dvector& b0, const rc2Dvector& b1,
		      const rc2Dvector& a) const;

  void genMinRectBounds(const rc2Dvector& topPt, const rc2Dvector& btmPt,
			const rc2Dvector& leftPt, const rc2Dvector& rightPt,
			double mx, double my,
			rc2Dvector& llPt, rc2Dvector& dim) const;

  rcAffineRectangle genMinEnclosingRect(bool area) const;

  const deque<rc2Dvector>& vert() const { return _vert; }

  deque<rc2Dvector> _vert;
  bool              _isConvex, _isValid, _knownConvex, _knownValid, _comValid, _areaValid;
  rc2Dvector _com;
  double _area;
};

inline bool operator!=(const rcPolygon& a, const rcPolygon& b)
{
  return !operator==(a, b);
}



#endif
