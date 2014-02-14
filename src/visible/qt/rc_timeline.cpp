#include <qpixmap.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>

#include <rc_window.h>

#include "rc_modifierkeys.h"
#include "rc_modeldomain.h"
#include "rc_trackmanager.h"
#include "rc_trackrender.h"
#include "rc_timeline.h"

// Display policy constants
static const bool cUsingDoubleBuffering = true;

// Color constants
static const QColor cOffEndColor( 180 , 200 , 180 );
static const QColor cBackgroundColor( 220 , 240 , 220 );
static const QColor cCursorColor( 20 , 20 , 200 );

// Zoom/scale constants
static const double cZoomYSensitivity = 0.25;
static const double cUserScaleYSensitivity = 0.01;
static const double cUserScaleMin = 0.0000001;
static const double cMinSecondsPerPixel = 0.001;
static const double cDefaultSecondsPerPixel = 0.02;

static const double cScalarScaleMin = 0.0;
static const double cScalarScaleMax = 1.0;

// Graph panel layout constants
static const int cLeftMargin = 6;
static const int cRightMargin = 66;
static const int cScaleTickWidth = 6;
static const int cSegmentTickHeight = 6;

// User-selectable scale modes

static rcResultScaleMode nextScale( rcResultScaleMode cur )
{
    switch ( cur ) {
        case eResultScaleCurrentMinToMax:
            return eResultScaleCurrentZeroToMax;
        case eResultScaleCurrentZeroToMax:
            return eResultScaleExpectedMinToMax;
        case eResultScaleUserMinToMax:
            return eResultScaleExpectedMinToMax;
        case eResultScaleExpectedMinToMax:
            // Wrap around
            return eResultScaleCurrentMinToMax;
        default:
            cerr << "Unexpected scale mode " << cur << endl;
            rmAssert( 0 );
    }
}

static rcResultScaleMode prevScale( rcResultScaleMode cur )
{
    switch ( cur ) {
        case eResultScaleCurrentMinToMax:
            // Wrap around
            return eResultScaleExpectedMinToMax;
        case eResultScaleCurrentZeroToMax:
            return eResultScaleCurrentMinToMax;
        case eResultScaleExpectedMinToMax:
            return eResultScaleCurrentZeroToMax;
        case eResultScaleUserMinToMax:
            return eResultScaleCurrentZeroToMax;
        default:
            cerr << "Unexpected scale mode " << cur << endl;
            rmAssert( 0 );
    }
}

#ifdef DEBUG_LOG
static void displayScale( rcResultScaleMode cur )
{
    switch ( cur ) {
        case eResultScaleCurrentMinToMax:
            cerr << "curMin - curMax" << endl;
            break;
        case eResultScaleCurrentZeroToMax:
            cerr << "0.0 - curMax" << endl;
            break;
        case eResultScaleExpectedMinToMax:
            cerr << "expectedMin - expectedMax" << endl;
            break;
        case eResultScaleCompassCurrentMinToMax:
            cerr << "expectedMin - expectedMax" << endl;
            break;
        case eResultScaleCompassExpectedMinToMax:
            cerr << "compass expectedMin - expectedMax" << endl;
            break;
        case eResultScaleUserMinToMax:
            cerr << "userMin - userMax" << endl;
            break;
        case eResultScaleSentinel:
            cerr << "sentinel" << endl;
            break;
        default:
            cerr << "unknown mode " << cur << endl;
            break;
    }
}
#endif // DEBUG_LOG

// Scale defaults
const rcResultScaleMode cDefaultScaleMode = eResultScaleExpectedMinToMax;

rcTimeline::rcTimeline( QWidget* parent, const char* name, Qt::WFlags f ) :
        QWidget( parent, name ), _fps( 15 )
{
    rcUNUSED( f );
    // Do not erase background at all, we always fill it with our own color
    setAttribute (Qt::WA_NoSystemBackground );

    setMinimumSize( 224, 162 );
    // Gets (keyboard) focus by mouse clicking
    setFocusPolicy( Qt::ClickFocus);
    
    _currentTime = cZeroTime;
    _cursorTime = cCursorTimeCurrent;
    _lastMouseDown = QPoint(0,0);
    _cursorPen = QPen( cCursorColor );
    _cursorPen.setWidth (1);
    resetAllScales();
    
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( elapsedTime( const rcTimestamp& ) ) ,
			 this   , SLOT( updateTime( const rcTimestamp& ) ) );
    connect( domain , SIGNAL( cursorTime( const rcTimestamp& ) ) ,
             this   , SLOT( updateCursorTime( const rcTimestamp& ) ) );
	connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
			 this   , SLOT( updateState( rcExperimentState ) ) );
    connect( domain , SIGNAL( updateTimelineRange( const rcTimestamp&, const rcTimestamp& ) ) ,
             this   , SLOT( updateRange( const rcTimestamp&, const rcTimestamp& ) ) );
    connect( domain , SIGNAL( updateTimelineScale( rcResultScaleMode ) ) ,
             this   , SLOT( updateScale( rcResultScaleMode ) ) );
    connect( domain , SIGNAL( updateDisplay( void ) ) ,
             this   , SLOT( updateDisplay( void ) ) );
    connect( domain , SIGNAL( updateSettings( void ) ) ,
             this   , SLOT( updateSettings( void ) ) );
}

rcTimeline::~rcTimeline()
{
}

// public slots:
void rcTimeline::updateTime( const rcTimestamp& time )
{
//    cerr << "rcTimeline::updateTime" << endl;
	_currentTime = time;
	if (_cursorTime == cCursorTimeCurrent) {
		rcTimestamp rangeLength = _rangeEnd - _rangeStart;
		_rangeEnd = _currentTime;
		_rangeStart = _rangeEnd - rangeLength;
	} 

    updateDisplay();
}

