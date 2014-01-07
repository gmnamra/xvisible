/******************************************************************************
*	rc_model.h
*
*	This file contains the definitions for the pure virtual classes
*	that constitute the interface contract between the user interface
*	and the model application layers.
******************************************************************************/

#ifndef _rcMODEL_H_
#define _rcMODEL_H_

#include <rc_value.h>
#include <rc_uitypes.h>
#include <rc_setting.h>
#include <rc_timestamp.h>
#include <rc_xml.h>
#include <rc_persistencemanager.h>
#include <rc_graphics.h>
#include <rc_affine.h>
#include <rc_polygongroup.h>

/******************************************************************************
*	Forwards
******************************************************************************/

class rcWindow;
class rcExperimentDomain;
class rcExperimentObserver;
class rcExperiment;
class rcExperimentData;
class rcTrackGroup;
class rcTrack;
class rcScalarTrack;
class rcScalarSegmentIterator;
class rcVideoTrack;
class rcVideoSegmentIterator;
class rcGraphicsTrack;
class rcGraphicsSegmentIterator;
class rcPositionTrack;
class rcPositionSegmentIterator;

/******************************************************************************
*	Constants
******************************************************************************/

// experiment state values returned by 'rcExperiment::getState()'.
enum rcExperimentState
{
    eExperimentEmpty = 0	// experiment was created but not run
  , eExperimentRunning		// experiment is running
  , eExperimentPlayback		// experiment is being played back
  , eExperimentEnded		// experiment has ended
  , eExperimentLocked		// experiment has ended and been saved
};

// experiment data track types
enum rcTrackType
{
	eVideoTrack = 0		// track contains video frames
  , eScalarTrack		// track contains scalar data
  , ePositionTrack		// track contains <x,y> position data
  , eGraphicsTrack      // track contains graphical data for visualization
  , eMaxTrack           // synthetic sentinel
};

// experiment data track type XML attribute values
// Note: these must match the order of rcTrackType
const std::string cTrackTypeNames[] =
{
    "video",
    "scalar",
    "position",
    "graphics",
    "unknown"
};

/******************************************************************************
*	Exceptions
******************************************************************************/

class rcModelException : public general_exception
{
public:
	rcModelException( const std::string& msg ) : general_exception( msg ) {}
};

/******************************************************************************
*	rcExperimentDomainFactory class definition
*
*	The rcExperimentDomainFactory class consists of a single static
*	method to get the linked-in (statically or dynamically) implementation
*	of the rcExperimentDomain interface.
******************************************************************************/

class rcExperimentDomainFactory
{
public:
	static rcExperimentDomain* getExperimentDomain( const char* implKey = 0 );
};

/******************************************************************************
*	rcExperimentDomain interface definition.
*
*	This is the top-level model singleton object.
******************************************************************************/

class rcExperimentDomain
{
public:
    // virtual dtor is required
    virtual ~rcExperimentDomain() { };

	// initialize the experiment domain and pass in an observer
	//	so the model can communicate asynchronous events to the
	//	domain client.
	virtual void initialize( rcExperimentObserver* observer ) = 0;

	// shutdown the experiment domain
	virtual void shutdown( void ) = 0;

	// get the current experiment.
	virtual rcExperiment* getExperiment( void ) = 0;

	// create a new experiment and make it current.
	virtual void newExperiment( void ) = 0;

    // load an experiment from a tree and make it current.
    // return number of errors
	virtual int loadExperiment( const char* fileName,
                                const rcXMLElementTree& tree,
                                rcExperimentImportMode mode,
                                rcProgressIndicator* progress = 0 ) = 0;

	// save the experiment to disk
    // returns errno
	virtual int saveExperiment( const char* fileName = 0,
                                rcExperimentFileFormat format = eExperimentCSVFormat,
                                rcProgressIndicator* progress = 0 ) = 0;

	// get the experiment's status (see rcExperimentState enum definition above)
	virtual rcExperimentState getExperimentState( void ) = 0;

