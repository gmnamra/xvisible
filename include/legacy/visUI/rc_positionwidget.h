/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_positionwidget.h 6383 2009-01-29 05:39:09Z arman $
 *
 *	This file 2D position display widget definitions
 *
 ******************************************************************************/

#ifndef _UI_rcPOSITIONWIDGET_H_
#define _UI_rcPOSITIONWIDGET_H_

#include <qframe.h>
#include <qcanvas.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpixmap.h>

#include <rc_window.h>
#include <rc_model.h>
#include <rc_pair.h>
#include <rc_generalhistogram.h>

#include "rc_trackmanager.h"

// Typedefs to make the syntax easier for reader and the compiler
typedef float m1DFType;
typedef rcHistogram<m1DFType,1> h1DFType;


// Simple integer histogram class
// TODO: use STL to implement a generic class

class rcValueHistogram {
public:
    rcValueHistogram( uint32 minValue, uint32 maxValue ) : mMinValue( minValue ), mMaxValue( maxValue ), mAdded( 0 ), mCount( vector<uint32>(maxValue-minValue+1, 0) ) {};
    ~rcValueHistogram() {}

    // Accessors
    uint32 size() const { return mCount.size(); }
    uint32 minValue() const { return mMinValue; }
    uint32 maxValue() const { return mMaxValue; }
    uint32 added() const { return mAdded; }
    uint32 maxCount() const {
        uint32 maxCount = 0;
        
        for ( uint32 i = mMinValue; i <= mMaxValue; ++i ) {
            uint32 c = count( i );
            if ( c > maxCount )
                maxCount = c;
        }
        return maxCount;
    }

    uint32 count( uint32 value ) const {
        rmAssertDebug( value >= mMinValue );
        rmAssertDebug( value <= mMaxValue );
        return mCount[value-mMinValue];
    }
    
    // Mutators
    void add( uint32 value, uint32 count = 1 ) {
        rmAssertDebug( value >= mMinValue );
        rmAssertDebug( value <= mMaxValue );
        mCount[value-mMinValue] += count;
        mAdded += count;
    }

    // Utilities
    void debugDisplay( bool allValues = true ) const {
        cout << "Value\tCount" << endl;
        for ( uint32 i = mMinValue; i <= mMaxValue; ++i ) {
            uint32 c = count( i );
            if ( allValues || c > 0 )
                cout << i << "\t" << c << endl;
        }
    }
    
private:
    uint32         mMinValue;
    uint32         mMaxValue;
    uint32         mAdded;
    vector<uint32> mCount;
};

class rc2Dcanvas;

// Canvas coordinate origin position
enum rcCanvasOrigin {
    eOriginUpperLeft = 0, // Axis directions left-to-right, top-to-bottom
    eOriginUpperRight,    // Axis directions right-to-left, top-to-bottom
    eOriginLowerLeft,     // Axis directions left-to-right, bottom-to-top
    eOriginLowerRight,    // Axis directions right-to-left, bottom-to-top
    eOriginUnknown        // Synthetic sentinel
};

// Displayable track selection
enum rcTrackDisplaySelection {
    eDisplayBodyPosition = 0,     // Display locomotive body (x,y) position tracks
    eDisplayVectorSpeedDirection, // Display body velocity speed and direction tracks
    eDisplayPersistence,          // Display persistence angle
    eDisplayDistance,             // Display distance traveled
    eDisplayACI,                  // Display distance traveled
    eDisplayMovingACI,            // Display distance traveled
    eDisplayUnknown               // Synthetic sentinel
};

//
// Widget for 2D position (coordinate) data display canvas
//

class rcPositionWidget : public QFrame
{
    Q_OBJECT

  public:
    rcPositionWidget( QWidget* parent=0, const char* name=0, WFlags f=0 );
    ~rcPositionWidget();
    
    // Is this track displayable in this widget
    bool isTrackDisplayable( int groupNo, int trackNo, bool countAll = false ) const;
    rcTrackDisplaySelection displayMode() const { return mDisplayMode; }
    // Set painted point count
    void paintedPoints( uint32 count );
    
  public slots:
    // External slots
    void updateDisplay( void );
    void updateTime( const rcTimestamp& time );
    void updateCursorTime( const rcTimestamp& time );
    void updateState( rcExperimentState );
    void settingChanged();
    void showGrid( bool s );
    void accumulate( bool s );
    void connectPoints( bool s );
    void updateScale( rcResultScaleMode s );
    void originSelected( int choice );
    void scaleSelected( int choice );
    void trackSelected( int choice );
    
