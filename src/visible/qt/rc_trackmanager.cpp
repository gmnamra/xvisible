/******************************************************************************
 *  @file Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_trackmanager.cpp 6565 2009-01-30 03:24:44Z arman $
 *
 *	This file contains data track manager implementation.
 *
 ******************************************************************************/
#include <strstream>

#include <qapplication.h>
#include <qclipboard.h>

#include <rc_model.h>

#include "rc_modeldomain.h"
#include "rc_trackmanager.h"

static rcTrackManager* _trackManager = 0;

//@note: replace one highlited track with a vector of them 

// get the singleton rcTrackManager
rcTrackManager* rcTrackManager::getTrackManager( void )
{
    if (_trackManager == 0) {
        _trackManager = new rcTrackManager();
    }

    return _trackManager;
}

// private constructor - can only be created through
//  rcTrackManager::getTrackManager().
rcTrackManager::rcTrackManager( QObject* parent , const char* name )
        : QObject( parent , name ),  _hiliteTrackGroup( -1 ), _hiliteTrack( -1 )
{
    // Default pens for tracks

    _defaultPens.push_back( QPen( QColor("black") ) );
    _defaultPens.push_back( QPen( QColor("magenta") ) );
    _defaultPens.push_back( QPen( QColor("darkRed") ) );
    _defaultPens.push_back( QPen( QColor("darkBlue") ) );
    _defaultPens.push_back( QPen( QColor("darkGreen") ) );
    _defaultPens.push_back( QPen( QColor("darkCyan") ) );
    _defaultPens.push_back( QPen( QColor("darkMagenta") ) );
    _defaultPens.push_back( QPen( QColor("darkGray") ) );
    
    rmAssert( !_defaultPens.empty() );

    rcModelDomain* domain = rcModelDomain::getModelDomain();

    connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
             this   , SLOT( updateState( rcExperimentState ) ) );
    connect( domain , SIGNAL( elapsedTime( const rcTimestamp& ) ) ,
			 this   , SLOT( addTracks( void ) ) );
    connect( domain , SIGNAL( cursorTime( const rcTimestamp& ) ) ,
             this   , SLOT( addTracks( void  ) ) );
    connect( domain , SIGNAL( updateTimelineRange( const rcTimestamp&, const rcTimestamp& ) ) ,
             this   , SLOT( addTracks( void  ) ) );
    connect( domain , SIGNAL( updateSettings( void ) ) ,
             this   , SLOT( addTracks( void ) ) );
    connect( domain , SIGNAL( updateDebugging( void ) ) ,
             this   , SLOT( rebuildTracks( void ) ) );
}

// get the managed info for the specified track
rcTrackInfo rcTrackManager::getTrackInfo( int trackGroupNo , int trackNo )
{
    if ( static_cast<unsigned>(trackGroupNo) < _trackInfo.size() ) {
        const rcTrackInfoVector& ti = _trackInfo[ trackGroupNo ];
        if ( static_cast<unsigned>(trackNo) < ti.size() ) {
            return ti[ trackNo ];
        }
    }
    // Someone is requesting a non-existent track, maybe this will add it
    addTracks();
    
    // Try again
    if ( static_cast<unsigned>(trackGroupNo) < _trackInfo.size() ) {
        const rcTrackInfoVector& ti = _trackInfo[ trackGroupNo ];
        if ( static_cast<unsigned>(trackNo) < ti.size() ) {
            return ti[ trackNo ];
        }
    }
        
    // Requesting a track that doesn't exist, return a dummy
    cerr << "rcTrackManager::getTrackInfo error: element [" << trackGroupNo << "," << trackNo << "] ";
    cerr << "requested, array size is [" << _trackInfo.size() << "," << (_trackInfo.size() ? _trackInfo[ trackGroupNo ].size() : 0) << "]";
    cerr << endl;

    rcTrackInfo empty;
    empty._isEnabled = false;
    
    return empty;
}

// is this track currently hilited?
bool rcTrackManager::isHiliteTrack( int trackGroupNo , int trackNo )
{
    return (_hiliteTrackGroup == trackGroupNo) && (_hiliteTrack == trackNo);
}