void rcTimeline::updateCursorTime( const rcTimestamp& time )
{
//    cerr << "rcTimeline::updateCursorTime" << endl;
    _cursorTime = time;
#if 0
    // Range sanity checks
    if (_cursorTime < cZeroTime)
        _cursorTime = cZeroTime;
    else if (_cursorTime > _rangeEnd)
        _cursorTime = _rangeEnd;
#endif
    updateDisplay();
}

void rcTimeline::updateState( rcExperimentState state )
{
//    cerr << "rcTimeline::updateState" << endl;
    rcModelDomain* domain = rcModelDomain::getModelDomain();

    switch ( state ) {
        case eExperimentEmpty:
            _cursorTime = cCursorTimeCurrent;
            domain->notifyCursorTime( _cursorTime );
            break;
            
        case eExperimentRunning:
        {
            if ( inputIsCamera() ) {
                _cursorTime = cCursorTimeCurrent;
                domain->notifyCursorTime( _cursorTime );
            }
        }
        break;
        
        case eExperimentEnded:
        {
            if ( inputIsCamera() ) {
                // set cursor time to current
                _cursorTime = cCursorTimeCurrent;
                domain->notifyCursorTime( _cursorTime );
            } 
        }
        break;

        default:
            break;
    }

    _fps.reset();
}

void rcTimeline::updateRange( const rcTimestamp& start, const rcTimestamp& end )
{
//    cerr << "rcTimeline::updateRange" << endl;
    rcTimestamp rangeLength = end - start;

    if ( rangeLength > cZeroTime ) {
        _rangeStart = start;
        _rangeEnd = end;
        //repaint( false );
        updateDisplay();
    }
}

void rcTimeline::updateScale( rcResultScaleMode scale )
{
//    cerr << "rcTimeline::updateScale" << endl;
    _currentScale = scale;

    updateDisplay();
}

void rcTimeline::updateDisplay( void )
{
//    cerr << "rcTimeline::updateDisplay" << endl;
    _fps.addTime( rcTimestamp::now() );
    update();
    //cerr << "rcTimeline " << _fps.fps() << " fps" << endl;
}

void rcTimeline::updateSettings( void )
{
//    cerr << "rcTimeline::updateSettings" << endl;
    
    // Make sure the live feed is always shown with camera input
    if ( inputIsCamera() ) {
        if ( _cursorTime != cCursorTimeCurrent ) {
            _cursorTime = cCursorTimeCurrent;
            rcModelDomain::getModelDomain()->notifyCursorTime( _cursorTime );
        }
    }
    updateDisplay();
}

// protected:
void rcTimeline::mousePressEvent( QMouseEvent* mouseEvent )
{
	_lastMouseDown = mouseEvent->pos();
    _lastRange = _rangeEnd - _rangeStart;
	mouseMoveEvent( mouseEvent );
}

void rcTimeline::mouseMoveEvent( QMouseEvent* mouseEvent )
{
    // When input is camera, only data scale operations are allowed
    if ( rcModifierKeys::isCtrlDown( mouseEvent ) )
    {
        double deltaY = _lastMouseDown.y() - mouseEvent->pos().y();

        if ( scaleClicked( mouseEvent->pos() ) || inputIsCamera() ) {
            // Change user scale min value
            double min, max;
            bool oneScaleForAll = checkScalarTrackScales( eResultScaleExpectedMinToMax, min, max );

            // Change user scale only if all enabled tracks have the same scale
            if ( oneScaleForAll ) {
                // User scale mode
                rcModelDomain::getModelDomain()->notifyTimelineScale( eResultScaleUserMinToMax );

                double newMin = _currentScalarScaleMin;
                
                deltaY *= cUserScaleYSensitivity * (_currentScalarScaleMax - newMin);
                if ( deltaY > 0.0 ) {
                    if ( newMin < cUserScaleMin )
                        newMin = cUserScaleMin;
                    else
                        newMin *= (1 + deltaY);
                }
                else if ( deltaY < 0.0 ) {
                    newMin *= 1.0 / (1.0 - deltaY);
                }

                // Don't go below expected min
                if ( newMin < min )
                    newMin = min;
                // Don't let the max become too close to min
                if ( newMin < _currentScalarScaleMax )
                    _currentScalarScaleMin = newMin;
                else
                    _currentScalarScaleMin = _currentScalarScaleMax - cUserScaleMin;
            }
        } else {
            // timeline "zoom" mode: modify range based on cursor y movements
            deltaY *= cZoomYSensitivity;
            double newRange = _lastRange.secs();
            if ( deltaY > 0.0 )
                newRange *= (1 + deltaY);
            else if ( deltaY < 0.0 )
                newRange *= 1.0 / (1.0 - deltaY);

            // broadcast new timeline range to everyone (including us).
            // position the new range around the cursor
            rcTimestamp newRangeStart = _cursorTime - (newRange / 2);
            rcTimestamp newRangeEnd = _cursorTime + (newRange / 2);
            rcModelDomain::getModelDomain()->notifyTimelineRange( newRangeStart , newRangeEnd );
        } 
    }
    else if ( rcModifierKeys::isShiftDown( mouseEvent ) )
    {
        if ( scaleClicked( mouseEvent->pos() ) || inputIsCamera() ) {
            // Change user scale max value
            double min, max;
            bool oneScaleForAll = checkScalarTrackScales( eResultScaleExpectedMinToMax, min, max );

            // Change user scale only if all enabled tracks have the same scale
            if ( oneScaleForAll ) {
                rcModelDomain::getModelDomain()->notifyTimelineScale( eResultScaleUserMinToMax );
                  
                // User scale mode
                double newMax = _currentScalarScaleMax;
                double deltaY = _lastMouseDown.y() - mouseEvent->pos().y();
                
                deltaY *= cUserScaleYSensitivity;
                if ( deltaY > 0.0 )
                    newMax *= (1 + deltaY);
                else if ( deltaY < 0.0 )
                    newMax *= 1.0 / (1.0 - deltaY);

                // Don't go above expected max
                if ( newMax > max )
                    newMax = max;
                // Don't let the max become too close to min
                if ( newMax > _currentScalarScaleMin + cUserScaleMin )
                    _currentScalarScaleMax = newMax;
            }
        } else {
            // timeline scroll mode
            rcTimestamp moveTime = pixelToTime( mouseEvent->pos().x() ) - _cursorTime;
            rcTimestamp newRangeStart = _rangeStart - moveTime;
            rcTimestamp newRangeEnd = _rangeEnd - moveTime;
            
            // update last y in case we enter zoom mode
            _lastMouseDown = mouseEvent->pos();

            // broadcast new timeline range to everyone (including us).
            rcModelDomain::getModelDomain()->notifyTimelineRange( newRangeStart , newRangeEnd );
        } 
    }
    else
    {
        if ( !scaleClicked( mouseEvent->pos() ) && !inputIsCamera() ) {
            // timeline "cursor" model
            rcTimestamp newCursorTime = pixelToTime( mouseEvent->pos().x() );
            if ( newCursorTime < cZeroTime )
                newCursorTime = cZeroTime;
            // update last y in case we enter zoom mode
            _lastMouseDown.setY( mouseEvent->pos().y() );
            
            // broadcast new cursor time to everyone (including us).
            rcModelDomain::getModelDomain()->notifyCursorTime( newCursorTime );
        } else {
            // User scale mode
        }
    }
}

