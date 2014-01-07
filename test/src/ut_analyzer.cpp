// Copyright (c) 2002 Reify Corp. All rights reserved.

#include <rc_framebuf.h>
#include <rc_analyzer.h>
#include <rc_moviegrabber.h>
#include <rc_qtime.h>

// Temporal window size of 1 and huge are not tested.

#include "ut_analyzer.h"

// Create a vector of random images
// Note: these images will leak memory...but we dont care right now
static vector<rcWindow> createRandomVector( uint32 size, uint32 width, uint32 height )
{
    vector<rcWindow> imageVector;

    // Create dummy images
    for ( uint32 i = 0; i < size; ++i ) {
        rcSharedFrameBufPtr frameBufPtr = new rcFrame( width, height, rcPixel8 );
        // Create a gray scale color map
        rfFillColorMap( 0, frameBufPtr );
        // Create image
        rcWindow dummyImage( frameBufPtr );
        // Fill with random data
        dummyImage.randomFill();
        imageVector.push_back( dummyImage );
    }

    return imageVector;
}

UT_Analyzer::UT_Analyzer()
{
}

UT_Analyzer::~UT_Analyzer()
{
    printSuccessMessage( "rcAnalyzer test", mErrors );
}

uint32
UT_Analyzer::run() {
   
    vector<rcWindow> imageVector3 = createRandomVector( 3, 160, 160 );
    vector<rcWindow> imageVector9 = createRandomVector( 9, 160, 160 );
    
    // Failure tests
    {
        // Invalid grabber test
        {
            // Create an empty grabber
            std::string s1;
            rcMovieGrabber input( s1, NULL );
            
            // Empty grabber should cause an immediate failure
            rcAnalyzerOptions parm;
            parm.setWindowSize( 66 );
            rcAnalyzer analyzer( parm, input );
            
            rcAnalyzerResult result;
            rcFrameGrabberStatus status = analyzer.getNextResult( result, true );
            rcUNITTEST_ASSERT( status == eFrameStatusError );
            rcUNITTEST_ASSERT( analyzer.getLastError() != eFrameErrorOK );
        }
    }

    // Basic tests
    {
        // Default analyzer options
        rcAnalyzerOptions defaultAnalyzerOptions;
        defaultAnalyzerOptions.setEntropyDefinition( rcSimilarator::eVisualEntropy );

#if 0
        // Basic blocking test for window size 1
        {
            // Create a grabber
            rcVectorGrabber input( imageVector9 );
            // Create an analyzer with default options
            rcAnalyzerOptions parm( defaultAnalyzerOptions ) ;
            parm.setWindowSize( 1 );
            rcAnalyzer analyzer( parm, input );
            // Vector of undefined entropies
            vector<double> results( imageVector9.size(), -1.0 );
            // Entropies should be undefined
            testAnalyzer( analyzer, imageVector9, eFrameStatusOK, eFrameErrorOK, results, true, true );
        }
#endif 
        // Basic blocking test for window size bigger than data size
        {
            // Create a grabber
            rcVectorGrabber input( imageVector9 );
            // Size greater than number of images available
            rcAnalyzerOptions parm( defaultAnalyzerOptions ) ;
            parm.setWindowSize( 10 );
            rcAnalyzer analyzer( parm, input );
            // Vector of undefined entropies
            vector<double> results( imageVector9.size(), -1.0 );
            // Entropies should be undefined
            testAnalyzer( analyzer, imageVector9, eFrameStatusOK, eFrameErrorOK, results, true, true );
        }
#if 0
        // Basic blocking test for "infinite" window size
        {
            // Vector of undefined entropies
            vector<double> results( imageVector9.size(), 0.0 );
            // Calculate baseline results
            rsCorrParams cp = defaultAnalyzerOptions.corrParams();
            // This produces eVisualEntropy
            rfOptoKineticEnergy( imageVector9, results, cp, 0 );

            // Create a grabber
            rcVectorGrabber input( imageVector9 );
            rcAnalyzerOptions parm( defaultAnalyzerOptions ) ;
            parm.setWindowSize( rcUINT32_MAX );
            rcAnalyzer analyzer( parm, input );

            // Vector of undefined entropies
            vector<double> uresults( imageVector9.size(), -1.0 );
            
            // Window size is infinite so entropies should be undefined
            testAnalyzer( analyzer, imageVector9, eFrameStatusOK, eFrameErrorOK, uresults, true, true );
        }
#endif
        // Basic blocking test for a few window sizes
        {
            uint32 increment = 2;
            
            for ( uint32 size = 3; size < 69; size += increment )
            {
                vector<rcWindow> imageVector = createRandomVector( size, 16, 16 );
                testWindowSize( defaultAnalyzerOptions, imageVector.size(), imageVector );
                increment += 2;
            }

        }
        
        // Options test
        testOptions();
        
        // Options mutation flush test
        {
            // Create default options
            rcAnalyzerOptions origOpt( defaultAnalyzerOptions ) ;
            origOpt.setWindowSize( 3 );
            origOpt.setOrigin( eAnalyzerResultOriginLeft );
            
            // Set new options, cached frames should be flushed
            rcAnalyzerOptions newOpt( origOpt );
            newOpt.setWindowSize( 5 );

            // Test mutation
            testOptionsMutation( origOpt, newOpt, false );
            // Test reset only
            testOptionsMutation( origOpt, newOpt, true );
            
            // Test again, another window size
            newOpt.setWindowSize( 2 );
            testOptionsMutation( origOpt, newOpt, false );
            testOptionsMutation( origOpt, newOpt, true );
        }

#if 0        
        // Basic non-blocking test for window size 1
        {
            // Create a grabber
            rcVectorGrabber input( imageVector9 );

            rcAnalyzerOptions parm;
            uint32 windowSize = 1;
            parm.setWindowSize( windowSize );
            rcAnalyzer analyzer( parm, input );

            rcAnalyzerResult result;

            // Window size is 1 so all calls should return immediately and
            // entropies should be undefined
            for( uint32 i = 0; i < imageVector9.size(); ++i ) {
                // Non-blocking mode not implemented yet
                rcFrameGrabberStatus status = analyzer.getNextResult( result, false );
                rcUNITTEST_ASSERT( status == eFrameStatusError );
                rcUNITTEST_ASSERT( analyzer.getLastError() == eFrameErrorNotImplemented );
                // Entropy should be undefined
                rcUNITTEST_ASSERT( result.entropy() == -1.0 );
            }
        }
#endif
        
    }
    
    return mErrors;
}

