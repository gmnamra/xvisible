// Copyright (c) 2002-2004 Reify Corp. All rights reserved.

#include <strstream>
#include <unistd.h>

#include <rc_timestamp.h>

#include "ut_timestamp.h"

UT_Timestamp::UT_Timestamp()
{
}

UT_Timestamp::~UT_Timestamp()
{

}

uint32 UT_Timestamp::run()
{
    testBasic();

    
    return mErrors;  
}

// Basic timestamp tests
void UT_Timestamp::testBasic()
{
    // Static method tests
    {
        int64 now = getCurrentTimestamp();
        rcUNITTEST_ASSERT( now > 0 );

        double nowSeconds = convertTimestampToSeconds(now);
        rcUNITTEST_ASSERT( nowSeconds > 0.0 );

        int64 now2 = convertSecondsToTimestamp(nowSeconds);
        int64 diff = rmABS(now - now2);
        
        rcUNITTEST_ASSERT( diff < 2 );
        if ( !(diff < 2) ) {
            cerr << "\tnow : " << now << endl
                 << "\tnow2: " << now2 << endl;
        }
        
        // String conversion tests
        // Timestamp is stored as two separate 32-bit uints
        {
            rcTimestamp nowStamp = rcTimestamp::now();
            strstream string;
            string << nowStamp << ends;
            rcTimestamp result;
            
            rcUNITTEST_ASSERT( result != nowStamp );
            rcUNITTEST_ASSERT( convertStringToTimestamp( result, string.str() ) );
            rcUNITTEST_ASSERT( result == nowStamp );
            rcUNITTEST_ASSERT( result.secs() == nowStamp.secs() );
            string.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
        }
        {
#ifdef __ppc__			
            std::string seconds( "0 5000000" ); // with uSecond resolution this is 5 seconds
#else
            std::string seconds( "5000000 0" ); // with uSecond resolution this is 5 seconds
#endif
            strstream string;
            rcTimestamp result;
            
            rcUNITTEST_ASSERT( convertStringToTimestamp( result, seconds ) );
			rcUTCheck(real_equal(result.secs(), 5.0 ));
            string << result << ends;
            rcUNITTEST_ASSERT( seconds == string.str() );
            string.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
        }
        {
#ifdef __ppc__				
            std::string seconds( "1 0" ); // with uSecond resolution 32-bit uint max value
                                       // is 4294.967295 seconds
#else
            std::string seconds( "0 1" ); 
#endif
            strstream string;
            rcTimestamp result;

            rcUNITTEST_ASSERT( convertStringToTimestamp( result, seconds ) );
			rcUTCheck(real_equal(result.secs(), 4294.967296));
            string << result << ends;
            rcUNITTEST_ASSERT( seconds == string.str() );
            string.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
        }
        {
            rcTimestamp result;
            
            // Conversion failure tests
            rcUNITTEST_ASSERT( !convertStringToTimestamp( result, "fail!" ) );
            rcUNITTEST_ASSERT( !convertStringToTimestamp( result, "666" ) );
        }
        
        double resolution = getTimestampResolution();
        rcUNITTEST_ASSERT( resolution > 0.0 );
        
        // sleep to guarantee a new tick
        sleep( 1 );
        int64 then = getCurrentTimestamp();
        rcUNITTEST_ASSERT( now != then );

        rcTimestamp stamp = rcTimestamp::now();
        rcUNITTEST_ASSERT( stamp.secs() > 0.0 );

        // Test special const
        rcUNITTEST_ASSERT( cCursorTimeCurrent != 0.0 );

        // Test localtime
        std::string localTime = stamp.localtime();
        rcUNITTEST_ASSERT( localTime.size() > 0 );
    }

    // Constructor tests
    {
        // Default ctor
        {
            rcTimestamp now;
            rcUNITTEST_ASSERT( now == rcTimestamp( 0.0 ) );
            rcUNITTEST_ASSERT( now == cZeroTime );
        }

        // ctor
        {
            double secs = 6.0;
            rcTimestamp now( secs );

            rcUNITTEST_ASSERT( now.secs() == secs );
        }
        
        // copy ctor
        {
            double secs = 6.0;
            rcTimestamp now( secs );
            rcTimestamp now2( now );
            
            rcUNITTEST_ASSERT( now == now2 );
        }

        // operator tests
        {
            {
                // operator +
                double secs = 6.0;
                double secs2 = 5.0;
                rcTimestamp now( secs );
                rcTimestamp now2( secs2 );
                rcUNITTEST_ASSERT( now != now2 );

                rcTimestamp now1plus2 = now + now2;
                rcUNITTEST_ASSERT( now1plus2.secs() == (secs + secs2) );
                rcUNITTEST_ASSERT( now1plus2 == now2 + now );

                // operator +=
                now1plus2 = now;
                now1plus2 += now2;
                rcUNITTEST_ASSERT( now1plus2.secs() == (secs + secs2) );

                // operator -
                rcTimestamp now1minus2 = now - now2;
                rcUNITTEST_ASSERT( now1minus2.secs() == (secs - secs2) );

                // operator -=
                now1minus2 = now;
                now1minus2 -= now2;
                rcUNITTEST_ASSERT( now1minus2.secs() == (secs - secs2) );

                // Rounding tests
                for ( double inc = 0.066; inc < 0.080; inc += 0.0001 ) 
                    for ( int n = 0; n < 100; n++ )
                        additionTest( n, inc );

            }
            // Comparison operators
            {
                // operator == and !=
                double secs = 6.0;
                double secs2 = 5.0;

                rcTimestamp now( secs );
                rcTimestamp now1( secs );

                rcUNITTEST_ASSERT( now == now1 );
                rcUNITTEST_ASSERT( !(now != now1) );

                rcTimestamp now2( secs2 );
                rcUNITTEST_ASSERT( now != now2 );
                rcUNITTEST_ASSERT( !(now == now2) );

                // operator > and >=
                rcUNITTEST_ASSERT( now > now2 );
                rcUNITTEST_ASSERT( now >= now2 );
                rcUNITTEST_ASSERT( now >= now );
                
                // operator < and <=
                rcUNITTEST_ASSERT( now2 < now );
                rcUNITTEST_ASSERT( now2 <= now );
                rcUNITTEST_ASSERT( now <= now );
            }
            // Unary operators
            {
                // Unary -
                double secs = 6.0;
                rcTimestamp now( secs );
                rcUNITTEST_ASSERT( now > -now );
                rcUNITTEST_ASSERT( -now.secs() == -secs );
            }
#ifdef notyet            
            // Monotonic increase test
            {
                rcTimestamp t1, t2;
                
                t2 = rcTimestamp::now();
                
                for ( uint32 i = 0; i < 5000000; ++i ) {
                    t1 = rcTimestamp::now();
                    rcTimestamp diff = t1 - t2;
                    rcUNITTEST_ASSERT( diff.secs() >= 0 );
                    if (diff.secs() < 0) { 
                        fprintf(stderr, "rcTimestamp jump: %.0lf usec\n", diff.secs()*1000000.0); 
                    } 
                    t2 = t1; 
                }
            }
#endif            
        }
    }
    printSuccessMessage( "rcTimestamp test", mErrors );
}

