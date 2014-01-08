// Copyright 2002 Reify, Inc.

#ifndef __rcTYPES_H__
#define __rcTYPES_H__

#include <string>
#include <assert.h>
#include <rc_platform.h>

// Integer types
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef uint64_t        uint64;
typedef char			int8;
typedef signed short    int16;
typedef signed int      int32;
typedef int64_t         int64;

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


#endif
