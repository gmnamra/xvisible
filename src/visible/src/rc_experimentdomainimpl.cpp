// @file rc_experimentdomainimpl.cpp

#include <rc_window.h>
#include <rc_fileutils.h>
#include "rc_experimentimpl.h"
#include "rc_experimentdomainimpl.h"
#include <rc_uitypes.h>

#if WIN32
using namespace std;
#endif

//////////////// rcExperimentDomain implementation ////////////////

rcExperimentDomainImpl::rcExperimentDomainImpl( void )
  : _cursorTime(0.0)
{
	_observer = 0;
	_experiment = 0;
	_engine = 0;
	_state = eExperimentEmpty;
	_length = cZeroTime;
}

// initialize the experiment domain and pass in an observer
//	so the model can communicate asynchronous events to the
//	domain client.
void rcExperimentDomainImpl::initialize( rcExperimentObserver* observer )
{
	_engine = rcEngineFactory::getEngine();
	_observer = observer;

	_engine->initialize( this );

	_experiment = new rcExperimentImpl();
}

// shutdown the experiment domain
void rcExperimentDomainImpl::shutdown( void )
{
	_engine->shutdown();
}

// get the current experiment.
rcExperiment* rcExperimentDomainImpl::getExperiment( void )
{
	return _experiment;
}

// create a new experiment and make it current.
void rcExperimentDomainImpl::newExperiment( void )
{
    rcExperimentImpl* oldExperiment = _experiment;
	_experiment = new rcExperimentImpl();
	_state = eExperimentEmpty;
	_length = cZeroTime;
    _engine->reset();
    _observer->notifyState( _state, true ); // Immediate signal dispatch
    _observer->notifyTime( _length );
    // Delete old experiment last so that GUI notifications
    // do no access deleted settings
    if ( oldExperiment )
        delete oldExperiment;
}

// load an experiment from a tree and make it current.
// return number of errors
int rcExperimentDomainImpl::loadExperiment( const char* fileName,
                                            const rcXMLElementTree& tree,
                                            rcExperimentImportMode mode,
                                            rcProgressIndicator* progress )
{
    int errors = 1;
    // Reset everything
    newExperiment();
    
    if ( _experiment ) {
        // Import XML tree into experiment
        errors = _experiment->loadExperiment( tree, mode, this, progress );
        
        if ( errors > 0 ) {
            // We may be in an inconsistent state
            // Reset everything
            newExperiment();
        } else {
            rcValue value( fileName );
            // Set actual XML file name that was loaded
            _experiment->setSettingValue( cFileNameSettingId, value );

            _state = eExperimentLocked;
            _observer->notifyState( _state );
        }
    }

    return errors;
}

// save the experiment to disk
int rcExperimentDomainImpl::saveExperiment( const char* fileName,
                                            rcExperimentFileFormat format,
                                            rcProgressIndicator* progress )
{
    if (_state == eExperimentEmpty)
      rmExceptionMacro (<< "experiment not running, there is nothing to save" );

    rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
    const std::string creator = persistenceManager->generatorComment();
    
    rcMovieConverterOptions opt;

    opt.creator( creator );
    
    int nPages = _experiment->getNSettingCategories();
	for (int i = 0; i < nPages; i++)
	{
		rcSettingCategory category = _experiment->getSettingCategory( i );

        // Look for analysis settings
        if ( !strcmp( category.getTag(), "analysis-settings" ) ) {
            const rcSettingInfo rect = category.getSettingInfo( "analysis-rect" );
            const rcSettingInfo firstFrame = category.getSettingInfo( "analysis-first-frame" );
            const rcSettingInfo lastFrame = category.getSettingInfo( "analysis-last-frame" );
            
            rcRect analysisRect = rect.getValue();
            // Hack: use analysis first and last frame to specify whoch frames to export
            int32 first = firstFrame.getValue();
            int32 last = lastFrame.getValue();

            opt.frameCount( last - first + 1 );
            opt.firstFrameIndex( first );
            if ( analysisRect.width() > 0 && analysisRect.height() > 0 )
                opt.clipRect( analysisRect );

        }
    }
	int err = _experiment->saveTo( fileName, format, opt, this, progress );
    _state = eExperimentLocked;
    return err;
}

// get the experiment's status (see rcExperimentState enum definition above)
rcExperimentState rcExperimentDomainImpl::getExperimentState( void )
{
	return _state;
}

	// get the engine live attributes 
