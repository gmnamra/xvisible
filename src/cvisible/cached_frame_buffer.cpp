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

#ifdef DEBUG_MEM
    cerr << "cached_frame_ref lock: " << mFrameBuf << endl;
#endif
    if ( !mFrameBuf && mCacheCtrl )
    {
        eQtimeCacheStatus status;
        eQtimeCacheError error;
        status = QtimeCache::cacheLock(mCacheCtrl, mFrameIndex, *this, &error);

#ifdef DEBUG_MEM
        cerr << "   Cached cached_frame_ref " << mFrameBuf
        << " Status " << status << " refCount ";
        rcLock frmLock(getMutex());
        if ( mFrameBuf )
            cerr << mFrameBuf->refCount() << endl;
        else
            cerr << "UNDEFINED" << endl;
#endif
    }
    
}


void cached_frame_ref::unlock() const
{
    const_cast<cached_frame_ref*>(this)->const_free_unlock (false);
}

void cached_frame_ref::const_free_unlock(bool force) 
{
#ifdef DEBUG_MEM
    cerr << "cached_frame_ref unlock: " << mFrameBuf << endl;
#endif
    if ( mFrameBuf && (mCacheCtrl || force))
    {
        bool cacheUnlock = false;
        {
            rcLock frmLock(getMutex());
            
            mFrameBuf->remRef();
            if ((mFrameBuf->refCount() == 1) && mCacheCtrl)
                cacheUnlock = true;
            
#ifdef DEBUG_MEM
            cerr << "   refCount " << mFrameBuf->refCount()
            << " cache unlock " << cacheUnlock << endl;
#endif
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


