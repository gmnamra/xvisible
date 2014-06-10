//
//  visible_app.h
//  XcVisible
//
//  Created by Arman Garakani on 5/8/14.
//
//

#ifndef XcVisible_visible_app_h
#define XcVisible_visible_app_h

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

#include "cvisible/AudioTestGui.h"
#include "cvisible/AudioDrawUtils.h"
#include "cvisible/vf_cinder.hpp"
#include "cvisible/vf_utils.hpp"
#include "vf_types.h"

#include "iclip.hpp"
#include "ui_contexts.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;

//class CVisibleApp;

// Namespace collision with OpenCv

#define Vec2i ci::Vec2i
#define Vec2f ci::Vec2f
#define Vec3f ci::Vec3f




class CVisibleApp : public AppBasic
{
public:

    static CVisibleApp* master ();
    
	void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
    void mouseMove( MouseEvent event );
  	void mouseUp( MouseEvent event );
    void mouseDrag( MouseEvent event );    
    
	void keyDown( KeyEvent event );
	void fileDrop( FileDropEvent event );
	void update();
	void draw();
	void draw_main();
    void draw_mat ();
    void draw_oned ();
    void enable_audio_output ();
	void setupBufferPlayer();
	void setupFilePlayer();
 
	void setSourceFile( const DataSourceRef &dataSource );
    void resize_areas (); 
    void loadMovieFile( const fs::path &moviePath );
    bool have_sampler () { return mSamplePlayer != 0; }
    bool have_movie () { return m_movie_valid; }
	void seek( size_t xPos );
    void clear_movie_params ();
    
    void receivedEvent ( InteractiveObjectEvent event );
    
    const baseContextRef getContextRef ();
    
    
    static void copy_to_vector (const ci::audio2::BufferRef &buffer, std::vector<float>& mBuffer)
    {
        mBuffer.clear ();
        const float *reader = buffer->getChannel( 0 );
        for( size_t i = 0; i < buffer->getNumFrames(); i++ )
            mBuffer.push_back (*reader++);
    }
    
    
    static size_t Normal2Index (const Rectf& box, const size_t& pos, const size_t& wave)
    {
        size_t xScaled = (pos * wave) / box.getWidth();
        xScaled = math<size_t>::clamp( xScaled, 0, wave );
    }
    

    
	audio2::BufferPlayerRef		mSamplePlayer;
	audio2::SourceFileRef		mSourceFile;
	audio2::ScopeRef			mScope;
	audio2::GainRef				mGain;
	audio2::Pan2dRef			mPan;
	WaveformPlot				mWaveformPlot;
    
	bool						mSamplePlayerEnabledState;
	std::future<void>			mAsyncLoadFuture;
    params::InterfaceGl         mTopParams;
    
    Rectf                       mGraphDisplayRect;    
    Rectf                       mMovieDisplayRect;
    Rectf                       mMenuDisplayRect;
    Rectf                       mLogDisplayRect;
    
    
    gl::Texture mImage;
    ci::qtime::MovieGl m_movie;    
    bool m_movie_valid;
    size_t m_fc;
    Signal_value                mSigv;

    params::InterfaceGl         mMovieParams;
    float mMoviePosition, mPrevMoviePosition;
    size_t mMovieIndexPosition, mPrevMovieIndexPosition;    
    float mMovieRate, mPrevMovieRate;
    bool mMoviePlay, mPrevMoviePlay;
    bool mMovieLoop, mPrevMovieLoop;
    Graph1DRef mGraph1D;
    vf_utils::dict::dict <size_t, baseContextRef > mWindow_dict;
    
 
};

CINDER_APP_BASIC( CVisibleApp, RendererGl )


#endif
