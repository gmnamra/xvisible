// Copyright 2002 Reify, Inc.

#include <rc_types.h>
#include "rc_resource_ctrl.h"
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <rc_assemblyfcts.h>

#include <signal.h>
#include <iostream>

char* rcExecWithShmem::_progPath = 0;

#define WD_MAX_REPEATS 150
#define WD_US_TO_SLEEP 100000

#ifdef DEBUGGING
int mpDebugging = 1;
#else
int mpDebugging = 0;
#endif

void* rcResCtrlWatchdog(void* watchInfoAddr)
{
  assert(watchInfoAddr);

  watchdogInfo& wdInfo = *(watchdogInfo*)watchInfoAddr;
  assert(wdInfo.shMemUser);
  assert(wdInfo.readP);
  assert(wdInfo.writeP);
  fprintf(stderr, "Watchdog starting up ra 0x%X wa 0x%X\n",
          (intptr_t) wdInfo.readP, (intptr_t)wdInfo.writeP);

  uint16 lastValueRead = 0;    // Last value read from peer
  uint16 checkCount = 0;       // # of times read has occurred without change
  uint16 lastValueWritten = 0; // Last value written to peer
  uint8  shutdown;

  for (; wdInfo.shutdown.getValue(shutdown) == 0;)
  {
    /* First, check to see that peer is alive.
     */
    uint16 curRead;

    if (wdInfo.readP->getValue(curRead) == lastValueRead)
    {
      if (!mpDebugging && (checkCount++ >= WD_MAX_REPEATS))
      {
	fprintf(stderr, "peer dead\n");
	/* Peer appears dead. Notify the rest of the process and quit thread.
	 */
	wdInfo.peerDead.setValue(1);
	/* Make sure processing thread awakens.
	 */
	rcSharedMemError err;
	if ((err = wdInfo.shMemUser->forceRelease()) != rcSharedMemNoError)
	  fprintf(stderr, "Error on call to forceRelease %d\n", err);
	
	fprintf(stderr, "returning from watchdog on death\n");
	return 0;
      }
    }
    else
    {
      lastValueRead = curRead;
      checkCount = 0;
    }
   
    /* Next, let peer know we are still alive.
     */
    wdInfo.writeP->setValue(++lastValueWritten);

    /* Finally, sleep for a while before repeating.
     */
    usleep(WD_US_TO_SLEEP);
  }

  fprintf(stderr, "returning from watchdog on shutdown\n");
  return 0;
}

rcSharedMemoryUser::rcSharedMemoryUser(const struct sembuf& bufAvailSem,
				       const struct sembuf& returnBufSem,
                                       int semSetId, void* shmemP, 
				       uint8 ownSharedMem,
				       const rcAtomicValue<uint8>& peerDead)
  : _bufAvailSem(bufAvailSem), _returnBufSem(returnBufSem), _semSetId(semSetId),
    _shmemP(shmemP), _ownSharedMem(ownSharedMem), _peerDead(peerDead)
{
  assert(_shmemP);
}

void* rcSharedMemoryUser::acquireSharedMemory(rcSharedMemError& err, bool wait)
{
  err = rcSharedMemNoError;

  //printf("acquireSharedMemory\n");
  
  uint8 pDead = 0;

  if (_peerDead.getValue(pDead)) // Check that peer is alive
  {
    err = rcSharedMemPeerDead;
    return NULL;
  }

  if (_ownSharedMem)
    return _shmemP;

  _bufAvailSem.sem_op = -1;
  _bufAvailSem.sem_flg = wait ? 0 : IPC_NOWAIT;

  /* Try to decrement the semaphore to zero.
   */
  if (semop(_semSetId, &_bufAvailSem, 1) == 0) // Command succeeded
  {
    /* Verify that peer didn't die while acquiring buffer
     */
    if (_peerDead.getValue(pDead))
    {
      err = rcSharedMemPeerDead;
      return NULL;
    }
    
    _ownSharedMem = true;
    return _shmemP;
  }

  /* If we're here the operation failed. If wait is true and errno is
   * EAGAIN, the current value of the semaphore is zero.
   */
  if (!wait && (errno == EAGAIN))
    err = rcSharedMemNotAvailable;
  else
    err = rcSharedMemSemopError;

  return NULL;
}

