/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: ut_graphics.cpp 4391 2006-05-02 18:40:03Z armanmg $
 *
 * Unit tests for scalable graphics objects
 *
 *****************************************************************************/

#include "ut_graphics.h"

UT_Graphics::UT_Graphics()
{
}

UT_Graphics::~UT_Graphics()
{
    printSuccessMessage( "rcVisualSegment test", mErrors );
}

uint32
UT_Graphics::run()
{
    testStyles();
    testBaseClass();
    testDerivedClasses();
    testCollections();
    
    return mErrors;
}

// private

// Test styles
uint32 UT_Graphics::testStyles()
{
    uint32 oldErrors = mErrors;

    // rcStyle basic tests
    {
        // default rcStyle ctor
        rcStyle s;
        rc2Fvector origin(0.0f, 0.0f);
        testStyle( &s, rfRgb( 255, 0, 0 ), 1, &origin);
    }
    {
        // Test ctors and accessors
        {
            // rcStyle ctor with 1 arg
            uint32 color1 = rfRgb( 0, 66, 0 );
            uint32 color2 = rfRgb( 1, 2, 3 );
            rcStyle s1( color1 );
            rcStyle s2( color2 );
            
            rcUNITTEST_ASSERT( s1.color() == color1 );
            rcUNITTEST_ASSERT( s2.color() == color2 );
            rcUNITTEST_ASSERT( s2.color() != s1.color() );
        }
        {
            // rcStyle ctor with 2 args
            uint32 color1 = rfRgb( 0, 66, 0 );
            uint32 color2 = rfRgb( 1, 2, 3 );
            uint32 lineWidth1 = 6;
            uint32 lineWidth2 = 512;
            
            rcStyle s1( color1, lineWidth1 );
            rcStyle s2( color2, lineWidth2 );

            rcUNITTEST_ASSERT( s1.lineWidth() == lineWidth1 );
            rcUNITTEST_ASSERT( s2.lineWidth() == lineWidth2 );
            rcUNITTEST_ASSERT( s1.lineWidth() != s2.lineWidth());
            rcUNITTEST_ASSERT( s1.color() == color1 );
            rcUNITTEST_ASSERT( s2.color() == color2 );
            rcUNITTEST_ASSERT( s2.color() != s1.color() );
        }
        {
            // rcStyle ctor with 3 args
            uint32 color1 = rfRgb( 0, 66, 0 );
            uint32 color2 = rfRgb( 1, 2, 3 );
            uint32 lineWidth1 = 6;
            uint32 lineWidth2 = 512;
            rc2Fvector origin1( 1.0f, 1.0f );
            rc2Fvector origin2( 0.5f, 0.5f );
            
            rcStyle s1( color1, lineWidth1, origin1 );
            rcStyle s2( color2, lineWidth2, origin2 );
            testStyle( &s1, color1, lineWidth1, &origin1 );
            rc2Fvector origin = s1.pixelOrigin();
            testStyle( &s1, s1.color(), s1.lineWidth(), &origin );
            testStyle( &s2, color2, lineWidth2, &origin2 );
            origin = s2.pixelOrigin();
            testStyle( &s2, s2.color(), s2.lineWidth(), &origin );
            rcUNITTEST_ASSERT( s1.lineWidth() != s2.lineWidth());
            rcUNITTEST_ASSERT( s2.color() != s1.color() );
            rcUNITTEST_ASSERT( s1.pixelOrigin() != s2.pixelOrigin());
        }
    }

    return mErrors - oldErrors;
}

