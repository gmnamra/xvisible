#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <rc_atomic.h>
#include <ut_atomic.h>
#include <typeinfo>
#include <stdio.h>

#include <assert.h>

using namespace std;
#define THREAD_COUNT 20

typedef struct {
    int a;
    int b;
    int c;
    int d;
} complexData;

static vector< vector< rcAtomicValue<complexData> > >sharedData(THREAD_COUNT);

typedef struct {
    int myId;
    int myErrors;
} raceTestInfo;

static void* raceTestFct( void *ptr )
{
    assert(ptr);
    raceTestInfo& tptr = *(raceTestInfo*)ptr;
    int myId = tptr.myId;

    complexData localData;

    for (int counter = 0; counter < 0x200000; counter++)
    {
	int newValue = rand();

	localData.a = localData.b = localData.c = localData.d = newValue;
	
	int writeId;
	do
	{
	    writeId = ((unsigned int)rand()) % THREAD_COUNT;
	} while (writeId == myId);

	sharedData[myId][writeId].setValue(localData);

	localData.a = 0; localData.b = 1; localData.c = 2; localData.d = 3;

	int readId;
	do
	{
	    readId = ((unsigned int)rand()) % THREAD_COUNT;
	} while (readId == myId);

	sharedData[readId][myId].getValue(localData);

	if ((localData.a != localData.b) ||
	    (localData.a != localData.c) ||
	    (localData.a != localData.d))
	  tptr.myErrors++;
    }

    return 0;
}

void UT_AtomicValue::liveThreadsTest()
{
    srand(0);

    vector<pthread_t> thread(THREAD_COUNT);
    pthread_attr_t attr_default;
    pthread_attr_init(&attr_default);

    {
	complexData initData = { 0, 0, 0, 0 };
	rcAtomicValue<complexData> temp(initData);
			static string is_boost = temp.is_boost == 1 ? " Is Based On Boost::atomic " : " Is Based Native ";
			std::cout << is_boost << std::endl;

	for (int i = 0; i < THREAD_COUNT; i++)
	    for (int j = 0; j < THREAD_COUNT; j++)
		sharedData[i].push_back(temp);
    }

    vector<raceTestInfo> raceInfo;

    for (int i = 0; i < THREAD_COUNT; i++)
    {
	raceTestInfo temp = { i, 0 };
	raceInfo.push_back(temp);
    }

    for (int i = 0; i < THREAD_COUNT; i++)
    {
	pthread_create(&thread[i], &attr_default,
		       &raceTestFct, (void*) &raceInfo[i]);
    }

    for (int i = 0; i < THREAD_COUNT; i++)
    {
	pthread_join(thread[i], 0);
	mErrors += raceInfo[i].myErrors;
    }

    return;
}

UT_AtomicValue::UT_AtomicValue()
{
}

UT_AtomicValue::~UT_AtomicValue()
{
    printSuccessMessage( "rcAtomicValue test", mErrors );
}

uint32 UT_AtomicValue::run()
{
  raceTestNoGuard();
  liveThreadsTest();


  return mErrors;  
}