// Test analyzer instance
void UT_Analyzer::testAnalyzer( rcAnalyzer& analyzer,
                                const vector<rcWindow>& images,
                                rcFrameGrabberStatus expectedStatus,
                                rcFrameGrabberError expectedError,
                                const vector<double>& expectedEntropies,
                                bool isBlocking,
                                bool testEOF )
{
    rmAssert( images.size() == expectedEntropies.size() );
    // Entropy scores should be at least this accurate...
    const double minAccuracy = 1.0/cmIntegerCorr;
    rcAnalyzerResult result;
            
    // Test all images
    for( uint32 i = 0; i < images.size(); ++i ) {
        // All blocking calls should return immediately
        rcFrameGrabberStatus status = analyzer.getNextResult( result, isBlocking );

        // Status test
        rcUNITTEST_ASSERT( status == expectedStatus );
        if ( status != expectedStatus ) {
            fprintf( stderr, "status %i != expected status %i\n",
                     status, expectedStatus );
        }
        // Error test
        rcUNITTEST_ASSERT( analyzer.getLastError() == expectedError );
        if ( analyzer.getLastError() != expectedError ) {
            fprintf( stderr, "error %i != expected error %i\n",
                     analyzer.getLastError(), expectedError );
        }
        // Image test
        rcUNITTEST_ASSERT( result.frameBuf() == images[i].frameBuf() );
        if ( result.frameBuf() != images[i].frameBuf() ) {
            fprintf( stderr, "frame %x != expected frame %x\n",
                       (uint32)result.frameBuf()->rawData(), (uint32)images[i].frameBuf()->rawData() );
        }
        // Entropy test
        const double diff = rmABS(result.entropy() - expectedEntropies[i]);
        rcUNITTEST_ASSERT( diff < minAccuracy );

        if ( diff >= minAccuracy) {
            fprintf( stderr, "idx %i entropy value %.16f != expected entropy value %.16f, diff %.16f\n",
                     i, result.entropy(), expectedEntropies[i], diff );
        }
    }
    // Exit test
    if ( testEOF ) {
        // There should be no more results
        rcFrameGrabberStatus status = analyzer.getNextResult( result, isBlocking );
        rcUNITTEST_ASSERT( status == eFrameStatusEOF );
    }
}

