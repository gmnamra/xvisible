#include <QtGui>
#include <qapplication>

#include <rc_uitypes.h>
#include <strstream>
#include "rc_appconstants.h"
#include "rc_modeldomain.h"
#include "rc_main_window.h"
#include "rc_appconstants.h"
#include "rc_monitor.h"
#include "rc_menubar.h"
#include "rc_timeline.h"
#include "rc_settingpanel.h"
#include "rc_controlpanel.h"
#include "rc_trackpanel.h"
#include "rc_statusbar.h"
#include "rc_modeldomain.h"
#include "rc_csvexporter.h"

#define QStrFrcStr(foo) QString ((foo).c_str())

rcMainWindow::rcMainWindow( ) //QWidget* parent, const char* name, Qt::WFlags f )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    _lastState = eExperimentEmpty;
    _new_matrix = false;
    createActions ();
    createMenus ();
    createHelpBrowser ();
    createDockWindows();
    
    // Model connections
    connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
            this   , SLOT( updateState( rcExperimentState ) ) );
    connect( domain , SIGNAL( updateInputSource( int ) ) ,
            this   , SLOT( inputSource( int ) ) );
    connect( domain , SIGNAL( updateAnalysisRect( const rcRect& ) ) ,
            this   , SLOT( updateAnalysisRect( const rcRect& ) ) );
    
    connect( domain , SIGNAL( updateSettings() ) , this   , SLOT( settingChanged() ) );
    connect( domain , SIGNAL( newSmMatrix () ) , this   , SLOT( newSmMatrix () ) );    

    connect( domain , SIGNAL( requestPlot ( const CurveData *) ) ,
            this  , SLOT( reload_plotter ( const CurveData *  ) ) );   
    
    

    // Create a Monitor
	rcMonitor* monitor = new rcMonitor( this, "monitor" );
    setCentralWidget(monitor);
    
    // Create a status bar
    mStatusBar = new rcStatusBar( this , "status" );
    setStatusBar(mStatusBar);
    
    setUnifiedTitleAndToolBarOnMac (true);
    
    // Set application (generator) info
    rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
    // Concatenate application name, version and build date
    std::string comment = cAppName;
    comment += " ";
    comment += cVersionName;
    comment += " ";
    comment += cAppBuildDate;
    
    persistenceManager->setGeneratorComment( comment );
    

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




void rcMainWindow::createDockWindows()
{
 
    QDockWidget *dock = new QDockWidget(this);
    plotlist = new QListWidget(dock);
    dock->setWidget(plotlist);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    _viewMenu->addAction(dock->toggleViewAction());
    
    // Create a rcSettingsPanel
	rcSettingPanel* settingPanel = new rcSettingPanel( this , "settingsPanel" );
    dock = new QDockWidget(this);    
    dock->setAllowedAreas(Qt::RightDockWidgetArea);    
    dock->setWidget(settingPanel );
    addDockWidget(Qt::RightDockWidgetArea, dock);
    _viewMenu->addAction(dock->toggleViewAction());

    rcControlPanel* controlPanel = new rcControlPanel( this , "controlPanel" );
    dock = new QDockWidget(this);    
    dock->setAllowedAreas(Qt::RightDockWidgetArea);    
    dock->setWidget(controlPanel );
    addDockWidget(Qt::RightDockWidgetArea, dock);
    _viewMenu->addAction(dock->toggleViewAction());
    
    // Create a Timeline
	rcTimeline* timeline = new rcTimeline( this, "timeline" );
    dock = new QDockWidget(this);    
    dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    dock->setWidget( timeline );
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    _viewMenu->addAction(dock->toggleViewAction());
    
    // Create a Track Panel
	rcTrackPanel* trackPanel = new rcTrackPanel( this, "trackPanel" );
    dock = new QDockWidget(this);    
    dock->setAllowedAreas(Qt::LeftDockWidgetArea);
    dock->setWidget( trackPanel );
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    _viewMenu->addAction(dock->toggleViewAction());
    
    
    
    

}

void rcMainWindow::createHelpBrowser()
{
    _helpBrowser = new QTextBrowser( this );
    _helpBrowser->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken );
    _helpBrowser->resize (640, 400);
}


