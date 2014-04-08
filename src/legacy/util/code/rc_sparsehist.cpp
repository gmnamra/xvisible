// Copyright 2002 Reify, Inc.

#include "rc_sparsehist.h"

rcSparseHistogram::rcSparseHistogram(uint32 maxBins) :
  _maxBins(maxBins), _elementCount(0), _weightedCount(0), _isValid(true)
{
}

bool rcSparseHistogram::add(uint32 slot)
{
  sparseArray::iterator index = _bins.find(slot);

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

double rcSparseHistogram::average() const
{
  if (_bins.size() == 0)
    return -1;

  return _weightedCount/_elementCount;
}

void rcSparseHistogram::range(uint32& min, uint32& max) const
{
  if (_bins.size() == 0)
    return;

  min = (*_bins.begin()).first;
  max = (*_bins.rbegin()).first;
}

void rcSparseHistogram::reset()
{
  _elementCount = _weightedCount = 0;
  _isValid = true;

  _bins.erase(_bins.begin(), _bins.end());
}