void rcTrackManager::reportTrackHighLighted () const
{
  emit const_cast<rcTrackManager *>(this)->trackhighlighted ();
}

// set the highlighted track (-1 to unhilite all)
void rcTrackManager::setHiliteTrack( int trackGroupNo , int trackNo, bool update )
{
    _hiliteTrackGroup = trackGroupNo;
    _hiliteTrack = trackNo;
    int nTrackGroups = _trackInfo.size();
    for (int i = 0; i < nTrackGroups; i++)
    {
        int nTracks = _trackInfo[ i ].size();
        for (int j = 0; j < nTracks; j++)
        {
            rcTrackInfo info = getTrackInfo( i , j );
            QPen pen = info._pen;
            pen.setWidth( isHiliteTrack( i, j ) ? 2 : 1 );
            info._pen = pen;
            setTrackInfo( i , j , info );
        }
    }

    reportTrackHighLighted ();

    if ( update )
        rcModelDomain::getModelDomain()->notifyUpdateDisplay();
}

// get the pen for the specified track
QPen rcTrackManager::getTrackPen( int trackGroupNo , int trackNo )
{
    rcTrackInfo info = getTrackInfo( trackGroupNo , trackNo );
    return info._pen;
}

// set the pen for the specified track
void rcTrackManager::setTrackPen( int trackGroupNo , int trackNo , const QPen& pen )
{
    rcTrackInfo info = getTrackInfo( trackGroupNo , trackNo );
    QPen newPen = pen;
    newPen.setWidth( 1 );
    info._pen = newPen;
    setTrackInfo( trackGroupNo , trackNo , info );
    rcModelDomain::getModelDomain()->notifyUpdateDisplay();
}

// is the track enabled to be displayed?
bool rcTrackManager::isTrackEnabled( int trackGroupNo , int trackNo )
{
    rcTrackInfo info = getTrackInfo( trackGroupNo , trackNo );
    return info._isEnabled;
}

// enable/disable the display of a track
// -1 to apply action to all tracks
void rcTrackManager::setTrackEnabled( int trackGroupNo , int trackNo , bool isEnabled )
{
    if ( trackNo == -1 )
    {
        // Do all tracks
        int nTracks = _trackInfo[ trackGroupNo ].size();
        for (int i = 0; i < nTracks; i++)
        {
            rcTrackInfo info = getTrackInfo( trackGroupNo , i );
            info._isEnabled = isEnabled;
            setTrackInfo( trackGroupNo , i , info );
        }
    }
    else
    {
        // Do one track
        rcTrackInfo info = getTrackInfo( trackGroupNo , trackNo );
        info._isEnabled = isEnabled;
        setTrackInfo( trackGroupNo , trackNo , info );
        // If disabling a track, unhilite it, too
        if ( !isEnabled && isHiliteTrack( trackGroupNo , trackNo ) )
            setHiliteTrack( -1, false );
    }
    hiliteTracks();
}

// enable/disable all tracks of the specified type
void rcTrackManager::setTrackEnabled( rcTrackType type , bool isEnabled )
{
    // scan all tracks
    int nTrackGroups = _trackInfo.size();
    for (int i = 0; i < nTrackGroups; i++)
    {
        int nTracks = _trackInfo[ i ].size();
        for (int j = 0; j < nTracks; j++)
        {
            rcTrackInfo info = getTrackInfo( i , j );
            if (info.type() == type)
                info._isEnabled = isEnabled;
            setTrackInfo( i , j , info );
        }
    }
    hiliteTracks();
}

// enable/disable all groups that contain this track type
void rcTrackManager::setTrackEnabled( rcWriterSemantics type, bool isEnabled )
{
    // scan all tracks
    int nTrackGroups = _trackInfo.size();
    for (int i = 0; i < nTrackGroups; i++)
    {
        int nTracks = _trackInfo[ i ].size();
        
        for (int j = 0; j < nTracks; j++)
        {
            rcTrackInfo info = getTrackInfo( i , j );
            rcTrack* track = info.track();
            if ( track ) {
                if ( rcWriterManager::tagType( track->getTag() ) == type ) {
                    // Enable group
                    setTrackGroupEnabled( i, isEnabled );
                    // enable/disable all tracks in a group
                    setTrackEnabled( i, -1, isEnabled );
                    // Break out
                    goto end;
                }
            }           
        }
      end:
        void(0);
    }
}

