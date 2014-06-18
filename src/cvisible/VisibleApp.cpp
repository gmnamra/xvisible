
#include "visible_app.h"



CINDER_APP_BASIC( CVisibleApp, RendererGl )


using namespace vf_utils::csv;

#if 0
size_t Wave2Index (const Rectf& box, const size_t& pos, const Waveform& wave)
{
    size_t xScaled = (pos * wave.sections()) / box.getWidth();
    xScaled *= wave.section_size ();
    xScaled = math<size_t>::clamp( xScaled, 0, wave.samples () );
}
#endif

template<>
std::map<size_t,matContextRef>& CVisibleApp::repository () { return mMatWindows; }

template<>
std::map<size_t,movContextRef>& CVisibleApp::repository () { return mMovWindows; }

template<>
std::map<size_t,clipContextRef>& CVisibleApp::repository () { return mClipWindows; }



template<>
void CVisibleApp::add_to (const matContextRef& m)
{
    repository<matContextRef>().insert (std::move(std::make_pair(m->Id(), m)));
}

template<>
void CVisibleApp::add_to (const movContextRef& m)
{
    repository<movContextRef>().insert (std::pair<size_t,movContextRef>(m->Id(), m));
}

template<>
void CVisibleApp::add_to (const clipContextRef& m)
{
    repository<clipContextRef>().insert (std::pair<size_t,clipContextRef>(m->Id(), m));
}



bool CVisibleApp::remove_from (size_t m)
{
    const std::map<size_t,matContextRef>::const_iterator matpos = mMatWindows.find(m);
    if (matpos != mMatWindows.end() )
    {
        mMatWindows.erase(matpos);
        return true;
    }
    const std::map<size_t,movContextRef>::const_iterator movpos = mMovWindows.find(m);
    if (movpos != mMovWindows.end() )
    {
        mMovWindows.erase(movpos);
        return true;
    }
    const std::map<size_t,clipContextRef>::const_iterator clippos = mClipWindows.find(m);
    if (clippos != mClipWindows.end() )
    {
        mClipWindows.erase(clippos);
        return true;
    }
    return false;
}


CVisibleApp* CVisibleApp::master ()
{
    return (CVisibleApp*) AppBasic::get();
}


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

void CVisibleApp::create_matrix_viewer ()
{
     matContextRef matw (new matContext () );
    add_to (matw);
}
void CVisibleApp::create_clip_viewer ()
{
    clipContextRef clipw (new clipContext () );
    add_to(clipw);

}
void CVisibleApp::create_qmovie_viewer ()
{
     movContextRef movw ( new movContext () );
    add_to(movw);
}

void CVisibleApp::setup()
{
	mSamplePlayerEnabledState = false;
    size_t nw = getNumWindows ();
    console () << "Initial # of Windows " << nw << std::endl;
    // Setup the parameters
	mTopParams = params::InterfaceGl::create( getWindow(), "Select", toPixels( Vec2i( 200, 400 ) ) );
    mTopParams->addSeparator();
	mTopParams->addButton( "Import Quicktime Movie", std::bind( &CVisibleApp::create_qmovie_viewer, this ) );
    mTopParams->addSeparator();
   	mTopParams->addButton( "Import SS Matrix", std::bind( &CVisibleApp::create_matrix_viewer, this ) );
    mTopParams->addSeparator();
   	mTopParams->addButton( "Import Result ", std::bind( &CVisibleApp::create_clip_viewer, this ) );
    getWindowIndex(0)->connectDraw ( &CVisibleApp::draw_main, this);

    
}


// a free function which sets gBackgroundColor to blue
Color	gBackgroundColor;

void setBackgroundToBlue()
{
	//gBackgroundColor = ColorA( 0.4f, 0.4f, 0.9f, 0.8f );
}

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


void CVisibleApp::update()
{
    if (mSettings.isResizable() || mSettings.isFullScreen())
    {
        resize_areas ();
    }

    
}

void CVisibleApp::draw_main ()
{
    gl::enableAlphaBlending();
    
    gl::clear();
    
    gl::setMatricesWindowPersp( getWindowSize() );
    
#if 0
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
#endif
    
    mTopParams->draw ();
    
}

#if 0

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


#endif
