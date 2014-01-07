/*
 *  rc_main.cpp
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *  Generate simple synthetic test movies
 *
 */

#include <rc_types.h>
#include <rc_videocache.h>
#include <rc_framebuf.h>
#include <rc_moviegrabber.h>
#include <rc_gen_movie_file.h>
#include <rc_utdrawutils.h>
#include <rc_ip.h>

// Application info
const char* const cAppName = "movie-generator";
const char* const cVersionName = "1.0";
const char* const cAppBuildDate = __DATE__ " " __TIME__ ;
// Creator name
static std::string cCreator;

// Verbosity
static bool verbose = true;

int printUsage()
{
    fprintf(stderr, "Usage: movie-generator \n"
                    "      -i <incr>     object center increment in pixels/frame (default:1)\n"
                    "      -r <incr>     object radius increment in pixels/frame (default:1)\n"
                    "      -s            perform gaussian smoothing and sampling (default:no)\n"
                    "      -n            number of frames to generate (default:50)\n"
            "\n");

    return 1;
}

// Generate experiment (assay) header for movie
static rcMovieFileExpExt generateHeader()
{
    // Generate an experiment header
    rcMovieFileExpExt expHdr;
    expHdr.lensMag( 1.0f );
    expHdr.otherMag( 0.0f );
    expHdr.temperature( 21.0f );
    expHdr.CO2( 0.035f );
    expHdr.O2( 21.0f );
    expHdr.userName( "Dr. Tester" );
    expHdr.cellType( "Synthetic Cell" );
    expHdr.imagingMode( "Synthetic" );

    return expHdr;
}

// Generate simulated cell translation movie
static void generateTranslationMovie( const std::string& fileName,
                                      const int& radius,
                                      const float& xIncr,
                                      const float& yIncr,
                                      const int32& frames,
                                      bool smooth = false )
{
    const movieFormatRev rev = movieFormatRevLatest;
    const movieOriginType origin = movieOriginSynthetic;
    // Add an experiment header
    rcMovieFileExpExt expHdr = generateHeader();
    expHdr.title( "Cell Translation" );
    char buf[512];
    snprintf( buf, rmDim(buf), "Translation xInc:%.2f yInc:%.2f r:%i",
              xIncr, yIncr, radius );
    expHdr.comment( buf );
    
    rcGenMovieFileError error = eGenMovieFileErrorOK;
    
    rcGenMovieFile movie(fileName, origin, cCreator,
                         rev, true, 1.0f);
    rmAssert( movie.valid() );

    uint8 bkgColor = 138;
    uint8 objColor = 0;

    int w = 128;
    int h = 128;
    int r = radius;
    
    if ( smooth ) {
        w *= 2;
        h *= 2;
        r *= 2;
    }
    
    if ( rmABS(xIncr)*frames+r*2 >= w/2 )
        w = int(2*(rmABS(xIncr)*frames+r*2) + 1);
    if ( rmABS(yIncr)*frames+r*2 >= h/2 )
        h = int(2*(rmABS(yIncr)*frames+r*2) + 1);
    rcWindow model(w,h);
    rcWindow smoothed(w,h);
    model.setAllPixels(bkgColor);

    if ( verbose ) {
        cerr << "Generating " << fileName <<  " xIncrement " << xIncr
             << " yIncrement " << yIncr 
             << " w " << w << " h " << h << " r " << r << endl;
    }
    
    int x, y;
    x = model.width()/2;
    y = model.height()/2;

    for ( int i = 0; i < frames; ++i ) {
        rc2Dvector origin(x+(i*xIncr), y+(i*yIncr) );
        rfDrawCircle(origin, r, model, objColor, bkgColor);
        if ( smooth ) {
            rfGaussianConv(model, smoothed, 3);
            error = movie.addFrame(rfPixelSample(smoothed, 2, 2));
        } else
            error = movie.addFrame( model );
        rmAssert( error == eGenMovieFileErrorOK );
    }
    // Add header and flush
    movie.addHeader( expHdr );
    error = movie.flush();
    rmAssert( error == eGenMovieFileErrorOK );
}