// Test one window
void UT_Analyzer::testWindow( const rcAnalyzerOptions& opt,
                              uint32 windowSize,
                              vector<rcWindow>& inputImages,
                              vector<double>& baselineResults )
{
  
    
    // Double up the vector for grabbing
    vector<rcWindow> imageVector2( inputImages );

    for (vector<rcWindow>::iterator i = inputImages.begin(); i < inputImages.end(); i++ )
        imageVector2.push_back( *i );

    vector<double> expectedResults;
    createExpectedResults( opt, windowSize, imageVector2, baselineResults, expectedResults );
    
    // Create a grabber
    rcVectorGrabber input( imageVector2 );
    // Create an analyzer
    rcAnalyzerOptions parm( opt );
    parm.setWindowSize( windowSize );
    rcAnalyzer analyzer( parm, input );
    // Image vector size is 2 x window size
    testAnalyzer( analyzer, imageVector2, eFrameStatusOK, eFrameErrorOK, expectedResults, true, false );
}

// Test one window
uint32 UT_Analyzer::createExpectedResults( const rcAnalyzerOptions& opt,
                                             uint32 windowSize,
                                             const vector<rcWindow>& imageVector,
                                             const vector<double>& imageResults,
                                             vector<double>& expectedResults )

{
    // Get result origin
    rcAnalyzerResultOrigin origin = opt.origin();
    uint32 cycleOffset = 0;
    uint32 definedResultCount = 0;
    uint32 undefinedCount = 0;
    rcSimilarator::rcEntropyDefinition eDef = opt.entropyDefinition();
    expectedResults.clear();
    
    switch ( origin ) {
        case eAnalyzerResultOriginLeft:
            cycleOffset = 0;
            definedResultCount = imageVector.size() - (windowSize - 1);
            undefinedCount = 0;
            break;
        case eAnalyzerResultOriginRight:
            cycleOffset = windowSize-1;
            definedResultCount = imageVector.size();
            undefinedCount = windowSize - 1;
            break;
        case eAnalyzerResultOriginCenter:
            cycleOffset = (windowSize-1)/2;
            undefinedCount = (windowSize - 1)/2;
            definedResultCount = imageVector.size() - undefinedCount;
            break;
        default:
            rmAssert( 0 );
            break;
    }
                
    // Produce expected values in expected order
    uint32 count = 0;
    while ( count < imageVector.size() ) {
        if ( count < undefinedCount ) {
            expectedResults.push_back( -1.0 );
            ++count;
#ifdef DEBUG_LOG            
            cout << expectedResults.back() << " ";
#endif            
        } else {
            for ( uint32 i = cycleOffset; i < windowSize; ++i ) {
                if ( count < definedResultCount ) {
                    if ( imageResults[i] != -1.0 ) {
                        switch ( eDef ) {
                            case rcSimilarator::eACI:
                            expectedResults.push_back( 1.0-imageResults[i] );
                            break;
                            case rcSimilarator::eVisualEntropy:
                                expectedResults.push_back( imageResults[i] );
                                break;
                            default:
                                // Unsupported
                                rmAssert( 0 );
                                break;
                        }
                    } else
                        expectedResults.push_back( imageResults[i] );
#ifdef DEBUG_LOG                        
                    cout << "[" << i+1 << "] " << expectedResults.back() << " ";
#endif                    
                }
                else {
                    expectedResults.push_back( -1.0 );
#ifdef DEBUG_LOG            
                    cout << expectedResults.back() << " ";
#endif                    
                }
                ++count;
            }
            cycleOffset = 0;
        }
        
    }
#ifdef DEBUG_LOG   
    cout << "total " << count << endl;
#endif    

    return count;
}
 
// Test one window with all origins
void UT_Analyzer::testWindowSize( const rcAnalyzerOptions& opt,
                                  uint32 windowSize,
                                  vector<rcWindow>& imageVector )
{
    // Mutable options
    rcAnalyzerOptions options( opt );

    // Vector of baseline results
    vector<double> baselineResults( imageVector.size(), -1.0 );
    
    if ( imageVector.size() > 1 ) {
        // Calculate baseline results
        rsCorrParams cp = opt.corrParams();
        // This produces eVisualEntropy
        rfOptoKineticEnergy( imageVector, baselineResults, cp, 0 );
    }
    
    options.setOrigin( eAnalyzerResultOriginLeft );
    testWindow( options, windowSize, imageVector, baselineResults );   

    options.setOrigin( eAnalyzerResultOriginCenter );
    testWindow( options, windowSize, imageVector, baselineResults );

    options.setOrigin( eAnalyzerResultOriginRight );
    testWindow( options, windowSize, imageVector, baselineResults );

    options.setEntropyDefinition( rcSimilarator::eACI );
    testWindow( options, windowSize, imageVector, baselineResults );

    options.setEntropyDefinition( rcSimilarator::eVisualEntropy );
    testWindow( options, windowSize, imageVector, baselineResults );
}

