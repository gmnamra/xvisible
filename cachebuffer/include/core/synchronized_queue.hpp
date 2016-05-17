#ifndef __SYNC_QUEUE__
#define __SYNC_QUEUE__

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <queue>

// Queue class that has thread synchronization
template <typename T>
class synchronized_queue
{
private:
	std::queue<T> m_queue;				// Use std::queue to store data
	mutable boost::mutex m_mutex;				// The mutex to synchronize on
	boost::condition_variable m_cond;	// The condition to wait for

public:

    bool empty () const
    {
       boost::lock_guard <boost::mutex> lock(m_mutex);
       return m_queue.empty();
    }
	// Add data to the queue and notify others
	void enqueue(const T& data)
	{
		// Acquire lock on the queue
		boost::lock_guard <boost::mutex> lock(m_mutex);

		// Add the data to the queue
		m_queue.push(data);

		// Notify others that data is ready
		m_cond.notify_one();

	} // Lock is automatically released here

	// Get data from the queue. Wait for data if not available
	T dequeue()
	{
		// Acquire lock on the queue
		boost::unique_lock<boost::mutex> lock(m_mutex);

		// When there is no data, wait till someone fills it.
		// Lock is automatically released in the wait and obtained again after the wait
		while (m_queue.size()==0) m_cond.wait(lock);

		// Retrieve the data from the queue
		T result=m_queue.front(); m_queue.pop();
		return result;

	} // Lock is automatically released here


    bool try_dequeue (T& result)
    {
         // Acquire lock on the queue
         boost::lock_guard<boost::mutex> lock(m_mutex);
         if (m_queue.empty())
            return false;
      
         // Retrieve the data from the queue
         result=m_queue.front(); m_queue.pop();
         return true;
   }

   unsigned size ()
   {
         // Acquire lock on the queue
         boost::lock_guard <boost::mutex> lock(m_mutex);
         return m_queue.size();
   }

};

#endif