// Test graphics base class
uint32 UT_Graphics::testBaseClass()
{
    uint32 oldErrors = mErrors;

    {
        // General sanity checks
        rcUNITTEST_ASSERT( rcVisualSegment::eLast < rcUINT8_MAX );
        rcUNITTEST_ASSERT( sizeof( rcVisualSegment ) == 20 );
    }
    {
        // Default ctor
        rcVisualSegment g;
        const rc2Fvector zeroPoint( 0.0f, 0.0f );
        
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eUnknown );
        rcUNITTEST_ASSERT( g.p1() == zeroPoint );
        rcUNITTEST_ASSERT( g.p2() == zeroPoint );
    }
    {
        rcVisualSegmentCollection c;
        const rc2Fvector onePoint( 1.0f, -1.0f );
        const rc2Fvector twoPoint( -2.0f, 2.0f );
            
        // Ctor with type and points
        for ( uint32 t = 0; t < rcVisualSegment::eLast; ++t ) {
            rcVisualSegment::rcVisualSegmentType tp = static_cast<rcVisualSegment::rcVisualSegmentType>( t );
            rcVisualSegment g( tp, onePoint, twoPoint );
            rcUNITTEST_ASSERT( g.type() == tp );
            rcUNITTEST_ASSERT( g.p1() == onePoint );
            rcUNITTEST_ASSERT( g.p2() == twoPoint );
            // Fill segment collection
            c.push_back( g );
        }
        // Access segment collection
        for ( uint32 t = 0; t < rcVisualSegment::eLast; ++t ) {
            rcVisualSegment::rcVisualSegmentType tp = static_cast<rcVisualSegment::rcVisualSegmentType>( t );

            const rcVisualSegment& g = c[t];
            rcUNITTEST_ASSERT( g.type() == tp );
            rcUNITTEST_ASSERT( g.p1() == onePoint );
            rcUNITTEST_ASSERT( g.p2() == twoPoint );
         }
        // Fill graphics collection
        rcStyle style;
        rcVisualGraphicsCollection gc( style, c );
        rcUNITTEST_ASSERT( gc.size() == c.size() );
        rcUNITTEST_ASSERT( !gc.empty() );
        rcUNITTEST_ASSERT( gc.style() == style );
        
        // Access segment collection
        for ( uint32 t = 0; t < rcVisualSegment::eLast; ++t ) {
            rcVisualSegment::rcVisualSegmentType tp = static_cast<rcVisualSegment::rcVisualSegmentType>( t );

            const rcVisualSegment& g = gc.segments()[t];
            rcUNITTEST_ASSERT( g.type() == tp );
            rcUNITTEST_ASSERT( g.p1() == onePoint );
            rcUNITTEST_ASSERT( g.p2() == twoPoint );
         }
    }

    // Test base class mutators
    {
        for ( uint32 t = 0; t < rcVisualSegment::eLast; ++t ) {
            rcVisualSegment::rcVisualSegmentType tp = static_cast<rcVisualSegment::rcVisualSegmentType>( t );
            const rc2Fvector onePoint( 1.0f, -1.0f );
            const rc2Fvector twoPoint( -2.0f, 2.0f );
            rcVisualSegment g( tp, onePoint, twoPoint );
            rcUNITTEST_ASSERT( g.type() == tp );
            rcUNITTEST_ASSERT( g.p1() == onePoint );
            rcUNITTEST_ASSERT( g.p2() == twoPoint );
            // p2 mutation
            g.p2( onePoint );
            rcUNITTEST_ASSERT( g.p2() == onePoint );
            rcUNITTEST_ASSERT( g.p1() == onePoint );
            // p1 mutation
            g.p1( twoPoint );
            rcUNITTEST_ASSERT( g.p1() == twoPoint );
            rcUNITTEST_ASSERT( g.p2() == onePoint );
            // type mutation
            rcVisualSegment::rcVisualSegmentType newType =
                static_cast<rcVisualSegment::rcVisualSegmentType>(tp + 1);
            g.type( newType );
            rcUNITTEST_ASSERT( g.type() == newType );
            rcUNITTEST_ASSERT( g.type() != tp );
        }
    }
        
    return mErrors - oldErrors;
}