void rcMainWindow::createActions()
{
	rcModelDomain* domain = rcModelDomain::getModelDomain();
    
    NewId  = new QAction ( tr("&New Experiment"), this);
    NewId->setShortcuts (QKeySequence::New); NewId->setStatusTip (tr("Creates a new Experiment ") );
    connect (NewId, SIGNAL (triggered () ), domain, SLOT( requestNewApp()) );
    
    OpenId = new QAction ( tr("&Open Experiment"), this);
    OpenId->setShortcuts (QKeySequence::Open); NewId->setStatusTip (tr("Opens an Experiment ") );
    connect (OpenId, SIGNAL (triggered () ), domain, SLOT( doOpen()) );

    SaveId = new QAction ( tr("&Save Experiment"), this);
    OpenId->setShortcuts (QKeySequence::SaveAs); NewId->setStatusTip (tr("Saves current Experiment ") );
    connect (SaveId, SIGNAL (triggered () ), domain, SLOT( doSave()) );

    CloseId = new QAction ( tr("&Close Experiment"), this);
    OpenId->setShortcuts (QKeySequence::Close); NewId->setStatusTip (tr("Closes current Experiment ") );
    connect (CloseId, SIGNAL (triggered () ), domain, SLOT( requestClose()) );
    
    ExportId = new QAction ( tr("&Export as CSV "), this);
    OpenId->setShortcut (tr("CTRL+Key_E" )); NewId->setStatusTip (tr("Exports current Experiment in Comma Separated Format ") );
    connect (ExportId, SIGNAL (triggered () ), domain, SLOT( doExport()) );
    
    MovieExportId = new QAction ( tr("&Export movie as Quicktime "), this);
    OpenId->setShortcut (tr("CTRL+SHIFT+Key_E" )); NewId->setStatusTip (tr("Exports current image data in Quicktime  ") );
    connect (MovieExportId, SIGNAL (triggered () ), domain, SLOT( doExportMovie()) );
    
    MovieNativeExportId = new QAction ( tr("&Export movie as rfymov "), this);
    OpenId->setShortcut (tr("CTRL+SHIFT+Key_S" )); NewId->setStatusTip (tr("Exports current image data in rfymov ") );
    connect (MovieNativeExportId, SIGNAL (triggered () ), domain, SLOT( doExportMovie()) );
    
    MatrixExportId = new QAction ( tr("&Export Similarity Matrix as csv  "), this);
    OpenId->setShortcut (tr("CTRLKey_M" )); NewId->setStatusTip (tr("Exports current similarity matrix in csv ") );
    connect (MatrixExportId, SIGNAL (triggered () ), domain, SLOT( doExportSmMatrix()) );
    
    ImportMovieId = new QAction ( tr("&Import movie import  "), this);
    OpenId->setShortcut (tr("CTRLKey_I" )); NewId->setStatusTip (tr("Imports a movie for analysis ") );
    connect (ImportMovieId, SIGNAL (triggered () ), domain, SLOT( requestMovieImport ()) );
    
    
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                domain, SLOT(importRecentMovie()));
    }


    QuitId = new QAction(tr("E&xit"), this);
    QuitId->setShortcuts(QKeySequence::Quit);
    QuitId->setStatusTip(tr("Exit the application"));
    connect(QuitId, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    
    HelpId = new QAction(tr("H&elp"), this);
    HelpId->setShortcuts(QKeySequence::HelpContents);
    HelpId->setStatusTip(tr("Help"));
    connect(HelpId, SIGNAL(triggered()), qApp, SLOT(help()));    

    AboutId = new QAction(tr("H&elp"), this);
    AboutId->setShortcut (tr("(CTRL+Key_H" ));
    AboutId->setStatusTip(tr("About"));
    connect(AboutId, SIGNAL(triggered()), qApp, SLOT(About()));   
    
    // create and populate the analysis menu
    AnalysisAnalyzeId  = new QAction(tr("R&un"), this);
    AnalysisAnalyzeId->setShortcut (tr("(CTRL+Key_R" ));
    AboutId->setStatusTip(tr("Run Experiment"));
    connect(AboutId, SIGNAL(triggered()), qApp, SLOT(requestProcess()));   

   

}

void rcMainWindow::createMenus ()
{
    _fileMenu = menuBar()->addMenu (tr("&File"));
    _helpMenu = menuBar()->addMenu (tr("&Help"));
    _viewMenu = menuBar()->addMenu (tr("&View"));
    
    _fileMenu->addAction(NewId);
    _fileMenu->addAction(OpenId);    
    _fileMenu->addAction(SaveId);  
    _fileMenu->addAction(CloseId);      
    
    _fileMenu->addAction(ExportId);    
    _fileMenu->addAction(MovieExportId);  
    _fileMenu->addAction(MovieNativeExportId);      
    _fileMenu->addAction(MatrixExportId);          
    
    _fileMenu->addAction(ImportMovieId);

    separatorAct = _fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        _fileMenu->addAction(recentFileActs[i]);

    updateRecentFileActions();
    
    _fileMenu->addSeparator();
    _fileMenu->addAction(QuitId);
    
    _helpMenu->addAction(AboutId);
    _helpMenu->addAction(HelpId);
   
    
}


