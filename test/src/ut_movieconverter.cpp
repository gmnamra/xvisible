/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   $Id: ut_movieconverter.cpp 4391 2006-05-02 18:40:03Z armanmg $
 *
 ***************************************************************************/

#include <ut_movieconverter.h>

static const bool cDeleteOutputFiles = true;

// public

UT_movieconverter::UT_movieconverter( const char* rfyInputMovie,
                                      const char* QTInputMovie ) :
        mRfyInputMovie( rfyInputMovie ),
        mQTInputMovie( QTInputMovie )
{
    rcUNITTEST_ASSERT( rfyInputMovie != NULL );
    rcUNITTEST_ASSERT( QTInputMovie != NULL );
}

UT_movieconverter::~UT_movieconverter()
{
  printSuccessMessage( "Movie converter tests", mErrors );
}

uint32 UT_movieconverter::run()
{
    testOptions();
    testToRfy();
    testToQT();
    
    return mErrors;
}

// private

// Test all options classes
void UT_movieconverter::testOptions()
{
    {
        rcMovieConverterOptions opt;
        
        testOptions( opt );
    }

    {
        rcMovieConverterOptionsRfy opt;

        testOptions( opt );
                
        rcUNITTEST_ASSERT( opt.channelConversion() != rcSelectBlue );
        opt.channelConversion( rcSelectBlue );
        rcUNITTEST_ASSERT( opt.channelConversion() == rcSelectBlue );

        rcUNITTEST_ASSERT( opt.rev() != movieFormatRev1 );
        opt.rev( movieFormatRev1 );
        rcUNITTEST_ASSERT( opt.rev() == movieFormatRev1 );
    }

    {
        rcMovieConverterOptionsQT opt;

        testOptions( opt );
        //cerr << opt;
    }
    
}

// Test options base class
void UT_movieconverter::testOptions( rcMovieConverterOptions& opt )
{
     rcUNITTEST_ASSERT( opt.frameInterval() != 67.0 );
     opt.frameInterval( 67.0 );
     rcUNITTEST_ASSERT( opt.frameInterval() == 67.0 );
     
     rcUNITTEST_ASSERT( opt.firstFrameIndex() != 14 );
     opt.firstFrameIndex( 14 );
     rcUNITTEST_ASSERT( opt.firstFrameIndex() == 14 );
     
     rcUNITTEST_ASSERT( opt.frameCount() != 99 );
     opt.frameCount( 99 );
     rcUNITTEST_ASSERT( opt.frameCount() == 99 );
     
     rcUNITTEST_ASSERT( opt.frameOffset() != 17 );
     opt.frameOffset( 17 );
     rcUNITTEST_ASSERT( opt.frameOffset() == 17 );
     
     rcUNITTEST_ASSERT( opt.samplePeriod() != 3 );
     opt.samplePeriod( 3 );
     rcUNITTEST_ASSERT( opt.samplePeriod() == 3 );

     rcUNITTEST_ASSERT( opt.creator() != std::string("UT_movieconverter") );
     opt.creator(  std::string("UT_movieconverter") );
     rcUNITTEST_ASSERT( opt.creator() ==  std::string("UT_movieconverter") );
     
     rcUNITTEST_ASSERT( opt.overWrite() != false );
     opt.overWrite( false );
     rcUNITTEST_ASSERT( opt.overWrite() == false );

     rcUNITTEST_ASSERT( opt.reversePixels() != true );
     opt.reversePixels( true );
     rcUNITTEST_ASSERT( opt.reversePixels() == true );

     rcUNITTEST_ASSERT( opt.verbose() != false );
     opt.verbose( false );
     rcUNITTEST_ASSERT( opt.verbose() == false );
}

