/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_segmentimpl.cpp 6549 2009-01-29 05:39:33Z arman $
 *
 *	This file contains the implementation of the rcSegmentIteratorImpl interface.
 ******************************************************************************/

#include "rc_segmentimpl.h"
#include "rc_trackimpl.h"

rcSegmentIteratorImpl::rcSegmentIteratorImpl( rcTrackImpl* track,
                                              const rcTimestamp& position,
                                              rcMutex& mutex ) :
        _currentSegment( cZeroTime, cZeroTime, rcRect() ), _mutex( mutex )
{
    rmAssert( track );

    // Position is a timestamp relative to start of experiment
    // We need to compute what it is in relation to track start time
    rcExperimentDomain* domain = rcExperimentDomainFactory::getExperimentDomain();
    const rcTimestamp expStart = domain->getExperimentStart();
    rcTimestamp trackStart = track->getTrackStart();
    const rcTimestamp pos = (expStart + position) - trackStart;
        
    if ( trackStart >= cZeroTime ) {
        // Track cannot start before experiment starts...
        if ( trackStart < expStart )
            trackStart = expStart;
        _experimentOffset = trackStart - expStart;
    } else
        _experimentOffset = cZeroTime;
    _track = track;
    _segmentIndex = -1;

    // handle case where position is negative - act like there's
    //	a dummy segment before the first real data segment
    if ( pos < cZeroTime ) {
        _position = pos;
        _currentSegment.setLength( _position );
        _segmentOffset = cZeroTime;
    }
    else
    {
        _position = cZeroTime;
        _segmentOffset = cZeroTime;
        advance( pos );
    }
}

 //////////////////// rcSegmentIterator implementation ////////////////////

// get the total length of the current segment.
rcTimestamp rcSegmentIteratorImpl::getSegmentLength( void )
{
    return _currentSegment.length();
}

// get the start time of the current segment relative to track start
rcTimestamp rcSegmentIteratorImpl::getSegmentStart( void )
{
    return _currentSegment.start();
}

// get the absolute start time of the current segment 
rcTimestamp rcSegmentIteratorImpl::getSegmentStartAbsolute( void )
{
    return _experimentOffset + _currentSegment.start();
}

// get our position relative to the beginning of the current segment.
rcTimestamp rcSegmentIteratorImpl::getSegmentOffset( void )
{
    return _segmentOffset;
}

// get our current segment index
int32 rcSegmentIteratorImpl::getSegmentIndex( void )
{
    return _segmentIndex;
}

// get our position relative to the beginning of the track.
rcTimestamp rcSegmentIteratorImpl::getTrackOffset( void )
{
    return _position;
}

// are we at the last segment?
bool rcSegmentIteratorImpl::hasNextSegment( bool lock )
{
    if ( lock ) {
        rcLock lock (_mutex);
        int nSegments = _track->segments();
        return _segmentIndex < (nSegments - 1);
    } else {
        int nSegments = _track->segments();
        return _segmentIndex < (nSegments - 1);
    }
}

// get the total number of segments.
uint32 rcSegmentIteratorImpl::getSegmentCount( void )
{
    rcLock lock (_mutex);
    return _track->segments();
}

// current segment contains this timestamp
bool rcSegmentIteratorImpl::contains( const rcTimestamp& time )
{
    rcTimestamp startSeg = _experimentOffset + _currentSegment.start();
    rcTimestamp endSeg = startSeg + _currentSegment.length();
    if ( time >= startSeg && time <= endSeg )
        return true;
    else
        return false;
}

// get the focus rect at the current position.
rcRect rcSegmentIteratorImpl::getFocus( void )
{
    return _currentSegment.focus();
}

// advance the iterator by the specified amount (new implementation).
void rcSegmentIteratorImpl::advance( const rcTimestamp& amount, bool lock )
{
    if ( lock ) {
        rcLock lock (_mutex);
        advanceNoLock( amount );
    } else {
        advanceNoLock( amount );
    }
}

// advance the iterator by the specified amount (new implementation).
void rcSegmentIteratorImpl::advanceNoLock( const rcTimestamp& amount )
{
    const int nSegments = _track->segments();
    
    if ( nSegments > 0 ) {
        // First advance call
        if ( _segmentIndex < 0 ) {
            _position = cZeroTime;
            _segmentOffset = cZeroTime;
            _segmentIndex = 0;
            _currentSegment = _track->segment( _segmentIndex );
        }
        
        // Sequential advance call
        if ( amount > cZeroTime ) {
            rcTimestamp start = _currentSegment.start();
            rcTimestamp target = _segmentOffset + amount;
            if ( _position >= start )
                target += start;
            // Find closest segment
            _currentSegment = findClosestSegment( target, _segmentIndex );
            start = _currentSegment.start();
            const rcTimestamp end = start + _currentSegment.length();
            
            if ( target < start ) {
                // Requested time is before start of track
                _position = target;
                _segmentOffset = target;
            } else {
                if ( target < end ) {
                    // Requested time is within a segment
                    _position = target;
                    _segmentOffset = _position - start;
                }
                else if ( target > end ) {
                    // Requested time is after end of track
                    _position = end;
                    _segmentOffset = cZeroTime;
                } else {
                    // Requested time is on a segment boundary
                    _position = target;
                    _segmentOffset = cZeroTime;
                }
            }
        }
    }
}

