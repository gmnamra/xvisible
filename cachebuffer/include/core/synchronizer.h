#ifndef __SYNCHRONIZER__
#define __SYNCHRONIZER__

namespace qInv
{
  /** /brief This template class synchronizes two data streams of different types.
   *         The data can be added using add0 and add1 methods which expects also a timestamp of type unsigned long.
   *         If two matching data objects are found, registered callback functions are invoked with the objects and the time stamps.
   *         The only assumption of the timestamp is, that they are in the same unit, linear and strictly monotonic increasing.
   *         If filtering is desired, e.g. thresholding of time differences, the user can do that in the callback method.
   *         This class is thread safe.
   * /ingroup common
   */
  template <typename T1, typename T2>
  class Synchronizer
  {
    typedef std::pair<unsigned long, T1> T1Stamped;
    typedef std::pair<unsigned long, T2> T2Stamped;
    boost::mutex mutex1_;
    boost::mutex mutex2_;
    boost::mutex publish_mutex_;
    std::deque<T1Stamped> queueT1;
    std::deque<T2Stamped> queueT2;

    typedef boost::function<void(T1, T2, unsigned long, unsigned long) > CallbackFunction;

    std::map<int, CallbackFunction> cb_;
    int callback_counter;
  public:

    Synchronizer () : callback_counter (0) { };

    int
    addCallback (const CallbackFunction& callback)
    {
      boost::unique_lock<boost::mutex> publish_lock (publish_mutex_);
      cb_[callback_counter] = callback;
      return callback_counter++;
    }

    void
    removeCallback (int i)
    {
      boost::unique_lock<boost::mutex> publish_lock (publish_mutex_);
      cb_.erase (i);
    }

    void
    add0 (const T1& t, unsigned long time)
    {
      mutex1_.lock ();
      queueT1.push_back (T1Stamped (time, t));
      mutex1_.unlock ();
      publish ();
    }

    void
    add1 (const T2& t, unsigned long time)
    {
      mutex2_.lock ();
      queueT2.push_back (T2Stamped (time, t));
      mutex2_.unlock ();
      publish ();
    }

  private:

    void
    publishData ()
    {
      boost::unique_lock<boost::mutex> lock1 (mutex1_);
      boost::unique_lock<boost::mutex> lock2 (mutex2_);

      for (typename std::map<int, CallbackFunction>::iterator cb = cb_.begin (); cb != cb_.end (); ++cb)
      {
        if (!cb->second.empty ())
        {
          cb->second.operator()(queueT1.front ().second, queueT2.front ().second, queueT1.front ().first, queueT2.front ().first);
        }
      }

      queueT1.pop_front ();
      queueT2.pop_front ();
    }

    void
    publish ()
    {
      // only one publish call at once allowed
      boost::unique_lock<boost::mutex> publish_lock (publish_mutex_);

      boost::unique_lock<boost::mutex> lock1 (mutex1_);
      if (queueT1.empty ())
        return;
      T1Stamped t1 = queueT1.front ();
      lock1.unlock ();

      boost::unique_lock<boost::mutex> lock2 (mutex2_);
      if (queueT2.empty ())
        return;
      T2Stamped t2 = queueT2.front ();
      lock2.unlock ();

      bool do_publish = false;

      if (t1.first <= t2.first)
      { // iterate over queue1
        lock1.lock ();
        while (queueT1.size () > 1 && queueT1[1].first <= t2.first)
          queueT1.pop_front ();

        if (queueT1.size () > 1)
        { // we have at least 2 measurements; first in past and second in future -> find out closer one!
          if ( (t2.first << 1) > (queueT1[0].first + queueT1[1].first) )
            queueT1.pop_front ();

          do_publish = true;
        }
        lock1.unlock ();
      }
      else
      { // iterate over queue2
        lock2.lock ();
        while (queueT2.size () > 1 && (queueT2[1].first <= t1.first) )
          queueT2.pop_front ();

        if (queueT2.size () > 1)
        { // we have at least 2 measurements; first in past and second in future -> find out closer one!
          if ( (t1.first << 1) > queueT2[0].first + queueT2[1].first )
            queueT2.pop_front ();

          do_publish = true;
        }
        lock2.unlock ();
      }

      if (do_publish)
        publishData ();
    }
  } ;
} // namespace

#endif // __SYNCHRONIZER__

