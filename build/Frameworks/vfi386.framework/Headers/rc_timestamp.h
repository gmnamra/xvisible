/******************************************************************************
*	rc_timestamp.h
*
*	This file contains the declarations for a timestamp class,
*	which represents time values with microsecond accuracy.
*	
******************************************************************************/

#ifndef _rcBASE_RCTIMESTAMP_H_
#define _rcBASE_RCTIMESTAMP_H_

#include "rc_types.h"

#include "rc_types.h"
#include "rc_framework_core.hpp"
#include <iostream>

#include <iostream>
#include <deque>


class RFY_API rcTimestamp : public boost::totally_ordered<rcTimestamp>, public boost::additive<rcTimestamp>
{
public:
	// default constructor (time = 0)
	rcTimestamp( void ) : _timestamp (0) {}

	// copy constructor
	rcTimestamp( const rcTimestamp& other )
	{
		_timestamp = other._timestamp;
	}

    static double get_time_resolution ()
    {
        return (double) 1.0 / visible_framework_core::instance().ticks_per_second();
    }
    

    static rcTimestamp from_seconds ( double dd )
    {
        return rcTimestamp::from_tick_type ( dd * visible_framework_core::instance().ticks_per_second() );
    }

	static rcTimestamp from_tick_type ( int64 timestamp )
    {
        rcTimestamp ts; ts._timestamp = timestamp; 
        return ts;
    }
    
    int64 tick_type_value () const 
    {
        return _timestamp;        
    }
        
	// static method to return current timestamp
	static rcTimestamp now( void )
    {  
        return rcTimestamp::from_tick_type ( visible_framework_core::instance().get_ticks () );
    } 

	// convert to double-precision seconds
	double secs( void ) const
	{
		return (double) _timestamp / visible_framework_core::instance().ticks_per_second();
	}

	// plus-equal operator
	rcTimestamp& operator += ( const rcTimestamp& other )
	{
		_timestamp += other._timestamp;
		return *this;
	}

	// minus-equal operator
	rcTimestamp& operator -= ( const rcTimestamp& other )
	{
		_timestamp -= other._timestamp;
		return *this;
	}

	// equals operator
	bool operator == ( const rcTimestamp& other ) const
	{
		return _timestamp == other._timestamp;
	}


	// less than operator
	bool operator < ( const rcTimestamp& other ) const
	{
		return _timestamp < other._timestamp;
	}

	// unary negation operator
	rcTimestamp operator - ( void ) const
	{
		return rcTimestamp::from_tick_type ( -_timestamp );
	}
    
    // interval helper
    void bi_compare (const rcTimestamp& before, const rcTimestamp& after, rcTimestamp& match) const
    {
        auto ag = after._timestamp - _timestamp;
        auto bg = _timestamp - before._timestamp;
        match = (ag < bg) ? after : before;
    }
    
    // convert to local time string
    std::string localtime() const;
    
private:
#if 0    
    friend class rcMovieFileFormat;
    friend class rcMovieFileFormat2;
	friend class rcEngineMovieSaver;
	friend class rcVideoCache;
	friend class rcGenMovieFile;
	friend bool rfCreateReifyMovie(rcVideoCache* cache, char* destFile, uint32 firstIndex, uint32 frameCount, uint32 offset, uint32 period, uint32 newFormat);
#endif
    
	// platform-specific information should fit in a 64bit word
	int64		_timestamp;
};

// Time for 0.0
// It is much faster to do "rcTimestamp foo = cZeroTime" than "rcTimestamp foo = 0.0"
extern const rcTimestamp cZeroTime;

/******************************************************************************
*	Persistence routines
******************************************************************************/

// convert string to timestamp, return true on success
bool convertStringToTimestamp( rcTimestamp& result, const std::string& str );

/******************************************************************************
*	Operators
******************************************************************************/

// insertion operator
ostream& operator << ( ostream& os, const rcTimestamp& timestamp );


#endif // _rcBASE_RCTIMESTAMP_H_
