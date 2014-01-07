/*
 *
 *$Id $
 *$Log: $
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_window.h>
#include <rc_moments.h>

// An example image
// [  0]  1  1  1  1  1  2           s25,ss25 s26, ss28
// [  1]  1  1  1  1  1  1           s26,ss28 s25, ss25
// [  2]  1  1  1  1  1  1           s26,ss27 s25, ss25
// [  3]  1  1  1  1  1  1
// [  4]  1  1  1  1  1  1
// [  5]  2  1  1  1  1  1
// [  6]  1  1  1  1  1  1

  // The squares are arranged progressively in X
  //        S              E
  //[  0]  1  2  3  4  5  9
  //[  1]  1  2  3  4  5  6
  //[  2]  1  2  3  4  5  6
  //[  3]  1  2  3  4  5  6
  //[  4]  1  2  3  4  5  6
  //[  5]  4  5  6  7  8  9
  //[  6]  1  2  3  4  5  6


  // The sums are arrange progressively in Y
  // [  0]  1  1  1  1  1  2  S
  // [  1]  2  2  2  2  2  3
  // [  2]  3  3  3  3  3  4
  // [  3]  4  4  4  4  4  5
  // [  4]  5  5  5  5  5  6  E
  // [  5]  7  6  6  6  6  7
  // [  6]  8  7  7  7  7  8



template<class P, class A>
void rfImageIntegrals (const P* pixelTypePtr, const A* accTypePtr, const rcWindow& image, rcWindow& sumColInt, rcWindow& sumRowInt)
{
  rmUnused (pixelTypePtr);
  rmUnused (accTypePtr);
  rmAssert (image.isBound());
  rmAssert (sumColInt.isBound());
  rmAssert (sumRowInt.isBound());
  rmAssert (image.size() == sumColInt.size());
  rmAssert (image.size() == sumRowInt.size());

  sumRowInt.set (0.0);
  sumColInt.set (0.0);

  // Fillup the first row of col integrators, and the first col of row integrators
  const P *p16 = (P *) image.pelPointer (0, 0);
  A *r32 = (A *) sumColInt.pelPointer (0, 0);
  for (int32 i = 0; i < image.width(); i++, p16++, r32++)
    {
      *r32 = *p16;
    }

  A *c32 = (A *) sumRowInt.rowPointer (0);
  uint32 crup = sumRowInt.rowPixelUpdate();
  for (int32 j = 0; j < image.height(); j++, c32+=crup)
    {
      A pel = A ( *((P *) image.rowPointer (j) ) );
      *c32 = pel * pel;
    }


  // Fillup the first column of the column integrators
  A acc = sumColInt.getPixel (0,0);
  r32 = (A *) sumColInt.rowPointer (1);
  p16 = (P *) image.rowPointer (1);
  uint32 rrup = sumColInt.rowPixelUpdate();
  uint32 irup = image.rowPixelUpdate();
  for (int32 j = 1; j < image.height(); j++, r32+=rrup, p16+=irup)
    {
      acc += *p16;
      *r32 = acc;
    }

  // Fillup the first row of the row integrators
  acc = sumRowInt.getPixel (0,0);
  c32 = (A *) sumRowInt.pelPointer (1, 0);
  p16 = (P *) image.pelPointer (1, 0);
  for (int32 i = 1; i < image.width(); i++, p16++, c32++)
    {
      A pel = A (*p16);
      acc += pel * pel;
      *c32 = acc;
    }


  // Now process from 1,1 onward. 

  
  for (int32 j = 1; j < image.height(); j++)
    {
      P *p16 = (P *) image.pelPointer (1, j);

      // current and previous rows for col integrators
      A *rprev32 = (A *) sumColInt.pelPointer (1, j-1);
      A *r32 = (A *) sumColInt.pelPointer (1, j);

      // current and previous integration pixel for row integrators
      A *cprev32 = (A *) sumRowInt.rowPointer (j);
      A *c32 = cprev32+1;

      // As we travel through pixels we are also traveling thorugh
      // equal size integrators. Update all the column integrators
      // and integration pixel of this row's integrator
      
	for (int32 i = 1; i < image.width(); i++, p16++, r32++, c32++, rprev32++)
	{
	  A pel = A (*p16);

	  // integrate row j of col integrators
	  *r32 = *rprev32 + pel;

	  // integrate col i of row integartors
	  *c32 = *cprev32 + pel * pel;

	  // Roll over
	  cprev32 = c32;
	}
    }
}

void rfImageIntegrals (const rcWindow& image, rcWindow& rproj, rcWindow& cproj)
{
  switch (image.depth())
    {
    case rcPixel8:
      return rfImageIntegrals<uint8, uint32> (image.rowPointer (0), (const uint32 *) rproj.rowPointer(0), image, rproj, cproj);
    case rcPixel16:
      return rfImageIntegrals<uint16, uint32> ((const uint16 *) image.rowPointer (0), (const uint32 *) rproj.rowPointer(0), image, rproj, cproj);
    default:
      rmAssert (0);
    }
}

rcIRect rfImageACfieldArea (const rcWindow& image, const rcIPair& target)
{
  rcIPair fieldSize = image.size() - target + rcIPair (1, 1);
  rcIPair origin = target / rcIPair (2, 2);
  return rcIRect (origin, origin+fieldSize);
}

float rfImageACfield (const rcWindow& image, const rcIPair& target, rcWindow& floatDest)
{
  rcWindow L (image.width(), image.height(), rcPixel32);
  rcWindow L2 (image.width(), image.height(), rcPixel32);
  rcWindow V2; rcWindow& dest = floatDest;
  rcIRect field = rfImageACfieldArea (image, target);

  // @note Window into the passed in destination if it is bound and of the right depth and size
  // else create one
  if (dest.isBound() && dest.size() == image.size () && dest.frameBuf()->isD32Float ())
    {
      V2 = rcWindow (dest, target.x() / 2, target.y() / 2, field.width(), field.height());
    }
  else
    {
      dest = rcWindow (image.width(), image.height(), rcPixel32);
      dest.frameBuf()->markD32Float (true);
      V2 = rcWindow (dest, field.ul().x(), field.ul().y(), field.width (), field.height());
    }

  uint32 lrup = L.rowUpdate() / L.depth();
  uint32 l2rup = L2.rowUpdate() / L2.depth();
  int64 maxvar (0);

  rfImageIntegrals (image, L, L2);
  int64 n = target.x() * target.y();
  int32 targetHeightbyRUP = (target.y()-1) * lrup;
  rcIPair steps = L2.size() / target;

  for (uint32 j = 0; j < V2.height();  j++)
    {
      uint32 *leftSqRowPtr = (uint32 *) L2.rowPointer (j);
      uint32 *topSumRowPtr = (uint32 *) L.rowPointer (j);
      float *varFloatPtr =  (float *) V2.rowPointer (j);

      // Row Processing
      int32 cols(0);
      for (cols = 0; cols < V2.width(); cols++, leftSqRowPtr++, topSumRowPtr++, varFloatPtr++)
	{
	  uint32 *leftSumSq = leftSqRowPtr, *rightSumSq = leftSumSq + target.x() - 1;
	  uint32 *topSum = topSumRowPtr, *botSum = topSum + targetHeightbyRUP;
	  double d (0.0), dd (0.0);

	  // The squares are arranged progressively in X
	  //        S              E
	  //[  0]  1  2  3  4  5  9
	  //[  1]  1  2  3  4  5  6
	  //[  2]  1  2  3  4  5  6
	  //[  3]  1  2  3  4  5  6
	  //[  4]  1  2  3  4  5  6
	  //[  5]  4  5  6  7  8  9
	  //[  6]  1  2  3  4  5  6

	  // If we are at the top left bin use the cached val (since we are writing results out to the same bin)
	  // If we are at other bins but at the first position in the row just subtract the bin
	  for (uint32 cc = 0; cc < target.y(); cc++, leftSumSq+= l2rup, rightSumSq+=l2rup)
	    {
	      dd += (*rightSumSq);
	      dd -= cols ? leftSumSq[-1] : 0;
	    }

	  // The sums are arrange progressively in Y
	  // [  0]  1  1  1  1  1  2  S
	  // [  1]  2  2  2  2  2  3
	  // [  2]  3  3  3  3  3  4
	  // [  3]  4  4  4  4  4  5
	  // [  4]  5  5  5  5  5  6  E
	  // [  5]  7  6  6  6  6  7
	  // [  6]  8  7  7  7  7  8

	  for (uint32 bb = 0; bb < target.y(); bb++, topSum ++, botSum ++)
	    {
	      d +=  (*botSum);
	      d -= j ? topSum[-lrup] : 0;
	    }

	  // Compute n SumOfSq - Sum^2
	  dd = dd * n - d * d;
	  *varFloatPtr = (float) dd;
	  if (dd > maxvar) maxvar = dd;
	}
    }
  
  // Window out the results
  floatDest = dest;
  return (float) maxvar;
}

///////////////
// Basic Projections
////////////////


template<class P>  
void rfImageOrthogonalProjections (const P* pixelTypePtr, const rcWindow& image, rcWindow& rproj, rcWindow& cproj)
{
  rmUnused (pixelTypePtr);
  rmAssert (image.isBound());
  rmAssert (rproj.isBound());
  rmAssert (cproj.isBound());
  rmAssert (image.height() == rproj.width ());
  rmAssert (image.width() == cproj.width ());
  rmAssert (rproj.height () == 1);
  rmAssert (cproj.height () == 1);
  rmAssert (cproj.depth() == rcPixel32);
  rmAssert (rproj.depth() == rcPixel32);   

  cproj.set (0.0);
  rproj.set (0.0);

  // Fillup the first row of col integrators, and the first col of row integrators
  uint32 *r32 = (uint32 *) rproj.pelPointer (0, 0);  // Row Sums 
  for (int32 j = 0; j < image.height(); j++)
    {
      const P *pp = (const P*) image.pelPointer (0, j);

      uint32 *c32 = (uint32 *) cproj.pelPointer (0, 0);
      uint32 rsum (0);
      for (int32 i = 0; i < image.width(); i++, pp++, c32++)
	{
	  P pel (*pp);
	  rsum += pel;
	  *c32 += pel;
	}
      r32[j] = rsum;
    }
}


void rfImageOrthogonalProjections (const rcWindow& image, rcWindow& rproj, rcWindow& cproj)
{
  switch (image.depth())
    {
    case rcPixel8:
      return rfImageOrthogonalProjections<uint8> (image.rowPointer (0), image, rproj, cproj);
    case rcPixel16:
      return rfImageOrthogonalProjections<uint16> ((const uint16 *) image.rowPointer (0), image, rproj, cproj);
    default:
      rmAssert (0);
    }
}

 