    // get the experiment's attributes (see rcExperimentAttributes
    //  struct definition)
    virtual const rcExperimentAttributes getExperimentAttributes( void ) = 0;

    // set the current experiment attributes
    virtual void setExperimentAttributes( const rcExperimentAttributes& attrs ) = 0;

	// get the engine live attributes
	virtual bool operatingOnLiveInput ( void ) = 0;
	virtual bool storingLiveInput ( void ) = 0;

	// get the current running rcTimestamp
	virtual rcTimestamp getExperimentLength( void ) = 0;

	// return the current cursor time
	virtual rcTimestamp getCursorTime( void ) = 0;

	// Called when cursor time is updated from the GUI.
	virtual void notifyCursorTime( const rcTimestamp& time ) = 0;

    // Called when movie has been imported
	virtual void notifyMovieImport() = 0;

    // get the absolute experiment start rcTimestamp
	virtual rcTimestamp getExperimentStart( void ) = 0;

	// start the experiment
	virtual void startExperiment( void ) = 0;

	// stop the experiment
	virtual void stopExperiment( void ) = 0;

    // process the experiment
    virtual void processExperiment( void ) = 0;

    // Pause a Tracking Experiment
    virtual bool pauseTrackingExperiment( void ) = 0;

    // stop pause a Tracking Experiment
    virtual bool doneSelecting ( void ) = 0;

	// if the observer is accepting polys, this is called to
    // tell the observer to put the polys
	virtual void notifyPolys (const rcPolygonGroupRef * polys  ) = 0;


};

/******************************************************************************
*	rcExperimentObserver interface definition.
*
*	This is the definition of the experiment observer class.
******************************************************************************/

class rcExperimentObserver
{
public:
    // virtual dtor is required
    virtual ~rcExperimentObserver() { };

	// called to notify the observer that an error occurred.
	virtual void notifyError( const char* errorString ) = 0;

	// called to warn the observer of some condition.
	virtual void notifyWarning( const char* warnString ) = 0;

	// called to send status to the observer
	virtual void notifyStatus( const char* statusString ) = 0;

	// called to notify observer of a state change.
	virtual void notifyState( rcExperimentState newState,
                              bool immediateDispatch = false ) = 0;

    // called to notify observer that time has elapsed
    virtual void notifyTime( const rcTimestamp& elapsedTime ) = 0;

    // called to notify observer that cursor time has programatically been changed
    virtual void notifyProgCursorTime( const rcTimestamp& elapsedTime ) = 0;

    // Returns the number of eNotifyCursorEvent events queued up for processing
    virtual int32 progCursorTimeEventCount( ) const = 0;

    // called to notify observer of a timeline range change
    virtual void notifyTimelineRange( const rcTimestamp& start ,
                                      const rcTimestamp& end ) = 0;

    // called by engine to notify observer of a timeline range change
    virtual void notifyEngineTimelineRange( const rcTimestamp& start ,
                                            const rcTimestamp& end ) = 0;

    // called to notify observer of a analysis focus rect change.
    virtual void notifyAnalysisRect( const rcRect& rect ) = 0;

    // called to notify observer of a analysis focus rect rotation change
    // angle is in degrees
    virtual void notifyAnalysisRectRotation( const rcAffineRectangle& affine) = 0;

    // called to notify observer of a change in multiplier
    virtual void notifyMultiplier( const double& multiplier ) = 0;

    // called to ask the observer if it should blast an image to
    //	a part of the screen of the observer's choosing.
    virtual bool acceptingImageBlits( void ) = 0;

    // if the observer is accepting image blits, this is called to
    // tell the observer to blit the image
    virtual void notifyBlitData( const rcWindow* image ) = 0;

    // if the observer is accepting image blits, this is called to
    // tell the observer to blit the graphics
    virtual void notifyBlitGraphics( const rcVisualGraphicsCollection* graphics ) = 0;

    // Carbon locking
    virtual void notifyLockApp( bool lock ) = 0;

