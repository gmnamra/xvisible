
#ifndef _Cached_Buffer_H_
#define _Cached_Buffer_H_


#include "rc_types.h"
#include "rc_thread.h"
#include "rc_framebuf.h"
#include "rc_timestamp.h"

#include "vf_utils.hpp"

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
