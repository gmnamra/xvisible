/******************************************************************************
*	rc_uitypes.h
*
*	This file contains the miscellaneous UI type definitions.
*	
******************************************************************************/

#ifndef _rcBASE_TYPES_H_
#define _rcBASE_TYPES_H_

#include <boost/shared_ptr.hpp>
#include <deque>
#include <rc_types.h>

using namespace std;

class CurveData2d 
{
public:
    
    CurveData2d (deque<deque<double> >& sm, uint32 matrix_size) :
    m_sm (sm), m_matrix (matrix_size)
    {
        
    }
    
    CurveData2d (const CurveData2d& other)
    {
        m_matrix = other.m_matrix;
        m_sm = other.m_sm;
    }
    
    
    CurveData2d& operator = (const CurveData2d& other)
    {
        m_matrix = other.m_matrix;
        m_sm = other.m_sm;
    }


    bool operator == (const CurveData2d other)
    {
        return m_matrix == other.m_matrix && m_sm == other.m_sm;
    }
    
    // produce profiles excluding data as described by execluded rcs:
    // excluded_rcs is the same size as matrix side
    // regions of 1 or more adjacent 1s are considerd included. 
    // regions of 0 or more adjacent 1s are considerd excluded.
    // the resultant profile is usable in conjuction with the mask
    // data in bin j of profile is valid only and only if bin j of mask is 1
    
    void masked_profiles (std::deque<int> mask, deque<float> & profile)
    {
        rmUnused(mask);
        profile.resize (m_matrix);
    }
    
    
    void mean_profile (int pos, int length, deque<float>& profile)
    {
        profile.resize (m_matrix);
        // calculate the end. Limit to the end
        int after  = pos + length - 1;
        after = after >= (int) m_matrix ? (m_matrix - 1) : after;
        assert (after > 0);
        uint32 uafter = (uint32) after;        
        for (uint32 i = pos; i < uafter; i++)
        {
            assert(m_sm[i].size() == m_matrix);
            profile[i-pos] = m_sm[i][i];
        }
        
        for (uint32 i = pos; i < uafter; i++)
            for (uint32 j = (i+1); j < uafter; j++)
            {
                profile[i] += m_sm[i][j];
                profile[j] += m_sm[i][j];
            }
        
        for (uint32 ii = 0; ii < profile.size (); ii++)
        {
            profile [ii] /= m_matrix;
        }
    }
    
    void mean_profile (deque<float>& profile)
    {
        mean_profile (0, m_matrix, profile);
    }
    
private:
    deque<deque<double> > m_sm;
    uint32 m_matrix;
    
};

typedef boost::shared_ptr<CurveData2d> SharedCurveData2dRef;

#endif // _rcBASE_TYPES_H_
