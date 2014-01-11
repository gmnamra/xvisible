/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_graphics.h 6919 2009-07-14 13:42:55Z arman $
 *
 * Definitions for scalable graphics objects.
 *
 *****************************************************************************/

#ifndef _rcGRAPHICS_H_
#define _rcGRAPHICS_H_

#include <rc_style.h>
#include <rc_rect.h>
#include <rc_vector2d.h>
#include <rc_window.h>

//
// Scalable graphics base class
//

class rcVisualSegment {
  public:
    // Version number for graphics persistence
    // Version number must be updated if new fields are adder or if existing
    // float fields are used to store non-float data (like eStyle color field)
    enum { ePersistenceVersion = 1 };
    
    // Type of graphics segment
    enum rcVisualSegmentType {
        eUnknown = 0,     // Sentinel, causes an assert failure in drawing code
        eEmpty,           // Empty NOP to break a line strip or loop
        ePoints,          // Draw 2 points
        eLine,            // Draw 1 line segment
        eLineStrip,       // Draw 1 line segment connected to previous and next segment
        eLineLoop,        // Draw eLineStrip plus last point connected to first point
        eArrow,           // Draw 1 line with an arrow head
        eRect,            // Draw 1 unfilled rectangle
        eEllipse,         // Draw 1 unfilled ellipse (circle is a special case)
        eCross,           // Draw 1 cross
        eStyle,           // Specify drawing style, draw nothing
        eLast             // Sentinel, causes an assert failure in drawing code
    };

    // ctor
    rcVisualSegment();
    rcVisualSegment( rcVisualSegmentType type, const rc2Fvector& p1, const rc2Fvector& p2 );
    // Note: non-virtual dtor by design (to save 4 bytes). Derived classes must destruct
    // everything in their dtors because base class dtor will not be called. It's not a
    // problem as long as there are no dynamically allocated members in the base class.
    ~rcVisualSegment();

    // Accessors
    const rc2Fvector& p1() const { return mP1;}
    const rc2Fvector& p2() const { return mP2;}

    rcVisualSegmentType type() const { return static_cast<rcVisualSegmentType>(mType); };

    // Mutators
    void p1(const rc2Fvector& p) { mP1 = p; }
    void p2(const rc2Fvector& p) { mP2 = p; }
    void type( rcVisualSegmentType t ) { mType = t; };
    rcVisualSegment&  operator+=(const rc2Fvector& _t_) { mP1 += _t_; mP2 += _t_; return *this; }
    
    // Operators
    bool operator==(const rcVisualSegment& o) const {
        return (mType == o.mType && mP1 == o.mP1 && mP2 == o.mP2 ); };
    bool operator!=(const rcVisualSegment& o) const {
        return !(*this == o );
    };
     
  protected:
    rc2Fvector mP1;      // Data point 1
    rc2Fvector mP2;      // Data point 2
    uint8    mType;    // Object type (rcVisualSegmentType)
};

//
// All graphics classes derived from rcVisualSegment have the following properties:
//
// 1. They utilize the same data members as base class.
// 2. They do not have data members of their own. This guarantees that
//    derived classes can be stored in STL containers without risk of
//    slicing.
// 3. They can have new ctors and accessors/mutators to improve ease
//    of use.
// 4. Each derived class defines the semantics of data in mP1 and mP2.

//
// Empty element to break line strip or loop. This element draws nothing.
//

class rcVisualEmpty : public rcVisualSegment {
  public:
    rcVisualEmpty() :
    rcVisualSegment( eEmpty, rc2Fvector(0.0f, 0.0f), rc2Fvector(0.0f, 0.0f) ) {};
    virtual ~rcVisualEmpty() {};
};

//
//  2 unconnected points
//

class rcVisualPoints : public rcVisualSegment {
  public:
    rcVisualPoints( const rc2Fvector& pt1, const rc2Fvector& pt2 ) :
                    rcVisualSegment( ePoints, pt1, pt2 ) {};
    ~rcVisualPoints() {};
};

//
// Unconnected line segment
//

