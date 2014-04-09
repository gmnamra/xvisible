/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_monitor.cpp 7191 2011-02-07 19:38:55Z arman $
 *
 *	This file contains video monitor implementation.
 *
 ******************************************************************************/

#include <algorithm>


#include <QtGui/QtGui>
#include <QtCore/QtCore>

#include <rc_window.h>
#include <rc_math.h>
#include <rc_writermanager.h>
#include <rc_main_window.h>

#include "rc_trackmanager.h"
#include "rc_modeldomain.h"
#include "rc_monitor.h"
#include "lpwidget.h"

// Min/max widget size
static const int cMaxWidth = 1600;
static const int cMaxHeight = 1280;

// Minimum scale factor
static double cMinScaleFactor = 0.25;
// Maximum scale factor
static double cMaxScaleFactor = 50000;
// Scale up/down increment 
static double cScaleUpFactor = 0.25;

// Widget background erase mode
static const Qt::BackgroundMode cWidgetBackgroundMode = Qt::FixedColor;

// Specify drawing styles
// Cell center position marker style
static const rcStyle cPositionCrossStyle( rfRgb( 30, 100, 255 ), 1, rc2Fvector( 0.0f, 0.0f ) );
// Cell center position marker cross length
static const int cPositionCrossLength = 6;
// Cell name text style
static const rcStyle cTextStyle( rfRgb( 255, 125, 125 ), 1, rc2Fvector( 0.0f, 0.0f ) );

