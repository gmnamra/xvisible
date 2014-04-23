/* @files
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.7  2005/01/25 09:40:10  arman
 *added two window output
 *
 *Revision 1.6  2005/01/21 18:28:17  arman
 *fixed bug in tiling
 *
 *Revision 1.5  2005/01/19 22:53:29  arman
 *added wholeSim
 *
 *Revision 1.4  2005/01/17 13:07:27  arman
 *fixed a bug in column/row calculation
 *
 *Revision 1.3  2005/01/14 21:31:43  arman
 *added typedef
 *
 *Revision 1.2  2005/01/14 18:16:28  arman
 *first basically working version
 *
 *Revision 1.1  2005/01/13 23:06:05  arman
 *lattice similarator
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_LATTICESIMILARITY_H
#define __RC_LATTICESIMILARITY_H

#include "rc_similarity.h"
#include <boost/shared_ptr.hpp>

  //@class rcLatticeSimilarator
  // 
class rcLatticeSimilarator 
{
 public:

  static const int32 defaultTileSize = 4;
  typedef vector<rcIRect> rowOfRects;
  typedef vector<rcSimilaratorRef> rowOfSims;

  rcLatticeSimilarator ();

  rcLatticeSimilarator(int32 tpipt, int32 lrad);

  template<class Iterator>
    bool fill(Iterator starting, Iterator afterLast);

  template<class Iterator>
    bool update(Iterator nextImage);

  void ttc (rcWindow&, rcWindow&);

  double wholeEntropy () const;

 private:
  int32 mTemporal;
  rcIPair mSize;
  rcIPair mImageSize;
  rcPixel mDepth;
  int32 mLrad;
  vector<rowOfRects> mTiles;
  vector<rowOfSims> mSims;
  rcSimilaratorRef mWholeSim;
};

typedef boost::shared_ptr<rcLatticeSimilarator> rcLatticeSimilaratorRef;


template<class Iterator>
bool rcLatticeSimilarator::fill(Iterator starting, Iterator afterLast)
{
  typedef typename std::iterator_traits<Iterator>::value_type value_type;

  if (mTemporal != distance (starting, afterLast)) return false;

  int32 width, height;
  value_type image (*starting);
  width = image.width(); height = image.height();
  rcIPair tileSize (defaultTileSize, defaultTileSize);

  // @note overlapping tiles. Amount of overlap is determined by step size. 
  // if mLrad is set to defaultTileSize divided by 2, then neighboring tiles overlap 50 percent
  // that is 100 - mLrad / defaultTileSize. 
  int32 rows ((height - defaultTileSize) / mLrad);
  int32 columns ((width - defaultTileSize) / mLrad);

  if (rows <= 0 || columns <=0) return false;
  mSize = rcIPair (columns, rows);
  mImageSize = rcIPair (width, height);
  mDepth = image.depth();

  // Allocate an array of tile rectangles
  mTiles.resize (rows);
  mSims.resize (rows);


  int32 tly (0);

  for (int32 j = 0; j < rows; j++)
    {
      mTiles[j].resize (columns);
      mSims[j].resize (columns);

      int32 tlx (0);
      for (int32 i = 0; i < columns; i++, tlx += mLrad)
	{
	  rcIPair tl (tlx, tly);
	  mTiles[j][i] = rcIRect (tl, tl + tileSize);
	}
      tly += mLrad;
    }

  bool rtn (false);
  for (int32 j = 0; j < rows; j++)
    for (int32 i = 0; i < columns; i++)
      {
	vector<rcWindow> wins;

	Iterator w = starting;
	while (w < afterLast)
	  {
	    value_type img (*w);
	    rtn = (img.size() == mImageSize);
	    if (rtn == false) return rtn;	    


	    static const int32 inside (0);
	    rcWindow tile (img, mTiles[j][i], inside);
	    wins.push_back (tile);
	    w++;
	  }

		  mSims[j][i] = boost::shared_ptr<rcSimilarator> (new rcSimilarator (rcSimilarator::eExhaustive,
																			 mDepth, mTemporal, mTemporal * 2));
	rtn = (mSims[j][i] && mSims[j][i]->fill (wins));
	if (rtn == false) return rtn;
	rmAssert (mSims[j][i]->longTermCache () == false);
	rtn = mSims[j][i]->longTermCache (true);
	if (rtn == false) return rtn;
      }

  if (rtn)
    {
	vector<rcWindow> wins;

	Iterator w = starting;
	while (w < afterLast)
	  {
	    value_type img (*w);
	    rtn = (img.size() == mImageSize);
	    if (rtn == false) return rtn;	    

	    wins.push_back (img);
	    w++;
	  }

		mWholeSim = boost::shared_ptr<rcSimilarator> (new rcSimilarator (rcSimilarator::eExhaustive,
				     mDepth, mTemporal, mTemporal * 2));
      
      rtn = (mWholeSim && mWholeSim->fill (wins));
      if (rtn == false) return rtn;
      rmAssert (mWholeSim->longTermCache () == false);
      rtn = mWholeSim->longTermCache (true);
      if (rtn == false) return rtn;
    }
      
  return rtn;
}


template<class Iterator>
bool rcLatticeSimilarator::update(Iterator nextImage)
{
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  value_type image (*nextImage);  

  bool rtn (false);

  rtn = mWholeSim->update (image);
  if (rtn == false) return rtn;

  for (int32 j = 0; j < mSize.y(); j++)
    for (int32 i = 0; i < mSize.x(); i++)
      {
	static const int32 inside (0);
	rcWindow tile (image, mTiles[j][i], inside);
	rtn = mSims[j][i]->update (tile);
	if (rtn == false) return rtn;
      }
  
  return rtn;
}


  


#endif /* __RC_LATTICESIMILARITY_H */
