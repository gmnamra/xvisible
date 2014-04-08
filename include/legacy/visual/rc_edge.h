
#ifndef _rcEDGE_H_
#define _rcEDGE_H_

#include <vector>
using namespace std;

#include <rc_types.h>
#include <rc_window.h>
#include <rc_pair.h>


uint32 rfSpatialEdge (rcWindow& magImage, rcWindow& angleImage, rcWindow& peaks, uint8 threshold, bool angleLabel = false);
void rfSobelEdge (const rcWindow& image, rcWindow& magnitudes,rcWindow& angles);
void rfSobel (const rcWindow& image, rcWindow& magnitudes,rcWindow& angles, bool highPrecAngle = false);
void rfDirectionSignal (const rcWindow& image, const rcWindow& mask, vector<double>& signal);

template<class T>
class rcGradientDir
{
  enum {size = 8};

public:
    // ctor
    rcGradientDir()
  {
    mTable[0] = rcPair<T> ((T)-1,(T) 0);
    mTable[1] = rcPair<T> ((T)-1,(T) -1);
    mTable[2] = rcPair<T> ((T)0,(T) -1);
    mTable[3] = rcPair<T> ((T)+1,(T) -1);
    mTable[4] = rcPair<T> ((T)+1,(T) 0);
    mTable[5] = rcPair<T> ((T)+1,(T) +1);
    mTable[6] = rcPair<T> ((T)0,(T) +1);
    mTable[7] = rcPair<T> ((T)-1,(T) +1);

  };
    
  const rcPair<T>& grad (int32 index) const
  {
    rmAssert (index >= 0 && index < 8);
    return mTable[index];
  }

 

private:
    rcPair<T> mTable[size];
};

#endif // _rcEDGE_H_

