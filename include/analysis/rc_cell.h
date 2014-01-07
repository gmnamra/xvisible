/*
 *  
 *  
 *
 *	$Id: rc_cell.h 7288 2011-03-04 22:08:39Z arman $
 *
 *  
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */


#ifndef __RC_CELL_H
#define __RC_CELL_H

// This file contains an increasing cell assay based result structures
#include <rc_types.h>
#include <rc_imageprocessing.h>

enum RC_CellPlateImaging {
  rcCellImagingUnknown = 0,
  rcHighGreyValueCells = 1,       // Cells have higher intensity values than the background
  rcLowGreyValueCells = 2,         // Cells have lower  intensity values than the background
  rcPhaseImagingFunction = 3        // Phase contrast: Cells have low/high value presentation
};

class RC_IMEX rcCell
{
 public:
  rcCell () : mIsValid (0) {}

  rcCell (uint32 id, rc2Dvector com, uint32 area, RC_CellPlateImaging ic = rcCellImagingUnknown) : 
    mId (id), mCom (com), mPelCount (area), mImaging (ic), mIsValid (1) {}

  // Default copy/assignment/destructor ok

  
  uint32 isValid () {return mIsValid;}

  uint32 id() const {return mId;}

  rc2Dvector com () const {return mCom;}  

  rc2Dvector ctr () const {return mBoxCtr;}  

  uint32 rawArea () const {return mPelCount; }

  void enclosingBox (rcRect& rect)
  { 
    mBox = rect; 
    mBoxCtr.x(((double) (rect.x() + rect.width()/2)));
    mBoxCtr.y(((double) (rect.y() + rect.height()/2)));
  }

  rcRect enclosingBox () { return mBox; }

  // required for STL
  bool operator<(const rcCell& rhs) const { return mId < rhs.mId; };
  bool operator==(const rcCell& rhs) const { return mId == rhs.mId; };

 private:
  uint32 mId;
  rcRect mBox;
  rc2Dvector mBoxCtr;
  rc2Dvector mCom;
  uint32   mPelCount;
  RC_CellPlateImaging  mImaging;
  uint32 mIsValid;
};

typedef vector<rcCell> rcVectOfCells;



class rcCellCountParams
{
 public:
  rcCellCountParams () : mSmoothThr (4), mSmoothKernel (5), mSoftness (8) {}

  // default copy, assignment, dtor ok

  uint32 smoothThr () { return mSmoothThr; }
  uint32 smoothThr (uint32 n) { return mSmoothThr = n; }  

  uint32 smoothkernel () { return mSmoothKernel; }
  uint32 smoothkernel (uint32 n) { return mSmoothKernel = n; }  

  uint32 softness () { return mSoftness; }
  uint32 softness (uint32 n) { return mSoftness = n; }  
 private:
  uint32 mSmoothThr, mSmoothKernel, mSoftness;
};


void rfCellCount (rcWindow& image, rcCellCountParams& rcp, vector<rcCell>& cellcounts);



// Return cell count for every image in the count vector
// Processing Function

template <class Iterator>
void rfCellDensity (Iterator begin, Iterator end, rcCellCountParams& ccp, vector<rcVectOfCells>& res)
{
  rmAssert (end > begin);

  int32 size = end - begin;

  vector<rcVectOfCells> ires (size);

  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  Iterator img = begin;
  vector<rcVectOfCells>::iterator plate = ires.begin();

  for (; img != end; img++, plate++)
    {
      value_type image( *img );

      // Convert to 8bit if needed
      rcWindow local;

      // Convert to 8 bit because 
      if (image.depth() > RC_PixelDepth8 )
	{
	  fprintf (stderr, "Converting to Depth8 for analysis");
	  // Reduce all color and gray 24/32-bit images to 8-bit gray images
	  rfRcWindow32to8(image, local); 
	}
      else
	local = image;

      rfCellCount (*img, ccp, *plate);

    }

  res = ires;
}

#endif /* __RC_CELL_H */

