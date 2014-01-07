/*
 *
 *$Id $
 *$Log$
 *Revision 1.2  2005/05/01 19:37:40  arman
 *added ut for newly added ellipse functionality
 *
 *Revision 1.1  2005/04/23 17:53:25  arman
 *initial
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_gshape.h"

UT_ellipse::UT_ellipse ()
{}

uint32 UT_ellipse::run ()
{
  test();

  return mErrors;
}


UT_ellipse::~UT_ellipse ()
{
  printSuccessMessage( "rcEllipse test", mErrors );  
}

void UT_ellipse::test()
{
  {
    rcEllipse e;
    rcUTCheck (e.degenerate ());
  }

  {
    rc2Dvector c (10.0, 12.0);
    rc2Dvector r (5.0, 10.0);
    rcRadian o (0.0);
    rcEllipse e (c, r, o);
    rcUTCheck (!e.degenerate ());

    rc2Dvector p (10.0 + r.x(), 12.0);
    rcUTCheck (e.contains (p));
  }

  {
    // Box of 33 x 21
    double width (63.0), height (21.0);
    rc2Dvector c (width/2, height/2);
    rc2Dvector r (width/6, height/6);
    rcRadian o (-rkPI/4.0);
    rcEllipse e (c, r, o);
    rcUTCheck (!e.degenerate ());
    rcDRect db = e.boundingOrthRectangle ();
    double epsilon (0.0001);
    rcUTCheckRealDelta (db.ul().x() , 23.6738, epsilon );
    rcUTCheckRealDelta (db.ul().y() , 2.67376, epsilon );
    rcUTCheckRealDelta (db.lr().x() , 39.3262, epsilon );
    rcUTCheckRealDelta (db.lr().y() , 18.3262, epsilon );

    cerr << db << endl;

    for (uint32 j = 0; j < (uint32) height ; j++)
      {
	cerr << endl;
	for (uint32 i = 0; i < (uint32) width ; i++)
	  {
	    rc2Dvector p ((double) i, (double) j);
	    if (e.contains (p))
	      cerr << "*";
	    else
	      cerr << ".";
	  }
      }
    cerr << endl;
  }
}


