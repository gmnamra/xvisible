/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: ut_graphics.h 4391 2006-05-02 18:40:03Z armanmg $
 *
 * Unit tests for scalable graphics objects
 *
 *****************************************************************************/

#ifndef _rcUT_GRAPHICS_H_
#define _rcUT_GRAPHICS_H_

#include <rc_unittest.h>
#include <rc_graphics.h>
#include <rc_window.h>

class UT_Graphics : public rcUnitTest {
public:

    UT_Graphics();
    ~UT_Graphics();

    virtual uint32 run();

  private:
    // Test graphics styles
    uint32 testStyles();
    // Test graphics base class
    uint32 testBaseClass();
    // Test graphics derived classes
    uint32 testDerivedClasses();
    // Test graphics collections
    uint32 testCollections();
    
    // Test one style object
    void testStyle( const rcStyle* style,
                    uint32 expectedColor,
                    uint32 expectedLineWidth,
                    const rc2Fvector* expectedOrigin );
    // Compare two style objects
    void testStyle( const rcStyle& style1,
                    const rcStyle& style2 );
};

#endif // _rcUT_GRAPHICS_H_

