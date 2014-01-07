void  utWriteValues();

#define UT_HELP_INSERT_RACE utWriteValues()

#define UT_HELP_INSERT_RACE_N_CODE { \
                                      UT_INSERT_WRITE; *vDest++ = *vSrc++; \
                                      UT_INSERT_WRITE; *vDest++ = *vSrc++; \
                                      return; \
                                    }

unsigned char* internalData = 0;
#define UT_HELP_GET_DATA_PTR  (internalData = (unsigned char*)v0)

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <rc_atomic.h>
#include <ut_atomic.h>

struct writePattern
{
  const bool operator==(const writePattern& w)
  {
    const int *t0 = (int*)&(this->slot[0]);
    const int *t1 = (int*)&(this->slot[4]);
    const int *w0 = (int*)&(w.slot[0]);
    const int *w1 = (int*)&(w.slot[4]);

    return (*t0 == *w0) && (*t1 == *w1);
  }

  unsigned char slot[8];
};

vector< vector<unsigned short> > interruptPatternBins(8);
vector< vector<writePattern> > writePatternBins(8);


typedef struct
{
  unsigned short intPattern;
  unsigned char  intPatternIndex, wrtPatternIndex;
  writePattern*  wrtPattern;
  unsigned char  highByte, lowByte;
  unsigned char  writtenCount;
} writeInfo;

writeInfo* writeInfoPtr = 0;

void utWriteValues()
{
  if (writeInfoPtr == 0)
    return;

  unsigned short iPattern = writeInfoPtr->intPattern;
  unsigned short iMask = (1 << writeInfoPtr->intPatternIndex);

  writeInfoPtr->intPatternIndex++;

  if ((iPattern & iMask) == 0)
    return;

  unsigned char wrtIndex = writeInfoPtr->wrtPatternIndex++;
  unsigned char writeCnt = writeInfoPtr->wrtPattern->slot[wrtIndex];

  for (unsigned char i = 0; i < writeCnt; i++)
  {
    unsigned char writtenCount = writeInfoPtr->writtenCount++;
    
    if ((writtenCount & 1) == 0)
	internalData[7 - writtenCount] = writeInfoPtr->highByte;
    else
	internalData[7 - writtenCount] = writeInfoPtr->lowByte;
  }
}

void genWritePatternBin(writePattern& scratch, unsigned char totSlots,
			unsigned char curSlot, unsigned char totValues)
{
  if (curSlot == totSlots)
    return;

  scratch.slot[curSlot] = totValues;

  if ((curSlot+1) == totSlots)
      writePatternBins[curSlot].push_back(scratch);
  
  for (unsigned char curValue = totValues - 1; curValue; curValue--)
  {
    scratch.slot[curSlot] = curValue;
    genWritePatternBin(scratch, totSlots, curSlot+1, totValues-curValue);
  }

  scratch.slot[curSlot] = 0;
}

/* Generate all the possible combinations of "writes per
 * interrupt" for between 1 and 8 writes. . Store these in
 * writePatternBins.
 */
void UT_AtomicValue::genTestCases()
{
  writePattern scratch = {{0, 0, 0, 0, 0, 0, 0, 0}};

  for (unsigned char totValues = 1; totValues <= 8; totValues++)
    for (unsigned char totSlots = 1; totSlots <= totValues; totSlots++)
      genWritePatternBin(scratch,  totSlots, 0, totValues);

#if 0
  for (int i = 0; i < 8; i++)
  {
    printf("Total patterns using %d slots: %d\n", i+1, 
	   writePatternBins[i].size());
    
    for (int j = 0; j < writePatternBins[i].size(); j++)
    {
      writePattern& p = writePatternBins[i][j];

      printf("%d %d %d %d %d %d %d %d\n", p.slot[0], p.slot[1], p.slot[2],
	     p.slot[3], p.slot[4], p.slot[5], p.slot[6], p.slot[7]);
    }
    printf("\n");
  }
#endif
}

/* ut_raceTestNoGuard 
 *
 * Do exhaustive test of all cases where writes occur during read, but
 * guard bit never gets cleared. This involves a simulation of between
 * 1 and 8 writes during each read. These writes must be interspersed
 * between every possible combination of "interruption points" (which
 * in the real world would be either when the read process gets
 * interrupted for processing by the writer, or in a multiprocessor
 * system, where concurrent processes are reading and writing
 * simultaneously.
 */
