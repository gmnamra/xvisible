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
#include "cinder/Rect.h"
#include "cinder/qtime/Quicktime.h"
#include "cinder/params/Params.h"

#if 0
#include "cinder/audio2/Source.h"
#include "cinder/audio2/Target.h"
#include "cinder/audio2/dsp/Converter.h"
#include "cinder/audio2/SamplePlayer.h"
#include "cinder/audio2/NodeEffect.h"
#include "cinder/audio2/Scope.h"
#include "cinder/audio2/Debug.h"
#include "cvisible/AudioTestGui.h"
#include "cvisible/AudioDrawUtils.h"
#include "cvisible/vf_cinder.hpp"
#endif

#include "assets/Resources.h"


#include "cvisible/vf_utils.hpp"
#include "vf_types.h"

#include "iclip.hpp"
#include "ui_contexts.h"
#include <map>

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
    
    typedef std::map<size_t,matContextRef> mat_map_t;
    typedef std::map<size_t,movContextRef> mov_map_t;
    typedef std::map<size_t,clipContextRef> clip_map_t;

    static CVisibleApp* master ();
    
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
	void draw_main();
    
#if 0
    void enable_audio_output ();
	void setupBufferPlayer();
	void setupFilePlayer();
    bool have_sampler () { return mSamplePlayer != 0; }
	void setSourceFile( const DataSourceRef &dataSource );
#endif
    void resize_areas (); 
    
    template<typename T>
    std::map<size_t, T>& repository ();
    
    template<typename T>
    void add_to (const T&);
    
    bool remove_from (size_t);

    
#if 0
    static void copy_to_vector (const ci::audio2::BufferRef &buffer, std::vector<float>& mBuffer)
    {
        mBuffer.clear ();
        const float *reader = buffer->getChannel( 0 );
        for( size_t i = 0; i < buffer->getNumFrames(); i++ )
            mBuffer.push_back (*reader++);
    }
#endif
    
    params::InterfaceGlRef         mTopParams;
#if 0
	audio2::BufferPlayerRef		mSamplePlayer;
	audio2::SourceFileRef		mSourceFile;
	audio2::ScopeRef			mScope;
	audio2::GainRef				mGain;
	audio2::Pan2dRef			mPan;
	WaveformPlot				mWaveformPlot;
#endif
	bool						mSamplePlayerEnabledState;
	std::future<void>			mAsyncLoadFuture;

    
    Rectf                       mGraphDisplayRect;    
    Rectf                       mMovieDisplayRect;
    Rectf                       mMenuDisplayRect;
    Rectf                       mLogDisplayRect;
    
    Signal_value                mSigv;

    mat_map_t mMatWindows;
    mov_map_t mMovWindows;
    clip_map_t mClipWindows;
 
};



#endif
