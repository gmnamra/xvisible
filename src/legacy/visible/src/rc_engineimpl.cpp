#include <rc_draw.h>
#include <rc_stlplus.h>
#include <rc_fileutils.h>
#include <file_system.hpp>
#include <boost/shared_ptr.hpp>
#include <rc_engineimpl.h>
#include <rc_experimentdomainimpl.h>
#include <rc_uitypes.h>



/******************************************************************************
 *	Debugging
 ******************************************************************************/

//#define STATE_CHG_DEBUG
#ifdef STATE_CHG_DEBUG
static char* eStateNames[7] = { "eEngineUninitialized", "eEngineStopped",
    "eEngineRunning", "eEnginePlayback",
    "eEngineProcessing",
    "eEngineShuttingDown", "unknown" };

#define PRT_ST_UNK 6
#define PRINT_STATE_CHANGE(STR, OLD, NEW)		\
printf("%s: State transition %s to %s\n", STR,	\
eStateNames[(int)OLD], eStateNames[(int)NEW])
#else
#define PRINT_STATE_CHANGE(STR, OLD, NEW)
#endif

// Make this true to enable some developer-only functionality
bool gDeveloperDebugging = false;

//
// developer debugging methods
//
bool developerDebugging ()
{
    return gDeveloperDebugging;
}

bool developerDebugging (bool b)
{
    gDeveloperDebugging = b;
    return developerDebugging ();
}




/******************************************************************************
 *	Constants
 ******************************************************************************/

static const char* cVideoWriterGroupStrings[] = {
    "Image Data",       // Display name
    "Image Data Group", // Description
};

static const char* cGraphicsWriterGroupStrings[] = {
    "Graphical data",       // Display name
    "Graphical Data Group", // Description
};

static const char* cPreviewWriterGroupStrings[] = {
    "Camera preview data",       // Display name
    "Camera Preview Data Group", // Description
};

// Group Statistics:
template<class T>
double rcEngineImpl::groupStatistics (const vector<T>& a)
{
    int32 stat = getSettingValue( cAnalysisStatTypeId );
    
    if (stat == cAnalysisStatMean)
    {
        return (double) rfMean (a);
    }
    else if (stat == cAnalysisStatMedian)
    {
        return (double) rfMedian (a);
    }
    else
    {
        rmAssert (0);
    }
}

// Construct experiment attributes from movie header
rcExperimentAttributes rcEngineImpl::attributes( const rcMovieFileExpExt& exp ) const
{
    rcExperimentAttributes attrs;
    
    attrs.title = exp.title();
    attrs.userName = exp.userName();
    attrs.comments = exp.comment();
    attrs.treatment1 = exp.treatment1();
    attrs.treatment2 = exp.treatment2();
    attrs.cellType = exp.cellType();
    attrs.imagingMode = exp.imagingMode();
	rcValue tmp = exp.lensMag(); attrs.lensMag = (std::string) tmp;
    attrs.otherMag = (std::string) rcValue(exp.otherMag());
    attrs.temperature = (std::string) rcValue(exp.temperature());
    attrs.CO2 = (std::string) rcValue(exp.CO2());
    attrs.O2 = (std::string) rcValue(exp.O2());
    
    // These values are not used but define values just in case
    attrs.fileName = "Unspecified";
    attrs.inputName = "Unspecified";
    attrs.frameWidth = 0;
    attrs.frameHeight = 0;
    attrs.frameDepth = 0;
    attrs.liveInput = false;
    attrs.liveStorage = false;
    
    return attrs;
}

/******************************************************************************
 * rcEngineImpl class implementation
 ******************************************************************************/

// Public methods

rcEngineImpl::rcEngineImpl() : _state(eEngineUninitialized),
_videoWriter(0),
_frameFocus(0),
_fpsCalculator( 15 ), // Speed fps for a buffer of 15 frames
_firstFrameSaved(true), _isPausedToggle (false)
{
    PRINT_STATE_CHANGE("ctor", getState(), eEngineUninitialized);
    _captureThread = 0;
    _importThread = 0;
    //  _movieCapture = 0;
    _analyzerThread = 0;
    _observer = 0;
    _writerManager = 0;
    _videoCacheP = 0;
    _playbackThread = 0;
    _player = 0;
    
    // reset default attributes
    internalReset();
}

// Must have a virtual dtor
rcEngineImpl::~rcEngineImpl()
{
    delete _captureThread;
    delete _importThread;
    delete _analyzerThread;
    delete _playbackThread;
    delete _importer;
    //  delete _movieCapture;
    delete _player;
    rcEngineFocusData* temp;
    delete _frameFocus.getValue(temp);
    delete _writerManager;
    delete _videoCacheProgress;
}

// initialize the engine
void rcEngineImpl::initialize( rcEngineObserver* observer )
{
    
	_resourceFolderPath = rcExecWithShmem::pathName ();
    
    // save the pointer to the instance of the engine observer
    _observer = observer;
    // create writer manager
    _writerManager = new rcWriterManager( observer );
    
    // create setting categories given the spec: Analysis
    _settings.clear ();
    _settings.insert (_settings.begin(), rcSettingCategory( this , &analysisSettingsSpec ));
    
    // set the initial state once we're done setting up
    //	(notifies the observer).
    setInternalState( eEngineStopped );
    
    _captureThread = 0;
    _importThread = 0;
    _analyzerThread = 0;
    _playbackThread = 0;
    
    //  _movieCapture = new rcEngineMovieCapture( this, _captureCtrl,
    //																					 _saveCtrl, _observer );
    //  _captureThread = new rcThread( _movieCapture );
    //  _captureThread->setPriority( eHighPriority );
    _importer = new rcEngineImporter( this );
    _importThread = new rcThread( _importer );
    _importThread->setPriority( eHighPriority );
    _analyzer = new rcEngineAnalyzer( this );
    _analyzerThread = new rcThread( _analyzer );
    _analyzerThread->setPriority( eNormalPriority );
    _player = new rcEngineMoviePlayback( this, _playbackCtrl, _observer,
                                        100.0, false );
    _playbackThread = new rcThread( _player );
    _playbackThread->setPriority( eNormalPriority );
    // rev0 TOC generation is the only slow video cache operation now
    _videoCacheProgress = new rcEngineProgress( this, _observer,
                                               "Loading frames...%.2f%% complete",
                                               rcTimestamp( 1.5 ) );
    
}

// shut down the engine
void rcEngineImpl::shutdown( void )
{
    PRINT_STATE_CHANGE("shutdown", getState(), eEngineShuttingDown);
    setState(eEngineShuttingDown);
    
    cout << "\nEngine shutdown sequence started\n";
    
    if ((_captureThread != 0) && (_captureThread->isRunning()))
    {
        cout << "    capture thread shutdown ";
        int status = _captureThread->join();
        if ( status )
        cout << "error " << status << endl;
        else
        cout << "OK" << endl;
    }
    if ((_importThread != 0) && (_importThread->isRunning()))
    {
        cout << "    import thread shutdown ";
        int status = _importThread->join();
        if ( status )
        cout << "error " << status << endl;
        else
        cout << "OK" << endl;
    }
    if ((_analyzerThread != 0) && (_analyzerThread->isRunning()))
    {
        cout << "    analyzer thread shutdown ";
        int status = _analyzerThread->join();
        if ( status )
        cout << "error " << status << endl;
        else
        cout << "OK" << endl;
    }
    if ((_playbackThread != 0) && (_playbackThread->isRunning()))
    {
        cout << "    playback thread shutdown ";
        int status = _playbackThread->join();
        if ( status )
        cout << "error " << status << endl;
        else
        cout << "OK" << endl;
    }
    // Delete all data
    delete _captureThread;
    _captureThread= 0;
    delete _importThread;
    _importThread = 0;
    delete _importer;
    _importer = 0;
    //  delete _movieCapture;
    // _movieCapture = 0;
    delete _analyzer;
    _analyzer = 0;
    delete _playbackThread;
    _playbackThread = 0;
    delete _player;
    _player = 0;
    cout << "Engine shutdown sequence finished\n";
}

// get the number of engine settings categories
int rcEngineImpl::getNSettingCategories( void )
{
    assertInitialized();
    
    return _settings.size();
}