// slots

void rcMainWindow::updateState( rcExperimentState state )
{
    switch( state ) {
    case eExperimentEmpty:	        // experiment was created but not run
        _fileMenu->setEnabled( true );
        NewId->setVisible ( true );
        OpenId->setVisible (true );
        OpenSettingsId->setVisible (true );
        CloseId->setVisible ( true );
        SaveId->setVisible ( false );
        ExportId->setVisible (false );
       MatrixExportId->setVisible  ( false );            
       MovieExportId->setVisible   (false );
       MovieNativeExportId->setVisible (false );
        QuitId->setVisible ( true );
        ImportMovieId->setVisible (true );


        break;

    case eExperimentRunning:		// experiment is running
    case eExperimentPlayback:		// experiment is playing back
        NewId->setVisible ( false );
        OpenId->setVisible ( false );
        CloseId->setVisible ( false );
        SaveId->setVisible (  false );
        ExportId->setVisible ( false );
        MatrixExportId ->setVisible( false );                        
        MovieExportId ->setVisible( false );
        MovieNativeExportId ->setVisible( false );
        ImportMovieId ->setVisible ( false );
        QuitId->setVisible ( true );

        AnalysisAnalyzeId->setVisible(false );

        break;

    case eExperimentEnded:		    // experiment has ended
            
            NewId->setVisible ( true );
            OpenId->setVisible ( true );
            CloseId->setVisible ( true );
            SaveId->setVisible (  true);
            ExportId->setVisible ( true);
            MatrixExportId ->setVisible( _new_matrix );                              
            MovieExportId ->setVisible( true );
            MovieNativeExportId ->setVisible( true );
            ImportMovieId ->setVisible ( true );
            QuitId->setVisible ( true );
            
            AnalysisAnalyzeId->setVisible(false );
      
        break;

    case eExperimentLocked:		    // experiment has ended and been saved
            
            NewId->setVisible ( true );
            OpenId->setVisible ( true );
            CloseId->setVisible ( true );
            SaveId->setVisible (  true);
            ExportId->setVisible ( true);
            MovieExportId ->setVisible( true );
            MovieNativeExportId ->setVisible( true );
            ImportMovieId ->setVisible ( true );
            QuitId->setVisible ( true );
            
            AnalysisAnalyzeId->setVisible(false );

    
        break;

    default:
        break;
    }

    _lastState = state;
}

void rcMainWindow::importRecentMovie()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    QAction *action = qobject_cast<QAction *>(sender () );
    if (action)
        domain->useRecentMovieFile(action->data().toString());
    
}

// Display about box
void rcMainWindow::about()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    std::string tmp = domain->getLicenseExpirationTime();
    QString expirationDate (tmp.c_str());
    QString svnversion ("Beta Release ");
    QString title( cAppName );
    QString version( cVersionName );
    QString richTitle =  "<h2>" + title + " " + version + "</h2>";
    QString buildDate = "<p>Build: ";
    buildDate += svnversion;
    buildDate += cAppBuildDate;

    QString expDate =   "<p>License expiration date: " + expirationDate;
    QString copyRight = "<p>Copyright (c) 2002-2009 Reify Corp. All rights reserved.";
    QString otherRight = "<p>Copyright (c) 1987-1992 Numerical Recipes Software";

    QString text = "<qt><center>" + richTitle + "</center>" + buildDate + expDate + copyRight + otherRight  + "</qt>";
    QMessageBox::about( NULL, title, text );
}



// Input source has been changed
void rcMainWindow::inputSource( int i )
{
//    // Hack alert: value 0 is file, greater than 0 camera
//    if ( i > 0 ) {
//        // Camera input
//        _analysisMenu->setItemEnabled( _analysisAnalyzeId  , false );
//	if (_lastState == eExperimentEmpty)
//	  _captureMenu->setItemEnabled( _fileSaveMovieId , true );
//    } else {
//        // File input
//        _captureMenu->setItemEnabled( _fileSaveMovieId , false );
//    }
}

void rcMainWindow::updateAnalysisRect( const rcRect& rect )
{
    // Enable/disable analyze command based on focus area size    
     AnalysisAnalyzeId->setVisible(false );    

}

