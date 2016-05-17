#ifndef __FRAMEBUFFERMANAGER__
#define __FRAMEBUFFERMANAGER__


#include <memory>

#include "membufCache.hpp"
#include "core/stl_utils.hpp"

typedef uint8_t byte;

class DS4MemoryAllocator
{
public:
    
    // This function should create the memory pool with the desired size.
    // The function may be called several time - then several pools will be created (with different sizes)
    // Should be called on "StartCapture"
    virtual void CreateMemoryPool(unsigned int size) = 0;
    
    
    // The function will find or allocate a new memory buffer from the pool with size "size".
    virtual byte* GetMemoryBuffer( unsigned int size ) = 0;
    
    
    // Releases the buffer previously allocated by GetMemoryBuffer
    virtual void ReleaseBuffer(byte* buffer) = 0;
    
    // Frees memory pool of size "size". If size is not specified = frees all pools.
    // Should be called on "StopCapture"
    virtual void DestroyMemoryPool(unsigned int size = 0) = 0;
    
    
    // DSAPI should also have an interface to register the allocator object with DSAPI
    // in DSAPI
    // DCM will register DS4Memoryallocator once it creates DSAPI object
    static void RegisterMemoryAllocator(DS4MemoryAllocator*);
};


class DS4StaticMemoryAllocator: DS4MemoryAllocator
{
public:
    typedef std::shared_ptr<DS4MemoryAllocator> DSAPIAllocatorRef;
    
    static DSAPIAllocatorRef createDefaultAllocator ();

    // This function should create the memory pool with the desired size.
    // The function may be called several time - then several pools will be created (with different sizes)
    // Should be called on "StartCapture"
    void CreateMemoryPool(unsigned int size);
    
    
    // The function will find or allocate a new memory buffer from the pool with size "size".
    byte* GetMemoryBuffer( unsigned int size );
    
    // Releases the buffer previously allocated by GetMemoryBuffer
    void ReleaseBuffer(byte* buffer);
    
    // Frees memory pool of size "size". If size is not specified = frees all pools.
    // Should be called on "StopCapture"
    void DestroyMemoryPool(unsigned int size = 0);

private:
    class spImpl;
    std::shared_ptr<spImpl> _impl;
    
};




#endif


