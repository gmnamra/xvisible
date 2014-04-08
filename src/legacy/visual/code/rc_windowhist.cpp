/*
 *  rc_windowhist.cpp
 *  framebuf
 *
 *  Created by Peter Roberts on Tue May 14 2002.
 *  Copyright (c) 2002 Reify, Inc. All rights reserved.
 *
 */

#include <rc_window.h>

vector<uint32>& rfGenDepth8Histogram(const rcWindow& src, vector<uint32>& histogram)
{
  rmAssert(src.depth() == rcPixel8);
  rmAssert(histogram.size () == 256);


//   if (rfHasSIMD())
//     {
//       vImage_Buffer vb;
//       if (src.vImage (vb))
// 	{
// 	  vImage_Error ve = vImageHistogramCalculation_Planar8(&vb, &histogram[0], kvImageNoFlags);

// 	  if (ve == kvImageNoError)
// 	    return histogram;
// 	}
//     }

    
    uint32 lastRow = src.height() - 1, row = 0;
    const uint32 opsPerLoop = 8;
    uint32 unrollCnt = src.width() / opsPerLoop;
    uint32 unrollRem = src.width() % opsPerLoop;
    
    for (uint32 i = 0; i < 256; i++)
        histogram[i] = 0;
        
    for ( ; row <= lastRow; row++)
    {
        const uint8* pixelPtr = src.rowPointer(row);
                
        for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
        {
            histogram[*pixelPtr++]++;
            histogram[*pixelPtr++]++;
            histogram[*pixelPtr++]++;
            histogram[*pixelPtr++]++;
            histogram[*pixelPtr++]++;
            histogram[*pixelPtr++]++;
            histogram[*pixelPtr++]++;
            histogram[*pixelPtr++]++;
        }
                
        for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++)
           histogram[*pixelPtr++]++;
    }
    
    return histogram;
}
