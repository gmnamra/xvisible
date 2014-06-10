//
//  visible_app.h
//  XcVisible
//
//  Created by Arman Garakani on 5/8/14.
//
//

#ifndef __ui_contexts___h
#define __ui_contexts___h

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"
#include "cinder/Timer.h"
#include "cinder/Camera.h"
#include "cinder/audio2/Source.h"
#include "cinder/audio2/Target.h"
#include "cinder/audio2/dsp/Converter.h"
#include "cinder/audio2/SamplePlayer.h"
#include "cinder/audio2/NodeEffect.h"
#include "cinder/audio2/Scope.h"
#include "cinder/audio2/Debug.h"
#include "cinder/qtime/Quicktime.h"
#include "cinder/params/Params.h"
#include "cinder/gl/Vbo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/ImageIo.h"
#include "cinder/audio2/Buffer.h"
#include "assets/Resources.h"

#include "cvisible/AudioDrawUtils.h"
#include "cvisible/vf_utils.hpp"
#include "vf_types.h"
#include "iclip.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;


// Namespace collision with OpenCv

#define Vec2i ci::Vec2i
#define Vec2f ci::Vec2f
#define Vec3f ci::Vec3f



// Animating signal marker
struct Signal_value
{
    Signal_value () : mValid (false) {}

    Signal_value (std::function<float (size_t)> fn)
    : mFn( fn ), mValid (true) {}
    
    void at_event (const Rectf& box, const Vec2i& location, const size_t& pos,  const Waveform& wave)
    {
        size_t xScaled = (pos * wave.sections()) / box.getWidth();
        xScaled *= wave.section_size ();
        mSignalIndex = math<size_t>::clamp( xScaled, 0, wave.samples () );
        mDisplayPos = location;
    }

    
    void at_event_value (const Rectf& box, const Vec2i& location, const size_t& pos, const size_t& wave)
    {
        size_t xScaled = (pos * wave) / box.getWidth();
        mSignalIndex = math<size_t>::clamp( xScaled, 0, wave );
        mDisplayPos = location;
    }
    
    void draw(const Rectf& display)
    {
        if ( ! mValid ) return;
        
        const Vec2i& readPos = mDisplayPos;
		gl::color( ColorA( 0, 1, 0, 0.7f ) );
		gl::drawSolidRoundedRect( Rectf( readPos.x - 2, 0, readPos.x + 2, (float)display.getHeight() ), 2 );
		gl::color( Color( 1.0f, 0.5f, 0.25f ) );
		gl::drawStringCentered(toString (mFn (mSignalIndex)), readPos);
    }
    
    Vec2i mDisplayPos;
    size_t mSignalIndex;
    std::function<float (size_t)> mFn;
    bool mValid;
};


class baseContext
{
public:
    baseContext ()
    {
        mId = AppBasic::get()->getNumWindows ();
    }
    size_t Id () { return mId; }


    virtual void draw () = 0;
    virtual void update () = 0;
//    virtual void resize () = 0;
    virtual void setup () = 0;
    virtual bool is_valid () = 0;
    
    // base implementation does nothing
    virtual void mouseDown( MouseEvent event ) {}
    virtual void mouseMove( MouseEvent event ) {}
  	virtual void mouseUp( MouseEvent event ) {}
    virtual void mouseDrag( MouseEvent event ) {}
    
	virtual void keyDown( KeyEvent event ) {}

    
protected:
    size_t mId;
  };


class mainContext : public baseContext
{
public:
    mainContext(AppBasic* appLink, app::WindowRef mWindow);
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();

private:
    bool m_valid;
    AppBasic* mLink;
    app::WindowRef mWindow;
};


class matContext : public baseContext
{
public:
    matContext(AppBasic* appLink);
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    virtual void mouseDrag( MouseEvent event );
    virtual void mouseDown( MouseEvent event );
    
private:
    
    void internal_setupmat_from_file (const fs::path &);
    void internal_setupmat_from_mat (const vf_utils::csv::matf_t &);
    Rectf render_box ();

    gl::VboMesh mPointCloud;
    MayaCamUI mCam;
    fs::path mPath;
    bool m_valid;
    AppBasic* mLink;
    app::WindowRef mWindow;
};


class movContext : public baseContext
{
public:
    movContext(AppBasic* appLink);
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    
    const params::InterfaceGl& ui_params ()
    {
        return mMovieParams;
    }
    
private:
    void loadMovieFile( const fs::path &moviePath );
    bool have_movie () { return m_movie_valid; }
	void seek( size_t xPos );
    void clear_movie_params ();
    Rectf render_box ();
    gl::Texture mImage;
    ci::qtime::MovieGl m_movie;
    bool m_movie_valid;
    size_t m_fc;
    
    params::InterfaceGl         mMovieParams;
    float mMoviePosition, mPrevMoviePosition;
    size_t mMovieIndexPosition, mPrevMovieIndexPosition;
    float mMovieRate, mPrevMovieRate;
    bool mMoviePlay, mPrevMoviePlay;
    bool mMovieLoop, mPrevMovieLoop;

    fs::path mPath;
    bool m_valid;
    AppBasic* mLink;
    app::WindowRef mWindow;
    
    static size_t Normal2Index (const Rectf& box, const size_t& pos, const size_t& wave)
    {
        size_t xScaled = (pos * wave) / box.getWidth();
        xScaled = math<size_t>::clamp( xScaled, 0, wave );
    }

};


class clipContext : public baseContext
{
public:
    clipContext(AppBasic* appLink);
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    virtual void mouseDrag( MouseEvent event );
    virtual void mouseMove( MouseEvent event );
    virtual void mouseDown( MouseEvent event );
  	virtual void mouseUp( MouseEvent event );
    
    void receivedEvent ( InteractiveObjectEvent event );
    
private:
    static DataSourcePathRef create (const std::string& fqfn)
    {
        return  DataSourcePath::create (fs::path  (fqfn));
    }

    Rectf render_box ();
    Graph1DRef mGraph1D;
    fs::path mPath;
    bool m_valid;
    std::vector<vector<float> > mdat;
    int m_column_select;
    size_t m_frames, m_file_frames, m_read_pos;
    size_t m_rows, m_columns;
    AppBasic* mLink;
    app::WindowRef mWindow;
};

typedef boost::shared_ptr<baseContext> baseContextRef;


#endif