// Test graphics derived classes
uint32 UT_Graphics::testDerivedClasses()
{
    uint32 oldErrors = mErrors;
    const rc2Fvector zeroPoint( 0.0f, 0.0f );
    const rc2Fvector onePoint( 0.5f, -1.5f );
    const rc2Fvector twoPoint( 2.0f, 2.5f );

    {
        // Test eEmpty
        rcVisualEmpty g;
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eEmpty );
        rcUNITTEST_ASSERT( g.p1() == zeroPoint );
        rcUNITTEST_ASSERT( g.p2() == zeroPoint );

        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::eEmpty );
        rcUNITTEST_ASSERT( vg.p1() == zeroPoint );
        rcUNITTEST_ASSERT( vg.p2() == zeroPoint );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
    }
    {
        // Test ePoints
        rcVisualPoints g( onePoint, twoPoint );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::ePoints );
        rcUNITTEST_ASSERT( g.p1() == onePoint );
        rcUNITTEST_ASSERT( g.p2() == twoPoint );

        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::ePoints );
        rcUNITTEST_ASSERT( vg.p1() == onePoint );
        rcUNITTEST_ASSERT( vg.p2() == twoPoint );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
    }
    {
        // Test eLine
        rcVisualLine g( onePoint, twoPoint );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eLine );
        rcUNITTEST_ASSERT( g.p1() == onePoint );
        rcUNITTEST_ASSERT( g.p2() == twoPoint );

        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::eLine );
        rcUNITTEST_ASSERT( vg.p1() == onePoint );
        rcUNITTEST_ASSERT( vg.p2() == twoPoint );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
    }
    {
        // Test eLineStrip
        rcVisualLineStrip g( onePoint, twoPoint );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eLineStrip );
        rcUNITTEST_ASSERT( g.p1() == onePoint );
        rcUNITTEST_ASSERT( g.p2() == twoPoint );

        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::eLineStrip );
        rcUNITTEST_ASSERT( vg.p1() == onePoint );
        rcUNITTEST_ASSERT( vg.p2() == twoPoint );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
    }
    {
        // Test eLineLoop
        rcVisualLineLoop g( onePoint, twoPoint );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eLineLoop );
        rcUNITTEST_ASSERT( g.p1() == onePoint );
        rcUNITTEST_ASSERT( g.p2() == twoPoint );

        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::eLineLoop );
        rcUNITTEST_ASSERT( vg.p1() == onePoint );
        rcUNITTEST_ASSERT( vg.p2() == twoPoint );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
    }
    {
        // Test eArrow
        rcVisualArrow g( onePoint, twoPoint );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eArrow );
        rcUNITTEST_ASSERT( g.p1() == onePoint );
        rcUNITTEST_ASSERT( g.p2() == twoPoint );

        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::eArrow );
        rcUNITTEST_ASSERT( vg.p1() == onePoint );
        rcUNITTEST_ASSERT( vg.p2() == twoPoint );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
    }
    {
        // Test eRect
        rcVisualRect g( onePoint, twoPoint );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eRect );
        rcUNITTEST_ASSERT( g.p1() == onePoint );
        rcUNITTEST_ASSERT( g.p2() == twoPoint );
        rcUNITTEST_ASSERT( g.ul() == onePoint );
        rcUNITTEST_ASSERT( g.lr() == twoPoint );
        rcUNITTEST_ASSERT( g.width() == twoPoint.x() - onePoint.x() );
        rcUNITTEST_ASSERT( g.height() == twoPoint.y() - onePoint.y() );
        rcUNITTEST_ASSERT( g.x() == onePoint.x() );
        rcUNITTEST_ASSERT( g.y() == onePoint.y() );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
        
        rcVisualRect g2( onePoint.x(), onePoint.y(),
                         twoPoint.x() - onePoint.x(), twoPoint.y() - onePoint.y() );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eRect );
        rcUNITTEST_ASSERT( g.p1() == onePoint );
        rcUNITTEST_ASSERT( g.p2() == twoPoint );
        
        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        c.push_back( g2 );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::eRect );
        rcUNITTEST_ASSERT( vg.p1() == onePoint );
        rcUNITTEST_ASSERT( vg.p2() == twoPoint );
    }
    {
        // Test eEllipse
        rcVisualEllipse g( onePoint, twoPoint );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eEllipse );
        rcUNITTEST_ASSERT( g.p1() == onePoint );
        rcUNITTEST_ASSERT( g.p2() == twoPoint );
        rcUNITTEST_ASSERT( g.center() == onePoint );
        rcUNITTEST_ASSERT( g.radius() == twoPoint );
        
        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::eEllipse );
        rcUNITTEST_ASSERT( vg.p1() == onePoint );
        rcUNITTEST_ASSERT( vg.p2() == twoPoint );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
    }

    {
        // Test eCross
        rcVisualCross g( onePoint, twoPoint );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eCross );
        rcUNITTEST_ASSERT( g.p1() == onePoint );
        rcUNITTEST_ASSERT( g.p2() == twoPoint );
        rcUNITTEST_ASSERT( g.center() == onePoint );
        rcUNITTEST_ASSERT( g.size() == twoPoint );
        
        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::eCross );
        rcUNITTEST_ASSERT( vg.p1() == onePoint );
        rcUNITTEST_ASSERT( vg.p2() == twoPoint );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
    }

    {
        // Test eStyle
        uint32 color = rfRgb( 13, 66, 211 );
        uint32 lineWidth = 3;
        const rc2Fvector pixelOrigin( 0.5f, 0.6f );
        
        rcVisualStyle g( color, lineWidth, pixelOrigin );
        rcUNITTEST_ASSERT( g.type() == rcVisualSegment::eStyle );
        rcUNITTEST_ASSERT( g.p1().y() == float(lineWidth) );
        rcUNITTEST_ASSERT( g.p2() == pixelOrigin );
        rcUNITTEST_ASSERT( g.color() == color );
        if ( g.color() != color )
            cerr << "Got color " << g.color() << ", expected " << color << endl;
        rcUNITTEST_ASSERT( g.lineWidth() == lineWidth );
        rcUNITTEST_ASSERT( g.pixelOrigin() == pixelOrigin );
         
        // Fill collection
        rcVisualSegmentCollection c;

        c.push_back( g );
        const rcVisualSegment& vg = c.front();
        rcUNITTEST_ASSERT( vg.type() == rcVisualSegment::eStyle );
        rcUNITTEST_ASSERT( g.color() == color );
        rcUNITTEST_ASSERT( g.lineWidth() == lineWidth );
        rcUNITTEST_ASSERT( g.pixelOrigin() == pixelOrigin );
        rcUNITTEST_ASSERT( sizeof( g ) == sizeof( rcVisualSegment ) );
    }
    
    return mErrors - oldErrors;
}