// is the track group enabled to be displayed?
bool rcTrackManager::isTrackGroupEnabled( int trackGroupNo )
{
    return _groupEnabled[ trackGroupNo ];
}

// enable/disable the display of a track group
void rcTrackManager::setTrackGroupEnabled( int trackGroupNo , bool isEnabled )
{
    _groupEnabled[ trackGroupNo ] = isEnabled;
}

// public slots:

// may need to accomodate new tracks on state change
void rcTrackManager::updateState( rcExperimentState state )
{
    switch( state ) {
        case eExperimentEmpty:
            _trackInfo.clear();
            break;
        default:
            addTracks();
            setColors();
            break;
    }
}

// set track info
void rcTrackManager::setTrackInfo( int trackGroupNo , int trackNo , const rcTrackInfo& info )
{
     if ( static_cast<unsigned>(trackGroupNo) < _trackInfo.size() &&
         static_cast<unsigned>(trackNo) < _trackInfo[ trackGroupNo ].size() ) {
         _trackInfo[ trackGroupNo ][ trackNo ] = info;
     }
     // Someone is requesting a non-existent track, maybe this will add it
     addTracks();

     // Try again
     if ( static_cast<unsigned>(trackGroupNo) < _trackInfo.size() &&
          static_cast<unsigned>(trackNo) < _trackInfo[ trackGroupNo ].size() ) {
         _trackInfo[ trackGroupNo ][ trackNo ] = info;
     }
     else {
         cerr << "rcTrackManager::setTrackInfo error: element [" << trackGroupNo << "," << trackNo << "] ";
         cerr << "requested, array size is [" << _trackInfo.size() << "," << (_trackInfo.size() ? _trackInfo[ trackGroupNo ].size() : 0) << "]";
         cerr << endl;
     }
}

// add track groups and tracks if necessary
void rcTrackManager::addTracks( void )
{
    int i;
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	rcExperiment* experiment = domain->getExperiment();

    // first see if we need to add track groups
	const int nTrackGroups = experiment->getNTrackGroups();
    const int nManagedTrackGroups = _trackInfo.size();

    for (i = nManagedTrackGroups; i < nTrackGroups; ++i)
    {
        rcTrackInfoVector newVector;
        _trackInfo.push_back( newVector );
        _groupEnabled.push_back( true );
    }

    // now go through each group and see if new tracks were added
    for (i = 0; i < nTrackGroups; ++i)
    {
        rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
        const int nTracks = trackGroup->getNTracks();
        const rcTrackInfoVector& ti = _trackInfo[ i ];
        const int nManagedTracks = ti.size();

        for (int j = nManagedTracks; j < nTracks; ++j)
        {
            rcTrack* track = trackGroup->getTrack( j );
            QPen pen( _defaultPens[ j % _defaultPens.size() ] );           
            pen.setJoinStyle( Qt::RoundJoin );
            rcTrackInfo newInfo( true, pen, track->getTrackType(), track );
            setTrackColor( newInfo );
            _trackInfo[ i ].push_back( newInfo );
        }
    }

  
}

// set special colors
void rcTrackManager::setColors( void )
{
    // scan all tracks to update color
    for (uint32 i = 0; i < _trackInfo.size(); i++)
    {
        int nTracks = _trackInfo[ i ].size();
        rcTrackInfoVector& ti = _trackInfo[ i ];
        
        for (int j = 0; j < nTracks; j++)
        {
            rcTrackInfo& t = ti[j];
            setTrackColor( t );
        }
    }  
}

// rebuild all tracks
void rcTrackManager::rebuildTracks( void )
{
    _trackInfo.clear();
    addTracks();
    setColors();
}

