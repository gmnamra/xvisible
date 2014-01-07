/****************************************************************************
 ** @file $Id: rc_imagecanvasgl.cpp 7191 2011-02-07 19:38:55Z arman $
 **
 ** Copyright (C) 2003 Reify Corporation.  All rights reserved.
 **
 **
 *****************************************************************************/

#include <rc_math.h>
#include <rc_vector2d.h>

#include <qapplication.h>
#include <qcursor.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>

#include <rc_writermanager.h>
#include <rc_ipconvert.h>
#include <rc_kinetoscope.h>

#include "rc_monitor.h" // Parent widget
#include "rc_modeldomain.h"
#include "rc_trackmanager.h"

#define QStrFrcStr(foo) QString ((foo).c_str())
//
// Image display area, implemented using OpenGL
//


// Min/max widget sizes
static const int cMinWidth = 240;
static const int cMinHeight = 180;
static const int cDefaultWidth = 320;
static const int cDefaultHeight = 240;
// Max scaled image bitmap size
static const int cMaxPicWidth  = 8192;
static const int cMaxPicHeight = 8192;
// Minimum scale factor
static double cMinScaleFactor = 0.25;
// Maximum scale factor
static double cMaxScaleFactor = 600;
// Scale up/down increment
static double cScaleUpFactor = 0.25;
// Widget background erase mode
static const Qt::BackgroundMode cWidgetBackgroundMode = Qt::FixedColor;
static const int cMouseMinL2 = 100;
static const rcDPair sMidOrigin (0.0, 0.5);
static const double sOtherDimension (10.0);

#define rubberBandColor glColor3d(0.1,0.5,1.0)

// Utilities
#define GLerrorNUM(a) ourGlErrFun(__FILE__, __LINE__, (a))

static int ourGlErrFun(char *Fname,int32 Lnumber,char *FuncName);

// Number of line segments used to approximate an ellipse
static const uint32 cEllipseSegments =rcCosTable::eTableSize;

// Cos/sin value lookup tables for circle drawing
static const rcCosTable cCosTable;
static const rcSinTable cSinTable;

// Angle of arrow head barbs
const rcRadian cArrowTipAngle(rkPI - rkPI/6.);

// Comparator for focus area selection
static bool focusCompare( const rcFocusTrack& lhs, const rcFocusTrack& rhs )
{
    return (lhs._centerDist <= rhs._centerDist);
}

/*! \fn static bool copyImage( const rcWindow& src, const QSize& newSize, rcWindow& dst, bool mirror, bool equalize = false)
 *  \brief Copies analysis images with appropriate depth for display in OpenGL
 *  \param src is the src analysis image 8 and 16 are displayed at 8, 32 is ARGB
 *  \param dst is output image. Caching scheme avoid creating new images unless necessary
 *  \param mirror VerticalReflect to adjust to camera centric openGL design.
 *  \param equalize for 8 bit images uses image equalization
 */

static bool copyImage( const rcWindow& src, const QSize& newSize, rcWindow& dst, bool mirror, bool equalize = false);


// Display OpenGL info returned by the system
static void displayOpenGLInfo()
{
    const GLubyte* v = glGetString( GL_VENDOR );
    if ( v )
        cerr << v << endl;
    const GLubyte* vr  = glGetString( GL_VERSION );
    if ( vr )
        cerr << vr << endl;
    const GLubyte* r = glGetString( GL_RENDERER );
     if ( r )
        cerr << r << endl;
    const GLubyte* e = glGetString( GL_EXTENSIONS );
    if ( e )
        cerr << e << endl;
}

// Display OpenGL error returned by the system
static void displayOpenGLError( const char* prefix, GLenum error )
{
    if ( error != GL_NO_ERROR ) {
        const GLubyte* errString = gluErrorString( error );

        cerr << prefix << " OpenGL error: ";
        if ( errString )
            cerr << errString;
        else
            cerr << error;
        cerr << endl;
    }
}

static int ourGlErrFun(char *Fname,int32 Lnumber,char *FuncName)
{
   GLenum err;
   err=glGetError();
   if (err  != GL_NO_ERROR)
     {
       printf("FileName:%s Line:%d Function Name:%s\nError:(%d)%s\n"
	      ,Fname,Lnumber,FuncName,err, gluErrorString(err));
     }
   return(err);
}


//
// rcImageCanvasGL class implementation
//
static const int32 sSelectBufferSize = 1000;
static GLuint sSelectBuffer[sSelectBufferSize];

// public

rcImageCanvasGL::rcImageCanvasGL( QWidget* parent, const char* name ) :
        QGLWidget( parent, name )
{
    displayOpenGLInfo();

    if ( !format().doubleBuffer() )
        cerr << "rcImageCanvasGL: no double buffering" << endl;
    if ( !format().hasOpenGLOverlays() )
        cerr << "rcImageCanvasGL: no OpenGL overlay support" << endl;

    mIndex = -1;
    mSelectBufferSize = sSelectBufferSize;
    mSelectRegion = rcIPair (5, 5);

    mScale = 1.0;
    mX = 0.0;
    mY = 0.0;
    mZ = 0.0;
    mRot = 0.0;
    mRatio = 1.0;
    mImageDlist = 0;
    mGlobalGraphicsDlist = 0;
    mCellGraphicsDlist = 0;
    mTextureName = 0;
    mMode = 0;

    mMinModelCoord = rc2Fvector( 0, 0 );
    mMaxModelCoord = rc2Fvector( cDefaultWidth, cDefaultHeight );
    mMinWorldCoord = rc2Fvector( -1, -1 );
    mMaxWorldCoord = rc2Fvector( 1, 1 );
    mMinRatioWorldCoord = mMinWorldCoord;
    mMaxRatioWorldCoord = mMaxWorldCoord;
    mMinDevCoord = rc2Fvector( 0, 0 );
    mMaxDevCoord = rc2Fvector( cDefaultWidth, cDefaultHeight );
    mMousePanned = false;
    mMousePressed = false;

    mMaxScaleFactor = cMaxScaleFactor;
    mShowSaturatedPixels = false;

    mExtensionsChecked = false;
    mExtPalettedTexture = false;
    mExtAppleStorage = false;

    // Gets (keyboard) focus by mouse clicking
    setFocusPolicy( Qt::ClickFocus);
    // Track mouse real-time
    setMouseTracking( TRUE );

    // Use a fixed color to erase background, it's faster than default widget erase pixmap
    mBackgroundColor = QColor( 238, 238, 238 );

    mTextFont = QFont( "Helvetica", 16 );
    setBackgroundMode( cWidgetBackgroundMode, cWidgetBackgroundMode );
    setPaletteBackgroundColor( mBackgroundColor );

    setMinimumSize( cDefaultWidth, cDefaultHeight );

    // Get maximum screen size
    setMaximumSize(getDisplaySize () );

    resetTransform();
    addDefaultText();
}

rcImageCanvasGL::~rcImageCanvasGL()
{
    resetAll();
    QToolTip::remove( this );
}

QSize rcImageCanvasGL::getDisplaySize ()
{
  CGRect cg = CGDisplayBounds(kCGDirectMainDisplay);
  return QSize ((int32) cg.size.width, (int32) cg.size.height);
}

void rcImageCanvasGL::resetAll()
{
    makeCurrent();
    mFocusRect = rcRect();
    mFocusAffine = rcAffineRectangle ();
    resetImages();
    resetCellGraphics();
    resetGlobalGraphics();
    resetTexts();
    resetTransform();
}

QSize rcImageCanvasGL::imageSize() const
{
    return QSize( mImageSize.width(), mImageSize.height() );
}

int rcImageCanvasGL::currentScale() const
{
    return int(mScale * 100);
}

// Get status (mouse position, colors etc.)
void rcImageCanvasGL::statusString( QString& text ) const
{
    QString message;
    static const std::string f ("Focus Window Mode");
    static const std::string d ("Dimension Outline Mode");
    const std::string& s = (mMode) ? d : f;

    if ( mImageSize == QSize( 0, 0 ) || !mWindow.isBound() ) {
        message += "No image data";
        text = message;
    } else {
        QString moremsg;

        // Do not display mouse position during live capture
        if ( !rcParent()->inputIsCamera() ) {
            rc2Fvector modelCursorCoord = devToModelCoord( mDevCursorCoord );
            rc2Fvector modelClickCoord = devToModelCoord( mDevClickCoord );
            rc2Fvector worldCursorCoord = devToWorldCoord( mDevCursorCoord );

            if ( mMousePressed && !mMousePanned ) {
                // Use temp rect during mouse focus movement
                message.sprintf( "%i x %i",
                                 int(0.5+rmABS( modelClickCoord.x() - modelCursorCoord.x())),
                                 int(0.5+rmABS( modelClickCoord.y() - modelCursorCoord.y())) );
            } else {
                // Use accepted focus rect
                message.sprintf( "%i x %i",
                                 mFocusRect.width(),
                                 mFocusRect.height() );
            }

            uint32 pixel = 0, color = 0;
            const int modelX = static_cast<int>(modelCursorCoord.x());
            const int modelY = static_cast<int>(modelCursorCoord.y());

            if ( mOrigWindow.isBound() && mOrigWindow.isWithin( modelX, modelY ) ) {
                pixel = mOrigWindow.getPixel( modelX, modelY );
                // Get color from colormap if it exists
                if ( mOrigWindow.frameBuf()->colorMapSize() > pixel )
                    color = mOrigWindow.frameBuf()->getColor( pixel );
                else
                    color = pixel;
            }

            if ( (mOrigWindow.depth() == rcPixel8 && mOrigWindow.isGray()) ||
		 mOrigWindow.depth() == rcPixel16 )
	      {
                moremsg.sprintf(",  (%.2f,%.2f) =%3i %s ",
                                modelCursorCoord.x(), modelCursorCoord.y(),
                                pixel, s.c_str()
                                );
                message += moremsg;
            } else {
                // Get color component values
                uint32 red = rfRed( color );
                uint32 green = rfGreen( color );
                uint32 blue = rfBlue( color );
                uint32 alpha = rfAlpha( color );

                moremsg.sprintf(", (%.2f,%.2f) =%3ir %3ig %3ib %3ia %s",
                                modelCursorCoord.x(), modelCursorCoord.y(),
                                red,
                                green,
                                blue,
                                alpha, s.c_str()
                                );
                message += moremsg;

                if ( mWindow.frameBuf()->colorMapSize() > 0 ) {
                    if ( mWindow.isWithin( modelX, modelY ) ) {
                        moremsg.sprintf(", %d/%d colors",
                                        pixel,
                                        mWindow.frameBuf()->colorMapSize());
                    } else {
                        moremsg.sprintf(", %d colors",  mWindow.frameBuf()->colorMapSize());
                    }
                    message += moremsg;
                }
                // TODO: add alpha info
            }
        } else {
            message.sprintf( "%i x %i",
                             mFocusRect.width(),
                             mFocusRect.height() );
        }
        text = message;
    }
}

