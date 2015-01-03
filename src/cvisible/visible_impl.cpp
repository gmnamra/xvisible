
#include "ui_contexts.h"
#include "stl_util.hpp"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"
#include "cinder/Timer.h"
#include "cinder/Camera.h"
#include "cinder/qtime/Quicktime.h"
#include "cinder/params/Params.h"

#include "cinder/ImageIo.h"
#include "assets/Resources.h"

using namespace vf_utils::csv;


template<> matContext* uContext::create_context (const std::string&);
template<> movContext* uContext::create_context (const std::string&);
template<> clipContext* uContext::create_context (const std::string&);

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
    
    
#if 0
    void copy_to_vector (const ci::audio2::BufferRef &buffer, std::vector<float>& mBuffer)
    {
        mBuffer.clear ();
        const float *reader = buffer->getChannel( 0 );
        for( size_t i = 0; i < buffer->getNumFrames(); i++ )
            mBuffer.push_back (*reader++);
    }
    size_t Wave2Index (const Rectf& box, const size_t& pos, const Waveform& wave)
    {
        size_t xScaled = (pos * wave.sections()) / box.getWidth();
        xScaled *= wave.section_size ();
        xScaled = math<size_t>::clamp( xScaled, 0, wave.samples () );
    }
    
#endif

  }



void uContextRegistry::print_out ()
{
    for (auto t = m_registry().begin(); t != m_registry().end(); t++)
    {
      //  std::cout << t->first << "        " << ((uContextHolderPtr)t->second)->u_Context() << std::endl;
    }
}

uContext * uContextRegistry::uContext(const std::string & name)
{
    const Registry::const_iterator it(m_registry().find(name));
    if (it == m_registry().end())
        return NULL;
    return it->second->u_Context();
}

uContextRegistry::Registry & uContextRegistry::m_registry()
{
    static uContextRegistry::Registry registry;
    return registry;
}

bool uContextRegistry::instantiate (const std::string & name_str)
{
    const Registry::iterator it(m_registry().find(name_str));
    if (it == m_registry().end())
        return false;
  //  if (it->second->create(name_str) == NULL)
   //     return false;
    return true;
}


bool uContextRegistry::unregister (const std::string & name)
{
    const Registry::iterator it(m_registry().find(name));
    if (it == m_registry().end())
        return false;
    if (it->second == NULL)
        return false;
    m_registry().erase (name);
    const Registry::iterator fit(m_registry().find(name));
    return fit == m_registry().end();
}


int uContextRegistry::size ()
{
    return m_registry().size();
}

///////////////  Matrix Viewer ////////////////////

matContext::matContext (const std::string& name_str) : mName (name_str)
{
    setup ();
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
    return render_window_box ();
}
void matContext::setup ()
{
    // Browse for a matrix file
    mPath = getOpenFilePath();
    if(! mPath.empty() ) internal_setupmat_from_file(mPath);
}

void matContext::internal_setupmat_from_file (const path & fp)
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


void matContext::update()
{
    
}


bool matContext::is_valid ()
{
    return m_valid;
}

/////////////  movContext Implementation  ////////////////

movContext::movContext (const std::string& name_str) : mName (name_str)
{
    setup();
}

