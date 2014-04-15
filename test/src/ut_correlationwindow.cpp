/*
 *  $Id: ut_correlationwindow.cpp 4420 2006-05-15 18:48:40Z armanmg $
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */

#include "ut_correlationwindow.h"

#include <rc_time.h>
#include <rc_ncs.h>

// Create a vector of random images
// Note: these images will leak memory...but we dont care right now
static vector<rcWindow> createRandomVector( uint32 size, int32 width, int32 height )
{
    vector<rcWindow> imageVector;

    // Create dummy images
    for ( uint32 i = 0; i < size; ++i ) {
        rcSharedFrameBufPtr frameBufPtr = new rcFrame( width, height, rcPixel8 );
        // Create image
        rcWindow dummyImage( frameBufPtr );
        // Fill with random data
        dummyImage.randomFill();
        imageVector.push_back( dummyImage );
    }

    return imageVector;
}

#ifdef notyet
static vector<rcWindow> createConstVector( uint32 size, int32 width, int32 height, uint32 value )
{
    vector<rcWindow> imageVector;

    // Create dummy images
    for ( uint32 i = 0; i < size; ++i ) {
        rcSharedFrameBufPtr frameBufPtr = new rcFrame( width, height, rcPixel8 );
        // Create image
        rcWindow dummyImage( frameBufPtr );
        // Fill with const data
        dummyImage.setAllPixels( value );
        imageVector.push_back( dummyImage );
    }

    return imageVector;
}
#endif

UT_Correlationwindow::UT_Correlationwindow()
{
}

UT_Correlationwindow::~UT_Correlationwindow()
{
    printSuccessMessage( "rcCorrelationWindow test", mErrors );
}