// get a settings category
rcSettingCategory rcEngineImpl::getSettingCategory( int categoryNo )
{
    assertInitialized();
    rmAssert( uint32(categoryNo) < _settings.size() );
    
    return _settings[categoryNo];
}


// get a settings category
rcSettingCategory rcEngineImpl::getSettingCategory( char *categoryName)
{
    assertInitialized();
    rmAssert (categoryName != 0);
    
    for (uint32 i = 0; i < _settings.size(); i++)
    {
        if (_settings[i].getName () == categoryName)
        return _settings[i];
    }
    
    rmAssert (0); // one of the compares must work.
}


// get the current running rcTimestamp
rcTimestamp rcEngineImpl::getElapsedTime( void )
{
    assertInitialized();
    
    return _currentTime - _startTime;
}

// get the absolute start rcTimestamp
rcTimestamp rcEngineImpl::getStartTime( void )
{
    assertInitialized();
    
    return _startTime;
}

// set the engine state (see rcEngineState enum definition above)
void rcEngineImpl::setState( rcEngineState newState )
{
    return _state.setValue(newState);
}

// get the engine state (see rcEngineState enum definition above)
rcEngineState rcEngineImpl::getState( void )
{
    rcEngineState dummy;
    return _state.getValue(dummy);
}

// get the engine attributes (see rcEngineAttributes definition in rc_engine.h)
bool rcEngineImpl::operatingOnLiveInput ( void )
{
    int inputMode = getSettingValue( cInputModeSettingId );
    return (inputMode != eFile && inputMode != eCmd);
}

bool rcEngineImpl::storingLiveInput ( void )
{
    return false; // return ( _movieCapture->saveState() == eMovieFileState_Save );
}

bool rcEngineImpl::pauseTracking ( void )
{
    bool tmp (true);
    _isPausedToggle.setValue (tmp);
    return tmp;
}

bool rcEngineImpl::resumeTracking ( void )
{
    bool tmp (false);
    _isPausedToggle.setValue (tmp);
    return tmp == false;
}

void rcEngineImpl::notifyEngineOfPlotRequests ( const CurveData* polys )
{
    _curveDataRef = boost::shared_ptr<CurveData> (new CurveData (*polys));
}


void rcEngineImpl::notifyEngineOfPlot2dRequests ( const CurveData2d* polys )
{
    _curveData2dRef = boost::shared_ptr<CurveData2d> (new CurveData2d (*polys));
}


void rcEngineImpl::notifyEngineOfPolys ( const rcPolygonGroupRef * polys )
{
    //  const vector<rcPolygon>& pgs = const_cast<vector<rcPolygon>&> (polys->polys ());
    if (polys->size ())
    {
		rcSharedPolygonGroupPtr c0 ( new rcPolygonGroup );
        rcPolygonGroupRef cref (c0);
        for (uint32 i = 0; i < polys->size (); i++)
        {
            cref.push_back (const_cast<rcPolygonGroupRef*> (polys) ->operator[] (i) );
        }
        _selectedPolys = cref;
    }
}

