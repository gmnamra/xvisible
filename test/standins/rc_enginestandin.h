/*
 *  rc_enginestandin.h
 *  bina
 *
 *  Created by Arman Garakani on 9/26/08.
 *  Copyright 2008 Reify Corporation. All rights reserved.
 *
 */

/******************************************************************************
 *	rc_enginestandin.h
 *
 *	This file contains a test implementation of an engine.  It supports
 *	a number of settings that will be reflected in a single settings tab,
 *	and generates some manufactured analysis data.  This engine starts up
 *	a thread to mimic frame interrupts from a camera.
 ******************************************************************************/

#include <math.h>
#include <string>
#include <strstream>

#include <rc_setting.h>
#include <rc_thread.h>
#include <rc_engine.h>
#include <rc_writermanager.h>

#if WIN32
using namespace std;
#endif

/******************************************************************************
 *	Constants
 ******************************************************************************/

// Settings id constants
const int cInputModeSettingId = 0;
const int cFrameRateSettingId = 1;
const int cInputMovieFileSettingId = 2;
const int cInputImageFilesSettingId = 3;
const int cEnableCellDetectionSettingId = 4;
const int cAnalysisRectSettingId = 5;
const int cAnalysisSlidingWindowSizeId = 6;
const int cAnalysisSlidingWindowOriginId = 7;
const int cAnalysisSlidingWindowEnabledId = 8;

// input mode constant definitions
const int cCameraInputMode = 0;
const int cFileInputMode = 1;

// input source constant definitions
const int cCameraInputSource = 0;
const int cFileInputSource = 1;
const int cMovieInputSource = 2;
const int cImageFilesInputSource = 3;

// sliding window result origin
const int cAnalysisSlidingWindowOriginLeft = 0;
const int cAnalysisSlidingWindowOriginCenter = 1;
const int cAnalysisSlidingWindowOriginRight = 2;

/******************************************************************************
 *	Writer strings
 ******************************************************************************/
// TODO: make this more dynamically manageable

const char* cVideoWriterStrings[] = {
"video",              // XML tag name
"Raw Video",          // Display name
"Raw Video Frames",   // Description
0                     // Printf format string
};

const char* cBoundaryVideoWriterStrings[] = {
"boundary-video",              // XML tag name
"Cell Boundary Video",        // Display name
"Cell Boundary Video Frames", // Description
0                             // Printf format string
};

const char* cEnergyWriterStrings[] = {
"change-index",              // XML tag name
"Aggregate Change Index",  // Display name
"Aggregate Change Index",    // Description
"%-8.5f"                     // Printf format string
};

const char* cSlidingEnergyWriterStrings[] = {
"sliding-change-index",              // XML tag name
"Aggregate Change Index Sliding Window", // Display name
"Aggregate Change Index - Sliding Window",    // Description
"%-8.5f"                              // Printf format string
};

/******************************************************************************
 *	Setting choices
 ******************************************************************************/

// choices for selecting an input source
const rcSettingChoice inputModeChoices[] = 
{
rcSettingChoice( cCameraInputMode,	"Camera"	  , "Select the camera as the input source to analyze" ),
rcSettingChoice( cFileInputMode	,	"File import" , "Select a data file as the input source to analyze" ),
rcSettingChoice( 0 , 0 , 0 )
};

// Choices for selecting the capture frame rate
const rcSettingChoice frameRateChoices[] = 
{
rcSettingChoice( 7.5	,	"7.5 fps"	, "capture at 7.5 fps" ),
rcSettingChoice( 3.25	,   "3.25 fps"	, "capture at 3.25 fps" ),
rcSettingChoice( 1.625	,	"1.625 fps"	, "capture at 1.625 fps" ),
rcSettingChoice( 0.8125	,	"0.8125 fps", "capture at 0.8125 fps" ),
rcSettingChoice( 1		,	"custom"	, "capture at custom fps" ),    
rcSettingChoice( 0 , 0 , 0 )
};

// choices for specifying the file name filter for movie file selection
const rcSettingChoice movieFileFilterChoices[] =
{
// Qt seems to be case-sensitive...bummer
rcSettingChoice( -1, "unused", "Movie Files (*.mov *.MOV *.avi *.AVI *.mpg *.MPG)" ),
rcSettingChoice( 0 , 0 , 0 )
};

// choices for specifying the file name filter for movie file selection
const rcSettingChoice imageFileFilterChoices[] =
{
rcSettingChoice( -1, "unused", "Image Files (*.tif *.TIF *.tiff *.TIFF *.jpg *.JPG *.jpeg *.JPEG *.gif *.GIF *.qtif *.QTIF *.bmp *.BMP)" ),
rcSettingChoice( 0 , 0 , 0 )
};

