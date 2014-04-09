/******************************************************************************
*	rc_thread.h
*
*	This file contains platform independent declarations for supporting
*	threads.
*	
*	A thread can be set up in one of two ways.
*
*	An object can implement rcRunnable, instantiate an rcThread with
*	itself as the runnable object specified in the constructor, and
*	start the thread; the object's 'run()' method will be invoked from
*	the new thread.
*
*	Alternatively, the rcThread class can be extended, and the run()
*	method overridden.
******************************************************************************/

#ifndef _BASE_RCTHREAD_H_
#define _BASE_RCTHREAD_H_

#include <rc_types.h>
#include <boost/atomic.hpp>

using namespace boost;

// Thread priority values
enum rcThreadPriority
{
    eMinPriority = -2
  , eLowPriority = -1
  , eNormalPriority = 0
  , eHighPriority = 1
  , eMaxPriority = 2
};

// classes implement this interface
class RFY_API rcRunnable
{
public:
    rcRunnable() : _seppuku(false) { }

	virtual ~rcRunnable () {}
	
	// implement this to define the thread's execution.
	virtual void run( void ) = 0;

	// Ask child to kill itself.
	void requestSeppuku() { _seppuku = true; }

	// So rcThread start method can clear this before startup.
	void clearSeppuku() { _seppuku = false; }

protected:
	// Child checks this to see if it should kill itself.
	bool seppukuRequested() { return _seppuku; }

private:
    boost::atomic<bool> _seppuku;
};

// or extend this class and override the run() method.
class RFY_API rcThread : public rcRunnable
{
public:
	// construct a thread with the runnable object.
	rcThread( rcRunnable* runnable );

	// virtual dtor is required
	virtual ~rcThread( void );

	// set the thread priority (use prior to starting)
	void setPriority( rcThreadPriority pri );
	
	// get the thread priority
	rcThreadPriority getPriority( void );

	// start the thread (call the run() method in a new thread).
	void start( void );

	// is this thread still running?
	bool isRunning( void );

	// rethrows any exception that might have terminated the thread.
	void checkException( void );

	// subclasses override this to define the thread's execution.
	virtual void run( void );

	// the calling thread is suspended until this thread has
	// terminated.  the return value is zero on success, an error
	// number otherwise.
	virtual int join(bool seppuku = true);
    
	// static method causes current thread to sleep for specified
	// number of milliseconds.
	static void sleep( long msec );

	struct state_t;

protected:
	// protected default constructor: either construct with an
	//	rcRunnable or subclass.
	rcThread( void );

private:
	state_t* _state;
};

// Provides support for thread based mutexes. Typically, users should
// only create rcMutex objects and then use rcLock (see below to
// actually lock/unlock the mutex.
//
class RFY_API rcMutex
{
public:
       // Create a mutex, Setting invertProtect to true eliminates
       // priority inversion problems. Setting errorCheck to true
       // causes an error to occur whenever either a thread attempts
       // to unlock a mutex it has not already locked or a thread
       // attempts to lock a thread that it has already locked.  tp
       // determines what priority locking thread will run at. It is
       // only used when invertProtect is true.
       //
  rcMutex(bool errorCheck = true, bool invertProtect = true, 
	  rcThreadPriority tp = eMaxPriority);

      // Destroy the mutex.
      virtual ~rcMutex();

      // Lock the mutex. If the mutex is already in use, the thread
      // will block.
      void lock();

      // Attempt to lock the mutex, but do not block. If it is free
      // the mutex is locked and true is returned. Otherwise, the
      // state the mutex is left unchanged and FALSE is returned.
      bool tryLock();

      // Unlock the mutex. This will never block.
      void unlock();

private:

      friend class rcConditionVariable;
      friend class rcSignalPending;

      /* Disallow copy ctor and assignment operator
       */
      rcMutex(const rcMutex&);
      rcMutex& operator=(const rcMutex&);

      void* _mutex;
};

// Preferred way of using rcMutex objects. Locks passed in rcMutex
// when rcLock gets created and unlocks it when rcLock gets deleted.
// Only use rcMutex directly if you need to use tryLock() function.
//
class RFY_API rcLock
{
 public:

  // Creates rcLock object and locks the mutex passed in.
  //
  rcLock(rcMutex& mutex) : _mutex(mutex) { _mutex.lock(); }

  // Delete the rclock and unlock the mutex.
  virtual ~rcLock() { _mutex.unlock(); }

 private:

  rcMutex& _mutex;
};

// Class to support thread based condition variables. Allows threads
// to synchronize based on the value of a shared variable.
class RFY_API rcConditionVariable
{
 public:

  // Create a mutex and condition object to control access to private
  // shared variable. Variable is initialized to initValue.
  //
  rcConditionVariable(int32 initValue = 0);

  // Free resources
  //
  virtual ~rcConditionVariable();

  // Add value to variable. If result is greater than goal, send signal.
  // Returns current value.
  //
  int32 incrementVariable(int32 value, int32 goal);

  // Subtract value from variable. If result is less than goal, send signal.
  // Returns current value.
  //
  int32 decrementVariable(int32 value, int32 goal);

  // Sleep until variable is less than goal. Returns current value.
  //
  int32 waitUntilLessThan(int32 goal);

  // Sleep until variable is greater than goal. Returns current value.
  //
  int32 waitUntilGreaterThan(int32 goal);

 private:

  int32 mutateFct(const int32 value, const int32 goal,
		    const bool increment);
  int32 waitFct(const int32 goal, const bool lessThan);

  /* Disallow copy ctor and assignment operator
   */
  rcConditionVariable(const rcConditionVariable&);
  rcConditionVariable& operator=(const rcConditionVariable&);

  rcMutex _mutex;
  void*   _cond;
  int32 _var;
};

class rcFrame;

/* Special class intended to support rcVideoCache pending load
 * mechanism.
 */
class rcSignalPending
{
 public:

  // Create condition variable to be used
  //
  rcSignalPending(rcMutex& mutex);

  // Free resources
  //
  virtual ~rcSignalPending();

  // Wait for ptr to get set
  //
  void wait(rcFrame*& ptr);

  // Send signal to all waiting threads. Note that this fct assumes
  // the mutex is already locked.
  //
  void broadcast();

 private:

  /* Disallow copy ctor and assignment operator
   */
  rcSignalPending(const rcSignalPending&);
  rcSignalPending& operator=(const rcSignalPending&);

  rcMutex& _mutex;
  void*    _cond;
};

#endif // _BASE_RCTHREAD_H_
