/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.11  2004/05/28 17:31:25  arman
 *added cylinder drawing
 *
 *Revision 1.10  2003/08/28 20:08:42  arman
 *fixed unsigned to signed
 *
 *Revision 1.9  2003/08/27 22:30:13  sami
 *Use int32 for all frame/window geometry values
 *
 *Revision 1.8  2003/06/09 19:04:29  arman
 *added couple of new methods
 *
 *Revision 1.7  2003/04/28 21:58:53  sami
 *Silenced compiler warning
 *
 *Revision 1.6  2003/04/17 21:30:26  arman
 *added drawshape
 *
 *Revision 1.5  2003/04/17 14:25:54  arman
 **** empty log message ***
 *
 *Revision 1.4  2003/04/17 13:30:02  arman
 **** empty log message ***
 *
 *Revision 1.3  2003/04/14 13:38:52  arman
 *added a double aware image printing
 *
 *Revision 1.2  2003/03/04 18:06:24  sami
 *Silenced compiler warning
 *
 *Revision 1.1  2003/02/27 13:57:46  arman
 *drawing utils for deevelopment and ut
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcUTDRAWUTILS_H
#define __rcUTDRAWUTILS_H

#include <rc_window.h>
#include <iomanip>

// These utility drawing functions are for UT and development puposes ONLY. 
// There are static and in an include file to avoid linking and building issues.

#define rmPrintImage(a){				\
  cout << (a).width() << "," << (a).height() << endl; \
  cout << "||--";					\
  for (int32 i = 0; i < (a).width(); i++)	\
    cout << setw (4) << i;		\
  cout << endl;						\
  for (int32 j = 0; j < (a).height(); j++)	\
    {						\
      cout << "[" << setw (4) << j << "]";			\
      for (int32 i = 0; i < (a).width(); i++)	\
	{						\
	  if ((a).depth() != 8)				\
	    cout <<  setw(4) << (int32) ((a).getPixel (i, j));       \
	  else cout <<   setw(4) << setprecision(2) << (a).getDoublePixel (i, j);  \
	}						\
      cout << endl;					\
    }}


/*
 * Utility function to draw a shape into a pelbuffer
 * optional scaling
 */
static void rfDrawShape(rcWindow& image, char **shape,int32 x = 0,int32 y = 0,int32 scale = 1)
{
   char **s;
   int32 width = x;
   int32 height = y;
   for(s = shape; *s; s++,height += scale)
   {
      int32 this_width = strlen(*s)*scale + x;
      if(this_width > width)
         width = this_width;
   }

   rcWindow src(width,height);
   src.setAllPixels (int32(0));

   for( ; *shape; shape++)
   {
      for(char *valPtr = *shape; *valPtr; valPtr++)
         for(int32 xx = 0; xx < scale; xx++)
            for(int32 yy = 0; yy < scale; yy++)
               src.setPixel(x + (valPtr - *shape)*scale + xx,y + yy,
                       (uint32)(*valPtr - '0'));
      y += scale;
   }

   image = src;
}

static void rfDrawCylinder (deque<rcWindow>& cylinder, rc2Dvector center, double ra, double rb)
{
  for (uint32 i = 0; i < cylinder.size(); i++)
    cylinder[i].setAllPixels (0);

  int32 width (cylinder[0].width());
  int32 height (cylinder[0].height());

  double dIdT = cylinder.size() / 255.0;
  double iDiDt (0.0);

  double minR = rmMin (ra, rb);
  double maxR = rmMax (ra, rb);

  // Basic Linear Model
  // TBD: Support power law models
  // Forward Diffusion
  for (uint32 tt = 0; tt < cylinder.size(); tt++)
    for(int32 i=0;i< width;i++)
      {
	for(int32 j=0;j< height; j++)
	  {
	    rc2Dvector pos(i+0.5,j+0.5);
	    if (center.distance (pos) <= maxR) 
	      cylinder[tt].setPixel (i,j,uint32 (iDiDt));
	    iDiDt += dIdT;
	  }
      }

  // Reverse Diffusion
  iDiDt = 0.0;
  for (uint32 tt = 0; tt < cylinder.size(); tt++)
    for(int32 i=0;i< width;i++)
      {
	for(int32 j=0;j< height; j++)
	  {
	    rc2Dvector pos(i+0.5,j+0.5);
	    if (center.distance (pos) <= minR) 
	      cylinder[tt].setPixel (i,j,255 - uint32 (iDiDt));
	    iDiDt += dIdT;
	  }
      }
}