// choices for selecting an input source
const rcSettingChoice slidingWindowOriginChoices[] = 
{
rcSettingChoice( cAnalysisSlidingWindowOriginLeft,	"Left"	 , "Result for frame T is computed from [T+1, T+2, ...]" ),
rcSettingChoice( cAnalysisSlidingWindowOriginCenter, "Center", "Result for frame T is computed from [..., T-1, T+1, ...]" ),
rcSettingChoice( cAnalysisSlidingWindowOriginRight,	 "Right" , "Result for frame T is computed from [...,T-2, T-1]" ),
rcSettingChoice( 0 , 0 , 0 )
};

/******************************************************************************
 *	Setting specs
 ******************************************************************************/

// capture setting specs
const rcSettingInfoSpec captureSettings[] =
{
{	// setting to select the input source
cInputModeSettingId,
"input-source",
"Analysis source",
"Selects the input source",
eRadioChoice,
0,
inputModeChoices
},
{	// setting to select the camera cature frame rate
cFrameRateSettingId,
"frame-rate",
"Frame rate",
"Sets the input frame rate",
eFramerateChoice,
0,
frameRateChoices
},
{   // setting to select the movie file to analyze
cInputMovieFileSettingId,
"movie-file",
"Input movie",
"Input movie file to analyze",
eFileChoice,
0,
movieFileFilterChoices
},
{   // setting to select the image files to analyze
cInputImageFilesSettingId,
"input-files",
"Input images",
"Input image files to analyze",
eMultiFileChoice,
0,
imageFileFilterChoices
},
{ 0 , 0 , 0 , 0 , 0 , 0 , 0 }
};

// analysis setting specs
const rcSettingInfoSpec analysisSettings[] =
{
{   // I don't know what this is, just a dummy boolean checkbox setting
cAnalysisRectSettingId,
"analysis-rect",
"Attention area",
"Select the area of each input frame to analyze",
eRect,
0,
0
},
#ifdef notyet //   Not implemented yet
{	
cEnableCellDetectionSettingId,
"cell-detection-enabled",
"Enable cell boundary detection",
"Graphic overlay of detected cell boundaries",
eCheckbox,
0,
0
},
#endif
#if 1 // def notyet    // Not fully implemented yet
{	
cAnalysisSlidingWindowEnabledId,
"sliding-window-enabled",
"Enable sliding window",
"Enable sliding window aggregate change index",
eCheckbox,
0,
0
},
{	
cAnalysisSlidingWindowSizeId,
"sliding-window-size",
"Sliding window size",
"Number of frames used for attentive capture computation\nSpecify an odd number (3,5,7...) for a balanced window",
eSpinbox,
0,
0
},
{	
cAnalysisSlidingWindowOriginId,
"sliding-window-origin",
"Sliding window result origin",
"Relative frame origin for results",
eRadioChoice,
0,
slidingWindowOriginChoices
},
#endif    
{ 0 , 0 , 0 , 0 , 0 , 0 , 0 }
};

// The capture setting category spec
const rcSettingCategorySpec captureSettingsSpec =
{
"capture" , "Capture" , "Settings that control video capturing"	, captureSettings
};

// the analysis setting category spec
const rcSettingCategorySpec analysisSettingsSpec =
{
"analysis" , "Analysis" , "Settings that control data analysis"	, analysisSettings
};

/******************************************************************************
 *	Dummy frame data class
 ******************************************************************************/

class rcFrameData 
	{
	};

/******************************************************************************
 *	Test engine implementation class
 ******************************************************************************/

class rcEngineStandIn : public rcEngine, public rcSettingSource, public rcRunnable
{
public:
	rcEngineStandIn()
	{
		_state = eEngineUninitialized;
		_frameThread = 0;
		_observer = 0;
		_energyWriter = 0;
		_motionWriter = 0;
        _videoWriter = 0;
        _currentTime = cZeroTime;
        _startTime = cZeroTime;
        
		// set default attributes
		_attributes.frameWidth = 640;
		_attributes.frameHeight = 480;
		_attributes.liveInput = false;
        _attributes.liveStorage = false;
        _attributes.inputName = "Simulated input";
        
		// set the initial (default) setting values
		_inputSource = cMovieInputSource;
		_frameRate = 10;
		_analysisRect = rcRect( 0 , 0 , _attributes.frameWidth , _attributes.frameHeight );
	}
    virtual ~rcEngineStandIn();
	
