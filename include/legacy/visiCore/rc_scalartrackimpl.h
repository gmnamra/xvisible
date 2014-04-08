/******************************************************************************
*	rc_scalartrackimpl.h
*
*	This file contains the declaration for the implementation of
*	the rcScalarTrack interface.
******************************************************************************/

#ifndef rcSCALARTRACKIMPL_H
#define rcSCALARTRACKIMPL_H

#include <string>
#include <deque>

#if WIN32
using namespace std;
#endif

#include <rc_setting.h>
#include <rc_engine.h>
#include <rc_model.h>

#include "rc_trackimpl.h"
#include "rc_segmentimpl.h"

class rcScalarSegmentIteratorImpl;

class rcScalarSegment : public rcSegmentImpl
{
  public:
	rcScalarSegment( void ) :
    rcSegmentImpl( cZeroTime, cZeroTime, rcRect( 0, 0, 0, 0 ) ), mValue( 0.0 )
	{
	}

	rcScalarSegment( rcTimestamp start, rcTimestamp length , rcRect focus, double value ) :
    rcSegmentImpl( start, length, focus ), mValue( value )
	{
#ifdef DEBUG_LOG        
        cerr << "rcScalarSegment( " << start << ", " << length << ", " << value << " )" << endl;
#endif        
	}

	double mValue;
};

class rcScalarTrackImpl : public rcTrackImpl, public rcScalarTrack, public rcScalarWriter
{
public:

	rcScalarTrackImpl( const char* tag , const char* name , const char* description,
                       const char* displayFormat,
                       uint32 sizeLimit );

    // virtual dtor is required
    virtual ~rcScalarTrackImpl() { };

    //////////////////// rcTrackImpl implementation ////////////////////
    // Number of segments
    virtual uint32 segments();
    // Segment at index
    virtual rcSegmentImpl segment( uint32 index );
    // Pop oldest segment
    virtual void popSegment();

	//////////////////// rcScalarTrack implementation ////////////////////

	// get the minimum value (hint) the engine expected to
	//	generate for this track.
	virtual double getExpectedMinimumValue( void );

	// get the maximum value (hint) the engine expected to
	//	generate for this track.
	virtual double getExpectedMaximumValue( void );

	// get the current minimum value written to this track.
	virtual double getCurrentMinimumValue( void );

	// get the current maximum value written to this track.
	virtual double getCurrentMaximumValue( void );

	// get an iterator for the track initialized to the
	//	specified position.  It is the responsibility of
	//	the caller to release the iterator
	virtual rcScalarSegmentIterator* getDataSegmentIterator( const rcTimestamp& pos );

    // get the total number of data segments.
    virtual uint32 getSegmentCount( void );
    
	//////////////////// rcScalarWriter implementation ////////////////////

	// set the expected minimum value for this scalar data
	virtual void setExpectedMinValue( const double& minValue );

	// set the expected maximum value for this scalar data
	virtual void setExpectedMaxValue( const double& maxValue );

	// write a new scalar value (int, float, boolean) with timestamp.
	virtual void writeValue( const rcTimestamp& timestamp, const rcRect& focus, const double& value );

    // flush and data that might be cached
    virtual void flush( void );

    // Remove all segments, reset start time
    virtual void reset();

    // Down cast to rcWriter*
    virtual rcWriter* down_cast();
    
	friend class rcScalarSegmentIteratorImpl;
    
private:
    // Update observed min/max values
    void setMinMaxValues( double value );
    // Update writer state
    void setWriterState( const rcTimestamp& time, rcRect focus, double value );
    // Add new segment
    void addSegment( const rcTimestamp& segmentLength );
    
	double			_expectedMinimumValue;
	double			_expectedMaximumValue;
	double			_currentMinimumValue;
	double			_currentMaximumValue;
	deque<rcScalarSegment>	_segments;
    double			_currentValue;
};

#endif // rcSCALARTRACKIMPL_H
