// @file Copyright (c) 2002 Reify Corp. All Rights reserved.

#ifndef _rcENGINEIMPL_H_
#define _rcENGINEIMPL_H_

// STL
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <strstream>
#include <algorithm>
#include <set>
#include <signal.h>
#include <sys/wait.h>
#include <stlplus_lite.hpp>
#include <cinder/Cinder.h>

// Visual
#include <rc_imagegrabber.h>
#include <rc_cinder_qtime_grabber.h>
#include <rc_reifymoviegrabber.h>


#include <rc_framebuf.h>
#include <rc_moviefileformat.h>
#include <rc_imageprocessing.h>
#include <rc_gen_movie_file.h>
    // #include <rc_capture.hpp>
#include <rc_persistencemanager.h>

// Analysis
#include <rc_analysis.h>
#include <rc_histstats.h>
#include <rc_analyzer.h>
#include <rc_similarity.h>
#include <rc_musclesegment.h>
#include <rc_ip.h>
#include <rc_ipconvert.h>
#include <rc_1dcorr.h>
#include <rc_find.hpp>
// Common
#include <rc_systeminfo.h>
#include <rc_setting.h>
#include <rc_thread.h>
#include <rc_engine.h>
#include <rc_atomic.h>
#include <rc_stlplus.h>
#include <rc_engineimplbase.h>
#include <rc_timinginfo.h>

    //#include "rc_engineimplcapture.h"
    //#include "rc_engineimplsave.h"
#include "rc_engineimplplayback.h"
#include "rc_featuredefs.h"
#include "rc_staticsettings.h"

// For plotting widget
#include <QVector>
#include <QPoint>

#if WIN32
using namespace std;
#endif



extern bool developerDebugging ();
extern bool developerDebugging (bool);



// Output Usage
ostream& rcVisibleBatchUsage (ostream&);

/******************************************************************************
 *	Forwards
 ******************************************************************************/

class rcEngineImpl;


/******************************************************************************
 *	Threaded importer implementation class
 ******************************************************************************/

class rcEngineImporter : public rcRunnable {
public:
    rcEngineImporter( rcEngineImpl* engine ) : _engine( engine ) { rmAssert( engine ); };


    virtual void run();

private:
    rcEngineImpl* _engine;
};


/******************************************************************************
 *	Threaded analyzer implementation class
 ******************************************************************************/

class rcEngineAnalyzer : public rcRunnable {
public:
    rcEngineAnalyzer( rcEngineImpl* engine ) : _engine( engine ) { rmAssert( engine ); };

    virtual void run();

private:
    rcEngineImpl* _engine;
};

/******************************************************************************
 *	rcEngine implementation class
 ******************************************************************************/

class rcEngineImpl : public rcEngine, public rcSettingSource, public rcCarbonLock
{
public:
    // default ctor
    rcEngineImpl();
    // virtual dtor is required
    virtual ~rcEngineImpl();



    //
    // Control methods
    //

    // initialize the engine
    virtual void initialize( rcEngineObserver* observer );

    // start the engine
    virtual void start( void );

    // stop the engine
    virtual void stop( void );

    // shut down the engine
    virtual void shutdown( void );

    // full internal reset, revert to built-in defaults
    virtual void internalReset();

    // unlocked reset called from ctor
    virtual void unlockedReset();

    // reset settings for a (re)start
    virtual void reset();

    // start process thread
    virtual void process();

    // perform analysis
    void analyze();

    // method to be called after rcMain to set the template path
    void setPathNameToTemplate(std::string argv0);

    //
    // Setting methods
    //
    bool setFromArgs (int32 argc, char **argv);

    std::string outputFile () const
        {
            return _outputFileName;
        }

    rcExperimentFileFormat outputFormat () const
        {
            return _saveFormat;
        }

    // get the number of engine settings categories
    virtual int getNSettingCategories( void );

    // get a settings category
    virtual rcSettingCategory getSettingCategory( int categoryNo );

    // get a settings category
    virtual rcSettingCategory getSettingCategory(char *categoryName);

    // Frame interrupt handler for frame capture
    void frameInterrupt(const rcAnalyzerResult& result,
                        const std::string& cameraStatus,
                        uint32 missedFramesOnCapture,
                        uint32 missedFramesOnSave,
                        bool frameSaved );
    // Flush writers after capture has ended
    void frameFlush();
    // Set video frame size
    void setFrameSize( uint32 width, uint32 height, rcPixel depth );
    // Preview start, called by camera
    void startPreview( );
    // Resume preview after save operation, called by camera
    void resumePreview();
    // Set camera info, called by camera
    void setCameraInfo(const rcMovieFileCamExt& info);
    // Set capture info, called by camera
    void setCaptureInfo(const rcMovieFileOrgExt& info);
    // Set capture info, called by camera
    void setCaptureInfo(const rcMovieFileOrgExt& org,
                        const rcMovieFileExpExt& exp);