// Scale is a percentage
double rcImageCanvasGL::scaleChanged( double scale, bool update )
{
    double sx = scale;

    // Throttle scale if necessary
    mScale = checkScale( sx );
    modelTransform();

    if ( update ) {
        updateGL();
	GLerrorNUM("scaleChanged");
        updateStatus();
    }

    return mScale;
}

//#define DEBUG_LOG 1
bool rcImageCanvasGL::loadrcWindow (const rcWindow& rw, const rcVisualGraphicsCollectionCollection& globalGraphics,
                                    const rcVisualGraphicsCollectionCollection& cellGraphics,
                                    const rcTextCollection& texts, bool showSaturatedPixels )
{
    mShowSaturatedPixels = showSaturatedPixels;

    // Set QImage ctor argument values
    // Return true if input image is valid, false otherwise
    bool ok = FALSE;

    // Copy all graphics
    mGlobalGraphics = globalGraphics;
    mCellGraphics = cellGraphics;
    // Copy all texts
    mTexts = texts;

    if ( rw.isBound() )
      {
        bool deleteTexture = false;

	// Depth has changed!
        deleteTexture = mWindow.isBound() && mWindow.depth() != rw.depth();

        QSize oldSize = mImageSize;
        // Original size before texture expansion
        mImageSize = QSize( rw.width(), rw.height() );

        if ( mImageSize.width() < oldSize.width() ||
             mImageSize.height() < oldSize.height() )
	  {
            // New image is smaller than old image
	    deleteTexture = mWindow.isBound();
	  }

	QSize textureForDisplay = textureSize (maximumSize());

	if ( mImageSize.width() > textureForDisplay.width() ||
             mImageSize.height() > textureForDisplay.height() )
	  {
            // New image is bigger than old image
	    deleteTexture = mWindow.isBound();
	  }

        // Expanded size (may be the same as original)
        mTextureSize = textureSize( mImageSize );

        mOrigWindow = rw;
        // Clip + expand + mirror
        copyImage( rw, mTextureSize, mWindow, true, false);
        // Regenerate image display list
        ok = reconvertImage( deleteTexture );

        if ( !ok ) {
            cerr << "rcMonitor::loadrcWindow: reconvertImage failed!" << endl;
        }
    } else {
        // Empty image causes a reset
        resetImages();
        ok = true;
    }

    // Regenerate graphics display lists
    mGlobalGraphicsDlist = buildGlobalGraphicsDlist();
    mCellGraphicsDlist = buildCellGraphicsDlist();

    updateGL();
    GLerrorNUM("loadrcWindow");

    return ok;
}

// Reset all image data
void rcImageCanvasGL::resetImages()
{
    if ( mTextureName ) {
        glDeleteTextures( 1, &mTextureName );
        mTextureName = 0;
    }
    deleteImageDlist();
    mOrigWindow = rcWindow();
    mWindow = rcWindow();
    mImageSize = QSize( 0, 0 );
}

// Reset all text collections
void rcImageCanvasGL::resetTexts()
{
    mTexts = rcTextCollection();
}

// Reset all cell position graphics collections
void rcImageCanvasGL::resetCellGraphics()
{
    mCellGraphics = rcVisualGraphicsCollectionCollection();
    deleteCellGraphicsDlist();
}

// Reset all global graphics collections
void rcImageCanvasGL::resetGlobalGraphics()
{
    mGlobalGraphics = rcVisualGraphicsCollectionCollection();
    deleteGlobalGraphicsDlist();
}

// public slots

void rcImageCanvasGL::updateState( rcExperimentState state )
{
    switch( state ) {
        case eExperimentEmpty:
            // Starting a new experiment
            resetAll();
            addDefaultText();
            break;
        default:
            break;
    }
}



void rcImageCanvasGL::updateAnalysisRect( const rcRect& rect )
{
    mFocusRect = rect;
    // Enable cell groups inside analysis area
    QRect focus( mFocusRect.x(), mFocusRect.y(), mFocusRect.width(), mFocusRect.height() );
    QPoint point( mFocusRect.x(), mFocusRect.y());
    enableCellGroup( point, focus );
    updateGL();
    GLerrorNUM("updateAnalysisRect");
}

void rcImageCanvasGL::selectButtonRelease ()
{
  rcModelDomain* domain = rcModelDomain::getModelDomain();
}



void rcImageCanvasGL::updateAnalysisRectRotation(  const rcAffineRectangle& affine )
{
  //cerr << "rcImageCanvasGL::updateAnalysisRectRotation " << rotation << endl;
  mFocusAffine = affine;
  updateGL();
  GLerrorNUM("updateAnalysisRectRotation");
}

void rcImageCanvasGL::showSaturatedPixels( bool s )
{
    if ( s != mShowSaturatedPixels ) {
        mShowSaturatedPixels = s;
        // Regenerate image display list
        mImageDlist = buildImageDlist();
        updateGL();
	GLerrorNUM("showSaturatedPixels");
    }
}

// protected

// Quick pan when shift is pressed
static const int cQuickPanFactor = 10;

void rcImageCanvasGL::keyPressEvent( QKeyEvent* keyEvent )
{
    // No pan or zoom allowed during capture storage
    if ( rcParent()->inputIsStored() ) {
        keyEvent->ignore();
        return;
    }

    // Pan
    int key = keyEvent->key();
    bool panned = false;

    // Pan by width of one pixel
    rc2Fvector pixelSize = worldPixelSize();
    int devPanIncrement = 1;

    // Quick pan
    if ( keyEvent->state() & Qt::ShiftButton ) {
        // Accelerate pan by a factor
        devPanIncrement *= cQuickPanFactor;
        pixelSize *= cQuickPanFactor;
    }
    rc2Fvector worldPanCoord;

    switch ( key ) {
        case Qt::Key_Up:
            // Pan up
            worldPanCoord = rc2Fvector( 0.0, -pixelSize.y() );
            mDevCursorCoord.setY( mDevCursorCoord.y() - devPanIncrement );
            panned = true;
            keyEvent->accept();
            break;

        case Qt::Key_Down:
            // Pan down
            worldPanCoord = rc2Fvector( 0.0, pixelSize.y() );
            mDevCursorCoord.setY( mDevCursorCoord.y() + devPanIncrement );
            panned = true;
            keyEvent->accept();
            break;

        case Qt::Key_Left:
            // Pan left
            worldPanCoord = rc2Fvector( pixelSize.x(), 0.0 );
            mDevCursorCoord.setX( mDevCursorCoord.x() - devPanIncrement );
            panned = true;
            keyEvent->accept();
            break;

        case Qt::Key_Right:
            // Pan right
            worldPanCoord = rc2Fvector( -pixelSize.x(), 0.0 );
            mDevCursorCoord.setX( mDevCursorCoord.x() + devPanIncrement );
            panned = true;
            keyEvent->accept();
            break;

        case Qt::Key_Home:
            // Reset pan
            worldPanCoord = -mWorldPanCoord;
            panned = true;
            keyEvent->accept();
            break;

        case Qt::Key_Escape:
            // Reset all transformations
            mWorldPanCoord = rc2Fvector(0,0);
            scaleChanged( 1.0 );
            updateStatus();
            keyEvent->accept();
            break;

        case Qt::Key_Plus:
            // Zoom in
            scaleChanged( mScale + mScale * cScaleUpFactor );
            break;

        case Qt::Key_Minus:
            // Zoom out
            scaleChanged( mScale + mScale * -cScaleUpFactor );
            break;

        case Qt::Key_Asterisk:
            // Zoom to focus area
        {
            // Reset all transformations
            mWorldPanCoord = rc2Fvector(0,0);
            scaleChanged( 1.0, false );
            // FIXME: this doesn't seem to work for all frame sizes
            // Convert focus rect from model to dev coords
            rc2Fvector ful( float(mFocusRect.ul().x()),
                            float(mFocusRect.ul().y()) );
            rc2Fvector flr( float(mFocusRect.lr().x()),
                            float(mFocusRect.lr().y()) );
            QPoint ul = modelToDevCoord( ful );
            QPoint lr = modelToDevCoord( flr );

            QRect zoomRect( ul.x(),
                            ul.y(),
                            rmABS(lr.x() - ul.x()),
                            rmABS(lr.y() - ul.y()) );
            zoomRect.normalize();
            zoomToRect( zoomRect, QSize(2,2) );
        }
        break;

        case Qt::Key_Shift:
            // Set pan cursor
            //QApplication::setOverrideCursor( panCursor() );
            keyEvent->accept();
            break;

        case Qt::Key_Control:
            // Set zoom cursor
            //QApplication::setOverrideCursor( zoomInCursor() );
            keyEvent->accept();
            break;

        case Qt::Key_Alt:
            // Set zoom cursor
            //QApplication::setOverrideCursor( zoomOutCursor() );
            keyEvent->accept();
            break;

        case Qt::Key_A:
            if ( keyEvent->state() & Qt::ControlButton ) {
                // Cmd-A, select all
                rcModelDomain* domain = rcModelDomain::getModelDomain();
                uint32 maxX = domain->getExperimentAttributes().frameWidth;
                uint32 maxY = domain->getExperimentAttributes().frameHeight;

                rcRect wholeImage( 0, 0, maxX, maxY );
                domain->notifyAnalysisRect( wholeImage );
            }

            keyEvent->accept();
            break;

        default:
            keyEvent->ignore();
            break;
    }

    if ( panned ) {
        // Translate current matrix and redisplay
        panCurrent( worldPanCoord );
        updateGL();
	GLerrorNUM("keyPressEvent");
    }
}

void rcImageCanvasGL::keyReleaseEvent( QKeyEvent* keyEvent )
{
    keyEvent->ignore();
}


// Pan image
bool rcImageCanvasGL::mousePanEvent( QMouseEvent *e )
{
    bool panned = false;

    if ( e ) {
        rc2Fvector p = devToModelCoord( e->pos() );
        QPoint modelPoint = QPoint( static_cast<int>(p.x()), static_cast<int>(p.y()) );

        // Pan only if we clicked inside the image
        if ( mWindow.isWithin(modelPoint.x(), modelPoint.y()) )
        {
            panToPoint( mDevCursorCoord );
            panned = true;
        }
    }

    return panned;
}

// Scale image
bool rcImageCanvasGL::mouseScaleEvent( QMouseEvent *e )
{
    bool scaled = false;

    if ( zoomKeyPressed( e ) ) {
        // Zoom
        double factor = 1.0;
        if ( zoomInKeyPressed( e ) ) {
            // Zoom in
            factor = cScaleUpFactor;
        } else {
            // Zoom out
            factor = -cScaleUpFactor;
        }
        scaleChanged( mScale + mScale * factor );
    } else {
        updateStatus();
    }

    return scaled;
}

