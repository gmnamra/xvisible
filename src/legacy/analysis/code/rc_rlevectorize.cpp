#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "rc_chaincode.h"
#include <rc_rlewindow.h>
#include "rc_rle.h"

// Vanilla image printing
#define cmPrintImage(a){                                    \
    for (int32 i = 0; i < (a).height(); i++)             \
    {                                                       \
        fprintf (stderr, "\n");                             \
        for (int32 j = 0; j < (a).width(); j++)          \
            fprintf (stderr, " %3d ", (a).getPixel (j, i)); \
        fprintf (stderr, "\n");                             \
    }}

//
// Algorithm original source: Graphics Gems V, ch 6-4
//

/* DEFINITION OF THE CONSTANTS */

#define CONTOUR 2
#define VISITED 3
#define BLACK 1
#define WHITE 0

/* DEFINITION OF THE MACROS */

#define MIN(x,y) ((x)<(y) ? (x) : (y))
#define MAX(x,y) ((x)>(y) ? (x) : (y))


void rcRleWindow::vectorize( rcVisualSegmentCollection& segments ) const
{
    pt2 size;

    size.x = width();
    size.y = height();

    pt2 fatSize;

    rcIPair newSize;
    // Create 4x4 bigger "fat" image
    uint8 * fatmap = createFatMap( newSize );
    fatSize.x = newSize.x();
    fatSize.y = newSize.y();

    // Create contour
    createFatMapContour( fatmap, newSize );
    // Create chain code from contour
    rcIPair startPixel;
    rcChainCode* code1 = createFatMapChainCode( fatmap, newSize, startPixel );

    float startX = startPixel.x()/(float)rcChainCode::eFatScale;
    float startY = startPixel.y()/(float)rcChainCode::eFatScale;

    rc2Fvector startPoint( startX, startY );
    // Add image offset
    startPoint += rc2Fvector( mRect.ul().x(), mRect.ul().y() );
    // Create line collection from chain code
    code1->segments( startPoint, segments );

#ifdef DEBUG_LOG        
    cerr << "start pixel " << startPixel << ": ";
    code1->printSelf();
    
    for ( int32 i = 0; i < lines.size(); ++i ) {
        const rc2Fvector p1 = lines[i].p1();
        const rc2Fvector p2 = lines[i].p2();
        cerr << p1 << " " << p2 << endl;
    }
#endif
    
    delete code1;
    delete [] fatmap;
}

#define FPIX(a,b) ((b) * fatSize.x() + (a))

/*************************************************************/
/*                                                           */
/* This is the main function. It receives as a parameter a   */
/* RLE image and outputs a chain code.                       */ 
/* The following constraints are placed on the bitmap:       */
/* + Only white (0) and black (1) pixels are taken into      */
/*   account.                                                */
/* + The shape to encode should have no holes and should be  */
/*   in a single piece.                                      */
/*                                                           */
/*************************************************************/