// Test basic options functionality
void UT_Analyzer::testOptions()
{
    // TODO: add more tests

    // Basic (in)equality operator tests
    rcAnalyzerOptions opt1;
    rcUNITTEST_ASSERT( opt1.windowSize() > 0 );
            
    {
        // Test all ctors
        rcAnalyzerOptions opt2( opt1 );
        rcAnalyzerOptions opt3;
        rcAnalyzerOptions opt4( opt1.corrParams(),
                                opt1.mode(),
                                opt1.origin(),
                                opt1.bound(),
                                opt1.windowSize(),
                                opt1.entropyDefinition(),
                                opt1.depth());
        rcUNITTEST_ASSERT( opt1 == opt2 );
        rcUNITTEST_ASSERT( opt1 == opt3 );
        rcUNITTEST_ASSERT( opt1 == opt4 );
        rcUNITTEST_ASSERT( !(opt1 != opt2) );
        rcUNITTEST_ASSERT( !(opt1 != opt3) );
        rcUNITTEST_ASSERT( !(opt1 != opt4) );
    }
    {
        // Test analysis mode
        rcAnalyzerOptions opt2( opt1 );
        if ( opt1.mode() == eAnalyzerFullCorrelation ) {
            opt2.setMode( eAnalyzerApproximation );
            rcUNITTEST_ASSERT( opt2.mode() == eAnalyzerApproximation );
        }
        else if ( opt1.mode() == eAnalyzerApproximation ) {
            opt2.setMode( eAnalyzerFullCorrelation );
            rcUNITTEST_ASSERT( opt2.mode() == eAnalyzerFullCorrelation );
        }
        rcUNITTEST_ASSERT( opt1 != opt2 );
    }
    {
        // Test result origin
        rcAnalyzerOptions opt2( opt1 );
        if ( opt1.origin() == eAnalyzerResultOriginLeft ) {
            opt2.setOrigin( eAnalyzerResultOriginCenter );
            rcUNITTEST_ASSERT( opt2.origin() == eAnalyzerResultOriginCenter );
        }
        else if ( opt1.origin() == eAnalyzerResultOriginRight ) {
            opt2.setOrigin( eAnalyzerResultOriginLeft );
            rcUNITTEST_ASSERT( opt2.origin() == eAnalyzerResultOriginLeft );
        }
        else if ( opt1.origin() == eAnalyzerResultOriginCenter ) {
            opt2.setOrigin( eAnalyzerResultOriginRight );
            rcUNITTEST_ASSERT( opt2.origin() == eAnalyzerResultOriginRight );
        }
        rcUNITTEST_ASSERT( opt1 != opt2 );
    }
    {
        // Test bounding rectangle
        rcAnalyzerOptions opt2( opt1 );
        rcAnalyzerOptions opt3( opt1 );
        rcRect rect1( 0, 0, 666, 666 );
        rcRect rect2( 0, 0, 666, 667 );
        rcUNITTEST_ASSERT( rect1 != rect2 );
                
        opt2.setBound( rect1 );
        rcUNITTEST_ASSERT( opt2.bound() == rect1 );
        opt3.setBound( rect2 );
        rcUNITTEST_ASSERT( opt3.bound() == rect2 );
                
        rcUNITTEST_ASSERT( opt1 != opt2 );
        rcUNITTEST_ASSERT( opt2 != opt3 );
        rcUNITTEST_ASSERT( opt1 != opt3 );
    }
    {
        // Test sliding window size
        rcAnalyzerOptions opt2( opt1 );
        rcAnalyzerOptions opt3( opt1 );
        uint32 size1 = 665;
        uint32 size2 = 42;
        rcUNITTEST_ASSERT( size1 != size2 );
                
        opt2.setWindowSize( size1 );
        rcUNITTEST_ASSERT( opt2.windowSize() == size1 );
        opt3.setWindowSize( size2 );
        rcUNITTEST_ASSERT( opt3.windowSize() == size2 );
                                
        rcUNITTEST_ASSERT( opt1 != opt2 );
        rcUNITTEST_ASSERT( opt2 != opt3 );
        rcUNITTEST_ASSERT( opt1 != opt3 );
                
        // Zero is not allowed, it will be rounded up
        opt2.setWindowSize( 0 );
        rcUNITTEST_ASSERT( opt1.windowSize() > 0 );
    }
    {
        // Test entropy definition
        rcAnalyzerOptions opt2( opt1 );
        rcAnalyzerOptions opt3( opt1 );
        rcSimilarator::rcEntropyDefinition d1 = rcSimilarator::eVisualEntropy;
        rcSimilarator::rcEntropyDefinition d2 = rcSimilarator::eRMSVisualEntropy;
        
        rcUNITTEST_ASSERT( d1 != d2 );
                
        opt2.setEntropyDefinition( d1 );
        rcUNITTEST_ASSERT( opt2.entropyDefinition() == d1 );
        opt3.setEntropyDefinition( d2 );
        rcUNITTEST_ASSERT( opt3.entropyDefinition() == d2 );
                                
        rcUNITTEST_ASSERT( opt1 != opt2 );
        rcUNITTEST_ASSERT( opt2 != opt3 );
        rcUNITTEST_ASSERT( opt1 != opt3 );
    }

    {
        // Test depth
        rcAnalyzerOptions opt2( opt1 );
        rcAnalyzerOptions opt3( opt1 );
        rcPixel d1 = rcPixel8;
        rcPixel d2 = rcPixel32;
        
        rcUNITTEST_ASSERT( d1 != d2 );
                
        opt2.setDepth( d1 );
        rcUNITTEST_ASSERT( opt2.depth() == d1 );
        opt3.setDepth( d2 );
        rcUNITTEST_ASSERT( opt3.depth() == d2 );
                                
        rcUNITTEST_ASSERT( opt2 != opt3 );
        rcUNITTEST_ASSERT( opt1 != opt3 );
    }
}

