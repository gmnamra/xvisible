/*
 * Copyright 2002 Reify, Inc.
 * $Id: ut_window.cpp 4420 2006-05-15 18:48:40Z armanmg $
 *
 */

#include "ut_window.h"
#include "rc_window.h"

#include <vector>
using namespace std;

static void rcReturn (rcWindow& src);
static rcWindow rcReturn2 ();


UT_Window::UT_Window()
{
}

UT_Window::~UT_Window()
{
    printSuccessMessage( "rcWindow test", mErrors );
}

uint32
UT_Window::run() {

    // rcWindow basic tests
    {
        rcFrameRef ptr = new rcFrame( 640, 480, rcPixel8 );
        rcUNITTEST_ASSERT( ptr->refCount() == 1 );
        rcWindow window1( ptr, 0, 0, 100, 120 );
        rcUNITTEST_ASSERT( ptr->refCount() == 2 );
        {
            const rcRect rect( 2, 3, 9, 8 );
            rcWindow window2( ptr, rect );
            rcUNITTEST_ASSERT( ptr->refCount() == 3 );
        }
        rcUNITTEST_ASSERT( ptr->refCount() == 2 );
    }

    // Large & Small windows
    {
        rcFrameRef ptr = new rcFrame( 64, 48, rcPixel8 );

        for ( int32 x = 0; x < ptr->width(); x++ ) {
            for ( int32 y = 0; y < ptr->height(); y++ ) {
                testWindow( x, y, ptr->width () - x, ptr->height () - y, ptr );
                testSubWindows( x, y, ptr->width () - x, ptr->height () - y, ptr );
            }
        }
    }

    // rowPointer Test
    {
        rcFrameRef ptr = new rcFrame( 23, 37, rcPixel8 );

        for ( int32 x = 0; x < ptr->width(); x++ ) {
            for ( int32 y = 0; y < ptr->height(); y++ ) {
                testRowPointers ( x, y, ptr->width () - x, ptr->height() - y, ptr );
            }
        }
    }

    // contain Pointer Test
    {
        rcWindow win (23, 37);

        rcUTCheck (win.contains (win.pelPointer (0,0)));
        rcUTCheck (!win.contains ((win.pelPointer (0,0) - 1)));

    }

#if 0
    // Test vImage accessor
    {
      vImage_Buffer vib;

      rcWindow win (83, 37);
      win.vImage (vib);

            //   rcUTCheck ((uint32) vib.data == (uint32) win.pelPointer (0,0));
      rcUTCheck (vib.width == (uint32) win.width());
      rcUTCheck (vib.height == (uint32) win.height());
      rcUTCheck (vib.rowBytes == (uint32) win.rowUpdate());
    }
#endif
    
    // Test translate and trim
    {
        rcIPair size (23, 47);
        rcIPair d1 (2, 4);     
        bool flag;

        rcWindow rw (size);
        rcUNITTEST_ASSERT(rw.width() == 23);
        rcUNITTEST_ASSERT(rw.height() == 47);
        rcUNITTEST_ASSERT(rw.x() == 0);
        rcUNITTEST_ASSERT(rw.y() == 0);

        rcWindow rw2 (rw, 20, 42, 1, 1);
        rcUNITTEST_ASSERT(rw2.x() == 20);
        rcUNITTEST_ASSERT(rw2.y() == 42);

        flag = rw2.translate (d1);
        rcUNITTEST_ASSERT(rw2.x() == 22);
        rcUNITTEST_ASSERT(rw2.y() == 46);
        rcUNITTEST_ASSERT(flag == true);
        rcUNITTEST_ASSERT(rw2.width() == 1);
        rcUNITTEST_ASSERT(rw2.height() == 1);

        flag = rw2.translate (d1);
        rcUNITTEST_ASSERT(flag == false);

        rcWindow rw3 (rw, 20, 42, 1, 1);
        flag = rw3.trim (d1.x());
        rcUNITTEST_ASSERT(flag == false);

        rcWindow rw5 (rw, 20, 42, 1, 1);
        flag = rw5.trim (-1 * d1.x());
        rcUNITTEST_ASSERT(flag == true);

        rcWindow rw4 (rw, 18, 42, 5, 5);
        flag = rw4.trim (d1.x());
        rcUNITTEST_ASSERT(rw4.width() == 1);
        rcUNITTEST_ASSERT(rw4.height() == 1);


    }
   
    return 0;
}

