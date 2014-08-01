#ifndef __basic_awb__
#define __basic_awb__


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

namespace anonymous
{
    
    class mean_world
    {
        static const uint32 default_p () { return 6; }
        
    public:
        mean_world (uint32 p = default_p ()) { init_p (p); }
        
        bool check (const Mat& src) { return src.channels() == 3; }
        
        
        
        
    private:
        vector<Mat> mChannels;
        Vec3f mLab;
        float mP;
        double mOverP;
        void init_p (uint32 p) { mP = p; mOverP = 1.0f / mP; }
        
        void run_file (const std::string& fqfn, uint32 p)
        {
            cv::Mat image = cv::imread (fqfn, 0);
            if (check (image) )
            {
                cv::Mat dst = image.clone ();
                init_p (p);
                lab_means (image, dst);
            }
            
        }
        
        void lab_means (Mat& src, Mat& dst)
        {
            assert (src.channels() == 3);
            static float overShoot = 1.1f;
               Mat lab;
            src.copyTo(lab);
            cvtColor(lab,lab,CV_BGR2Lab);
            cv::split (lab, mChannels);
            assert (mChannels.size() == 3);
            
            cv::Mat tmp (src.rows, src.cols, CV_32F);

            // move each channel to float
            for (uint32 cc=0; cc < mChannels.size(); cc++)
            {
                mChannels[cc].create (src.rows, src.cols, CV_32F);
            }

            // get minkowski mean of a and b channels. Negate for later
            for (uint32 cc=1; cc < mChannels.size(); cc++)
            {
                cv::pow (mChannels[cc], mP, tmp);
                mLab[cc] = - std::pow ((double)cv::mean(tmp)(0),  mOverP);
            }
            
            // apply correction using the means
            // these steps could also be parallezed using OpenCv basic math ops
            uint32 width = src.cols;
            uint32 height = src.rows;
            for (uint32 cc=1; cc < mChannels.size(); cc++)
            {
                float cadj = mLab[cc];
                for (uint32 rows = 0; rows < height; rows++)
                    for (uint32 cols = 0; cols < width; cols++)
                    {
                          // move chroma distance luminance amount plus overshoot given the uncertainty
                        float v = mChannels[cc].at<float>(rows, cols);
                        float l = mChannels[0].at<float>(rows, cols);
                        float adj = cadj + (l / 100) * overShoot;
                        mChannels[cc].at<float>(rows, cols) = v + adj;
                    }
            }

            // luminance untouched, a and b adjusted. time to merge
            cv::merge (mChannels, lab);
            cvtColor(lab,dst,CV_Lab2BGR);
        }
        
    };
    
    
    
    
    
    
}

#endif


