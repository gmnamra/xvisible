// Copyright 2002 Reify, Inc.

#ifndef _rcUNITTEST_H_
#define _rcUNITTEST_H_

#include <stdio.h>
#include "rc_types.h"
#include "rc_math.h"
#include "rc_timestamp.h"

#define UT_SOURCELINE() (__FILE__, __LINE__)

#define rcUNITTEST_ASSERT(condition)  \
    ( rcUnitTest::Assert( (condition), \
    (#condition),       \
    __FILE__,           \
    __LINE__ ) )

#define rcUTCheck(condition)  \
    ( rcUnitTest::Assert( (condition), \
    (#condition),       \
    __FILE__,           \
    __LINE__ ) )

#define rcUTCheckEqual(A, B)				\
  ( rcUnitTest::Assert(((A) == (B)),			\
    (#A),       \
    __FILE__,           \
    __LINE__ ) )

#define rcUTCheckRealEq(A, B)				\
  ( rcUnitTest::Assert( (rfRealEq((A), (B))),		\
    (#A),       \
    __FILE__,           \
    __LINE__ ) )

#define rcUTCheckRealDelta(A, B, C)				\
  ( rcUnitTest::Assert( (rfRealEq((A), (B), (C))),		\
    (#A),       \
    __FILE__,           \
    __LINE__ ) )

// Unit test base class

class rcUnitTest {
public:
    rcUnitTest() : mErrors( 0 ) { };
    // Virtual dtor is required
    virtual ~rcUnitTest() { };

    // Run tests
    virtual uint32 run() = 0;

    // Print success/failure message
    void printSuccessMessage( const char* msg, uint32 errors ) {}
#if 0
   {
        if ( errors > 0 ) {
            fprintf( stderr, "%-38s Failed, %d errors\n", msg, errors );
        }
        else {
            fprintf( stderr, "%-38s OK\n", msg );
        }
    }
#endif
    // Assert which accumulates the error count
    uint32 Assert( uint32 expr, const char* exprString, const char* file, int lineNumber ) {
        if ( ! expr ) {
            ++mErrors;
            fprintf( stderr, "Assertion %s failed in %s at line %i\n", 
                     exprString, file, lineNumber );
        }

        return expr;
    }

    void randomSeed ()
    {
      // Set a seed from two clocks
      uint32 seed;
      rcTimestamp now = rcTimestamp::now();
      uint32 secs = uint32(now.secs());
      seed = uint32( (now.secs() - secs) / getTimestampResolution() + clock() );
      srandom (seed);
    }
    
    // Construct a temporary file name
    static std::string makeTmpName(const char *pathFormat, const char* baseName )
    {
      char buf[2048];
      static const char *defaultFormat = "/tmp/um%d_%s";

      // Use current time in seconds as a pseudo-unique prefix
      double secs = rcTimestamp::now().secs();
      snprintf( buf, rmDim(buf), pathFormat ? pathFormat : defaultFormat, 
		uint32(secs), baseName );
      return std::string( buf );
    }

    // Construct a temporary file name
    static std::string makeTmpName(const char* baseName )
    {
      return makeTmpName (0, baseName);
    }
      
protected:
    uint32   mErrors;
    std::string   mFileName;
};

#define utClassDeclare(x) \
class UT_##x: public rcUnitTest {\
public:\
    UT_##x ();\
    ~UT_##x ();\
    virtual uint32 run();\
private:\
    void test ();\
};


#define utClassDefine(x)\
  UT_##x ::UT_##x ()\
{}\
uint32 UT_##x ::run ()\
{\
  test();\
  return mErrors;\
}\
UT_##x ::~UT_##x ()\
{\
  printSuccessMessage( #x, mErrors );\
}




#endif // _rcUNITTEST_H_
