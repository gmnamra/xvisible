#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <rc_chaincode.h>

// Decompose diagonal directions to horz/vert components
#define NO_DIAGONALS 1

static float cSquare2 = sqrt( 2.0 );

/*************************************************************/
/*                                                           */
/* Two utility functions to add and subtract 2D integer      */
/* points.                                                   */
/*                                                           */
/*************************************************************/

// TODO: replace these with Reify classes

pt2* addPt2(pt2 *a, pt2 *b, pt2 *c)
{
    c->x = a->x + b->x;
    c->y = a->y + b->y;
    return c;
}

pt2* subPt2(pt2 *a, pt2 *b, pt2 *c)
{
    c->x = a->x - b->x;
    c->y = a->y - b->y;
    return c;
}

//
// Static data
//

// Native image resolution offsets
static pt2 sContourScaledOffset[8] = {{ 1, 0},
                                      { 0,-1},
                                      {-1, 0},
                                      { 0, 1},
                                      { 1,-1},
                                      {-1,-1},
                                      {-1, 1},
                                      { 1, 1}};
// Fat map resolution offsets
static rc2Fvector sContourOffset[8] = {rc2Fvector(1.0f/rcChainCode::eFatScale, 0.0f),
                                       rc2Fvector( 0.0f, -1.0f/rcChainCode::eFatScale),
                                       rc2Fvector(-1.0f/rcChainCode::eFatScale, 0.0f),
                                       rc2Fvector( 0.0f, 1.0f/rcChainCode::eFatScale),
                                       rc2Fvector( 1.0f/rcChainCode::eFatScale, -1.0f/rcChainCode::eFatScale),
                                       rc2Fvector(-1.0f/rcChainCode::eFatScale, -1.0f/rcChainCode::eFatScale),
                                       rc2Fvector(-1.0f/rcChainCode::eFatScale, 1.0f/rcChainCode::eFatScale),
                                       rc2Fvector( 1.0f/rcChainCode::eFatScale, 1.0f/rcChainCode::eFatScale)};
// Displayable direction characters
static char sDirectionChar[8] = {'0', 
                                 '2', 
                                 '4', 
                                 '6', 
                                 '1', 
                                 '3', 
                                 '5', 
                                 '7' };

#ifdef NO_DIAGONALS
// Digonal direction decomposed to horz/vert components
static uint8 sDiagonalComponents[4][2] = {
    {1 ,0},
    {2, 1},
    {3, 2},
    {0, 3}
};
#endif

// Is horizontal or vertical direction
#define rmGridDirection( dir ) ((dir<4))
// Is diagonal direction
#define rmDiagonalDirection( dir ) ((dir>3))

/*************************************************************/
/*                                                           */
/* Class constructor.                                        */
/*                                                           */
/*************************************************************/

rcChainCode::rcChainCode() :
        mLength( 0 ),
        mCapacity( eDefaultChainCapacity ),
        mDiagonalEdges( 0 ),
        mStraightEdges( 0 )
{
    mCode = (uint8*) malloc( mCapacity * sizeof(uint8) );
    rmAssert( mCode != NULL );
}

/*************************************************************/
/*                                                           */
/* Class destructor.                                         */
/*                                                           */
/*************************************************************/

rcChainCode::~rcChainCode()
{
    if ( mCode )
        free( mCode );
}


/*************************************************************/
/*                                                           */
/* This method appends a new code to the chain. If there     */
/* is not enough memory left, the function doubles the size  */
/* of the chain code.                                        */
/* It receives as a parameter the new code to be added (c).  */
/*                                                           */
/*************************************************************/

void rcChainCode::add( uint8 code )
{
    if ( mLength >= mCapacity-3 ) {
        mCapacity *= 2;
        mCode = (uint8*) realloc( mCode, mCapacity );
    }

    if ( rmDiagonalDirection( code ) ) {
        ++mDiagonalEdges;
#ifdef NO_DIAGONALS
        // Convert diagonal direction to horz and vert
        uint8 d1 = sDiagonalComponents[code-4][0];
        uint8 d2 = sDiagonalComponents[code-4][1];
        mCode[mLength++] = d1;
        mCode[mLength++] = d2;
#endif
    } else {
        ++mStraightEdges;
        mCode[mLength] = code;
        ++mLength;
    }
}


