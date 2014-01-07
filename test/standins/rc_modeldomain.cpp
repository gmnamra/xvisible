/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_modeldomain.cpp 6064 2008-09-19 09:08:11Z arman $
 *
 *	This file contains model domain implementation.
 *
 ******************************************************************************/

#include <qapplication.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qthread.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QCustomEvent>

#define QStrFrcStr(foo) QString ((foo).c_str())

#include <rc_window.h>

#include "rc_appconstants.h"
#include "rc_modeldomain.h"
#include "rc_xmlparser.h"
#include "rc_trackmanager.h"

// #define DEBUG_LOG

// Utility functions

// Count lines to estimate progress better
static uint32 countLines( QFile& file )
{
    uint32 lines = 0;
    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull())
    {
        lines+=1;
        line = in.readLine();
    }

    return lines;
}

// Progress indicator class definition
class rcModelProgress: public rcProgressIndicator {
  public:
    // ctor
    rcModelProgress( rcModelDomain* domain, const std::string& message, double updateInterval,
                     QApplication* app, bool updateDisplay = false ) :
            mDomain( domain), mFormatMessage( message ), mLastPercentage( -1.0 ),
            mApp( app ), mUpdateInterval( updateInterval), mLastUpdate( cZeroTime ),
            mUpdateDisplay( updateDisplay ) {};
    // virtual dtor
    virtual ~rcModelProgress() {};

    //
    // rcProgressIndicator API
    //

    // Call this to update status bar with completion percentage
    virtual bool progress( uint32 percentComplete ) {
        return progress( static_cast<double>(percentComplete) );
    }

    // Call this to update status bar with completion percentage
    virtual bool progress( double percentComplete ) {
        if ( percentComplete != mLastPercentage )
        {
            rcTimestamp now = rcTimestamp::now();
            rcTimestamp timeSinceLastUpdate = now - mLastUpdate;

            // Update only if specified interval has passed
            if ( timeSinceLastUpdate.secs() > mUpdateInterval )
            {
                snprintf( mMessageBuf, rmDim( mMessageBuf ), mFormatMessage.c_str(), percentComplete );
                mDomain->notifyStatus( mMessageBuf );
                mLastPercentage = percentComplete;
                if ( mUpdateDisplay ) {
                    mDomain->notifyTime( mDomain->getExperimentLength() );
                    mApp->processEvents(QEventLoop::ExcludeUserInputEvents, 5 ); // Allow 5 ms for event processing
                } else {
                    mApp->processEvents( QEventLoop::ExcludeUserInputEvents, 1 ); // Allow 1 ms for event processing
                }
                mLastUpdate = now;
                // TODO: add abort logic for shutting down
            }
        }
        return false;
    }

  private:
    rcModelDomain*    mDomain;
    const std::string    mFormatMessage;
    char              mMessageBuf[1024];
    double            mLastPercentage; // Last progress percentage
    QApplication*     mApp;            // App handle to force event processing so GUI can be updated
    double            mUpdateInterval; // Update interval in seconds
    rcTimestamp       mLastUpdate;     // Last time progress was displayed
    bool              mUpdateDisplay;  // Update display in addition to status
};

// Model domain implementation

const char* cModelDomainName = "modelDomain";

static rcModelDomain* _modelDomainSingleton = 0;

rcModelDomain::rcModelDomain( QObject* parent)
        : QObject( parent, cModelDomainName ),
          mLicenseManager( cLicenseFilePath, true ),
          mEventQueueManager( this ),
          mCursorTime( 0.0 )
{
  

  mDomain = (rcExperimentDomain*)	rcExperimentDomainFactory::getExperimentDomain( "testDomain" );
  if (mDomain == 0)
    throw general_exception( "Can't find experiment domain" );
  mDomain->initialize( this );
  _modelDomainSingleton = this;

  // Optimize all Qt pixmaps for speed
//  QPixmap::setDefaultOptimization( QPixmap::BestOptim );

  rcSharedPolygonGroupPtr c0 = new rcPolygonGroup;
  rcPolygonGroupRef cellsRef (c0) ;
  mSelectPolygons = cellsRef;
}

