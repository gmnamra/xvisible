// Copyright 2002 Reify, Inc.

#ifndef __rcTYPES_H__
#define __rcTYPES_H__


// Platform-specific definitions

// @note preferred way of including platform specific headers

#if defined (__ppc__)
#include "architecture/ppc/alignment.h"
#else
#include "architecture/i386/alignment.h"
#endif


#include <string>
#include <assert.h>


// Generic helper definitions for shared library support
#define RFY_HELPER_DLL_IMPORT __attribute__ ((visibility("default")))
#define RFY_HELPER_DLL_EXPORT __attribute__ ((visibility("default")))
#define RFY_HELPER_DLL_LOCAL  __attribute__ ((visibility("hidden")))

// Now we use the generic helper definitions above to define RFY_API and RFY_LOCAL.
// RFY_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// RFY_LOCAL is used for non-api symbols.

#ifdef RFY_DLL // defined if RFY is compiled as a DLL
#ifdef RFY_DLL_EXPORTS // defined if we are building the RFY DLL (instead of using it)
#define RFY_API RFY_HELPER_DLL_EXPORT
#else
#define RFY_API RFY_HELPER_DLL_IMPORT
#endif // RFY_DLL_EXPORTS
#define RFY_LOCAL RFY_HELPER_DLL_LOCAL
#else // RFY_DLL is not defined: this means RFY is a static lib.
#define RFY_API
#define RFY_LOCAL
#endif // RFY_DLL


#ifndef MAC_OS_X_VERSION_MIN_REQUIRED
#define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_5
#endif // Min_REQUIRED

#ifndef MAC_OS_X_VERSION_MAX_ALLOWED
#define MAC_OS_X_VERSION_MAX_ALLOWED MAC_OS_X_VERSION_10_5
#endif // Max_Allowed

#ifndef _PTHREADS
#define _PTHREADS 1 // to turn on thread support in STL
#endif // _PTHREADS

// Declaration for API to check if SIMD (Altivec) is supported
bool rfHasSIMD ( );
bool rfForceSIMD (bool v);


// 64-bit integer types

#if WIN32
typedef  __int64 int64;
typedef int64 uint64;
#elif defined(__APPLE__)
#include <sys/types.h>
typedef uint64_t        uint64;
typedef int64_t         int64;
#endif

// Integer types
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

typedef char			int8;
typedef signed short    int16;
typedef signed int      int32;




// See http://en.wikipedia.org/wiki/Byte-order_mark

// Platform byte order
enum reByteOrder {
    eByteOrderUnknown = 0,
    eByteOrderBigEndian,
    eByteOrderLittleEndian
};

#define rcBOM_BE 0xFEFF     // Byte order mark big-endian
#define rcBOM_LE 0xFFFE     // Byte order mark little-endian

//
// Custom macros
//

// Custom assert, always defined regardless of debug level
#define _rcassert(expression, file, lineno)  \
    (printf ("%s:%u: failed assertion '%s'\n", file, lineno, expression),	\
   abort (), 0)

// These assertions are enabled even in an optimized release build
#define rmAssert(expression)  ((void) ((expression) ? 0 : _rcassert (#expression, __FILE__, __LINE__)))
// These assertions are enabled only in a debug build and disabled in an optimized release build
#define rmAssertDebug(expression) assert(expression)

// Use this to silence compiler warnings about unused function arguments
#define rcUNUSED(x) ((void) x) // TODO: obsolete this
#define rmUnused(x) ((void) x)
// Number of elements in an array
#define rmDim(x) (sizeof(x)/sizeof(x[0]))


// Callback class for generic progress indication
class rcProgressIndicator {

  public:
	virtual ~rcProgressIndicator () {}
    // Call these with a number [0-100] indicating
    // progress of operation. If the return value is
    // true, the analysis should be aborted
    virtual bool progress( uint32 percentComplete ) = 0;
    virtual bool progress( double percentComplete ) = 0;
};

/**
 * Check if pointer is not null
 * @ingroup Misc
 */
inline bool isNotNullP( const void *pointer)
{
    return pointer != NULL;
}
/**
 * Check if pointer is null
 * @ingroup Misc
 */
inline bool isNullP( const void *pointer)
{
    return pointer == NULL;
}

/**
 * Check if pointers are equal
 * @ingroup Misc
 */
inline bool areEqP( const void *p1, const void *p2)
{
    return p1 == p2;
}

/**
 * Check if pointers are not equal
 * @ingroup Misc
 */
inline bool areNotEqP( const void *p1, const void *p2)
{
    return p1 != p2;
}



// 16 byte Address Alignment and count multiple of 4
#define rmIsAlignedAddr(a)    ( ((long)a & 15L) == 0 )
#define rmIsAlignedCount(n)   ( (n > 0) && ((n & 3L) == 0) )


#define rmBytesInGig 1000000000

#define SetTrait(nspace,trait,type,val)\
namespace nspace { \
template <> \
struct trait < type > { \
public: \
static const bool value = val; \
}; }






#   define rmStaticConstMacro(name,type,value) enum { name = value }

#   define rmGetStaticConstMacro(name) (Self::name)

#define rmWarningMacro(x) \
{\
cerr << "WARNING: In " __FILE__ ", line " << __LINE__ << "\n" \
x << "\n\n"; \
}

/** Get built-in type.  Creates member Get"name"() (e.g., GetVisibility());
 * This is the "const" form of the itkGetMacro.  It should be used unless
 * the member can be changed through the "Get" access routine. */
#define rmGetConstMacro(name,type) \
virtual type Get##name () const \
{ \
rmWarningMacro("returning " << #name " of " << this->m##name ); \
return this->m##name; \
}



/** Set built-in type.  Creates member Set"name"() (e.g., SetVisibility()); */
#define rmSetMacro(name,type) \
virtual void Set##name (const type _arg) \
{ \
if (this->m##name != _arg) \
{ \
this->m##name = _arg; \
} \
}

/** Get built-in type.  Creates member Get"name"() (e.g., GetVisibility()); */

#define rmGetMacro(name,type) \
virtual type Get##name () \
{ \
return this->m_##name; \
}


/** Set character string.  Creates member Set"name"()
 * (e.g., SetFilename(char *)). The macro assumes that
 * the class member (name) is declared a type std::string.
 */

#define rmSetStringMacro(name) \
virtual void Set##name (const char* _arg) \
{ \
if ( _arg && (_arg == this->m##name) ) { return;} \
if (_arg) \
{ \
this->m##name = _arg;\
} \
else \
{ \
this->m##name = ""; \
} \
}


/** Get character string.  Creates member Get"name"()
 * (e.g., SetFilename(char *)). The macro assumes that
 * the class member (name) is declared a type std::string.
 */

#define rmGetStringMacro(name) \
const char* Get##name () const		\
{ \
return this->m##name.c_str(); \
}



// Use this instead of incorrectly names rcFoo

#define rmExceptionMacro(x) \
{ \
ostringstream message; \
message << "Reify Class Libray: " x; \
throw general_exception(__FILE__, __LINE__, message.str().c_str()); \
}


#define rmGplot(a,b,c)\
{    GnuPlotInterface gpi;			\
gpi.Plot2DFct ((a), \
0.0, (double) ((a).width() -1), (uint32) (a).width(),\
0.0, (double) ((a).height() -1), (uint32) (a).height());\
}



#endif
