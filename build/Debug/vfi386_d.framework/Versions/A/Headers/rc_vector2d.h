/*
 *
 *  $Header $
 *  $Id $
 *  $Log$
 *  Revision 1.21  2005/11/20 03:52:15  arman
 *  motion 2 fixed fixed
 *
 *  Revision 1.20  2005/08/30 21:08:51  arman
 *  Cell Lineage
 *
 *  Revision 1.20  2005/07/21 22:07:41  arman
 *  incremental
 *
 *  Revision 1.19  2005/07/01 21:05:40  arman
 *  version2.0p
 *
 *  Revision 1.19  2005/05/19 22:10:33  arman
 *  added overload of adding pairs to vectors of like type
 *
 *  Revision 1.18  2005/04/23 18:13:04  arman
 *  len 0 now throws an exception
 *
 *  Revision 1.17  2004/01/21 21:41:47  arman
 *  added a fpair cotr
 *
 *  Revision 1.16  2004/01/14 20:15:39  arman
 *  added project and prependicular
 *
 *  Revision 1.15  2003/11/27 02:53:43  arman
 *  added distanceSquared
 *
 *  Revision 1.14  2003/06/16 14:56:03  arman
 *  added Ref()
 *
 *  Revision 1.13  2003/04/09 02:17:30  arman
 *  angle returns rcRadians
 *
 *  Revision 1.12  2003/04/03 22:55:25  sami
 *  Obsoleted stuff
 *
 *  Revision 1.11  2003/03/23 14:12:45  arman
 *  added stream operation and warning on integer support
 *
 *  Created by Arman Garakani on Thu Sep 05 2002.
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rc2DVECTOR_H_
#define _rc2DVECTOR_H_

#include <math.h>
#include <rc_types.h>
#include <rc_exception.h>
#include <rc_pair.h>
#include <rc_math.h>
#include <rc_macro.h>

// Note:
// Other than doubles and floats, integer vectors are not currently 
// implemented correctly for angle calculation.
// TBD: add integer and cordic support

template <class T>
class RFY_API rc2dVector
{
public:
   // default copy/assign/dtors OK.

   rc2dVector ()                     {mX[0] = 0.;         mX[1] = 0.; }
   rc2dVector (T x, T y)   {mX[0] = x ;         mX[1] = y; }
   rc2dVector (T r, rcRadian t)   {mX[0] = r * cos(t); mX[1] = r * sin(t);}
   rc2dVector (const rcDPair& p)     {mX[0] = p.x();      mX[1] = p.y();}
   rc2dVector (const rcIPair& p)     {mX[0] = p.x();      mX[1] = p.y();}
   rc2dVector (const rcFPair& p)     {mX[0] = p.x();      mX[1] = p.y();}

   // Future constructors for radians, etc

   T x () const      { return mX[0]; }
   T y () const      { return mX[1]; }
   void   x (T newX) { mX[0] = newX; }
   void   y (T newY) { mX[1] = newY; }
  T len () const    { return sqrt( mX[0] * mX[0] + mX[1] * mX[1]) ; }
  T l2 () const    { return ( mX[0] * mX[0] + mX[1] * mX[1]) ; }
   /*
    * effect   Returns the vector length.
    */
   bool isNull() const { return ((rfRealEq (mX[0], 0)) && (rfRealEq (mX[1], 0)));}
   bool isLenNull() const { return rfRealEq (len(), 0); }


   T& operator [] (int d)       {assert(d >= 0 && d < 2); return mX[d];}
   T  operator [] (int d) const {assert(d >= 0 && d < 2); return mX[d];}
   T *Ref () const;

   rc2dVector<T> perpendicular() const
   {
     return rc2dVector<T>(-mX[1], mX[0]);
   }

   /*
    * effect   Returns the unit vector parallel to me.
    */

   rc2dVector<T> unit() const;
   /*
    * effect   Returns the unit vector parallel to me.
    */

   rc2dVector<T> project(const rc2dVector<T>&) const;
     
  /*
   * effect   Returns the vector which is the result of projecting the argument
   * vector onto me.
   */

   T distance(const rc2dVector<T>&) const;
   T distanceSquared(const rc2dVector<T>&) const;

   /*
    * returns  the distance to the other vector (length of the difference)
    */

   rcRadian angle () const;
   /*
    * returns  angle in the range of -PI and PI
    */

   rc2dVector<T> operator+(const rc2dVector<T>&) const;
   rc2dVector<T> operator+(const rcPair<T>&) const;

   rc2dVector<T>& operator+=(const rc2dVector<T>&);
   rc2dVector<T>& operator+=(const rcPair<T>&);

   rc2dVector<T> operator-() const;
   rc2dVector<T> operator-(const rc2dVector<T>&) const;
   rc2dVector<T> operator-(const rcPair<T>&) const;
   rc2dVector<T>& operator-=(const rc2dVector<T>&);
   rc2dVector<T>& operator-=(const rcPair<T>&);

   rc2dVector<T>  operator*(T d) const;
   rc2dVector<T>& operator*=(T);

   T operator*(const rc2dVector<T>&) const;  // dot (inner) product
   T dot(const rc2dVector<T>&) const;
   T cross(const rc2dVector<T>&) const; // Note: 2D cross product is a scalar

   rc2dVector<T>  operator/(T d) const;
   rc2dVector<T>& operator/=(T);

   bool operator==(const rc2dVector<T>&) const;
   bool operator!=(const rc2dVector<T>&) const;

   bool operator<  (const rc2dVector<T>& rhs) const;
   // note	These declarations are required by the standard template
   //		headers.  The functions are NOT implemented.

  friend ostream& operator<< (ostream& ous, const rc2dVector<T>& vec)
  {
    ous << "[" << vec.x() << "," << vec.y() << "]";
    return ous;
  }

