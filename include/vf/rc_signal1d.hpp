#ifndef __SIGNAL1D__
#define __SIGNAL1D__


#include "linlsq.h"
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include "rc_stats.h"
#include <boost/function.hpp>


typedef std::pair<uint,double> extrema_pos; 



template<typename Elem, template<typename T, typename = std::allocator<T> > class Container>
struct norm_scaler
{
    typedef typename Container<Elem>::iterator Iterator;
    typedef typename Container<Elem>::const_iterator constIterator;
    
    
    void operator()(Container<Elem>& src, Container<Elem>& dst, int pw)
    {
        constIterator bot = std::min_element (src.begin (), src.end() );
        constIterator top = std::max_element (src.begin (), src.end() );
        
        float scaleBy = *top - *bot;
        dst.resize (src.size ());
        Elem bias = 1.0 / std::min (1, pw);
        for (uint ii = 0; ii < src.size (); ii++)
        {
            Elem dd = (src[ii] - *bot) / scaleBy;
            dst[ii] = std::pow (dd, bias);
       }
    }
};





template<typename Elem, template<typename T, typename = std::allocator<T> > class Container>
struct second_derivative_producer
{
    typedef typename Container<Elem>::iterator Iterator;
    typedef typename Container<Elem>::const_iterator constIterator;


    /*!
     Central Difference. F" = (d[+1] - 2 * d[0] / 2 + d[-1]) / 1 
     a 1 x 3 operation
     */
    
   void operator () (Container<Elem>& data, Container<Elem>& deriv, Container<Elem>& zcm )
    {  
            deriv.resize (data.size ());
            constIterator dm1 = data.begin();
            constIterator dend = data.end ();
            dend--;        dend--;        dend--;
            Iterator d1 = deriv.begin(); 
            d1++; // first place we compute
            for (; dm1 < dend; d1++, dm1++)
            {
                Elem s = dm1[0];
                Elem c = dm1[1];
                s -= (c + c);
                s += (dm1[2]);
                *d1 = s;
            }
            
            // use replicates for first and last 
            deriv.at(0) = deriv.at(1);
            deriv.at(data.size()-1) = deriv.at(data.size()-2);
        
        Iterator d2 = deriv.begin ();
        Iterator de = deriv.end ();
        zcm.resize (deriv.size ());
        Iterator dzc = zcm.begin ();
        d2++;de--;dzc++;
        for (; d2 < de; d2++) { float zcv (*d2 * d2[-1]); *dzc++ = zcv < 0 ? -zcv : 0; }
     }
    
};
    


template<typename Elem, template<typename T, typename = std::allocator<T> > class Container>
struct peak_detector
{
    typedef typename Container<Elem>::iterator Iterator;
    typedef typename Container<Elem>::const_iterator constIterator;
    

    
    void operator()(Container<Elem>& src, std::vector<extrema_pos>& peaks, int half_window = 4, float low_threshold = 0.00009)
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
                extrema_pos pp (peak_index, src[peak_index]);
                peaks.push_back (pp);
                std::cerr << peak_index << "," << std::endl;
            }
        }
        while (left++ < right++ && right < src.end());
        
        
    }
    
};

template<typename Elem, template<typename T, typename = std::allocator<T> > class Container>
struct valley_detector
{

    typedef typename Container<Elem>::iterator Iterator;
    typedef typename Container<Elem>::const_iterator constIterator;
    
    void operator()(Container<Elem>& src, std::vector<extrema_pos>& peaks, int half_window = 4)
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
                extrema_pos pp (peak_index, src[peak_index]);
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
    
    
    fpad sin(fpad t) 
    { 
        using std::sin; 
        using std::cos; 
        double v = t.value(); 
        t.equalsTransform(sin(v),cos(v)); // Verkettung 
        return t; 
    } 
    
    
    
    friend const fpad operator+(fpad const& a, fpad const& b) 
    { fpad r(a); r+=b; return r; } 
    
    friend const fpad operator-(fpad const& a, fpad const& b) 
    { fpad r(a); r-=b; return r; } 
    
    friend const fpad operator*(fpad const& a, fpad const& b) 
    { fpad r(a); r*=b; return r; } 
}; 

#endif
