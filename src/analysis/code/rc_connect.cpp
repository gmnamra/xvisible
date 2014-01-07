/*
 *  connect.cpp
 *  framebuf
 *
 *  Created by Arman Garakani on Mon Aug 19 2002.
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_window.h>

#include <rc_histstats.h>
#include <rc_polygon.h>
#include <map>

//----------------------------------------------------------------------------
// Connected component labeling.
//
// The 1D connected components of each row are labeled first.  The labels
// of row-adjacent components are merged by using an associative memory
// scheme.  The associative memory is stored as an array that initially
// represents the identity permutation M = {0,1,2,...}.  Each association of
// i and j is represented by applying the transposition (i,j) to the array.
// That is, M[i] and M[j] are swapped.  After all associations have been
// applied during the merge step, the array has cycles that represent the
// connected components.  A relabeling is done to give a consecutive set of
// positive component labels.
//
// For example:
//
//     Image       Rows labeled
//     00100100    00100200
//     00110100    00330400
//     00011000    00055000
//     01000100    06000700
//     01000000    08000000
//
// Initially, the associated memory is M[i] = i for 0 <= i <= 8.  Background
// label is 0 and never changes.
// 1. Pass through second row.
//    a. 3 is associated with 1, M = {0,3,2,1,4,5,6,7,8}
//    b. 4 is associated with 2, M = {0,3,4,1,2,5,6,7,8}
// 2. Pass through third row.
//    a. 5 is associated with 3, M = {0,3,4,5,2,1,6,7,8}.  Note that
//       M[5] = 5 and M[3] = 1 have been swapped, not a problem since
//       1 and 3 are already "equivalent labels".
//    b. 5 is associated with 4, M = {0,3,4,5,1,2,6,7,8}
// 3. Pass through fourth row.
//    a. 7 is associated with 5, M = {0,3,4,5,1,7,6,2,8}
// 4. Pass through fifth row.
//    a. 8 is associated with 6, M = {0,3,4,5,1,7,8,2,6}
//
// The M array has two cycles.  Cycle 1 is
//   M[1] = 3, M[3] = 5, M[5] = 7, M[7] = 2, M[2] = 4, M[4] = 1
// and cycle 2 is
//   M[6] = 8, M[8] = 6
// The image is relabeled by replacing each pixel value with the index of
// the cycle containing it.  The result for this example is
//     00100100
//     00110100
//     00011000
//     02000100
//     02000000
//----------------------------------------------------------------------------

static void AddToAssociative (uint32 i0, uint32 i1, vector<uint32>& amm);

void rfGetComponents (const rcWindow& src, uint32& regions, rcWindow& lComponents)
{
   // Create a temporary copy of image to store intermediate information
   // during component labeling.  The original image is embedded in an
   // image with two more rows and two more columns so that the image
   // boundary pixels are properly handled.
   // To prevent duplicate clears, when src is 32 bit we are assuming that it is 2 pixels
   // larger already. This is a hack to be cleanly handled 

  rcWindow buffer, label;
  int32 iX, iY;



  if (src.depth() == rcPixel8)
    {
      buffer = rcWindow (src.width()+2,src.height()+2, rcPixel32);  // initially zero
      buffer.setAllPixels(uint32(0));
      label = rcWindow (buffer, 1, 1, src.width(), src.height ());

      for (iY = 0; iY < src.height(); ++iY)
	{
	  const uint8 *row = src.rowPointer (iY);
	  uint32 *brow = (uint32 *) label.rowPointer (iY);
	  for (iX = 0; iX < src.width(); ++iX, ++row,   ++brow)
	    {
	      if ( *row )
		*brow = 1;
	    }
	}
    }
  else if (src.depth() == rcPixel16)
    {
      buffer = rcWindow (src.width()+2,src.height()+2, rcPixel32);  // initially zero
      buffer.setAllPixels(uint32(0));
      label = rcWindow (buffer, 1, 1, src.width(), src.height ());


      for (iY = 0; iY < src.height(); ++iY)
	{
	  const uint16 *row = (uint16 *) src.rowPointer (iY);
	  uint32 *brow = (uint32 *) label.rowPointer (iY);
	  for (iX = 0; iX < src.width(); ++iX)
	    {
          // Buffer was initialized with 0's so only set non-zero pixels
          if ( *row )
              *brow = 1; // Change to a map look up table with no conditional
          ++row;
          ++brow;
	    }
	}
    }

  else if (src.depth() == rcPixel32)
    {
      buffer = src;
      label = rcWindow (buffer, 1, 1, src.width()-2, src.height ()-2);
    }
  else
    rmAssert (0);

   
   // label connected components in 1D array
   uint32 i, iComponent = 0;

   for (iY = 1; iY < buffer.height() - 1; ++iY)
   {
      // Next row, second column
      uint32 *brow = (uint32 *) buffer.rowPointer (iY);
      ++brow;

      // Label runs with an incrementing component number
      for (iX = 0; iX < buffer.width() - 1; ++iX)
      {
         if (brow[iX])
         {
            ++iComponent;
            while ((iX < (buffer.width() - 1)) && brow[iX])
            {
               brow[iX] = iComponent;
               ++iX;
            }
         }
      }
   }

   // If there are no non zero pels
   // Labled component image is not assigned a real one
   if ( iComponent == 0 )
   {
      regions = 0;
      return;
   }

   // associative memory for merging
   vector<uint32> amm (iComponent+1);
   for (i = 0; i < iComponent + 1; ++i)
      amm[i] = i;

   // Merge equivalent components.  Pixel (x,y) has previous neighbors
   // (x-1,y-1), (x,y-1), (x+1,y-1), and (x-1,y) [4 of 8 pixels visited
   // before (x,y) is visited, get component labels from them].

   const uint32 rup = buffer.rowUpdate () / rcPixel32;		// rup in ints;
   
   for (iY = 1; iY < buffer.height()-1; ++iY)
   {
      uint32 *brow = (uint32 *) buffer.rowPointer (iY);
      ++brow;
      
      for (iX = 1; iX < buffer.width()-1; ++iX)
      {
         const uint32 iValue = *brow++; // at +1 after this
         if ( iValue > 0 )
         {
            AddToAssociative(iValue, brow [-rup - 2], amm); // brow[-rup - 2]
            AddToAssociative(iValue, brow [-rup - 1], amm); // brow [-rup - 1]
            AddToAssociative(iValue, brow [-rup], amm); // brow [-rup]
            AddToAssociative(iValue, brow [rup - 2],amm); // brow [rup - 2]
         }
      }
   }

   // replace each cycle of equivalent labels by a single label
   regions = 0;
   for (i = 1; i <= iComponent; ++i)
   {
      if ( i <= amm[i] )
      {
         regions++;
         uint32 iCurrent = i;
         while ( amm[iCurrent] != i )
         {
            const uint32 iNext = amm[iCurrent];
            amm[iCurrent] = regions;
            iCurrent = iNext;
         }
         amm[iCurrent] = regions;
      }
   }

   // pack a relabeled image in smaller size output
   // create a rcWindow at 1,1 and label according to the associative memory TBD
   
   for (iY = 0; iY < label.height(); ++iY)
   {
      uint32* brow = (uint32 *) label.rowPointer (iY);
      
      for (iX = 0; iX < label.width(); ++iX, ++brow)
      {
         *brow = amm[*brow]; // *brow++ here appears to be a compiler bug
      }
   }

   // Out the label image
   lComponents = label;
}


static void AddToAssociative (uint32 i0, uint32 i1, vector<uint32>&amm) 
{
   // Adjacent pixels have labels i0 and i1. Associate them so that they
   // represent the same component.  [assert: i0 > 0]
   rmAssertDebug( i0 );
   
   if ( i1 == 0 )
      return;

   uint32 iSearch = i1;
   do
   {
      iSearch = amm[iSearch];
   }
   while ( iSearch != i1 && iSearch != i0 );

   if ( iSearch == i1 )
   {
      uint32 iSave = amm[i0];
      amm[i0] = amm[i1];
      amm[i1] = iSave;
   }
}

/*	*************************
	*                       *
	*     RLE Process       *
	*                       *
	*************************
*/