	// initialize the engine
	virtual void initialize( rcEngineObserver* observer )
	{
		// save the pointer to the instance of the engine observer
		_observer = observer;
		
        // create setting categories given the specs defined above
        _settings.push_back(rcSettingCategory( this , &captureSettingsSpec ));
        _settings.push_back(rcSettingCategory( this , &analysisSettingsSpec ));
		
		// set the initial state once we're done setting up
		//	(notifies the observer).
		setInternalState( eEngineStopped );
		
		// set up our dummy frame interrupt. 
		_frameThread = new rcThread( this );
		_frameThread->setPriority( eHighPriority );
		_frameThread->start();
	}
	
	// shut down the engine
	virtual void shutdown( void )
	{
		_state = eEngineShuttingDown;
		
		if (_frameThread != 0)
		{
            if ((_frameThread != 0) && (_frameThread->isRunning()))
            {
                int status = _frameThread->join();
                if ( status ) {
                    throw general_exception( "engin frame thread shutdown error" );
                }
            }
		}
	}
	
	// get the number of engine settings categories
	virtual int getNSettingCategories( void )
	{
		assertInitialized();
		
		return _settings.size();
	}
	
	// get a settings category
	virtual rcSettingCategory getSettingCategory( int categoryNo )
	{
        rcUNUSED( categoryNo );
		assertInitialized();
		
		return _settings[ categoryNo ];
	}
	
	// get the current running rcTimestamp
	virtual rcTimestamp getElapsedTime( void )
	{
		assertInitialized();
		
		return _currentTime - _startTime;
	}
	
    // get the start rcTimestamp
	virtual rcTimestamp getStartTime( void )
	{
		assertInitialized();
		
		return _startTime;
	}
    
	// get the engine state (see rcEngineState enum definition above)
	virtual rcEngineState getState( void )
	{
		return _state;
	}
	
	// get the engine live attributes 
	virtual bool operatingOnLiveInput ( void ) { return false; }
	virtual bool storingLiveInput ( void ) { return false; }
	virtual void notifyEngineOfPolys ( const rcPolygonGroupRef * polys ) { }
	
	// process captured data within user-selected focus area.
	// can be called repeatedly. 
	virtual bool pauseTracking ( void ) { return false; }
	virtual bool resumeTracking ( void ) { return false; }
	
	// get the engine attributes (see rcEngineAttributes enum definition above)
	virtual const rcEngineAttributes getAttributes( void )
	{
		return _attributes;
	}
	
	// start the engine
	virtual void start( void )
	{
        _startTime = 0.0;
        rcWriterManager writerManager( _observer );
        
        // create a couple of writer groups
        for (int i = 0; i < 1; i++)
        {
	        strstream name;
            name << "Test Group " << (i+1) << ends;
            rcWriterGroup* writerGroup = writerManager.createWriterGroup( name.str() , "A test group",
																		 eGroupSemanticsGlobalMeasurements );
            _writerGroups.push_back( writerGroup );
            name.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
        }
		
        rcRect analysisRect;
		// create a couple of scalar data output writers
		rcWriter* writer;
		writer = writerManager.createWriter( _writerGroups[ 0 ] , eWriterACI,
											rcUINT32_MAX, analysisRect );
		_energyWriter = dynamic_cast<rcScalarWriter*>( writer );
		_energyWriter->setExpectedMinValue( -2.0 );
		_energyWriter->setExpectedMaxValue( 2.0 );
        writer = writerManager.createWriter( _writerGroups[ 0 ] , eWriterCellCount,
											rcUINT32_MAX, analysisRect );
		_motionWriter = dynamic_cast<rcScalarWriter*>( writer );
		_motionWriter->setExpectedMinValue( -2.0 );
		_motionWriter->setExpectedMaxValue( 2.0 );
		
        writer = writerManager.createWriter( _writerGroups[ 0 ] , eWriterVideo,
											rcUINT32_MAX, analysisRect );
        _videoWriter = dynamic_cast<rcVideoWriter*>( writer );
        
		setInternalState( eEngineRunning );
		
        _observer->notifyTimelineRange( 0.0, 10.0 );
	}
	
	// stop the engine
	virtual void stop( void )
	{
		setInternalState( eEngineStopped );
	}
	
    // reset the engine
    virtual void reset ( void )
    {
        // TODO: implement this
    }
    
    // process focus area
    virtual void process( void )
    {
        assertInitialized();
    }
	
	// get the latest video frame captured.
	virtual rcSharedMemError getCurrentFrameData(rcSharedMemoryUser&,
												 rcSharedFrameBufPtr&)
	{
		assertInitialized();
		
		return rcSharedMemNoError;
	}
	
