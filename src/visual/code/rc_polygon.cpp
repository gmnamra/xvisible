/*
 *  rc_polygon.cpp
 *
 *  Created by Peter Roberts on Tue Oct 17 2003.
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 * Unless otherwise noted, all the algorithms here are derived from
 * "Computational Geometry in C" by Joseph O'Rourke.
 */

#include <rc_polygon.h>
#include <map>
#include <set>
#include <iostream>
#include <rc_macro.h>

#define DBGPRT  0
#define DBGPRT2 0
#define DBGPRT3 0


void rfPolygonToSegmentsCollection(const rcPolygon& p,
				   rcVisualSegmentCollection& v, bool drawCOM)
{
  rc2Dvector pt1, pt2;
  int32 vCnt = p.vertexCnt();

  if (vCnt == 0) {
    return;
  }
  else if (vCnt == 1) {
    pt1 = p.vertexAt(0);
    v.push_back(rcVisualCross(rc2Fvector((float)pt1.x(), (float)pt1.y()),
			      rc2Fvector((float)3.0, (float)3.0)));
  }
  else if (vCnt == 2) {
    pt1 = p.vertexAt(0);
    pt2 = p.vertexAt(1);
    v.push_back(rcVisualLine(rc2Fvector((float)pt1.x(), (float)pt1.y()),
			     rc2Fvector((float)pt2.x(), (float)pt2.y())));
  }
  else if (vCnt & 1) {
    const int32 segCnt = vCnt/2 + 1;

    for (int32 si = 0; si < segCnt-1; si++) {
      pt1 = p.vertexAt(si*2);
      pt2 = p.vertexAt(si*2+1);
      v.push_back(rcVisualLineStrip(rc2Fvector((float)pt1.x(),(float)pt1.y()),
				    rc2Fvector((float)pt2.x(),(float)pt2.y())));
    }

    pt1 = p.vertexAt(vCnt-1);
    pt2 = p.vertexAt(0);
    v.push_back(rcVisualLineStrip(rc2Fvector((float)pt1.x(), (float)pt1.y()),
				  rc2Fvector((float)pt2.x(), (float)pt2.y())));
  }
  else {
    const int32 segCnt = vCnt/2;

    for (int32 si = 0; si < segCnt; si++) {
      pt1 = p.vertexAt(si*2);
      pt2 = p.vertexAt(si*2+1);
      v.push_back(rcVisualLineLoop(rc2Fvector((float)pt1.x(), (float)pt1.y()),
				   rc2Fvector((float)pt2.x(), (float)pt2.y())));
    }
  }

  if (drawCOM)
    {
      rc2Fvector com;
      p.centerOf (com);
      rcIRect r = p.orthogonalEnclosingRect ();
      bool aspect = r.width() > r.height();
      v.push_back(rcVisualCross (com, rc2Fvector (aspect ? 7.0f : 5.0f, aspect ? 5.0f : 7.0f)));
    }

  v.push_back(rcVisualEmpty());
}

void rfGenerateSmoothingPts(const rcPolygon& p, vector<rc2Dvector>& smoothedPts,
			    double smoothingDistance)
{
  smoothedPts.clear();
  
  const int32 vertexCnt = p.vertexCnt();

  if (vertexCnt == 0)
    return;

  rc2Dvector curPt = p.vertexAt(0);
  smoothedPts.push_back(p.vertexAt(0));

  if (vertexCnt == 1)
    return;

  double curDistance = 0.0;
  for (int32 pi = 1; pi < vertexCnt; pi++) {
    const rc2Dvector nextPt = p.vertexAt(pi);
    curDistance += curPt.distance(nextPt);
    if (curDistance >= smoothingDistance) {
      smoothedPts.push_back(nextPt);
      curDistance = 0.0;
    }
    curPt = nextPt;
  }
}

rcPolygon::rcPolygon() : _isConvex(true), _isValid(true),
			 _knownConvex(true), _knownValid(true), 
			 _comValid (false),  _areaValid (false)
{
}

rcPolygon::rcPolygon(const float r, const int32 n) : _isConvex(true), _isValid(true),
			 _knownConvex(true), _knownValid(true),
			 _comValid (false),  _areaValid (false)
{
  if (n <= 0) return;
  for (int32 i = 0; i < n; i++)
    {
      double x = ( -r * sin ((2 * i * rkPI) / n));
      double y = (  r * cos ((2 * i * rkPI) / n));
      rc2Dvector p (x, y);
      pushVertex (p);
      if (!isConvex()) return;
    }
}

rcPolygon::rcPolygon(const float a, const float b, const int32 n) : _isConvex(true), _isValid(true),
			 _knownConvex(true), _knownValid(true),
			 _comValid (false),  _areaValid (false)
{
  if (n <= 0) return;
  for (int32 i = 0; i < n; i++)
    {
      double x = ( -a * sin ((2 * i * rkPI) / n));
      double y = (  b * cos ((2 * i * rkPI) / n));
      rc2Dvector p (x, y);
      pushVertex (p);
      if (!isConvex()) return;
    }
}


rcPolygon::rcPolygon(const rcAffineRectangle& af) : _isConvex(true), _isValid(true),
			 _knownConvex(true), _knownValid(true), _comValid (false)
{
  if (! af.isValid ()) return;
  const rc2Dvector ul = af.affineToImage(rc2Dvector(0, 0));
  const rc2Dvector ur = af.affineToImage(rc2Dvector(1, 0));
  const rc2Dvector ll = af.affineToImage(rc2Dvector(0, 1));
  const rc2Dvector lr = af.affineToImage(rc2Dvector(1, 1));
  pushVertex (ul);
  if (!isConvex()) return;  
  pushVertex (ur);
  if (!isConvex()) return;
  pushVertex (lr);
  if (!isConvex()) return;
  pushVertex (ll);
  if (!isConvex()) return;
}


void rcPolygon::pushVertex(const rc2Dvector& vertex)
{
  rc2Dvector rounded(floor(vertex.x() + 0.5), floor(vertex.y() + 0.5));

  _vert.push_back(rounded);

  _isConvex = _isValid = _knownValid = _knownConvex = _comValid = _areaValid = false;

}

bool rcPolygon::pushUniqueVertex(const rc2Dvector& vertex)
{
  rc2Dvector rounded(floor(vertex.x() + 0.5), floor(vertex.y() + 0.5));

  if (!_vert.empty() && (_vert.back() == rounded))
    return false;

  _vert.push_back(rounded);

  _isConvex = _isValid = _knownValid = _knownConvex = _comValid = _areaValid = false;
  return true;
}

void rcPolygon::popVertex()
{
  rmAssert(!_vert.empty());

  _vert.pop_back();

  _isConvex = _isValid = _knownValid = _knownConvex = _comValid = _areaValid = false;
}

void rcPolygon::deleteVertex(int32 index)
{
  rmAssert(!_vert.empty());

  index = index % (int32)_vert.size();
  _vert.erase(_vert.begin() + index);

  _isConvex = _isValid = _knownValid = _knownConvex = _comValid = _areaValid = false;
}

void rcPolygon::insertVertex(const rc2Dvector& vertex, int32 after)
{
  rc2Dvector rounded(floor(vertex.x() + 0.5), floor(vertex.y() + 0.5));

  if (_vert.size()) {
    after = (after+1) % (int32)(_vert.size() + 1);
    _vert.insert(_vert.begin() + after, rounded);
  }
  else
    _vert.push_back(rounded);

  _isConvex = _isValid = _knownValid = _knownConvex = _comValid = _areaValid = false;
}

void rcPolygon::translate(const rc2Fvector& delta)
{
  rc2Dvector d (delta.x(), delta.y());
  translate (d);
}

void rcPolygon::translate(const rc2Dvector& delta)
{
  rc2Dvector rounded(floor(delta.x() + 0.5), floor(delta.y() + 0.5));

  int32 vCnt = vertexCnt();

  for (int32 i = 0; i < vCnt; i++)
    _vert[i] = _vert[i] + rounded;
  _comValid = false;
  _areaValid = false;
}

