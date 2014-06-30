#ifndef __RC_CONTRACTION_SIGNAL_H
#define __RC_CONTRACTION_SIGNAL_H

#include <vector>
#include <map>
#include <algorithm>
#include "rc_signal1d.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/math/special_functions/sign.hpp>

#include "rc_stlplus.h>


struct contraction_window 
{
   
    uint id;
    uint posid;
    float value;
    std::vector<uint> contraction_indices;
    std::vector<uint> relaxation_indices;
    uint contraction, relaxation;        
    int state; // invalid/delete = -1, init 0, set = 1, edit set ++
    bool valid;
};

/*!
    @class ContractionSignal
    @Periodic Signal Recognition Class
    @discussion 
    @throws exception when not initialized
*/

template<typename Elem, template<typename T, typename = std::allocator<T> > class Container>
class ContractionSignal
{
public:

    typedef typename Container<Elem>::iterator Iterator;
    typedef typename Container<Elem>::const_iterator constIterator;
    typedef typename Container<extrema_pos>::iterator posIterator;
    typedef typename Container<extrema_pos>::const_iterator posConstIterator;  
    typedef std::vector<Elem> ct_thresholder;
    
/*! 
  @Default Constructor
*/
    ContractionSignal ( int N = 100 )
        : m_valid (false), m_quant (N),  m_thr_mapper (m_quant)
        {
        }

    void set_raw_data (Container<Elem>& d, Container<Elem>& ddd, std::vector<extrema_pos>& v)
    {
        m_valid = false;
        assert (d.size() == ddd.size ());
        m_data.resize (d.size () );
        std::copy (d.begin(), d.end(), m_data.begin ());
        m_d2_data.resize (ddd.size () );
        std::copy (ddd.begin(), ddd.end(), m_d2_data.begin());
        m_valleys.resize (d.size () );
        std::copy (v.begin(), v.end(), m_valleys.begin());
        setup_threshold_quantization ();

        // initialize a contraction for every valley
        m_bounds.resize (0);
        m_bounds.push_back (m_valleys[0].first / 2);       // before the the First contraction

        m_contractions.resize (m_valleys.size ());
        for (uint vc=0; vc < m_valleys.size(); vc++)
        {
            m_contractions[vc].id = vc;
            m_contractions[vc].posid = m_valleys[vc].first;
            m_contractions[vc].value = m_valleys[vc].second;
            m_contractions[vc].state = 0;
            if (vc > 0)
                m_bounds.push_back ((m_valleys[vc].first - m_valleys[vc-1].first)/ 2);
        }

    m_bounds.push_back ( ( m_data.size () - m_valleys[m_valleys.size()-1].first) / 2);

    assert (m_bounds.size () == m_valleys.size () + 1);
    // set the bounds in contractions at the end
    m_contractions[0].contraction = m_bounds[0];
    m_contractions[0].relaxation = m_bounds[1];
    if (m_contractions.size() > 1)
    {
        m_contractions[1].contraction = m_bounds[m_bounds.size()-2];
        m_contractions[1].relaxation = m_bounds[m_bounds.size()-1];
    }


    for (uint vc=1; vc < m_contractions.size()-1; vc++)
    {
        m_contractions[vc].contraction = m_contractions[vc-1].relaxation;
        m_contractions[vc].relaxation = m_contractions[vc+1].contraction;
    }
                                         
        m_valid = true;

  }

/*! 
  @function Constructor
  @discussion Initializes from 3 containers. Default "x-axis" is array indices.
  @discussion Data, peaks and valleys
  @todo: The 2nd form supplies "x-axis" of time stamps, etc.
*/

  bool isValid () const { return m_valid; }

    // >= 0 is thr, -1 is no change
  void operator () (int contraction_index, int ct_thr, int rl_thr)
    {
        // @todo set an error code
        if (contraction_index < 0 || contraction_index >= m_contractions.size()) return;
        contraction_window& cw = m_contractions[contraction_index];

        // Assume we have to go all the way
        uint contraction_span = cw.contraction;
        uint relaxation_span = cw.relaxation;
        
        if (ct_thr >= 0)
        {
            Elem dthr = convert_percent_thr (ct_thr);
            cw.contraction_indices.resize (0);

            // see if we can be shorter
            for (uint cf = cw.posid; cf > cw.contraction; cf--)
                if (dthr >= m_d2_data[cf])
                {
                    contraction_span = cf;
                    break;
                }
            // Stop before contraction frame
            for (uint cf = contraction_span; cf < cw.posid; cf++) cw.contraction_indices.push_back (cf);
        }

        if (rl_thr >= 0)
        {
            Elem dthr = convert_percent_thr (rl_thr);
            cw.relaxation_indices.resize (0);

            // see if we can be shorter
            for (uint cf = cw.posid; cf > cw.relaxation; cf++)
                if (dthr >= m_d2_data[cf])
                {
                    relaxation_span = cf;
                    break;
                }
            // Start at frame after contraction
            for (uint cf = cw.posid; cf < cw.relaxation;++cf) cw.relaxation_indices.push_back (cf);
        }      
        
    }

  bool isEmpty () const { return m_contractions.size () == 0; }
  uint numberOfContractions () { return m_contractions.size (); }
  
  const std::vector<contraction_window> & contractions () { return m_contractions; }
    
  void setup_threshold_quantization ()
    {
        m_d2_range = boost::minmax_element (m_d2_data.begin(), m_d2_data.end()); 
        const double& mind2 = *m_d2_range.first;
        const double& maxd2 = *m_d2_range.second;        
       
        std::cerr << mind2 << " < - > " << maxd2 << ": " << m_step_ << std::endl;
        int min_sign = boost::math::sign (mind2);
        int max_sign = boost::math::sign (maxd2);
        if (min_sign == 0 || (min_sign == max_sign && min_sign != 0 ))
            m_zero_thr = 0;
        else if (max_sign == 0)
            m_zero_thr = m_quant - 1;
        else if (min_sign < max_sign)  // min < 0 && max > 0
        {
            int min_tz = floor (-mind2 / m_step_);
            int max_tz = m_quant -1 - floor (maxd2 / m_step_ );
            std::cerr << min_tz << " < - > " << max_tz << std::endl;
        }

        m_thr_mapper.resize (m_quant);        
        for (int hh=0; hh<m_quant; hh++)
        {
            m_thr_mapper[hh] = mind2 + hh * m_step_;
        }
    }
    
    Elem convert_percent_thr (int thr)
    { 
        assert (m_valid);
        return m_thr_mapper [thr % m_quant];
    }

private:
  bool m_valid;
  int m_zero_thr;
    
  Container<Elem> m_data;
  Container<Elem> m_d2_data;
  std::vector<extrema_pos> m_valleys;
  std::vector<contraction_window> m_contractions;
  
  pair<constIterator, constIterator> m_d2_range;
  double m_step_;
  int m_quant;
  std::vector<uint> m_bounds;     
  ct_thresholder m_thr_mapper;    
};



#endif /* __RC_SIGNALFOLD_H */


