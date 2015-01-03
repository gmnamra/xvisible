

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Rect.h"
#include "cinder/qtime/Quicktime.h"
#include "cinder/params/Params.h"
#include "assets/Resources.h"
#include "cinder/Rand.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "ui_contexts.h"
#include "stl_util.hpp"
#include "vf_utils.hpp"
#include "vf_types.h"
#include "vf_cinder.hpp"
#include "iclip.hpp"
#include <map>

#include <functional>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;

//class CVisibleApp;

// Namespace collision with OpenCv

//#define Vec2i ci::Vec2i
//#define Vec2f ci::Vec2f
//#define Vec3f ci::Vec3f





class CVisibleApp : public AppBasic
{
public:
    
    void prepareSettings( Settings *settings );
    void setup();
    void create_matrix_viewer ();
    void create_clip_viewer ();
    void create_qmovie_viewer ();
    
#if 0
    void mouseDown( MouseEvent event );
    void mouseMove( MouseEvent event );
    void mouseUp( MouseEvent event );
    void mouseDrag( MouseEvent event );
    
    void keyDown( KeyEvent event );
#endif
    
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
    
    //   Signal_value                mSigv;
    
    
    
    
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

    template<class V>
    class uContextViewer
    {
    public:
        void operator()()
        {
            //is equivalent to boost::uuids::basic_random_generator<boost::mt19937>
            boost::uuids::random_generator gen;
            boost::uuids::uuid u = gen();
            
            const std::string nam = boost::uuids::to_string (u);
            uContextRegistry::registeruContext<V>(nam);
            if (! uContextRegistry::instantiate(nam) )
            {
                std::cout << "instantiate" << uContextRegistry::size() << std::endl;
                std::cout << "Instantiating matrix viewer failed " << std::endl;
            }
            
        }
    };
    
    
}



bool CVisibleApp::remove_from (const std::string& name)
{
    bool ok = uContextRegistry::unregister(name);

    return ok;
}


void CVisibleApp::window_close()
{
    uContextRegistry::print_out ();
    uContext* ud = getWindow()->getUserData<uContext>();
    if (ud) remove_from (ud->name());
    console() << "Closing " << getWindow() << std::endl;
}



void CVisibleApp::prepareSettings( Settings *settings )
{
    settings->enableHighDensityDisplay();
}



void CVisibleApp::resize ()
{
    const Vec2i& c_ul = getWindowBounds().getUL();
    const Vec2i& c_lr = getWindowBounds().getLR();
    Vec2i c_mr (c_lr.x, (c_lr.y - c_ul.y) / 2);
    Vec2i c_ml (c_ul.x, (c_lr.y - c_ul.y) / 2);
    mGraphDisplayRect = Area (c_ul, c_mr);
    mMovieDisplayRect = Area (c_ml, c_lr);
}

void CVisibleApp::create_matrix_viewer ()
{
    uContextViewer<matContext> () ();
}
void CVisibleApp::create_clip_viewer ()
{
    uContextViewer<clipContext> () ();
}
void CVisibleApp::create_qmovie_viewer ()
{
    uContextViewer<movContext> () ();
}

void CVisibleApp::setup()
{
    // Setup our default camera, looking down the z-axis
    mCam.lookAt( Vec3f( -20, 0, 0 ), Vec3f::zero() );
    const ColorA &color = ColorA( 0.3f, 0.3f, 0.3f, 0.4f );
    
     // Setup the parameters
	mTopParams = params::InterfaceGl::create( getWindow(), "Select", toPixels( Vec2i( 200, 400)), color );
#if 0
    mTopParams->addSeparator();
	mTopParams->addButton( "Import Quicktime Movie", std::bind( &CVisibleApp::create_qmovie_viewer, this ) );
    mTopParams->addSeparator();
   	mTopParams->addButton( "Import SS Matrix", std::bind( &CVisibleApp::create_matrix_viewer, this ) );
    mTopParams->addSeparator();
   	mTopParams->addButton( "Import Result ", std::bind( &CVisibleApp::create_clip_viewer, this ) );
    getWindowIndex(0)->connectDraw ( &CVisibleApp::draw_main, this);
    getWindowIndex(0)->connectClose( &CVisibleApp::window_close, this);
#endif

}


#if 0
void CVisibleApp::mouseMove( MouseEvent event )
{
    //            if (! mWaveformPlot.getWaveforms().empty())
    //              mSigv.at_event (mGraphDisplayRect, event.getPos(), event.getX(),mWaveformPlot.getWaveforms()[0]);
}


void CVisibleApp::mouseDrag( MouseEvent event )
{
}


void CVisibleApp::mouseDown( MouseEvent event )
{
}


void CVisibleApp::mouseUp( MouseEvent event )
{
}


void CVisibleApp::keyDown( KeyEvent event )
{
//	if( event.getCode() == KeyEvent::KEY_s ) mSamplePlayer->seekToTime( 1.0 );
}
#endif

void CVisibleApp::update()
{
    if (mSettings.isResizable() || mSettings.isFullScreen())
    {
        resize ();
    }
    
    WindowRef cw = getWindow();
    
    if (cw != getWindowIndex(0) && cw->getUserData<uContext>())
        cw->getUserData<uContext>()->update ();
    
}

void CVisibleApp::draw ()
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
CINDER_APP_BASIC( CVisibleApp, RendererGl )

