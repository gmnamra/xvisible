/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*
*	$Id: rc_engineimplbase.cpp 7172 2011-02-01 00:31:41Z arman $
*
*	This file contains the implementation for engine base implementation class.
*
******************************************************************************/

#include <rc_engineimplbase.h>
#include <rc_qtime.h>
#include <rc_histstats.h>

/******************************************************************************
 *   Utility methods
 ******************************************************************************/

// Fold angle value with a specified mod value
double foldAngle( float& angle, const double& modValue )
{
    if ( angle >= modValue ) {
        const int mod = static_cast<int32>(angle / modValue);
        angle = angle - (mod * static_cast<int32>(modValue));
    }

    return angle;
}

// Offset graphics origin (to transform subwindow coords to image coords)
void offsetGraphics( rcVisualGraphicsCollection& graphics, const rc2Fvector& offset )
{
    if ( offset.x() != 0.0 || offset.y() != 0.0 ) {
        rcVisualSegmentCollection& lines = graphics.segments();

        for ( uint32 i = 0; i < lines.size(); ++i ) {
            rcVisualSegment line = lines[i];
            const rcVisualSegment::rcVisualSegmentType type = line.type();
            switch( type ) {
                case rcVisualSegment::eEllipse:
                case rcVisualSegment::eCross:
                    // Do not offset second point
                    line = rcVisualSegment( line.type(), line.p1() + offset, line.p2() );
                    break;

                case rcVisualSegment::ePoints:
                case rcVisualSegment::eLine:
                case rcVisualSegment::eLineStrip:
                case rcVisualSegment::eLineLoop:
                case rcVisualSegment::eArrow:
                case rcVisualSegment::eRect:
                    // Offset both points
                    line = rcVisualSegment( line.type(), line.p1() + offset, line.p2() + offset );
                    break;

                case rcVisualSegment::eUnknown:
                case rcVisualSegment::eEmpty:
                case rcVisualSegment::eStyle:
                case rcVisualSegment::eLast:
                    // Do nothing
                    break;
            }
            lines[i] = line;
        }
    }
}

//
//  rcEngineProgress class implementation
//

// ctor
rcEngineProgress::rcEngineProgress( rcEngine* engine,
                                    rcEngineObserver* observer,
                                    const std::string& message ) :
        mEngine( engine ), mObserver( observer ), mFormatMessage( message ),
        mUpdateInterval( cZeroTime ), // Update with every call
        mLastUpdate( cZeroTime )
{
    rmAssert( mEngine );
    rmAssert( mFormatMessage.size() );
    rmAssert( mFormatMessage.size() < sizeof( mMessageBuf ) );
}

// ctor
rcEngineProgress::rcEngineProgress( rcEngine* engine,
                                    rcEngineObserver* observer,
                                    const std::string& message,
                                    const rcTimestamp& updateInterval ) :
        mEngine( engine ), mObserver( observer ), mFormatMessage( message ),
        mUpdateInterval( updateInterval ), // Minimum time between updates
        mLastUpdate( cZeroTime )
{
    rmAssert( mEngine );
    rmAssert( mFormatMessage.size() );
    rmAssert( mFormatMessage.size() < sizeof( mMessageBuf ) );
}



// virtual dtor
rcEngineProgress::~rcEngineProgress()
{
}

// Call this to update status bar with completion percentage
bool rcEngineProgress::progress( uint32 percentComplete )
{
    return progress( static_cast<double>(percentComplete) );
}

