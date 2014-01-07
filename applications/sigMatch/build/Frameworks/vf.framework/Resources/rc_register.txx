/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.7  2005/12/12 23:58:21  arman
 *added optional motion compensation output
 *
 *Revision 1.6  2005/12/05 00:25:30  arman
 *added motion compensation to aci short and long term
 *
 *Revision 1.5  2005/12/04 19:41:27  arman
 *register unit test plus lattice bug fix
 *
 *Revision 1.4  2005/12/03 00:35:36  arman
 *tiff plus motion compensation
 *
 *Revision 1.3  2005/10/25 21:52:27  arman
 *incremental
 *
 *Revision 1.2  2005/10/24 02:07:46  arman
 *incremental
 *
 *Revision 1.1  2005/10/22 23:10:40  arman
 *image set registration
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include <rc_register.h>
#include <rc_1dcorr.h>
#include <rc_kinepyr.h>
#include <rc_numeric.h>

template<class Iterator>
rcImageSetRegister<Iterator>::rcImageSetRegister (Iterator start, Iterator passedFinish, int32 resolution)
{
  mStart = start;
  mEnd = passedFinish;
  mResolution = resolution;

  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  mN = mEnd - mStart;
  if (mN <= 0)
    {
      rmExceptionMacro (<<"VisualFunction Correspondence");
    }
  value_type im (*start);
  mSize = im.size();
  if (mSize.x() <= 0 || (mSize.y() <= 0))
    {
      rmExceptionMacro (<<"VisualFunction Correspondence");
    }
  mIsValid = true;
}