void rcPolygon::transform(const rc2Xform& xfm)
{
  int32 vCnt = vertexCnt();

  for (int32 i = 0; i < vCnt; i++)
    const_cast<rcPolygon *>(this)->_vert[i] = xfm.mapPoint (const_cast<rcPolygon *>(this)->_vert[i]) + 0.5;
  const_cast<rcPolygon *>(this)->_comValid = false;
  const_cast<rcPolygon *>(this)->_areaValid = false;
}

const rc2Dvector& rcPolygon::vertexAt(int32 index) const
{
  rmAssert(_vert.size());

  index = index % (int32)_vert.size();

  return _vert[index];
}

bool rcPolygon::contains(const rc2Dvector& point) const
{
  rmAssert(isValid());

  //  rc2Dvector rounded(floor(point.x() + 0.5), floor(point.y() + 0.5));
  rc2Dvector rounded = point;

  uint32 vCnt = _vert.size();
  if (vCnt < 3) {
    if (vCnt == 0)
      return false;
    
    if (vCnt == 1)
      return rounded == _vert[0];

    /* vCnt == 2
     */
    rc2Dvector inter = crossing(_vert[0], _vert[1], rounded);
    return inter == rounded && between(_vert[0], _vert[1], rounded);
  }

  uint32 crossings = 0;

  rc2Dvector normHead = _vert[vCnt-1] - rounded;

  for (uint32 headI = 0; headI < vCnt; headI++) {
    /* Normalize all vertices to have their origin at point. In this
     * way we can check to see if edges intersect the ray formed by
     * starting at the origin and moving in a positive direction along
     * the x axis.
     */
    rc2Dvector normTail = normHead;
    normHead = _vert[headI] - rounded;

    if (normHead == rc2Dvector(0,0))
      return true;
    else if ((normTail.y() == 0 && normHead.y() == 0) &&
	     ((normHead.x() > 0 && normTail.x() < 0) ||
	      (normHead.x() < 0 && normTail.x() > 0)))
      return true;

    /* Check to see if the edge created by these two vertices
     * crosses the y axis.
     */
    if (((normHead.y() > 0) && (normTail.y() <= 0)) ||
	((normTail.y() > 0) && (normHead.y() <= 0))) {
      /* If so, find the x value for the location where the edge
       * crosses the y axis. If the crossing point is positive, the
       * edge crosses the ray.
       */
      double x = normHead.cross(normTail)/(normTail.y() - normHead.y());
      if (x == 0)
	return true;

      if (x > 0)
	crossings++;
    }
  }
  
  /* An odd number of crossings imply the point is inside the polygon,
   * otherwise it lies outside it.
   */
  return (crossings & 1) == 1;
}

bool rcPolygon::contains(const rcPolygon& polygon) const
{
  rmAssert(isValid());
  rmAssert(polygon.isValid());

  const int32 tEnd = _vert.size();
  const int32 bEnd = polygon._vert.size();

  if ((tEnd < 2) || (bEnd < 2)) {
    if (bEnd == 0)
      return true;
    else if (tEnd == 0)
      return false;

    if (tEnd == 1) {
      if (bEnd == 1)
	return _vert[0] == polygon._vert[0];
      else // bEnd > 1 ==> polygon is at least a 2D thing.
	return false;
    }
    
    /* tEnd >= 2 ==> bEnd == 1
     */
    rmAssert(bEnd == 1);
    return contains(polygon._vert[0]);
  }

  /* If the 2 polygons are convex, an O(N) algorithm, based on the
   * convex polygon intersecion code, can be used.  Otherwise, in lieu
   * of any faster known algorithm, the following O(N^2) algorithm is
   * used.
   */
  if (isConvex() && polygon.isConvex())
    return convexComparedToConvex(polygon, true);

  /* Go through and see if any of the edges in this cross any of the
   * edges in polygon. If so, this does not contain polygon. If not,
   * then check to see if any single point in polygon is contained in
   * *this. If this is true, then this contains polygon.
   */
  int32 tHeadI = 0; 
  int32 tTailI = tEnd - 1;
  rc2Dvector i1, i2; // Used to to store values calculated by segmentInter().
  
  vector<bool> onEdge(bEnd), isCollinear(bEnd);
  for (int32 i = 0; i < bEnd; i++)
    onEdge[i] = isCollinear[i] = false;

  while (tHeadI != tEnd) {
    int32 bHeadI = 0;
    int32 bTailI = bEnd - 1;
    while(bHeadI != bEnd) {
      switch (segmentInter(_vert[tHeadI], _vert[tTailI],
			   polygon._vert[bHeadI], polygon._vert[bTailI],
			   i1, i2))
      {
      case eNoIntersection:
	break;

      case eProperIntersection:
	if (DBGPRT)
	  cout << "Return false at: case eProperIntersection" << endl;
	return false;

      case eVertexIntersection:
	if (i1 == polygon._vert[bHeadI])
	  onEdge[bHeadI] = true;
	else if (i1 != polygon._vert[bTailI]) {
	  if (DBGPRT)
	    cout << "Return false at: case eVertexIntersection" << endl;
	  return false;
	}
	break;

      case eSharedCollinear:
	onEdge[bHeadI] = true;
	isCollinear[bHeadI] = true;
	break;

      case eInterUnknown:
	rmAssert(0);
	break;
     }

      bTailI = bHeadI;
      bHeadI++;
    }
    tTailI = tHeadI;
    tHeadI++;
  }

  /* No proper intersections were found. Look to see if any of the
   * touching edges can be used to find points outside of this.
   */
  int32 bHeadI = 0;
  int32 bTailI = bEnd - 1;
  while(bHeadI != bEnd) {
    if (onEdge[bHeadI] && !isCollinear[bHeadI]) {
      if (onEdge[bTailI]) {
	double x = polygon._vert[bHeadI].x() +
	  (polygon._vert[bTailI].x() - polygon._vert[bHeadI].x())/2;
	double y = polygon._vert[bHeadI].y() +
	  (polygon._vert[bTailI].y() - polygon._vert[bHeadI].y())/2;
	rc2Dvector point(x, y);

	if (!contains(point)) {
	  if (DBGPRT)
	    cout << "Return false at: if (!contains(point))" << endl;
	  return false;
	}
      }
      else if (!contains(polygon._vert[bTailI])) {
	if (DBGPRT)
	  cout << "Return false at: if (!contains(polygon._vert[bTailI]))" << endl;
	return false;
      }
    }

    bTailI = bHeadI;
    bHeadI++;
  }

  if (DBGPRT)
    cout << "Returning: " <<  contains(polygon._vert[0]) << endl;

  /* Either all the vertices in polygon are contained within this or
   * they are all outside of this.
   */
  return contains(polygon._vert[0]);
}

bool rcPolygon::intersects(const rcPolygon& polygon) const
{
  rmAssert(isValid());
  rmAssert(polygon.isValid());

  const int32 tEnd = _vert.size();
  const int32 bEnd = polygon._vert.size();

  if ((tEnd < 2) || (bEnd < 2)) {
    if (bEnd == 0 || tEnd == 0)
      return false;

    if (bEnd == 1 && tEnd == 1)
	return _vert[0] == polygon._vert[0];

    if (bEnd == 1) // and tEnd > 2
      return contains(polygon._vert[0]);
    else // tEnd == 1 and bEnd > 2
      return polygon.contains(_vert[0]);
  }

  /* If the 2 polygons are convex, an O(N) algorithm, based on the
   * convex polygon intersecion code, can be used.  Otherwise, in lieu
   * of any faster known algorithm, the following O(N^2) algorithm is
   * used.
   */
  if (isConvex() && polygon.isConvex())
    return convexComparedToConvex(polygon, false);

  /* Go through and see if any of the edges in this cross any of the
   * edges in polygon. If so, they intersect. If not, then check to
   * see if the polygons are nested.
   */
  int32 tHeadI = 0; 
  int32 tTailI = tEnd - 1;
  rc2Dvector i1, i2; // Used to to store values calculated by segmentInter().
  
  while (tHeadI != tEnd) {
    int32 bHeadI = 0;
    int32 bTailI = bEnd - 1;
    while(bHeadI != bEnd) {
      switch (segmentInter(_vert[tHeadI], _vert[tTailI],
			   polygon._vert[bHeadI], polygon._vert[bTailI],
			   i1, i2))
      {
      case eNoIntersection:
	break;

      case eProperIntersection:
      case eVertexIntersection:
      case eSharedCollinear:
	return true;

      case eInterUnknown:
	rmAssert(0);
	break;
     }

      bTailI = bHeadI;
      bHeadI++;
    }
    tTailI = tHeadI;
    tHeadI++;
  }

  /* Either all the vertices in polygon are contained within this or
   * they are all outside of this.
   */
  return contains(polygon._vert[0]) || polygon.contains(_vert[0]);
}

