/****************************************************************************
** $Id: rc_imagecanvasgl.h 7179 2011-02-05 22:25:05Z arman $
**
** Copyright (C) 2003 Reify Corporation.  All rights reserved.
**
** @file
*****************************************************************************/

#ifndef _rcIMAGECANVASGL_H
#define _rcIMAGECANVASGL_H

#include <qwidget.h>
#include <qimage.h>
#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>

#include <rc_window.h>
#include <rc_graphics.h>
#include <rc_model.h>
#include <OpenGL/glext.h>
#include <rc_affine.h>
#include <rc_polygon.h>

class rcMonitor;

//
// Struct for focus area/cell selection
//

struct rcFocusTrack {
  int    _trackGroupNo;
  int    _trackNo;
  rcRect _focusRect;
  int    _centerDist;
  QPoint _centerPoint;
    
  rcFocusTrack( int trackGroupNo , int trackNo , const rcRect& focusRect ,
		int centerDist, const QPoint& centerPoint ) :
    _trackGroupNo( trackGroupNo ),
    _trackNo( trackNo ),
    _focusRect( focusRect ),
    _centerDist( centerDist ),
    _centerPoint( centerPoint ) { }
};

//
// Image display area, implemented using OpenGL
//

class rcImageCanvasGL : public QGLWidget
{
  Q_OBJECT

public:
  rcImageCanvasGL( QWidget* parent=0, const char* name=0 );
  ~rcImageCanvasGL();

  // Accessors
  QSize imageSize() const;
  int currentScale() const;
  // Get status string (mouse position, colors etc.)
  void statusString( QString& text ) const;
  // Saturation indicator status
  bool showSaturatedPixels() const { return mShowSaturatedPixels; };
    
  // Mutators
  double scaleChanged( double scale, bool update = true );
  // Display rcWindow and draw graphics in image area
  bool loadrcWindow (const rcWindow& rw, const rcVisualGraphicsCollectionCollection& globalGraphics,
		     const rcVisualGraphicsCollectionCollection& cellGraphics,
		     const rcTextCollection& allTexts, bool showSaturatedPixels );
  // Reset all image data
  void resetImages();
  // Reset all text collections
  void resetTexts();
  // Reset all cell position graphics collections
  void resetCellGraphics();
  // Reset all global graphics collections
  void resetGlobalGraphics();
  // Reset all cached values
  void resetAll();

  const rcRect& focusRect () const
  {
    return mFocusRect;
  }

  QSize getDisplaySize ();
  
public slots:
  void updateState( rcExperimentState );
  void updateAnalysisRect( const rcRect& rect );
  void updateAnalysisRectRotation( const rcAffineRectangle& affine);
  void showSaturatedPixels( bool s );
  void selectButtonRelease ();


protected:
  void keyPressEvent( QKeyEvent* keyEvent );
  void keyReleaseEvent( QKeyEvent* keyEvent );
  void mousePressEvent( QMouseEvent * );
  void mouseReleaseEvent( QMouseEvent * );
  void select ( QMouseEvent * );
  void mouseMoveEvent( QMouseEvent * );
  bool mouseScaleEvent( QMouseEvent * );
  bool mousePanEvent( QMouseEvent * );
  void mouseDoubleClickEvent ( QMouseEvent *);

  void initializeGL();
  void paintGL();
  void resizeGL( int w, int h );

  // @todo implement overlay for rubber band
  //    void initializeOverlayGL();
  //    void paintOverlayGL();
  //    void resizeOverlayGL( int w, int h );
    
private:
  void deleteImageDlist();
  GLuint buildImageDlist( bool deleteTexture = false );
  void deleteGlobalGraphicsDlist();
  void deleteCellGraphicsDlist();
  GLuint buildGlobalGraphicsDlist();
  GLuint buildCellGraphicsDlist();
  void modelTransform();
  void modelTransformCurrent();
  void modelRotate( const rc2Fvector& pivot );
  void resetTransform();


  // Apply translate to current matrix
  void panCurrent( const rc2Fvector& translate );

