/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*
*	$Id: rc_positiontrackimpl.cpp 6561 2009-01-29 19:10:59Z arman $
*
*	This file contains the implementation of
*	the rcPositionTrack interface.
******************************************************************************/

#include "rc_positiontrackimpl.h"

class rcPositionSegmentIteratorImpl : public rcPositionSegmentIterator
{
public:
  rcPositionSegmentIteratorImpl( rcPositionTrackImpl* track ,
                                 const rcTimestamp& position,
                                 rcMutex& mutex ) : _iter( track, position, mutex ),
                                                    _track( track ),
                                                    _currentValue( rcFPair() )
	{
        rcLock lock( _iter.mutex() );
        rmAssert( track );
        setCurrentValue();
	}

	/////////////////// rcSegmentIterator implementation ////////////////

	// get the total length of the current segment.
	virtual rcTimestamp getSegmentLength( void )
        {
            return _iter.getSegmentLength();
        }

	// get our position relative to the beginning of the current segment.
	virtual rcTimestamp getSegmentOffset( void )
        {
            return _iter.getSegmentOffset();
        }

	// get our position relative to the beginning of the track.
	virtual rcTimestamp getTrackOffset( void )
        {
            return _iter.getTrackOffset();
        }

    // get our current segment index
    virtual int32 getSegmentIndex( void )
        {
            return _iter.getSegmentIndex();
        }

    // get our current relative segment start
	virtual rcTimestamp getSegmentStart( void )
        {
            return _iter.getSegmentStart();
        }
    
    // get our current absolute segment start
	virtual rcTimestamp getSegmentStartAbsolute( void )
        {
            return _iter.getSegmentStartAbsolute();
        }
    
	// are we at the last segment?
	virtual bool hasNextSegment( bool lock )
        {
            return _iter.hasNextSegment( lock );
        }

    // get the total number of segments.
    virtual uint32 getSegmentCount( void )
        {
            return _iter.getSegmentCount();
        }
    
    // current segment contains this timestamp
    virtual bool contains( const rcTimestamp& time )
        {
            return _iter.contains( time );
        }

    // lock iterator (can be done before high-frequency calls to advance(lock=false) )
    virtual void lock()
        {
            _iter.lock();
        }

    // unlock iterator
    virtual void unlock()
        {
            _iter.unlock();
        }
    
    // reset iterator back to time 0.0
    virtual void reset( bool lock )
        {
            if ( lock ) 
                _iter.lock();
            _iter.reset( false );
            setCurrentValue();
            if ( lock )
                _iter.unlock();
        }
    
	// release this iterator instance
	virtual void release( void )
        {
            delete this;
        }

    // get the focus rect at the current position.
    virtual rcRect getFocus( void )
        {
            return _iter.getFocus();
        }

	// advance the iterator by the specified amount.
	virtual void advance( const rcTimestamp& amount, bool lock )
        {
            if ( lock ) {
                rcLock lock( _iter.mutex() );
                _iter.advanceNoLock( amount );
                setCurrentValue();
            } else {
                _iter.advanceNoLock( amount );
                setCurrentValue();
            }
        }

    // advance the iterator by the specified segment offset
    virtual bool advance( int32 offset, bool lock )
        {
            if ( lock ) {
                rcLock lock( _iter.mutex() );
                bool result = _iter.advanceNoLock( offset );
                setCurrentValue();
                return result;
            } else {
                bool result = _iter.advanceNoLock( offset );
                setCurrentValue();
                return result;
            }
        }
    
    /////////////////// rcPositionSegmentIterator implementation ////////////////
    
	// get the value at the current position.
	virtual rcFPair getValue( void )
        {
            return _currentValue;
        }

private:
     // fetch the value at the current position.
	virtual void setCurrentValue( void )
        {
            int32 index = _iter.getSegmentIndex();
            if ( index >= 0 && static_cast<uint32>(index) < _track->segments() )
                _currentValue = _track->_segments[index].mValue;
            else
                _currentValue = rcFPair();
        }

    rcSegmentIteratorImpl _iter;
	rcPositionTrackImpl*  _track;
    rcFPair            _currentValue;
};


rcPositionTrackImpl::rcPositionTrackImpl( const char* tag , const char* name , const char* description,
                                          const char* displayFormat, uint32 sizeLimit )
        : rcTrackImpl( ePositionTrack, ePositionWriter, tag, name, description, displayFormat, sizeLimit )
{
    _expectedMinimumValue = rcFPair( 0.0, 0.0 );
	_expectedMaximumValue = rcFPair( 0.0, 0.0 );
	_currentMinimumValue = rcFPair( 1.0, 1.0 );
	_currentMaximumValue = rcFPair( 0.0, 0.0 );
	_currentValue = rcFPair( 0.0, 0.0 );
    _isExportable = true;
}

//////////////////// rcTrackImpl implementation ////////////////////

// Number of segments
uint32 rcPositionTrackImpl::segments()
{
    return _segments.size();
}

