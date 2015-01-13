

#ifndef Cachedbuffer_membufCached_hpp
#define Cachedbuffer_membufCached_hpp
#include <string.h>
#include <iostream>
#include <ostream>
#include <vector>
#include <map>
#include <assert.h>

using namespace std;
#define MSG(text) std::cout<< text << std::endl;


template< class T >
std::ostream & operator << ( std::ostream & os, const std::vector< T > & v ) {
    for ( const auto & i : v ) {
        os << i << std::endl;
    }
    return os;
}


template<class A,class B>
inline ostream& operator<<(ostream& out, const map<A,B>& m)
{
    out << "{";
    for (typename map<A,B>::const_iterator it = m.begin();
         it != m.end();
         ++it) {
        if (it != m.begin()) out << ",";
        out << it->first << "=" << &it->second;
    }
    out << "}";
    return out;
}

template<class A,class B>
inline ostream& operator<<(ostream& out, const multimap<A,B>& m)
{
    out << "{{";
    for (typename multimap<A,B>::const_iterator it = m.begin();
         it != m.end();
         ++it) {
        if (it != m.begin()) out << ",";
        out << it->first << "=" << &it->second;
    }
    out << "}}";
    return out;
}

class Cached: public enable_shared_from_this<Cached>
{
public:
    
    class buffer
    {
    public:
        buffer(int32_t id):m_id(id)
        {
            m_name = std::to_string(id);
            m_buffer = new uint8_t[m_id];
            cout << m_id << " , " << m_name << " + " << endl;
        }
        ~buffer()
        {
            cout << m_id << " , " << m_name << " + " << endl;
            delete m_buffer;
        }
        friend std::ostream & operator << (std::ostream&, const buffer& );
        
    private:
        int32_t     m_id;
        string    m_name;
        uint8_t*     m_buffer;
    };

    
    size_t size() const { return _available.size(); }
    
    shared_ptr<buffer> get(int32_t const& name)
    {
        // Look for available buffers
        cid2weak_iter it = _available.find(name);

        // If we have a refurbished available:
        // Find one by name in all and return a new shared_ptr
        // Erase it from available
        if (it != _available.end())
        {
            cout << "Found " << name << endl;
            cid2weak_iter cit = _all.find(name);
            shared_ptr<buffer> refurbished = cit->second.lock();
            _available.erase(it);
            return refurbished;
        }

        cout << "Not Found " << name << endl;
        
        // Get a shared_ptr to the new one and register it with both all map and viaptr
        shared_ptr<Cached> this_Cached =shared_from_this();
        //add to size -> shared_ptr map
        // insert returns iterator for multimap ( allows duplicates )
        shared_ptr<buffer> ptr(new buffer(name),Cached::deleter(this_Cached));
        
        id2weak_iter kv = _all.insert(std::make_pair(name, ptr));

        //add to pointer to all iterator map
        _viaptr.insert(std::make_pair(ptr.get(), kv));
        
        return ptr;
    }
    
private:
    //Cached map holding name to object's weak_ptr
    typedef std::multimap<int32_t, weak_ptr<buffer> >    id2weak;
    typedef  id2weak::iterator        id2weak_iter;
    typedef  id2weak::const_iterator        cid2weak_iter;
    
    
    typedef std::map<buffer*, id2weak_iter>     ptr2map;
    typedef ptr2map::iterator        ptr2map_iter;
    typedef ptr2map::const_iterator  cptr2map_iter;
    
    //custom deleter for buffer's shared_ptr
    struct deleter
    {
        deleter(weak_ptr<Cached> c): _Cached(c) {}
        
        void operator()(buffer* v)
        {
            shared_ptr<Cached> Cached = _Cached.lock();
            if (Cached.get() == 0) { delete v; return; }
            
            //find iterator in all via ptr
            ptr2map_iter it = Cached->_viaptr.find(v);
            
            // move it over to available.
            if (it != Cached->_viaptr.end())
                Cached->_available.insert(make_pair(it->second->first, it->second->second));
        }
        
        weak_ptr<Cached> _Cached;
    };
    
private:
    mutable id2weak _available;
    mutable id2weak _all;
    mutable ptr2map _viaptr;
};



std::ostream & operator << (std::ostream& o, const Cached::buffer& ib)
{
    o << ib.m_id << " , " << ib.m_name << endl;
    return o;
}



#endif
