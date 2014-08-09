#include "cached_frame_buffer.h"
#include <stdio.h>
#include "rc_types.h"
#include "qtime_cache.h"


void cached_frame_ref::interlock_force_unlock ()
{
    rcFrameRef::internalUnlock(true, &QtimeCache::cacheUnlock);
 //   std::cout << "cache_frame_ref" << __FUNCTION__ << std::endl;
}

// Assignment operators
cached_frame_ref& cached_frame_ref::operator= ( const cached_frame_ref& p )
{
    if ( (mFrameBuf == p.mFrameBuf) && (mCacheCtrl == p.mCacheCtrl) &&
        (mFrameIndex == p.mFrameIndex) )
        return *this;
    
    internalUnlock(true, &QtimeCache::cacheUnlock);
    
    mCacheCtrl = p.mCacheCtrl;
    mFrameIndex = p.mFrameIndex;
    mFrameBuf = p.mFrameBuf;
    if ( mFrameBuf )
    {
        mFrameBuf->addRef();
    }
    return *this;
}

cached_frame_ref& cached_frame_ref::operator= ( rcFrame* p )
{
    if ( (mFrameBuf == p) && (mFrameBuf || !mCacheCtrl) )
        return *this;
    
    internalUnlock(true,&QtimeCache::cacheUnlock);
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

cached_frame_ref::~cached_frame_ref()
{
 //   std::cout << "cache_frame_ref dtor called" << std::endl;
    interlock_force_unlock();
}

void cached_frame_ref::lock() const
{
    const_cast<cached_frame_ref*>(this)->const_free_lock ();
}

void cached_frame_ref::const_free_lock()
{
    if ( !mFrameBuf && mCacheCtrl )
    {
        eQtimeCacheStatus status;
        eQtimeCacheError error;
        status = QtimeCache::cacheLock(mCacheCtrl, mFrameIndex, *this, &error);
    }
    
}


void cached_frame_ref::unlock() const
{
    const_cast<cached_frame_ref*>(this)->internalUnlock(false,&QtimeCache::cacheUnlock);
}


void cached_frame_ref::prefetch() const
{
    if ((mFrameBuf == 0) && mCacheCtrl)
    {
        QtimeCache::cachePrefetch(mCacheCtrl, mFrameIndex);
    }
}


