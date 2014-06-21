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
#include "visible_app.h"

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
  //  virtual ~uContext () {}
    virtual const std::string & name() const = 0;
    virtual void name (const std::string& ) = 0;
    
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
    
    enum Type {
        matrix_viewer = 0,
        qtime_viewer = matrix_viewer+1,
        clip_viewer = qtime_viewer+1,
        viewer_types = clip_viewer+1
    };
    
    template<class T>
    static T* create_context (const std::string& name_str)
    {
        T* nm = new T(name_str);
        ci::app::WindowRef wref = AppBasic::get()->createWindow( Window::Format().size( 400, 400 ) );
        wref->setUserData( nm );
        wref->getSignalClose().
        connect([name_str,nm] { (dynamic_cast<CVisibleApp*>(AppBasic::get()))->remove_from (name_str); } );
        wref->getSignalDraw().connect (std::bind(&T::draw, nm) );
        wref->connectMouseDown(&T::mouseDown, nm);
        wref->connectMouseUp(&T::mouseUp, nm);
        wref->connectMouseDrag(&T::mouseDrag, nm);
        wref->connectMouseMove(&T::mouseMove, nm);
        wref->setTitle (nm->caption () + nm->name ());
        return nm;
    }
    
 protected:
    bool m_valid;
};



class matContext : public uContext
{
public:
    matContext(const std::string& name);
    static const std::string& caption () { static std::string cp ("Smm Viewer # "); return cp; }

    virtual const std::string & name() const { return mName; }
    virtual void name (const std::string& name) { mName = name; }
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    virtual void mouseDrag( MouseEvent event );
    virtual void mouseDown( MouseEvent event );
    void draw_window ();
    void remove_from ();
    
private:

    void internal_setupmat_from_file (const fs::path &);
    void internal_setupmat_from_mat (const vf_utils::csv::matf_t &);
    Rectf render_box ();
    params::InterfaceGl         mMatParams;
    
    gl::VboMesh mPointCloud;
    MayaCamUI mCam;
    fs::path mPath;
    std::string mName;
};


class movContext : public uContext
{
public:
    movContext(const std::string& name);
    static const std::string& caption () { static std::string cp ("Qtime Viewer # "); return cp; }
    virtual const std::string & name() const { return mName; }
    virtual void name (const std::string& name) { mName = name; }
    virtual void draw ();
    virtual void setup ();
    virtual bool is_valid ();
    virtual void update ();
    void draw_window ();
    void remove_from ();
    
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
    std::string mName;
    params::InterfaceGl         mMovieParams;
    float mMoviePosition, mPrevMoviePosition;
    size_t mMovieIndexPosition, mPrevMovieIndexPosition;
    float mMovieRate, mPrevMovieRate;
    bool mMoviePlay, mPrevMoviePlay;
    bool mMovieLoop, mPrevMovieLoop;
    
    fs::path mPath;
    
    static size_t Normal2Index (const Rectf& box, const size_t& pos, const size_t& wave)
    {
        size_t xScaled = (pos * wave) / box.getWidth();
        xScaled = math<size_t>::clamp( xScaled, 0, wave );
    }
    
};


class clipContext : public uContext
{
public:
    clipContext(const std::string& name);
    static const std::string& caption () { static std::string cp ("Result Clip Viewer # "); return cp; }    
    virtual const std::string & name() const { return mName; }
    virtual void name (const std::string& name) { mName = name; }
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
    std::string mName;
    std::vector<vector<float> > mdat;
    int m_column_select;
    size_t m_frames, m_file_frames, m_read_pos;
    size_t m_rows, m_columns;
};


class uContextHolder
{
public:
    template <class P>
    uContextHolder(vf_utils::id<P>, const std::string&);
    ~uContextHolder() {} // Needed for std::unique_ptr
    uContext * u_Context () const;
    uContext * create(const std::string& );
    
private:
    uContextHolder(const uContextHolder &);
    uContextHolder & operator=(const uContextHolder &);
    
    template <class P>
    class uContextFactory {
    public:
        uContext * operator()(const std::string& name) const //{ return new P(name); }
        {
            return uContext::create_context<P>(name);
        }
    };
    
    typedef boost::function<uContext * (const std::string& )> uContextFactoryFunction;
    typedef std::unique_ptr<uContext> uContextPtr;
    
    uContextFactoryFunction m_factoryFunction;
    uContextPtr m_uContext;
};

class uContextsRegistry
{
public:
    template <class P>
    static void registeruContext(const std::string & name);
    static bool unregister (const std::string & name);
    static bool instantiate (const std::string & name);
    static uContext * uContext(const std::string & name);
    static int size ();
    
private:
    typedef std::unique_ptr<uContextHolder> uContextHolderPtr;
    typedef std::map<std::string, uContextHolderPtr> Registry;
    static Registry & m_registry();
};

template<class P>
inline
uContextHolder::uContextHolder (vf_utils::id<P>, const std::string& name_str)
{
    m_factoryFunction = uContextFactory<P> ();
    m_uContext.reset(m_factoryFunction(name_str));
}


inline
uContext * uContextHolder::create(const std::string& name)
{
    m_uContext.reset(m_factoryFunction(name));
    return m_uContext.get();
}


inline
uContext * uContextHolder::u_Context() const
{
    if (!m_uContext)
        return NULL;
    return m_uContext.get();
}


template <class P>
inline
void uContextsRegistry::registeruContext(const std::string & name_str)
{
    std::cout << "Registry Count: " << m_registry().size() << " .... ";
    m_registry().insert(std::move(std::make_pair(name_str, uContextHolderPtr(new uContextHolder(vf_utils::id<P> () , name_str)))));
    std::cout << m_registry().size() << std::endl;
    
//    uContextsRegistry::uContext(name_str)->name (name_str);
    //m_registry().emplace(name, uContextHolderPtr(new uContextHolder(id<P>())));
}

template<> matContext* uContext::create_context (const std::string&);
template<> movContext* uContext::create_context (const std::string&);
template<> clipContext* uContext::create_context (const std::string&);




#endif