    // Set timeline display range based on current framerate
    void setTimeLineRange();
    bool isEntire ();

    // Make a template if we have info in the app or from the command line
    // @note this is not currently setup for a dynamic model update
    void model2use (); 
	
    //
    // State accessors
    //

    // get the current running rcTimestamp
    virtual rcTimestamp getElapsedTime( void );

    // get the absolute start rcTimestamp
    virtual rcTimestamp getStartTime( void );

    // get the engine state (see rcEngineState enum definition above)
    virtual rcEngineState getState( void );

    // get the engine attributes (see rcEngineAttributes enum definition above)
    virtual const rcEngineAttributes getAttributes( void );

    // get the engine live attributes
    virtual bool operatingOnLiveInput ( void );
    virtual bool storingLiveInput ( void );

    // Pause tracking for user input
    virtual bool pauseTracking ();

    // continue tracking
    virtual bool resumeTracking ();

    virtual void notifyEngineOfPolys ( const rcPolygonGroupRef * polys );
    
    virtual void notifyEngineOfPlotRequests ( const CurveData * );    
    
    virtual void notifyEngineOfPlot2dRequests ( const CurveData2d * );        
    

    // Model access
    rcEngineObserver&	observer() { rmAssert(_observer); return *_observer; };

    //////////// rcSettingSource implementation //////////////////

    // get the current value of a setting from the setting source
    virtual const rcValue getSettingValue( int settingId );

    // set the new value of a setting to the setting source.
    virtual void setSettingValue( int settingId , const rcValue& value );

    // return whether this setting is current enabled.
    virtual bool isSettingEnabled( int settingId );

    // return whether this setting is current changable.
    virtual bool isSettingEditable( int settingId );

    // return whether this setting is currently persistable.
    virtual bool isSettingPersistable( int settingId );

    ////////// rcCarbonLock implementation ///////////////////////

    // Lock qApp
    virtual void lock();
    // Unlock qApp
    virtual void unlock();

    friend class rcEngineImporter;

protected:

/*! \fn void pauseIfPauseToggleOn ()
 *  \brief A member function.
 *  \brief if pauseToggle is false, immediately returns, else waits until pause is off
 *  \brief _isPausedToggle is a Atomic variable
 */

    bool isTrackingPaused ()
        {
            bool tmp (false);
            return _isPausedToggle.getValue (tmp);
        }

    // Create shared writers (all focus areas share these)
    void createSharedWriters();
    // Flush shared writers
    void flushSharedWriters();
    // Create graphics writers based on analysis params

    // One for cell segmentation and another for general
    void createGraphicsWriters( bool motionVectors, bool bodyVectors,
                                bool bodyHistoryVectors, bool segmentVectors );

    void createGraphicsWriters();

    // Flush graphics writers
    void flushGraphicsWriters();
    // Create camera preview writers
    void createCameraPreviewWriters();

    // To be used to just set state variable
    void setState( rcEngineState newState);

    // our internal state changing method
    void setInternalState( rcEngineState newState );

    // called to abort from frame interrupt handler
    void engineAbort( const char* msg );

    // utility assertion
    void assertInitialized( void );

    // Generic method to load frames from a rcFrameGrabber
    int loadFrames( rcFrameGrabber& grabber, rcFrameGrabberError& error );

    // Load images to memory, return number of images loaded
    int loadImages();

    // Load movie frames to memory, return number of frames loaded
    int loadMovie();
    // Load movie frames to memory, return number of frames loaded
    int loadMovie( const std::string& movieFile, rcFrameGrabberError& error );

    // Return number of loaded image frames
    int32 framesLoaded() const;
    // Return number of frames to be analyzed (not necessarily all loaded frames )
    uint32 framesToAnalyze() const;

    // Save imported frames out as JPG
    int32  saveFrames(std::string imageExportDir) const;

    // Perform analysis on a focus area
    uint32 analyzeFocusArea( rcEngineFocusData* focus );

    int32 convertToQpointData ( const rcEngineFocusData* focus, const vector<double>& signal,
                         const vector<rcWindow>& focusImages, QVector<QPointF>& dst);
    
    // Write entropy data from vector
    void writeEntropyData( rcEngineFocusData* focus,
                           const vector<rcWindow>& images,
                           const vector<double>& signal );

