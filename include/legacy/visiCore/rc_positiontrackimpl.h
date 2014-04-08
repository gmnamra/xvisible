/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*
*	$Id: rc_positiontrackimpl.h 4391 2006-05-02 18:40:03Z armanmg $
*
*	This file contains the declaration for the implementation of
*	the rcPositionTrack interface.
******************************************************************************/

#ifndef rcPOSITIONTRACKIMPL_H
#define rcPOSITIONTRACKIMPL_H

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

class rcPositionSegmentIteratorImpl;

class rcPositionSegment : public rcSegmentImpl
{
  public:
	rcPositionSegment( void ) :
    rcSegmentImpl( cZeroTime, cZeroTime, rcRect( 0, 0, 0, 0 ) ), mValue( rcFPair(0.0, 0.0) )
	{
	}

	rcPositionSegment( const rcTimestamp& start, const rcTimestamp& length , const rcRect& focus, const rcFPair& value ) :
    rcSegmentImpl( start, length, focus ), mValue( value )
	{
#ifdef DEBUG_LOG        
        cerr << "rcPositionSegment( " << start << ", " << length << ", " << value << " )" << endl;
#endif        
	}

	rcFPair mValue;
};

class rcPositionTrackImpl : public rcTrackImpl, public rcPositionTrack, public rcPositionWriter
{
public:

	rcPositionTrackImpl( const char* tag , const char* name , const char* description,
                         const char* displayFormat,
                         uint32 sizeLimit );

    // virtual dtor is required
    virtual ~rcPositionTrackImpl() { };

    //////////////////// rcTrackImpl implementation ////////////////////
    // Number of segments
    virtual uint32 segments();
    // Segment at index
    virtual rcSegmentImpl segment( uint32 index );
    // Pop oldest segment
    virtual void popSegment();

	//////////////////// rcPositionTrack implementation ////////////////////

    // get the minimum value (hint) the engine expected to
	// generate for this track.
	virtual rcFPair getExpectedMinimumValue( void );

	// get the maximum value (hint) the engine expected to
	// generate for this track.
	virtual rcFPair getExpectedMaximumValue( void );

	// get the current minimum value written to this track.
	virtual rcFPair getCurrentMinimumValue( void );

	// get the current maximum value written to this track.
	virtual rcFPair getCurrentMaximumValue( void );
    
	// get an iterator for the track initialized to the
	//	specified position.  It is the responsibility of
	//	the caller to release the iterator
	virtual rcPositionSegmentIterator* getDataSegmentIterator( const rcTimestamp& pos );

    // get the total number of data segments.
    virtual uint32 getSegmentCount( void );
    
	//////////////////// rcPositionWriter implementation ////////////////////

    // set the expected minimum value for this 2D data
	virtual void setExpectedMinValue( const rcFPair& minValue );

	// set the expected maximum value for this 2D data
	virtual void setExpectedMaxValue( const rcFPair& maxValue );
    
	// write a new scalar value (int, float, boolean) with timestamp.
	virtual void writeValue( const rcTimestamp& timestamp, const rcRect& focus, const rcFPair& value );

    // flush and data that might be cached
    virtual void flush( void );

    // Down cast to rcWriter*
    virtual rcWriter* down_cast();
    
	friend class rcPositionSegmentIteratorImpl;
    
private:
     // Update writer state
    void setWriterState( const rcTimestamp& time, const rcRect& focus, const rcFPair& value );
    // Update observed min/max values
    void setMinMaxValues( const rcFPair& value );
    // Add new segment
    void addSegment( const rcTimestamp& segmentLength );
    
	deque<rcPositionSegment>	_segments;
    rcFPair			            _expectedMinimumValue;
	rcFPair			            _expectedMaximumValue;
	rcFPair			            _currentMinimumValue;
	rcFPair                     _currentMaximumValue;
    rcFPair	     		        _currentValue;
};

#endif // rcPOSITIONTRACKIMPL_H