void rcImageCanvasGL::mousePressEvent( QMouseEvent *e )
{
    mMousePanned = false;
    mMousePressed = true;

    // No pan or zoom allowed during capture storage
    if ( rcParent()->inputIsStored() ) {
        return;
    }

    // Store click coordinates, action is perfromed on mouse release
    mDevClickCoord = e->pos();
    mDevMoveCoord = mDevClickCoord;
    convertEvent( e );
}


void rcImageCanvasGL::mouseDoubleClickEvent ( QMouseEvent * e )
{
  rmUnused(e);
  int32 lastMode = mMode;
  mMode ^= 1;
  rcModelDomain* domain = rcModelDomain::getModelDomain();
  // If we switched back from dimension setting to focus mode
  // And we have polys, notify
  domain->notifySelectionState (mMode != 0);
  if (lastMode == 1 && mMode == 0 && domain->polysSize ())
    {
      domain->notifyPolys ();
    }
}



void rcImageCanvasGL::mouseMoveEvent( QMouseEvent *e )
{

    // No pan or zoom allowed during capture storage
    if ( rcParent()->inputIsStored() ) {
        return;
    }

    bool mouseButtonDown = ( e->state() & Qt::LeftButton ||
                             e->state() & Qt::RightButton );

    mMousePressed = mouseButtonDown;
    if ( mouseButtonDown ) {
        if ( zoomKeyPressed( e ) ) {

	  if (!mMode)
	  {
            // Focus area selection
            convertEvent( e );
            // Draw new focus rect
            // Draw new zoom rect
            drawZoomRect( mDevClickCoord, mDevCursorCoord );
            swapBuffers();
        }
	else
	  {
         // Focus area selection
            convertEvent( e );
            // Draw new focus rect
            drawAffineRect ( mDevClickCoord, mDevCursorCoord, true );
            swapBuffers();
	  }

        } else if ( panKeyPressed( e ) ) {
            mMousePanned = true;
            // When pan key is pressed, image can be dragged
            convertEvent( e );
            // Move offset
            QPoint movedCoord = mDevMoveCoord - mDevCursorCoord;
            // Transform mouse move offset to world coord offset
            rc2Fvector worldCenterCoord = canvasCenterCoord();
            QPoint devCenterCoord = worldToDevCoord( worldCenterCoord, false );
            QPoint newCenterCoord = devCenterCoord + movedCoord;
            rc2Fvector newWorldCenterCoord = devToWorldCoord( newCenterCoord, false );
            rc2Fvector worldDiff = newWorldCenterCoord - worldCenterCoord;
            // Pan with offset
            panCurrent( -worldDiff );
            updateGL();
        } else if (!mMode)
	  {
            // Focus area selection
            convertEvent( e );
            // Draw new focus rect
            drawFocusRect( mDevClickCoord, mDevCursorCoord );
            swapBuffers();
        }
	else
	  {
         // Focus area selection
            convertEvent( e );
            // Draw new focus rect
            drawAffineRect ( mDevClickCoord, mDevCursorCoord, false );
            swapBuffers();
	  }

        mDevMoveCoord = mDevCursorCoord;
    } else {
        convertEvent( e );
        QString tipText;

        getToolTipText( e->pos(), tipText );
        updateToolTip( tipText );
    }
}

void rcImageCanvasGL::mouseReleaseEvent( QMouseEvent *e )
{
    mMousePressed = false;

    // No pan or zoom allowed during capture storage
    if ( rcParent()->inputIsStored() ) {
        return;
    }

    QPoint devReleaseCoord = e->pos();
    QRect zoomRect( mDevClickCoord, devReleaseCoord );
    const QSize minSize( 2, 2 );

    // Final selection must be normalized
    zoomRect = zoomRect.normalize();

    convertEvent( e );

    if ( zoomKeyPressed( e ) )
      {
        if ( ! zoomToRect( zoomRect, minSize ) )
	  mouseScaleEvent( e );
      } else if ( panKeyPressed( e ) )
      {
        // Pan to click point
        if ( !mMousePanned )
	  {
            mousePanEvent( e );
            updateGL();
	    GLerrorNUM("mouseReleaseEvent");
	  }
      } else
      {

	if (!mMode)
	  {
	    // Regular focus rect drawing
	    // Convert from device to image coordinates
	    rc2Fvector tl = devToModelCoord( zoomRect.topLeft() );
	    rc2Fvector lr = devToModelCoord( zoomRect.bottomRight() );
	    int i;
	    QRect modelRect( rfRound(tl.x(), i),
			     rfRound(tl.y(), i),
			     rfRound(lr.x() - tl.x(), i),
			     rfRound(lr.y() - tl.y(), i) );

	    // Clip selection to image boundaries
	    QRect iRect = rcParent()->imageRect();
	    if ( modelRect.intersects( iRect ) ) {
	      QRect cr = modelRect.intersect( iRect );
	      cr.normalize();
	      rcRect newRect( cr.x(), cr.y(), cr.width(), cr.height() );
	      rcModelDomain* domain = rcModelDomain::getModelDomain();
	      // A focus area was produced
	      domain->notifyAnalysisRect( newRect );
	    } else {
	      rcRect newRect( modelRect.x(), modelRect.y(), modelRect.width(), modelRect.height() );

	      if ( newRect.width() == 0 && iRect.contains( newRect.x(), newRect.y() ) ) {
                // Single click with no dragging
                rcModelDomain* domain = rcModelDomain::getModelDomain();


                // Enable cell group that was clicked on
                if ( enableCellGroup( e->pos(), modelRect ) ) {
		  // Cell(s) were selected
		  domain->notifyAnalysisRect( newRect );
                } else {
		  // No cells were selected, select whole image
		  rcRect wholeImage( 0,
				     0,
				     domain->getExperimentAttributes().frameWidth,
				     domain->getExperimentAttributes().frameHeight);
		  domain->notifyAnalysisRect( wholeImage );
                }
	      }
	    }
	    updateGL();
	    GLerrorNUM("mouseReleaseEvent");
	  }
	else
	  {
	    rcModelDomain* domain = rcModelDomain::getModelDomain();
	    rcRect wholeImage( 0,0,domain->getExperimentAttributes().frameWidth,
			       domain->getExperimentAttributes().frameHeight);
	    domain->notifyAnalysisRect( wholeImage );

	    // Affine dimension setting
	    // Convert from device to image coordinates
	    rc2Fvector tl = devToModelCoord( mDevClickCoord );
	    rc2Fvector lr = devToModelCoord( devReleaseCoord );

	    if (tl.distanceSquared(lr) > sOtherDimension)
	      {
		rcAffineRectangle newaff (rc2Dvector((double) tl.x(), (double) tl.y()),
					  rc2Dvector((double) lr.x(), (double) lr.y()),
					  sOtherDimension, sMidOrigin);
		domain->notifyAnalysisRectRotation ( newaff );
		rcPolygon newpol (newaff);
		rcPolygonGroupRef pgr = domain->polys ();
		const_cast<vector<rcPolygon>&> (pgr.polys()).push_back (newpol);
		cerr << "User Poly Size: " << pgr.size () << endl;
	      }

	    updateGL();
	    GLerrorNUM("mouseReleaseEvent");
	  }
      }
}

void rcImageCanvasGL::select(QMouseEvent* e)
{
  // Make openGL context current
  makeCurrent();

  glSelectBuffer(sSelectBufferSize, sSelectBuffer);
  glRenderMode(GL_SELECT);
  glInitNames();

  // Loads the matrices
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);
  gluPickMatrix(static_cast<GLdouble>(e->x()), static_cast<GLdouble>(viewport[3] - e->y()),
		mSelectRegion.x(), mSelectRegion.y(), viewport);

  // Render scene with objects ids
  draw( mGlobalGraphics, true);
  glFlush();

  // Get the results
  GLint nb_hits = glRenderMode(GL_RENDER);

  // Interpret results
  unsigned int zMin = UINT_MAX;
  mIndex = -1;
  for (int i=0; i<nb_hits; ++i)
    {
      if (sSelectBuffer[i*4+1] < zMin)
	{
	  zMin = sSelectBuffer[i*4+1];
	  mIndex = sSelectBuffer[i*4+3];
	}
    }

  cout << nb_hits << " spiral" << ((nb_hits>1)?"s":"") << " under the cursor";
  if (mIndex >= 0)
    cout << ", selected = " << mIndex;
  cout << endl;
}

/*!
  Render display lists
*/

