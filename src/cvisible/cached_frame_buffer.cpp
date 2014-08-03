#include "cached_frame_buffer.h"
#include <stdio.h>
#include "rc_types.h"
#include "qtime_cache.h"



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
    const_cast<cached_frame_ref*>(this)->internalUnlock(false);
}

void cached_frame_ref::internalUnlock( bool force )
{
    if ( mFrameBuf && (mCacheCtrl || force))
    {
        bool cacheUnlock = false;
        {
            uint32 rc = mFrameBuf->remRef();
            if (mCacheCtrl && rc == 1) cacheUnlock = true;
            mFrameBuf = 0;
        }
        if (cacheUnlock)
            QtimeCache::cacheUnlock(mCacheCtrl, mFrameIndex);
    }
    
}

void cached_frame_ref::prefetch() const
{
    if ((mFrameBuf == 0) && mCacheCtrl)
    {
        QtimeCache::cachePrefetch(mCacheCtrl, mFrameIndex);
    }
}


