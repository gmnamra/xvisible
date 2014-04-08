#ifndef rcTRACK_MANAGER_H
#define rcTRACK_MANAGER_H

#include <deque>

#include <qobject.h>
#include <qpen.h>
#include <qfont.h>

#include <rc_timestamp.h>
#include <rc_model.h>
#include <rc_writermanager.h>

#include "rc_modeldomain.h"

class rcTrackManager;

// track info managed by rcTrackManager
class rcTrackInfo
{
public:
    
    // is this track enabled to view?
    bool isEnabled( void ) const { return _isEnabled; }

    // get the pen to render this track with
    QPen getPen( void ) const { return _pen; }

    // get track type
    rcTrackType type( void ) const { return _type; }

    // get track ptr
    rcTrack* track( void ) const { return _track; }
    
private:
    // only the rcTrackManager can access rcTrackInfo members
    friend class rcTrackManager;

    // only the rcTrackManager can create a rcTrackInfo
    rcTrackInfo( void )
        : _isEnabled( true )
        , _pen( QPen(Qt::black) )
        , _type( eMaxTrack )
        , _track( NULL )
    {}
    rcTrackInfo( bool enabled, const QPen& pen, rcTrackType t, rcTrack* p)
        : _isEnabled( enabled )
        , _pen( pen )
        , _type( t )
        , _track ( p )
    {}
    
    bool        _isEnabled;
    QPen        _pen;
    rcTrackType _type;
    rcTrack*    _track;
};

typedef deque<rcTrackInfo> rcTrackInfoVector;

// definition of the singleton rcTrackManager
class rcTrackManager : public QObject
{
    Q_OBJECT

    friend class rcTrackInfo;

public:

  enum rcCutFormat
  {
    eBasic,
    eMathematica,
    eTrackScript
  };

    // get the singleton rcTrackManager
    static rcTrackManager* getTrackManager( void );
    // Is this track displayble now
    static bool displayableTrack( rcTrack* track );
    static bool monitorableTrack( rcTrack* track );
    
    // get the managed info for the specified track
    rcTrackInfo getTrackInfo( int trackGroupNo , int trackNo );

    // is this track currently hilited?
    bool isHiliteTrack( int trackGroupNo , int trackNo );

    // set the highlighted track (-1 to unhilite all)
    void setHiliteTrack( int trackGroupNo , int trackNo, bool update = true );

    // get the pen for the specified track
    QPen getTrackPen( int trackGroupNo , int trackNo );

    // set the pen for the specified track
    void setTrackPen( int trackGroupNo , int trackNo , const QPen& pen );

    // is the track enabled to be displayed?
    bool isTrackEnabled( int trackGroupNo , int trackNo );

    // enable/disable the display of a track
    void setTrackEnabled( int trackGroupNo , int trackNo , bool isEnabled );

    // enable/disable the display of all tracks of type
    void setTrackEnabled( rcTrackType type, bool isEnabled );
    // enable/disable all groups that contain this track type
    void setTrackEnabled( rcWriterSemantics type, bool isEnabled );
    
    // is the track group enabled to be displayed?
    bool isTrackGroupEnabled( int trackGroupNo );

    // enable/disable the display of a track group
    void setTrackGroupEnabled( int trackGroupNo , bool isEnabled );
    
    // Determine whether this group has manageable tracks
    static bool hasManageableTracks( int group );
    // Determine whether this group has manageable tracks
    static bool hasManageableTracks( int group, bool cameraInput, bool cameraStorage );
    
    // return group type
    rcGroupSemantics groupType( int trackGroupNo );

    // set track color based on track channel type
    void setTrackColor( rcTrackInfo& trackInfo );

    // Return currently highlighted track
    rcTrack* getHiliteTrack() const;

    // Copy track to clipboard
    void copyTrackToClipboard( rcTrack* track, rcCutFormat cf = eBasic);

    void reportTrackHighLighted (void) const;

public slots:
    // may need to accomodate new tracks on state change
	void updateState( rcExperimentState state );
    // add tracks to be managed
    void addTracks( void );
    // set special colors
    void setColors();
    // rebuild track list
    void rebuildTracks();
    
signals:
    void trackhighlighted (void);

protected:

private:
    // can't construct an rcTrackManager, use the static
    //  singleton accessor method 'getTrackManager'
    rcTrackManager( QObject* parent=0, const char* name=0 );

    // set track info
    void setTrackInfo( int trackGroupNo , int trackNo , const rcTrackInfo& info );

    // hilite tracks
    void hiliteTracks( void );
        
    class rcTrackInfoVector : public deque<rcTrackInfo>
    {
    };

    deque<rcTrackInfoVector> _trackInfo;
    deque<bool>        _groupEnabled;
    deque<QPen>        _defaultPens;
    int                 _hiliteTrackGroup;
    int                 _hiliteTrack;
};


#endif // rcTRACK_MANAGER_H
