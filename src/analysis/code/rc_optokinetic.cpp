/*
 *  optokinetic.cpp
 *  correlation
 *  $Id: rc_optokinetic.cpp 7297 2011-03-07 00:17:55Z arman $
 *  Created by Arman Garakani on Tue Jun 04 2002.
 *  Copyright (c) 2002 Reify Corp. . All rights reserved.
 *
 *  $Log$
 *  Revision 1.35  2004/01/18 22:27:11  proberts
 *  new muscle tracker and support code
 *
 *  Revision 1.34  2003/04/18 17:57:47  sami
 *  rcAnalysisProgressIndicator is now rcProgressIndicator
 *
 *  Revision 1.33  2003/02/24 20:53:23  sami
 *  Removed debug printouts
 *
 *  Revision 1.32  2003/02/19 15:29:42  arman
 *  Added lut declaration
 *
 *  Revision 1.31  2003/02/18 21:09:12  arman
 *  added log
 *
 */

//TBD: Higher level function, individual accumulation, normalization, and a mask display function

#include <rc_window.h>
#include <rc_edge.h>
#include <rc_analysis.h>
#include <rc_ip.h>
#include <rc_vector2d.h>
#include <vector>
#include <rc_qtime.h>
#include <rc_imageprocessing.h>
#include <iomanip.h>
#include <rc_similarity.h>

using namespace std;

#define cmPrintSpace(a){ \
   for (int i = 0; i < (a).height(); i++) \
   { \
      fprintf (stderr, "\n"); \
         for (int j = 0; j < (a).width(); j++) \
            fprintf (stderr, " % 6d ", (a).getPixel (j, i)); \
               fprintf (stderr, "\n");\
   }}



/// Processing Function
// Return true if analysis finished, false if it was aborted
bool rfOptoKineticEnergy (vector<rcWindow>& images, vector<double>& energy, const rsCorrParams& params,
                          rcProgressIndicator* pIndicator )
{
   assert (images.size () > 1);
   assert (images.size () == energy.size ());
   vector<rcWindow> locals(images.size());
   vector<double> scoreTotals (images.size());
   vector<vector<double> > scores(images.size());
   const double tinyNum = 1e-10; // When using rcSumPlogPNorm, added to every score to guarantee non-zero value
   bool finished = true;
   
   // O (n^2/2 - n)
   // Pre-initialize with unity for identity cross correlate
   for (unsigned int i = 0; i < images.size(); i++) energy[i] = 1.0;
   
   if (params.pp != rcPreNone)
   {
      rmAssert (1);  // Not implemented
   }
   else if (images[0].depth() > rcPixel8 ) {
       switch ( params.rc ) {
           case rcColorReduceAll32to8:
               // Reduce all color and gray 24/32-bit images to 8-bit gray images
               rfRcWindow32to8(images, locals); 
               break;
           case rcColorReduceGray32to8:
               // Reduce depth of all gray 24/32-bit images to 8-bit gray images
               if ( images[0].isGray() )
                   rfRcWindow32to8(images, locals); 
               else
		 locals = images; // Analyze all bytes of color originals
               break;
           case rcColorUse32:
               // Analyze all bytes of all originals
	       locals = images;
               break;
           default:
               rmAssert( 1 );
               break;
       }
   }
   else
      locals = images;
   
   if (params.em == rcSumPlogPNorm)
   {
     for (unsigned int i = 0; i < images.size(); i++)
     {
       scores[i].resize(scores.size());
       scores[i][i] = scoreTotals[i] = 1 + tinyNum;
       energy[i] = 0;
     }
   }
   else if (params.em == rcSumPlogP)
     // Pre-initialize with unity for identity cross correlate
     for (unsigned int i = 0; i < images.size(); i++) energy[i] = 0.0; // Because log2(1) == 0
   else
     // Pre-initialize with unity for identity cross correlate
     for (unsigned int i = 0; i < images.size(); i++) energy[i] = 1.0;

   // Progress update variables
   uint32 lastProgress = 0;
   const uint32 imageSize = images[0].width() * images[0].height();
   // Progress update after this many pixels have been processed
   const uint32 updateLimit = 2000000;
   uint32 processedPixels = 0;


   for (unsigned int i = 0; i < images.size (); i++)
   {
       for (unsigned int j = i + 1; j < images.size(); j++)
       {
           rcCorr res;
           
           switch (params.match)
           {
               default:
               case rcNormalizedCorr:
                   rfCorrelate (locals[i], locals[j], params, res);
                   break;
               case rcIntFrequencyDist:
               {
                   rcWindow iw (locals[j], 1, 1, locals[j].width() - 2, locals[j].height() - 2);
                   double minn (1.0), maxx (0.0);
                   for (uint32 p = 0; p < 3; p++)
                       for (uint32 q = 0; q < 3; q++)
                       {
                           rcWindow jw (locals[i], p, q, locals[i].width() - 2, locals[i].height() - 2);
                           rfCorrelate (iw, jw, params, res);
                           if (res.r() <= minn) minn = res.r();
                           if (res.r() >= maxx) maxx = res.r();
                       }
                   res.r(maxx - minn);
                   assert (res.r() >= 0 && res.r() <= 1.0);
               }
               break;
           }

           switch (params.em)
           {
               case rcSumSquare:
                   energy[i] += res.r();
                   energy[j] += res.r();
                   break;
               case rcSumPlogP:
		 {
                       double rr = res.r() + 0.0000000000001;
                       energy[i] += -1.0 * rr * log2 (rr);
                       energy[j] += -1.0 * rr * log2 (rr);
		 }
		 break;
               default:
               case rcSumPlogPNorm:
               {
                   double rr = res.r() + tinyNum;
                   scoreTotals[i] += rr;
                   scoreTotals[j] += rr;
                   scores[i][j] = rr;
                   scores[j][i] = rr;
               }
               break;
           }
         
           if ( pIndicator ) {
               processedPixels += imageSize;
               // Update frequency depends on total number of processed pixels
               if ( processedPixels > updateLimit ) {
                   processedPixels -= updateLimit;
                   uint32 percent = uint32(double(i+1)/images.size() * 100.0);
                   // However, minimum progress increment is 1%
                   if ( percent > lastProgress ) {
                       bool abort = pIndicator->progress( percent );
                       lastProgress = percent;
                       if ( abort ) {
                           finished = false;
                           goto abort;
                       }
                   }
               }
           }
       }
   }

   if (params.em == rcSumPlogPNorm)
   {
       for (unsigned int i = 0; i < images.size(); i++)
       {
           for (unsigned int j = i; j < images.size(); j++)
           {
               double rr = scores[i][j]/scoreTotals[i]; // Normalize for total energy in samples
               energy[i] += -1.0 * rr * log2 (rr);

               if (i != j)
               {
                   rr = scores[i][j]/scoreTotals[j]; // Normalize for total energy in samples
                   energy[j] += -1.0 * rr * log2 (rr);
               }
           } // End of : for (unsigned int j = i; j < images.size(); j++)
           
           energy[i] = energy[i]/log2(images.size()); // Normalize for number of samples
       } // End of: for (unsigned int i = 0; i < images.size(); i++)
   }
   else
   {
       double n (images.size());
       
       for (unsigned int i = 0; i < energy.size(); i++) energy[i] = fabs(energy[i]) / n;
   }


   // Reporting: Derivative (raw) or cumulative
   if (params.rp == rcTimeIntegral)
   {
       rfIntegrateEnergy (energy);
   }
  abort:
   // Abort analysis, results are not guaranteed to be in a consistent state

   return finished;
}

