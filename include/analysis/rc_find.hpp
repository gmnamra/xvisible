#ifndef _rc_FIND_
#define _rc_FIND_

#include <rc_diamondscan.h>
#include <bitset>
#include <vector>
#include <rc_find.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;


#define DCLmarkers(a)                                                   \
    bool have##a (enum Have##a what) const { return mHave##a[what];}    \
    bool mark##a (enum Have##a what) { return mHave##a[what]=1;}        \
    bool clear##a (enum Have##a what) { return mHave##a[what]=0;}       \
    void reset##a () { mHave##a.reset();}

#define DFNmarkers(a) std::bitset<eNum##a> mHave##a


class rcLocation : public rc2Fvector
{
public:
    enum HaveCondition
    {
        eUnknown=0,
        eLeftEdge,
        eRightEdge,
        eTopEdge,
        eBotEdge,
        eNumCondition
    };
		
    rcLocation () : _f (0.0f) {_s.x=_s.y=0; markCondition (rcLocation::eUnknown);}
    
	bool isValid () const { return ! haveCondition (rcLocation::eUnknown) && _f > 0.0 && _s.x > 0 && _s.y > 0; }

    void set (const float f, const cv::Point&loc, const rcFRect& box, const rc2Fvector& pos)
    {
        x( pos.x() );
        y(pos.y() );
        _s.x = loc.x;
        _s.y = loc.y;
        _f = f;
        _box = box;
    }
		
    const rc2Fvector& pos () { return *this; }
    float score () const { return _f; }
    
    DCLmarkers(Condition);
		
    bool notOnEdge () { return mHaveCondition.none (); }
    bool onEdge () { return mHaveCondition.any (); }
    void point (cv::Point& p) { _s = p; }
    const cv::Point& point () const { return _s; }
    const rcFRect& box () const { return _box; }
		
    
    friend ostream& operator<< (ostream& ous, const rcLocation& dis)
        {
            ous << dis.x() << "," << dis.y() << " score: " << dis.score();
            return ous;
        }
private:
    rcFRect _box;
    float _f;
    cv::Point _s;
    // Info markers
    DFNmarkers(Condition);
};


rcLocation rcFind (const cv::Mat& fixed, const rcFRect& fixedROI, const cv::Mat& moving, const rcFRect& movingROI);

struct rcFindHelper
{
    rcFindHelper () : threshold (0.37f), method (TM_CCORR_NORMED), debug (0) {};
    float threshold;
    int method;
    int debug;
    cv::Mat corrspace;
    rcIPair minloc;
    rcIPair maxloc;	
    rcFPair minmaxval;
};

    //void rcFind (const sharedIpl& model, const sharedIpl& moving, std::vector<rcLocation>& peaks, rcFindHelper& helper);


#undef DCLmarkers
#undef DFNmarkers


#endif

