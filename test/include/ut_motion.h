/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.3  2005/08/30 21:08:50  arman
 *Cell Lineage
 *
 *Revision 1.3  2005/08/22 15:33:23  arman
 *added basic tests for visualFunction
 *
 *Revision 1.2  2004/01/14 21:35:00  arman
 *in progress
 *
 *Revision 1.1  2003/06/08 13:26:46  arman
 *ut for kinetoscope class
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __UT_MOTION_H
#define __UT_MOTION_H

#include <rc_unittest.h>
#include <rc_kinetoscope.h>

class UT_motion : public rcUnitTest {
public:

	UT_motion(std::string );
  UT_motion() {}	
   ~UT_motion();

   virtual uint32 run();
  vector<rcWindow>& images ()
  {return mImages; }

private:
  vector<rcWindow> mImages;
  void buildImages (int32, int32, rcPixel);
  void testBasic ();
  void testVisualFunction ();
  void testMotion ();
  void testfile ();
};


#endif /* __UT_MOTION_H */