// Test timestamp addition vs. multiplication
void UT_Timestamp::additionTest( int n, double inc )
{
    rcTimestamp increment( inc );
    rcTimestamp addition( 0.0 );
    rcTimestamp multiplication( inc * n );

    for ( int i = 0; i < n; i++ ) {
        addition += increment;
    }
    rcUNITTEST_ASSERT( addition == multiplication );
    
    if ( addition != multiplication ) {
        cerr << n << "x" << inc << " multiplication result " << multiplication;
        cerr << " differs from addition result " << addition << endl;
    } 
}

// Speed calculator tests
void UT_Timestamp::testFps()
{
    uint32 oldErrors = mErrors;

    // 1 fps
    testFpsInterval( 1.0 );
    // 1.5 fps
    testFpsInterval( 0.75 );
    // 2 fps
    testFpsInterval( 0.5 );
    // 3.75 fps
    testFpsInterval( 0.2666666667 );
    // 5 fps
    testFpsInterval( 0.2 );
    // 7.5 fps
    testFpsInterval( 0.1333333333 );
    // 10 fps
    testFpsInterval( 0.1 );
    // 15 fps
    testFpsInterval( 0.0666666667 );
    // 30 fps
    testFpsInterval( 0.0333333333 );
    // 60 fps
    testFpsInterval( 0.016666667 );

    printSuccessMessage( "rcFpsCalculator test", mErrors - oldErrors );
}
    
// Speed calculator tests with a specific frame interval
void UT_Timestamp::testFpsInterval( double frameInterval )
{
    // Min window size should be at least 50% of frame rate
    int32 minWindowSize = static_cast<uint32>((1.0/frameInterval) / 2);
    if ( minWindowSize < 1 )
        minWindowSize = 1;
    const int32 maxWindowSize = minWindowSize * 4;
    // Expected speed
    const double expectedFps = 1.0 / frameInterval;
    // Minimum required accuracy
    const double epsilon = 0.0005;
            
    for ( int32 i = minWindowSize; i <= maxWindowSize; i++ ) {
        // Fill the buffer with different amiunts of time stamps
        int32 minOverflow = -i/2;
        if ( i < 3 )
            minOverflow = 1;
        for ( int32 overflow = minOverflow; overflow < maxWindowSize; overflow++ ) {
            rcFpsCalculator calc( i );
            // Fill calculator
            for ( int32 x = 1; x <= i+overflow; x++ ) {
                rcTimestamp t = frameInterval * x;
                calc.addTime( t );
            }

            const double fps = calc.fps();
            const double diff = rmABS (expectedFps - fps);
            
            rcUNITTEST_ASSERT( diff < epsilon );
            if ( diff >= epsilon )
            {
                cerr << "window size " << i << " items " << (i+overflow) << " interval " << frameInterval*1000 << " ms: expected " << expectedFps;
                cerr << ", got " <<  fps << " diff " << diff << endl;
            }
        }
    }
}