// Test graphics collections
uint32 UT_Graphics::testCollections()
{
    uint32 oldErrors = mErrors;
    
    // rcVisualGraphicsCollection basic tests
    {
        // default ctor
        {
            rcVisualGraphicsCollection graphics;
            rcUNITTEST_ASSERT( graphics.empty() );
            rcUNITTEST_ASSERT( graphics.size() == 0 );

            const rcStyle& cs = graphics.style();
            rcStyle& s = graphics.style();
            testStyle( cs, s );

            const rcVisualSegmentCollection& cl = graphics.segments();
            rcVisualSegmentCollection& l = graphics.segments();
            rcUNITTEST_ASSERT( cl.empty() );
            rcUNITTEST_ASSERT( cl.size() == 0 );
            rcUNITTEST_ASSERT( l.empty() );
            rcUNITTEST_ASSERT( l.size() == 0 );
        }
        // ctor with 2 args
        {
            uint32 color1 = rfRgb( 0, 66, 0 );
            uint32 lineWidth1 = 6;
            rc2Fvector origin1( 1.0f, 1.0f );
            
            rcStyle s1( color1, lineWidth1, origin1 );
            
            rc2Fvector p1( 0.0f, 0.66f );
            rc2Fvector p2( 1.0f, 0.56f );
            rc2Fvector p3( 100.0f, 0.77f );
            rc2Fvector p4( 170.0f, 0.78f );

            rcVisualLine line1( p1, p2 );
            rcVisualLine line2( p3, p4 );

            // Create line collection
            rcVisualSegmentCollection lines;
            rcUNITTEST_ASSERT( lines.empty() );
            rcUNITTEST_ASSERT( lines.size() == 0 );
        
            lines.push_back( line1 );
            lines.push_back( line2 );

            rcVisualGraphicsCollection graphics( s1, lines );
            const rcStyle& cs = graphics.style();
            rcStyle& s = graphics.style();
            testStyle( cs, s1 );
            testStyle( s, s1 );

            const rcVisualSegmentCollection& cl = graphics.segments();
            rcVisualSegmentCollection& l = graphics.segments();
            rcUNITTEST_ASSERT( cl.size() == l.size() );
            rcUNITTEST_ASSERT( cl.size() == lines.size() );
            rcUNITTEST_ASSERT( cl == lines );
            rcUNITTEST_ASSERT( l == lines );
        }
    }
    return mErrors - oldErrors;
}

// Test one style object
void UT_Graphics::testStyle( const rcStyle* style,
                             uint32 expectedColor,
                             uint32 expectedLineWidth,
                             const rc2Fvector* expectedOrigin )
{
    rcUNITTEST_ASSERT( style != NULL );
    rcUNITTEST_ASSERT( style->color() == expectedColor );
    rcUNITTEST_ASSERT( style->lineWidth() == expectedLineWidth );
    rcUNITTEST_ASSERT( style->pixelOrigin() == *expectedOrigin );
}

// Compare two style objects
void UT_Graphics::testStyle( const rcStyle& style1,
                             const rcStyle& style2 )
{
    rcUNITTEST_ASSERT( style1.color() == style2.color() );
    rcUNITTEST_ASSERT( style1.lineWidth() == style2.lineWidth() );
    rcUNITTEST_ASSERT( style1.pixelOrigin() == style2.pixelOrigin()  );
}
