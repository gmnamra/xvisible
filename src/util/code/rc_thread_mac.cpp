/******************************************************************************
*	rc_thread_mac.cpp
*
*	This file contains the Macintosh-specific support for threads.
******************************************************************************/

#include <stdio.h>
#include <string>
#include <exception>
#include <pthread.h>
#include <assert.h>
#include <sched.h>
#include <rc_exception.h>
#include <rc_thread.h>

#ifdef REAL_TIME_SUPPORT

class threadPriorityInfo
{
public:
  static int minPriority();
  static int maxPriority();
  static int defaultPriority();

private:
  static void init();

  static int _min;
  static int _max;
  static int _default;
  static int _inited;
};

int threadPriorityInfo::_inited = 0;
int threadPriorityInfo::_min;
int threadPriorityInfo::_max;
int threadPriorityInfo::_default;

int threadPriorityInfo::minPriority()
{
  if (!_inited)
    init();

  return _min;
}

int threadPriorityInfo::maxPriority()
{
  if (!_inited)
    init();

  return _max;
}

int threadPriorityInfo::defaultPriority()
{
  if (!_inited)
    init();

  return _default;
}

void threadPriorityInfo::init()
{
  char errMsg[100];
  pthread_attr_t attr;
  struct sched_param schedInfo;
  int error, policy;

#ifdef _POSIX_THREAD_PROCESS_SHARED
  printf("_POSIX_THREAD_PROCESS_SHARED defined\n");
#else
  printf("_POSIX_THREAD_PROCESS_SHARED undefined\n");
#endif

#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
  printf("_POSIX_THREAD_PRIORITY_SCHEDULING defined\n");
#else
  printf("_POSIX_THREAD_PRIORITY_SCHEDULING undefined\n");
#endif

#ifdef _POSIX_THREAD_PRIO_PROTECT
  printf("_POSIX_THREAD_PRIO_PROTECT defined\n");
#else
  printf("_POSIX_THREAD_PRIO_PROTECT undefined\n");
#endif

#ifdef _POSIX_THREAD_PRIO_INHERIT
  printf("_POSIX_THREAD_PRIO_INHERIT defined\n");
#else
  printf("_POSIX_THREAD_PRIO_INHERIT undefined\n");
#endif

  if ((error = pthread_attr_init(&attr)))
  {
    snprintf(errMsg, rmDim( errMsg ), "error initing pthread attr %d", error);
    throw general_exception( errMsg );
  }

  if ((error = pthread_attr_getschedpolicy(&attr, &policy)))
  {
    snprintf(errMsg, rmDim( errMsg ), "error getting sched policy %d\n", error);
    throw general_exception( errMsg );
  }

  if ((error = pthread_attr_getschedparam(&attr, &schedInfo)))
  {
    snprintf(errMsg, rmDim( errMsg ), "error getting sched params %d\n", error);
    throw general_exception( errMsg );
  }

  _default = schedInfo.sched_priority;

  _min = sched_get_priority_min(policy);
  if (_min == -1)
  {
    snprintf(errMsg, rmDim( errMsg ), "error getting sched min priority %d\n", errno);
    throw general_exception( errMsg );
  }

  _max = sched_get_priority_max(policy);
  if (_max == -1)
  {
    snprintf(errMsg, rmDim( errMsg ), "error getting sched max priority %d\n", errno);
    throw general_exception( errMsg );
  }

  if (((_default + eMinPriority) < _min) ||
      ((_default + eMaxPriority) > _max))
  {
    snprintf(errMsg, rmDim( errMsg ), "Insufficient priority range\n");
    throw general_exception( errMsg );
  }

  _inited = 1;

  fprintf(stderr, "pthreads: policy %d pri default %d pri min %d pri max %d\n",
	  policy, _default, _min, _max);
}
#endif

// platform-specific state info.
struct rcThread::state_t
{
    rcRunnable*		runnable;
    rcThreadPriority	priority;
    pthread_t 		threadId;
    bool		running;
    bool		died;
	std::string		error;

    state_t( void )
    {
        runnable = 0;
	priority = eNormalPriority;
	threadId = 0;
	running = false;
	died = false;
    }
};

#define _self		(_state->self)
#define _runnable	(_state->runnable)
#define _priority	(_state->priority)
#define _threadId	(_state->threadId)
#define _running	(_state->running)
#define _died		(_state->died)
#define _error		(_state->error)


