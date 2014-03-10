/******************************************************************************
*	rc_engine.h
*
*	This file contains the definitions for the pure virtual classes
*	that constitute the interface contract between the model and
*	the engine application layers.
******************************************************************************/

#ifndef _rcENGINE_H_
#define _rcENGINE_H_

#include <exception>

#include <rc_window.h>
#include <rc_pair.h>
#include <rc_graphics.h>
#include <rc_affine.h>
#include <rc_polygongroup.h>

#if WIN32
using namespace std;
#endif

#include <rc_uitypes.h>
#include <rc_value.h>
#include <rc_timestamp.h>
#include <rc_setting.h>
#include <rc_resource_ctrl.h>
#include <rc_persistencemanager.h>
#include <rc_typedefs.h>
#include <rc_polygon.h>
#include <lightplot2d.h>  // For Plotter Data
#include "lpwidget.h"

class rcSharedFrameBufPtr;

/******************************************************************************
*	Forwards
******************************************************************************/

class rcWindow;
class rcEngine;
class rcEngineObserver;
class rcWriterGroup;
class rcWriter;


/******************************************************************************
*	Constants
******************************************************************************/

// engine state values returned by 'rcEngine::getState()'.
enum rcEngineState
{
    eEngineUninitialized = 0  // engine has not been initialized
    , eEngineStopped        // engine is stopped and not generating data
    , eEngineRunning        // engine is running and generating data
    , eEnginePlayback       // engine is playing back data
    , eEngineProcessing     // engine is processing a focus area
    , eEngineShuttingDown   // engine is in the process of shutting down
};

// types of data writers that the engine can create
enum rcWriterType
{
    eVideoWriter = 0    // writer accepts video data from engine
  , eScalarWriter       // writer accepts scalar values from engine
  , ePositionWriter     // writer accepts position values from engine
  , eGraphicsWriter     // writer accepts graphical values from engine
};

/******************************************************************************
*	Exceptions
******************************************************************************/

class rcEngineException : public general_exception
{
public:
	rcEngineException( const std::string& msg ) : general_exception( msg ) {}
};

/******************************************************************************
*	rcEngineAttributes struct definition
*
*	A rcEngineAttributes struct is returned by the 'rcEngine::getAttributes()
*	to describe the engine's current (read-only) attributes.
******************************************************************************/

class rcEngineAttributes
{
  public:
  int		 frameWidth;	// get the width of the input source frame
  int		 frameHeight;	// get the height of the input source frame
  int      frameDepth;    // get the depth of input source frame, bytes per pixel
  bool	 liveInput;		// returns true if the input source is real-time.
  bool     liveStorage;   // returns true if the input is being stored to disk
  std::string inputName;     // returns input image file name if input source is a file
                            // and camera name if input source is a camera
};

/******************************************************************************
*	rcEngineFactory class definition
*
*	The rcEngineFactory class consists of a single static method to
*	get the linked-in (statically or dynamically) implementation of
*	the rcEngine interface.
******************************************************************************/

class rcEngineFactory
{
public:
	static rcEngine* getEngine( const char* implKey = 0 );
};

/******************************************************************************
*	rcEngine interface definition
*
*	This defines the interface to the singleton engine object (there
*	is only one engine per application/process).
*
*	The constructor for 'myEngineImpl' probably
*	shouldn't do too much; all initialization should
*	happen in the 'initialize' method.
*
*	The 'getNSettingCategories' and 'getSettingCategory' methods
*	allow the engine to specify different categories of settings
*	that can be controlled from the UI.
*
******************************************************************************/

class rcEngine
{
  public:
    // virtual dtor is required
    virtual ~rcEngine() { };

	// initialize the engine
	virtual void initialize( rcEngineObserver* observer ) = 0;

	// shut down the engine
	virtual void shutdown( void ) = 0;

	// get the number of engine settings categories
	virtual int getNSettingCategories( void ) = 0;

	// get a settings category
	virtual rcSettingCategory getSettingCategory( int categoryNo ) = 0;

	// get the current running rcTimestamp
	virtual rcTimestamp getElapsedTime( void ) = 0;

    // get the absolute start rcTimestamp
	virtual rcTimestamp getStartTime( void ) = 0;

	// get the engine state (see rcEngineState enum definition above)
	virtual rcEngineState getState( void ) = 0;

	// get the engine attributes (see rcEngineAttributes enum definition above)
	virtual const rcEngineAttributes getAttributes( void ) = 0;

	// get the engine live attributes
	virtual bool operatingOnLiveInput ( void ) = 0;
	virtual bool storingLiveInput ( void ) = 0;

	// start the engine
	virtual void start( void ) = 0;

	// stop the engine
	virtual void stop( void ) = 0;