// Generate simulated cell expansion movie
static void generateExpansionMovie( const std::string& fileName,
                                    const int& radius,
                                    const float& rIncr,
                                    const int32& frames,
                                    bool smooth = false )
{
    const movieFormatRev rev = movieFormatRevLatest;
    const movieOriginType origin = movieOriginSynthetic;  
    rcGenMovieFileError error = eGenMovieFileErrorOK;
    // Add an experiment header
    rcMovieFileExpExt expHdr = generateHeader();
    expHdr.title( "Cell Expansion" );
    char buf[512];
    snprintf( buf, rmDim(buf), "Expansion rInc:%.2f r:%i",
              rIncr, radius );
    expHdr.comment( buf );
    
    rcGenMovieFile movie(fileName, origin, cCreator,
                         rev, true, 1.0f);
    rmAssert( movie.valid() );

    uint8 bkgColor = 138;
    uint8 objColor = 0;
    int w = 64;
    int h = 64;
    int r = radius;
  
    if ( rmABS(rIncr*2)*frames > w ) {
        w = int((rmABS(rIncr*2)*frames) + 1);
        w = h;
    }
    if ( verbose ) {
        cerr << "Generating " << fileName << " rIncrement " << rIncr
             << " w " << w << " h " << h << endl;
    }
       
    if ( smooth ) {
        w *= 2;
        h *= 2;
        r *= 2;
    }
    rcWindow model(w,h);
    rcWindow smoothed(w,h);
    model.setAllPixels(bkgColor);
   
    int x, y;
    x = model.width()/2;
    y = model.height()/2;
    
    for ( int i = 0; i < frames; ++i ) {
        rc2Dvector origin(x, y);
        rfDrawCircle(origin, int(r+i*rIncr), model, objColor, bkgColor);
        if ( smooth ) {
            rfGaussianConv(model, smoothed, 3);
            error = movie.addFrame(rfPixelSample(smoothed, 2, 2 ));
        } else
            error = movie.addFrame( model );
        rmAssert( error == eGenMovieFileErrorOK );
    }
    // Add header and flush
    movie.addHeader( expHdr );
    error = movie.flush();
    rmAssert( error == eGenMovieFileErrorOK );
}

// Generate simulated cell random walk movie
static void generateRandomMovie( const std::string& fileName,
                                 const int& radius,
                                 const float& xIncr,
                                 const float& yIncr,
                                 const int32& frames,
                                 bool smooth = false )
{
    const movieFormatRev rev = movieFormatRevLatest;
    const movieOriginType org = movieOriginSynthetic;
    // Add an experiment header
    rcMovieFileExpExt expHdr = generateHeader();
    expHdr.title( "Cell Random Walk" );
    char buf[512];
    snprintf( buf, rmDim(buf), "Random translation xInc:%.2f yInc:%.2f r:%i",
              xIncr, yIncr, radius );
    expHdr.comment( buf );
    
    rcGenMovieFileError error = eGenMovieFileErrorOK;
    
    rcGenMovieFile movie(fileName, org, cCreator,
                         rev, true, 1/30.0f);
    rmAssert( movie.valid() );

    uint8 bkgColor = 138;
    uint8 objColor = 0;
    int w = 64;
    int h = 64;
    int r = radius;

    if ( smooth ) {
        w *= 2;
        h *= 2;
        r *= 2;
    }
    if ( rmABS(xIncr)*frames+r*2 >= w/2 )
        w = int(2*(rmABS(xIncr)*frames+r*2) + 1);
    if ( rmABS(yIncr)*frames+r*2 >= h/2 )
        h = int(2*(rmABS(yIncr)*frames+r*2) + 1);
    // Double size for sampling later
    w += 2;
    h += 2;
    rcWindow model(w,h);
    rcWindow smoothed(w,h);
    model.setAllPixels(bkgColor);

    if ( verbose ) {
        cerr << "Generating " << fileName <<  " xIncrement " << xIncr
             << " yIncrement " << yIncr 
             << " w " << w << " h " << h << " r " << r << endl;
    }
    
    int x, y;
    x = model.width()/2;
    y = model.height()/2;

    rc2Dvector origin(x,y);
    
    for ( int i = 0; i < frames; ++i ) {
        int doX = uint8(random())/85-1;
        int doY = uint8(random())/85-1;
        if ( doX )
            origin.x( origin.x() + doX*xIncr );
        if ( doY )
            origin.y( origin.y() + doX*yIncr );
        rfDrawCircle(origin, r, model, objColor, bkgColor);
        if ( smooth ) {
            rfGaussianConv (model, smoothed, 3);
            error = movie.addFrame(rfPixelSample( smoothed, 2, 2 ));
        } else
            error = movie.addFrame(model);
        rmAssert( error == eGenMovieFileErrorOK );
    }
    // Add header and flush
    movie.addHeader( expHdr );
    error = movie.flush();
    rmAssert( error == eGenMovieFileErrorOK );
}

