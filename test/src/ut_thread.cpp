#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "rc_thread.h"
#include <ut_thread.h>
#include <pthread.h>
#include <rc_atomic.h>

// Static thread test data and defines

#define LOCKWORK_CNT 0x100000
#define TRYLOCK_ONLOCKED_FAILED 1
#define TRYLOCK_ONLOCKED_PASSED 2
#define TRYLOCK_ONUNLOCKED_FAILED 3
#define TRYLOCK_ONUNLOCKED_PASSED 4

static rcMutex staticTestMutex;
static rcAtomicValue<int> lockWorkA(0);
static rcAtomicValue<int> lockWorkB(0);
static rcAtomicValue<int> trylockFctWrite(0);
static rcAtomicValue<int> trylockFctRead(0);

// Live thread test data and defines

#define THREAD_COUNT 20
#define DATA_COUNT   16

static vector< int >sharedData(DATA_COUNT);
static vector< rcMutex* >sharedDataCtrl;

typedef struct {
    int myId;
    int myErrors;
} raceTestInfo;

// Priority test data and defines

#define PRIWORK_CNT 0x200000
#define UNUSED_SLOT 1000

static rcAtomicValue<int> priReady(0);
static rcMutex priMutex;
static int completionOrder[6];

// Priority inversion test data and defines

#define INVWORK_CNT 0x100000
#define HIGH_PRI_WORK_CNT 6
#define HIGH_PRI_STATE_INIT    0
#define HIGH_PRI_STATE_WORKING 1
#define HIGH_PRI_STATE_DONE    2
#define INV_DEBUG_PRT 0

static vector< rcAtomicValue<int> > highPriState;
static rcAtomicValue<int> lowPriUp(0);
static rcAtomicValue<int> highPriBlk(0);
static rcAtomicValue<int> doneCount(0);
static rcMutex* inversionTestMutex;

// Static thread test fcts

static void* lockTestAFct( void* )
{
  staticTestMutex.lock();

  for (int i = 0; i <= LOCKWORK_CNT; i++)
    lockWorkA.setValue(i);

  staticTestMutex.unlock();

  return 0;
}

static void* lockTestBFct( void* )
{
  for (int i = 0; i <= LOCKWORK_CNT; i++)
    lockWorkB.setValue(i);

  return 0;
}

static void* trylockTestFct( void* )
{
  if (staticTestMutex.tryLock())
    trylockFctWrite.setValue(TRYLOCK_ONLOCKED_FAILED);
  else
    trylockFctWrite.setValue(TRYLOCK_ONLOCKED_PASSED);

  int temp;
  while (trylockFctRead.getValue(temp) == 0)
    ;

  if (staticTestMutex.tryLock())
  {
    trylockFctWrite.setValue(TRYLOCK_ONUNLOCKED_PASSED);
    staticTestMutex.unlock();
  }
  else
    trylockFctWrite.setValue(TRYLOCK_ONUNLOCKED_FAILED);

  return 0;
}

void UT_Thread::mutexStaticTest()
{
  int temp;
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  /* Lock test - verify that locked out thread waits until mutex is
   * unlocked before it does anything else.
   */
  staticTestMutex.lock();
  lockWorkA.setValue(0);
  lockWorkB.setValue(0);

  pthread_t lockThreadA, lockThreadB;
  pthread_create(&lockThreadA, &attr, &lockTestAFct, 0);
  pthread_create(&lockThreadB, &attr, &lockTestBFct, 0);
  
  while (lockWorkB.getValue(temp) != LOCKWORK_CNT)
    usleep(10);

  rcUNITTEST_ASSERT(lockWorkA.getValue(temp) == 0);

  staticTestMutex.unlock();

  pthread_join(lockThreadB, 0);
  pthread_join(lockThreadA, 0);

  rcUNITTEST_ASSERT(lockWorkA.getValue(temp) == LOCKWORK_CNT);

  /* Trylock test - Verify that trylock: doesn't block, returns true
   * if the lock succeeded, returns false if it did not.
   */
  trylockFctWrite.setValue(0);
  trylockFctRead.setValue(0);
  
  staticTestMutex.lock();

  pthread_t trylockThread;
  pthread_create(&trylockThread, &attr, &trylockTestFct, 0);

  while (trylockFctWrite.getValue(temp) == 0)
    ;
  rcUNITTEST_ASSERT(temp == TRYLOCK_ONLOCKED_PASSED);

  staticTestMutex.unlock();

  trylockFctRead.setValue(1);

  pthread_join(trylockThread, 0);

  trylockFctWrite.getValue(temp);
  rcUNITTEST_ASSERT(temp == TRYLOCK_ONUNLOCKED_PASSED);
}

