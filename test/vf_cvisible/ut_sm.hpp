#ifndef __UT_SM__
#define __UT_SM__

#include <iostream>
#include <string>
#include "vf_sm_producer.h"
//using namespace ci;
using namespace std;


struct cb_similarity_producer
{
    cb_similarity_producer (const std::string& filename, bool auto_run = false)
    {
        m_filename = filename;
        m_auto = auto_run;
    }
    
    int run ()
    {
        try
        {
            sp =  boost::shared_ptr<sm_producer> ( new sm_producer () );
            boost::function<void (int&,double&)> frame_loaded_cb = boost::bind (&cb_similarity_producer::signal_frame_loaded, this, _1, _2);
            boost::function<void ()> content_loaded_cb = boost::bind (&cb_similarity_producer::signal_content_loaded, this);
            boost::signals2::connection fl_connection = sp->registerCallback(frame_loaded_cb);
            boost::signals2::connection ml_connection = sp->registerCallback(content_loaded_cb);
            
            rcFrameGrabberError error;
            if (m_auto) sp->set_auto_run_on ();
            
            sp->load_content_file (m_filename);
            if (! m_auto) sp->operator()(0, 0);
            
        }
        catch (...)
        {
            return 1;
        }
        return 0;
        
    }
    void signal_content_loaded ()
    {
        movie_loaded = true;
        std::cout << "Content Loaded " << std::endl;
    }
    void signal_frame_loaded (int& findex, double& timestamp)
    {
        static int times = 0;
    //    std::cout << ++times << " : " << findex << " @ " << timestamp << std::endl;
        frame_indices.push_back (findex);
        frame_times.push_back (timestamp);
        if (! (equal (timestamp, exected_movie_times[findex], 0.0001 )) )
        {
            mlies.push_back (false);
        }
    }
    
    boost::shared_ptr<sm_producer> sp;
    bool m_auto;
    std::vector<int> frame_indices;
    std::vector<double> frame_times;
    std::vector<bool> mlies;
    std::string m_filename;
    bool movie_loaded;
    void clear_movie_loaded () { movie_loaded = false; }
    bool is_movie_loaded () { return movie_loaded; }
    int frameCount () { if (sp) return sp->frames_in_content (); return -1; }
    
    
    double exected_movie_times [57] = {0, 0.033333, 0.066666, 0.099999, 0.133332, 0.166665, 0.199998, 0.233331, 0.266664, 0.299997, 0.33333, 0.366663, 0.399996, 0.433329, 0.466662, 0.499995, 0.533328, 0.566661, 0.599994, 0.633327, 0.66666, 0.699993, 0.733326, 0.766659, 0.799992, 0.833325, 0.866658, 0.899991, 0.933324, 0.966657, 0.99999, 1.03332, 1.06666, 1.09999, 1.13332, 1.16665, 1.19999, 1.23332, 1.26665, 1.29999, 1.33332, 1.36665, 1.39999, 1.43332, 1.46665, 1.49998, 1.53332, 1.56665, 1.59998, 1.63332, 1.66665, 1.69998, 1.73332, 1.76665, 1.79998, 1.83332, 1.86665 };
    
};


#endif

