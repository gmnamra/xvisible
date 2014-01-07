/*
 *  main.cpp
 *  zoubin
 *
 *  Created by Arman Garakani on 9/21/08.
 *  Copyright 2008 Reify Corporation. All rights reserved.
 *
 */




#include <Carbon/Carbon.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include "main.h"
#include <gtest/gtest.h>



std::string gTestData ("/Users/arman/WorkingCopies/test-content/");


#if 0

TEST(UT_RLE, run)
{
	UT_RLE test;
	EXPECT_EQ(0, test.run());
}

// rcSimilarator tests
TEST(UT_similarity, run)
{
  UT_similarity test;
  EXPECT_EQ(0, test.run());
}
TEST(UT_moreip, run)
{
  UT_moreip test;
  EXPECT_EQ(0, test.run());
}
TEST(UT_RLE, run)
{
  UT_RLE test;
  EXPECT_EQ(0, test.run());
}
// Pyramid Processing tests
TEST(UT_pyramid, run)
{
  UT_pyramid test;
  EXPECT_EQ(0, test.run());
}
// Correlation tests
TEST(UT_Correlation, run)
{
  UT_Correlation test;
  EXPECT_EQ(0, test.run());
}
// Image Set Registration Tests
TEST(UT_register, run)
{
  UT_register test;
  EXPECT_EQ(0, test.run());
}
// Moment generating tests
TEST(UT_moments, run)
{
  UT_moments test;
  EXPECT_EQ(0, test.run());
}
// rcCorrelationWindow tests
TEST(UT_Correlationwindow, run)
{
  UT_Correlationwindow test;
  EXPECT_EQ(0, test.run());
}
// Numerical Receipe tests
TEST(UT_nr , run)
{
  UT_nr test;
  EXPECT_EQ(0, test.run());
}
// Point Correlation tests
TEST(UT_point_corr , run)
{
  UT_point_corr test;
  EXPECT_EQ(0, test.run());
}

// Connectivity tests
TEST(UT_Connect , run)
{
  UT_Connect test;
  EXPECT_EQ(0, test.run());
}

// Distance Clustering tests
TEST( UT_Blum , run)
{
  UT_Blum test;
  EXPECT_EQ(0, test.run());
}

// rcAnalyzer tests
TEST( UT_Analyzer , run)
{
  UT_Analyzer test;
  EXPECT_EQ(0, test.run());
}
// Shape Calculator tests
TEST(  UT_shape , run)
{
  UT_shape test;
  EXPECT_EQ(0, test.run());
}



TEST(  UT_motion , run)
{
  std::string movieName = ("kine-ut-testmovie.rfymov");
  rmAssert(movieName.length());
  movieName = gTestData + movieName;
  UT_motion test (movieName);
  EXPECT_EQ(0, test.run());
}

#endif

#if 0
TEST(UT_moreip, run)
{
	UT_moreip test;
	EXPECT_EQ(0, test.run());
}

// Stats test
TEST ( UT_stats, run )
{
  UT_stats test;
  EXPECT_EQ(0,  test.run());
}

// System info tests
TEST ( UT_Systeminfo, run )
{
  UT_Systeminfo test;
  EXPECT_EQ(0,  test.run());
}
// Line segment tests
TEST ( UT_line, run )
{
  UT_line test;
  EXPECT_EQ(0,  test.run());
}

// rcSparseHistogram tests
//TEST ( UT_SparseHistogram, run )
//{
//  UT_SparseHistogram test;
//  EXPECT_EQ(0,  test.run());
//}

// atomicValue template tests
TEST ( UT_AtomicValue, run )
{
  UT_AtomicValue test;
  EXPECT_EQ(0,  test.run());
}

// Thread tests
TEST ( UT_Thread, run )
{
  UT_Thread test;
  EXPECT_EQ(0,  test.run());
}

// ringBuffer tests
TEST ( UT_RingBuffer, run )
{
  UT_RingBuffer test;
  EXPECT_EQ(0,  test.run());
}