bool rcExperimentDomainImpl::operatingOnLiveInput ( void )
{
  return _engine->operatingOnLiveInput ();
}

bool rcExperimentDomainImpl::storingLiveInput ( void )
{
  return _engine->storingLiveInput ();
}


// get the experiment's attributes (see rcExperimentAttributes
//  struct definition above)
const rcExperimentAttributes rcExperimentDomainImpl::getExperimentAttributes( void )
{
    rcExperimentAttributes attrs;
    rcEngineAttributes engineAttrs = _engine->getAttributes();
    
    attrs.frameWidth    = engineAttrs.frameWidth;
    attrs.frameHeight   = engineAttrs.frameHeight;
    attrs.frameDepth    = engineAttrs.frameDepth;
    attrs.liveInput     = engineAttrs.liveInput;
    attrs.liveStorage   = engineAttrs.liveStorage;
    attrs.inputName     = engineAttrs.inputName;
	attrs.title         = (std::string) _experiment->getSettingValue(cTitleSettingId);
	attrs.fileName      = (std::string) _experiment->getSettingValue(cFileNameSettingId);
    attrs.userName      = (std::string) _experiment->getSettingValue(cUserNameSettingId);
    attrs.comments      =(std::string) _experiment->getSettingValue(cCommentsSettingId);
    attrs.lensMag       = (std::string)_experiment->getSettingValue(cMicroscopeMagnificationSettingId);
    attrs.otherMag      = (std::string)_experiment->getSettingValue(cOtherMagnificationSettingId);
    attrs.imagingMode   = (std::string)_experiment->getSettingValue(cImagingModeSettingId);
    attrs.cellType      = (std::string)_experiment->getSettingValue(cCellTypeSettingId);
    attrs.treatment1    = (std::string)_experiment->getSettingValue(cCellTreatmentSettingId);
    attrs.treatment2    = (std::string)_experiment->getSettingValue(cCellTreatment2SettingId);
    attrs.temperature   = (std::string)_experiment->getSettingValue(cCellTemperatureSettingId);
    attrs.CO2           = (std::string)_experiment->getSettingValue(cCellCO2SettingId);
    attrs.O2            = (std::string)_experiment->getSettingValue(cCellO2SettingId);
    
    return attrs;
}

// set current experiment attributes
void rcExperimentDomainImpl::setExperimentAttributes( const rcExperimentAttributes& attrs )
{
    _experiment->setSettingValue( cTitleSettingId, rcValue(attrs.title) );
    _experiment->setSettingValue( cUserNameSettingId, rcValue(attrs.userName) );
    _experiment->setSettingValue( cCommentsSettingId, rcValue(attrs.comments) );
    _experiment->setSettingValue( cMicroscopeMagnificationSettingId, rcValue(attrs.lensMag) );
    _experiment->setSettingValue( cOtherMagnificationSettingId, rcValue(attrs.otherMag) );
    _experiment->setSettingValue( cImagingModeSettingId, rcValue(attrs.imagingMode) );
    _experiment->setSettingValue( cCellTypeSettingId, rcValue(attrs.cellType) );
    _experiment->setSettingValue( cCellTreatmentSettingId, rcValue(attrs.treatment1) );
    _experiment->setSettingValue( cCellTreatment2SettingId, rcValue(attrs.treatment2) );
    _experiment->setSettingValue( cCellTemperatureSettingId, rcValue(attrs.temperature) );
    _experiment->setSettingValue( cCellCO2SettingId, rcValue(attrs.CO2) );
    _experiment->setSettingValue( cCellO2SettingId, rcValue(attrs.O2) );
}


// get the current running rcTimestamp
rcTimestamp rcExperimentDomainImpl::getExperimentLength( void )
{
	return _engine->getElapsedTime();
}

// return the current cursor time
rcTimestamp rcExperimentDomainImpl::getCursorTime( void )
{
  rcTimestamp x;
  
  return _cursorTime.getValue(x);
}

// Called when cursor time is updated from the GUI.
void rcExperimentDomainImpl::notifyCursorTime( const rcTimestamp& time )
{
  _cursorTime.setValue(time);
}


// Called when movie has been imported
void rcExperimentDomainImpl::notifyMovieImport()
{
    if ( _experiment ) {
        std::string fileName = _engine->getAttributes().inputName;
        
        // Get file name without preceding path
        std::string rawFileName = rfStripPath( fileName );
        std::string strippedName = rfStripExtension( rawFileName );
        // Use file name as default title
        _experiment->setSettingValue( cTitleSettingId, rcValue( strippedName ) );
    }
}