rcMonitor::rcMonitor( QWidget* parent, const char* name)
        : QWidget( parent, name ), mSpeedCalc( 15 )
{

    mCellText = mCellPath = mCellPos = mDevelopmentData = false;
    
    mCursorTime = cCursorTimeCurrent;

    // Create top level layout
    mTopLayout = new QVBoxLayout( this );
    mTopLayout->setAlignment( Qt::AlignLeft | Qt::AlignBottom );

    // 2D information display
    mCanvas = new rcImageCanvasGL( this , "dummy" );
    
    mTopLayout->addWidget( mCanvas );
    mTopLayout->setStretchFactor( mCanvas, 20 );
    
    // Create a horizontal layout
    mStatusLayout = new QHBoxLayout( mTopLayout );
    mStatusLayout->setAlignment( Qt::AlignRight | Qt::AlignBottom );
    mStatusLayout->addSpacing( 2 );
    
    // Create scale control widget
    mScaleWidget = new QSpinBox( int(cMinScaleFactor*100), int(cMaxScaleFactor*100), int(cScaleUpFactor*100), this, "scale" );
    mScaleWidget->setSuffix( " %" );
    mScaleWidget->setValue( static_cast<int>(100) );
    QToolTip::add( mScaleWidget, "Image scale factor" );
    mScaleWidget->setMaximumWidth( fontMetrics().width('M') * 7 );
    mStatusLayout->addWidget( mScaleWidget );

    connect( mScaleWidget , SIGNAL( valueChanged( int ) )  ,
             this , SLOT( scaleChanged( int ) ) );

    // Create saturation detector widget label
    mSaturationWidgetLabel = new QLabel( this );
    mSaturationWidgetLabel->setText( "Sat" );
    mSaturationWidgetLabel->setTextFormat( Qt::PlainText );
    mSaturationWidgetLabel->setAlignment( Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine );
    QToolTip::add( mSaturationWidgetLabel, "Enable/disable saturated pixel coloring (red)" );
    mStatusLayout->addWidget( mSaturationWidgetLabel );
    mSaturationWidgetLabel->setPaletteForegroundColor( Qt::red );
    
    // Create saturation detector widget
    mSaturationWidget = new QCheckBox( this );
    QToolTip::add( mSaturationWidget, "Enable/disable saturated pixel coloring (red)" );
    mSaturationWidget->setChecked( true );
    mStatusLayout->addWidget( mSaturationWidget );
    mStatusLayout->addSpacing( 2 );
    connect( mSaturationWidget , SIGNAL( toggled( bool ) ) , this , SLOT( saturationChanged( bool ) ) );

    // Create status display widget
    mStatusWidget = new QLabel( this );
    mStatusWidget->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    mStatusWidget->setFixedHeight( fontMetrics().height() + 4 );
    mStatusWidget->setMinimumWidth( fontMetrics().width('M') * 40 );
    // Make the text format as simple as possible for speed
    mStatusWidget->setTextFormat( Qt::PlainText );
    mStatusWidget->setAlignment( Qt::AlignAuto | Qt::AlignVCenter | Qt::SingleLine );
    QToolTip::add( mStatusWidget, "Image status details" );
    mStatusLayout->addWidget( mStatusWidget );
    mStatusLayout->setStretchFactor( mStatusWidget, 10 );
     
    // Receive mouse events even when no buttons are pressed
    setMouseTracking( true );
    // Gets (keyboard) focus by mouse clicking
    setFocusPolicy( Qt::ClickFocus );
    
    // Connect image display refresh
    rcModelDomain* domain = rcModelDomain::getModelDomain();

    connect( domain , SIGNAL( updateDisplay( const rcWindow* ) ) ,
             this   , SLOT( updateDisplay( const rcWindow* ) ) );
    
    connect( domain , SIGNAL( updateDisplay( const rcVisualGraphicsCollection* ) ) ,
             this   , SLOT( updateDisplay( const rcVisualGraphicsCollection* ) ) );
   
    connect( domain , SIGNAL( updateDisplay( void ) ) ,
             this   , SLOT( updateDisplay( void ) ) );

    connect( domain , SIGNAL( updateMonitorDisplay( void ) ) ,
             this   , SLOT( updateDisplay( void ) ) );

    connect( domain , SIGNAL( updateDebugging( void ) ) ,
             this   , SLOT( updateDisplay( void ) ) );
     
    connect( domain , SIGNAL( updateAnalysisRect( const rcRect& ) ) ,
             this   , SLOT( updateAnalysisRect( const rcRect& ) ) );

    connect( domain , SIGNAL( elapsedTime( const rcTimestamp& ) ) ,
             this   , SLOT( updateTime( const rcTimestamp& ) ) );
    
    connect( domain , SIGNAL( cursorTime( const rcTimestamp& ) ) ,
             this   , SLOT( updateCursorTime( const rcTimestamp& ) ) );

    connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
             this   , SLOT( updateState( rcExperimentState ) ) );

	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );

    connect( domain , SIGNAL( updateMonitorScale( double ) ),
             this   , SLOT( updateScale( double ) ) );

    connect( domain , SIGNAL( updateVideoRect( const rcRect& ) ) ,
             this   , SLOT( updateMonitorSize( const rcRect& ) ) );

    connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
             mCanvas   , SLOT( updateState( rcExperimentState ) ) );

    connect( domain , SIGNAL( updateAnalysisRectRotation( const rcAffineRectangle& ) ) ,
             this   , SLOT( updateAnalysisRectRotation( const rcAffineRectangle& ) ) );
    
    
   // connect( domain , SIGNAL( requestPlot ( SharedCurveDataRef&) ) ,
    //        this  , SLOT( reload_plotter ( SharedCurveDataRef& ) ) );    

    // Use a fixed color to erase background, it's faster than default widget erase pixmap
    mBackgroundColor = QColor( 238, 238, 238 );
    
    setBackgroundMode( cWidgetBackgroundMode, cWidgetBackgroundMode );
    setPaletteBackgroundColor( mBackgroundColor );
    mStatusWidget->setPaletteBackgroundColor( mBackgroundColor );
    mStatusWidget->setBackgroundMode( cWidgetBackgroundMode, cWidgetBackgroundMode );
    mSaturationWidgetLabel->setPaletteBackgroundColor( mBackgroundColor );
    mSaturationWidgetLabel->setBackgroundMode( cWidgetBackgroundMode, cWidgetBackgroundMode );
    // TODO: saturation checkbox has some white background, fix it
    mSaturationWidget->setPaletteBackgroundColor( mBackgroundColor );
    mSaturationWidget->setBackgroundMode( cWidgetBackgroundMode, cWidgetBackgroundMode );
    mScaleWidget->setPaletteBackgroundColor( mBackgroundColor );
    mScaleWidget->setBackgroundMode( cWidgetBackgroundMode, cWidgetBackgroundMode );
   
    
}


