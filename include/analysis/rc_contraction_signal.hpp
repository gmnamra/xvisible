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


  
/*! 
  @Default Constructor
*/
    ContractionSignal () { m_valid = false; }


/*! 
  @function Constructor
  @discussion Initializes from 3 containers. Default "x-axis" is array indices.
  @discussion Data, peaks and valleys
  @todo: The 2nd form supplies "x-axis" of time stamps, etc.
*/

  bool isValid () const { return m_valid; }
  
  void operator () (Container<Elem>& d, std::vector<extrema_pos>& p, std::vector<extrema_pos>& v)
  {
    m_data.resize (d.size () );
    std::copy (d.begin(), d.end(), m_data.begin ());
    m_peaks.resize (d.size () );
    std::copy (p.begin(), p.end(), m_peaks.begin());
    m_valleys.resize (d.size () );
    std::copy (v.begin(), v.end(), m_valleys.begin());

    // initialize a contraction for every valley
    m_contractions.resize (v.size ());
    for (uint vc=0; vc < v.size(); vc++)
      {
        m_contractions[vc].id = vc;
        m_contractions[vc].posid = m_valleys[vc].first;
        m_contractions[vc].value = m_data[m_valleys[vc].second];
        m_contractions[vc].state = 0;
      }
    m_valid = true;
  }

  bool isEmpty () const { return m_contractions.size () == 0; }
  uint numberOfContractions () { return m_contractions.size (); }
  
    const std::vector<contraction_window> & contractions () { return m_contractions; }
  

private:
  bool m_valid;
  Container<Elem> m_data;
    std::vector<extrema_pos> m_peaks;
    std::vector<extrema_pos> m_valleys;
    std::vector<contraction_window> m_contractions;

};




#endif /* __RC_SIGNALFOLD_H */