void rcTimeline::mouseReleaseEvent( QMouseEvent* mouseEvent )
{
    // Ignore everything if camera is enabled
    if ( inputIsCamera() )
        return;
    
    if ( rcModifierKeys::isCtrlDown( mouseEvent ) || rcModifierKeys::isShiftDown( mouseEvent ) )
        return;

    if ( scaleClicked( mouseEvent->pos() ) ) 
	{
        rcModelDomain::getModelDomain()->notifyCursorTime( cCursorTimeCurrent );
	}
}

void rcTimeline::keyPressEvent( QKeyEvent* keyEvent )
{
     // Limited functionality if camera is enabled
    if ( inputIsCamera() ) {
        keyCameraEvent( keyEvent );
        return;
    }

    const bool ctrlDown = rcModifierKeys::isCtrlDown( keyEvent );
    const bool shiftDown = rcModifierKeys::isShiftDown( keyEvent );
    const bool altDown = rcModifierKeys::isAltDown( keyEvent );
    const int key = keyEvent->key();
    
    if ( ctrlDown && key == Qt::Key_C ) {
        // Copy to clipboard
        rcTrackManager* tmp = rcTrackManager::getTrackManager();
        tmp->copyTrackToClipboard( tmp->getHiliteTrack() );
    }

   


    if ( ctrlDown && shiftDown && altDown && key == Qt::Key_D ) {
        // Toggle development debug mode
        gDeveloperDebugging = !gDeveloperDebugging;
        rcModelDomain* domain = rcModelDomain::getModelDomain();
        if ( ! gDeveloperDebugging ) {
            rcExperiment* experiment = domain->getExperiment();
            int nPages = experiment->getNSettingCategories();
            
            for (int i = 0; i < nPages; i++) {
                rcSettingCategory category = experiment->getSettingCategory( i );
                
                // Look for cell visualization settings
                if ( !strcmp( category.getTag(), "visualization-settings" ) ) {
                    // Disable development display
                    const rcSettingInfo dev = category.getSettingInfo( "visualize-development-data" );
                    rcValue value ( false );
                    dev.setValue( value );
                }
            }
        }
        if ( gDeveloperDebugging )
            cerr << "Developer debugging mode enabled" << endl;
        else
            cerr << "Developer debugging mode disabled" << endl;
        domain->notifyUpdateDebugging();
    }
         
    if ( _cursorTime != cCursorTimeCurrent ) {
        if ( shiftDown ) 
            keyScrollEvent( keyEvent );
        else if ( ctrlDown )
            keyRangeScaleEvent( keyEvent );
        else 
            keyNavigateEvent( keyEvent );
        return;
    } else {
        // User scale control mode
        switch ( key ) {
            case Qt::Key_Up:
                keyEvent->accept();
                break;

            case Qt::Key_Down:
                keyEvent->accept();
                break;

            case Qt::Key_Home:
                // Reset scale max value to default
                _currentScalarScaleMax = cScalarScaleMax;
                updateDisplay();
                keyEvent->accept();
                break;

            case Qt::Key_End:
                // Reset scale min value to default
                _currentScalarScaleMin = cScalarScaleMin;
                updateDisplay();
                keyEvent->accept();
                break;

            case Qt::Key_Escape:
                // Reset everything
                resetAllScales();
                updateDisplay();
                keyEvent->accept();
                break;
           
            default:
                keyEvent->ignore();
                break;
        }
    }
}

void rcTimeline::keyReleaseEvent( QKeyEvent* keyEvent )
{
    keyEvent->ignore();
}

void rcTimeline::paintEvent( QPaintEvent* /*paintEvent*/ )
{
    QRect cr = rect();

	if ( cUsingDoubleBuffering )
	{
        // Reuse _pixmap for better speed, just resize it when necessary
        if ( cr.size() != _pixmap.size() )
            _pixmap.resize( cr.size() );

		QPainter p( &_pixmap );
		paintTimeline( p , cr );
		p.end();

		p.begin( this );
		p.drawPixmap( cr.topLeft(), _pixmap );
	}
	else
	{
		QPainter p( this );
		paintTimeline( p , cr );
	}
}

// private:

void rcTimeline::paintTimeline( QPainter& p , QRect& contentRect )
{
	QRect dataRect = contentRect;
	rcTimestamp startTime = _rangeStart;
	rcTimestamp rangeLength = _rangeEnd - _rangeStart;
    
	// if the current range starts before the beginning, fill with the
	//	off-end color
	if (_rangeStart < cZeroTime)
	{
		QRect offEndRect = contentRect;
		if (_rangeEnd.secs() > 0.0)
		{
			int offEndWidth = -int(_rangeStart.secs() / rangeLength.secs()) * contentRect.width();
			offEndRect.setWidth( offEndWidth );
		}
		p.fillRect( offEndRect , cOffEndColor );
		startTime = cZeroTime;
		dataRect.setLeft( dataRect.left() + offEndRect.width() );
	}
	
	// fill the normal background color
	QLinearGradient linearGrad(QPointF(0,0), QPointF(0,dataRect.height()));
	linearGrad.setColorAt(0, QColor (100, 100, 100));
	linearGrad.setColorAt(1, QColor (50, 50, 50));
	
	p.fillRect( dataRect , QBrush (linearGrad)); //cBackgroundColor );

	// draw each data track.
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	rcExperiment* experiment = domain->getExperiment();
    double min, max;
    bool oneScaleForAll = checkScalarTrackScales( _currentScale, min, max );
    bool scaleMarksPainted = false;
    int nTrackGroups = experiment->getNTrackGroups();

    rcWriterSemantics aciTrack = eWriterACI;
    rcWriterSemantics aciwTrack = eWriterACIWindow;

    for (int i = 0; i < nTrackGroups; i++)
    {
        rcTrackGroup* trackGroup = experiment->getTrackGroup( i );

        if ( hasViewableTracks( trackGroup ) ) {
            int nTracks = trackGroup->getNTracks();
            for (int j = 0; j < nTracks; j++)
            {
                rcTrack* track = trackGroup->getTrack( j );

		const int32 logdisp = rcWriterManager::tagType( track->getTag() ) == aciTrack || 
		  rcWriterManager::tagType( track->getTag() ) == aciwTrack;

                switch( track->getTrackType() )
                {
                    case eScalarTrack:		// track contains scalar data
                    {
		      rcTrackRenderInfo tri( i , j, track, startTime , _rangeEnd , dataRect, logdisp);

		      if (!tri._isEnabled)
			break;
		      getScalarTrackMinMax( _currentScale, tri );
                        paintScalarTrack( p , tri );
                        
                        // paint scale markers if a track is highlighted
                        // or if all the tracks have the same scale
                        if ( tri._isHilited || oneScaleForAll ) {
                            if ( !scaleMarksPainted ) {
                                paintScalarScaleMarks( p, tri );
                                // Draw scale marks only once
                                scaleMarksPainted = true;
                            }
                        }
                    }
                    break;
                    
                    // Use video track to paint frame ticks    
                    case eVideoTrack:
                    {
                        rcTrackRenderInfo tri( i , j, track, startTime , _rangeEnd , dataRect );   
                        paintFrameTicks( p , tri );
                    }
                    break;
                    
                    default:
                        break;
                }
            }
        }
    }

	// paint cursor
    paintCursor( p );
}

void rcTimeline::paintScalarTrack( QPainter& p , rcTrackRenderInfo& tri )
{
	rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( tri._track );
	if (scalarTrack == 0)
		return;

	rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( tri._startTime );
    rcTimestamp curPos = iter->getSegmentStartAbsolute();
    
#ifdef DEBUG_LOG    
    {
        cerr << "timeline startTime " << tri._startTime.secs();
        cerr << " endTime " << tri._endTime.secs() << endl;
        cerr << "iterStartTime " << curPos.secs() << endl;

        rcModelDomain* domain = rcModelDomain::getModelDomain();
        rcTimestamp startTime = domain->getExperimentStart();
        rcTimestamp elapsedTime = domain->getExperimentLength();
        cerr << "exp startTime " << startTime.secs() << endl;
        cerr << "exp elapsedTime " << elapsedTime.secs() << endl;
        cerr << *iter;
    }
#endif

    p.setPen( tri._dataPen );

    int x = 0, y = 0;
    int32 timeOffset = getTimeOffset();
    int32 valueOffset = getValueOffset();
    double timeScale = getTimeScale();
    double valueScale = getValueScale( tri );
    bool nextSegment = true;
    
    if ( iter->getSegmentIndex() == -1 ) {
        // No valid value at curPos, advance one
        nextSegment = iter->advance( 1 );
        curPos = iter->getSegmentStartAbsolute();
    }
    
    x = timeToPixel( curPos, timeScale, timeOffset );
    if ( x < 0 )
        x = 0;

    double val = (tri._logDisplay) ? log10 (iter->getValue ()) : iter->getValue ();

    y = valueToPixel(val, tri, valueScale, valueOffset );
    int prevX = x, prevY = y;

    // Lock iterator for the total duration of value access
    iter->lock();

    
    //Draws a line from (x1, y1) to (x2, y2) and sets the current pen position to (x2, y2).
    while ( nextSegment )
    {
        // Get next segment
        nextSegment = iter->advance( 1, false ); // Unlocked advance for speed
        if ( nextSegment ) {
            rcTimestamp segStart = iter->getSegmentStartAbsolute();
            
            if ( segStart > tri._endTime ) 
                break;
            if ( segStart < tri._startTime )
                continue;
            
            x = timeToPixel( segStart, timeScale, timeOffset );

	    double val = (tri._logDisplay) ? log10 (iter->getValue ()) : iter->getValue ();
            y = valueToPixel( val, tri, valueScale, valueOffset );
            // Optimization: draw point only if it differs from the previously drawn point
            if ( x != prevX || y != prevY )
            {
                p.drawLine( prevX, prevY, x , y );
                prevX = x;
                prevY = y;
            } 
        }
    }
    iter->unlock();
	delete iter;
}