rcMonitor::~rcMonitor()
{
    deleteCachedImage();
    // Remove old tips
    QToolTip::remove( this );
    QToolTip::remove( mScaleWidget );
    QToolTip::remove( mStatusWidget );
    QToolTip::remove( mSaturationWidget );
    QToolTip::remove( mSaturationWidgetLabel );
}

// public slots

// Update elapsed time, display current image
void rcMonitor::updateTime( const rcTimestamp& cursorTime )
{
    rmUnused( cursorTime );
}

// Update cursor time, display current image
void rcMonitor::updateCursorTime( const rcTimestamp& cursorTime )
{
    mCursorTime = cursorTime;
    internalUpdateDisplay();
}

// Update image display with a new frame
void rcMonitor::updateDisplay( const rcWindow* image )
{
    if ( image ) {
        // Disable scaling for capture input
        if ( mScaleWidget->isEnabled() && inputIsStored() )
            mScaleWidget->setEnabled( false );
        // Update screen only when cursor is at current time
        if ( mCursorTime == cCursorTimeCurrent ) {
            updateCachedImage( image );
        }
        updateStatus();
    } else {
        deleteCachedImage();
    }
}

// Update display with new graphics
void rcMonitor::updateDisplay( const rcVisualGraphicsCollection* graphics )
{
    rmUnused( graphics );
    cerr << "rcMonitor::updateDisplay( const rcVisualGraphicsCollection* graphics ) unimplemented!" << endl;
    updateStatus();
}

// Update image display, get a fresh frame
void rcMonitor::updateDisplay( void )
{
    internalUpdateDisplay();
}

void rcMonitor::updateAnalysisRect( const rcRect& rect )
{
    mCanvas->updateAnalysisRect( rect );
    internalUpdateDisplay();
}

void rcMonitor::updateAnalysisRectRotation( const rcAffineRectangle& affine )
{
  mCanvas->updateAnalysisRectRotation( affine );
  internalUpdateDisplay();
}

void rcMonitor::updateState( rcExperimentState state )
{
    // Disable scaling for capture input
    mScaleWidget->setEnabled( !inputIsStored() );

    switch( state ) {
        case eExperimentEmpty:
            // Starting a new experiment
            // Remove cached image
            deleteCachedImage();
            break;
        default:
            break;
    }
    mSpeedCalc.reset();
}

// Scale is a factor
void rcMonitor::updateScale( double scale )
{
    if ( scale > 0.0 ) {
        scaleChanged( static_cast<int>(scale*100), true );
        internalUpdateDisplay();
    } else {
        // Disable resizing for capture
        if ( inputIsStored() ) {
            setMaximumSize( size() );
        } else {
            setMaximumSize( QSize( cMaxWidth, cMaxHeight ) );
        }
    }
}


// Resize video canvas to nSize
void rcMonitor::updateMonitorSize( const rcRect& nSize )
{
    rmUnused( nSize );
}

// Scale is a percentage
void rcMonitor::scaleChanged( int scale )
{
    scaleChanged( scale, false );
}

