/******************************************************************************
 *	rcVideoTrackImpl.cpp
 *
 *	This file contains the definition for the implementation of
 *	the rcVideoTrack interface.
 ******************************************************************************/

#include "rc_videotrackimpl.h"
#include <rc_thread.h>

class rcVideoSegmentIteratorImpl : public rcVideoSegmentIterator
{
public:
    rcVideoSegmentIteratorImpl( rcVideoTrackImpl* track ,
                                const rcTimestamp& position,
                                rcMutex& mutex ) : _iter( track, position, mutex ),
                                                   _track( track ),
                                                   _currentValue()
        {
            rcLock lock( _iter.mutex() );
            rmAssert( track );
            setCurrentValue();
        }

    virtual ~rcVideoSegmentIteratorImpl() { };
    
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
     /////////////////// rcVideoSegmentIterator implementation ////////////////
    
	// get the value at the current position.
	virtual rcWindow getValue( void )
        {
            return _currentValue;
        }
    
    // advance the iterator by count frames, count can be positive or negative
    // Return true if a valid segment can be returned, false otherwise
    virtual bool advance( int32 count, bool lock )
        {
            if ( lock ) {
                rcLock lock( _iter.mutex() );
                bool result = _iter.advanceNoLock( count );
                setCurrentValue();
                return result;
            } else {
                bool result = _iter.advanceNoLock( count );
                setCurrentValue();
                return result;
            }
        }

private:
    // fetch the value at the current position.
	virtual void setCurrentValue( void )
        {
            int32 index = _iter.getSegmentIndex();
            if ( index >= 0 && static_cast<uint32>(index) < _track->segments() )
                _currentValue = _track->_segments[index].mValue;
            else
                _currentValue = rcWindow();
        }
    
    rcSegmentIteratorImpl _iter;
	rcVideoTrackImpl*	  _track;
    rcWindow              _currentValue;
};

rcVideoTrackImpl::rcVideoTrackImpl( const char* tag , const char* name , const char* description,
                                    const char* displayFormat, uint32 sizeLimit )
        : rcTrackImpl( eVideoTrack , eVideoWriter , tag , name , description, displayFormat, sizeLimit ),
          _currentValue( rcWindow() )
{
    _isExportable = false;
}

rcVideoTrackImpl::~rcVideoTrackImpl()
{
}

//////////////////// rcTrackImpl implementation ////////////////////

// Number of segments
uint32 rcVideoTrackImpl::segments()
{
    return _segments.size();
}

// Segment at index
rcSegmentImpl rcVideoTrackImpl::segment( uint32 index )
{
    if ( index < _segments.size() )
        return _segments[index];
    else
        return rcSegmentImpl( cZeroTime, cZeroTime, rcRect() );
}

// Pop oldest segment
void rcVideoTrackImpl::popSegment()
{
    // We have hit the maximum size limit, we must prune oldest value
    // TODO: this wont work right until we use absolute timestamps instead of relative timestamps
    _segments.pop_front();
    // Try to compact container
    deque<rcVideoSegment>(_segments).swap(_segments);
}

//////////////////// rcVideoTrack implementation ////////////////////

rcVideoSegmentIterator* rcVideoTrackImpl::getDataSegmentIterator( const rcTimestamp& pos )
{
    return new rcVideoSegmentIteratorImpl( this , pos, _mutex );
}

// get the total number of data segments.
uint32 rcVideoTrackImpl::getSegmentCount( void )
{
    rcLock lock (_mutex);
    return _segments.size();
}

//////////////////// rcVideoWriter implementation ////////////////////

void rcVideoTrackImpl::writeValue( const rcTimestamp& timestamp, const rcRect& focus, const rcWindow* value )
{
    if ( value ) {
        rcLock lock (_mutex);
        // Update first value time
        updateFirstTime( timestamp );
        // Add new segment
        addSegment( nextSegmentLength( timestamp ) );
        // Update write state
        setWriterState( timestamp, focus, *value );
    }
}

void rcVideoTrackImpl::flush( void )
{
    rcLock lock (_mutex);
    rcTimestamp lastLength = lastSegmentLength();
    // Add final segment with length equal to previous segment length
    addSegment( lastLength );
    // Update writer state
    setWriterState( _currentLength, _currentFocus, _currentValue );
}

// Down cast to rcWriter*
rcWriter* rcVideoTrackImpl::down_cast()
{
    return dynamic_cast<rcWriter*>( this );
}

// Update writer state
void rcVideoTrackImpl::setWriterState( const rcTimestamp& time, rcRect focus, const rcWindow& frame )
{
    rcTrackImpl::setWriterState( time, focus );
    _currentValue = frame;
}

// Add new segment
void rcVideoTrackImpl::addSegment( const rcTimestamp& segmentLength )
{
    // Zero segment length is not allowed
    if ( segmentLength > cZeroTime ) {
        rcTimestamp startTime = nextSegmentStart();
#ifdef DEBUG_LOG
        rcTimestamp cumLength = cZeroTime;
        for ( int32 i = segments() - 1; i >= 0; i-- )
            cumLength += segment( i ).length();
        if ( cumLength != startTime ) {
            rcTimestamp diff = cumLength - startTime;
            cerr << "Video addSegment mismatch: cumLength ";
            cerr << cumLength.secs() << " segStart " << startTime.secs() << " diff " << diff.secs() << endl;
        }
#endif        
        _segments.push_back( rcVideoSegment( startTime, segmentLength, _currentFocus, _currentValue ) );
    }
}