void UT_Window::testWindow( int32 x, int32 y, int32 width, int32 height, rcFrameRef& buf )
{
 
    int32 oldRefCount = buf.refCount();
   
    // Set the four corner pels to a value
    buf->setPixel (x, y, 23);
    if (width > 1) buf->setPixel (x + width - 1, y, 33);
    if (height > 1) buf->setPixel (x, y + height - 1, 43);
    if (height > 1 && width > 1) buf->setPixel (x + width - 1, y + height - 1, 53);

    // Ctor test
    rcWindow window( buf, x, y, width, height );
    {
        rcWindow window2( buf, rcRect( x, y, width, height ) );
        rcUNITTEST_ASSERT( compare( window, window2 ) );
    }

    // Check the pixels
    rcUNITTEST_ASSERT(window.getPixel (0, 0) == 23);
    if (width > 1) rcUNITTEST_ASSERT(window.getPixel (width - 1, 0) == 33);
    if (height > 1) rcUNITTEST_ASSERT(window.getPixel (0, height - 1) == 43);
    if (height > 1 && width > 1) rcUNITTEST_ASSERT(window.getPixel (width - 1, height - 1) == 53);
    
    // Set and test corner pels one more time to guarentee test didn't pass by chance
    buf->setPixel (x, y, 24);
    if (width > 1) buf->setPixel (x + width - 1, y, 34);
    if (height > 1) buf->setPixel (x, y + height - 1, 44);
    if (height > 1 && width > 1) buf->setPixel (x + width - 1, y + height - 1, 54);
    
    // Check the pixels
    rcUNITTEST_ASSERT(window.getPixel (0, 0) == 24);
    if (width > 1) rcUNITTEST_ASSERT(window.getPixel (width - 1, 0) == 34);
    if (height > 1) rcUNITTEST_ASSERT(window.getPixel (0, height - 1) == 44);
    if (height > 1 && width > 1) rcUNITTEST_ASSERT(window.getPixel (width - 1, height - 1) == 54);
    
	// Accessor tests
    rcUNITTEST_ASSERT( buf->refCount() == oldRefCount + 1 );
    rcUNITTEST_ASSERT( window.x() == x );
    rcUNITTEST_ASSERT( window.y() == y );
    rcUNITTEST_ASSERT( window.height() == height );
    rcUNITTEST_ASSERT( window.width() == width );
    rcUNITTEST_ASSERT( window.frameBuf() != 0 );

    rcUNITTEST_ASSERT( buf->refCount() == oldRefCount + 1 );

    //    fprintf (stderr,"Window Size %dX%d @ %d,%d ok \n", width, height, x, y);

}