// Some UI setting changed
void rcMonitor::settingChanged()
{
    // Update scale widget
    mScaleWidget->setEnabled( !inputIsStored() );

    // TBD: should we enable saturation indicator in non-capture mode?
    
    // Update saturation widget
    mSaturationWidget->setEnabled( inputIsCamera() );
    // Hide saturation widget if the setting is currently not enabled.
	if ( mSaturationWidget->isHidden() && mSaturationWidget->isEnabled() )
	{
        mSaturationWidget->setChecked( true );
		mSaturationWidget->show();
        mSaturationWidgetLabel->show();
	}
	else if ( !mSaturationWidget->isHidden() && !mSaturationWidget->isEnabled() )
	{
		mSaturationWidget->hide();
        mSaturationWidgetLabel->hide();
	}

    // Disable resizing for capture
    if ( inputIsStored() ) {
        setMaximumSize( size() );
    } else {
        setMaximumSize( QSize( cMaxWidth, cMaxHeight ) );
    }

	rcModelDomain* domain = rcModelDomain::getModelDomain();
	rcExperiment* experiment = domain->getExperiment();
	int nPages = experiment->getNSettingCategories();
    
	for (int i = 0; i < nPages; i++)
	{
		rcSettingCategory category = experiment->getSettingCategory( i );

        // Look for cell visualization settings
        if ( !strcmp( category.getTag(), "visualization-settings" ) ) {
            const rcSettingInfo path = category.getSettingInfo( "visualize-cell-path" );
            const rcSettingInfo position = category.getSettingInfo( "visualize-cell-position" );
            const rcSettingInfo text = category.getSettingInfo( "visualize-cell-text" );
            const rcSettingInfo dev = category.getSettingInfo( "visualize-development-data" );
            bool changed = false;
            
            int value = int(path.getValue());
            if ( value != mCellPath ) {
                mCellPath = value;
                changed = true;
            }
            value = int(position.getValue());
            if ( value != mCellPos ) {
                mCellPos = value;
                changed = true;
            }
            value = int(text.getValue());
            if ( value != mCellText ) {
                mCellText = value;
                changed = true;
            }
            value = int(dev.getValue());
            if ( value != mDevelopmentData ) {
                mDevelopmentData = value;
                changed = true;
            }
            if ( changed ) {
                // Update whole widget
                internalUpdateDisplay();
            }
        }
    }
}

// Saturation display setting changed
void rcMonitor::saturationChanged( bool enabled )
{
    // With camera input the next image will obey the setting
    if ( !inputIsCamera() )
        mCanvas->showSaturatedPixels( enabled );
}

// Set maximum scale for scale widget
void rcMonitor::setMaxScale( int scale )
{
    mScaleWidget->setMaxValue( scale );
}

// Update status bar display
void rcMonitor::updateStatus()
{
    QString text;

    // Update status bar
    mCanvas->statusString( text );

    bool cameraInput = inputIsCamera();
    rcExperimentState state = rcModelDomain::getModelDomain()->getExperimentState();
    
    // Display FPS for camera or movie playback
    if ( cameraInput || state == eExperimentPlayback ) {
        QString moremsg;
        
        moremsg.sprintf(", display %3.1f fps",
                        mSpeedCalc.fps() );
        text += moremsg;
    }
        
    mStatusWidget->setText( text );

    // Update scale widget
    int scale = mCanvas->currentScale();
    mScaleWidget->setValue( scale );
}

// Public methods

void rcMonitor::paintEvent( QPaintEvent* /*paintEvent*/ )
{
    internalUpdateDisplay();
}

void rcMonitor::resizeEvent( QResizeEvent */*resizeEvent*/ )
{
}

// Protected methods

void rcMonitor::mousePressEvent( QMouseEvent* mouseEvent )
{
    rmUnused( mouseEvent );
}

void rcMonitor::mouseMoveEvent( QMouseEvent* mouseEvent )
{
    rmUnused( mouseEvent );
}

void rcMonitor::mouseReleaseEvent( QMouseEvent* mouseEvent )
{
    rmUnused( mouseEvent );
#if 0
    // Just a simple click
    deque<rcFocusTrack> tracks;
    rcTrackManager* tmp = rcTrackManager::getTrackManager();
    
    // Check if existing focus areas were hit
    if ( findFocusTracks( p, tracks ) > 0 ) {
            // Get the track (focus area) with the shortest distance to p
        partial_sort( tracks.begin(), tracks.begin()+1, tracks.end(), focusCompare );
        // Highlight the track
        tmp->setHiliteTrack( -1 , -1 );
        tmp->setHiliteTrack( tracks[0]._trackGroupNo , tracks[0]._trackNo );
        // Update current analysis rect
        domain->notifyAnalysisRect( tracks[0]._focusRect );
    } else {
        // Nothing hit, erase current focus and highlighting
        tmp->setHiliteTrack( -1 , -1 );
        newRect.setHeight( 0 );
        newRect.setWidth( 0 );
        domain->notifyAnalysisRect( newRect );
    }
#endif    
}