// Call this to update status bar with completion percentage
bool rcEngineProgress::progress( double percentComplete )
{
    bool ret = true;

    if ( mEngine ) {
        if ( mObserver ) {
            rcTimestamp now = rcTimestamp::now();
            // If interval is zero, update with every call
            // Otherwise update after mUpdateInterval seconds have passd since last update
            if ( mUpdateInterval == cZeroTime ||
                 ( (now - mLastUpdate) > mUpdateInterval ) ) {
                mLastUpdate = now;
                snprintf( mMessageBuf, rmDim( mMessageBuf ), mFormatMessage.c_str(), percentComplete );
                // Update status bar
                mObserver->notifyStatus( mMessageBuf );
                // Update elapsed time to refresh display
                mObserver->notifyTime( mEngine->getElapsedTime() );
            }
        }
        if ( mEngine->getState() == eEngineShuttingDown ||
             mEngine->getState() == eEngineStopped )
            ret = true; // Abort analysis
        else
            ret = false;
    }
    return ret;
}

/******************************************************************************
*  Base engine implementation class
******************************************************************************/

#ifdef STATE_CHG_DEBUG
static char* eStateNames[7] = { "eEngineUninitialized", "eEngineStopped",
                                "eEngineRunning", "eEnginePlayback",
                                "eEngineProcessing",
                                "eEngineShuttingDown", "unknown" };

#define PRT_ST_UNK 5
#define PRINT_STATE_CHANGE(STR, OLD, NEW) \
  printf("%s: State transition %s to %s\n", STR, eStateNames[(int)OLD], \
	 eStateNames[(int)NEW])
#else
#define PRINT_STATE_CHANGE(STR, OLD, NEW)
#endif

// public

rcEngineBaseImpl::rcEngineBaseImpl() :
        _observer( 0 ),
        _state( eEngineUninitialized ),
        _frameWidth( 0 ),
        _frameHeight( 0 ),
        _frameDepth( rcPixelUnknown ),
        _writerManager( 0 ),
        _startTime( 0.0 ),
        _currentTime( 0.0 )
{
}

rcEngineBaseImpl::~rcEngineBaseImpl()
{
    delete _writerManager;
}

// rcEngine API

// initialize the engine
void rcEngineBaseImpl::initialize( rcEngineObserver* observer )
{
    // save the pointer to the instance of the engine observer
    _observer = observer;
    // create writer manager
    _writerManager = new rcWriterManager( observer );
    // reset state
    reset();
    // Check settings consistency
    bool settingsOK = checkSettings();
    rmAssert( settingsOK );
}

// start the engine
void rcEngineBaseImpl::start( void )
{
    PRINT_STATE_CHANGE("start", getState(), eEngineStart);
}

// stop the engine
void rcEngineBaseImpl::stop( void )
{
    PRINT_STATE_CHANGE("stop", getState(), eEngineStop);
    setInternalState( eEngineStopped );
}

// shut down the engine
void rcEngineBaseImpl::shutdown( void )
{
    _state = eEngineShuttingDown;
}

// reset the engine so it can be restarted later
// all attributes will be reset to default values
void rcEngineBaseImpl::reset( void )
{
    _startTime = 0.0;
    _currentTime = 0.0;
    _state = eEngineStopped; // Do not send a notification
    _frameWidth = 640;
    _frameHeight = 480;
    _frameDepth = rcPixelUnknown;
}

// process captured data within user-selected focus area.
// can be called repeatedly.
void rcEngineBaseImpl::process( void )
{
    PRINT_STATE_CHANGE("process", getState(), eEngineRunning);
    // Gentlemen, start your engines
    setInternalState( eEngineRunning );
    setInternalState( eEngineStopped );
}

// get the current running rcTimestamp
rcTimestamp rcEngineBaseImpl::getElapsedTime( void )
{
    assertInitialized();

    return _currentTime - _startTime;
}

// get the absolute start rcTimestamp
rcTimestamp rcEngineBaseImpl::getStartTime( void )
{
    assertInitialized();

    return _startTime;
}

// get the engine state (see rcEngineState enum definition above)
rcEngineState rcEngineBaseImpl::getState( void )
{
    return _state;
}

