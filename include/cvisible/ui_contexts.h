

#ifndef __ui_contexts___h
#define __ui_contexts___h

#include "cinder/app/App.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/gl.h"
#include "cinder/Rect.h"
#include "cinder/qtime/QuicktimeGl.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "iclip.hpp"
#include "vf_utils.hpp"
#include "vf_types.h"


#include <memory>
#include <utility>

#include <boost/function.hpp>
#include <boost/bind.hpp>


#include <boost/integer_traits.hpp>
// use int64_t instead of long long for better portability
#include <boost/filesystem.hpp>
#include <boost/cstdint.hpp>






using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;
namespace fs=boost::filesystem;


// Namespace collision with OpenCv

//#define Vec2i ci::Vec2i
//#define Vec2f ci::Vec2f
//#define Vec3f ci::Vec3f





class uContext
{
public:
    uContext (ci::app::WindowRef window) : mWindow (window)
    {
       mRect = Rectf( vec2( -40, -40 ), vec2( 40, 40 ) );
       mRect.offset( window->getCenter() );
       mSelected = false;
    }
    
    virtual ~uContext ()
    {
        std::cout << "uContext Base Dtor is called " << std::endl;
    }
    virtual const std::string & name() const = 0;
    virtual void name (const std::string& ) = 0;
    
    virtual void draw () = 0;
    virtual void update () = 0;
    //    virtual void resize () = 0;
    virtual void setup () = 0;
    virtual bool is_valid () = 0;
    
    // u implementation does nothing
    virtual void mouseDown( MouseEvent event ) {}
    virtual void mouseMove( MouseEvent event ) {}
  	virtual void mouseUp( MouseEvent event ) {}
    virtual void mouseDrag( MouseEvent event ) {}
    
	virtual void keyDown( KeyEvent event ) {}
    
    enum Type {
        matrix_viewer = 0,
        qtime_viewer = matrix_viewer+1,
        clip_viewer = qtime_viewer+1,
        viewer_types = clip_viewer+1
    };
    
    
  protected:
    bool m_valid;
    ci::Rectf		mRect;
    bool			mSelected;
    ci::app::WindowRef				mWindow;
    ci::signals::ScopedConnection	mCbMouseDown, mCbMouseDrag;
    ci::signals::ScopedConnection	mCbMouseMove, mCbMouseUp;
};



class matContext : public uContext
{
public:
    matContext(ci::app::WindowRef window) : uContext(window)
    {
       mCbMouseDown = mWindow->getSignalMouseDown().connect( std::bind( &matContext::mouseDown, this, std::placeholders::_1 ) );
       mCbMouseDrag = mWindow->getSignalMouseDrag().connect( std::bind( &matContext::mouseDrag, this, std::placeholders::_1 ) );
       mWindow->setTitle (matContext::caption ());
        setup ();
    }
    static const std::string& caption () { static std::string cp ("Smm Viewer # "); return cp; }

    virtual const std::string & name() const { return mName; }
    virtual void name (const std::string& name) { mName = name; }
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    virtual void mouseDrag( MouseEvent event );
    virtual void mouseDown( MouseEvent event );
    void draw_window ();
    
private:

    void internal_setupmat_from_file (const boost::filesystem::path &);
    void internal_setupmat_from_mat (const vf_utils::csv::matf_t &);
    Rectf render_box ();
    params::InterfaceGl         mMatParams;
    
    gl::VboMeshRef mPointCloud;
    gl::BatchRef	mPointsBatch;

    
    CameraPersp		mCam;
    CameraUi		mCamUi;
    
    boost::filesystem::path mPath;
    std::string mName;
};


class movContext : public uContext
{
public:
    movContext(ci::app::WindowRef window) : uContext(window)
    {
        mCbMouseDown = mWindow->getSignalMouseDown().connect( std::bind( &movContext::mouseDown, this, std::placeholders::_1 ) );
        mCbMouseDrag = mWindow->getSignalMouseDrag().connect( std::bind( &movContext::mouseDrag, this, std::placeholders::_1 ) );
        mCbMouseUp = mWindow->getSignalMouseUp().connect( std::bind( &movContext::mouseUp, this, std::placeholders::_1 ) );
        mCbMouseMove = mWindow->getSignalMouseMove().connect( std::bind( &movContext::mouseMove, this, std::placeholders::_1 ) );
        
        mWindow->setTitle (movContext::caption ());
        setup ();
    }
    
