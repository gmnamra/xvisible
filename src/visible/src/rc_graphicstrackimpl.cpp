/******************************************************************************
*	rcGraphicsTrackImpl.cpp
*
*	This file contains the definition for the implementation of
*	the rcGraphicsTrack interface.
******************************************************************************/

#include "rc_graphicstrackimpl.h"

class rcGraphicsSegmentIteratorImpl : public rcGraphicsSegmentIterator
{
public:
  rcGraphicsSegmentIteratorImpl( rcGraphicsTrackImpl* track ,
                                 const rcTimestamp& position,
                                 rcMutex& mutex ) : _iter( track, position, mutex ),
                                                    _track( track )
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
    
    /////////////////// rcGraphicsSegmentIterator implementation ////////////////
    
	// get the value at the current position.
	virtual rcVisualGraphicsCollection getValue( void )
        {
            return _currentValue;
        }

    // get the count of graphics objects at the current position.
	virtual uint32 getCount( void )
        {
            return _currentValue.size();
        }

private:
     // fetch the value at the current position.
	virtual void setCurrentValue( void )
        {
            int32 index = _iter.getSegmentIndex();
            if ( index >= 0 && static_cast<uint32>(index) < _track->segments() )
                _currentValue = _track->_segments[index].mValue;
            else
                _currentValue = rcVisualGraphicsCollection();
        }

    rcSegmentIteratorImpl _iter;
	rcGraphicsTrackImpl*  _track;
    rcVisualGraphicsCollection  _currentValue;
};


rcGraphicsTrackImpl::rcGraphicsTrackImpl( const char* tag , const char* name , const char* description,
                                      const char* displayFormat, uint32 sizeLimit )
        : rcTrackImpl( eGraphicsTrack , eGraphicsWriter , tag , name , description, displayFormat, sizeLimit )
{
    _isExportable = true;
	_currentValue = rcVisualGraphicsCollection();
}

//////////////////// rcTrackImpl implementation ////////////////////

// Number of segments
uint32 rcGraphicsTrackImpl::segments()
{
    return _segments.size();
}

// Segment at index
rcSegmentImpl rcGraphicsTrackImpl::segment( uint32 index )
{
    if ( index < _segments.size() )
        return _segments[index];
    else
        return rcSegmentImpl( cZeroTime, cZeroTime, rcRect() );
}

// Pop oldest segment
void rcGraphicsTrackImpl::popSegment()
{
    // We have hit the maximum size limit, we must prune oldest value
    // TODO: this wont work right until we use absolute timestamps instead of relative timestamps
    _segments.pop_front();
    // Try to compact container
    deque<rcGraphicsSegment>(_segments).swap(_segments);
}

//////////////////// rcGraphicsTrack implementation ////////////////////

// get an iterator for the track initialized to the
//	specified position.  It is the responsibility of
//	the caller to release the iterator
rcGraphicsSegmentIterator* rcGraphicsTrackImpl::getDataSegmentIterator( const rcTimestamp& pos )
{
  return new rcGraphicsSegmentIteratorImpl( this , pos, _mutex );
}

// get the total number of data segments.
uint32 rcGraphicsTrackImpl::getSegmentCount( void )
{
    rcLock lock (_mutex);
    return _segments.size();
}

//////////////////// rcGraphicsWriter implementation ////////////////////

// write a new scalar value  with timestamp.
void rcGraphicsTrackImpl::writeValue( const rcTimestamp& timestamp, const rcRect& focus, const rcVisualGraphicsCollection& value )
{
    rcLock lock (_mutex);
    // Update first value time
    updateFirstTime( timestamp );
    // Add new segment
    rcTimestamp length = nextSegmentLength( timestamp );
    addSegment( length );
    // Update writer state
    setWriterState( timestamp, focus, value );
}

// Flush cached data
void rcGraphicsTrackImpl::flush()
{
    rcLock lock (_mutex);
    // Add final segment
    addSegment( lastSegmentLength() );
    // Update writer state
    setWriterState( _currentLength, _currentFocus, _currentValue );
}

// Down cast to rcWriter*
rcWriter* rcGraphicsTrackImpl::down_cast()
{
    return dynamic_cast<rcWriter*>( this );
}

// Update writer state
void rcGraphicsTrackImpl::setWriterState( rcTimestamp time, rcRect focus, const rcVisualGraphicsCollection& value )
{
    rcTrackImpl::setWriterState( time, focus );
    _currentValue = value;
}

// Add new segment
void rcGraphicsTrackImpl::addSegment( rcTimestamp segmentLength )
{
    // Zero segment length is not allowed
    if ( segmentLength > cZeroTime ) {
         // Push new segment
        _segments.push_back( rcGraphicsSegment( nextSegmentStart(), segmentLength, _currentFocus, _currentValue ) );
    }
}


