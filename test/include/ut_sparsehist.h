// Copyright 2002 Reify, Inc.

#ifndef _rcUT_SPARSEHIST_H_
#define _rcUT_SPARSEHIST_H_

#include <rc_unittest.h>

#define TEST_COUNT 10
#define TEST_BINS_MASK 0x1F
#define UNIQUE_SIZE (TEST_BINS_MASK+1)

#define UNKNOWN  0
#define UVALID   1
#define UINVALID 2

#define TEST_ARRAY_MASK 0xFF
#define TEST_ARRAY_SIZE (TEST_ARRAY_MASK+1)

class UT_SparseHistogram : public rcUnitTest {
 public:
  UT_SparseHistogram();

  ~UT_SparseHistogram();

  virtual uint32 run();
    
 private:
  uint32 uniqueBins[UNIQUE_SIZE];
  uint32 uniqueAndValid[UNIQUE_SIZE];
  uint16 testArrayValueIndices[TEST_ARRAY_SIZE];
  uint16 testArrayCurUniqueValues[TEST_ARRAY_SIZE];
  uint16 testArrayMinValue[TEST_ARRAY_SIZE];
  uint16 testArrayMaxValue[TEST_ARRAY_SIZE];
};

#endif // _rcUT_SPARSEHIST_H_