void rcMonitor::keyPressEvent( QKeyEvent* keyEvent )
{
    // No zoom/pan allowed during live storage
    if ( inputIsStored() )
        return;

    keyEvent->ignore();
}

void rcMonitor::keyReleaseEvent( QKeyEvent* keyEvent )
{
    keyEvent->ignore();
}

// Private methods

// Scale is a percentage
bool rcMonitor::scaleChanged( int scale, bool updateWidget )
{
    scale = int(100 * mCanvas->scaleChanged( scale/100.0 ));
    // Update widget
    if ( updateWidget )
        mScaleWidget->setValue( scale );

    updateStatus();

    return true;
}

// Display current track
void rcMonitor::displayTrackFocus( rcTrackType trackType )
{
    // draw focus of each data track.
    bool drawCurrentFocus = false;

    if ( trackType == eScalarTrack )
        drawCurrentFocus = true;
    
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    rcTrackManager* tmp = rcTrackManager::getTrackManager();
    int lastVideoGroup = -1;
    int lastVideoTrack = -1;
    
    rcWriterSemantics wantedTrack = eWriterVideo;
    if ( showDevelopmentData() )
        wantedTrack = eWriterVideoDevelopment;
    
    // paint video track focus areas (and raw full frame video, too)
    int nTrackGroups = experiment->getNTrackGroups();
    for (int i = 0; i < nTrackGroups; i++)
    {
        rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
        int nTracks = trackGroup->getNTracks();
        for ( int j = 0; j < nTracks; j++)
        {
            rcTrack* track = trackGroup->getTrack( j );
            if ( track->getTrackType() == trackType )
            {
                switch ( trackType ) {
                    case eVideoTrack:
                        if ( rcWriterManager::tagType( track->getTag() ) == wantedTrack ) {
                            rcTrackInfo trackInfo = tmp->getTrackInfo( i, j );
                            if ( trackInfo.isEnabled() ) {
                                lastVideoGroup = i;
                                lastVideoTrack = j;
                            }
                        }
                        break;
                    
                    default:
                        rmAssert( 0 );
                        break;
                }
            }
        }
    }

    // Display only last valid track, there is no video overlay support yet
    if ( lastVideoGroup >= 0 && lastVideoTrack >= 0 ) {
            rcTrackGroup* trackGroup = experiment->getTrackGroup( lastVideoGroup );
            rcTrack* track = trackGroup->getTrack( lastVideoTrack );
            paintVideoTrackFocus( mCursorTime, track );
    } else {
        // Paint graphics and text with no video
        rcWindow empty;
        updateCachedImage( &empty );
    }
}

