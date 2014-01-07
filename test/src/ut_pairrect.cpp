/*
 *
 *$Id $
 *$Log$
 *Revision 1.3  2005/08/30 21:08:51  arman
 *Cell Lineage
 *
 *Revision 1.3  2005/07/30 06:06:50  arman
 * added roundRectangle
 *
 *Revision 1.2  2003/02/23 22:31:10  arman
 *added ut support for boundingBox
 *
 *Revision 1.1  2002/12/08 22:55:48  arman
 *pair and rect unit tests
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include <rc_rectangle.h>
#include <rc_pair.h>
#include "ut_pairrect.h"
#include <rc_vector2d.h>
#include <deque>


static int copair(const rcIPair& p1, const rcIPair& p2)
{
  return( p1.x()==p2.x() && p1.y()==p2.y() );
}


UT_pair::UT_pair ()
{}

uint32 UT_pair::run ()
{
  test();

  return mErrors;
}


UT_pair::~UT_pair ()
{
  printSuccessMessage( "rcPair test", mErrors );  
}

UT_rect::UT_rect ()
{}

UT_rect::~UT_rect ()
{
  printSuccessMessage( "rcRect test", mErrors );  
}

uint32 UT_rect::run ()
{
  test();

  return mErrors;
}

void UT_pair::test ()
{
  {
    rcIPair ip1(1, 4);
    rcIPair ip2(3, 2);

    rcIPair ip3 = ip1 + ip2;
    rcUNITTEST_ASSERT(ip3.x() == 4);
    rcUNITTEST_ASSERT(ip3.y() == 6);

    ip3 += ip2;
    rcUNITTEST_ASSERT(ip3.x() == 7);
    rcUNITTEST_ASSERT(ip3.y() == 8);

    rcIPair ip4 = -ip1;
    rcUNITTEST_ASSERT(ip4.x() == -1);
    rcUNITTEST_ASSERT(ip4.y() == -4);

    ip4 = ip1 - ip2;
    rcUNITTEST_ASSERT(ip4.x() == -2);
    rcUNITTEST_ASSERT(ip4.y() == 2);

    ip3 -= ip2;
    rcUNITTEST_ASSERT(ip3.x() == 4);
    rcUNITTEST_ASSERT(ip3.y() == 6);
  }

  {

    rcIPair ip1(1, 2);
    rcIPair ip2(3, 4);

    rcIPair ip3 = ip1 * ip2;
    rcUNITTEST_ASSERT(ip3.x() == 3);
    rcUNITTEST_ASSERT(ip3.y() == 8);

    ip3 *= ip2;
    rcUNITTEST_ASSERT(ip3.x() == 9);
    rcUNITTEST_ASSERT(ip3.y() == 32);
  }

  {
    rcIPair ip1(1, 2);
    rcIPair ip2(3, 4);

    rcIPair ip3 = ip2 / ip1;
    rcUNITTEST_ASSERT(ip3.x() == 3);
    rcUNITTEST_ASSERT(ip3.y() == 2);

    ip3 /= ip1;
    rcUNITTEST_ASSERT(ip3.x() == 3);
    rcUNITTEST_ASSERT(ip3.y() == 1);

  }

  {
    rcIPair ip1(1, 2);
    rcIPair ip2(3, 4);

    rcUNITTEST_ASSERT(ip1 == ip1);
    rcUNITTEST_ASSERT(ip2 == ip2);
    rcUNITTEST_ASSERT(ip1 != ip2);
    rcUNITTEST_ASSERT(ip2 != ip1);
  }
}

void UT_rect::test ()
{

// test size
{
  rcIRect r1(1, 1, 1, 2);
  rcUNITTEST_ASSERT(copair(r1.size(), rcIPair(1, 2)));
  rcUNITTEST_ASSERT(copair(r1.origin(), rcIPair(1, 1)));

  r1.size(rcIPair(-2, -1));
  rcUNITTEST_ASSERT(copair(r1.size(), rcIPair(2, 1)));
  rcUNITTEST_ASSERT(copair(r1.origin(), rcIPair(-1, 0)));

}


// test overlaps
{
  rcIRect r1(30, 30, 10, 10);

  /* upper left, no overlap */
  rcIRect r2(0, 0, 29, 29);
  rcUNITTEST_ASSERT(r1.overlaps(r2) ==  0);
  rcUNITTEST_ASSERT(r2.overlaps(r1) ==  0);

  /* upper left, just touching */
  rcIRect r3(1, 1, 29, 29);
  rcUNITTEST_ASSERT(r1.overlaps(r3, true) ==  1);
  rcUNITTEST_ASSERT(r3.overlaps(r1, true) ==  1);

  /* upper left, overlapping */
  rcIRect r4(1, 1, 30, 30);
  rcUNITTEST_ASSERT(r1.overlaps(r4) ==  1);
  rcUNITTEST_ASSERT(r4.overlaps(r1) ==  1);

  /* upper right, no overlap */
  rcIRect r5(41, 0, 1, 29);
  rcUNITTEST_ASSERT(r1.overlaps(r5) ==  0);
  rcUNITTEST_ASSERT(r5.overlaps(r1) ==  0);

  /* upper right, just touching */
  rcIRect r6(40, 0, 1, 30);
  rcUNITTEST_ASSERT(r1.overlaps(r6, true) ==  1);
  rcUNITTEST_ASSERT(r6.overlaps(r1, true) ==  1);

  /* upper right, overlapping */
  rcIRect r7(39, 0, 1, 31);
  rcUNITTEST_ASSERT(r1.overlaps(r7) ==  1);
  rcUNITTEST_ASSERT(r7.overlaps(r1) ==  1);

}

