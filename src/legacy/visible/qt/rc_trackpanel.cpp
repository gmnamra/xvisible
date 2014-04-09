// rc_trackpanel.cpp


#include <QtGui/QtGui>
#include <QtCore/QtCore>
#include <Qt3Support/q3scrollview.h>
#include <Qt3Support/Q3VBox>
#include <rc_model.h>

#include "rc_modeldomain.h"
#include "rc_trackgroupwidget.h"
#include "rc_trackpanel.h"
#include "rc_trackmanager.h"

rcTrackPanel::rcTrackPanel( QWidget* parent, const char* name)
: QWidget( parent, name, 0 ), mContents( 0 ), mOldState(eExperimentEmpty),
mManagedGroups( 0 ), mLiveInput( false ), mLiveStorage( false )
{

	setMinimumWidth( 250 );
	setMaximumWidth( 300 );
    m_scrollarea = new QScrollArea;
    m_scrollarea->setContentsMargins( 0, 0, 0, 0 );
    m_scrollarea->setWidget( this );
    m_scrollarea->setWidgetResizable( true );
    m_scrollarea->setStyleSheet( "border: 1px solid blue" );
    m_scrollarea->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    


    
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
            this   , SLOT( updateState( rcExperimentState ) ) );
    
    connect( domain , SIGNAL( updateInputSource( int ) ) ,
            this   , SLOT( updateSource( int ) ) );
    
    connect( domain , SIGNAL( updateCameraState( bool, bool ) ) ,
            this   , SLOT( updateCamera( bool, bool ) ) );
    
    connect( domain , SIGNAL( updateDebugging() ) ,
            this   , SLOT( updateTrackGroups() ) );
    
    updateState( eExperimentEmpty );
}

rcTrackPanel::~rcTrackPanel()
{
    // Qt will delete children widgets automatically
}

void rcTrackPanel::updateCamera( bool live, bool storage )
{
#ifdef DEBUG_LOG   
    cerr << "rcTrackPanel::updateCamera " << live << " " << storage << endl;
#endif    
    // Camera state has changed, preview vs. storage
    rmUnused( storage );
    updateTrackGroups( live, storage );
}

//public slots:
void rcTrackPanel::updateState( rcExperimentState state )
{
#ifdef DEBUG_LOG  
    cerr << "rcTrackPanel::updateState " << state << endl;
#endif
    // Ignore video playback states
    if ( state == eExperimentPlayback ||
        (mOldState == eExperimentPlayback && state == eExperimentEnded)) {
        mOldState = state;
        return;
    }
    
    mOldState = state;
    // when the state of the experiment changes, the
    // track group list needs to be recomputed.
    
    if ( mLiveStorage && state == eExperimentEnded )
    mLiveStorage = false;
    
    updateTrackGroups( mLiveInput, mLiveStorage );
}

void rcTrackPanel::updateSource( int source )
{
#ifdef DEBUG_LOG   
    cerr << "rcTrackPanel::updateSource " << source << " " << mManagedGroups << endl;
#endif    
    // when the state of the experiment changes, the
    // track group list needs to be recomputed.
    
    if ( source > 0 ) {
        // Ignore redundant calls if we have a track list already
        if ( mLiveInput && mContents && mManagedGroups )
        return;
        mLiveInput = true;
        mLiveStorage = false;
    } else {
        if ( !mLiveInput )
        return;
        mLiveInput = false;
        mLiveStorage = false;
    }
    updateTrackGroups( mLiveInput, mLiveStorage );
}

// repopulate the panel with track group widgets
void rcTrackPanel::updateTrackGroups( )
{
    updateTrackGroups( mLiveInput, mLiveStorage );
}



// repopulate the panel with track group widgets
void rcTrackPanel::updateTrackGroups( bool cameraInput, bool cameraStorage )
{
#if 0
    mLiveInput = cameraInput;
    mLiveStorage = cameraStorage;
    
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	rcExperiment* experiment = domain->getExperiment();
	int nTrackGroups = experiment->getNTrackGroups();
    
    // TODO: implement a redundancy check, sometimes all the new widgets will be
    // identical to the old widgets so there is no need to recreate them.
    
    // remove any existing track widgets.
    if ( mContents != 0 )
    {
        // this will automatically remove the contents from the scroll
        //  view - thanks Qt!
        removeChild( mContents );
        delete mContents;
        mContents = 0;
    }
    
    if ( nTrackGroups == 0 )
    {
        hide();
        return;
    }
    
    // Add new groups
    Q3VBox* vBox = new Q3VBox( viewport() );
    addChild( vBox );
    mContents = vBox;
    
    mManagedGroups = 0;
	for (int i = 0; i < nTrackGroups; i++)
	{
        if ( rcTrackManager::hasManageableTracks( i, cameraInput, cameraStorage ) ) {
            // Create a widget only for groups that have manageable tracks
            new rcTrackGroupWidget( vBox , i );
            ++mManagedGroups;
        }
	}
    
    if ( mManagedGroups ) {
        show();
        vBox->show();
    } else {
        hide();
#ifdef DEBUG_LOG        
        if ( cameraInput )
            if ( cameraStorage )
                cerr << "rcTrackPanel::updateTrackGroups: camera enabled but no storage track(s) found" << endl;
            else
                cerr << "rcTrackPanel::updateTrackGroups: camera enabled but no preview track(s) found" << endl;
#endif        
        
    }
#endif    
}
    
   