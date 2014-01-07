// Copyright (c) 2002 Reify Corp. All rights reserved.

#include "ut_qtime.h"

UT_Qtime::UT_Qtime()
{
}

UT_Qtime::~UT_Qtime()
{
    printSuccessMessage( "QuickTime test", mErrors );
}

uint32
UT_Qtime::run() {
    rcUNITTEST_ASSERT( 1 );

    //
    // rfImageNameSort tests
    //
    
    // Stability test (names without numbers should keep their order)
    {
        std::string s1("ImageX");
        std::string s2("ImageA");
        std::string s3("ImageD");

        vector<std::string> v;
        v.push_back( s1 );
        v.push_back( s2 );
        v.push_back( s3 );

        rfImageNameSort( v );
        rcUNITTEST_ASSERT( v[0] == s1 );
        rcUNITTEST_ASSERT( v[1] == s2 );
        rcUNITTEST_ASSERT( v[2] == s3 );
    }

    // Basic number test
    {
        std::string s1("Image1.tif");
        std::string s2("Image9.tif");
        std::string s3("Image10.tif");

        vector<std::string> v;
        v.push_back( s2 );
        v.push_back( s1 );
        v.push_back( s3 );

        rfImageNameSort( v );
        rcUNITTEST_ASSERT( v[0] == s1 );
        rcUNITTEST_ASSERT( v[1] == s2 );
        rcUNITTEST_ASSERT( v[2] == s3 );
    }

    // Basic number test
    {
        std::string s1("monoframe1.tif");
        std::string s2("monoframe10.tif");
        std::string s3("monoframe100.tif");

        vector<std::string> v;
        v.push_back( s2 );
        v.push_back( s1 );
        v.push_back( s3 );

        rfImageNameSort( v );
        rcUNITTEST_ASSERT( v[0] == s1 );
        rcUNITTEST_ASSERT( v[1] == s2 );
        rcUNITTEST_ASSERT( v[2] == s3 );
    }
    
    // Basic number test, no extensions
    {
        std::string s1("Image1");
        std::string s2("Image9");
        std::string s3("Image10");

        vector<std::string> v;
        v.push_back( s2 );
        v.push_back( s1 );
        v.push_back( s3 );

        rfImageNameSort( v );
        rcUNITTEST_ASSERT( v[0] == s1 );
        rcUNITTEST_ASSERT( v[1] == s2 );
        rcUNITTEST_ASSERT( v[2] == s3 );
    }

    // Large number test, no extensions
    {
        std::string s1("Image0000000");
        std::string s2("Image1000000");
        std::string s3("Image9000000");

        vector<std::string> v;
        v.push_back( s2 );
        v.push_back( s1 );
        v.push_back( s3 );

        rfImageNameSort( v );
        rcUNITTEST_ASSERT( v[0] == s1 );
        rcUNITTEST_ASSERT( v[1] == s2 );
        rcUNITTEST_ASSERT( v[2] == s3 );
    }

    // Numbers with preceding zeroes
    {
        std::string s1("Image00100.tif");
        std::string s2("Image00200.tif");
        std::string s3("Image00300.tif");

        vector<std::string> v;
        v.push_back( s2 );
        v.push_back( s3 );
        v.push_back( s1 );

        rfImageNameSort( v );
        rcUNITTEST_ASSERT( v[0] == s1 );
        rcUNITTEST_ASSERT( v[1] == s2 );
        rcUNITTEST_ASSERT( v[2] == s3 );
    }

    // Paths with numbers
    {
        std::string s1("/users/beast/666/Image00100.tif");
        std::string s2("/users/beast/666/Image00200.tif");
        std::string s3("/users/beast/666/Image00300.tif");

        vector<std::string> v;
        v.push_back( s2 );
        v.push_back( s3 );
        v.push_back( s1 );

        rfImageNameSort( v );
        rcUNITTEST_ASSERT( v[0] == s1 );
        rcUNITTEST_ASSERT( v[1] == s2 );
        rcUNITTEST_ASSERT( v[2] == s3 );
    }

    //
    // Color map tests
    //
    
    testRfFillColormap();
    
    return mErrors;
}

// Test color map filling
uint32
UT_Qtime::testRfFillColormap()
{
    const uint32 width = 179;
    const uint32 height = 377;

    // Empty handle on purpose
    CTabHandle ctb = 0;
    // Create 8-bit buffer and image
    rcFrame* buf8 = new rcFrame( width, height, rcPixel8 );
    rcWindow img8( buf8 );
    uint32 cMapSize = buf8->colorMapSize();

    const uint32 bogusColor = 666;
    // Reset color map values
    for ( uint32 i = 0; i < cMapSize; i++ ) {
        buf8->setColor( i, bogusColor );
    }

    // Test default color map, it should be linear gray
    rfFillColorMap( ctb, img8.frameBuf() );

    uint32 prevColor = bogusColor;
    
    for ( uint32 i = 0; i < cMapSize; i++ ) {
        uint32 color = buf8->getColor( i );
        uint32 r = rfRed( color );
        uint32 g = rfGreen( color );
        uint32 b = rfBlue( color );

        // Test that something was filled
        rcUNITTEST_ASSERT( color != bogusColor );
        // Test grayness
        rcUNITTEST_ASSERT( r == g && g == b );
        // All colors should be different
        rcUNITTEST_ASSERT( color != prevColor );
        prevColor = color;
    }
     
    return mErrors;
}