// Gather displayable graphics and text data
void rcMonitor::gatherTrackData( rcVisualGraphicsCollectionCollection& globalGraphics,
                                 rcVisualGraphicsCollectionCollection& cellGraphics,
                                 rcTextSegmentCollection& texts ) const
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    rcTrackManager* tmp = rcTrackManager::getTrackManager();

    // paint video track focus areas (and raw full frame video, too)
    int nTrackGroups = experiment->getNTrackGroups();
    for (int i = 0; i < nTrackGroups; i++)
    {
        rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
        int nTracks = trackGroup->getNTracks();
        for ( int j = 0; j < nTracks; j++)
        {
            rcTrack* track = trackGroup->getTrack( j );
            rcTrackType trackType = track->getTrackType();

            switch ( trackType ) {
                case ePositionTrack:
                {
                    rcTrackInfo trackInfo = tmp->getTrackInfo( i , j );
                    if ( trackInfo.isEnabled() ) {
                        // Use only body position values (there are other position tracks with other data)
                        if ( rcWriterManager::tagType( track->getTag() ) == eWriterBodyPosition ) {
                            rcFPair pos;
                            rcRect rect;
                            bool validPosition = getPosition( mCursorTime, track, pos, rect );

                            if ( validPosition ) {
                                // Make a line segment collection with drawing style
                                rcVisualSegmentCollection lines;
                                rcVisualGraphicsCollection posGraphics( cPositionCrossStyle, lines );

                                // TBD: can we get the whole path from kinetoscope or do we need
                                // to accumulate cell positions?
                                if ( mCellPath ) {
                                    // Get cumulative position path for each cell
                                    buildPositionPath( mCursorTime, track, posGraphics );
                                }
                                if ( mCellPos ) {
                                    // Get cross for each cell
                                    buildPositionCross( pos, posGraphics );
                                }
#ifdef notyet // We probably want the rect from a separate data track                                
                                if ( mCellPos ) {
                                    // Get boundary for each cell
                                    buildCellBoundary( rect, posGraphics );
                                }
#endif                                
                                cellGraphics.push_back( posGraphics );
                            
                                if ( mCellText ) {
                                    // Get name for each cell
                                    std::string cellName = trackGroup->getName();
                                    rcTextSegment text( rc2Fvector(pos.x(), pos.y()), cellName );
                                    texts.push_back( text );
                                }
                            }
                        }
                    }
                }
                break;
                        
                case eGraphicsTrack:
                {
                    if ( rcWriterManager::tagType( track->getTag() ) != eWriterGraphicsDevelopment ) {
                        rcTrackInfo trackInfo = tmp->getTrackInfo( i , j );
                        if ( trackInfo.isEnabled() ) {
                            // Gather all graphics
                            addGraphics( mCursorTime, track, globalGraphics );
                        }
                    }
                }
                break;
                
                default:
                    break;
            }
        }
    }
}

// Gather displayable graphics and text data from development tracks
void rcMonitor::gatherDevelopmentTrackData( rcVisualGraphicsCollectionCollection& globalGraphics,
                                            rcVisualGraphicsCollectionCollection& cellGraphics,
                                            rcTextSegmentCollection& texts ) const
{
    rmUnused( cellGraphics );
    
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    rcTrackManager* tmp = rcTrackManager::getTrackManager();

    // paint video track focus areas (and raw full frame video, too)
    int nTrackGroups = experiment->getNTrackGroups();
    for (int i = 0; i < nTrackGroups; i++)
    {
        rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
        int nTracks = trackGroup->getNTracks();
        for ( int j = 0; j < nTracks; j++)
        {
            rcTrack* track = trackGroup->getTrack( j );
            rcTrackType trackType = track->getTrackType();

            switch ( trackType ) {
                case eGraphicsTrack:
                {
                    if ( rcWriterManager::tagType( track->getTag() ) == eWriterGraphicsDevelopment ) {
                        rcTrackInfo trackInfo = tmp->getTrackInfo( i , j );
                        if ( trackInfo.isEnabled() ) {
                            // Gather all graphics
                            addGraphics( mCursorTime, track, globalGraphics );
                        }
                    }
                }
                break;
                
                default:
                    break;
            }
        }
    }

    // Write our frame count
    rc2Fvector z;
    ostringstream os;
    os<< "Development Display [" << mCursorTime.secs() << "]";
    rcTextSegment text( rc2Fvector(0.0f, 0.0f), os.str());
    texts.push_back( text );
}

// Get position in model coordinates
bool rcMonitor::getPosition( rcTimestamp cursorTime, rcTrack* track,
                             rcFPair& pos, rcRect& rect ) const
{
    bool valid = false;
    
    rcPositionTrack* pTrack = dynamic_cast<rcPositionTrack*>( track );
    if ( pTrack == 0 )
        return valid;

    if ( cursorTime != cCursorTimeCurrent ) {
        rcPositionSegmentIterator* iter = pTrack->getDataSegmentIterator( cursorTime );
        if ( iter->contains( cursorTime ) ) {
            // Current segment contains cursorTime
	  // Get focus rect position in the frame
	  rcRect fr = mCanvas->focusRect ();
            pos = iter->getValue();
	    pos.x() = pos.x() + fr.ul().x();
	    pos.y() = pos.y() + fr.ul().y();
            rect = iter->getFocus();
            valid = true;
        }
        delete iter;
    }

    return valid;
}