    // Write a timed scalar vector
    int writeTimedScalar ( const rcEngineFocusData* focus, const vector<double>& signal,
                           const vector<rcWindow>& focusImages, rcScalarWriter* writer);

    int writeTimedFloats ( const rcEngineFocusData* focus, const vector<float>& signal,
                           const vector<rcWindow>& focusImages, rcScalarWriter* writer);
    
    int writeTimedIndexScalarPairs ( const rcEngineFocusData* focus, const vector<int>& mins, 
                                    const vector<int>& maxs, const vector<rcWindow>& focusImages, rcScalarWriter* writer); 

    // Construct experiment attributes from movie header
    rcExperimentAttributes attributes( const rcMovieFileExpExt& exp ) const;

    // Captured frame processing method
    void processCapturedData(rcEngineFocusData* focus,
                             rcTimestamp timestamp,
                             const std::string& cameraStatus,
                             const rcAnalyzerResult& result,
                             bool delay,
                             uint32 missedFramesOnCapture,
                             uint32 missedFramesOnSave,
                             bool frameSaved );

    void postTrackingResults( rcEngineFocusData* focus, rcLocation&, bool found,
                              const rcRect& focusRect, const rc2Xform& mx, const rcTimestamp& curTimeStamp, const rcVisualSegmentCollection& );

	
    // Produce cell-specific development image/graphics
    void produceDebugResults( rcEngineFocusData* focus,
                              rcWindow& debugImage,   rcVisualGraphicsCollection& debugGraphics,
                              const rcRect& focusRect, const rcTimestamp& curTimeStamp );

    // ACI computation code
    // @note mutator
    uint32 entropyTracker( rcEngineFocusData* focus,
                             vector<rcWindow>& focusImages,
                             int32 cacheSz );

    // ACI sliding window computation code
    // @note mutator
    uint32 entropyTrackerWindow( rcEngineFocusData* focus,
                                   vector<rcWindow>& focusImages,
                                   int32 cacheSz,
                                   bool clippedFrameBuf );

    // Motion tracking code
    uint32 motionTracker( rcEngineFocusData* focus,
                            const vector<rcWindow>& focusImages,
                            int32 cacheSz );

    // Model tracking code
    uint32 modelTracker( rcEngineFocusData* focus,
                           const vector<rcWindow>& focusImages,
                           int32 cacheSz );
	
    // Muscle tracking code
    int32 muscleTracker(rcEngineFocusData* focus,
                          const vector<rcWindow>& focusImages, int32 cacheSz,
                          double msPerFrame);

    // Update all header logs
    void updateHeaderLogs();
    // Update capture data log text
    void updateCaptureLog();
    // Update camera info text
    void updateCameraLog();
    // Update conversiob info text
    void updateConversionLog();

    // Produce group statistics with the desired method
    template<class T>
    double groupStatistics (const vector<T>&);

    // Image Pre-Processing function to be applied to every frame
    void ipp (const rcWindow& image, rcWindow& dst, rcTimestamp& current);

    std::string focusScript ();

    void focusMyocyteSelection (const rcIRect& rect, vector<rc2Fvector>& ends, float& rad);

    // Produce registered image windows
    void registeredWindows (vector<rcWindow>& focusImages, int32& range);

    // Report on a bad grabber
    // @note should take base class !
    void reportOnBadGrabber (const rcVectorGrabber& imageGrabber) const;

    //
    bool produceMuscleDebugResults (const vector<rcPolygon>& rejects, rcMuscleSegmenter& segmenter,
                                    rcVisualSegmentCollection& segments);


    // Construct a temporary file name
    std::string makeTmpName(std::string exten);

    // Setting variables
    float              _contractionThreshold;
    bool                _batchMode; // Are we running from cmd line or the UI
    rcInputSource           _inputSource;
    rcInputMode		    _inputMode;
    rcIPPMode               _ippMode;
    float		        _frameRate;
    std::string            _movieFile;
    std::string            _imageFileNames;
    rcRect              _analysisRect;
    int                 _analysisMode;
    rcAffineRectangle _analysisAffine;
    rcSimilarator::rcEntropyDefinition     _aciOptions;
    uint32            _writerSizeLimit;
    uint32            _previewWriterSizeLimit;
    int                 _slidingWindowSize;
    rcAnalyzerResultOrigin _slidingWindowOrigin;
    rcMutex             _settingMutex;
    rcMutex             _utilMutex;
    int32             _doNormalize;    
    int32             _focusRotation;
    