// Live thread test fcts

static void* raceTestFct( void *ptr )
{
  assert(ptr);
  raceTestInfo& tptr = *(raceTestInfo*)ptr;
  
  for (int counter = 0; counter < 0x200000; counter++)
  {
    int index = rand() & (DATA_COUNT - 1);

    rcLock lock(*(sharedDataCtrl[index]));

    int temp = sharedData[index];

    sharedData[index]++;
    sharedData[index]--;

    if (temp != sharedData[index])
      tptr.myErrors++;
  }
  
  return 0;
}

void UT_Thread::mutexLiveThreadsTest()
{
  vector<pthread_t> thread(THREAD_COUNT);
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  for (int i = 0; i < DATA_COUNT; i++)
  {
    sharedData[i] = 0;
      
    rcMutex* mPtr = new rcMutex();
    sharedDataCtrl.push_back(mPtr);
  }

  vector<raceTestInfo> raceInfo;

  for (int i = 0; i < THREAD_COUNT; i++)
  {
    raceTestInfo temp = { i, 0 };
    raceInfo.push_back(temp);
  }
  
  for (int i = 0; i < THREAD_COUNT; i++)
    pthread_create(&thread[i], &attr, &raceTestFct, (void*) &raceInfo[i]);

  for (int i = 0; i < THREAD_COUNT; i++)
  {
    pthread_join(thread[i], 0);
    rcUNITTEST_ASSERT(raceInfo[i].myErrors == 0);
  }

  for (int i = 0; i < DATA_COUNT; i++)
    delete sharedDataCtrl[i];

  return;
}

// Priority test fcts

class utPriorityWorking : public rcRunnable
{
public:

  utPriorityWorking(int index) : _index(index) { }
  
  void run( void )
  {
    int temp;

    while (priReady.getValue(temp) == 0)
      usleep(100);

    /* Do something completely nonsensical for a while. Only used
     * rcAtomicValue because it guarantees compiler won't try to
     * optimize stuff out.
     */
    rcAtomicValue<int> work(0);
    for (int i = 0; i < PRIWORK_CNT; i++)
      work.setValue(i);

    priMutex.lock();
    for (int i = 0; i < 6; i++)
      if (completionOrder[i] == UNUSED_SLOT)
      {
	completionOrder[i] = _index;
	break;
      }
    priMutex.unlock();
  }

  int _index;
};

