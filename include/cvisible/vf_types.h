#ifndef __VF_TYPES__
#define __VF_TYPES__


#if WIN32
typedef  __int64 int64;
typedef int64 uint64;
#elif defined(__APPLE__)
#include <sys/types.h>
typedef u_int64_t        uint64;
typedef int64_t         int64;
#endif


#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>


namespace fs = boost::filesystem;


// Integer types
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

typedef char			int8;
typedef signed short    int16;
typedef signed int      int32;

#define _Internal_UnusedStringify(macro_arg_string_literal) #macro_arg_string_literal

#define UnusedParameter(macro_arg_parameter) _Pragma(_Internal_UnusedStringify(unused(macro_arg_parameter)))

#endif

