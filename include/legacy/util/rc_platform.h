// Copyright 2002 Reify, Inc.

#ifndef _rcPLATFORM_H_
#define _rcPLATFORM_H_

// Platform-specific definitions

// @note preferred way of including platform specific headers

#if defined (__ppc__)
#include "architecture/ppc/alignment.h"
#else
#include "architecture/i386/alignment.h"
#endif

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
typedef int64 rcUint64;
#elif defined(__APPLE__)
#include <sys/types.h>
typedef quad_t int64;
typedef int64 rcUInt64;
#endif

// See http://en.wikipedia.org/wiki/Byte-order_mark

// Platform byte order
enum reByteOrder {
    eByteOrderUnknown = 0,
    eByteOrderBigEndian,
    eByteOrderLittleEndian
};

#define rcBOM_BE 0xFEFF     // Byte order mark big-endian
#define rcBOM_LE 0xFFFE     // Byte order mark little-endian


#endif // _rcPLATFORM_H_
