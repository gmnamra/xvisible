
#include "visible_app.h"

using namespace vf_utils::csv;


size_t xposInRect2Index (size_t pixel_pos, size_t pixel_range, size_t index_range)
{
    assert (pixel_range != 0);
    return math<size_t>::clamp(pixel_pos * index_range / pixel_range, 0, index_range);
}

CVisibleApp* CVisibleApp::master () { return (CVisibleApp*) AppNative::get(); }


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
    mTopParams.addButton("Select Signature ", std::bind(&CVisibleApp::setupBufferPlayer, this ) );
    mTopParams.addButton("Select Matrix ", std::bind(&CVisibleApp::setupMatrixPlayer, this ) );    
    
	ctx->enable();
	CI_LOG_V( "context samplerate: " << ctx->getSampleRate() );
    
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
    
    //	CI_LOG_V( "async load: " << boolalpha << asyncLoad << dec );
    //	if( asyncLoad ) {
    //		mWaveformPlot.clear();
    //		mAsyncLoadFuture = std::async( [=] {
    //			loadFn();
    //			dispatchAsync( [=] {
    //				connectFn();
    //			} );
    //		} );
    //	}
    //	else 
    
    {
		loadFn();
		connectFn();
	};
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
    if (have_movie ()) mMovieIndexPosition = xposInRect2Index (xPos, mMovieDisplayRect.getWidth(), m_fc);
}


marker_t CVisibleApp::marker_at( MouseEvent& me )
{
	auto bufferPlayer = dynamic_pointer_cast<audio2::BufferPlayer>( mSamplePlayer );
	if( ! bufferPlayer )
		return;
	auto buffer = bufferPlayer->getBuffer();
    auto waves = mWaveformPlot.getWaveforms ();
    marker_t current;
    size_t xScaled = xposInRect2Index (me.getX(), mGraphDisplayRect.getWidth(), waves[0].samples () );
    //    size_t xScaled = (me.getX() * waves[0].sections()) / mGraphDisplayRect.getWidth();
    //    xScaled *= waves[0].section_size ();
    //    xScaled = math<size_t>::clamp( xScaled, 0, waves[0].samples () );
    current.display_pixel_pos = me.getPos ();    
    current.buffer_index = xScaled;
    current.readout = buffer->getChannel( 0 )[xScaled];
    return current;
}

// a free function which sets gBackgroundColor to blue
Color	gBackgroundColor;

void setBackgroundToBlue()
{
	gBackgroundColor = ColorA( 0.4f, 0.4f, 0.9f, 0.8f );
}

void CVisibleApp::mouseMove( MouseEvent event )
{
    if (getWindow()->getUserData<WindowData>()->mId != 1) return;
    
    timeline().apply( &mSigv.mark_val, marker_at (event), 1.0f)
    .startFn( setBackgroundToBlue )
    .updateFn( std::bind( &Signal_value::post_update, &mSigv ))
    .finishFn( setBackgroundToBlue );
    
}


void CVisibleApp::mouseDrag( MouseEvent event )
{
    if (getWindow()->getUserData<WindowData>()->mId != 2) return;
    mCam.mouseDrag( event.getPos(), event.isLeft(), event.isMiddle(), event.isRight() );
}


void CVisibleApp::mouseDown( MouseEvent event )
{
    if (getWindow()->getUserData<WindowData>()->mId != 2) return;
    mCam.mouseDown( event.getPos() );
    
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
            
            
            auto bufferPlayer = dynamic_pointer_cast<audio2::BufferPlayer>( mSamplePlayer );
            if( bufferPlayer )
            {
                if (mSettings.isResizable() || mSettings.isFullScreen())
                {
                    mWaveformPlot.load( bufferPlayer->getBuffer(), mGraphDisplayRect);
                }
                mWaveformPlot.draw();
            }
            
            
            //	drawWidgets( mWidgets );
            
            if (m_movie_valid && mImage )
            {
                gl::draw (mImage, mMovieDisplayRect);
                mImage.disable ();
            }        
            
            mSigv.draw (mGraphDisplayRect);
            break;
    }
}




CINDER_APP_NATIVE( CVisibleApp, RendererGl )