rcModelDomain::~rcModelDomain()
{
	mDomain->shutdown();
}

// static finder for the singleton model domain
rcModelDomain* rcModelDomain::getModelDomain( void )
{
	// this will should never happen (we should create an instance of
	//	the model domain right after the Qt application object), but
	//	just to be sure....
	if (_modelDomainSingleton == 0)
		throw general_exception( "Fatal error: null model domain singleton" );

	return _modelDomainSingleton;
}

// get the current experiment
rcExperiment* rcModelDomain::getExperiment( void )
{
	return mDomain->getExperiment();
}


// get the current experiment attributes
const rcExperimentAttributes rcModelDomain::getExperimentAttributes( void )
{
    return mDomain->getExperimentAttributes();
}


	// get the engine live attributes
bool rcModelDomain::operatingOnLiveInput ( void )
{
  return mDomain->operatingOnLiveInput ();
}

bool rcModelDomain::storingLiveInput ( void )
{
  return mDomain->storingLiveInput ();
}


// set current experiment attributes
void rcModelDomain::setExperimentAttributes( const rcExperimentAttributes& attrs )
{
     mDomain->setExperimentAttributes( attrs );
}

// get the current experiment state
rcExperimentState rcModelDomain::getExperimentState( void )
{
    return mDomain->getExperimentState();
}

// get the current running rcTimestamp
rcTimestamp rcModelDomain::getExperimentLength( void )
{
    return mDomain->getExperimentLength();
}

// get the absolute experiment start rcTimestamp
rcTimestamp rcModelDomain::getExperimentStart( void )
{
    return mDomain->getExperimentStart();
}

// get expiration time from license file
std::string rcModelDomain::getLicenseExpirationTime( void )
{
    return mLicenseManager.getLicenseExpirationTime();
}

// get the current cursor rcTimestamp
rcTimestamp rcModelDomain::getCursorTime( void )
{
    return mCursorTime;
}

// notify all setting widgets to update themselves
void rcModelDomain::notifySettingChange( void )
{
    emit updateSettings();
}

// notify all setting widgets to update themselves
void rcModelDomain::notifyUpdateDisplay( void )
{
    emit updateDisplay();
}

// called to notify observer of a timeline range change
void rcModelDomain::notifyTimelineRange( const rcTimestamp& start,
                                         const rcTimestamp& end )
{
    emit updateTimelineRange( start, end );
}

// called to notify observer of a timeline scale change
void rcModelDomain::notifyTimelineScale( rcResultScaleMode scale )
{
    emit updateTimelineScale( scale );
}

// called to notify observer of a time cursor selection change.
void rcModelDomain::notifyCursorTime( const rcTimestamp& cTime )
{
    mCursorTime = cTime;
    mDomain->notifyCursorTime(cTime);
    emit cursorTime( cTime );
}

// notifation that vieo monitor must update itself
void rcModelDomain::notifyUpdateMonitor( void )
{
    emit updateMonitorDisplay();
}

// Notifation that development debugging mode has changed
void rcModelDomain::notifyUpdateDebugging( void )
{
    emit updateDebugging();
    notifySettingChange();
}

// rcExperimentObserver implementation

// These will be called by a multi-threaded engine so they must be MT-safe.
// Qt is NOT MT-safe so all Qt calls should be protetced by a global application
// lock if they can be called from multiple threads.

// Locking every Qt call in application code would be tedious and error-prone
// so Qt postEvent mechanism is used instead. Notifications coming from
// a MT engine will be posted as custom events to the main thread.
// Since the main thread is responsible for all Qt GUI activity,
// simultaneous unsafe calls to Qt methods cannot occur. When
// the main event loop processes the custom events, it will call
// rcModelDomain::customEvent() for each event.

// called to notify the observer that an error occurred.
void rcModelDomain::notifyError( const char* errorString )
{
    QCustomEvent* event = new QCustomEvent( eNotifyErrorEvent );

    event->setData( new QString( errorString ) );
    mEventQueueManager.postEvent( event );
}

