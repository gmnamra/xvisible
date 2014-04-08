/******************************************************************************
 * @file 
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_level.h 6239 2009-01-15 06:56:26Z arman $
 *
 *****************************************************************************/

#ifndef rrclevel_h_h
#define rc_level_h_h

#include <rc_window.h>

class rcLevelSets
{
public:
  // default ctor, dtor ok?

  // getters
  uint32 threshold() const {return mThreshold;}
  double score() const {return mScore;}
  double darkArea() const {return darkArea_;}

  // setters
  void update(uint32 threshold, double score, uint32 darkArea)
    {mThreshold = threshold, mScore = score, darkArea_ = darkArea;}
private:
  uint32 mThreshold;
  double mScore;
  double darkArea_;
};


class rcSegmentationLevel
{
public:

  virtual ~rcSegmentationLevel() {}
  // effect    Dtor for virtual base class.

  // utilities
  rcWindow binaryImage(const rcWindow& src, uint8 toGrayValue); 
  rcWindow binaryImage(const rcWindow& src, uint8 toGrayValue, uint8 setVal);
  // effect    Creates a binarized image of the source image, 
  //           using computed threshold. The pixels whose gray value is equal
  //           to or greater than the threshold are mapped to toGrayValue.
  // throws    ImageUnbound
  //           UniformIntensity

  // getters

  virtual rcLevelSets& results() {return mResults;}
  // effect    Returns results.

private:
  rcLevelSets mResults;
};

// Conventions: 
//   leftTail and rightTail specifies the range of histogram index to be used. 
//   leftTail specifies the lower limit, and rightTail specifies the upper 
//   limit. Both are inclusive.
//
class rcTails
{
public:
  rcTails(const vector<uint32>& histogram, 
	  uint32 leftCutoffCount,
	  uint32 rightCutoffCount); 
  // effect    Computes the members.

  // getters
  vector<uint32>::const_iterator leftTail() const 
  {return mLeftTail;}
  // effect    The lower limit of the interval. Note this is inclusive.

  vector<uint32>::const_iterator rightTail() const 
    {return mRightTail;}
  // effect    The upper limit of the interval. Note this is inclusive.

  uint32 offset() const {return mOffset;}
  // effect    Returns the index of the left tail.

  uint32 cumulativeOffset() const {return mCumulativeOffset;}

private:
   vector<uint32>::const_iterator mLeftTail;
   vector<uint32>::const_iterator mRightTail;
  uint32 mOffset;
  uint32 mCumulativeOffset;
};

class rcHornMoments
{
public:
  rcHornMoments(vector<uint32>::const_iterator leftTail,
		vector<uint32>::const_iterator rightTail)
    {compute(leftTail, rightTail);}
  // effect    Computes zeroth, first and second moment.
  //           leftTail and rightTail specifies a range of indices to
  //           histogram to be used for the computation. Both limits are 
  //           inclusive. 

  void compute(vector<uint32>::const_iterator leftTail,
	       vector<uint32>::const_iterator rightTail);
  // effect    

  double zeroth() const {return mZeroth;}
  double first()  const {return mFirst;}
  double second() const {return mSecond;}

private:
  double mZeroth;
  double mFirst;
  double mSecond;
};



class rcWithinGroupInv : public rcSegmentationLevel
{
public:
  rcWithinGroupInv( vector<uint32>::const_iterator leftTail,
                  vector<uint32>::const_iterator rightTail,
		 uint32 offset = 0,
                 uint32 cumulativeOffset = 0);
  // effect    Ctor to compute the Within Group Variance threshold 
  //           only using the partial histogram between leftTail and 
  //           rightTail. This is useful for noisy images, saturated 
  //           images or multi-modal histogram.
  //           offset is the index to the original histogram such that 
  //           leftTail == histogram.begin() + offset.
  //           cumulativeOffset specifies the cumulative below the 
  //           leftTail. This argument can be omitted when the darkArea
  //           member in rcLevelSets is not necessary.

  rcWithinGroupInv(const  vector<uint32>& histogram, 
		 uint32 leftCutoffPels = 0,
		 uint32 rightCutoffPels = 0);
  // effect    Constructs a thresholding object based on the specified 
  //           histogram. Left tail and right tail are computed from the 
  //           specified cutoff pel counts.

  rcWithinGroupInv(const rcWindow& image, 
		 uint32 leftCutoffPels = 0,
		 uint32 rightCutoffPels = 0);


private:
  void compute( vector<uint32>::const_iterator leftTail,
	        vector<uint32>::const_iterator rightTail,
	       uint32 offset,
	       uint32 cumulativeOffset,
	       rcLevelSets& result);
  // effect   Computes a within group variance threshold.
  // throws   tails out of range

  bool mIsValid;
};


class rcMutualInfoGrouping : public rcSegmentationLevel
{
public:

  rcMutualInfoGrouping(const rcWindow& image);

private:
  bool mIsValid;
};


#endif /* rc_level_h_h */