// Create 4x4 fat map
uint8* rcRleWindow::createFatMap( rcIPair& fatSize ) const
{
/* RESCAN THE BITMAP AT A GREATER RESOLUTION (4x4 GREATER) */
/* ADD TWO BLANK LINES TO THE LEFT, RIGHT, TOP AND BOTTOM  */
/* OF THE FATMAP. THESE COULD BE NECESSARY TO AVOID THE    */
/* CONTOUR TO BE DRAWN OUTSIDE OF THE BOUNDS OF THE MATRIX */

    fatSize.x() = rcChainCode::eFatMargin*2 + rcChainCode::eFatScale*width();
    fatSize.y() = rcChainCode::eFatMargin*2 + rcChainCode::eFatScale*height();
    const int size = fatSize.x() * fatSize.y() * sizeof(uint8);

    uint8* fatmap = new uint8[size];
    rmAssert( fatmap );
    // Init all pixels with white
    memset( fatmap, WHITE, size );
    
    uint16 len;
    // Initialize fat bitmap from RLE
    for ( int32 y = 0; y < height(); ++y ) {
        const Run* runs = mRep->runTable_[y];
        uint32 x = 0;
        while ( (len = runs->length) != 0 )
        {
            if ( runs->value ) {
                // Set black pixels from x to x+len
                for ( uint32 i = x; i < x + len; ++i ) {
                    const uint32 xx = 2+rcChainCode::eFatScale*i;
                    for( uint32 v=0; v < rcChainCode::eFatScale; ++v ) {
                        const uint32 yy = (2+rcChainCode::eFatScale*y+v) * fatSize.x();
                        for( uint32 u=0; u < rcChainCode::eFatScale; ++u )
                            fatmap[xx + u + yy] = BLACK;
                    }
                }
            }
            ++runs;
            x += len;
        }
    }
   
#ifdef DEBUG_LOG    
    // Print fatmap
    cerr << "GG fatmap " << endl;
    for (int32 j=0; j<fatSize.y(); j++) {
        for(int32 i=0; i<fatSize.x(); i++) {
            if (fatmap[FPIX(i,j)]==CONTOUR)
                cerr << "c";
            else if (fatmap[FPIX(i,j)]==BLACK)
                cerr << "1";
            else
                cerr << "0";
        }
        cerr << endl;
    }
#endif
    return fatmap;
}

// Create contour
void rcRleWindow::createFatMapContour( uint8* fatmap, const rcIPair& fatSize ) const
{
    bool seenBlack = false;

/* GENERATE THE CONTOUR OF THE BITMAP USING 2 SUCCESSIVE */
/* PASSES: FOR EACH DIRECTION, WE SCAN EACH LINE UNTIL   */
/* WE REACH A BLACK/WHITE PIXEL BOUNDARY */
    
    const int contourOffset = 1;
    const int margin = rcChainCode::eFatMargin;
    const int w = fatSize.x() - margin;
    const int h = fatSize.y() - margin;
    
    rmAssert( w > 0);
    
    // Note: there is a margin of two pixels on all sides,
    // we don't need to read the margin pixels.

    // Note: contour can be 0.25 pixels outside original image region

    // Horizontal pass
    for ( int32 j=margin; j < h; ++j ) {
        seenBlack = 0;
        const uint32 y = j * fatSize.x();
        for( int32 i=margin; i < w+1; ++i ) {
            const uint8 p = fatmap[i+y];
            if ( p == BLACK ) {
                if ( !seenBlack ) {
                    // Transition from white to black
                    fatmap[i-contourOffset+y] = CONTOUR;
                    seenBlack = true;
                }
            }
            else {
                if ( p == WHITE ) {
                    // Transition from black to white
                    if ( seenBlack ) 
                        fatmap[i+y] = CONTOUR;
                }
                seenBlack = false;
            }
        }
    }

    // Vertical pass
    for ( int32 i=margin; i < w; ++i ) {
        seenBlack = 0;
        for ( int32 j=margin; j < h+1; ++j ) {
            const uint8 p = fatmap[FPIX(i,j)];
            if ( p == BLACK ) {
                if ( !seenBlack ) {
                    // Transition from white to black
                    fatmap[FPIX(i, j-contourOffset)] = CONTOUR;
                    seenBlack = true;
                }
            }
            else {
                if ( p == WHITE ) {
                    // Transition from black to white
                    if ( seenBlack ) 
                        fatmap[FPIX(i,j)] = CONTOUR;
                }
                seenBlack = false;
            }
        }
    }
    
#ifdef DEBUG_LOG    
    // Print contour image
    cerr << "GG contour fatmap" << endl;
    for ( int32 j=0; j<fatSize.y(); j++) {
        for( int32 i=0; i<fatSize.x(); i++) {
            if (fatmap[FPIX(i,j)]==CONTOUR)
                cerr << "C";
            else if (fatmap[FPIX(i,j)]==VISITED)
                cerr << "V";
            else if (fatmap[FPIX(i,j)]==BLACK)
                cerr << "1";
            else if (fatmap[FPIX(i,j)]==WHITE)
                cerr << ".";
            else
                cerr << ".";
        }
        cerr << endl;
    }
#endif
}