// bidirectionalRing tests
TEST ( UT_BidirectionalRing, run )
{
  UT_BidirectionalRing test;
  EXPECT_EQ(0,  test.run());
}

// pair / rect tests
TEST ( UT_pair , run )
{
  UT_pair test;
  EXPECT_EQ(0,  test.run());
}

TEST ( UT_rect, run )
{
  UT_rect test;
  EXPECT_EQ(0,  test.run());
}


TEST ( UT_FrameBuf, run )
{
	UT_FrameBuf test;
	EXPECT_EQ(0, test.run () );
}

rcSparseHistogram tests
TEST ( UT_SparseHistogram, run )
{
  UT_SparseHistogram test;
  EXPECT_EQ(0,  test.run());
}

TEST (UT_moreip, run )
{
	UT_moreip test;
	EXPECT_EQ(0, test.run() );
}

// rcCorrelationWindow tests
TEST(UT_Correlationwindow, run)
{
	UT_Correlationwindow test;
	EXPECT_EQ(0, test.run());
}

	// Image Set Registration Tests
TEST(UT_register, run)
{
	UT_register test;
	EXPECT_EQ(0, test.run());
}


#endif

#if 1



TEST( UT_WindowMutator, run )
{
	UT_WindowMutator test;
	EXPECT_EQ (0, test.run() );
}


TEST( UT_Window, run )
{
	UT_Window test;
	EXPECT_EQ(0, test.run());
}


	// Correlation tests
TEST(UT_Correlation, run)
{
	UT_Correlation test;
	EXPECT_EQ(0, test.run());
}


TEST(UT_similarity, run)
{
	UT_similarity test;
	EXPECT_EQ(0, test.run());
}


// Stats test
TEST ( UT_stats, run )
{
  UT_stats test;
  EXPECT_EQ(0,  test.run());
}

// System info tests
TEST ( UT_Systeminfo, run )
{
  UT_Systeminfo test;
  EXPECT_EQ(0,  test.run());
}
// Line segment tests
TEST ( UT_line, run )
{
  UT_line test;
  EXPECT_EQ(0,  test.run());
}

// Fixed Point Cordic tests
TEST ( UT_cordic, run )
{
  UT_cordic test;
  EXPECT_EQ(0,  test.run());
}


// security code tests
TEST ( UT_Security, run )
{
  UT_Security test;
  EXPECT_EQ(0,  test.run());
}

// rcTimestamp tests
TEST ( UT_Timestamp, run )
{
  UT_Timestamp test;
  EXPECT_EQ(0,  test.run());
}


// Thread tests
TEST ( UT_Thread, run )
{
  UT_Thread test;
  EXPECT_EQ(0,  test.run());
}



// atomicValue template tests
TEST ( UT_AtomicValue, run )
{
  UT_AtomicValue test;
  EXPECT_EQ(0,  test.run());
}


// ringBuffer tests
TEST ( UT_RingBuffer, run )
{
  UT_RingBuffer test;
  EXPECT_EQ(0,  test.run());
}

// bidirectionalRing tests
TEST ( UT_BidirectionalRing, run )
{
  UT_BidirectionalRing test;
  EXPECT_EQ(0,  test.run());
}
	// Pyramid Processing tests
TEST(UT_pyramid, run)
{
	UT_pyramid test;
	EXPECT_EQ(0, test.run());
}

	// Moment generating tests
TEST(UT_moments, run)
{
	UT_moments test;
	EXPECT_EQ(0, test.run());
}


#endif

int main(int argc, char **argv)
{
	std::cout << " Build Version " << SVN_VERSION << std::endl;
	testing::InitGoogleTest(&argc, argv);
	int rtn_val = RUN_ALL_TESTS();
	std::cout << " Build Version " << SVN_VERSION << std::endl;	
	
	
}// TEST(GTestMainTest, ShouldSucceed) { }