void rcImageCanvasGL::paintGL()
{
  //@note depth processing
  glClear( GL_COLOR_BUFFER_BIT ); // |  GL_DEPTH_BUFFER_BIT );
  GLerrorNUM("paintGL");

  GLsizei activeLists = 0;
  GLuint lists[3];

  if ( mImageDlist ) {
    lists[activeLists] = mImageDlist;
    ++activeLists;
  }
  if ( mGlobalGraphicsDlist ) {
    lists[activeLists] = mGlobalGraphicsDlist;
    ++activeLists;
  }
  if ( mCellGraphicsDlist ) {
    lists[activeLists] = mCellGraphicsDlist;
    ++activeLists;
  }

  // Draw all lists at once
  if ( activeLists ) {
    glListBase( 0 );
    glCallLists( activeLists, GL_UNSIGNED_INT, &lists[0] );
  }

  // Draw texts
  draw( mTexts );
  // Draw current focus area rectangle
  drawCurrentFocus();

  GLerrorNUM("paintGL");
  glFlush();
  displayOpenGLError( "rcImageCanvasGL::paintGL:",
		      glGetError() );
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void rcImageCanvasGL::initializeGL()
{
    //cerr << "initializeGL" << endl;

    detectAppleExtensions();

    qglClearColor( mBackgroundColor );		// Let OpenGL clear to color

    // Set viewport
    setViewport( size() );

    // Set up various things for optimum speed
    glDisable( GL_DITHER );
    glDisable ( GL_DEPTH_TEST ); //  @note depth select
    glShadeModel( GL_FLAT );
    // Set up projection
    gluOrtho2D( -1.0, 1.0, -1.0, 1.0 );

    // Set up the textures
    if ( mWindow.isBound() ) {
        // Build the display lists
        mImageDlist = buildImageDlist( true );
    } else {
        //cerr << "rcImageCanvasGL::initializeGL: NULL image" << endl;
    }

    mGlobalGraphicsDlist = buildGlobalGraphicsDlist();
    mCellGraphicsDlist = buildCellGraphicsDlist();
    displayOpenGLError( "rcImageCanvasGL::initializeGL:",
                        glGetError() );
}

/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void rcImageCanvasGL::resizeGL( int w, int h )
{
    //cerr << "resizeGL " << w << " x " << h << endl;

    setViewport( QSize( w, h ) );
    modelTransform();
    displayOpenGLError( "rcImageCanvasGL::resizeGL:",
                        glGetError() );
}

// private
void rcImageCanvasGL::deleteImageDlist()
{
    // Delete old display list
    if ( mImageDlist ) {
        makeCurrent();
        glDeleteLists( mImageDlist, 1 );
        mImageDlist = 0;
    }
}

#if __BIG_ENDIAN__
#define ARGB_IMAGE_TYPE GL_UNSIGNED_INT_8_8_8_8_REV
#else
#define ARGB_IMAGE_TYPE GL_UNSIGNED_INT_8_8_8_8
#endif

//@note see http://developer.apple.com/documentation/MacOSX/Conceptual/universal_binary/universal_binary_tips/chapter_5_section_25.html

GLuint rcImageCanvasGL::buildImageDlist( bool deleteTexture )
{
    // Delete old list if it exists
    deleteImageDlist();

// #ifdef GL_TEXTURE_RECTANGLE_EXT
//     GLenum textureTarget = GL_TEXTURE_RECTANGLE_EXT;
// #else
      GLenum textureTarget = GL_TEXTURE_2D;
    //#endif

    if ( deleteTexture && mTextureName ) {
        // Delete old texture
        glDeleteTextures( 1, &mTextureName );
        mTextureName = 0;
    }

    if ( mWindow.isBound() ) {
        GLuint imgObj = glGenLists(1);

        glNewList( imgObj, GL_COMPILE );
        glPushMatrix();
        glPushAttrib( GL_CURRENT_BIT );

        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
        // Eliminate one copy, use original image data for textures
        if ( mExtAppleStorage )
            glPixelStorei( GL_UNPACK_CLIENT_STORAGE_APPLE, 1 );
        bool firstTime = false;

        // Create named texture if it doesn't exist
        if ( mTextureName == 0 ) {
            glGenTextures( 1, &mTextureName );
            firstTime = true;
        }

        glBindTexture( textureTarget, mTextureName );

        // Do not interpolate, get nearest pixel value
        glTexParameterf( textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameterf( textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

        glEnable( textureTarget );
        // Use replace mode
        glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

        // Create/update texture
        const int rawDepth = mWindow.depth() * 8;
        const rcFrame& frame = *mWindow.frameBuf();
        const uint8* imgData = frame.alignedRawData();

        switch ( rawDepth ) {
            case 8:
                if ( mExtPalettedTexture ) {
                    // Use paletted texture extension
                    // Note: All 8-bit images are assumed to be gray scale images
                    // Color indices are only used for visualization tricks like saturated
                    // pixel indication.
                    uint32 cMapSize = frame.colorMapSize();
                    rmAssert( cMapSize == 256 );
                    GLubyte ct[256][4];

                    // Construct OpenGL color table from framebuf color table
                    for ( uint32 i = 0; i < cMapSize; i++ ) {
                        uint32 color = frame.getColor( i );
                        // Set each color component separately
                        ct[i][0] = rfRed( color );
                        ct[i][1] = rfGreen( color );
                        ct[i][2] = rfBlue( color );
                        ct[i][3] = rfAlpha( color );
                    }
                    // Pixel saturation indicator
                    if ( mShowSaturatedPixels ) {
                        // Color saturated pixels bright red
                        ct[255][0] = 255;
                        ct[255][1] = ct[255][2] = ct[255][3] = 0;
                    }
                    glColorTable( textureTarget, GL_RGBA, 256, GL_RGBA, GL_UNSIGNED_BYTE, ct );
                    displayOpenGLError( "rcImageCanvasGL::buildImageDlist glColorTable:", glGetError() );

                    if ( firstTime ) {
                        glTexImage2D( textureTarget, 0, GL_COLOR_INDEX8_EXT, mWindow.width(), mWindow.height(),
                                      0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, imgData );
                    } else {
                        glTexSubImage2D( textureTarget, 0, 0, 0, mWindow.width(), mWindow.height(),
                                         GL_COLOR_INDEX, GL_UNSIGNED_BYTE, imgData );
                    }
                } else {
                    // No paletted textures available
                    // Note: All 8-bit images are assumed to be gray scale images
                    if ( firstTime ) {
                        glTexImage2D( textureTarget, 0, GL_INTENSITY8, mWindow.width(), mWindow.height(),
                                      0, GL_RED, GL_UNSIGNED_BYTE, imgData );
                    } else {
                        glTexSubImage2D( textureTarget, 0, 0, 0, mWindow.width(), mWindow.height(),
                                         GL_RED, GL_UNSIGNED_BYTE, imgData );
                    }
                }
                if ( !mWindow.isGray() ) {
                    cerr << "rcImageCanvasGL::buildImageDlist() warning: 8-bit color image ";
                    cerr << "displayed as 8-bit gray scale image" << endl;
                }
                break;

            case 32:
                if ( firstTime ) {
                    glTexImage2D( textureTarget, 0, GL_RGBA, mWindow.width(), mWindow.height(),
                                  0, GL_BGRA, ARGB_IMAGE_TYPE, imgData );
                } else {
                    glTexSubImage2D( textureTarget, 0, 0, 0, mWindow.width(), mWindow.height(),
                                     GL_BGRA, ARGB_IMAGE_TYPE, imgData);
                }
                break;

            case 16:
	      glPixelStorei(GL_UNPACK_SWAP_BYTES, true);
                if ( firstTime ) {
                    glTexImage2D( textureTarget, 0, GL_INTENSITY8, mWindow.width(), mWindow.height(),
                                  0, GL_LUMINANCE, GL_UNSIGNED_SHORT, imgData );
                } else {
                    glTexSubImage2D( textureTarget, 0, 0, 0, mWindow.width(), mWindow.height(),
                                     GL_LUMINANCE, GL_UNSIGNED_SHORT, imgData);
                }
                break;

            default:
                // Unsupported depth
                cerr << "rcImageCanvasGL::buildImageDlist() error: unsupported depth " << rawDepth << endl;
                rmAssert( 0 );
                break;
        }
        if ( firstTime )
            displayOpenGLError( "rcImageCanvasGL::buildImageDlist glTexImage2D:", glGetError() );
        else
            displayOpenGLError( "rcImageCanvasGL::buildImageDlist glTexSubImage2D:", glGetError() );

        // Map image to a rectangle
        glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0-mMaxWorldCoord.y());	              glVertex2f(mMinRatioWorldCoord.x(), mMinWorldCoord.y());
        glTexCoord2f(mMaxWorldCoord.x(), 1.0-mMaxWorldCoord.y()); glVertex2f(mMaxRatioWorldCoord.x(), mMinWorldCoord.y());
        glTexCoord2f(mMaxWorldCoord.x(), 1.0);		              glVertex2f(mMaxRatioWorldCoord.x(), mMaxWorldCoord.y());
        glTexCoord2f(0.0, 1.0);		                              glVertex2f(mMinRatioWorldCoord.x(), mMaxWorldCoord.y());
        glEnd();
        glFlush();
        glDisable( textureTarget);

        glPopAttrib();
        glPopMatrix();
        glEndList();
        return imgObj;
    } else
        return 0;
}

void rcImageCanvasGL::deleteGlobalGraphicsDlist()
{
    // Delete old display list
    if ( mGlobalGraphicsDlist ) {
        makeCurrent();
        glDeleteLists( mGlobalGraphicsDlist, 1 );
        mGlobalGraphicsDlist = 0;
    }
}

void rcImageCanvasGL::deleteCellGraphicsDlist()
{
    // Delete old display list
    if ( mCellGraphicsDlist ) {
        makeCurrent();
        glDeleteLists( mCellGraphicsDlist, 1 );
        mCellGraphicsDlist = 0;
    }
}

GLuint rcImageCanvasGL::buildCellGraphicsDlist()
{
    // Delete old list if it exists
    deleteCellGraphicsDlist();

    if ( !mCellGraphics.empty() ) {
        GLuint gObj = glGenLists(1);
        glNewList( gObj, GL_COMPILE );
        glPushMatrix();
        glPushAttrib( GL_CURRENT_BIT );
        glMatrixMode( GL_MODELVIEW );

        // Draw graphics
        draw( mCellGraphics );

        glPopAttrib();
        glPopMatrix();
        glEndList();

        return gObj;
    } else
        return 0;
}

GLuint rcImageCanvasGL::buildGlobalGraphicsDlist()
{
  // Delete old list if it exists
  deleteGlobalGraphicsDlist();

  if ( !mGlobalGraphics.empty() ) {
    GLuint gObj = glGenLists(1);
    glNewList( gObj, GL_COMPILE );
    glPushMatrix();
    glPushAttrib(  GL_CURRENT_BIT ); // @note depth processing ( GL_CURRENT_BIT );
    glMatrixMode( GL_MODELVIEW );

    // Draw graphics
    draw( mGlobalGraphics );

    glPopAttrib();
    glPopMatrix();
    glEndList();

    return gObj;
  } else
    return 0;
}

// Reset current matrix and apply all model transforms
void rcImageCanvasGL::modelTransform()
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    modelTransformCurrent();
}

// Apply all model transforms to current matrix
void rcImageCanvasGL::modelTransformCurrent()
{
    // Scale
    // Set X scaling factor depending on viewport size so that aspect ratio is retained
    GLfloat scaleX = mScale * size().height()/(GLfloat)size().width();
    glScalef( scaleX, mScale, mScale );

    // Translate
    mX = mWorldPanCoord.x();
    mY = mWorldPanCoord.y();
    mZ = 0.0;
    glTranslatef( mX, mY, mZ );
}

// Apply rotation + all model transforms
void rcImageCanvasGL::modelRotate( const rc2Fvector& pivot )
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // Scale
    // Set X scaling factor depending on viewport size so that aspect ratio is retained
    GLfloat scaleX = mScale * size().height()/(GLfloat)size().width();
    glScalef( scaleX, mScale, mScale );

    mX = mWorldPanCoord.x();
    mY = mWorldPanCoord.y();
    mZ = 0.0;

    // Translate
    glTranslatef( pivot.x() + mX, pivot.y() + mY, mZ );

    // Rotate around origin
    glRotatef( mRot, 0.0, 0.0, 1.0 );
}

// Reset all model transforms
void rcImageCanvasGL::resetTransform()
{
    mScale = 1.0;
    mRot = 0.0f;
    mDevCursorCoord = QPoint( 0, 0 );
    mDevClickCoord = QPoint( 0, 0 );
    mWorldPanCoord = rc2Fvector( 0, 0 );
    modelTransform();
}

// Apply translate to current matrix
void rcImageCanvasGL::panCurrent( const rc2Fvector& translate )
{
    glMatrixMode( GL_MODELVIEW );
    // Pan
    glTranslatef( translate.x(), translate.y(), mZ );
    // Update pan coord
    mWorldPanCoord += translate;
}

void rcImageCanvasGL::setViewport( const QSize& hint )
{
    int newX = 0, newY = 0;
    // Set viewport
    //cerr << "viewport " << newX << "," << newY << "," <<  hint.width() << "," <<  hint.height() << endl;
    glViewport( newX, newY, hint.width(), hint.height() );
}