void rfIntegrateEnergy (vector<double>& energy)
{
  assert (energy.size());
  double sum (0.0);

  for (unsigned int i = 0; i < energy.size(); i++)
    {
      sum += energy[i];
      energy[i] = sum;
    }
}


// Precomputed squares
static const rcSquareTable squareTable;

void rfOptoKineticVariance (vector <rcWindow>& sequence, rcWindow& varImage)
{
   rmAssert (sequence.size());
   rmAssert (sequence.size() <= 257); // guarantee no numerical overflow
   uint32 n (sequence.size());
   
   // Check the sizes
   uint32 width = sequence[0].width();
   uint32 height = sequence[0].height();

   rcWindow sum (width, height, rcPixel16);
   rcWindow sumsq (width, height, rcPixel32);
   sum.setAllPixels (0);
   sumsq.setAllPixels (0);
   
   // Produce Sum and Sum sq

   for (vector<rcWindow>::iterator img = sequence.begin(); img != sequence.end(); img++)
   {
      for (uint32 j = 0; j < height; j++)
      {
         uint8 *pelRow = img->rowPointer (j);
         uint16 *sumRow = (uint16 *) sum.rowPointer (j);
         uint32 *sumSqRow = (uint32 *) sumsq.rowPointer (j);

         for (uint32 i = 0; i < width; i++, pelRow++, sumRow++, sumSqRow++)
         {
            *sumRow += *pelRow;
            *sumSqRow += squareTable [*pelRow];
         }
      }
   }

   // Now produce variance using:  variance = n * sumSq - sum * sum
   for (uint32 j = 0; j < height; j++)
   {
      uint16 *sumRow = (uint16 *) sum.rowPointer (j);
      uint32 *sumSqRow = (uint32 *) sumsq.rowPointer (j);

      for (uint32 i = 0; i < width; i++, sumRow++, sumSqRow++)
      {
         uint32 var = *sumSqRow * n;
         uint32 tmp = *sumRow;
         var = var - tmp * tmp;
         *sumSqRow = var;
      }
   }

   varImage = sumsq;
         
}