// Add graphics to collection
bool rcMonitor::addGraphics( rcTimestamp cursorTime, rcTrack* track,
                             rcVisualGraphicsCollectionCollection& globalGraphics ) const
{
    bool valid = false;

    rcGraphicsTrack* graphicsTrack = dynamic_cast<rcGraphicsTrack*>( track );
    if ( graphicsTrack == 0 )
        return valid;

    if ( cursorTime != cCursorTimeCurrent ) {
        rcGraphicsSegmentIterator* iter = graphicsTrack->getDataSegmentIterator( cursorTime );
        if ( iter->contains( cursorTime ) ) {
            // Current segment contains cursorTime
            globalGraphics.push_back( iter->getValue() );
            valid = true;
        }
        delete iter;
    }

    return valid;
}

// Build cell path graphics
void rcMonitor::buildPositionPath( const rcTimestamp& cursorTime, rcTrack* track,
                                   rcVisualGraphicsCollection& graphics ) const
{
    rcPositionTrack* posTrack = dynamic_cast<rcPositionTrack*>( track );
    if ( posTrack == 0 )
        return;

    if ( cursorTime != cCursorTimeCurrent ) {
        rcPositionSegmentIterator* iter = posTrack->getDataSegmentIterator( cZeroTime );
        const uint32 segmentCount = iter->getSegmentCount();

        if ( segmentCount > 0 ) {
            rcFPair prevPos( 0, 0 );
        
            // Lock iterator for the total duration of value access
            iter->lock();
            // Gather points in point array
            vector<rcFPair> posPoints;
            posPoints.reserve( segmentCount );
        
            // Accumulate all values up to current time
            while ( iter->getSegmentStartAbsolute() <= cursorTime ) {
                const rcFPair pos = iter->getValue();
                // Add only unique points
                if ( pos != prevPos ) {
                    // Add one point to array
                    posPoints.push_back( pos );
                    prevPos = pos;
                }
            
                if ( ! iter->advance( 1, false ) ) // Unlocked advance for speed
                    break;
            }
            if ( !posPoints.empty() ) {
                // Add one more point if number of points is odd
                if ( posPoints.size() % 2 )
                    posPoints.push_back( posPoints.back() );
                // Build line strip collection
                rcVisualSegmentCollection& lines = graphics.segments();
                lines.reserve( lines.size() + posPoints.size()/2 );
                uint32 size = posPoints.size()-1;
                for ( uint32 i = 0; i < size; i +=2 ) {
                    const rcFPair& pos = posPoints[i];
                    const rcFPair& pos2 = posPoints[i+1];
                    rcVisualLineStrip line( rc2Fvector( pos.x(), pos.y() ),
                                            rc2Fvector( pos2.x(), pos2.y()) );
                    lines.push_back( line );
                }
            }
            iter->unlock();
        }
        delete iter;
    }
}

// Build a cross for cell position and add it to collection
void rcMonitor::buildPositionCross( const rcFPair& pos, rcVisualGraphicsCollection& graphics ) const
{
    // Get cross for each cell
    rcVisualSegmentCollection& segments = graphics.segments();

    rcVisualCross cross( rc2Fvector( pos.x(), pos.y() ),
                         rc2Fvector( cPositionCrossLength, cPositionCrossLength ) );
    segments.push_back( cross );
}

// Build an enclosing boundary for cell and add it to collection
void rcMonitor::buildCellBoundary( const rcRect& rect, rcVisualGraphicsCollection& graphics ) const
{
    // Get cross for each cell
    rcVisualSegmentCollection& segments = graphics.segments();

    rcVisualRect bounds( rect.x(), rect.y(), rect.width(), rect.height() );
    segments.push_back( bounds );
}

