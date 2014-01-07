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
#include <file_system.hpp>

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



#if 0
template<typename AccumulatorType>
class order_2_accumulator
{
  public:
    typedef AccumulatorType value_type;

    order_2_accumulator():
      Count(),
      Sum(),
      Sum2()
    {}
    order_2_accumulator(unsigned int count_,
                        const value_type& sum_,
                        const value_type& sum2_):
      Count(count_),
      Sum(sum_),
      Sum2(sum2_)
    {}

    unsigned int count() const     { return Count; }
    value_type sum() const         { return Sum; }
    value_type sum_squares() const { return Sum2; }
    value_type mean() const        { return Sum/Count; }
    value_type variance() const    { return Sum2/Count - (Sum*Sum)/(Count*Count); }

    // Scott Kirkwood: added these - would require sqrt() though.
    value_type std() const         { return std::sqrt(variance); }
    value_type std_error_of_mean() { return std() / std::sqrt(Count); }
    value_type root_mean_square()  { return std::sqrt(Sum2 / Count); }
    value_type coefficient_of_variation() { return 100 * std() / mean(); }

    template<typename T>
    order_2_accumulator<value_type> bump(const T& value_)
    {
      ++Count;
      Sum  += value_;
      Sum2 += value_*value_;
      return *this;
    }
  private:
    unsigned int Count;
    value_type   Sum;
    value_type   Sum2;
};


// Default operator used by std::accumulate
template<typename AccumulatorType, typename T>
AccumulatorType operator+(const AccumulatorType& init_, const T& value_)
{
  AccumulatorType accum(init_);
  return accum.bump(value_);
}



#include <iostream>
#include <numeric>
int main()
{
  int seq[] ={1, 2, 3, 4};
  order_2_accumulator<double> sum=std::accumulate(seq, seq+4, order_2_accumulator<double>());
  std::cout << sum.count() << ' ' << sum.sum() << ' ' << sum.sum_squares() << std::endl;
  std::cout << sum.mean() << ' ' << sum.variance() << std::endl;

  double seq2[] = {1., 2., 3., 4.};
  sum=std::accumulate(seq2, seq2+4, sum);
  std::cout << sum.count() << ' ' << sum.sum() << ' ' << sum.sum_squares() << std::endl;
  std::cout << sum.mean() << ' ' << sum.variance() << std::endl;

}


 #include <cmath>

 template<class InputIterator>
 inline
 typename std::iterator_traits<InputIterator>::value_type
 mean(InputIterator begin,
      InputIterator end,
      typename std::iterator_traits<InputIterator>::value_type zero = 0)     
 {
   unsigned int count = 0;
   typename std::iterator_traits<InputIterator>::value_type sum = zero;
   for(InputIterator i=begin; i < end; ++i) {
     sum += *i;
     count++;
   }
   return sum/count;
 }


 template<class InputIterator>
 inline
 typename std::iterator_traits<InputIterator>::value_type
 variance(InputIterator begin,
          InputIterator end,
          typename std::iterator_traits<InputIterator>::value_type zero = 0)
 {
   typename std::iterator_traits<InputIterator>::value_type
     mn = mean(begin,end);
   unsigned int count = 0;
   typename std::iterator_traits<InputIterator>::value_type sum = zero;
   for(InputIterator i=begin; i < end; ++i) {
     sum += std::pow(*i - mn, 2);
     count++;
   }
   return sum/(count-1);
 }
 
 template<class InputIterator>
 inline
 typename std::iterator_traits<InputIterator>::value_type
 standard_deviation(InputIterator begin,
                    InputIterator end,
                    typename std::iterator_traits<InputIterator>::value_type zero = 0)   
 {
   return std::sqrt(variance(begin, end, zero));
 } 




/*******************************************************************************
* APPLICATION:  scientific & fixed
*
* COMMENTS:
*
* The standards of the C++ Library say that there are many stream manipulators
* in ios, but I found some to be absent from the platform that I am using.  Two
* are scientific and fixed, the manipulators that control the appearance of
* floating-point numbers.  Therefore, we shall try to provide these two
* ourselves.
*
* We shall implement these so that each sets its corresponding bit flag in the
* format state and resets the other.  Also, each has an istream and ostream
* version.  The istream version is useless since ios:scientific and ios::fixed
* do not affect input streams; however, we can see how they look.
*
* This is an example of a user-defined manipulator.  Notice that these
* manipulators require no argument from the user.
*
* ACTION:
*
* Provide the other manipulators missing from your platform.
*/ 

/*
******************************************************************************/

// #include // iostream
// #include // stdlib.h:  EXIT_SUCCESS

/******************************************************************************/
const char SIXTEEN[] = " (in base 16)";

void rfDisplayStreamFlags (long flag) {
 cout << hex << flag << SIXTEEN << endl;
 cout << "========================" << endl;
 cout << "  skipws     = " << ((flag & ios::skipws    ) ? "ON" : "OFF") << endl;
 cout << "  left       = " << ((flag & ios::left      ) ? "ON" : "OFF") << endl;
 cout << "  right      = " << ((flag & ios::right     ) ? "ON" : "OFF") << endl;
 cout << "  internal   = " << ((flag & ios::internal  ) ? "ON" : "OFF") << endl;
 cout << "  dec        = " << ((flag & ios::dec       ) ? "ON" : "OFF") << endl;
 cout << "  oct        = " << ((flag & ios::oct       ) ? "ON" : "OFF") << endl;
 cout << "  hex        = " << ((flag & ios::hex       ) ? "ON" : "OFF") << endl;
 cout << "  showbase   = " << ((flag & ios::showbase  ) ? "ON" : "OFF") << endl;
 cout << "  showpoint  = " << ((flag & ios::showpoint ) ? "ON" : "OFF") << endl;
 cout << "  uppercase  = " << ((flag & ios::uppercase ) ? "ON" : "OFF") << endl;
 cout << "  showpos    = " << ((flag & ios::showpos   ) ? "ON" : "OFF") << endl;
 cout << "  scientific = " << ((flag & ios::scientific) ? "ON" : "OFF") << endl;
 cout << "  fixed      = " << ((flag & ios::fixed     ) ? "ON" : "OFF") << endl;
 cout << "  unitbuf    = " << ((flag & ios::unitbuf   ) ? "ON" : "OFF") << endl;
 // cout << "  stdio      = " << ((flag & ios::stdio     ) ? "ON" : "OFF") << endl;
 cout << endl;
 cout.flags(flag);   // restore format flag settings disturbed by '<< hex'.
}
/******************************************************************************/

#endif


#endif /* __rcSTLPLUS_H */