// called to warn the observer of some condition.
void rcModelDomain::notifyWarning( const char* warnString )
{
    QCustomEvent* event = new QCustomEvent( eNotifyWarningEvent );

    event->setData( new QString( warnString ) );
    mEventQueueManager.postEvent( event );
}

// called to send status to the observer
void rcModelDomain::notifyStatus( const char* statusString )
{
    QCustomEvent* event = new QCustomEvent( eNotifyStatusEvent );

    event->setData( new QString( statusString ) );
    mEventQueueManager.postEvent( event );
}

// called to notify observer of a state change.
void rcModelDomain::notifyState( rcExperimentState state,
                                 bool immediateDispatch )
{
    if ( immediateDispatch ) {
        emit newState( state );
        emit updateSettings();
    } else {
        QCustomEvent* event = new QCustomEvent( eNotifyStateEvent );

        event->setData( new rcExperimentState( state ) );
        mEventQueueManager.postEvent( event );
    }
}

// called periodically to notify observer of a time change.
void rcModelDomain::notifyEngineTimelineRange( const rcTimestamp& start,
                                               const rcTimestamp& end )
{
    QCustomEvent* event = new QCustomEvent( eNotifyTimelineRangeEvent );

    event->setData( new rcTimestampPair( start, end ) );
    mEventQueueManager.postEvent( event );
}

// called periodically to notify observer of a time change.
void rcModelDomain::notifyTime( const rcTimestamp& cTime )
{
    const int32 queued = mEventQueueManager.queuedEvents( eNotifyTimeEvent );

	// Drop time events during live camera input if they start
    // queuing up
    if ( queued > 1 ) {
		if (mDomain->operatingOnLiveInput ())
			return;

    }
    QCustomEvent* event = new QCustomEvent( eNotifyTimeEvent );
    event->setData( new rcTimestamp( cTime ) );
    mEventQueueManager.postEvent( event );
}

// called periodically to notify observer of a programatic cursor change.
void rcModelDomain::notifyProgCursorTime( const rcTimestamp& cTime )
{
    QCustomEvent* event = new QCustomEvent( eNotifyCursorEvent );

    event->setData( new rcTimestamp( cTime ) );
    mEventQueueManager.postEvent( event );
}

// Returns the number of eNotifyCursorEvent events queued up for processing
int32 rcModelDomain::progCursorTimeEventCount( ) const
{
  rcModelDomain* ptr = const_cast<rcModelDomain*>(this);

  return ptr->mEventQueueManager.queuedEvents( eNotifyCursorEvent );
}

// called to notify observer of a analysis focus rect change.
void rcModelDomain::notifyAnalysisRect( const rcRect& rect )
{
    QCustomEvent* event = new QCustomEvent( eNotifyAnalysisRectEvent );

    event->setData( new rcRect( rect ) );
    mEventQueueManager.postEvent( event );
}

// called to notify observer of a analysis focus rect rotation
void rcModelDomain::notifyAnalysisRectRotation( const rcAffineRectangle& affine )
{
    QCustomEvent* event = new QCustomEvent( eNotifyAnalysisRectRotationEvent );

    event->setData( new rcAffineRectangle (affine) );
    mEventQueueManager.postEvent( event );
}

// called to ask the observer if it should blast an image to
//	a part of the screen of the observer's choosing.
bool rcModelDomain::acceptingImageBlits( void )
{
  return true;
}

// called to ask the observer if it should send us segmentation polys
bool rcModelDomain::acceptingPolys( void )
{
  return true;
}

// if the observer is accepting image blits, this is called to
// tell the observer to blit the image
void rcModelDomain::notifyBlitData( const rcWindow* image )
{
    int32 queued = mEventQueueManager.queuedEvents( eNotifyBlitEvent );

    // Post new event only if queue is empty
    if ( queued < 1 ) {
        QCustomEvent* event = new QCustomEvent( eNotifyBlitEvent );

        event->setData( new rcWindow( *image ) );
        mEventQueueManager.postEvent( event );
    }
}