rcSharedMemError rcSharedMemoryUser::releaseSharedMemory()
{
    //printf("releaseSharedMemory\n");
  if (!_ownSharedMem)
    return rcSharedMemNotLocked;

  _returnBufSem.sem_op = 1;
  _returnBufSem.sem_flg = 0;

  /* Try to increment the semaphore to one.
   */
  if (semop(_semSetId, &_returnBufSem, 1) == 0) // Command succeeded
  {
    _ownSharedMem = false;
    return rcSharedMemNoError;
  }
    
  return rcSharedMemSemopError;
}

/* Only to be called when fatal error has occurred to allow sleeping
 * processing thread to wake up for cleanup.
 */
rcSharedMemError rcSharedMemoryUser::forceRelease()
{
  _bufAvailSem.sem_op = 1;
  _bufAvailSem.sem_flg = 0;

  /* Try to increment the semaphore to one.
   */
  if (semop(_semSetId, &_bufAvailSem, 1) == 0) // Command succeeded
    return rcSharedMemNoError;
    
  return rcSharedMemSemopError;
}

rcExecWithShmem::rcExecWithShmem(char* progName, int arg1, char* arg2,
				 uint32 sz, uint8 childControlsBufferFirst)
        : _shMemUserP(0), _semSetId(-1), _shmId(-1), _childPID( 0 ),
          _createError(eNoError)
{
  assert(sz);
  assert(_progPath);
  assert(progName);
 
  /* Figure out how to round up the size of the shared memory control area
   * so that the user data starts on a 0 mod 16 boundary.
   */
  uint32 ctrlSz = sizeof(shMemCtrl);
  ctrlSz += ((ctrlSz & 0xF) == 0) ? 0 : (16 - (ctrlSz & 0xF));
  assert((ctrlSz & 0xF) == 0);
  
  /* Set up complete path to executable.
   */
  int progPathLen = strlen(_progPath);
  char* execPath = (char*)malloc(progPathLen + strlen(progName) + 1);
  assert(execPath);
  strcpy(execPath, _progPath);
  strcpy(execPath + progPathLen, progName);

  /* Allocate System V shared memory */

  _shmId = shmget(IPC_PRIVATE, ctrlSz + sz, (IPC_CREAT | 0600));

  if (_shmId == -1)
  {
    perror("shmem init step 1");
    free(execPath);
    _createError = eShmemInit;
    return;
  }

  _shMemCtrlP = (shMemCtrl *)shmat(_shmId, 0, 0);

  if ((intptr_t)((char*)_shMemCtrlP) == -1)
  {
    perror("shmem init step 2");
    freeSystemResources();
    free(execPath);
    _createError = eShmemInit;
    return;
  }
  
  /* Allocate System V semaphores */
  
  _semSetId = semget(IPC_PRIVATE, 2, (IPC_CREAT | 0600));

  if (_semSetId == -1)
  {
    perror("sem init step 1");
    freeSystemResources();
    free(execPath);
    _createError = eSemaphoreInit;
    return;
  }
  
  struct sembuf semBufParent, semBufChild;
  
  semBufParent.sem_num = 0;
  semBufParent.sem_op = 0; // Do a read operation
  semBufParent.sem_flg = IPC_NOWAIT;

  /* Check to see that semaphores have initial value of 0 (locked).
   */
  if (semop(_semSetId, &semBufParent, 1) == -1)
  {
    perror("sem init step 2");
    freeSystemResources();
    free(execPath);
    assert(0);
  }
 
  semBufChild.sem_num = 1;
  semBufChild.sem_op = 0; // Do a read operation
  semBufChild.sem_flg = IPC_NOWAIT;

  if (semop(_semSetId, &semBufChild, 1) == -1)
  {
    perror("sem init step 3");
    freeSystemResources();
    free(execPath);
    assert(0);
  }

  /* As final setup before exec'ing the new process, initialize shared memory.
   */
  _shMemCtrlP->semSetId = _semSetId;
  _shMemCtrlP->shmId = _shmId;
  _shMemCtrlP->childDone = 0;
  _shMemCtrlP->parentDone = 0;
  _shMemCtrlP->childControlsBufferFirst = childControlsBufferFirst;

  new ((void*)&_shMemCtrlP->childTouch) rcAtomicValue<uint16> (0);
  new ((void*)&_shMemCtrlP->parentTouch) rcAtomicValue<uint16> (0);

  /* Do fork.
   */
  _childPID = fork();
  if (_childPID == -1)
  {
    perror("fork step");
    freeSystemResources();
    free(execPath);
    _createError = eProcessFork;
    return;
  }
  else if (_childPID) // Then this is parent
  {
    /* After the fork() the child cannot see writes by the parent.
     * This seems like an OS bug. To get around it, reattach the
     * current address and then detach.
     */
    shMemCtrl* tempShmemP = (shMemCtrl *)shmat(_shmId, 0, 0);
    if ((intptr_t)tempShmemP == -1)
    {
      perror("shmem init step 3");
      freeSystemResources();
      free(execPath);
      _createError = eShmemInit;
      return;
    }

    if (shmdt(_shMemCtrlP) == -1)
    {
      perror("shmem init step 4");
      _shMemCtrlP = (shMemCtrl*)-1;
      freeSystemResources();
      free(execPath);
      _createError = eShmemInit;
      return;
    }

    _shMemCtrlP = tempShmemP;
    
    /* Set up semaphore values the way the parent wants them and then
     * use them to construct a shared memory control object.
     */
    semBufParent.sem_num = 1;
    semBufParent.sem_op = -1; // Decrement semaphore (lock operation) 
    semBufParent.sem_flg = 0;
    semBufChild.sem_num = 0;
    semBufChild.sem_op = 1; // Increment semaphore (unlock operation)
    semBufChild.sem_flg = 0;

    _shMemUserP = 
      new rcSharedMemoryUser(semBufParent, semBufChild, _semSetId,
			     (void*)((uint8 *)_shMemCtrlP + ctrlSz),
			     !childControlsBufferFirst, _watchdogData.peerDead);

    if (!_shMemUserP)
    {
      perror("shmem init step 5");
      freeSystemResources();
      free(execPath);
      _createError = eShmemInit;
      return;
    }

    /* Set the watchdogData fields.
     */
    printf("Parent: Shared memory start at 0x%X, size %u + %u. sizeof atomic 0x%lX Creating "
	   "atomics at locations 0x%X and 0x%X\n",
	   (intptr_t) _shMemCtrlP,
       ctrlSz, sz,
       sizeof(rcAtomicValue<uint16>),
	   (intptr_t)(&_shMemCtrlP->childTouch),
	   (intptr_t)(&_shMemCtrlP->parentTouch));

    _watchdogData.readP = (rcAtomicValue<uint16>*)(&_shMemCtrlP->childTouch);
    _watchdogData.writeP =(rcAtomicValue<uint16>*)(&_shMemCtrlP->parentTouch);
    _watchdogData.shMemUser = _shMemUserP;

    /* Start watchdog and then done.
     */
    pthread_create(&_watchdogThread, NULL, rcResCtrlWatchdog,
		   (void*)&_watchdogData);
    return;
  }

  /* Otherwise, this is the child. Set up and perform exec()
   */
  char shmIdString[50];
  char arg1String[50];
   
  snprintf(shmIdString, rmDim(shmIdString), "%d", _shmId);
  snprintf(arg1String, rmDim(arg1String), "%d", arg1);

  if (execl(execPath, execPath, shmIdString, arg1String, arg2, 0) == -1)
  {
    perror("error on exec");
    exit(0);
  }

  assert(0); // Should never get here!
}