  // Coordinate transforms
  rc2Fvector devToWorldCoord( const QPoint& devPos, bool correctRatio = true ) const;
  QPoint worldToDevCoord(const rc2Fvector& worldPos, bool correctRatio = true ) const;
  rc2Fvector worldToPixel( const rc2Fvector& worldPos ) const;
  // Transfrom device coords to model coords
  rc2Fvector devToModelCoord( const QPoint& dp ) const;
  // Transfrom model coords to dev coords
  QPoint modelToDevCoord( const rc2Fvector& mp ) const;
  // Transfrom model coords to world coords
  rc2Fvector modelToWorldCoord( const rc2Fvector& mp ) const;
  // Canvas center in world coordinates
  rc2Fvector canvasCenterCoord() const; 
  void setViewport( const QSize& hint );
  // Single pixel size in world coordinates
  rc2Fvector worldPixelSize() const;
  // View size in dev coordinates
  QSize devViewSize() const;
  // Get (expanded) texture size
  QSize textureSize( const QSize& rawImageSize ) const;
    
  // Return parent widget
  rcMonitor* rcParent() const;
  // Get custom cursors
  QCursor zoomInCursor() const;
  QCursor zoomOutCursor() const;
  QCursor panCursor() const;
  // Zoom/pan shortcut detection
  bool zoomKeyPressed( QMouseEvent* e );
  bool zoomInKeyPressed( QMouseEvent* e );
  bool zoomOutKeyPressed( QMouseEvent* e );
  bool panKeyPressed( QMouseEvent* e );
    
  // Return a pan that meets min/max requirements
  QPoint checkPan( const QPoint& panOffset ) const;
  // Return a scale that meets min/max requirements
  double checkScale( double scaleCandidate ) const;
  // Pan to a dev point
  bool panToPoint( const QPoint& point );
  // Zoom to fit rect to screen
  bool zoomToRect( const QRect& zoomRect, const QSize& minSize );
    
  double scaleChanged();
  bool reconvertImage( bool deleteTexture );
  // Update status bar
  void updateStatus();
  // Convert mouse point, update status
  bool convertEvent( QMouseEvent* e, QPoint* modelPoint = NULL );
    
  // Draw lines to graphics display list
  void draw( const rcVisualGraphicsCollectionCollection& segments, bool pushId = false);
  void draw( rcVisualGraphicsCollectionCollection::const_iterator, bool pushId = false);
  // Draw text to current context
  void draw( const rcTextCollection& texts );
  // Draw text to current context
  void draw( const rcTextSegment& text, const rc2Fvector& pixelOrigin );
  // Draw graphics to scaled pixmap which has been already scaled with matrix
  void draw( const rcVisualSegment& g, const rcStyle& style );
  // Draw rect with rotation around top-left point
  
  
  void drawModelRect( const rc2Fvector& x1, const rc2Fvector& x2 );
  // Draw zoom rect rubber band
  void drawZoomRect( const QPoint& upperLeft, const QPoint& lowerRight );
  // Draw focus rect rubber band
  void drawFocusRect( const QPoint& upperLeft, const QPoint& lowerRight );
  // Draw Length Line
  void drawAffineRect ( const QPoint& upperLeft, const QPoint& lowerRight, bool zoom );
  // Draw rect rubber band
  void drawRubberBandRect( const QPoint& upperLeft, const QPoint& lowerRight, bool zoom );
  // Draw current focus area
  void drawCurrentFocus();
  // Update tooltip for widget
  void updateToolTip( const QString& newText );
  // Add default canvas text
  void addDefaultText();
   
  // Return cell position tracks which are inside focus and/or close to the point
  uint32 findCellPositionTracks( const QPoint& p, const QRect& focus, deque<rcFocusTrack>& tracks );
  // Return index of group closest to point, -1 if no applicable groups
  int32 findClosestCellGroup( const QPoint& p, const QRect& focus, QPoint& centerPoint );
  // Enable cell group closest to mouse point
  bool enableCellGroup( const QPoint& mousePoint, const QRect& focus );
  // Get tooltip text
  bool getToolTipText( const QPoint& mousePoint, QString& text );
  // Detect Apple extensions
  void detectAppleExtensions();
  // Set OpenGL drawing style
  void setStyle( const rcStyle& style );
  // Get track value 
  bool getTrackValue( rcTrack* track, const rcTimestamp& cursorTime,
		      rcFPair& pos, double& value );
    
  double      mMaxScaleFactor;      // Maximum scale factor for this image
  bool        mShowSaturatedPixels; // Visually indicate saturated pixels
  QColor      mBackgroundColor;     // Widget background color
  QFont       mTextFont;            // Font for text drawing
    
