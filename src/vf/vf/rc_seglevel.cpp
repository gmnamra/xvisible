/* @file
 *
 *$Id $
 *$Log$
 *Revision 1.8  2005/11/29 20:13:38  arman
 *incremental
 *
 *Revision 1.7  2005/11/04 22:00:51  arman
 *removed unnecessary statements
 *
 *Revision 1.6  2005/11/04 00:28:58  arman
 *first prototype version of mutualInformationGroup
 *
 *Revision 1.5  2005/11/03 16:31:47  arman
 *added debugging info for mutualInfo approach
 *
 *Revision 1.4  2005/11/02 16:07:31  arman
 *fixed a bug in mutual info thresholding
 *
 *Revision 1.3  2005/11/01 22:59:41  arman
 *mutualInfo level setting
 *
 *Revision 1.2  2005/10/31 22:47:51  arman
 *added level segmentation support
 *
 *Revision 1.1  2005/10/31 17:07:59  arman
 *basic level setting support
 *
 *Revision 1.57  2005/08/30 20:57:16  arman
 *Cell Lineage
 */


#include "rc_histstats.h"
#include "rc_ip.h"
#include "rc_similarity.h"
#include "rc_level.h"
#include "rc_filter1d.h"
#include "rc_stats.h"
#include <map>

rcTails::rcTails(const  vector<uint32>& histogram,
		   uint32 leftCutoff,
		   uint32 rightCutoff) 
  : mOffset(0), mCumulativeOffset(0) 
{
  uint32 count = 0;

  // go up from the left end
  if(leftCutoff == 0) {
    mLeftTail = histogram.begin();
  }
  else {
     vector<uint32>::const_iterator p = histogram.begin();
    for(count = 0; (count += *p) < leftCutoff; p++);
    mLeftTail = ++p;
    mCumulativeOffset = count;
  }

  // down from the right end
  if(rightCutoff == 0) {
    mRightTail = histogram.end() - 1;
  }
  else {
     vector<uint32>::const_iterator p = histogram.end() - 1;
    for(count = 0; (count += *p) < rightCutoff; p--);
    mRightTail = --p;
  }

  rmAssert (distance (histogram.begin(), mLeftTail) < distance (histogram.begin(), mRightTail));
}

void 
rcHornMoments::compute( vector<uint32>::const_iterator leftTail,
		     vector<uint32>::const_iterator rightTail)
{
  // we shift the interval [left tail, right tail] to [0, right - left]
  double m0 = 0.0, m1 = 0.0, m2 = 0.0;

  // Horn Accumulator
  // the left tail is intentionally excluded
  for( vector<uint32>::const_iterator p = rightTail; 
      p > leftTail; 
      p--){
    m0 += *p;
    m1 += m0;
    m2 += m1;
  }
  
  // since we assume left tail == 0, we do not have to add i * hist[i] to m1
  mZeroth = m0 + *leftTail;     // number of sample
  mFirst = m1;                  // mean * number of sample
  mSecond = m2 * 2 - m1;        // var = second_/zeroth_ - (first_/zeroth_)^2
}

rcWithinGroupInv::
rcWithinGroupInv( vector<uint32>::const_iterator leftTail,
	        vector<uint32>::const_iterator rightTail,
	       uint32 offset,
		  uint32 cumulativeOffset) : mIsValid (false)
{
  compute(leftTail, rightTail, offset, cumulativeOffset, results());
  mIsValid = true;
}

rcWithinGroupInv::rcWithinGroupInv(const  vector<uint32>& histogram,
			       uint32 leftCutoffPels,
			       uint32 rightCutoffPels)  : mIsValid (false)
{
  rcTails tails(histogram, leftCutoffPels, rightCutoffPels);
  compute(tails.leftTail(), tails.rightTail(), tails.offset(),
	  tails.cumulativeOffset(), results());  
  mIsValid = true;
}

rcWithinGroupInv::rcWithinGroupInv(const rcWindow& image,
			       uint32 leftCutoffPels,
			       uint32 rightCutoffPels)  : mIsValid (false)
{
  rcHistoStats h (image);
  rcTails tails(h.histogram (), leftCutoffPels, rightCutoffPels);
  compute(tails.leftTail(), tails.rightTail(), tails.offset(),
	  tails.cumulativeOffset(), results()); 
  mIsValid = true;   
}


