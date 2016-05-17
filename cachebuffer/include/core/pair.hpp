#ifndef __TPAIR_H
#define __TPAIR_H


using namespace std;

template<class T>
class tpair : public  pair<T,T>

/* jsize is a simple pair with identical types 

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
  tpair() :  pair<T,T>() {}
  tpair(T x, T y) :  pair<T,T>(x, y) {}
  tpair(T x) :  pair<T,T>(x, x) {}

  /* default copy ctor, assign, dtor OK */

#if defined (_OPENCV_INC_)
	tpair (const CvPoint& r)
	{
		tpair ((T) r.x, (T) r.y);
	}
	tpair& operator=(const CvPoint& r)
	{
		pair<T,T>::first = (T) r.x;
		pair<T,T>::second = (T) r.y;
		return *this;
	}
	
	CvPoint to_cv ()
	{
		CvPoint r;
		r.x = (int) x();
		r.y = (int) y();            
		return r;
	}
#endif
	
  const T& x() const {return pair<T,T>::first;}
  T& x() {return pair<T,T>::first;}

  const T& y() const {return pair<T,T>::second;}
  T& y() {return pair<T,T>::second;}

  tpair<T> operator+ (const tpair<T>& r) const;
  tpair<T>& operator+= (const tpair<T>& r);

  tpair<T> operator- () const;
  tpair<T> operator- (const tpair<T>& r) const;
  tpair<T>& operator-= (const tpair<T>& r);

  tpair<T> operator* (const tpair<T>& r) const;
  tpair<T>& operator*= (const tpair<T>& r);

  tpair<T> operator/ (const tpair<T>& r) const;
  tpair<T>& operator/= (const tpair<T>& r);

  tpair<T> operator/ (const T& r) const;
  tpair<T>& operator/= (const T& r);

  T& operator [] (int d);
  T  operator [] (int d) const;

  bool operator== (const tpair<T>& r) const;
  bool operator!= (const tpair<T>& r) const { return !(*this == r); }

  friend ostream& operator<< (ostream& ous, const tpair<T>& dis)
  {
    ous << dis.x() << "," << dis.y();
    return ous;
  }

};

typedef tpair<unsigned int> tpair_UI;
typedef tpair<int> tpair_I;
typedef tpair<float> tpair_F;
typedef tpair<double> tpair_D;
typedef tpair<bool> tpair_b;


template<class T>
tpair<T> tpair<T>::operator+ (const tpair<T>& r) const
{
  return tpair<T>(T(x() + r.x()), T(y() + r.y()));
}
template<class T>
tpair<T>& tpair<T>::operator+= (const tpair<T>& r)
{
  x() += r.x();
  y() += r.y();
  return *this;
}

template<class T>
T& tpair<T>::operator[] (int d)
{
  assert(d >= 0 && d < 2);
  return (!d) ? pair<T,T>::first : pair<T,T>::second;
}

template<class T>
T tpair<T>::operator[] (int d) const
{
  assert(d >= 0 && d < 2);
  return (!d) ? pair<T,T>::first : pair<T,T>::second;
}

template<class T>
tpair<T> tpair<T>::operator- () const
{
  return tpair<T>(T(-pair<T,T>::first), T(-pair<T,T>::second));
}

template<class T>
tpair<T> tpair<T>::operator- (const tpair<T>& r) const
{
  return tpair<T>(T(pair<T,T>::first - r.x()), T(pair<T,T>::second - r.y()));
}

template<class T>
tpair<T>& tpair<T>::operator-= (const tpair<T>& r)
{
  pair<T,T>::first -= r.x();
  pair<T,T>::second -= r.y();
  return *this;
}

template<class T>
tpair<T> tpair<T>::operator* (const tpair<T>& r) const
{
  return tpair<T>(T(pair<T,T>::first * r.x()), T(pair<T,T>::second * r.y()));
}

template<class T>
tpair<T>& tpair<T>::operator*= (const tpair<T>& r)
{
  pair<T,T>::first *= r.x();
  pair<T,T>::second *= r.y();
  return *this;
}

template<class T>
tpair<T> tpair<T>::operator/ (const tpair<T>& r) const
{
  return tpair<T>(T(pair<T,T>::first / r.x()), T(pair<T,T>::second / r.y()));
}

template<class T>
tpair<T>& tpair<T>::operator/= (const tpair<T>& r)
{
  pair<T,T>::first /= r.x();
  pair<T,T>::second /= r.y();
  return *this;
}

template<class T>
tpair<T> tpair<T>::operator/ (const T& r) const
{
  return tpair<T>(T(pair<T,T>::first / r), T(pair<T,T>::second / r));
}

template<class T>
tpair<T>& tpair<T>::operator/= (const T& r)
{
  pair<T,T>::first /= r;
  pair<T,T>::second /= r;
  return *this;
}
template<class T>
bool tpair<T>::operator== (const tpair<T>& r) const
{
  return pair<T,T>::first == r.x() && pair<T,T>::second == r.y();
}


#endif /* __PAIR_H */
