// rc_trackpanel.cpp

#include <qlayout.h> 
#include <qpushbutton.h>
#include <qlabel.h>
#include <q3vbox.h>
#include <QKeyEvent>
#include <rc_model.h>

#include "rc_modeldomain.h"
#include "rc_trackgroupwidget.h"
#include "rc_trackpanel.h"
#include "rc_trackmanager.h"

rcTrackPanel::rcTrackPanel( QWidget* parent, const char* name, Qt::WFlags f )
: Q3ScrollView( parent, name ), mContents( 0 ), mOldState(eExperimentEmpty),
mManagedGroups( 0 ), mLiveInput( false ), mLiveStorage( false )
{
    rcUNUSED( f );
    if(viewport())
      {
         viewport()->setMouseTracking(true);
      }
    setMouseTracking(true);

    viewport()->setEraseColor(QColor(0,0,0));
    setVScrollBarMode(Q3ScrollView::AlwaysOn);
    setHScrollBarMode(Q3ScrollView::AlwaysOff);
    setMinimumWidth( 250 );
    setMaximumWidth( 300 );
     setFocusPolicy(Qt::WheelFocus);
    viewport()->setBackgroundColor(backgroundColor());
    setMargin(0);

       //    setResizePolicy( Q3ScrollView::AutoOneFit );
    
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
void rcTrackPanel::updateTrackGroups( bool cameraInput, bool cameraStorage )
{
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
        if ( rcTrackManager::hasManageableTracks( i, cameraInput, cameraStorage ) )
          {
            // Create a widget only for groups that have manageable tracks
            new rcTrackGroupWidget( vBox , i );
            ++mManagedGroups;
        }
	}
    
    if ( mManagedGroups ) {
        show();
        vBox->show();
    } else
        hide();

    updateContents();
    updateScrollBars();
}



