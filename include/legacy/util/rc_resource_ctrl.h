// Copyright 2002 Reify, Inc.

#ifndef _rcRESOURCE_CTRL_H_
#define _rcRESOURCE_CTRL_H_

#include <sys/types.h>
#include <sys/sem.h>

#if WIN32
typedef int pid_t;
typedef void *pthread_t;
#endif

#include <rc_types.h>
#include <rc_atomic.h>

enum rcSharedMemError {
  rcSharedMemNoError = 0,
  rcSharedMemNotAvailable,
  rcSharedMemShmem0Error,
  rcSharedMemShmem1Error,
  rcSharedMemSemopError,
  rcSharedMemPeerDead,
  rcSharedMemNotLocked
};

class rcExecWithShmem;
class rcCreateChildShmem;

class rcSharedMemoryUser {
  friend class rcExecWithShmem;
  friend class rcCreateChildShmem;
  friend void* rcResCtrlWatchdog(void* shMemAddr);

 public:

  /* Checks to see if shared memory can be acquired.  If it cannot,
   * and wait is true, then sleep until shared memory is available.
   *
   * In either case, if shared memory is available, it is locked from
   * use by the corresponding rcSharedMemoryUser.
   *
   * Returns a pointer to shared memory if it is available. Otherwise,
   * NULL is returned. Check err to see reason for NULL return value.
   *
   * err is set to rcSharedMemNotAvailable if shared memory is not
   * available (this can only happen when wait was set to FALSE).
   *
   * If any other error occurs, rcSharedMemSemopError is
   * returned. Read errno and see description of sem_wait(2) in unix
   * system calls documentation.
   *
   * If no error has occurred, err is set to rcSharedMemNoError.
   */
  void* acquireSharedMemory(rcSharedMemError& err, bool wait = true);

  /* Releases shared resource back to corresponding
   * rcSharedMemoryUser. Returns rcSharedMemNoErr if this
   * succeeds. Returns rcSharedMemNotLocked if this object doesn't
   * currently have control over shared memory.
   *
   * If any other error occurs, rcSharedMemSemopError is
   * returned. Read errno and see description of sem_post(2) in unix
   * system calls documentation.
   *
   * NOTE: This call invalidates any existing pointers into shared memory.
   * Call acquireSharedMemory() again to get a valid pointer.
   */
  rcSharedMemError releaseSharedMemory();

 private:

  /* Disallow copy ctor and assignment operator
   */
  rcSharedMemoryUser(const rcSharedMemoryUser&);
  rcSharedMemoryUser& operator=(const rcSharedMemoryUser&);

  /* Only to be constructed by rcSharedMemory class objects.
   */
  rcSharedMemoryUser(const struct sembuf& bufAvailSem,
		     const struct sembuf& returnBufSem, 
		     int semSetId, void* shmemP, uint8 ownSharedMem,
		     const rcAtomicValue<uint8>& peerDead);

  /* Only to be called in case of fatal error by watchdog fct
   */
  rcSharedMemError forceRelease();

  struct sembuf                  _bufAvailSem;
  struct sembuf                  _returnBufSem;
  int                            _semSetId;
  void*                          _shmemP;
  uint8                        _ownSharedMem;
  const rcAtomicValue<uint8>&  _peerDead;
};

/* Defines a private area set up in shared memory that the
 * parent/child processes use to communicate with each other, above
 * and beyond what is requested by the clients of rcExecWithShmem and
 * rcCreateChildShmem.
 */

#define SZ_AV_UINT16 sizeof(rcAtomicValue<uint16>)
#define RND4_AV_UINT16 (((SZ_AV_UINT16&0x3) == 0) ? 0 : 4 - (SZ_AV_UINT16&0x3))
// The # of bytes required for a 16 bit rcAtomicValue with the size rounded
// up to the nearest 4 byte boundary
#define SZ_RND4_AV_UINT16 (SZ_AV_UINT16 + RND4_AV_UINT16)

typedef struct shMemCtrl {
  int      semSetId;
  int      shmId;
  uint8  childTouch[SZ_RND4_AV_UINT16];  // Used by watchdog fct
  uint8  parentTouch[SZ_RND4_AV_UINT16]; // Used by watchdog fct
  uint8  childDone;
  uint8  parentDone;
  uint8  childControlsBufferFirst;
} shMemCtrl;

