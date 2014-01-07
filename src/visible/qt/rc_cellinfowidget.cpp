/******************************************************************************
 *   Copyright (c) 2003 Reify Corp. All Rights reserved.
 *
 *	$Id: rc_cellinfowidget.cpp 7191 2011-02-07 19:38:55Z arman $
 *
 *  Cell info display widget
 *
 ******************************************************************************/

#include <qlayout.h> 
#include <qlabel.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <rc_model.h>
#include <rc_kinetoscope.h>

#include "rc_modeldomain.h"
#include "rc_trackmanager.h"
#include "rc_cellinfowidget.h"
#include "rc_appconstants.h"

// Widget background erase mode
static const Qt::BackgroundMode cWidgetBackgroundMode = Qt::FixedColor;
// Automatically break on word boundaries
static const int cWordBreakAlignment = (Qt::AlignLeft | Qt::AlignTop | Qt::WordBreak);
// Base window caption
static const QString cBaseCaption( "Locomotive Body Data: ");


// Count displayable tracks and compose a window caption string
static uint32 countDisplayableGroups( QString& caption )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    rcTrackManager* tmp = rcTrackManager::getTrackManager();
    uint32 displayableGroups = 0;
    
    // Count displayable tracks and compose a window caption
    int nTrackGroups = experiment->getNTrackGroups();
    for (int i = 0; i < nTrackGroups; i++)
    {
        rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
        if ( tmp->isTrackGroupEnabled( i ) ) {
            // Only display data from cell info groups
            if ( trackGroup->getType() == eGroupSemanticsBodyMeasurements ) {
                // Get track name
                if ( !displayableGroups ) {
                    caption.sprintf( "%s%s",
                                     cBaseCaption.latin1(), trackGroup->getName() );
                }
                ++displayableGroups;
            }
        }
    }

    if ( !displayableGroups ) {
         caption.sprintf( "%sno data",
                          cBaseCaption.latin1() );
    } else if ( displayableGroups > 1 ) {
        caption.sprintf("%s%i cells",
                        cBaseCaption.latin1(), displayableGroups );
    } 
    return displayableGroups;
}

// Get track value string
static void getTrackValue( rcTrack* track, const rcTimestamp& timeToDisplay, QString& value )
{
    rcTrackType type = track->getTrackType();
    
    if ( track ) {
        char valueBuf[ 256 ];
        rcSegmentIterator* iter = 0;
        
        if ( type == eScalarTrack ) {
            // get the scalar track.
            rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );
            if (scalarTrack == 0)
                return;
            // get the scalar value at the time to display
            iter = scalarTrack->getDataSegmentIterator( timeToDisplay );
        } else if ( type == ePositionTrack ) {
            
            // get the position track.
            rcPositionTrack* pTrack = dynamic_cast<rcPositionTrack*>( track );
            if (pTrack == 0) {
                return;
            }
            // get the value at the time to display
            iter = pTrack->getDataSegmentIterator( timeToDisplay );
        } 

        if ( iter ) {
            if ( iter->contains( timeToDisplay ) ) {
                // timeToDisplay is within the segment
                if ( type == eScalarTrack ) {
                    rcScalarSegmentIterator* si = dynamic_cast<rcScalarSegmentIterator*>(iter);
                    double value = si->getValue();
                    // Special case for cell state track
                    if ( rcWriterManager::tagType( track->getTag() ) == eWriterBodyState ) {
                        rfVisualFunctionStateName( static_cast<rcVisualFunction::State>(value),
                                   valueBuf,
                                   rmDim(valueBuf) );
                    } else {
                        // Generic scalar track
                        snprintf( valueBuf , rmDim(valueBuf),
                                  track->getDisplayFormatString(), value );
                    }
                } else if ( type == ePositionTrack ) {
                    rcPositionSegmentIterator* pi = dynamic_cast<rcPositionSegmentIterator*>(iter);
                    rcFPair value = pi->getValue();
                    snprintf( valueBuf, rmDim(valueBuf),
                              "%.4f, %.4f", value.x(), value.y() );
                }
            } else {
                // timeToDisplay is either before track start or after track end
                snprintf( valueBuf , rmDim(valueBuf), "Undefined" );
            }
            value.sprintf( "%s\n", valueBuf );
            delete iter;
        }
    }
}

