#ifndef _STL_UTILS_HPP
#define _STL_UTILS_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

struct null_deleter
{
  void operator() (void const *) const {}
};

template <typename PT> 
inline PT align(PT val, std::size_t alignment)
   { 
   return val+(alignment - val%alignment)%alignment; 
   }

template<typename PT>
struct iBuffer
   {
   iBuffer (int capture_size, int alignment)
      {  
      _base  = boost::shared_ptr<PT>(new PT [capture_size] );
      unsigned char* tmp=(alignment>0) ? (unsigned char*)align((std::size_t) _base.get(),alignment) : _base.get();
      _diff = (std::size_t)(tmp) -  (std::size_t) _base.get();
      assert (_diff >= 0);
      }

   boost::shared_ptr<PT> _base;
   std::ptrdiff_t _diff;
   };


template<class T>
std::string t_to_string(T i)
   {
   std::stringstream ss;
   std::string s;
   ss << i;
   s = ss.str();

   return s;
   }


// int r = signextend<signed int,5>(x);  // sign extend 5 bit number x to r
template <typename T, unsigned B>
inline T signextend(const T x)
   {
   struct {T x:B;} s;
   return s.x = x;
   }

template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
  std::istringstream iss(s);
  return !(iss >> f >> t).fail ();
}


static std::string get_random_string (int length) 
   {
   static std::string chars(
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "1234567890");

   boost::random::random_device rng;
   boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
   std::vector<char> str_c;
   str_c.resize (length);
   for(int i = 0; i < length; ++i) str_c[i] = chars[index_dist(rng)];
   return std::string (&str_c[0]);

   }



#endif
