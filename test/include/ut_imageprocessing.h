// Copyright 2002 Reify, Inc.

#ifndef _rcUT_IMAGEPROCESSING_H_
#define _rcUT_IMAGEPROCESSING_H_

#include <rc_unittest.h>
#include <rc_imageprocessing.h>

class UT_ImageProcessing : public rcUnitTest {
public:

    UT_ImageProcessing();
    ~UT_ImageProcessing();

    virtual uint32 run();

  private:
  uint32 testConvertPixels16();
    uint32 testGenHalfRes();
    uint32 testReversePixels8( int32 width, int32 height );
    void testPixels( const rcWindow& image, uint32 expectedValue );
    void testAndImage();
    
    uint32 resetRfRcWindow32to8( rcWindow& img32, rcWindow& img8 );
    uint32 verifyRfRcWindow32to8( rcWindow& img32, rcWindow& img8 );
    uint32 testRfRcWindow32to8( bool isGray );
    uint32 rfRcWindow32to8( rcWindow& img32, rcWindow& img8 );

    uint32 resetRfRcWindow8to32( rcWindow& img32, rcWindow& img8 );
    uint32 verifyRfRcWindow8to32( rcWindow& img32, rcWindow& img8 );
    uint32 testRfRcWindow8to32( bool isGray );
    uint32 rfRcWindow8to32( rcWindow& img32, rcWindow& img8 );
};

#endif // _rcUT_IMAGEPROCESSING_H_