    // Experiment has changed, notify all widgets to update themselves
    virtual void notifyExperimentChange( void ) = 0;

    // called to notify observer of a video frame change.
    virtual void notifyVideoRect( const rcRect& rect ) = 0;

		// called to ask the observer if it accepts
    // put polys calls
    virtual bool acceptingPolys ( void ) = 0;

	// if the observer is accepting polys, this is called to
    // tell the observer to put the polys
	virtual void notifyPolys () = 0;

	// if the observer is accepting polys, this is called to
    // tell the observer to get the possible changed polys
  virtual void getPolys ( rcPolygonGroupRef& polys ) = 0;

};

/******************************************************************************
*	rcExperiment interface definition.
*
*	This is the definition of the experiment "document" class.
*	It manages the persistence of both the settings used to
*	control the experiment and the data that the experiment
*	generated.  It also allows control over starting, stopping,
*	and pausing the experiment and monitoring it's progress.
******************************************************************************/

class rcExperiment
{
public:
    // virtual dtor is required
    virtual ~rcExperiment() { };

	// get the number of experiment settings categories
	virtual int getNSettingCategories( void ) = 0;

	// get a settings category
	virtual rcSettingCategory getSettingCategory( int categoryNo ) = 0;

	// get the number of experiment data track groups.
	virtual int getNTrackGroups( void ) = 0;

	// get a particular experiment data track group
	virtual rcTrackGroup* getTrackGroup( int trackGroupNo ) = 0;

	// get the maximum track length
	virtual rcTimestamp getLength( void ) = 0;
};

/******************************************************************************
*	rcTrackGroup interface definitions.
*
*	The track groups allows experiment results to be grouped according
*   to some analysis-specific need (e.g., one group per analysis area).
*   Each track group is composed of one or more tracks.
******************************************************************************/

class rcTrackGroup
{
public:
    // virtual dtor is required
    virtual ~rcTrackGroup() { };

	// get the meta-data tag for this track group
	virtual const char* getTag( void ) const = 0;

	// get the display name for the track group
	virtual const char* getName( void ) const = 0;

	// get the description for the track group
	virtual const char* getDescription( void ) const = 0;

    // get the semantic type of this group
    virtual rcGroupSemantics getType() = 0;

	// get the number of data tracks in this group.
	virtual int getNTracks( void ) const = 0;

	// get a particular group data track
    virtual rcTrack* getTrack( int trackNo ) = 0;
};

/******************************************************************************
*	rcTrack interface definitions.
*
*	This is the interface that all experiment data tracks share.
*	Experiment data is composed of any number of data tracks.
*	Each data track contains one type of data.  The different
*	types of data that a track can contain are defined by the
*	rcTrackType enum (defined above).  All tracks have an
*	associated meta-data tag, name, and description.
******************************************************************************/

class rcTrack
{
public:
    // virtual dtor is required
    virtual ~rcTrack() { };

	// get the track's data type
	virtual rcTrackType getTrackType( void ) = 0;

	// get the meta-data tag for this track
	virtual const char* getTag( void ) = 0;

	// get the display name for the track
	virtual const char* getName( void ) = 0;

	// get the description for the track
	virtual const char* getDescription( void ) = 0;

    // get the number of values that will be kept in memory
    virtual uint32 getSizeLimit() const = 0;

    // get format string for string display (sprintf etc.)
    virtual const char* getDisplayFormatString( void ) = 0;

    // get track start time
    virtual rcTimestamp getTrackStart( void ) = 0;

    // is this track exportable ?
    virtual bool isExportable( void ) = 0;

    // Data semantics, what does it represent
    virtual rcTrackSemantics getSemantics( void ) = 0;

    // Get track analysis focus rect
    virtual rcRect getAnalysisRect( void ) = 0;

    // static name-to-type mapper
    static rcTrackType type( const std::string& name ) {
        // TODO: replace linear search with something faster
        for ( uint32 i = 0; i < eMaxTrack; i++ )
            if ( cTrackTypeNames[i] == name )
                return static_cast<rcTrackType>(i);
        return eMaxTrack;
    };