template<class Iterator>
void rcImageSetRegister<Iterator>::setRegister  (Iterator model, Iterator mask, rcIPair range, registerTo option)
{
  // Get container value type

  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  value_type md (*model);
  value_type mk (*mask);	
  bool canPeekOutSide = md.canPeekOutSide (range);
  rcIPair  limit = range + range + rcIPair (minSize, minSize);
  rcWindow space, noMask;

  // If we can not peekOut, use an eroded region within
  if (!canPeekOutSide &&
      limit.x() < mSize.x () &&
      limit.y() < mSize.y())
    {
      rcIPair size (mSize);
      size -= (range + range);
      rcWindow fixedWin (md, range.x(), range.y(), size.x(), size.y());
      rcWindow fixedMask = (mk.isBound ()) ? rcWindow (mk, range.x(), range.y(), size.x(), size.y()) 
	: rcWindow ();
      addModel (fixedWin, fixedMask);
      mOrigin += rc2Fvector ((float) range.x(), (float) range.y());  
      mSize = size;

      Iterator beg = mStart;
      for (uint32 i = 0; beg < mEnd; beg++, i++)
	{
	  //@note if we are in full resolution and on the first image (the one we created the first 
	  // model from), then use dead reckoning. 
	  // In lower resolutions dead reckonning is not reliable. 
	  if ( i == 0 && beg == mStart && resolution() == 1)
	    {
	      rc2Fvector pos (mOrigin);
	      rc2Fvector peakPosition (pos.x() - mOrigin.x(), pos.y() - mOrigin.y());
	      mSequential2Dmoves.push_back (peakPosition);
	      mSequentialCorrelations.push_back (1.0f);
	      continue;
	    }
	  rcFindList peaks (1);
	  rc2Fvector pos;
	  rcFindList::iterator tmp;
			
	  //@note we specifically not use mask if we are using lower resolutions
	  if (resolution() > 1)
	    {
	      rc2LevelSpatialPyramid ip (*beg, 2, 5, 0.0f);
	      rfImageFind (ip.past (1), mModels.back(), peaks, space, true);
	      tmp = peaks.begin();
	      pos = ip.mapPoint (tmp->interp);
	    }
	  else
	    {
	      rfImageFind (*beg, mModels.back(), mModelMasks.back (), peaks, space, true);
	      //	      		rfPrintCorrSpace (space, rcNumericTraits<unsigned short>::One, (float) mQuantz);
	      tmp = peaks.begin();
	      pos = tmp->interp;
	    }

	  /*@note:
	    peakPositions are movements of the first frame to align with Nth frame:
	    peaks is the position of the best peak in the coordinate system of the image
	    @note top left corner of the image is 0,0 and not pos in underlying framebuf
	    Composition of coordinates is as follow:
	    frame[n],frame[0] = model[n-1],frame[n] - model[-1],frame[0]
	    Model was trained at mOrigin. If at half res, transform to full res and then
	    calculate how much it has moved from mOrigin. 
	  */
	  rc2Fvector peakPosition (pos.x() - mOrigin.x(), pos.y() - mOrigin.y());
	  mSequential2Dmoves.push_back (peakPosition);
	  mSequentialCorrelations.push_back (tmp->score);

	  //@todo mask for progressive mode
	  if (option == eString)
	    {
	      const rcSharedFrameBufPtr framePtr = beg->frameBuf();
	      rcIPair xy ( (int32) ( beg->x()+ pos.x() ), (int32) ( beg->y()+ pos.y()));
	      if (xy.x() < 0) xy.x() = 0;
	      if (xy.y() < 0) xy.y() = 0;
	      rcWindow w (framePtr,xy.x(), xy.y(), size.x(), size.y());
	      addModel (w, noMask);
	    }
	}
    }
  else if (canPeekOutSide) // Use an expanded area to look for 
    {
      rcWindow fixedWin (md, range.x(), range.y(), mSize.x(), mSize.y());
      if (mk.isBound ())
	{
	  rcWindow fixedMask (mk, range.x(), range.y(), mSize.x(), mSize.y());
	  addModel (fixedWin, fixedMask);
	}
      else
	addModel (fixedWin, noMask);

      mOrigin = rc2Fvector ((float) range.x(), (float) range.y());

      Iterator beg = mStart;
      for (uint32 i = 0; beg < mEnd; beg++, i++)
	{
	  rcFindList peaks (1);
	  value_type im (*beg);
	  im.trim (-range);
	  rfImageFind (im, mModels.back (), mModelMasks.back (), peaks, space);
	  rcFindList::iterator tmp = peaks.begin();
	  rc2Fvector peakPosition (tmp->interp.x() - (float) mOrigin.x(), tmp->interp.y() - (float) mOrigin.y());
	  mSequential2Dmoves.push_back (peakPosition);
	  mSequentialCorrelations.push_back (tmp->score);

	}
    }
}