// Paint tick marks for each video frame
void rcTimeline::paintFrameTicks( QPainter& p , rcTrackRenderInfo& tri )
{
    rcVideoTrack* videoTrack = dynamic_cast<rcVideoTrack*>( tri._track );
    if (videoTrack == 0)
        return;
    // Use new pen
    p.setPen( tri._tickMarkPen );

    int prevX = -1;
    const int32 offset = getTimeOffset();
    const double scale = getTimeScale();
    
    rcVideoSegmentIterator* iter = videoTrack->getDataSegmentIterator( tri._startTime );
    rcTimestamp segmentStart = iter->getSegmentStart();
    // Draw first tick only if it's exactly at range start
    if ( segmentStart == tri._startTime ) {
        const int x = timeToPixel( segmentStart, scale, offset );
        paintSegmentTick( p, x, tri );
        prevX = x;
    }

    bool nextSegment = true;
    // Lock iterator for the total duration of value access
    iter->lock();
    
    while ( nextSegment )
    {
        // Get next segment
        nextSegment = iter->advance( 1, false ); // Unlocked advance for speed
        if ( nextSegment ) {
            rcTimestamp segmentStart = iter->getSegmentStart();
            // If segment starts after range end, bail out
            if ( segmentStart > tri._endTime )
                break;
            const int x = timeToPixel( segmentStart, scale, offset );
            // Optimization: draw new tick only if it's different from previous tick
            if ( x != prevX ) {
                paintSegmentTick( p, x, tri );
                prevX = x;
            } 
        }
    }
    iter->unlock();
    delete iter;

    // Restore old pen
	p.setPen( tri._dataPen );
}

// Get scaling factor for timeToPixel conversion
double rcTimeline::getTimeScale() const
{
    const double totalTime = (_rangeEnd - _rangeStart).secs();
    const double width = rect().width() - cLeftMargin - cRightMargin;

    return width / totalTime;
}

// Get start offset for timeToPixel conversion
int32 rcTimeline::getTimeOffset() const
{
    return rect().left() + cLeftMargin;
}

// Slightly optimized, pass in invariants as arguments
int32 rcTimeline::timeToPixel( const rcTimestamp& currentTime, const double& scale,
                                 const int32& offset ) const
{
    const double offsetTime = (currentTime - _rangeStart).secs();
   
    return (int) (offset + scale * offsetTime);
}

// Convert time to X pixel position
int32 rcTimeline::timeToPixel( const rcTimestamp& currentTime ) const
{
    const int32 offset = getTimeOffset();
    const double scale = getTimeScale();
    
    return timeToPixel( currentTime, scale, offset );
}

// Get scaling factor for valueToPixel conversion
double rcTimeline::getValueScale( const rcTrackRenderInfo& tri ) const
{
    QRect cr = rect();
    double minY = cr.bottom() - cSegmentTickHeight;
	double maxY = cr.top() + cSegmentTickHeight;
    
    return (maxY - minY)/(tri._maxValue - tri._minValue);
}

// Get start offset for valueToPixel conversion
int32 rcTimeline::getValueOffset() const
{
    return rect().bottom() - cSegmentTickHeight;
}

// Convert value to pixel Y position
int32 rcTimeline::valueToPixel( const double& value, const rcTrackRenderInfo& tri ) const
{
    int32 offset = getValueOffset();
    double scale = getValueScale( tri );
    
	return valueToPixel( value, tri, scale, offset );
}

// Convert value to pixel Y position
int32 rcTimeline::valueToPixel( const double& v, const rcTrackRenderInfo& tri,
                                  const double& scale, const int32& offset ) const
{
    double minValue = tri._minValue;
	double maxValue = tri._maxValue;
    
    double value = (v < minValue) ? minValue : v;
	if ( value > maxValue )
		value = maxValue;
    
    return int(offset + scale * (value - minValue));
}

rcTimestamp rcTimeline::pixelToTime( int xPos ) const
{
    QRect cr = reducedContentsRect();
	double range = _rangeEnd.secs() - _rangeStart.secs();
	double frac = (double) (xPos - cr.x()) / (double) cr.width();
	double secs = _rangeStart.secs() + (frac * range);
    rcTimestamp time = rcTimestamp( secs );

	return time;
}

// Paint tick mark for each segment
void rcTimeline::paintSegmentTick( QPainter& p,
                                   int x,
                                   rcTrackRenderInfo& tri )
{
    int minY = tri._dataRect.bottom();
    int maxY = tri._dataRect.top();
    
    // Bottom tick
    int tickY = minY - cSegmentTickHeight;
    if ( tickY < maxY )
        tickY = maxY;

    p.drawLine( x, minY, x, tickY );

    // Top tick
    tickY = maxY + cSegmentTickHeight;
    if ( tickY > minY )
        tickY = minY;

    p.drawLine( x, maxY, x, tickY );
}

// Paint one vertical scale mark (tick and numeric value)
void rcTimeline::paintScalarScaleMark( QPainter& p, const char* formatString, double value, 
                                       int tickX, int tickY, int textX, int textY )
{
    char valueBuf[ 128 ];

    p.drawLine( tickX, tickY, tickX - cScaleTickWidth, tickY );
    snprintf( valueBuf , rmDim(valueBuf), formatString , value );
    p.drawText( textX, textY, QString( valueBuf ) );
}