bool rcPolygon::isValid() const
{
  if (_knownValid)
    return _isValid;

  rcPolygon* This = const_cast<rcPolygon*>(this);
  
  This->_knownValid = This->_isValid = true;

  int32 vertCnt = vertexCnt();

  if (vertCnt < 3)
    return true;

  for (int32 i = 1; i < vertCnt-1; i++) {
    /* Checks for duplicate points between consecutive line segments.
     */
    if ((_vert[i-1] == _vert[i]) ||
	(_vert[i-1] == _vert[i+1]) ||
	(_vert[i] == _vert[i+1])) {
      if (DBGPRT)
	cout << "Not valid 0: duplicates " << _vert[i-1] << _vert[i]
	     << _vert[i+1] << endl;
      This->_isValid = false;
      return false;
    }

    /* For all others but the first line segment, check for a crossing
     * with the last line segment.
     */
    if ((i != 1) && intersection(_vert[vertCnt-1], _vert[0],
				 _vert[i-1], _vert[i], 0)) {
      if (DBGPRT)
	cout << "Not valid 1: intersection " << _vert[vertCnt-1] << _vert[0]
	     << " and " << _vert[i-1] << _vert[i] << endl;
      This->_isValid = false;
      return false;
    }

    /* Now check for a crossing with all other line segments in the
     * polygon.
     */
    for (int32 j = i+2; j < vertCnt; j++) {
      if (intersection(_vert[i-1], _vert[i],
		       _vert[j-1], _vert[j], 0)) {
	if (DBGPRT)
	  cout << "Not valid 2: intersection " << _vert[i-1] << _vert[i]
	       << " and " << _vert[j-1] << _vert[j] << endl;
	This->_isValid = false;
	return false;
      }
    }
  } // End of: for (int32 i = 1; i < vertCnt; i++) {

  /* Finally, check for duplicate points between consecutive line
   * segments in the range [vertCnt-2, 1].
   */
  if ((_vert[vertCnt-2] == _vert[0]) ||
      (_vert[vertCnt-1] == _vert[0]) ||
      (_vert[vertCnt-1] == _vert[1])) {
    if (DBGPRT)
      cout << "Not valid 3: duplicates " << _vert[vertCnt-2]
	   << _vert[vertCnt-1] << _vert[1] << _vert[0] << endl;
    This->_isValid = false;
    return false;
  }

  return true;
}

bool rcPolygon::isConvex() const
{
  if (_knownConvex)
    return _isConvex;

  if (!_knownValid && !isValid())
    return false;

  rcPolygon* This = const_cast<rcPolygon*>(this);

  This->_knownConvex = This->_isConvex = true;

  int32 vertCnt = vertexCnt();

  if (vertCnt < 3)
    return true;

  for (int32 i = 2; i < vertCnt; i++)
    if (!leftTurn(_vert[i-2], _vert[i-1], _vert[i])) {
      This->_isConvex = false;
      return false;
    }

  if (!leftTurn(_vert[vertCnt-2], _vert[vertCnt-1], _vert[0]) ||
      !leftTurn(_vert[vertCnt-1], _vert[0], _vert[1]))
      This->_isConvex = false;

  return _isConvex;
}

int32 rcPolygon::vertexCnt() const
{
  return (int32)_vert.size();
}

// @note let negative area speak for itself
double rcPolygon::area() const 
{
  rmAssert(isValid());

  if (_areaValid == false)
    {
	  const_cast<rcPolygon *>(this)->_area = polyArea2() / 2;
      const_cast<rcPolygon *>(this)->_areaValid = true;
    }

  return _area;
}

double rcPolygon::perimeter() const
{
  if (_vert.size() < 2)
    return 0;

  deque<rc2Dvector>::const_iterator pHeadI = _vert.begin();
  deque<rc2Dvector>::const_iterator pTailI = _vert.end() - 1;

  double length = 0.0;

  while (pHeadI != _vert.end()) {
    rc2Dvector delta = *pHeadI - *pTailI;

    length += delta.len();

    pTailI = pHeadI;
    pHeadI++;
  }

  return length;
}


bool rcPolygon::centerOf (rc2Dvector& cof) const
{
  if (_vert.size() < 1)
    return false;

  if (_comValid)
    {
      cof = _com;
      return true;
    }

  double cx (0.0), cy (0.0);
  double a = 1.0 / _vert.size();

  for (uint32 i = 0; i < _vert.size(); i++)
    {
      cx += _vert[i].x();
      cy += _vert[i].y();
    }

  cof.x(a * cx);
  cof.y(a * cy);
  const_cast<rcPolygon *>(this)->_com = cof;
  const_cast<rcPolygon *>(this)->_comValid = true;
  return true;
}

bool rcPolygon::centerOf (rc2Fvector& cof) const
{
  if (_vert.size() < 1)
    return false;

  if (_comValid)
    {
      cof.x((float) _com.x());
      cof.y((float) _com.y());      
      return true;
    }

  float cx (0.0f), cy (0.0f);
  float a = 1.0f / _vert.size();

  for (uint32 i = 0; i < _vert.size(); i++)
    {
      cx += _vert[i].x();
      cy += _vert[i].y();
    }

  cof.x(a * cx);
  cof.y(a * cy);
  const_cast<rcPolygon *>(this)->_com = rc2Dvector ((double) cof.x(), (double) cof.y());
  const_cast<rcPolygon *>(this)->_comValid = true;
  return true;
}

double rcPolygon::circularity () const
{
  if (_vert.size() < 2) return 0.0;
  double p (perimeter());
  p = 1.0 / rmSquare (p);
  return 2.0 * rk2PI *  const_cast<rcPolygon *>(this)->area() * p;
}

double rcPolygon::ellipseRatio () const
{
  if (_vert.size() < 2) return 0.0;

  double ar =  const_cast<rcPolygon *>(this)->area() / rkPI;
  double alpha = (sqrt (ar) + perimeter() / rkPI) / 3.0;
  double a = alpha + sqrt (rmSquare(alpha) - ar);
  return rmSquare (a) / ar;
}


