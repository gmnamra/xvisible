/******************************************************************************
*	timestamp.cpp
*
*	Non-platform-specific support for timestamps.
******************************************************************************/

#include <exception>

#include <rc_timestamp.h>

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