    //////////// rcSettingSource implementation //////////////////
	
    // get the current value of a setting from the setting source
    virtual const rcValue getSettingValue( int settingId )
    {
        rcLock lock( _settingMutex);
        assertInitialized();
		
        switch (settingId)
        {
		    case cInputModeSettingId:
			    return _inputMode;
				
		    case cFrameRateSettingId:
			    return _frameRate;
				
            case cInputMovieFileSettingId:
                return _movieFile;
				
            case cInputImageFilesSettingId:
                return _imageFileNames;
                
            case cAnalysisRectSettingId:
                return _analysisRect;
                
            case cEnableCellDetectionSettingId:
			    return _cellDetectionEnabled;
				
            case cAnalysisSlidingWindowSizeId:
                return _slidingWindowSize;
				
            case cAnalysisSlidingWindowOriginId:
                return _slidingWindowOrigin;
				
            case cAnalysisSlidingWindowEnabledId:
                return _slidingWindowEnabled;
        }
		
        return 0;
    }
	
    // set the new value of a setting to the setting source.
    virtual void setSettingValue( int settingId , const rcValue& value )
    {
        rcLock lock( _settingMutex);
		
        assertInitialized();
		
        switch (settingId)
        {
		    case cInputModeSettingId:
			    _inputMode = value;
                if ( _inputMode == cCameraInputSource ) {
                    _frameThread->start();
                    _slidingWindowEnabled = true;
                    _attributes.liveInput = true;
                }
                else
                    _slidingWindowEnabled = false;
				_attributes.liveInput = false;                
			    break;
				
		    case cFrameRateSettingId:
			    _frameRate = value;
			    break;
				
            case cInputMovieFileSettingId:
                _movieFile = value;
                _inputMode = cFileInputMode;
                _inputSource = cMovieInputSource;
				//                if ( _importThread )
				//                    _importThread->start();
                break;
				
            case cInputImageFilesSettingId:
                _imageFileNames = value;
                _inputMode = cFileInputMode;
                _inputSource = cImageFilesInputSource;
				//                if ( _importThread )
				//                    _importThread->start();
				
                break;
				
            case cAnalysisRectSettingId:
                _analysisRect = value;
                break;
				
		    case cEnableCellDetectionSettingId:
			    _cellDetectionEnabled = value;
			    break;
				
            case cAnalysisSlidingWindowSizeId:
                _slidingWindowSize = value;
                // Zero size window is not allowed
                if ( _slidingWindowSize < 1 )
                    _slidingWindowSize = 1;
                break;
				
            case cAnalysisSlidingWindowOriginId:
                _slidingWindowOrigin = value;
                break;
				
            case cAnalysisSlidingWindowEnabledId:
                _slidingWindowEnabled = value;
                break;
        }
    }
	
    // return whether this setting is current enabled.
    virtual bool isSettingEnabled( int settingId )
    {
        rcLock lock( _settingMutex);
        assertInitialized();
		
        switch (settingId)
        {
            case cInputMovieFileSettingId:
                return _inputMode == cFileInputMode && _inputSource == cMovieInputSource;
				
            case cInputImageFilesSettingId:
                return _inputMode == cFileInputMode && _inputSource == cImageFilesInputSource;
				
            case cAnalysisRectSettingId:
                return _state != eEngineUninitialized;
				
            case cEnableCellDetectionSettingId:
                return _state != eEngineUninitialized;
				
            case cAnalysisSlidingWindowEnabledId:
                return _state != eEngineUninitialized;
				
            case cAnalysisSlidingWindowSizeId:
            case cAnalysisSlidingWindowOriginId:
                return _slidingWindowEnabled;
        }
		
        return true;
    }
	
    // return whether this setting is current changable.
    virtual bool isSettingEditable( int settingId )
    {
        rcLock lock( _settingMutex);
        switch (settingId)
        {
            case cFrameRateSettingId:
                return true;
            case cInputMovieFileSettingId:
            case cInputImageFilesSettingId:
            case cInputModeSettingId:
                return true;
            case cAnalysisRectSettingId:
                if ( _inputMode == cCameraInputSource )
                    return true;
                break;
            case cEnableCellDetectionSettingId:
                return false;
                break;
        }
        
        return _state != eEngineRunning;
    }
	
    // return whether this setting is current persistable.
    virtual bool isSettingPersistable( int settingId )
    {
        rmUnused( settingId );
        
        return false;
    }
    
	///////// rcRunnable implementation ///////////////////////////
	
