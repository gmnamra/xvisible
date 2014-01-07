#ifndef rcTIMELINE_H
#define rcTIMELINE_H

#include <q3frame.h>
#include <qpen.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>

#include "rc_timestamp.h"
#include "rc_modeldomain.h"

enum rcTimelineNavigationCmd
{
    eTimelineThisFrame = 0,
    eTimelineNextFrame,
    eTimelinePrevFrame
};

class rcTrackRenderInfo;

class rcTimeline : public QWidget
{
    Q_OBJECT

public:
    rcTimeline( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~rcTimeline();

public slots:
	void updateTime( const rcTimestamp& time );
	void updateCursorTime( const rcTimestamp& time ); 
	void updateState( rcExperimentState state );
	void updateRange( const rcTimestamp& start, const rcTimestamp& end );
    void updateScale( rcResultScaleMode );
	void updateDisplay( void );
    void updateSettings( void );
    
protected:
	void mousePressEvent( QMouseEvent* mouseEvent );
	void mouseReleaseEvent( QMouseEvent* mouseEvent );
	void mouseMoveEvent( QMouseEvent* mouseEvent );
    void keyPressEvent( QKeyEvent* keyEvent );
    void keyReleaseEvent( QKeyEvent* keyEvent );
	void paintEvent( QPaintEvent* paintEvent );
/*
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
*/

private:

    // Return a rect that leaves room for scale marks
    QRect reducedContentsRect() const;
    // Paint methods
	void paintTimeline( QPainter& p , QRect& cr );
	void paintScalarTrack( QPainter& p , rcTrackRenderInfo& tri );
    void paintScalarScaleMarks( QPainter& p, rcTrackRenderInfo& tri );
    void paintScalarScaleMark( QPainter& p, const char* formatString, double value, int tickX, int tickY, int textX, int textY );
    void paintFrameTicks( QPainter& p , rcTrackRenderInfo& tri );
    void paintSegmentTick( QPainter& p, int x, rcTrackRenderInfo& tri );
    void paintCursor( QPainter& p );
    // Time <-> pixel conversions
    double getTimeScale() const;
    int32 getTimeOffset() const;
    int32 timeToPixel( const rcTimestamp& currentTime, const double& scale,
                         const int32& offset ) const;
	int32 timeToPixel( const rcTimestamp& currentTime ) const;
    rcTimestamp pixelToTime( int xPos ) const;
    // Value <-> pixel conversions
    double getValueScale( const rcTrackRenderInfo& tri ) const;
    int32 getValueOffset() const;
    int32 valueToPixel( const double& v, const rcTrackRenderInfo& tri,
                          const double& scale, const int32& offset ) const;
	int32 valueToPixel( const double& value, const rcTrackRenderInfo& tri ) const;

    // Frame navigation
    rcVideoTrack* getEnabledVideoTrack() const;
    rcScalarTrack* getEnabledScalarTrack() const;
    rcTimestamp getFrameTime( const rcTimestamp& current, rcTimelineNavigationCmd cmd ) const;
    rcTimestamp getSegmentLength( const rcTimestamp& current ) const;
    // Scroll range if possible
	void scrollRange( const rcTimestamp& start, const rcTimestamp& end );
    // Special event handlers
    void keyScrollEvent( QKeyEvent* keyEvent );
    void keyNavigateEvent( QKeyEvent* keyEvent );
    void keyRangeScaleEvent( QKeyEvent* keyEvent );
    void keyCameraEvent( QKeyEvent* keyEvent );
    
    // Check whether all enabled tracks have the same scale
    bool checkScalarTrackScales( rcResultScaleMode scaleMode, double& min, double& max );
    // Assign track min/max values to tri
    void getScalarTrackMinMax( rcResultScaleMode scaleMode, rcTrackRenderInfo& tri );
    // Check if point is over scalar scale
    bool scaleClicked( const QPoint& point );
    // Reset all scales and ranges to default values
    void resetAllScales();
    // Check if video data source is a camera
    bool inputIsCamera() const;
    // Check if input is being saved in real-time
    bool inputIsStored() const;
    // Determine whether this group has viewable tracks
    bool hasViewableTracks( rcTrackGroup* trackGroup );
    
    rcTimestamp		_currentTime;
	rcTimestamp		_cursorTime;
	rcTimestamp		_rangeStart;
	rcTimestamp		_rangeEnd;
    rcTimestamp     _lastRange;
	QPoint			_lastMouseDown;
    QPen            _cursorPen;
    rcResultScaleMode _currentScale;
    double            _currentScalarScaleMin;
    double            _currentScalarScaleMax;
    QPixmap           _pixmap; // For double buffering
    rcFpsCalculator   _fps;    // Used to calculate display speed in FPS
};


#endif // rcTIMELINE_H