void UT_Window::testSubWindows( int32 x, int32 y, int32 width, int32 height, rcFrameRef& buf )
{
 
    int32 oldRefCount = buf.refCount();

    rcWindow window( buf, x, y, width, height );

    //  fprintf (stderr,"Window Size %dX%d @ %d,%d ok \n", width, height, x, y);    

    // Test subwindowing ctor
    int32 subX = x + width/4, subY = y + height/4, subWidth = (width+1)/2, subHeight = (height+1)/2;
                
    // Set the four corner pels to a value
    buf->setPixel(subX, subY, 03);
    if (subWidth > 1) buf->setPixel(subX + subWidth - 1, subY, 13);
    if (subHeight > 1) buf->setPixel(subX, subY + subHeight - 1, 23);
    if (subWidth > 1 && subHeight > 1) buf->setPixel(subX + subWidth - 1, subY + subHeight - 1, 33);

    rcWindow* subWin = new rcWindow(window, width/4, height/4, subWidth, subHeight);
    rcUNITTEST_ASSERT(subWin != 0);


    // Check the pixels
    rcUNITTEST_ASSERT(subWin->getPixel(0, 0) == 03);
    if (subWidth > 1) rcUNITTEST_ASSERT(subWin->getPixel(subWidth - 1, 0) == 13);
    if (subHeight > 1) rcUNITTEST_ASSERT(subWin->getPixel(0, subHeight - 1) == 23);
    if (subWidth > 1 && subHeight > 1)
        rcUNITTEST_ASSERT(subWin->getPixel(subWidth - 1, subHeight - 1) == 33);
    
    // Set and test corner pels one more time to guarentee test didn't pass by chance
    buf->setPixel(subX, subY, 04);
    if (subWidth > 1) buf->setPixel(subX + subWidth - 1, subY, 14);
    if (subHeight > 1) buf->setPixel(subX, subY + subHeight - 1, 24);
    if (subWidth > 1 && subHeight > 1) buf->setPixel(subX + subWidth - 1, subY + subHeight - 1, 34);

    rcUNITTEST_ASSERT(subWin->getPixel(0, 0) == 04);
    if (subWidth > 1) rcUNITTEST_ASSERT(subWin->getPixel(subWidth - 1, 0) == 14);
    if (subHeight > 1) rcUNITTEST_ASSERT(subWin->getPixel(0, subHeight - 1) == 24);
    if (subWidth > 1 && subHeight > 1)
        rcUNITTEST_ASSERT(subWin->getPixel(subWidth - 1, subHeight - 1) == 34);

    // Accessor tests
    rcUNITTEST_ASSERT( buf->refCount() == oldRefCount + 2 );
    rcUNITTEST_ASSERT( subWin->x() == subX );
    rcUNITTEST_ASSERT( subWin->y() == subY );
    rcUNITTEST_ASSERT( subWin->height() == subHeight );
    rcUNITTEST_ASSERT( subWin->width() == subWidth );
    rcUNITTEST_ASSERT( subWin->frameBuf() != 0 );

    delete subWin;
        
    rcUNITTEST_ASSERT( buf->refCount() == oldRefCount + 1 );

    // Test windowRelativeTo(). Note that this can trigger changes in parent
    // frame reference counting, so create an extra frame to test this feature.
    //
    rcFrameRef buf2 = new rcFrame( 640, 480, rcPixel8 );

    int32 oldRefCount2 = buf2.refCount();

    rcWindow window2( buf2, x, y, width, height );

    rcUNITTEST_ASSERT( buf2->refCount() == oldRefCount2 + 1 );

    // Set the four corner pels to a value
    buf->setPixel(subX, subY, 43);
    if (subWidth > 1) buf->setPixel(subX + subWidth - 1, subY, 53);
    if (subHeight > 1) buf->setPixel(subX, subY + subHeight - 1, 63);
    if (subWidth > 1 && subHeight > 1) buf->setPixel(subX + subWidth - 1, subY + subHeight - 1, 73);

    subWin = &window2.windowRelativeTo(window, width/4, height/4, subWidth, subHeight);
    rcUNITTEST_ASSERT(subWin == &window2);
    rcUNITTEST_ASSERT( buf2->refCount() == oldRefCount2 );
    rcUNITTEST_ASSERT( buf->refCount() == oldRefCount + 2 );
        
    // Check the pixels
    rcUNITTEST_ASSERT(window2.getPixel(0, 0) == 43);
    if (subWidth > 1) rcUNITTEST_ASSERT(window2.getPixel(subWidth - 1, 0) == 53);
    if (subHeight > 1) rcUNITTEST_ASSERT(window2.getPixel(0, subHeight - 1) == 63);
    if (subWidth > 1 && subHeight > 1)
        rcUNITTEST_ASSERT(window2.getPixel(subWidth - 1, subHeight - 1) == 73);
    
    // Set and test corner pels one more time to guarentee test didn't pass by chance
    buf->setPixel(subX, subY, 44);
    if (subWidth > 1) buf->setPixel(subX + subWidth - 1, subY, 54);
    if (subHeight > 1) buf->setPixel(subX, subY + subHeight - 1, 64);
    if (subWidth > 1 && subHeight > 1) buf->setPixel(subX + subWidth - 1, subY + subHeight - 1, 74);

    rcUNITTEST_ASSERT(window2.getPixel(0, 0) == 44);
    if (subWidth > 1) rcUNITTEST_ASSERT(window2.getPixel(subWidth - 1, 0) == 54);
    if (subHeight > 1) rcUNITTEST_ASSERT(window2.getPixel(0, subHeight - 1) == 64);
    if (subWidth > 1 && subHeight > 1)
        rcUNITTEST_ASSERT(window2.getPixel(subWidth - 1, subHeight - 1) == 74);

    // Accessor tests
    rcUNITTEST_ASSERT( buf->refCount() == oldRefCount + 2 );
    rcUNITTEST_ASSERT( window2.x() == subX );
    rcUNITTEST_ASSERT( window2.y() == subY );
    rcUNITTEST_ASSERT( window2.height() == subHeight );
    rcUNITTEST_ASSERT( window2.width() == subWidth );
    rcUNITTEST_ASSERT( window2.frameBuf() != 0 );

    // Test Copy and Assignment. Note that this can trigger changes in parent
    // frame reference counting, so create an extra frame to test this feature.
    //
    rcFrameRef buf3 = new rcFrame( 640, 480, rcPixel8 );

    int32 oldRefCount3 = buf3.refCount();

    rcWindow window3( buf3, x, y, width, height );

    rcUNITTEST_ASSERT( buf3->refCount() == oldRefCount3 + 1 );

    rcWindow rw = window3;

    rcUNITTEST_ASSERT( buf3->refCount() == oldRefCount3 + 2 );

    rcWindow rw2 (window3);

    rcUNITTEST_ASSERT( buf3->refCount() == oldRefCount3 + 3 );

    rcWindow rw4;

    rcReturn (rw4);

    rcFrameRef fb = rw4.frameBuf ();

    rcUNITTEST_ASSERT( fb->refCount() == 2);

    rcWindow rw5 = rcReturn2 ();

    rcFrameRef fb2 = rw5.frameBuf ();

    rcUNITTEST_ASSERT( fb2->refCount() == 2);
   
}

