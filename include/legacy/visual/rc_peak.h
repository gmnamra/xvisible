#ifndef _rcPEAK_H_
#define _rcPEAK_H_

#include <list>
#include <rc_window.h>
#include <rc_vector2d.h>
#include <bitset>

template <typename T> 
class rcPeak
{
public:
  typedef T PeakType;

  rcPeak() : _location (0,0), _value ((T) 0), _interp (0.0f, 0.0f)  {};

  rcPeak(T& v,const rcIPair& l)
	: _value (v), _location(l)
	{
	}
	
  rcPeak(T v,const rcIPair& l)
	: _value (v), _location(l)
	{
	}
	
  rcPeak(T v,const rcIPair& l, const rc2Fvector& interp, bool low_x_on_edge, bool high_x_on_edge, bool low_y_on_edge, bool high_y_on_edge)
	: _value (v), _location(l), _interp (interp)
	{
		_edge[0] = low_x_on_edge;
		_edge[1] = high_x_on_edge;
		_edge[2] = low_y_on_edge;
		_edge[3] = high_y_on_edge;
	}
	
	const T& score() const { return _value; }
        void score (T sc )  { _value = sc;  }
	const rc2Fvector& location ()  const { return _interp; }
	const rc2Fvector& interpolated () const { return _interp; }
	const bitset<4>& edges () const { return _edge; }

	void interpolated (rc2Fvector& itp) { _interp = itp; }
	void location (rc2Fvector& itp) { _interp = itp; }
	void edges (bitset<4>& ned) { _edge = ned; }

  // Output Functions
  friend ostream& operator<< (ostream& o, const rcPeak& p)
  {
    o << p.location << " , " << (int32) p.value << " , " << p.interp << "," << p.score;
    return o;
  }

  bool operator<(const rcPeak& rhs) const
  {return score () < rhs.score ();}
  bool operator==(const rcPeak& rhs) const
    {return score () == rhs.score ();}
  bool operator!=(const rcPeak& rhs) const;
  bool operator>(const rcPeak& rhs) const;
	
private:
  rcIPair _location;
  rc2Fvector _interp;
  T _value;
  bitset<4> _edge;
	
};


template <class T,class S>
  void rfDetectPeaks(const rcWindow& pb,T accept, list<rcPeak<S> >& peaks)
{
  if (!peaks.empty()) peaks.clear();

  const int32 height(pb.height());
  const int32 width(pb.width());
  const int32 rowUpdate(pb.rowUpdate() / sizeof (T));
  const int32 twoRowUpdate(rowUpdate+rowUpdate);
  rmAssert (sizeof(T) == pb.depth());

  for(int32 j = 2; j < (height-2); j++)
  {
    // Start on the 2, 2 pixel since we are testing both 3x3 and 5x5
    const T *pel = (T *) pb.pelPointer (2, j);
    for(int32 i = 2; i < width-2; pel++,i++)
    {
      T pv (*pel);

      if(pv < accept ||
         pv < *(pel - 1) ||
         pv < *(pel - rowUpdate - 1) ||
         pv < *(pel - rowUpdate) ||
         pv < *(pel - rowUpdate + 1) ||
         pv <= *(pel + 1) ||
         pv <= *(pel + rowUpdate + 1) ||
         pv <= *(pel + rowUpdate) ||
         pv <= *(pel + rowUpdate - 1))
	continue;

      if(pv < *(pel - 2) ||
         pv < *(pel - twoRowUpdate - 2) ||
         pv < *(pel - twoRowUpdate) ||
         pv < *(pel - twoRowUpdate + 2) ||
         pv <= *(pel + 2) ||
         pv <= *(pel + twoRowUpdate + 2) ||
         pv <= *(pel + twoRowUpdate) ||
         pv <= *(pel + twoRowUpdate - 2))
	continue;

      // We have a peak
      peaks.push_front(rcPeak<S>(pv,rcIPair(i,j)));
    }
  }

  // Sort and put the highest first
  peaks.sort();
  peaks.reverse(); 
}

void rfPeakDetectAsym8Bit (const rcWindow& win, uint32 accept, list<rcPeak<int16> >& peaks);
void rfPeakDetect8Bit(const rcWindow& win, uint32 accept, list<rcPeak<int16> >& peaks);

#endif /* _rcPEAK_H_ */

