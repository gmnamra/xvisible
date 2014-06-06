
#include "visible_app.h"

using namespace vf_utils::csv;


size_t Wave2Index (const Rectf& box, const size_t& pos, const Waveform& wave)
{
    size_t xScaled = (pos * wave.sections()) / box.getWidth();
    xScaled *= wave.section_size ();
    xScaled = math<size_t>::clamp( xScaled, 0, wave.samples () );
}

size_t Normal2Index (const Rectf& box, const size_t& pos, const size_t& wave)
{
    size_t xScaled = (pos * wave) / box.getWidth();
    xScaled = math<size_t>::clamp( xScaled, 0, wave );
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

void CVisibleApp::setup()
{
    getWindow()->setUserData( new WindowData );
    
    m_movie_valid = false;
	mSamplePlayerEnabledState = false;
    
	auto ctx = audio2::master();
    
	mPan = ctx->makeNode( new audio2::Pan2d() );
    //	mPan->enableMonoInputMode( false );
    
	mGain = ctx->makeNode( new audio2::Gain() );
	mGain->setValue( 0.6f );
    
	mGain >> mPan >> ctx->getOutput();
    
    mTopParams = params::InterfaceGl (" CVisible ", Vec2i( 200, 400) );
    mTopParams.addButton("Select Movie ", std::bind(&CVisibleApp::setupFilePlayer, this ) );
    mTopParams.addParam("valid movie", &m_movie_valid);
    mTopParams.addButton("Select Signature ", std::bind(&CVisibleApp::setupBufferPlayer, this ) );
    mTopParams.addButton("Select Matrix ", std::bind(&CVisibleApp::setupMatrixPlayer, this ) );
    
	ctx->enable();
	CI_LOG_V( "context samplerate: " << ctx->getSampleRate() );
    
    // mGraphs.push_back (graph1D(EaseInAtan (), "Ease In Atan "));
    
}

void CVisibleApp::setupMatrixPlayer ()
{
    // Browse for a matrix file
    fs::path matPath = getOpenFilePath();
    if(! matPath.empty() ) internal_setupmat_from_file(matPath);
    createNewWindow ();
}


void CVisibleApp::createNewWindow()
{
	app::WindowRef newWindow = createWindow( Window::Format().size( 400, 400 ) );
	newWindow->setUserData( new WindowData );
	
	// for demonstration purposes, we'll connect a lambda unique to this window which fires on close
	int uniqueId = getNumWindows();
	newWindow->getSignalClose().connect(
                                        [uniqueId,this] { this->console() << "You closed window #" << uniqueId << std::endl; }
                                        );
}

void CVisibleApp::internal_setupmat_from_file (const fs::path & fp)
{
    vf_utils::csv::matf_t mat;
    vf_utils::csv::csv2vectors(fp.string(), mat, false, false, true);
    if (vf_utils::csv::validate_matf (mat) ) internal_setupmat_from_mat (mat);
}

void CVisibleApp::internal_setupmat_from_mat (const vf_utils::csv::matf_t & mat)
{
    m_result_valid = false;
    size_t dL = mat.size ();
    int numPixels = dL * dL;
    gl::VboMesh::Layout layout;
    layout.setDynamicColorsRGB();
    layout.setDynamicPositions();
    mPointCloud = gl::VboMesh( numPixels, 0, layout, GL_POINTS );
    
    
    gl::VboMesh::VertexIter vertexIt( mPointCloud );
    size_t my = 0;
    while (my < dL)
    {
        const rowf_t& rowf = mat[my];
        const float* elm_ptr = &rowf[0];
        size_t mx = 0;
        while (mx < dL)
        {
            float val = *elm_ptr++;
            vertexIt.setPosition(static_cast<float>(mx),static_cast<float>(dL - my), val * 100 );
            Color color( val, 0, (1 - val) );
            vertexIt.setColorRGB( color );
            ++vertexIt;
            mx++;
        }
        my++;
    }
    
    Vec3f center( (float)dL/2.0f, (float)dL/2.0f, 50.0f );
    CameraPersp camera( getWindowWidth(), getWindowHeight(), 60.0f );
    camera.setEyePoint( Vec3f( center.x, center.y, (float)dL ) );
    camera.setCenterOfInterestPoint( center );
    mCam.setCurrentCam( camera );
    m_result_valid = true;
}

void CVisibleApp::draw_mat()
{
    
	gl::clear( Color( 0, 0, 0 ) );
    gl::setMatrices( mCam.getCamera() );
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    if( mPointCloud ){
        gl::draw( mPointCloud );
    }
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
    
    //    fs::path fpath ( "/Users/arman/Desktop/visibleqt105/resources/assets/data.txt" );
    //    setSourceFile(loadResource( RES_SINGLE_NORM_COLUMN ) );
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
    
    mGraph1D = Graph1DRef (new graph1D ("signature", mMovieDisplayRect) );
    mGraph1D->addListener( this, &CVisibleApp::receivedEvent );

    vector<float> abuf;
    CVisibleApp::copy_to_vector ( bufferPlayer->getBuffer(), abuf );
    mGraph1D->load_vector(abuf);
}

void CVisibleApp::setupFilePlayer()
{
    // Browse for the movie file
    fs::path moviePath = getOpenFilePath();
    m_movie_valid = false;
    
  	getWindow()->setTitle( moviePath.filename().string() );
    
    mMovieParams = params::InterfaceGl( "Movie Controller", Vec2i( 200, 300 ) );
    
    if ( ! moviePath.empty () )
    {
        std::cout << moviePath.string ();
        loadMovieFile (moviePath);
    }
    
    if( m_movie_valid )
    {
        string max = ci::toString( m_movie.getDuration() );
        mMovieParams.addParam( "Position", &mMoviePosition, "min=0.0 max=" + max + " step=0.5" );
        mMovieParams.addParam( "Rate", &mMovieRate, "step=0.01" );
        mMovieParams.addParam( "Play/Pause", &mMoviePlay );
        mMovieParams.addParam( "Loop", &mMovieLoop );
    }
}

void CVisibleApp::clear_movie_params ()
{
    mMoviePosition = 0.0f;
    mPrevMoviePosition = mMoviePosition;
    mMovieRate = 1.0f;
    mPrevMovieRate = mMovieRate;
    mMoviePlay = false;
    mPrevMoviePlay = mMoviePlay;
    mMovieLoop = false;
    mPrevMovieLoop = mMovieLoop;
}

void CVisibleApp::loadMovieFile( const fs::path &moviePath )
{
	
	try {
		m_movie = qtime::MovieGl( moviePath );
        m_movie_valid = m_movie.checkPlayable ();
        
        if (m_movie_valid)
        {
            console() << "Dimensions:" <<m_movie.getWidth() << " x " <<m_movie.getHeight() << std::endl;
            console() << "Duration:  " <<m_movie.getDuration() << " seconds" << std::endl;
            console() << "Frames:    " <<m_movie.getNumFrames() << std::endl;
            console() << "Framerate: " <<m_movie.getFramerate() << std::endl;
            
            //   m_movie.setLoop( true, false );
            //   m_movie.play();
            m_fc = m_movie.getNumFrames ();
        }
	}
	catch( ... ) {
		console() << "Unable to load the movie." << std::endl;
		return;
	}
	
}

void CVisibleApp::seek( size_t xPos )
{
    auto waves = mWaveformPlot.getWaveforms ();
	if (have_sampler () ) mSamplePlayer->seek( waves[0].sections() * xPos / mGraphDisplayRect.getWidth() );
    
    if (have_movie ()) mMovieIndexPosition = Normal2Index ( mGraphDisplayRect, xPos, m_fc);
}


// a free function which sets gBackgroundColor to blue
Color	gBackgroundColor;

void setBackgroundToBlue()
{
	//gBackgroundColor = ColorA( 0.4f, 0.4f, 0.9f, 0.8f );
}

void CVisibleApp::mouseMove( MouseEvent event )
{
    switch (getWindow()->getUserData<WindowData>()->mId )
    {
      case 1:
            if (mGraph1D) mGraph1D->mouseMove( event );
//            if (! mWaveformPlot.getWaveforms().empty())
   //              mSigv.at_event (mGraphDisplayRect, event.getPos(), event.getX(),mWaveformPlot.getWaveforms()[0]);
            break;
        default:
            break;
    }
    
}


void CVisibleApp::mouseDrag( MouseEvent event )
{
    switch (getWindow()->getUserData<WindowData>()->mId )
    {
        case 2:
            mCam.mouseDrag( event.getPos(), event.isLeft(), event.isMiddle(), event.isRight() );
            break;
        case 1:
            if (mGraph1D) mGraph1D->mouseDrag( event );
        default:
            break;
    }
}


void CVisibleApp::mouseDown( MouseEvent event )
{
    switch (getWindow()->getUserData<WindowData>()->mId )
    {
        case 2:
            mCam.mouseDown( event.getPos() );
            break;
        case 1:
            if (mGraph1D) mGraph1D->mouseDown( event );
            break;
    }
    
    
    
}


void CVisibleApp::mouseUp( MouseEvent event )
{
    switch (getWindow()->getUserData<WindowData>()->mId )
    {
        case 2:
            break;
        case 1:
            if (mGraph1D) mGraph1D->mouseUp( event );
            break;
    }
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
    switch (getWindow()->getUserData<WindowData>()->mId )
    {
            
        case 2:
            break;
        default:
        case 1:
            if (mSettings.isResizable() || mSettings.isFullScreen())
            {
                resize_areas ();
            }
            movie_update ();
            break;
    }
    
}

void CVisibleApp::movie_update ()
{
    
    if( m_movie_valid )
    {
        if( mMovieIndexPosition != mPrevMovieIndexPosition )
        {
            mPrevMovieIndexPosition = mMovieIndexPosition;
            m_movie.seekToFrame(mMovieIndexPosition);
        }
        else
        {
            mMoviePosition = m_movie.getCurrentTime();
            mPrevMoviePosition = mMoviePosition;
        }
        if( mMovieRate != mPrevMovieRate )
        {
            mPrevMovieRate = mMovieRate;
            m_movie.setRate( mMovieRate );
        }
        if( mMoviePlay != mPrevMoviePlay )
        {
            mPrevMoviePlay = mMoviePlay;
            if( mMoviePlay ) m_movie.play();
            else m_movie.stop();
        }
        if( mMovieLoop != mPrevMovieLoop ){
            mPrevMovieLoop = mMovieLoop;
            m_movie.setLoop( mMovieLoop );
        }
    }
    
    if( m_movie ){
        mImage = m_movie.getTexture();
    }
    
    
}

void CVisibleApp::draw_oned ()
{
    // time cycles every 1 / TWEEN_SPEED seconds, with a 50% pause at the end
    float time = math<float>::clamp( fmod( getElapsedSeconds() * 0.5, 1 ) * 1.5f, 0, 1 );
    if (mGraph1D) mGraph1D->draw ( time );
    
}

void CVisibleApp::draw()
{
    switch (getWindow()->getUserData<WindowData>()->mId )
    {
            
        case 2:
            if (m_result_valid) draw_mat ();
            break;
            
        default:
        case 1:
            gl::enableAlphaBlending();
            
            gl::clear();
            
            gl::setMatricesWindowPersp( getWindowSize() );
            
            mTopParams.draw ();
            mMovieParams.draw();
            
            mSigv.draw (mGraphDisplayRect);
            
            
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
            
            
            //	drawWidgets( mWidgets );
            
            if (m_movie_valid && mImage )
            {
                gl::draw (mImage, mMovieDisplayRect);
                mImage.disable ();
            }
            
            draw_oned();
            
            
            break;
    }
}


void CVisibleApp::receivedEvent( InteractiveObjectEvent event ){
    string text;
    switch( event.type ){
        case InteractiveObjectEvent::Pressed:
            text = "Pressed event";
            break;
        case InteractiveObjectEvent::PressedOutside:
            text = "Pressed outside event";
            break;
        case InteractiveObjectEvent::Released:
            text = "Released event";
            break;
        case InteractiveObjectEvent::ReleasedOutside:
            text = "Released outside event";
            break;
        case InteractiveObjectEvent::RolledOver:
            text = "RolledOver event";
            break;
        case InteractiveObjectEvent::RolledOut:
            text = "RolledOut event";
            break;
        case InteractiveObjectEvent::Dragged:
            text = "Dragged event";
            break;
        default:
            text = "Unknown event";
    }
    console() << "Received " + text << endl;
}




