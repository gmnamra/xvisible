/*
 *
 *$Id $
 *$Log$
 *Revision 1.4  2005/10/27 16:42:15  arman
 *removed references to iRectangle ()
 *
 *Revision 1.3  2005/01/07 16:30:54  arman
 *fixed a minor bug in hyst
 *
 *Revision 1.2  2004/09/24 21:03:13  arman
 *corrected normalization for hessian
 *
 *Revision 1.1  2004/07/12 19:43:28  arman
 *more interesting ip
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include <rc_window.h>
#include <rc_ip.h>

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

