
#include "visible_main.h"
#include "stl_util.hpp"
#include "ui_contexts.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace vf_utils::csv;


namespace
{
    Rectf render_window_box ()
    {
        auto pos = AppBasic::get()->getWindow()->getPos ();
        auto size = AppBasic::get()->getWindow()->getSize ();
        return Area (pos, size);
    }
    
    std::ostream& ci_console ()
    {
        return AppBasic::get()->console();
    }
    
    double				ci_getElapsedSeconds()
    {
        return AppBasic::get()->getElapsedSeconds();
    }
    
    uint32 ci_getNumWindows ()
    {
        return AppBasic::get()->getNumWindows();
    }

    template<class V>
    class uContextViewer
    {
    public:
        void operator()()
        {
            //is equivalent to boost::uuids::basic_random_generator<boost::mt19937>
            boost::uuids::random_generator gen;
            boost::uuids::uuid u = gen();
            
            const std::string nam = boost::uuids::to_string (u);
            uContextRegistry::registeruContext<V>(nam);
            if (! uContextRegistry::instantiate(nam) )
            {
                std::cout << "instantiate" << uContextRegistry::size() << std::endl;
                std::cout << "Instantiating matrix viewer failed " << std::endl;
            }
            
        }
    };
    
    
}



bool CVisibleApp::remove_from (const std::string& name)
{
    bool ok = uContextRegistry::unregister(name);

    return ok;
}


void CVisibleApp::window_close()
{
    uContextRegistry::print_out ();
    uContext* ud = getWindow()->getUserData<uContext>();
    remove_from (ud->name());
    console() << "Closing " << getWindow() << std::endl;
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
    uContextViewer<matContext> () ();
}
void CVisibleApp::create_clip_viewer ()
{
    uContextViewer<clipContext> () ();
}
void CVisibleApp::create_qmovie_viewer ()
{
    uContextViewer<movContext> () ();
}

void CVisibleApp::setup()
{
    size_t nw = getNumWindows ();
    ci_console () << "Initial # of Windows " << nw << std::endl;
    // Setup the parameters
	mTopParams = params::InterfaceGl::create( getWindow(), "Select", toPixels( Vec2i( 200, 400 ) ) );
    mTopParams->addSeparator();
	mTopParams->addButton( "Import Quicktime Movie", std::bind( &CVisibleApp::create_qmovie_viewer, this ) );
    mTopParams->addSeparator();
   	mTopParams->addButton( "Import SS Matrix", std::bind( &CVisibleApp::create_matrix_viewer, this ) );
    mTopParams->addSeparator();
   	mTopParams->addButton( "Import Result ", std::bind( &CVisibleApp::create_clip_viewer, this ) );
    getWindowIndex(0)->connectDraw ( &CVisibleApp::draw_main, this);
    getWindowIndex(0)->connectClose( &CVisibleApp::window_close, this);

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

    
    mTopParams->draw ();
    
}



#if 0


// a free function which sets gBackgroundColor to blue
Color	gBackgroundColor;

void setBackgroundToBlue()
{
	//gBackgroundColor = ColorA( 0.4f, 0.4f, 0.9f, 0.8f );
}


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
