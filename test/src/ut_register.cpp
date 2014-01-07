/*
 *
 *$Id $
 *$Log$
 *Revision 1.3  2006/01/01 21:30:58  arman
 *kinetoscope ut
 *
 *Revision 1.2  2005/12/04 19:41:27  arman
 *register unit test plus lattice bug fix
 *
 *Revision 1.1  2005/10/24 02:09:04  arman
 *ut for register
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_register.h"
#include <rc_ip.h>
static float sEpsilon = 1.e-4;

#define UTCHECKVEC(a, b, c, d) {				\
    rcUTCheck (rfRealEq((a).x(), (b), d));	\
    rcUTCheck (rfRealEq((a).y(), (c), d)); }

UT_register::UT_register()
{
}

UT_register::~UT_register()
{
  printSuccessMessage( "rcregister test", mErrors );
}

uint32
UT_register::run()
{
  // Create a a known image sequence with shifts
  vector <rcWindow> iseq;

  vector<rc2Dvector> ds;
  int32 width (731), height (137);
  rcIPair dias  (width / 4, height / 4);
  rc2Dvector ctrs (width / 2.0, height / 2.0);
  rcWindow tmp (width, height);
  rcWindow tmp2 (width, height);
  double time = 0.0;

  for (int32 j = -1; j < 2; j++)
    for (int32 i = -1; i < 2; i++)
      {
	tmp.setAllPixels (128);
	rcWindow r (tmp, i*2 + int32 (ctrs.x() - dias.x() / 2.0) , j*2 + int32 (ctrs.y() - dias.y() / 2.0), 
		    dias.x(), dias.y());
	r.setAllPixels ((uint32) 32);
	rfGaussianConv (tmp, tmp2, 7);
	rcWindow celld = rfPixelSample (tmp2, 2, 2);
	celld.frameBuf()->setTimestamp (rcTimestamp (time));
	time += 0.033;
	iseq.push_back (celld);
      }

  {
    rcIPair range (6, 6);
    typedef vector<rcWindow>::iterator iterator;
  
    rcImageSetRegister<iterator> whole (iseq.begin(), iseq.end());
    whole.setRegister  (iseq.begin(), range);
   	const vector<rc2Fvector>&  winOffsetsToFirst = whole.sequential2Dmoves();        // Offsets to the first window

    UTCHECKVEC(winOffsetsToFirst[0], 0.0f, 0.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[1], 1.0f, 0.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[2], 2.0f, 0.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[3], 0.0f, 1.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[4], 1.0f, 1.0f, sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[5], 2.0f, 1.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[6], 0.0f, 2.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[7], 1.0f, 2.0f, sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[8], 2.0f, 2.0f,sEpsilon);
  }


  {
    // Create windows in to seq that allow pick outside
    rcIPair range (6, 6);
    typedef vector<rcWindow>::iterator iterator;
    vector<rcWindow> wseq;
    vector<rcWindow>::iterator seqi = iseq.begin();
    for (; seqi < iseq.end(); seqi++)
      {
	wseq.push_back (rcWindow (*seqi, range.x()+1, range.y()+1, 
				  seqi->width() - 2 * (range.x() + 1), 
				  seqi->height() - 2 * (range.y() + 1)));
      }

    rcImageSetRegister<iterator> whole (wseq.begin(), wseq.end());
    whole.setRegister  (wseq.begin(), range);
 	const vector<rc2Fvector>&  winOffsetsToFirst = whole.sequential2Dmoves();        // Offsets to the first window

    UTCHECKVEC(winOffsetsToFirst[0], 0.0f, 0.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[1], 1.0f, 0.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[2], 2.0f, 0.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[3], 0.0f, 1.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[4], 1.0f, 1.0f, sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[5], 2.0f, 1.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[6], 0.0f, 2.0f,sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[7], 1.0f, 2.0f, sEpsilon);
    UTCHECKVEC(winOffsetsToFirst[8], 2.0f, 2.0f,sEpsilon);
  }

}
