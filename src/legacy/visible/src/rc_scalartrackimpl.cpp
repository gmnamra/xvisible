/******************************************************************************
*	rcScalarTrackImpl.cpp
*
*	This file contains the definition for the implementation of
*	the rcScalarTrack interface.
******************************************************************************/

#include "rc_scalartrackimpl.h"

class rcScalarSegmentIteratorImpl : public rcScalarSegmentIterator
{
public:
  rcScalarSegmentIteratorImpl( rcScalarTrackImpl* track ,
                               const rcTimestamp& position,
                               rcMutex& mutex ) : _iter( track, position, mutex ),
                                                  _track( track ),
                                                  _currentValue( 0.0 )
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
    
    /////////////////// rcScalarSegmentIterator implementation ////////////////
    
	// get the value at the current position.
	virtual double getValue( void )
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
                _currentValue = 0.0;
        }

    rcSegmentIteratorImpl _iter;
	rcScalarTrackImpl*	  _track;
    double                _currentValue;
};


rcScalarTrackImpl::rcScalarTrackImpl( const char* tag , const char* name , const char* description,
                                      const char* displayFormat, uint32 sizeLimit )
        : rcTrackImpl( eScalarTrack , eScalarWriter , tag , name , description, displayFormat, sizeLimit )
{
	_expectedMinimumValue = 0.0;
	_expectedMaximumValue = 0.0;
	_currentMinimumValue = 1.0;
	_currentMaximumValue = 0.0;
	_currentValue = 0.0;
    _isExportable = true;
}

//////////////////// rcTrackImpl implementation ////////////////////

// Number of segments
uint32 rcScalarTrackImpl::segments()
{
    return _segments.size();
}

// Segment at index
rcSegmentImpl rcScalarTrackImpl::segment( uint32 index )
{
    if ( index < _segments.size() )
        return _segments[index];
    else
        return rcSegmentImpl( cZeroTime, cZeroTime, rcRect() );
}

// Pop oldest segment
void rcScalarTrackImpl::popSegment()
{
    // We have hit the maximum size limit, we must prune oldest value
    _segments.pop_front();
    // Try to compact container
    deque<rcScalarSegment>(_segments).swap(_segments);
}

//////////////////// rcScalarTrack implementation ////////////////////

// get the minimum value (hint) the engine expected to
// generate for this track.
double rcScalarTrackImpl::getExpectedMinimumValue( void )
{
    rcLock lock (_mutex);
	return _expectedMinimumValue;
}

// get the maximum value (hint) the engine expected to
// generate for this track.
double rcScalarTrackImpl::getExpectedMaximumValue( void )
{
    rcLock lock (_mutex);
	return _expectedMaximumValue;
}

// get the current minimum value written to this track.
double rcScalarTrackImpl::getCurrentMinimumValue( void )
{
    rcLock lock (_mutex);
	return _currentMinimumValue;
}

// get the current maximum value written to this track.
double rcScalarTrackImpl::getCurrentMaximumValue( void )
{
    rcLock lock (_mutex);
	return _currentMaximumValue;
}

// get an iterator for the track initialized to the
//	specified position.  It is the responsibility of
//	the caller to release the iterator
rcScalarSegmentIterator* rcScalarTrackImpl::getDataSegmentIterator( const rcTimestamp& pos )
{
    return new rcScalarSegmentIteratorImpl( this , pos, _mutex );
}

// get the total number of data segments.
uint32 rcScalarTrackImpl::getSegmentCount( void )
{
    rcLock lock (_mutex);
    return _segments.size();
}

//////////////////// rcScalarWriter implementation ////////////////////

// set the expected minimum value for this scalar data
void rcScalarTrackImpl::setExpectedMinValue( const double& minValue )
{
	_expectedMinimumValue = minValue;
}

// set the expected maximum value for this scalar data
void rcScalarTrackImpl::setExpectedMaxValue( const double& maxValue )
{
	_expectedMaximumValue = maxValue;
}

// write a new scalar value  with timestamp.
void rcScalarTrackImpl::writeValue( const rcTimestamp &timestamp, const rcRect& focus, const double& value )
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
void rcScalarTrackImpl::flush()
{
    rcLock lock (_mutex);
    // Add final segment
    addSegment( lastSegmentLength() );
    // Update writer state
    setWriterState( _currentLength, _currentFocus, _currentValue );
}

// Remove all segments, reset start time
void rcScalarTrackImpl::reset()
{
    rcLock lock (_mutex);
    _segments.clear();

    _currentLength = cZeroTime;
    _firstTime = -1.0;
        
	_currentMinimumValue = 1.0;
	_currentMaximumValue = 0.0;
	_currentValue = 0.0;
}

// Down cast to rcWriter*
rcWriter* rcScalarTrackImpl::down_cast()
{
    return dynamic_cast<rcWriter*>( this );
}

// Set observed min/max values
void rcScalarTrackImpl::setMinMaxValues( double value )
{
    // First time through, set current min and max values
    if ( _currentMinimumValue > _currentMaximumValue )
    {
        _currentMinimumValue = value;
        _currentMaximumValue = value;
    }

	// Update min and max current values
	if ( value < _currentMinimumValue )
		_currentMinimumValue = value;
	if ( value > _currentMaximumValue )
		_currentMaximumValue = value;
}

// Update writer state
void rcScalarTrackImpl::setWriterState( const rcTimestamp& time, rcRect focus, double value )
{
    rcTrackImpl::setWriterState( time, focus );
    _currentValue = value;
}

// Add new segment
void rcScalarTrackImpl::addSegment( const rcTimestamp& segmentLength )
{
    // Zero segment length is not allowed
    if ( segmentLength > cZeroTime ) {
         // Push new segment
        _segments.push_back( rcScalarSegment( nextSegmentStart(), segmentLength, _currentFocus, _currentValue ) );
    }
}


