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
#include <map>

using namespace std;
#define MSG(text) std::cout<< text << std::endl;

class Book {
public:
    Book(long id, const string& name):m_id(id),m_name(name)
    {
        cout << m_name << " + " << endl;
    }
    ~Book()
    {
        cout << m_name << " - " << endl;
    }
private:
    long      m_id;
    string    m_name;
};

class Cache: public enable_shared_from_this<Cache>
{
public:
    size_t size() const { return _key2val.size(); }
    
    shared_ptr<Book> get(string const& name)
    {
        KVCIter it = _key2val.find(name);
        if (it != _key2val.end())
        {
            cout << "Found " << name << endl;
            return shared_ptr<Book>(it->second);
        }
        
        // need to create book ptr
        shared_ptr<Cache> myThis =shared_from_this();
        shared_ptr<Book> ptr(new Book(1,name),Deleter(myThis) );
     
        //add map all map
        KVIter kv = _all.insert(std::make_pair(name, ptr)).first;
        
        //add map2
        _delmap.insert(std::make_pair(ptr.get(), kv));
        return ptr;
    }
    
private:
    //cache map holding name to object's weak_ptr
    typedef std::map<string, weak_ptr<Book> >    KeyValueMap;
    typedef  KeyValueMap::iterator        KVIter;
    typedef  KeyValueMap::const_iterator  KVCIter;
    
    
    //helper map, the 2nd index helps Deleter works easily
    //when no need the object anymore(strong ref count ==0),
    //the object shared_ptr's customized Deleter will delete
    //the object from cache & 2nd index
    
    typedef std::map<Book*, KVIter>     DeleterMap;
    typedef DeleterMap::iterator        DMIter;
    typedef DeleterMap::const_iterator DMCIter;
    
    //custome deleter for book's shared_ptr
    struct Deleter {
        Deleter(weak_ptr<Cache> c): _cache(c) {}
        
        void operator()(Book* v) {
            shared_ptr<Cache> cache = _cache.lock();
            if (cache.get() == 0) { delete v; return; }
            
            //2
            //find object in all
            DMIter it = cache->_delmap.find(v);
            KVIter vit = it->second;
            // move it over to available.
            cache->_key2val.insert(make_pair(vit->first, vit->second));
        }
        
        weak_ptr<Cache> _cache;
    }; // Deleter
    
private:
    mutable KeyValueMap _key2val;
    mutable KeyValueMap _all;
    mutable DeleterMap _delmap;
};

#endif