	// simulate frame interrupts at the selected frame rate
	void run( void )
	{
		while (_state != eEngineShuttingDown)
		{
			int frameMillis = int(1000 / _frameRate);
			rcThread::sleep( frameMillis );
			frameInterrupt( 0 );
		}
	}
	
private:
	// our frame interrupt handler
	void frameInterrupt( rcFrameData* frameData )
	{
		// ignore interrupts if we're not running
		if (_state != eEngineRunning)
			return;
		
		// process the frame interrupt
		try
		{
			_currentTime = rcTimestamp::now();
			if (_startTime == 0.)
				_startTime = _currentTime;
			
			rcTimestamp timestamp = _currentTime - _startTime;
			processFrameData( timestamp , frameData );
		}
		catch (general_exception& x)
		{
			engineAbort( x.what() );
		}
		catch (exception& x)
		{
			engineAbort( x.what() );
		}
		catch (...)
		{
			engineAbort( "unknown exception type" );
		}
	}
	
	// our frame processing method
	void processFrameData( rcTimestamp timestamp , rcFrameData* frameData )
	{
        rcUNUSED( frameData );
		
		double x = timestamp.secs();
		if (_energyWriter != 0)
		{
			double energy = 10 + sin( x * 5.221 ) + sin( x * 1.231 );
			_energyWriter->writeValue( timestamp , rcRect(), energy );
		}
		if (_motionWriter != 0)
		{
			double motion = 5 + sin( x * 2.934 ) + sin( x * 0.1409 );
			_motionWriter->writeValue( timestamp , rcRect(), motion );
		}
        if (_videoWriter != 0)
        {
            _videoWriter->writeValue( timestamp , rcRect(), 0 );
        }
        _observer->notifyTime( timestamp );
	}
	
	// our internal state changing method
	void setInternalState( rcEngineState newState )
	{
		// ignore setting to current state
		if (newState == _state)
			return;
		
		// validate state transition
		switch (_state)
		{
			case eEngineUninitialized:	// engine has not been initialized
				if (newState != eEngineStopped)
					throw general_exception( "illegal state transition from eUninitialized" );
				break;
				
			case eEngineStopped:			// engine is stopped and not generating data
				if (newState != eEngineRunning)
					throw general_exception( "illegal state transition from eEngineStopped" );
				break;
				
			case eEngineRunning:			// engine is running and generating data
				if (newState != eEngineStopped)
					throw general_exception( "illegal state transition from eEngineRunning" );
				break;
				
			case eEnginePlayback:			// engine is in playback mode
				if (newState != eEngineStopped)
					throw general_exception( "illegal state transition from eEnginePlayback" );
				break;
				
			case eEngineShuttingDown:		// engine is in the process of shutting down
				throw general_exception( "illegal state transition from eEngineShuttingDown" );
				break;
				
			case eEngineProcessing:
				if (newState != eEngineStopped)
					throw general_exception( "illegal state transition from eEngineProcessing" );
				break;
		}
		
		_state = newState;
		_observer->notifyState( _state );
	}
	
	// called to abort from frame interrupt handler
	void engineAbort( const char* msg )
	{
		setInternalState( eEngineStopped );
		_observer->notifyError( msg );
	}
	
	// utility assertion
	void assertInitialized( void )
	{
		if (_state == eEngineUninitialized)
			throw rcEngineException( "engine uninitialized" );
    }
	
    // Setting variables
	int					_inputSource;
    int					_inputMode;
	float			    _frameRate;
    std::string            _movieFile;
    std::string            _imageFileNames;
    rcRect              _analysisRect;
	bool				_cellDetectionEnabled;
    uint32            _writerSizeLimit;
    int                 _slidingWindowSize;
    int                 _slidingWindowOrigin;
    bool                _slidingWindowEnabled;
    rcMutex             _settingMutex;
	
	// the current state of the engine (stopped, running, etc).
	rcEngineState		_state;
	rcEngineAttributes	_attributes;
	
	// the settings categories that let the UI control us
	vector<rcSettingCategory> _settings;
	
	// the engine observer we were created with
	rcEngineObserver*	_observer;
	
    // the writer groups we've created
    vector<rcWriterGroup*>  _writerGroups;
	
	// a thread to simulate real-time capture
	rcThread*			_frameThread;
	
	// run-time state
	rcTimestamp			_startTime;
	rcTimestamp			_currentTime;
	
	// the writers we pass our generated results to.
	rcScalarWriter*		_energyWriter;
	rcScalarWriter*		_motionWriter;
    rcVideoWriter*		_videoWriter;
};