// if the observer is accepting graphics, this is called to
// tell the observer to blit the graphics
void rcModelDomain::notifyBlitGraphics( const rcVisualGraphicsCollection* graphics )
{
    int32 queued = mEventQueueManager.queuedEvents( eNotifyBlitGraphicsEvent );

    // Post new event only if queue has less than 2 pending events
    if ( queued < 2 ) {
        QCustomEvent* event = new QCustomEvent( eNotifyBlitGraphicsEvent );

        event->setData( new rcVisualGraphicsCollection( *graphics ) );
        mEventQueueManager.postEvent( event );
    }
}

// if the observer is accepting polygon group, this is called to
// tell the observer to send it over to the engine
void rcModelDomain::notifyPolys ()
{
  mDomain->notifyPolys (&mSelectPolygons);
}

void rcModelDomain::getPolys (rcPolygonGroupRef& pgs )
{
  pgs = mSelectPolygons;
}

// called to notify observer of a change in multiplier
void rcModelDomain::notifyMultiplier( const double& multiplier )
{
    QCustomEvent* event = new QCustomEvent( eNotifyMultiplierEvent );

    event->setData( new double(multiplier) );
    mEventQueueManager.postEvent( event );
}

// called to notify that experiment has changed, all widgets need to update themselves
void rcModelDomain::notifyExperimentChange()
{
    QCustomEvent* event = new QCustomEvent( eNotifyExperimentChange );

    mEventQueueManager.postEvent( event );
}

// called to notify observer of the current video frame size
void rcModelDomain::notifyVideoRect( const rcRect& rect )
{
    QCustomEvent* event = new QCustomEvent( eNotifyVideoRectEvent );

    event->setData( new rcRect( rect ) );
    mEventQueueManager.postEvent( event );
}

// Global app lock
void rcModelDomain::notifyLockApp( bool lock )
{
#ifdef DEBUG_LOG
    cerr << "notifyLockApp " << lock << endl;
#endif
    if ( lock )
        qApp->lock();
    else
        qApp->unlock();
}

// public slots:
void rcModelDomain::requestStart( void )
{
	try
	{
        if ( vl() )
            mDomain->startExperiment();
	}
	catch (exception& x)
	{
		QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
	}
}

void rcModelDomain::requestStop( void )
{
	try
	{
		mDomain->stopExperiment();
        // Camerate state change, no more storage
        rcExperimentAttributes attr = mDomain->getExperimentAttributes();
        emit updateCameraState(mDomain->operatingOnLiveInput(), false );
	}
	catch (exception& x)
	{
		QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
	}
}

void rcModelDomain::requestProcess( void )
{
    try
    {
        if ( vl() )
            mDomain->processExperiment();
    }
    catch (exception& x)
    {
        QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
    }
}


void rcModelDomain::requestTrackingPause ( void )
{
    try
    {
      if ( vl() )
	{
	  mDomain->pauseTrackingExperiment();

	}
    }
    catch (exception& x)
    {
      QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
    }
}

void rcModelDomain::stopTrackingPause ( void )
{
    try
    {
      if ( vl() )
	{
	  mDomain->doneSelecting ();
	}
    }
    catch (exception& x)
    {
      QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
    }
}

void rcModelDomain::requestNew( void )
{
    try
    {
        rcExperimentState state = getExperimentState();

        if ( state != eExperimentEmpty && state != eExperimentLocked ) {
            // No data has been saved, prompt user
            switch( QMessageBox::warning( 0, cAppName,
                                          "Do you want to save experiment data before starting a new experiment?\n\n"
                                          "If you don't save, your data will be lost.",
                                          "&Don't Save", "Cancel", "&Save",
                                          2, 1 ) ) {
                case 0: // Don't Save clicked or Alt+D pressed
                    mDomain->newExperiment();
                    break;
                case 1: // Cancel clicked or Alt+C pressed or Escape pressed
                    // Do nothing
                    break;
                case 2: // Save clicked or Alt+S pressed
                    requestSave( eExperimentNativeFormat );
                    if ( getExperimentState() == eExperimentLocked )
                        mDomain->newExperiment();
                    break;
            }
        } else
            mDomain->newExperiment();
    }
    catch (exception& x)
    {
        QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
    }
}

