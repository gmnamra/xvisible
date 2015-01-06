
#include "cinder/app/AppBasic.h"
#include "cinder/Camera.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"
#include "ui_contexts.h"


using namespace ci;
using namespace ci::app;
using namespace std;








class VisibleApp : public AppBasic
{
public:
    
    void prepareSettings( Settings *settings );
    void setup();
    void create_matrix_viewer ();
    void create_clip_viewer ();
    void create_qmovie_viewer ();
    
    void mouseDown( MouseEvent event );
    void mouseMove( MouseEvent event );
    void mouseUp( MouseEvent event );
    void mouseDrag( MouseEvent event );
    void keyDown( KeyEvent event );
    
    //void fileDrop( FileDropEvent event );
    void update();
    void draw();
    void close_main();
    void resize();
    void window_close ();
    
      
    bool remove_from (const std::string& );
    
    params::InterfaceGlRef         mTopParams;
    
    Rectf                       mGraphDisplayRect;
    Rectf                       mMovieDisplayRect;
    Rectf                       mMenuDisplayRect;
    Rectf                       mLogDisplayRect;
    CameraPersp				mCam;
    
    std::vector<std::shared_ptr<uContext> > mContexts;
    
    
    
    
};

// The window-specific data for each window
class WindowData {
public:
    WindowData()
    : mColor( Color( CM_HSV, randFloat(), 0.8f, 0.8f ) ) // a random color
    {}
    
    Color			mColor;
    list<Vec2f>		mPoints; // the points drawn into this window
};
using namespace vf_utils::csv;


namespace
{
    Rectf render_window_box ()
    {
        auto pos = AppBasic::get()->getWindow()->getPos ();
        auto size = AppBasic::get()->getWindow()->getSize ();
        return Area (pos, size);
    }
    
    std::ostream& ci_console ()
    {
        return AppBasic::get()->console();
    }
    
    double				ci_getElapsedSeconds()
    {
        return AppBasic::get()->getElapsedSeconds();
    }
    
    uint32 ci_getNumWindows ()
    {
        return AppBasic::get()->getNumWindows();
    }

 
    
    
}



bool VisibleApp::remove_from (const std::string& name)
{
//    bool ok = uContextRegistry::unregister(name);

    return ok;
}


void VisibleApp::window_close()
{
//    uContext* ud = getWindow()->getUserData<uContext>();
//    if (ud) remove_from (ud->name());
    console() << "Closing " << getWindow() << std::endl;
}



void VisibleApp::prepareSettings( Settings *settings )
{
    settings->enableHighDensityDisplay();
}



void VisibleApp::resize ()
{
    const Vec2i& c_ul = getWindowBounds().getUL();
    const Vec2i& c_lr = getWindowBounds().getLR();
    Vec2i c_mr (c_lr.x, (c_lr.y - c_ul.y) / 2);
    Vec2i c_ml (c_ul.x, (c_lr.y - c_ul.y) / 2);
    mGraphDisplayRect = Area (c_ul, c_mr);
    mMovieDisplayRect = Area (c_ml, c_lr);
}

void VisibleApp::create_matrix_viewer ()
{
    mContexts.push_back(std::shared_ptr<uContext>(new matContext(getWindow())));
}
void VisibleApp::create_clip_viewer ()
{
    mContexts.push_back(std::shared_ptr<uContext>(new clipContext(getWindow())));
}
void VisibleApp::create_qmovie_viewer ()
{
    mContexts.push_back(std::shared_ptr<uContext>(new movContext(getWindow())));
}

void VisibleApp::setup()
{
    ci::ThreadSetup threadSetup; // instantiate this if you're talking to Cinder from a secondary thread
    // Setup our default camera, looking down the z-axis
    mCam.lookAt( Vec3f( -20, 0, 0 ), Vec3f::zero() );
    const ColorA &color = ColorA( 0.3f, 0.3f, 0.3f, 0.4f );
    
     // Setup the parameters
	mTopParams = params::InterfaceGl::create( getWindow(), "Select", toPixels( Vec2i( 200, 400)), color );

    mTopParams->addSeparator();
	mTopParams->addButton( "Import Quicktime Movie", std::bind( &VisibleApp::create_qmovie_viewer, this ) );
    mTopParams->addSeparator();
   	mTopParams->addButton( "Import SS Matrix", std::bind( &VisibleApp::create_matrix_viewer, this ) );
    mTopParams->addSeparator();
   	mTopParams->addButton( "Import Result ", std::bind( &VisibleApp::create_clip_viewer, this ) );
    getWindowIndex(0)->connectDraw ( &VisibleApp::draw, this);
    getWindowIndex(0)->connectClose( &VisibleApp::window_close, this);

}



void VisibleApp::mouseMove( MouseEvent event )
{
    //            if (! mWaveformPlot.getWaveforms().empty())
    //              mSigv.at_event (mGraphDisplayRect, event.getPos(), event.getX(),mWaveformPlot.getWaveforms()[0]);
}


void VisibleApp::mouseDrag( MouseEvent event )
{
}


void VisibleApp::mouseDown( MouseEvent event )
{
}


void VisibleApp::mouseUp( MouseEvent event )
{
}


void VisibleApp::keyDown( KeyEvent event )
{
//	if( event.getCode() == KeyEvent::KEY_s ) mSamplePlayer->seekToTime( 1.0 );
}


void VisibleApp::update()
{
    if (mSettings.isResizable() || mSettings.isFullScreen())
    {
        resize ();
    }
    
    WindowRef cw = getWindow();
    
    if (cw != getWindowIndex(0) && cw->getUserData<uContext>())
        cw->getUserData<uContext>()->update ();
    
}

void VisibleApp::draw ()
{
    // this pair of lines is the standard way to clear the screen in OpenGL
    gl::enableDepthRead();
    gl::enableDepthWrite();
    gl::clear( Color::gray( 0.1f ) );
    
    gl::enableAlphaBlending();
    
    gl::clear();
    
    gl::setMatricesWindowPersp( getWindowSize() );

    
    mTopParams->draw ();
    
}


// This line tells Cinder to actually create the application
CINDER_APP_BASIC( VisibleApp, RendererGl )