// test contains
{
  rcIRect r1(30, 30, 10, 10);

  /* upper left, tiny */
  rcIRect r2(30, 30, 0, 0);
  rcUNITTEST_ASSERT(r1.contains(r2) ==  1);
  rcUNITTEST_ASSERT(r2.contains(r1) ==  0);

  /* upper left, entire */
  rcIRect r3(30, 30, 10, 10);
  rcUNITTEST_ASSERT(r1.contains(r3) ==  1);
  rcUNITTEST_ASSERT(r3.contains(r1) ==  1);

  /* upper left, too big */
  rcIRect r4(30, 30, 11, 0);
  rcUNITTEST_ASSERT(r1.contains(r4) ==  0);
  rcUNITTEST_ASSERT(r4.contains(r1) ==  0);

  /* upper right, tall */
  rcIRect r5(39, 30, 1, 10);
  rcUNITTEST_ASSERT(r1.contains(r5) ==  1);
  rcUNITTEST_ASSERT(r5.contains(r1) ==  0);

  /* upper right, wide */
  rcIRect r6(30, 30, 10, 1);
  rcUNITTEST_ASSERT(r1.contains(r6) ==  1);
  rcUNITTEST_ASSERT(r6.contains(r1) ==  0);

  /* upper right, too big */
  rcIRect r7(39, 29, 1, 2);
  rcUNITTEST_ASSERT(r1.contains(r7) ==  0);
  rcUNITTEST_ASSERT(r7.contains(r1) ==  0);

}

