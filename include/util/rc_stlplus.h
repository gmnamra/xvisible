/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.9  2005/08/30 21:08:51  arman
 *Cell Lineage
 *
 *Revision 1.9  2005/08/19 21:10:52  arman
 *added rfPRINT_STL_ELEMENTS
 *
 *Revision 1.8  2005/07/01 21:05:40  arman
 *version2.0p
 *
 *Revision 1.8  2005/06/06 20:59:56  arman
 *corrected a range bug
 *
 *Revision 1.7  2005/04/25 00:48:39  arman
 *added a yet unused STL algorithm approach to stats
 *
 *Revision 1.6  2005/04/12 21:25:35  arman
 *added rfToString
 *
 *Revision 1.5  2004/09/19 17:51:49  arman
 *silenced the warnings
 *
 *Revision 1.4  2004/08/09 13:21:15  arman
 *added deque
 *
 *Revision 1.3  2004/04/01 23:51:41  arman
 *switched to mathematica format and turned of scientific notation
 *
 *Revision 1.2  2004/03/31 00:18:35  arman
 *STL convenience help
 *
 *Revision 1.1  2004/03/30 21:20:39  arman
 *printing lists and vectors
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcSTLPLUS_H
#define __rcSTLPLUS_H

#include <iomanip>
#include <vector>
#include <list>
#include <deque>
#include <iostream>
#include <iterator>
#include <rc_types.h>
#include <sstream>
#include "file_system.hpp"

using namespace std;


#define fileokandwritable(a) ( ! ( (a).empty() || ! is_present ((a)) || \
! is_file ((a)) || ! file_writable ((a)) || \
! is_full_path ((a)) ) )

#define fileokandreadable(a) ( ! ( (a).empty() || ! is_present ((a)) || \
! is_file ((a)) || ! file_readable ((a)) || \
! is_full_path ((a)) ) )

#define dirokandwritable(a) ( ! ( (a).empty() || ! folder_exists ((a)) || \
! folder_writable ((a)) || ! is_full_path ((a))))

#define dirokandreadable(a) ( ! ( (a).empty() || ! folder_exists ((a)) || \
! folder_readable ((a)) || ! is_full_path ((a))))


/**
 * Famous Constants
 */
template <typename T>
class Constants
{
public:
    typedef T value_type;
    
    // Pi, the ratio of a circle's circumference to its diameter.
    static inline value_type pi() { return static_cast<value_type>
        (3.14159265358979323846264338327950288419716939937510);
    }
    
    // Base of natural logarithm
    static inline value_type e() { return static_cast<value_type>
        (2.71828182845904523536028747135266249775724709369995);
    }
    
    // Euler's (Euler-Mascheroni) Constant
    static inline value_type g() { return static_cast<value_type>
        (0.57721566490153286060651209008240243104215933593992);
    }
    
    static inline value_type goldenRatio()
    {
        return static_cast<value_type>
        (1.61803398874989484820458683436563811);
    }
};


template <typename Array>
double vectorSum(Array a, long count) { // Array can be a pointer or an iterator
  double sum = 0.0;
  for (long i = 0; i<count; ++i) 
    sum += a[i];
  return sum;                  
} 


template<typename ForwardIter,
  typename OutputIter, typename UnaryPred>
OutputIter copy_if(ForwardIter begin, ForwardIter end,
  OutputIter dest, UnaryPred f) {
  while(begin != end) {
    if(f(*begin))
      *dest++ = *begin;
    begin++;
  }
  return dest;
}


bool rfNocaseEqual (const std::string& s1, const std::string& s2);

template<class T>
std::string rfToString(const T& val)
{
    std::ostringstream strm;
    strm << val;
    return strm.str();
}

template <class T>
inline void rfPRINT_STL_ELEMENTS (const T& coll, const char * optcstr = "")
{
  typename T::const_iterator pos;
  std::cout << optcstr;
  for (pos=coll.begin(); pos !=coll.end(); ++pos)
    std::cout << *pos << " ";

  std::cout << std::endl;
}

template <class Storable>
ostream& operator<<(ostream& os, const vector<Storable>& v)
{
  os.setf (ios::fixed);
  os << setprecision (7);
  os << "{ ";

  // These typename definitions silence gcc compiler warnings
  typedef typename vector<Storable>::const_iterator const_iterator;

  const_iterator p, q;
  int32 i (0);
  q = v.end() - 1;
  for( p = v.begin(); p < q; p++, i++)
    os << "{" << i << "," << *p << "}" << ',';
  os << "{" << i << "," << *q << "}" << '}';
  return os;
}