void rcTrackManager::hiliteTracks( void )
{
#if 0    
    int enabledScalarTracks = 0;
    int lastEnabledGroup = -1;
    int lastEnabledScalarTrack = -1;

    // Count the number of enabled scalar tracks
    int nTrackGroups = _trackInfo.size();
    for (int i = 0; i < nTrackGroups; i++)
    {
        int nTracks = _trackInfo[ i ].size();
        for (int j = 0; j < nTracks; j++)
        {
            rcTrackInfo info = _trackInfo[ i ][ j ];
            if (info.isEnabled() && info.type() == eScalarTrack )
            {
                ++enabledScalarTracks;
                lastEnabledGroup = i;
                lastEnabledScalarTrack = j;
            }
        }
    }

    if ( enabledScalarTracks == 1 ) {
        // Only one scalar track enabled, hilite it!
        setHiliteTrack( lastEnabledGroup , lastEnabledScalarTrack, false );
    } else {
        // Unhilite all
        setHiliteTrack( lastEnabledGroup , -1, false );
    }

    rcModelDomain::getModelDomain()->notifyUpdateDisplay();
#endif    
}

// Determine whether this group has manageable tracks
bool rcTrackManager::hasManageableTracks( int group )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    bool cameraInput = domain->getExperimentAttributes().liveInput;
    bool cameraStorage = domain->getExperimentAttributes().liveStorage;

    return hasManageableTracks( group, cameraInput, cameraStorage );
}

// Determine whether this group has manageable tracks
bool rcTrackManager::hasManageableTracks( int group, bool cameraInput, bool cameraStorage )
{
    bool managed = false;
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    rcTrackGroup* trackGroup = experiment->getTrackGroup( group );
        
    if ( cameraInput && !cameraStorage ) {
        // Camera input preview
        if ( trackGroup->getType() == eGroupSemanticsCameraPreview ) {
            const int nTracks = trackGroup->getNTracks();
            for ( int j = 0; j < nTracks; ++j) {
                rcTrack* track = trackGroup->getTrack( j );
                managed = displayableTrack( track );
                if ( managed )
                    goto stop;
            }
        }
    } else {
        // File input or camera input save
        // Don't show cell group tracks in this widget
        if ( trackGroup->getType() != eGroupSemanticsBodyMeasurements &&
             trackGroup->getType() != eGroupSemanticsCameraPreview )
        {
            const int nTracks = trackGroup->getNTracks();
            for ( int j = 0; j < nTracks; ++j) {
                rcTrack* track = trackGroup->getTrack( j );
                managed = displayableTrack( track );
                if ( managed )
                    goto stop;
            }
        }
    }

  stop:
    return managed;
}

// return group type
rcGroupSemantics rcTrackManager::groupType( int trackGroupNo )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    rcTrackGroup* trackGroup = experiment->getTrackGroup( trackGroupNo );
    if ( trackGroup )
        return trackGroup->getType();

    return eGroupSemanticsUnknown;
}

// set track color based on track channel type
void rcTrackManager::setTrackColor( rcTrackInfo& trackInfo )
{
    const char* name = trackInfo._track->getName();

    // FIXME: horrible hack, use semantic type or some track attribute or
    // at least STL string map
    if ( strstr( name, "[Red]" ) ) {
        trackInfo._pen.setColor( QColor("red") );
        //cerr << "setTrackColor red" << endl;
    } else if ( strstr( name, "[Green]" ) ) {
        trackInfo._pen.setColor( QColor("green") );
        //cerr << "setTrackColor green" << endl;
    } else if ( strstr( name, "[Blue]" ) ) {
        trackInfo._pen.setColor( QColor("blue") );
        //cerr << "setTrackColor blue" << endl;
    } 
}

// Return currently highlighted track
rcTrack* rcTrackManager::getHiliteTrack() const
{
    if ( _hiliteTrackGroup >= 0 && _hiliteTrack >= 0 ) {
        rcModelDomain* domain = rcModelDomain::getModelDomain();
        rcExperiment* experiment = domain->getExperiment();
        rcTrackGroup* trackGroup = experiment->getTrackGroup( _hiliteTrackGroup );
        return trackGroup->getTrack( _hiliteTrack );
    }

    return 0;
}