// Create a new QImage to display
bool rcMonitor::updateCachedImage( const rcWindow* image )
{
    bool wasAdded = false;

    if ( image ) {
        mSpeedCalc.addTime( rcTimestamp::now() );
        bool showSaturated = false;
        
        // Saturation display
        if ( mSaturationWidget->isEnabled() && mSaturationWidget->isChecked() ) {
            showSaturated = true;
        }
        // Gather displayble graphics and text
        rcVisualGraphicsCollectionCollection globalGraphics, cellGraphics;
        rcTextSegmentCollection textSegments;

        if ( showDevelopmentData() )
            gatherDevelopmentTrackData( globalGraphics, cellGraphics, textSegments );
        else
            gatherTrackData( globalGraphics, cellGraphics, textSegments );
        rcTextCollection texts( cTextStyle, textSegments );
        // Update OpenGL canvas
        mCanvas->loadrcWindow( *image, globalGraphics, cellGraphics, texts, showSaturated );
        wasAdded = true;
    }
    
#ifdef DEBUG_LOG    
    if ( wasAdded == false )
        cout << "updateCachedImage NOP" << endl;
#endif
    
    return wasAdded;
}

void rcMonitor::deleteCachedImage()
{
    // Delete all canvas data
    rcVisualGraphicsCollectionCollection lines;
    rcTextSegmentCollection textSegments;
    rcTextCollection texts( cTextStyle, textSegments );
    rcWindow empty;
    mCanvas->loadrcWindow( empty, lines, lines, texts,
                           mSaturationWidget->isEnabled() && mSaturationWidget->isChecked() );
}

void rcMonitor::displayScalarFocusArea( const rcRect& rect, rcTrackInfo& trackInfo )
{
#if 0        
    if ( rect.width() > 0 && rect.height() > 0 ) {
        QPainter paint( this );

        // Use the same pen the timeline graph uses
        paint.setPen( trackInfo.getPen() );
        // Focus area is in model coords, tranform it to device coords
        rcRect devRect = modelToDevCoord( rect );
        paint.drawRect( devRect.x(),
                        devRect.y(),
                        devRect.width(),
                        devRect.height() );
    }
#else
    // TODO: implement this
    rmUnused( rect );
    rmUnused( trackInfo );
#endif    
}

// Paint current focus area
rcRect rcMonitor::paintScalarTrackFocus( rcTimestamp cursorTime , rcTrack* track, rcTrackInfo& trackInfo )
{
    rcRect focusRect;
    
    rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );
    if ( scalarTrack == 0 )
        return focusRect;

    if ( cursorTime != cCursorTimeCurrent ) {
        rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( cursorTime );
        if ( iter->contains( cursorTime ) ) {
            focusRect = iter->getFocus();
            displayScalarFocusArea( focusRect, trackInfo );
        }
        delete iter;
    }

    return focusRect;
}

// Paint current image from video track
void rcMonitor::paintVideoTrackFocus( rcTimestamp cursorTime , rcTrack* track )
{
    rcVideoTrack* videoTrack = dynamic_cast<rcVideoTrack*>( track );
    if ( videoTrack == 0 )
        return;

    if ( cursorTime != cCursorTimeCurrent ) {
        if ( videoTrack->getSegmentCount() > 0 ) {
            rcVideoSegmentIterator* iter = videoTrack->getDataSegmentIterator( cursorTime );
            rcWindow image = iter->getValue();
            updateCachedImage( &image );
            delete iter;
        }
    }
}

// Return bounding rect of image
QRect rcMonitor::imageRect() const
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    uint32 maxX = domain->getExperimentAttributes().frameWidth;
    uint32 maxY = domain->getExperimentAttributes().frameHeight;
    
    return QRect( 0, 0, maxX, maxY );
}

// Update image display with a cached frame
// displayTrackFocus of scalar tracks will cause flicker
void rcMonitor::internalUpdateDisplay( void )
{
    if ( !inputIsCamera() ) {
        // This method updates cached image if appropriate
        displayTrackFocus( eVideoTrack );

//        displayTrackFocus( eScalarTrack );

    }
    
    updateStatus();
}


// Check if video data source is a camera
bool rcMonitor::inputIsCamera() const
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    return domain->getExperimentAttributes().liveInput;
}

// Check if input is being saved in real-time
bool rcMonitor::inputIsStored() const
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    return domain->getExperimentAttributes().liveStorage;
}

// Show development image/graphics data instead of main data
bool rcMonitor::showDevelopmentData() const
{
    return mDevelopmentData;
}