    static const std::string& caption () { static std::string cp ("Qtime Viewer # "); return cp; }
    virtual const std::string & name() const { return mName; }
    virtual void name (const std::string& name) { mName = name; }
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    void draw_window ();

    virtual void mouseDown( MouseEvent event );
    virtual void mouseMove( MouseEvent event );
  	virtual void mouseUp( MouseEvent event );
    virtual void mouseDrag( MouseEvent event );
    
	//virtual void keyDown( KeyEvent event );

    const params::InterfaceGl& ui_params ()
    {
        return mMovieParams;
    }
    
private:
   
    void loadMovieFile( const boost::filesystem::path &moviePath );
    bool have_movie () { return m_movie_valid; }
    void seek( size_t xPos );
    void clear_movie_params ();
    Rectf render_box ();
    vec2 texture_to_display_zoom ();
    
    gl::TextureRef mImage;
    ci::qtime::MovieGlRef m_movie;
    bool m_movie_valid;
    size_t m_fc;
    std::string mName;
    params::InterfaceGl         mMovieParams;
    float mMoviePosition, mPrevMoviePosition;
    size_t mMovieIndexPosition, mPrevMovieIndexPosition;
    float mMovieRate, mPrevMovieRate;
    bool mMoviePlay, mPrevMoviePlay;
    bool mMovieLoop, mPrevMovieLoop;
    vec2 m_zoom;
    Rectf m_display_rect;
    float mMovieCZoom;
    boost::filesystem::path mPath;
	vec2		mMousePos;

    bool mMouseIsDown;
    bool mMouseIsMoving;
    bool mMouseIsDragging;
    
    
    static size_t Normal2Index (const Rectf& box, const size_t& pos, const size_t& wave)
    {
        size_t xScaled = (pos * wave) / box.getWidth();
        xScaled = math<size_t>::clamp( xScaled, 0, wave );
    }
    
};


class clipContext : public uContext
{
public:
    clipContext(ci::app::WindowRef window) : uContext(window)
    {
        mCbMouseDown = mWindow->getSignalMouseDown().connect( std::bind( &clipContext::mouseDown, this, std::placeholders::_1 ) );
        mCbMouseDrag = mWindow->getSignalMouseDrag().connect( std::bind( &clipContext::mouseDrag, this, std::placeholders::_1 ) );
        mCbMouseUp = mWindow->getSignalMouseUp().connect( std::bind( &clipContext::mouseUp, this, std::placeholders::_1 ) );
        mCbMouseDrag = mWindow->getSignalMouseMove().connect( std::bind( &clipContext::mouseMove, this, std::placeholders::_1 ) );
        
        mWindow->setTitle (clipContext::caption ());
        setup ();
    }
    
    clipContext(const std::string& name);
    static const std::string& caption () { static std::string cp ("Result Clip Viewer # "); return cp; }    
    virtual const std::string & name() const { return mName; }
    virtual void name (const std::string& name) { mName = name; }
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    virtual void mouseDrag( MouseEvent event );
    virtual void mouseMove( MouseEvent event );
    virtual void mouseDown( MouseEvent event );
    virtual void mouseUp( MouseEvent event );
    void draw_window ();
    void receivedEvent ( InteractiveObjectEvent event );
    
private:
  
    static DataSourcePathRef create (const std::string& fqfn)
    {
        return  DataSourcePath::create (boost::filesystem::path  (fqfn));
    }
    
    void select_column (size_t col) { m_column_select = col; }
    
    Rectf render_box ();
    params::InterfaceGl         mClipParams;
    Graph1DRef mGraph1D;
    boost::filesystem::path mPath;
    std::string mName;
    std::vector<vector<float> > mdat;
    int m_column_select;
    size_t m_frames, m_file_frames, m_read_pos;
    size_t m_rows, m_columns;
};




#endif