void rcPolygon::convexIntersection(const rcPolygon& poly, rcPolygon& polyInt) const
{
  rmAssert(isValid());
  rmAssert(poly.isValid());

  polyInt._vert.clear();
  polyInt._isConvex = polyInt._knownConvex = true;
  polyInt._isValid = polyInt._knownValid = true;

  if ((poly.vertexCnt() < 2) || vertexCnt() < 2) {
    if ((poly.vertexCnt() == 0) || vertexCnt() == 0)
      return;

    if (vertexCnt() == 1) {
      if (poly.contains(_vert[0]))
	polyInt.pushVertex(_vert[0]);

      return;
    }

    /* vertexCnt() >= 2 ==> poly.vertexCnt() == 1
     */
    rmAssert(poly.vertexCnt() == 1);

    if (contains(poly._vert[0]))
      polyInt.pushVertex(poly._vert[0]);

    return;
  }

  rcPolygon convexHullP, convexHullQ;

  if (!isConvex())
    convexHull(convexHullP);

  if (!poly.isConvex())
    poly.convexHull(convexHullQ);

  const rcPolygon& polyP = isConvex() ? *this : convexHullP;
  const rcPolygon& polyQ = poly.isConvex() ? poly : convexHullQ;

  const rc2Dvector origin(0, 0);

  const int32 pSize = polyP.vertexCnt();
  const int32 qSize = polyQ.vertexCnt();

  deque<rc2Dvector>::const_iterator pHeadI = polyP.vert().begin(),
    qHeadI = polyQ.vert().begin();

  deque<rc2Dvector>::const_iterator pTailI = polyP.vert().end() - 1,
    qTailI = polyQ.vert().end() - 1;

  int32 pAdvances = 0, qAdvances = 0;
  eInState inFlag = eInUnknown;
  bool firstPoint = true;

  rc2Dvector nv1, nv2;

  do {
    rc2Dvector pHead = *pHeadI, pTail = *pTailI;
    rc2Dvector qHead = *qHeadI, qTail = *qTailI;

    rc2Dvector pDelta = pHead - pTail;
    rc2Dvector qDelta = qHead - qTail;

    int cross = areaSign(origin, pDelta, qDelta);
    int qHP = areaSign(pTail, pHead, qHead);
    int pHQ = areaSign(qTail, qHead, pHead);

    eIntersection code = segmentInter(pTail, pHead, qTail, qHead, nv1, nv2);

    if (DBGPRT) cout << "cross " << cross << " q in H(P) " << qHP
		<< " p in H(Q) " << pHQ << endl << endl;

    if ((code == eProperIntersection) || (code == eVertexIntersection)) {
      if ((inFlag == eInUnknown) && firstPoint) {
	pAdvances = qAdvances = 0;
	firstPoint = false;
      }

      if (DBGPRT) cout << "Case 0 Outputting: " << nv1 << endl;

      polyInt.pushUniqueVertex(nv1);
      if (pHQ > 0)
	inFlag = ePIn;
      else if (qHP > 0)
	inFlag = eQIn;
    }

    /* Advance rules */

    /* Special case: A & B overlap and oppositely oriented.
     */
    if ((code == eSharedCollinear) && (pDelta.dot(qDelta) < 0)) {
      if (DBGPRT) printf("special case: A & B overlap and oppositely oriented.\n");
      polyInt._vert.clear();
      polyInt.pushUniqueVertex(nv1);
      polyInt.pushUniqueVertex(nv2);
      polyInt._isConvex = polyInt._knownConvex = true;
      polyInt._isValid = polyInt._knownValid = true;
      return;
    }
    
    /* Special case: A & B parallel and separated.
     */
    if ((cross == 0) && (pHQ < 0) && (qHP < 0)) {
      if (DBGPRT) printf("special case: A & B parallel and separated.\n");
      if (polyInt.vertexCnt() != 0)
	rmExceptionMacro(<< "Unresolved Geometry");

      return;
    }

    if ((cross == 0) && (pHQ == 0) && (qHP == 0)) {
      /* Advance but do not output point.
       */
      if (inFlag == ePIn) {
	qTailI = qHeadI;
	if (++qHeadI == polyQ.vert().end())
	  qHeadI = polyQ.vert().begin();
	qAdvances++;
      }
      else {
	pTailI = pHeadI;
	if (++pHeadI == polyP.vert().end())
	  pHeadI = polyP.vert().begin();
	pAdvances++;
      }
    } // else generic cases
    else if (cross >= 0) {
      if (qHP > 0) {
	if (inFlag == ePIn) {
	  if (DBGPRT) cout << "Case 1 Outputting: " << pHead << endl;
	  polyInt.pushUniqueVertex(pHead);
	}
	pTailI = pHeadI;
	if (++pHeadI == polyP.vert().end())
	  pHeadI = polyP.vert().begin();
	pAdvances++;
      }
      else {
	if (inFlag == eQIn) {
	  if (DBGPRT) cout << "Case 2 Outputting: " << qHead << endl;
	  polyInt.pushUniqueVertex(qHead);
	}
	qTailI = qHeadI;
	if (++qHeadI == polyQ.vert().end())
	  qHeadI = polyQ.vert().begin();
	qAdvances++;
      }
    }
    else { // cross < 0
      if (pHQ > 0) {
	if (inFlag == eQIn) {
	  if (DBGPRT) cout << "Case 3 Outputting: " << qHead << endl;
	  polyInt.pushUniqueVertex(qHead);
	}
	qTailI = qHeadI;
	if (++qHeadI == polyQ.vert().end())
	  qHeadI = polyQ.vert().begin();
	qAdvances++;
      }
      else {
	if (inFlag == ePIn) {
	  if (DBGPRT) cout << "Case 4 Outputting: " << pHead << endl;
	  polyInt.pushUniqueVertex(pHead);
	}
	pTailI = pHeadI;
	if (++pHeadI == polyP.vert().end())
	  pHeadI = polyP.vert().begin();
	pAdvances++;
      }
    }      
	
  } while(((pAdvances < pSize) || (qAdvances < qSize)) &&
	  (pAdvances < pSize*2) && (qAdvances < qSize*2));
  
  if ((polyInt._vert.size() >= 2) &&
      (polyInt._vert.front() == polyInt._vert.back()))
    polyInt._vert.pop_back();

  if (DBGPRT) {
    cout << polyP << endl;
    cout << polyQ << endl;
    cout << polyInt << endl;
  }

  /* If inFlag has never been set, either the polygons don't intersect
   * or are nested.
   */
  if (inFlag == eInUnknown) {
    if (polyP.contains(*(polyQ.vert().begin())))
      polyInt = polyQ;
    else if (polyQ.contains(*(polyP.vert().begin())))
      polyInt = polyP;
  }
  else {
    polyInt._isConvex = polyInt._knownConvex = true;
    polyInt._isValid = polyInt._knownValid = true;
  }
}

template <class T>
class uniqueOrder
{
public:
  bool operator() (const rc2dVector<T>& p1, const rc2dVector<T>& p2)
  {
    if (p1.y() < p2.y())
      return true;

    return p1.x() < p2.x();
  }
};


void rcPolygon::convexUnion(const rcPolygon& polygon, rcPolygon& polyUnion) const
{
  rmAssert(isValid());
  rmAssert(polygon.isValid());

  /* The following algorithm is not from the O'Rourke book, so don't
   * blame him!
   *
   * Step 1: Get the convex hull of each polygon to guarantee that the
   *         minimum number of points are being looked at.
   *
   * Step 2: Create a polygon that is the union of unique vertices in
   *         the two convex hulls.
   *
   * Step 3: Convert the new polygon into a convex hull.
   *
   */
  rcPolygon convexHullP, convexHullQ;

  if (!isConvex())
    convexHull(convexHullP);

  if (!polygon.isConvex())
    polygon.convexHull(convexHullQ);

  const rcPolygon& polyP = isConvex() ? *this : convexHullP;
  const rcPolygon& polyQ = polygon.isConvex() ? polygon : convexHullQ;

  set<rc2Dvector, uniqueOrder<double> > uvert;

  uvert.insert(polyP._vert.begin(), polyP._vert.end());
  uvert.insert(polyQ._vert.begin(), polyQ._vert.end());

  polyUnion._vert.resize(uvert.size());
  polyUnion._isConvex = false;

  set<rc2Dvector>::iterator cur = uvert.begin(), end = uvert.end();

  uint32 i = 0;
  while (cur != end)
    polyUnion._vert[i++] = *cur++;

  polyUnion._knownValid = polyUnion._isValid = true;

  polyUnion.convexHullConvert();
}

void rcPolygon::orthogonalEnclosingRect(rcDRect& r) const
{
  rmAssert(isValid());

  if (_vert.empty())
    {
      r = rcDRect ();
      return;
    }

  double minX = _vert[0].x(), minY = _vert[0].y();
  double maxX = minX, maxY = minY;

  for (uint32 i = 1; i < _vert.size(); i++) {
    if (_vert[i].x() < minX)
      minX = _vert[i].x();
    else if (_vert[i].x() > maxX)
      maxX = _vert[i].x();

    if (_vert[i].y() < minY)
      minY = _vert[i].y();
    else if (_vert[i].y() > maxY)
      maxY = _vert[i].y();
  }
  
  r = rcDRect(minX, minY, (maxX-minX), (maxY-minY));
}

void rcPolygon::orthogonalEnclosingRect(rcFRect& fr) const
{
  rcDRect d;

  orthogonalEnclosingRect(d);

  fr = rcFRect((float) d.ul().x(), 
		 (float) d.ul().y(), 
		 (float) d.width(), (float) d.height());
}


