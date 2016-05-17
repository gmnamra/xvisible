#ifndef Cachedbuffer_util_hpp
#define Cachedbuffer_util_hpp
#include <string.h>
#include <iostream>
#include <ostream>
#include <vector>
#include <map>
#include <assert.h>

using namespace std;



template< class T >
std::ostream & operator << ( std::ostream & os, const std::vector< T > & v ) {
    for ( const auto & i : v ) {
        os << i << std::endl;
    }
    return os;
}


template<class A,class B>
inline ostream& operator<<(ostream& out, const map<A,B>& m)
{
    out << "{";
    for (typename map<A,B>::const_iterator it = m.begin();
         it != m.end();
         ++it) {
        if (it != m.begin()) out << ",";
        out << it->first << "=" << &it->second;
    }
    out << "}";
    return out;
}

template<class A,class B>
inline ostream& operator<<(ostream& out, const multimap<A,B>& m)
{
    out << "{{";
    for (typename multimap<A,B>::const_iterator it = m.begin();
         it != m.end();
         ++it) {
        if (it != m.begin()) out << ",";
        out << it->first << "=" << &it->second;
    }
    out << "}}";
    return out;
}


#endif
