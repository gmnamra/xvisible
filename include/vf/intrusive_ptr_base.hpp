
#ifndef __INTRUSIVE_PTR__
#define __INTRUSIVE_PTR__

#include <boost/detail/atomic_count.hpp>
#include <boost/checked_delete.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/assert.hpp>


template<class T>
struct intrusive_ptr_base
{
    intrusive_ptr_base(intrusive_ptr_base<T> const&)
    : m_refs(0) {}
    
    intrusive_ptr_base& operator=(intrusive_ptr_base const& rhs)
    { return *this; }
    
    friend void intrusive_ptr_add_ref(intrusive_ptr_base<T> const* s)
    {
        BOOST_ASSERT(s->m_refs >= 0);
        BOOST_ASSERT(s != 0);
        ++s->m_refs;
    }
    
    friend void intrusive_ptr_release(intrusive_ptr_base<T> const* s)
    {
        BOOST_ASSERT(s->m_refs > 0);
        BOOST_ASSERT(s != 0);
        if (--s->m_refs == 0)
            boost::checked_delete(static_cast<T const*>(s));
    }
    
    boost::intrusive_ptr<T> self()
    { return boost::intrusive_ptr<T>((T*)this); }
    
    boost::intrusive_ptr<const T> self() const
    { return boost::intrusive_ptr<const T>((T const*)this); }
    
    int refcount() const { return m_refs; }
    
    intrusive_ptr_base(): m_refs(0) {}
private:
    // reference counter for intrusive_ptr
    mutable boost::detail::atomic_count m_refs;
};




#endif
