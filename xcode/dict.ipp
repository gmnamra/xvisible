
#ifndef qInv_TYPES_DICT_IPP
#define qInv_TYPES_DICT_IPP

#include <util\exception.hpp>
#include <boost\foreach.hpp>
#include <boost\format.hpp>
#include <boost\lexical_cast.hpp>
#include <typeinfo>

namespace qInv{

    namespace /*anon*/{
        template<typename Key, typename Val>
        struct key_not_found: qInv::key_error{
            key_not_found(const Key &key): qInv::key_error(
                str(boost::format(
                    "key \"%s\" not found in dict(%s, %s)"
                    ) % boost::lexical_cast<std::string>(key)
                    % typeid(Key).name() % typeid(Val).name()
                )
            ){
                /* NOP */
            }
        };
    } // namespace /*anon*/

    template <typename Key, typename Val>
    dict<Key, Val>::dict(void){
        /* NOP */
    }

    template <typename Key, typename Val> template <typename InputIterator>
    dict<Key, Val>::dict(InputIterator first, InputIterator last):
        _map(first, last)
    {
        /* NOP */
    }

    template <typename Key, typename Val>
    std::size_t dict<Key, Val>::size(void) const{
        return _map.size();
    }

    template <typename Key, typename Val>
    std::vector<Key> dict<Key, Val>::keys(void) const{
        std::vector<Key> keys;
        BOOST_FOREACH(const pair_t &p, _map){
            keys.push_back(p.first);
        }
        return keys;
    }

    template <typename Key, typename Val>
    std::vector<Val> dict<Key, Val>::vals(void) const{
        std::vector<Val> vals;
        BOOST_FOREACH(const pair_t &p, _map){
            vals.push_back(p.second);
        }
        return vals;
    }

    template <typename Key, typename Val>
    bool dict<Key, Val>::has_key(const Key &key) const{
        BOOST_FOREACH(const pair_t &p, _map){
            if (p.first == key) return true;
        }
        return false;
    }

    template <typename Key, typename Val>
    const Val &dict<Key, Val>::get(const Key &key, const Val &other) const{
        BOOST_FOREACH(const pair_t &p, _map){
            if (p.first == key) return p.second;
        }
        return other;
    }

    template <typename Key, typename Val>
    const Val &dict<Key, Val>::get(const Key &key) const{
        BOOST_FOREACH(const pair_t &p, _map){
            if (p.first == key) return p.second;
        }
        throw key_not_found<Key, Val>(key);
    }

    template <typename Key, typename Val>
    void dict<Key, Val>::set(const Key &key, const Val &val){
        (*this)[key] = val;
    }

    template <typename Key, typename Val>
    const Val &dict<Key, Val>::operator[](const Key &key) const{
        BOOST_FOREACH(const pair_t &p, _map){
            if (p.first == key) return p.second;
        }
        throw key_not_found<Key, Val>(key);
    }

    template <typename Key, typename Val>
    Val &dict<Key, Val>::operator[](const Key &key){
        BOOST_FOREACH(pair_t &p, _map){
            if (p.first == key) return p.second;
        }
        _map.push_back(std::make_pair(key, Val()));
        return _map.back().second;
    }

    template <typename Key, typename Val>
    Val dict<Key, Val>::pop(const Key &key){
        typename std::list<pair_t>::iterator it;
        for (it = _map.begin(); it != _map.end(); it++){
            if (it->first == key){
                Val val = it->second;
                _map.erase(it);
                return val;
            }
        }
        throw key_not_found<Key, Val>(key);
    }

} //namespace uhd

#endif /* qINV_TYPES_DICT_IPP */
