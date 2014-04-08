/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2004/10/06 21:23:05  arman
 *a size abstraction
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcSIZE_H
#define __rcSIZE_H


/** \class rcSize
 * \brief Represent the size (bounds) of a n-dimensional image.
 *
 * rcSize is a class to represent multi-dimensional array bounds,
 * templated over the dimension.  Insight assumes that the first
 * element of rcSize is the fastest moving index.
 *
 * For the sake of efficiency, rcSize does not define a default constructor, a
 * copy constructor, or an operator=. We rely on the compiler to provide
 * efficient bitwise copies.
 *
 * rcSize is an "aggregate" class.  Its data is public (mSize)
 * allowing for fast and convienent instantiations/assignments.
 *
 * The following syntax for assigning a size is allowed/suggested:
 *    rcSize<3> size = {256, 256, 20};
 *
 * \sa Index
 * \ingroup ImageObjects
 */
template<unsigned int VDimension=2>
class rcSize {
public:
  /** Standard class typedefs. */
  typedef rcSize  Self;
  
  /** Compatible Size and value typedef */
  typedef   rcSize<VDimension>  SizeType;
  typedef   unsigned long  SizeValueType;
  
  /** Get the dimension of the size object. */
  static unsigned int GetSizeDimension(void) { return VDimension; }

  /** Add two sizes.  */
  const Self
  operator+(const Self &vec)
    {
    Self result;
    for (unsigned int i=0; i < VDimension; i++)
      { result[i] = mSize[i] + vec.mSize[i]; }
    return result;
    }

  /** Increment size by a size.  */
  const Self &
  operator+=(const Self &vec)
    {
    for (unsigned int i=0; i < VDimension; i++)
      { mSize[i] += vec.mSize[i]; }
    return *this;
    }

  /** Subtract two sizes.  */
  const Self
  operator-(const Self &vec)
    {
    Self result;
    for (unsigned int i=0; i < VDimension; i++)
      { result[i] = mSize[i] - vec.mSize[i]; }
    return result;
    }

  /** Decrement size by a size.  */
  const Self &
  operator-=(const Self &vec)
    {
    for (unsigned int i=0; i < VDimension; i++)
      { mSize[i] -= vec.mSize[i]; }
    return *this;
    }

  /** Multiply two sizes (elementwise product).  */
  const Self
  operator*(const Self &vec)
    {
    Self result;
    for (unsigned int i=0; i < VDimension; i++)
      { result[i] = mSize[i] * vec.mSize[i]; }
    return result;
    }

  /** Multiply two sizes (elementwise product).  */
  const Self &
  operator*=(const Self &vec)
    {
    for (unsigned int i=0; i < VDimension; i++)
      { mSize[i] *= vec.mSize[i]; }
    return *this;
    }

  /** Compare two sizes. */
  bool
  operator==(const Self &vec) const
    {
    bool same=1;
    for (unsigned int i=0; i < VDimension && same; i++)
      { same = (mSize[i] == vec.mSize[i]); }
    return same;
    }

  /** Compare two sizes. */
  bool
  operator!=(const Self &vec) const
    {
    bool same=1;
    for (unsigned int i=0; i < VDimension && same; i++)
      { same = (mSize[i] == vec.mSize[i]); }
    return !same;
    }

  /** Access an element of the size. Elements are numbered
   * 0, ..., VDimension-1. No bounds checking is performed. */
  SizeValueType & operator[](unsigned int dim)
    { return mSize[dim]; }

  /** Access an element of the size. Elements are numbered
   * 0, ..., VDimension-1. This version can only be an rvalue.
   * No bounds checking is performed. */
  SizeValueType operator[](unsigned int dim) const
    { return mSize[dim]; }

  /** Get the size. This provides a read only reference to the size.
   * \sa SetSize */
  const SizeValueType *GetSize() const { return mSize; };

  /** Set the size.
   * Try to prototype this function so that val has to point to a block of
   * memory that is the appropriate size. \sa GetSize */
  void SetSize(const SizeValueType val[VDimension])
    { memcpy(mSize, val, sizeof(SizeValueType)*VDimension); }

  /** Set an element of the Size.
   * sets the value of one of the elements in the rcSize
   * This method is mainly intended to facilitate the access to elements
   * from Tcl and Python where C++ notation is not very convenient.
   * \warning No bound checking is performed.
   * \sa SetSize() \sa GetElement() */
  void SetElement(unsigned long element, SizeValueType val )
    { mSize[ element ] = val;  }

  /** Get an element of the rcSize.
   * gets the value of one of the elements in the size
   * This method is mainly intended to facilitate the access to elements
   * from Tcl and Python where C++ notation is not very convenient.
   * \warning No bound checking is performed
   * \sa GetSize() \sa SetElement() */
  SizeValueType GetElement( unsigned long element ) const
    { return mSize[ element ]; }

  /** Set one value for the index in all dimensions.  Useful for initializing
   * an offset to zero. */
  void Fill(SizeValueType value)
    { for(unsigned int i=0;i < VDimension; ++i) mSize[i] = value; }

  /** rcSize is an "aggregate" class.  Its data is public (mSize)
   * allowing for fast and convenient instantiations/assignments.
   *
   * The following syntax for assigning a size is allowed/suggested:
   *
   * rcSize<3> size = {{256, 256, 20}};
   *
   * The doubled braces {{ and }} are required to prevent `gcc -Wall'
   * (and perhaps other compilers) from complaining about a partly
   * bracketed initializer. */
  SizeValueType mSize[VDimension];

};


template<unsigned int VDimension>
std::ostream & operator<<(std::ostream &os, const rcSize<VDimension> &size)
{
  os << "[";
  for (unsigned int i=0; i+1 < VDimension; ++i)
    {
    os << size[i] << ", ";
    }
  if (VDimension >= 1)
    {
    os << size[VDimension-1];
    }
  os << "]";
  return os;
}
#ifdef ITK_EXPLICIT_INSTANTIATION
   extern template class Size<1>;
   extern template class Size<2>;
   extern template class Size<3>;
   extern template class Size<4>;
   extern template class Size<5>;
   extern template std::ostream & operator<<(std::ostream &os, const Size<1> &size);
   extern template std::ostream & operator<<(std::ostream &os, const Size<2> &size);
   extern template std::ostream & operator<<(std::ostream &os, const Size<3> &size);
   extern template std::ostream & operator<<(std::ostream &os, const Size<4> &size);
   extern template std::ostream & operator<<(std::ostream &os, const Size<5> &size);
#endif


#endif /* __rcSIZE_H */