// Create chain code
rcChainCode* rcRleWindow::createFatMapChainCode( uint8* fatmap, const rcIPair& fatSize,
                                                 rcIPair& startPixel ) const
{
    rcChainCode* c4 = new rcChainCode();
    rcChainCode& code4 = *c4;
    
    int i,j, distance;
    
    pt2 pixel,
        test_pixel,
        start_pixel,
        bbox[2] = {{INT_MAX, INT_MAX},
                   {-INT_MAX, -INT_MAX}};

    /* COMPUTE THE BOUNDING BOX OF THE CHARACTER (L,T,R,B) */
    for (j=1; j<fatSize.y()-1; ++j) {
        const uint32 y = j * fatSize.x();
        for(i=1; i<fatSize.x()-1; ++i)   
            if (fatmap[i+y]==CONTOUR){
                bbox[0].x = MIN(i, bbox[0].x);
                bbox[0].y = MIN(j, bbox[0].y);
                bbox[1].x = MAX(i, bbox[1].x);
                bbox[1].y = MAX(j, bbox[1].y);
            }
    }

#ifdef DEBUG_LOG
    cerr << "bbox " <<  bbox[0].x << " " <<  bbox[0].y << " " <<  bbox[1].x << " "  << bbox[1].y << endl;
#endif
    
/* DETERMINE THE CONTOUR PIXEL CLOSEST TO THE UPPER LEFT CORNER */
/* OF THE BOUNDING BOX                                          */

    distance = INT_MAX;
    for (j=1; j<fatSize.y()-1; ++j) {
        const uint32 y = j * fatSize.x();
        for(i=1; i<fatSize.x()-1; ++i)   
            if (fatmap[i+y]==CONTOUR){
                const int d = (i-bbox[0].x) * (i-bbox[0].x) + (j-bbox[0].y) * (j-bbox[0].y);
                if (d < distance) {
                    distance = d;
                    start_pixel.x = i;
                    start_pixel.y = j;
                }
            }
    }

  /* BEGIN THE ENCODING PROCEDURE */
    pixel.x = start_pixel.x;
    pixel.y = start_pixel.y;
    fatmap[FPIX(pixel.x, pixel.y)] = VISITED;
    uint8 last_dir = 4;
    uint8 dir = 0;
    
    for( ;; ) {
        /* AT FIRST, CHECK THE PIXEL IN THE LAST KNOWN DIRECTION */
        pt2 offset = code4.getDirectionOffset( last_dir );
        addPt2( &pixel, &offset, &test_pixel );
        uint32 idx = FPIX(test_pixel.x, test_pixel.y);
        if ( fatmap[idx] == CONTOUR ){
            pixel.x = test_pixel.x;
            pixel.y = test_pixel.y;
            fatmap[idx] = VISITED;
            code4.add(last_dir);
        }
        /* CHECK ALL THE POSSIBLE DIRECTIONS, CLOCKWISE */
        for ( dir=0; dir < 8; ++dir ) {
            pt2 offset = code4.getDirectionOffset( dir );
            addPt2( &pixel, &offset, &test_pixel );
            idx = FPIX(test_pixel.x, test_pixel.y);
            if ( fatmap[idx] == CONTOUR ){
                pixel.x = test_pixel.x;
                pixel.y = test_pixel.y;
                fatmap[idx] = VISITED;
                code4.add( dir );
                last_dir = dir;
                break;
            }
        }
        if (dir == 8)
            break;
    }

/* WRITE THE LAST MOVE TO THE OUTPUT VECTOR */
    for ( dir=0; dir < 8; ++dir ) {
        subPt2(&start_pixel, &pixel, &test_pixel);
        pt2 offset = code4.getDirectionOffset( dir );
        if ( test_pixel.x==offset.x && test_pixel.y==offset.y ){
            code4.add( dir );
            break;
        }
    }

    startPixel = rcIPair( start_pixel.x, start_pixel.y );

    // Return high-res chain
    return c4;
}
