/******************************************************************************
*	rc_experimentdomainimpl.h
*
*	This file contains the declaration for the implementation of
*	the rcExperimentDomain interface.
******************************************************************************/

#ifndef rcEXPERIMENTDOMAINIMPL_H
#define rcEXPERIMENTDOMAINIMPL_H

#include <rc_engine.h>
#include <rc_model.h>
#include <rc_framegrabber.h>

#include <rc_atomic.h>

class rcExperimentImpl;

class rcExperimentDomainImpl : public rcExperimentDomain, public rcEngineObserver, public rcCarbonLock
{
public:
	rcExperimentDomainImpl( void );

	//////////////// rcExperimentDomain implementation ////////////////

	// initialize the experiment domain and pass in an observer
	//	so the model can communicate asynchronous events to the
	//	domain client.
	virtual void initialize( rcExperimentObserver* observer );

	// shutdown the experiment domain
	virtual void shutdown( void );

	// get the current experiment.
	virtual rcExperiment* getExperiment( void );

	// create a new experiment and make it current.
	virtual void newExperiment( void );

    // load an experiment from a tree and make it current.
    // return number of errors
	virtual int loadExperiment( const char* fileName,
                                const rcXMLElementTree& tree,
                                rcExperimentImportMode mode,
                                rcProgressIndicator* progress = NULL);
    
	// save the experiment to disk
    // returns errno
	virtual int saveExperiment( const char* fileName,
                                rcExperimentFileFormat format,
                                rcProgressIndicator* progress = NULL );

	// get the experiment's status (see rcExperimentState enum definition above)
	virtual rcExperimentState getExperimentState( void );

    // get the experiment's attributes (see rcExperimentAttributes
    //  struct definition above)
    virtual const rcExperimentAttributes getExperimentAttributes( void );

  // get the engine live attributes 
  virtual bool operatingOnLiveInput ( void );
  virtual bool storingLiveInput ( void );

    // set current experiment attributes
    virtual void setExperimentAttributes( const rcExperimentAttributes& attr );
    
	// get the current running rcTimestamp
	virtual rcTimestamp getExperimentLength( void );

	// return the current cursor time
	virtual rcTimestamp getCursorTime( void );

	// Called when cursor time is updated from the GUI.
	virtual void notifyCursorTime( const rcTimestamp& time );

    // Called when movie has been imported
	virtual void notifyMovieImport();
    
    // get the absolute experiment start rcTimestamp
	virtual rcTimestamp getExperimentStart( void );
    
	// start the experiment
	virtual void startExperiment( void );

	// stop the experiment
	virtual void stopExperiment( void );

    // process user-selected focs area of the experiment
    virtual void processExperiment( void );

    // Pause a Tracking Experiment
    virtual bool pauseTrackingExperiment( void );

    // stop pause a Tracking Experiment
    virtual bool doneSelecting ( void );

    
	//////////////// rcEngineObserver implementation ////////////////

	// create a new writer
    virtual rcWriterGroup* createWriterGroup( const char* tag ,
                                              const char* name ,
                                              const char* description,
                                              rcGroupSemantics type );

	// called by the engine to notify us that an error occurred.
	virtual void notifyError( const char* errorString );

	// called by the engine to warn us of some condition.
	virtual void notifyWarning( const char* warnString );

	// called by the engine to send status to us
	virtual void notifyStatus( const char* statusString );

	// called by the engine to notify us of a state change.
	virtual void notifyState( rcEngineState newState );

	// called periodically while running to notify observer
	//	of the current processing time.
	virtual void notifyTime( const rcTimestamp& timestamp );

	// Returns the number of queued up eNotifyCursorEvent events
	virtual int32 timeEventCount() const;

    // called to notify observer of a timeline range change
    virtual void notifyTimelineRange( const rcTimestamp& start ,
                                      const rcTimestamp& end );

    // called to notify observer of a analysis focus rect change.
    virtual void notifyAnalysisRect( const rcRect& rect );

    // called to notify observer of a analysis focus rect rotation change
    // angle is in degrees
    virtual void notifyAnalysisRectRotation( const rcAffineRectangle& affine);
    
    // called to notify observer of a change in multiplier
    virtual void notifyMultiplier( const double& multiplier );

	// called by the engine to ask us if it should blast an image to
	//	a part of the screen of our choosing.
	virtual bool acceptingImageBlits( void );

    // if the observer is accepting image blits, this is called to
    // tell the observer to blit the image
    virtual void notifyBlitData( const rcWindow* image );

    // if the observer is accepting image blits, this is called to
    // tell the observer to blit the graphics
    virtual void notifyBlitGraphics( const rcVisualGraphicsCollection* graphics );
    
    // Carbon lock
    virtual void notifyLockApp( bool lock );

    // called to request experiment save from observer
	virtual int notifyRequestSave( const char* fileName,
                                   rcExperimentFileFormat format );

    // called to notify observer of the current video frame size
	virtual void notifyVideoRect( const rcRect& rect );

    // Experiment has changed, notify all widgets to update themselves
    virtual void notifyExperimentChange( void );

	// called to ask the observer if it accepts
    // put polys calls
    virtual bool acceptingPolys ( void );
    
	// if the observer is accepting polys, this is called to
    // tell the observer to put the polys
	virtual void notifyPolys ( const rcPolygonGroupRef * polys );

	// if the observer is accepting polys, this is called to
    // tell the observer to put the polys
	virtual void getPolys ( rcPolygonGroupRef& polys );

	// if the observer is accepting polys, this is called to
    // tell the observer to put the polys
	virtual void notifyPlotRequest ( const CurveData * );
    

    //////////////// rcCarbonLock implementation ////////////////
    
    virtual void lock();
    virtual void unlock();
    
private:

	// internal state update
	void setInternalState( rcExperimentState newState );

	rcEngine*				_engine;
	rcExperimentObserver*	_observer;
	rcExperimentImpl*		_experiment;
	rcExperimentState		_state;
	rcTimestamp				_length;
	rcAtomicValue<rcTimestamp>      _cursorTime;
    rcTimestamp             _start;
};

#endif // rcEXPERIMENTDOMAINIMPL_H
