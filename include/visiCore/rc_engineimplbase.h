/******************************************************************************
 *   @file Copyright (c) 2003 Reify Corp. All Rights reserved.
 *
 *	$Id: rc_engineimplbase.h 6913 2009-07-01 03:31:53Z arman $
 *
 *	This file contains the definitions for engine base implementation class.
 *
 ******************************************************************************/

#ifndef _rcENGINE_BASEIMPL_H_
#define _rcENGINE_BASEIMPL_H_

#include <rc_analyzer.h>
#include <rc_engine.h>
#include <rc_writermanager.h>


// @note touch here when adding new tracking objects
#include <map>

#if WIN32
using namespace std;
#endif

/******************************************************************************
 *	Forwards
 ******************************************************************************/

/******************************************************************************
 *	Constants
 ******************************************************************************/

// analysis operation definitions
static const int cAnalysisACISlidingWindow = 0;
static const int cAnalysisACI = 1;
static const int cAnalysisCellTracking = 2;
static const int cAnalysisTemplateTracking = 3;



static const int cAnalysisStatMean = 0;
static const int cAnalysisStatMedian = 1;

// Re-use for more options
// Analysis mode constant definition
enum rcAnalysisMode {
    eAnalysisACISlidingWindow = cAnalysisACISlidingWindow,
    eAnalysisACI = cAnalysisACI,
    eAnalysisCellTracking = cAnalysisCellTracking,
    eAnalysisTemplateTracking = cAnalysisTemplateTracking
};


// Analysis mode constant definition
enum rcAnalysisAdvOptions
 {
   eNoProcessingSelected = 0, 
   eLineageProcessing = 1,
   eTemplateTracking = 2,
   eMacroDRGdiffusionProcessing = 3, 
};


/******************************************************************************
 *   Utility methods
 ******************************************************************************/

// Fold angle value with a specified mod value
double foldAngle( float& angle, const double& modValue );
// Offset graphics 
void offsetGraphics( rcVisualGraphicsCollection& graphics, const rc2Fvector& offset );

/******************************************************************************
 * Progress indication utility class
 ******************************************************************************/

class rcEngineProgress: public rcProgressIndicator {
  public:
    // ctor, update with every call
    rcEngineProgress( rcEngine* engine,
                      rcEngineObserver* observer,
                      const std::string& message );
    // ctor, specify updte interval in seconds
    rcEngineProgress( rcEngine* engine,
                      rcEngineObserver* observer,
                      const std::string& message,
                      const rcTimestamp& updateInterval );
    // virtual dtor
    virtual ~rcEngineProgress();

    //
    // rcProgressIndicator API
    //
    
    // Call this to update status bar with completion percentage
    virtual bool progress( uint32 percentComplete );

    // Call this to update status bar with completion percentage
    virtual bool progress( double percentComplete );
    
  private:
    rcEngine*         mEngine;
    rcEngineObserver* mObserver;
    const std::string    mFormatMessage;
    char              mMessageBuf[1024];
    rcTimestamp       mUpdateInterval; // Update interval in seconds
    rcTimestamp       mLastUpdate;     // Last time progress was updated
};


// Cell key; unique to each cell
typedef uint32 rcCellKey;


/*
 * @note added visualTarget as quick way of tracking a single / simple target without
 *       running kinetoscope
 */

#if 0
// A class encapsulating the group and its writers for a cell
class rcCellWriterGroup {
  public:
    // ctors
    rcCellWriterGroup();

    rcCellWriterGroup( rcWriterManager* manager, const rcVisualFunction& bfun, const rcRect& imageRect,
                       const rcRect& analysisRect, const rcTimestamp& trackStart,
                       const char* label);
	
    rcCellWriterGroup( rcWriterManager* manager, const rcVisualTarget& bfun, const rcRect& imageRect,
                       const rcRect& analysisRect, const rcTimestamp& trackStart,
                       const char* label);

    
    // dtor
    ~rcCellWriterGroup();

    // Accessors
    uint32 size() const;
    rcWriter* getWriter( rcWriterSemantics type ) const;

    // Mutators
	void writeValues( const rcTimestamp& timestamp, const rcRect& focus, const rcVisualFunction& result );
	void writeValues( const rcTimestamp& timestamp, const rcRect& focus, const rcVisualTarget& result );	
    void flush();
    
  private:
    rcWriterGroup*                  mCellGroup; // Cell writer group
    rcCellKey                       mKey;       // Unique cell key
    map<rcWriterSemantics, rcWriter*>   mWriters;   // Cell result writers
};

// A class encapsulating a collection of cell writer groups
class rcCellWriterGroupCollection {
  public:
    // ctor
    rcCellWriterGroupCollection( rcWriterManager* manager, const rcRect& imageRect,
                                 const rcRect& analysisRect );