// Launch new application instance
void rcModelDomain::requestNewApp( void )
{
    // TODO: we should use multiple document window architecture

    QString cmd;
    const char* appName = qApp->argv()[0];

    // Background task
    cmd.sprintf("%s -psn &", appName );

    // Launch a new application instance
    system( cmd.latin1() );
}

void rcModelDomain::requestOpen( rcExperimentImportMode mode )
{
    try
    {
        if ( !vl() )
            return;

        notifyStatus( "Loading experiment data..." );

        rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
        rcExperimentFileFormat format =  eExperimentNativeFormat;

        // First, prompt for unsaved data from current experiment
        rcExperimentState state = getExperimentState();

        if ( state != eExperimentEmpty && state != eExperimentLocked ) {
            // No data has been saved, prompt user

            switch( QMessageBox::warning( 0, cAppName,
                                          "Do you want to save experiment data before opening a new experiment?\n\n"
                                          "If you don't save, your data will be lost.",
                                          "&Don't Save", "Cancel", "&Save",
                                          2, 1 ) ) {
                case 0: // Don't Save clicked or Alt+D pressed
                    break;
                case 1: // Cancel clicked or Alt+C pressed or Escape pressed
                    // Do nothing
                    return;
                    break;
                case 2: // Save clicked or Alt+S pressed
                    requestSave( format );
                    break;
            }
        }

        // TODO: query (some of) these from engine settings
        QString caption = QStrFrcStr (persistenceManager->fileFormatImportCaption( format ));

        switch ( mode ) {
            case eExperimentImportAllSettings:
                caption += " Settings";
                break;
            case eExperimentImportExperimentSettings:
                caption += " Template";
                break;
            case eExperimentImportAllData:
                caption += " Data";
                break;
            case eExperimentImportAll:
            default:
                break;
        }

        QString filter = QStrFrcStr (persistenceManager->fileFormatImportFilter( eExperimentNativeFormat ));

        // Ready to choose a file to open
        QString s = Q3FileDialog::getOpenFileName(
            "Untitled",
            filter,
            0,
            caption,
            caption );

        if (s != QString::null)
        {
            rcModelProgress prepareProgress( this, "Preparing to load experiment data...%.2f%% complete", 1.0, qApp );
            rcModelProgress loadProgress( this, "Loading experiment data...%.2f%% complete", 2.5, qApp );

            prepareProgress.progress( 30.0 );
            // Import from file to XML tree
            QApplication::setOverrideCursor(Qt::waitCursor );

            // Determine import directory
            int idx = s.findRev( '/' );
            if ( idx != -1 ) {
                QString dir = s.left( idx + 1 );
                std::string d( dir.latin1() );
                // Set import directory so that engine can access it
                persistenceManager->setImportDirectory( d );
            }

            // Input file
            QFile xmlFile( s );

            // Special Reify handlers
            rcXMLErrorHandler errorHandler( s );
            rcXMLParser parser( mode, countLines( xmlFile ),
                                &loadProgress, &errorHandler );
            prepareProgress.progress( 60.0 );

            // Setup parser
            QXmlInputSource source( &xmlFile );
            QXmlSimpleReader reader;
            reader.setContentHandler( &parser );
            reader.setErrorHandler( &errorHandler );
            prepareProgress.progress( 100.0 );
            // Parse input file
            reader.parse( source );
            qApp->processEvents( QEventLoop::ExcludeUserInputEvents, 10 );
            QApplication::restoreOverrideCursor();

            if ( errorHandler.fatalErrors() > 0 ) {
                // Fatal errors, do not import
                QString message = QString( "Experiment loading aborted: %1 fatal error(s):\n\n%2\n" )
                    .arg( errorHandler.fatalErrors() )
                    .arg( errorHandler.errorString() );
                // Display error dialog
                QMessageBox::critical( 0 , cAppName , message , 1 , 0 );
            } else {
                // Import from tree to experiment
                rcModelProgress treeIndicator( this, "Processing experiment data...%.2f%% complete",
                                               2.5, qApp, true );
                notifyStatus( "Processing experiment data..." );
                QApplication::setOverrideCursor( Qt::waitCursor );
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10 );

                int errors = mDomain->loadExperiment( s.latin1(), parser.elementTree(), mode, &treeIndicator );
                QApplication::restoreOverrideCursor();
                if ( errors > 0 ) {
                    // Errors during import, display warning
                    QString message = QString( "Experiment loading aborted: %1 fatal error(s) during XML tree import.\n\nExperiment data file may contain unsupported elements or it may be corrupted.\n" )
                        .arg( errors );
                    // Display error dialog
                    QMessageBox::critical( 0, cAppName, message, 1, 0 );
                }
            }
        }
        notifyStatus( "Ready" );
    }
    catch (exception& x)
    {
        QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
    }
}

