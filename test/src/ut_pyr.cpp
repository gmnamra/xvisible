/*
 *
 *$Id $
 *$Log$
 *Revision 1.8  2005/11/17 23:19:06  arman
 *kinetoscope resync
 *
 *Revision 1.7  2004/08/24 21:38:15  arman
 **** empty log message ***
 *
 *Revision 1.6  2004/08/24 16:11:29  arman
 *added and updated tests.
 *
 *Revision 1.5  2004/08/24 15:25:15  arman
 *added copy test
 *
 *Revision 1.4  2004/08/19 17:02:40  arman
 *fixed up and ready to go!
 *
 *Revision 1.3  2004/08/19 16:07:34  arman
 *added test for transformation
 *
 *Revision 1.2  2004/07/13 21:06:37  arman
 *added ut
 *
 *Revision 1.1  2004/07/12 19:49:18  arman
 *added kine pyramid tests
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_pyr.h"
#include <algorithm>
#include  <numeric>
#include <rc_mathmodel.h>
#include <rc_histstats.h>

UT_pyramid::UT_pyramid()
{
}

UT_pyramid::~UT_pyramid()
{
        printSuccessMessage( "Kinetic Pyramid Tests", mErrors );
}

uint32 UT_pyramid::run()
{
  // Test different sizes
  {        
    test ();
  }

  return mErrors;
}



void UT_pyramid::test ()
{

  // Create a buffer for testing at all shifts for src and dst
  int32 width = 32;
  int32 height = 32;
  rcWindow src (width, height);

  // create an image set them to identity
  for (int32 i = 0; i < src.width(); i++)
    {
      for (int32 j = 0; j < src.height(); j++)
	{
	  src.setPixel (i, j, (j*src.width() + i) % 255);
	}
    }

  rc2LevelSpatialPyramid pyr;
  rcUTCheck (!pyr.isValid ());

  // Default setting does edge detection
  rc2LevelSpatialPyramid pyr2 (src, 2, 5, 0.0f);
  rcUTCheck (pyr2.isValid ());
  rcUTCheck (pyr2.hysteresis() == 0.0f);
  rcUTCheck (pyr2.granularity() == 5);
  
  rcUTCheck (pyr2.base(0).isBound());
  rcUTCheck (pyr2.past(0).isBound());
  rcUTCheck (!pyr2.gradient(0).isBound());
  rcUTCheck (!pyr2.direction (0).isBound());
  rcUTCheck (pyr2.base(1).isBound());
  rcUTCheck (pyr2.past(1).isBound());
  rcUTCheck (!pyr2.gradient(1).isBound());
  rcUTCheck (!pyr2.direction (1).isBound());

  rc2Dvector lowResPoint (8.0, 8.0);
  rcUTCheck (pyr2.mapPoint (lowResPoint).x() == 16.0);
  rcUTCheck (pyr2.mapPoint (lowResPoint).y() == 16.0);

  rc2LevelSpatialPyramid pyr3 (src, 2, 5, 1.0, 5.0f);
  rcUTCheck (pyr3.isValid ());  
  rcUTCheck (pyr3.base(0).isBound());
  rcUTCheck (pyr3.past(0).isBound());
  rcUTCheck (pyr3.gradient(0).isBound());
  rcUTCheck (pyr3.edge (0).isBound());
  rcUTCheck (pyr3.direction (0).isBound());
  rcUTCheck (pyr3.base(1).isBound());
  rcUTCheck (pyr3.past(1).isBound());
  rcUTCheck (pyr3.gradient(1).isBound());
  rcUTCheck (pyr3.edge (1).isBound());
  rcUTCheck (pyr3.direction (1).isBound());

#if 0
  cerr << src << endl;
  cerr << pyr3.base(0) << endl;
  cerr << pyr3.base(1) << endl;
  cerr << pyr3.gradient(1) << endl;
  cerr << pyr3.edge(1) << endl;
#endif

  // test default copy
  rc2LevelSpatialPyramid pyrcpy;
  rcUTCheck (!pyrcpy.isValid ());
  pyrcpy = pyr3;
  rcUTCheck (pyrcpy.isValid ());  
  rcUTCheck (pyrcpy.base(0).isBound());
  rcUTCheck (pyrcpy.past(0).isBound());
  rcUTCheck (pyrcpy.gradient(0).isBound());
  rcUTCheck (pyrcpy.edge (0).isBound());
  rcUTCheck (pyrcpy.direction (0).isBound());
  rcUTCheck (pyrcpy.base(1).isBound());
  rcUTCheck (pyrcpy.past(1).isBound());
  rcUTCheck (pyrcpy.gradient(1).isBound());
  rcUTCheck (pyrcpy.edge (1).isBound());
  rcUTCheck (pyrcpy.direction (1).isBound());

  vector<rc2LevelSpatialPyramid> levels (2);
  levels[0] = pyr3;
  levels[1] = pyr2;
  rcUTCheck (levels[0].edge (1).isBound());  
  rcUTCheck (!levels[1].edge (1).isBound()); 

  rotate (levels.begin(), levels.end() - 1, levels.end());
  rcUTCheck (!levels[0].edge (1).isBound());  
  rcUTCheck (levels[1].edge (1).isBound()); 

// test backGroundMaskGeneration  
	{
		rcWindow buf (32, 32, rcPixel8);
		rcMathematicalImage g;
		g.gauss (buf, rcMathematicalImage::eHiPeak);
		rc2LevelSpatialPyramid pyr (buf, 2, 5, 1.0, 5.0f);
		rcWindow mask;
			//	pyr.makeBackGroundMask (mask, 0);
		for (uint32 i = 0; i < 5; i++)
		{
			rcUTCheck (mask.getPixel (14 + i, 13) == 255);			
			rcUTCheck (mask.getPixel (14 + i, 19) == 255);						
		}
		for (uint32 j = 0; j < 5; j++)
		  for (uint32 i = 0; i < 7; i++)
		{
			rcUTCheck (mask.getPixel (13+i, 14 + j) == 255);						
		}
		rcHistoStats h (mask);
		rcUTCheck (h.histogram()[255] == 45);
		rcUTCheck (h.histogram()[0] == (32 * 32 - 45));
	}

}