  // Cached visualization data
  rcWindow    mOrigWindow;      // Original rcWindow to keep a valid reference to framebuf
  rcWindow    mWindow;          // Mirrored rcWindow to keep a valid reference to framebuf
  rcVisualGraphicsCollectionCollection mGlobalGraphics; // Global raphics to be drawn
  rcVisualGraphicsCollectionCollection mCellGraphics; // Cell graphics to be drawn
  rcTextCollection               mTexts;    // Texts to be drawn
  rcRect                         mFocusRect; // Current focus area in model coordinates
  rcAffineRectangle         mFocusAffine; // Affine rectangles for cell areas

  // Cached coordinate/transform data
  double   mScale;           // Scaling factor (1.0 = fit to window)
  double   mFitScale;        // Scale at which this image fits the window (used to be assumed to be 1.0). 
  QPoint   mDevCursorCoord;  // Current cursor in device coords
  QPoint   mDevClickCoord;   // Last clicked point in device coords
  QPoint   mDevMoveCoord;    // Last mouse move point in device coords
  rc2Fvector mWorldPanCoord; // Pan offset in world coords
  bool       mMousePanned;   // Pan performed during mouse move
  bool       mMousePressed;  // Is mouse button pressed?
     
  rc2Fvector mMinModelCoord; // Minimum model(pixel) coordinate values
  rc2Fvector mMaxModelCoord; // Maximum model(pixel) coordinate values
  rc2Fvector mMinWorldCoord; // Minimum world coordinate values
  rc2Fvector mMaxWorldCoord; // Maximum world coordinate values
  rc2Fvector mMinRatioWorldCoord; // Minimum world coordinate values with aspect ratio correction
  rc2Fvector mMaxRatioWorldCoord; // Maximum world coordinate values with aspect ratio correction
  rc2Fvector mMinDevCoord;   // Minimum device(screen) coordinate values
  rc2Fvector mMaxDevCoord;   // Maximum device(screen) coordinate values
    
  // OpenGL data
  GLfloat mX;            // Translation X coord
  GLfloat mY;            // Translation Y coord
  GLfloat mZ;            // Translation Z coord
  GLfloat mRot;          // Rotation angle in degrees
  GLfloat mRatio;        // Expanded wexture width/height ratio
  QSize   mImageSize;    // Size of original image
  QSize   mTextureSize;  // Size of texture image
    
  GLuint mImageDlist;          // Image display list
  GLuint mGlobalGraphicsDlist; // Global graphics display list
  GLuint mCellGraphicsDlist;   // Cell graphics display list
  GLuint  mTextureName;        // Mapped texture name

  int32 mMode;                       //0 is focus rect, 1 is cell-dimension-highlight
  int32 mIndex;                      // Selected Polygon index; -1 means non selected
  int32 mSelectBufferSize;
  rcIPair mSelectRegion;
  rcPolygonGroupRef mPolys; 

  // Available Apple extensions
  bool mExtensionsChecked;      // Check extension list only once
  bool mExtPalettedTexture;     // EXT_paletted_texture
  bool mExtAppleStorage;        // GL_APPLE_client_storage
};

// A class to encapsulate a table of pre-computed cosine values
class rcCosTable {


public:
    // ctor
    rcCosTable() {
        for (uint32 i = 0; i < eTableSize; i++)
            mTable[i] = cos(2*rkPI*i/eTableSize);
    };

    enum { eTableSize = 90};
    
    // Array access operator
    double operator [] (int index) const { rmAssertDebug( index < eTableSize ); return mTable[index]; };
    // Array size
    uint32 size() const { return eTableSize; };
    
private:
    double mTable[eTableSize];
};

// A class to encapsulate a table of pre-computed sine values
class rcSinTable {


public:
    // ctor
    rcSinTable() {
        for (uint32 i = 0; i < eTableSize; i++)
            mTable[i] = sin(2*rkPI*i/eTableSize);
    };

  enum { eTableSize = rcCosTable::eTableSize };    

    // Array access operator
    double operator [] (int index) const { rmAssertDebug( index < eTableSize ); return mTable[index]; };
    // Array size
    uint32 size() const { return eTableSize; };
    
private:
    double mTable[eTableSize];
};

#endif // _rcIMAGECANVASGL_H
