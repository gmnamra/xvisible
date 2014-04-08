/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2004/01/29 19:53:47  proberts
 *Speed up of muscle cell segmentation - renamed rcMCT to rcMuscleSegmenter Fixed rcPolygon bug
 *
 *Revision 1.2  2003/03/29 11:31:02  arman
 *image to draw in to can not be const
 *
 *Revision 1.1  2003/03/27 23:12:32  arman
 *flood filling for testing
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcFILL_H
#define __rcFILL_H

#include <rc_window.h>

void
rc_fill_poly (rcWindow& image,	/* image to draw into */
	       int32 num_pts,	/* number of polygon vertices */
	       double *coords,	/* array: x1, y1, x2, y2, ... */
	       uint32 colors	/* colors to use */
	      );

/* Utility function to do gray-scale filling of a polygon defined by the
   vertices whose coordinates are given in <coords>.

This routine REQUIRES that the points be ordered in circular fashion to
   outline a convex polygon. In otherwords, the centroid of the polygon may
   not be contained in any of the boundary segments. */


#endif /* __rcFILL_H */
