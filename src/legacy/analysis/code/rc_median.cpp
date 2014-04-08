/*
 *  median.cpp
 *  framebuf
 *
 *  Created by Arman Garakani on Thu Sep 05 2002.
 *  Copyright (c) 2002 Reify. All rights reserved.
 *
 */

#include <rc_window.h>
#include <rc_ip.h>

/*
 A reasonably efficient C implementation of the true 3x3 median
 filter. This routine uses the TI algorithm as documented in the
 DSP example code.

 1. Compute the min, max and median for each column

 2. Find the minimum of the 3 column max's, the maximum of the
 3 column min's, and the median of the 3 column medians.

 3. The median of the 3x3 is then found as:

 med(minMax, maxMin, medMed)

 This is a clever algorithm because no 9 point sort is required
 and only 3 pt medians are needed. Additionally each column term
 can be reused over three neighborhoods.
 */

void rfMedian3By3Depth8(const rcWindow& srcImg, rcWindow& dstImg);

extern void rfCopyWindowBorder(rcWindow& win, const rcWindow& src);


void rfMedian (const rcWindow& srcImg, rcWindow& dstImg)
{
   assert (srcImg.isBound());
   assert (dstImg.isBound());
   assert (srcImg.width() == (dstImg.width()));
   assert (srcImg.height() == (dstImg.height()));

   rfCopyWindowBorder(dstImg, srcImg);
   
   rcWindow dwin (dstImg, 1, 1, dstImg.width() - 2, dstImg.height() - 2);

   rfMedian3By3Depth8(srcImg, dwin);
}




#define rmMedOf3(a, b, c)                                       \
((a) > (b) ? ((a) < (c) ? (a) : ((b) < (c) ? (c) : (b))) :    \
 ((a) > (c) ? (a) : ((b) < (c) ? (b) : (c))))

#define rmMaxOf3(a, b, c) (rmMax(rmMax(a, b), c))

#define rmMinOf3(a, b, c) (rmMin(rmMin(a, b), c))

void rfMedian3By3Depth8(const rcWindow& srcImg, rcWindow& dstImg)
{
   const uint8  *srcPtr1, *srcPtr2, *srcRowBase;
   uint8        *dstPtr, *dstRowBase;
   uint8         pelFIFO[9], maxOfMin, minOfMax, medOfMed;
   uint8         colMin[3], colMax[3], colMed[3];
   int             i, j, k, m, a, b, c;

   assert (srcImg.isBound());
   assert (dstImg.isBound());
   assert (srcImg.width() == (dstImg.width() + 2));
   assert (srcImg.height() == (dstImg.height() + 2));


   int srcRUC = srcImg.rowUpdate();
   int dstRUC = dstImg.rowUpdate();
   int rowIter = srcImg.height() - 2;
   int colIter = srcImg.width() - 2 - 1;

   srcRowBase = srcImg.rowPointer(0);
   dstRowBase = dstImg.rowPointer(0);

   for(i = rowIter; i;
       i--, srcRowBase += srcRUC, dstRowBase += dstRUC)
   {
      dstPtr = dstRowBase;

      // Initialize FIFO for current row pass
      srcPtr1 = srcRowBase;
      for(j = 3, k = 0; j; j--, srcPtr1++)
      {
         srcPtr2 = srcPtr1;
         for(m = 3; m; m--, srcPtr2 += srcRUC, k++)
            pelFIFO[k] = *srcPtr2;
      }

      // Initalize column miniums
      colMin[0] = rmMinOf3(pelFIFO[0], pelFIFO[1], pelFIFO[2]);
      colMin[1] = rmMinOf3(pelFIFO[3], pelFIFO[4], pelFIFO[5]);
      colMin[2] = rmMinOf3(pelFIFO[6], pelFIFO[7], pelFIFO[8]);

      // Initalize column maximums
      colMax[0] = rmMaxOf3(pelFIFO[0], pelFIFO[1], pelFIFO[2]);
      colMax[1] = rmMaxOf3(pelFIFO[3], pelFIFO[4], pelFIFO[5]);
      colMax[2] = rmMaxOf3(pelFIFO[6], pelFIFO[7], pelFIFO[8]);

      // Initalize column medians
      colMed[0] = rmMedOf3(pelFIFO[0], pelFIFO[1], pelFIFO[2]);
      colMed[1] = rmMedOf3(pelFIFO[3], pelFIFO[4], pelFIFO[5]);
      colMed[2] = rmMedOf3(pelFIFO[6], pelFIFO[7], pelFIFO[8]);

      // Compute column partial orderings
      maxOfMin = rmMaxOf3(colMin[0], colMin[1], colMin[2]);
      minOfMax = rmMinOf3(colMax[0], colMax[1], colMax[2]);
      medOfMed = rmMedOf3(colMed[0], colMed[1], colMed[2]);

      // Compute final median for inital row destination
      *dstPtr = rmMedOf3(maxOfMin, minOfMax, medOfMed);

      for(j = colIter, dstPtr++, m = 0, k = 0;
          j; j--, dstPtr++, srcPtr1++, k += 3, m++)
      {
         if(m == 3)
            m = k = 0;

         // Update current column buffers
         srcPtr2 = srcPtr1;
         a = *srcPtr2;
         srcPtr2 += srcRUC;
         b = *srcPtr2;
         srcPtr2 += srcRUC;
         c = *srcPtr2;

         colMin[m] = rmMinOf3(a, b, c);
         colMax[m] = rmMaxOf3(a, b, c);
         colMed[m] = rmMedOf3(a, b, c);

         // Compute column partial orderings
         maxOfMin = rmMaxOf3(colMin[0], colMin[1], colMin[2]);
         minOfMax = rmMinOf3(colMax[0], colMax[1], colMax[2]);
         medOfMed = rmMedOf3(colMed[0], colMed[1], colMed[2]);

         // Find median 3x3 as the median of the partial min, max, med
         *dstPtr = rmMedOf3(maxOfMin, minOfMax, medOfMed);
      }
   }
}