rcIRect rcPolygon::orthogonalEnclosingRect() const
{
  rcDRect d;

  orthogonalEnclosingRect(d);

  return rcIRect((int32) d.ul().x(), 
		 (int32) d.ul().y(), 
		 (int32) d.width(), (int32) d.height());
}

inline double rcPolygon::triArea2(const rc2Dvector& a, const rc2Dvector& b,
				  const rc2Dvector& c) const
{
  return (a.x()*b.y() - a.y()*b.x() +
	  a.y()*c.x() - a.x()*c.y() +
	  b.x()*c.y() - b.y()*c.x());
}
    
double rcPolygon::polyArea2() const
{
  int32 count = (int32)_vert.size() - 1;
  double sum = 0.0;
  
  for (int32 i = 1; i < count; i++) {
    sum += triArea2(_vert[0], _vert[i], _vert[i+1]);
  }

  return (sum < 0 ? -sum : sum);
}

inline bool rcPolygon::leftTurn(const rc2Dvector& a, const rc2Dvector& b,
				const rc2Dvector& c) const
{
  return triArea2(a, b, c) > 0.0;
}

inline bool rcPolygon::leftTurnOrOn(const rc2Dvector& a, const rc2Dvector& b,
				    const rc2Dvector& c) const
{
  return triArea2(a, b, c) >= 0.0;
}

inline bool rcPolygon::collinear(const rc2Dvector& a, const rc2Dvector& b,
				 const rc2Dvector& c) const
{
  return triArea2(a, b, c) == 0.0;
}

bool rcPolygon::intersection(const rc2Dvector& a0, const rc2Dvector& a1,
			     const rc2Dvector& b0, const rc2Dvector& b1,
			     rc2Dvector* inter) const
{
  double denom =
    a0.x() * (b1.y() - b0.y()) +
    a1.x() * (b0.y() - b1.y()) +
    b0.x() * (a0.y() - a1.y()) +
    b1.x() * (a1.y() - a0.y());

  /* If denom is zero, the line segments are parallel. In this case,
   * return false even though the segments might overlap.
   */
  if (denom == 0.0)
    return false;

  double s =
    (a0.x() * (b1.y() - b0.y()) +
     b0.x() * (a0.y() - b1.y()) +
     b1.x() * (b0.y() - a0.y())) / denom;

  double t =
    -(a0.x() * (b0.y() - a1.y()) +
      a1.x() * (a0.y() - b0.y()) +
      b0.x() * (a1.y() - a0.y())) / denom;

  if (inter)
    *inter = rc2Dvector(a0.x() + s*(a1.x() - a0.x()),
			a0.y() + s*(a1.y() - a0.y()));

  return (0.0 <= s) && (s <= 1.0) && (0.0 <= t) && (t <= 1.0);
}

template <class T>
class thetaOrder
{
public:
  bool operator() (const rc2dVector<T>& p1, const rc2dVector<T>& p2)
  {
    T area = p1.cross(p2);

    if (area > 0)
      return true;
    else if (area < 0)
      return false;
      
    return p1.len() < p2.len();
  }
};

void rcPolygon::createConvexHull(rcPolygon& poly, bool mastBeValid) const
{
  if (mastBeValid) 
    rmAssert(isValid());

  if (_vert.size() < 3) {
    if (&poly != this)
      poly = *this;

    return;
  }
    
  /* Find the rightmost lowest point and store an index to it in
   * oIndex.
   */
  uint32 oIndex = 0;
  for (uint32 i = _vert.size() - 1; i; i--) {
    if ((_vert[i].y() < _vert[oIndex].y()) ||
	((_vert[i].y() == _vert[oIndex].y()) && 
	 (_vert[i].x() > _vert[oIndex].x())))
      oIndex = i;
  }
   
  /* Sort the vertices in ascending theta order around the origin. In
   * case of a tie, the vertice closer to the origin will be deemed to
   * have a smaller value.
   */
  map<rc2Dvector, rc2Dvector, thetaOrder<double> > tOrder;

  const rc2Dvector origin = _vert[oIndex];

  for (uint32 i = 0; i < _vert.size(); i++) {
    const rc2Dvector normLoc = _vert[i] - origin;
    tOrder.insert(pair<rc2Dvector, rc2Dvector>(normLoc, _vert[i]));
  }

  uint32 hullTop = 1;
  deque<rc2Dvector> hullVertices;
  map<rc2Dvector, rc2Dvector>::iterator curr(tOrder.begin());
  map<rc2Dvector, rc2Dvector>::iterator last(tOrder.end());
  const map<rc2Dvector, rc2Dvector>::iterator end(tOrder.end());
  last--;
  hullVertices.push_back(last->second);
  hullVertices.push_back(curr->second);
  curr++;
  
  while (curr != end) {
    if (leftTurn(hullVertices[hullTop-1], hullVertices[hullTop],
		 curr->second)) {
      hullTop++;
      hullVertices.push_back(curr->second);
      curr++;
    }
    else {
      hullTop--;
      hullVertices.pop_back();
    }
    if (0) { 
      for (uint32 xx = 0; xx <= hullTop; xx++)
	cout << " " << hullVertices[xx];
      cout << endl;
    }
  }
  
  hullTop--;
  hullVertices.pop_back(); // Last elmt pushed twice, so pop it off

  poly._vert.clear();
  for (uint32 i = 0; i <= hullTop; i++)
    poly._vert.push_back(hullVertices[i]);

  poly._knownValid = poly._knownConvex = true;
  poly._isValid = poly._isConvex = true;
}

int32 rcPolygon::areaSign(const rc2Dvector& a, const rc2Dvector& b,
			    const rc2Dvector& c) const
{
  double area2 =
    (b.x() - a.x())*(c.y() - a.y()) - (c.x() - a.x())*(b.y() - a.y());

  if (area2 >  0.0001)
    return  1;
  else if (area2 < -0.0001)
    return -1;
  else
    return  0;
}

/* Returns TRUE iff point c lies on the closed segement ab.  Assumes
 * it is already known that abc are collinear.
 */
bool rcPolygon::between(const rc2Dvector& a, const rc2Dvector& b,
			const rc2Dvector& c) const
{
  /* If ab not vertical, check betweenness on x; else on y. */
  if ( a.x() != b.x() )
    return ((a.x() <= c.x()) && (c.x() <= b.x())) ||
      ((a.x() >= c.x()) && (c.x() >= b.x()));
  else
    return ((a.y() <= c.y()) && (c.y() <= b.y())) ||
      ((a.y() >= c.y()) && (c.y() >= b.y()));
}

rcPolygon::eIntersection
rcPolygon::parallelInter(const rc2Dvector& a0, const rc2Dvector& a1,
			 const rc2Dvector& b0, const rc2Dvector& b1,
			 rc2Dvector& inter1, rc2Dvector& inter2) const
{
  eIntersection retVal = eNoIntersection;

  if (areaSign(a0, a1, b0) != 0) 
    retVal = eNoIntersection;
  else if (between(a0, a1, b0) && between(a0, a1, b1)) {
    inter1 = b0;
    inter2 = b1;
    retVal = eSharedCollinear;
  }
  else if (between(b0, b1, a0) && between(b0, b1, a1)) {
    inter1 = a0;
    inter2 = a1;
    retVal = eSharedCollinear;
  }
  else if (between(a0, a1, b0) && between(b0, b1, a1)) {
    inter1 = b0;
    inter2 = a1;
    retVal = eSharedCollinear;
  }
  else if (between(a0, a1, b0) && between(b0, b1, a0)) {
    inter1 = b0;
    inter2 = a0;
    retVal = eSharedCollinear;
  }
  else if (between(a0, a1, b1) && between(b0, b1, a1)) {
    inter1 = b1;
    inter2 = a1;
    retVal = eSharedCollinear;
  }
  else if (between(a0, a1, b1) && between(b0, b1, a0)) {
    inter1 = b1;
    inter2 = a0;
    retVal = eSharedCollinear;
  }
  
  if (DBGPRT) {
    if (retVal == eNoIntersection) cout << " No intersection" << endl;
    if (retVal == eSharedCollinear) {
      cout << " INTER1: " << inter1 << " INTER2: " << inter2;
      cout << " Shared collinear vertices" << endl;
    }
  }

  return retVal;
}

