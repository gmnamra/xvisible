#ifndef _rcCHAINCODE_
#define _rcCHAINCODE_

#include <rc_types.h>
#include <rc_graphics.h>

typedef struct pt2Struct{
  int x,y;
} pt2;

class rcChainCode{
    public:

    enum { eDefaultChainCapacity = 512 }; // Chain array default capacity
    enum { eFatScale = 4 };               // Scale of fat pixel array
    enum { eFatMargin = 2 };              // Number of margin pixels on all sides
    
        rcChainCode();
        ~rcChainCode();

        // Mutators
        void add( uint8 code );
        rcChainCode* postProcess();

        // Accessors
        // Display chain letter codes to stderr
        void printSelf() const;
        // Translate chain code to a collection of graphics segments
        void segments( const rc2Fvector& startPoint,
                       rcVisualSegmentCollection& segments ) const;
        // Get direction offset (image coords) given direction code
        pt2 getDirectionOffset( uint8 code ) const;
        // Get direction offset (hires coords) given direction code
        rc2Fvector getDirectionOffsetHires( uint8 code ) const;
        // Get perimeter length (straight edge len is 1, diagonal edge len is sqrt(2)
        float perimeterLength () const;
        
  private:
        // Get direction char given direction code
        char getDirectionChar( uint8 code ) const;
        
        uint8* mCode;     // Array for direction codes
        int      mLength;   // Current length of code array
        int      mCapacity; // Capacity of code array
        uint32 mDiagonalEdges; // Number of diagonal edges (for perimeter calculation)
        uint32 mStraightEdges; // Number of diagonal edges (for perimeter calculation)
};

extern pt2* addPt2(pt2 *a, pt2 *b, pt2 *c);
extern pt2* subPt2(pt2 *a, pt2 *b, pt2 *c);

#endif //  _rcCHAINCODE_
