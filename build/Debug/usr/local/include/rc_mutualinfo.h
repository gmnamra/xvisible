/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/03/11 22:15:50  arman
 *basic mi placeholder
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcMUTUALINFO_H
#define __rcMUTUALINFO_H

class rcMutualInfo
{
 public:

  rcMutualInfo ()
  {
    mMi = rcWindow (256, 256, rcPixel32);
    mMi.setAllPixels (0);
  }

  
  void add (const rcWindow& a, const rcWindow& b)
  {
    rmAssert (a.size() == b.size());
    rcIPair largeAoverB (-1,-1);
    rcIPair largeBoverA (-1,-1);

    for (int32 j = 0; j < a.height(); j++)
      {
	const uint8* aPtr = a.rowPointer (j);
	const uint8* bPtr = b.rowPointer (j);

	for (int32 i = 0; i < a.width(); i++)
	  {
	    int32 ai = (*aPtr++);
	    int32 bi = (*bPtr++);
	    add (ai, bi);
	    if (ai > bi && ai != largeAoverB.x() && bi != largeAoverB.y())
	      largeAoverB = rcIPair (ai,bi);
	    if (bi > ai && ai != largeBoverA.x() && bi != largeBoverA.y())
	      largeBoverA = rcIPair (ai,bi);
	  }
      }
  }

  void add (int32 a, int32 b)
  {
    uint32 p = mMi.getPixel (a, b);
    p += 1;
    mMi.setPixel (a, b, p);
  }

  const rcWindow& hist () const
  {
    return mMi;
  }

 // Output Functions
  friend ostream& operator<< (ostream& o, const rcMutualInfo& mu)
  {
    o << "{";
    for (int32 j = 0; j < mu.hist().height(); j++)
    {
      o << "{";
      for (int32 i = 0; i < mu.hist().width(); i++)
	{
	  o <<  mu.hist().getPixel (i, j);
	  if (i < mu.hist().width() - 1) 
	    o << ",";
	} 
      o << "}";
      if (j < mu.hist().height() - 1) 
	o << ",";
      o << endl;
    }
    o << "};" << endl;

    return o;
  }

    

 private:
  rcWindow mMi;
  rcWindow mRatio;
};

#endif /* __rcMUTUALINFO_H */