private:
      T mX[2];
};

typedef rc2dVector<double> rc2Dvector;
typedef rc2dVector<float> rc2Fvector;

template<class T>
inline T rc2dVector<T>::distance (const rc2dVector<T>& v) const
{ return (*this - v).len(); }

template<class T>
inline rc2dVector<T> rc2dVector<T>::operator+(const rc2dVector<T>& v) const
{ return rc2dVector<T> ( mX[0] + v.mX[0], mX[1] + v.mX[1]); }

template<class T>
inline rc2dVector<T> rc2dVector<T>::operator+(const rcPair<T>& v) const
{ return rc2dVector<T> ( mX[0] + v.x(), mX[1] + v.y()); }

template<class T>
inline rc2dVector<T>& rc2dVector<T>::operator+=(const rc2dVector<T>& v)
{ return *this = *this + v; }

template<class T>
inline rc2dVector<T>& rc2dVector<T>::operator+=(const rcPair<T>& v)
{ return *this = *this + v; }

template<class T>
inline rc2dVector<T> rc2dVector<T>::operator- () const
{ return rc2dVector<T>(-mX[0], -mX[1]); }

template<class T>
inline rc2dVector<T> rc2dVector<T>::operator-(const rc2dVector<T>& v) const
{ return rc2dVector<T>( mX[0] - v.mX[0], mX[1] - v.mX[1]); }

template<class T>
inline rc2dVector<T> rc2dVector<T>::operator-(const rcPair<T>& v) const
{ return rc2dVector<T>( mX[0] - v.x(), mX[1] - v.y()); }

template<class T>
inline rc2dVector<T>& rc2dVector<T>::operator-=(const rc2dVector<T>& v)
{ return *this = *this - v; }

template<class T>
inline rc2dVector<T>& rc2dVector<T>::operator-=(const rcPair<T>& v)
{ return *this = *this - v; }

template<class T>
inline rc2dVector<T> rc2dVector<T>::operator* (T d) const
{ return rc2dVector<T>(mX[0] * d, mX[1] * d); }


template<class T>
inline rc2dVector<T>& rc2dVector<T>::operator*=(T d)
{ return *this = *this * d; }

template<class T>
inline T rc2dVector<T>::operator*(const rc2dVector<T>& v) const
{ return (mX[0] * v.mX[0] + mX[1] * v.mX[1]); }

template<class T>
inline T rc2dVector<T>::dot(const rc2dVector<T>& v) const
{ return *this * v; }

template<class T>
inline T rc2dVector<T>::distanceSquared(const rc2dVector<T>& v) const
{ return (rmSquare (mX[0] - v.mX[0]) + rmSquare (mX[1] - v.mX[1]));}

template<class T>
inline T rc2dVector<T>::cross(const rc2dVector<T>& v) const
{ return mX[0] * v.mX[1] - mX[1] * v.mX[0]; }

template<class T>
inline rc2dVector<T> rc2dVector<T>::operator/ (T d) const
{ 
  if (d == 0.0)
    rmExceptionMacro(<<"Zero Length");
  return *this * (1. / d);
 } // divide once

template<class T>
inline rc2dVector<T>& rc2dVector<T>::operator/=(T d)
{ return *this = *this / d; }

template<class T>
inline bool rc2dVector<T>::operator==(const rc2dVector<T>& v) const
{ return ((mX[0] == v.mX[0]) && (mX[1] == v.mX[1])); }

template<class T>
inline bool rc2dVector<T>::operator !=(const rc2dVector<T>& v) const
{ return !(*this == v); }


template<class T>
inline rcRadian rc2dVector<T>::angle () const
{
  if (isNull())
    {
      rmExceptionMacro(<<"Zero Length");
    }
  return rcRadian ((atan2 (double (y()), double (x()))));
}

template<class T>
inline rc2dVector<T> rc2dVector<T>::unit() const
{
  rmAssertDebug (!isLenNull());
  return *this / len();
}

template<class T>
inline T* rc2dVector<T>::Ref() const
{
  return ((T*) mX);
}

template<class T>
inline rc2dVector<T> rc2dVector<T>::project(const rc2dVector<T>& other) const
{
  return ((other * *this) / (*this * *this)) * *this;
}
     

#endif // _rc2DVECTOR_H_