// Segment at index
rcSegmentImpl rcPositionTrackImpl::segment( uint32 index )
{
    if ( index < _segments.size() )
        return _segments[index];
    else
        return rcSegmentImpl( cZeroTime, cZeroTime, rcRect() );
}

// Pop oldest segment
void rcPositionTrackImpl::popSegment()
{
    // We have hit the maximum size limit, we must prune oldest value
    // TODO: this wont work right until we use absolute timestamps instead of relative timestamps
    _segments.pop_front();
    // Try to compact container
    deque<rcPositionSegment>(_segments).swap(_segments);
}

//////////////////// rcPositionTrack implementation ////////////////////


// get the minimum value (hint) the engine expected to
// generate for this track.
rcFPair rcPositionTrackImpl::getExpectedMinimumValue( void )
{
    rcLock lock (_mutex);
    return _expectedMinimumValue;
}

// get the maximum value (hint) the engine expected to
// generate for this track.
rcFPair rcPositionTrackImpl::getExpectedMaximumValue( void )
{
    rcLock lock (_mutex);
    return _expectedMaximumValue;
}


// get the current minimum value written to this track.
rcFPair rcPositionTrackImpl::getCurrentMinimumValue( void )
{
    rcLock lock (_mutex);
    return _currentMinimumValue;
}

// get the current maximum value written to this track.
rcFPair rcPositionTrackImpl::getCurrentMaximumValue( void )
{
    rcLock lock (_mutex);
    return _currentMaximumValue;
}

// get an iterator for the track initialized to the
//	specified position.  It is the responsibility of
//	the caller to release the iterator
rcPositionSegmentIterator* rcPositionTrackImpl::getDataSegmentIterator( const rcTimestamp& pos )
{
  return new rcPositionSegmentIteratorImpl( this , pos, _mutex );
}

// get the total number of data segments.
uint32 rcPositionTrackImpl::getSegmentCount( void )
{
    rcLock lock (_mutex);
    return _segments.size();
}

//////////////////// rcPositionWriter implementation ////////////////////

// set the expected minimum value for this 2D data
void rcPositionTrackImpl::setExpectedMinValue( const rcFPair& minValue )
{
    _expectedMinimumValue = minValue;
}

// set the expected maximum value for this 2D data
void rcPositionTrackImpl::setExpectedMaxValue( const rcFPair& maxValue )
{
    _expectedMaximumValue = maxValue;
}

// write a new position value with timestamp.
void rcPositionTrackImpl::writeValue( const rcTimestamp& timestamp , const rcRect& focus, const rcFPair& value )
{
    rcLock lock (_mutex);
    // Update first value time
    updateFirstTime( timestamp );
    // Update historical min/max values
    setMinMaxValues( value );
    // Add new segment
    rcTimestamp length = nextSegmentLength( timestamp );
    addSegment( length );
    // Update writer state
    setWriterState( timestamp, focus, value );
}

// Flush cached data
void rcPositionTrackImpl::flush()
{
    rcLock lock (_mutex);
    // Add final segment
    addSegment( lastSegmentLength() );
    // Update writer state
    setWriterState( _currentLength, _currentFocus, _currentValue );
}

// Down cast to rcWriter*
rcWriter* rcPositionTrackImpl::down_cast()
{
    return dynamic_cast<rcWriter*>( this );
}

// Update writer state
void rcPositionTrackImpl::setWriterState( const rcTimestamp& time, const rcRect& focus, const rcFPair& value )
{
    rcTrackImpl::setWriterState( time, focus );
    _currentValue = value;
}

// Set observed min/max values
void rcPositionTrackImpl::setMinMaxValues( const rcFPair& value )
{
    // First time through, set current min and max values
    if ( _currentMinimumValue.x() > _currentMaximumValue.x() )
    {
        _currentMinimumValue.x() = value.x();
        _currentMaximumValue.x() = value.x();
    }
    // First time through, set current min and max values
    if ( _currentMinimumValue.y() > _currentMaximumValue.y() )
    {
        _currentMinimumValue.y() = value.y();
        _currentMaximumValue.y() = value.y();
    }
    
	// Update min and max current values
	if ( value.x() < _currentMinimumValue.x() )
		_currentMinimumValue.x() = value.x();
	if ( value.x() > _currentMaximumValue.x() )
		_currentMaximumValue.x() = value.x();
	if ( value.y() < _currentMinimumValue.y() )
		_currentMinimumValue.y() = value.y();
	if ( value.y() > _currentMaximumValue.y() )
		_currentMaximumValue.y() = value.y();
}

// Add new segment
void rcPositionTrackImpl::addSegment( const rcTimestamp& segmentLength )
{
    // Zero segment length is not allowed
    if ( segmentLength > cZeroTime ) {
         // Push new segment
        _segments.push_back( rcPositionSegment( nextSegmentStart(), segmentLength, _currentFocus, _currentValue ) );
    }
}