    // static type-to-name mapper
    static const std::string& name( rcTrackType type ) {
        rmAssert( type <= eMaxTrack );
        rmAssert( type < static_cast<int>(rmDim(cTrackTypeNames)) );
        return cTrackTypeNames[type];
    }
};

/******************************************************************************
*	rcScalarTrack interface definition.
*
*	This is the interface implemented by a data track containing
*	scalar data.
******************************************************************************/

class rcScalarTrack
{
public:
    enum { ePersistenceVersion = 1 };

    // virtual dtor is required
    virtual ~rcScalarTrack() { };

	// get the minimum value (hint) the engine expected to
	//	generate for this track.
	virtual double getExpectedMinimumValue( void ) = 0;

	// get the maximum value (hint) the engine expected to
	//	generate for this track.
	virtual double getExpectedMaximumValue( void ) = 0;

	// get the current minimum value written to this track.
	virtual double getCurrentMinimumValue( void ) = 0;

	// get the current maximum value written to this track.
	virtual double getCurrentMaximumValue( void ) = 0;

	// get an iterator for the track initialized to the
	//	specified position.  It is the responsibility of
	//	the caller to release the iterator
	virtual rcScalarSegmentIterator* getDataSegmentIterator( const rcTimestamp& pos ) = 0;

    // get the total number of data segments.
    virtual uint32 getSegmentCount( void ) = 0;
};

/******************************************************************************
 *   rcVideoTrack interface definition.
 *
 *   This is the interface implemented by a data track containing
 *   video data.
 ******************************************************************************/

class rcVideoTrack  {
  public:
    // virtual dtor is required
    virtual ~rcVideoTrack() { };

    // get an iterator for the track initialized to the
    //  specified position.  It is the responsibility of
    //  the caller to release the iterator
    virtual rcVideoSegmentIterator* getDataSegmentIterator( const rcTimestamp& pos ) = 0;

    // get the total number of data segments.
    virtual uint32 getSegmentCount( void ) = 0;
};

/******************************************************************************
 *   rcGraphicsTrack interface definition.
 *
 *   This is the interface implemented by a data track containing
 *   graphics data.
 ******************************************************************************/

class rcGraphicsTrack  {
  public:
    // Version number for persistence
    enum { ePersistenceVersion = 1 };

    // virtual dtor is required
    virtual ~rcGraphicsTrack() { };

    // get an iterator for the track initialized to the
    //  specified position.  It is the responsibility of
    //  the caller to release the iterator
    virtual rcGraphicsSegmentIterator* getDataSegmentIterator( const rcTimestamp& pos ) = 0;

    // get the total number of data segments.
    virtual uint32 getSegmentCount( void ) = 0;
};

/******************************************************************************
 *   rcPositionTrack interface definition.
 *
 *   This is the interface implemented by a data track containing
 *   floating point position coordinate [x,y] data.
 ******************************************************************************/

class rcPositionTrack  {
  public:
    // Version number for persistence
    enum { ePersistenceVersion = 1 };

    // virtual dtor is required
    virtual ~rcPositionTrack() { };

    // Note: minimum and maximum values for x and y are recorded and tested separately.

    // get the minimum value (hint) the engine expected to
	// generate for this track.
	virtual rcFPair getExpectedMinimumValue( void ) = 0;

	// get the maximum value (hint) the engine expected to
	// generate for this track.
	virtual rcFPair getExpectedMaximumValue( void ) = 0;

	// get the current minimum value written to this track.
	virtual rcFPair getCurrentMinimumValue( void ) = 0;

	// get the current maximum value written to this track.
	virtual rcFPair getCurrentMaximumValue( void ) = 0;

    // get an iterator for the track initialized to the
    // specified position.  It is the responsibility of
    // the caller to release the iterator
    virtual rcPositionSegmentIterator* getDataSegmentIterator( const rcTimestamp& pos ) = 0;