template<class Iterator>
void rcImageSetRegister<Iterator>::setRegister (Iterator model, rcIPair range, registerTo option)
{
  // Get container value type

  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  value_type md (*model);
  bool canPeekOutSide = md.canPeekOutSide (range);
  rcIPair  limit = range + range + rcIPair (minSize, minSize);
  rcWindow space, noMask;
	
  // If we can not peekOut, use an eroded region within
  if (!canPeekOutSide &&
      limit.x() < mSize.x () &&
      limit.y() < mSize.y())
    {
      rcIPair size (mSize);
      size -= (range + range);
      rcWindow fixedWin (md, range.x(), range.y(), size.x(), size.y());
      addModel (fixedWin, noMask);

      if (option == eSpring)
	{
	  rcWindow fixedWin (*(mEnd-1), range.x(), range.y(), size.x(), size.y()); 
	  addModel (fixedWin, noMask);
	}

      mOrigin += rc2Fvector ((float) range.x(), (float) range.y());  
      mSize = size;

	  //@note (removed) if we are in full resolution and on the first image (the one we created the first 
	  // model from), then use dead reckoning. 
	  // In lower resolutions dead reckonning is not reliable. 
	  // 	  bool useDeadReckonning = ((beg == mStart && i == 0) ||
	  // 				    (option == eLast && beg == (mEnd-1)));
	  // 	  if ( useDeadReckonning && resolution() == 1)
	  // 	    {
	  // 	      rc2Fvector pos (mOrigin);
	  // 	      rc2Fvector 	peakPosition (pos.x() - mOrigin.x(), pos.y() - mOrigin.y());
	  // 	      mSequential2Dmoves.push_back (peakPosition);
	  // 	      mSequentialCorrelations.push_back (1.0f);
	  // 	      continue;
	  // 	    }

      Iterator beg = mStart;
      for (uint32 i = 0; beg < mEnd; beg++, i++)
	{

	  rcFindList peaks (1);
	  rc2Fvector pos;
	  rcFindList::iterator tmp;
	  if (resolution() > 1)
	    {
	      rc2LevelSpatialPyramid ip (*beg, 2, 5, 0.0f);
	      rfImageFind (ip.past (1), mModels.back(), peaks, space, true);
	      tmp = peaks.begin();
	      pos = ip.mapPoint (tmp->interp);
	    }
	  else
	    {
	      rfImageFind (*beg, mModels.back(), peaks, space, true);
	      //	      rfPrintCorrSpace (space, rcNumericTraits<unsigned short>::One, (float) mQuantz);
	      tmp = peaks.begin();
	      pos = tmp->interp;
	      {
		const rcSharedFrameBufPtr framePtr = beg->frameBuf();
		rcIPair xy ( (int32) ( beg->x()+ pos.x() ), (int32) ( beg->y()+ pos.y()));
		if (xy.x() < 0) xy.x() = 0;
		if (xy.y() < 0) xy.y() = 0;
		rcWindow w (framePtr,xy.x(), xy.y(), size.x(), size.y());
		if (w.size() == mModels.back ().size ())
		  {
		    rsCorrParams rp;
		    rcCorr rs;
		    rfCorrelate (w, mModels.back (), rp, rs);
		  }
				      
	      }
	    }
	  /*@note:
	    peakPositions are movements of the first frame to align with Nth frame:
	    peaks is the position of the best peak in the coordinate system of the image
	    @note top left corner of the image is 0,0 and not pos in underlying framebuf
	    Composition of coordinates is as follow:
	    frame[n],frame[0] = model[n-1],frame[n] - model[-1],frame[0]
	    Model was trained at mOrigin. If at half res, transform to full res and then
	    calculate how much it has moved from mOrigin. 
	  */
	  rc2Fvector 	peakPosition (pos.x() - mOrigin.x(), pos.y() - mOrigin.y());
	  mSequential2Dmoves.push_back (peakPosition);
	  mSequentialCorrelations.push_back (tmp->score);

	  if (option == eString)
	    {
	      const rcSharedFrameBufPtr framePtr = beg->frameBuf();
	      rcIPair xy ( (int32) ( beg->x()+ pos.x() ), (int32) ( beg->y()+ pos.y()));
	      if (xy.x() < 0) xy.x() = 0;
	      if (xy.y() < 0) xy.y() = 0;
	      rcWindow w (framePtr,xy.x(), xy.y(), size.x(), size.y());
	      addModel (w, noMask);
	    }
	}
    }
  else if (canPeekOutSide) // Use an expanded area to look for 
    {
      rcWindow fixedWin (md, range.x(), range.y(), mSize.x(), mSize.y());
      addModel (fixedWin, noMask);

      mOrigin = rc2Fvector ((float) range.x(), (float) range.y());

      Iterator beg = mStart;
      for (uint32 i = 0; beg < mEnd; beg++, i++)
	{
	  rcFindList peaks (1);
	  value_type im (*beg);
	  im.trim (-range);
	  rfImageFind (im, mModels.back (), peaks, space);
	  rcFindList::iterator tmp = peaks.begin();
	  rc2Fvector peakPosition (tmp->interp.x() - (float) mOrigin.x(), tmp->interp.y() - (float) mOrigin.y());
	  mSequential2Dmoves.push_back (peakPosition);
	  mSequentialCorrelations.push_back (tmp->score);

	}
    }
}