// Test to .rfymov conversion
void UT_movieconverter::testToRfy()
{
    {
        rcMovieConverterOptionsRfy opt;
        opt.verbose( false );
        opt.creator( "UT_movieconverter" );
        
        // Test with non-existent QT input movie
        {
            rcMovieConverterToRfy converter( opt );
            // This conversion MUST fail because input file doesn't exist
            std::string input("/tmp/rfy-non-existent-input-file666.mov");
            std::string output = makeTmpName("output.rfymov");
            rcMovieConverterError err = converter.convert( input, output );
            rcUNITTEST_ASSERT( err == eMovieConverterErrorOpen );
            if ( err != eMovieConverterErrorOpen ) {
                cerr << rcMovieConverter::getErrorString ( err ) << endl;
            }
            if ( cDeleteOutputFiles )
                system( std::string( "rm -f " + output ).c_str() );
        }
        // Test with non-existent rfy input movie
        {
            rcMovieConverterToRfy converter( opt );
            // This conversion MUST fail because input file doesn't exist
            std::string input("/tmp/rfy-non-existent-input-file666.rfymov");
            std::string output = makeTmpName("output.rfymov");
            rcMovieConverterError err = converter.convert( input, output );
            rcUNITTEST_ASSERT( err == eMovieConverterErrorOpen );
            if ( err != eMovieConverterErrorOpen ) {
                cerr << rcMovieConverter::getErrorString ( err ) << endl;
            }
            if ( cDeleteOutputFiles )
                system( std::string( "rm -f " + output ).c_str() );
        }

        opt.verbose( true );
        // Test with valid rfy input movie
        {
            opt.reversePixels( true );
            rcMovieConverterToRfy converter( opt );

            std::string output = makeTmpName("output-from-rfy-rev.rfymov");
            rcMovieConverterError err = converter.convert( mRfyInputMovie, output );
            rcUNITTEST_ASSERT( err == eMovieConverterErrorOK );
            if ( err != eMovieConverterErrorOK ) {
                cerr << rcMovieConverter::getErrorString ( err ) << " "
                     << mRfyInputMovie << endl;
            }
            if ( cDeleteOutputFiles )
                system( std::string( "rm -f " + output ).c_str() );
        }
        // Test with valid rfy input movie
        {
            opt.reversePixels( false );
            rcMovieConverterToRfy converter( opt );

            std::string output = makeTmpName("output-from-rfy.rfymov");
            rcMovieConverterError err = converter.convert( mRfyInputMovie, output );
            rcUNITTEST_ASSERT( err == eMovieConverterErrorOK );
            if ( err != eMovieConverterErrorOK ) {
                cerr << rcMovieConverter::getErrorString ( err ) << " "
                     << mRfyInputMovie << endl;
            }
            if ( cDeleteOutputFiles )
                system( std::string( "rm -f " + output ).c_str() );
        }
        // Test with valid rfy input movie, clip input
        {
            rcRect clipRect( 0, 0, 15, 15 );
            opt.reversePixels( false );
            opt.clipRect( clipRect );
            rcMovieConverterToRfy converter( opt );

            std::string output = makeTmpName("output-from-rfy-clip.rfymov");
            rcMovieConverterError err = converter.convert( mRfyInputMovie, output );
            rcUNITTEST_ASSERT( err == eMovieConverterErrorOK );
            if ( err != eMovieConverterErrorOK ) {
                cerr << rcMovieConverter::getErrorString ( err ) << " "
                     << mRfyInputMovie << endl;
            }
            if ( cDeleteOutputFiles )
                system( std::string( "rm -f " + output ).c_str() );
        }
        
        // Test with valid QT input movie
        {
            opt.reversePixels( false );
            opt.clipRect( rcRect() );
            rcMovieConverterToRfy converter( opt );

            std::string output = makeTmpName("output-from-qt.rfymov");
            rcMovieConverterError err = converter.convert( mQTInputMovie, output );
            rcUNITTEST_ASSERT( err == eMovieConverterErrorOK );
            if ( err != eMovieConverterErrorOK ) {
                cerr << rcMovieConverter::getErrorString ( err ) << " "
                     << mQTInputMovie << endl;
            }
            if ( cDeleteOutputFiles )
                system( std::string( "rm -f " + output ).c_str() );
        }        
    }
}
    
// Test to QT conversion
void UT_movieconverter::testToQT()
{
    {
        rcMovieConverterOptionsQT opt;
        opt.verbose( true );
        opt.creator( "UT_movieconverter" );
        
        // Test with valid rfy input movie
        {
            opt.reversePixels( true );
            rcMovieConverterToQT converter( opt );

            std::string output = makeTmpName("output-from-rfy-reversed.mov");
            rcMovieConverterError err = converter.convert( mRfyInputMovie, output );
            rcUNITTEST_ASSERT( err == eMovieConverterErrorOK );
            if ( err != eMovieConverterErrorOK ) {
                cerr << rcMovieConverter::getErrorString ( err ) << " "
                     << mRfyInputMovie << endl;
            }
            if ( cDeleteOutputFiles )
                system( std::string( "rm -f " + output ).c_str() );
        }

        // Test with valid rfy input movie
        {
            rcMovieConverterToQT converter( opt );
            opt.reversePixels( false );
            std::string output = makeTmpName("output-from-rfy.mov");
            rcMovieConverterError err = converter.convert( mRfyInputMovie, output );
            rcUNITTEST_ASSERT( err == eMovieConverterErrorOK );
            if ( err != eMovieConverterErrorOK ) {
                cerr << rcMovieConverter::getErrorString ( err ) << " "
                     << mRfyInputMovie << endl;
            }
            if ( cDeleteOutputFiles )
                system( std::string( "rm -f " + output ).c_str() );
        }
    }
}