    // Accessors
    bool hasCell( const rcCellKey& key ) const;
    rcCellWriterGroup* getCell( const rcCellKey& key );
    uint32 size() const;

    // Mutators
	void addCell( const rcVisualFunction& result, const rcTimestamp& trackStart);
	void addCell( const rcVisualTarget& result, const rcTimestamp& trackStart);	

    void flush();
    
  private:
    rcWriterManager* mManager;
    map<rcCellKey, rcCellWriterGroup> mCellWriters;   // Cell writer group objects
    rcRect           mImageRect;     // Whole image rect
    rcRect           mAnalysisRect;  // Analysis area rect
};
#endif
/******************************************************************************
*  Class to encapsulate the state for each focus area
******************************************************************************/

// TODO: refactor this mess to use writer collections
// @note ADD_Track

class rcEngineFocusData {
public:
    // ctor/dtor
    rcEngineFocusData( rcWriterManager* writerManager, const rcRect& analysisRect, const rcRect& imageRect,
                       uint32 imageCount, rcAnalysisMode analysisMode, uint32 sizeLimit, uint32 slidingWindowSize,
                       rcAnalyzerResultOrigin slidingWindowOrigin, uint32& groupCount, uint32 maxSpeed,
                       int treatmentObjectType, int32 cellType);
    
    ~rcEngineFocusData();

    //
    // Accessors
    // @note ADD_Track
    // Writer accessors
    rcScalarWriter*      cFreqWriter () { return mCfreqWriter; }
    rcScalarWriter*      cMeanFreqWriter() { return mcMeanFreqWriter; }
    rcScalarWriter*      cShortnWriter () { return mcShortnWriter;  }
    rcScalarWriter*      cMeanShortnWriter() { return mcMeanShortnWriter;  }

    rcScalarWriter*      energyWriter() { return mEnergyWriter; }
    rcScalarWriter*      slidingEnergyWriter() { return mSlidingEnergyWriter; }
    rcScalarWriter*      energyPeriodWriter() { return mPeriodEnergyWriter; }
    rcScalarWriter*      cellCountWriter() { return mCellCountWriter; }
    rcScalarWriter*      cellSpeedMeanWriter() { return mCellSpeedMeanWriter; }
    rcScalarWriter*      cellMeanSquareDisplacement() { return mCellMeanSquareDisplacementWriter; }
    rcScalarWriter*      cellPersistenceMeanWriter() { return mCellPersistenceMeanWriter; }
    rcScalarWriter*      cellObjectPersistenceMeanWriter() { return mCellObjectPersistenceMeanWriter; }
    rcScalarWriter*      cellCellPersistenceMeanWriter() { return mCellCellPersistenceMeanWriter; }
    rcScalarWriter*      cellAxisXMeanWriter() { return mCellAxisXMeanWriter; }
    rcScalarWriter*      cellAxisYMeanWriter() { return mCellAxisYMeanWriter; }
    rcScalarWriter*      cellMajorMeanWriter() {return mCellMajorMeanWriter;}
    rcScalarWriter*      cellMinorMeanWriter() {return mCellMinorMeanWriter;}
    rcScalarWriter*      cellCircMeanWriter() { return mCellCircMeanWriter; }
    rcScalarWriter*      cellEllipseMeanWriter() { return mCellEllipseMeanWriter; }
    rcScalarWriter*      tipDistanceWriter () { return mTipDistanceWriter;}
    rcGraphicsWriter*    plotterWriter() { return mPlotterWriter; }
    rcVideoWriter*       developmentVideoWriter() { return mDevelopmentVideoWriter; }
    rcGraphicsWriter*    developmentGraphicsWriter() { return mDevelopmentGraphicsWriter; }
    //    rcCellWriterGroupCollection* cellWriters() { return mCellWriters; }

    // Attribute accessors
    const rcRect&        focusRect() const { return mAnalysisRect; }
    rcRect&              focusRect() { return mAnalysisRect; }
    // Result accessors
    const vector<double>& cellCountResults() const { return mCellCountResults; }
    vector<double>&       cellCountResults() { return mCellCountResults; }

    const vector<rcTimestamp>& timeStamps() const { return mTimeStamps; }
    vector<rcTimestamp>&       timeStamps() { return mTimeStamps; }

    //
    // Mutators
    //
    void createCellWriters( );

