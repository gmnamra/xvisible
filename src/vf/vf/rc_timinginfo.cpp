// Copyright 2002 Reify, Inc.

#include "rc_types.h"
#include "rc_timinginfo.h"

#include <stdio.h>

bool rcTimingInfo::mPrintOff = false;

rcTimingInfo::rcTimingInfo(uint32 locations, uint32 passes)
  : _curPassLoc(0)
{
  rmAssert(locations > 1);
  rmAssert(passes);

  _times.resize(locations);
  _labels.resize(locations);
  _deltas.resize(passes);
  for (vector<vector<double> >::iterator i = _deltas.begin(); i != _deltas.end(); i++) {
    i->resize(locations);
    for (uint32 j = 0; j < i->size(); j++)
      (*i)[j] = -1;
  }

  for (uint32 i = 0; i < locations; i++)
    _labels[i] = std::string ("");

  _firstValidPass = _curPass = _deltas.begin();
  _resetTime = rcTimestamp::now();
}

void rcTimingInfo::touch(uint32 location, std::string label)
{
  touch (location);
  _labels[location] = label;
}

void rcTimingInfo::touch(uint32 location)
{
  rmAssert(location < _times.size());

  if (location < _curPassLoc)
    return;

  _times[location] = rcTimestamp::now();
  if (location)
    (*_curPass)[location] = _times[location].secs() - _times[_curPassLoc-1].secs();
  else
    (*_curPass)[0] = _times[location].secs() - _resetTime.secs();

  for ( ; _curPassLoc < location; _curPassLoc++) {
    (*_curPass)[_curPassLoc] = -1;
  }
  
  _curPassLoc++;
}

void rcTimingInfo::nextPass(bool reset)
{
  _curPassLoc = 0;

  if (++_curPass == _deltas.end())
    _curPass = _deltas.begin();

  /* Note: Its not necessary to set the iterators back to _deltas.begin() in
   * the reset case, but it makes for less testing.
   */
  if (reset)
    _firstValidPass = _curPass = _deltas.begin();
  else if (_curPass == _firstValidPass) {
    if (++_firstValidPass == _deltas.end())
      _firstValidPass = _deltas.begin();
  }
  _resetTime = rcTimestamp::now();
}

void rcTimingInfo::printInfo(uint32 passes)
{
  vector<std::string> dummy;

  printInfo(passes, dummy);
}

void rcTimingInfo::printInfo(uint32 passes, vector<std::string>& labels)
{
  if (!mPrintOff)
    {
  fprintf(stderr, "Timing Info Dump\nRequested passes: %d\n\n", passes);

  vector<vector<double> >::iterator prtPass = _firstValidPass;

  for (uint32 i = 0; i < passes; i++) {
    uint32 endLoc = (*prtPass).size();
    if (prtPass == _curPass)
      endLoc = _curPassLoc;
    
    fprintf(stderr, " Pass %02d Index   Time\n", i);

    for (uint32 j = 0; j < endLoc; j++) {
      const char* label;
      if (labels.size() > j)
	label = labels[j].c_str();
      else
	label = _labels[j].c_str();

      if ((*prtPass)[j] < 0)
	fprintf(stderr,"         %02d       ------ %s\n", j, label);
      else if ((*prtPass)[j] < 1e-3)
	fprintf(stderr,"         %02d       %3.0f us %s\n", j, (*prtPass)[j]*(1e6), label);
      else if ((*prtPass)[j] < 1)
	fprintf(stderr,"         %02d       %3.0f ms %s *\n", j, (*prtPass)[j]*(1e3), label);
      else
	fprintf(stderr,"         %02d       %3.0f s %s **\n", j, (*prtPass)[j], label);
    }
    fprintf(stderr, "\n");

    if (prtPass == _curPass)
      break;
    if (++prtPass == _deltas.end())
      prtPass = _deltas.begin();
  }
    }
}