void UT_Thread::priorityTest()
{
  utPriorityWorking pMin(eMinPriority);
  utPriorityWorking pLow(eLowPriority);
  utPriorityWorking pNormal(eNormalPriority);
  utPriorityWorking pHigh(eHighPriority);
  utPriorityWorking pMax(eMaxPriority);
  utPriorityWorking pMax2(eMaxPriority);

  rcThread tMin(&pMin);
  tMin.setPriority(eMinPriority);
  rcThread tLow(&pLow);
  tLow.setPriority(eLowPriority);
  rcThread tNormal(&pNormal);
  tNormal.setPriority(eNormalPriority);
  rcThread tHigh(&pHigh);
  tHigh.setPriority(eHighPriority);
  rcThread tMax(&pMax);
  tMax.setPriority(eMaxPriority);
  rcThread tMax2(&pMax2);
  tMax2.setPriority(eMaxPriority);

  priReady.setValue(0);
  for (int i = 0; i < 6; i++)
    completionOrder[i] = UNUSED_SLOT;

  /* Do test first time
   */
  tMin.start();
  tLow.start();
  tNormal.start();
  tHigh.start();
  tMax.start();
  tMax2.start();

  priReady.setValue(1);

  tMax2.join();
  tMax.join();
  tHigh.join();
  tNormal.join();
  tLow.join();
  tMin.join();

  /* Check that values decrease monotonically
   */
  int lastValue = eMaxPriority;
  for (int i = 0; i < 6; i++)
  {
    rcUNITTEST_ASSERT(lastValue >= completionOrder[i]);
    lastValue = completionOrder[i];
  }

  /* Repeat test but reverse order of starts to guarantee that
   * start order isn't coloring the result.
   */
  priReady.setValue(0);
  for (int i = 0; i < 6; i++)
    completionOrder[i] = UNUSED_SLOT;

  tMax2.start();
  tMax.start();
  tHigh.start();
  tNormal.start();
  tLow.start();
  tMin.start();

  priReady.setValue(1);

  tMax2.join();
  tMax.join();
  tHigh.join();
  tNormal.join();
  tLow.join();
  tMin.join();

  /* Check that values decrease monotonically
   */
  lastValue = eMaxPriority;
  for (int i = 0; i < 6; i++)
  {
    rcUNITTEST_ASSERT(lastValue >= completionOrder[i]);
    lastValue = completionOrder[i];
  }
}

// Priority inversion test fcts

class utHighPriorityWorking : public rcRunnable
{
public:

  utHighPriorityWorking(int index) : _index(index) { }
  
  void run( void )
  {
    int temp;

    highPriState[_index].setValue(HIGH_PRI_STATE_WORKING);

    while (lowPriUp.getValue(temp) == 0)
      usleep(100);

    if (INV_DEBUG_PRT) printf("WS\n");
    /* Do something completely nonsensical for a while. Only used
     * rcAtomicValue because it guarantees compiler won't try to
     * optimize stuff out.
     */
    rcAtomicValue<int> work(0);
    for (int i = 0; i < INVWORK_CNT; i++)
      work.setValue(i);

    if (INV_DEBUG_PRT) printf("WC\n");
    highPriState[_index].setValue(HIGH_PRI_STATE_DONE);
  }

  int _index;
};

class utHighPriorityBlocked : public rcRunnable
{
public:

  utHighPriorityBlocked() { }
  
  void run( void )
  {
    int temp;

    highPriBlk.setValue(HIGH_PRI_STATE_WORKING);

    while (lowPriUp.getValue(temp) == 0)
      usleep(100);

    if (INV_DEBUG_PRT) printf("BS\n");

    inversionTestMutex->lock();
    inversionTestMutex->unlock();

    if (INV_DEBUG_PRT) printf("BC\n");

    int doneCnt = 0;
    for (int i = 0; i < HIGH_PRI_WORK_CNT; i++)
      doneCnt += (highPriState[i].getValue(temp) == HIGH_PRI_STATE_DONE) ? 1 : 0;

    doneCount.setValue(doneCnt);
  }
};

class utLowPriorityBlocking : public rcRunnable
{
public:

  utLowPriorityBlocking() { }
  
  void run( void )
  {
    if (INV_DEBUG_PRT) printf("L1\n");

    inversionTestMutex->lock();

    if (INV_DEBUG_PRT) printf("L2\n");

    /* Let high priority tasks know they can get to work.
     */
    lowPriUp.setValue(1);

    if (INV_DEBUG_PRT) printf("L2.5\n");

    usleep(100);

    if (INV_DEBUG_PRT) printf("L3\n");

    inversionTestMutex->unlock();

    if (INV_DEBUG_PRT) printf("L4\n");
  }
};