void 
rcWithinGroupInv::compute( vector<uint32>::const_iterator leftTail, 
			 vector<uint32>::const_iterator rightTail,
			uint32 offset, uint32 cumulativeOffset,
			rcLevelSets& results)
{
  //  Between Group Variance (BGV) is defined as:
  //  BGV = (m0_l/m0) * (m0_r/m0) * (m1_l/m0_l - m1_r/m0_r)^2
  // oops, the above variable names are not quite descriptive
  rcHornMoments moment(leftTail, rightTail);

  double zerothMomentLeft = 0.0;
  double zerothMomentRight = 0.0;
  double firstMomentLeft = 0.0;
  double firstMomentRight = 0.0;

  double max = 0.0;
  double cumulative = 0.0;
  uint32 threshold = 0;
  uint32 i = 0;  // index to the (partial) histogram
   vector<uint32>::const_iterator p = leftTail;
  for(; p < rightTail; p++, i++) {
    if(*p == 0) continue;
    zerothMomentLeft += *p;
    zerothMomentRight = moment.zeroth() - zerothMomentLeft;
    firstMomentLeft += *p * i;
    firstMomentRight = moment.first() - firstMomentLeft;
    double dm = firstMomentRight * zerothMomentLeft - 
                firstMomentLeft * zerothMomentRight;
    double score = dm * dm / (zerothMomentLeft * zerothMomentRight);

    if(score > max) {
      threshold = i;
      cumulative = zerothMomentLeft;
      max = score;
    }
  }

  double denom = moment.second()*moment.zeroth()-moment.first()*moment.first();
  results.update(threshold + offset + 1, max/denom, (int32) cumulative); 
}


rcWindow rcSegmentationLevel::binaryImage (const rcWindow& src, uint8 level)
{
  return binaryImage (src, level, level);
}

rcWindow rcSegmentationLevel::binaryImage (const rcWindow& src, 
					   uint8 level, uint8 setVal)
{
  rmAssert (level != 0 && level != 255);
  vector<uint8> lut (256);

  memset (&lut[0], 0, level-1);
  memset (&lut[level], setVal, 256 - level);
  rcWindow dst (src.width(), src.height());
  rfPixel8Map (src, dst, lut);
  return dst;
}




///////////////
///// Segmentation threshold selection by maximization of mutual information among potential segmentations
///////////////



rcMutualInfoGrouping::rcMutualInfoGrouping(const rcWindow& image)  : mIsValid (false)
{
 rcHistoStats h (image);
 vector<rcWindow> slices;
 deque<double> entf;
 
 // Default sorting criteria is less<double>. 
 multimap<double, uint32, less<double> > aciToIndex;

 for (uint32 i =1; i < h.histogram().size() - 1; i++)
   {
     if (!h.histogram()[i]) continue;
     slices.push_back (binaryImage (image, i));
   }

 // @todo correlation of binary templates can be substantially sped up (not a binary packing scheme)
	rcSimilaratorRef sim = boost::shared_ptr<rcSimilarator> (new rcSimilarator (rcSimilarator::eExhaustive,
																																							rcPixel8, slices.size(), 0) );
 sim->fill (slices);
 sim->entropies(entf);
 deque<double>::iterator mi = entf.begin(); 

 for (uint32 i = 1; i < h.histogram().size() - 1; i++)
   {
     if (!h.histogram()[i]) continue;
     double aci = *mi++;
     aciToIndex.insert(pair<double, uint32>(aci,i));
   }


 // Compute the median value
 // Median does not actually compute index in the original list.
 double med = rfMedian (entf);
 multimap<double, uint32>::iterator si = aciToIndex.lower_bound (med);
 results().update (si->second, 1.0, 0);

#if 0
 cerr << "Median: " << med << "Level: " << results().threshold () << endl;
 cerr.setf (ios::fixed);
 cerr << setprecision (7);
 cerr << "{";
 si = aciToIndex.begin();
 for (; si != aciToIndex.end(); si++)
   {
     cerr << "{" << si->second <<  "," << si->first << "},";
   }
 cerr << "}";
#endif

}


/*
 * Can be solved with carring the normalization:
 *
 * (a + b - sqrt ((a - b)^2 + 4c^2)) / 2x
 * a,b, and c are gxx, gyy, and gxy uint8 sums
 * x is n * sum (255*255) where n is number of pixels
 * TBD: AltiVec Speedup 
 */

float rfSurfaceHessian (rcWindow& src)
{
    float gxx(0.0), gyy(0.0), gxy(0.0);
    int32 rup (src.rowUpdate());
    float nn (src.pixelCount() * 2);
    
    for (int32 j = 1; j < src.height() - 1; j++)
    {
        uint8 * g = src.rowPointer (j);
        
        for (int32 i = 1; i < src.width() - 1; i++, g++) 
        {
            float gp (g[0] );
            float gx (gp - g[1]);
            float gy (gp - g[rup]);
            gxx += (rmSquare (gx));
            gyy += (rmSquare (gy));
            gxy += (gx * gy);
        }
    }
    
    // Return the min eigenvalue
    return (gxx + gyy - sqrt((gxx - gyy)*(gxx - gyy) + 4*gxy*gxy))/ nn;  
}


struct rsHystNode
{
    uint8 *destAddr;
    uint8 *workAddr;
};


