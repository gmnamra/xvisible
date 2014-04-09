// rc_trackpanel.cpp


#include <QtGui/QtGui>
#include <QtCore/QtCore>
#include <Qt3Support/Q3VBox>

#include <rc_model.h>

#include "rc_modeldomain.h"
#include "rc_trackmanager.h"
#include "rc_appconstants.h"
#include "rc_scalartrackwidget.h"
#include "rc_trackgroupwidget.h"

// If rcHIDE_TRACKS is defined, all group tracks will be hidden when a group is disabled.
// If it is undefined, tracks remain visible but they will be disabled.

//#define rcHIDE_TRACKS 1

rcTrackGroupWidget::rcTrackGroupWidget( QWidget* parent , int trackGroupNo )
	: QWidget( parent , "trackWidget" )
{
    _trackGroupNo = trackGroupNo;
    
    //setSizePolicy( QSizePolicy( QSizePolicy::Fixed , QSizePolicy::Fixed ) );
    
	rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    rcTrackGroup* trackGroup = experiment->getTrackGroup( _trackGroupNo );
    _trackGroupName = trackGroup->getName();
    
    rcTrackManager* tmp = rcTrackManager::getTrackManager();

	QBoxLayout* layout = new QHBoxLayout( this );

    // add the group enable toggle pushbutton
    _enableButton = new rcTrackGroupEnableButton( this );
    _enableButton->setToggleButton( true );
    _enableButton->setMinimumWidth( 7 );
    _enableButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum , QSizePolicy::Minimum ) );
    if (tmp->isTrackGroupEnabled( _trackGroupNo ))
    {
        _enableButton->setOn( true );
        _enableButton->setText( "" );
    }
    else
    {
        _enableButton->setOn( false );
        _enableButton->setText( _trackGroupName );
    }
	QToolTip::add( _enableButton , trackGroup->getDescription() );
    layout->addWidget( _enableButton , 1 );

    // add a vertical box with the track widgets
    _trackWidgets = new Q3VBox( this );
    int nTracks = trackGroup->getNTracks();
	for (int i = 0; i < nTracks; i++)
	{
        rcTrack* track = trackGroup->getTrack( i );
        if ( rcTrackManager::displayableTrack( track ) ) {
            switch (track->getTrackType())
            {
                case eScalarTrack:
                    new rcScalarTrackWidget( _trackWidgets , _trackGroupNo , i );
                    break;
                case eVideoTrack:
                    new rcScalarTrackWidget( _trackWidgets , _trackGroupNo , i );
                    break;
                case eGraphicsTrack:
                    new rcScalarTrackWidget( _trackWidgets , _trackGroupNo , i );
                    break;
#ifdef notyet            
                case ePositionTrack:
                    new rcScalarTrackWidget( _trackWidgets , _trackGroupNo , i );
                    break;
#endif            
                default:
                    // unsupported track types don't get an entry.
                    break;
            }
        }
    }
	layout->addWidget( _trackWidgets , 10 );

    // connect the checkbox signal
    connect( _enableButton , SIGNAL( toggled( bool ) ) ,
             this          , SLOT( setEnabled( bool ) ) );

    // connect to some domain signals
	connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
			 this   , SLOT( updateState( rcExperimentState ) ) );
    connect( domain , SIGNAL( updateSettings( void ) ) ,
             this   , SLOT( updateSettings( void ) ) );
}

rcTrackGroupWidget::~rcTrackGroupWidget()
{
    QToolTip::remove( _enableButton );
}

// public slots:
void rcTrackGroupWidget::setEnabled( bool isEnabled )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcTrackManager* tmp = rcTrackManager::getTrackManager();
    tmp->setTrackGroupEnabled( _trackGroupNo , isEnabled );


    // Enable/disable all group members, too
    rcExperiment* experiment = domain->getExperiment();
    rcTrackGroup* trackGroup = experiment->getTrackGroup( _trackGroupNo );
    
    int nTracks = trackGroup->getNTracks();
#ifndef rcHIDE_TRACKS            
	for (int i = 0; i < nTracks; i++)
	{
        tmp->setTrackEnabled( _trackGroupNo, i, isEnabled );
    }
#endif
    
    domain->notifySettingChange();
    // Enabling graphics groups can affect video monitor
    if ( trackGroup->getType() == eGroupSemanticsGlobalGraphics )
        domain->notifyUpdateMonitor();
}

void rcTrackGroupWidget::updateState( rcExperimentState state )
{
    rmUnused( state );
}

void rcTrackGroupWidget::updateSettings()
{
    rcTrackManager* tmp = rcTrackManager::getTrackManager();

    // disconnect the checkbox signal to avoid recursion
    disconnect( _enableButton , SIGNAL( toggled( bool ) ) ,
                this          , SLOT( setEnabled( bool ) ) );

    if (tmp->isTrackGroupEnabled( _trackGroupNo ))
    {
        _enableButton->setOn( true );
        _enableButton->setText( "" );
        _enableButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum , QSizePolicy::Minimum ) );
        _trackWidgets->show();
    }
    else
    {

#ifdef rcHIDE_TRACKS        
        _enableButton->setText( _trackGroupName );
        _enableButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum , QSizePolicy::Maximum ) );
        _trackWidgets->hide();
#else
        _enableButton->setOn( false );
        _enableButton->setText( "" );
        _enableButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum , QSizePolicy::Minimum ) );
#endif        
    }
    // connect the checkbox signal
    connect( _enableButton , SIGNAL( toggled( bool ) ) ,
             this          , SLOT( setEnabled( bool ) ) );
}