    int32             _aciTemporalSampling;
    int32             _motionTrackingTargetSize;
    int32             _motionTrackingTargetSizeMultiplier;
    int32             _motionTrackingSampleSize;
    int32             _motionTrackingMaxSpeed;
    bool                _motionTrackingMotionVectorDisplay;
    bool                _motionTrackingBodyVectorDisplay;
    bool                _motionTrackingBodyHistoryVectorDisplay;
    int32             _motionTrackingFrameSampling;
    int32             _motionTrackingStep;
    bool                _exportMotionVectors;
    bool                _exportBodyGraphics;
    bool                _segmentationVectorDisplay;
    int32             _analysisFirstFrame;
    int32             _analysisLastFrame;
    rcChannelConversion _channelConversion;
    std::string            _captureLog;
    std::string            _cameraInfo;
    std::string            _conversionInfo;
    int                 _analysisTreatmentObject;     // custom options @note redo
    int                 _analysisTreatmentObjectSize; // Treatment object size
    bool                _developmentVideoEnabled;
    bool                _developmentGraphicsEnabled;
    bool                _visualizeDevelopment;
    int                 _cellType;
    int                 _statType;
    int                 _msPerContraction; // 0 implies AutoGen
    bool                _energyGraphicsEnabled;
    bool                _exportEnergyGraphics;
    int                   _enableGlobalMotionEst;
    // Internal state
    rcAtomicValue<rcEngineState> _state;
    int		            _frameWidth;
    int		            _frameHeight;
    int                 _frameDepth;
    vector<rcSettingCategory> _settings;

    // API to model
    rcEngineObserver*	_observer;

    // Common
    rcTimestamp		_startTime;
    rcTimestamp		_currentTime;
    rcWriterGroup*      _writerGroup; // Default writer group
    rcWriterGroup*      _graphicsWriterGroup; // Default graphics writer group
    rcGraphicsWriter*   _motionVectorWriter; // Motion vector visualization
    rcGraphicsWriter*   _bodyVectorWriter;   // Locomotive body vector visualization
    rcGraphicsWriter*   _bodyHistoryVectorWriter;   // Locomotive body history vector visualization
    rcGraphicsWriter*   _segmentVectorWriter;   // Segment vector visualization
    rcGraphicsWriter*   _energyPlotWriter;   // Segment vector visualization

    rcAtomicValue<rcVideoWriter*> _videoWriter; // Video frames
    uint32            _groupCount;  // Number of created writer groups
    rcAtomicValue<rcEngineFocusData*>  _frameFocus;
    rcFpsCalculator     _fpsCalculator; // Used to calculate display speed in FPS
    rcWriterManager*    _writerManager; // Data writer manager

    // Video capture/save specific
    rcThread*		  _captureThread;
    rcAtomicValue<bool>   _firstFrameSaved;
    rcWriterGroup*        mPreviewWriterGroup; // Camera preview writer group
    rcScalarWriter*       mSlidingEnergyPreviewWriter; // Total sliding energy, camera preview

    // Analysis specific
    rcEngineAnalyzer*   _analyzer;
    rcThread*		_analyzerThread;
    uint32            _processCount;        // Number of analysis executions

    // File import specific
    vector<rcWindow>    _fileImages;          // Loaded images
    vector<rc2Fvector>  _offsetsToFirst;        // Offsets to the first image
    rcThread*		_importThread;
    rcEngineImporter*   _importer;
    rcVideoCache*       _videoCacheP;
    rcEngineProgress*   _videoCacheProgress;
    rcMovieFileOrgExt   mOrgExt;

    // Video playback specific
    rcMoviePlaybackCtrl    _playbackCtrl;
    rcThread*		   _playbackThread;
    rcEngineMoviePlayback* _player;

    // Visualization specific
    bool               _showCellText;
    bool               _showCellPosition;
    bool               _showCellPath;

    // Result saving
    rcExperimentFileFormat _saveFormat;
    std::string           _outputFileName;

    // CmdLine temporary
    rcIPair _cLineRectTL;
    rcIPair _cLineRectSize;
    int32 _cLineStartFrame;
    int32 _cLineEndFrame;

    // Image Export Support
    std::string _exportImagesDir;
	
    // Template to use @todo
    std::string _model2useAbsPathFileName;
    rcIRect _rect2use;
    rcWindow _model2use;
    std::string _resourceFolderPath;
    std::string _installPath;
    
    // Toggle Mechanism
    rcAtomicValue<bool>   _isPausedToggle;
    rcPolygonGroupRef _selectedPolys; // Polygon Collection
    SharedCurveDataRef   _curveDataRef;
    SharedCurveData2dRef _curveData2dRef;

};


#endif // _rcENGINEIMPL_H_
