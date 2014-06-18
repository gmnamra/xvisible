
#include "ui_contexts.h"
#include "stl_util.hpp"
#include "visible_app.h"

using namespace vf_utils::csv;

matContext::matContext () 
{
    ci::app::WindowRef wref = CVisibleApp::master()->createWindow( Window::Format().size( 400, 400 ) );
    mWindow = wref;
    wref->setUserData( this );
    mId = CVisibleApp::master()->getNumWindows();
    size_t idd = Id();
    wref->getSignalClose().
    connect([idd,this] { (static_cast<CVisibleApp*>(CVisibleApp::master()))->remove_from (idd); } );
    wref->getSignalDraw().connect (std::bind(&matContext::draw_window, this) );
    wref->connectMouseDown(&matContext::mouseDown, this);
    wref->connectMouseUp(&matContext::mouseUp, this);
    wref->connectMouseDrag(&matContext::mouseDrag, this);
    wref->connectMouseMove(&matContext::mouseMove, this);
    assert(CVisibleApp::master()->getWindowIndex(idd-1)->getUserData<matContext>()->Id() == idd);
    static std::string caption = "Smm Viewer # ";
    wref->setTitle (caption + stl_utils::tostr(mId));
}

matContext::~matContext ()
{
    std::cout << "mat context dtor callled " << std::endl;
}


void matContext::mouseDrag( MouseEvent event )
{
    mCam.mouseDrag( event.getPos(), event.isLeft(), event.isMiddle(), event.isRight() );
}


void matContext::mouseDown( MouseEvent event )
{
    mCam.mouseDown( event.getPos() );
}

Rectf matContext::render_box ()
{
    Rectf rb;
    if (! m_valid ) return rb;
    ci::app::WindowRef wref = mWindow.lock ();
    if (wref)
        rb = Area (wref->getPos(), wref->getSize());
    return rb;
}
void matContext::setup ()
{
    // Browse for a matrix file
    mPath = getOpenFilePath();
    if(! mPath.empty() ) internal_setupmat_from_file(mPath);
}

void matContext::internal_setupmat_from_file (const fs::path & fp)
{
    vf_utils::csv::matf_t mat;
    vf_utils::csv::csv2vectors(fp.string(), mat, false, false, true);
    if (vf_utils::csv::validate_matf (mat) ) internal_setupmat_from_mat (mat);
}

void matContext::internal_setupmat_from_mat (const vf_utils::csv::matf_t & mat)
{
    m_valid = false;
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
    m_valid = true;
}

void matContext::draw()
{
    if (! m_valid ) return;
    
	gl::clear( Color( 0, 0, 0 ) );
    gl::setMatrices( mCam.getCamera() );
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    if( mPointCloud ){
        gl::draw( mPointCloud );
    }
}


void matContext::draw_window ()
{
    ci::app::WindowRef wref = mWindow.lock ();
    if (! wref || ! getWindow()->getUserData<matContext>()->is_valid() ) return;
    
	gl::clear( Color( 0, 0, 0 ) );
    gl::setMatrices( mCam.getCamera() );
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    if( mPointCloud ){
        gl::draw( mPointCloud );
    }
}


void matContext::update()
{
    
}


bool matContext::is_valid ()
{
    return m_valid;
}

// movContext Implementation

movContext::movContext ()
{
    ci::app::WindowRef wref = CVisibleApp::master()->createWindow( Window::Format().size( 400, 400 ) );
    mWindow = wref;
    wref->setUserData( this );

    mId = CVisibleApp::master()->getNumWindows();
    size_t idd = Id();
    wref->getSignalClose().
    connect([idd,this] { (static_cast<CVisibleApp*>(CVisibleApp::master()))->remove_from (idd); } );
    wref->getSignalDraw().connect (std::bind(&movContext::draw_window, this) );
    wref->connectMouseDown(&movContext::mouseDown, this);
    wref->connectMouseUp(&movContext::mouseUp, this);
    wref->connectMouseDrag(&movContext::mouseDrag, this);
    wref->connectMouseMove(&movContext::mouseMove, this);
    assert(CVisibleApp::master()->getWindowIndex(idd-1)->getUserData<movContext>()->Id() == idd);
    static std::string caption = "mov Viewer # ";
    wref->setTitle (caption + stl_utils::tostr(mId));
    
}