  protected:
    void paintEvent( QPaintEvent* paintEvent );
    void resizeEvent( QResizeEvent * sizeEvent );
    void mousePressEvent( QMouseEvent* mouseEvent );
    void mouseReleaseEvent( QMouseEvent* mouseEvent );
    void mouseMoveEvent( QMouseEvent* mouseEvent );
    void keyPressEvent( QKeyEvent* keyEvent );
    void keyReleaseEvent( QKeyEvent* keyEvent );
    
  private:
    // Display widget
    void internalUpdateDisplay();
    // Display status line
    void displayStatus( QPoint* point = 0);
    // Create new label
    QLabel* createLabel( QBoxLayout* layout, const char* labelText, const char* toolTipText );
    // Create new checkbox
    QCheckBox* createCheckbox( QBoxLayout* layout, const char* toolTipText, bool enabled );
    // Count displayable tracks and construct window caption
    uint32 countDisplayableTracks( QString& caption ) const;
    
    QColor          mBackgroundColor;          // Widget background color
    QBoxLayout     *mStatusLayout;             // Status area layout
    QBoxLayout     *mTopLayout;                // Top level layout
    rc2Dcanvas     *mCanvas;                   // Display canvas
    QLabel         *mStatusWidget;             // Status bar
    QCheckBox      *mGridWidget;               // Coordinate grid on/off switch
    QLabel         *mGridWidgetLabel;          // Coordinate grid on/off switch label
    QCheckBox      *mAccumulateWidget;         // Data accumulation on/off switch
    QLabel         *mAccumulateWidgetLabel;    // Data accumulation on/off switch label
    QCheckBox      *mConnectPointsWidget;      // Point conenction on/off switch
    QLabel         *mConnectPointsWidgetLabel; // Point conenction on/off switch label
    QComboBox      *mOriginWidget;             // Widget for coordinate origin selection
    QComboBox      *mScaleWidget;              // Widget for scale mode selection
    QComboBox      *mTrackSelectionWidget;     // Widget for displayable track selection
    rcTrackDisplaySelection mDisplayMode;      // Track display selection
    uint32        mPaintedPoints;            // Number of points displayed
};

//
// 2D display area
//

class rc2Dcanvas : public QWidget
{
    Q_OBJECT

 public:
    // Coordinate grid drawing mode
    enum rcGridDrawMode {
        eDrawPositiveAxisName = 0,
        eDrawNegativeAxisName,
        eDrawTick
    };
    
    rc2Dcanvas( QWidget* parent=0, const char* name=0, WFlags f=0 );
    ~rc2Dcanvas();

    bool showGrid() const { return mShowGrid; };
    bool accumulate() const { return mAccumulate; };
    bool connectPoints() const { return mConnectPoints; };
    rcResultScaleMode scaleMode() const { return mScaleMode; };
    
  public slots:
    void updateTime( const rcTimestamp& time );
    void updateCursorTime( const rcTimestamp& time );
    void showGrid( bool s );
    void accumulate( bool s );
    void connectPoints( bool s );
    void updateScale( rcResultScaleMode s );
    void originSelected( int choice );
    void scaleSelected( int choice );
    void settingChanged();
    
  protected:
    void paintEvent( QPaintEvent* paintEvent );
    void resizeEvent( QResizeEvent * sizeEvent );

  private:
    // Paint one vertical scale mark (tick and numeric value)
    void paintVerticalScaleMark( QPainter& p, const char* formatString, const rcFPair& value, 
                                 const rc2DTrackRenderInfo& tri, rcGridDrawMode mode );
    // Paint one horizontal scale mark (tick and numeric value)
    void paintHorizontalScaleMark( QPainter& p, const char* formatString, const rcFPair& value, 
                                   const rc2DTrackRenderInfo& tri, rcGridDrawMode mode );
    // Display coordinate grid
    void displayGrid( QPainter& painter, const rc2DTrackRenderInfo& tri );
    // Display current tracks, return number of tracks displayed
    uint32 displayTracks( QPainter& painter );
    // Display current position tracks
    uint32 displayPositionTracks( QPainter& painter, rcFPair& min, rcFPair& max, uint32& paintedPoints );
    // Display current scalar tracks
    uint32 displayScalarTracks( QPainter& painter, rcFPair& min, rcFPair& max, uint32& paintedPoints );
    // Display current scalar histogram tracks
    uint32 displayScalarHistogramTracks( QPainter& painter, rcFPair& min, rcFPair& max, uint32& paintedPoints );
    // Display current scalar distance tracks
    uint32 displayScalarDistanceTracks( QPainter& painter, rcFPair& min, rcFPair& max, uint32& paintedPoints );
    // Gather persistence values for a histogram
    uint32 gatherPersistenceValues( rcScalarSegmentIterator* iter,
                                      h1DFType::Pointer histogram ) const;
    uint32 gatherACIValues( rcScalarSegmentIterator* iter,
			      h1DFType::Pointer histogram ) const;