rc2Fvector rcImageCanvasGL::devToWorldCoord(const QPoint& devPos, bool correctRatio ) const
{
    GLint viewport[4];
    GLdouble mvmatrix[16], projmatrix[16];
    GLint realy;          /*  OpenGL y coordinate position  */
    GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
    int x = devPos.x();
    int y = devPos.y();

    glGetIntegerv (GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
    /*  note viewport[3] is height of window in pixels  */
    realy = viewport[3] - (GLint) y - 1;
    gluUnProject ((GLdouble) x, (GLdouble) realy, 0.0,
                  mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
    // Undo texture ratio correction
    if ( correctRatio )
        wx = wx / mRatio;
    return rc2Fvector(static_cast<float>(wx),static_cast<float>(wy));
}

QPoint rcImageCanvasGL::worldToDevCoord(const rc2Fvector& worldPos, bool correctRatio ) const
{
    GLint viewport[4];
    GLdouble mvmatrix[16], projmatrix[16];
    GLdouble wx, wy, wz;  /*  returned window x, y, z coords  */
    GLdouble x = worldPos.x();
    GLdouble y = worldPos.y();

    glGetIntegerv (GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);

    gluProject ( x, y, 0.0,
                 mvmatrix, projmatrix, viewport, &wx, &wy, &wz);
    // Undo texture ratio correction
    if ( correctRatio )
        wx = wx * mRatio;
    //cerr << "worldToDevCoord " << worldPos << " to " << wx << " " << wy << " " << wz << endl;
    return QPoint(static_cast<int>(wx),static_cast<int>(wy));
}

rc2Fvector rcImageCanvasGL::worldToPixel( const rc2Fvector& worldPos ) const
{
    // TODO: use OpenGL matrices to calculate this

    // Map from [-1.0,1.0] range to image pixels
    double x = worldPos.x();
    double y = worldPos.y();

    // Target coordinate limits
    const double minTargetX = mMinModelCoord.x();
    const double minTargetY = mMinModelCoord.y();
    const double maxTargetX = mMaxModelCoord.x();
    const double maxTargetY = mMaxModelCoord.y();
    // Source coordinate limits
    const double minSourceX = mMinWorldCoord.x();
    const double maxSourceX = mMaxWorldCoord.x();
    const double minSourceY = mMinWorldCoord.y();
    const double maxSourceY = mMaxWorldCoord.y();

    float pixX = float(minTargetX + ((maxTargetX - minTargetX) * (x - minSourceX) / (maxSourceX - minSourceX)));
    float pixY = float(maxTargetY - ((maxTargetY - minTargetY) * (y - minSourceY) / (maxSourceY - minSourceY)));

    return rc2Fvector( pixX, pixY );
}

// Transfrom model coords to device coords
QPoint rcImageCanvasGL::modelToDevCoord( const rc2Fvector& mp ) const
{
    GLint viewport[4];
    glGetIntegerv (GL_VIEWPORT, viewport);

    QPoint dp = worldToDevCoord( modelToWorldCoord( mp ) );
    // Reverse Y coordinate
    /*  note viewport[3] is height of window in pixels  */
    dp.setY( viewport[3] - dp.y() - 1 );
    return dp;
}

// Transfrom model coords to world coords
rc2Fvector rcImageCanvasGL::modelToWorldCoord( const rc2Fvector& mp ) const
{
    // Map from image coords to world coords
    const double x = mp.x();
    const double y = mp.y();

    // X scaling factor depends on viewport size so that
    // aspect ratio is retained
    const double minDevX = mMinRatioWorldCoord.x();
    const double maxDevX = mMaxRatioWorldCoord.x();
    const double minDevY = mMinWorldCoord.y();
    const double maxDevY = mMaxWorldCoord.y();

    const double minValueX = mMinModelCoord.x();
    const double minValueY = mMinModelCoord.y();
    const double maxValueX = mMaxModelCoord.x();
    const double maxValueY = mMaxModelCoord.y();

    double wX = (minDevX + ((maxDevX - minDevX) * (x - minValueX) / (maxValueX - minValueX)));
    double wY = (maxDevY - ((maxDevY - minDevY) * (y - minValueY) / (maxValueY - minValueY)));

    return rc2Fvector( static_cast<float>(wX), static_cast<float>(wY) );
}

// Transfrom device coords to model coords
rc2Fvector rcImageCanvasGL::devToModelCoord( const QPoint& dp ) const
{
    return worldToPixel( devToWorldCoord( dp ) );
}

// Single pixel size in world coordinates
rc2Fvector rcImageCanvasGL::worldPixelSize() const
{
    const float w = (mMaxRatioWorldCoord.x() - mMinRatioWorldCoord.x())/mMaxModelCoord.x();
    const float h = (mMaxRatioWorldCoord.y() - mMinRatioWorldCoord.y())/mMaxModelCoord.y();

    return rc2Fvector( w, h );
}

// View size in dev coordinates
QSize rcImageCanvasGL::devViewSize() const
{
    // Width
    rc2Fvector l( mMinWorldCoord.x(), mMinWorldCoord.y() );
    rc2Fvector r( mMaxWorldCoord.x(), mMaxWorldCoord.y() );

    QPoint left = worldToDevCoord( l );
    QPoint right = worldToDevCoord( r );
    int viewWidth = right.x() - left.x();
    if ( viewWidth > width() )
        viewWidth = width();
    // Height
    int viewHeight = right.y() - left.y();
    if ( viewHeight > height() )
        viewHeight = height();

    return QSize( viewWidth, viewHeight );
}

// Return rc parent widget
rcMonitor* rcImageCanvasGL::rcParent() const
{
    QWidget* parent = parentWidget();
    rcMonitor* imageWidget = dynamic_cast< rcMonitor*>(parent);

    return imageWidget;
}

// Get (expanded) texture size
QSize  rcImageCanvasGL::textureSize( const QSize& rawImageSize ) const
{
    // Texture map image size must be 2^n
    // 1E-3 needed. Just try with width=128 and see !
    const int newWidth  = 1<<(int)(1+log(rawImageSize.width() -1+1E-3) / log(2.0));
    const int newHeight = 1<<(int)(1+log(rawImageSize.height()-1+1E-3) / log(2.0));

    return QSize( newWidth, newHeight );
}

// Regenerate image display list using new data
bool rcImageCanvasGL::reconvertImage( bool deleteTexture )
{
    bool success = true;

    if ( mImageSize == QSize(0,0) )
        return FALSE;

    mMaxModelCoord = rc2Fvector( mImageSize.width(), mImageSize.height() );
    mMaxScaleFactor = cMaxScaleFactor;

    // Texture map image size must be 2^n
    float umax = mImageSize.width()  / (float)mTextureSize.width();
    float vmax = mImageSize.height() / (float)mTextureSize.height();
    mRatio = mTextureSize.width() / (float) mTextureSize.height();
    mMinWorldCoord = rc2Fvector( -umax, -vmax );
    mMaxWorldCoord = rc2Fvector( umax, vmax );
    mMinRatioWorldCoord = rc2Fvector( mMinWorldCoord.x()*mRatio, mMinWorldCoord.y() );
    mMaxRatioWorldCoord = rc2Fvector( mMaxWorldCoord.x()*mRatio, mMaxWorldCoord.y() );

    // Regenerate image display list
    mImageDlist = buildImageDlist( deleteTexture );

    return success;				// TRUE if loaded OK
}


// Canvas center in world coordinates
rc2Fvector rcImageCanvasGL::canvasCenterCoord() const
{
    QPoint canvasCenter(width()/2, height()/2);
    return devToWorldCoord( canvasCenter, false );
}

// Return a scale that meets min/max requirements
double rcImageCanvasGL::checkScale( double scaleCandidate ) const
{
    double acceptedScale = scaleCandidate;

    if ( scaleCandidate < cMinScaleFactor ) {
        acceptedScale = cMinScaleFactor;
        printf( "Scaling factor %.2f throttled to %.2f\n", scaleCandidate, acceptedScale );
    } else if ( scaleCandidate > cMaxScaleFactor ) {
        acceptedScale = cMaxScaleFactor;
        printf( "Scaling factor %.2f throttled to %.2f\n", scaleCandidate, acceptedScale );
    } else if ( scaleCandidate > mMaxScaleFactor ) {
        acceptedScale = mMaxScaleFactor;
        printf( "Scaling factor %.2f throttled to %.2f\n", scaleCandidate, acceptedScale );
    }

    return acceptedScale;
}

// Return a pan that meets min/max requirements
QPoint rcImageCanvasGL::checkPan( const QPoint& panOffset ) const
{
    QPoint acceptedPan( panOffset );
    // TODO: implement some reasonable limits

    return acceptedPan;
}

// Draw lines to graphics display list
void rcImageCanvasGL::draw( const rcVisualGraphicsCollectionCollection& graphics, bool pushId )
{
  displayOpenGLError( "rcImageCanvasGL::draw rcGraphicsCollectionCollection:",
		      glGetError() );


  if ( !graphics.empty() ) {

    displayOpenGLError( "rcImageCanvasGL::draw glBegin:",
			glGetError() );
    rcVisualGraphicsCollectionCollection::const_iterator g;

    for ( g = graphics.begin(); g < graphics.end(); g++ )
      {
	if (g->segments().empty())
	  continue;

	draw (g);
#if 0
	// @note id() < 0 indicates this is not a selection collection
	if (g->id() >= 0)
	  {
	    if (g->id() != mIndex)
	      {
		glPushAttrib( GL_CURRENT_BIT );
		glPolygonOffset( -1.f, -1.f );
		glEnable( GL_POLYGON_OFFSET_LINE );
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		draw (g, pushId);
		glPopAttrib();
	      }

	    if (g->id() == mIndex)
	      {
		// If the object is selected, use polygon offset to draw it in
		//   wireframe, thus "highlighting" the object of interest.
		glPushAttrib( GL_CURRENT_BIT );
		glPolygonOffset( -1.f, -1.f );
		glEnable( GL_POLYGON_OFFSET_FILL );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		draw (g, pushId);
		glPopAttrib();
	      }
	  }
	else
	  {
	    draw (g);
	  }
#endif
      }
  }
}


void rcImageCanvasGL::draw( rcVisualGraphicsCollectionCollection::const_iterator g, bool pushId)
{
  const rcVisualSegmentCollection& lines = g->segments();
  if ( lines.empty() )
    return;

  // Get default style from collection
  rcStyle style = g->style();
  setStyle( style );

  rcVisualSegmentCollection::const_iterator line;
  rcVisualSegment::rcVisualSegmentType prevType = rcVisualSegment::eUnknown;

  if (pushId && g->id() >= 0)
    {
      glPushName (g->id ());
    }

  // Iterate all lines
  for ( line = lines.begin(); line != lines.end(); line++ )
    {
    rcVisualSegment::rcVisualSegmentType type = line->type();

    switch ( type ) {
    case rcVisualSegment::eEmpty:
      // Empty filler element, do nothing
      break;

    case rcVisualSegment::eStyle:
      // Specify drawing style
      {
	const rcVisualSegment& l = *line;
	// Warning: explicit static cast
	const rcVisualStyle& s = static_cast<const rcVisualStyle&>(l);
	// Change current style
	style.color( s.color() );
	style.lineWidth( s.lineWidth() );
	style.pixelOrigin( s.pixelOrigin() );
	setStyle( style );
      }
      break;

    case rcVisualSegment::ePoints:
      glBegin( GL_POINTS );
      draw( *line, style );
      glEnd();
      break;

    case rcVisualSegment::eLine:
    case rcVisualSegment::eArrow:
    case rcVisualSegment::eCross:
      glBegin( GL_LINES );
      draw( *line, style );
      glEnd();
      break;

    case rcVisualSegment::eLineStrip:
      {
	rcVisualSegmentCollection::const_iterator nextLine = line+1;
	rcVisualSegment::rcVisualSegmentType nextType = rcVisualSegment::eUnknown;
	if ( nextLine != lines.end() )
	  nextType = nextLine->type();

	if ( prevType != type )
	  glBegin( GL_LINE_STRIP );
	draw( *line, style );
	if ( nextType != type )
	  glEnd();
      }
      break;

    case rcVisualSegment::eLineLoop:
      {
	rcVisualSegmentCollection::const_iterator nextLine = line+1;
	rcVisualSegment::rcVisualSegmentType nextType = rcVisualSegment::eUnknown;
	if ( nextLine != lines.end() )
	  nextType = nextLine->type();

	if ( prevType != type )
	  glBegin( GL_LINE_LOOP );
	draw( *line, style );
	if ( nextType != type )
	  glEnd();
      }
      break;

    case rcVisualSegment::eRect:
      glBegin( GL_LINE_LOOP );
      draw( *line, style );
      glEnd();
      break;

    case rcVisualSegment::eEllipse:
      glBegin( GL_LINE_LOOP );
      draw( *line, style );
      glEnd();
      break;

    case rcVisualSegment::eUnknown:
    case rcVisualSegment::eLast:
      rmAssert( 0 );
      break;
    }
    prevType = type;
  }

  if (pushId && g->id() >= 0)
    {
      glPopName ();
    }

}


// Draw graphics to scaled pixmap which has been already scaled with matrix
void rcImageCanvasGL::draw( const rcVisualSegment& g, const rcStyle& style )
{
    const rc2Fvector pixelOrigin = style.pixelOrigin();

    // Points are in model coordinates, scale them
    // and correct for pixel origin
    const rc2Fvector p1 = g.p1() + pixelOrigin;
    const rc2Fvector p2 = g.p2() + pixelOrigin;
    const rc2Fvector x1 = modelToWorldCoord( p1 );
    const rc2Fvector x2 = modelToWorldCoord( p2 );
    const rcVisualSegment::rcVisualSegmentType type = g.type();

    switch ( type ) {
        case rcVisualSegment::eArrow:
        {
            rc2Fvector x12 (x2 - x1);
            if ( !x12.isNull() ) {
                const float len = x12.len();
                const rcRadian rad = x12.angle();
                const float tip (len * 0.25);

                // Draw stem
                glVertex2f( x1.x(), x1.y() );
                glVertex2f( x2.x(), x2.y() );

                // Draw tip barb 1
                const rc2Fvector arrow (tip, rad - cArrowTipAngle);
                glVertex2f( x2.x(), x2.y() );
                glVertex2f( x2.x() + arrow.x(), x2.y() + arrow.y() );
                // Draw tip barb 2
                const rc2Fvector arrow2 = rc2Fvector(tip, rad + cArrowTipAngle);
                glVertex2f( x2.x(), x2.y() );
                glVertex2f( x2.x() + arrow2.x(), x2.y() + arrow2.y() );
            }
        }
        break;

        case rcVisualSegment::ePoints:
        case rcVisualSegment::eLine:
        case rcVisualSegment::eLineStrip:
        case rcVisualSegment::eLineLoop:
            glVertex2f( x1.x(), x1.y() );
            glVertex2f( x2.x(), x2.y() );
            break;

        case rcVisualSegment::eRect:
            glVertex2f( x1.x(), x1.y() );
            glVertex2f( x2.x(), x1.y() );
            glVertex2f( x2.x(), x2.y() );
            glVertex2f( x1.x(), x2.y() );
            break;

        case rcVisualSegment::eEllipse:
        {
            const rc2Fvector x3 = modelToWorldCoord( p2+p1 );
            const rc2Fvector radius = x3 - x1;
            // Approximate an ellipse with line segments
            for ( uint32 i = 0; i < cCosTable.size(); ++i ) {
                glVertex2f( x1.x() + cCosTable[i]*radius.x(),
                            x1.y() + cSinTable[i]*radius.y());
            }
        }
        break;

        case rcVisualSegment::eCross:
        {
            const rc2Fvector x3 = modelToWorldCoord( p2+p1 );
            const rc2Fvector size = x3 - x1;
            const float x = size.x()/2;
            const float y = size.y()/2;
            // Horizontal line
            glVertex2f( x1.x()-x, x1.y() );
            glVertex2f( x1.x()+x, x1.y() );
            // Vertical line
            glVertex2f( x1.x(), x1.y()-y );
            glVertex2f( x1.x(), x1.y()+y );
        }
        break;

        case rcVisualSegment::eEmpty:
        case rcVisualSegment::eStyle:
        case rcVisualSegment::eUnknown:
        case rcVisualSegment::eLast:
            rmAssert( 0 );
            break;
    }
}

// Draw text to current context
void rcImageCanvasGL::draw( const rcTextSegment& text, const rc2Fvector& pixelOrigin )
{
    // Points are in model coordinates, scale them
    // and correct for pixel origin
    const rc2Fvector& p1 = text.pos() + pixelOrigin;
    const rc2Fvector x1 = modelToWorldCoord( p1 );

    const std::string& s = text.text();
    renderText( x1.x(), x1.y(), 0.0, QStrFrcStr ( s ), mTextFont );
}

// Draw text to current context
void rcImageCanvasGL::draw( const rcTextCollection& texts )
{
    if ( !texts.empty() ) {
        glPushMatrix();
        glPushAttrib( GL_CURRENT_BIT );

        // Each collection has its own style
        const rcStyle& style = texts.style();
        setStyle( style );

        const rcTextSegmentCollection& text = texts.texts();
        const rc2Fvector pixelOrigin = style.pixelOrigin();
        rcTextSegmentCollection::const_iterator t;

        // Iterate all texts
        for ( t = text.begin(); t < text.end(); ++t ) {
            draw( *t, pixelOrigin );
        }
        displayOpenGLError( "rcImageCanvasGL::paintGL draw rcTextCollection:",
                            glGetError() );
        glPopAttrib();
        glPopMatrix();
    }
}

// TODO: minimum size should be calculated in screen pixels
#define CORNER_MARKER 1

// Draw rect with rotation around top-left point
void rcImageCanvasGL::drawModelRect( const rc2Fvector& x1, const rc2Fvector& x2 )
{
    const rc2Fvector canvasCenter( (mMaxModelCoord.x() - mMinModelCoord.x())/2,
                                   (mMaxModelCoord.y() - mMinModelCoord.y())/2 );
    const rc2Fvector orig = modelToWorldCoord( x1 );
    const rc2Fvector size( x2.x() - x1.x(), x2.y() - x1.y() );

#ifdef CORNER_MARKER
    int32 markerSize = 5;
    // Marker has to less than half of width/height to be useful
    const rcIPair maxMarkerSize( static_cast<int32>(rmABS(size.x()/2-1)),
                                 static_cast<int32>(rmABS(size.y()/2-1)) );

    if ( markerSize > maxMarkerSize.x() )
        markerSize = maxMarkerSize.x();
    if ( markerSize >  maxMarkerSize.y() )
        markerSize = maxMarkerSize.y();

    const rc2Fvector originMarker1( canvasCenter.x() + markerSize, canvasCenter.y() );
    const rc2Fvector originMarker2( canvasCenter.x(), canvasCenter.y() + markerSize );
    const rc2Fvector om1 = modelToWorldCoord( originMarker1 );
    const rc2Fvector om2 = modelToWorldCoord( originMarker2 );
#endif

    // Draw rect at world origin because it will be rotated at origin
    const rc2Fvector ul = modelToWorldCoord( canvasCenter );
    const rc2Fvector lr = modelToWorldCoord( canvasCenter + size );
    const rc2Fvector origOffset = orig - ul;

    // Rotate and translate back to original location
    modelRotate( origOffset );

    glBegin(GL_LINE_LOOP);			// Draw Box
    glVertex2f( ul.x(), ul.y() );	// Top left
    glVertex2f( lr.x(), ul.y() );	// Top right
    glVertex2f( lr.x(), lr.y() );	// Bottom right
    glVertex2f( ul.x(), lr.y() );	// Bottom left
    glEnd();
#ifdef CORNER_MARKER
    glBegin(GL_LINE_STRIP);	        // Draw marker to indicate top left
    glVertex2f( om1.x(), om1.y() );	// Origin marker
    glVertex2f( om2.x(), om2.y() );	// Origin marker
    glEnd();
#endif

#ifdef DEBUG_LOG2
    cerr << "x1 " << x1 << " x2 " << x2 << "\tul " << ul << " lr " << lr << " canvasCenter " << canvasCenter;
    cerr << " origOffset " << origOffset << endl;
#endif
}

// Draw zoom rect rubber band
void rcImageCanvasGL::drawZoomRect( const QPoint& upperLeft, const QPoint& lowerRight )
{
    drawRubberBandRect( upperLeft, lowerRight, true );
}

// Draw focus rect rubber band
void rcImageCanvasGL::drawFocusRect( const QPoint& upperLeft, const QPoint& lowerRight )
{
    drawRubberBandRect( upperLeft, lowerRight, false );
}

// Draw zoom rect rubber band
void rcImageCanvasGL::drawRubberBandRect( const QPoint& upperLeft, const QPoint& lowerRight,
                                          bool zoom )
{
    makeCurrent();

    QRect devRect( upperLeft, lowerRight );
    // Rectangle must be normalized
    devRect = devRect.normalize();

    updateGL();
    GLerrorNUM("drawRubberBandRect");
    glPushMatrix();
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_XOR);
    glLogicOp(GL_COPY);

    if ( zoom ) {
        glEnable(GL_LINE_STIPPLE);
	glLineStipple (1, 0x00FF);
	  //        glLineStipple(2, 0xAAAA);
    }

    glLineWidth(1.0f);
    rubberBandColor;

    rc2Fvector x1 = devToModelCoord( devRect.topLeft() );
    rc2Fvector x2 = devToModelCoord( devRect.bottomRight() );

    if ( zoom )
      {
        // Draw unrotated rect
        rc2Fvector ul = modelToWorldCoord( x1 );
        rc2Fvector lr = modelToWorldCoord( x2 );

        glBegin(GL_LINE_LOOP);			// Draw Box
        glVertex2f( ul.x(), ul.y() );	// Top left
        glVertex2f( lr.x(), ul.y() );	// Top right
        glVertex2f( lr.x(), lr.y() );	// Bottom right
        glVertex2f( ul.x(), lr.y() );	// Bottom left
        glEnd();
    } else
      {
        // Draw rotated rect
        drawModelRect( x1, x2 );
      }

    if ( zoom )
        glDisable(GL_LINE_STIPPLE);
    glDisable(GL_COLOR_LOGIC_OP);
    glPopMatrix();
    displayOpenGLError( "rcImageCanvasGL::drawRubberBandRect:",
                        glGetError() );
}

// Draw Length Line
void rcImageCanvasGL::drawAffineRect ( const QPoint& endPoint, const QPoint& anchor,
				       bool zoom )

{
  rmUnused(zoom);
  makeCurrent();
  // set angle to angle connecting anchor to end point
  // angle (positive x to positive y
  rc2Dvector ep ((double) endPoint.x(), (double) endPoint.y());
  rc2Dvector anch ((double) anchor.x(), (double) anchor.y());
  rcAffineRectangle p (anch, ep, sOtherDimension, sMidOrigin);
  rc2Fvector ul = p.affineToImage(rc2Fvector(0, 0));
  rc2Fvector ur = p.affineToImage(rc2Fvector(1, 0));
  rc2Fvector ll = p.affineToImage(rc2Fvector(0, 1));
  rc2Fvector lr = p.affineToImage(rc2Fvector(1, 1));

  updateGL();
  GLerrorNUM("drawAffineRect");
  glPushMatrix();
  glEnable(GL_COLOR_LOGIC_OP);
  glLogicOp(GL_XOR);
  glLogicOp(GL_COPY);

  glEnable(GL_LINE_STIPPLE);
  glLineStipple (1, 0x00FF);
  //        glLineStipple(2, 0xAAAA);

  glLineWidth(3.0f);
  rubberBandColor;

  ul = modelToWorldCoord( devToModelCoord( QPoint( static_cast<int>(ul.x()), static_cast<int>(ul.y()))));
  ur = modelToWorldCoord( devToModelCoord(QPoint( static_cast<int>(ur.x()), static_cast<int>(ur.y()))));
  lr = modelToWorldCoord( devToModelCoord(QPoint( static_cast<int>(lr.x()), static_cast<int>(lr.y()))));
  ll = modelToWorldCoord( devToModelCoord( QPoint( static_cast<int>(ll.x()), static_cast<int>(ll.y()))));


  glBegin(GL_LINES);			// Draw Box
  glVertex2f( ul.x(), ul.y() );	// Top left
  glVertex2f( ur.x(), ur.y() );	// Top left
  glVertex2f( ur.x(), ur.y() );	// Top left
  glVertex2f( lr.x(), lr.y() );	// Bottom left
  glVertex2f( lr.x(), lr.y() );	// Bottom left
  glVertex2f( ll.x(), ll.y() );	// Bottom left
  glVertex2f( ll.x(), ll.y() );	// Bottom left
  glVertex2f( ul.x(), ul.y() );	// Top left
  glEnd();

  glDisable(GL_LINE_STIPPLE);
  glDisable(GL_COLOR_LOGIC_OP);
  glPopMatrix();
  displayOpenGLError( "rcImageCanvasGL::drawAffineRect:",
		      glGetError() );
}

// Draw current focus area
void rcImageCanvasGL::drawCurrentFocus()
{
    if ( mFocusRect.width() > 0 && mFocusRect.height() > 0 ) {
        glPushMatrix();

        glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_XOR);
        glLogicOp(GL_COPY);

        glLineWidth(1.0f);
	rubberBandColor;

        rc2Fvector x1( float(mFocusRect.x()), float(mFocusRect.y()) );
        rc2Fvector x2( float(mFocusRect.x() + mFocusRect.width()),
                       float(mFocusRect.y() + mFocusRect.height()) );
        // Draw rect with rotation
        drawModelRect( x1, x2 );

        glDisable(GL_COLOR_LOGIC_OP);
        glPopMatrix();
	GLerrorNUM("drawCurrentFocus");
    }

    // Draw Polygons within this focus

  rcModelDomain* domain = rcModelDomain::getModelDomain();
  //    @note move to its own  add user polys
  rcVisualGraphicsCollectionCollection polyg;
  for (uint32 i = 0; i < domain->polys().size(); i++)
    {
      rcStyle pickCell(rfRgb(0, 128, 250), 5, rc2Fvector(0.0f, 0.0f));
      rcVisualSegmentCollection lines;
      rfPolygonToSegmentsCollection (domain->polys().polys()[i], lines);
      rcVisualGraphicsCollection boo (pickCell, lines);
      boo.id ((int32) i);
      polyg.push_back (boo);
    }

  if (polyg.size()) draw (polyg, 0);

}