class rcVisualLine : public rcVisualSegment {
  public:
    rcVisualLine( const rc2Fvector& pt1, const rc2Fvector& pt2 ) :
                  rcVisualSegment( eLine, pt1, pt2 ) {};

  rcVisualLine( const rc2Dvector& pt1, const rc2Dvector& pt2 ) :
      rcVisualSegment( eLine, rc2Fvector ((float) pt1.x(), (float) pt1.y()) ,
		       rc2Fvector ((float) pt2.x(), (float) pt2.y()) ) {};

    ~rcVisualLine() {};
};


//
// Connected line segment (polyline). This line segment connects to preceding and following
// rcVisualLineStrip objects in the collection.
//

class rcVisualLineStrip : public rcVisualSegment {
  public:
    rcVisualLineStrip( const rc2Fvector& pt1, const rc2Fvector& pt2 ) :
                       rcVisualSegment( eLineStrip, pt1, pt2 ) {};
    ~rcVisualLineStrip() {};
};

//
// Connected line segment (polyline) loop. This line segment connects to preceding and following
// rcVisualLineStrip objects in the collection. The last segment in the collection is also
// connected to the first segment in the collection.
//

class rcVisualLineLoop : public rcVisualSegment {
  public:
    rcVisualLineLoop( const rc2Fvector& pt1, const rc2Fvector& pt2 ) :
                      rcVisualSegment( eLineLoop, pt1, pt2 ) {};
    ~rcVisualLineLoop() {};
};


//
// Rectangle. First point is upper left corner, second point is lower right corner.
//

class rcVisualRect : public rcVisualSegment {
  public:
    rcVisualRect( const rc2Fvector& ul, const rc2Fvector& lr ) :
                  rcVisualSegment( eRect, ul, lr ) {};

  rcVisualRect (const rcIRect& ir) : 
    rcVisualSegment( eRect, rc2Fvector (ir.ul()), rc2Fvector (ir.lr())) {};

  rcVisualRect (const rcFRect& ir) :
    rcVisualSegment( eRect, rc2Fvector (ir.ul()), rc2Fvector (ir.lr())) {};

  rcVisualRect (const rcRect& ir) :
    rcVisualSegment( eRect, rc2Fvector (ir.ul()), rc2Fvector (ir.lr())) {};

    rcVisualRect( const float& x, const float& y, const float& w, const float& h ) :
                  rcVisualSegment( eRect, rc2Fvector(x, y), rc2Fvector(x+w, y+h) ) {};
    
    ~rcVisualRect() {};

    // Accessors
    const rc2Fvector& ul() const { return mP1;}
    const rc2Fvector& lr() const { return mP2;}
    float             x() const { return mP1.x(); }
    float             y() const { return mP1.y(); }
    float             width() const { return mP2.x() - mP1.x(); }
    float             height() const { return mP2.y() - mP1.y(); }
};

//
// Arrow. First point is arrow base, second point is arrow tip.
//

class rcVisualArrow : public rcVisualSegment {
  public:
    rcVisualArrow( const rc2Fvector& base, const rc2Fvector& tip ) :
                   rcVisualSegment( eArrow, base, tip ) {};

    ~rcVisualArrow() {};
};

//
// Ellipse. First point is center point, second point is x and y radius.
//

class rcVisualEllipse : public rcVisualSegment {
  public:
    rcVisualEllipse( const rc2Fvector& center, const rc2Fvector& radius ) :
                     rcVisualSegment( eEllipse, center, radius ) {};

    // Accessors
    const rc2Fvector& center() const { return mP1;}
    const rc2Fvector& radius() const { return mP2;}

    ~rcVisualEllipse() {};
};

//
// Cross. First point is center point, second point is x and y line length.
//

class rcVisualCross : public rcVisualSegment {
  public:
    rcVisualCross( const rc2Fvector& center, const rc2Fvector& size ) :
                   rcVisualSegment( eCross, center, size ) {};

    // Accessors
    const rc2Fvector& center() const { return mP1;}
    const rc2Fvector& size() const { return mP2;}

