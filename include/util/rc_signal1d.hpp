#include "linlsq.h"
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <rc_stats.h>
#include <boost/function.hpp>

struct peak_detector
{
    typedef std::pair<uint, float> peak_pos;
    typedef std::vector<float> Container;
    typedef Container::iterator Iterator;
    
    void operator()(Container& src, std::vector<peak_pos>& peaks, int half_window = 4, float low_threshold = 0.00009)
    {
        int fw = 2 * half_window + 1;
        if (src.size() <= fw) return;
        peaks.resize (0);
        std::cerr << "Peak Detector src size: " << src.size() << std::endl;            
        Iterator left = src.begin();
        Iterator right = src.begin();
        int peak_index = half_window - 1;
        std::advance(right, fw);
        do
        {
            peak_index++;                
            Iterator maxItr = std::max_element (left, right);
            if (maxItr == src.end()) continue;
            if (*maxItr < low_threshold) continue;
            vector<float>::difference_type ds = std::distance (left, maxItr);
            if (ds == half_window)
            {
                peak_pos pp (peak_index, src[peak_index]);
                peaks.push_back (pp);
                std::cerr << peak_index << "," << std::endl;
            }
        }
        while (left++ < right++ && right < src.end());
        
        
    }
    
};


struct valley_detector
{
    typedef std::pair<uint, float> peak_pos;
    typedef std::vector<float> Container;
    typedef Container::iterator Iterator;
    
    void operator()(Container& src, std::vector<peak_pos>& peaks, int half_window = 4)
    {
        int fw = 2 * half_window + 1;
        if (src.size() <= fw) return;

        // use regression to find a flat threshold
        LLSQ lsqr;
        double dsize = (double) src.size ();        
        for (uint ii = 0; ii < src.size (); ii++)
            lsqr.add(ii / dsize, src[ii]);
        
        // Check if we have a plausible fit. Check to see if the angle is less than a degree
        rcRadian onedegree (rcDegree (1.0 ));
        float high_threshold = lsqr.vector_fit_angle().Double() < onedegree.Double() ?
        lsqr.c(lsqr.m()) : rfMedian (src);
        
        peaks.resize (0);
        
        std::cerr << "Valley Detector src size: " << src.size() << std::endl;            
        Iterator left = src.begin();
        Iterator right = src.begin();
        int peak_index = half_window - 1;
        std::advance(right, fw);
        do
        {
            peak_index++;                
            Iterator maxItr = std::min_element (left, right);
            if (maxItr == src.end()) continue;
            if (*maxItr > high_threshold) continue;
            vector<float>::difference_type ds = std::distance (left, maxItr);
            if (ds == half_window)
            {
                peak_pos pp (peak_index, src[peak_index]);
                peaks.push_back (pp);
                std::cerr << peak_index << "," << std::endl;
            }
        }
        while (left++ < right++ && right < src.end());
        
        
    }
    
};

class fpad // forward propagating automatic differentiation 
{ 
    double value_; 
    double deriv_; 
public: 
    fpad(double v, double d=0) : value_(v), deriv_(d) {} 
    
    double value() const {return value_;} 
    double derivative() const {return deriv_;} 
    
    const fpad& equalsTransform(double newVal, double outer_deriv) 
    { deriv_ = deriv_ * outer_deriv; // Kettenregel 
        value_ = newVal; 
        return *this;
    } 
    
    const fpad& operator+=(fpad const& x) 
    { value_ += x.value_; 
        deriv_ += x.deriv_; 
        return *this; 
    } 
    
    const fpad& operator-=(fpad const& x) 
    { value_ -= x.value_; 
        deriv_ -= x.deriv_; 
        return *this; 
    } 
    
    const fpad& operator*=(fpad const& x) 
    { // Produkt-Regel: 
        deriv_ = deriv_ * x.value_ + value_ * x.deriv_; 
        value_ *= x.value_; 
        return *this; 
    } 
    
    friend const fpad operator+(fpad const& a, fpad const& b) 
    { fpad r(a); r+=b; return r; } 
    
    friend const fpad operator-(fpad const& a, fpad const& b) 
    { fpad r(a); r-=b; return r; } 
    
    friend const fpad operator*(fpad const& a, fpad const& b) 
    { fpad r(a); r*=b; return r; } 
}; 

fpad sin(fpad t) 
{ 
    using std::sin; 
    using std::cos; 
    double v = t.value(); 
    t.equalsTransform(sin(v),cos(v)); // Verkettung 
    return t; 
} 