rcPolygon::eIntersection
rcPolygon::segmentInter(const rc2Dvector& a0, const rc2Dvector& a1,
			const rc2Dvector& b0, const rc2Dvector& b1,
			rc2Dvector& inter1, rc2Dvector& inter2) const
{
  eIntersection retVal = eInterUnknown;

  if (DBGPRT) cout << "P (H,T): (" << a1 << "," << a0
		   << ") Q (H,T): (" << b1 << "," << b0 << ") ==>";

  double denom =
    a0.x() * (b1.y() - b0.y()) +
    a1.x() * (b0.y() - b1.y()) +
    b0.x() * (a0.y() - a1.y()) +
    b1.x() * (a1.y() - a0.y());

  /* If denom is zero, the line segments are parallel. call special
   * case code to handle it.
   */
  if (denom == 0.0)
    return parallelInter(a0, a1, b0, b1, inter1, inter2);

  double s =
    (a0.x() * (b1.y() - b0.y()) +
     b0.x() * (a0.y() - b1.y()) +
     b1.x() * (b0.y() - a0.y()));

  if ((s == 0) || (s == denom))
    retVal = eVertexIntersection;

  s = s/denom;

  double t =
    -(a0.x() * (b0.y() - a1.y()) +
      a1.x() * (a0.y() - b0.y()) +
      b0.x() * (a1.y() - a0.y()));

  if ((t == 0) || (t == denom))
    retVal = eVertexIntersection;

  t = t/denom;

  inter1 = rc2Dvector(a0.x() + s*(a1.x() - a0.x()),
		      a0.y() + s*(a1.y() - a0.y()));

  if ((0.0 < s) && (s < 1.0) && (0.0 < t) && (t < 1.0))
    retVal = eProperIntersection;
  else if ((0.0 > s) || (s > 1.0) || (0.0 > t) || (t > 1.0))
    retVal = eNoIntersection;

  rmAssert(retVal != eInterUnknown);

  if (DBGPRT) {
    if (retVal != eNoIntersection)
      cout << " INTER1: " << inter1;
  
    if (retVal == eNoIntersection) cout << " No intersection" << endl;
    if (retVal == eProperIntersection) cout << " Proper intersection" << endl;
    if (retVal == eVertexIntersection) cout << " Vertex intersection" << endl;
    if (retVal == eSharedCollinear) cout << " Shared collinear vertices" << endl;
  }

  return retVal;
}

bool rcPolygon::convexComparedToConvex(const rcPolygon& polygon,
				       const bool contains) const
{
  rmAssert(isConvex());
  rmAssert(polygon.isConvex());

  rcPolygon polyInt;

  const rcPolygon& polyP = *this;
  const rcPolygon& polyQ = polygon;

  const rc2Dvector origin(0, 0);

  const int32 pSize = polyP.vertexCnt();
  const int32 qSize = polyQ.vertexCnt();

  deque<rc2Dvector>::const_iterator pHeadI = polyP.vert().begin(),
    qHeadI = polyQ.vert().begin();

  deque<rc2Dvector>::const_iterator pTailI = polyP.vert().end() - 1,
    qTailI = polyQ.vert().end() - 1;

  int32 pAdvances = 0, qAdvances = 0;
  eInState inFlag = eInUnknown;
  bool firstPoint = true;

  rc2Dvector nv1, nv2;

  do {
    rc2Dvector pHead = *pHeadI, pTail = *pTailI;
    rc2Dvector qHead = *qHeadI, qTail = *qTailI;

    rc2Dvector pDelta = pHead - pTail;
    rc2Dvector qDelta = qHead - qTail;

    int cross = areaSign(origin, pDelta, qDelta);
    int qHP = areaSign(pTail, pHead, qHead);
    int pHQ = areaSign(qTail, qHead, pHead);

    eIntersection code = segmentInter(pTail, pHead, qTail, qHead, nv1, nv2);

    if (!contains && (code != eNoIntersection))
      return true;

    if (DBGPRT3) cout << "cross " << cross << " q in H(P) " << qHP
		<< " p in H(Q) " << pHQ << endl << endl;

    if ((code == eProperIntersection) || (code == eVertexIntersection)) {
      if ((inFlag == eInUnknown) && firstPoint) {
	pAdvances = qAdvances = 0;
	firstPoint = false;
      }

      if (DBGPRT3) cout << "Case 0 Outputting: " << nv1 << endl;

      polyInt.pushUniqueVertex(nv1);
      if (pHQ > 0)
	inFlag = ePIn;
      else if (qHP > 0)
	inFlag = eQIn;
    }

    /* Advance rules */

    /* Special case: A & B overlap and oppositely oriented.
     */
    if ((code == eSharedCollinear) && (pDelta.dot(qDelta) < 0)) {
      if (DBGPRT3) printf("special case: A & B overlap and oppositely oriented.\n");
      if ((qSize == 2) &&
	  between(pHead, pTail, qHead) && between(pHead, pTail, qTail))
	return true;
      else
	return false;
    }
    
    /* Special case: A & B parallel and separated.
     */
    if ((cross == 0) && (pHQ < 0) && (qHP < 0)) {
      if (DBGPRT3) printf("special case: A & B parallel and separated.\n");
      return false;
    }

    if ((cross == 0) && (pHQ == 0) && (qHP == 0)) {
      /* Advance but do not output point.
       */
      if (inFlag == ePIn) {
	qTailI = qHeadI;
	if (++qHeadI == polyQ.vert().end())
	  qHeadI = polyQ.vert().begin();
	qAdvances++;
      }
      else {
	pTailI = pHeadI;
	if (++pHeadI == polyP.vert().end())
	  pHeadI = polyP.vert().begin();
	pAdvances++;
      }
    } // else generic cases
    else if (cross >= 0) {
      if (qHP > 0) {
	if (inFlag == ePIn) {
	  if (DBGPRT3) cout << "Case 1 Outputting: " << pHead << endl;
	  polyInt.pushUniqueVertex(pHead);
	}
	pTailI = pHeadI;
	if (++pHeadI == polyP.vert().end())
	  pHeadI = polyP.vert().begin();
	pAdvances++;
      }
      else {
	if (inFlag == eQIn) {
	  if (DBGPRT3) cout << "Case 2 Outputting: " << qHead << endl;
	  polyInt.pushUniqueVertex(qHead);
	}
	qTailI = qHeadI;
	if (++qHeadI == polyQ.vert().end())
	  qHeadI = polyQ.vert().begin();
	qAdvances++;
      }
    }
    else { // cross < 0
      if (pHQ > 0) {
	if (inFlag == eQIn) {
	  if (DBGPRT3) cout << "Case 3 Outputting: " << qHead << endl;
	  polyInt.pushUniqueVertex(qHead);
	}
	qTailI = qHeadI;
	if (++qHeadI == polyQ.vert().end())
	  qHeadI = polyQ.vert().begin();
	qAdvances++;
      }
      else {
	if (inFlag == ePIn) {
	  if (DBGPRT3) cout << "Case 4 Outputting: " << pHead << endl;
	  polyInt.pushUniqueVertex(pHead);
	}
	pTailI = pHeadI;
	if (++pHeadI == polyP.vert().end())
	  pHeadI = polyP.vert().begin();
	pAdvances++;
      }
    }      
	
  } while(((pAdvances < pSize) || (qAdvances < qSize)) &&
	  (pAdvances < pSize*2) && (qAdvances < qSize*2));
  
  if ((polyInt._vert.size() >= 2) &&
      (polyInt._vert.front() == polyInt._vert.back()))
    polyInt._vert.pop_back();

  if (DBGPRT3) {
    cout << polyP << endl;
    cout << polyQ << endl;
    cout << polyInt << endl;
  }

  /* If inFlag has never been set, either the polygons don't intersect
   * or are nested.
   */
  if (inFlag == eInUnknown) {
    if (contains)
      return polyP.contains(polyQ.vert().front());
    else
      return (polyP.contains(polyQ.vert().front()) ||
	      polyQ.contains(polyP.vert().front()));
  }

  rmAssert(contains);

  return polyQ == polyInt;
}