    // Data writing
    void writeFocusData( rcTimestamp currentTime, int32 i );
    // Flush all writers
    void flushFocusData();
    // Clear all result containers
    void clearFocusData();
    // Set global track start times
    void setTrackStartTimes( rcTimestamp start );
    // Set cell-based track start times
    void setCellTrackStartTimes( rcTimestamp start );
              
private:
    // @note ADD_Track add Writers
    rcWriterManager*     mWriterManager;
    rcScalarWriter*      mcShortnWriter; 
    rcScalarWriter*      mcMeanShortnWriter; 
    rcScalarWriter*      mCfreqWriter; 
    rcScalarWriter*      mcMeanFreqWriter; 
    rcScalarWriter*      mEnergyWriter;                    // Total derivative energy
    rcScalarWriter*      mSlidingEnergyWriter;             // Total sliding energy
    rcScalarWriter*      mPeriodEnergyWriter;             // AutoCorrelation of Energy or other derived time series
    rcScalarWriter*      mCellCountWriter;                 // Total number of cells
    rcScalarWriter*      mCellSpeedMeanWriter;             // Mean speed
    rcScalarWriter*      mCellPersistenceMeanWriter;       // Mean cell persistence
    rcScalarWriter*      mCellObjectPersistenceMeanWriter; // Mean cell-to-object persistence
    rcScalarWriter*      mCellCellPersistenceMeanWriter;   // Mean cell-to-cell persistence
    rcScalarWriter*      mCellMeanSquareDisplacementWriter;// Mean square of cell displacements
    rcScalarWriter*      mCellAxisXMeanWriter;             // Mean X axis scale
    rcScalarWriter*      mCellAxisYMeanWriter;             // Mean Y axis scale
    rcScalarWriter*      mCellMajorMeanWriter;             // Major axis
    rcScalarWriter*      mCellMinorMeanWriter;             // Minor axis
    rcScalarWriter*      mCellCircMeanWriter;              // Mean Circularity
    rcScalarWriter*      mCellEllipseMeanWriter;           // Mean Ellipse Ratio
    rcScalarWriter*      mTipDistanceWriter;               // Tip Distance over time
    rcGraphicsWriter*    mPlotterWriter;                   // Graphics for Plotting 1D signals
    rcVideoWriter*       mDevelopmentVideoWriter;          // Images for development and debugging
    rcGraphicsWriter*    mDevelopmentGraphicsWriter;       // Graphics for development and debugging

    // Muscle cell demo writers
    rcScalarWriter*      mCellLengthWriter; // Cell length
    rcPositionWriter*    mCellBoundsWriter; // Cell bounds (edges)

    rcRect               mAnalysisRect;        // Analysis area
    rcRect               mImageRect;           // Image area
    rcWriterGroup*       mWriterGroup; // Writer group

    // Result arrays
    vector<double>       mCellCountResults;   // Total cell count
    vector<rcTimestamp>  mTimeStamps; // Time stamps for each result
    uint32             mMaxSpeed;  // Maximum cell speed
    vector<rcWriter*>    mGlobalWriters;       // All global (non-cell-specific) writers
    vector<rcWriter*>    mGlobalCellWriters;   // All global cell  writers
};

/******************************************************************************
*  Base engine implementation class
******************************************************************************/

class rcEngineBaseImpl : public rcEngine {
  public:
    rcEngineBaseImpl();
    virtual ~rcEngineBaseImpl();

    //
    // rcEngine API
    //
    
    // initialize the engine
    virtual void initialize( rcEngineObserver* observer );

	// start the engine
	virtual void start( void );

	// stop the engine
	virtual void stop( void );

    // shut down the engine
	virtual void shutdown( void );
    
	// reset the engine so it can be restarted later
	// all attributes will be reset to default values
	virtual void reset( void );
    
	// process captured data within user-selected focus area.
	// can be called repeatedly. 
	virtual void process( void );

    // get the current running rcTimestamp
	virtual rcTimestamp getElapsedTime( void );

    // get the absolute start rcTimestamp
	virtual rcTimestamp getStartTime( void );
    
	// get the engine state (see rcEngineState enum definition above)
	virtual rcEngineState getState( void );

	// get the engine attributes (see rcEngineAttributes enum definition above)
	virtual const rcEngineAttributes getAttributes( void );
    
    // get the number of engine settings categories
	virtual int getNSettingCategories( void );

	// get a settings category
	virtual rcSettingCategory getSettingCategory( int categoryNo );
    
  protected:
    //
    // Base implementation API
    //

    // Setting consistency check (no duplicate tag names etc.)
    bool checkSettings();
    // our internal state changing method
    virtual void setInternalState( rcEngineState newState );
    // utility assertion
    void assertInitialized( void );
    // Write all focus and image data to writers
    void writeData( rcEngineFocusData* focus );
    
	rcEngineObserver*	_observer;       // API to model
    rcEngineState		_state;          // Internal state
    int		            _frameWidth;	 // Image width
	int		            _frameHeight; 	 // Image height
    rcPixel        _frameDepth;     // Image depth, bytes per pixel
    rcWriterManager*    _writerManager;  // Data writer manager
    rcTimestamp			_startTime;      // First frame time
    rcTimestamp			_currentTime;    // Current frame time
    vector<rcSettingCategory> _settings; // Application settings
};

#endif // _rcENGINE_BASEIMPL_H_
