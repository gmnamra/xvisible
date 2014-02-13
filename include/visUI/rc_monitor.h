/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_monitor.h 7179 2011-02-05 22:25:05Z arman $
 *
 *	This file contains video monitor definitions.
 *
 ******************************************************************************/

#ifndef UI_RCMONITOR_H
#define UI_RCMONITOR_H

#include <q3frame.h>
#include <q3pointarray.h>
#include <qimage.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3BoxLayout>
#include <QMouseEvent>
#include <QLabel>
#include <QKeyEvent>
#include <QPaintEvent>

#include <rc_window.h>
#include <rc_model.h>

#include "rc_imagecanvasgl.h"

class rcTrackInfo;
class QPainter;
class Q3BoxLayout;
class QSpinBox;
class QLabel;
class QCheckBox;

//
// Monitor for image display
//

class rcMonitor : public QWidget
{
    Q_OBJECT

  public:
    rcMonitor( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~rcMonitor();

    // Check if video data source is a camera
    bool inputIsCamera() const;
    // Check if video data is being stored in real-time
    bool inputIsStored() const;
    // Set maximum scale for scale widget
    void setMaxScale( int scale );
    // Update status bar
    void updateStatus();
    // Return bounding rect of engine image
    QRect imageRect() const;
    
  protected:
    void paintEvent( QPaintEvent* paintEvent );
    void resizeEvent( QResizeEvent * sizeEvent );

    public slots:
    // External slots
    void updateDisplay( const rcWindow* image );
    void updateDisplay( const rcVisualGraphicsCollection* graphics );
    void updateDisplay( void );
    void updateAnalysisRect( const rcRect& rect );
    void updateAnalysisRectRotation( const rcAffineRectangle& );
    void updateTime( const rcTimestamp& time );
    void updateCursorTime( const rcTimestamp& time );
    void updateState( rcExperimentState );
    void updateScale( double );
    void updateMonitorSize( const rcRect& size );
    void doPlot (const CurveData* );

    // For monitor widgets
    void scaleChanged( int );
    void settingChanged();
    void saturationChanged( bool );
    
  signals:

  protected:
    void mousePressEvent( QMouseEvent* mouseEvent );
    void mouseReleaseEvent( QMouseEvent* mouseEvent );
    void mouseMoveEvent( QMouseEvent* mouseEvent );
    void keyPressEvent( QKeyEvent* keyEvent );
    void keyReleaseEvent( QKeyEvent* keyEvent );
    
  private:
    // Return true if scaled bitmap was produced
    bool scaleChanged( int, bool updateWidget );

    // Image caching
    bool updateCachedImage( const rcWindow* image );
    void deleteCachedImage();

    // Display current track
    void displayTrackFocus( rcTrackType trackType );
    void displayScalarFocusArea( const rcRect& rect, rcTrackInfo& info );
    rcRect paintScalarTrackFocus( rcTimestamp currentTime, rcTrack* track, rcTrackInfo& info );
    void paintVideoTrackFocus( rcTimestamp currentTime, rcTrack* track );
    void internalUpdateDisplay( void );

    // New OpenGL methods
    // Gather displayable graphics and text data
    void gatherTrackData( rcVisualGraphicsCollectionCollection& globalGraphics,
                          rcVisualGraphicsCollectionCollection& cellGraphics,
                          rcTextSegmentCollection& texts ) const;
    // Gather displayable development graphics and text data
    void gatherDevelopmentTrackData( rcVisualGraphicsCollectionCollection& globalGraphics,
                                     rcVisualGraphicsCollectionCollection& cellGraphics,
                                     rcTextSegmentCollection& texts ) const;
    // Get position in model coordinates
    bool getPosition( rcTimestamp cursorTime, rcTrack* track,
                      rcFPair& pos, rcRect& rect ) const;
    // Add graphics to collection
    bool addGraphics( rcTimestamp cursorTime, rcTrack* track,
                      rcVisualGraphicsCollectionCollection& globalGraphics ) const;
    // Build a cross for cell position and add it to collection
    void buildPositionCross( const rcFPair& pos, rcVisualGraphicsCollection& graphics ) const;
    // Build an enclosing boundary for cell and add it to collection
    void buildCellBoundary( const rcRect& rect, rcVisualGraphicsCollection& graphics ) const;
    // Build cell path graphics
    void buildPositionPath( const rcTimestamp& cursorTime, rcTrack* track,
                            rcVisualGraphicsCollection& graphics ) const;

    bool showDevelopmentData() const;
    
    // Data members
    rcTimestamp mCursorTime;       // Current cursor time

    rcImageCanvasGL *mCanvas;           // OpenGL display canvas
    Q3BoxLayout      *mTopLayout;        // Top level layout
    Q3BoxLayout      *mStatusLayout;     // Status area layout
    QSpinBox        *mScaleWidget;      // Magnification scale control
    QLabel          *mStatusWidget;     // Status bar
    QCheckBox       *mSaturationWidget; // Checkbox for saturation detector
    QLabel          *mSaturationWidgetLabel; // Label for saturation detector
      
    rcFpsCalculator  mSpeedCalc;         // Used to calculate display speed in FPS
    QColor           mBackgroundColor;   // Widget background color
    bool             mCellText;          // Cell text display
    bool             mCellPos;           // Cell position display
    bool             mCellPath;          // Cell path display
    bool             mDevelopmentData;   // Internal development image/graphics display
};


#endif // UI_RCMONITOR_H
