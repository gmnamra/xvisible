/******************************************************************************
 *   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
 *
 *	$Id: rc_scalartrackwidget.cpp 7191 2011-02-07 19:38:55Z arman $
 *
 ******************************************************************************/

#include <qlayout.h> 
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <q3frame.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3BoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>

#include <rc_model.h>

#include "rc_modeldomain.h"
#include "rc_trackmanager.h"
#include "rc_scalartrackwidget.h"
#include "rc_appconstants.h"
#include "rc_modifierkeys.h"

// TODO: change widget name, it works for all track types, not just for scalar tracks

rcScalarTrackWidget::rcScalarTrackWidget( QWidget* parent , int trackGroupNo , int trackNo )
	: QWidget( parent , "trackWidget" )
{
    // Gets (keyboard) focus by mouse clicking
    setFocusPolicy( Qt::ClickFocus);
    
    _trackGroupNo = trackGroupNo;
    _trackNo = trackNo;

	rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    rcTrackGroup* trackGroup = experiment->getTrackGroup( _trackGroupNo );
    rcTrack* track = trackGroup->getTrack( _trackNo );
    rcTrackManager* tmp = rcTrackManager::getTrackManager();
    bool hilited = tmp->isHiliteTrack( _trackGroupNo , _trackNo );
	Q3BoxLayout* layout = new Q3HBoxLayout( this );

    _checkbox = new QCheckBox( this );
    // retain old enablement information
    if ( tmp->isTrackEnabled( _trackGroupNo , _trackNo ) )
        _checkbox->setChecked( true );
    else
        _checkbox->setChecked( false );
	QToolTip::add( _checkbox , "Show/hide track" );
    layout->addWidget( _checkbox );

    // Simulate graph line with a colored character
	_legend = new QLabel( "-" , this );
    _legend->setAlignment( cUINoBreakAlignment );
    _legend->setFrameStyle( Q3Frame::Panel );
	_legend->setFrameShadow( Q3Frame::Sunken );
    _legend->setFixedWidth( 25 );

    // Set the color to reflect graph color
    _legend->setPaletteForegroundColor( tmp->getTrackPen( _trackGroupNo , _trackNo ).color() );

    // Set big bold "graph" font for better visibility
    QFont font;
    font.setWeight( QFont::Black );
    font.setPointSize( 20 );
    _legend->setFont( font );
	layout->addWidget( _legend );

    // Name/value font
    QFont textFont;
    textFont.setPointSize( 10 );
    if ( hilited )
        textFont.setWeight( QFont::Bold );
    else
        textFont.setWeight( QFont::Normal );
    
    // Track name
	_name = new QLabel( track->getName() , this );
    _name->setAlignment( cUIWordBreakAlignment );
    _name->setFrameStyle( Q3Frame::Panel );
	_name->setFrameShadow( Q3Frame::Sunken );
	_name->setFixedWidth( 180 );
    _name->setFont( textFont );
	layout->addWidget( _name );

    // Track value
	_value = new QLabel( "N/A" , this );
    _value->setAlignment( cUINoBreakAlignment );
    //_value->setAlignment( cUIWordBreakAlignment );
    _value->setFrameStyle( Q3Frame::Panel );
	_value->setFrameShadow( Q3Frame::Sunken );
	_value->setFixedWidth( 180 );
    _value->setFont( textFont );
	layout->addWidget( _value );

	layout->addSpacing( 10 );
	layout->addStretch(1);

	// use the track description as a tool tip
	QToolTip::add( _name , track->getDescription() );

    // connect the checkbox signal
    connect( _checkbox , SIGNAL( toggled( bool ) ) ,
             this      , SLOT( setEnabled( bool ) ) );

    // connect to some domain signals
	connect( domain , SIGNAL( elapsedTime( const rcTimestamp& ) ) ,
			 this   , SLOT( updateElapsedTime( const rcTimestamp& ) ) );
    connect( domain , SIGNAL( cursorTime( const rcTimestamp& ) ) ,
             this   , SLOT( updateCursorTime( const rcTimestamp& ) ) );
	connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
			 this   , SLOT( updateState( rcExperimentState ) ) );
    connect( domain , SIGNAL( updateDisplay( void ) ) ,
             this   , SLOT( updateDisplay( void ) ) );
    connect( domain , SIGNAL( updateSettings( void ) ) ,
             this   , SLOT( updateSettings( void ) ) );
}

rcScalarTrackWidget::~rcScalarTrackWidget()
{
    QToolTip::remove( _name );
    QToolTip::remove( _value );
    QToolTip::remove( _checkbox );  
}

