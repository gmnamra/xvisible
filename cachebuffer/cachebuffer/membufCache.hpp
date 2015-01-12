//
//  membufCache.hpp
//  cachebuffer
//
//  Created by Arman Garakani on 1/10/15.
//  Copyright (c) 2015 Arman Garakani. All rights reserved.
//

#ifndef cachebuffer_membufCache_hpp
#define cachebuffer_membufCache_hpp
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



class ImageBuffer
{
public:
    ImageBuffer(int64_t id):m_id(id)
    {
        m_name = std::to_string(id);
        cout << m_id << " , " << m_name << " + " << endl;
    }
    ~ImageBuffer()
    {
        cout << m_id << " , " << m_name << " + " << endl;
    }
    friend std::ostream & operator << (std::ostream&, const ImageBuffer& );
    
private:
    int64_t     m_id;
    string    m_name;
};

std::ostream & operator << (std::ostream& o, const ImageBuffer& ib)
{
    o << ib.m_id << " , " << ib.m_name << " + " << endl;
    return o;
}


class Cache: public enable_shared_from_this<Cache>
{
public:
    size_t size() const { return _available.size(); }
    
    shared_ptr<ImageBuffer> get(int64_t const& name)
    {
        // Look for available buffers
        cKVIter it = _available.find(name);

        // If we have a refurbished available:
        // Find one by name in all and return a new shared_ptr
        // Erase it from available
        if (it != _available.end())
        {
            cout << "Found " << name << endl;
            cKVIter cit = _all.find(name);
            shared_ptr<ImageBuffer> refurbished = cit->second.lock();
            _available.erase(it);
            return refurbished;
        }

        cout << "Not Found " << name << endl;
        
        // Get a shared_ptr to the new one and register it with both all map and viaptr
        shared_ptr<Cache> this_cache =shared_from_this();
        //add to size -> shared_ptr map
        // insert returns iterator for multimap ( allows duplicates )
        shared_ptr<ImageBuffer> ptr(new ImageBuffer(name),Cache::deleter(this_cache));
        
        KVIter kv = _all.insert(std::make_pair(name, ptr));

        //add to pointer to all iterator map
        _viaptr.insert(std::make_pair(ptr.get(), kv));
        
        return ptr;
    }
    
private:
    //cache map holding name to object's weak_ptr
    typedef std::multimap<int64_t, weak_ptr<ImageBuffer> >    KeyWeakValueMap;
    typedef  KeyWeakValueMap::iterator        KVIter;
    typedef  KeyWeakValueMap::const_iterator        cKVIter;
    
    
    typedef std::map<ImageBuffer*, KVIter>     BackViaPtr;
    typedef BackViaPtr::iterator        DMIter;
    typedef BackViaPtr::const_iterator DMCIter;
    
    //custome deleter for ImageBuffer's shared_ptr
    struct deleter
    {
        deleter(weak_ptr<Cache> c): _cache(c) {}
        
        void operator()(ImageBuffer* v)
        {
            shared_ptr<Cache> cache = _cache.lock();
            if (cache.get() == 0) { delete v; return; }
            
            std::cout << "available" << cache->_available << endl;
            std::cout << "all" << cache->_all << endl;
            std::cout << "viaptr" << cache->_viaptr << endl;

            //find iterator in all via ptr
            DMIter it = cache->_viaptr.find(v);
            if (it != cache->_viaptr.end())
            {
                // move it over to available.
                cache->_available.insert(make_pair(it->second->first, it->second->second));
            }

            std::cout << "available" << cache->_available << endl;
            std::cout << "all" << cache->_all << endl;
            std::cout << "viaptr" << cache->_viaptr << endl;
     
        }
        
        weak_ptr<Cache> _cache;
    }; // deleter
    
private:
    mutable KeyWeakValueMap _available;
    mutable KeyWeakValueMap _all;
    mutable BackViaPtr _viaptr;
};

#endif
