/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_menubar.cpp 7191 2011-02-07 19:38:55Z arman $
 *
 *	This file contains application menu bar implementation.
 *
 ******************************************************************************/

#include <qapplication.h>
#include <qmessagebox.h>
#include <q3textbrowser.h>
#include <q3filedialog.h>
//Added by qt3to4:
#include <Q3Frame>
#include <Q3PopupMenu>
#include <CoreFoundation/CFPreferences.h>
#include <rc_uitypes.h>

#include "rc_appconstants.h"
#include "rc_modeldomain.h"
#include "rc_menubar.h"



//static void Dump_HighScores(void);

rcMenuBar::rcMenuBar( QWidget* parent, const char* name, Qt::WFlags f )
	: QMenuBar( parent, name )
{
    rcUNUSED( f );

    _lastState = eExperimentEmpty;

    setSeparator( QMenuBar::InWindowsStyle );
	rcModelDomain* domain = rcModelDomain::getModelDomain();

    // create and populate the file menu
    _fileMenu = new Q3PopupMenu( this );
    _fileNewId  = _fileMenu->insertItem( "&New Experiment" , domain , SLOT( requestNewApp() ) , tr("CTRL+Key_N") );
    _fileOpenId = _fileMenu->insertItem( "&Open Experiment...", this , SLOT( doOpen() ) , tr("CTRL+Key_O "));
    _fileOpenSettingsId = _fileMenu->insertItem( "&Open Experiment Template...", this , SLOT( doOpenSettings() ) , tr("CTRL+SHIFT+Key_O"));

    _fileMenu->insertSeparator();
    _fileCloseId = _fileMenu->insertItem( "&Close", domain , SLOT( requestClose() ) , tr("CTRL+Key_W" ));
    _fileSaveId = _fileMenu->insertItem( "&Save As...", this , SLOT( doSave() ) , tr("(CTRL+Key_S" ));
    _movieNativeExportId = _fileMenu->insertItem( "&Save Movie As...", this , SLOT( doExportNativeMovie() ), tr("(CTRL+SHIFT+Key_S" ));

    _fileMenu->insertSeparator();
    _fileExportId = _fileMenu->insertItem( "&Export As CSV...", this , SLOT( doExport() ) ,  tr("CTRL+Key_E" ));
    _movieExportId = _fileMenu->insertItem( "&Export Movie As QuickTime...", this , SLOT( doExportMovie() ) , tr("(CTRL+SHIFT+Key_E" ));

    _fileMenu->insertSeparator();
    //    _fileImportLastMovieId = _fileMenu->insertItem( "&Import Last Movie...", domain , SLOT( requestLastMovieImport() ) , CTRL+Key_L );
    _fileImportMovieId = _fileMenu->insertItem( "&Import Movie...", domain , SLOT( requestMovieImport() ) ,  tr("CTRL+Key_I" ));
    _fileImportMovieId = _fileMenu->insertItem( "&Import STK...", domain , SLOT( requestSTKImport() ) ,  tr("CTRL+Key_U" ));
    _fileImportImagesId = _fileMenu->insertItem( "&Import Image Sequence...", domain , SLOT( requestImageImport() ) ,  tr("(CTRL+SHIFT+Key_I" ));
    _dirImportImagesId = _fileMenu->insertItem( "&Import Tiff Directory...", domain , SLOT( requestTifDirImport() ) ,  tr("(CTRL+SHIFT+Key_T" ));
    _fileQuitId = _fileMenu->insertItem( "&Quit", qApp   , SLOT( closeAllWindows() ) ,  tr("(CTRL+Key_Q" ));

    // create and populate the help menu
    Q3PopupMenu *help = new Q3PopupMenu( this );
    Q_CHECK_PTR( help );
    help->insertItem( "&About", this, SLOT( about() ),  tr("(CTRL+Key_H" ));
    help->insertItem( "&Help" , this, SLOT( help() ) ,  tr("(CTRL+Key_Question" ));

    // create and populate the analysis menu
    _analysisMenu = new Q3PopupMenu( this );
    _analysisAnalyzeId  = _analysisMenu->insertItem( "&Analyze" , domain , SLOT( requestProcess() ) ,  tr("(CTRL+Key_Z" ));
    _analysisMenu->setItemEnabled( _analysisAnalyzeId  , false );

    // create and populate the view menu
    _viewMenu = new Q3PopupMenu( this );

    // create and populate the capture menu
    _captureMenu = new Q3PopupMenu( this );
    _fileSaveMovieId = _captureMenu->insertItem( "&Capture Movie...", domain , SLOT( requestMovieSave() ) ,  tr("(CTRL+Key_M" ));
    _captureMenu->setItemEnabled( _fileSaveMovieId  , false );

    // Populate the menu bar
    insertItem( "&File", _fileMenu );
    insertItem( "&Analysis", _analysisMenu );
   // insertItem( "&Capture", _captureMenu );
//    insertItem( "&View", _viewMenu );
    insertItem( "&Help", help );

    // create help browser
    _helpBrowser = new Q3TextBrowser( this );
    _helpBrowser->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken );
    _helpBrowser->resize (640, 400);

    // Model connections
    connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
             this   , SLOT( updateState( rcExperimentState ) ) );
    connect( domain , SIGNAL( updateInputSource( int ) ) ,
             this   , SLOT( inputSource( int ) ) );
    connect( domain , SIGNAL( updateAnalysisRect( const rcRect& ) ) ,
             this   , SLOT( updateAnalysisRect( const rcRect& ) ) );

    // in case we don't get an initial signal
    updateState( eExperimentEmpty );
    //    requestTrackingDisplayGL();

}

