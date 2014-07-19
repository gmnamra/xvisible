/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/08/30 21:10:26  arman
 *Cell Lineage
 *
 *Revision 1.2  2005/08/23 23:32:32  arman
 *Cell Lineage II
 *
 *Revision 1.1  2005/08/16 13:11:28  arman
 *a starting place for polygon group operations
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcPOLYGONGROUP_H
#define __rcPOLYGONGROUP_H

#include "rc_polygon.h"
#include <boost/shared_ptr.hpp>

// Two polygon groups represent fixed and moving set in a motion segmentation iteration.
// The moving set has motion with respect to fixed. 
class rcPolygonGroup;

typedef boost::shared_ptr<rcPolygonGroup> rcSharedPolygonGroupPtr;

class rcPolygonGroup 
{
public: 
  rcPolygonGroup () {}
  virtual ~rcPolygonGroup ();

  const uint32 size () const { return const_cast<rcPolygonGroup*>(this)->polys().size (); }

  vector<rcPolygon>& polys () { return const_cast<rcPolygonGroup*>(this)->_polys; }

private:
  rcPolygonGroup(const rcPolygonGroup& rhs);
  rcPolygonGroup& operator=(const rcPolygonGroup& rhs);
  vector<rcPolygon> _polys;
};

class rcPolygonGroupRef
{
public:
  // Constructors
 //! default constructor takes no arguments
    /*!
    */
  rcPolygonGroupRef () : mGroup( new rcPolygonGroup () ) {}

 //! Attach to an existing buffer
    /*!
      \param ptr is a group pointer (ref counted) 
    */
  rcPolygonGroupRef( rcSharedPolygonGroupPtr ptr) : mGroup (ptr) {} 

  //! copy / assignment  takes one arguments 
    /*!
      \param other
      \desc  Copy and assignment constructors increment the ref_count
    */
  rcPolygonGroupRef ( const rcPolygonGroupRef& other) :
    mGroup( other.group () ) {}

//! copy / assignment  takes one arguments 
    /*!
      \param rhs
      \desc  Copy and assignment constructors increment the ref_count
    */
  const rcPolygonGroupRef& operator= (const rcPolygonGroupRef& rhs)
  {
    if (this == &rhs) return *this;
    mGroup = rhs.group();
    return *this;
  }

   // Accessors
  const rcSharedPolygonGroupPtr& group () const { return mGroup; }
  rcSharedPolygonGroupPtr& group () { return mGroup; }

  // Is this group bound ?
  uint32 isBound () const { return mGroup != 0; }
  const vector<rcPolygon>& polys () { rmAssert (isBound ()); return mGroup->polys (); }
  const uint32 size () const { return mGroup->size ();}
  void push_back (const rcPolygon& p) { mGroup->polys().push_back (p); }
  const rcPolygon& operator[](int n) { return const_cast<vector<rcPolygon>&> (mGroup->polys()) [n]; }

  friend bool operator==(const rcPolygonGroupRef& a, const rcPolygonGroupRef& b);
  friend bool operator!=(const rcPolygonGroupRef& a, const rcPolygonGroupRef& b);

private:
protected:
  rcSharedPolygonGroupPtr mGroup;
};
  
/* rfPolygonToSegmentsCollection - Takes the source polygon p and puts a
 * displayable version of it into v.
 */
//void rfPolygonGroupToGraphicsCollection(const rcPolygonGroupRef& pg, rcVisualGraphicsCollectionCollection& v, bool drawCOM = false);

void rfPolygonGroupMutualMerge (const vector<rcPolygon>& fixed, const vector<rcPolygon>& moving, 
				vector<rcPolygon>& merged, double discDiameter, float areaRatio = 0.25f);

void rfPolygonGroupCenters (vector<rcPolygon>& polygons, vector<rc2Dvector>& centers);


inline bool operator!=(const rcPolygonGroupRef& a, const rcPolygonGroupRef& b)
{
  return !operator==(a, b);
}



#endif /* __rcPOLYGONGROUP_H */