// test intersect
{
  rcIRect r1(30, 30, 10, 10);
  rcIRect intersect;
  rcIRect r;

  /* upper left, just touching */
  rcIRect r3(1, 1, 29, 29);
  intersect = r1.intersect(r3);
  rcUNITTEST_ASSERT(intersect == r3.intersect(r1));
  rcUNITTEST_ASSERT(intersect == (r3 & r1));
  rcUNITTEST_ASSERT(intersect == (r1 & r3));
  rcUNITTEST_ASSERT(intersect.origin().x() ==  30);
  rcUNITTEST_ASSERT(intersect.origin().y() ==  30);
  rcUNITTEST_ASSERT(intersect.width() ==  0);
  rcUNITTEST_ASSERT(intersect.height() ==  0);
  r = r3;
  r &= r1;
  rcUNITTEST_ASSERT(r == intersect);

  /* upper left, overlapping */
  rcIRect r4(1, 1, 30, 30);
  intersect = r1.intersect(r4);
  rcUNITTEST_ASSERT(intersect == r4.intersect(r1));
  rcUNITTEST_ASSERT(intersect == (r4 & r1));
  rcUNITTEST_ASSERT(intersect == (r1 & r4));
  rcUNITTEST_ASSERT(intersect.origin().x() ==  30);
  rcUNITTEST_ASSERT(intersect.origin().y() ==  30);
  rcUNITTEST_ASSERT(intersect.width() ==  1);
  rcUNITTEST_ASSERT(intersect.height() ==  1);
  r = r4;
  r &= r1;
  rcUNITTEST_ASSERT(r == intersect);

  /* upper right, just touching */
  rcIRect r6(40, 0, 1, 30);
  intersect = r1.intersect(r6);
  rcUNITTEST_ASSERT(intersect == r6.intersect(r1));
  rcUNITTEST_ASSERT(intersect == (r6 & r1));
  rcUNITTEST_ASSERT(intersect == (r1 & r6));
  rcUNITTEST_ASSERT(intersect.origin().x() ==  40);
  rcUNITTEST_ASSERT(intersect.origin().y() ==  30);
  rcUNITTEST_ASSERT(intersect.width() ==  0);
  rcUNITTEST_ASSERT(intersect.height() ==  0);
  r = r6;
  r &= r1;
  rcUNITTEST_ASSERT(r == intersect);

  /* upper right, overlapping */
  rcIRect r7(39, 0, 1, 31);
  intersect = r1.intersect(r7);
  rcUNITTEST_ASSERT(intersect == r7.intersect(r1));
  rcUNITTEST_ASSERT(intersect == (r7 & r1));
  rcUNITTEST_ASSERT(intersect == (r1 & r7));
  rcUNITTEST_ASSERT(intersect.origin().x() ==  39);
  rcUNITTEST_ASSERT(intersect.origin().y() ==  30);
  rcUNITTEST_ASSERT(intersect.width() ==  1);
  rcUNITTEST_ASSERT(intersect.height() ==  1);
  r = r7;
  r &= r1;
  rcUNITTEST_ASSERT(r == intersect);

}

// test enclose
{
  rcIRect r1(30, 30, 10, 10);
  rcIRect enclose;
  rcIRect r;

  /* upper left, non-intersecting, null rect*/
  rcIRect r3a(0, 0, 0, 0);
  enclose = r1.enclose(r3a);
  rcUNITTEST_ASSERT(enclose == r3a.enclose(r1));
  rcUNITTEST_ASSERT(enclose == (r3a | r1));
  rcUNITTEST_ASSERT(enclose == (r1 | r3a));
  rcUNITTEST_ASSERT(enclose.origin().x() ==  30);
  rcUNITTEST_ASSERT(enclose.origin().y() ==  30);
  rcUNITTEST_ASSERT(enclose.width() ==  10);
  rcUNITTEST_ASSERT(enclose.height() ==  10);
  r = r3a;
  r |= r1;
  rcUNITTEST_ASSERT(r == enclose);

  /* upper left, non-intersecting */
  rcIRect r3b(0, 0, 1, 1);
  enclose = r1.enclose(r3b);
  rcUNITTEST_ASSERT(enclose == r3b.enclose(r1));
  rcUNITTEST_ASSERT(enclose == (r3b | r1));
  rcUNITTEST_ASSERT(enclose == (r1 | r3b));
  rcUNITTEST_ASSERT(enclose.origin().x() ==  0);
  rcUNITTEST_ASSERT(enclose.origin().y() ==  0);
  rcUNITTEST_ASSERT(enclose.width() ==  40);
  rcUNITTEST_ASSERT(enclose.height() ==  40);
  r = r3b;
  r |= r1;
  rcUNITTEST_ASSERT(r == enclose);

  /* upper left, within */
  rcIRect r4(30, 30, 10, 9);
  enclose = r1.enclose(r4);
  rcUNITTEST_ASSERT(enclose == r4.enclose(r1));
  rcUNITTEST_ASSERT(enclose == (r4 | r1));
  rcUNITTEST_ASSERT(enclose == (r1 | r4));
  rcUNITTEST_ASSERT(enclose.origin().x() ==  30);
  rcUNITTEST_ASSERT(enclose.origin().y() ==  30);
  rcUNITTEST_ASSERT(enclose.width() ==  10);
  rcUNITTEST_ASSERT(enclose.height() ==  10);
  r = r4;
  r |= r1;
  rcUNITTEST_ASSERT(r == enclose);

  /* upper right, non-intersecting */
  rcIRect r6(40, 0, 1, 1);
  enclose = r1.enclose(r6);
  rcUNITTEST_ASSERT(enclose == r6.enclose(r1));
  rcUNITTEST_ASSERT(enclose == (r6 | r1));
  rcUNITTEST_ASSERT(enclose == (r1 | r6));
  rcUNITTEST_ASSERT(enclose.origin().x() ==  30);
  rcUNITTEST_ASSERT(enclose.origin().y() ==  0);
  rcUNITTEST_ASSERT(enclose.width() ==  11);
  rcUNITTEST_ASSERT(enclose.height() ==  40);
  r = r6;
  r |= r1;
  rcUNITTEST_ASSERT(r == enclose);

  /* upper right, within */
  rcIRect r7(39, 30, 1, 1);
  enclose = r1.enclose(r7);
  rcUNITTEST_ASSERT(enclose == r7.enclose(r1));
  rcUNITTEST_ASSERT(enclose == (r7 | r1));
  rcUNITTEST_ASSERT(enclose == (r1 | r7));
  rcUNITTEST_ASSERT(enclose.origin().x() ==  30);
  rcUNITTEST_ASSERT(enclose.origin().y() ==  30);
  rcUNITTEST_ASSERT(enclose.width() ==  10);
  rcUNITTEST_ASSERT(enclose.height() ==  10);
  r = r7;
  r |= r1;
  rcUNITTEST_ASSERT(r == enclose);

}