const rcEngineAttributes rcEngineImpl::getAttributes( void )
{
    rcEngineAttributes attr;
    
    int inputMode = getSettingValue( cInputModeSettingId );
    rmAssert ( inputMode == eFile || inputMode == eCmd);
#if 0
    if ( inputMode != eFile && inputMode != eCmd)
    {
        rmAssert(_movieCapture);
        rcRect captureRect = _movieCapture->captureStatus().getCaptureRect();
        rcPixel captureDepth = _movieCapture->captureStatus().getCaptureDepth();
        attr.frameWidth = captureRect.width();
        attr.frameHeight = captureRect.height();
        attr.frameDepth = captureDepth;
        
        attr.liveInput = true;
        attr.liveStorage = (_movieCapture->saveState() == eMovieFileState_Save);
        
        // In capture mode the output movie file name becomes the experiment
        // image input source name
        attr.inputName = (std::string) getSettingValue( cOutputMovieFileSettingId );
    }
    else {
#endif
        
        attr.frameWidth = _frameWidth;
        attr.frameHeight = _frameHeight;
        attr.frameDepth = _frameDepth;
        attr.liveInput = false;
        attr.liveStorage = false;
        // In file mode the input image/movie file name becomes the experiment
        // image input source name
        
        if (isSettingEnabled( cInputMovieFileSettingId ) )
        attr.inputName = (std::string) getSettingValue( cInputMovieFileSettingId );
        else if ( isSettingEnabled( cInputImageFilesSettingId ) )
        attr.inputName = (std::string) getSettingValue( cInputImageFilesSettingId );
        
        return attr;
    }
    
    // start the engine
    void rcEngineImpl::start( void )
    {
#ifdef STATE_CHG_DEBUG
        fprintf(stderr, "rcEngineImpl::start called\n");
#endif
        //_writerSizeLimit = 10;
        createSharedWriters();
        
        switch (_inputMode)
        {
            case eFile:
            {
                _isPausedToggle.setValue (false);
                int pCtrl = getSettingValue(cPlaybackCtrlSettingId);
                if ((pCtrl & rcSettingInfo::eCurrentStateStopped) == 0)
                {
                    setInternalState( eEnginePlayback );
                }
            }
            break;
            case eCamera:
            {
                // Reset start time
                _startTime = 0.0;
                _isPausedToggle.setValue (false);
                _firstFrameSaved.setValue(true);
                _processCount++;
                rcRect iRect( 0, 0, _frameWidth, _frameHeight );
                rcEngineFocusData* temp =
                new rcEngineFocusData( _writerManager,
                                      getSettingValue( cAnalysisRectSettingId ),
                                      iRect,
                                      1,
                                      static_cast<rcAnalysisMode>(int(getSettingValue( cAnalysisModeSettingId ))),
                                      _writerSizeLimit,
                                      int(getSettingValue( cAnalysisACISlidingWindowSizeSettingId )),
                                      rcAnalyzerResultOrigin(int(getSettingValue( cAnalysisACISlidingWindowOriginSettingId ))),
                                      _groupCount,
                                      static_cast<unsigned>(static_cast<int>(getSettingValue( cAnalysisMotionTrackingSpeedSettingId))),
                                      static_cast<unsigned>(static_cast<int>(getSettingValue( cAnalysisObjectSettingId))),
                                      static_cast<unsigned>(static_cast<int>(getSettingValue( cAnalysisCellTypeId))));
                _frameFocus.setValue(temp);
                setInternalState( eEngineRunning );
            }
            break;
            case eCmd:
            {
                if (_inputMode == eCmd && _importThread ) {
                    
                    if (_importThread->isRunning()) {
                        int status = _importThread->join();
                        if (status)
                        cout << "error " << status << endl;
                    }
                    
                    // Now using movie file information interprete first frame last frame
                    // TBD: another cmdline function
                    setSettingValue (cAnalysisFirstFrameSettingId, 0);
                    setSettingValue (cAnalysisLastFrameSettingId, framesLoaded() - 1);
                    
                    if (_cLineStartFrame >= 0 && _cLineStartFrame < framesLoaded())
                    setSettingValue (cAnalysisFirstFrameSettingId, _cLineStartFrame);
                    
                    if (_cLineEndFrame >= 0 && _cLineEndFrame < framesLoaded() &&
						_cLineEndFrame > _analysisFirstFrame)
                    setSettingValue (cAnalysisLastFrameSettingId, _cLineEndFrame);
                    
                    rcIRect cmdRect (_cLineRectTL.x(), _cLineRectTL.y(), _cLineRectSize.x(), _cLineRectSize.y());
                    
                    if (_analysisRect.contains (cmdRect))
                    setSettingValue (cAnalysisRectSettingId, rcValue (cmdRect));
                }
                
                ostringstream buf;
                buf << "Analyzing " << _movieFile.c_str() <<
                " saving results to " << _outputFileName.c_str();
                
                _observer->notifyStatus( buf.str().c_str() );
                setInternalState( eEngineRunning);
            }
            break;
            default:
            rmAssert (0);
            
        }
    }
    
    // stop the engine
    void rcEngineImpl::stop( void )
    {
#ifdef STATE_CHG_DEBUG
        fprintf(stderr, "rcEngineImpl::stop called\n");
#endif
        setInternalState( eEngineStopped );
        _observer->notifyStatus( "Stopping..." );
    }
    
    // Full internal reset, revert to built-in defaults
    void rcEngineImpl::internalReset()
    {
        PRINT_STATE_CHANGE("internalReset", getState(), eEngineUninitialized);
        setState(eEngineUninitialized);
        _slidingWindowSize = 3;
        _slidingWindowOrigin = eAnalyzerResultOriginCenter;
        _analysisMode = cAnalysisACISlidingWindow;
        // _cellType = cAnalysisCellGeneral;
        _statType = cAnalysisStatMedian;
        _msPerContraction = cAnalysisMuscleMsPerContractionDefault;
        _aciOptions = rcSimilarator::eACI;
        
        // set the initial (default) setting values
        _inputSource = eMovie;
        _inputMode = eFile;
        _ippMode = eIPPNone;
        _frameRate = 0;
        _movieFile = "";
        _imageFileNames = "";
        //_saveCtrl.outputFileName("");
        // _saveCtrl.movieFileName("");
        _imageFileNames = "";
        _writerSizeLimit = INT_MAX;
        _previewWriterSizeLimit = 512;
        _playbackCtrl.ctrlInfo(rcSettingInfo::eCurrentStateStopped |
                               rcSettingInfo::eCurrentStateFwd | 1);
        
        _showCellText = true;
        _showCellPosition = true;
        _showCellPath = false;
        
        _aciTemporalSampling = 1;
        _segmentationVectorDisplay = true;
        _exportMotionVectors = false;
        _exportBodyGraphics = true;
        _analysisFirstFrame = -1;
        _analysisLastFrame = -1;
        _channelConversion = rcSelectAverage;
        _analysisTreatmentObject = false;
        _developmentVideoEnabled = false;
        _developmentGraphicsEnabled = false;
        _energyGraphicsEnabled = false;
        _exportEnergyGraphics = false;
        _visualizeDevelopment = false;
        _cLineRectTL = rcIPair (-1, -1);
        _cLineRectSize = rcIPair (-1, -1);
        _cLineStartFrame = -1;
        _cLineEndFrame = -1;
        _enableGlobalMotionEst = 0;
        _doNormalize = 0;
        
        unlockedReset();
    }
    
    // Reset settings for a (re)start
    // Don't lock
    void rcEngineImpl::unlockedReset()
    {
        //_observer; // Retain old value
        //_captureThread; // TODO: do the right thing
        _startTime = 0.0;
        _currentTime = 0.0;
        _fileImages.clear();
        _movieFile = "";
        //  _saveCtrl.outputFileName("");
        // if (_movieCapture)
        //   _movieCapture->saveReset();
        _analysisRect = rcRect(0, 0, 0, 0);
        _motionVectorWriter = 0;
        _bodyVectorWriter = 0;
        _bodyHistoryVectorWriter = 0;
        _segmentVectorWriter = 0;
        _energyPlotWriter = 0;
        _videoWriter = (rcVideoWriter*) (0); // The old one should have been freed by observer
        //    _selectedPolys.setValue (0); // as above
        _writerGroup = 0;
        _graphicsWriterGroup = 0;
        mPreviewWriterGroup = 0;
        mSlidingEnergyPreviewWriter = 0;
        _processCount = 0;
        _groupCount = 0;
        if (_videoCacheP) {
            rcVideoCache::rcVideoCacheDtor(_videoCacheP);
            _videoCacheP = 0;
        }
        _fpsCalculator.reset();
        // set default attributes
        setFrameSize( 640, 480, rcPixelUnknown );
        updateHeaderLogs();
        
        PRINT_STATE_CHANGE("unlockedReset", getState(), eEngineStopped);
        setState(eEngineStopped); // Do not send a notification
    }
    // reset settings for a (re)start
    void rcEngineImpl::reset()
    {
        
        rcInputMode inputMode;
        
        {
            rcLock lock( _settingMutex );
            unlockedReset();
            inputMode = _inputMode;
        }
        
        
        if (inputMode != eFile && inputMode != eCmd) {
            rmAssert(!_importThread || !_importThread->isRunning());
            
            if (_captureThread) {
                if (_captureThread->isRunning()) {
                    cout << "    reset capture thread shutdown ";
                    int status = _captureThread->join();
                    if (status)
                    cout << "error " << status << endl;
                    else
                    cout << "OK" << endl;
                }
                
                _captureThread->start();
            }
        }
    }
    
    // Process new focus area
    void rcEngineImpl::process()
    {
        if (_inputMode == eCmd)
        {
            analyze ();
            return;
        }
        
        if ( _analyzerThread ) {
            if (_analyzerThread->isRunning()) {
                cout << "    analyzer thread shutdown ";
                int status = _analyzerThread->join();
                if (status)
                cout << "error " << status << endl;
                else
                cout << "OK" << endl;
            }
            
            _analyzerThread->start();
        }
    }
    
    
    // Set video frame size attribute values
    void rcEngineImpl::setFrameSize( uint32 width, uint32 height, rcPixel depth )
    {
        _frameWidth = width;
        _frameHeight = height;
        _frameDepth = depth;
    }
    
    // Preview start, called by camera
    void rcEngineImpl::startPreview()
    {
        createCameraPreviewWriters();
        setTimeLineRange();
    }
    
    // Preview resume, called by camera
    void rcEngineImpl::resumePreview()
    {
        // Reset writer, remove all segments and reset start time
        if ( mSlidingEnergyPreviewWriter )
        mSlidingEnergyPreviewWriter->reset();
    }
    
    // Set camera info, called by camera
    void rcEngineImpl::setCameraInfo(const rcMovieFileCamExt& info)
    {
        rcLock lock( _settingMutex);
        strstream s;
        
        // Write log to stream
        info.log( s, false ); // Don't show undefined values
        
        s << ends;
        _cameraInfo = s.str();
        s.freeze();
    }
    
    // Set capture info, called by camera
    void rcEngineImpl::setCaptureInfo(const rcMovieFileOrgExt& info)
    {
        rcLock lock( _settingMutex);
        strstream s;
        
        // Write log to stream
        info.log( s );
        
        s << ends;
        _captureLog = s.str();
        s.freeze();
    }
    
    // Set capture info, called by camera
    void rcEngineImpl::setCaptureInfo( const rcMovieFileOrgExt& org,
                                      const rcMovieFileExpExt& exp )
    {
        rcLock lock( _settingMutex);
        strstream s;
        
        // Write log to stream
        org.log( s );
        s << endl << "Experiment:" << endl;
        exp.log( s );
        
        s << ends;
        _captureLog = s.str();
        s.freeze();
    }
    
    // Set timeline display range based on current framerate
    void rcEngineImpl::setTimeLineRange()
    {
        // Set preview timeline range to 200 frames at current speed
        int rate = 15; //(int)_captureCtrl.getDecimationRate();
        double rangeLen =  100 * (1/15.0) * rate;
        rcTimestamp elapsed = getElapsedTime();
        _observer->notifyTimelineRange( elapsed-rangeLen, elapsed );
    }
    
    ///////// rcRunnable implementation ///////////////////////////
    
    // See rc_engineimplcapture.cpp
    
    ////////// rcCarbonLock implementation ///////////////////////
    
    // Lock qApp
    void rcEngineImpl::lock()
    {
        if ( _observer )
        _observer->notifyLockApp( true );
    }
    
    // Unlock qApp
    void rcEngineImpl::unlock()
    {
        if ( _observer )
        _observer->notifyLockApp( false );
    }
    
    // Analyze focus area
    void rcEngineImpl::analyze()
    {
        ++_processCount;
        
        
        if ( _inputMode == eFile || _inputMode == eCmd) {
            if ( framesLoaded() > 1 ) {
                uint32 imageCount = framesToAnalyze();
                
                if ( imageCount > 0 ) {
                    // At least one analysis operation is selected
                    rcEngineFocusData* focus = 0;
                    uint32 resultCount = 0;
                    rcRect aRect = getSettingValue( cAnalysisRectSettingId );
                    rcRect iRect = rcRect( 0, 0, _frameWidth, _frameHeight );
                    
                    if ( aRect.width() > 0 && aRect.height() > 0 ) {
                        if ( _analysisMode == cAnalysisCellTracking && ! _videoCacheP ) {
                            // No video cache
                            // Motion analysis enabled for Reify movies only
                            _observer->notifyError( "Functional visual analysis can be performed for Reify movies (.rfymov) only" );
                        } else {
                            // Create a new focus object
                            focus = new rcEngineFocusData( _writerManager,
                                                          aRect,
                                                          iRect,
                                                          framesLoaded(),
                                                          static_cast<rcAnalysisMode>(int(getSettingValue( cAnalysisModeSettingId ))),
                                                          _writerSizeLimit,
                                                          static_cast<unsigned>(static_cast<int>(getSettingValue( cAnalysisACISlidingWindowSizeSettingId ) )),
                                                          static_cast<rcAnalyzerResultOrigin>(static_cast<int>(getSettingValue( cAnalysisACISlidingWindowOriginSettingId ))),
                                                          _groupCount,
                                                          static_cast<unsigned>(static_cast<int>(getSettingValue( cAnalysisMotionTrackingSpeedSettingId ))),
                                                          static_cast<unsigned>(static_cast<int>(getSettingValue( cAnalysisObjectSettingId))),
                                                          static_cast<unsigned>(static_cast<int>(getSettingValue( cAnalysisCellTypeId ))));
                            // Gentlemen, start your engines
                            setInternalState( eEngineRunning );
                            setInternalState( eEngineStopped );
                            setInternalState( eEngineRunning );
                            // Analyze current focus area
                            resultCount = analyzeFocusArea( focus );
                            
                            delete focus;
                        }
                    } else {
                        // We need two or more images for analysis
                        _observer->notifyError( "No attention area specified for analysis" );
                    }
                } else {
                    // Image range is not valid
                    char buf[512];
                    
                    snprintf( buf, rmDim(buf), "Invalid range of images to analyze: [%i-%i]",
                             _analysisFirstFrame, _analysisLastFrame );
                    _observer->notifyError( buf );
                }
            } else {
                // We need two or more images for analysis
                _observer->notifyError( "Not enough images loaded for analysis" );
            }
        }
        
        if (_inputMode != eCmd)
        setInternalState( eEngineStopped );
        
        _observer->notifyAnalysisRect( getSettingValue( cAnalysisRectSettingId ) );
        
        // cerr << focusScript () << endl;
    }
    
    // our frame interrupt handler for frame capture
    void rcEngineImpl::frameInterrupt(const rcAnalyzerResult& result,
                                      const std::string& cameraStatus,
                                      uint32 missedFramesOnCapture,
                                      uint32 missedFramesOnSave,
                                      bool frameSaved )
    {
        rmAssert (!result.frameBuf());
        //    return;
        
#if 0
        // process the frame interrupt
        try {
            // Calculate average capture speed
            _fpsCalculator.addTime( result.frameBuf()->timestamp() );
            rmAssert(_movieCapture);
            
            bool delay = (_movieCapture->saveState() == eMovieFileState_Delay);
            _currentTime = rcTimestamp::now();
            bool first;
            _firstFrameSaved.getValue(first);
            if ((_startTime == 0.0) || delay || (first && frameSaved)) {
                _startTime = _currentTime;
                if (first && frameSaved) {
                    _firstFrameSaved.setValue(false);
                    // Start time is time of first saved frame
                    _startTime =  result.frameBuf()->timestamp();
                }
            }
            rcTimestamp timestamp = _currentTime - _startTime;
            
            rcEngineFocusData* temp;
            processCapturedData(_frameFocus.getValue(temp), timestamp,
                                cameraStatus, result, delay,
                                missedFramesOnCapture, missedFramesOnSave,
                                frameSaved );
            
        }
        
        
        
        catch (general_exception& x) {
            engineAbort(x.what());
        }
        catch (...) {
            engineAbort("unknown exception type");
        }
#endif
        
    }
    
    // Flush writers after capture has ended
    void rcEngineImpl::frameFlush()
    {
#ifdef STATE_CHG_DEBUG
        fprintf(stderr, "rcEngineImpl::frameFlush called\n");
#endif
        rcEngineFocusData* frameFocus;
        _frameFocus.getValue(frameFocus);
        if ( frameFocus ) {
            // Flush writers
            frameFocus->flushFocusData();
            _frameFocus.setValue(0);
        }
    }
    
    // Private methods
    
    static const double updateMovieInterval = 1.0;  // Movie display update interval in seconds
    static rcTimestamp lastMovieUpdateTime = 0.0;
    
    // our captured frame processing method
    void rcEngineImpl::processCapturedData(rcEngineFocusData* focus,
                                           rcTimestamp timestamp,
                                           const std::string& cameraStatus,
                                           const rcAnalyzerResult& result,
                                           bool delay,
                                           uint32 missedFramesOnCapture,
                                           uint32 missedFramesOnSave,
                                           bool frameSaved )
    {
        // Keep a reference to result frame
        rcWindow window(result.frameBuf());
        const size_t blen = 2048;
        char buf[blen];
        double captureFps = _fpsCalculator.fps();
        
        // ignore interrupts if we're not running
        if ( (getState() == eEngineRunning) || frameSaved ) {
            // Capturing
            
            // Timestamps for last video update
            // Initial value will cause an immediate update
            rcTimestamp now = rcTimestamp::now();
            rcTimestamp updateInterval = now - lastMovieUpdateTime;
            
            if ( updateInterval.secs() > updateMovieInterval ) {
                // Update movie display
                lastMovieUpdateTime = now;
                // Write video frame
                rcVideoWriter* temp;
                if (_videoWriter.getValue(temp) != 0) {
                    if (_observer->acceptingImageBlits()) {
                        _observer->notifyBlitData(&window);
                    }
                }
            }
        }
        else 
        {
            if ( frameSaved ) {
                cerr << "rcEngineImpl::processCapturedData error: frame " /*<< result.frameBuf()*/ << " at " << result.frameBuf()->timestamp();
                cerr << " saved but " << "result " << result.entropy() << " not stored" << endl;
            }
            
            // Previewing
            if (_observer->acceptingImageBlits() ) {
                _observer->notifyBlitData(&window);
            }
            
            if (missedFramesOnCapture)
            snprintf(buf, blen, "Previewing    Missed Frames: %d    Rate: %4.2f fps",
                     missedFramesOnCapture, captureFps);
            else
            snprintf(buf, blen, "Previewing    Rate: %4.2f fps",
                     captureFps);
            
            if ( mSlidingEnergyPreviewWriter ) {
                rcRect aRect = getSettingValue( cAnalysisRectSettingId );
                double res = result.entropy();
                // Replace undefined values
                if ( res < 0.0 )
                res = 0.0;
                mSlidingEnergyPreviewWriter->writeValue( result.frameBuf()->timestamp(), aRect, res );
            }
            // Notify elapsed time
            _observer->notifyTime(timestamp);
        }
        
        std::string compositeStatus = cameraStatus + std::string(buf);
        // Time does not REALLY pass in preview mode, update status only
        _observer->notifyStatus(compositeStatus.c_str());
    }
    
    // Create shared writers (all focus areas share these)
    void rcEngineImpl::createSharedWriters()
    {
        // Create default image data writer group
        if ( !_writerGroup ) {
            _writerGroup = _writerManager->createWriterGroup( cVideoWriterGroupStrings[0],
                                                             cVideoWriterGroupStrings[1],
                                                             eGroupSemanticsGlobalGraphics );
        }
        
        // Create default graphics data writer group
        if ( !_graphicsWriterGroup ) {
            _graphicsWriterGroup = _writerManager->createWriterGroup( cGraphicsWriterGroupStrings[0],
                                                                     cGraphicsWriterGroupStrings[1],
                                                                     eGroupSemanticsGlobalGraphics );
        }
        
        // Create shared writers (all focus areas share these)
        // Writer for displaying video
        
        if (_inputMode != eCmd)
        {
            rcVideoWriter* temp;
            if ( _videoWriter.getValue(temp) == 0) {
				_videoWriter = _writerManager->createVideoWriter( _writerGroup,
                                                                 eWriterVideo,
                                                                 _writerSizeLimit,
                                                                 _analysisRect );
            }
        }
    }
    
    // Create camera preview writers
    void rcEngineImpl::createCameraPreviewWriters()
    {
        // Create default writer group
        if ( !mPreviewWriterGroup ) {
            mPreviewWriterGroup = _writerManager->createWriterGroup( cPreviewWriterGroupStrings[0],
                                                                    cPreviewWriterGroupStrings[1],
                                                                    eGroupSemanticsCameraPreview );
        }
        
        if ( !mSlidingEnergyPreviewWriter ) {
            // Keep only a limited amount of values
            mSlidingEnergyPreviewWriter = _writerManager->createScalarWriter( mPreviewWriterGroup,
                                                                             eWriterACIPreview,
                                                                             _previewWriterSizeLimit,
                                                                             _analysisRect,
                                                                             0.0,
                                                                             1.0 );
            // Preview only, nothing to export
            mSlidingEnergyPreviewWriter->down_cast()->setExportable( false );
            // Add a human-readable string listing window options to track name and description
            int32 orig = getSettingValue( cAnalysisACISlidingWindowOriginSettingId );
            std::string optionString = rcWriterManager::optionString( eWriterACIWindow,
                                                                     static_cast<rcWriterWindowOrigin>(orig),
                                                                     static_cast<int32>(getSettingValue( cAnalysisACISlidingWindowSizeSettingId ) ) );
            rcWriter* writer = mSlidingEnergyPreviewWriter->down_cast();
            std::string nameWithOptions = writer->getName() + optionString;
            std::string descrWithOptions = writer->getDescription() + optionString;
            writer->setName( nameWithOptions.c_str() );
            writer->setDescription( descrWithOptions.c_str() );
        }
    }
    
    // Flush shared writers (all focus areas share these)
    void rcEngineImpl::flushSharedWriters()
    {
        // Writer for displaying video
        rcVideoWriter* videoWriter;    
        videoWriter = _videoWriter.getValue (videoWriter);
        if (videoWriter != 0)
        videoWriter->flush();
    }
    
    void rcEngineImpl::createGraphicsWriters()
    {
        // Create default visualization writer group
        if ( !_graphicsWriterGroup ) {
            _graphicsWriterGroup = _writerManager->createWriterGroup( cGraphicsWriterGroupStrings[0],
                                                                     cGraphicsWriterGroupStrings[1],
                                                                     eGroupSemanticsGlobalGraphics );
        }
        
        // Writer for displaying all Plotting
        if (!_energyPlotWriter ) {
            _energyPlotWriter = _writerManager->createGraphicsWriter( _graphicsWriterGroup,
                                                                     eWriterPlotter,
                                                                     _writerSizeLimit,
                                                                     _analysisRect );
            _energyPlotWriter->down_cast()->setExportable( true );
        }
    }
    
    void rcEngineImpl::createGraphicsWriters( bool motionVectors, bool bodyVectors,
                                             bool bodyHistoryVectors, bool segmentVectors )
    {
        // Create default visualization writer group
        if ( !_graphicsWriterGroup ) {
            _graphicsWriterGroup = _writerManager->createWriterGroup( cGraphicsWriterGroupStrings[0],
                                                                     cGraphicsWriterGroupStrings[1],
                                                                     eGroupSemanticsGlobalGraphics );
        }
        
        // Writer for displaying all motion vector graphics
        if ( motionVectors && !_motionVectorWriter ) {
            _motionVectorWriter = _writerManager->createGraphicsWriter( _graphicsWriterGroup,
                                                                       eWriterMotionVector,
                                                                       _writerSizeLimit,
                                                                       _analysisRect );
            _motionVectorWriter->down_cast()->setExportable( _exportMotionVectors );
        }
        // Writer for displaying locomotive body graphics
        if ( bodyVectors && !_bodyVectorWriter ) {
            _bodyVectorWriter = _writerManager->createGraphicsWriter( _graphicsWriterGroup,
                                                                     eWriterBodyVector,
                                                                     _writerSizeLimit,
                                                                     _analysisRect );
            _bodyVectorWriter->down_cast()->setExportable( _exportBodyGraphics );
        }
        
        // Writer for displaying locomotive body history vectors
        if ( bodyHistoryVectors && !_bodyHistoryVectorWriter ) {
            _bodyHistoryVectorWriter = _writerManager->createGraphicsWriter( _graphicsWriterGroup,
                                                                            eWriterBodyHistoryVector,
                                                                            _writerSizeLimit,
                                                                            _analysisRect );
            _bodyHistoryVectorWriter->down_cast()->setExportable( true );
        }
        
        // Writer for displaying segmentation vectors
        if ( segmentVectors && !_segmentVectorWriter ) {
            _segmentVectorWriter = _writerManager->createGraphicsWriter( _graphicsWriterGroup,
                                                                        eWriterSegmentVector,
                                                                        _writerSizeLimit,
                                                                        _analysisRect );
            rmAssert(_segmentVectorWriter);
            _segmentVectorWriter->down_cast()->setExportable( true );
        }
    }
    
    // Flush shared graphics writers (all focus areas share these)
    void rcEngineImpl::flushGraphicsWriters()
    {
        // Writer for displaying graphics
        if ( _motionVectorWriter ) {
            _motionVectorWriter->flush();
        }
        if ( _bodyVectorWriter ) {
            _bodyVectorWriter->flush();
        }
        if ( _bodyHistoryVectorWriter ) {
            _bodyHistoryVectorWriter->flush();
        }
        if ( _segmentVectorWriter ) {
            _segmentVectorWriter->flush();
        }
        if ( _energyPlotWriter ) {
            _energyPlotWriter->flush();
        }
        
    }
    
    // our internal state  method
    void rcEngineImpl::setInternalState( rcEngineState newState )
    {
        // ignore setting to current state
        if (newState == getState())
        return;
        
        // validate state transition
        rcEngineState foo;
        switch (foo = getState())
        {
            case eEngineUninitialized:	// engine has not been initialized
            if (newState != eEngineStopped)
            throw general_exception( "illegal state transition from eUninitialized" );
            break;
            
            case eEngineStopped:			// engine is stopped and not generating data
            if (newState != eEngineRunning && newState != eEnginePlayback
                && newState != eEngineProcessing )
            throw general_exception( "illegal state transition from eEngineStopped" );
            break;
            
            case eEngineRunning:			// engine is running and generating data
            if (newState != eEngineStopped)
            throw general_exception( "illegal state transition from eEngineRunning" );
            _isPausedToggle.setValue (false);     // set pause default off setting
            break;
            
            case eEnginePlayback:			// engine is in playback mode
            if (newState != eEngineStopped)
            throw general_exception( "illegal state transition from eEnginePlayback" );
            break;
            
            case eEngineProcessing:            // engine is processing a focus area
            if (newState != eEngineStopped)
            throw general_exception( "illegal state transition from eEngineProcessing" );
            _isPausedToggle.setValue (false);     // set pause default off setting
            break;
            
            case eEngineShuttingDown:		// engine is in the process of shutting down
            throw general_exception( "illegal state transition from eEngineShuttingDown" );
            break;
        }
        
        PRINT_STATE_CHANGE("setInternalState", getState(), newState);
        setState(newState);
        if ( _observer ) {
            _observer->notifyState( newState );
        }
    }
    
    // called to abort from frame interrupt handler
    void rcEngineImpl::engineAbort( const char* msg )
    {
        setInternalState( eEngineStopped );
        _observer->notifyError( msg );
    }
    
    // utility assertion
    void rcEngineImpl::assertInitialized( void )
    {
        if (getState() == eEngineUninitialized)
        throw rcEngineException( "engine uninitialized" );
    }
    
    // Perform ACI computation
    uint32 rcEngineImpl::entropyTracker( rcEngineFocusData* focus,
                                        vector<rcWindow>& focusImages,
                                        int32 cacheSz )
    {
        // Compute entropy
        
        uint32 analysisCount = 0;
        vector<double> results;
        double statusUpdateInterval = 1.5;
        
        // Write Appropriate Track Titles
        rcWriterManager::renameEntropyTrack( focus->energyWriter()->down_cast(), _aciOptions,
                                            rcPixel(_frameDepth), _channelConversion );
        // Construct progress message string
        std::string progressMsg = "Computing ";
        progressMsg += focus->energyWriter()->down_cast()->getName();
        progressMsg += "...%.2f%% complete";
        
        // Create graphics writers if applicable
        createGraphicsWriters();
        
        rcEngineProgress progressIndicator( this, _observer, progressMsg,statusUpdateInterval );
        rcSimilarator::rcCorrelationDefinition cdl =  rcSimilarator::eNorm;
        
        if ( progressIndicator.progress( (100.0 * (analysisCount))/focusImages.size() ) ) {
            // Abort
            _observer->notifyStatus( "Analysis stopped" );
            return 0;	}
        
        
        rcSimilarator sim(rcSimilarator::eExhaustive,
                          focusImages[0].depth(), focusImages.size(),
                          cacheSz, cdl, true, &progressIndicator);
        
        deque<double> signal;
        sim.fill(focusImages);
        
        if (sim.entropies(signal, _aciOptions))
        {
               
            deque<double>normed;
            norm_scaler<double, std::deque> ns;
            ns.operator () (signal, normed, 1);
         
            results.resize(0);
            copy(normed.begin(), normed.end(), back_inserter(results));
            
            rmAssert( results.size() == focusImages.size() );
            analysisCount = results.size();

            writeEntropyData( focus, focusImages, results );
            
            QVector<QPointF> data;
            if (convertToQpointData(focus, results, focusImages, data))
            {
                rcLock lock (_utilMutex);
                QString legend = focus->energyWriter()->down_cast()->getName();
                SharedCurveDataRef cv ( new CurveData (data, legend) );                
                _curveDataRef = cv;
                _observer->notifyPlotRequest (cv);

            }
           
            {
                rcLock lock (_utilMutex);
                deque<deque<double> > smm;
                uint32 msize = sim.matrixSz ();
                sim.selfSimilarityMatrix(smm);
                
                SharedCurveData2dRef cvv ( new CurveData2d (smm, msize) );
                _curveData2dRef = cvv;
                
                QString legend = focus->energyWriter()->down_cast()->getName();
                _observer->notifyPlot2dRequest (_curveData2dRef);
                std::cerr << " EngineImpl _observer notify called for 2d Matrix " << std::endl;
            }            
            
        }
        return analysisCount;
    }
    
    // Perform ACI computation for a sliding temporal window
    uint32 rcEngineImpl::entropyTrackerWindow( rcEngineFocusData* focus,
                                              vector<rcWindow>& focusImages,
                                              int32 cacheSz,
                                              bool clippedFrameBuf )
    {
        // Compute sliding window entropy
        uint32 analysisCount = 0;
        double statusUpdateInterval = 1.5;
        
        int slidingWindowSz = static_cast<int>(getSettingValue( cAnalysisACISlidingWindowSizeSettingId ));
        
        rcScalarWriter* sWriter = focus->slidingEnergyWriter();
        rcWriter* writer = sWriter->down_cast();
        
        rcAnalyzerOptions opt;
        // Set result origin
        opt.setOrigin( rcAnalyzerResultOrigin(static_cast<int>(getSettingValue( cAnalysisACISlidingWindowOriginSettingId ))) );
        // Set sliding window size
        opt.setWindowSize( static_cast<unsigned>( slidingWindowSz ) );
        // Set entropy definition
        opt.setEntropyDefinition( _aciOptions );
        // Set clip rect
        if ( !clippedFrameBuf )
        opt.setBound( focus->focusRect() );
        opt.setDepth (focusImages[0].depth());
        
        
        // Write Appropriate Track Titles
        rcWriterManager::renameEntropyTrack( writer, opt.entropyDefinition(),
                                            rcPixel(_frameDepth), _channelConversion );
        // Add a human-readable string listing window options to track name and description
        std::string optionString = rcWriterManager::optionString( eWriterACIWindow,
                                                                 static_cast<rcWriterWindowOrigin>(opt.origin()),
                                                                 opt.windowSize() );
        std::string name = writer->getName();
        std::string nameWithOptions = writer->getName() + std::string(" Short Term") + optionString;
        std::string descrWithOptions = writer->getDescription() + std::string(" Short Term") + optionString;
        writer->setName( nameWithOptions.c_str() );
        writer->setDescription( descrWithOptions.c_str() );
        
        
        // Create a grabber to read the image vector
        rcVectorGrabber imageGrabber( focusImages, cacheSz );
        
        // Create an analyzer
        rcAnalyzer analyzer( opt, imageGrabber, true);
        
        
        
        uint32 i = 0;
        rcFrameGrabberStatus status = eFrameStatusOK;
        // Construct progress message string
        std::string progressMsg = "Computing ";
        progressMsg += name;
        progressMsg += "...%.2f%% complete";
        rcEngineProgress progressIndicator( this, _observer,
                                           progressMsg,
                                           statusUpdateInterval );
        
        vector<double> signal;
        
        while ( status != eFrameStatusEOF ) {
            rcAnalyzerResult result;
            status = analyzer.getNextResult( result, true );
            if ( status == eFrameStatusOK ) {
                ++i;
                ++analysisCount;
                double entropy = result.entropy();
                // Ignore undefined values
                if ( entropy == -1.0 )
                continue;
                signal.push_back (entropy);
                sWriter->writeValue(result.frameBuf()->timestamp(), focus->focusRect(), entropy);
                
                if ( progressIndicator.progress( (100.0 * i)/(focusImages.size() - static_cast<int>(getSettingValue( cAnalysisACISlidingWindowSizeSettingId ))) ) ) {
                    // Abort
                    // Update elapsed time to refresh display
                    _observer->notifyStatus( "Analysis stopped" );
                    status = eFrameStatusEOF;
                }
                rmAssert( i <= focusImages.size() );
            } else {
                rcFrameGrabberError error = analyzer.getLastError();
                _observer->notifyStatus( rcFrameGrabber::getErrorString( error ).c_str() );
                // Bail out, we don't want to get stuck in an infinite loop
                break;
            }
        }
        
        
        return analysisCount;
    }
    
    
    
    int rcEngineImpl::writeTimedIndexScalarPairs ( const rcEngineFocusData* focus, const vector<int>& mins, const vector<int>& maxs,
                                                  const vector<rcWindow>& focusImages, rcScalarWriter* writer)
    {
        int32 count (0);
        
        if (mins.size() || maxs.size () )
        {
            
            for ( uint32 i = 0; i < focusImages.size(); ++i )
            {
                int which = 0; // 0 not a min or max, -1 is min, +1 is max
                for (vector<int>::const_iterator p = mins.begin(); p != mins.end(); p++)
                if (*p == (int) i) { which = -1; break; }
                
                if (which == 0)
                {
                    for (vector<int>::const_iterator p = maxs.begin(); p != maxs.end(); p++)
                    if (*p == (int) i) { which = 1; break; }
                }
                assert (which == 0 || which == -1 || which == 1);
                double value = (which == 0) ? 1.0 : 0.0;
                
                writer->writeValue( focusImages[i].frameBuf()->timestamp(), focus->focusRect(), value);
                count++;
            }
        }
        return count;
    }
    
    
    int rcEngineImpl::convertToQpointData ( const rcEngineFocusData* focus, const vector<double>& signal,
                                        const vector<rcWindow>& focusImages, QVector<QPointF>& dst)
    {
        rcUNUSED(focus);
        int32 count (0);
        dst.resize(0);

        if (signal.size())
        {
            float scaling_step = 1.0f / signal.size ();
            float ttime = 0.0f;
            for ( uint32 i = 0; i < focusImages.size(); ++i )
            {
                dst.append (QPointF (ttime, (float) signal[i]));
                ttime += scaling_step;
                count++;
            }
        }
        return count;
    }
    
    
    int rcEngineImpl::writeTimedScalar ( const rcEngineFocusData* focus, const vector<double>& signal,
                                        const vector<rcWindow>& focusImages, rcScalarWriter* writer)
    {
        int32 count (0);
        
        if (signal.size())
        {
            for ( uint32 i = 0; i < focusImages.size(); ++i )
            {
                // Write all data
                writer->writeValue( focusImages[i].frameBuf()->timestamp(), focus->focusRect(), signal[i]);
                count++;
            }
        }
        return count;
    }
    
    // @note: ACI values might be better floats ? Or atleast have the option ?
    int rcEngineImpl::writeTimedFloats ( const rcEngineFocusData* focus, const vector<float>& signal,
                                        const vector<rcWindow>& focusImages, rcScalarWriter* writer)
    {
        int32 count (0);
        
        if (signal.size())
        {
            for ( uint32 i = 0; i < focusImages.size() ; i++ )
            {
                // Write all data
                writer->writeValue( focusImages[i].frameBuf()->timestamp(), focus->focusRect(), (double) signal[i]);
                count++;
            }
        }
        
        
        return count;
    }
    
    
    // Perform analysis on a focus area
    uint32 rcEngineImpl::analyzeFocusArea( rcEngineFocusData* focus )
    {
        uint32 analysisCount = 0;
        rcIPPMode userMode = (rcIPPMode) (int) getSettingValue(cImagePreMappingSettingId);
        
        try {
            rcTimestamp duration = rcTimestamp::now();
            
            if ( focus ) {
                // Do analysis
                
                int32 cacheSz = 0;
                
                int32 aciTemporalSample (1);
                if (_analysisMode == cAnalysisACI || _analysisMode == cAnalysisACISlidingWindow)
				{
					aciTemporalSample  = getSettingValue( cAnalysisACIStepSettingId );
					cerr << "Temporal ACI Sampling " << aciTemporalSample << " [" << _analysisFirstFrame << "..." << _analysisLastFrame << "] " << endl;
				}
                
                uint32 imageCount = framesToAnalyze();
                cerr << imageCount << " Images ";
                if (aciTemporalSample > 1 && imageCount > ((uint32) (10 * aciTemporalSample)))
                imageCount = imageCount / aciTemporalSample;
                cerr << "Sampled " << "1 out of " << aciTemporalSample << endl;
                
                vector<rcWindow> focusImages (imageCount);
                const rcRect clipRect = focus->focusRect();
                bool clippedFrameBuf = false;
                
                bool combinedPhaseAndFluor (false);
                if (_videoCacheP)
				{
					const vector<rcMovieFileOrgExt>& orgHdrs = _videoCacheP->movieFileOrigins();
					rmAssert( !orgHdrs.empty() );
					mOrgExt = orgHdrs[0];
				}
                
                
                if ( clipRect.height() > 0 && clipRect.width() > 0 ) {
                    
                    _observer->notifyStatus( "Preparing analysis..." );
                    
                    // Clip to focus area
                    for (uint32 i = 0; i < imageCount; i++)
					{
						rcRect translatedRect (clipRect);
                        
						rcWindow rw (_fileImages[_analysisFirstFrame+i*aciTemporalSample], translatedRect);
                        rcTimestamp _ts = rw.frameBuf()->timestamp ();
                        
						/* @note: Do not access rw properties or data if _videoCache is used and
						 * depth is one that analysis can take in (@todo). In all other cases
						 * we check depth of the videoCache first.
						 * @note read the above note in loadFrames regarding access frames through videoCache
						 */
						if ((_videoCacheP && _videoCacheP->frameDepth() == rcPixel8) ||
                            (!_videoCacheP && rw.depth() == rcPixel8))
                        {
                            ipp (rw, focusImages[i], _ts);
                            clippedFrameBuf = userMode != eIPPNone;
                        }
						else if ((_videoCacheP && _videoCacheP->frameDepth() == rcPixel32) ||
                                 (!_videoCacheP && rw.depth() == rcPixel32))
                        {
                            if (!combinedPhaseAndFluor)
                            {
                                focusImages[i] = rcWindow(clipRect.width(), clipRect.height());
                                rfImageConvert32to8(rw, focusImages[i], _channelConversion);
                                focusImages[i].atTime (rw);
                                clippedFrameBuf = true;
                            }
                            else  // @note: right way is to implement Channel conversion all
                            {
                                focusImages[i] = rw;
                                clippedFrameBuf = false;
                            }
                        }
						else if ((_videoCacheP && _videoCacheP->frameDepth() == rcPixel16) ||
                                 (!_videoCacheP && rw.depth() == rcPixel16))
                        {
                            ipp (rw, focusImages[i], _ts);
                            clippedFrameBuf = userMode != eIPPNone;
                        }
						else
                        {
                            rmExceptionMacro (<< "Invalid Source and Depth");
                        }
						rmAssert ( _ts == focusImages[i].frameBuf()->timestamp ());
                        
					}
                    
                    
                    // Perform chosen analysis operation
                    switch ( _analysisMode )
                    {
                        case cAnalysisACI:
                        analysisCount = entropyTracker( focus, focusImages, cacheSz );
                        break;
                        
                        case cAnalysisACISlidingWindow:
                        analysisCount = entropyTrackerWindow( focus, focusImages, cacheSz,
                                                             clippedFrameBuf );
                        break;
                        
                        //                        case cAnalysisTemplateTracking:
                        //                            modelTracker( focus, focusImages, cacheSz );
                        //                            break;
                        
                        
                        default:
                        _observer->notifyError( "Unsupported analysis operation selected" );
                        break;
                    } // Analysis Mode
                } // Valid Clip Rect
                
                // Flush all writers
                flushSharedWriters();
                flushGraphicsWriters();
                focus->flushFocusData();
                
                // Display status message to stdout
                duration = rcTimestamp::now() - duration;
                cout << analysisCount << " image(s) analyzed in " << duration.secs() << " seconds";
                cout << ", " << duration.secs()/analysisCount << " seconds/image" << endl;
            } // Focus
        } // Try Block
        
        catch  (general_exception& x )
        {
            cerr << "Caught exception " << x.what() << endl;
            _observer->notifyError( x.what() );
        }
        catch (...)
        {
            cerr << "Caught unknown exception" << endl;
            _observer->notifyError( "Caught unknown exception\n" );
        }
        
        return analysisCount;
    }
    
    
    // Write ACI (entropy) data to appropriate writer
    void rcEngineImpl::writeEntropyData( rcEngineFocusData* focus, const vector<rcWindow>& images,
                                        const vector<double>& signal )
    {
        rmAssert( signal.size() == images.size() );
        rcScalarWriter* sWriter = focus->energyWriter();
        rcGraphicsWriter* plotter = focus->plotterWriter();
        rcVisualGraphicsCollection graphics;
        
        if ( plotter) {
            rcVisualSegmentCollection& segments = graphics.segments();
            // Green color, line width of 1 display pixel, 0.0 pixel origin
            rcVisualStyle style( rfRgb( 10, 140, 220 ), 1, rc2Fvector( 0.0f, 0.0f ) );
            rcVisualStyle laststyle( rfRgb( 220, 140, 10 ), 1, rc2Fvector( 0.0f, 0.0f ) );
            
#if 0
            rcFRect frame ((float) focus->focusRect().x(),
                           (float) focus->focusRect().y(),
                           (float) focus->focusRect().width(),
                           (float) focus->focusRect().height());
            rfPlot1Dsignal (signal.begin(), signal.end(), frame, segments);
#endif
            
            for ( uint32 i = 0; i < images.size(); ++i )
            {
				segments.push_back( (i == images.size () - 1) ? laststyle : style );
				segments.push_back(rcVisualRect (images[i].rectangle ()));
				plotter->writeValue( images[i].frameBuf()->timestamp(), focus->focusRect(), graphics );
            }
        }
        
        int32 count = writeTimedScalar (focus, signal, images, sWriter);
        rmAssert (count != 0);
    }
    
    // Produce cell-specific development image/graphics
    void rcEngineImpl::produceDebugResults( rcEngineFocusData* focus, rcWindow& debugImage,
                                           rcVisualGraphicsCollection& debugGraphics,
                                           const rcRect& focusRect, const rcTimestamp& curTimeStamp )
    
    {
        if ( !developerDebugging() )
        return;
        
        const int writeImage = getSettingValue( cAnalysisDevelopmentVideoDisplaySettingId );
        const int writeGraphics = getSettingValue( cAnalysisDevelopmentGraphicsDisplaySettingId );
        
        // Write debugging image and graphics to debug tracks
        if ( writeImage ) {
            rcVideoWriter* devVideo = focus->developmentVideoWriter();
            if ( devVideo )
            {
				devVideo->writeValue( curTimeStamp, focus->focusRect(), &debugImage );
            }
        }
        if ( writeGraphics  )
        {
            rcGraphicsWriter* devGraphics = focus->developmentGraphicsWriter();
            if ( devGraphics )
            {
                const rc2Fvector centerPixelOffset((float)1.5, (float)1.5);
                const rc2Fvector focusOffset(static_cast<int>(focusRect.x()),
                                             static_cast<int>(focusRect.y())); // Origin offset
                offsetGraphics( debugGraphics, /* focusOffset + */ centerPixelOffset);
                devGraphics->writeValue( curTimeStamp, focus->focusRect(), debugGraphics );
            }
        }
        
        
        
    }
    
    // Return number of loaded image frames
    int32 rcEngineImpl::framesLoaded() const
    {
        return (int32) _fileImages.size();
    }
    
    // Return number of frames to be analyzed (not necessarily all loaded frames )
    uint32 rcEngineImpl::framesToAnalyze() const
    {
        int32 n = 0;
        
        if ( _analysisFirstFrame >= 0 && _analysisLastFrame >= 0 ) {
            n = _analysisLastFrame - _analysisFirstFrame + 1;
            if ( n < 0 )
            n = 0;
        }
        
        return n;
    }
    
    void rcEngineImpl::updateHeaderLogs()
    {
        updateCameraLog();
        updateCaptureLog();
        updateConversionLog();
    }
    
    // Update capture log text
    void rcEngineImpl::updateCaptureLog()
    {
        if (_inputMode == eFile) {
            if ( _videoCacheP ) {
                strstream s;
                // Origin headers
                const vector<rcMovieFileOrgExt>& orgExts = _videoCacheP->movieFileOrigins();
                if ( !orgExts.empty() ) {
                    for ( uint32 i = 0; i < orgExts.size(); ++i ) {
                        // Write log to stream
                        orgExts[i].log( s );
                        if ( i < (orgExts.size()-1) )
                        s << endl;
                    }
                }
                // Add Avg frame rate
                s << " AvgFrameRate " << _videoCacheP->averageFrameRate() << " fps";
                
                // Experiment headers
                const vector<rcMovieFileExpExt>& expExts = _videoCacheP->movieFileExperiments();
                if ( !expExts.empty() ) {
                    s << endl << endl;
                    for ( uint32 i = 0; i < expExts.size(); ++i ) {
                        s << "Experiment:" << endl;
                        // Write log to stream
                        expExts[i].log( s );
                        if ( i < (expExts.size()-1) )
                        s << endl;
                    }
                }
                
                s << ends;
                s.freeze();
                _captureLog = s.str();
            } else
            _captureLog = " ";
        } else
        _captureLog = " ";
    }
    
    // Update camera info text
    void rcEngineImpl::updateCameraLog()
    {
        if (_inputMode == eFile) {
            if ( _videoCacheP ) {
                // Camera headers
                const vector<rcMovieFileCamExt>& camExts = _videoCacheP->movieFileCameras();
                
                if ( !camExts.empty() ) {
                    strstream s;
                    for ( uint32 i = 0; i < camExts.size(); ++i ) {
                        rcMovieFileCamExt cam = camExts[i];
                        // Write log to stream
                        cam.log( s, false ); // Don't show undefined values
                        if ( i < (camExts.size()-1))
                        s << endl;
                    }
                    s << ends;
                    s.freeze();
                    _cameraInfo = s.str();
                } else
                _cameraInfo = "Unspecified";
            } else
            _cameraInfo = " ";
        } else
        _cameraInfo = " ";
    }
    
    // Update conversion info text
    void rcEngineImpl::updateConversionLog()
    {
        if (_inputMode == eFile) {
            if ( _videoCacheP ) {
                strstream s;
                // Conversion headers
                const vector<rcMovieFileConvExt>& cnvExts = _videoCacheP->movieFileConversions();
                
                if ( !cnvExts.empty() ) {
                    s  << "Applied Conversions ["<< cnvExts.size() << "]"
                    << endl;
                    for ( uint32 i = 0; i < cnvExts.size(); ++i ) {
                        s << endl;
                        rcMovieFileConvExt cnv = cnvExts[i];
                        // Write log to stream
                        cnv.log( s );
                    }
                    s << ends;
                    s.freeze(); // Without freeze(), strstream::str() leaks the string it returns
                    _conversionInfo = s.str();
                }
                else
                _conversionInfo = "No conversions";
            } else
            _conversionInfo = " ";
        } else
        _conversionInfo = " ";
    }
    
    /******************************************************************************
     *	Engine instantiation
     ******************************************************************************/
    
    // The implementation of the factory method
    rcEngine* rcEngineFactory::getEngine( const char* implKey )
    {
        //  The engine singleton is created at run-time
        static rcEngineImpl engineImpl;
        
        rmUnused( implKey );
        
        return &engineImpl;
    }
    
    /******************************************************************************
     *	Importer instantiation
     ******************************************************************************/
    
    void rcEngineImporter::run()
    {
        if ( _engine ) {
            int inputSource = _engine->_inputSource;
            if ( inputSource == eImages)
            _engine->loadImages();
            else if ( inputSource == eMovie )
            _engine->loadMovie();
            
            // Save loaded images if we have been asked to
            _engine->saveFrames(_engine->_exportImagesDir);
        }
    }
    
    
    /******************************************************************************
     *	Analyzer instantiation
     ******************************************************************************/
    
    void rcEngineAnalyzer::run()
    {
        if ( _engine ) {
            _engine->analyze();
        }
    }
    
    bool rcEngineImpl::isEntire ()
    {
        rmAssert (_frameWidth <= 0 || _frameHeight <= 0);
        rcRect aRect = getSettingValue( cAnalysisRectSettingId );
        rcRect iRect = rcRect( 0, 0, _frameWidth, _frameHeight );
        return aRect == iRect;
    }
    
  
    
    
    // Make temporary file names
    std::string rcEngineImpl::makeTmpName(std::string exten)
    {
        // Get file name without preceding path
        std::string rawFileName = _movieFile;
        
        if (_inputMode == eCmd && !_movieFile.empty())
        {
            std::string slash( "/" );
            uint32 s = _movieFile.find_last_of( slash );
            if ( s != std::string::npos ) {
				uint32 len = _movieFile.size() - s - 1;
				if ( len > 0 )
                rawFileName = _movieFile.substr( s+1, len );
            }
        }
        else
        rawFileName = std::string ("/tmp/exp");
        
        // Use current time in seconds as a pseudo-unique prefix
        double secs = rcTimestamp::now().secs();
        
        ostringstream strout;
        std::string resFileName = rawFileName + rfToString (secs) + exten;
        return resFileName;
    }
    
    
    // Report on a bad grabber
    // Imagegrabber not valid
    void rcEngineImpl::reportOnBadGrabber (const rcVectorGrabber& imageGrabber) const
    {
        rcFrameGrabberError error = imageGrabber.getLastError();
        
        // Display grabber error message
        if (error != eFrameErrorOK) {
            strstream s;
            s << rcFrameGrabber::getErrorString( error ) << " "
            << const_cast<rcVectorGrabber&>(imageGrabber).getInputSourceName() << ends;
            _observer->notifyError(s.str());
            s.freeze(false);
            cerr << s.str() << endl;
        }
    }
    
    // Target is a tip. The exact model is declared in this file
    // The usual method would be to use the template file name
    //  std::string filename = getSettingValue( cTemplateFileNameId );
    //  rmAssert (file_exists (filename));
    //	_model2use = rcWindow (filename);
    
    void rcEngineImpl::model2use ()
    {
        _rect2use = rcIRect (308, 34, 24, 24);
        //  _model2use = rcWindow ();
        
    }
    
    