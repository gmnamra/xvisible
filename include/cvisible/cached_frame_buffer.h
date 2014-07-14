
#ifndef _Cached_Buffer_H_
#define _Cached_Buffer_H_


#include "rc_types.h"
#include "rc_thread.h"
#include "rc_framebuf.h"
#include "rc_timestamp.h"

#include "vf_utils.hpp"

class cached_frame_ref : public rcFrameRef
{
//    // Constructors
//    cached_frame_ref() : rcFrameRef ()  {}
//    cached_frame_ref( rcFrame* p ) : rcFrameRef (p) {}
//
//    cached_frame_ref( const cached_frame_ref& p )
//    : mCacheCtrl ( p.mCacheCtrl ), mFrameIndex ( p.mFrameIndex ), mFrameBuf( p.mFrameBuf )
//    {
//        if ( mFrameBuf ) {
//            rcLock frmLock(getMutex());
//            mFrameBuf->addRef();
//        }
//    }
    using rcFrameRef::rcFrameRef;
    
public:
    // Destructor
    ~cached_frame_ref()
    {
        const_free_unlock(true);
    }

    void lock () const;
    void unlock () const;
    void prefetch () const;
    
private:
    void const_free_lock ();
    void const_free_unlock (bool);
};



#endif
