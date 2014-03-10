/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/05/08 13:15:51  arman
 *initial version
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_CONTRACTION_SIGNAL_H
#define __RC_CONTRACTION_SIGNAL_H

#include <vector>
#include <algorithm>
#include <rc_signal1d.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/math/special)_functions/sign.hpp>

#include <rc_stlplus.h>


struct contraction_window 
{
    uint id;
    uint posid;
    float value;
    std::vector<uint> contraction_indices;
    std::vector<uint> relaxation_indices;          
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
    typedef LinearSelector<int, Elem> Mapper;
    
/*! 
  @Default Constructor
*/
    ContractionSignal ( int N = 100 )
    : m_valid (false), m_quant (N), m_last_thr (-1)
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
        m_valid = true;
    }  


/*! 
  @function Constructor
  @discussion Initializes from 3 containers. Default "x-axis" is array indices.
  @discussion Data, peaks and valleys
  @todo: The 2nd form supplies "x-axis" of time stamps, etc.
*/

  bool isValid () const { return m_valid; }
  
  void operator () (int thr)
  {
      if (m_last_thr > 0 && thr == m_last_thr) return;
      
      Elem dthr = convert_percent_thr (thr);
      
     // initialize a contraction for every valley
    m_contractions.resize (m_valleys.size ());
    for (uint vc=0; vc < m_valleys.size(); vc++)
      {
        m_contractions[vc].id = vc;
        m_contractions[vc].posid = m_valleys[vc].first;
        m_contractions[vc].value = m_data[m_valleys[vc].second];
        m_contractions[vc].state = 0;
      }
      m_last_thr = thr;
    m_valid = true;
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
        min_sign = boost::math::sign (mind2);
        max_sign = boost::math::sign (maxd2);
        if (min_sign == 0 || (min_sign == max_sign && min_sign != 0 ))
            m_zero_thr = 0;
        else if (max_sign == 0)
            m_zero_thr = m_quant - 1;
        else if (min_sign < max_sign)  // min < 0 && max > 0
        {
            int min_tz = floor (-mind2 / m_step_);
            int max_tz = m_qunat -1 - floor (maxd2 / m_step_ );
            std::cerr << mintz << " < - > " << maxtz << std::endl;
        }
    }
    
    Elem convert_percent_thr (int thr)
    { 
        assert (m_valid);
        Mapper mapper (m_step_, *m_d2_range.first);        
        return mapper.operator () ( thr % m_quant );
    }        

private:
  bool m_valid;
  int m_last_thr;
    int m_zero_thr;
    
  Container<Elem> m_data;
  Container<Elem> m_d2_data;
    std::vector<extrema_pos> m_valleys;
    std::vector<contraction_window> m_contractions;
    
    pair<constIterator, constIterator> m_d2_range;
    double m_step_;
    int m_quant;
};



#endif /* __RC_SIGNALFOLD_H */