void UT_Window::testRowPointers(int32 x, int32 y, int32 width, int32 height,  rcFrameRef& buf)
{
    rcWindow win( buf, x, y, width, height);
  
    for ( int32 row = 0; row < win.height(); row++ )
    {
        if (!row) continue;
        rcUNITTEST_ASSERT ((win.rowPointer (row) - win.rowPointer (row - 1)) == win.rowUpdate ());
    }

}

// Retrun true if windows are identical, false otherwise
bool UT_Window::compare( const rcWindow& w1, const rcWindow& w2 )
{
    // Check geometry
    if ( w1.x() != w2.x() )
        return false;
    if ( w1.y() != w2.y() )
        return false;
    if ( w1.height() != w2.height() )
        return false;
    if ( w1.width() != w2.width() )
        return false;
    if ( w1.frameBuf() != w2.frameBuf() )
        return false;
    if ( w1.depth() != w2.depth() )
        return false;
    
    // Check pixels
    for (int32 x = 0; x < w1.width(); x++)
        for (int32 y = 0; y < w1.height(); y++) 
            if ( w1.getPixel(x, y) != w2.getPixel(x, y) )
                return false;
    
    return true;
}

UT_WindowMutator::UT_WindowMutator()
{
}

UT_WindowMutator::~UT_WindowMutator()
{
    printSuccessMessage( "rcWindow mutator test", mErrors );
}

uint32 UT_WindowMutator::run()
{
    setTest();
    copyTest();
    randomTest();
    mirrorTest();
    
    return mErrors;
}

