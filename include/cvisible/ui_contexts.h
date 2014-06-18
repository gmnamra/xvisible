//
//  visible_app.h
//  XcVisible
//
//  Created by Arman Garakani on 5/8/14.
//
//

#ifndef __ui_contexts___h
#define __ui_contexts___h

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Timeline.h"
#include "cinder/Timer.h"
#include "cinder/Camera.h"
#include "cinder/qtime/Quicktime.h"
#include "cinder/params/Params.h"
#include "cinder/gl/Vbo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/ImageIo.h"
#include "assets/Resources.h"
#include <string>
#include <map>
#include <memory>
#include <utility>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#if 0
#include "cinder/audio2/Buffer.h"
#include "cvisible/AudioDrawUtils.h"
#endif


#include "cvisible/vf_utils.hpp"
#include "vf_types.h"
#include "iclip.hpp"
#include <boost/integer_traits.hpp>
// use int64_t instead of long long for better portability
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>

#include <map>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;


// Namespace collision with OpenCv

#define Vec2i ci::Vec2i
#define Vec2f ci::Vec2f
#define Vec3f ci::Vec3f



// Animating signal marker
struct Signal_value
{
    Signal_value () : mValid (false) {}
    
    Signal_value (std::function<float (size_t)> fn)
    : mFn( fn ), mValid (true) {}
    
#if 0
    void at_event (const Rectf& box, const Vec2i& location, const size_t& pos,  const Waveform& wave)
    {
        size_t xScaled = (pos * wave.sections()) / box.getWidth();
        xScaled *= wave.section_size ();
        mSignalIndex = math<size_t>::clamp( xScaled, 0, wave.samples () );
        mDisplayPos = location;
    }
    
#endif
    void at_event_value (const Rectf& box, const Vec2i& location, const size_t& pos, const size_t& wave)
    {
        size_t xScaled = (pos * wave) / box.getWidth();
        mSignalIndex = math<size_t>::clamp( xScaled, 0, wave );
        mDisplayPos = location;
    }
    
    void draw(const Rectf& display)
    {
        if ( ! mValid ) return;
        
        const Vec2i& readPos = mDisplayPos;
		gl::color( ColorA( 0, 1, 0, 0.7f ) );
		gl::drawSolidRoundedRect( Rectf( readPos.x - 2, 0, readPos.x + 2, (float)display.getHeight() ), 2 );
		gl::color( Color( 1.0f, 0.5f, 0.25f ) );
		gl::drawStringCentered(toString (mFn (mSignalIndex)), readPos);
    }
    
    Vec2i mDisplayPos;
    size_t mSignalIndex;
    std::function<float (size_t)> mFn;
    bool mValid;
};


class uContext  : private boost::noncopyable
{
private:
    
public:
    uContext () : mId (uContext::invalid_id()), m_valid (false) {}
    virtual ~uContext () {}
    
    size_t Id () const { return mId; }
    
    virtual void draw () = 0;
    virtual void update () = 0;
    //    virtual void resize () = 0;
    virtual void setup () = 0;
    virtual bool is_valid () = 0;
    
    // u implementation does nothing
    virtual void mouseDown( MouseEvent event ) {}
    virtual void mouseMove( MouseEvent event ) {}
  	virtual void mouseUp( MouseEvent event ) {}
    virtual void mouseDrag( MouseEvent event ) {}
    
	virtual void keyDown( KeyEvent event ) {}
    
    typedef size_t result_type;
    BOOST_STATIC_CONSTANT(size_t, max_value = boost::integer_traits<size_t>::const_max);
    static size_t invalid_id () { return max_value; }
    
    bool friend operator < (const uContext& lh, const uContext& rh) { return lh.Id() < rh.Id() ; }
    
    enum Type {
        matrix_viewer = 0,
        qtime_viewer = matrix_viewer+1,
        clip_viewer = qtime_viewer+1,
        viewer_types = clip_viewer+1
    };
protected:
    size_t mId;
    bool m_valid;
};



class matContext : public uContext
{
public:
    matContext();
    ~matContext ();
    
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    virtual void mouseDrag( MouseEvent event );
    virtual void mouseDown( MouseEvent event );
    void draw_window ();
    void remove_from ();
    bool friend operator < (const matContext& lh, const matContext& rh) { return lh.Id() < rh.Id() ; }
    
private:
    
    void internal_setupmat_from_file (const fs::path &);
    void internal_setupmat_from_mat (const vf_utils::csv::matf_t &);
    Rectf render_box ();
    params::InterfaceGl         mMatParams;
    
