/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.6  2005/11/07 17:32:09  arman
 *cell lineage iv and bug fix
 *
 *Revision 1.5  2005/07/01 21:05:40  arman
 *version2.0p
 *
 *Revision 1.5  2005/05/20 21:55:43  arman
 *added [] operators
 *
 *Revision 1.4  2004/02/11 03:07:04  arman
 *added rcpair<bool>
 *
 *Revision 1.3  2003/03/03 14:14:24  arman
 *added stream output
 *
 *Revision 1.2  2003/01/06 02:38:58  arman
 *added comments
 *
 *Revision 1.1  2002/12/08 22:56:46  arman
 *template pair of 2 values with arithmatic support
 *derived from stl pair
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcPAIR_H
#define __rcPAIR_H

#include "rc_types.h"
using namespace std;

template<class T>
class rcPair : public  pair<T,T>

/* A rcPair is a pair with pair-wise arithmetic operations
   added to the interface.

  T must have:
    T()
    T(const T&)
    ~T()
    T& operator= (const T&)

    operator+ (T, T)
    operator+= (T)
    operator- (T)
    operator- (T, T)
    operator-= (T)
    operator* (T, T)
    operator*= (T)
    operator/ (T, T)
    operator/= (T)
    operator== (T, T)

*/
{
public:
  rcPair() :  pair<T,T>() {}
  rcPair(T x, T y) :  pair<T,T>(x, y) {}
  rcPair(T x) :  pair<T,T>(x, x) {}

  /* default copy ctor, assign, dtor OK */

  const T& x() const {return pair<T,T>::first;}
  T& x() {return pair<T,T>::first;}

  const T& y() const {return pair<T,T>::second;}
  T& y() {return pair<T,T>::second;}

  rcPair<T> operator+ (const rcPair<T>& r) const;
  rcPair<T>& operator+= (const rcPair<T>& r);

  rcPair<T> operator- () const;
  rcPair<T> operator- (const rcPair<T>& r) const;
  rcPair<T>& operator-= (const rcPair<T>& r);

  rcPair<T> operator* (const rcPair<T>& r) const;
  rcPair<T>& operator*= (const rcPair<T>& r);

  rcPair<T> operator/ (const rcPair<T>& r) const;
  rcPair<T>& operator/= (const rcPair<T>& r);

  rcPair<T> operator/ (const T& r) const;
  rcPair<T>& operator/= (const T& r);

  T& operator [] (int d);
  T  operator [] (int d) const;

  bool operator== (const rcPair<T>& r) const;
  bool operator!= (const rcPair<T>& r) const { return !(*this == r); }

  friend ostream& operator<< (ostream& ous, const rcPair<T>& dis)
  {
    ous << dis.x() << "," << dis.y();
    return ous;
  }

};

typedef rcPair<uint32> rcUIPair;
typedef rcPair<int32> rcIPair;
typedef rcPair<float> rcFPair;
typedef rcPair<double> rcDPair;
typedef rcPair<bool> rcbPair;


template<class T>
rcPair<T> rcPair<T>::operator+ (const rcPair<T>& r) const
{
  return rcPair<T>(T(x() + r.x()), T(y() + r.y()));
}
template<class T>
rcPair<T>& rcPair<T>::operator+= (const rcPair<T>& r)
{
  x() += r.x();
  y() += r.y();
  return *this;
}

template<class T>
T& rcPair<T>::operator[] (int d)
{
  assert(d >= 0 && d < 2);
  return (!d) ? pair<T,T>::first : pair<T,T>::second;
}

template<class T>
T rcPair<T>::operator[] (int d) const
{
  assert(d >= 0 && d < 2);
  return (!d) ? pair<T,T>::first : pair<T,T>::second;
}

template<class T>
rcPair<T> rcPair<T>::operator- () const
{
  return rcPair<T>(T(-pair<T,T>::first), T(-pair<T,T>::second));
}

template<class T>
rcPair<T> rcPair<T>::operator- (const rcPair<T>& r) const
{
  return rcPair<T>(T(pair<T,T>::first - r.x()), T(pair<T,T>::second - r.y()));
}

template<class T>
rcPair<T>& rcPair<T>::operator-= (const rcPair<T>& r)
{
  pair<T,T>::first -= r.x();
  pair<T,T>::second -= r.y();
  return *this;
}

template<class T>
rcPair<T> rcPair<T>::operator* (const rcPair<T>& r) const
{
  return rcPair<T>(T(pair<T,T>::first * r.x()), T(pair<T,T>::second * r.y()));
}

template<class T>
rcPair<T>& rcPair<T>::operator*= (const rcPair<T>& r)
{
  pair<T,T>::first *= r.x();
  pair<T,T>::second *= r.y();
  return *this;
}

template<class T>
rcPair<T> rcPair<T>::operator/ (const rcPair<T>& r) const
{
  return rcPair<T>(T(pair<T,T>::first / r.x()), T(pair<T,T>::second / r.y()));
}

template<class T>
rcPair<T>& rcPair<T>::operator/= (const rcPair<T>& r)
{
  pair<T,T>::first /= r.x();
  pair<T,T>::second /= r.y();
  return *this;
}

template<class T>
rcPair<T> rcPair<T>::operator/ (const T& r) const
{
  return rcPair<T>(T(pair<T,T>::first / r), T(pair<T,T>::second / r));
}

template<class T>
rcPair<T>& rcPair<T>::operator/= (const T& r)
{
  pair<T,T>::first /= r;
  pair<T,T>::second /= r;
  return *this;
}
template<class T>
bool rcPair<T>::operator== (const rcPair<T>& r) const
{
  return pair<T,T>::first == r.x() && pair<T,T>::second == r.y();
}




#endif /* __rcPAIR_H */
