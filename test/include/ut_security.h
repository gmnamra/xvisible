//
// $Id: ut_security.h 4391 2006-05-02 18:40:03Z armanmg $
//
// Copyright (c) 2002 Reify Corp. All rights reserved.
//

#ifndef _rcUT_SECURITY_H_
#define _rcUT_SECURITY_H_

#include "rc_unittest.h"
#include "rc_security.h"

class UT_Security : public rcUnitTest {
public:

    UT_Security();
    ~UT_Security();

    virtual uint32 run();

  private:
    void testKey( const rcSecurityKey& key, const std::string& keyText );
    void testEncryptionEngine( const rcSecurityKey& key, const std::string& clearText );
};

#endif // _rcUT_SECURITY_H_