void rcModelDomain::requestClose( void )
{
    try
    {
        notifyStatusInternal( "Closing experiment..." );

        rcExperimentState state = getExperimentState();

        if ( state != eExperimentEmpty && state != eExperimentLocked ) {
            // No data has been saved, prompt user

            switch( QMessageBox::warning( 0, cAppName,
                                          "Do you want to save experiment data before closing?\n\n"
                                          "If you don't save, your data will be lost.",
                                          "&Don't Save", "Cancel", "&Save",
                                          2, 1 ) ) {
                case 0: // Don't Save clicked or Alt+D pressed
                    mDomain->newExperiment();
                    break;
                case 1: // Cancel clicked or Alt+C pressed or Escape pressed
                    // Do nothing
                    break;
                case 2: // Save clicked or Alt+S pressed
                    requestSave( eExperimentNativeFormat );
                    if ( getExperimentState() == eExperimentLocked )
                        mDomain->newExperiment();
                    break;
            }
        } else {
            QApplication::setOverrideCursor( Qt::waitCursor );

            mDomain->newExperiment();
            QApplication::restoreOverrideCursor();
        }
    }
    catch (exception& x)
    {
        QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
    }
}

// Replace native extension with current extension if necessary
static void processExtension( QString& defaultName,
                              const QString& currentExt,
                              const QString& nativeExt )
{
    // Purge native extension from name
    if ( defaultName.endsWith( nativeExt ) ) {
        defaultName.truncate( defaultName.length() - nativeExt.length() );
    }
    // Add new extension if necessary
    if ( defaultName.length() > 0 && !defaultName.endsWith( currentExt ) )
        defaultName += currentExt;
}