// get the absolute experiment start rcTimestamp
rcTimestamp rcExperimentDomainImpl::getExperimentStart( void )
{
    return _engine->getStartTime();
}

// start the experiment
void rcExperimentDomainImpl::startExperiment( void )
{
    if (_state == eExperimentLocked)
      rmExceptionMacro (<< "Experiment is locked");

    if (_state != eExperimentEmpty)
      rmExceptionMacro (<< "Experiment is not empty");

	_engine->start();
}

// stop the experiment
void rcExperimentDomainImpl::stopExperiment( void )
{
    if (_state == eExperimentEmpty)
      rmExceptionMacro ( << "experiment has not run yet" );
	if (_state != eExperimentRunning)
	  rmExceptionMacro ( << "experiment has already stopped" );
	_engine->stop();
}

// process the experiment data for user-selected focus area
void rcExperimentDomainImpl::processExperiment( void )
{
    if (_state == eExperimentEmpty) {
        _engine->process();
    }
    else {
        if (_state == eExperimentEmpty)
	  rmExceptionMacro ( << "experiment has not started yet" );
        _engine->process();
    }
}

// Pause a tracking experiment if it wants to
bool rcExperimentDomainImpl::pauseTrackingExperiment( void )
{
    if (_state == eExperimentLocked)
      rmExceptionMacro (<< "Experiment is locked");

    return _engine->pauseTracking ();
}

// stop pausing a tracking experiment if it wants to
bool rcExperimentDomainImpl::doneSelecting ( void )
{
    if (_state == eExperimentLocked)
      rmExceptionMacro (<< "Experiment is locked");

    return _engine->resumeTracking ();
}


//////////////// rcEngineObserver implementation ////////////////

// create a new writer group
rcWriterGroup* rcExperimentDomainImpl::createWriterGroup( const char* tag ,
                                                          const char* name ,
                                                          const char* description,
                                                          rcGroupSemantics type )
{
	rcTrackGroupImpl* trackGroup = _experiment->createTrackGroup( tag , name , description, type );
	rcWriterGroup* writerGroup = dynamic_cast<rcWriterGroup*>( trackGroup );
	return writerGroup;
}

// called by the engine to notify us that an error occurred.
void rcExperimentDomainImpl::notifyError( const char* errorString )
{
	_observer->notifyError( errorString );
}

// called by the engine to warn us of some condition.
void rcExperimentDomainImpl::notifyWarning( const char* warnString )
{
	_observer->notifyWarning( warnString );
}

// called by the engine to send status to us
void rcExperimentDomainImpl::notifyStatus( const char* statusString )
{
	_observer->notifyStatus( statusString );
}

// called by the engine to notify us of a state change.
void rcExperimentDomainImpl::notifyState( rcEngineState engineState )
{
	rcExperimentState newState = _state;
    
	switch (engineState)
	{
        case eEngineUninitialized:
            if ( _experiment )
                _experiment->editableSettings( true );
            break;
            
        case eEngineRunning:		// engine is running and generating data
            newState = eExperimentRunning;
            if ( _experiment )
                _experiment->editableSettings( false );
            break;
            
        case eEnginePlayback:
            newState = eExperimentPlayback;
            break;
            
        case eEngineStopped:		// engine is stopped and not generating data
            // If it's locked already, keep it locked
            if ( _state == eExperimentLocked )
                newState = eExperimentLocked;
            else
                newState = eExperimentEnded;
            if ( _experiment )
                _experiment->editableSettings( false );
            break;
            
        case eEngineShuttingDown:	// engine is in the process of shutting down
            _experiment->editableSettings( false );
            break;
            
        default:
            return;		// ignore.
	}

    if ( _experiment ) {
        // Set experiment image input source name
        std::string inputName = _engine->getAttributes().inputName;
        rcValue value( inputName );
        _experiment->setSettingValue( cInputImageSourceNameSettingId, value );
    }

	setInternalState( newState );
}

// called periodically while running to notify observer
//	of the current processing time.
void rcExperimentDomainImpl::notifyTime( const rcTimestamp& timestamp )
{
  if (_state == eExperimentPlayback) {
    notifyCursorTime( timestamp );
    _observer->notifyProgCursorTime( timestamp );
  }
  else {
    _length = timestamp;
    _observer->notifyTime( timestamp );
  }
}