	// reset the engine so it can be restarted later
	// all attributes will be reset to default values
	virtual void reset( void ) = 0;

	// process captured data within user-selected focus area.
	// can be called repeatedly.
	virtual void process( void ) = 0;

	// Pause Tracking for user selection edits
        // returns true
	virtual bool pauseTracking ( void ) = 0;

	// Continue
        // returns false
	virtual bool resumeTracking ( void ) = 0;

    virtual void notifyEngineOfPolys ( const rcPolygonGroupRef * polys ) = 0;

};

/******************************************************************************
*	rcEngineObserver interface definition
*
*	An instance of this is passed to rcEngine::initialize() to allow
*	the engine to communicate to the model layer.
*
*	The 'createWriter' method is used by the engine to create a
*	writer for a particular kind of timestamped data that it
*	wants to generate and have displayed/stored.
*
*	The 'notifyXXX' methods allow the engine to asynchronously
*	notify the model layer of events.
*
*	The 'acceptingImageBlits' and 'setBlitData' methods support
*	the direct (asynchronous) blasting of video frames to the
*	monitor portion of the UI.
******************************************************************************/

class rcEngineObserver
{
public:
    // virtual dtor is required
    virtual ~rcEngineObserver() { };

	// create a new writer group
	virtual rcWriterGroup* createWriterGroup( const char* tag ,
									          const char* name ,
                                              const char* description,
                                              rcGroupSemantics type ) = 0;

    // called to ask the observer if it accepts
    // notifyBlitdDta calls
    virtual bool acceptingImageBlits( void ) = 0;

	// called to notify the observer that an error occurred.
	virtual void notifyError( const char* errorString ) = 0;

	// called to warn the observer of some condition.
	virtual void notifyWarning( const char* warnString ) = 0;

	// called to send status to the observer
	virtual void notifyStatus( const char* statusString ) = 0;

	// called to notify observer of a state change.
	virtual void notifyState( rcEngineState newState ) = 0;

	// called periodically while running to notify observer
	// of the current processing time.
	virtual void notifyTime( const rcTimestamp& timestamp ) = 0;

	// Returns the number of queued up eNotifyCursorEvent events
	virtual int32 timeEventCount() const = 0;

    // called to notify observer of a timeline range change
    virtual void notifyTimelineRange( const rcTimestamp& start ,
                                      const rcTimestamp& end ) = 0;

    // called to notify observer of a analysis focus rect change.
    virtual void notifyAnalysisRect( const rcRect& rect ) = 0;

    // called to notify observer of a analysis focus rect rotation change
    // angle is in degrees
    virtual void notifyAnalysisRectRotation( const rcAffineRectangle& ) = 0;

    // called to notify observer of a change in multiplier
    virtual void notifyMultiplier( const double& multiplier ) = 0;

    // if the observer is accepting image blits, this is called to
    // tell the observer to blit the image
    virtual void notifyBlitData( const rcWindow* image ) = 0;

    // if the observer is accepting image blits, this is called to
    // tell the observer to blit the graphics
    virtual void notifyBlitGraphics( const rcVisualGraphicsCollection* graphics ) = 0;

    // Carbon locking
    virtual void notifyLockApp( bool lock ) = 0;

    // called to request experiment save from observer
    // returns errno
	virtual int notifyRequestSave( const char* fileName,
                                   rcExperimentFileFormat format ) = 0;

    // called to notify observer of the current video frame size
	virtual void notifyVideoRect( const rcRect& rect ) = 0;

    // called to notify that experiment has changed and widgets need to be updated
    virtual void notifyExperimentChange() = 0;

    // return the current cursor time
    virtual rcTimestamp getCursorTime( void ) = 0;

    // get current experiment attributes
    virtual const rcExperimentAttributes getExperimentAttributes( void ) = 0;

    // set current experiment attributes
    virtual void setExperimentAttributes( const rcExperimentAttributes& attr ) = 0;

	// called to ask the observer if it accepts
    // put polys calls
      virtual bool acceptingPolys ( void ) = 0;

	// if the observer is accepting polys, this is called to
    // tell the observer to put the polys
    virtual void notifyPolys ( const rcPolygonGroupRef * polys ) = 0;

    // if the observer is accepting polys, this is called to
    // tell the observer to put the polys
    virtual void notifyPlotRequest ( SharedCurveDataRef& ) = 0;
    
    // if the observer is accepting polys, this is called to
    // tell the observer to put the polys
    virtual void notifyPlot2dRequest ( SharedCurveData2dRef& ) = 0;
    
    
	// if the observer is accepting polys, this is called to
    // tell the observer to put the polys
    virtual void getPolys ( rcPolygonGroupRef& polys ) = 0;