movContext::~movContext ()
{

}

void movContext::setup()
{
    // Browse for the movie file
    fs::path moviePath = getOpenFilePath();
    m_valid = false;
    
  	getWindow()->setTitle( moviePath.filename().string() );
    
    mMovieParams = params::InterfaceGl( "Movie Controller", Vec2i( 200, 300 ) );
    
    if ( ! moviePath.empty () )
    {
        CVisibleApp::master()->console () << moviePath.string ();
        loadMovieFile (moviePath);
    }
    
    if( m_valid )
    {
        string max = ci::toString( m_movie.getDuration() );
        mMovieParams.addParam( "Position", &mMoviePosition, "min=0.0 max=" + max + " step=0.5" );
        mMovieParams.addParam( "Rate", &mMovieRate, "step=0.01" );
        mMovieParams.addParam( "Play/Pause", &mMoviePlay );
        mMovieParams.addParam( "Loop", &mMovieLoop );
    }
}

void movContext::clear_movie_params ()
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

void movContext::loadMovieFile( const fs::path &moviePath )
{
	
	try {
		m_movie = qtime::MovieGl( moviePath );
        m_valid = m_movie.checkPlayable ();
        
        if (m_valid)
        {
            CVisibleApp::master()->console() << "Dimensions:" <<m_movie.getWidth() << " x " <<m_movie.getHeight() << std::endl;
            CVisibleApp::master()->console() << "Duration:  " <<m_movie.getDuration() << " seconds" << std::endl;
            CVisibleApp::master()->console() << "Frames:    " <<m_movie.getNumFrames() << std::endl;
            CVisibleApp::master()->console() << "Framerate: " <<m_movie.getFramerate() << std::endl;
            
            //   m_movie.setLoop( true, false );
            //   m_movie.play();
            m_fc = m_movie.getNumFrames ();
        }
	}
	catch( ... ) {
		CVisibleApp::master()->console() << "Unable to load the movie." << std::endl;
		return;
	}
	
}


Rectf movContext::render_box ()
{
    Rectf rb;
    if (! m_valid ) return rb;
    ci::app::WindowRef wref = mWindow.lock ();
    if (wref)
        rb = Area (wref->getPos(), wref->getSize());
    return rb;
}

void movContext::seek( size_t xPos )
{
    //  auto waves = mWaveformPlot.getWaveforms ();
    //	if (have_sampler () ) mSamplePlayer->seek( waves[0].sections() * xPos / mGraphDisplayRect.getWidth() );
    
    if (have_movie ()) mMovieIndexPosition = movContext::Normal2Index ( render_box (), xPos, m_fc);
}


bool movContext::is_valid ()
{
    return m_valid;
}

