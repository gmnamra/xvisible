/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_mainwindow.cpp 7191 2011-02-07 19:38:55Z arman $
 *
 *	This file contains main window ui implementation.
 *
 ******************************************************************************/

#include <strstream>

#include <qapplication.h>
#include <qlayout.h> 
#include <qsplitter.h> 
#include <qmessagebox.h>
//Added by qt3to4:
#include <Q3BoxLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QCloseEvent>
#include <QScrollArea>
#include <QDockWidget>
#include <QListWidget>

#include "rc_appconstants.h"
#include "rc_monitor.h"
#include "rc_menubar.h"
#include "rc_timeline.h"
#include "rc_settingpanel.h"
#include "rc_controlpanel.h"
#include "rc_trackpanel.h"
#include "rc_statusbar.h"
#include "rc_modeldomain.h"
#include "rc_mainwindow.h"


// Spacing between layout groups
static const int cLayoutSpacing = 3;

// public

/* 
 *	Constructs a rcMainWindow which is a child of 'parent', with the 
 *	name 'name' and widget flags set to 'f'.
 *
 */
rcMainWindow::rcMainWindow(QWidget* parent)
{
    rcUNUSED(parent);
    // Make the top-level layout; a vertical box to contain all widgets
    // and sub-layouts.
    QWidget* dummy = new QWidget (this);
    setCentralWidget (dummy);
    Q3BoxLayout *topLayout = new Q3VBoxLayout( dummy , cLayoutSpacing );
      
    viewMenu = menuBar()->addMenu(tr("&View"));    
 
    // Create a menubar and tell the layout about it.
	rcMenuBar* menuBar = new rcMenuBar( this , "menubar" );
    topLayout->setMenuBar( menuBar );

	// Create a horizontal layout to put the Monitor in.
	Q3BoxLayout *setMonLayout = new Q3HBoxLayout( topLayout );

	// Create a Monitor
	rcMonitor* monitor = new rcMonitor( this, "monitor" );
	setMonLayout->addWidget( monitor );

	// Create a vertical layout to put the SettingsPanel and ControlPanel in.
	Q3BoxLayout *tlCpLayout = new Q3VBoxLayout( setMonLayout );

	// Create a rcSettingsPanel
	rcSettingPanel* settingPanel = new rcSettingPanel( this , "settingsPanel" );
	tlCpLayout->addWidget( settingPanel, Qt::AlignTop );

	// Create a rcControlPanel
	rcControlPanel* controlPanel = new rcControlPanel( this , "controlPanel" );
	tlCpLayout->addWidget( controlPanel, Qt::AlignTop );

	// Create a horizontal layout to put the trackpanel and timeline in.
	Q3BoxLayout *tpTlLayout = new Q3HBoxLayout( topLayout );

	// Create a Track Panel
	rcTrackPanel* trackPanel = new rcTrackPanel( this, "trackPanel" );
	tpTlLayout->addWidget( trackPanel );
   
	// Create a Timeline
	rcTimeline* timeline = new rcTimeline( this, "timeline" );
	tpTlLayout->addWidget( timeline , 2 );

    // Create a status bar
    mStatusBar = new rcStatusBar( this , "status" );
    topLayout->addWidget( mStatusBar );
    
    // Stretch factors for resizing
    topLayout->setStretchFactor( setMonLayout , 10 );
    topLayout->setStretchFactor( tlCpLayout , 1 );
    topLayout->setStretchFactor( tpTlLayout , 2 );
    topLayout->setStretchFactor( mStatusBar , 0 );
    
    setMonLayout->setStretchFactor( monitor , 10 ); 
    tpTlLayout->setStretchFactor( trackPanel, 2 );
    tpTlLayout->setStretchFactor( timeline, 2 );
    tlCpLayout->setStretchFactor( settingPanel, 1 );
    tlCpLayout->setStretchFactor( controlPanel, 0 );
	topLayout->activate();

    rcModelDomain* domain = rcModelDomain::getModelDomain();
    // connect this widget to be notified by the updateSettings()
	//	signal from the model domain
	connect( domain , SIGNAL( updateSettings() ) , this, SLOT( settingChanged() ) );

    connect( domain , SIGNAL( requestPlot ( const CurveData *) ) ,
            this  , SLOT( reload_plotter ( const CurveData *  ) ) );    
    
    
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
    
    createDockWindows();
    
}

/*	
 *	Destroys the object and frees any allocated resources
 */
rcMainWindow::~rcMainWindow()
{
	// no need to delete child widgets, Qt does it all for us
}

// public slots

void rcMainWindow::settingChanged()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    // Get current experiment attributes
    rcExperimentAttributes attr = domain->getExperimentAttributes();
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
    LPWidget* plotter = new LPWidget (plotlist, cv);
    plotlist->insertItem (plotlist->count()+1, (QListWidgetItem*) plotter);
    plotlist->repaint();
    
}


void rcMainWindow::createDockWindows()
{
    QDockWidget *dock = new QDockWidget(tr("Plots"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    plotlist = new QListWidget(dock);
    dock->setWidget(plotlist);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
  
}
