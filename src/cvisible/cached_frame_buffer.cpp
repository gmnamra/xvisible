#include "cached_frame_buffer.h"
#include <stdio.h>
#include "vf_types.h"
#include "qtime_cache.h"


void cached_frame_ref::interlock_force_unlock ()
{
    internalUnlock(true, &QtimeCache::cacheUnlock);
 //   std::cout << "cache_frame_ref" << __FUNCTION__ << std::endl;
}

cached_frame_ref::cached_frame_ref()
    : mCacheCtrl ( 0 ), mFrameIndex ( 0 ), mFrameBuf( 0 )   
{
}

cached_frame_ref::cached_frame_ref( const cached_frame_ref& p )
    : mCacheCtrl ( p.mCacheCtrl ), mFrameIndex ( p.mFrameIndex ), mFrameBuf( p.mFrameBuf )
{
        if ( mFrameBuf )
        {
            mFrameBuf->addRef();
        }
 }

cached_frame_ref::cached_frame_ref( raw_frame* p)
    : mCacheCtrl ( 0 ), mFrameIndex ( 0 ), mFrameBuf( p )
{
        if (p) {
            mCacheCtrl = p->cacheCtrl();
            mFrameIndex = p->frameIndex();
        }
        if ( mFrameBuf )
        {
            mFrameBuf->addRef();
        }
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

cached_frame_ref& cached_frame_ref::operator= ( raw_frame* p )
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
        QtimeCacheStatus status;
        QtimeCacheError error;
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


    
    // Comparison operators
    bool cached_frame_ref::operator== ( const cached_frame_ref& p ) const {
        if ( mCacheCtrl || p.mCacheCtrl )
            return ((mCacheCtrl == p.mCacheCtrl) &&
                    (mFrameIndex == p.mFrameIndex));
        return ( mFrameBuf == p.mFrameBuf );
    }
    bool cached_frame_ref::operator!= ( const cached_frame_ref& p ) const {
        return !(this->operator==( p ));
    }
    bool cached_frame_ref::operator== ( const raw_frame* p ) const {
        if ( mCacheCtrl )
            return (p && (mCacheCtrl == p->cacheCtrl()) &&
                    (mFrameIndex == p->frameIndex()));
        return ( mFrameBuf == p );
    }
    bool cached_frame_ref::operator!= ( const raw_frame* p ) const {
        return !(this->operator==( p ));
    }
    bool cached_frame_ref::operator!() const {
        return ( ( mFrameBuf == 0 ) && (mCacheCtrl == 0));
    }
    
    // Accessors
    cached_frame_ref::operator raw_frame*() const {
        if ( !mFrameBuf && mCacheCtrl ) lock();
            return mFrameBuf;
    }
    const raw_frame& cached_frame_ref::operator*() const {
        if ( !mFrameBuf && mCacheCtrl ) lock();
        assert( mFrameBuf );
        return *mFrameBuf;
    }
    raw_frame& cached_frame_ref::operator*() {
        if ( !mFrameBuf && mCacheCtrl ) lock();
        assert( mFrameBuf );
        return *mFrameBuf;
    }
    const raw_frame* cached_frame_ref::operator->() const {
        if ( !mFrameBuf && mCacheCtrl ) lock();
        return mFrameBuf;
    }
    raw_frame* cached_frame_ref::operator->() {
        if ( !mFrameBuf && mCacheCtrl ) lock();
        return mFrameBuf;
    }
    
    int cached_frame_ref::refCount() const {
        int retVal = 0;
        if ( !mFrameBuf && mCacheCtrl ) lock();
        if ( mFrameBuf )
        {
            retVal = mFrameBuf->refCount();
        }
        return retVal;
    }




    
    void cached_frame_ref::printState() const {
        cerr << "fb c, i, b [" << mCacheCtrl << ", " << mFrameIndex << ", " << mFrameBuf << "]" << endl;
    }

    
    /* Helper fct intended for use by rcVideoCache class only. It allows
     * a shared frame buffer pointer to be initialized pointing to a
     * cached frame.
     */
    void cached_frame_ref::setCachedFrame(cached_frame_ref& p, uint32 cacheID,
                        uint32 frameIndex)
    {
        assert (p.mFrameBuf);
        assert (!mFrameBuf);
        assert (cacheID);
        mFrameBuf = p.mFrameBuf;
        mCacheCtrl = cacheID;
        mFrameIndex = frameIndex;
        mFrameBuf->addRef();
    }
    
    /* Helper fct intended for use by rcVideoCache class only. It allows
     * a shared frame buffer pointer to be initialized pointing to a
     * cached frame without reading that frame into memory.
     */
    void cached_frame_ref::setCachedFrameIndex(uint32 cacheID, uint32 frameIndex)
    {
        assert (!mFrameBuf);
        assert (cacheID);
        mCacheCtrl = cacheID;
        mFrameIndex = frameIndex;
    }



void cached_frame_ref::internalLock()
{
    if ( !mFrameBuf && mCacheCtrl )
    {
        QtimeCacheStatus status;
        QtimeCacheError error;
        status = QtimeCache::cacheLock(mCacheCtrl, mFrameIndex, *this, &error);

#ifdef DEBUG_MEM
        cerr << "   Cached rcSharedFrameBufPtr " << mFrameBuf
        << " Status " << status << " refCount ";
        std::lock_guard<std::mutex> lk (mMutex);
        
        if ( mFrameBuf )
            cerr << mFrameBuf->refCount() << endl;
        else
            cerr << "UNDEFINED" << endl;
#endif
        
    }
}

void cached_frame_ref::internalUnlock(bool force, locker_fn locker)
{

    if ( mFrameBuf && (mCacheCtrl || force))
      {
        bool cacheUnlock = false;
        {
            std::lock_guard<std::mutex> lk (mMutex);
            
            uint32 rc = mFrameBuf->remRef();
            if (mCacheCtrl && rc == 1) cacheUnlock = true;

            mFrameBuf = 0;
        }
        if (cacheUnlock)
            locker (mCacheCtrl, mFrameIndex);
    }
}