uint32
UT_Correlationwindow::run() {
    // Failure tests
    {
        rcWindow i8( 640, 480, rcPixel8 );
        rcWindow i16( 640, 480, rcPixel16 );
        rcWindow i32( 640, 480, rcPixel32S );

        {
            uint32 exceptions = 0;
            // These should work
            try {
                rcCorrelationWindow<uint8>  w8( i8 );
                rcCorrelationWindow<uint16> w16( i16 );
                rcCorrelationWindow<uint32> w32( i32 );
            } catch ( general_exception& e ) {
                ++exceptions;
                cerr << "Error, caught exception: " << e.what() << endl;
            }
            rcUNITTEST_ASSERT( exceptions == 0 );
        }
        {
            uint32 exceptions = 0;
            // This should throw an exception
            try {
                rcCorrelationWindow<uint8> w( i16 );
            } catch ( general_exception& e ) {
                ++exceptions;
            }
            rcUNITTEST_ASSERT( exceptions == 1 );
        }
        {
            uint32 exceptions = 0;
            // This should throw an exception
            try {
                rcCorrelationWindow<uint8> w( i32 );
            } catch ( general_exception& e ) {
                ++exceptions;
            }
            rcUNITTEST_ASSERT( exceptions == 1 );
        }
        {
            uint32 exceptions = 0;
            // This should throw an exception
            try {
                rcCorrelationWindow<uint16> w( i8 );
            } catch ( general_exception& e ) {
                ++exceptions;
            }
            rcUNITTEST_ASSERT( exceptions == 1 );
        }
        {
            uint32 exceptions = 0;
            // This should throw an exception
            try {
                rcCorrelationWindow<uint16> w( i32 );
            } catch ( general_exception& e ) {
                ++exceptions;
            }
            rcUNITTEST_ASSERT( exceptions == 1 );
        }
        {
            uint32 exceptions = 0;
            // This should throw an exception
            try {
                rcCorrelationWindow<uint32> w( i8 );
            } catch ( general_exception& e ) {
                ++exceptions;
            }
            rcUNITTEST_ASSERT( exceptions == 1 );
        }
        {
            uint32 exceptions = 0;
            // This should throw an exception
            try {
                rcCorrelationWindow<uint32> w( i16 );
            } catch ( general_exception& e ) {
                ++exceptions;
            }
            rcUNITTEST_ASSERT( exceptions == 1 );
        }
    }

    // Basic tests
    {
        rcWindow testImage8( 5, 11, rcPixel8 );
        rcWindow testImage16( 5, 11, rcPixel16 );
        rcWindow testImage32( 5, 11, rcPixel32S );

        rcCorrelationWindow<uint8> w8( testImage8 );
        rcCorrelationWindow<uint16> w16( testImage16 );
        rcCorrelationWindow<uint32> w32( testImage32 );

        // Test pixel accessor methods for various pixel widths
        {
            uint32 max = rcUINT8_MAX - 1;
            for ( uint8 i = 0; i < max; ++i )
                testPixelAccessors( testImage8, i );
        }
        {
            uint32 max = rcUINT16_MAX - 1;
            for ( uint16 i = 0; i < max; i+=(rcUINT16_MAX/667) ) {
                testPixelAccessors( testImage16, i );
            }
        }
        {
            double max = rcUINT32_MAX - 1;
            for ( double i = 0; i < max; i+=(rcUINT32_MAX/667) ) {
                testPixelAccessors( testImage32, int32(i) );
            }
        }

        rcUNITTEST_ASSERT( !w8.sumValid() );
        rcUNITTEST_ASSERT( !w16.sumValid());
        rcUNITTEST_ASSERT( !w32.sumValid() );
        double sum = 6.6;
        double sumSquares = sum * sum;

        w8.sum( sum );
        w8.sumSquares( sumSquares );
        rcUNITTEST_ASSERT( w8.sumValid() );
        rcUNITTEST_ASSERT( (w8.sum() == sum) && (w8.sumSquares() == sumSquares) );

        w16.sum( sum );
        w16.sumSquares( sumSquares );
        rcUNITTEST_ASSERT( w16.sumValid() );
        rcUNITTEST_ASSERT( (w16.sum() == sum) && (w16.sumSquares() == sumSquares) );

        w32.sum( sum );
        w32.sumSquares( sumSquares );
        rcUNITTEST_ASSERT( w32.sumValid() );
        rcUNITTEST_ASSERT( (w32.sum() == sum) && (w32.sumSquares() == sumSquares) );

        // Accessors
        rcWindow i(640, 480, rcPixel8 );
        rcWindow i8( i, 1, 1, 64, 48 );
        rcCorrelationWindow<uint8> cw8( i8 );
        rcUNITTEST_ASSERT( cw8.x() == i8.x() );
        rcUNITTEST_ASSERT( cw8.y() == i8.y() );
        rcUNITTEST_ASSERT( cw8.width() == i8.width() );
        rcUNITTEST_ASSERT( cw8.height() == i8.height() );

        rcUNITTEST_ASSERT( cw8.depth() == i8.depth() );

        rcUNITTEST_ASSERT( cw8.rowPointer( 1 ) == i8.rowPointer( 1 ) );
        rcUNITTEST_ASSERT( cw8.rowUpdate() == i8.rowUpdate() );
        rcUNITTEST_ASSERT( cw8.rowPelUpdate() == i8.rowUpdate() );
        rcUNITTEST_ASSERT( cw8.pelPointer(1,2) == i8.pelPointer(1,2) );
        rcUNITTEST_ASSERT( cw8.height() == i8.height() );
        rcUNITTEST_ASSERT( cw8.frameBuf() == i8.frameBuf() );

        // Mutators
        cw8.sum( sum );
        cw8.sumSquares( sumSquares );
        rcUNITTEST_ASSERT( cw8.sumValid() );
        cw8.invalidate();
        rcUNITTEST_ASSERT( !cw8.sumValid()  );
        cw8.sum( sum );
        cw8.sumSquares( sumSquares );
        rcUNITTEST_ASSERT( cw8.sumValid() );
        rcIPair offset(-5,-5);
        bool mutated = cw8.translate( offset );
        rcUNITTEST_ASSERT( !mutated && cw8.sumValid() );
        offset = rcIPair(0,1);
        mutated = cw8.translate( offset );
        rcUNITTEST_ASSERT( mutated && !cw8.sumValid() );
    }

    // Correlation result test
    testResults( false );
    testResults( true );
    // Performance test
    //testPerformance( false );
#if defined (PERFORMANCE)
    testPerformance( true );
#endif

    return mErrors;
}

