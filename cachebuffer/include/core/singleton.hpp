#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>

class singleton_module : 
   public boost::noncopyable
   {
   private:
      static bool & get_lock(){
         static bool lock = false;
         return lock;
         }
   public:
      //    static const void * get_module_handle(){
      //        return static_cast<const void *>(get_module_handle);
      //    }
      static void lock(){
         get_lock() = true;
         }
      static void unlock(){
         get_lock() = false;
         }
      static bool is_locked() {
         return get_lock();
         }
   };

namespace detail {

   template<class T>
   class singleton_wrapper : public T
      {
      public:
         static bool m_is_destroyed;
         ~singleton_wrapper(){
            m_is_destroyed = true;
            }
      };

   template<class T>
   bool detail::singleton_wrapper< T >::m_is_destroyed = false;

   } // detail

template <class T>
class singleton : public singleton_module
   {
   private:
      static T & m_instance;
      // include this to provoke instantiation at pre-execution time
      static void use(T const &) {}
      static T & get_instance() {
         static detail::singleton_wrapper< T > t;
         // refer to instance, causing it to be instantiated (and
         // initialized at startup on working compilers)
         BOOST_ASSERT(! detail::singleton_wrapper< T >::m_is_destroyed);
         use(m_instance);
         return static_cast<T &>(t);
         }
   public:
      static T & get_mutable_instance(){
         BOOST_ASSERT(! is_locked());
         return get_instance();
         }
      static const T & instance(){
         return get_instance();
         }
      static bool is_destroyed(){
         return detail::singleton_wrapper< T >::m_is_destroyed;
         }
   };

template<class T>
 T & singleton< T >::m_instance = singleton< T >::get_instance();




#endif