// get the engine attributes (see rcEngineAttributes enum definition above)
const rcEngineAttributes rcEngineBaseImpl::getAttributes( void )
{
    rcEngineAttributes attr;

    attr.frameWidth = _frameWidth;
	attr.frameHeight = _frameHeight;
    attr.frameDepth = _frameDepth;
    attr.liveInput = false;
    attr.liveStorage = false;
    attr.inputName = std::string();

    return attr;
}

// get the number of engine settings categories
int rcEngineBaseImpl::getNSettingCategories( void )
{
    assertInitialized();

    return _settings.size();
}

// get a settings category
rcSettingCategory rcEngineBaseImpl::getSettingCategory( int categoryNo )
{
    assertInitialized();
    rmAssert( uint32(categoryNo) < _settings.size() );

    return _settings[categoryNo];
}

// protected

// Setting consistency check (no duplicate tag names etc.)
// Return true for success, false for failure
bool rcEngineBaseImpl::checkSettings()
{
    bool success = true;
    vector<rcSettingCategory>::iterator i, j;

    for ( i = _settings.begin(); i < _settings.end(); ++i ) {
        rcSettingCategory c1 = *i;
        uint32 nSettings = c1.getNSettings();

        // Verify that all settings have a unique tag
        for ( uint32 x = 0; x < nSettings; ++x )
        {
            rcSettingInfo s1 = c1.getSettingInfo( x );
            for ( uint32 y = x+1; y < nSettings; ++y )
            {
                rcSettingInfo s2 = c1.getSettingInfo( y );
                if ( !strcmp( s1.getTag(), s2.getTag() ) ) {
                    success = false;
                    cerr << "rcEngineBaseImpl::checkSettings error: ";
                    cerr << "two rcSettingInfo objects have the same XML tag \"" << c1.getTag() << "\"" << endl;
                }
            }
        }

        // Verify that all categories have a unique tag
        for ( j = i+1; j < _settings.end(); ++j ) {
            rcSettingCategory c2 = *j;
            if ( !strcmp( c1.getTag(), c2.getTag() ) ) {
                success = false;
                cerr << "rcEngineBaseImpl::checkSettings error: ";
                cerr << "two rcSettingCategory objects have the same XML tag \"" << c1.getTag() << "\"" << endl;
            }
        }
    }

    return success;
}

// our internal state changing method
void rcEngineBaseImpl::setInternalState( rcEngineState newState )
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
			if (newState != eEngineRunning && newState != eEngineProcessing )
				throw general_exception( "illegal state transition from eEngineStopped" );
			break;

		case eEngineRunning:			// engine is running and generating data
			if (newState != eEngineStopped)
				throw general_exception( "illegal state transition from eEngineRunning" );
			break;

		case eEnginePlayback:			// engine is running and generating data
			if (newState != eEngineStopped)
				throw general_exception( "illegal state transition from eEnginePlayback" );
			break;

        case eEngineProcessing:            // engine is processing a focus area
            if (newState != eEngineStopped)
                throw general_exception( "illegal state transition from eEngineProcessing" );
            break;

		case eEngineShuttingDown:		// engine is in the process of shutting down
			throw general_exception( "illegal state transition from eEngineShuttingDown" );
			break;
    }

    PRINT_STATE_CHANGE("setInternalState", _state, newState);
    _state = newState;
    if ( _observer )
        _observer->notifyState( _state );
}

// utility assertion
void rcEngineBaseImpl::assertInitialized( void )
{
    if (_state == eEngineUninitialized)
        throw rcEngineException( "engine uninitialized" );
}

// Write all focus and image data to writers
void rcEngineBaseImpl::writeData( rcEngineFocusData* focus )
{
    const vector<rcTimestamp>& sTimestamps = focus->timeStamps();

    // Produce data for all images
    for ( uint32 i = 0; i < sTimestamps.size(); ++i ) {
        // Write all data
        focus->writeFocusData( sTimestamps[i], i );
    }
    // Flush writers
    focus->flushFocusData();

    // Update elapsed time
    _observer->notifyTime( _currentTime );
}