void rcModelDomain::requestSave( rcExperimentFileFormat format )
{
    try
    {
        notifyStatusInternal( "Saving experiment data..." );

        rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
        rcExperimentAttributes attr =  mDomain->getExperimentAttributes();
        QString defaultName;

	if ( !attr.inputName.empty() )
	  defaultName = attr.inputName.c_str();
        else if ( !attr.fileName.empty() )
            defaultName = attr.fileName.c_str();
        else if (! attr.title.empty() ) {
            defaultName = attr.title.c_str();
            // We need to replace slashes with something else
            defaultName.replace( QChar('/'), "-" );
        }

        QString filter = QStrFrcStr (persistenceManager->fileFormatExportFilter( format ));
        QString ext = QStrFrcStr (persistenceManager->fileFormatExportExtension( format ));
        QString caption = QStrFrcStr (persistenceManager->fileFormatExportCaption( format ));
        QString widgetName;
        QString errorProlog;
        // Native file format extension
        QString nativeExt = QStrFrcStr (persistenceManager->fileFormatExportExtension( eExperimentNativeFormat ));
        std::string progressMessage( "Saving experiment data...%.2f%% complete" );

        switch ( format ) {
            case eExperimentCSVFormat:
                widgetName = QString( "export-dialog" );
                progressMessage = std::string( "Exporting experiment data...%.2f%% complete" );
                processExtension( defaultName, ext, nativeExt );

            case eExperimentNativeFormat:
                widgetName = QString( "save-dialog" );
                progressMessage = std::string( "Saving experiment data...%.2f%% complete" );
                break;

            case eExperimentMolDevSTKFormat :
                widgetName = QString( "save-dialog" );
                progressMessage = std::string( "Saving stk data...%.2f%% complete" );
                processExtension( defaultName, ext, nativeExt );
                break;

            case eExperimentQuickTimeMovieFormat:
                widgetName = QString( "export-dialog" );
                progressMessage = std::string( "Exporting image data...%.2f%% complete" );
                processExtension( defaultName, ext, nativeExt );
                break;

            case eExperimentNativeMovieFormat:
                widgetName = QString( "save-dialog" );
                progressMessage = std::string( "Saving image data...%.2f%% complete" );
                processExtension( defaultName, ext, nativeExt );
                break;

            default:
                rmAssert( 0 );
                break;
        };

        QString fileName = Q3FileDialog::getSaveFileName(
            defaultName,
            filter,
            0,
            widgetName,
            caption,
            &filter,
            true );

        // If user canceled, the fileName will be QString::null
        if (fileName == QString::null) {
            notifyStatusInternal( "Ready" );
            return;
        }

        // Add extension if necessary
        if (!fileName.endsWith( ext ))
            fileName += ext;

        // Warn if file already exists
        if (QFile::exists( fileName ))
        {
            if (QMessageBox::warning( 0 , cAppName , "File already exists: overwrite?" ,
                                      QMessageBox::Ok , QMessageBox::Cancel ) == QMessageBox::Cancel) {
                notifyStatusInternal( "Ready" );
                return;
            }
        }

        // Export the experiment data.
        QApplication::setOverrideCursor(Qt::waitCursor );
        rcModelProgress progressIndicator( this, progressMessage, 2.5, qApp );

        int err = mDomain->saveExperiment( fileName.latin1(), format, &progressIndicator );
        QApplication::restoreOverrideCursor();

        if ( err ) {
            // Error during export/save!
            QString error( caption );
            error += " error: ";
            error += strerror( err );
            QMessageBox::critical( 0 , cAppName , error , 1 , 0 );
        }
        // Experiment file name may have changed, do a notification
        notifySettingChange();
        notifyStatus( "Ready" );
    }
    catch (exception& x)
    {
        QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
    }
}

void rcModelDomain::requestImageImport( void )
{
    if ( vl() )
        emit imageImport();
}

void rcModelDomain::requestTifDirImport( void )
{
    if ( vl() )
        emit tifDirImport();
}

void rcModelDomain::requestMovieImport( void )
{
    if ( vl() ) {
        emit movieImport();
        // Notify domain so that experiment title can be updated with movie name
        mDomain->notifyMovieImport();
    }
}

void rcModelDomain::requestSTKImport( void )
{
    if ( vl() ) {
        emit stkImport();
        // Notify domain so that experiment title can be updated with STK name
        mDomain->notifyMovieImport();
    }
}

void rcModelDomain::requestMovieSave( void )
{
    if ( vl() ) {
         // Scale monitor to fit
        emit updateMonitorScale( -1.0 );
        // Change timeline mode
        emit updateTimelineScale( eResultScaleCurrentMinToMax );
        // Start saving capture to disk
        emit movieSave();
        // Camerate state change
        emit updateCameraState( true, true );
    }
}

void rcModelDomain::requestInputSource( int i )
{
    emit updateInputSource( i );
}


void rcModelDomain::timerTick( void )
{
	rcTimestamp time = mDomain->getExperimentLength();

#ifdef DEBUG_LOG
    cout << "timerTick " << time.secs() << endl;
#endif

	emit elapsedTime( time );
}

// protected

