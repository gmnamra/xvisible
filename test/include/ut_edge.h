/*
 *  ut_edge.h
 *  framebuf
 *	$Id
 *  Created by Arman Garakani on Mon May 27 2002.
 *  Copyright (c) 2002 Reify, Corp. All rights reserved.
 *
 */


#ifndef _UT_EDGEHIST_H_
#define _UT_EDGEHIST_H_

#include "rc_unittest.h"

class UT_Edge : public rcUnitTest {
public:

   UT_Edge();
   ~UT_Edge();

   virtual uint32 run();

private:
      void edgeTest();
      void edgeTime ( uint32 width );
};

#endif // _UT_EDGEHIST_H_


