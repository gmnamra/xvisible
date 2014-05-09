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

#include "assets/Resources.h"

#include "cvisible/AudioTestGui.h"
#include "cvisible/AudioDrawUtils.h"
#include "cvisible/vf_cinder.hpp"



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

struct Signal_value
{
    void post_update ()
    {
        label = toString(mark_val().readout);
        //        CI_LOG_V( "post_update: " );
        //        console() << mark_val().readout << endl;
        //        console() << mark_val().display_pixel_pos << endl;        
        //        console() << mark_val().buffer_index << endl;                
        
    }
    
    void draw()
    {
        const Vec2i& readPos = mark_val().display_pixel_pos;
		gl::color( ColorA( 0, 1, 0, 0.7f ) );
		gl::drawSolidRoundedRect( Rectf( readPos.x - 2, 0, readPos.x + 2, (float)getWindowHeight() ), 2 );
		gl::color( Color( 1.0f, 0.5f, 0.25f ) );
		gl::drawStringCentered(label, readPos );
    }
    
    Anim<marker_t> mark_val;
    std::string label;
};



class CVisibleApp : public AppNative {
public:
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
  	void mouseMove( MouseEvent event );
	void keyDown( KeyEvent event );
	void fileDrop( FileDropEvent event );
	void update();
    void movie_update ();
	void draw();
    
	void setupBufferPlayer();
	void setupFilePlayer();
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
    
    
    gl::Texture mImage;
    Surface mImageSurface; 
    ci::qtime::MovieGl m_movie;    
    bool m_movie_valid, m_result_valid;
    int m_fc;
    float vtick_scaled;
    
    Signal_value                mSigv;

    params::InterfaceGl         mMovieParams;
    float mMoviePosition, mPrevMoviePosition;
    float mMovieRate, mPrevMovieRate;
    bool mMoviePlay, mPrevMoviePlay;
    bool mMovieLoop, mPrevMovieLoop;
    
    
    params::InterfaceGl         mTopParams;
    Rectf                       mGraphDisplayRect;    
    Rectf                       mMovieDisplayRect;
    Rectf                       mControls;
    
    
};


#endif
