/******************************************************************************
*	timestamp.cpp
*
*	Non-platform-specific support for timestamps.
******************************************************************************/

#include <rc_exception.h>

#include "rc_timestamp.h"
#include <sys/time.h>




// get the current timestamp
int64 getCurrentTimestamp( void )
{
    // TODO: use high resolution clock
    struct timeval t;
#if 1
    // Warning: gettimeofday values do not always monotonically increase
    if ( gettimeofday( &t, NULL ) != 0 )
        throw general_exception( "timestamp not available" );
#else
    // Hack from Apple technical support
    // TODO: validate that this works, it seems that sometimes 0 is returned in t
#include <sys/syscall.h>
    
    if ( syscall(SYS_gettimeofday, &t, NULL) < 0 )
        throw general_exception( "timestamp not available" );
#endif
  
    int64 now = t.tv_sec * 1000000 + t.tv_usec;
    return now;
}

// get the timestamp resolution
double getTimestampResolution( void )
{
    static const double cResolution = 1.0/1000000.0;    
    return cResolution;
}


// convert timestamp to seconds.
double convertTimestampToSeconds( int64 timestamp )
{

	return (double) (timestamp / 1000000); // ts * ( 1 / mMicroSeconds )
}

// convert timestamp to seconds.
int64 convertSecondsToTimestamp( double secs )
{
	return (int64) (secs * 1000000 ); // secs / ( 1 / mMicroSeconds )
}


// Timestamp for 0.0.
// It is much faster to do "rcTimestamp foo = cZeroTime" than "rcTimestamp foo = 0.0"
const rcTimestamp cZeroTime = rcTimestamp();

// convert string to timestamp, return true on success
bool convertStringToTimestamp( rcTimestamp& result, const std::string& str )
{
    rmAssertDebug( sizeof( rcTimestamp ) == (sizeof( uint32 )*2) );
    
    // Timestamp is stored in two 32-bit components
    uint32 one, two;

    if ( sscanf( str.c_str() , "%u %u" , &one, &two ) != 2 ) {
        result = cZeroTime;
        return false;
    }

    uint32* component = (uint32*)&result;
    component[0] = one;
    component[1] = two;
    return true;
}

#include <strstream>

// insertion operator
ostream& operator << ( ostream& os, const rcTimestamp& timestamp )
{
    rmAssertDebug( sizeof( rcTimestamp ) == (sizeof( uint32 )*2) );
    
    const uint32* component = (uint32*)&timestamp;
    
    // Output as two separate 32-bit components
    os << component[0] << " " << component[1];

	return os;
}

// binary addition operator
rcTimestamp operator + ( const rcTimestamp& ts1 , const rcTimestamp& ts2 )
{
	rcTimestamp retVal = ts1;
	retVal += ts2;
	return retVal;
}

// binary subtraction operator
rcTimestamp operator - ( const rcTimestamp& ts1 , const rcTimestamp& ts2 )
{
	rcTimestamp retVal = ts1;
	retVal -= ts2;
	return retVal;
}

// convert to local time string
std::string rcTimestamp::localtime() const
{
    std::string localTime;
    
    time_t seconds = static_cast<time_t>(secs() + 0.5);
    char buf[64];
    
    if ( ctime_r( &seconds, buf ) ) {
        localTime = buf;
        // Remove trailing NL
        if ( !localTime.empty() )
            if ( localTime[localTime.size()-1] == '\n' )
                localTime[localTime.size()-1] = ' ';
    }

    return localTime;
}




