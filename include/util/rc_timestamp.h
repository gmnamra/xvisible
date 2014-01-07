/******************************************************************************
*	rc_timestamp.h
*
*	This file contains the declarations for a timestamp class,
*	which represents time values with microsecond accuracy.
*	
******************************************************************************/

#ifndef _rcBASE_RCTIMESTAMP_H_
#define _rcBASE_RCTIMESTAMP_H_

#include <rc_types.h>
#include <iostream>
#include <deque>

class rcMovieFileFormat;
class rcMovieFileFormat2;
class rcEngineMovieSaver;
class rcVideoCache;
class rcGenMovieFile;


using namespace std;


/******************************************************************************
*	Platform-specific timestamp support routines
******************************************************************************/

// get the current timestamp
int64 getCurrentTimestamp( void );

// get timestamp resolution (in seconds)
double getTimestampResolution( void );

/******************************************************************************
*	Conversion routines
******************************************************************************/

// convert timestamp to seconds.
inline double convertTimestampToSeconds( int64 timestamp )
{
	return (double) (timestamp * getTimestampResolution());
}

// convert timestamp to seconds.
inline int64 convertSecondsToTimestamp( double secs )
{
	return (int64) (secs / getTimestampResolution());
}

/******************************************************************************
*	rcTimestamp class definition
*
*	The rcTimestamp class represents wall-clock time with microsecond
*	accuracy.  It is intended to be used with "value" semantics.
******************************************************************************/

class RFY_API rcTimestamp
{
public:
	// default constructor (time = 0)
	rcTimestamp( void ) : _timestamp (0) {}

	// copy constructor
	rcTimestamp( const rcTimestamp& other )
	{
		_timestamp = other._timestamp;
	}

	// constructor 
	rcTimestamp( double secs ) :  _timestamp (convertSecondsToTimestamp( secs )) {}

	// internal constructor
	rcTimestamp( int64 timestamp ) : _timestamp (timestamp) {}
        
	// static method to return current timestamp
	static rcTimestamp now( void ) {  return (rcTimestamp( getCurrentTimestamp() ) ); }

	// convert to double-precision seconds
	double secs( void ) const
	{
		return convertTimestampToSeconds( _timestamp );
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

	// not equals operator
	bool operator != ( const rcTimestamp& other ) const
	{
		return _timestamp != other._timestamp;
	}

	// less than operator
	bool operator < ( const rcTimestamp& other ) const
	{
		return _timestamp < other._timestamp;
	}

	// less than or equals operator
	bool operator <= ( const rcTimestamp& other ) const
	{
		return _timestamp <= other._timestamp;
	}

	// greater than operator
	bool operator > ( const rcTimestamp& other ) const
	{
		return _timestamp > other._timestamp;
	}

	// greater than or equals operator
	bool operator >= ( const rcTimestamp& other ) const
	{
		return _timestamp >= other._timestamp;
	}

	// unary negation operator
	rcTimestamp operator - ( void ) const
	{
		return rcTimestamp( -_timestamp );
	}

    // convert to local time string
    std::string localtime() const;
    
private:
    friend class rcMovieFileFormat;
    friend class rcMovieFileFormat2;
	friend class rcEngineMovieSaver;
	friend class rcVideoCache;
	friend class rcGenMovieFile;
	friend bool rfCreateReifyMovie(rcVideoCache* cache, char* destFile,
				       uint32 firstIndex, uint32 frameCount,
				       uint32 offset, uint32 period,
                                       uint32 newFormat);

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

// addition operator
rcTimestamp operator + ( const rcTimestamp& ts1 , const rcTimestamp& ts2 );

// subtraction operator
rcTimestamp operator - ( const rcTimestamp& ts1 , const rcTimestamp& ts2 );

/******************************************************************************
*	Constants
******************************************************************************/

const rcTimestamp cCursorTimeCurrent = -1.0;

/* Little helper class.
 */
class RFY_API rcTimestampPair
{
 public:
  rcTimestampPair (const rcTimestamp& first, const rcTimestamp& second) 
    : _first(first), _second(second) {}

  rcTimestamp first() { return _first; }
  rcTimestamp second() { return _second; }

 private:
  rcTimestamp _first;
  rcTimestamp _second;
};

//
// Class for calculating a sliding average FPS (frames per second) speed
//

class rcFpsCalculator {
  public:
    // Ctor
    rcFpsCalculator( uint32 size ) :  mLastTime( cZeroTime ), mWindowSize( size ), mTotalTime( -1.0 ) {
        rmAssert( mWindowSize > 0 ); };

    // Accessor: get current speed
    double fps() const {
        if ( mTotalTime > 0.0 ) {
            if ( !mFrameIntervals.empty() ) {
                return 1.0 / (mTotalTime / (mFrameIntervals.size()));
            }
        }
         
        return 0.0;
    }
    
    // Mutator: add new time
    void addTime( const rcTimestamp& time ) {
        if ( mTotalTime >= 0.0 ) {
            // Add interval
            rcTimestamp newInterval = time - mLastTime;
            addInterval( newInterval.secs() );
        } else {
            // First time ever, just store the time
            mTotalTime = 0.0;
        }
        mLastTime = time;
    }

    // Mutator: reset queue
    void reset() {
        mLastTime = cZeroTime;
        mTotalTime = 0.0;
        mFrameIntervals.clear();
    };
    
  private:
    // Mutator: add new interval
    void addInterval( double frameInterval ) {
        // Check for window overflow
        if ( mFrameIntervals.size() > mWindowSize ) {
            // Remove first interval
            mTotalTime -= mFrameIntervals.front();
            mFrameIntervals.pop_front();
        } 
        // Add new interval
        mFrameIntervals.push_back( frameInterval );
        mTotalTime += frameInterval;
    }
    
    rcTimestamp   mLastTime;       // Timestamp of last update
    uint32      mWindowSize;     // Size of sliding window
    double        mTotalTime;      // Total of all frame intervals
    deque<double> mFrameIntervals; // Queue of frame intervals
};

#endif // _rcBASE_RCTIMESTAMP_H_
