
#ifndef _iclip_H_
#define _iclip_H_



#include "cinder/gl/gl.h"

#include "cinder/Function.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"

#include "InteractiveObject.h"

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
        mLabelTex = cinder::gl::Texture::create(text.render( true ) );
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
        
        mFn = bind (&graph1D::get, std::placeholders::_1, std::placeholders::_2);
        lock.unlock();
        cond_.notify_one ();
    }
    
    //  bool is_valid () const { return (mFn != std::function<float (float)> () ); }
    
    float get (float tnormed) const
    {
        const std::vector<float>& buf = buffer();
        if (empty()) return -1.0;
        
        // NN
        int32_t x = floor (tnormed * (buf.size()-1));
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
        ci::gl::draw( mLabelTex, vec2(rect.getCenter()[0] - mLabelTex->getSize()[0] / 2, rect.getCenter()[1] - mLabelTex->getSize()[1] / 2 ));
        
		// draw graph
		gl::color( ColorA( 0.25f, 0.5f, 1.0f, 0.5f ) );
        gl::begin( GL_LINE_STRIP );
		for( float x = 0; x < rect.getWidth(); x += 0.25f ) {
			float y = 1.0f - mFn ( this, x / rect.getWidth() );
            ci::gl::vertex(vec2( x, y * rect.getHeight() ) + rect.getUpperLeft() );
		}
        gl::end();
		
		// draw animating circle
		gl::color( Color( 1, 0.5f, 0.25f ) );
		//gl::drawSolidCircle( rect.getUpperLeft() + mFn ( this, t ) * rect.getSize(), 5.0f );
        glLineWidth(2.f);
        float px = norm_pos().x * rect.getWidth();
        ci::gl::drawLine (vec2(px, 0.f), vec2(px, rect.getHeight()));
        
    }
    
    const std::vector<float>&       buffer () const { return mBuffer; }
    bool empty () const { return mBuffer.empty (); }
    
	
    std::vector<float>                   mBuffer;
	cinder::gl::TextureRef						mLabelTex;
    std::function<float (const graph1D*, float)> mFn;
    std::condition_variable cond_;
    mutable std::mutex mutex_;
    
};






#endif // _iclip_H_