void rfHysteresisThreshold(const rcWindow& magImage,
                           rcWindow& dst,
                           uint8 magLowThreshold, uint8 magHighThreshold,
                           int32& nSurvivors, int32 nPels, 
                           uint8 outVal,uint8 inVal)
{
    const uint8 noEdgeLabel = outVal;
    const uint8 edgeLabel = inVal;
    uint8 oscEdgeLabel;
    
    if (noEdgeLabel != 0 && edgeLabel != 0) oscEdgeLabel = 0;
    else if (noEdgeLabel != 1 && edgeLabel != 1) oscEdgeLabel = 1;
    else oscEdgeLabel = 2;
    
    if (!dst.isBound())
    {
        rcWindow dstRoot = rcWindow (magImage.width()+2, magImage.height()+2);
        dstRoot.setAllPixels (noEdgeLabel);
        dst = rcWindow(dstRoot, 1, 1, magImage.width(), magImage.height());
    }
    else
    {
        dst.setAllPixels (noEdgeLabel);
        rmAssert (dst.width() == magImage.width() && 
                  dst.height() ==magImage.height());
    }
    
    const int32 width(dst.width()), height(dst.height());
    
    rcWindow work(width + 2,height + 2);
    rfSetWindowBorder (work, uint8(noEdgeLabel));
    rcWindow win(work, 1, 1, width, height);
    
    // Create the identity map for this set of thresholds
    vector<uint8> map(256);
    int32 i(0);
    for (; i < magLowThreshold; i++) map[i] = noEdgeLabel;
    for (i = 0; i < (magHighThreshold - magLowThreshold); i++) 
        map[magLowThreshold + i] = oscEdgeLabel;
    for (i = 0; i < (256 - magHighThreshold); i++)
        map[magHighThreshold + i] = edgeLabel;
    
    rfPixel8Map(magImage, win, map);
    
    const int32 maxPeaks(nPels >= 0 ? nPels : width*height);
    vector<rsHystNode> stack(maxPeaks);
    
    rmAssert(win.size() == dst.size());
    
    int32 nPeaks(0);
    i = 0;
    const int32 workUpdate(win.rowUpdate()), dstUpdate(dst.rowUpdate());
    
    // Note:
    // -- and ++ are push and pop operations for the stack
    for(int32 y = 0; y < height; y++)
    {
        uint8 *p    = win.rowPointer(y);
        uint8 *dest = dst.rowPointer(y);
        
        for(int32 x = width; x--; p++,dest++)
        {
            if(*p == edgeLabel)
            {
                stack[i].workAddr = p;
                stack[(i++)].destAddr = dest;
                
                while(i)
                {
                    uint8 *pp = stack[(--i)].workAddr;
                    uint8 *dd = stack[i].destAddr;
                    
                    // If it is not marked, mark it an edge in dest
                    // and mark it a no edge in mag
                    if(!*dd)
                    {
                        *dd = edgeLabel;
                        nPeaks++;
                    }
                    *pp = noEdgeLabel;
                    
                    // If any of our neighbors is a possible edge push their address in
                    
                    if(*(pp - workUpdate - 1) == oscEdgeLabel)
                    {
                        stack[i].workAddr = pp - workUpdate - 1;
                        stack[(i++)].destAddr = dd - dstUpdate - 1;
                    }
                    if(*(pp - workUpdate) == oscEdgeLabel)
                    {
                        stack[i].workAddr = pp - workUpdate;
                        stack[(i++)].destAddr = dd - dstUpdate;
                    }
                    if(*(pp - workUpdate + 1) == oscEdgeLabel)
                    {
                        stack[i].workAddr = pp - workUpdate + 1;
                        stack[(i++)].destAddr = dd - dstUpdate + 1;
                    }
                    if(*(pp + 1) == oscEdgeLabel)
                    {
                        stack[i].workAddr = pp + 1;
                        stack[(i++)].destAddr = dd + 1;
                    }
                    if(*(pp + workUpdate + 1) == oscEdgeLabel)
                    {
                        stack[i].workAddr = pp + workUpdate + 1;
                        stack[(i++)].destAddr = dd + dstUpdate + 1;
                    }
                    if(*(pp + workUpdate) == oscEdgeLabel)
                    {
                        stack[i].workAddr = pp + workUpdate;
                        stack[(i++)].destAddr = dd + dstUpdate;
                    }
                    if(*(pp + workUpdate - 1) == oscEdgeLabel)
                    {
                        stack[i].workAddr = pp + workUpdate - 1;
                        stack[(i++)].destAddr = dd + dstUpdate - 1;
                    }
                    if(*(pp - 1) == oscEdgeLabel)
                    {
                        stack[i].workAddr = pp - 1;
                        stack[(i++)].destAddr = dd - 1;
                    }
                }
            }
        }
    }
    
    nSurvivors = nPeaks;
    return;
}