// Paint vertical scale marks
void rcTimeline::paintScalarScaleMarks( QPainter& p,
                                        rcTrackRenderInfo& tri )
{
    rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( tri._track );
    if (scalarTrack == 0)
        return;
    
    double minValue = tri._minValue;
    double maxValue = tri._maxValue;
    if ( minValue == maxValue )
        return;
    
    double incr  = (maxValue - minValue) / 10.0;
    // If incr <= 0, we'll loop indefinitely.
    if ( incr <= 0.0 )
        return;

    // Save current painter state
    p.save();
    // Draw in the margin
    QRect cr = rect();
    QRect dr = reducedContentsRect();
    int x = cr.right();
    // Use new pen
    p.setPen( tri._tickMarkPen );
    p.setFont( tri._tickMarkFont );

    // Draw full-width ticks and numbers
    double value;
    const char* formatString = tri._track->getDisplayFormatString();
    int32 valueOffset = getValueOffset();
    double valueScale = getValueScale( tri );
    
    // Draw min tick
    int y = valueToPixel( minValue, tri, valueScale, valueOffset );
    int textX = dr.right() + 4;
    paintScalarScaleMark( p, formatString, minValue, x, y, textX, y + 4 );
    // Draw max tick
    y = valueToPixel( maxValue, tri, valueScale, valueOffset );
    paintScalarScaleMark( p, formatString, maxValue, x, y, textX, y + 4 );
    // Draw remaining ticks
    for ( value = minValue + incr; value < maxValue; value += incr ) {
        y = valueToPixel( value, tri, valueScale, valueOffset );
        paintScalarScaleMark( p, formatString, value, x, y, textX, y + 4 );
    }
    
    // Draw half-width ticks
    for ( value = minValue + incr/2; value < maxValue; value += incr ) {
        int y = valueToPixel( value, tri, valueScale, valueOffset );
        p.drawLine( x, y, x - cScaleTickWidth/2, y );
    }
    
    // Restore painter state
    p.restore();
}

// Return a rect that leaves room for right scale markers
QRect rcTimeline::reducedContentsRect() const
{
    QRect cr = rect();
    cr.setLeft( cr.left() + cLeftMargin );
    cr.setRight( cr.right() - cRightMargin );
    cr.setTop(cr.top() + cSegmentTickHeight);
    cr.setBottom(cr.bottom() - cSegmentTickHeight);
    return cr;
}

// Paint vertical time cursor
void rcTimeline::paintCursor( QPainter& p )
{
    // paint cursor
    if (_cursorTime != cCursorTimeCurrent)
    {
        int cursorX = timeToPixel( _cursorTime );
        QRect cr = rect();
        p.setPen( _cursorPen );
        p.drawLine( cursorX-1, cr.top(), cursorX-1 , cr.bottom() );
        p.drawLine( cursorX+1 , cr.top() , cursorX+1, cr.bottom() );

    }
}

// Return currently enabled video track
rcVideoTrack* rcTimeline::getEnabledVideoTrack() const
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperiment* experiment = domain->getExperiment();
    
    int nTrackGroups = experiment->getNTrackGroups();
    for (int i = 0; i < nTrackGroups; i++)
    {
        rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
        int nTracks = trackGroup->getNTracks();
        for (int j = 0; j < nTracks; j++)
        {
            rcTrack* track = trackGroup->getTrack( j );
            switch( track->getTrackType() )
            {
                // Return the first video track
                // TODO: verify that the first track is the base "raw video" track
                case eVideoTrack:
                    return dynamic_cast<rcVideoTrack*>( track );

                default:
                    break;
            }
        }
    }

    return 0;
}

// Return currently enabled track
rcScalarTrack* rcTimeline::getEnabledScalarTrack() const
{
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	rcExperiment* experiment = domain->getExperiment();
    rcScalarTrack* enabledTrack = 0;
    
    int nTrackGroups = experiment->getNTrackGroups();
    for ( int i = 0; i < nTrackGroups; i++ )
    {
        rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
        int nTracks = trackGroup->getNTracks();
        for ( int j = 0; j < nTracks; j++ )
        {
            rcTrack* track = trackGroup->getTrack( j );
          
            switch( track->getTrackType() )
            {
                case eScalarTrack:
                {
                    rcTrackRenderInfo tri( i, j, track, cZeroTime, cZeroTime, QRect() );
                    if ( tri._isEnabled ) {
                        if ( tri._isHilited ) {
                            // First highlighted tracks trumps all, bail out
                            return dynamic_cast<rcScalarTrack*>( track );
                        }
                        // Pick any enabled track
                        enabledTrack = dynamic_cast<rcScalarTrack*>( track );
                    }
                }
                break;
                
                default:
                    break;
            }
        }
    }
    
    return enabledTrack;
}
                    
// Return timestamp offset to next/previous video frame
rcTimestamp rcTimeline::getFrameTime( const rcTimestamp& current, rcTimelineNavigationCmd cmd ) const
{
    rcSegmentIterator* iter = NULL;
    rcTimestamp newTime = current;
    
    rcVideoTrack* videoTrack = getEnabledVideoTrack();
    if ( videoTrack ) {
        // Use video track for navigation
        iter = videoTrack->getDataSegmentIterator( current );
    } else {
        // No video track available, navigate using a scalar track
        rcScalarTrack* scalarTrack = getEnabledScalarTrack();
        if ( scalarTrack )
            iter = scalarTrack->getDataSegmentIterator( current );
    }
    
    if ( !iter )
        return current;

    switch ( cmd ) {
        case eTimelineNextFrame:
            // Go forward one frame
            if ( iter->advance( 1 ) )
                newTime = iter->getTrackOffset();
            break;

        case eTimelinePrevFrame:
            // Go back one frame
            if ( iter->advance( -1 ) )
                newTime = iter->getTrackOffset();
            break;
            
        case eTimelineThisFrame:
        default:
            break;
    }
    delete iter;
    
    // Throttle value
    if ( newTime > _currentTime )
        newTime = _currentTime;
    if ( newTime < cZeroTime )
        newTime = cZeroTime;
    
    return newTime;
}