// Handle custom events
void rcModelDomain::customEvent( QCustomEvent* e )
{
    int type = e->type();

#ifdef DEBUG_LOG
    cout << "rcModelDomain user event " << type << " with value " << e->data() << endl;
#endif

    switch ( type ) {
        case eNotifyErrorEvent:
        {
            QString* string = (QString*)e->data();
            QMessageBox::critical( 0 , cAppName , *string , 1 , 0 );
            delete string;
        }
        break;

        case eNotifyWarningEvent:
        {
            QString* string = (QString*)e->data();
            QMessageBox::warning( 0 , cAppName , *string , 1 , 0 );
            delete string;
        }
        break;

        case eNotifyStatusEvent:
        {
            QString* string = (QString*)e->data();
            // Use status bar widget
            notifyStatusInternal( string->latin1() );
            delete string;
        }
        break;

        case eNotifyTimeEvent:
        {
            // Produce elapsed time notification
            rcTimestamp* time = (rcTimestamp*)e->data();
            emit elapsedTime( *time );
            delete time;
        }
        break;

        case eNotifyCursorEvent:
        {
            // Produce cursor movement notification
            rcTimestamp* time = (rcTimestamp*)e->data();
            emit cursorTime( *time );
            delete time;
        }
        break;

        case eNotifyTimelineRangeEvent:
        {
            // Notify timeline range change
            rcTimestampPair* timePair = (rcTimestampPair*)e->data();
            emit updateTimelineRange( timePair->first(), timePair->second() );
            delete timePair;
        }
        break;

        case eNotifyStateEvent:
        {

            rcExperimentState* state = (rcExperimentState*)e->data();
            // Notify the rest
            emit newState( *state );
            emit updateSettings();
            delete state;
        }
        break;

        case eNotifyBlitEvent:
        {
            rcWindow* image = (rcWindow*) e->data();
            emit updateDisplay( image );
            delete image;
        }
        break;

        case eNotifyBlitGraphicsEvent:
        {
            rcVisualGraphicsCollection* graphics = (rcVisualGraphicsCollection*) e->data();
            emit updateDisplay( graphics );
            delete graphics;
        }
        break;

        case eNotifyAnalysisRectEvent:
        {
            rcRect* rect = (rcRect*)e->data();
            emit updateAnalysisRect( *rect );
            delete rect;
        }
        break;

        case eNotifyAnalysisRectRotationEvent:
        {
	  rcAffineRectangle* affine = (rcAffineRectangle*)e->data();
	  emit updateAnalysisRectRotation( *affine );
	  delete affine;
        }
        break;

        case eNotifyMultiplierEvent:
        {
            double* multiplier = (double*)e->data();
            emit updateMultiplier( *multiplier );
            delete multiplier;
        }
        break;

        case eNotifyExperimentChange:
            emit updateSettings();
            break;

        case eNotifyVideoRectEvent:
        {
            rcRect* rect = (rcRect*)e->data();
            emit updateVideoRect( *rect );
            delete rect;
        }
        break;

    }

    e->setData( 0 );
    mEventQueueManager.processedEvent( e );
}

// private

// Display license error message
// Note: intentionally obfuscated function name
void rcModelDomain::rle( rcSecurityError status )
{
    // Get basic error message
    std::string errorString = rcLicenseManager::getErrorString( status );
    if ( status == eSecurityErrorInvalidHost ) {
        std::string host = rcLicenseManager::hexifyString( mLicenseManager.getHostId() );
        std::string licensedHost = rcLicenseManager::hexifyString( mLicenseManager.getLicenseHost() );
        // Display mismatching host ids
        errorString += "\nLicensed host is \"";
        errorString += licensedHost;
        errorString += "\", this host is \"";
        errorString += host;
        errorString += "\"";
    }
    // Concatenate license file name
    errorString += "\n\nLicense file: ";
    errorString += cLicenseFilePath;

    QMessageBox::critical( 0 , cAppName , errorString.c_str() , 1 , 0 );
}

// License validity check
// Note: intentionally obfuscated function name
bool rcModelDomain::vl()
{
    bool valid = true; // @Danger: takes this out

    // Check host validity
    rcSecurityError status = mLicenseManager.validHost();

    if ( status == eSecurityErrorOK ) {
        // Check expiration time
        status = mLicenseManager.validTime();

        if ( status == eSecurityErrorOK ) {
            // Lucky, everything is valid
            valid = true;
        } else {
            // Invalid time
            rle( status );
        }
    } else {
        // Invalid host
        rle( status );
    }

    return valid;
}

// Immediate status update
void rcModelDomain::notifyStatusInternal( const char* statusString )
{
    // Use status bar widget
    emit updateStatus( statusString );
}
