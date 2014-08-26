
#ifndef _sshist_H_
#define _sshist_H_

#include "vf_types.h"
#include <map>

/* sshist - Allows users to create a "bounded" sparse
 * histogram. By bounded is meant that the maximum number of bins to
 * be used can be specified in at object construction time. The intent
 * was to provide a reasonably fast, memory efficient, way of
 * generating a sparse histogram. This was done using STL maps and
 * this class is mostly a thin wrapper around map functionality.
 *
 * Note: If a way could be devised to guarantee that all the memory
 * required will be allocated a construction time this would be a
 * plus. This class is being used during real time operations and
 * anything that can prevent the need to allocate heap would be a
 * help. STL maps don't seem to provide a way of directly specifying
 * how many elements you will be using. Maybe elements could be
 * preallocated by creating a map that has the requested number of
 * elements, using keys that are out of range. When a new bin is
 * needed one of these preallocated entries could be taken and its key
 * modified. Whether or not maps would allow this sort of scheme needs
 * to be investigated.
 */

template <typename T1, typename T2>
struct greater_second {
    typedef std::pair<T1, T2> type;
    bool operator ()(type const& a, type const& b) const {
        return a.second > b.second;
    }
};

class sshist
{
public:

    typedef std::pair<uint32,uint32> pair_t;
    typedef std::map<pair_t::first_type, pair_t::second_type> container_t;
    typedef std::vector<pair_t> sorted_container_t;
    
  /* Create a sparse histogram which will use at most maxBins bins. If
   * maxBins is 0, then there is no limit on the number of bins that
   * can be used.
   */
  sshist(uint32 maxBins = 0)
    : _maxBins(maxBins), _elementCount(0), _weightedCount(0), _isValid(true)
    {
    }


  /* Increments the count in the bin specified by slot and returns
   * true. If performing this operation would require creating more
   * than the maximum number of allowed bins, the operation is not
   * performed and false is returned.
   */
  bool add(uint32 slot)
    {
        container_t::iterator index = _bins.find(slot);
        
        if (index == _bins.end())
        {
            if (_maxBins && (_bins.size() == _maxBins))
            {
                _isValid = false;
                return false;
            }
            
            _bins[slot] = 0;
            index = _bins.find(slot);
            assert(index != _bins.end());
        }
        
        (*index).second++;
        
        _elementCount += 1;
        _weightedCount += slot;
        
        return true;
    }


  /* Returns true if all increments completed successfully, otherwise
   * it returns false.
   */
  bool isValid() const { return _isValid; }

  /* Returns the average slot value passed to add(). Return value undefined
   * if add() has never been called.
   */
  double average() const
    {
        if (_bins.size() == 0)
            return -1;
        
        return _weightedCount/_elementCount;
    }


  /* Returns the number of times add() has been called.
   */
  double count() const { return _elementCount; }

  /* Returns the smallest and the largest slot values passed to
   * add(). Return value undefined if add() has never been called.
   */
  void range(uint32& min, uint32& max) const
    {
        if (_bins.size() == 0)
            return;
        
        min = (*_bins.begin()).first;
        max = (*_bins.rbegin()).first;
    }


  /* Returns the number of bins required.
   */
  size_t binsUsed() const { return _bins.size(); }

  /* Clears out the histogram and any associated statistics.
   */
  void reset()
    {
        _elementCount = _weightedCount = 0;
        _isValid = true;
        
        _bins.erase(_bins.begin(), _bins.end());
    }
    


  const container_t& getArray() const { return _bins; }
    
  void get_value_sorted (sorted_container_t& sorted)
    {
        std::vector<pair_t> binscopy(_bins.begin(), _bins.end());
        sort(binscopy.begin(), binscopy.end(), greater_second<pair_t::first_type,pair_t::second_type>());
        sorted = binscopy;        
    }
    

private:

  container_t   _bins;
  uint32      _maxBins;
  double        _elementCount;
  double        _weightedCount;
  bool          _isValid;
};

#endif // _sshist_H_