// Test options mutation
void UT_Analyzer::testOptionsMutation( const rcAnalyzerOptions& origOptions,
                                       const rcAnalyzerOptions& newOptions,
                                       bool resetOnly )
{
    vector<rcWindow> imageVector9 = createRandomVector( 9, 320, 160 );
    
      // Create a grabber
    rcVectorGrabber input( imageVector9 );

    rcAnalyzer analyzer( origOptions, input );
    rcUNITTEST_ASSERT( origOptions == analyzer.getOptions() );
    uint32 oldWindowSize = origOptions.windowSize();
    
    // Vector of undefined entropies
    vector<double> results( imageVector9.size(), -1.0 );
    // Entropies should be undefined
    rcAnalyzerResult result;
    
    // Get first frame
    rcFrameGrabberStatus status = analyzer.getNextResult( result, true );
    rcUNITTEST_ASSERT( status == eFrameStatusOK );
    rcUNITTEST_ASSERT( analyzer.getLastError() == eFrameErrorOK );
    // We should get the first frame
    rcUNITTEST_ASSERT( result.frameBuf() == imageVector9[0].frameBuf() );
    rcUNITTEST_ASSERT( result.entropy() != -1.0 );

    if ( resetOnly ) {
        // Do a manual reset, keep old options
        rcFrameGrabberStatus s = analyzer.reset();
        rcUNITTEST_ASSERT( s == eFrameStatusOK );
    } else {
        // Set new options
        analyzer.setOptions( newOptions );
        rcUNITTEST_ASSERT( origOptions != analyzer.getOptions() );
    }
    
    uint32 newWindowSize = analyzer.getOptions().windowSize();
    
    // We should get the oldWindowSize frame now
    status = analyzer.getNextResult( result, true );
    rcUNITTEST_ASSERT( status == eFrameStatusOK );
    rcUNITTEST_ASSERT( analyzer.getLastError() == eFrameErrorOK );
    rcUNITTEST_ASSERT( result.frameBuf() == imageVector9[oldWindowSize].frameBuf() );
    // @note for now
    //    rcUNITTEST_ASSERT( result.entropy() != -1.0 );

    // Check remaining valid results
    for( uint32 i = oldWindowSize+1; i < imageVector9.size() - newWindowSize + 1; i++ ) {
        status = analyzer.getNextResult( result, true );
        rcUNITTEST_ASSERT( status == eFrameStatusOK );
        rcUNITTEST_ASSERT( analyzer.getLastError() == eFrameErrorOK );
        rcUNITTEST_ASSERT( result.frameBuf() == imageVector9[i].frameBuf() );
    // @note for now
        // rcUNITTEST_ASSERT( result.entropy() != -1.0 );
    }
    // Remaining entropies should be undefined
    uint32 i = imageVector9.size() - newWindowSize + 1;
    while( status != eFrameStatusEOF && status != eFrameStatusError ) {
        status = analyzer.getNextResult( result, true );
        rcUNITTEST_ASSERT( status != eFrameStatusError );
        rcUNITTEST_ASSERT( analyzer.getLastError() == eFrameErrorOK );
        if ( status != eFrameStatusEOF )
            rcUNITTEST_ASSERT( result.frameBuf() == imageVector9[i++].frameBuf() );
        rcUNITTEST_ASSERT( result.entropy() == -1.0 );
    }
}