typedef struct watchdogInfo {
  rcAtomicValue<uint16>* readP;  // Where in shared memory peer writes to
  rcAtomicValue<uint16>* writeP; // Where into shared memory peer reads from
  rcAtomicValue<uint8> peerDead; // Location in local memory used to inform
                                   // process its peer is not updating its
                                   // "touch" variable
  rcAtomicValue<uint8> shutdown; // Location in local memory used to inform
                                   // watchdog thread shutdown has started
  rcSharedMemoryUser*    shMemUser;// Allows watchdog to unlock semaphore
  watchdogInfo() : readP(0), writeP(0), peerDead(0),
                   shutdown(0), shMemUser(0) { }
} watchdogInfo;

enum rcEWSError {
  eNoError = 0,
  eShmemInit,
  eSemaphoreInit,
  eProcessFork
};

/* Initializes shared memory, semaphores to control its access, and exec's
 * a child process to share these resources.
 */
class rcExecWithShmem {
 public:

  /* Does all the work, including exec'ing new process. Only returns
   * to parent process -- pretty cool huh?
   *
   * It is assumed that the program to be called resides in the same
   * directory as the calling program. To facilitate finding it, the
   * static function rcExecWithShmem::setPathName() is provided.  Call
   * it once, AND ONLY ONCE, passing as its argument argv[0].  This
   * function will pick off the directory part of the calling program's
   * path. Do this call BEFORE calling this constructor.  The
   * constructor will append the progName passed to the end of the
   * stored path name to generate the complete file name/path.
   */
  rcExecWithShmem(char* progName, int arg1, char* arg2, uint32 sz,
		  uint8 childControlsBufferFirst = true);

  virtual ~rcExecWithShmem();

  rcSharedMemoryUser& getShmemUser() { return *_shMemUserP; }

  pid_t getChildPID() { return _childPID; }

  bool isValid() const { return _createError == eNoError; }

  rcEWSError getCreationError() const { return _createError; }

  /* The following fcts allow the parent and child to pass status
   * info to each other without resorting to signals.
   */
  uint8 isChildDone();
  uint8 isParentDone();
  void setParentDone();

  
  static void setPathName(char* nameArg);
	static std::string pathName ()  { return std::string (_progPath); }
	
 private:

  /* Disallow copy ctor and assignment operator
   */
  rcExecWithShmem(const rcExecWithShmem&);
  rcExecWithShmem& operator=(const rcExecWithShmem&);

  /* Helper fct that deletes shared memory and semaphores
   */
  void freeSystemResources();

  rcSharedMemoryUser* _shMemUserP;
  shMemCtrl*          _shMemCtrlP;
  int                 _semSetId;
  int                 _shmId;
  pid_t               _childPID;
  rcEWSError          _createError;
  pthread_t           _watchdogThread;
  watchdogInfo        _watchdogData;
  static char*        _progPath;
};

/* To be used by exec'd child of parent c reated by creation of
 * rcExecWithShmem object.
 */
class rcCreateChildShmem {
 public:

  rcCreateChildShmem(int argc, char **argv);

  virtual ~rcCreateChildShmem();

  rcSharedMemoryUser& getShmemUser() { return *_shMemUserP; }

  int getArg1() { return _arg1; }

  char* getArg2() { return _arg2; }

  /* The following fcts allow the parent and child to pass status
   * info to each other without resorting to signals.
   */
  uint8 isParentDone();
  uint8 isChildDone();
  void setChildDone();

 private:

  /* Disallow copy ctor and assignment operator
   */
  rcCreateChildShmem(const rcCreateChildShmem&);
  rcCreateChildShmem& operator=(const rcCreateChildShmem&);

  rcSharedMemoryUser* _shMemUserP;
  int                 _semSetId;
  int                 _shmId;
  shMemCtrl*          _shMemCtrlP;
  uint32            _arg1;
  char*               _arg2;
  pthread_t           _watchdogThread;
  watchdogInfo        _watchdogData;
};

#endif // _rcRESOURCE_CTRL_H_