// Generate a single pixel walk movie
static void generateTinyMovie( const std::string& fileName,
                               const int& radius,
                               const float& xIncr,
                               const float& yIncr,
                               const int32& frames,
                               bool smooth = false )
{
    const movieFormatRev rev = movieFormatRevLatest;
    const movieOriginType org = movieOriginSynthetic;
    // Add an experiment header
    rcMovieFileExpExt expHdr = generateHeader();
    expHdr.title( "Tiny Cell Edge Walk" );
    char buf[512];
    snprintf( buf, rmDim(buf), "Translation xInc:%.2f yInc:%.2f r:%i",
              xIncr, yIncr, radius );
    expHdr.comment( buf );
    
    rcGenMovieFileError error = eGenMovieFileErrorOK;
    
    rcGenMovieFile movie(fileName, org, cCreator,
                         rev, true, 1/30.0f);
    rmAssert( movie.valid() );

    uint8 bkgColor = 138;
    uint8 objColor = 0;
    int w = 16;
    int h = 16;
    int r = radius;

    if ( smooth ) {
        w *= 2;
        h *= 2;
        r *= 2;
    }

    rcWindow model(w,h);
    model.setAllPixels(bkgColor);

    if ( verbose ) {
        cerr << "Generating " << fileName <<  " xIncrement " << xIncr
             << " yIncrement " << yIncr 
             << " w " << w << " h " << h << " r " << r << endl;
    }
    
    int x, y;
    x = 1;
    y = 1;

    rc2Dvector origin(x,y);
    
    for ( int i = 0; i < frames; ) {
        for ( x = x; x < w; x += int(xIncr) ) {
            if ( i >= frames )
                break;
            origin.x( x );
            rfDrawCircle(origin, r, model, objColor, bkgColor);
            error = movie.addFrame(model);
            rmAssert( error == eGenMovieFileErrorOK );
            ++i;
        }
        for ( y = y+1; y < h; y += int(yIncr) ) {
            if ( i >= frames )
                break;
            origin.y( float(y) );
            rfDrawCircle(origin, r, model, objColor, bkgColor);
            error = movie.addFrame(model);
            rmAssert( error == eGenMovieFileErrorOK );
            ++i;
        }
        for ( x = x-2; x > 0; x -= int(xIncr) ) {
            if ( i >= frames )
                break;
            origin.x( x );
            rfDrawCircle(origin, r, model, objColor, bkgColor);
            error = movie.addFrame(model);
            rmAssert( error == eGenMovieFileErrorOK );
            ++i;
        }
        for ( y = y-2; y > 0; y -= int(yIncr) ) {
            if ( i >= frames )
                break;
            origin.y( y );
            rfDrawCircle(origin, r, model, objColor, bkgColor);
            error = movie.addFrame(model);
            rmAssert( error == eGenMovieFileErrorOK );
            ++i;
        }
    }
    // Add header and flush
    movie.addHeader( expHdr );
    error = movie.flush();
    rmAssert( error == eGenMovieFileErrorOK );
}

int main(int argc, char** argv)
{
    float increment = 1.0f;
    float rIncrement = 1.0f;
    int32 frames = 50;
    bool smooth = false;
    
    // Creator name
    cCreator = cAppName;
    cCreator += " ";
    cCreator += cVersionName;
    cCreator += " ";
    cCreator += cAppBuildDate;
    
    for (int i = 1; i < argc; i++) {
        if (*(argv[i]) != '-')
            return printUsage();
        char cmdType = *(argv[i]+1);
        
        switch (cmdType) {
            case 'i':
                {
                    char* incrStr = argv[++i];
                    int lth = strlen(incrStr);
                    if ( lth )
                        increment = atof( incrStr );
                }
                break;
            case 'r':
                {
                    char* incrStr = argv[++i];
                    int lth = strlen(incrStr);
                    if ( lth )
                        rIncrement = atof( incrStr );
                }
                break;
                
            case 's':
                smooth = true;
                break;
                
            case 'n':
            {
                char* str = argv[++i];
                int lth = strlen(str);
                if ( lth )
                    frames = atoi( str );
            }
            break;
                
            default:
                return printUsage();
        } // End of: switch(*(argv[i] + 1))
    } // End of: for (int i = 1; i < argc; i++)

    int r = 8; // Circle radius

    // Translation movies
    generateTranslationMovie( std::string("circle-move-left-to-right.rfymov"),
                              r, increment, 0.0f, frames, smooth );
    generateTranslationMovie( std::string("circle-move-right-to-left.rfymov"),
                              r, -increment, 0.0f, frames, smooth );
    generateTranslationMovie( std::string("circle-move-top-to-bottom.rfymov"),
                              r, 0.0f, increment, frames, smooth );
    generateTranslationMovie( std::string("circle-move-bottom-to-top.rfymov"),
                              r, 0.0f, -increment, frames, smooth );
    generateTranslationMovie( std::string("circle-move-top-to-bottom-diagonally.rfymov"),
                              r, increment, increment, frames, smooth );
    generateTranslationMovie( std::string("circle-move-bottom-to-top-diagonally.rfymov"),
                              r, -increment, -increment, frames, smooth );
    generateTranslationMovie( std::string("circle-move-top-to-bottom-diagonally2.rfymov"),
                              r, -increment, increment, frames, smooth );
    generateTranslationMovie( std::string("circle-move-bottom-to-top-diagonally2.rfymov"),
                              r, increment, -increment, frames, smooth );
    // Random movies
    generateRandomMovie( std::string("circle-move-random-walk.rfymov"),
                         r, increment, increment, frames, smooth );
#if 1    
    generateTinyMovie( std::string("tiny-circle-move-around.rfymov"),
                       1, increment, increment, frames, false );
#endif    
    // Expansion movies
    generateExpansionMovie( std::string("circle-expand.rfymov"),
                            1, rIncrement, frames, smooth );
    generateExpansionMovie( std::string("circle-shrink.rfymov"),
                            50, -rIncrement, frames, smooth );
    return 0;
}