rc2Dvector rcPolygon::crossing(const rc2Dvector& b0, const rc2Dvector& b1,
			       const rc2Dvector& a) const
{
  rc2Dvector d = b1 - b0;

  if (d.x() == 0 || d.y() == 0) {
    if (d.y() == 0) {
      rmAssert(d.x() != 0);
      return rc2Dvector(a.x(), b1.y());
    }

    return rc2Dvector(b1.x(), a.y());
  }

  const double mBase = d.y()/d.x(), mOrth = -d.x()/d.y();

  const double intBase = b1.y() - mBase*b1.x();
  const double intOrth = a.y() - mOrth*a.x();

  const double crossingX = (intOrth - intBase)/(mBase - mOrth);
  const double crossingY = mOrth*crossingX + intOrth;

  return rc2Dvector(crossingX, crossingY);
}

int32 rcPolygon::findPeak(const int32 startIndex,
			    const rc2Dvector& b0, const rc2Dvector& b1) const
{
  int32 curIndex = startIndex, maxIndex = startIndex;
  int32 vertEnd = vertexCnt() - 1;
  
  rc2Dvector cross = crossing(b0, b1, _vert[curIndex]);
  rc2Dvector delta = _vert[curIndex] - cross;

  bool lt = leftTurn(b0, b1, _vert[curIndex]);

  double mag = lt ? -1 : 1;
  double maxValue = delta.len() * mag;

  if (DBGPRT2) cout << "      FINDPEAK: SI " <<  startIndex << " B T " << b0
		    << " H " << b1 << " VERT " << _vert[curIndex] << " CROSS "
		    << cross << " LT " << lt << " VALUE " << maxValue << endl;

  for (;;) {
    if (curIndex == vertEnd)
      curIndex = 0;
    else 
      curIndex++;
    
    cross = crossing(b0, b1, _vert[curIndex]);
    delta = _vert[curIndex] - cross;

    lt = leftTurn(b0, b1,  _vert[curIndex]);

    mag = lt ? -1 : 1;
    double value = delta.len() * mag;

    if (DBGPRT2) cout << "                CI " <<  curIndex << " VERT "
		      << _vert[curIndex] << " CROSS " << cross
		      << " LT " << lt << " MI " << maxIndex
		      << " V " << value << " MV " << maxValue << endl;

    if (value <= maxValue) {
      if (DBGPRT2) cout << "                DONE MI " << maxIndex << endl;
      return maxIndex;
    }

    maxIndex = curIndex;
    maxValue = value;
  }

  rmAssert(0);
  return 0;
}

void rcPolygon::genMinRectBounds(const rc2Dvector& topPt,
				 const rc2Dvector& btmPt,
				 const rc2Dvector& leftPt,
				 const rc2Dvector& rightPt,
				 double mx, double my,
				 rc2Dvector& llPt,
				 rc2Dvector& dim) const
{
  if (DBGPRT2) cout << "gmrb T " << topPt << " B " << btmPt
		    << " L " << leftPt << " R " << rightPt
		    << " mx " << mx << " my " << my << endl;

  /* First, handle case of rectangle orthogonal to image plane.
   */
  if ((mx == 0) || (my == 0)) {
    if (my == 0) {
      rmAssert(mx != 0);
      if (rightPt.x() > leftPt.x())
	dim = rc2Dvector(rightPt.x() - leftPt.x(), topPt.y() - btmPt.y()); // x+
      else
	dim = rc2Dvector(leftPt.x() - rightPt.x(), btmPt.y() - topPt.y()); // x-

      llPt = rc2Dvector(leftPt.x(), btmPt.y());
    }
    else { // mx == 0
      if (rightPt.y() > leftPt.y())
	dim = rc2Dvector(rightPt.y() - leftPt.y(), btmPt.x() - topPt.x()); // y+
      else
	dim = rc2Dvector(leftPt.y() - rightPt.y(), topPt.x() - btmPt.x()); // y-

      llPt = rc2Dvector(btmPt.x(), leftPt.y());
    }

    return;
  }

  /* Handle all other cases by using the following algorithm:
   *
   * 1) Find the slope and y-intercept formula for each side.
   *
   * 2) Set the equations equal for 3 of the pairs of intersecting
   *    sides.
   *
   * The result of this will be the extreme values of the enclosing
   * rectangle.
   */

  const double mHor = my/mx;
  const double mVert = -mx/my;

  double intT = topPt.y() - mHor*topPt.x();
  double intB = btmPt.y() - mHor*btmPt.x();
  double intL = leftPt.y() - mVert*leftPt.x();
  double intR = rightPt.y() - mVert*rightPt.x();

  const double upperLeftX = (intT - intL)/(mVert - mHor);
  const double upperLeftY = mHor*upperLeftX + intT;

  const double lowerLeftX = (intB - intL)/(mVert - mHor);
  const double lowerLeftY = mHor*lowerLeftX + intB;

  const double lowerRightX = (intB - intR)/(mVert - mHor);
  const double lowerRightY = mHor*lowerRightX + intB;

  /* Set up return values.
   */
  llPt = rc2Dvector(lowerLeftX, lowerLeftY);

  const rc2Dvector ulPt = rc2Dvector(upperLeftX, upperLeftY);
  const rc2Dvector hgtVec = ulPt - llPt;

  const rc2Dvector lrPt = rc2Dvector(lowerRightX, lowerRightY);
  const rc2Dvector widthVec = lrPt - llPt;

  dim = rc2Dvector(widthVec.len(), hgtVec.len());
}