// Copy track to clipboard
// @note copy to clip board data in Excel or mathematica OR a perlscript line running this track

void rcTrackManager::copyTrackToClipboard( rcTrack* track, rcCutFormat cf )
{
    if ( track && track->getTrackType() == eScalarTrack ) {
        rcModelDomain* domain = rcModelDomain::getModelDomain();
        std::string experimentName = domain->getExperimentAttributes().title;
        
        // Copy to clipboard
        rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );
        if (scalarTrack == 0)
            return;

        strstream s;
        rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( 0.0 );
        bool nextSegment = true;
        uint32 values = 0;
        // Output with high accuracy
        s.precision( 12 );
	s.setf (ios::fixed);

	//@note create util function or use boost
	if (cf == eMathematica)
	  {
	    std::string iname = domain->getExperimentAttributes().inputName;
	    std::string slash( "/" );
	    uint32 sl = iname.find_last_of( slash );
	    uint32 len (0);
	    if ( sl != std::string::npos )
	      len = iname.size() - sl - 1;
	    if ( len > 0 ) 
	      s << "{" << iname.substr( sl+1, len ) << "}" << endl;
	    else
	      s << "{" << experimentName << "}" << endl;
	  }
	else
	  s << experimentName << endl << track->getName() << endl;
	  
        if ( iter->getSegmentIndex() == -1 ) {
            // No valid value at curPos, advance one
            nextSegment = iter->advance( 1 );
        }

        if ( nextSegment ) {
            ++values;

	if (cf == eMathematica)
	  s << "{" << iter->getValue() << "," << endl; 

	else
	  s << iter->getValue() << endl;

            // Lock iterator for the total duration of value access
            iter->lock();
            // Copy all values separated by a newline
            while ( nextSegment ) {
                // Get next segment
                nextSegment = iter->advance( 1, false ); // Unlocked advance for speed
                if ( nextSegment ) {
                    ++values;

		    if (cf == eMathematica && iter->hasNextSegment (false))
		      s << iter->getValue() << "," << endl;
		    else
		      s << iter->getValue() << endl;
                } 
            }

	    if (cf == eMathematica)
	      s << "};" << endl;

            iter->unlock();
        }
        delete iter;

        s << ends;
        s.freeze(); // Without freeze(), strstream::str() leaks the string it returns
        
        QClipboard *cb = QApplication::clipboard();
        cb->setText( s.str() );
        cerr << "Copied " << values << " values from experiment \"" << experimentName << "\" track \"" << track->getName()
             << "\" to clipboard";
	if (cf == eMathematica) cerr << " (Mathematica Format) " << endl;
	else cerr << endl;
    }
}

// Is this track displayble now
bool rcTrackManager::displayableTrack( rcTrack* track ) 
{
    bool managed = false;
    rcWriterSemantics blockedVideoTrack = eWriterVideoDevelopment;
    rcWriterSemantics blockedGraphicsTrack = eWriterGraphicsDevelopment;
    if ( gDeveloperDebugging ) {
        blockedVideoTrack = eWriterUnknown;
        blockedGraphicsTrack = eWriterUnknown;
    }
    
    switch ( track->getTrackType() ) {
        case eScalarTrack:
            managed = true;
            break;
        case eVideoTrack:
            if ( rcWriterManager::tagType( track->getTag() ) != blockedVideoTrack ) {
                managed = true;
            }
            break;
        case eGraphicsTrack:
            if ( rcWriterManager::tagType( track->getTag() ) != blockedGraphicsTrack ) {
                managed = true;
            }
            break;
            
        default:
            break;
    }

    return managed;
}

// Is this track displayble in the video monitor
bool rcTrackManager::monitorableTrack( rcTrack* track ) 
{
    if ( track->getTrackType() == eVideoTrack || track->getTrackType() == eGraphicsTrack )
        return displayableTrack( track );
            
    return false;
}