/*****************************************************************/
/*                                                               */
/* This method post-processes a 4x chain code to generate a 1x   */
/* chain code. A pointer to the 1x code is returned. The method  */
/* uses the 4 following rules:                                   */
/* CCCC ->  C :  reduce to one copy                              */
/*  CCC -> {} :  eliminate                                       */
/*   CC -> CC :  (ignored)                                       */
/*    C ->  C :  identity                                        */
/*                                                               */
/*****************************************************************/

rcChainCode* rcChainCode::postProcess()
{
    int    i = 0, j;
    int    trueLength = mLength;

    rcChainCode * filtCode = new rcChainCode();
    while ( i < trueLength ){
        if ( i+eFatScale-1 < trueLength ){
            for ( j=0; j<eFatScale-1; ++j )
                if ( mCode[i+j] != mCode[i+j+1] )
                    break;
            if ( j == eFatScale-1 ){
                filtCode->add( mCode[i] );
                i += eFatScale;
                continue;
            }
        }

        if ( i+eFatScale-2 < trueLength ){
            for ( j=0; j < eFatScale-2; ++j )
                if ( mCode[i+j] != mCode[i+j+1] )
                    break;
            if ( j == eFatScale-2 ){
                i += eFatScale-1;
                continue;
            }
        }
        filtCode->add( mCode[i] );
        ++i;
    }
    return filtCode;
}

// Get direction offset (image coords) given direction code
pt2 rcChainCode::getDirectionOffset( uint8 code ) const
{
    rmAssertDebug( code < 8 );
    return sContourScaledOffset[code];
}

// Get direction offset (hires coords) given direction code
rc2Fvector rcChainCode::getDirectionOffsetHires( uint8 code ) const
{
    rmAssertDebug( code < 8 );
    return sContourOffset[code];
}

/*****************************************************************/
/*                                                               */
/* A utility method to display the chain code                    */
/*                                                               */
/*****************************************************************/

void rcChainCode::printSelf() const
{
    for ( int i = 0; i < mLength; ++i )
        fprintf( stderr, "%c", getDirectionChar(mCode[i]) );
    fprintf( stderr, "\n" );
}

// Translate chain code to a collection of line segments
void rcChainCode::segments( const rc2Fvector& startPoint,
                            rcVisualSegmentCollection& segments ) const
{
    if ( mCode && mLength ) {
        // Compensate for margin pixels
        rc2Fvector marginOffset( -eFatMargin/(float)eFatScale, -eFatMargin/(float)eFatScale );
        // Add 1/8 pixel offset
        marginOffset += rc2Fvector( 1/(float)(eFatScale*2), 1/(float)(eFatScale*2) );
        rc2Fvector cur = startPoint + marginOffset;
        rc2Fvector prev = cur;
        uint8 prevCode = mCode[0];
        uint8 novelPoints = 1;
        
        for ( int i = 0; i < mLength; ++i ) {
            const uint8 c = mCode[i];
            
            // Keep accumulating until direction changes
            if ( prevCode != c ) {
                ++novelPoints;
                // Gather two novel points before producing a segment
                // because line strips in a collection are connected
                // together automatically.
                if ( novelPoints == 2 ) {
                    segments.push_back( rcVisualLineStrip( prev, cur ) );
                    novelPoints = 0;
                }
                prev = cur;
            } 
            // Accumulate direction offset
            cur +=  getDirectionOffsetHires( c );
            prevCode = c;
        }
        // Add final segment
        segments.push_back( rcVisualLineStrip( prev, cur ) );
    }
}

// Get perimeter length (straight edge len is 1, diagonal edge len is sqrt(2))
float rcChainCode::perimeterLength () const
{
    return mStraightEdges + mDiagonalEdges * cSquare2;
}

// private

// Get direction char given direction code
char rcChainCode::getDirectionChar( uint8 code ) const
{
    rmAssertDebug( code < 8 );
    return sDirectionChar[code];
}
