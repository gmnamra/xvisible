
#ifndef _Cached_Buffer_H_
#define _Cached_Buffer_H_


#include "rc_types.h"
#include "rc_thread.h"
#include "rc_framebuf.h"
#include "rc_timestamp.h"

#include "vf_utils.hpp"

#if 0
class cached_frame_ref : public rcFrameRef
{
  //  @note: Not "using rcFrameRef::rcFrameRef; " because the compiler will generate default ctor regardless
    
public:
    
    
    
    // Constructors
    cached_frame_ref() : rcFrameRef () {}
    cached_frame_ref( rcFrame* p ) : rcFrameRef (p) {}
    cached_frame_ref( const cached_frame_ref& p ) : rcFrameRef (p) {}

    
    // Destructor
    virtual ~cached_frame_ref();
   
    // Assignment operators
    virtual cached_frame_ref& operator= ( const cached_frame_ref& p );
    virtual cached_frame_ref& operator= ( rcFrame* p ) override;
   
    void lock () const;
    void unlock () const;
    void prefetch () const;
    
private:
    void const_free_lock ();
    virtual void interlock_force_unlock () override;
   
};
#endif


class  cached_frame_ref
{
    
public:
    
    typedef boost::function<void(uint32, uint32)> locker_fn;
    
    // Constructors
  cached_frame_ref();
  cached_frame_ref( rcFrame* p );
  cached_frame_ref( const cached_frame_ref& p );
    
    // Destructor
    virtual ~cached_frame_ref();
      
    // Assignment operators
    virtual cached_frame_ref& operator= ( const cached_frame_ref& p );
    virtual cached_frame_ref& operator= ( rcFrame* p );
    
    /* For cached frames, the cache controller is called to load the
     * underlying frame from either cache or disk and to lock it into
     * the cache so it is safe to use it.
     *
     * Note: This is a NOOP for uncached frames.
     */
    virtual void lock() const;
    
    /* For cached frames, the cache controller is called to mark this
     * frame as available for reuse. The frame's data is retained in
     * cache, so that a subsequent call to lock() can retrieve the
     * data. Caching is controlled using a least-recently-used
     * algorithm.
     *
     * WARNING: Any rcFrame related memory references, such as the
     * pointer returned by rawData, or a reference to the rcFrame
     * object itself is only valid while the frame is locked. To use
     * these values again, it is necessary to relock the frame and
     * then reread these values.
     *
     * Note: This is a NOOP for uncached frames.
     */
    virtual void unlock() const;
    
    /* Ask cache controller to try and read in this frame now without
     * actually locking it to this object.
     */
    virtual void prefetch() const;
    
    uint32 frameIndex() const {
        return mFrameIndex;
    }
    
    // Comparison operators
  bool operator== ( const cached_frame_ref& p ) const;
  bool operator!= ( const cached_frame_ref& p ) const;
  bool operator== ( const rcFrame* p ) const;
  bool operator!= ( const rcFrame* p ) const;
  bool operator!() const;
    
    // Accessors
  operator rcFrame*() const;
  const rcFrame& operator*() const;
  rcFrame& operator*();
  const rcFrame* operator->() const;
  rcFrame* operator->();
  int refCount() const;

  void printState() const;
    
    /* Helper fct intended for use by rcVideoCache class only. It allows
     * a shared frame buffer pointer to be initialized pointing to a
     * cached frame.
     */
    void setCachedFrame(cached_frame_ref& p, uint32 cacheID,
                        uint32 frameIndex);
    
    /* Helper fct intended for use by rcVideoCache class only. It allows
     * a shared frame buffer pointer to be initialized pointing to a
     * cached frame without reading that frame into memory.
     */
  void setCachedFrameIndex(uint32 cacheID, uint32 frameIndex);
    
    /*
     * public but only to avoid excessive friendship
     */
    uint32      mCacheCtrl;
    uint32      mFrameIndex;
    mutable rcFrame*    mFrameBuf;
    mutable std::mutex mMutex;
    
protected:
    virtual void internalLock();
    virtual void interlock_force_unlock ();
    void internalUnlock( bool , locker_fn );
    void const_free_lock();
   

};


#endif
