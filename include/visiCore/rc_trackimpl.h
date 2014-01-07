/******************************************************************************
*	rc_trackimpl.h
*
*	This file contains the definition for the implementation of
*	the rcTrack interface.
******************************************************************************/

#ifndef rcTRACKIMPL_H
#define rcTRACKIMPL_H

#if WIN32
using namespace std;
#endif

#include <rc_setting.h>
#include <rc_engine.h>
#include <rc_model.h>

#include <rc_segmentimpl.h>

class rcTrackImpl : public rcTrack, public rcWriter
{
  public:

	rcTrackImpl( rcTrackType trackType , rcWriterType writerType ,
                 const char* tag , const char* name , const char* description,
                 const char* displayFormat, uint32 limit,
                 rcTimestamp firstTime = -1.0 )

	{
        _dataSemantics = eTrackSemanticsUnknown;
		_trackType = trackType;
		_writerType = writerType;
		_tag = tag;
		_name = name;
		_description = description;
        if ( !displayFormat )
            _displayFormat = "%-8.4f";
        else
            _displayFormat = displayFormat;
        _sizeLimit = limit;
        _currentLength = cZeroTime;
        _firstTime = firstTime;
        if ( _firstTime >= cZeroTime )
            _currentLength = _firstTime;
        else
            _currentLength = cZeroTime;
        // Exportability has to be explicitly allowed by each derived class
        _isExportable = false;
	}
    
	// virtual dtor is required
	virtual ~rcTrackImpl() { };

	//////////////// rcTrack and rcWriter implementation ////////////////////

	// get the track's data type
	virtual rcTrackType getTrackType( void )
	{
		return _trackType;
	}

	// get the track's data type
	virtual rcWriterType getWriterType( void )
	{
		return _writerType;
	}

	// get the meta-data tag for this track
	virtual const char* getTag( void )
	{
		return _tag.c_str();
	}

	// get the display name for the track
	virtual const char* getName( void )
	{
		return _name.c_str();
	}

	// get the description for the track
	virtual const char* getDescription( void )
	{
		return _description.c_str();
	}

    // get the number of values that will be kept in memory
    virtual uint32 getSizeLimit() const
    {
        return _sizeLimit;
    }

    // get format string for string display (sprintf etc.)
    virtual const char* getDisplayFormatString( void )
    {
        return _displayFormat.c_str();
    }

    // get track start time
    virtual rcTimestamp getTrackStart( void )
    {
        return _firstTime;
    }

    // set absolute track start time
    virtual void setTrackStart( const rcTimestamp& time )
    {
        // We can only set track start once at the beginning
        rmAssert( _firstTime == -1.0 );
         _firstTime = time;
         _currentLength = _firstTime;
    }

    // set exportability
    virtual void setExportable( bool e )
    {
        _isExportable = e;
    }

    // Set the description text
	virtual void setDescription( const char* text ) {
        _description = text;
    }

    // Set the name text
	virtual void setName( const char* text ) {
        _name = text;
    }
    
     // flush any data that might be cached
    virtual void flush( void )
    {
        rmAssert( 0 ); // Derived classes must implement this
    }

    // is this track exportable ?
    virtual bool isExportable( void )
    {
        return _isExportable;
    }
    
    // Data semantics, what does it represent
    virtual rcTrackSemantics getSemantics( void )
    {
        return _dataSemantics;
    }

    // Data semantics, what does it represent
    virtual void setSemantics( const rcTrackSemantics& s )
    {
         _dataSemantics = s;
    }

    // Get track analysis rect
    virtual rcRect getAnalysisRect( void )
    {
        return _analysisFocus;
    }

    // Set track analysis rect
    virtual void setAnalysisRect( const rcRect& f )
    {
        _analysisFocus = f;
    }
    
    //////////////// internal implementation ////////////////////
    
    // Number of segments
    virtual uint32 segments( void )
    {
        // Derived classed MUST implement this
        rmAssert( 0 );
        return 0;
    }

    // Segment at index
    virtual rcSegmentImpl segment( uint32 index )
    {
        rmUnused( index );

        rmAssert( 0 );
        // Derived classed MUST implement this
        return rcSegmentImpl( cZeroTime, cZeroTime, rcRect( 0, 0, 0, 0) );
    }

    // Pop oldest segment
    virtual void popSegment()
    {
        rmAssert( 0 );
        // Derived classed MUST implement this
    }

  protected:
    // Return last segment length
    rcTimestamp lastSegmentLength()
    {
        int segCount = segments();

        if ( segCount > 0 ) {
            rcSegmentImpl lastSegment = segment( segCount -1 );
            return lastSegment.length();
        } else
            return cZeroTime;
    }
    
    // Return length of segment to be added
    rcTimestamp nextSegmentLength( const rcTimestamp& timestamp )
    {
        if ( timestamp <= _firstTime )
            return cZeroTime;

        return timestamp - _currentLength;
    }

    // Return start time of segment to be added
    rcTimestamp nextSegmentStart()
    {
        rcTimestamp startTime =  _currentLength - _firstTime;
        if ( startTime < cZeroTime ) {
            cerr << endl;
            cerr << "nextSegmentStart error for track \"" << _name << "\": negative first segment timestamp ";
            cerr << startTime << ", first segment time "  << _firstTime << endl << endl;
            startTime = cZeroTime;
            // TODO: make this assert when code has been debugged better
        }
        return startTime;
    }
    
    // Update writer state
    virtual void setWriterState( const rcTimestamp& time, rcRect focus )
    {
        rmAssert( time >= cZeroTime );
        
        // Set current state
        _currentLength = time;
        _currentFocus = focus;
        // Prune old values if necessary
        if ( segments() > _sizeLimit )
            popSegment();
    };

    // We must keep the time of first value for offset calculations
    void updateFirstTime( const rcTimestamp& timestamp )
    {
        if ( _firstTime < cZeroTime ) {
            _firstTime = timestamp;
            _currentLength = _firstTime;
        }
    };

    rcMutex         _mutex;
    rcTimestamp     _currentLength;
    rcRect          _currentFocus; // Focus at current time
    uint32        _sizeLimit;
    rcTimestamp     _firstTime;    // Timestamp of first value
    rcTimestamp     _expStartTime; // Timestamp of first experiment frame
    bool            _isExportable;
    rcTrackSemantics _dataSemantics;
    rcRect           _analysisFocus;  // Focus when track was created
    
  private:
    // For rcTrack and rcWriter APIs
	rcTrackType		 _trackType;
	rcWriterType	 _writerType;
	std::string		 _tag;
	std::string		 _name;
	std::string		 _description;
    std::string		 _displayFormat;  
};


#endif // rcEXPERIMENTIMPL_H
