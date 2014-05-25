//
//  visible_app.h
//  XcVisible
//
//  Created by Arman Garakani on 5/8/14.
//
//

#ifndef XcVisible_visible_app_h
#define XcVisible_visible_app_h

#include "cinder/app/AppNative.h"
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
#include "cinder/Easing.h"
#include "assets/Resources.h"

#include "cvisible/AudioTestGui.h"
#include "cvisible/AudioDrawUtils.h"
#include "cvisible/vf_cinder.hpp"
#include "cvisible/vf_utils.hpp"


using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;


#define Vec2i ci::Vec2i
#define Vec2f ci::Vec2f
#define Vec3f ci::Vec3f

struct marker_t
{
    Vec2i display_pixel_pos;
    size_t buffer_index;
    float readout; 
    
    const marker_t		operator+( const marker_t &o ) const
    { 
        marker_t cp = *this; 
        cp.readout += o.readout; 
        cp.display_pixel_pos = o.display_pixel_pos;
        cp.buffer_index = o.buffer_index; 
        return cp;
    }
    const marker_t		operator*( const float &o ) const 
    { 
        marker_t cp = *this; 
        cp.readout *= o; 
        return cp;
    }
};

class CVisibleApp;

struct Signal_value
{
    void post_update ()
    {
        label = toString(mark_val().readout);
    }
    
    void draw(const Rectf& display)
    {
        const Vec2i& readPos = mark_val().display_pixel_pos;
		gl::color( ColorA( 0, 1, 0, 0.7f ) );
		gl::drawSolidRoundedRect( Rectf( readPos.x - 2, 0, readPos.x + 2, (float)display.getHeight() ), 2 );
		gl::color( Color( 1.0f, 0.5f, 0.25f ) );
		gl::drawStringCentered(label, readPos );
    }
    
    Anim<marker_t> mark_val;
    std::string label;
};

// The window-specific data for each window
class WindowData
{
public:
	WindowData()
    : mColor( Color( CM_RGB, 0.1f, 0.8f, 0.8f ) ) 
	{
        mId = AppNative::get()->getNumWindows ();
    }
    
	Color			mColor;
    size_t          mSignalLength;
    size_t          mId;
};


struct OneDbox {
public:
	OneDbox( std::function<float (float)> fn, string name )
    : mFn( fn )
	{
		// create label
		TextLayout text; text.clear( Color::white() ); text.setColor( Color(0.5f, 0.5f, 0.5f) );
		try { text.setFont( Font( "Futura-CondensedMedium", 18 ) ); } catch( ... ) { text.setFont( Font( "Arial", 18 ) ); }
		text.addLine( name );
		mLabelTex = gl::Texture( text.render( true ) );
	}
	
	void draw( float t ) const
	{
		// draw box and frame
		gl::color( Color( 1.0f, 1.0f, 1.0f ) );
		gl::drawSolidRect( mDrawRect );
		gl::color( Color( 0.4f, 0.4f, 0.4f ) );
		gl::drawStrokedRect( mDrawRect );
		gl::color( Color::white() );
		gl::draw( mLabelTex, mDrawRect.getCenter() - mLabelTex.getSize() / 2 );
        
		// draw graph
		gl::color( ColorA( 0.25f, 0.5f, 1.0f, 0.5f ) );
		glBegin( GL_LINE_STRIP );
		for( float x = 0; x < mDrawRect.getWidth(); x += 0.25f ) {
			float y = 1.0f - mFn( x / mDrawRect.getWidth() );
			gl::vertex( Vec2f( x, y * mDrawRect.getHeight() ) + mDrawRect.getUpperLeft() );
		}
		glEnd();
		
		// draw animating circle
		gl::color( Color( 1, 0.5f, 0.25f ) );
		gl::drawSolidCircle( mDrawRect.getUpperLeft() + mFn( t ) * mDrawRect.getSize(), 5.0f );
	}
	
	std::function<float (float)>	mFn;
	Rectf							mDrawRect;
	gl::Texture						mLabelTex;
};


class CVisibleApp : public AppNative {
public:

    static CVisibleApp* master ();
    
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
  	void mouseMove( MouseEvent event );
    void mouseDrag( MouseEvent event );    
    
	void keyDown( KeyEvent event );
	void fileDrop( FileDropEvent event );
	void update();
    void movie_update ();
	void draw();
    void draw_mat ();
    void draw_oned ();
    
	void setupBufferPlayer();
	void setupFilePlayer();
    void setupMatrixPlayer ();
    void createNewWindow();
    void internal_setupmat_from_file (const fs::path &);
    void internal_setupmat_from_mat (const vf_utils::csv::matf_t &);
    
	void setSourceFile( const DataSourceRef &dataSource );
    void resize_areas (); 
    void loadMovieFile( const fs::path &moviePath );
    bool have_sampler () { return mSamplePlayer != 0; }
    bool have_movie () { return m_movie_valid; }
	void seek( size_t xPos );
	marker_t marker_at ( MouseEvent& );    
    void clear_movie_params ();

    
	audio2::BufferPlayerRef		mSamplePlayer;
	audio2::SourceFileRef		mSourceFile;
	audio2::ScopeRef			mScope;
	audio2::GainRef				mGain;
	audio2::Pan2dRef			mPan;
	WaveformPlot				mWaveformPlot;
    
	bool						mSamplePlayerEnabledState;
	std::future<void>			mAsyncLoadFuture;
    marker_t                    mCurrent;
    
    params::InterfaceGl         mTopParams;
    
    Rectf                       mGraphDisplayRect;    
    Rectf                       mMovieDisplayRect;
    Rectf                       mMenuDisplayRect;
    Rectf                       mLogDisplayRect;
    
    
    gl::Texture mImage;
    ci::qtime::MovieGl m_movie;    
    bool m_movie_valid, m_result_valid;
    int m_fc;
    Signal_value                mSigv;

    params::InterfaceGl         mMovieParams;
    float mMoviePosition, mPrevMoviePosition;
    size_t mMovieIndexPosition, mPrevMovieIndexPosition;    
    float mMovieRate, mPrevMovieRate;
    bool mMoviePlay, mPrevMoviePlay;
    bool mMovieLoop, mPrevMovieLoop;

    gl::VboMesh mPointCloud;
    MayaCamUI mCam;
    vector<OneDbox> mGraphs;
    

};


#endif