template <class T>
void UT_Correlationwindow::testPixelAccessors( rcWindow& testImage, const T& value1 )
{
        const T value2 = value1 + 1;

        testImage.setAllPixels( value2 );
        // Set first pixel of each row to another value
        for (int32 y = 0; y < testImage.height(); ++y ) {
            testImage.setPixel( 0, y, value1 );
        }

        // Test accessors
        rcCorrelationWindow<T> wn( testImage );

        rcUNITTEST_ASSERT( testImage.x() == wn.x() );

        for (int32 y = 0; y < wn.height(); ++y ) {
            const uint8* pel = wn.rowPointer( y );
            const T* pel2 = wn.pelPointer( 0, y );
            rcUNITTEST_ASSERT( pel != 0 );
            rcUNITTEST_ASSERT( *pel2 == value1 );
            if ( *pel2 != value1 )
                cerr << "got " << uint32(*pel2) << ", expected " << value1 << endl;
        }

        for (int32 x = 1; x < wn.width(); ++x ) {
            for (int32 y = 0; y < wn.height(); ++y ) {
                const T* pel = wn.pelPointer( x, y );
                rcUNITTEST_ASSERT( *pel == value2 );
                if ( *pel != value2 )
                    cerr << "got " << uint32(*pel) << ", expected " << value2 << endl;
            }
        }

        for (int32 y = 0; y < wn.height(); ++y ) {
            const T* row = wn.rowPelPointer( y );
            rcUNITTEST_ASSERT( row[0] == value1 );
            const T* pel2 = wn.pelPointer( 0, y );
            rcUNITTEST_ASSERT( row[0] == *pel2 );
            for (int32 x = 1; x < wn.width(); ++x ) {
                const T pel = row[x];
                rcUNITTEST_ASSERT( pel == value2 );
                pel2 = wn.pelPointer( x, y );
                rcUNITTEST_ASSERT( pel == *pel2 );
            }
        }

        int32 height = wn.height();
        const T* row = wn.rowPelPointer( 0 );
        while (height-- ) {
            rcUNITTEST_ASSERT( row[0] == value1 );
            for (int32 w = 1; w < wn.width(); ++w ) {
                const T pel = row[w];
                rcUNITTEST_ASSERT( pel == value2 );
            }
            row += wn.rowPelUpdate();
        }
    }