template <class Storable>
ostream& operator<<(ostream& os, const deque<Storable>& v)
{
  os.setf (ios::fixed);
  os << setprecision (12);

  // These typename definitions silence gcc compiler warnings
  typedef typename deque<Storable>::const_iterator const_iterator;

  const_iterator p, q;
  int32 i (0);
  q = v.end() - 1;
  for( p = v.begin(); p <= q; p++, i++)
  {
      os << *p;
      if (p < q) os << ",";
  }
  return os;
}

template <class Storable>
ostream& operator<<(ostream& os, const list<Storable>& v)
{
  os << "{ ";

  // These typename definitions silence gcc compiler warnings
  typedef typename list<Storable>::const_iterator const_iterator;

  const_iterator p;
  for( p = v.begin(); p != v.end(); p++)
    os << *p << ' ';
  os << "}";
  return os;
}


//@ note A circular sequence container. 
//  If you reach the end, it just wraps around to the beginning. 

template<class T> class rcRingContainer {
list<T> lst;
public:
  typedef T value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
 
// Declaration necessary so the following
// 'friend' statement sees this 'iterator'
// instead of std::iterator:
	class iterator;
	friend class iterator;
	class iterator : public std::iterator<
	std::bidirectional_iterator_tag,T,ptrdiff_t>{
		typename list<T>::iterator it;
		list<T>* r;
	public:
		iterator(list<T>& lst,
			const typename list<T>::iterator& i)
			: it(i), r(&lst) {}
		bool operator==(const iterator& x) const {
			return it == x.it;
		}
		bool operator!=(const iterator& x) const {
			return !(*this == x);
		}
		typename list<T>::reference operator*() const {
			return *it;
		}
		iterator& operator++() {
			++it;
			if(it == r->end())
				it = r->begin();
			return *this;
		}
		iterator operator++(int) {
			iterator tmp = *this;
			++*this;
			return tmp;
		}
		iterator& operator--() {
			if(it == r->begin())
				it = r->end();
			--it;
			return *this;
		}
		iterator operator--(int) {
			iterator tmp = *this;
			--*this;
			return tmp;
		}
		iterator insert(const T& x) {
			return iterator(*r, r->insert(it, x));
		}
		iterator erase() {
			return iterator(*r, r->erase(it));
		}
	};
	void push_back(const T& x) { lst.push_back(x); }
	iterator begin() { return iterator(lst, lst.begin()); }
	int size() { return lst.size(); }
};

template <typename T>
class LinearFunc
{
public:
    typedef T value_type;
    typedef T result_type;
    
    LinearFunc(const T a, const T b) : _a(a), _b(b) {}
    
    T operator()(const T &x) const { return _a*x + _b; }
    
private:
    const T _a, _b;
};

// A quadratic function: y = a*x^2 + b*x + c
template <typename T>
class QuadraticFunc
{
public:
    typedef T value_type;
    typedef T result_type;
    
    QuadraticFunc(const T a, const T b, const T c) :
    _a(a), _b(b), _c(c) {}
    
    T operator()(const T &x) const { return _a*x*x + _b*x + _c; }
    
private:
    const T _a, _b, _c;
};

template <typename T>
struct Square
{
	typedef T value_type;
	typedef T result_type;
    
	T operator()(const T &x) const { return x*x; }
};

// Gaussian Normal
template <typename T>
class Gaussian
{
public:
    typedef T value_type;
    typedef T result_type;
    
    Gaussian(const T mu, const T sigmaSq):_mu(mu),
    _OneOver2SigmaSq(T(1) / (T(2) * sigmaSq)),
    _OneOverSigma(T(1) / std::sqrt(sigmaSq)) { }
    
    T operator()(const T &x) const
    {
        const T OneOverSqrt2Pi = T(1) /
        std::sqrt( T(2)*Constants<T>::pi() );
        const T xmu = x - _mu;
        
        return OneOverSqrt2Pi * _OneOverSigma *
        std::exp(- xmu*xmu * _OneOver2SigmaSq );
    }
    
private:
    const T _mu, _OneOver2SigmaSq, _OneOverSigma;
};

template <typename T>
class Clamp
{
public:
    typedef T value_type;
    typedef T result_type;
    
    Clamp(const T low, const T high) : _low(low), _high(high) { }
    
    T operator()(const T &x) const
    { return std::min(_high, std::max(x, _low)); }
    
private:
    const T _low, _high;
};




#endif /* __rcSTLPLUS_H */