rcAffineRectangle rcPolygon::genMinEnclosingRect(bool area) const
{
  rmAssert(isValid());

  /* Another algorithm not from the O'Rourke book. Use the "rotating
   * caliper" algorithm to determine the polygon's minimum enclosing
   * rectangle.
   */
  rcPolygon hull;

  if (!isConvex())
    convexHull(hull);

  const rcPolygon& hullP = isConvex() ? *this : hull;
  
  if (hullP._vert.size() <= 2) {
    int32 width = 1;
    rcRadian theta(0); 
    if (hullP._vert.size() == 2) {
      rc2Dvector span = _vert[1] - _vert[0];
      width = (int32)span.len();
      if (width == 0)
	width = 1;
      theta = span.angle().norm();
    }
    
    rc2Dvector origin(-1000000, -1000000);
    
    if (!hullP._vert.empty())
      origin = _vert[0];

    return rcAffineRectangle(origin, theta, rcDPair(width, 1),
			     rcIPair(width, 1), rcDPair(0, 0));
  }

  /* Step 1 - Choose an inital pair of consecutive points to serve
   *          as the initial base. Using this base, determine an
   *          initial set of points that describe the left, right
   *          and upper limits of the polygon.
   *
   * Step 2 - Replace one of the base points with the next
   *          consecutive point, effectively rotqating the polygon
   *          one turn. Using the existing left, right and upper
   *          limit points, calculate new ones. Check to see of
   *          these describe a new minimum enclosing rectangle.
   *
   * Step 3 - Repeat step 2 until the original base has been rotated
   *          >= pi/2 degrees.
   *
   * Note: This code assumes the points in a hull are stored
   *       counterclockwise.
   */
  int32 prevLoc = 0, curLoc = 1, nextLoc = 2;
  int32 vertEnd = vertexCnt() - 1;
  const rc2Dvector base = _vert[curLoc] - _vert[prevLoc];
  rcRadian minAreaTheta(base.angle().norm()), minPerimTheta(minAreaTheta);

  rc2Dvector orthCross = crossing(_vert[prevLoc], _vert[curLoc], _vert[nextLoc]);

  if (DBGPRT2) cout << "I: curLoc " << curLoc
		    << " angle " << minAreaTheta.Double()*180/rkPI
		    << " base T " <<  _vert[prevLoc] 
		    << " H " <<  _vert[curLoc] 
		    << " orth T " <<  orthCross 
		    << " H " <<  _vert[nextLoc] << endl;

  int32 leftPt, rightPt, topPt;

  /* Calculate extreme points and generate initial minimum area and
   * lower-left corner point.
   */
  rightPt = findPeak(curLoc, orthCross, _vert[nextLoc]);
  topPt = findPeak(curLoc, _vert[curLoc], _vert[prevLoc]);
  leftPt = findPeak(topPt, _vert[nextLoc], orthCross);

  if (DBGPRT2) cout << "   leftPt " << leftPt << " rightPt " << rightPt
		    << " topPt " << topPt << endl;

  rc2Dvector minPerimLL, minPerimDim, minAreaLL, minAreaDim;
  genMinRectBounds(_vert[topPt], _vert[curLoc],
		   _vert[leftPt], _vert[rightPt],
		   base.x(), base.y(), minAreaLL, minAreaDim);
  minPerimLL = minAreaLL;
  minPerimDim = minAreaDim;

  if (DBGPRT2) cout << "   MIN AREA: lowerleft " << minAreaLL
		    << " dim " << minAreaDim << " theta "
		    << minAreaTheta.Double()*180/rkPI << endl
		    << "   MIN PERIM: lowerleft " << minPerimLL
		    << " dim " << minPerimDim << " theta "
		    << minPerimTheta.Double()*180/rkPI << endl << endl;

  do {
    prevLoc = curLoc;
    curLoc = nextLoc;
    if (nextLoc == vertEnd)
      nextLoc = 0;
    else 
      nextLoc++;

    const rc2Dvector curBase = _vert[curLoc] - _vert[prevLoc];
    const rcRadian curTheta = curBase.angle().norm();

    orthCross = crossing(_vert[prevLoc], _vert[curLoc], _vert[nextLoc]);
    
    if (DBGPRT2) cout << "M: curLoc " << curLoc
		      << " angle " << curTheta.Double()*180/rkPI
		      << " base T " <<  _vert[prevLoc] 
		      << " H " <<  _vert[curLoc] 
		      << " orth T " <<  _vert[nextLoc] 
		      << " H " <<  orthCross << endl;

    rightPt = findPeak(rightPt, orthCross, _vert[nextLoc]);
    topPt = findPeak(topPt, _vert[curLoc], _vert[prevLoc]);
    leftPt = findPeak(leftPt, _vert[nextLoc], orthCross);
      
    if (DBGPRT2) cout << "   leftPt " << leftPt << " rightPt " << rightPt
		      << " topPt " << topPt << endl;

    rc2Dvector curLL, curDim;
    genMinRectBounds(_vert[topPt], _vert[curLoc],
		     _vert[leftPt], _vert[rightPt],
		     curBase.x(), curBase.y(), curLL, curDim);

    if (curDim.x() + curDim.y() < minPerimDim.x() + minPerimDim.y()) {
      minPerimLL = curLL;
      minPerimDim = curDim;
      minPerimTheta = curTheta;
    }
    else if ((curDim.x() + curDim.y() == minPerimDim.x() + minPerimDim.y()) &&
	     (curDim.x()*curDim.y() < minPerimDim.x()*minPerimDim.y())) {
      minPerimLL = curLL;
      minPerimDim = curDim;
      minPerimTheta = curTheta;
    }

    if (curDim.x()*curDim.y() < minAreaDim.x()*minAreaDim.y()) {
      minAreaLL = curLL;
      minAreaDim = curDim;
      minAreaTheta = curTheta;
    }
    else if ((curDim.x()*curDim.y() == minAreaDim.x()*minAreaDim.y()) &&
	     (curDim.x() + curDim.y() < minAreaDim.x() + minAreaDim.y())) {
      minAreaLL = curLL;
      minAreaDim = curDim;
      minAreaTheta = curTheta;
    }

    if (DBGPRT2) cout << "  ?lowerleft " << curLL << " ?dim " << curDim
		      << " ?theta " << curTheta.Double()*180/rkPI << endl
		      << "   MIN AREA: lowerleft " << minAreaLL
		      << " dim " << minAreaDim << " theta "
		      << minAreaTheta.Double()*180/rkPI << endl
		      << "   MIN PERIM: lowerleft " << minPerimLL
		      << " dim " << minPerimDim << " theta "
		      << minPerimTheta.Double()*180/rkPI << endl << endl;
  } while (curLoc != 1);

  if (area) {
    rcDPair size;
    size.x() = minAreaDim.x() + 1;
    size.y() = minAreaDim.y() + 1;
    static const int32 dum (0);
    return rcAffineRectangle(minAreaLL, minAreaTheta, size, 
			     rcIPair(rfRoundPlus(size.x(), dum),
				     rfRoundPlus(size.y(), dum)),
			     rcDPair(0, 0));
  }
  else {
    rcDPair size;
    size.x() = minPerimDim.x() + 1;
    size.y() = minPerimDim.y() + 1;
    static const int32 dum (0);
    return rcAffineRectangle(minPerimLL, minPerimTheta, size, 
			     rcIPair(rfRoundPlus(size.x(), dum),
				     rfRoundPlus(size.y(), dum)),
			     rcDPair(0, 0));
  }
}

ostream& operator<< (ostream& ous, const rcPolygon& p)
{
  const int32 vertexCnt = p.vertexCnt();
  rc2Dvector cop;

  const_cast<rcPolygon *>(&p)->centerOf (cop);
  std::string yConvex("CY"), nConvex("CN"), uConvex("C?");
  std::string yValid("VY"), nValid("VN"), uValid("V?");

  ous << "{ " << vertexCnt << " ";
  ous << "Area: " << const_cast<rcPolygon *>(&p)->area () << " Perimeter: " << p.perimeter () << " Center Of Mass: " << cop << ", ";
  ous << (p._knownValid ? (p._isValid ? yValid.c_str() : nValid.c_str())
	  : uValid.c_str());
  ous << (p._knownConvex ? (p._isConvex ? yConvex.c_str() : nConvex.c_str())
	  : uConvex.c_str());
  ous << " |";

  for (int32 i = 0; i < vertexCnt; i++)
    ous << " " << i << " " << p.vertexAt(i);

  ous << " }";

  return ous;
}

bool operator==(const rcPolygon& a, const rcPolygon& b)
{
  int32 count = a.vertexCnt();

  if (count != b.vertexCnt())
    return false;
  else if (count == 0)
    return true;

  int32 base = 0;
  for ( ; base < count; base++)
    if (a.vertexAt(0) == b.vertexAt(base))
      break;

  if (base == count)
    return false;

  for (int32 i = 0; i < count; i++)
    if (a.vertexAt(i) != b.vertexAt(base+i))
      return false;

  if (a._knownValid && b._knownValid)
    rmAssert(a.isValid() == b.isValid());

  if (a._knownConvex && b._knownConvex)
    rmAssert(a.isConvex() == b.isConvex());

  if (a._knownValid && !b._knownValid) {
    (const_cast<rcPolygon&>(b))._knownValid = a._knownValid;
    (const_cast<rcPolygon&>(b))._isValid = a._isValid;
  }

  if (!a._knownValid && b._knownValid) {
    (const_cast<rcPolygon&>(a))._knownValid = b._knownValid;
    (const_cast<rcPolygon&>(a))._isValid = b._isValid;
  }

  if (a._knownConvex && !b._knownConvex) {
    (const_cast<rcPolygon&>(b))._knownConvex = a._knownConvex;
    (const_cast<rcPolygon&>(b))._isConvex = a._isConvex;
  }

  if (!a._knownConvex && b._knownConvex) {
    (const_cast<rcPolygon&>(a))._knownConvex = b._knownConvex;
    (const_cast<rcPolygon&>(a))._isConvex = b._isConvex;
  }

  return true;
}


rcPolygon rcPolygon::discBuffer (double diameter) const
{
  rcPolygon buffer;
  for (uint32 i = 0; i < _vert.size(); i++)
    for (uint32 j = 0; j < _vert.size(); j++)
      {
	if (i == j) continue;
	rc2Dvector u2me = _vert[i] - _vert[j];
	double length = u2me.len ();
	rc2Dvector disc (length+diameter, u2me.angle());
	u2me = disc + _vert[j]; // the new stretched end point
	buffer.pushVertex (u2me);
      }

  // It might not be valid, so dont assert and make me valid
  buffer.createConvexHull (buffer, false);
  return buffer;
}