// protected default constructor: either construct with an
//	rcRunnable or subclass.
rcThread::rcThread( void )
{
	_state = new state_t;
	_runnable = this;
}

// construct a thread with the runnable object.
rcThread::rcThread( rcRunnable* runnable )
{
	_state = new state_t;
	_runnable = runnable;
}

rcThread::~rcThread( void )
{
	if (_running)
		throw general_exception( "deleting running thread" );
	delete _state;
}

// set the thread priority (use prior to starting)
void rcThread::setPriority( rcThreadPriority pri )
{
  if ((pri < eMinPriority) || (pri > eMaxPriority))
    throw general_exception( "New priority invalid" );

  _priority = pri;
}

// get the thread priority
rcThreadPriority rcThread::getPriority( void )
{
	return _priority;
}


static void* threadEntry( void* parm );

// start the thread (call the run() method in a new thread).
void rcThread::start( void )
{
  if (_running)
    return;
  try
  {
    int pthread_err;
    char errMsg[100];

    pthread_attr_t attr;
    if ((pthread_err = pthread_attr_init(&attr)))
    {
      snprintf(errMsg, rmDim( errMsg ), "pthread_attr_init failed %d", pthread_err);
      throw general_exception( errMsg );
    }

#ifdef REAL_TIME_SUPPORT
    if ((pthread_err = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)))
    {
      snprintf(errMsg, rmDim( errMsg ), "error setting inherit sched %d\n", pthread_err);
      throw general_exception( errMsg );
    }

    if ((pthread_err = pthread_attr_setschedpolicy(&attr, SCHED_RR)))
    {
      snprintf(errMsg, rmDim( errMsg ), "error setting sched policy %d\n", pthread_err);
      throw general_exception( errMsg );
    }

    if (getPriority() != eNormalPriority)
    {
      struct sched_param sched;

      if ((pthread_err = pthread_attr_getschedparam(&attr, &sched)))
      {
	snprintf(errMsg, rmDim( errMsg ), "pthread_attr_getschedparam failed %d", pthread_err);
	throw general_exception( errMsg );
      }

      sched.sched_priority += getPriority();

      if ((pthread_err = pthread_attr_setschedparam(&attr, &sched)))
      {
	snprintf(errMsg, rmDim( errMsg ), "pthread_attr_setschedparam failed %d", pthread_err);
	throw general_exception( errMsg );
      }
    }
#endif

    if ((pthread_err = pthread_create( &_state->threadId, &attr, threadEntry,
				      _state)))
    {
      snprintf(errMsg, rmDim( errMsg ), "pthread_create failed %d", pthread_err);
      throw general_exception( errMsg );
    }

    _running = true;
    _died = false;

#ifdef rcTHREAD_TIMEOUT_CHECK
    // Note: Even if enabled this check won't work correctly without
    // using locks or rcAtomicValue. The logic has been changed so
    // that neither of these are required, but the new logic doesn't
    // have the called thread set _running at all, so a new variable
    // would have to be added to get the desired effect.

    // wait for thread to become active
    int timeout = 200;
    while (!_running)
    {
      usleep( 1000 );
      checkException();
      if (--timeout < 0)
	throw general_exception( "timeout creating thread" );
    }
#endif
  }
  catch (general_exception& x)
  {
    if ( _state->threadId )
      pthread_cancel( _state->threadId );
    throw x;
  } catch (exception& x)
  {
    throw x;
  }
}

// is this thread still running?
bool rcThread::isRunning( void )
{
	return _running;
}

// rethrows any general_exception that might have terminated the thread.
void rcThread::checkException( void )
{
	if (_died)
		throw general_exception( _error.c_str() );
}

// subclasses override this to define the thread's execution.
void rcThread::run( void )
{
	if (_runnable != 0)
		_runnable->run();
}

// The calling thread is suspended until this thread has terminated.
// The return value is zero on success, an error number otherwise.
//
// Note: Setting seppuku to true only informs child thread that it
// should terminate. It is up to the child thread to read this state
// (by calling seppukuRequested). Even in this case, the appropriate
// thing might be to run to normal completion (for example, when saving
// a file).
int rcThread::join(bool seppuku)
{
    int ret = 0;
    int retVal;
    if (_threadId) {
      if (seppuku)
	_runnable->requestSeppuku();
      ret = pthread_join(_threadId, (void**)&retVal);
      if (seppuku)
	_runnable->clearSeppuku();
      _running = false;
      _died = retVal ? true : false;
    }

    rmAssert(!_running);
    return ret;
}

