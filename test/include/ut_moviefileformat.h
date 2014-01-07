/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *  $Id: ut_moviefileformat.h 4420 2006-05-15 18:48:40Z armanmg $
 *
 ***************************************************************************/

#ifndef _UT_MOVIEFILEFORMAT_H_
#define _UT_MOVIEFILEFORMAT_H_

#include <rc_unittest.h>
#include <rc_moviefileformat.h>

class UT_moviefileformat : public rcUnitTest {
 public:

  UT_moviefileformat();
  ~UT_moviefileformat();

  virtual uint32 run();

 private:
  // Test base ID header
  void testId();
  // Test rev1 header
  void testHeaderRev1();
  // Test rev2 header
  void testHeaderRev2();
  // Test base extension header
  void testExt();
  // Test TOC extension header
  void testTocExt();
  // Test ORG extension header
  void testOrgExt();
  // Test ORG extension header
  void testOrgExt( movieOriginType o, int64 bt, uint32 fc,
                   int32 w, int32 h, rcPixel depth,
                   movieFormatRev r, const char* c );
  // Test CONV extension header
  void testConvExt();
  // Test CONV extension header
  void testConvExt( uint32 so, uint32 fc, uint32 sm,
                    const rcRect& r, movieChannelType ch,
                    float frameInterval, bool pixelsRev, const char* c );
  // Test CAM extension header
  void testCamExt();
  // Test EXP extension header
  void testExpExt();
};

#endif /* _UT_MOVIEFILEFORMAT_H_ */
