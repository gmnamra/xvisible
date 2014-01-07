/*
 *
 *$Id $
 *$Log$
 *Revision 1.2  2004/03/16 20:51:54  arman
 *updated test
 *
 *Revision 1.1  2004/03/16 19:00:02  arman
 *minimal test for smart pointers
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_smartptr.h>
#include "ut_sptr.h"

UT_smartPtr::UT_smartPtr()
{
}

UT_smartPtr::~UT_smartPtr()
{
    printSuccessMessage( "rc smartPtr test", mErrors );
}


class Boo : public virtual rcRefCounter
{
  
public:
  Boo() : mD(0) {}
  Boo(const Boo& r) : rcRefCounter (), mD(r.mD) {}

private:
  int32 mD;
};


uint32 UT_smartPtr::run ()
{
  rcSharedPtr<Boo> bb = new Boo;
  rcUTCheck (bb.refCount() == 1);

  // 
  rcSharedPtr<Boo> b;  
  rcUTCheck (b.refCount() == 0);

  // Copy
  rcSharedPtr<Boo> bz (bb);
  rcUTCheck (bz.refCount() == 2);
  return mErrors;
} 