// public slots:
void rcScalarTrackWidget::setEnabled( bool isEnabled )
{
    rcTrackManager* tmp = rcTrackManager::getTrackManager();
    tmp->setTrackEnabled( _trackGroupNo , _trackNo , isEnabled );
    // Enable the group, too
    if ( isEnabled )
        tmp->setTrackGroupEnabled( _trackGroupNo , isEnabled );
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    domain->notifySettingChange();

    // Enabling graphics groups can affect video monitor
    if ( tmp->groupType( _trackGroupNo ) == eGroupSemanticsGlobalGraphics )
        domain->notifyUpdateMonitor();
    else {
        // Enabling some tracks can affect video monitor
        rcTrackInfo info = tmp->getTrackInfo( _trackGroupNo , _trackNo );
//        if ( tmp->monitorableTrack( info.track() ) )
//             domain->notifyUpdateMonitor();
    }
}

void rcScalarTrackWidget::updateElapsedTime( const rcTimestamp& time )
{
    _elapsedTime = time;

    updateTrackDisplay();
}

void rcScalarTrackWidget::updateCursorTime( const rcTimestamp& time )
{
    _cursorTime = time;
    updateTrackDisplay();
}

void rcScalarTrackWidget::updateState( rcExperimentState state )
{
    rmUnused( state );
}

void rcScalarTrackWidget::updateDisplay( void )
{
    updateTrackDisplay();
}

void rcScalarTrackWidget::updateSettings( void )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    int groupCount = experiment->getNTrackGroups();

    if ( groupCount ) {
        rcTrackManager* tmp = rcTrackManager::getTrackManager();
        if ( tmp->isTrackEnabled( _trackGroupNo , _trackNo ) )
            _checkbox->setChecked( true );
        else
            _checkbox->setChecked( false );
        
        updateTrackDisplay();
    } else {
        // No groups available
        // Do nothing because we are about to be destructed soon
    }
}

// protected

void rcScalarTrackWidget::mousePressEvent( QMouseEvent* mouseEvent )
{
    rmUnused( mouseEvent );
    
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    rcTrackGroup* trackGroup = experiment->getTrackGroup( _trackGroupNo );
    rcTrack* track = trackGroup->getTrack( _trackNo );

    if ( track->getTrackType() == eScalarTrack ) {
        // Grab keyboard focus
        setFocus();
        // Highlight this track
        rcTrackManager* tmp = rcTrackManager::getTrackManager();
        // Only enabled tracks can be highlighted
        if ( tmp->isTrackEnabled( _trackGroupNo , _trackNo ) ) {
            tmp->setHiliteTrack( _trackGroupNo , -1 );
            tmp->setHiliteTrack( _trackGroupNo , _trackNo );
            // Change current analysis rect to track analysis rect
            rcRect focusRect = track->getAnalysisRect();
            if ( focusRect.width() && focusRect.height() )
                domain->notifyAnalysisRect( focusRect );
            domain->notifyUpdateDisplay();
        }
    }
}

void rcScalarTrackWidget::mouseMoveEvent( QMouseEvent* mouseEvent )
{
    rmUnused( mouseEvent );
}

void rcScalarTrackWidget::mouseReleaseEvent( QMouseEvent* mouseEvent )
{
    rmUnused( mouseEvent );
}

void rcScalarTrackWidget::keyPressEvent( QKeyEvent* keyEvent )
{
    int key = keyEvent->key();
    
    bool cmdKeyDown = rcModifierKeys::isCtrlDown( keyEvent );
    rcTrackManager* tmp = rcTrackManager::getTrackManager();
    
    switch ( key ) {
        case Qt::Key_Escape:
            // Reset highlighting
            tmp->setHiliteTrack( _trackGroupNo , -1 );
            break;

        case Qt::Key_C:
            if ( cmdKeyDown ) {
                // Copy track contents to clipboard
                tmp->copyTrackToClipboard( tmp->getHiliteTrack() );
            }
            break;
    
    }
}

void rcScalarTrackWidget::keyReleaseEvent( QKeyEvent* keyEvent )
{
    keyEvent->ignore();
}

// private

