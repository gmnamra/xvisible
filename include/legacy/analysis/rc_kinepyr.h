/* @file
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.11  2005/08/30 21:08:50  arman
 *Cell Lineage
 *
 *Revision 1.11  2005/08/15 12:53:55  arman
 *Cell Lineage II
 *
 *Revision 1.10  2004/12/19 18:20:41  arman
 *added finer processing of gradient processing
 *
 *Revision 1.9  2004/12/15 19:28:20  arman
 *added doxygen file flag
 *
 *Revision 1.8  2004/12/15 11:40:09  arman
 *added imageRect accessor to get image size
 *
 *Revision 1.7  2004/08/24 21:36:44  arman
 **** empty log message ***
 *
 *Revision 1.6  2004/08/24 16:10:50  arman
 *added more robust testing of unbound. Also added hysteresis accessor
 *
 *Revision 1.5  2004/08/19 19:23:26  arman
 *added comments
 *
 *Revision 1.4  2004/07/13 21:07:16  arman
 *added base
 *
 *Revision 1.3  2004/07/12 19:42:38  arman
 *first working version
 *
 *Revision 1.2  2004/07/12 05:53:50  arman
 *added accessors
 *
 *Revision 1.1  2004/07/12 02:15:46  arman
 *Pyramid processing class implementation
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_KINEPYR_H
#define __RC_KINEPYR_H
#include <rc_ip.h>
#include <rc_edge.h>
#include <rc_xforms.h>

/*!
    @class rc2LevelSpatialPyramid
    @abstract 2 Level Gaussian Pyramid w/ Edge detection
    @discussion 
    @throws exception when not initialized
    @throws bar_exception
    @namespace 
    @updated 2003-03-15
*/

template <int32 D> class rcSpatialPyramid;

typedef rcSpatialPyramid<2> rc2LevelSpatialPyramid;

#define EDGELabel 127

template<>
class rcSpatialPyramid<2> : public rc2Xform
{

 public:
  rcSpatialPyramid ();
 
  rcSpatialPyramid (rcWindow& image, int32 sample, 
		    int32 baseSmooth = 3, float edgeDetect = 1.0f,
		    float hyst = 0.0f, bool zeroCross = false, bool highAccDir = false);

  rcIRect imageRect (int32 level) const;
  const rcWindow& base (int32 level) const;
  const rcWindow& past (int32 level) const;
  const rcWindow& gradient (int32 level) const;
  const rcWindow& edge (int32 level) const;
  const rcWindow& direction (int32 level) const;
  const vector<uint32>& directionHistogram (int32 level) const;

  const float hysteresis () const
  {
    return mHysteresis;
  }

  const int32 granularity () const
  {
    return mGranularity;
  }

  rcIPair kernelBorder () const
  {
    return (rcIPair (granularity()/2,granularity()/2) + 
	    rcIPair (5/2,5/2));
  }

  bool isValid () const;

  rcIRect region () const
  {
    rcIPair kb = kernelBorder ();
    return rcIRect (kb.x(), kb.x(), 
		    base(0).width() - 2*kb.x(), 
		    base(0).height() - 2*kb.y());
  }    

  uint8 edgeLabel () { return EDGELabel; }

  int32 gradientCount (int32 level) const { return mEdgeCount[level]; };

  int32 optimizedGradientCount (int32 level) const { return mLeft[level]; };

 private:
  void intensityProcess (rcWindow& image, int32 level);
  void gradientProcess ();
  int32 mSample;
  int32 mGranularity;
  float mEdgeDetect;
  float mHysteresis;
  vector <vector<rcWindow> > mLevels;
  vector <vector<uint32> > mLevelDirectionHists;
  vector<int32> mEdgeCount;
  vector<int32> mLeft;
  rcWindow mUnbound;
  bool mZc;
  bool m16bitDir;
};

#endif /* __RC_KINEPYR_H */