void UT_AtomicValue::raceTestNoGuard()
{
  /* Generate all the possible bit patterns for up to 10 bits and
   * store those with between 1 and 8 bits set in interruptPatternBins.
   */
  for (unsigned short i = 0; i < 1024; i++)
  {
    unsigned char cnt = 0;

    for (unsigned short j = (1 << 9); j; j >>= 1)
      if (i & j)
        cnt++;

    if ((cnt == 0) || (cnt > 8))
      continue;

    interruptPatternBins[cnt-1].push_back(i);
  }

#if 0
  for (int i = 0; i < 8; i++)
  {
    int count = interruptPatternBins[i].size();
    printf("%d interrupt patterns of %d interrupts\n", count, i+1);

    for (int j = 0; j < count; j++)
    {
      unsigned short value = interruptPatternBins[i][j];

      printf("%s%s%s%s%s%s%s%s%s%s\n",
             ((value & (1 << 9)) ? "1" : "0"),
             ((value & (1 << 8)) ? "1" : "0"),
             ((value & (1 << 7)) ? "1" : "0"),
             ((value & (1 << 6)) ? "1" : "0"),
             ((value & (1 << 5)) ? "1" : "0"),
             ((value & (1 << 4)) ? "1" : "0"),
             ((value & (1 << 3)) ? "1" : "0"),
             ((value & (1 << 2)) ? "1" : "0"),
             ((value & (1 << 1)) ? "1" : "0"),
             ((value & (1 << 0)) ? "1" : "0"));

    printf("\n");
  }
#endif   

  /* Generate all the possible combinations of "writes per
   * interrupt" for between 1 and 8 writes. . Store these in
   * writePatternBins.
   */
  genTestCases();

#if 0
  for (int i = 0; i < 8; i++)
  {
    printf("%d slots: Interrupt Patterns %03d Write Patterns %02d Total Cases %05d\n",
           i+1, interruptPatternBins[i].size(), writePatternBins[i].size(),
           interruptPatternBins[i].size()*writePatternBins[i].size());
  }
#endif

/* Test cases all calculated. Now run tests. Init our test variable,
 * a. Because this code is using a gimmicked up version of
 * rcAtomicValue, clear writeInfoPtr before the call to the ctor so that
 * the calls to utWriteValue() will all be nops.
 */

  short oldVal = rand();
  rcAtomicValue<short> a(oldVal);
  writeInfo wInfo;
  writeInfoPtr = &wInfo;
#if 0
  unsigned int iteration = 0;
#endif

  for (unsigned int slots = 1; slots <= 8; slots++)
  {
    unsigned int maxIntPat = interruptPatternBins[slots-1].size(),
	         maxWrtPat = writePatternBins[slots-1].size();

    for (unsigned int intPat = 0; intPat < maxIntPat; intPat++)
    {
      wInfo.intPattern = interruptPatternBins[slots-1][intPat];

      for (unsigned int wrtPat = 0; wrtPat < maxWrtPat; wrtPat++)
      {
	wInfo.wrtPattern = &writePatternBins[slots-1][wrtPat];

#if 0
	printf("iPat 0x%08X wPat %d %d %d %d %d %d %d %d\n",
	       wInfo.intPattern, wInfo.wrtPattern->slot[0],
	       wInfo.wrtPattern->slot[1], wInfo.wrtPattern->slot[2],
	       wInfo.wrtPattern->slot[3], wInfo.wrtPattern->slot[4],
	       wInfo.wrtPattern->slot[5], wInfo.wrtPattern->slot[6],
	       wInfo.wrtPattern->slot[7]);
#endif
	
	for (unsigned int memoryMatch = 0; memoryMatch < 3; memoryMatch++)
	{
	  short newVal;

	  if (memoryMatch == 0) // Both new bytes are different
	  {
	    do
	    {
	      newVal = rand();
	    } while (((newVal & 0xff) == (oldVal & 0xff)) ||
		     ((newVal & 0xff00) == (oldVal & 0xff00)));
	  }
	  else if (memoryMatch == 1) // Only left value is different
	  {
	    do
	    {
	      newVal = (rand() & 0xff00) | (oldVal & 0xff);
	    } while ((newVal & 0xff00) == (oldVal & 0xff00));
	  }
	  else // Only right value is different
	  {
	    do
	    {
	      newVal = (rand() & 0xff) | (oldVal & 0xff00);
	    } while ((newVal & 0xff) == (oldVal & 0xff));
	  }

	  wInfo.highByte = (newVal & 0xff00) >> 8;
	  wInfo.lowByte = newVal & 0xff;
	  wInfo.intPatternIndex = wInfo.wrtPatternIndex = 0;
	  wInfo.writtenCount = 0;

#if 0
	  printf("iteration = %d\n", iteration++);
#endif
	  
	  short actualResult;
	  a.getValue(actualResult);

	  if ((actualResult != oldVal) && (actualResult != newVal))
	  {
	    printf("Error! oldVal: 0x%04X newVal: 0x%04X result: 0x%04X\n",
		   oldVal, newVal, actualResult);
	    printf("Interrupt Pattern: %s%s%s%s%s%s%s%s%s%s\n",
		   ((wInfo.intPattern & (1 << 9)) ? "1" : "0"),
		   ((wInfo.intPattern & (1 << 8)) ? "1" : "0"),
		   ((wInfo.intPattern & (1 << 7)) ? "1" : "0"),
		   ((wInfo.intPattern & (1 << 6)) ? "1" : "0"),
		   ((wInfo.intPattern & (1 << 5)) ? "1" : "0"),
		   ((wInfo.intPattern & (1 << 4)) ? "1" : "0"),
		   ((wInfo.intPattern & (1 << 3)) ? "1" : "0"),
		   ((wInfo.intPattern & (1 << 2)) ? "1" : "0"),
		   ((wInfo.intPattern & (1 << 1)) ? "1" : "0"),
		   ((wInfo.intPattern & (1 << 0)) ? "1" : "0"));

	    writePattern& p = *wInfo.wrtPattern;

	    printf("Write Pattern %d %d %d %d %d %d %d %d\n",
		   p.slot[0], p.slot[1], p.slot[2], p.slot[3],
		   p.slot[4], p.slot[5], p.slot[6], p.slot[7]);
	    rcUNITTEST_ASSERT(0);
	  }

	  a.setValue(newVal);
	  oldVal = newVal;
	} // End of: for ( ... ; memoryMatch < 3; memoryMatch++)
      } // End of: for ( ... ; wrtPat < maxWrtPat; wrtPat++)
    } // End of: for ( ... ; intPat < maxIntPat; intPat++)
  } // End of: for ( ... ; slots <= 8; slots++)
}
