#include <stdio.h>
#include <stdlib.h>
#include <rc_sparsehist.h>
#include <ut_sparsehist.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

UT_SparseHistogram::UT_SparseHistogram()
{
}

UT_SparseHistogram::~UT_SparseHistogram()
{
    printSuccessMessage( "rcSparseHistogram test", mErrors );
}

uint32 UT_SparseHistogram::run()
{
  srand(0);

  const int passCount = 0x20000;

  for (int pass = 0; (pass < passCount) && (mErrors == 0); pass++) {

    /* Calculate how many bins to allow.
     */
    uint32 maxBins = uint32((rand() >> 8) & TEST_BINS_MASK);
    rcUNITTEST_ASSERT(maxBins <= UNIQUE_SIZE);

    /* Create object and test its initial state.
     */
    rcSparseHistogram hist(maxBins);

    rcUNITTEST_ASSERT(hist.isValid());
    rcUNITTEST_ASSERT(hist.sum() == 0);
    rcUNITTEST_ASSERT(hist.binsUsed() == 0);
    
    /* Calculate how many bin values to use.
     */
    bool forceTestPass = bool((rand() >> 8) & 1);
    uint32 uniqueValues = 0;
    if (forceTestPass && maxBins) {
	uniqueValues = (uint32)((double)maxBins*rand()/(RAND_MAX + 1.0)) + 1;
	rcUNITTEST_ASSERT(uniqueValues <= maxBins);
      }
    else
      uniqueValues = uint32((rand() >> 8) & TEST_BINS_MASK) + 1;

    rcUNITTEST_ASSERT(uniqueValues);
    rcUNITTEST_ASSERT(uniqueValues <= UNIQUE_SIZE);

    /* Initialize these to values out of a 32 bit range so they will
     * never be confused for some randomly chosen bin (slot) number.
     */
    for (uint32 cnt = 0; cnt < uniqueValues; cnt++)
      uniqueBins[cnt] = 0xFFFFFFFF;

    /* Now fill the unique array with uniqueValues number of random bin
     * (slot) values to use.
     */
    for (uint32 cnt = 0; cnt < uniqueValues; cnt++) {
      uint16 slot;
      uint32 i;

      do {
	slot = uint32((rand() >> 8) & 0xFFFF);

	for (i = 0; i < cnt; i++)
	  if (slot == uniqueBins[i])
	    break;
      }	while (i != cnt);

      uniqueBins[cnt] = slot;
      uniqueAndValid[cnt] = UNKNOWN;
    }
      
    /* Store a random permutation of of indices into unique in
       testArrayValueIndices. Store the current number of unique
       values/slots in use in testArrayCurUniqueValues.
     */
    uint32 minValue = 0xFFFF;
    uint32 maxValue = 0;
    uint32 curUniqueValues = 0;
    for (uint32 cnt = 0; cnt < TEST_ARRAY_SIZE; cnt++) {
      uint32 nextValueIndex = (uint32)((double)uniqueValues*rand()/
					   (RAND_MAX + 1.0));
      rcUNITTEST_ASSERT(nextValueIndex < uniqueValues);
      if (uniqueAndValid[nextValueIndex] == UNKNOWN) {
	if (maxBins && (curUniqueValues >= maxBins))
	  uniqueAndValid[nextValueIndex] = UINVALID;
	else {
	  uniqueAndValid[nextValueIndex] = UVALID;
	  uint32 nextValue = uniqueBins[nextValueIndex];
	  if (nextValue > maxValue)
	    maxValue = nextValue;
	  if (nextValue < minValue)
	    minValue = nextValue;
	}
	curUniqueValues++;
	rcUNITTEST_ASSERT(curUniqueValues <= uniqueValues);
      }

      testArrayValueIndices[cnt] = nextValueIndex;
      testArrayCurUniqueValues[cnt] = curUniqueValues;
      testArrayMinValue[cnt] = minValue;
      testArrayMaxValue[cnt] = maxValue;
    } // End of: for (int cnt = 0; cnt < TEST_ARRAY_SIZE; cnt++)

    /* Now run through stored permutation and calidate results.
     */
    uint32 curWeightedSum = 0;
    uint32 curCount = 0;
    bool isValid = true;

    for (uint32 cnt = 0; cnt < TEST_ARRAY_SIZE; cnt++) {
      uint32 index = testArrayValueIndices[cnt];
      rcUNITTEST_ASSERT(index < uniqueValues);
      uint32 slot = uniqueBins[index];
      const bool valid = hist.add(slot);

      rcUNITTEST_ASSERT(uniqueAndValid[index] != UNKNOWN);
      const bool shouldBeValid = (uniqueAndValid[index] == UVALID);
      rcUNITTEST_ASSERT(shouldBeValid == valid);

      if (valid) {
	curCount++;
	curWeightedSum += slot;
      }
      else
	isValid = false;

      rcUNITTEST_ASSERT(hist.isValid()    == isValid);
      rcUNITTEST_ASSERT(hist.sum()        == curCount);
      if (maxBins)
	rcUNITTEST_ASSERT(hist.binsUsed() == MIN(maxBins,
						  testArrayCurUniqueValues[cnt]));
      else
	rcUNITTEST_ASSERT(hist.binsUsed() == testArrayCurUniqueValues[cnt]);
      rcUNITTEST_ASSERT(hist.average()    == (double)curWeightedSum/curCount);
      
      uint32 curMin, curMax;
      hist.range(curMin, curMax);
      rcUNITTEST_ASSERT(curMin == testArrayMinValue[cnt]);
      rcUNITTEST_ASSERT(curMax == testArrayMaxValue[cnt]);

    } // End of: for (int cnt = 0; cnt < TEST_ARRAY_SIZE; cnt++)

    /* Don't really need to test STL. Mostly just to show that this is
     * useable.
     */
    const rcSparseHistogram::sparseArray& mapRef = hist.getArray();

    rcSparseHistogram::sparseArray::const_iterator start = mapRef.begin();
    rcSparseHistogram::sparseArray::const_iterator end = mapRef.end();
    
    curCount = 0;
    for ( ; start != end; start++)
      curCount += (*start).second;
    rcUNITTEST_ASSERT(curCount == hist.sum());
    
  } // End of: for (int pass = 0; (pass < passCount) && (mErrors == 0); pass++)

  return mErrors;  
}
