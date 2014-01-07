#ifndef UI_RCMODELDOMAIN_H
#define UI_RCMODELDOMAIN_H

#include <qobject.h>
//Added by qt3to4:
#include <QCustomEvent>

#include <rc_model.h>
#include <rc_security.h>
#include <rc_affine.h>


// This is true when internal development diagnostics are enabled
extern bool gDeveloperDebugging;

// Scaling mode for result display
enum rcResultScaleMode
{
      eResultScaleCurrentMinToMax = 1   // Result display scale is [currentMin-currentMax]
    , eResultScaleCurrentZeroToMax      // Result display scale is [0-currentMax]
    , eResultScaleExpectedMinToMax      // Result display scale is [expectedMin-expectedMax]
    , eResultScaleCompassCurrentMinToMax  // Result display scale is centered [expectedMin-expectedMax]
    , eResultScaleCompassExpectedMinToMax // Result display scale is centered [expectedMin-expectedMax]
    , eResultScaleUserMinToMax           // Result display scale is user-defined
    , eResultScaleSentinel               // Synthetic sentinel, not used as a scale value
};

class rcModelDomain : public QObject, public rcExperimentObserver
{
Q_OBJECT

public:
  rcModelDomain( QObject* parent, const char* modelKey );
  ~rcModelDomain();

  // static finder for the singleton model domain
  static rcModelDomain* getModelDomain( void );

  // get the current experiment
  rcExperiment* getExperiment( void );

  // get the current experiment attributes
  const rcExperimentAttributes getExperimentAttributes( void );

  // get the engine live attributes 
  bool operatingOnLiveInput ( void );
  bool storingLiveInput ( void );

  // set current experiment attributes
  virtual void setExperimentAttributes( const rcExperimentAttributes& attr );
    
  // get the current experiment state
  rcExperimentState getExperimentState( void );

  // get the current running rcTimestamp
  rcTimestamp getExperimentLength( void );

  // get the absolute experiment start rcTimestamp
  rcTimestamp getExperimentStart( void );
    
  // get expiration time from license file
  std::string getLicenseExpirationTime( void );

  // get the current cursor rcTimestamp
  rcTimestamp getCursorTime( void );
    
  // notify all setting widgets to update themselves
  void notifySettingChange( void );
    
  // general notification of a need to update the UI
  void notifyUpdateDisplay( void );

  // called to notify observer of a time cursor selection change.
  virtual void notifyCursorTime( const rcTimestamp& cursorTime );

  // called to notify observer of a analysis focus rect change.
  virtual void notifyAnalysisRect( const rcRect& rect );

  // called to notify observer of a analysis focus rect rotation
  virtual void notifyAnalysisRectRotation( const rcAffineRectangle&);
    
  // called to notify observer of a timeline scale change
  virtual void notifyTimelineScale( rcResultScaleMode scale );

  // called to notify observer of a change in multiplier
  virtual void notifyMultiplier( const double& multiplier );

  // notifation that video monitor must update itself
  void notifyUpdateMonitor( void );

  // notifation that development debugging mode has changed
  void notifyUpdateDebugging( void );
    
  // rcExperimentObserver implementation
  // called to ask the observer if it accepts notifyBlitData calls
  virtual bool acceptingImageBlits( void );
    
  // These will be called by a multi-threaded engine so they must be MT-safe
    
  // called to notify the observer that an error occurred.
  virtual void notifyError( const char* errorString );

  // called to warn the observer of some condition.
  virtual void notifyWarning( const char* warnString );

  // called to send status to the observer
  virtual void notifyStatus( const char* statusString );

  // Called to notify observer of a state change.
  virtual void notifyState( rcExperimentState newState,
			    bool immediateDispatch );

  // called to notify observer of elapsed time
  virtual void notifyTime( const rcTimestamp& elapsedTime );
    
  // called to notify observer that cursor time has programatically been changed
  virtual void notifyProgCursorTime( const rcTimestamp& elapsedTime );

  // Returns the number of eNotifyCursorEvent events queued up for processing
  virtual int32 progCursorTimeEventCount( ) const;

