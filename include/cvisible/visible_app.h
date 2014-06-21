//
//  visible_app.h
//  XcVisible
//
//  Created by Arman Garakani on 5/8/14.
//
//

#ifndef XcVisible_visible_app_h

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
#include "stl_util.hpp"
#include "vf_types.h"

#include "iclip.hpp"

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
    void close_main();
    void resize_areas ();
    
    template<class V>
    void create_viewer (vf_utils::id<V>);
    
    bool remove_from (const std::string& );
    
    params::InterfaceGlRef         mTopParams;
    
    Rectf                       mGraphDisplayRect;
    Rectf                       mMovieDisplayRect;
    Rectf                       mMenuDisplayRect;
    Rectf                       mLogDisplayRect;
    
 //   Signal_value                mSigv;
    
#if 0
    void enable_audio_output ();
	void setupBufferPlayer();
	void setupFilePlayer();
    bool have_sampler () { return mSamplePlayer != 0; }
	void setSourceFile( const DataSourceRef &dataSource );
	audio2::BufferPlayerRef		mSamplePlayer;
	audio2::SourceFileRef		mSourceFile;
	audio2::ScopeRef			mScope;
	audio2::GainRef				mGain;
	audio2::Pan2dRef			mPan;
	WaveformPlot				mWaveformPlot;
	bool						mSamplePlayerEnabledState;
	std::future<void>			mAsyncLoadFuture;
    
#endif

  
    
};

  #define XcVisible_visible_app_h

#endif