template<class Iterator>
void rcImageSetRegister<Iterator>::setHistogramCompare (Iterator model, registerTo option, int32 range)
{
  // Get container value type
  typedef typename std::iterator_traits<Iterator>::value_type value_type;
  value_type md (*model);
  rcWindow mk; // note

  vector<double> ih;
  rfDirectionSignal (md, mk, ih);
  mDirSignals.push_back (ih);
  if (option == eSpring)
    {
      rcWindow notbounded;
      rfDirectionSignal (*(mEnd-1), notbounded, ih);
      mDirSignals.push_back (ih);
    }
	    
  Iterator beg = mStart;
  double rCal (0.0);
	
  for (uint32 i = 0; beg < mEnd; beg++, i++)
    {
      vector<double> mh;		
      rfDirectionSignal (*beg, mk, mh);
      vector<double>::iterator mtr = mh.end();
      vector<double>::iterator itr = mDirSignals.back().end();
      double r (0.0);
      double q = (i == 0) ? rf1DRegister (mh.begin(), mtr, mDirSignals.back().begin(), itr, (uint32) range, r, rCal) :
	rf1DRegister (mh.begin(), mtr, mDirSignals.back().begin(), itr, (uint32) range, r);
      if (i == 0 && option == eNorm) rmAssert (rCal > 0.0);
      r = (option == eNorm) ? rmMin (1.0, r / rCal) : r ;
      mSequentialCorrelations.push_back ( (float) r);;
      mSequential1Dmoves.push_back ((float) q);
		
      if (option == eString || option == eNorm)
	{
	  vector<double> mh;		
	  rfDirectionSignal (*beg, mk, mh);
	  vector<double>::iterator mtr = mh.end();
	  vector<double>::iterator itr = mDirSignals.back().end();
	  double r (0.0);
	  double q = (i == 0) ? rf1DRegister (mh.begin(), mtr, mDirSignals.back().begin(), itr, (uint32) range, r, rCal) :
	    rf1DRegister (mh.begin(), mtr, mDirSignals.back().begin(), itr, (uint32) range, r);
	  if (option == eString) rmAssert (rCal > 0.0);
	  r = (option == eString) ? rmMin (1.0, r / rCal) : r ;
	  mSequentialCorrelations.push_back ( (float) r);;
	  mSequential1Dmoves.push_back ((float) q);
	  mDirSignals.push_back(mh);
	  continue;
	}

      if (option == eSpring)
	{
	  rmAssert (mDirSignals.size () == 2);
		  
	  mDirSignals.push_back(mh);
	  continue;
	}
		
    }
}

template<class Iterator>
void rcImageSetRegister<Iterator>::addModel (rcWindow& fixedWin, rcWindow& fixedMask)
{
  if (mResolution == 1)
    {
      rcWindow fixed (fixedWin.width(), fixedWin.height(), fixedWin.depth());
      fixed.copyPixelsFromWindow (fixedWin);
      mModels.push_back (fixed);
      if (fixedMask.isBound() && fixedMask.size () == fixedWin.size ())
	{
	  rcWindow fixedmk (fixedWin.width(), fixedWin.height(), fixedWin.depth());
	  fixedmk.copyPixelsFromWindow (fixedMask);
	  mModelMasks.push_back (fixedmk);	
	}
    }
  else
    {
      // @todo implement reduction of passed in mask
      rmAssert (!fixedMask.isBound ());
      rc2LevelSpatialPyramid mp (fixedWin, 2, 5);
      mModels.push_back (mp.past (1));
    }

  // Figure out model resolution
  float xOrg = (fixedWin.width() - mModels.back ().width () * resolution ()) / resolution ();
  float yOrg = (fixedWin.height() - mModels.back ().height () * resolution ()) / resolution ();
  mOrigin = rc2Fvector (xOrg, yOrg);
}