    gl::VboMesh mPointCloud;
    MayaCamUI mCam;
    fs::path mPath;
    std::weak_ptr<app::Window>	mWindow;
};


class movContext : public uContext
{
public:
    movContext();
    ~movContext ();
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    void draw_window ();
    void remove_from ();
    bool friend operator < (const movContext& lh, const movContext& rh)  { return lh.Id() < rh.Id() ; }
    
    const params::InterfaceGl& ui_params ()
    {
        return mMovieParams;
    }
    
private:
    void loadMovieFile( const fs::path &moviePath );
    bool have_movie () { return m_movie_valid; }
	void seek( size_t xPos );
    void clear_movie_params ();
    Rectf render_box ();
    gl::Texture mImage;
    ci::qtime::MovieGl m_movie;
    bool m_movie_valid;
    size_t m_fc;
    
    params::InterfaceGl         mMovieParams;
    float mMoviePosition, mPrevMoviePosition;
    size_t mMovieIndexPosition, mPrevMovieIndexPosition;
    float mMovieRate, mPrevMovieRate;
    bool mMoviePlay, mPrevMoviePlay;
    bool mMovieLoop, mPrevMovieLoop;
    
    fs::path mPath;
    std::weak_ptr<app::Window>	 mWindow;
    
    static size_t Normal2Index (const Rectf& box, const size_t& pos, const size_t& wave)
    {
        size_t xScaled = (pos * wave) / box.getWidth();
        xScaled = math<size_t>::clamp( xScaled, 0, wave );
    }
    
};


class clipContext : public uContext
{
public:
    clipContext();
    ~clipContext ();
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    virtual void mouseDrag( MouseEvent event );
    virtual void mouseMove( MouseEvent event );
    virtual void mouseDown( MouseEvent event );
  	virtual void mouseUp( MouseEvent event );
    void draw_window ();
    void remove_from ();
    void receivedEvent ( InteractiveObjectEvent event );
    bool friend operator < (const clipContext& lh, const clipContext& rh)  { return lh.Id() < rh.Id() ; }
    
    
private:
    static DataSourcePathRef create (const std::string& fqfn)
    {
        return  DataSourcePath::create (fs::path  (fqfn));
    }
    
    void select_column (size_t col) { m_column_select = col; }
    
    Rectf render_box ();
    params::InterfaceGl         mClipParams;
    Graph1DRef mGraph1D;
    fs::path mPath;
    std::vector<vector<float> > mdat;
    int m_column_select;
    size_t m_frames, m_file_frames, m_read_pos;
    size_t m_rows, m_columns;
    std::weak_ptr<app::Window>	 mWindow;
};


class uContextHolder {
public:
    template <class P>
    uContextHolder(vf_utils::id<P>);
    ~uContextHolder() {} // Needed for std::unique_ptr
    uContext * create();
    
private:
    uContextHolder(const uContextHolder &);
    uContextHolder & operator=(const uContextHolder &);
    
    template <class P>
    class uContextFactory {
    public:
        uContext * operator()() const { return new P(); }
    };
    
    typedef boost::function<uContext * ()> uContextFactoryFunction;
    typedef std::unique_ptr<uContext> uContextPtr;
    
    uContextFactoryFunction m_factoryFunction;
    uContextPtr m_uContext;
};

class uContextsRegistry {
public:
    template <class P>
    static void registeruContext(const std::string & name);
    static uContext * uContext(const std::string & name);
    
private:
    typedef std::unique_ptr<uContextHolder> uContextHolderPtr;
    typedef std::map<std::string, uContextHolderPtr> Registry;
    static Registry & m_registry();
};

template<class P>
inline
uContextHolder::uContextHolder (vf_utils::id<P> )
: m_factoryFunction(uContextFactory<P>()), m_uContext ()
{
    
}


inline
uContext * uContextHolder::create()
{
    m_uContext.reset(m_factoryFunction());
    return m_uContext.get();
}


template <class P>
inline
void uContextsRegistry::registeruContext(const std::string & name)
{
    m_registry().insert(std::move(std::make_pair(name, uContextHolderPtr(new uContextHolder(vf_utils::id<P> () ) ) ) ) );
    //m_registry().emplace(name, uContextHolderPtr(new uContextHolder(id<P>())));
}


typedef boost::shared_ptr<matContext> matContextRef;
typedef boost::shared_ptr<movContext> movContextRef;
typedef boost::shared_ptr<clipContext> clipContextRef;



    
#endif
