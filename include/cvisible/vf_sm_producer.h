
#ifndef __RC_SIMILARITY_PRODUCER_H
#define __RC_SIMILARITY_PRODUCER_H

#include <stdio.h>
#include <deque>
#include "rc_types.h"
#include "rc_framegrabber.h"
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/signals2/slot.hpp>
#include <typeinfo>
#include <string>
#include "signaler.h"
using namespace std;
using namespace boost;

namespace vf

{
    class sm_roducer
    {
    public:
        typedef std::deque<double> sMatrixProjectionType;
        typedef std::deque< std::deque<double> > sMatrixType;
        typedef void (sig_cb_movie_loaded) ();
        typedef void (sig_cb_frame_loaded) (int&, double&);
        typedef void (sig_cb_sm1d_available) ();
        typedef void (sig_cb_sm2d_available) ();
        SimilarityProducer ();
        
        bool load_content_file (const string& fq_path);
        int load_content_grabber (rcFrameGrabber& grabber, rcFrameGrabberError& error);
        
        void operator () (int start_frame, int frames) const;
        
        bool has_content () const;
        int bytes_per_pixel () const;
        int pixels_per_channel () const;
        int channels_per_plane () const;
        int planes_per_image () const;
        int images_in_movie () const;
        
        const std::string& content_file () const;
        int process_start_frame () const;
        int process_last_frame () const;
        int process_count () const;
        
        const sMatrixType& similarityMatrix () const;
        
        const sMatrixProjectionType& meanProjection () const;
        
        const sMatrixProjectionType& shannonProjection () const;
        
        /** \brief registers a callback function/method to a signal with the corresponding signature
         * \param[in] callback: the callback function/method
         * \return Connection object, that can be used to disconnect the callback method from the signal again.
         */
        template<typename T>
        boost::signals2::connection registerCallback (const boost::function<T>& callback);
        
        /** \brief indicates whether a signal with given parameter-type exists or not
         * \return true if signal exists, false otherwise
         */
        template<typename T>
        bool providesCallback () const;
        
        
    private:
        class spImpl;
        boost::shared_ptr<spImpl> _impl;
        
        
    };
    
    
}



#endif /* __RC_SIMILARITY_H */