void UT_WindowMutator::setTest ()
{
    const int32 depthCount = 3;
    rcPixel depths[depthCount] = { rcPixel8, rcPixel16, rcPixel32S };
    uint32 pixelMasks[depthCount] = { 0x000000FF, 0x0000FFFF, 0xFFFFFFFF };
   
    const int32 widthCount = 20;
    int32 widths[widthCount] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 21, 32, 33 };
    const int32 heightCount = 5;
    int32 heights[heightCount] = { 1, 2, 3, 10, 32 };
    uint32 testValue = 0x11223344;

    for (int32 d = 0; d < depthCount; d++)
    {
        // Setup base frame
        rcPixel depth = depths[d];
        rcFrameRef buf = new rcFrame( 640, 480, depth );
        uint32 pixelMask = pixelMasks[d];
        
        for (int32 w = 0; w < widthCount; w++)
        {
            int32 width = widths[w];

            for (int32 h = 0; h < heightCount; h++)
            {
                int32 height = heights[h];
                
                // Define 2 windows - 1 the window to do the "test" call to setAllPixels in, the other
                // its parent "guard window". The guard window provides a 1 pixel border around the
                // edge of the entire test window.
                //
                rcWindow guardWindow( buf, 0, 0, width+2, height+2 );
                rcWindow testWindow( guardWindow, 1, 1, width, height );
                
                guardWindow.setAllPixels(0);
                testWindow.setAllPixels(testValue);
                
                // Check guard pixels
                for (int32 x = 0; x < width+2; x++)
                {
                    rcUNITTEST_ASSERT(buf->getPixel(x, 0) == 0);
                    rcUNITTEST_ASSERT(buf->getPixel(x, height+1) == 0);
                }
                
                for (int32 y = 0; y < height+2; y++)
                {
                    rcUNITTEST_ASSERT(buf->getPixel(0, y) == 0);
                    rcUNITTEST_ASSERT(buf->getPixel(width+1, y) == 0);
                }
                
                // Check testWindow pixels
                uint32 expectedValue = testValue & pixelMask;

                for (int32 x = 1; x < width+1; x++)
                    for (int32 y = 1; y < height+1; y++)
		      {
                        rcUNITTEST_ASSERT(buf->getPixel(x, y) == expectedValue);
			rcUTCheck (buf ->operator () ((double) x, (double) y) == expectedValue);
		      }
            } // End of: for (int32 h = 0; h < heightCount; h++)
        } // End of: for (int32 w = 0; w < widthCount; w++)
    } // for (int32 d = 0; d < depthCount; d++)
}