  // called to tell the observer if it should get the
    //	modified polys
  //    virtual bool doneSelecting ( void ) = 0;

};

/******************************************************************************
*	rcWriterGroup interface definitions
*
*	The engine creates one or more writer groups for each set of parameters.
******************************************************************************/

class rcWriterGroup
{
public:
    // virtual dtor is required
    virtual ~rcWriterGroup() { };

    // create a new writer
    virtual rcWriter* createWriter( rcWriterType type , const char* tag ,
                                    const char* name , const char* description,
                                    const char* formatString,
                                    uint32 sizeLimit, const rcRect& analysisRect ) = 0;

    // release this writer group and all writers within the group.
    virtual void release() = 0;
};

/******************************************************************************
*	rcWriter interface definitions
*
*	The engine creates a separate writer for each parameter (or
*	video stream) that it extracts from the captured data.
*
*	The writers are implemented within the model layer; the engine
*	uses the rcEngineObserver instance that was passed down during
*	rcEngine::initialize() to create a writer.
******************************************************************************/

// base writer interface
class rcWriter
{
public:
    // virtual dtor is required
    virtual ~rcWriter() { };

    // Accessors

	// get the writer type (see rcWriterType enum defined above).
	virtual rcWriterType getWriterType( void ) = 0;

	// get the meta data tag
	virtual const char* getTag( void ) = 0;

	// get the name of the track this writer will generate
	virtual const char* getName( void ) = 0;

	// get the description of the track this writer will generate
	virtual const char* getDescription( void ) = 0;

    // get track start time (absolute time of first segment)
    virtual rcTimestamp getTrackStart( void ) = 0;

    // Mutators

    // set absolute track start time
    virtual void setTrackStart( const rcTimestamp& time ) = 0;

    // flush any data that might be cached
    virtual void flush( void ) = 0;

    // Data semantics, what does it represent
    virtual void setSemantics( const rcTrackSemantics& s ) = 0;

    // Set exportability
    virtual void setExportable( bool e ) = 0;

    // Set the description text
	virtual void setDescription( const char* text ) = 0;

    // Set the name text
	virtual void setName( const char* text ) = 0;

    // Set track analysis focus rect
    virtual void setAnalysisRect( const rcRect& f ) = 0;
};

// writer for video data
class rcVideoWriter
{
public:
    // virtual dtor is required
    virtual ~rcVideoWriter() { };

	// write a new video frame with timestamp.
	virtual void writeValue( const rcTimestamp& timestamp , const rcRect& focus, const rcWindow* frame ) = 0;

    // flush any data that might be cached
    virtual void flush( void ) = 0;

    // Down cast to rcWriter*
    virtual rcWriter* down_cast() = 0;
};

// writer for scalar data
class rcScalarWriter
{
public:
    // virtual dtor is required
    virtual ~rcScalarWriter() { };

	// set the expected minimum value for this scalar data
	virtual void setExpectedMinValue( const double& minValue ) = 0;

	// set the expected maximum value for this scalar data
	virtual void setExpectedMaxValue( const double& maxValue ) = 0;

	// write a new scalar value with timestamp.
	virtual void writeValue( const rcTimestamp& timestamp, const rcRect& focus, const double& value ) = 0;

    // flush any data that might be cached
    virtual void flush( void ) = 0;

    // Remove all segments, reset start time
    virtual void reset() = 0;

    // Down cast to rcWriter*
    virtual rcWriter* down_cast() = 0;
};

// writer for coordinate position data
class rcPositionWriter
{
public:
    // virtual dtor is required
    virtual ~rcPositionWriter() { };

    // Note: minimum and maximum values for x and y are recorded and tested separately.

    // set the expected minimum value for this 2D data
	virtual void setExpectedMinValue( const rcFPair& minValue ) = 0;

	// set the expected maximum value for this 2D data
	virtual void setExpectedMaxValue( const rcFPair& maxValue ) = 0;

	// write a new position <x,y> value with timestamp.
	virtual void writeValue( const rcTimestamp& timestamp , const rcRect& focus, const rcFPair& position ) = 0;

    // flush any data that might be cached
    virtual void flush( void ) = 0;

    // Down cast to rcWriter*
    virtual rcWriter* down_cast() = 0;
};

// writer for line graphics data
class rcGraphicsWriter
{
public:
    // virtual dtor is required
    virtual ~rcGraphicsWriter() { };

	// write a new graphics object with timestamp
	virtual void writeValue( const rcTimestamp& timestamp, const rcRect& focus, const rcVisualGraphicsCollection& graphics ) = 0;

    // flush any data that might be cached
    virtual void flush( void ) = 0;

    // Down cast to rcWriter*
    virtual rcWriter* down_cast() = 0;
};

#endif // _rcENGINE_H_