rcCellInfoWidget::rcCellInfoWidget( QWidget* parent )
	: QWidget( parent , "cellInfoWidget" )
{
    setCaption( cBaseCaption );

    setMinimumSize( 400, 160 );
	rcModelDomain* domain = rcModelDomain::getModelDomain();
    mTopLayout = new Q3VBoxLayout( this );
	mDataLayout = new Q3HBoxLayout( mTopLayout );
    
    // Name/value font
    QFont textFont;
    textFont.setPointSize( 10 );
    textFont.setWeight( QFont::Normal );

    mDataLayout->addSpacing( 10 );
    // Track name
	mDataName = new QLabel( "\t" , this );
    mDataName->setAlignment( cWordBreakAlignment );
    mDataName->setFont( textFont );
    QToolTip::add( mDataName, "Data field name" );
	mDataLayout->addWidget( mDataName );
    mDataLayout->addSpacing( 10 );
    
    // Track value
	mDataValue = new QLabel( "\t" , this );
    mDataValue->setAlignment( cWordBreakAlignment );
    mDataValue->setFont( textFont );
    QToolTip::add( mDataValue, "Data field value" );
	mDataLayout->addWidget( mDataValue );
	mDataLayout->addSpacing( 10 );
    
    // Track comment
	mDataComment = new QLabel( "\t" , this );
    mDataComment->setAlignment( cWordBreakAlignment );
    mDataComment->setFont( textFont );
    QToolTip::add( mDataComment, "Exportability" );
	mDataLayout->addWidget( mDataComment );
    
	mDataLayout->addSpacing( 10 );
	mDataLayout->addStretch();
    
    // connect to some domain signals
    connect( domain , SIGNAL( cursorTime( const rcTimestamp& ) ) ,
             this   , SLOT( updateCursorTime( const rcTimestamp& ) ) );
	connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
			 this   , SLOT( updateState( rcExperimentState ) ) );
    connect( domain , SIGNAL( updateDisplay( void ) ) ,
             this   , SLOT( updateDisplay( void ) ) );
    connect( domain , SIGNAL( updateSettings( void ) ) ,
             this   , SLOT( updateSettings( void ) ) );

    // Use a fixed color to erase background, it's faster than default widget erase pixmap
    QColor backgroundColor = QColor( 238, 238, 238 );
    
    setBackgroundMode( cWidgetBackgroundMode, cWidgetBackgroundMode );
    setPaletteBackgroundColor( backgroundColor );

    updateTrackDisplay();
}

rcCellInfoWidget::~rcCellInfoWidget()
{
    QToolTip::remove( mDataName );
    QToolTip::remove( mDataValue );
}

void rcCellInfoWidget::updateCursorTime( const rcTimestamp& time )
{
    rmUnused( time );
    updateTrackDisplay();
}

void rcCellInfoWidget::updateState( rcExperimentState state )
{
    rmUnused( state );
    updateTrackDisplay();
}

void rcCellInfoWidget::updateDisplay( void )
{
    updateTrackDisplay();
}

void rcCellInfoWidget::updateSettings( void )
{
    updateTrackDisplay();
}

// protected

// private

// called to update the value label, etc.
void rcCellInfoWidget::updateTrackDisplay( void )
{
    // determine the time to display
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcTimestamp timeToDisplay  = domain->getCursorTime();
    
    rcExperiment* experiment = domain->getExperiment();
    rcTrackManager* tmp = rcTrackManager::getTrackManager();
    int nTrackGroups = experiment->getNTrackGroups();

    // Count displayable groups and get caption
    QString caption;
    countDisplayableGroups( caption );
    setCaption( caption );

    QString name, value, comment;

    if ( timeToDisplay >= cZeroTime ) {
        // Gather all data track values
        for (int i = 0; i < nTrackGroups; ++i)
        {
            rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
            if ( tmp->isTrackGroupEnabled( i ) ) {
                // Only display data from cell info groups
                if ( trackGroup->getType() == eGroupSemanticsBodyMeasurements ) {
                    int nTracks = trackGroup->getNTracks();
                    
                    name += trackGroup->getName();
                    name += "\n";
                    value += "\n";
                    comment += "\n";
                    for ( int j = 0; j < nTracks; j++)
                    {
                        rcTrack* track = trackGroup->getTrack( j );
                        // Track name
                        QString tn;
                        tn.sprintf( " %s\n", track->getDescription() );
                        name += tn;
                        // Track value
                        QString tv;
                        getTrackValue( track, timeToDisplay, tv );
                        value += tv;
                        // Track comment
                        if ( track->isExportable() ) {
                            // Track is exportable to file
                            comment += "E\n";
                        }
                        else
                            comment += "\n";
                    }
            }
            }
        }
    }
    // Update texts
    mDataName->setText( name );
    mDataValue->setText( value );
    mDataComment->setText( comment );
    
    mLastUpdateTime = timeToDisplay;
}