void UT_WindowMutator::copyTest ()
{
    const int32 depthCount = 3;
    rcPixel depths[depthCount] = { rcPixel8, rcPixel16, rcPixel32S };
    uint32 pixelMasks[depthCount] = { 0x000000FF, 0x0000FFFF, 0xFFFFFFFF };
   
    const int32 widthCount = 20;
    int32 widths[widthCount] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 21, 32, 33 };
    const int32 heightCount = 5;
    int32 heights[heightCount] = { 1, 2, 3, 10, 32 };

    for (int32 d = 0; d < depthCount; d++)
    {
        // Setup base frame
        rcPixel depth = depths[d];
        rcFrameRef buf = new rcFrame( 640, 480, depth );
        rcFrameRef srcbuf = new rcFrame( 640, 480, depth );

        // Set up a source buffer with all unique pixel values (well, in 8 bit case it won't
        // be unique, but hopefully close enough -- maybe this can get revisited).
        // Note: widths and heights arrays need to always have last element as largest. Could
        // do search, but want to see this work.
        //
        uint32 pixelMask = pixelMasks[d];
        uint32 temp = 0;
        for (int32 x = 0; x < widths[widthCount-1]; x++)
            for (int32 y = 0; y < heights[heightCount-1]; y++)
                srcbuf->setPixel(x, y, (temp++ & pixelMask));
                 
        for (int32 w = 0; w < widthCount; w++)
        {
            int32 width = widths[w];

            for (int32 h = 0; h < heightCount; h++)
            {
                int32 height = heights[h];

                // Add junk to size of one or the other window to make sure window intersection
                // code works correctly.
                //
                uint32 extraWT, extraWS, extraHT, extraHS;                
                extraWT = (h & 1) ? 1 : 0;
                extraWS = (h & 1) ? 0 : h;
                extraHT = (w & 1) ? 1 : 0;
                extraHS = (w & 1) ? 0 : w;

                rcWindow guardWindow( buf, 0, 0, width+2, height+2 );
                rcWindow testWindow( guardWindow, 1, 1, width + extraWT, height + extraHT);
                rcWindow srcWindow( srcbuf, 0, 0, width + extraWS, height + extraHS);

                guardWindow.setAllPixels(0);
                rcWindow* ptr = &testWindow.copyPixelsFromWindow(srcWindow);
                rcUNITTEST_ASSERT(ptr == &testWindow);

                // Check guard pixels
                for (int32 x = 0; x < width+2; x++)
                {
                    rcUNITTEST_ASSERT(buf->getPixel(x, 0) == 0);
                    rcUNITTEST_ASSERT(buf->getPixel(x, height+1) == 0);
                }
                
                for (int32 y = 0; y < height+2; y++)
                {
                    rcUNITTEST_ASSERT(buf->getPixel(0, y) == 0);
                    rcUNITTEST_ASSERT(buf->getPixel(width+1, y) == 0);
                }
                
                // Check testWindow pixels
                for (int32 x = 0; x < width; x++)
                    for (int32 y = 0; y < height; y++)
                        rcUNITTEST_ASSERT(srcWindow.getPixel(x, y) == testWindow.getPixel(x, y));                
            } // End of: for (int32 h = 0; h < heightCount; h++)
        } // End of: for (int32 w = 0; w < widthCount; w++)
    } // for (int32 d = 0; d < depthCount; d++)}
}

// Random fill test
void UT_WindowMutator::randomTest ()
{
    const int32 maxWidth = 512;
    const int32 maxHeight = 512;

    for ( int32 width = 32; width < maxWidth; width*=2 ) {
        for ( int32 height = 32; height < maxHeight; height*=2 ) {
            rcFrameRef buf1 = new rcFrame( width, height, rcPixel32S );
            rcFrameRef buf2 = new rcFrame( width, height, rcPixel32S );
            rcFrameRef buf3 = new rcFrame( width, height, rcPixel8 );
            rcFrameRef buf4 = new rcFrame( width, height, rcPixel8 );
            
            rcWindow w1( buf1 );
            rcWindow w2( buf2 );
            rcWindow w3( buf3 );
            rcWindow w4( buf4 );
            
            testRandomFill( w1, w2 );
            testRandomFill( w3, w4 );
        }
    }
}

