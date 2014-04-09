/*
 *
 *$Id $
 *$Log$
 *Revision 1.1  2005/08/30 21:12:03  arman
 *Cell Lineage
 *
 *Revision 1.5  2005/08/24 02:40:39  arman
 *Cell Lineage II
 *
 *Revision 1.4  2005/08/23 23:32:32  arman
 *Cell Lineage II
 *
 *Revision 1.3  2005/08/18 11:13:17  arman
 *Cell Lineage II
 *
 *Revision 1.2  2005/08/16 03:30:40  arman
 *Cell Lineage II
 *
 *Revision 1.1  2005/08/15 17:01:53  arman
 *c
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_polygongroup.h>


#if 0
void rfPolygonGroupToGraphicsCollection(rcPolygonGroupRef& pg, 
					rcVisualGraphicsCollectionCollection& v, 
					bool drawCOM)
{
  v.clear ();
  const vector<rcPolygon>& polys = pg.polys();
  for (uint32 i = 0; i < pg.size(); i++)
    {
      rcVisualGraphicsCollection graphics; 
      rcVisualSegmentCollection& segments = graphics.segments();
      rfPolygonToSegmentsCollection(polys[i], segments, drawCOM); 
      graphics.id ((int32) i);
      v.push_back (graphics);
    }
}
#endif

rcPolygonGroup::~rcPolygonGroup () {}
  
bool operator==(rcPolygonGroupRef& cells, rcPolygonGroupRef& cells2)
{
  if (cells2.size() != cells.size())
    {
      cerr << cells2.size() << " != " << cells.size() << endl;
      return 0;
    }

  for (uint32 i = 0; i < cells.size(); i++)
    {
      if (cells.polys()[i] != cells2.polys()[i])
	{
	  cerr << cells.polys()[i] << " != " << cells2.polys()[i] << endl;
	  return 0;
	}
    }
  return true;
}
  

// @note change these to use rcPolygonGroup 

bool rcPolygon::intersects (const vector<rcPolygon>& group, 
			    vector<rcPolygon>& groupIntersects, float areaRatio) const
{
  vector<rcPolygon>::const_iterator mpolys = group.begin();
  groupIntersects.clear ();
  rmAssert (areaRatio > 0.0 || areaRatio <= 1.0);

  rcIRect br = orthogonalEnclosingRect ();

  for (; mpolys != group.end(); mpolys++)
    {
      rcIRect ar = mpolys->orthogonalEnclosingRect ();
      if (!br.overlaps (ar))
	continue;

      if (intersects (*mpolys) && (&(*mpolys) != this))
	{
	  rcPolygon intPoly;
	  float areaThr = mpolys->area () * areaRatio;
	  convexIntersection(*mpolys, intPoly);
	  if (intPoly.area() > areaThr)
	    groupIntersects.push_back (*mpolys);
	}
    }
  return groupIntersects.size() != 0;
}

int32 rcPolygon::intersects (const vector<rcPolygon>& group, float areaRatio) const
{
  vector<rcPolygon>::const_iterator mpolys = group.begin();
  rmAssert (areaRatio > 0.0 || areaRatio <= 1.0);
  int32 num (0);

  rcIRect br = orthogonalEnclosingRect ();

  for (; mpolys != group.end(); mpolys++)
    {
      rcIRect ar = mpolys->orthogonalEnclosingRect ();
      if (!br.overlaps (ar))
	continue;

      if (intersects (*mpolys) && (&(*mpolys) != this))
	{
	  rcPolygon intPoly;
	  float areaThr = mpolys->area () * areaRatio;
	  convexIntersection(*mpolys, intPoly);
	  num += intPoly.area() > areaThr;
	}
    }
  return num;
}

void rfPolygonGroupMutualMerge (const vector<rcPolygon>& fixed, const vector<rcPolygon>& moving, vector<rcPolygon>& merged, double discDiameter, float areaRatio)
{
   vector<rcPolygon>::const_iterator polys = fixed.begin();
   merged.clear ();
   for (; polys != fixed.end(); polys++)
     {
       rcPolygon buffer = polys->discBuffer (discDiameter);

       vector<rcPolygon > ipolPtrs, uniquePolPtrs;

       // The intersected list would include me
       buffer.intersects (moving, ipolPtrs, areaRatio);
       
       // Check if the intersect group intersects with any other polygon
       // @note Combinatorics of this could be improved
       bool intersects (false);
       for (uint32 pi = 0; pi < ipolPtrs.size(); pi++)
	 {
	   // Skip me. Since we know that it does and we want to get it out
	   if (ipolPtrs[pi] == *polys) continue;

	   vector<rcPolygon> tmpPtrs;
	   intersects = ipolPtrs[pi].intersects (moving, tmpPtrs, areaRatio);
	   if (intersects)
	     {
	       break;
	     }
	   uniquePolPtrs.push_back (ipolPtrs[pi]);
	 }

       // Either we will have the original poly or a union of the merged.
       rcPolygon unionPoly = (uniquePolPtrs.size() > 1) ? uniquePolPtrs[0] : *polys;

       if (uniquePolPtrs.size() > 1)
	 {
	   rmAssert (!intersects);
	   // There were more than one
	   for (uint32 pi = 1; pi < uniquePolPtrs.size(); pi++)
	     {
	       rcPolygon tmpPoly;
	       unionPoly .convexUnion(uniquePolPtrs[pi], tmpPoly);
	       unionPoly = tmpPoly;
	    }
	 }

       merged.push_back (unionPoly);
     }
}

void rfPolygonGroupCenters (vector<rcPolygon>& polygons, vector<rc2Dvector>& centers)
{
  vector<rcPolygon>::iterator polys = polygons.begin();
  centers.clear ();
  for (; polys != polygons.end(); polys++)
     {
       rc2Dvector cop;
       polys->centerOf (cop);
       centers.push_back (cop);
     }
  rmAssert (polygons.size() == centers.size());
}