rcExecWithShmem::~rcExecWithShmem()
{
  if (!isValid())
    return;

  if (_childPID == 0)
    return; // Only parent should be freeing stuff

  /* Tell watchdog handler to shutdown and wait for it to complete.
   */
  _watchdogData.shutdown.setValue(1);

  int err;

  fprintf(stderr, "~rcExecWithShmem: Waiting on watchdog thread\n");
  if ((err = pthread_join(_watchdogThread, 0)))
    fprintf(stderr, "Error on join with watchdog thread %d\n", err);

  fprintf(stderr, "~rcExecWithShmem: freeing system resources\n");
  freeSystemResources();

  if (_shMemUserP) {
    delete _shMemUserP;
    _shMemUserP = 0;
  }
}

uint8 rcExecWithShmem::isChildDone()
{
  if ( !isValid() )
      return 1;

  genSync();
  uint8 ret = _shMemCtrlP->childDone;
  genSync();
  return ret;
}

uint8 rcExecWithShmem::isParentDone()
{
  assert(isValid());

  return _shMemCtrlP->parentDone;
}

void rcExecWithShmem::setParentDone()
{
  assert(isValid());

  genSync();
  _shMemCtrlP->parentDone = true;
  genSync();
}

void rcExecWithShmem::setPathName(char* pathName)
{
  assert(pathName);
  assert(!_progPath);

  int i = strlen(pathName) - 1;

  while ((i >= 0) && (pathName[i] != '/'))
    i--;

  _progPath = (char*)malloc(i+2);
  assert(_progPath);

  if (i >= 0)
    strncpy(_progPath, pathName, i+1);

  _progPath[i+1] = 0;
}