// Get enclosing rects from label image
void rfGetRects (const rcWindow& label, uint32 nLabels, const rcIPair& minSize,
                 bool combineRects, vector<rcIRect>& gRects,
		 vector<uint32>& gRectLabel)
{
    // Check that label is the right depth and that we have labels
    rmAssert (label.depth() == rcPixel32);
    rmAssert (label.isBound());
    rmAssert (nLabels);

    // Regardless of label is a window or not, lower right should be initialized to zero
    const int height = static_cast<int>( label.height() );
    const int width = static_cast<int>( label.width() );
    rcIPair zeropair (0,0);
    vector<rcIPair> lrp (nLabels+1,zeropair);
    vector<rcIPair> ulp (nLabels+1,label.size());

    /*
     * The following computes the min containing rectangle for 
     * the connected region. That is for the integer top left position
     * of the most top left (lowest x and lowest y) pixel position in the runs and
     * the most bottom right (highest x and highest y) pixel position in 
     * the runs. 
     * The integer bounding box is at upper left x and y and has width and height
     * of lower - upper + 1. 
     */

    for (int32 j = 0; j < height; ++j)
    {
        uint32* pSrc = (uint32*) label.rowPointer(j);
        for (int32 i=0; i < width; ++i, ++pSrc)
        {
            const uint32 l = *pSrc;
            // Note: first rect is at index 1 to avoid computing l-1 inside the inner loop
            if (l)
            {
                // Update min enclosing rectangle
                if (i < ulp[l].x())
                    ulp[l].x() = i;
                if (j < ulp[l].y())
                    ulp[l].y() = j;
                
                if (i > lrp[l].x())
                    lrp[l].x() = i;
                if (j > lrp[l].y())
                    lrp[l].y() = j;
            }
        }
    }

    // Enclosed rects will be removed
    if ( !gRects.empty() )
      gRects.clear();
    gRectLabel.reserve( nLabels );
    if ( !gRectLabel.empty() )
      gRectLabel.clear();
    gRectLabel.reserve( nLabels );

    // Construct rects
    for (uint32 l = 1; l <= nLabels; ++l )
      {
	/* The integer bounding box is at upper left x and y and has width and height
	 * of lower - upper + 1. 
	 * We use the rectangle ctor for passing the position and width and height
	 */
	rcIRect r(ulp[l].x(), ulp[l].y(), lrp[l].x() - ulp[l].x() + 1, lrp[l].y() - ulp[l].y() + 1);
	
	//	cerr << "r[" << l << "]: " << r.ul() << "," << r.size() << endl;

	// Add only rects which meet minimum size requirement
	if ( r.width() >= minSize.x() && r.height() >= minSize.y() ) {
	  gRects.push_back( r );
	  gRectLabel.push_back( l );
	}
      }

  
    if ( combineRects ) {
      vector<bool> erasedRects( gRects.size(), false );
        uint32 erasedCount = 0;
        
        // Remove regions enclosed by another region
	// Erase rect of true == remove
        for ( uint32 i = 0; i <  gRects.size(); ++i ) {
	  const rcIRect r = gRects[i];

	  if (erasedRects[i]) continue;

	  rmAssert (erasedRects[i] == false);

	  for ( uint32 j = i+1; j < gRects.size(); ++j ) {
	    if ( !erasedRects[j] ) {
	      const rcIRect r2 =  gRects[j];
	      if ( r.contains( r2 ) ) {
		++erasedCount;
		erasedRects[j] = true;
	      } else if ( r2.contains( r ) ) {
		++erasedCount;
		erasedRects[i] = true;
	      }
	    }
	  }
        }

        // Remove smaller regions intersecting with larger ones
        for ( uint32 i = 0; i <  gRects.size(); ++i ) {

	  if (erasedRects[i]) continue;

	  rmAssert (erasedRects[i] == false);

	  const rcIRect r = gRects[i];
	  for ( uint32 j = i+1; j < gRects.size(); ++j ) {
	    if ( !erasedRects[j] ) {
	      const rcIRect r2 =  gRects[j];
	      if ( r.overlaps ( r2 )  &&  r.area() >= r2.area() ) {
		++erasedCount;
		erasedRects[j] = true;
	      } else if (r.overlaps ( r2 )  && r.area() >= r2.area()) {
		++erasedCount;
		erasedRects[i] = true;
	      }
	    }
	  }
        }

        if ( erasedCount ) {
	  if ( erasedCount < gRects.size() ) {
	    vector<rcIRect> combinedRects;
	    vector<uint32> combinedRectLabel;
	    combinedRects.reserve( gRects.size() - erasedCount );
	    combinedRectLabel.reserve( gRects.size() - erasedCount );
                
	    for ( uint32 i = 0; i <  gRects.size(); ++i ) {
	      if ( !erasedRects[i] ) {
		combinedRects.push_back( gRects[i] );
		combinedRectLabel.push_back( gRectLabel[i] );
	      }
	    }
	    gRects.clear();
	    gRectLabel.clear();
		
	    if (combinedRects.size() && combinedRectLabel.size())
	      {
		gRects = combinedRects;
		gRectLabel = combinedRectLabel;
	      }
	  }
	}
    }
}

