// Copyright 2002-2004 Reify, Inc.

#include <stlplus.hpp>
#include <libgen.h>
#include <unistd.h>
#include "ut_visual_main.h"

#include <rc_mpeg2grabber.h>
#include <rc_fileutils.h>
#include <rc_window2jpg.h>


// Test one movie file
static int testMovie( const std::string& movieLocation, bool cacheTest );
// Test one mpeg movie
static int testmpeg (const char* appName);

static string absoluteMovieName( std::string& resourcepath, const std::string movieName);


int ut_visual (std::string& resourcePath )
{

  uint32 errors = 0;

  try {
    
	  // Image generating tests
	  {
		  std::string nnrot ("nearest_rot.rfymov");
		  std::string birot ("bilin_rot.rfymov");
		  std::string curot ("bicube_rot.rfymov");
		  
		  nnrot = absoluteMovieName (resourcePath, nnrot);
		  birot = absoluteMovieName (resourcePath, birot);
		  curot = absoluteMovieName (resourcePath, curot);		
		  UT_generator test(nnrot.c_str(), birot.c_str(), curot.c_str());
		  errors += test.run();
	  }


#if UT_JPG
    {
      // Test window2jpeg
      rcWindow test (33, 67);
      int32 i,j;
      for (j=0;j<test.height();j++)
	for (i=0;i<test.width();i++)
	  test.setPixel (i, j, floor ((i*j*255)/(33*67)));
      rfImageExport2JPG (test, std::string ("/Users/arman/tmp/export2jpgtest.jpg"));
    }
#endif 

#if UT_MPEG
    {
      errors += testmpeg (argv[0]); 
    }
#endif
	  
  
      // Image processing fct tests
      {
	UT_ImageProcessing test;
	errors += test.run();
      }

      // Edge Processing Routines
      {
	UT_Edge test;
	errors += test.run();
      }

      // Capture support code tests
      {
	UT_capture test;
	errors += test.run();
      }
      // Movie converter tests
      {																			  
	std::string qtMovieName ("box-move.mov");
	qtMovieName =  absoluteMovieName (resourcePath , qtMovieName);
	std::string rfyMovieName ("box-move.rfymov");
	rfyMovieName =  absoluteMovieName (resourcePath , rfyMovieName);
	UT_movieconverter test( rfyMovieName.c_str(), qtMovieName.c_str() );
      errors += test.run();
    }
    // Polygon tests
    {
      UT_Polygon test;
      errors += test.run();
    }
    // Scalable graphics tests
    {
      UT_Graphics test;
      errors += test.run();
    }
    // Rect tests
    {
      UT_Rect test;
      errors += test.run();
    }
    // Frame buffer tests
    {
      UT_FrameBuf test;
      errors += test.run();
    }
    // Window tests
    {
      UT_Window test;
      errors += test.run();
    }
    // Window Mutator tests
    {
      UT_WindowMutator test;
      errors += test.run();
    }
    // Window Histogram tests
    {
      UT_WindowHistogram test;
      errors += test.run();
    }
    // Video Cache and Reify Movie Grabber tests
    {
      {
	// Basic test
	std::string moviedat ("movie.dat");
	std::string movieName = absoluteMovieName( resourcePath,moviedat);
	errors += testMovie(movieName, true );
      }
      {
	// File revision 0 test
	std::string rev0rfymov ("rev0.rfymov");
	std::string movieName = absoluteMovieName( resourcePath,rev0rfymov);
	errors += testMovie(movieName, false );
      }
      {
	// File revision 1 test
	std::string rev1rfymov ("rev1.rfymov");
	std::string movieName = absoluteMovieName( resourcePath,rev1rfymov);
	errors += testMovie(movieName, false );
      }
      {
	// File revision 2 test
	std::string rev2rfymov ("rev2.rfymov");
	std::string movieName = absoluteMovieName( resourcePath,rev2rfymov);
	errors += testMovie(movieName, false );
      }
    }

    // Movie file format tests
    {
      UT_moviefileformat test;
      errors += test.run();
    }
     // Affine rectangle tests
    {
      UT_affine test;
      errors += test.run();
    }
    // Playback Utilities tests
    {
      UT_PlaybackUtils test;
      errors += test.run();
    }       
    // QuickTime tests
    {
      UT_Qtime test;
      errors += test.run(); 
    }
    // rcImageGrabber tests
    {
      UT_Imagegrabber test;
      errors += test.run();
    }
    // rcMovieGrabber tests
    {
      UT_Moviegrabber test;
      errors += test.run();
    }
	  	  
  }

  catch ( general_exception& e ) {
	  fprintf(stderr, "%s error: exception \"%s\" thrown\n", resourcePath.c_str(), e.what());
    ++errors;
  }
  catch (...) {
    fprintf(stderr, "%s error: Unknown exception thrown\n", resourcePath.c_str());
    ++errors;
  }

  return errors;
}

static int testmpeg (const char* appName)
{
	int errors = 0;

	std::string appNameStr (appName);
	std::string visualSrc = folder_up (appNameStr, 5);
	std::string mpeg2Extract = folder_up (visualSrc,1) + std::string ("mpeg2/bin/extract_m2v");
	std::string testMovie ("/src/ut/test-movies/a_movie.");
	std::string m2vName = visualSrc + testMovie + std::string ("m2v");
	std::string rfyName = visualSrc + testMovie + std::string ("rfymov");

	if (file_exists (m2vName)) file_delete (m2vName);
	if (file_exists (rfyName)) file_delete (rfyName);
	testMovie = visualSrc + testMovie + std::string ("mpg");

	std::string cmd = mpeg2Extract + std::string (" -s 0xe0 ") + testMovie + std::string (" > ") + m2vName;
	system( cmd.c_str() );	
	std::string whichMovie = m2vName;
	if (file_size (m2vName) == 0) 
	  whichMovie = testMovie;

	rcMPEG2Grabber mpegg (whichMovie, NULL);
	mpegg.start ();
	const rcChannelConversion conv = rcSelectAll;
	const movieFormatRev rev = movieFormatRevLatest;
	const float fInt = 1/30.0f;
	mpegg.getReifyMovie (rfyName, conv, rev, true, fInt);


	return errors;
}

/*
  *  Utility Functions for grabbing movieFile name
  */

static string absoluteMovieName( std::string& resourcePath, const std::string movieName )
{
	return resourcePath + movieName;
}



// Test one movie file
static int testMovie( const std::string& movieName,bool cacheTest )
{
  int errors = 0;

  // Video Cache and Reify Movie Grabber tests
  {
    if ( cacheTest ) 
      {
	UT_VideoCache test(movieName);
	errors += test.run();
      }

    {
      UT_ReifyMovieGrabber test(movieName);
      errors += test.run();
    }
  }
	
  return errors;
}