void rcExecWithShmem::freeSystemResources()
{
  semun dummy;

  if (_semSetId != -1)
  {
    if (semctl(_semSetId, 0, IPC_RMID, dummy) == -1)
      perror("freeSystemResources(): Free of semaphores failed");
    else
      fprintf(stderr, "freeSystemResources: Semaphores freed\n");
    _semSetId = -1;
  }

  if ((intptr_t)_shMemCtrlP != -1)
  {
    if (shmdt(_shMemCtrlP) == -1)
      perror("freeSystemResources(): Detach of shared memory failed");
    else
      fprintf(stderr, "freeSystemResources: Shared memory detached\n");
    _shMemCtrlP = (shMemCtrl*)-1;
  }

  if (_shmId != -1) 
  {
    if (shmctl(_shmId, IPC_RMID, 0) == -1)
      perror("freeSystemResources(): Free of shared memory failed");
    else
      fprintf(stderr, "freeSystemResources: Shared memory freed\n");
    _shmId = -1;
  }
}

rcCreateChildShmem::rcCreateChildShmem(int argc, char **argv)
  : _shMemUserP(0), _semSetId (-1), _shmId(-1), _arg2(0)
{
  assert((argc == 3) || (argc == 4));
  assert(argv);

  /* Set up debugging support
   */
  if (mpDebugging)
  {
    struct sigaction sig;

    sig.sa_handler = SIG_IGN;
    sig.sa_mask = 0;
    sig.sa_flags = 0;

    if (sigaction(SIGINT, &sig, 0) == -1)
      perror("Disabling SIGINT in rcCreateChildShmem");
  }
  _shmId = atoi(argv[1]);
  _arg1 = atoi(argv[2]);

  if (argc == 4)
    {
      _arg2 = (char*)malloc(strlen(argv[3]) + 1);
      assert(_arg2);
      strcpy(_arg2, argv[3]);
    }

  /* Attach to System V shared memory */

  _shMemCtrlP = (shMemCtrl *)shmat(_shmId, 0, 0);

  if ((intptr_t) _shMemCtrlP != -1)
  {
    perror("shmem init step 1");
    if (_arg2)
    {
      free(_arg2);
      _arg2 = 0;
    }
    assert(0);
  }
  
  if (_shMemCtrlP->shmId != _shmId)
  {
    if (_arg2)
    {
      free(_arg2);
      _arg2 = 0;
    }
    assert(0);
  }

  /* Test that System V semaphores are accessible */
  
  _semSetId = _shMemCtrlP->semSetId;
  struct sembuf semBufParent, semBufChild;
  
  semBufParent.sem_num = 0;
  semBufParent.sem_op = 0; // Do a read operation
  semBufParent.sem_flg = IPC_NOWAIT;

  /* Check to see that parent semaphore has an initial value of 0 (locked).
   */
  if (semop(_semSetId, &semBufParent, 1) == -1)
    {
      perror("sem check step 1");
      if (_arg2)
      {
	free(_arg2);
	_arg2 = 0;
      }
      assert(0);
    }
 
  semBufChild.sem_num = 1;
  semBufChild.sem_op = 0; // Do a read operation
  semBufChild.sem_flg = IPC_NOWAIT;

  /* Check to see that child semaphore is accesible (parent may have
   * already unlocked it.
   */
  if ((semop(_semSetId, &semBufChild, 1) == -1) && (errno != EAGAIN))
  {
    perror("sem check step 2");
    if (_arg2)
    {
      free(_arg2);
      _arg2 = 0;
    }
    assert(0);
  }

  /* Set up semaphore values the way the parent wants them and then
   * use them to construct a shared memory control object.
   */
  semBufParent.sem_num = 1;
  semBufParent.sem_op = 1; // Increment semaphore (unlock operation)
  semBufParent.sem_flg = 0;
  semBufChild.sem_num = 0;
  semBufChild.sem_op = -1; // Decrement semaphore (lock operation) 
  semBufChild.sem_flg = 0;

  /* Figure out how much space is needed for both the shared memory
   * control area and to get the following user data on a 0 mod 16
   * boundary.
   */
  uint32 ctrlSz = sizeof(shMemCtrl);
  ctrlSz += ((ctrlSz & 0xF) == 0) ? 0 : (16 - (ctrlSz & 0xF));
  assert((ctrlSz & 0xF) == 0);

  try
  {
    _shMemUserP = 
      new rcSharedMemoryUser(semBufChild, semBufParent, _semSetId,
			     (void*)((unsigned char *)_shMemCtrlP + ctrlSz),
			     _shMemCtrlP->childControlsBufferFirst,
			     _watchdogData.peerDead);
  }
  catch (...)
  {
    free(_arg2);
    _arg2 = 0;

    throw; // Propogate exception
  }

  assert(_shMemUserP);

  /* Set up pointers to touch areas the way the child kies them and
   * start up watchdog thread.
   */
  printf("Child: Shared memory start at 0x%X. sizeof atomic 0x%lX Using atomics at "
         "locations 0x%X and 0x%X\n",
	 (intptr_t) _shMemCtrlP, sizeof(rcAtomicValue<uint16>), 
	 (intptr_t)(&_shMemCtrlP->childTouch),
	 (intptr_t)(&_shMemCtrlP->parentTouch));

  _watchdogData.readP = (rcAtomicValue<uint16>*)(&_shMemCtrlP->parentTouch);
  _watchdogData.writeP = (rcAtomicValue<uint16>*)(&_shMemCtrlP->childTouch);
  _watchdogData.shMemUser = _shMemUserP;

  pthread_create(&_watchdogThread, NULL, rcResCtrlWatchdog, (void*)&_watchdogData);
}