// Convert mouse event coordinates
bool rcImageCanvasGL::convertEvent( QMouseEvent* e, QPoint* modelPoint )
{
    if ( mImageSize != QSize( 0, 0 ) ) {
        QPoint mousePoint = e->pos();
        mDevCursorCoord = mousePoint;
        if ( modelPoint ) {
            rc2Fvector p = devToModelCoord( mousePoint );
            *modelPoint = QPoint( static_cast<int>(p.x()), static_cast<int>(p.y()) );
        }
        updateStatus();
        return TRUE;
    }

    return FALSE;
}

double rcImageCanvasGL::scaleChanged()
{
    return scaleChanged( mScale );
}

void rcImageCanvasGL::updateStatus()
{
    rcParent()->updateStatus();
}

// Special cursors
// TODO: implement custom bitmaps for cursors

QCursor rcImageCanvasGL::zoomInCursor() const
{
    return QCursor( Qt::PointingHandCursor );
}

QCursor rcImageCanvasGL::zoomOutCursor() const
{
    return QCursor( Qt::PointingHandCursor );
}

QCursor rcImageCanvasGL::panCursor() const
{
    return QCursor( Qt::UpArrowCursor );
}

// Zoom/pan event detection
bool rcImageCanvasGL::zoomKeyPressed( QMouseEvent* e )
{
    return ( zoomInKeyPressed( e ) || zoomOutKeyPressed( e ) );
}

