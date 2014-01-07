/****************************************************************************
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 *  $Id: ut_capture.h 4391 2006-05-02 18:40:03Z armanmg $
 *
 ***************************************************************************/

#ifndef _UT_CAPTURE_H_
#define _UT_CAPTURE_H_

#include <rc_unittest.h>
#include <rc_capture.hp>

class UT_capture: public rcUnitTest {
  public:

    UT_capture();
    ~UT_capture();
    
    virtual uint32 run();
    
  private:
    void testCameraMapper();
};

#endif // _UT_CAPTURE_H_