static void rfDrawCircle(rc2Dvector center, int32 r, rcWindow& dst, uint8 circleColor, uint8 backgroundColor)
{
   dst.setAllPixels (uint32 (backgroundColor));

   for(int32 i=0;i< dst.width();i++)
   {
      for(int32 j=0;j< dst.height(); j++)
      {
         rc2Dvector pos(i+0.5,j+0.5);
         if (center.distance (pos) <= r) dst.setPixel (i,j,circleColor);
      }
   }
}


static void rfDrawCircle(rc2Dvector center, int32 r, rcWindow& dst, uint8 circleColor)
{
   for(int32 i=0;i< dst.width();i++)
   {
      for(int32 j=0;j< dst.height(); j++)
      {
         rc2Dvector pos(i+0.5,j+0.5);
         if (center.distance (pos) <= r) dst.setPixel (i,j,circleColor);
      }
   }
}


// Draw a image of circles
static int32 drawCells (rcWindow& w, int32 size, int32 spacing, uint8 circleColor, uint8 backgroundColor)
{
   int32 r = size / (spacing * 3);

   rcWindow dis (size, size);
   dis.setAllPixels (uint32 (backgroundColor));

   int32 n = (size -4*r) / (r+r+r);

   for (int32 i = 0; i < n; i++)
   {
      for (int32 j = 0; j < n; j++)
      {
          rc2Dvector center (static_cast<double>(r), static_cast<double>(r));
          rc2Dvector pos (static_cast<double>((i+1)*3*r), static_cast<double>((j+1)*3*r));
          rcWindow tmp (dis, static_cast<int32>(pos.x() - r), static_cast<int32>(pos.y()) - r, r+r, r+r);
          rfDrawCircle (center, r, tmp, circleColor, backgroundColor);
      }
   }

   w = dis;

   return n*n;
}


static void rfDrawLine(rcWindow& image, rcIPair& p1, rcIPair& p2,uint8 ink)
{
  int32& x1 = p1.x();
  int32& y1 = p1.y();
  int32& x2 = p2.x();
  int32& y2 = p2.y();

  int32 d, x, y, ax, ay, sx, sy, dx, dy;
  dx = x2-x1; ax = rmABS(dx)<<1; sx = rmSGN(dx);
  dy = y2-y1; ay = rmABS(dy)<<1; sy = rmSGN(dy);
  x = x1;
  y = y1;
  if (ax>ay) { /* x dominant */
    d = ay-(ax>>1);
    for (;;) {
      image.setPixel (x, y, ink);
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
      image.setPixel (x, y, ink);
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


/*******************************************************************************
 * ZonePlate
 *******************************************************************************/
static unsigned char sineTab[256];


static void
MakeSineTab(void)
{
  long i;
  for (i = 0; i < 256; i++)
    {
      sineTab[i] = (uint8) (127.5 * sin(rkPI * (i - 127.5) / 127.5) + 127.5);
    }
}

static rcWindow
rc_zonePlate(long width, long height, long scale)
{
  long cX, cY;
  long i, j;
  long x, y;
  long d;
	
  cX = width / 2;
  cY = height / 2;

  rcWindow rw (width, height);
  for (i = height, y = -cY; i--; y++)
    {
      for (j = width, x = -cX; j--; x++)
	{
	  d = ((x * x + y * y) * scale) >> 8;
	  rw.setPixel(j, i, sineTab[d & 0xFF]);
	}
    }
  return rw;
}




#endif /* __rcUTDRAWUTILS_H */
