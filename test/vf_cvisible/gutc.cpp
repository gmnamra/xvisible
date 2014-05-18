



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
#include "cvisible/vf_utils.hpp"


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



struct cb_similarity_producer
{
    cb_similarity_producer (rcFrameGrabber* grabber)
    {
        sp =  boost::shared_ptr<SimilarityProducer> ( new SimilarityProducer () );
        boost::function<void (int&,double&)> frame_loaded_cb = boost::bind (&cb_similarity_producer::signal_frame_loaded, this, _1, _2);
        boost::signals2::connection fl_connection = sp->registerCallback(frame_loaded_cb);
        
        rcFrameGrabberError error;
        sp->load_content_grabber(*grabber, error);
        
    }
    
    int run ()
    {
        try
        {
            
            sp->operator()(0, 0);
            
        }
        catch (...)
        {
            return 1;
        }
        return 0;
        
    }
    void signal_movie_loaded () { movie_loaded = true; }
    void signal_frame_loaded (int& findex, double& timestamp)
    {
        frame_indices.push_back (findex);
        frame_times.push_back (timestamp);
        if (! (equal (timestamp, exected_movie_times[findex], 0.0001 )) ) mlies.push_back (false);
    }
    
    boost::shared_ptr<SimilarityProducer> sp;
    
    std::vector<int> frame_indices;
    std::vector<double> frame_times;
    std::vector<bool> mlies;
    
    bool movie_loaded;
    void clear_movie_loaded () { movie_loaded = false; }
    bool is_movie_loaded () { return movie_loaded; }
    
    double exected_movie_times [57] = {0, 0.033333, 0.066666, 0.099999, 0.133332, 0.166665, 0.199998, 0.233331, 0.266664, 0.299997, 0.33333, 0.366663, 0.399996, 0.433329, 0.466662, 0.499995, 0.533328, 0.566661, 0.599994, 0.633327, 0.66666, 0.699993, 0.733326, 0.766659, 0.799992, 0.833325, 0.866658, 0.899991, 0.933324, 0.966657, 0.99999, 1.03332, 1.06666, 1.09999, 1.13332, 1.16665, 1.19999, 1.23332, 1.26665, 1.29999, 1.33332, 1.36665, 1.39999, 1.43332, 1.46665, 1.49998, 1.53332, 1.56665, 1.59998, 1.63332, 1.66665, 1.69998, 1.73332, 1.76665, 1.79998, 1.83332, 1.86665 };
    
};


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
    
    // Grab everything
    EXPECT_TRUE (grabber->start());
    EXPECT_TRUE (grabber->frameCount() == 56);
    std::vector<rcSharedFrameBufPtr> images;
    // Note: infinite loop
    for( i = 0; ; ++i )
    {
        rcTimestamp curTimeStamp;
        rcRect videoFrame;
        rcWindow image, tmp;
        rcSharedFrameBufPtr framePtr;
        rcFrameGrabberStatus status = grabber->getNextFrame( framePtr, true );
        EXPECT_TRUE((status == rcFrameGrabberStatus::eFrameStatusEOF || status == rcFrameGrabberStatus::eFrameStatusOK ) );
        if (status == rcFrameGrabberStatus::eFrameStatusEOF) break;
        images.push_back (framePtr);
    }// End of For i++

    EXPECT_TRUE(images.size() == 56);
    
    if ( ! grabber->stop() )
        error = grabber->getLastError();
    
}

TEST(cb_similarity_producer, run)
{
    // vf does not support QuickTime natively. The ut expectes and checks for failure
    genv* gvp = reinterpret_cast<genv*>(envp);
    EXPECT_TRUE (gvp != 0 );
    static std::string qmov_name ("box-move.mov");
    std::string qmov = create_filespec (gvp->test_data_folder (), qmov_name);
    boost::shared_ptr<rcFrameGrabber> grabber_ref (reinterpret_cast<rcFrameGrabber*> (new vf_utils::qtime_support::CinderQtimeGrabber( qmov)) );
    cb_similarity_producer test (grabber_ref.get());
    EXPECT_EQ(0, test.run () );
    EXPECT_EQ(true, test.mlies.empty());
    
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






