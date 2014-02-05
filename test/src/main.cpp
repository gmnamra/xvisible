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

TEST (UT_1dp, run)
{
    UT_1dp test;
    EXPECT_EQ (0, test.run());
}


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



int main(int argc, char **argv)
{
        //	std::cout << " Build Version " << SVN_VERSION << std::endl;
	testing::InitGoogleTest(&argc, argv);
	int rtn_val = RUN_ALL_TESTS();
        //	std::cout << " Build Version " << SVN_VERSION << std::endl;
	
	
}// TEST(GTestMainTest, ShouldSucceed) { }