    // get the total number of data segments.
    virtual uint32 getSegmentCount( void ) = 0;
};

/******************************************************************************
 *   rcSegmentIterator interface definition.
 *
 *   This is the interface implemented by a data track containing
 *   any data.
 ******************************************************************************/

class rcSegmentIterator
{
  public:
    // virtual dtor is required
    virtual ~rcSegmentIterator() {};

    // get the total length of the current segment.
    virtual rcTimestamp getSegmentLength( void ) = 0;

    // get our position relative to the beginning of the current segment.
    virtual rcTimestamp getSegmentOffset( void ) = 0;

    // get our position relative to the beginning of the track.
    virtual rcTimestamp getTrackOffset( void ) = 0;

    // get our current segment index (-1 denotes undefined segment)
    virtual int32 getSegmentIndex( void ) = 0;

    // get our current relative segment start time
    virtual rcTimestamp getSegmentStart( void ) = 0;

    // get our current absolute segment start time
    virtual rcTimestamp getSegmentStartAbsolute( void ) = 0;

    // are we at the last segment?
    virtual bool hasNextSegment( bool lock = true ) = 0;

    // get the total number of segments.
    virtual uint32 getSegmentCount( void ) = 0;

    // does current segment contain this timestamp?
    virtual bool contains( const rcTimestamp& time ) = 0;

    // get the focus rect at the current position.
    virtual rcRect getFocus( void ) = 0;

    // advance the iterator by the specified time amount.
    virtual void advance( const rcTimestamp& amount, bool lock = true ) = 0;

    // advance the iterator by offset segments, count can be positive or negative
    // Return true if a valid segment can be returned, false otherwise
    virtual bool advance( int32 offset, bool lock = true ) = 0;

    // lock iterator (can be done before high-frequency calls to advance(lock=false) )
    virtual void lock() = 0;

    // unlock iterator
    virtual void unlock() = 0;

    // reset iterator back to time 0.0
    virtual void reset( bool lock = true ) = 0;

    // release this iterator instance
    virtual void release( void ) = 0;
};


/******************************************************************************
 *   rcVideoSegmentIterator interface definition.
 *
 *   This is the interface implemented by a data track containing
 *   video data.
 ******************************************************************************/

class rcVideoSegmentIterator : public rcSegmentIterator
{
  public:
    // virtual dtor is required
    virtual ~rcVideoSegmentIterator() {};

    // get the value at the current position.
    virtual rcWindow getValue( void ) = 0;
};

/******************************************************************************
*	rcScalarSegmentIterator interface definition.
*
*	This is the interface implemented by a data track containing
*	scalar data.
******************************************************************************/

class rcScalarSegmentIterator : public rcSegmentIterator
{
public:
    // virtual dtor is required
    virtual ~rcScalarSegmentIterator() {};

	// get the value at the current position.
	virtual double getValue( void ) = 0;
};

/******************************************************************************
*	rcGraphicsSegmentIterator interface definition.
*
*	This is the interface implemented by a data track containing
*	graphics data for visualization.
******************************************************************************/

class rcGraphicsSegmentIterator : public rcSegmentIterator
{
public:
    // virtual dtor is required
    virtual ~rcGraphicsSegmentIterator() {};

	// get the graphics collection at the current position.
	virtual rcVisualGraphicsCollection getValue( void ) = 0;

    // get the count of graphics objects at the current position.
	virtual uint32 getCount( void ) = 0;
};

/******************************************************************************
*	rcPositionSegmentIterator interface definition.
*
*	This is the interface implemented by a data track containing
*	floating point position coordinate data
******************************************************************************/

class rcPositionSegmentIterator : public rcSegmentIterator
{
public:
    // virtual dtor is required
    virtual ~rcPositionSegmentIterator() {};

	// get the position coordinate [x,y] value
	virtual rcFPair getValue( void ) = 0;
};

// Stream operators for debugging display
ostream& operator << ( ostream& os, rcSegmentIterator& i );

#endif // _rcMODEL_H_
