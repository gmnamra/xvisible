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

#include "cinder/audio2/Buffer.h"

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

class CVisibleApp;

// Animating signal marker
struct Signal_value
{
    Signal_value (std::function<float (size_t)> fn, const Rectf& box, const Vec2i& location, const size_t& pos, const Waveform& wave)
       : mFn( fn )
    {
        size_t xScaled = (pos * wave.sections()) / box.getWidth();
        xScaled *= wave.section_size ();
        mSignalIndex = math<size_t>::clamp( xScaled, 0, wave.samples () );
        mDisplayPos = location;
    }
    
    Signal_value (std::function<float (float)> fn,const Rectf& box, const Vec2i& location, const size_t& pos, const size_t& wave)
       : mFn( fn )
    {
        size_t xScaled = (pos * wave) / box.getWidth();
        mSignalIndex = math<size_t>::clamp( xScaled, 0, wave );
        mDisplayPos = location;
    }
    
    void draw(const Rectf& display)
    {
        const Vec2i& readPos = mDisplayPos;
		gl::color( ColorA( 0, 1, 0, 0.7f ) );
		gl::drawSolidRoundedRect( Rectf( readPos.x - 2, 0, readPos.x + 2, (float)display.getHeight() ), 2 );
		gl::color( Color( 1.0f, 0.5f, 0.25f ) );
		gl::drawStringCentered(toString (mFn (mSignalIndex)), readPos);
    }
    
    Anim<Vec2i> mDisplayPos;
    Anim<size_t> mSignalIndex;
    std::function<float (size_t)> mFn;
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
	OneDbox( string name )
	{
		// create label
		TextLayout text; text.clear( Color::white() ); text.setColor( Color(0.5f, 0.5f, 0.5f) );
		try { text.setFont( Font( "Futura-CondensedMedium", 18 ) ); } catch( ... ) { text.setFont( Font( "Arial", 18 ) ); }
		text.addLine( name );
		mLabelTex = gl::Texture( text.render( true ) );
	}

	void load_vector (const vector<float>& buffer)
    {
         mBuffer.clear ();
        vector<float>::const_iterator reader = buffer.begin ();
        while (reader != buffer.end())
        {
            mBuffer.push_back (*reader++);
        }
    }


    void load (const ci::audio2::BufferRef &buffer)
    {
        mBuffer.clear ();
        const float *reader = buffer->getChannel( 0 );
        for( size_t i = 0; i < buffer->getNumFrames(); i++ )
            mBuffer.push_back (*reader++);

    }
    
    float get (float tnormed) const
    {
        // NN
        int32 x = floor (tnormed * (mBuffer.size()-1));
        return mBuffer[x];
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
			float y = 1.0f - get( x / mDrawRect.getWidth() );
			gl::vertex( Vec2f( x, y * mDrawRect.getHeight() ) + mDrawRect.getUpperLeft() );
		}
		glEnd();
		
		// draw animating circle
		gl::color( Color( 1, 0.5f, 0.25f ) );
		gl::drawSolidCircle( mDrawRect.getUpperLeft() + get( t ) * mDrawRect.getSize(), 5.0f );
   }
	
    std::vector<float>                   mBuffer;
	Rectf                           mDrawRect;
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
    void clear_movie_params ();

    
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
    bool m_movie_valid, m_result_valid;
    size_t m_fc;
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