bool rcImageCanvasGL::zoomInKeyPressed( QMouseEvent* e )
{
    return ( e->state() & Qt::ControlButton );
}

bool rcImageCanvasGL::zoomOutKeyPressed( QMouseEvent* e )
{
    return ( e->state() & Qt::AltButton );
}

bool rcImageCanvasGL::panKeyPressed( QMouseEvent* e )
{
    return (  e->state() & Qt::ShiftButton );
}

// Pan to a dev point
bool rcImageCanvasGL::panToPoint( const QPoint& point )
{
    bool panned = true;

    rc2Fvector worldCursorCoord = devToWorldCoord( point, false );
    rc2Fvector worldCenterCoord = canvasCenterCoord();
    rc2Fvector worldDiff = worldCursorCoord - worldCenterCoord;
    // Pan
    panCurrent( -worldDiff );

    return panned;
}

// Zoom to fit rect to screen
bool rcImageCanvasGL::zoomToRect( const QRect& zoomRect,
                                  const QSize& minSize )
{
    bool zoomed = false;

    // Zoom rect is in device coordinates
    if ( zoomRect.width() > minSize.width() && zoomRect.height() > minSize.height() ) {
        // Zoom to a rect
        // Pan to rect center
        panToPoint( QPoint( zoomRect.x() + zoomRect.width()/2,
                            zoomRect.y() + zoomRect.height()/2 ) );
        // Compute size of visible view area
        QSize viewSize = devViewSize();

        // Zoom to fill viewport
        double scaleX = mScale * viewSize.width() / zoomRect.width();
        double scaleY = mScale * viewSize.height() / zoomRect.height();
        // Choose smallest scale to guarantee a fit
        scaleChanged(rmMin (scaleX, scaleY));
        zoomed = true;
    }

    return zoomed;
}

// Update tooltip for widget
void rcImageCanvasGL::updateToolTip( const QString& newText )
{
            QString oldText = toolTip ();

    // Update tip only if text has changed
    if ( oldText != newText ) {
        QToolTip::remove( this );
        QToolTip::add( this, newText );
    }
}

// Add default canvas text
void rcImageCanvasGL::addDefaultText()
{
    // Empty image text
    rcStyle textStyle;
    textStyle.color( rfRgb( 0, 0, 0 ) );    // black
    rcTextSegmentCollection textSegmentCollection;

    rc2Fvector pos( float(mMaxModelCoord.x()/2), float(mMaxModelCoord.y()/2) );
    rcTextSegment testText( pos, "No image" );
    textSegmentCollection.push_back( testText );
    mTexts = rcTextCollection( textStyle, textSegmentCollection );
}

// Find cell position tracks which are close to the point
uint32 rcImageCanvasGL::findCellPositionTracks( const QPoint& p, const QRect& focus, deque<rcFocusTrack>& tracks )
{
    rmAssert( tracks.empty() );

    tracks.clear();
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcTimestamp cursorTime = domain->getCursorTime();

    if ( cursorTime != cCursorTimeCurrent )
    {
        rcExperiment* experiment = domain->getExperiment();
        int nTrackGroups = experiment->getNTrackGroups();

        for (int i = 0; i < nTrackGroups; i++)
        {
            rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
            if ( trackGroup->getType() == eGroupSemanticsBodyMeasurements ) {
                int nTracks = trackGroup->getNTracks();
                for (int j = 0; j < nTracks; j++)
                {
                    rcTrack* track = trackGroup->getTrack( j );

                    if ( track->getTrackType() == ePositionTrack ) {
                        rcPositionTrack* pTrack = dynamic_cast<rcPositionTrack*>( track );
                        if ( pTrack == 0 )
                            continue;

                        rcWriterSemantics semanticType = rcWriterManager::tagType( track->getTag() );
                        if ( semanticType == eWriterBodyPosition ) {
                            rcPositionSegmentIterator* iter = pTrack->getDataSegmentIterator( cursorTime );

                            if ( iter->contains( cursorTime ) ) {
                                // Use actual cell position as comparison point
                                rcFPair pos = iter->getValue();
                                QPoint centerPoint( rfRound(pos.x(),i), rfRound(pos.y(),i) );

                                // Choose tracks near the point
                                if ( !focus.width() )
                                {
                                    rc2Fvector centerModel( pos.x(), pos.y() );
                                    rc2Fvector mouseModel = devToModelCoord( p );
                                    rc2Fvector distDev = centerModel - mouseModel;
				    int32 dist = (int32) distDev.l2 ();
                                    // Mouse has to be pretty close to center for the cell to be picked
                                    // Do comparison in pixels so scale wont affect this
				    // @note use distance square. Distance and distance square are positive.
                                    if ( dist  < cMouseMinL2) {
                                        rcRect rect = iter->getFocus();
                                        // Assign proper values
                                        rcFocusTrack f( i, j , rect, dist, centerPoint );
                                        tracks.push_back( f );
                                    }
                                } else {
                                    // Choose tracks inside focus rect
                                    if ( focus.contains( QPoint( int(pos.x()), int(pos.y()) ) ) ) {
                                        rcRect rect = iter->getFocus();
                                        // Assign proper values
                                        rcFocusTrack f( i, j , rect, 0, centerPoint );
                                        tracks.push_back( f );
                                    }
                                }
                            }
                            delete iter;
                        }
                    }
                }
            }
        }
    }

    return tracks.size();
}

