#pragma once

#include "vf_types.h"
#include "cinder/Rect.h"
#include "cinder/Color.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "cinder/Function.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace boost;
class InteractiveObject;

class InteractiveObjectEvent: public ci::app::Event
{
public:
    enum EventType{ Pressed, PressedOutside, Released, ReleasedOutside, RolledOut, RolledOver, Dragged };
    
    InteractiveObjectEvent( InteractiveObject *sender, EventType type )
    {
        this->sender = sender;
        this->type = type;
    }
    
    InteractiveObject *sender;
    EventType type;
};

class InteractiveObject
{
public:
    InteractiveObject( const ci::Rectf& rect );
    virtual ~InteractiveObject();
    
    virtual void draw();
    
    virtual void pressed();
    virtual void pressedOutside();
    virtual void released();
    virtual void releasedOutside();
    virtual void rolledOver();
    virtual void rolledOut();
    virtual void dragged();
    
    void mouseDown( ci::app::MouseEvent& event );
    void mouseUp( ci::app::MouseEvent& event );
    void mouseDrag( ci::app::MouseEvent& event );
    void mouseMove( ci::app::MouseEvent& event );
    
    template< class T >
    ci::CallbackId addListener( T* listener, void (T::*callback)(InteractiveObjectEvent) )
    {
        return mEvents.registerCb( std::bind1st( std::mem_fun( callback ), listener ) );
    }
    
    void removeListener( ci::CallbackId callId );
    
    ci::Rectf rect;
    ci::Color pressedColor, idleColor, overColor, strokeColor;
    
protected:
    bool mPressed, mOver;
    ci::CallbackMgr< void(InteractiveObjectEvent) > mEvents;
};

#include "cinder/gl/Texture.h"


// Namespace collision with OpenCv

#define Vec2i ci::Vec2i
#define Vec2f ci::Vec2f
#define Vec3f ci::Vec3f

class graph1D;

typedef std::shared_ptr<graph1D> Graph1DRef;

class graph1D:  public InteractiveObject
{
   public:
 
    
	graph1D( std::string name, const ci::Rectf& display_box) : InteractiveObject(display_box)
	{
		// create label
		TextLayout text; text.clear( cinder::Color::white() ); text.setColor( Color(0.5f, 0.5f, 0.5f) );
		try { text.setFont( Font( "Futura-CondensedMedium", 18 ) ); } catch( ... ) { text.setFont( Font( "Arial", 18 ) ); }
		text.addLine( name );
		mLabelTex = cinder::gl::Texture( text.render( true ) );
	}
    
	void load_vector (const std::vector<float>& buffer)
    {
        std::unique_lock<std::mutex> lock (mutex_);
        mBuffer.clear ();
        std::vector<float>::const_iterator reader = buffer.begin ();
        while (reader != buffer.end())
        {
            mBuffer.push_back (*reader++);
        }
        
        mFn = boost::bind (&graph1D::get, _1, _2);
        lock.unlock();
        cond_.notify_one ();
    }
    
       //  bool is_valid () const { return (mFn != std::function<float (float)> () ); }
    
    float get (float tnormed) const
    {
        const std::vector<float>& buf = buffer();
        if (empty()) return -1.0;
        
        // NN
        int32 x = floor (tnormed * (buf.size()-1));
        if (x >= 0 && x < buf.size())
            return buf[x];
        else
            return -1.0f;
    }
	void draw( float t ) const
	{
		// draw box and frame
		cinder::gl::color( ci::Color( 1.0f, 1.0f, 1.0f ) );
		cinder::gl::drawSolidRect( rect );
		cinder::gl::color( ci::Color( 0.4f, 0.4f, 0.4f ) );
        ci::gl::drawStrokedRect( rect );
        ci::gl::color( ci::Color::white() );
        ci::gl::draw( mLabelTex, rect.getCenter() - mLabelTex.getSize() / 2 );
        
		// draw graph
		gl::color( ColorA( 0.25f, 0.5f, 1.0f, 0.5f ) );
		glBegin( GL_LINE_STRIP );
		for( float x = 0; x < rect.getWidth(); x += 0.25f ) {
			float y = 1.0f - mFn ( this, x / rect.getWidth() );
            ci::gl::vertex(Vec2f( x, y * rect.getHeight() ) + rect.getUpperLeft() );
		}
		glEnd();
		
		// draw animating circle
		gl::color( Color( 1, 0.5f, 0.25f ) );
		gl::drawSolidCircle( rect.getUpperLeft() + mFn ( this, t ) * rect.getSize(), 5.0f );
    }
    
    const std::vector<float>&       buffer () const { return mBuffer; }
    bool empty () const { return mBuffer.empty (); }
    
	
    std::vector<float>                   mBuffer;
	cinder::gl::Texture						mLabelTex;
    std::function<float (const graph1D*, float)> mFn;
    std::condition_variable cond_;
    mutable std::mutex mutex_;
    
    
};