// Advance by count frame and round _position to a segment boundary
bool rcSegmentIteratorImpl::advance( int32 count, bool lock )
{
    if ( lock ) {
        rcLock lock (_mutex);
        return advanceNoLock( count );
    } else {
        return advanceNoLock( count );
    }
}

// Advance by count frame and round _position to a segment boundary
bool rcSegmentIteratorImpl::advanceNoLock( int32 count )
{
    int32 nSegments = _track->segments();
    int32 newIndex = _segmentIndex + count;

    if ( newIndex >= nSegments ) {
        // Can't advance forward any more
        return false;
    }
    else if ( newIndex < 0 ) {
        // Can't advance backwards any more
        return false;
    }
    _currentSegment = _track->segment( newIndex );
    _segmentOffset = cZeroTime;
    _position = _currentSegment.start();
    _segmentIndex = newIndex;
    
    return true;
}

// lock iterator (can be done before high-frequency calls to advance(lock=false) )
void rcSegmentIteratorImpl::lock()
{
    _mutex.lock();
}

// unlock iterator
void rcSegmentIteratorImpl::unlock()
{
    _mutex.unlock();
}

// reset iterator back to time 0.0
void rcSegmentIteratorImpl::reset( bool lock )
{
    rcExperimentDomain* domain = rcExperimentDomainFactory::getExperimentDomain();
    const rcTimestamp expStart = domain->getExperimentStart();
    rcTimestamp trackStart = _track->getTrackStart();
    rcTimestamp position = expStart - trackStart;
        
    if ( lock ) 
        _mutex.lock();
    
    _segmentIndex = -1;
    _currentSegment = rcSegmentImpl( cZeroTime, cZeroTime, rcRect() );
    _segmentOffset = cZeroTime;
    // handle case where position is negative - act like there's
    // a dummy segment before the first real data segment
    if ( position < cZeroTime ) {
        _position = position;
        _currentSegment.setLength( position );
    } else {
        _position = cZeroTime;
        advanceNoLock( position );
    }
    
    if ( lock ) 
        _mutex.unlock();
}

// release this iterator instance
void rcSegmentIteratorImpl::release( void )
{
    delete this;
}

//////////////////// internal implementation ////////////////////

// get the mutex
rcMutex& rcSegmentIteratorImpl::mutex( void )
{
    return _mutex;
}

// private

// Return segment that contains (or is closest to) time
// Binary search
rcSegmentImpl rcSegmentIteratorImpl::findClosestSegment( const rcTimestamp& time,
                                                         int32& index )
{
    index = -1;
    const int32 nSegments = _track->segments();
    
    if ( nSegments > 0 ) {
        int32 first = 0;
        int32 last = nSegments - 1;

        // Binary search the segments
        while ( first <= last ) {
            int32 mid = (first + last) / 2;  
            rcSegmentImpl currentSegment = _track->segment( mid );
            rcTimestamp start = currentSegment.start();

            if ( time < start ) {
                last = mid - 1;
            }
            else if ( time > start ) {
                rcTimestamp end = start + currentSegment.length();
                if ( time < end ) {
                    index = mid;
                    return currentSegment;
                }
                first = mid + 1;
            }
            else {
                index = mid;
                return currentSegment;
            }
        }
        if ( last < 0 )
            index = 0;    // Time is before start of track
        else 
            index = last; // Time is after end of track
        
        return _track->segment( index );
    }
    
    rmAssert( nSegments > 0 );
    // Error, this should never happen
    return _currentSegment;
}

// For debugging display
ostream& operator << ( ostream& os, rcSegmentIterator& i )
{
    os << "rcSegmentIterator";
    rcTimestamp segStart = i.getSegmentStart();
    rcTimestamp segAbsStart = i.getSegmentStartAbsolute();
    rcTimestamp segLen = i.getSegmentLength();

    os << " segRelStart " << segStart << " segAbsStart " << segAbsStart << " segLen " << segLen <<  " segRelEnd " << segStart + segLen;
    os << " position " << i.getTrackOffset() << " segIdx " << i.getSegmentIndex() << " segOffset " << i.getSegmentOffset() << endl;
   
    return os;
}