// Return the length of video segment at timestamp
rcTimestamp rcTimeline::getSegmentLength( const rcTimestamp& current ) const
{
    rcTimestamp stamp = current;
    rcSegmentIterator* iter = NULL;
    
    rcVideoTrack* videoTrack = getEnabledVideoTrack();
    if ( videoTrack ) {
        // Use video track for navigation
        iter = videoTrack->getDataSegmentIterator( current );
    } else {
        // No video track available, navigate using a scalar track
        rcScalarTrack* scalarTrack = getEnabledScalarTrack();
        if ( scalarTrack )
            iter = scalarTrack->getDataSegmentIterator( current );
    }
    
    if ( iter ) {
        stamp = iter->getSegmentLength();
        delete iter;
    }
    
    return stamp;
}

// Scroll range if possible
void rcTimeline::scrollRange( const rcTimestamp& start, const rcTimestamp& end )
{
    const rcTimestamp rangeLength = end - start;

    if ( rangeLength > cZeroTime ) {
        const rcTimestamp oldRangeLength = _rangeEnd - _rangeStart;
        
        // Don't let the range length decrease, this is a scroll operation
        // not a scaling operation
        if ( oldRangeLength <= rangeLength ) {
            const rcTimestamp& maxTime = _currentTime;
            rcTimestamp s = start;
            rcTimestamp e = end;
            
            if ( s < cZeroTime )
                s = cZeroTime;
            if ( e > maxTime )
                e = maxTime;
            
            rcModelDomain* domain = rcModelDomain::getModelDomain();
            domain->notifyTimelineRange( s, e );
        }
    }
}

// Scroll event handler
void rcTimeline::keyScrollEvent( QKeyEvent* keyEvent )
{
    // timeline scroll mode
    int key = keyEvent->key();
    
    switch ( key ) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        {
            rcTimestamp newTime = getSegmentLength( _cursorTime );
            if ( key == Qt::Key_Left )
                newTime = -newTime;
            rcTimestamp newRangeStart = _rangeStart + newTime;
            rcTimestamp newRangeEnd = _rangeEnd + newTime;
            scrollRange( newRangeStart, newRangeEnd );
            keyEvent->accept();
            return;
        }
        default:
            break;
    }
    keyEvent->ignore();
}

// Frame navigation event handler
void rcTimeline::keyNavigateEvent( QKeyEvent* keyEvent )
{
    // Frame-by-frame navigation mode
    rcTimelineNavigationCmd cmd = eTimelineNextFrame;
    int key = keyEvent->key();
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    
    switch ( key ) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Escape:
            keyCameraEvent( keyEvent );
            break;

        case Qt::Key_Home:
        {
            // Go to first frame
            rcTimestamp newTime = cZeroTime;
            if ( _rangeStart > newTime ) {
                // First frame not visible, need to scroll
                rcTimestamp rangeLen = _rangeEnd - _rangeStart;
                scrollRange( newTime, rangeLen );
            }
            domain->notifyCursorTime( newTime );
            keyEvent->accept();
        }
        break;

        case Qt::Key_End:
        {
            // Go to last frame
            rcTimestamp newTime = domain->getExperimentLength();
            
            if ( _rangeEnd < newTime ) {
                // First frame not visible, need to scroll
                rcTimestamp rangeLen = _rangeEnd - _rangeStart;
                scrollRange( newTime-rangeLen, newTime );
            }
            domain->notifyCursorTime( newTime );
            keyEvent->accept();
        }
        break;
            
        case Qt::Key_Left:
            cmd = eTimelinePrevFrame;
            // Warning, falling through on purpose
        case Qt::Key_Right:
        {
            rcTimestamp newTime = getFrameTime( _cursorTime, cmd );
            rcTimestamp rangeLen = _rangeEnd - _rangeStart;

            // Automatically scroll to keep cursor visible
            if ( newTime < _rangeStart ) {
                rcTimestamp newRangeStart = newTime;
                rcTimestamp newRangeEnd = newRangeStart + rangeLen;
                scrollRange( newRangeStart, newRangeEnd );
            } else if ( newTime > _rangeEnd ) {
                rcTimestamp newRangeEnd = newTime;
                rcTimestamp newRangeStart = newRangeEnd - rangeLen;
                scrollRange( newRangeStart, newRangeEnd );
            }
            domain->notifyCursorTime( newTime );
            keyEvent->accept();
        }
        break;
                
        default:
            keyEvent->ignore();
            break;
    }
}

// Range scaling event handler
void rcTimeline::keyRangeScaleEvent( QKeyEvent* keyEvent )
{
    int key = keyEvent->key();
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    double deltaY = 1;

    switch ( key ) {
        case Qt::Key_Home:
            // Scale to fit
            domain->notifyTimelineRange( cZeroTime , domain->getExperimentLength() );
            keyEvent->accept();
            break;
        case Qt::Key_Left:
        case Qt::Key_Down:
            deltaY = -1;
            // Warning, falling through on purpose
        case Qt::Key_Right:
        case Qt::Key_Up:
        {
            _lastRange = _rangeEnd - _rangeStart;
            deltaY *= cZoomYSensitivity;
            double newRange = _lastRange.secs();
            if (deltaY > 0)
                newRange *= (1 + deltaY);
            else if (deltaY < 0)
                newRange *= 1.0 / (1.0 - deltaY);

            // broadcast new timeline range to everyone (including us).
            // position the new range around the cursor
            rcTimestamp newRangeStart = _cursorTime - (newRange / 2);
            rcTimestamp newRangeEnd = _cursorTime + (newRange / 2);
            domain->notifyTimelineRange( newRangeStart , newRangeEnd );
            keyEvent->accept();
        }
        default:
            keyEvent->ignore();
            break;
    }
}

