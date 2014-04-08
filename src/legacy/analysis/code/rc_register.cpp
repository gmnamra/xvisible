/*
 *
 *$Id $
 *$Log: $
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_register.h>
#include <rc_rowfunc.h>


void rfRegisteredWindows (vector<rcWindow>& focusImages, int32& range,  
			  enum rcImageSetRegister<rcWindowVectorIter>::registerTo option,
			  bool debugOn)

{
  static const int32 dummy (0);
  rcIPair r (range);
  vector<rc2Fvector> peakPositions;
  typedef vector<rcWindow>::iterator iterator;
  rcImageSetRegister<iterator> whole (focusImages.begin(), focusImages.end());
  rcWindowVectorIter mItr = focusImages.begin (); 
  whole.setRegister  ( mItr , r);
  vector<rc2Fvector>::iterator oItr = whole.sequential2Dmoves().begin();
  vector<rcWindow>::iterator wItr = focusImages.begin();
  for (; wItr !=  focusImages.end() && oItr != whole.sequential2Dmoves().end(); wItr++,  oItr++)
    {
      rcIPair trans (rfRound (range + wItr->x() + oItr->x(), dummy), 
		     rfRound (range + wItr->y() + oItr->y(), dummy));
      if (debugOn)
	{
	  cerr << "f: " << wItr->position () << "w: " << *oItr << " m: " << trans.x() << "," << trans.y();
	}
      *wItr = rcWindow (wItr->frameBuf(), trans.x(), trans.y(), whole.size().x(), whole.size().y());
      if (debugOn)
	{
	  cerr << "w: " << wItr->position () << endl;
	}
    }
}