rcMenuBar::~rcMenuBar()
{
}

// slots

void rcMenuBar::updateState( rcExperimentState state )
{
    switch( state ) {
    case eExperimentEmpty:	        // experiment was created but not run
        _fileMenu->setEnabled( true );
        _fileMenu->setItemEnabled( _fileNewId  , true );
        _fileMenu->setItemEnabled( _fileOpenId , true );
        _fileMenu->setItemEnabled( _fileOpenSettingsId , true );
        _fileMenu->setItemEnabled( _fileCloseId, true );
        _fileMenu->setItemEnabled( _fileSaveId , false );
        _fileMenu->setItemEnabled( _fileExportId , false );
        _fileMenu->setItemEnabled( _matrixExportId , false );            
        _fileMenu->setItemEnabled( _movieExportId , false );
        _fileMenu->setItemEnabled( _movieNativeExportId , false );
        _fileMenu->setItemEnabled( _fileQuitId , true );
        _fileMenu->setItemEnabled( _fileImportImagesId , true );
        _fileMenu->setItemEnabled( _fileImportMovieId , true );
        _fileMenu->setItemEnabled( _fileImportSTKId , true );
        _fileMenu->setItemEnabled( _dirImportImagesId , true);

        _analysisMenu->setEnabled( true );
        _analysisMenu->setItemEnabled( _analysisAnalyzeId  , false );

        _captureMenu->setEnabled( true );
        _captureMenu->setItemEnabled( _fileSaveMovieId , false );

        break;

    case eExperimentRunning:		// experiment is running
    case eExperimentPlayback:		// experiment is playing back
        _fileMenu->setItemEnabled( _fileNewId  , false );
        _fileMenu->setItemEnabled( _fileOpenId , false );
        _fileMenu->setItemEnabled( _fileOpenSettingsId , false );
        _fileMenu->setItemEnabled( _fileCloseId, false );
        _fileMenu->setItemEnabled( _fileSaveId , false );
        _fileMenu->setItemEnabled( _fileExportId , false );
        _fileMenu->setItemEnabled( _matrixExportId , false );                        
        _fileMenu->setItemEnabled( _movieExportId , false );
        _fileMenu->setItemEnabled( _movieNativeExportId , false );
        _fileMenu->setItemEnabled( _fileImportImagesId , false );
        _fileMenu->setItemEnabled( _dirImportImagesId , false);
        _fileMenu->setItemEnabled( _fileImportMovieId , false );
        _fileMenu->setItemEnabled( _fileImportSTKId , false );
        _fileMenu->setItemEnabled( _fileQuitId , true );
	//        _fileMenu->setItemEnabled( _fileImportLastMovieId , false );

        _analysisMenu->setItemEnabled( _analysisAnalyzeId  , false );

        _captureMenu->setItemEnabled( _fileSaveMovieId , false );

//        _viewMenu->setItemEnabled( _viewTrackingGLId, false );
//        _viewMenu->setItemEnabled( _viewCellInfoId, false );
        break;

    case eExperimentEnded:		    // experiment has ended
        _fileMenu->setItemEnabled( _fileNewId  , true );
        _fileMenu->setItemEnabled( _fileOpenId , true );
        _fileMenu->setItemEnabled( _fileOpenSettingsId , true );
        _fileMenu->setItemEnabled( _fileCloseId, true );
        _fileMenu->setItemEnabled( _fileSaveId , true );
        _fileMenu->setItemEnabled( _fileExportId , true );
        _fileMenu->setItemEnabled( _matrixExportId , true );   
        _fileMenu->setItemEnabled( _movieExportId , true );
        _fileMenu->setItemEnabled( _movieNativeExportId , true );
        _fileMenu->setItemEnabled( _fileQuitId , true );
        _fileMenu->setItemEnabled( _fileImportImagesId , false );
        _fileMenu->setItemEnabled( _dirImportImagesId , false);
        _fileMenu->setItemEnabled( _fileImportMovieId , false );
        _fileMenu->setItemEnabled( _fileImportSTKId , false );
	//        _fileMenu->setItemEnabled( _fileImportLastMovieId , true );
        _captureMenu->setItemEnabled( _fileSaveMovieId , false);

        break;

    case eExperimentLocked:		    // experiment has ended and been saved
        _fileMenu->setItemEnabled( _fileNewId  , true );
        _fileMenu->setItemEnabled( _fileOpenId , true );
        _fileMenu->setItemEnabled( _fileOpenSettingsId , true );
        _fileMenu->setItemEnabled( _fileCloseId, true );
        _fileMenu->setItemEnabled( _fileSaveId , true );
        _fileMenu->setItemEnabled( _fileExportId , true );
        _fileMenu->setItemEnabled( _movieExportId , true );
        _fileMenu->setItemEnabled( _movieNativeExportId , true );
        _fileMenu->setItemEnabled( _fileQuitId , true );
        _fileMenu->setItemEnabled( _fileImportImagesId , false );
        _fileMenu->setItemEnabled( _dirImportImagesId , false);
        _fileMenu->setItemEnabled( _fileImportMovieId , false );
        _fileMenu->setItemEnabled( _fileImportSTKId , false );
	//        _fileMenu->setItemEnabled( _fileImportLastMovieId , true );


        break;

    default:
        break;
    }

    _lastState = state;
}

