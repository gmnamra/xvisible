
#include "visible_app.h"

using namespace vf_utils::csv;


size_t Wave2Index (const Rectf& box, const size_t& pos, const Waveform& wave)
{
    size_t xScaled = (pos * wave.sections()) / box.getWidth();
    xScaled *= wave.section_size ();
    xScaled = math<size_t>::clamp( xScaled, 0, wave.samples () );
}

CVisibleApp* CVisibleApp::master () { return (CVisibleApp*) AppBasic::get(); }


void CVisibleApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1200, 576 );
    settings->setResizable(true);
}

void CVisibleApp::resize_areas ()
{
    const Vec2i& c_ul = getWindowBounds().getUL();
    const Vec2i& c_lr = getWindowBounds().getLR();
    Vec2i c_mr (c_lr.x, (c_lr.y - c_ul.y) / 2);
    Vec2i c_ml (c_ul.x, (c_lr.y - c_ul.y) / 2);
    mGraphDisplayRect = Area (c_ul, c_mr);
    mMovieDisplayRect = Area (c_ml, c_lr);
}

void CVisibleApp::enable_audio_output ()
{
	auto ctx = audio2::master();
	mPan = ctx->makeNode( new audio2::Pan2d() );
    //	mPan->enableMonoInputMode( false );
	mGain = ctx->makeNode( new audio2::Gain() );
	mGain->setValue( 0.6f );
	mGain >> mPan >> ctx->getOutput();
	ctx->enable();
	CI_LOG_V( "context samplerate: " << ctx->getSampleRate() );
}

void CVisibleApp::setup()
{
    m_movie_valid = false;
	mSamplePlayerEnabledState = false;
    
    // Register Main Window
    baseContextRef mainw ( static_cast<baseContext*> (new mainContext (this, getWindow() ) ) );
    mWindow_dict[mainw->Id()] = mainw;
    
    // Setup a Matrix Viewer
    baseContextRef matw ( static_cast<baseContext*> (new matContext (this ) ) );
    mWindow_dict[matw->Id()] = matw;
    
    // Setup a Movie Player
    baseContextRef movw ( static_cast<baseContext*> (new movContext (this ) ) );
    mWindow_dict[movw->Id()] = movw;

    // Setup a Movie Player
    baseContextRef clipw ( static_cast<baseContext*> (new clipContext (this ) ) );
    mWindow_dict[clipw->Id()] = clipw;
    
    mTopParams = params::InterfaceGl (" CVisible ", Vec2i( 200, 400) );
    mTopParams.addButton("Select Movie ", std::bind(&baseContext::setup, movw ) );
    mTopParams.addButton("Select Signature ", std::bind(&baseContext::setup, clipw ) );
    mTopParams.addButton("Select Matrix ", std::bind(&baseContext::setup, matw ) );
    
}


void CVisibleApp::setSourceFile( const DataSourceRef &dataSource )
{
    mSourceFile =  unique_ptr<SourceFile>( new vf_cinder::VisibleAudioSource ( dataSource ) );
	getWindow()->setTitle( dataSource->getFilePath().filename().string() );
    
	CI_LOG_V( "SourceFile info: " );
    console() << "Source File: " << dataSource->getFilePath().filename().string() << endl;
	console() << "samplerate: " << mSourceFile->getSampleRate() << endl;
	console() << "channels: " << mSourceFile->getNumChannels() << endl;
	console() << "native samplerate: " << mSourceFile->getNativeSampleRate() << endl;
	console() << "native channels: " << mSourceFile->getNativeNumChannels() << endl;
	console() << "frames: " << mSourceFile->getNumFrames() << endl;
	console() << "metadata:\n" << mSourceFile->getMetaData() << endl;
}

void CVisibleApp::setupBufferPlayer()
{
	auto bufferPlayer = audio2::master()->makeNode( new audio2::BufferPlayer() );
    
    fs::path fp = getOpenFilePath();
    setSourceFile (vf_cinder::VisibleAudioSource::create ( fp.string() ) );
    
	auto loadFn = [bufferPlayer, this]
    {
        auto sfb = mSourceFile->loadBuffer();
		bufferPlayer->setBuffer(sfb);
        mGraphDisplayRect = getWindowBounds();
        mGraphDisplayRect.offset (Vec2i (0, mGraphDisplayRect.getHeight() / 4));
        mWaveformPlot.load( bufferPlayer->getBuffer(), mGraphDisplayRect);
		CI_LOG_V( "loaded source buffer, frames: " << bufferPlayer->getBuffer()->getNumFrames() );
        
	};
    
	auto connectFn = [bufferPlayer, this] {
		mSamplePlayer = bufferPlayer;
		mSamplePlayer >> mGain >> mPan >> audio2::master()->getOutput();
		audio2::master()->printGraph();
        
        //	mSamplePlayer->setLoopEnabled( mLoopButton.mEnabled );
        //	mSamplePlayer->setLoopBeginTime( mLoopBeginSlider.mValueScaled );
        //	mSamplePlayer->setLoopEndTime( mLoopEndSlider.mValueScaled != 0 ? mLoopEndSlider.mValueScaled : mSamplePlayer->getNumSeconds() );
	};
    
    
    {
		loadFn();
		connectFn();
	};
    
  }