// test  trim
{
  rcIRect r1(30, 20, 10, 40);
  rcIRect trim;

  // Trim (or grow) the rectangle on each side
  for (int l = -1; l < 2; ++l)
    for (int r = -1; r < 2; ++r)
      for (int t = -1; t < 2; ++t)
        for (int b = -1; b < 2; ++b)
        {
          trim = r1.trim(l, r, t, b);
          rcUNITTEST_ASSERT(trim.origin().x() ==  r1.origin().x() + l);
          rcUNITTEST_ASSERT(trim.origin().y() ==  r1.origin().y() + t);
          rcUNITTEST_ASSERT(trim.width() ==  r1.width() - (l + r));
          rcUNITTEST_ASSERT(trim.height() ==  r1.height() - (t + b));
        }

  // Trim the rectangle to a null rectangle.
  trim = r1.trim(5, 5, 20, 20);
  rcUNITTEST_ASSERT(trim.origin().x() ==  35);
  rcUNITTEST_ASSERT(trim.origin().y() ==  40);
  rcUNITTEST_ASSERT(trim.width() ==  0);
  rcUNITTEST_ASSERT(trim.height() ==  0);
  rcUNITTEST_ASSERT (trim.isNull());

}

// test  transpose
{
  rcIRect r1(15, 25, 35, 45);

  // Transpose the pelRect.
  rcIRect transpose = r1.transpose();

  rcUNITTEST_ASSERT(transpose.transpose() == r1);
  rcUNITTEST_ASSERT(transpose.origin().x() ==  r1.origin().y());
  rcUNITTEST_ASSERT(transpose.origin().y() ==  r1.origin().x());
  rcUNITTEST_ASSERT(transpose.height() ==  r1.width());
  rcUNITTEST_ASSERT(transpose.width() ==  r1.height());

}

// Test bounding rect
#if 0	
{
  deque<rc2Dvector> pts (4);
  pts[0] = rc2Dvector (1.0, 2.0);
  pts[1] = rc2Dvector (5.0, 2.0);
  pts[2] = rc2Dvector (1.0, 9.0);
  pts[3] = rc2Dvector (2.0, 3.0);

  rcDRect b;
  rfBoundingRect (pts, b);
  rcUTCheck (b.ul().x() == 1.0);
  rcUTCheck (b.ul().y() == 2.0);
  rcUTCheck (b.lr().x() == 5.0);
  rcUTCheck (b.lr().y() == 9.0);

}

// Test rectangle rounding

 {
   deque<rcDPair> pts (2);
  pts[0] = rcDPair (1.2, 2.7);
  pts[1] = rcDPair (5.5, 8.6);
  rcDRect dr (pts[0], pts[1]);
  rcIRect ir = rfRoundRectangle (dr);
  rcUTCheck (ir.ul().x() == 1);
  rcUTCheck (ir.ul().y() == 3);
  rcUTCheck (ir.lr().x() == 5);
  rcUTCheck (ir.lr().y() == 9);
 }
#endif
	
}