void movContext::setup()
{
    // Browse for the movie file
    path moviePath = getOpenFilePath();
    m_valid = false;
    
  	getWindow()->setTitle( moviePath.filename().string() );
    
    mMovieParams = params::InterfaceGl( "Movie Controller", Vec2i( 90, 160 ) );
    
    if ( ! moviePath.empty () )
    {
        ci_console () << moviePath.string ();
        loadMovieFile (moviePath);
    }
    
    if( ! m_valid ) return;
   {
        string max = to_string( m_movie.getDuration() );
       mMovieParams.addParam( "Position", &mMoviePosition, "min=0.0 max=" + max + " step=0.5" );
        mMovieParams.addParam( "Rate", &mMovieRate, "step=0.01" );
        mMovieParams.addParam( "Play/Pause", &mMoviePlay );
        mMovieParams.addParam( "Loop", &mMovieLoop );
       mMovieParams.addSeparator();
       mMovieParams.addParam( "Zoom", &mMovieCZoom, "min=0.1 max=+10 step=0.1" );
       
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
    mMovieCZoom=1.0f;
}

void movContext::loadMovieFile( const path &moviePath )
{
	
	try {
		m_movie = qtime::MovieGl( moviePath );
        m_valid = m_movie.checkPlayable ();
        
        if (m_valid)
        {
            ci_console() << "Dimensions:" <<m_movie.getWidth() << " x " <<m_movie.getHeight() << std::endl;
            ci_console() << "Duration:  " <<m_movie.getDuration() << " seconds" << std::endl;
            ci_console() << "Frames:    " <<m_movie.getNumFrames() << std::endl;
            ci_console() << "Framerate: " <<m_movie.getFramerate() << std::endl;
            
            //   m_movie.setLoop( true, false );
            //   m_movie.play();
            m_fc = m_movie.getNumFrames ();
            // assert aspacts are the same

            
        }
	}
	catch( ... ) {
		ci_console() << "Unable to load the movie." << std::endl;
		return;
	}
	
}



void movContext::mouseMove( MouseEvent event )
{
    auto dis = mMousePos.distanceSquared(event.getPos());
    mMouseIsMoving = dis > 0;
    mMousePos = event.getPos();
}



void movContext::mouseDrag( MouseEvent event )
{
    mMouseIsDragging = true;
}


void movContext::mouseDown( MouseEvent event )
{
    mMouseIsDown = true;
}


void movContext::mouseUp( MouseEvent event )
{
    mMouseIsDown = false;
    mMouseIsDragging = false;
}


Vec2f movContext::texture_to_display_zoom()
{
    Rectf textureBounds = mImage.getBounds();
    return Vec2f(m_display_rect.getWidth() / textureBounds.getWidth(),m_display_rect.getHeight() / textureBounds.getHeight());
}

Rectf movContext::render_box ()
{
    Rectf textureBounds = mImage.getBounds();
    return textureBounds.getCenteredFit( getWindowBounds(), true );
}

void movContext::seek( size_t xPos )
{
    //  auto waves = mWaveformPlot.getWaveforms ();
    //	if (have_sampler () ) mSamplePlayer->seek( waves[0].sections() * xPos / mGraphDisplayRect.getWidth()() );
    
    if (is_valid()) mMovieIndexPosition = movContext::Normal2Index ( render_box (), xPos, m_fc);
}


bool movContext::is_valid ()
{
    return m_valid;
}

void movContext::update ()
{
    ci_console() << ((mMouseIsDown) ? "V" : "A" ) << ((mMouseIsDragging) ? "G" : "-") << std::endl;
    
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
    
    if( m_movie )
    {
        mImage = m_movie.getTexture();
        mImage.setMagFilter(GL_NEAREST_MIPMAP_NEAREST);
        m_display_rect = render_box();
        m_zoom = texture_to_display_zoom();
        m_display_rect.scaleCentered(mMovieCZoom);
    }
}

void movContext::draw ()
{
  	gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
    
    if (m_valid && mImage )
    {
        gl::draw (mImage, m_display_rect);
        mImage.disable ();
    }

    mMovieParams.draw();
    
}




////////////////// ClipContext  ///////////////////

clipContext::clipContext (const std::string& name_str) : mName (name_str)
{
    setup ();
}

Rectf clipContext::render_box ()
{
    return render_window_box ();
}


void clipContext::draw ()
{
    // time cycles every 1 / TWEEN_SPEED seconds, with a 50% pause at the end
    float time = math<float>::clamp( fmod( ci_getElapsedSeconds() * 0.5, 1 ) * 1.5f, 0, 1 );
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
    ci_console() << "Received " + text << endl;
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

