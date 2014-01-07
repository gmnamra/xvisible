/*
 *  ut_windowhist.cpp
 *  framebuf
 *
 *  Created by Peter Roberts on Tue May 14 2002.
 *  Copyright (c) 2002 Reify, Inc. All rights reserved.
 *
 */

#include <stdlib.h>
#include "ut_windowhist.h"
#include <rc_framebuf.h>
#include <rc_window.h>
#include <rc_windowhist.h>
#include <rc_time.h>



UT_WindowHistogram::UT_WindowHistogram()
{
}

UT_WindowHistogram::~UT_WindowHistogram()
{
    printSuccessMessage( "rcWindow histogram test", mErrors );
}

uint32 UT_WindowHistogram::run()
{
    hist8Test();
	
#if defined (PERFORMANCE)	
    histTime (512, 512);
    histTime (1280, 960);
#endif
	
    return mErrors;
}

void UT_WindowHistogram::hist8Test ()
{   
    const uint32 widthCount = 20;
    uint32 widths[widthCount] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 21, 32, 33 };
    const uint32 heightCount = 5;
    uint32 heights[heightCount] = { 1, 2, 3, 10, 32 };
    
    rcSharedFrameBufPtr buf = new rcFrame( 640, 480, rcPixel8 );
    rcWindow mainWindow(buf, 0, 0, 640, 480);
    rcWindow subWindow(mainWindow, 0, 0, 1, 1);
    rcWindow guardWindow(mainWindow, 0, 0, 1, 1);
    rcWindow copyWindow(mainWindow, 0, 0, 1, 1);
    const uint32 xOffMax = 16;
    const uint32 yOffMax = 8;
    
    // Put random pixel values into frame.
    srand(0);    
    for (uint32 y = 0; y < 64; y++)
        for (uint32 x = 0; x < 64; )
        {
            const uint32 mask = 0xFF;
            uint32 newValue = rand();
            for (uint32 i = 0; i < 4; i++, x++)
            {
                buf->setPixel(x, y, newValue & mask);
                newValue >>= 8;
            }
        }
        
    // Now generate histograms and compare against expected values
    for (uint32 w = 0; w < widthCount; w++)
    {
        uint32 width = widths[w];

        for (uint32 h = 0; h < heightCount; h++)
        {
            uint32 height = heights[h];
            
            for (uint32 xOff = 0; xOff < xOffMax; xOff++)
            {
                for (uint32 yOff = 0; yOff < yOffMax; yOff++)
                {
		  rc256BinHist expHistogram(256), hist(256);
		  rc256BinHist *histP;

                    subWindow.windowRelativeTo( mainWindow, xOff, yOff, width, height );
                
                    for (uint32 i = 0; i < 256; i++)
                        expHistogram[i] = 0;
                    
                    for (uint32 x = 0; x < width; x++)
                        for (uint32 y = 0; y < height; y++)
                            expHistogram[subWindow.getPixel(x, y)]++;
            
                    histP = &rfGenDepth8Histogram(subWindow, hist);
                    rcUNITTEST_ASSERT(histP == &hist);
 
                    for (uint32 i = 0; i < 256; i++)
                        rcUNITTEST_ASSERT(expHistogram[i] == hist[i]);
                } // End of: for (uint32 yOff = 0; yOff < 2; yOff++)
            } // End of: for (uint32 xOff = 0; xOff < 2; xOff++)
        } // End of: for (uint32 h = 0; h < heightCount; h++)
    } // End of: for (uint32 w = 0; w < widthCount; w++)
    
    // Do tests with setAllPixels()
    for (uint32 w = 0; w < widthCount; w++)
    {
        uint32 width = widths[w];

        for (uint32 h = 0; h < heightCount; h++)
        {
            uint32 height = heights[h];
            
            for (uint32 xOff = 0; xOff < xOffMax; xOff++)
            {
                for (uint32 yOff = 0; yOff < yOffMax; yOff++)
                {
                    rc256BinHist hist(256);
                    uint32 pixelValue = xOff*yOffMax + yOff + 1;
                    rcUNITTEST_ASSERT(pixelValue < 256);
                    
                    guardWindow.windowRelativeTo( mainWindow, xOff, yOff, width+2, height+2 );
                    subWindow.windowRelativeTo( guardWindow, 1, 1, width, height );
                
                    guardWindow.setAllPixels(0);
                    rfGenDepth8Histogram(guardWindow, hist);
                    rcUNITTEST_ASSERT(guardWindow.pixelCount() == hist[0]);
                    for (uint32 i = 1; i < 256; i++)
                        rcUNITTEST_ASSERT(0 == hist[i]);

                    subWindow.setAllPixels(pixelValue);
                    rfGenDepth8Histogram(guardWindow, hist);
                    rcUNITTEST_ASSERT((guardWindow.width()*2 + guardWindow.height()*2 - 4) == int32(hist[0]));
                    for (uint32 i = 1; i < 256; i++)
                        if (i == pixelValue)
                            rcUNITTEST_ASSERT(subWindow.pixelCount() == hist[i]);
                        else
                            rcUNITTEST_ASSERT(0 == hist[i]);

                    rfGenDepth8Histogram(subWindow, hist);
                    for (uint32 i = 0; i < 256; i++)
                        if (i == pixelValue)
                            rcUNITTEST_ASSERT(subWindow.pixelCount() == hist[i]);
                        else
                            rcUNITTEST_ASSERT(0 == hist[i]);
                } // End of: for (uint32 yOff = 0; yOff < 2; yOff++)
            } // End of: for (uint32 xOff = 0; xOff < 2; xOff++)
        } // End of: for (uint32 h = 0; h < heightCount; h++)
    } // End of: for (uint32 w = 0; w < widthCount; w++)

    // Do tests with copyPixelsFromWindow()
    for (uint32 w = 0; w < widthCount; w++)
    {
        uint32 width = widths[w];

        for (uint32 h = 0; h < heightCount; h++)
        {
            uint32 height = heights[h];
            
            for (uint32 xOff = 0; xOff < xOffMax; xOff++)
            {
                for (uint32 yOff = 0; yOff < yOffMax; yOff++)
                {
                    rc256BinHist hist(256);
                    uint32 pixelValue = xOff*yOffMax + yOff + 1;
                    rcUNITTEST_ASSERT(pixelValue < 256);
                    
                    copyWindow.windowRelativeTo( mainWindow, 300, 300, width, height );
                    guardWindow.windowRelativeTo( mainWindow, xOff, yOff, width+2, height+2 );
                    subWindow.windowRelativeTo( guardWindow, 1, 1, width, height );
                
                    copyWindow.setAllPixels(0);
                    guardWindow.setAllPixels(pixelValue);
                    rfGenDepth8Histogram(guardWindow, hist);
                    for (uint32 i = 0; i < 256; i++)
                        if (i == pixelValue)
                            rcUNITTEST_ASSERT(guardWindow.pixelCount() == hist[i]);
                        else
                            rcUNITTEST_ASSERT(0 == hist[i]);

                    subWindow.copyPixelsFromWindow(copyWindow);
                    rfGenDepth8Histogram(guardWindow, hist);
                    rcUNITTEST_ASSERT(subWindow.pixelCount() == hist[0]);
                    for (uint32 i = 1; i < 256; i++)
                        if (i == pixelValue)
                            rcUNITTEST_ASSERT((guardWindow.width()*2 + guardWindow.height()*2 - 4) == 							int32(hist[i]));
                        else
                            rcUNITTEST_ASSERT(0 == hist[i]);

                    rfGenDepth8Histogram(subWindow, hist);
                    rcUNITTEST_ASSERT(subWindow.pixelCount() == hist[0]);
                    for (uint32 i = 1; i < 256; i++)
                        rcUNITTEST_ASSERT(0 == hist[i]);
                } // End of: for (uint32 yOff = 0; yOff < 2; yOff++)
            } // End of: for (uint32 xOff = 0; xOff < 2; xOff++)
        } // End of: for (uint32 h = 0; h < heightCount; h++)
    } // End of: for (uint32 w = 0; w < widthCount; w++)
}


void UT_WindowHistogram::histTime ( uint32 width, uint32 height )
{
   rcWindow win (width, height);
   win.randomFill ();

   rcTime timer;
   rc256BinHist hist(256);
   timer.start ();
   rfGenDepth8Histogram (win, hist);
   timer.end ();

   fprintf (stderr, "Performance: Histogram for [%d x %d x %d] in %3.3f ms, %3.3f MB/s, %.2f fps\n", 	
            win.width (), win.height(), win.depth() *8, timer.milliseconds(), (win.height() * win.width())/timer.microseconds(), 1000/timer.milliseconds());   
   
}