// Return index of cell group closest to point, -1 if no applicable tracks
int32 rcImageCanvasGL::findClosestCellGroup( const QPoint& p, const QRect& focus, QPoint& centerPoint )
{
    deque<rcFocusTrack> tracks;

    if ( findCellPositionTracks( p, focus, tracks ) > 0 ) {
        // Get the track (focus area) with the shortest distance to p
        partial_sort( tracks.begin(), tracks.begin()+1, tracks.end(), focusCompare );

        centerPoint = tracks[0]._centerPoint;
        return tracks[0]._trackGroupNo;
    }

    return -1;
}

// Get tooltip text
bool rcImageCanvasGL::getToolTipText( const QPoint& mousePoint, QString& text )
{
    QPoint centerPoint;
    QRect focus;

    // Change tooltip to show info of closest cell group
    int32 closestGroup = findClosestCellGroup( mousePoint, focus, centerPoint );

    if ( closestGroup >= 0 ) {
        rcModelDomain* domain = rcModelDomain::getModelDomain();
        rcExperiment* experiment = domain->getExperiment();
        rcTrackGroup* trackGroup = experiment->getTrackGroup( closestGroup );
        rcTimestamp cursorTime = domain->getCursorTime();

        // TODO: get position with full float accuracy
        text.sprintf( "%s\nPosition       (%i, %i)",
                      trackGroup->getDescription(),
                      centerPoint.x(), centerPoint.y() );

        if ( trackGroup->getType() == eGroupSemanticsBodyMeasurements ) {
            int nTracks = trackGroup->getNTracks();
            for (int j = 0; j < nTracks; j++)
            {
                rcTrack* track = trackGroup->getTrack( j );
                // Display some selected values
                rcFPair pos;
                double val(0.0);
                QString tip;
                rcWriterSemantics semanticType = rcWriterManager::tagType( track->getTag() );

                // Use XML tag to find appropriate tracks
		if ( semanticType == eWriterBodyMajor ) {
		  if ( getTrackValue( track, cursorTime, pos, val ) ) {
		    tip.sprintf("\nMajor Dimension %.2f",
				val );
		  }
		}
                else if ( semanticType == eWriterBodySpeedDirection ) {
		  if ( getTrackValue( track, cursorTime, pos, val ) ) {
		    tip.sprintf("\nSpeed           %.4f pel/s\nDirection     %.2f deg",
				pos.x(), pos.y() );
		  }
		} else if ( semanticType == eWriterBodyDistance ) {
		  if ( getTrackValue( track, cursorTime, pos, val ) ) {
		    tip.sprintf("\nDistance      %.2f pel",
				val );
		  }
		} else if ( semanticType == eWriterBodyState ) {
		  if ( getTrackValue( track, cursorTime, pos, val ) ) {
		    char valueBuf[ 32 ];
		    rfVisualFunctionStateName ( static_cast<rcVisualFunction::State>(val),
						valueBuf, rmDim(valueBuf));
		    tip.sprintf("\n %s", valueBuf );
		  }
		}
		text += tip;
            }
        }
        return true;
    }

    return false;
}

// Enable cell group(s) closest to mouse point or inside focus rect
bool rcImageCanvasGL::enableCellGroup( const QPoint& mousePoint, const QRect& focus )
{
    deque<rcFocusTrack> tracks;
    rcTrackManager* tmp = rcTrackManager::getTrackManager();

    if ( findCellPositionTracks( mousePoint, focus, tracks ) > 0 ) {
        // Get the track with the shortest distance to mousePoint
        if ( !focus.width() )
            partial_sort( tracks.begin(), tracks.begin()+1, tracks.end(), focusCompare );

        // Disable all other tracks
        tmp->setTrackEnabled( eWriterBodyPosition, false );
        // Enable chosen groups and all their tracks
        for ( uint32 i = 0; i < tracks.size(); ++i ) {
            uint32 group = tracks[i]._trackGroupNo;
            tmp->setTrackGroupEnabled( group, true );
            tmp->setTrackEnabled( group, -1, true );

#ifdef DEBUG_OUTPUT
            rcModelDomain* domain = rcModelDomain::getModelDomain();
            rcExperiment* experiment = domain->getExperiment();
            rcTrackGroup* trackGroup = experiment->getTrackGroup( group );
            QPoint centerPoint = tracks[i]._centerPoint;

            QString text;
            text.sprintf( "%s at (%i,%i)",
                          trackGroup->getDescription(),
                          centerPoint.x(), centerPoint.y() );
            cerr << "Selected " << text.latin1() << endl;
#endif
        }
        return true;
    } else {
        // Nothing selected, disable all tracks
        // TODO: disabling everythinbg breaks cell selection by simple clicking, fix it
        //tmp->setTrackEnabled( eWriterBodyPosition, false );
    }

    return false;
}

// Detect Apple OpenGL extensions
void rcImageCanvasGL::detectAppleExtensions()
{
    if ( ! mExtensionsChecked ) {
        const char* extString = (const char*)glGetString(GL_EXTENSIONS);

        if ( extString ) {
            mExtPalettedTexture = strstr( extString, "EXT_paletted_texture" );
            if ( ! mExtPalettedTexture )
                cerr << "rcImageCanvasGL::detectAppleExtensions warning: OpenGL extension EXT_paletted_texture not available" << endl;

            mExtAppleStorage = strstr( extString, "GL_APPLE_client_storage" );
            if ( ! mExtAppleStorage )
                cerr << "rcImageCanvasGL::detectAppleExtensions warning: OpenGL extension GL_APPLE_client_storage not available" << endl;
        } else {
            cerr << "rcImageCanvasGL::detectAppleExtensions error: glGetString(GL_EXTENSIONS) returned NULL" << endl;
        }

        mExtensionsChecked = true;
    }
}

// Set OpenGL drawing style
void rcImageCanvasGL::setStyle( const rcStyle& style )
{
    // Specify drawing color
    const uint32 sColor = style.color();
    glColor3ub( rfRed( sColor ), rfGreen( sColor ), rfBlue( sColor ) );
    displayOpenGLError( "rcImageCanvasGL::draw rcGraphicsCollectionCollection glColor3ub:",
                        glGetError() );

    // Specify line width
    glLineWidth( style.lineWidth() );
    displayOpenGLError( "rcImageCanvasGL::draw rcGraphicsCollectionCollection glLineWidth:",
                        glGetError() );

}

// Get track value
bool rcImageCanvasGL::getTrackValue( rcTrack* track, const rcTimestamp& cursorTime,
                                     rcFPair& pos, double& value )
{
    bool ok = false;

    if ( track->getTrackType() == ePositionTrack ) {
        rcPositionTrack* pTrack = dynamic_cast<rcPositionTrack*>( track );
        if ( pTrack ) {
            rcPositionSegmentIterator* iter = pTrack->getDataSegmentIterator( cursorTime );
            if ( iter->contains( cursorTime ) ) {
                pos = iter->getValue();
                ok = true;
            }
            delete iter;
        }
    } else if ( track->getTrackType() == eScalarTrack ) {
        rcScalarTrack* sTrack = dynamic_cast<rcScalarTrack*>( track );
        if ( sTrack ) {
            rcScalarSegmentIterator* iter = sTrack->getDataSegmentIterator( cursorTime );
            if ( iter->contains( cursorTime ) ) {
                value = iter->getValue();
                ok = true;
            }
            delete iter;
        }
    }

    return ok;
}

// See note on top

bool copyImage( const rcWindow& src, const QSize& newSize, rcWindow& dst, bool mirror, bool equalize)
{
    bool copied = false;
    rcWindow tmp;

    // 16 bit but we are display its 8 bit range
    rcPixel dp = (src.depth() == rcPixel16) ? rcPixel8 : src.depth();

    if ( src.isBound() )
      {

	if (src.depth() == rcPixel16)
	  {
	    // 16 bit but we are display its 8 bit range
	    tmp = rcWindow (src.width(), src.height());
	    rfImageConvert168 (src, tmp);
	  }

        if ( !dst.isBound() )
	  {
            // No cached framebuf exists, create a new one
            rcWindow w( newSize.width(), newSize.height(), dp);
            dst = w;
        } else if ( static_cast<int>(dst.width()) < newSize.width() ||
                    static_cast<int>(dst.height()) < newSize.height() )
	  {
            // Cached framebuf is too small, create a bigger one
            rcWindow w( newSize.width(), newSize.height(), dp );
            dst = w;
	  } else if ( !(src.depth() == rcPixel16 && dst.depth() == rcPixel8) && src.depth() != dst.depth() )
	  {
            // Cached framebuf has wrong depth, create a new one
            rcWindow w( newSize.width(), newSize.height(), dp );
            dst = w;
	  }
	rmAssert (dst.depth() != rcPixel16);

        // Copy grayness property
        dst.frameBuf()->setIsGray(dp == rcPixel8 );
        // Copy pixel values, mirror vertically for OpenGL
	if (mirror)
	  {
	    rcIPair delta = dst.size() - src.size();
	    rcWindow dWin (dst, 0, delta.y() < 0 ? 0 : delta.y(),
			   src.width(),src.height());
	    rfImageVerticalReflect (tmp.isBound() ? tmp : src, dWin);
	  }
	else
	  dst.copyPixelsFromWindow(tmp.isBound() ? tmp : src, mirror);


        // Copy color map values
        const rcFrame& srcFrame =  (tmp.isBound()) ? *tmp.frameBuf() : *src.frameBuf();
        uint32 cMapSize = srcFrame.colorMapSize();
	rcFrame& dstFrame =  *dst.frameBuf();
	uint32 csize = dstFrame.colorMapSize();

        rmAssert(csize == cMapSize );

        for ( uint32 i = 0; i < cMapSize; ++i ) {
            dstFrame.setColor( i, srcFrame.getColor( i ) );
        }
        copied = true;
    }

    return copied;
}
