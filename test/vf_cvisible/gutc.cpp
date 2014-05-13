



#include <iostream>
#include <string>
#include <stlplus_lite.hpp>
#include <unistd.h>
#include <gtest/gtest.h>
#include "ut_file.hpp"
#include "ut_framebuf.h"
#include "ut_similarity.h"
#include "ut_videocache.h"
#include "cinder/audio2/Source.h"
#include "cvisible/vf_cinder.hpp"

using namespace ci;
using namespace std;

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

        fs::path fs_test_content_path (m_test_content_path);
        fs::directory_iterator itr(fs_test_content_path); fs::directory_iterator end_itr;
        std::cout << fs_test_content_path.root_path() << std::endl; std::cout << fs_test_content_path.parent_path() << std::endl;
        while(itr!=end_itr && !fs::is_directory(itr->path())){
            std::cout << "-----------------------------------"<< std::endl;
            std::cout << "Path: "<< itr->path()<< std::endl;
            std::cout << "Filename: "<< itr->path().filename()<< std::endl;
            std::cout << "Is File: "<< fs::is_regular_file(itr->path()) << std::endl; std::cout << "File Size :"<< fs::file_size(itr->path())<< std::endl; std::cout << "-----------------------------------"<< std::endl;
            itr ++;
        }
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
    
    //UT_fileutils test (gvp->test_data_folder ());
    //EXPECT_EQ(0, test.run () );

    std::string txtfile ("onecolumn.txt");
    std::string csv_filename = create_filespec (gvp->test_data_folder(), txtfile);
    
    fs::path fpath ( csv_filename );
    
    unique_ptr<SourceFile> sfr =  unique_ptr<SourceFile>( new vf_cinder::VisibleAudioSource (DataSourcePath::create (fpath )) );
    EXPECT_TRUE (sfr->getNumChannels() == 1);
    EXPECT_TRUE (sfr->getNumFrames () == 3296);
    
    std::string matfile ("matrix.txt");
    std::string mat_filename = create_filespec (gvp->test_data_folder(), matfile);
    vector<vector<float> > matrix;
    vf_utils::csv::csv2vectors(mat_filename, matrix, false, false, true);
    EXPECT_TRUE(matrix.size() == 300);
    for (int rr=0; rr < matrix.size(); rr++) 
        EXPECT_TRUE(matrix[rr].size() == 300);
    
    
    //    fs::path fpath ( csv_filename );

    
}

TEST( UT_FrameBuf, run )
{
	UT_FrameBuf test;
	EXPECT_EQ(0, test.run());
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


TEST (UT_videocache, run)
{
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string rfymov_name ("rev2.rfymov");
    std::string rfymov = create_filespec (gvp->test_data_folder (), rfymov_name);
    
    UT_VideoCache test (rfymov);
    EXPECT_EQ(0, test.run () );
}


#if 0

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
	return RUN_ALL_TESTS();

	
	
}// TEST(GTestMainTest, ShouldSucceed) { }






