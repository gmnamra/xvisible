//
// C++  Interfacen: histogram
//
//
// Author: Nicholas Phillips <nicholas.G.Phillips@nasa.gov>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <deque>

/*
 Used for computing and storing histogram data.
 
 Once the histogram has been computed, it can be accessed
 via given it values between 0 and 1, corresponding to the
 lowest and highest values found/used. The returned value
 is between 0 and 1. Use the operator()
 methods.
 
 Also can provide stats on the underlying data that was binned.
 
 @author Nicholas Phillips <Nicholas.G.Phillips@nasa.gov>
 */
class Histogram
{
public:
    // Compute stats
    void setup(std::deque<double> &x);
    // Compute the histogram
    void build(std::deque<double> &, const double, const double, long bins = 2048);
    void build(std::deque<double> &, long bins = 2048);
    
    // return some pre-computed stats
    double min()     const   { return minv; };
    double max()     const   { return maxv; };
    double amax()    const   { return amaxv; };
    double mean()    const   { return meanv; };
    double stddev()  const   { return stddevv; };
    double max_bin () const { return bmax; }
    double max_value () const { return hmax; }
    
    
    // Access the histogram
    double operator()(const double x) const;
    double operator()(const double x0,const double x1) const;
    long bins () { return nbin; }
    long bins (long nb) { long ob = bins (); if (nb > 0) nbin = nb; return ob; }
    int index_from_bin (int bin)
    {
        if (! bin_2_index.empty () && bin < bin_2_index.size () && bin_2_index[bin] > 0)
            return bin_2_index[bin];
        return -1;
    }
protected:
    std::deque<double> norm; // Normalized
    std::deque<int> bin_2_index; // 

    
    std::deque<int> h;     // The stored histogram
    long nbin;              // Number of bins
    int hmax;               // Largest bin value
    int bmax;               // Bin holding the largest value
    double minr;           // Bottom of histogram range
    double maxr;           // Top of histogram range
    double minv;             // Smallest value in the input vector
    double maxv;             // Largest  value in the input vector
    double amaxv;            // Max absolute value of input values
    double meanv;            // Mean of input vector of values
    double stddevv;          // Standard Deviation of input vector of values
    
    bool add(int bin, int index)
    {
        if (bin < bin_2_index.size () && bin_2_index[bin] < 0)
            bin_2_index[bin] = index;
    }

};

#endif