#if 0
// Get RLEs from label image
void rfGetRLEs (const rcWindow& label, uint32 nLabels, const rcIPair& minSize,
                bool combineRects, vector<rcRleWindow>& gRLEs )
{
    vector<rcIRect> rects;
    vector<uint32> rectLabel;

    // Get bounding rects meeting the minimum size requirement
    // TODO: get rects directly in rfGetComponents
    rfGetRects( label, nLabels, minSize, combineRects, rects, rectLabel );
    const uint32 acceptedRects =  rects.size();
    rmAssert( acceptedRects <= nLabels );

    // Reserve the right amount of space
    if ( !gRLEs.empty() )
        gRLEs.clear();
    gRLEs.reserve( acceptedRects );


    // Now setup each image window and encode the RLE
    for (uint32 l = 0; l < acceptedRects; ++l )
    {
        const rcIRect r = rects[l];
        const rcWindow rw (label, r.ul().x(), r.ul().y(), r.width(), r.height());
        rcRleWindow rle;
        rle.encode (rw, nLabels, rectLabel[l]);
        rle.rectangle( r );
        gRLEs.push_back( rle );
    }
}
#endif


void rfComponentPolygons(const rcWindow& segImg, const int32 minDim, 
			 vector<rcPolygon>& mergedCH, vector<rcIRect>& mergedCHR, bool doMerge)
{
  mergedCH.clear();
  mergedCHR.clear();

  uint32 blobCnt;
  rcWindow blobWin;

  rfGetComponents(segImg, blobCnt, blobWin);

  if (blobCnt == 0)
    return;

  vector<rcIRect> rects;
  vector<uint32> rectLabel;
  const rcIPair minPairSz(minDim, minDim);
  vector<rcPolygon> poly;

  // combineRects ?= false
  rfGetRects(blobWin, blobCnt, minPairSz, false, rects, rectLabel);

  // Get raw Polygons
  for (uint32 i = 0; i < rects.size(); i++)
    {
      const rcIRect r = rects[i];
      const rcWindow rw(blobWin, r.ul().x(), r.ul().y(), r.width(), r.height());
#if 0			
      rcRleWindow rle;
      rle.encode(rw, blobCnt, rectLabel[i]);

      rcPolygon p;
      rle.polygon(p);
      p.translate(rc2Dvector(r.ul().x(), r.ul().y()));
      poly.push_back(p);
#endif						
    }

  // Create ConvexHulls
  vector<rcPolygon> polyCH(poly.size());
  for (uint32 fi = 0; fi < poly.size(); fi++)
    poly[fi].convexHull(polyCH[fi]);

  rmAssert(poly.size() == polyCH.size());

  if (!doMerge)
    {
      mergedCH = polyCH;
      mergedCHR = rects;
      return;
    }

  // Merge (Initialy from MuscleSegmenter)

  // Cache area 
  multimap<double, uint32> areaToIndex;
  for (uint32 si = 0; si < poly.size(); si++) {
    double area = poly[si].area();
    areaToIndex.insert(pair<double, uint32>(area,si));
  }

  vector<uint32> mergedToPoly(poly.size());

  /* 
   * Iterative merging: if a more than X percent of a polygon's intersects with another, merge them
   */

  for (multimap<double, uint32>::reverse_iterator si = areaToIndex.rbegin();
       si != areaToIndex.rend(); si++)
    {
      rcPolygon& pCH = polyCH[si->second];
      rcIRect boxCH = pCH.orthogonalEnclosingRect();
      double sAreaCH = -1.0;
      uint32 mi;
      for (mi = 0; mi < mergedCH.size(); mi++) {
	if (mergedCHR[mi].overlaps(boxCH, true)) {
	  if (sAreaCH < 0.0)
	    sAreaCH = pCH.area()*0.5;
	  uint32 mpi = mergedToPoly[mi];
	  rcPolygon intPoly;
	  polyCH[mpi].convexIntersection(pCH, intPoly);
	  if (intPoly.area() > sAreaCH) {
	    if (intPoly != pCH) {
	      rcPolygon unionPoly;
	      mergedCH[mi].convexUnion(pCH, unionPoly);
	      mergedCH[mi] = unionPoly;
	      mergedCHR[mi] = mergedCH[mi].orthogonalEnclosingRect();
	    }
	    break;
	  }
	}
      }

      if (mi == mergedCH.size()) {
	mergedCH.push_back(pCH);
	mergedCHR.push_back(boxCH);
	mergedToPoly[mergedCH.size()-1] = si->second;
      }
    }

  

}

