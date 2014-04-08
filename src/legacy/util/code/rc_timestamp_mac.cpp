/******************************************************************************
*	rc_timestamp_mac.cpp
*
*	Macintosh-specific support for timestamps.
******************************************************************************/

#include <rc_exception.h>

#include <sys/time.h>

#include <rc_timestamp.h>

static const int64 cMicrosPerSec = 1000000;
static const double cResolution = 1.0/cMicrosPerSec;

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

    int64 now = t.tv_sec * cMicrosPerSec + t.tv_usec;
    return now;
}

// get the timestamp resolution
double getTimestampResolution( void )
{
    return cResolution;
}
