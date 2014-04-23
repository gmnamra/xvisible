
#ifndef rc_peak_cpp
#define rc_peak_cpp

#include "rc_types.h"
#include "rc_peak.h"
#include "rc_window.h"

void rfPeakDetect8Bit(const rcWindow& win, uint32 accept, list<rcPeak <uint8> >& peaks)
{
  assert (win.depth() == 1);

  // Make sure it is empty
  peaks.clear();

  const int32 height(win.height());
  const int32 width(win.width());
  const int32 rowUpdate(win.rowUpdate());
  const int32 rowSkip(win.rowUpdate() - width);
  const uint8 *pel = win.rowPointer (0);

  for(int32 j = 0; j < height; j++)
  {
    for(int32 i = 0; i < width; pel++,i++)
    {
      if(*pel >= accept &&
         *pel >= *(pel - 1) &&
         *pel >= *(pel - rowUpdate - 1) &&
         *pel >= *(pel - rowUpdate) &&
         *pel >= *(pel - rowUpdate + 1) &&
         *pel >= *(pel + 1) &&
         *pel >= *(pel + rowUpdate + 1) &&
         *pel >= *(pel + rowUpdate) &&
         *pel >= *(pel + rowUpdate - 1))
	{
	  rcIPair f (i, j);
	  peaks.push_front(rcPeak<uint8> (*pel, f));
	}
    }
    pel += rowSkip;    
  }
  peaks.sort();
  peaks.reverse(); // Best Peaks First
}

void rfPeakDetectAsym8Bit (const rcWindow& win, int16 accept, list<rcPeak<uint8> >& peaks)
{
  // make sure it is empty
  peaks.clear();

  const int32 rowUpdate(win.rowUpdate());
  const int32 height(win.height());
  const int32 width(win.width());
  const int32 rowSkip(win.rowUpdate() - width);
  const uint8 *pel = win.rowPointer (0);

  for(int32 j = 0; j < height; j++)
  {
    for(int32 i = 0; i < width; i++)
    {
      if(*pel >= accept &&
         *pel > *(pel - 1) &&
         *pel > *(pel - rowUpdate - 1) &&
         *pel > *(pel - rowUpdate) &&
         *pel > *(pel - rowUpdate + 1) &&
         *pel >= *(pel + 1) &&
         *pel >= *(pel + rowUpdate + 1) &&
         *pel >= *(pel + rowUpdate) &&
         *pel >= *(pel + rowUpdate - 1))
	{
	  rcIPair f (i, j);
	  peaks.push_front(rcPeak<uint8> (*pel, f));
	}
    }
    pel += rowSkip;
  }
  peaks.sort();
  peaks.reverse(); // Results are in descending order
}

#endif // rc_peak_cpp
