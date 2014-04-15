/*
 *  $Id: ut_correlationwindow.h 4391 2006-05-02 18:40:03Z armanmg $
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */

#ifndef _UT_CORRELATIONWINDOW_H_
#define _UT_CORRELATIONWINDOW_H_

#include <rc_unittest.h>
#include <rc_correlationwindow.h>
#include <rc_ncs.h>

class UT_Correlationwindow : public rcUnitTest {
  public:

    UT_Correlationwindow();
    ~UT_Correlationwindow();
    
    virtual uint32 run();
    
  private:
    void compareResults( bool useAltivec, const vector<rcCorr>& results1, const vector<rcCorr>& results2 );
    void testResults( bool useAltivec );
    void testPerformance( bool useAltivec );

    template <class T>
        void testPixelAccessors( rcWindow& testImage, const T& value1 );
};

#endif // _UT_CORRELATIONWINDOW_H_