// called to update the value label, etc.
void rcScalarTrackWidget::updateTrackDisplay( void )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();

    // In camera mode we can only see the last value
    if ( domain->getExperimentAttributes().liveInput ) 
        _cursorTime = cCursorTimeCurrent;
      
    // determine the time to display
    rcTimestamp timeToDisplay = _cursorTime;
  
    if (_cursorTime < cZeroTime)    
        timeToDisplay = _elapsedTime;

    // if we don't have a valid time we can skip out now.
    if ( timeToDisplay < cZeroTime )
        return;

    rcExperiment* experiment = domain->getExperiment();
    int nTrackGroups = experiment->getNTrackGroups();

    if ( nTrackGroups > _trackGroupNo ) {
        rcTrackManager* tmp = rcTrackManager::getTrackManager();

        if ( tmp->isTrackEnabled( _trackGroupNo , _trackNo ) ) {
            // Show current values only for enabled tracks
            rcTrackGroup* trackGroup = experiment->getTrackGroup( _trackGroupNo );
	    const int32 semantic = trackGroup->getType();

            rcTrack* track = trackGroup->getTrack( _trackNo );
            
            if ( track ) {
                rcTrackType type = track->getTrackType();

                rcSegmentIterator* iter = 0;
                
                if ( type == eScalarTrack ) {
                    // get the scalar track.
                    rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );
                    if (scalarTrack == 0)
                        return;
                    // get the scalar value at the time to display
                    iter = scalarTrack->getDataSegmentIterator( timeToDisplay );
                } else if ( type == eVideoTrack ) {
                    // get the video track
                    rcVideoTrack* videoTrack = dynamic_cast<rcVideoTrack*>( track );
                    if (videoTrack == 0)
                        return;
                    iter = videoTrack->getDataSegmentIterator( timeToDisplay );
                } else if ( type == eGraphicsTrack ) {
                    // get the graphics track.
                    rcGraphicsTrack* graphicsTrack = dynamic_cast<rcGraphicsTrack*>( track );
                    if (graphicsTrack == 0)
                        return;
                    // get the value at the time to display
                    iter = graphicsTrack->getDataSegmentIterator( timeToDisplay );                    
                } else if ( type == ePositionTrack ) {

                    // get the position track.
                    rcPositionTrack* pTrack = dynamic_cast<rcPositionTrack*>( track );
                    if (pTrack == 0) {
                        rmAssert( 0 );
                        return;
                    }
                    // get the value at the time to display
                    iter = pTrack->getDataSegmentIterator( timeToDisplay );
                } else {
                    rmAssert( 0 );
                }

                if ( iter ) {
                    char valueBuf[ 128 ];
                    bool hasValue = iter->contains( timeToDisplay );

                    // Trying to get the very last value?
                    if ( !hasValue && _cursorTime == cCursorTimeCurrent ) {
                        // The very last value may not be available yet so go back one segment
                        iter->advance( -1 );
                        if ( iter->getSegmentIndex() != -1 )
                            hasValue = true;
                    }

                    if ( hasValue ) {
                        // timeToDisplay is within the segment
                        if ( type == eScalarTrack ) {
                            rcScalarSegmentIterator* si = dynamic_cast<rcScalarSegmentIterator*>(iter);
			    double value = si->getValue();
			    if ( semantic == eGroupSemanticsGlobalMeasurements)
			      value = log (value);
                            snprintf( valueBuf , rmDim(valueBuf),
                                      track->getDisplayFormatString(), value );
                        } else if ( type == eVideoTrack ) {
                            int idx = iter->getSegmentIndex();
                            snprintf( valueBuf , rmDim(valueBuf), "Frame " );
                            int len = strlen( valueBuf );
                            snprintf( valueBuf + len , rmDim(valueBuf)-len,
                                      track->getDisplayFormatString(), idx );
                        } else if ( type == eGraphicsTrack ) {
                            rcGraphicsSegmentIterator* gi = dynamic_cast<rcGraphicsSegmentIterator*>(iter);
                            uint32 graphicCount = gi->getCount();
                            snprintf( valueBuf, rmDim(valueBuf),
                                      track->getDisplayFormatString(), graphicCount );
                        } else if ( type == ePositionTrack ) {
                            rcPositionSegmentIterator* pi = dynamic_cast<rcPositionSegmentIterator*>(iter);
                            rcFPair value = pi->getValue();
                            snprintf( valueBuf, rmDim(valueBuf),
                                      track->getDisplayFormatString(), value.x(), value.y() );
                        }
                    } else {
                        // timeToDisplay is either before track start or after track end
                        snprintf( valueBuf , rmDim(valueBuf), "Undefined" );
                    }
                    
                    _value->setText( valueBuf );
                    // Add tool tip to show the value, too
                    QToolTip::remove( _value );
                    QToolTip::add( _value , valueBuf );
                    //cerr << *iter;
                    delete iter;
                }
            }
        } else {
            // Value not available
            _value->setText( "N/A" );
            // Add tool tip to show the value, too
            QToolTip::remove( _value );
            QToolTip::add( _value , "N/A" );
        }
        
        QFont textFont = _value->font();
        bool hilited = tmp->isHiliteTrack( _trackGroupNo , _trackNo );
#if 1    
        if ( hilited ) {
            textFont.setWeight( QFont::Bold );
        } else {
            textFont.setWeight( QFont::Normal );
        }
#else    
        if ( hilited ) {
            textFont.setUnderline( true );
        } else {
            textFont.setUnderline( false );
        }
#endif    
        _value->setFont( textFont );
        _name->setFont( textFont );
    }
    
    _currentTime = timeToDisplay;
}