int32 rcExperimentDomainImpl::timeEventCount() const
{
  return _observer->progCursorTimeEventCount();
}

// called to notify observer of a timeline range change
void rcExperimentDomainImpl::notifyTimelineRange( const rcTimestamp& start ,
                                                  const rcTimestamp& end )
{
    _observer->notifyEngineTimelineRange( start, end );
}

// called to notify observer of a analysis focus rect change.
void rcExperimentDomainImpl::notifyAnalysisRect( const rcRect& rect )
{
    _observer->notifyAnalysisRect( rect );
}

// called to notify observer of a change in multiplier
void rcExperimentDomainImpl::notifyMultiplier( const double& multiplier )
{
    _observer->notifyMultiplier( multiplier );
}

// called to notify observer of a analysis focus rect rotation change
// angle is in degrees
void rcExperimentDomainImpl::notifyAnalysisRectRotation( const rcAffineRectangle& affine)
{
    _observer->notifyAnalysisRectRotation( affine );
}

// called by the engine to ask us if it should blast an image to
//	a part of the screen of our choosing.
bool rcExperimentDomainImpl::acceptingImageBlits( void )
{
	return _observer->acceptingImageBlits();
}

// called by the engine to ask us if it should 
// give us the polys 
bool rcExperimentDomainImpl::acceptingPolys ( void )
{
  return _observer->acceptingPolys ();
}

// if the observer is accepting image blits, this is called to
// tell the observer to blit the image
void rcExperimentDomainImpl::notifyBlitData( const rcWindow* image )
{
    _observer->notifyBlitData( image );
}

// if the observer is accepting requests for plot, this is called to
// tell the observer to request plot
void rcExperimentDomainImpl::notifyPlotRequest ( SharedCurveDataRef& cv )
{
   _observer->notifyPlotRequest(cv);
}


// if the observer is accepting requests for plot, this is called to
// tell the observer to request plot
void rcExperimentDomainImpl::notifyPlot2dRequest ( SharedCurveData2dRef& cv )
{
    _observer->notifyPlot2dRequest(cv);
}


// if the observer is accepting image blits, this is called to
// tell the observer to blit the image
void rcExperimentDomainImpl::notifyPolys ( const rcPolygonGroupRef* polys )
{
  _engine->notifyEngineOfPolys (polys); 
}


// Pass engine possibly improved polygons
void rcExperimentDomainImpl::getPolys (rcPolygonGroupRef& polys )
{
  _observer->getPolys ( polys );
}

// if the observer is accepting image blits, this is called to
// tell the observer to blit the graphics
void rcExperimentDomainImpl::notifyBlitGraphics( const rcVisualGraphicsCollection* graphics )
{
    _observer->notifyBlitGraphics( graphics );
}

// Global app lock/unlock
void rcExperimentDomainImpl::notifyLockApp( bool l )
{
    if ( l )
        lock();
    else
        unlock();
}

// called to request experiment save from observer
int rcExperimentDomainImpl::notifyRequestSave( const char* fileName,
                                               rcExperimentFileFormat format )
{
    int status = saveExperiment( fileName, format, NULL );
    // Update widgets
    _observer->notifyExperimentChange();
    return status;
}

// called to notify observer of the current video frame size
void rcExperimentDomainImpl::notifyVideoRect( const rcRect& rect )
{
    _observer->notifyVideoRect( rect );
}

// Experiment has changed, notify all widgets to update themselves
void rcExperimentDomainImpl::notifyExperimentChange( void )
{
    _observer->notifyExperimentChange();
}

//////////////// rcCarbonLock implementation ////////////////

void rcExperimentDomainImpl::lock()
{
    _observer->notifyLockApp( true );
}

void rcExperimentDomainImpl::unlock()
{
    _observer->notifyLockApp( false );
}

///////////////////////// private methods ///////////////////////////////////

// internal state update
void rcExperimentDomainImpl::setInternalState( rcExperimentState newState )
{
	if (newState != _state)
	{
		_state = newState;
		_observer->notifyState( newState );
	} else {
        cerr << "rcExperimentDomainImpl::setInternalState already in state " << newState << endl;
    }
}

/******************************************************************************
*	Experiment domain instantiation
******************************************************************************/

// The implementation of the factory method
rcExperimentDomain* rcExperimentDomainFactory::getExperimentDomain( const char* implKey )
{
    // The domain singleton is created at run-time
    static rcExperimentDomainImpl experimentDomain;

  	rcUNUSED( implKey );
	return &experimentDomain;
}