// Key event handler during camera (pre)view
void rcTimeline::keyCameraEvent( QKeyEvent* keyEvent )
{
    int key = keyEvent->key();
    rcModelDomain* domain = rcModelDomain::getModelDomain();

    // Only a limited set of actions is allowed with camera input
    switch ( key ) {
        case Qt::Key_Up:
            // Scale up
            domain->notifyTimelineScale( prevScale( _currentScale ) );
            keyEvent->accept();
            break;

        case Qt::Key_Down:
            // Scale down
            domain->notifyTimelineScale( nextScale( _currentScale ) );
            keyEvent->accept();
            break;

        case Qt::Key_Escape:
            // Reset everything
            resetAllScales();
            updateDisplay();
            keyEvent->accept();
            break;
            
        default:
            keyEvent->ignore();
            break;
    }
}

// Return true if all enabled tracks have the same scale.
// Assign the common minimum and maximum values to min and max.
bool rcTimeline::checkScalarTrackScales( rcResultScaleMode scaleMode, double& min, double& max )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
	rcExperiment* experiment = domain->getExperiment();
    bool firstTrack = true;
    bool oneScaleForAll = true;
    
    int nTrackGroups = experiment->getNTrackGroups();
    for (int i = 0; i < nTrackGroups; i++)
    {
        rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
	const int32 semantic = trackGroup->getType();        

        if ( hasViewableTracks( trackGroup ) ) {
            int nTracks = trackGroup->getNTracks();
            for (int j = 0; j < nTracks; j++)
            {
                rcTrack* track = trackGroup->getTrack( j );
                QRect dataRect; // Just a dummy
                
                switch( track->getTrackType() )
                {
                    case eScalarTrack:		// track contains scalar data
                    {
		      rcTrackRenderInfo tri( i, j, track, _rangeStart, _rangeEnd, dataRect, semantic == eGroupSemanticsGlobalMeasurements );
                        if (!tri._isEnabled)
                            break;
                        getScalarTrackMinMax( scaleMode, tri );
                        if ( firstTrack ) {
                            min = tri._minValue;
                            max = tri._maxValue;
                            firstTrack = false;
                        } else {
                            if ( min != tri._minValue || max != tri._maxValue ) {
                                oneScaleForAll = false;
                                break;
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

    return oneScaleForAll;
}

// Assign track min/max values to tri
void rcTimeline::getScalarTrackMinMax( rcResultScaleMode scaleMode, rcTrackRenderInfo& tri )
{
	rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( tri._track );
	if (scalarTrack == 0)
		return;

    rmAssert( scaleMode >= 0 );
    
    switch ( scaleMode ) {
        case eResultScaleCurrentMinToMax:
            tri._minValue = scalarTrack->getCurrentMinimumValue();
            tri._maxValue = scalarTrack->getCurrentMaximumValue();
            if ( tri._maxValue <= tri._minValue )
                tri._maxValue = scalarTrack->getExpectedMaximumValue();
            break;

        case eResultScaleCurrentZeroToMax:
	  tri._minValue = 1.e-15;
            tri._maxValue = scalarTrack->getCurrentMaximumValue();
            if ( tri._maxValue <= tri._minValue )
                tri._maxValue = scalarTrack->getExpectedMaximumValue();
            break;
            
        case eResultScaleExpectedMinToMax:
            tri._minValue = scalarTrack->getExpectedMinimumValue();
            tri._maxValue = scalarTrack->getExpectedMaximumValue();
            break;

        case eResultScaleUserMinToMax:
        case eResultScaleCompassExpectedMinToMax:
        case eResultScaleCompassCurrentMinToMax:
            tri._minValue = _currentScalarScaleMin;
            tri._maxValue = _currentScalarScaleMax;
            break;
            
        case eResultScaleSentinel:
            rmAssert( 0 );
            break;
    }

    if (tri._logDisplay)
      {
	tri._minValue = log10 (tri._minValue);
	tri._maxValue = log10 (tri._maxValue);
      }    
}

// Check if point is over scalar scale
bool rcTimeline::scaleClicked( const QPoint& point )
{
 	QRect cr = rect();

	if ( point.x() > (cr.right() - cRightMargin) )
        return true;
    else
        return false;
}

// Reset all scales and ranges to default values
void rcTimeline::resetAllScales()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    
    _currentScalarScaleMin = cScalarScaleMin;
    _currentScalarScaleMax = cScalarScaleMax;
    _rangeStart = cZeroTime;
    _rangeEnd = domain->getExperimentLength();
    _currentScale = eResultScaleExpectedMinToMax;
    _currentScale = eResultScaleCurrentMinToMax;
}

// Check if video data source is a camera
bool rcTimeline::inputIsCamera() const
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    return domain->getExperimentAttributes().liveInput;
}

// Check if input is being saved in real-time
bool rcTimeline::inputIsStored() const
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    return domain->getExperimentAttributes().liveStorage;
}

// Determine whether this group has viewable tracks
bool rcTimeline::hasViewableTracks( rcTrackGroup* trackGroup )
{
    bool viewable = false;

    if ( inputIsCamera() && !inputIsStored() ) {
        // Camera input preview
        if ( trackGroup->getType() == eGroupSemanticsCameraPreview ) {
            const int nTracks = trackGroup->getNTracks();
            for ( int j = 0; j < nTracks; ++j)
            {
                rcTrack* track = trackGroup->getTrack( j );
                switch ( track->getTrackType() ) {
                    case eScalarTrack:
                        viewable = true;
                        goto stop;
                        
                    default:
                        break;
                }
            }
        }
    } else {
        // File input or camera storage
        // Don't show cell group tracks in this widget
        if ( trackGroup->getType() != eGroupSemanticsBodyMeasurements &&
             trackGroup->getType() != eGroupSemanticsCameraPreview )
        {
            const int nTracks = trackGroup->getNTracks();
            for ( int j = 0; j < nTracks; ++j)
            {
                rcTrack* track = trackGroup->getTrack( j );
                switch ( track->getTrackType() ) {
                    case eScalarTrack:
                    case eVideoTrack: // For frame ticks
                        viewable = true;
                        goto stop;
                        
                    default:
                        break;
                }
            }
        }
    }

  stop:
    return viewable;
}
