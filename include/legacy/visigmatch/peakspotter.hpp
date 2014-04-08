/*
 *
 *$Header $
 *$Id $
 *$Log: $
 *
 *
 *
 * Copyright (c) 2008 Reify Corp. All rights reserved.
 */
#ifndef __PEAKSPOTTER_H
#define __PEAKSPOTTER_H

#include <rc_math.h>
#include <rc_fileutils.h>
#include <deque>
#include <vector>
#include <numeric>
#include <rc_stats.h>
#include <cstddef>
#include <algorithm>
#include <rc_thread.h>
#include <linlsq.h>
#include "histogram.h"

using std::distance;
using std::copy;
using std::lower_bound;
using std::sort;

using namespace std;


class PeakSpotter
{
public:
    typedef double value_type;

    typedef PeakSpotter ThisType;
    
    static const double invalid_val () 
    {
        return std::numeric_limits<double>::max ();        
    }
    
    /// Constructor. 
    PeakSpotter(const std::deque<double>& src):
    {
        rmAssert (src.size());
        _copy.resize (src.size ()); 
        std::copy (src.begin (), src.end(), _copy.begin ());        
        _sH.setup (_copy);
        _sH.build (_copy, _sH.min(), _sH.max(), 100);
        for (int i = 0; i < src.size (); i++)
            _lsq.Add ((double) i, src[i]);
        _m = _lsq.m();
        _c = _lsq.c(_m);
        _mean_point = _lsq.mean_point ();
        _direction = _lsq.vector_fit();
        
        std::cerr << " PeakSpotter Ctor " << std::endl;
    }
    

    // Points above the fitted line are capped to the line
    bool transform ()
    {
         rcLock lock (_mu);
        
        _dst.resize (_copy.size ());
        std::copy (_copy.begin(), _copy.end(), _dst.begin ());
        
        int frame_index = 0;
        _frames.clear ();
        uint32 count (_dst.size ());
        for (; frame_index < count; frame_index++)
        {
            double d = 1.0 - _dst[frame_index];
            _frames.push_back (frame_index);
            _dst[frame_index] = (d < tqueryP) ? d : (d + tqueryP) / 2;
        }
        return true;
    }

    const std::deque<double>& transformed () const
    {
        return _copy;
    }
    
    const std::deque<int>& frame_indices () const
    {
        return _frames;
    }

    const std::deque<double>& peaks () const
    {
        return _peaks;
    }    
    
    bool transformed (const std::string& filef, bool row_wise ) const
    {
        return to_csv (_copy, filef, row_wise );
    }
    

    bool frame_indices (const std::string& filef, bool row_wise ) const
    {
        return to_csv (_frames, filef, row_wise );
    }

    bool peaks (const std::string& filef, bool row_wise ) const
    {
        return to_csv (_peaks, filef, row_wise );
    }


private:
    
    bool to_csv(const std::deque<double>& dm, const std::string& filef, bool row_wise) const
    {
         rcLock lock (_mu);
        
        bool ok = false; // assume failure
        try
        {
            std::string ending = row_wise ? "," : ",\n";
            ofstream output_stream(filef.c_str());
            uint32 j;
            output_stream.precision(12);
            output_stream.setf(std::ios::fixed, std::ios::floatfield);
            for (j = 0; j < dm.size() - 1 ; j++)
                output_stream << dm[j] << ending;
            output_stream << dm[j] << endl;
            output_stream.close ();
            ok = true;
        }
        catch (...)
        {
            ok = false;
        }
        return ok;
    }
    
    bool to_csv(const std::deque<int>& dm, const std::string& filef, bool row_wise) const
    {
         rcLock lock (_mu);
        
        bool ok = false; // assume failure
        try
        {
            std::string ending = row_wise ? "," : ",\n";
            ofstream output_stream(filef.c_str());
            uint32 j;
            output_stream.precision(12);
            for (j = 0; j < dm.size() - 1 ; j++)
                output_stream << dm[j] << ending;
            output_stream << dm[j] << endl;
            output_stream.close ();
            ok = true;
        }
        catch (...)
        {
            ok = false;
        }
        return ok;
    }


private:
    bool _acc_hist_done;
    std::deque<double> _acc_hist;
    std::deque<double> _sorted;
    std::deque<double> _copy;
    std::deque<double> _dst; 
    std::deque<int> _frames;
    std::deque<double> _peaks;
    LLSQ _lsq;
    double _m, _c;
    rc2fvector _mean_point;
    rcRadian _direction;
    Histogram _sH;
    mutable rcMutex _mu;
    
public:
 
};





#endif /* __PEAKSPOTTER_H */