// Display about box
void rcMenuBar::about()
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
void rcMenuBar::inputSource( int i )
{
    // Hack alert: value 0 is file, greater than 0 camera
    if ( i > 0 ) {
        // Camera input
        _analysisMenu->setItemEnabled( _analysisAnalyzeId  , false );
	if (_lastState == eExperimentEmpty)
	  _captureMenu->setItemEnabled( _fileSaveMovieId , true );
    } else {
        // File input
        _captureMenu->setItemEnabled( _fileSaveMovieId , false );
    }
}

void rcMenuBar::updateAnalysisRect( const rcRect& rect )
{
    // Enable/disable analyze command based on focus area size
    _analysisMenu->setItemEnabled( _analysisAnalyzeId,  rect.width() > 0 && rect.height() > 0 && _lastState != eExperimentRunning );
}

// Display help box
void rcMenuBar::help()
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
void rcMenuBar::doSave()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSave( eExperimentNativeFormat );
}

// Request CSV export
void rcMenuBar::doExport()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSave( eExperimentCSVFormat );
}


void rcMenuBar::reload_plotter2d (const CurveData2d * cv)
{
    if (cv == 0) return;

    try
    {
        rcModelDomain* domain = rcModelDomain::getModelDomain();
        SharedCurveData2dRef wrapit (new CurveData2d (*cv) );
        domain->getLast2dData (wrapit);

    } 
    catch (general_exception& gx)
    {
        std::cerr << " Error saving 2d Matrix " << gx.GetDescription() << std::endl;
    }
    
}

// @Note: Not following export design for SM Matrix export 
void rcMenuBar::doExportMatrix()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSaveSmMatrix();
}

// Request movie export to QT
void rcMenuBar::doExportMovie()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSave( eExperimentQuickTimeMovieFormat );
}


// Request movie export to .rfymov
void rcMenuBar::doExportNativeMovie()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestSave( eExperimentNativeMovieFormat );
}

// Request full experiment open
void rcMenuBar::doOpen()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestOpen( eExperimentImportAll );
}

// Request only experiment settings open
void rcMenuBar::doOpenSettings()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->requestOpen( eExperimentImportExperimentSettings );
}

// Request cell info window display
void rcMenuBar::requestCellInfoDisplay()
{
//    rcCellInfoWidget* w = new rcCellInfoWidget( 0 );
  //  w->show();
}