// a free function which sets gBackgroundColor to blue
Color	gBackgroundColor;

void setBackgroundToBlue()
{
	//gBackgroundColor = ColorA( 0.4f, 0.4f, 0.9f, 0.8f );
}

const baseContextRef CVisibleApp::getContextRef ()
{
    size_t wid = getWindow()->getUserData<baseContext>()->Id();
    bool valid = getWindow()->getUserData<baseContext>()->is_valid();
    valid &= mWindow_dict.has_key(wid);
    if (valid) return mWindow_dict[wid];
    else return boost::shared_ptr<baseContext>();
}

void CVisibleApp::mouseMove( MouseEvent event )
{
    baseContextRef dr = getContextRef();
    if (dr) dr->mouseMove(event);
    //            if (! mWaveformPlot.getWaveforms().empty())
    //              mSigv.at_event (mGraphDisplayRect, event.getPos(), event.getX(),mWaveformPlot.getWaveforms()[0]);
}


void CVisibleApp::mouseDrag( MouseEvent event )
{
    baseContextRef dr = getContextRef();
    if (dr) dr->mouseDrag(event);
 
}


void CVisibleApp::mouseDown( MouseEvent event )
{
    baseContextRef dr = getContextRef();
    if (dr) dr->mouseDown(event);
}


void CVisibleApp::mouseUp( MouseEvent event )
{
    baseContextRef dr = getContextRef();
    if (dr) dr->mouseUp(event);
}

void CVisibleApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_s )
		mSamplePlayer->seekToTime( 1.0 );
}

//@todo Update
void CVisibleApp::fileDrop( FileDropEvent event )
{
	const fs::path &filePath = event.getFile( 0 );
	CI_LOG_V( "File dropped: " << filePath );
    
	setSourceFile( loadFile( filePath ) );
	mSamplePlayer->seek( 0 );
    
	CI_LOG_V( "output samplerate: " << mSourceFile->getSampleRate() );
    
	auto bufferPlayer = dynamic_pointer_cast<audio2::BufferPlayer>( mSamplePlayer );
	if( bufferPlayer )
    {
		bufferPlayer->loadBuffer( mSourceFile );
        mWaveformPlot.load( bufferPlayer->getBuffer(), mGraphDisplayRect);
	}
	else
    {
		auto filePlayer = dynamic_pointer_cast<audio2::FilePlayer>( mSamplePlayer );
		CI_ASSERT_MSG( filePlayer, "expected sample player to be either BufferPlayer or FilePlayer" );
        
		filePlayer->setSourceFile( mSourceFile );
	}
    
	//mLoopBeginSlider.mMax = mLoopEndSlider.mMax = (float)mSamplePlayer->getNumSeconds();
    
	CI_LOG_V( "loaded and set new source buffer, channels: " << mSourceFile->getNumChannels() << ", frames: " << mSourceFile->getNumFrames() );
	audio2::master()->printGraph();
}


void CVisibleApp::update()
{
    baseContextRef dr = getContextRef();
    if (dr) dr->update();
    
    //            if (mSettings.isResizable() || mSettings.isFullScreen())
    //            {
    //                resize_areas ();
    //            }
    //            movie_update ();
    
}

void CVisibleApp::draw_oned ()
{
    // time cycles every 1 / TWEEN_SPEED seconds, with a 50% pause at the end
    float time = math<float>::clamp( fmod( getElapsedSeconds() * 0.5, 1 ) * 1.5f, 0, 1 );
    if (mGraph1D) mGraph1D->draw ( time );
    
}

void CVisibleApp::draw()
{
    draw_main ();
    baseContextRef dr = getContextRef();
    if (dr) dr->draw();
}

void CVisibleApp::draw_main ()
{
    gl::enableAlphaBlending();
    
    gl::clear();
    
    gl::setMatricesWindowPersp( getWindowSize() );
    
    mTopParams.draw ();
    mMovieParams.draw();
    
    auto bufferPlayer = dynamic_pointer_cast<audio2::BufferPlayer>( mSamplePlayer );
    if( bufferPlayer )
    {
        mWaveformPlot.load( bufferPlayer->getBuffer(), mGraphDisplayRect);
        if (mSettings.isResizable() || mSettings.isFullScreen())
        {
            // mWaveformPlot.load( bufferPlayer->getBuffer(), mGraphDisplayRect);
        }
        mWaveformPlot.draw();
    }

}