// static method causes current thread to sleep for specified
//	number of milliseconds.
void rcThread::sleep( long msec )
{
	usleep( msec * 1000 );
}

// private thread entry function
static void* threadEntry( void* parm )
{
	int retVal = 0;
	rcThread::state_t* _state = (rcThread::state_t*) parm;

	try
	{
		_runnable->run();
	}
	catch (general_exception& x)
	{
                 _error = x.what(); // Not thread safe! Needs to use lock or
		                    // atomic value. Not being used currently...
		retVal = 1;
	}
	catch (...)
	{       // Not thread safe! Needs to use lock or
	        // atomic value. Not being used currently... xyzzy
		_error = "thread died with unknown exception";
		retVal = 1;
	}

	return (void*) retVal;
}

rcMutex::rcMutex(bool errorCheck, bool invertProtect, rcThreadPriority tp)
{
  pthread_mutexattr_t mutexAttr;
  int error;

  if ((error = pthread_mutexattr_init(&mutexAttr))) {
    char errMsg[100];
    snprintf(errMsg, rmDim( errMsg ), "error creating attr %d", error);
    throw general_exception( errMsg );
  }

  int type = errorCheck ? PTHREAD_MUTEX_ERRORCHECK : PTHREAD_MUTEX_NORMAL;

  if ((error = pthread_mutexattr_settype(&mutexAttr, type))) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "error setting attr type %d", error);
    throw general_exception(errMsg);
  }

#ifdef REAL_TIME_SUPPORT

  int protocol = PTHREAD_PRIO_NONE;

  if (invertProtect) {
    protocol = PTHREAD_PRIO_PROTECT;

    if ((error = pthread_mutexattr_setprotocol(&mutexAttr, protocol))) {
      char errMsg[100];
      snprintf(errMsg, rmDim(errMsg), "error setting protocol %d", error);
      throw general_exception(errMsg);
    }

    int priority = threadPriorityInfo::defaultPriority() + tp;

    if ((priority < threadPriorityInfo::minPriority()) ||
	(priority > threadPriorityInfo::maxPriority())) {
      char errMsg[100];
      snprintf(errMsg, rmDim(errMsg), "invalid new priority %d", priority);
      throw general_exception(errMsg);
    }

    if ((error =pthread_mutexattr_setprioceiling(&mutexAttr, priority))) {
      char errMsg[100];
      snprintf(errMsg, rmDim(errMsg), "error setting priority ceiling %d", error);
      throw general_exception(errMsg);
    }
  }
  else {
    if ((error = pthread_mutexattr_setprotocol(&mutexAttr, protocol))) {
      char errMsg[100];
      snprintf(errMsg, rmDim(errMsg), "error setting protocol %d", error);
      throw general_exception(errMsg);
    }
  }
#else
  rcUNUSED(invertProtect);
  rcUNUSED(tp);
#endif

  pthread_mutex_t* mutex = new pthread_mutex_t;

  if ((error = pthread_mutex_init(mutex, &mutexAttr))) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "error initializing mutex %d", error);
    delete mutex;
    throw general_exception(errMsg);
  }

  _mutex = (void *)mutex;
}

rcMutex::~rcMutex()
{
  delete (pthread_mutex_t*)_mutex;
  _mutex = 0;
}

void rcMutex::lock()
{
  if (!_mutex) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "Invalid mutex");
    throw general_exception(errMsg);
  }

  int error;

  if ((error = pthread_mutex_lock((pthread_mutex_t *)_mutex))) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "mutex lock failed %d", error);
    throw general_exception(errMsg);
  }
}

bool rcMutex::tryLock()
{
  if (!_mutex) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "Invalid mutex");
    throw general_exception(errMsg);
  }

  int error;

  if ((error = pthread_mutex_trylock((pthread_mutex_t *)_mutex))) {
    if (error != EBUSY) {
      char errMsg[100];
      snprintf(errMsg, rmDim(errMsg), "mutex trylock failed %d", error);
      throw general_exception(errMsg);
    }
    return FALSE;
  }

  return TRUE;
}

void rcMutex::unlock()
{
  if (!_mutex) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "Invalid mutex");
    throw general_exception(errMsg);
  }

  int error;

  if ((error = pthread_mutex_unlock((pthread_mutex_t *)_mutex))) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "mutex unlock failed %d", error);
    throw general_exception(errMsg);
  }
}