void movContext::update ()
{
    
    if( m_valid )
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

void movContext::draw ()
{
    mMovieParams.draw();
    if (m_valid && mImage )
    {
        gl::draw (mImage, render_box ());
        mImage.disable ();
    }
    
}


void movContext::draw_window ()
{
    ci::app::WindowRef wref = mWindow.lock ();
    if (! wref || ! getWindow()->getUserData<movContext>()->is_valid() ) return;
    
    mMovieParams.draw();
    
    if (m_valid && mImage )
    {
        gl::draw (mImage, render_box ());
        mImage.disable ();
    }
    
}



// ClipContext

clipContext::clipContext ()
{
    ci::app::WindowRef wref = CVisibleApp::master()->createWindow( Window::Format().size( 400, 400 ) );
    mWindow = wref;
    wref->setUserData( this );
    mId = CVisibleApp::master()->getNumWindows();
    size_t idd = Id();
    wref->getSignalClose().
    connect([idd,this] { (static_cast<CVisibleApp*>(CVisibleApp::master()))->remove_from (idd); } );
    wref->getSignalDraw().connect (std::bind(&clipContext::draw_window, this) );
    wref->connectMouseDown(&clipContext::mouseDown, this);
    wref->connectMouseUp(&clipContext::mouseUp, this);
    wref->connectMouseDrag(&clipContext::mouseDrag, this);
    wref->connectMouseMove(&clipContext::mouseMove, this);
    assert(CVisibleApp::master()->getWindowIndex(idd-1)->getUserData<clipContext>()->Id() == idd);
    static std::string caption = "Result Time Series Viewer # ";
    wref->setTitle (caption + stl_utils::tostr(mId));
    
    
}

clipContext::~clipContext()
{

}

Rectf clipContext::render_box ()
{
    Rectf rb;
    if (! m_valid ) return rb;
    ci::app::WindowRef wref = mWindow.lock ();
    if (wref)
    
    rb = Area (wref->getPos(), wref->getSize());
    return rb;
}


void clipContext::draw ()
{
    // time cycles every 1 / TWEEN_SPEED seconds, with a 50% pause at the end
    float time = math<float>::clamp( fmod( CVisibleApp::master()->getElapsedSeconds() * 0.5, 1 ) * 1.5f, 0, 1 );
    if (mGraph1D) mGraph1D->draw ( time );
}


void clipContext::draw_window ()
{
    ci::app::WindowRef wref = mWindow.lock ();
     if ( ! wref || ! getWindow()->getUserData<clipContext>()->is_valid() ) return;
    // time cycles every 1 / TWEEN_SPEED seconds, with a 50% pause at the end
    float time = math<float>::clamp( fmod( CVisibleApp::master()->getElapsedSeconds() * 0.5, 1 ) * 1.5f, 0, 1 );
    if (mGraph1D) mGraph1D->draw ( time );
}


void clipContext::update () {}

bool clipContext::is_valid () { return m_valid; }

void clipContext::setup()
{
    // Browse for the movie file
    mPath = getOpenFilePath();
    m_valid = false;
  	getWindow()->setTitle( mPath.filename().string() );
    const std::string& fqfn = mPath.string ();
    m_columns = m_rows = 0;
    mdat.resize (0);
    bool c2v_ok = vf_utils::csv::csv2vectors ( fqfn, mdat, false, true, true);
    if ( c2v_ok )
        
        m_columns = mdat.size ();
    m_rows = mdat[0].size();
    // only handle case of long columns of data
    if (m_columns < m_rows)
    {
        m_column_select = (m_column_select >= 0 && m_column_select < m_columns) ? m_column_select : 0;
        m_frames = m_file_frames = m_rows;
        m_read_pos = 0;
        m_valid = true;
    }

    if (m_valid)
    {
        mGraph1D = Graph1DRef (new graph1D ("signature", render_box () ) );
        mGraph1D->addListener( this, &clipContext::receivedEvent );
        mGraph1D->load_vector(mdat[m_column_select]);

    }
    
    
    mClipParams = params::InterfaceGl (" Clip ", Vec2i( 200, 400) );
    mClipParams.addParam("Column ", &m_column_select, "min=0 max=" + stl_utils::tostr(m_columns)) ;

    //        string max = ci::toString( m_movie.getDuration() );
    //        mMovieParams.addParam( "Column", &m_column_select, "min=0 max=" + columns );
}


void clipContext::receivedEvent( InteractiveObjectEvent event )
{
    string text;
    switch( event.type )
    {
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
    CVisibleApp::master()->console() << "Received " + text << endl;
}


void clipContext::mouseMove( MouseEvent event )
{
   if (mGraph1D) mGraph1D->mouseMove( event );
}


void clipContext::mouseDrag( MouseEvent event )
{
   if (mGraph1D) mGraph1D->mouseDrag( event );
}


void clipContext::mouseDown( MouseEvent event )
{
   if (mGraph1D) mGraph1D->mouseDown( event );
}


void clipContext::mouseUp( MouseEvent event )
{
   if (mGraph1D) mGraph1D->mouseUp( event );
}