  // called to notify observer of a timeline range change
  virtual void notifyTimelineRange( const rcTimestamp& start ,
				    const rcTimestamp& end );

  // called by engine to notify observer of a timeline range change
  virtual void notifyEngineTimelineRange( const rcTimestamp& start ,
					  const rcTimestamp& end );

  // if the observer is accepting image blits, this is called to
  // tell the observer to blit the image
  virtual void notifyBlitData( const rcWindow* image );

  // if the observer is accepting image blits, this is called to
  // tell the observer to blit the graphics
  virtual void notifyBlitGraphics( const rcVisualGraphicsCollection* graphics );
    
  // Experimental lock notification, calls qApp->lock() and qApp->unlock()
  virtual void notifyLockApp( bool lock );

  // Experiment has changed, notify all widgets to update themselves
  virtual void notifyExperimentChange( void );

  // called to notify observer of the current video frame size
  virtual void notifyVideoRect( const rcRect& rect );

  // Immediate status update
  void notifyStatusInternal( const char* statusString );
	
  // called to ask the observer if it accepts
  // put polys calls
  virtual bool acceptingPolys ( void );
    
  // if the observer is accepting polys, this is called to
  // tell the observer to put the polys
  virtual void notifyPolys ();

  // if the observer is accepting polys, this is called to
  // tell the observer to get the possible changed polys    
  virtual void getPolys (rcPolygonGroupRef& polys );	
	
  rcPolygonGroupRef polys ()
  {
    return mSelectPolygons;
  }

  uint32 polysSize () const 
  {
    return mSelectPolygons.size();
  }

  void dumpPolys () const
  {
    const vector<rcPolygon>& pgs = const_cast<rcModelDomain*>(this)->mSelectPolygons.polys ();
    for (uint32 i = 0; i < pgs.size (); i++)
      {
	cerr << pgs[i] << endl;
      }
  }

  void notifySelectionState (bool b)
  {
    emit updateSelectionState (b);
  }

public slots:
	void requestStart( void );
	void requestStop( void );
    void requestProcess( void );
    void requestNew( void );
    void requestNewApp( void );    
    void requestOpen( rcExperimentImportMode mode );
    void requestClose( void );
    void requestSave( rcExperimentFileFormat format );
    void requestImageImport( void );
    void requestTifDirImport( void );
    void requestMovieImport( void );
    void requestSTKImport( void );
    void requestMovieSave( void );
    void requestInputSource( int );
    void timerTick( void );
    //  void requestTrackingDisplayGL ();
  void requestTrackingPause( void );    
  void stopTrackingPause ( void );    

signals:
	void elapsedTime( const rcTimestamp& time );
    void cursorTime( const rcTimestamp& time );
	void newState( rcExperimentState state );
    void updateDisplay( const rcWindow* image );
    void updateDisplay( const rcVisualGraphicsCollection* graphics );
    void updateSettings();
    void updateAnalysisRect( const rcRect& rect );
    void updateAnalysisRectRotation(  const rcAffineRectangle& affine );
    void updateTimelineRange( const rcTimestamp& start,
                              const rcTimestamp& end );
    void updateTimelineScale( rcResultScaleMode scale );
    void updateMultiplier( double multiplier );
    
    void updateDisplay();
    void updateStatus( const char* statusString );
    void imageImport();
    void tifDirImport();
    void movieImport();
    void stkImport();
    void movieSave();
    void updateInputSource( int i );
    void updateMonitorScale( double scaleFactor );
    void updateMonitorDisplay();
    void updateVideoRect( const rcRect& rect );
    void updateCameraState( bool liveCamera, bool liveStorage );
    void updateDebugging();
    void updateSelectionState (bool b);
	void requestTrackingDisplayGL ();



protected:


        
private:
    // Report license error
    // Note: intentionally obfuscated function name
    void rle( rcSecurityError status ); 
    // Return true if application license is valid
    // Note: intentionally obfuscated function name
    bool vl(); 
    
    rcExperimentDomain*	  mDomain;
    rcLicenseManager      mLicenseManager;
    rcTimestamp           mCursorTime;

    rcPolygonGroupRef mSelectPolygons;    

};


#endif // UI_RCMODELDOMAIN_H
