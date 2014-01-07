/******************************************************************************
*	$Id: rc_graphicstrackimpl.h 4391 2006-05-02 18:40:03Z armanmg $
*
*	This file contains the declaration for the implementation of
*	the rcGraphicsTrack interface.
******************************************************************************/

#ifndef _rcGRAPHICSTRACKIMPL_H_
#define _rcGRAPHICSTRACKIMPL_H_

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

class rcGraphicsSegmentIteratorImpl;

class rcGraphicsSegment : public rcSegmentImpl
{
  public:
	rcGraphicsSegment( void ) :
    rcSegmentImpl( cZeroTime, cZeroTime, rcRect( 0, 0, 0, 0 ) ), mValue( rcVisualGraphicsCollection() )
	{
	}

	rcGraphicsSegment( rcTimestamp start, rcTimestamp length , rcRect focus, const rcVisualGraphicsCollection& value ) :
    rcSegmentImpl( start, length, focus ), mValue( value )
	{
#ifdef DEBUG_LOG        
        cerr << "rcGraphicsSegment( " << start << ", " << length << ", " << value.size() << " )" << endl;
#endif        
	}

	rcVisualGraphicsCollection mValue;
};

class rcGraphicsTrackImpl : public rcTrackImpl, public rcGraphicsTrack, public rcGraphicsWriter
{
public:

	rcGraphicsTrackImpl( const char* tag , const char* name , const char* description,
                       const char* displayFormat,
                       uint32 sizeLimit );

    // virtual dtor is required
    virtual ~rcGraphicsTrackImpl() { };

    //////////////////// rcTrackImpl implementation ////////////////////
    // Number of segments
    virtual uint32 segments();
    // Segment at index
    virtual rcSegmentImpl segment( uint32 index );
    // Pop oldest segment
    virtual void popSegment();

	//////////////////// rcGraphicsTrack implementation ////////////////////

	// get an iterator for the track initialized to the
	//	specified position.  It is the responsibility of
	//	the caller to release the iterator
	virtual rcGraphicsSegmentIterator* getDataSegmentIterator( const rcTimestamp& pos );

    // get the total number of data segments.
    virtual uint32 getSegmentCount( void );
    
	//////////////////// rcGraphicsWriter implementation ////////////////////

	// write a new graphics value with timestamp.
	virtual void writeValue( const rcTimestamp& timestamp, const rcRect& focus, const rcVisualGraphicsCollection& value );

    // flush and data that might be cached
    virtual void flush( void );

    // Down cast to rcWriter*
    virtual rcWriter* down_cast();
    
	friend class rcGraphicsSegmentIteratorImpl;
    
private:
    // Update writer state
    void setWriterState( rcTimestamp time, rcRect focus, const rcVisualGraphicsCollection& value );
    // Add new segment
    void addSegment( rcTimestamp segmentLength );
    
	deque<rcGraphicsSegment>	_segments;
    rcVisualGraphicsCollection	_currentValue;
};

#endif // _rcGRAPHICSTRACKIMPL_H_
