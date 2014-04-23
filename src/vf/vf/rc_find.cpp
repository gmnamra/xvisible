

#include "rc_find.hpp"
#include <limits>
#include "rc_vector2d.h"
#include "rc_fit.h"

using namespace std;
using namespace cv;

/*
 * Find template defined by fixedROI of fixed within movingROI of moving. 
 * On return rcLocation
 contains the match score and position of the top left in moving
 * movingROI is rest to the rect at the found location
 * Consider selecting an origin
 
 * note: will set the ROIs with the passed in ROI
 */

void to_cv (const rcFRect& f, cv::Rect& r)
{
    r.x = (int) f.ul().x();
    r.y = (int) f.ul().y();
    r.width = (int) f.width();
    r.height = (int) f.height ();
}

#if 1
/*
 OpenCv Support
 */

rcLocation rcFind (const cv::Mat& fixed, const rcFRect & fixedROI, const cv::Mat& moving, const rcFRect & movingROI)
{
    cv::Rect r(fixedROI.ul().x(), fixedROI.ul().y(), fixedROI.width(), fixedROI.height());
    cv::Rect s(movingROI.ul().x(), movingROI.ul().y(), movingROI.width(), movingROI.height());
    cv::Size cspaceSize (s.width-r.width+1, s.height-r.height+1);
    if (cspaceSize.width < 1 || cspaceSize.height < 1) return rcLocation ();
	
    cv::Mat corr_space_m = cv::Mat(cspaceSize, CV_32FC1);
    cv::Mat fmat (fixed, r);
    cv::Mat mmat (moving, s);
    
    cv::matchTemplate (fmat, mmat, corr_space_m, CV_TM_CCORR_NORMED);
    
//    cout << "CS = "<< endl << " "  << setprecision(3) << corr_space_m << endl << endl;

    cv::Point mi, ma; double miv, mav;
    cv::minMaxLoc (corr_space_m, &miv, &mav, &mi, &ma);
    
    rcLocation res;

    if (ma.x == 0) res.markCondition (rcLocation::eLeftEdge);
    if (ma.x == cspaceSize.width-1) res.markCondition (rcLocation::eRightEdge);
    if (ma.y == 0) res.markCondition (rcLocation::eTopEdge);
    if (ma.y == cspaceSize.height-1) res.markCondition (rcLocation::eBotEdge);
 
    const float ctr = corr_space_m.at<float>(ma.y, ma.x);
    // Interpolate as much as you can
    const float top = (res.onEdge() || res.haveCondition (rcLocation::eTopEdge)) ? 0 : corr_space_m.at<float>(ma.y - 1, ma.x);
    const float bot = (res.onEdge() || res.haveCondition (rcLocation::eBotEdge)) ? 0 : corr_space_m.at<float>(ma.y + 1, ma.x);
    const float left = (res.onEdge() || res.haveCondition (rcLocation::eLeftEdge)) ? 0 :corr_space_m.at<float>(ma.y, ma.x-1);
    const float right = (res.onEdge() || res.haveCondition (rcLocation::eRightEdge)) ? 0 : corr_space_m.at<float>(ma.y, ma.x+1);

    assert (real_equal (ctr, (float) mav));
    int check = (ctr >= top) + (ctr >= bot) + (ctr >= left) + (ctr >= right);
    assert (check == 4); //         
    float interpvx=-1.0, interpvy=-1.0;
    float interpx = ma.x + parabolicFit (left, ctr, right, &interpvx);
    float interpy = ma.y + parabolicFit (bot, ctr, top, &interpvy);        
    rc2Fvector pos (movingROI.ul().x() + interpx, movingROI.ul().y() + interpy);
    rcFRect newmoving (pos.x(), pos.y(), fixedROI.width(), fixedROI.height());
    res.set (mav, ma, newmoving, pos);

    return res;
}

#endif

#if 0

void rfFind ( const sharedIpl& moving, const sharedIpl& model, std::vector<rcLocation >& peaks, rcFindHelper& info)
{
    rcIPair  modelSize = sIPLsizePair (model);
    rcIPair  movingSize = sIPLsizePair (moving);
	
    assert (movingSize >= modelSize );
    CvSize cspaceSize = cvSize (-modelSize.x()+movingSize.x()+1, -modelSize.y()+movingSize.y()+1);
    assert (cspaceSize.width > 1 && cspaceSize.height > 1);

	
    sharedIpl corr_space(cvCreateImage( cspaceSize, IPL_DEPTH_32F, 1), IplReleaser ());    
    cvMatchTemplate ((CvArr*) moving.get(), (CvArr*) model.get(), (CvArr*) corr_space.get(), info.method);
    if (info.debug) info.corrspace = corr_space;
    CvPoint mi, ma; double miv, mav;
    cvMinMaxLoc ( (const IplImage*) corr_space.get(), &miv, &mav, &mi, &ma);
    info.minloc[0]=mi.x;	info.minloc[1]=mi.y;	info.maxloc[0]=ma.x;	info.maxloc[1]=ma.y;
    info.minmaxval[0] = miv;info.minmaxval[1]=mav;
    rfPeakDetect (corr_space, peaks, info);
}

#endif