rcConditionVariable::rcConditionVariable(int32 initValue)
  : _mutex(), _cond((void*) new pthread_cond_t), _var(initValue)
{
  int error;

  if ((error = pthread_cond_init((pthread_cond_t*)_cond, NULL)))
  {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg),
	     "CV: condition variable init failed %d", error);
    throw general_exception(errMsg);
  }
}

rcConditionVariable::~rcConditionVariable()
{
  int error;

  if (!_cond) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "CV: Invalid condition variable");
    throw general_exception(errMsg);
  }

  if ((error = pthread_cond_destroy((pthread_cond_t*)_cond)))
  {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg),
	     "CV: condition variable destroy failed %d", error);
    throw general_exception(errMsg);
  }
}

int32 rcConditionVariable::incrementVariable(int32 value, int32 goal)
{
  return mutateFct(value, goal, true);
}

int32 rcConditionVariable::decrementVariable(int32 value, int32 goal)
{
  return mutateFct(value, goal, false);
}

int32 rcConditionVariable::waitUntilLessThan(int32 goal)
{
  return waitFct(goal, true);
}

int32 rcConditionVariable::waitUntilGreaterThan(int32 goal)
{
  return waitFct(goal, false);
}

int32 rcConditionVariable::mutateFct(const int32 value, const int32 goal,
				       const bool increment)
{
  int error;

  if (!_cond) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "Invalid condition variable");
    throw general_exception(errMsg);
  }

  rcLock lock(_mutex);

  if (increment)
    _var += value;
  else
    _var -= value;

  /* Why bother with retVal? Playing it safe. Not clear if unlock of
   * mutex occurs before or after return value is calculated.
   */
  int32 retVal = _var;

  if ((increment && (_var > goal)) || (!increment && (_var < goal))) {
    if ((error = pthread_cond_signal((pthread_cond_t *)_cond))) {
      char errMsg[100];
      snprintf(errMsg, rmDim(errMsg),
	       "CV: condition variable signal failed %d", error);
      throw general_exception(errMsg);
    }
  }

  return retVal;
}

int32 rcConditionVariable::waitFct(const int32 goal, const bool lessThan)
{
  int error;

  if (!_cond) {
    char errMsg[100];
    snprintf(errMsg, rmDim( errMsg ), "Invalid condition variable");
    throw general_exception( errMsg );
  }

  rcLock lock(_mutex);

  while ((lessThan && (_var >= goal)) || (!lessThan && (_var <= goal))) {
    if ((error = pthread_cond_wait((pthread_cond_t *)_cond,
				   (pthread_mutex_t *)(_mutex._mutex)))) {
      char errMsg[100];
      snprintf(errMsg, rmDim(errMsg), "condition variable signal failed %d", error);
      throw general_exception(errMsg);
    }
  }

  /* Why bother with retVal? Playing it safe. Not clear if unlock of
   * mutex occurs before or after return value is calculated.
   */
  int32 retVal = _var;

  return retVal;
}

rcSignalPending::rcSignalPending(rcMutex& mutex)
  : _mutex(mutex), _cond((void*) new pthread_cond_t)
{
  int error;

  if ((error = pthread_cond_init((pthread_cond_t*)_cond, NULL))) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg),
	     "SP: condition variable init failed %d", error);
    throw general_exception(errMsg);
  }
}

rcSignalPending::~rcSignalPending()
{
  int error;

  if (!_cond) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg), "SP: Invalid condition variable");
    throw general_exception(errMsg);
  }

  if ((error = pthread_cond_destroy((pthread_cond_t*)_cond))) {
    char errMsg[100];
    snprintf(errMsg, rmDim( errMsg ),
	     "SP: condition variable destroy failed %d", error);
    throw general_exception( errMsg );
  }
}

void rcSignalPending::wait(rcFrame*& ptr)
{
  rcLock lock(_mutex);
  int error;

  while (ptr == 0) {
    if ((error = pthread_cond_wait((pthread_cond_t *)_cond,
				   (pthread_mutex_t *)(_mutex._mutex)))) {
      char errMsg[100];
      snprintf(errMsg, rmDim(errMsg),
	       "SP: condition variable signal failed %d", error);
      throw general_exception(errMsg);
    }
  }
}

void rcSignalPending::broadcast()
{
  int error;
  if ((error = pthread_cond_broadcast((pthread_cond_t *)_cond))) {
    char errMsg[100];
    snprintf(errMsg, rmDim(errMsg),
	     "SP: condition variable signal failed %d", error);
    throw general_exception(errMsg);
  }
}