// Display help box
void rcMainWindow::help()
{
  QString title = cAppName;

  QString cmd;
  string docindexhtml (qApp->argv()[0]);
  string slash( "/" );
  uint32 s = docindexhtml.find_last_of( slash ); // |Visible
  if ( s != std::string::npos )
    {
      uint32 len = s + 1;
      if ( s > 0 )
	docindexhtml = docindexhtml.substr(0, len );
    }

  docindexhtml += "../Documentation/index.html";

  // Background task
  cmd.sprintf("open %s", docindexhtml.c_str());

  // Launch a new application instance
  system( cmd.latin1() );

}

// Request native format save
void rcMainWindow::doSave()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSave( eExperimentNativeFormat );
}

// Request CSV export
void rcMainWindow::doExport()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSave( eExperimentCSVFormat );
}



// Request movie export to QT
void rcMainWindow::doExportMovie()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSave( eExperimentQuickTimeMovieFormat );
}



// Request movie export to QT
void rcMainWindow::doExportSmMatrix()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSmMatrixSave();
}


// Request movie export to .rfymov
void rcMainWindow::doExportNativeMovie()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSave( eExperimentNativeMovieFormat );
}

// Request full experiment open
void rcMainWindow::doOpen()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestOpen( eExperimentImportAll );
}

// Request only experiment settings open
void rcMainWindow::doOpenSettings()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestOpen( eExperimentImportExperimentSettings );
}



void rcMainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
    setWindowFilePath(curFile);
    
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();
    
    settings.setValue("recentFileList", files);
    
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        rcMainWindow *mainWin = qobject_cast<rcMainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void rcMainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    
    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);
    
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(this->strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
    
    separatorAct->setVisible(numRecentFiles > 0);
}

QString rcMainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}




// public slots

void rcMainWindow::newSmMatrix()
{
    _new_matrix     = true;
}

void rcMainWindow::settingChanged()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    // Get current experiment attributes
    rcExperimentAttributes attr = domain->getExperimentAttributes();
    QString curf; curf.fromStdString ( domain->getExperimentAttributes().inputName );
    setCurrentFile(curf);
    
    
    uint32 maxX = attr.frameWidth;
    uint32 maxY = attr.frameHeight;
    int depth = attr.frameDepth * 8; // Bits per pixel
    std::string title = attr.title;
    if (title == std::string ("Untitled"))
      title = attr.inputName;

    string::size_type pos = title.find (';', 0);
    if ( pos != string::npos)
      {
	//make a string up to the ;
	title = std::string (title, 0, pos);
	pos = title.find_last_of ('/', string::npos);
	if ( pos != string::npos) 
	  {
	    title = std::string (title, 0, pos);
	    pos = title.find ('/', 0);
	    if ( pos != string::npos) 
	      {
		title = std::string (title, pos, title.size());
	      }
	  }
      }

    strstream caption;
    // Concatenate experiment title, video frame size and depth
    caption << title;
    if ( depth > 0 )
        caption << " (" << maxX << "x" << maxY << "x" << depth << ")";
    caption << ends;
    // Set window bar caption
    setCaption( caption.str() );
    caption.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns

    // Disable resizing for camera
    if ( attr.liveInput ) {
        mStatusBar->setSizeGripEnabled( FALSE );
    } else {
        mStatusBar->setSizeGripEnabled( TRUE );
    }
}

// protected

void rcMainWindow::closeEvent( QCloseEvent* event )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperimentState state = domain->getExperimentState();

    if ( state != eExperimentEmpty && state != eExperimentLocked ) {
        // No data has been saved, prompt user
        switch( QMessageBox::warning( 0, cAppName,
                                      "Do you want to save experiment data before quitting?\n\n"
                                      "If you don't save, your data will be lost.",
                                      "&Don't Save", "Cancel", "&Save",
                                      2, 1 ) ) { 
            case 0: // Don't Save clicked or Alt+D pressed
                event->accept();
                break;
            case 1: // Cancel clicked or Alt+C pressed or Escape pressed
                    // Do nothing
                break;
            case 2: // Save clicked or Alt+S pressed
                domain->requestSave( eExperimentNativeFormat );
                if ( domain->getExperimentState() == eExperimentLocked )
                    event->accept();
                break;
        }

    } else
        event->accept();
}

void rcMainWindow::reload_plotter (const CurveData * cv)
{
    LPWidget* plotter = new LPWidget (this, cv);
    
    QDockWidget *dock = new QDockWidget(this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(plotter);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    _viewMenu->addAction(dock->toggleViewAction());
    update ();
    
}


