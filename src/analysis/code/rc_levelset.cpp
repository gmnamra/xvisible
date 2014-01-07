/*
 *
 *$Id $
 *$Log$
 *Revision 1.2  2004/01/14 20:43:37  arman
 *incremental
 *
 *Revision 1.1  2003/12/01 10:31:52  arman
 *level setting algorithm
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_window.h>


void segment (rcWindow& image, rcWindow& connect, float dt, float lambda, int32 times)
{
  int32 i, j, w_1, h_1, rup;
  float p, pu, pl, pr, pd, pul, pur, pdl, pdr;
  float ux, uy, uxx, uxy, uyy;
  float norm;
  int32 n;
  float curvature_term;
  float dpx, dpy, dmx, dmy;
  float tmx, tmy, tpx, tpy;
  float nabla_m, nabla_p;
  double sum (0.0);

  rmAssert (image.isBound());
  w_1 = image.width() - 1;
  h_1 = image.height() - 1;
  rup = image.rowUpdate ();

  // We use a 32 bit image. It is casted to float
  connect = rcWindow (image.width(), image.height(), rcPixel32);
  rcWindow Buf (image.width(), image.height(), rcPixel32);

  // Convert the image to a float image. Note that for other type of labeld 
  // Information, we would go direct and through a different API
  for (j = 0; j < image.height(); j++)  
    {
      float *fp = (float *) connect.rowPointer (j);
      uint8 *pp = image.rowPointer (j);
      for (i = 0; i < image.width(); i++, fp++, pp++)
	sum += (*fp = ((float) (*pp)) / 255.);
    }

  // Produce mean
  sum /= (float) (image.width() * image.height());

  for (n = 0; n < times; n++)
    {
      for (j = 0; j < image.height(); j++)  
	{
	  float *Fp = (float *) connect.rowPointer (j);
	  float *Bp = (float *) Buf.rowPointer (j);
	  for (i = 0; i < image.width(); i++, Fp++, Bp++)
	    {
	      
	      // Set up pointers. Note that we setup pointers to create a 0 output value at the edges
	      p = *Fp;
	      pl = (i > 0) ? *(Fp-1) : p;
	      pr = (i < w_1) ? *(Fp+1) : p;
	      pu = (j > 0) ? *(Fp-rup) : p;
	      pd = (j < h_1) ? *(Fp+rup) : p;
	      pul = (i > 0) ? *(Fp-rup-1) : pu;
	      pur = (i > w_1) ? *(Fp-rup+1) : pu;
	      pdl = (i > 0) ? *(Fp+rup-1) : pd;
	      pdr = (i > 0) ? *(Fp+rup+1) : pd;

	      dpx = pr - p;
	      dmx = p - pl;
	      dpy = pd - p;
	      dmy = p - pu;

	      tmx = rmMax (dmx, 0);
	      tpx = rmMin (dpx, 0);
	      tmy = rmMax (dmy, 0);
	      tpy = rmMin (dpy, 0);

	      nabla_p = sqrt (rmSquare (tmx) + rmSquare (tpx) + rmSquare (tmy) + rmSquare (tpy));
	      
	      tmx = rmMin (dmx, 0);
	      tpx = rmMax (dpx, 0);
	      tmy = rmMin (dmy, 0);
	      tpy = rmMax (dpy, 0);

	      nabla_m = sqrt (rmSquare (tmx) + rmSquare (tpx) + rmSquare (tmy) + rmSquare (tpy));

	      ux = (pr - pl) / 2.0f;
	      uy = (pd - pu) / 2.0f;
	      uxx = pr - p - p + pl;
	      uxy = (pdr - pur - pdl + pul) / 4.0f;
	      uyy = pd - p - p + pu;

	      cout << "Ux: " << ux << " Uy: " << uy << " Uxx: " << uxx << " Uyy: " << uyy << " Uxy: " << uxy << endl;

	      norm = sqrt (rmSquare (ux) + rmSquare (uy) + 0.000001);
	      curvature_term = (uxx * uy * uy - 2.0 * ux * uy * uxy * uyy + uyy * ux * ux) / norm;



	      *Bp = p + dt * (- rmMax ((1.0f / (p - sum)) - (p - sum), 0) * nabla_p + 
			      rmMin ((1.0f / (p - sum)) - (p - sum), 0) * nabla_m + lambda * curvature_term);

	      cout << "norm: " << norm << " curvature_term: " << curvature_term << " U: " << *((float *) Bp)  << endl;

	      // Check against limits if any
	    }
	}
      connect.copyPixelsFromWindow (Buf);
    }
}
