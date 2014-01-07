



/*
 *  ut_analyzer.h
 *
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _UT_ANALYZER_H_
#define _UT_ANALYZER_H_

#include <rc_unittest.h>
#include <rc_framegrabber.h>
#include <rc_analyzer.h>

class rcVectorGrabber;

class UT_Analyzer : public rcUnitTest {
public:

   UT_Analyzer();
   ~UT_Analyzer();

   virtual uint32 run();

  private:
   void testAnalyzer( rcAnalyzer& analyzer,
                                   const vector<rcWindow>& images,
                                   rcFrameGrabberStatus expectedStatus,
                                   rcFrameGrabberError expectedError,
                                   const vector<double>& expectedEntropies,
                                   bool isBlocking,
                                   bool testEOF );
   // Test one window
   void testWindow( const rcAnalyzerOptions& opt,
                    uint32 windowSize,
                    vector<rcWindow>& inputImages,
                    vector<double>& baselineResults );

   // Test one window size with all origins
   void testWindowSize( const rcAnalyzerOptions& opt,
                        uint32 windowSize,
                        vector<rcWindow>& imageVector );
   
   // Create expected entropy results
   uint32 createExpectedResults( const rcAnalyzerOptions& opt,
                                   uint32 windowSize,
                                   const vector<rcWindow>& imageVector,
                                   const vector<double>& imageResults,
                                   vector<double>& expectedResults );

   // Test basic options functionality
   void testOptions();
   
   // Test options mutation
   void testOptionsMutation( const rcAnalyzerOptions& origOptions,
                                          const rcAnalyzerOptions& newOptions,
                                          bool resetOnly );
};

#endif // _UT_ANALYZER_H_

