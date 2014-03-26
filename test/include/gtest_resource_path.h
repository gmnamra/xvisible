/*
 *  gtest_resource_path.h
 *  utviewer
 *
 *  Created by arman on 2/16/09.
 *  Copyright 2009 Reify Corporation. All rights reserved.
 *
 */


#incude <string>
#include <gtest/gtest.h>

std::string __gTestData ("/Users/arman/WorkingCopies/test-content/");

#ifdef GTEST_HAS_PARAM_TEST

using ::testing::TestWithParam;
using ::testing::Values;


// As a general rule, tested objects should not be reused between tests.
// Also, their constructors and destructors of tested objects can have
// side effects. Thus you should create and destroy them for each test.
// In this sample we will define a simple factory function for PrimeTable
// objects. We will instantiate objects in test's SetUp() method and
// delete them in TearDown() method.

// Inside the test body, fixture constructor, SetUp(), and TearDown()
// you can refer to the test parameter by GetParam().
// In this case, the test parameter is a PrimeTableFactory interface pointer
// which we use in fixture's SetUp() to create and store an instance of
// PrimeTable.
class testPack 
	{   public:
		testPack (rcUnitTest& ut, std::string _str = std::string ()) : _ut (ut), _resourcePath (_str) {}
		rcUnitTest	_ut;
		std::string _resourcePath;
	};

typedef testPack* getTestPack ();
testPack* GetOnTheFlyut () {
	return new testPack (__gTestData); 
}

class rcGTest : public TestWithParam<getut*> {
public:
	virtual ~rcGTest() {  delete pathStr_; }
	virtual void SetUp() { pathStr_ = (*GetParam())(); }
	virtual void TearDown() {
		delete pathStr_; pathStr_ = NULL;
	}
	
protected:
	testPack* pathStr_;
};


TEST_P(rcGTest, analysis_tests)
{
	EXPECT_true(