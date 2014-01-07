/*
 *  ut_motionanalyzer.h
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */

#ifndef _UT_MOTIONANALYZER_H_
#define _UT_MOTIONANALYZER_H_

#include <rc_unittest.h>
#include "ut_grabber.h"
#include <rc_motionanalyzer.h>
#include <vector>

class UT_MotionAnalyzer : public rcUnitTest {
public:

  UT_MotionAnalyzer();
  ~UT_MotionAnalyzer();

  virtual uint32 run();

  private:

  void genTestData();
  void calcExpectedValues(vector<uint32>& frameID,
			  vector<rcSharedFrameBufPtr>& sequence,
			  vector<rcMotionAnalyzerResult>& exp);
  void testSequence(vector<uint32>& frameID,
		    vector<rcSharedFrameBufPtr>& sequence);
  void genTestSequence(vector<uint32>& frameID,
		       vector<rcSharedFrameBufPtr>& sequence,
		       uint32 index);

  /* Test Data
   */
  vector<rcSharedFrameBufPtr> _frames;
  vector<vector<double> >      _corrScores;
  vector<vector<double> >      _energyScores;
};

#endif // _UT_MOTIONANALYZER_H_