void UT_Correlationwindow::testResults( bool useAltivec )
{
    vector<rcWindow> imageVectorSource = createRandomVector( 100, 149, 123 );
    vector<rcCorr> imageResults;
    const rsCorrParams params;

    rfForceSIMD ( useAltivec );

    // Generate correct results using old API
    for ( uint32 j = 0; j < imageVectorSource.size(); ++j ) {
        rcCorr res;
        rfCorrelate( imageVectorSource[0], imageVectorSource[j], params, res );
        imageResults.push_back( res );
    }

    // Produce rcCorrelationWindow objects from rcWindow objects
    vector<rcCorrelationWindow<uint8> > imageVector;
    for ( uint32 i = 0; i < imageVectorSource.size(); ++i ) {
        imageVector.push_back( rcCorrelationWindow<uint8>( imageVectorSource[i]) );
    }

    // Correlate, no cached sums
    {
        vector<rcCorr> imageCorrelationResults;
        for ( uint32 j = 0; j < imageVector.size(); ++j ) {
            rcCorr res;
            rcCorrelationWindow<uint8> imageA( imageVector[0] );
            imageA.invalidate();
            imageVector[j].invalidate();
            // Correlate first image with all others
            rfCorrelateWindow( imageA, imageVector[j], params, res );
            imageCorrelationResults.push_back( res );
        }
        // Compare results
        compareResults( useAltivec, imageResults, imageCorrelationResults );
    }

    // Correlate, cached sums for imageA
    {
        vector<rcCorr> imageCorrelationResults;
        rcCorrelationWindow<uint8> imageA( imageVector[0] );
        rcCorr resA;
        // Self-correlate to produce cached sums
        rfCorrelateWindow( imageA, imageA, params, resA );

        for ( uint32 j = 0; j < imageVector.size(); ++j ) {
            rcCorr res;
            imageVector[j].invalidate();
            // Correlate first image with all others
            rfCorrelateWindow( imageA, imageVector[j], params, res );
            imageCorrelationResults.push_back( res );
        }
        // Compare results
        compareResults( useAltivec, imageResults, imageCorrelationResults );
    }

    // Correlate, cached sums for imageB
    {
        vector<rcCorr> imageCorrelationResults;
        rcCorrelationWindow<uint8> imageA( imageVector[0] );

        for ( uint32 j = 0; j < imageVector.size(); ++j ) {
            rcCorr res1, res2;
            // Self-correlate to produce cached sums
            rfCorrelateWindow( imageVector[j], imageVector[j], params, res1 );
            imageA.invalidate();
            // Correlate first image with all others
            rfCorrelateWindow( imageA, imageVector[j], params, res2 );
            imageCorrelationResults.push_back( res2 );
        }
        // Compare results
        compareResults( useAltivec, imageResults, imageCorrelationResults );
    }

    // Correlate, cached sums for all images
    {
        vector<rcCorr> imageCorrelationResults;
        rcCorr res1;
        rcCorrelationWindow<uint8> imageA( imageVector[0] );
        // Self-correlate to produce cached sums
        rfCorrelateWindow( imageA, imageA, params, res1 );

        for ( uint32 j = 0; j < imageVector.size(); ++j ) {
            rcCorr res2;
            rcCorrelationWindow<uint8> imageB( imageVector[j] );
            // Self-correlate to produce cached sums
            rfCorrelateWindow( imageB, imageB, params, res1 );
            // Correlate first image with all others
            rfCorrelateWindow( imageA, imageB, params, res2 );
            imageCorrelationResults.push_back( res2 );
        }
        // Compare results
        compareResults( useAltivec, imageResults, imageCorrelationResults );
    }

    // Correlate, call multiple times with the same images
    {
        vector<rcCorr> imageCorrelationResults;
        rcCorrelationWindow<uint8> imageA( imageVector[0] );

        for ( uint32 j = 0; j < imageVector.size(); ++j ) {
            rcCorr res, res1, res2, res3, res4;
            rcCorrelationWindow<uint8> imageB( imageVector[j] );
            imageB.invalidate();
            imageA.invalidate();
            // None cached
            rfCorrelateWindow( imageA, imageB, params, res1 );
            // imageA cached
            imageB.invalidate();
            rfCorrelateWindow( imageA, imageB, params, res2 );
            // imageB cached
            imageA.invalidate();
            rfCorrelateWindow( imageA, imageB, params, res3 );
            // Both cached
            rfCorrelateWindow( imageA, imageB, params, res4 );
            rcUNITTEST_ASSERT( res1 == res2 );
            rcUNITTEST_ASSERT( res2 == res3 );
            rcUNITTEST_ASSERT( res3 == res4 );
            rfCorrelateWindow( imageA, imageB, params, res );
            rcUNITTEST_ASSERT( res == res1 );
            rfCorrelateWindow( imageB, imageA, params, res );
            rcUNITTEST_ASSERT( res.r() == res1.r() );
            rfCorrelateWindow( imageA, imageB, params, res );
            rcUNITTEST_ASSERT( res == res1 );
        }
    }
}

void UT_Correlationwindow::testPerformance( bool useAltivec )
{
    rmUnused ( useAltivec );

}

void UT_Correlationwindow::compareResults( bool useAltivec, const vector<rcCorr>& results1, const vector<rcCorr>& results2 )
{
    rmUnused( useAltivec );
    // Compare results
    rcUNITTEST_ASSERT( results1.size() == results2.size() );
    for ( uint32 j = 0; j < results1.size(); ++j ) {
        rcCorr r1 = results1[j];
        rcCorr r2 = results2[j];
        rcUNITTEST_ASSERT( r1.Si() == r2.Si() );
        rcUNITTEST_ASSERT( r1.Sii() == r2.Sii() );
        rcUNITTEST_ASSERT( r1.Sm() == r2.Sm() );
        rcUNITTEST_ASSERT( r1.Smm() == r2.Smm() );
        rcUNITTEST_ASSERT( r1.Sim() == r2.Sim() );
        if ( r1.Sim() != r2.Sim() )
            cerr << "r1.Sim = " << r1.Sim() << ", r2.Sim = " << r2.Sim() << endl;
        if ( r1.Sii() != r2.Sii() )
            cerr << "r1.Sii = " << r1.Sii() << ", r2.Sii = " << r2.Sii() << endl;
        if ( r1.Smm() != r2.Smm() )
            cerr << "r1.Smm = " << r1.Smm() << ", r2.Smm = " << r2.Smm() << endl;
        rcUNITTEST_ASSERT( r1.n() == r2.n() );
    }
}