    ~rcVisualCross() {};
};

//
// Style. Specifies color, line width and pixel origin.
//

class rcVisualStyle : public rcVisualSegment {
  public:
    rcVisualStyle( uint32 color, uint32 lineWidth, const rc2Fvector& pixelOrigin ) :
    rcVisualSegment( eStyle, rc2Fvector( 0.0, static_cast<float>(lineWidth) ), pixelOrigin )
    {
        // Warning: coercion from float to int
        // Color is stored as uint32
        uint32* c = (uint32*)(mP1.Ref()); *c = color;
    };

    // Accessors
    uint32 color() const { return *(uint32*)(mP1.Ref());}  // Warning: coercion from float to int
    uint32 lineWidth() const { return static_cast<uint32>(mP1.y());}
    const rc2Fvector& pixelOrigin() const { return mP2;}

    ~rcVisualStyle() {};
};

// Collection of graphics segments
typedef vector<rcVisualSegment> rcVisualSegmentCollection;


//
// Styled collection of float-based graphics objects
//

class rcVisualGraphicsCollection
{
public:
  rcVisualGraphicsCollection() {};
  rcVisualGraphicsCollection( const rcStyle& style,
			      const rcVisualSegmentCollection& segments ) :
    mStyle( style ), mLines( segments ), mId (-1) {};
  
  // Accessors
  const rcStyle& style() const { return mStyle; };
  rcStyle& style() { return mStyle; };

  const rcVisualSegmentCollection& segments() const { return mLines; };
  rcVisualSegmentCollection& segments() { return mLines; };
  
  bool empty() const { return mLines.empty(); };
  uint32 size() const { return mLines.size(); };
  int32 id () const {return mId; }

  // Mutators
  void style( const rcStyle& s ) { mStyle = s; };
  void id (const int32 ii) {if (ii >= 0) mId = ii; }

  private:
  rcStyle                    mStyle; // Drawing style
  rcVisualSegmentCollection  mLines; // Graphics objects
  int32                    mId;    // User definable id
};

// Collection of graphics collections
typedef vector<rcVisualGraphicsCollection> rcVisualGraphicsCollectionCollection;

//
// One text segment with lower left position coordinate
//

class rcTextSegment {
  public:
    rcTextSegment() {};
    rcTextSegment( const rc2Fvector& pos, const std::string& text ) : mPos( pos ), mText( text ) {};

    // Accessors
    const std::string& text() const { return mText; };
    std::string& text() { return mText; };
    const rc2Fvector& pos() const { return mPos; };
    rc2Fvector& pos() { return mPos; };

    // Mutators
    void text( const std::string& text ) { mText = text; };
    void pos( const rc2Fvector& pos ) { mPos = pos; };
    
  private:
    rc2Fvector mPos;  // Text position
    std::string   mText; // Text string
};

// Collection of unstyled polylines
typedef vector<rcTextSegment> rcTextSegmentCollection;

//
// Styled collection of texts
// TODO: move this to a more appropriate place
//

class rcTextCollection {
  public:
    rcTextCollection() {};
    rcTextCollection( const rcStyle& style, const rcTextSegmentCollection& texts ) :
                      mStyle( style ), mTexts( texts ) {};

    // Accessors
    const rcStyle& style() const { return mStyle; };
    rcStyle& style() { return mStyle; };
    const rcTextSegmentCollection& texts() const { return mTexts; };
    rcTextSegmentCollection& texts() { return mTexts; };
        
    bool empty() const { return mTexts.empty(); };
    uint32 size() const { return mTexts.size(); };
    
    // Mutators
    void style( const rcStyle& s ) { mStyle = s; };
    
  private:
    rcStyle                   mStyle; // Drawing style
    rcTextSegmentCollection   mTexts; // Text segments
};


void rfGradientImageToVisualSegment (const rcWindow& mag,  const rcWindow& angle,  rcVisualSegmentCollection& v);
void rfImageGradientToVisualSegment (const rcWindow& image,  rcVisualSegmentCollection& v);

#endif // _rcGRAPHICS_H_
