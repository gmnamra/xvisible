



#include <iostream>
#include <string>
#include <stlplus_lite.hpp>
#include <unistd.h>
#include <gtest/gtest.h>
#include "ut_file.hpp"
#include "ut_framebuf.h"
#include "ut_similarity.h"
#include "ut_videocache.h"
#include "ut_sm.hpp"
#include "cinder/audio2/Source.h"
#include "cvisible/qtime_cache.h"
#include "cvisible/vf_utils.hpp"
#include "cvisible/vf_cinder_qtime_grabber.h"
#include "vf_cinder_audio2.hpp"
#include "vf_sm_producer.h"
#include "ut_qtime.h"

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


TEST (UT_QtimeCache, run)
{
    
    // vf does not support QuickTime natively. The ut expectes and checks for failure
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string qmov_name ("box-move.mov");
    std::string qmov = create_filespec (gvp->test_data_folder (), qmov_name);
    
    UT_QtimeCache test (qmov);
    EXPECT_EQ(0, test.run () );
}

TEST( ut_similarity, run )
{
	UT_similarity test;
	EXPECT_EQ(0, test.run());
}




TEST(UT_similarity_producer, run)
{
    // vf does not support QuickTime natively. The ut expectes and checks for failure
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string qmov_name ("box-move.mov");
    static std::string rfymov_name ("box-move.rfymov");
    std::string qmov = create_filespec (gvp->test_data_folder (), qmov_name);
    std::string rfymov = create_filespec (gvp->test_data_folder (), rfymov_name);
    
	UT_similarity_producer test (rfymov, qmov);
    EXPECT_EQ(0, test.run());
}

TEST(cinder_qtime_grabber, run)
{
    // vf does not support QuickTime natively. The ut expectes and checks for failure
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string qmov_name ("box-move.mov");
    std::string qmov = create_filespec (gvp->test_data_folder (), qmov_name);
    boost::shared_ptr<rcFrameGrabber> grabber (reinterpret_cast<rcFrameGrabber*> (new vf_utils::qtime_support::CinderQtimeGrabber( qmov )) );
    
    EXPECT_TRUE (grabber.get() != NULL);
    
    rcFrameGrabberError error = grabber->getLastError();
    int i = 0;
    uint8 dummy;
    // Grab everything
    EXPECT_TRUE (grabber->start());
    EXPECT_TRUE (grabber->frameCount() == 56);
    std::vector<rcFrameRef> images;
    // Note: infinite loop
    for( i = 0; ; ++i )
    {
        rcTimestamp curTimeStamp;
        rcRect videoFrame;
        rcWindow image, tmp;
        rcFrameRef framePtr;
        rcFrameGrabberStatus status = grabber->getNextFrame( framePtr, true );
        EXPECT_TRUE((status == rcFrameGrabberStatus::eFrameStatusEOF || status == rcFrameGrabberStatus::eFrameStatusOK ) );
        if (status != rcFrameGrabberStatus::eFrameStatusOK) break;
        images.push_back (framePtr);
        rcWindow win (framePtr);
        //        win.print(dummy, std::cout);
    }// End of For i++
    
    EXPECT_TRUE(images.size() == 56);
    
    EXPECT_TRUE(grabber->stop());
    
}


TEST(cinder_qtime_grabber_and_similarity, run)
{
    // vf does not support QuickTime natively. The ut expectes and checks for failure
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string qmov_name ("box-move.mov");
    std::string qmov = create_filespec (gvp->test_data_folder (), qmov_name);
    boost::shared_ptr<rcFrameGrabber> grabber (reinterpret_cast<rcFrameGrabber*> (new vf_utils::qtime_support::CinderQtimeGrabber( qmov )) );
    ((vf_utils::qtime_support::CinderQtimeGrabber*)grabber.get())->print_to_ (std::cout);
    
    EXPECT_TRUE (grabber.get() != NULL);
    
    rcFrameGrabberError error = grabber->getLastError();
    int i = 0;
    uint8 dummy;
    // Grab everything
    EXPECT_TRUE (grabber->start());
    EXPECT_TRUE (grabber->frameCount() == 56);
    //    std::vector<rcFrameRef> images;
    std::vector<rcWindow> images;
    // Note: infinite loop
    for( i = 0; ; ++i )
    {
        rcTimestamp curTimeStamp;
        rcRect videoFrame;
        rcWindow image, tmp;
        rcFrameRef framePtr;
        rcFrameGrabberStatus status = grabber->getNextFrame( framePtr, true );
        EXPECT_TRUE((status == rcFrameGrabberStatus::eFrameStatusEOF || status == rcFrameGrabberStatus::eFrameStatusOK ) );
        if (status == rcFrameGrabberStatus::eFrameStatusEOF) break;
        rcWindow win (framePtr);
        images.push_back (win);
        
        //  win.print(dummy, std::cout);
    }// End of For i++
    
    EXPECT_TRUE(images.size() == 56);
    
    std::vector<rcWindow> rand_images;

    for (uint32 i = 0; i < images.size(); i++)
    {
        EXPECT_EQ(true, images[i].isGray());
    }
    
    
    for (uint32 i = 0; i < images.size(); i++)
    {
        rcWindow tmp(16,16);
        tmp.copyPixelsFromWindow(images[i]);
        rand_images.push_back(tmp);
    }
    

    boost::shared_ptr<sm_producer> sp;
    sp =  boost::shared_ptr<sm_producer> ( new sm_producer () );
    sp->load_images (rand_images);
    EXPECT_EQ(56, sp->frames_in_content ());
    sp->operator()(0, 0);
    EXPECT_EQ(56, sp->frames_in_content());
    EXPECT_EQ(56, sp->shannonProjection().size () );
    EXPECT_EQ(false, vf_utils::math::contains_nan(sp->shannonProjection().begin(), sp->shannonProjection().end()));

    
    EXPECT_TRUE(grabber->stop());
    

}


TEST(cb_similarity_producer, run)
{
    // vf does not support QuickTime natively. The ut expectes and checks for failure
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string qmov_name ("box-move.mov");
    static std::string rfymov_name ("box-move.rfymov");
    std::string qmov = create_filespec (gvp->test_data_folder (), qmov_name);
    std::string rfymov = create_filespec (gvp->test_data_folder (), rfymov_name);
    
    {
        cb_similarity_producer test (rfymov);
        EXPECT_EQ(0, test.run () );
        EXPECT_EQ(true, test.is_movie_loaded () );
        EXPECT_EQ(57, test.frameCount ());
        EXPECT_EQ(57, test.sp->shannonProjection().size () );
        EXPECT_EQ(false, vf_utils::math::contains_nan(test.sp->shannonProjection().begin(), test.sp->shannonProjection().end()));
        
        EXPECT_EQ(true, test.mlies.empty());
    }
    {
        cb_similarity_producer test (qmov);
        EXPECT_EQ(0, test.run () );
        EXPECT_EQ(true, test.is_movie_loaded () );
        EXPECT_EQ(56, test.frameCount ());
        EXPECT_EQ(56, test.sp->shannonProjection().size () );
        EXPECT_EQ(false, vf_utils::math::contains_nan(test.sp->shannonProjection().begin(), test.sp->shannonProjection().end()));
        
        EXPECT_EQ(true, test.mlies.empty());
    }
    
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