    // Paint persistence histogram
    uint32 paintPersistenceHistogram( QPainter& painter, rc2DTrackRenderInfo& tri,
                                        h1DFType::Pointer histogram );

    // Paint persistence histogram
    uint32 paintACIHistogram( QPainter& painter, rc2DTrackRenderInfo& tri,
                                        h1DFType::Pointer histogram );


    // Paint one distance value
    uint32 paintDistanceValue( QPainter& painter, rc2DTrackRenderInfo& tri, rcScalarSegmentIterator* iter, uint32 idx );
    // Initialize data point painter
    void initPointPainter( QPainter& painter, const rc2DTrackRenderInfo& tri ) const;
    // Initialize coordinate grid painter
    void initGridPainter( QPainter& painter, const rc2DTrackRenderInfo& tri ) const;
    
    // Paint position value
    uint32 paintPositionValue( QPainter& painter, rc2DTrackRenderInfo& tri, rcPositionSegmentIterator* iter );
    // Paint speed+direction value
    uint32 paintSpeedDirectionValue( QPainter& painter, rc2DTrackRenderInfo& tri, rcPositionSegmentIterator* iter );
    // Paint speed+direction value centered in a 360 degree circle
    uint32 paintSpeedDirectionCompassValue( QPainter& painter, rc2DTrackRenderInfo& tri, rcPositionSegmentIterator* iter );
    // Paint persistence value centered in a 360 degree circle
    uint32 paintPersistenceCompassValue( QPainter& painter, rc2DTrackRenderInfo& tri, rcScalarSegmentIterator* iter );

    // Paint an array of points
    uint32 paintPoints( QPainter& painter, const vector<rcIPair>& posPoints, bool connectPoints );
    // Paint an array of centered points
    uint32 paintCompassPoints( QPainter& painter, const vector<rcIPair>& posPoints,
                                 const rcIPair& origin );
    
    // Transform position values to pixel values
    rcFPair getPositionScale( const rc2DTrackRenderInfo& tri ) const;
    rcFPair getPositionMinOffset( const rc2DTrackRenderInfo& tri ) const;
    rcFPair getPositionMaxOffset( const rc2DTrackRenderInfo& tri ) const;
    rcIPair positionToPixel( const rcFPair& position, const rc2DTrackRenderInfo& tri ) const;
    rcIPair positionToPixel( const rcFPair& position, const rc2DTrackRenderInfo& tri,
                             const rcFPair scale, const rcFPair& minDev, const rcFPair& maxDev ) const;
    // Assign 2D track min/max values to tri
    void getTrackMinMax( rcResultScaleMode scaleMode, rc2DTrackRenderInfo& tri );
    // Assign scalar track min/max values to tri
    void getScalarTrackMinMax( rcResultScaleMode scaleMode, rcTrackDisplaySelection mode,
                               uint32 displayableTracks, rc2DTrackRenderInfo& tri );
    // Return a rect that leaves room for scale markers
    QRect reducedContentsRect() const;
    // Assign the (common) minimum and maximum values to min and max.
    // Return true if all tracks have the ame scale
    bool checkTrackScales( rcResultScaleMode scaleMode, rcFPair& min, rcFPair& max,
                           const uint32& displayAbleTracks );
    // Return parent widget
    rcPositionWidget* rcParent() const;
    // Count displayable tracks
    uint32 countDisplayableTracks( bool countAll = false ) const;
    // Get cell index (number)
    int32 getCellIndex( rcTrackGroup* trackGroup );
    
    // Return opposite origin
    static rcCanvasOrigin oppositeHorizontalOrigin( rcCanvasOrigin o );
    static rcCanvasOrigin oppositeVerticalOrigin( rcCanvasOrigin o );
    
    bool              mShowGrid;          // Show coordinate grid
    rcTimestamp       mCursorTime;        // Current cursor time
    QColor            mBackgroundColor;   // Widget background color
    rcCanvasOrigin    mOrigin;            // Coordinate system origin 
    bool              mAccumulate;        // Accumulate all values up to current time
    bool              mConnectPoints;     // Connect all points
    rcResultScaleMode mScaleMode;         // Scaling mode
    QPixmap           mPixmap;            // Pixmap for double-buffered display
};

#endif //  _UI_rcPOSITIONWIDGET_H_

