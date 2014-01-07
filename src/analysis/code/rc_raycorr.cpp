/*
 *
 *$Id $
 *$Log$
 *Revision 1.1  2004/01/11 15:16:50  arman
 *first version of ray processing
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_1dcorr.h>
#include <rc_fit.h>
#include <rc_types.h>


bool rcRayCorr::hasResults() const
{
  bool rtn = (mSpace.size() != 0) && (mPathWays.size() != 0) && mIs;
  return rtn;
}

    
bool rcRayCorr::rayCorr (rcWindow& image, rcWindow& site, rcFPair& org, int32 direction, rcIPair& start)
{
  int32 misses (0);
  int32 dir = mPathWays.size();
  rcIPair size = site.size();
  rcIPair origin ((int32) (org.x() * size.x()),
		  (int32) (org.y() * size.y()));
  static rcIPair nowhere (rcINT32_MAX, rcINT32_MAX);

  // Note that rowUpdate is the same between a window and its 
  // framebuf
  const uint32& fixedRUP = site.rowUpdate();
  const uint32& movingRUP = image.rowUpdate();
  mDiffusion = 0.0;
  rcIPair& rmax = nowhere;
  int32 rMaxIndex (-1);
  float rMax (0.0);

  if (dir == 0) return false;

  if (mPathWays[direction].size() == 0) return false;
  if (direction >= dir) return false;

  vector<rcIPair>::const_iterator ip = mPathWays[direction].begin();

  // Walk the line and fill up the correlation space
  int32 i (0);
  for (; ip != mPathWays[direction].end(); ip++, i++)
    {
      rcIPair spot (*ip - origin + start);
      if (image.isWithin (spot))
	{
	  rcCorr res;	      
	  int32 inSide (0);
	  rcIRect rect (spot, spot+size);
	  rcWindow rw (image, rect, inSide);
	  misses += !inSide;
	  if (!inSide) continue;
	  rfCorrelate (rw.pelPointer(0,0),
		       site.pelPointer(0,0),
		       fixedRUP, movingRUP,
		       size.x(), size.y(),
		       res);
	  float r = (float) res.r();
	  mDiffusion += r;
	  mSpace.push_back (r);
	  if (r > rMax)
	    {
	      rMax = r;
	      rmax = *ip;
	      rMaxIndex = i;
	    }
	}
    }

  // If we do have a max, interpolate in the ray direction
  // Size of correlation space defines existence of resutls.
  float ds (0.0);
  int32 s = mSpace.size();

  if (s > 1)
    {
      // at least 3 and max not at the borders
      if (s > 2 && rMaxIndex > 0 && rMaxIndex < (s - 1))
	{
	  static float *fdum (0);

	  ds = parabolicFit (mSpace[rMaxIndex - 1],
			     mSpace[rMaxIndex],
			     mSpace[rMaxIndex + 1], fdum);
	}
      else  // at least 2 and Max at the borders
	{
	  ds = mSpace[s - 2] / (mSpace[s-1] + mSpace[s-2]);
	}
    }

  mBest = rc2Fvector (mR + ds, (rk2PI * direction) / mDirs);
  
  return (bool (s));
}
  
	      
  
rcRayCorr::rcRayCorr (int32 dirQ, int32 r)
  : mDirs ((float) dirQ), mR ((float) r), mIs (false), mIsClipped (false)
{
  static rcIPair nullIPair(0, 0);
  static const int32 dummy (0);

  // Create dirQ lists of points
  for (int32 i = 0; i < dirQ; i++)
    mPathWays.resize(dirQ);

  // Angle is with respect to tv coordinates
  // Record offsets from origin
  // Note that we are using the integer dir
  for (int32 i = 0; i < dirQ; i++)
    {
      rc2Fvector point (mR, (rk2PI * i) / mDirs);
      rcIPair ipoint ((point.x() < 0 ? rfRoundNeg (point.x(), dummy) : 
		       rfRoundPlus (point.x(), dummy)), 
		      (point.y() < 0 ? rfRoundNeg (point.x(), dummy) : 
		       rfRoundPlus (point.x(), dummy)));
      int32& x1 = nullIPair.x();
      int32& y1 = nullIPair.y();
      int32& x2 = ipoint.x();
      int32& y2 = ipoint.y();

      int32 d, x, y, ax, ay, sx, sy, dx, dy;
      dx = x2-x1; ax = rmABS(dx)<<1; sx = rmSGN(dx);
      dy = y2-y1; ay = rmABS(dy)<<1; sy = rmSGN(dy);
      x = x1;
      y = y1;
      if (ax>ay) { /* x dominant */
	d = ay-(ax>>1);
	for (;;) {
	  mPathWays[i].push_back (rcIPair(x, y));
	  if (x==x2) return;
	  if (d>=0) {
	    y += sy;
	    d -= ax;
	  }
	  x += sx;
	  d += ay;
	}
      }
      else { /* y dominant */
	d = ax-(ay>>1);
	for (;;) {
	  mPathWays[i].push_back (rcIPair(x, y));
	  if (y==y2) return;
	  if (d>=0) {
	    x += sx;
	    d -= ay;
	  }
	  y += sy;
	  d += ax;
	}
      }
    }

  // Finally mark it ok
  mIs = true;
}


      
