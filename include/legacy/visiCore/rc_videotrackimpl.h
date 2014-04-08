/******************************************************************************
*	rc_videotrackimpl.h
*
*	This file contains the declaration for the implementation of
*	the rcVideoTrack interface.
******************************************************************************/

#ifndef rcVIDEOTRACKIMPL_H
#define rcVIDEOTRACKIMPL_H

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

class rcVideoSegment : public rcSegmentImpl
{
  public:
    
    rcVideoSegment( void ) :
    rcSegmentImpl( cZeroTime, cZeroTime, rcRect( 0, 0, 0, 0 ) ), mValue( rcWindow() )
    {
    }

    rcVideoSegment( const rcTimestamp& start, const rcTimestamp& length, const rcRect& focus, rcWindow& value ) :
    rcSegmentImpl( start, length, focus ), mValue( value )
    {
#ifdef DEBUG_LOG             
        cerr << "rcVideoSegment( " << start << ", " << length << ") " << endl;
#endif        
    }
    
    rcWindow        mValue;
};

class rcVideoTrackImpl : public rcVideoTrack, public rcVideoWriter, public rcTrackImpl
{
  public:

	rcVideoTrackImpl( const char* tag , const char* name , const char* description,
                      const char* displayFormat, uint32 sizeLimit );
    // virtual dtor is required
    virtual ~rcVideoTrackImpl();

    //////////////////// rcTrackImpl implementation ////////////////////
    // Number of segments
    virtual uint32 segments();
    // Segment at index
    virtual rcSegmentImpl segment( uint32 index );
    // Pop oldest segment
    virtual void popSegment();
    
	//////////////////// rcVideoTrack implementation ////////////////////
    virtual rcVideoSegmentIterator* getDataSegmentIterator( const rcTimestamp& pos );
    // get the total number of data segments.
    virtual uint32 getSegmentCount( void );
    
	//////////////////// rcVideoWriter implementation ////////////////////
    virtual void writeValue( const rcTimestamp& timestamp , const rcRect& focus, const rcWindow* frame );
    virtual void flush( void );
    // Down cast to rcWriter*
    virtual rcWriter* down_cast();
    
	friend class rcVideoSegmentIteratorImpl;
    
  private:
    // Update writer state
    virtual void setWriterState( const rcTimestamp& time, rcRect focus, const rcWindow& frame );
    // Add new segment
    void addSegment( const rcTimestamp& segmentLength );

    deque<rcVideoSegment> _segments;
    rcWindow              _currentValue;
};


#endif // rcVIDEOTRACKIMPL_H
