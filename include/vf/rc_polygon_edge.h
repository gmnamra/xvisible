/*
 *
 *$Header $
 *$Id $
 *$Log: $
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_POLYGON_EDGE_H
#define __RC_POLYGON_EDGE_H


#include <iterator>
#include <rc_vector2d.h>

//#include <CGAL/circulator.h>

//-----------------------------------------------------------------------//
//                          rcPolygon_edge_iterator
//-----------------------------------------------------------------------//
// A polygon Edge is the line segments that connects two neighbouring 
// vertices

class rcPolygonSegment 
{
public:
  rcPolygonSegment () {} 
  rcPolygonSegment (const rc2Dvector& p, const rc2Dvector& q) : mFirst(p), mSecond(q) {}
  const rc2Dvector& source () { return mFirst; }
  const rc2Dvector& target () { return mSecond; }
private:
  rc2Dvector mFirst, mSecond;
};

template <class Segment_>
class Polygon_2__Segment_ptr
{
public:
    typedef Segment_ Segment;
    Polygon_2__Segment_ptr(Segment const &seg) :m_seg(seg){}
    Segment* operator->() {return &m_seg;}
private:
    Segment m_seg;
};

template <class Container_>
class Polygon_2_edge_iterator {
  public:
    typedef typename std::iterator_traits<typename Container_::iterator>::iterator_category iterator_category;
    typedef rcPolygonSegment Segment_2;
    typedef rcPolygonSegment value_type;
    typedef Container_ Container;
    typedef typename Container_::const_iterator const_iterator;
    typedef typename Container_::difference_type difference_type;
    typedef Segment_2*           pointer;
    typedef Segment_2&           reference;
  private:
    const Container_* container;   // needed for dereferencing the last edge
    const_iterator first_vertex;   // points to the first vertex of the edge
    Segment_2 m_seg;
  public:
    Polygon_2_edge_iterator() {}
    Polygon_2_edge_iterator(const Container_* c, const_iterator f)
      : container(c), first_vertex(f) {}

    bool operator==(
      const Polygon_2_edge_iterator<Container_>& x) const
    {
      return first_vertex == x.first_vertex;
    }
    
    bool operator!=(
      const Polygon_2_edge_iterator<Container_>& x) const
    {
      return !(first_vertex == x.first_vertex);
    }

  Segment_2 operator*() {
      const_iterator second_vertex = first_vertex;
      ++second_vertex;
      if (second_vertex == container->end())
        second_vertex = container->begin();
      m_seg = rcPolygonSegment (*first_vertex, *second_vertex);
      return m_seg;
    }

  
  Polygon_2__Segment_ptr<Segment_2> operator->()
  {return Polygon_2__Segment_ptr<Segment_2>(operator*());}
    
  Polygon_2_edge_iterator<Container_>& operator++() {
      ++first_vertex;
      return *this;
    }

    Polygon_2_edge_iterator<Container_> operator++(int) {
      Polygon_2_edge_iterator<Container_> tmp = *this;
      ++*this;
      return tmp;
    }

    Polygon_2_edge_iterator<Container_>& operator--() {
      --first_vertex;
      return *this;
    }

    Polygon_2_edge_iterator<Container_> operator--(int) {
      Polygon_2_edge_iterator<Container_> tmp = *this;
      --*this;
      return tmp;
    }

// random access iterator requirements
    Polygon_2_edge_iterator<Container_>&
    operator+=(difference_type n) {
      first_vertex += n;
      return *this;
    }

    Polygon_2_edge_iterator<Container_>
    operator+(difference_type n) {
      return Polygon_2_edge_iterator<Container_>(
        container, first_vertex + n);
    }

    Polygon_2_edge_iterator<Container_>&
    operator-=(difference_type n) {
      return (*this) -= n;
    }

    Polygon_2_edge_iterator<Container_>
    operator-(difference_type n) {
      return Polygon_2_edge_iterator<Container_>(
        container, first_vertex - n);
    }

    difference_type
    operator-(const Polygon_2_edge_iterator<Container_>& a) {
      return first_vertex - a.first_vertex;
    }

    Segment_2 operator[](int n) {
      return *Polygon_2_edge_iterator<Container_>(
        container, first_vertex+n);
    }

    bool operator<(const Polygon_2_edge_iterator<Container_>& a)
    {
      return first_vertex < a.first_vertex;
    }

    bool operator>(const Polygon_2_edge_iterator<Container_>& a)
    {
      return first_vertex > a.first_vertex;
    }

    bool operator<=(const Polygon_2_edge_iterator<Container_>& a)
    {
      return first_vertex <= a.first_vertex;
    }

    bool operator>=(const Polygon_2_edge_iterator<Container_>& a)
    {
      return first_vertex >= a.first_vertex;
    }

};


template <class Container_>
typename Container_::difference_type
distance_type(const Polygon_2_edge_iterator<Container_>&)
{ return Container_::difference_type(); }

template <class Container_>
rcPolygonSegment* value_type(const Polygon_2_edge_iterator<Container_>&)
{ return (rcPolygonSegment *)(0); }


//-----------------------------------------------------------------------//
//                          implementation
//-----------------------------------------------------------------------//

//--------------------------------------------------------------------//
// I don't know how to implement the following function:
//
// template <class class Container_>
// inline
// Polygon_2_edge_iterator<Container_>
// operator+(Container_::difference_type n,
//           Polygon_2_edge_iterator<Container_>& a)
// { return a+n; }
//--------------------------------------------------------------------//


#endif /* __RC_POLYGON_EDGE_H */