rcCreateChildShmem::~rcCreateChildShmem()
{
  printf("~rcCreateChildShmem called\n");

  /* Tell watchdog handler to shutdown and wait for it to complete.
   */
  _watchdogData.shutdown.setValue(1);

  int err;

  if ((err = pthread_join(_watchdogThread, 0)))
    fprintf(stderr, "Error on join with watchdog thread %d\n", err);

  /* Note: Parent is normally responsible for freeing up shared
   * memory/semaphore resources. Only if parent is dead should we
   * try to free these resources.
   */
  uint8 peerDead;
  if (_watchdogData.peerDead.getValue(peerDead))
  {
    semun dummy;

    if (_semSetId != -1)
    {
      if (semctl(_semSetId, 0, IPC_RMID, dummy) == -1)
	perror("~rcCreateChildShmem(): Free of semaphores failed");
      else
	fprintf(stderr, "~rcCreateChildShmem(): Semaphores freed\n");
      _semSetId = -1;
    }

    if ((intptr_t) _shMemCtrlP != -1)
    {
      if (shmdt(_shMemCtrlP) == -1)
	perror("~rcCreateChildShmem(): Detach of shared memory failed");
      else
	fprintf(stderr, "~rcCreateChildShmem(): Shared memory detached\n");
      _shMemCtrlP = (shMemCtrl*)-1;
  }

    if (_shmId != -1)
    {
      if (shmctl(_shmId, IPC_RMID, 0) == -1)
	perror("~rcCreateChildShmem(): Free of shared memory failed");
      else
	fprintf(stderr, "~rcCreateChildShmem(): Shared memory freed\n");
      _shmId = -1;
    }
  }

  if (_arg2)
  {
    free(_arg2);
    _arg2 = 0;
  }

  if (_shMemUserP)
  {
    delete _shMemUserP;
    _shMemUserP = 0;
  }
}

uint8 rcCreateChildShmem::isParentDone()
{
  genSync();
  uint8 ret = _shMemCtrlP->parentDone;
  genSync();
  return ret;
}

uint8 rcCreateChildShmem::isChildDone()
{
  genSync();
  return _shMemCtrlP->childDone;
}

void rcCreateChildShmem::setChildDone()
{
  genSync();
  _shMemCtrlP->childDone = true;
  genSync();
}
