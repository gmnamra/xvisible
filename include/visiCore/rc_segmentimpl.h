/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All rights reserved.
*
*   $Id: rc_segmentimpl.h 4391 2006-05-02 18:40:03Z armanmg $
*
*	This file contains the implementation of the rcSegment interface.
******************************************************************************/

#ifndef _rcSEGMENTIMPL_H_
#define _rcSEGMENTIMPL_H_

#if WIN32
using namespace std;
#endif

#include <rc_thread.h>
#include <rc_model.h>

class rcTrackImpl;

/******************************************************************************
 *   rcSegment interface implementation
 *
 ******************************************************************************/

class rcSegmentImpl
{
  public:
    // ctors
    rcSegmentImpl() :
        mStart( cZeroTime ), mLength( cZeroTime ), mFocus( rcRect( 0, 0, 0, 0 ) ) { };
    rcSegmentImpl( const rcTimestamp& start, const rcTimestamp& length, const rcRect& focus ) :
        mStart( start ), mLength( length ), mFocus( focus ) { };

    // virtual dtor is required
    virtual ~rcSegmentImpl() {};

    // get the start of the current segment.
    virtual rcTimestamp start( void ) const { return mStart; };
    // get the total length of the current segment.
    virtual rcTimestamp length( void ) const { return mLength; };
    // set the total length of the current segment.
    virtual void setLength( const rcTimestamp& length ) { mLength = length; };
    
    // get the focus rect at the current position.
    virtual rcRect focus( void ) const { return mFocus; }
    
  private:
    rcTimestamp     mStart;    // Absolute start time of this segment
    rcTimestamp     mLength;   // Length of this segment
    rcRect          mFocus;    // Focus area for this segment
};

/******************************************************************************
 *   rcSegmentIteratorImpl interface implementation
 *
 ******************************************************************************/
class rcSegmentIteratorImpl : public rcSegmentIterator
{
  public:
    // ctor
    rcSegmentIteratorImpl( rcTrackImpl* track, const rcTimestamp& position, rcMutex& mutex );
    // virtual dtor is required
    virtual ~rcSegmentIteratorImpl() {};

    //////////////////// rcSegmentIterator implementation ////////////////////
    
    // get the total length of the current segment.
    virtual rcTimestamp getSegmentLength( void );

    // get our position relative to the beginning of the current segment.
    virtual rcTimestamp getSegmentOffset( void );

    // get our current segment index
    virtual int32 getSegmentIndex( void );

    // get the absolute start time of the current segment.
    virtual rcTimestamp getSegmentStartAbsolute( void );

    // get the start time of the current segment relative to track start
    virtual rcTimestamp getSegmentStart( void );
    
    // get our position relative to the beginning of the track.
    virtual rcTimestamp getTrackOffset( void );

    // are we at the last segment?
    virtual bool hasNextSegment( bool lock = true );

    // get the total number of segments.
    virtual uint32 getSegmentCount( void );
    
    // current segment contains this timestamp
    virtual bool contains( const rcTimestamp& time );
    
    // get the focus rect at the current position.
    virtual rcRect getFocus( void );
    
    // advance the iterator by the specified time amount.
    virtual void advance( const rcTimestamp& amount, bool lock = true );

    // advance the iterator by the specified time amount.
    // do not lock with mutex, caller must have a lock
    virtual void advanceNoLock( const rcTimestamp& amount );
    
    // advance the iterator by the specified segment offset
    virtual bool advance( int32 offset, bool lock = true );

    // advance the iterator by the specified segment offset
    // do not lock with mutex, caller must have a lock
    virtual bool advanceNoLock( int32 offset );

    // lock iterator (can be done before high-frequency calls to advance(lock=false) )
    virtual void lock();
    
    // unlock iterator
    virtual void unlock();

    // reset iterator back to time 0.0
    virtual void reset( bool lock = true );
    
    // release this iterator instance
    virtual void release( void );

    //////////////////// internal implementation ////////////////////

    // get the mutex
    rcMutex& mutex ( void );
    
  protected:
    rcTimestamp			_position; // Position from start of track
	rcSegmentImpl		_currentSegment;
	int32				_segmentIndex;
	rcTimestamp			_segmentOffset;
    rcMutex&            _mutex;
    rcTrackImpl*	    _track; // Pointer to derived track class
    rcTimestamp         _experimentOffset; // Offset of track start from experiment start

  private:
    // Return segment that contains (or is closest to) time
    // and set index
    rcSegmentImpl findClosestSegment( const rcTimestamp& time,
                                      int32& index );
};

#endif // _rcSEGMENTIMPL_H_

