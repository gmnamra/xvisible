#pragma once

#include "cinder/Rect.h"
#include "cinder/Color.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "cinder/Function.h"


class InteractiveObject;

class InteractiveObjectEvent: public ci::app::Event{
public:
    enum EventType{ Pressed, PressedOutside, Released, ReleasedOutside, RolledOut, RolledOver, Dragged };
    
    InteractiveObjectEvent( InteractiveObject *sender, EventType type ){
        this->sender = sender;
        this->type = type;
    }
    
    InteractiveObject *sender;
    EventType type;
};

class InteractiveObject{
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
    ci::CallbackId addListener( T* listener, void (T::*callback)(InteractiveObjectEvent) ){
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

class Button: public InteractiveObject{
public:
    Button( const ci::Rectf& rect, ci::gl::Texture idleTex, ci::gl::Texture overTex, ci::gl::Texture pressTex ):InteractiveObject( rect )
    {
        mIdleTex = idleTex;
        mOverTex = overTex;
        mPressTex = pressTex;
    }
    
    virtual void draw(){
        if( mPressed ){
            ci::gl::draw( mPressTex, rect );
        } else if( mOver ){
            ci::gl::draw( mOverTex, rect );
        } else {
            ci::gl::draw( mPressTex, rect );
        }
    }
    
private:
    ci::gl::Texture mIdleTex, mOverTex, mPressTex; 
};