// Test random fill
void UT_WindowMutator::testRandomFill( rcWindow& w1, rcWindow& w2 )
{
    rcUNITTEST_ASSERT( w1.height() == w2.height() );
    rcUNITTEST_ASSERT( w1.width() == w2.width() );

    // Default seed is 0
    uint32 seed = w1.randomFill();
    uint32 seed2 = seed + 1;
    
    // Windows should have different pixels with seed 0
    uint32 diffs = 0;
    for (int32 x = 0; x < w1.width(); x++)
        for (int32 y = 0; y < w1.height(); y++)
            if ( w1.getPixel(x, y) != w2.getPixel(x, y) )
                ++diffs;
    // At least one pixel should be different
    if ( seed != seed2 ) {
        rcUNITTEST_ASSERT( diffs > 0 );
        if ( diffs == 0 )
            cerr << "Identical pixels in " << w1.width() << "x" << w1.height()
                 << "x" << w1.depth()*8 << " window with seeds "
                 << seed << " and " << seed2 << endl;
    } else {
        // Same seed, all pixels should be identical
        rcUNITTEST_ASSERT( diffs == 0 );
    }
    
    // Explicit seed 0
    seed = w1.randomFill( 0 );
    seed2 = seed + 2;

    // Windows should have different pixels with seed 0
    diffs = 0;
    for (int32 x = 0; x < w1.width(); x++)
        for (int32 y = 0; y < w1.height(); y++)
            if (  w1.getPixel(x, y) != w2.getPixel(x, y) )
                ++diffs;
    if ( seed != seed2 ) {
        // At least one pixel should be different    
        rcUNITTEST_ASSERT( diffs > 0 );
        if ( diffs == 0 )
              cerr << "Identical pixels in " << w1.width() << "x" << w1.height()
                 << "x" << w1.depth()*8 << " window with seeds "
                 << seed << " and " << seed2 << endl;
    } else {
        // Same seed, all pixels should be identical
        rcUNITTEST_ASSERT( diffs == 0 );
    }

    // Generate new seeds, two consequtive seeds should differ
    // All pixels should be the same for a non-zero seed
    for ( seed = 2; seed < 32; seed *= 2 ) {
        w1.randomFill( seed );
        w2.randomFill( seed );
        for (int32 x = 0; x < w1.width(); x++)
            for (int32 y = 0; y < w1.height(); y++)
                rcUNITTEST_ASSERT( w1.getPixel(x, y) == w2.getPixel(x, y));
        // Different seeds, different pixels
        seed2 = seed+1;
        w2.randomFill( seed2 );
        diffs = 0;
        for (int32 x = 0; x < w1.width(); x++)
            for (int32 y = 0; y < w1.height(); y++)
                if ( w1.getPixel(x, y) != w2.getPixel(x, y))
                    ++diffs;
        rcUNITTEST_ASSERT( diffs > 0 );
        if ( diffs == 0 )
             cerr << "Identical pixels in " << w1.width() << "x" << w1.height()
                 << "x" << w1.depth()*8 << " window with seeds "
                 << seed << " and " << seed2 << endl;
    }
}

// Test mirror operations
void UT_WindowMutator::mirrorTest()
{
    int32 maxWidth = 4096;
    int32 maxHeight = 255;

    // Produce images of varying dimensions

    // Test for 8-bit images
    for ( int32 incr = 0; incr < 3; ++incr ) {
        for ( int32 width = 1; width < maxWidth; width = width*2 + incr ) {
            for ( int32 height = 1; height < maxHeight; height = height*2 + incr ) {
                rcFrameRef buf8 = new rcFrame( width, height, rcPixel8 );
                rcWindow w8( buf8 );
                testMirror( w8 );
            }
        }
    }

    // Test for 16-bit and 32-bit images
    maxWidth = 2048;
    maxHeight = 2048;
    for ( int32 incr = 0; incr < 3; ++incr ) {
        for ( int32 width = 1; width < maxWidth; width = width*3 + incr ) {
            for ( int32 height = 1; height < maxHeight; height = height*4 + incr ) {
                rcFrameRef buf32 = new rcFrame( width, height, rcPixel32S );
                rcFrameRef buf16 = new rcFrame( width, height, rcPixel16 );
                rcWindow w32( buf32 );
                rcWindow w16( buf16 );
                testMirror( w32 );
                testMirror( w16 );
            }
        }
    }
}

// Mirror one window
void UT_WindowMutator::testMirror( rcWindow& w )
{
    const uint32 lastRow = w.height() - 1;
    
    // Pixel value = row number
    for (int32 x = 0; x < w.width(); x++)
        for (int32 y = 0; y < w.height(); y++)
            w.setPixel(x, y, y);
    
    // Mirror vertically
    w.mirror();
    
    // Rows should be now reversed
    for (int32 x = 0; x < w.width(); x++)
        for (int32 y = 0; y < w.height(); y++)
            rcUNITTEST_ASSERT( w.getPixel(x, y) == (lastRow - y) );
                
}
    
static void rcReturn (rcWindow& src)
{
  rcWindow newWindow (100,100,rcPixel8);
    src = newWindow;
}

static rcWindow rcReturn2 ()
{
    rcWindow newWindow (100,100,rcPixel8);
    return newWindow;
}

