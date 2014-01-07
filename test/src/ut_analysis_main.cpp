/*
 *  ut_analysis_main.cpp
 *  vf
 *
 *  Created by arman on 1/22/09.
 *  Copyright 2009 Reify Corporation. All rights reserved.
 *
 */

// Add a test class for correlation methods. Include synthetic image productio
#include <stlplus.hpp>
#include "ut_analysis_main.h"
#include <limits.h>
#include <gtest/gtest.h>
#define  UT_KINETOSCOPE


#define rmPrintImage(a){						\
for (int32 i__temp = 0; i__temp < (a).height(); i__temp++) {	\
fprintf (stderr, "\n");						\
for (int32 j__temp = 0; j__temp < (a).width(); j__temp++)	\
fprintf (stderr, "%3d ", (a).getPixel (j__temp, i__temp));	\
fprintf (stderr, "\n");						\
}}

/*
 *  Utility Functions for grabbing movieFile name
 */

static string absoluteMovieName( std::string& resourcePath, const std::string movieName )
{
	return resourcePath + movieName;
}


int ut_analysis (std::string& resourcePath )
{
	
	uint32 errors = 0;
	
	try
    {


#ifdef UT_KINETOSCOPE
		// motion tracking tests
		// Grabbing absolute test movie path (thanks Peter)
		// Need to make this cleaner
		{
			std::string movieName = ("kine-ut-testmovie.rfymov");
			rmAssert(movieName.length());
			movieName = resourcePath + movieName;
			UT_motion test (movieName);
			errors += test.run();
		}
#endif
		// rcSimilarator tests
		{
			UT_similarity test;
			errors += test.run();
		}
		{
			UT_moreip test;
			errors += test.run();
		}
		
			
		{
			UT_RLE test;
			errors += test.run();
		}
		
		
		// Pyramid Processing tests
		{
			UT_pyramid test;
			errors += test.run();
		}
		
		
		
		// Correlation tests
		{
			UT_Correlation test;
			errors += test.run();
		}
		
		
		
		// Image Set Registration Tests
		{
			UT_register test;
			errors += test.run();
		}
		
		
		// Moment generating tests
		{
			UT_moments test;
			errors += test.run();
		}
		// rcCorrelationWindow tests
		{
			UT_Correlationwindow test;
			errors += test.run();
		}
		
#ifdef UT_KINEIMAGE

		// Kinetic Image tests
		{
			char* appName = argv[0];
			rmAssert(appName);
			
			int i = strlen(appName) - 1;
			
			while ((i >= 0) && (appName[i] != '/'))
				i--;
			
			char* movieLocation = "../../../../src/ut/gen_cmd.txt";
			int movieNameLth = i+2+strlen(movieLocation);
			char* movieName = (char*)malloc(movieNameLth);
			rmAssert(movieName);
			
			if (i >= 0)
				strncpy(movieName, appName, i+1);
			
			strcpy(&movieName[i+1], movieLocation);
			movieName[movieNameLth-1] = 0;
			
			UT_kineticimage test(movieName);
			errors += test.run();
			
			free(movieName);
#ifdef RUN_GEN
			return 0;
#endif
		}
#endif
		
		// Numerical Receipe tests
		{
			UT_nr test;
			errors += test.run();
		}
		// Point Correlation tests
		{
			UT_point_corr test;
			errors += test.run();
		}
		
		// Connectivity tests
		{
			UT_Connect test;
			errors += test.run();
		}
		
		// Distance Clustering tests
		{
			UT_Blum test;
			errors += test.run();
		}
		
		// rcAnalyzer tests
		{
			UT_Analyzer test;
			errors += test.run();
		}
		// Shape Calculator tests
		{
			UT_shape test;
			errors += test.run();
		}
		

		
	}
	catch ( exception& e ) {
		fprintf(stderr, "exception \"%s\" thrown\n",  e.what() );
		++errors;
	}
	catch (...) {
		fprintf(stderr, "Unknown exception thrown\n");
		++errors;
	}
	
	return errors;
}
