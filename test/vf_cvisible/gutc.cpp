/*
 *  main.cpp
 *  zoubin
 *
 *  Created by Arman Garakani on 9/21/08.
 *  Copyright 2008 Reify Corporation. All rights reserved.
 *
 */




#include <iostream>
#include <string>
#include <stlplus_lite.hpp>
#include <unistd.h>
#include "ut_file.hpp"
#include <gtest/gtest.h>

class genv: public testing::Environment
{
public:
    genv (std::string exec_path) : m_exec_path (exec_path)
    {
    }

    const std::string& root_folder () { return m_root_path; } 
    const std::string& test_folder () { return m_test_path; } 
    const std::string& test_data_folder () { return m_test_content_path; }     
    
    void SetUp ()
    {
        static std::string test_folder_name ("test");
        static std::string test_data_folder_name ("test_data");
        
        folder_set_current (m_exec_path);
        do
        {
            folder_set_current (folder_up (folder_current_full ()));
        }
        while (! is_folder (folder_append_separator (folder_current_full () ) + test_folder_name ) );  
        
        m_root_path = folder_current_full ();
        m_test_path = folder_append_separator (folder_current_full () ) + test_folder_name;
        m_test_content_path = folder_append_separator ( test_folder () ) + test_data_folder_name;

    }
    
    void TearDown () {}
private:
    std::string m_root_path;
    std::string m_test_content_path;
    std::string m_test_path;    
    std::string m_exec_path;
};


static ::testing::Environment* envp = 0;


TEST (UT_fileutils, run)
{
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    
    UT_fileutils test (gvp->test_data_folder ());
    EXPECT_EQ(0, test.run () );
}

#if 0
TEST( UT_FrameBuf, run )
{
	UT_FrameBuf test;
	EXPECT_EQ(0, test.run());
}



TEST (UT_movieconverter, run)
{
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string qmov_name ("box-move.mov");
    static std::string rfymov_name ("box-move.rfymov");
    std::string qmov = create_filespec (gvp->test_data_folder (), qmov_name);
    std::string rfymov = create_filespec (gvp->test_data_folder (), rfymov_name);    
    
    UT_movieconverter test (rfymov.c_str(), qmov.c_str () );
    
    EXPECT_EQ (0, test.run());
}


TEST ( UT_ReifyMovieGrabber, run )
{
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string rfymov_name ("rev2.rfymov");
    std::string rfymov = create_filespec (gvp->test_data_folder (), rfymov_name);    
    
    UT_ReifyMovieGrabber test ( rfymov );
    EXPECT_EQ(0, test.run () );
}


TEST (UT_videocache, run)
{
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string rfymov_name ("rev2.rfymov");
    std::string rfymov = create_filespec (gvp->test_data_folder (), rfymov_name);    
    
    UT_VideoCache test (rfymov);
    EXPECT_EQ(0, test.run () );
}



TEST(UT_similarity_producer, run)
{
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string qmov_name ("box-move.mov");
    static std::string rfymov_name ("box-move.rfymov");
    std::string qmov = create_filespec (gvp->test_data_folder (), qmov_name);
    std::string rfymov = create_filespec (gvp->test_data_folder (), rfymov_name);    
    
	UT_similarity_producer test (rfymov, qmov);
    EXPECT_EQ(0, test.run());
}


#endif


int main(int argc, char **argv)
{
    std::string installpath = install_path (std::string (argv[0]));
    ::testing::Environment* const g_env  = ::testing::AddGlobalTestEnvironment(new     genv (installpath) );
    envp = g_env;
	testing::InitGoogleTest(&argc, argv);
	int rtn_val = RUN_ALL_TESTS();

	
	
}// TEST(GTestMainTest, ShouldSucceed) { }