void UT_Thread::priorityInversionTest()
{
  /* Create all the threads needed for this test.
   */
  int doneCnt;
  rcAtomicValue<int> temp(0);
  vector<utHighPriorityWorking*> worker;
  vector<rcThread*> workerThread;
  for (int i = 0; i < HIGH_PRI_WORK_CNT; i++)
  {
    worker.push_back(new utHighPriorityWorking(i));
    workerThread.push_back(new rcThread(worker[i]));
    workerThread[i]->setPriority(eHighPriority);
    highPriState.push_back(temp);
  }

  utHighPriorityBlocked blocked;
  rcThread blockedThread(&blocked);
  blockedThread.setPriority(eHighPriority);
  
  utLowPriorityBlocking blocking;
  rcThread blockingThread(&blocking);
  blockingThread.setPriority(eLowPriority);

  /* Now test that mutexes with priority inversion protection enabled
   * really do get their priority level bumped up
   */
  rcMutex invertProtect(true, true); // 2nd arg std::true enables inversion prot

  inversionTestMutex = &invertProtect;

  lowPriUp.setValue(0);
  doneCount.setValue(0);

  for (int i = 0; i < HIGH_PRI_WORK_CNT; i++)
  {
    int dummy;

    highPriState[i].setValue(HIGH_PRI_STATE_INIT);
    workerThread[i]->start();
    while (highPriState[i].getValue(dummy) != HIGH_PRI_STATE_WORKING)
      ;
  }
  
  {
    int dummy;

    highPriBlk.setValue(HIGH_PRI_STATE_INIT);
    blockedThread.start();
    while (highPriBlk.getValue(dummy) != HIGH_PRI_STATE_WORKING)
      ;
  }

  blockingThread.start();

  blockingThread.join();
  blockedThread.join();
  for (int i = 0; i < HIGH_PRI_WORK_CNT; i++)
    workerThread[i]->join();

  doneCount.getValue(doneCnt);
  rcUNITTEST_ASSERT(doneCnt == 0);

  for (int i = 0; i < HIGH_PRI_WORK_CNT; i++)
  {
    delete worker[i];
    delete workerThread[i];
  }

  /* Now test that mutexes with priority inversion protection disabled
   * get blocked by higher priority tasks.
   */
  for (int i = 0; i < HIGH_PRI_WORK_CNT; i++)
  {
    worker[i] = new utHighPriorityWorking(i);
    workerThread[i] = new rcThread(worker[i]);
    workerThread[i]->setPriority(eHighPriority);
   }

  rcMutex dontProtect(true, false); // 2nd arg std::false disables inversion prot

  inversionTestMutex = &dontProtect;

  lowPriUp.setValue(0);
  doneCount.setValue(0);

  for (int i = 0; i < HIGH_PRI_WORK_CNT; i++)
  {
    int dummy;

    highPriState[i].setValue(HIGH_PRI_STATE_INIT);
    workerThread[i]->start();
    while (highPriState[i].getValue(dummy) != HIGH_PRI_STATE_WORKING)
      ;
  }

  rcThread blockedThread2(&blocked);
  {
    int dummy;

    blockedThread2.setPriority(eHighPriority);
    highPriBlk.setValue(HIGH_PRI_STATE_INIT);
    blockedThread.start();
    while (highPriBlk.getValue(dummy) != HIGH_PRI_STATE_WORKING)
      ;
  }

  rcThread blockingThread2(&blocking);
  blockingThread2.setPriority(eLowPriority);
  blockingThread2.start();

  blockingThread2.join();
  blockedThread2.join();
  for (int i = 0; i < HIGH_PRI_WORK_CNT; i++)
    workerThread[i]->join();

  /* Note: This 2nd test is a little fragile -- as long as the amount
   * of work done by the working threads requires multiple time
   * slices, this should be doing a valid test.
   */
  doneCount.getValue(doneCnt);
  rcUNITTEST_ASSERT(doneCnt >= (HIGH_PRI_WORK_CNT-1));
  
  /* Clean up before returning.
   */
  for (int i = 0; i < HIGH_PRI_WORK_CNT; i++)
  {
    delete worker[i];
    delete workerThread[i];
  }

  return;
}

UT_Thread::UT_Thread()
{
}

UT_Thread::~UT_Thread()
{
  printSuccessMessage( "rcThread test", mErrors );
}

uint32 UT_Thread::run()
{
  srand(0);

  // priorityTest();
  // priorityInversionTest();
  mutexStaticTest();
  mutexLiveThreadsTest();

  return mErrors;  
}
