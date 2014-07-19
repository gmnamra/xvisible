
#ifndef _Cached_Buffer_H_
#define _Cached_Buffer_H_


#include "rc_types.h"
#include "rc_thread.h"
#include "rc_framebuf.h"
#include "rc_timestamp.h"

#include "vf_utils.hpp"

class cached_frame_ref : public rcFrameRef
{
    using rcFrameRef::rcFrameRef;
    
public:
    // Destructor
    ~cached_frame_ref()
    {
        internalUnlock (true);
    }

    // Assignment operators
    cached_frame_ref& operator= ( const cached_frame_ref& p )
    {
        if ( (mFrameBuf == p.mFrameBuf) && (mCacheCtrl == p.mCacheCtrl) &&
            (mFrameIndex == p.mFrameIndex) )
            return *this;
        
        internalUnlock(true);
        
        mCacheCtrl = p.mCacheCtrl;
        mFrameIndex = p.mFrameIndex;
        mFrameBuf = p.mFrameBuf;
        if ( mFrameBuf )
        {
            mFrameBuf->addRef();
        }
        return *this;
    }
    
    cached_frame_ref& operator= ( rcFrame* p )
    {
        if ( (mFrameBuf == p) && (mFrameBuf || !mCacheCtrl) )
            return *this;

        internalUnlock(true);
        mFrameBuf = p;
        if ( mFrameBuf )
        {
            mCacheCtrl = p->cacheCtrl();
            mFrameIndex = p->frameIndex();
            mFrameBuf->addRef();
        }
        else {
            mCacheCtrl = 0;
            mFrameIndex = 0;
        }
        
        return *this;
    }

    void lock () const;
    void unlock () const;
    void prefetch () const;
    
private:
    void const_free_lock ();
    void internalUnlock( bool force );
};



#endif
