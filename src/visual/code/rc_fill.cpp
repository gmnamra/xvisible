/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2004/01/29 19:53:47  proberts
 *Speed up of muscle cell segmentation - renamed rcMCT to rcMuscleSegmenter Fixed rcPolygon bug
 *
 *Revision 1.2  2003/03/29 11:31:43  arman
 *image that is draw in to can not be const
 *
 *Revision 1.1  2003/03/27 23:13:54  arman
 *Basic Flood Filling For test purposes.
 *
 *Revision 1.1  2003/03/27 23:12:32  arman
 *flood filling for testing
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#define TRUE 1
#define FALSE 0

#include <rc_math.h>
#include <rc_window.h>

#include <string.h>  // for memset


/*-------------------------------------------------------------------------*/
/* Debug stuff. */

#ifndef DEBUG
#define DEBUG 1
#endif

#if DEBUG
#include <string.h>		/* strcpy() */
#include <stdlib.h>		/* atoi() */

static uint32 diag_flags;	/* Debug flag for internal diagnostics */

#define DIAG_ENTRY		0x01
#define DIAG_FILL		0x02
#define DIAG_POLY_CLIP		0x04
#define DIAG_POLY_CLIP_FINE	0x08

#endif


/*-------------------------------------------------------------------------*/
/* Private defines and data types. */

#define MAX_STK_SEGS	16	/* maximum segments in polygon for stack
				   allocation */

#define CLAMP(A,MIN,MAX) {if (A<MIN) A=MIN; else if (A>MAX) A=MAX;}

/* Boundary type of polygon segment: */

#define BNDRY_TYPE_BOTTOM_RIGHT	0
#define BNDRY_TYPE_BOTTOM_LEFT	1
#define BNDRY_TYPE_TOP_RIGHT	2
#define BNDRY_TYPE_TOP_LEFT	3

#define LEFT_BNDRY 0x1		/* flag to indicate "is a left boundary" */
#define TOP_BNDRY  0x2		/* flag to indicate "is a top boundary" */

/* Type of intersection of boundary segment with pel: */

#define ISECT_NONE		0
#define ISECT_TOP_LEFT		1
#define ISECT_TOP_BOTTOM	2
#define ISECT_TOP_RIGHT		3
#define ISECT_LEFT_RIGHT	4
#define ISECT_LEFT_BOTTOM	5
#define ISECT_BOTTOM_RIGHT	6

/* Type of segment slope: */

#define SLOPE_VERT		0
#define SLOPE_HORZ		1
#define SLOPE_POS		2
#define SLOPE_NEG		3

#if DEBUG

/* Table of 2-character names to correspond to the BNDRY_TYPE_xxx constants: */

static const char *bndry_type_name_tab[] = {
    "BR",
    "BL",
    "TR",
    "TL"
};
/* Table of 2-character names to correspond to the ISECT_xxx constants: */

static const char *isect_type_name_tab[] = {
    "NO",
    "TL",
    "TB",
    "TR",
    "LR",
    "LB",
    "BR"
};

/* Table of 1-character names to correspond to the SLOPE_xxx constants: */

static const char *slope_type_name_tab[] = {
    "V",
    "H",
    "+",
    "-"
};

#endif

/* Table of booleans that tells whether the area to one side of a boundary
   segment that intersects a unit square, is outside the region partially
   enclosed by the segment. Rectangular and trapezoidal areas are always to
   the left or above the line; triangular regions are whereever they are
   formed. The table lists a 1 if the area is outside, 0 if it is inside, or
   -1 for an illegal combination (should never occur). The rows of the table
   are ordered to correspond with the ordering of the BNDRY_TYPE_xxx line
   types. */

static short blisect_outside_tab[4][7] = {

    /** Line type	          Intersection type
			  None TL   TB   TR   LR   LB   BR	*/

    /*  bottom-right */	{ 0,   0,   0,  -1,   0,  -1,   1	},
    /*  bottom-left */	{ 0,  -1,   1,   0,   0,   1,  -1	},
    /*  top-right */	{ 1,  -1,   0,   1,   1,   0,  -1	},
    /*  top-left */	{ 1,   1,   1,  -1,   1,  -1,   0	},
			
};

/* This defines the tolerance to determine if the coordinates of the
   intersection of two lines are within the bounds of the line segments. At
   first we used a small multiple of DBL_EPSILON, but this is routinely
   exceeded when we intersect nearly-parallel lines. We now set the limit
   arbitrarily, keeping in mind that we're talking about pels, and 1/1000 of
   a pel is below any significance. */

#define CLOSE_ENOUGH (1E-6)


/*-------------------------------------------------------------------------*/

/* Internal structure to hold information about a segment (i.e. line segment,
   arc) of a polygon: */

typedef struct poly_segment {

    /* Basic information: */

    struct poly_segment *next;	/* ptr for linked list */
    double x;			/* X coord of starting vertex */
    double y;			/* Y coord of starting vertex */
    int32 ix;			/* x as an int (upper-left X coord) */
    int32 iy;			/* y as an int (upper-left Y coord) */
    int32 vertex_in_image;	/* is starting vertex in the image? */

    /* These fields are used by the main polygon filling routine: */

    double x1;			/* X coord of isect with pel upper line */
    double x2;			/* X coord of isect with pel lower line */
    double y1;			/* Y coord of isect with pel left line */
    double y2;			/* Y coord of isect with pel right line */
    double vx;			/* X coord of raw seg vector */
    double vy;			/* Y coord of raw seg vector */
    double y1_start;		/* y1 for starting pel column */
    short type;			/* boundary type (BNDRY_TYPE_xxx) */
    short slope;		/* slope type (SLOPE_xxx) */
    short *isect_outside;	/* bool array: isect outside rect? */
    double dx;			/* change in X isect per pel row */
    double dy;			/* change in Y isect per pel column */
    int32 min_col;		/* lowest pel col # containing me */
    int32 max_col;		/* highest pel col # containing me */
    int32 min_row;		/* lowest pel row # containing me */
    int32 max_row;		/* highest pel row # containing me */
    int32 first_col;		/* lowest min_col for any row */
    int32 last_col;		/* highest max_col for any row */
    int32 use_me;		/* use this segment for intersection calcs? */
    unsigned char old_vert_int; /* intensity of vertex pel on entry */
    char name[3];		/* name of segment (for debugging) */
}            poly_segment, poly_segment_a[1];

/* The internal polygon structure. This is also used to hold the fill state. */

typedef struct {
    int32 max_segs;		/* maximum number of segments */
    int32 num_segs;		/* number of segments */
    poly_segment *segs;		/* array of segments */
    int32 bndry_wraps_pos;	/* angles are positive traversing boundary? */
    int32 min_col;		/* lowest pel col # containing me */
    int32 max_col;		/* highest pel col # containing me */
    int32 min_row;		/* lowest pel row # containing me */
    int32 max_row;		/* highest pel row # containing me */

    /* In-progress filling state information for the current row: */

    poly_segment *left_bndry;	/* left boundary segment */
    poly_segment *right_bndry;	/* right boundary segment */
    int32 single_bndry;		/* single boundary on both left & right? */
    int32 col1;			/* pel < this is outside polygon */
    int32 col2;			/* col2 < pel < col3 is all inside polygon */
    int32 col3;
    int32 col4;			/* pel > col4 is outside polygon */
}      polygon, polygon_a[1];


/*-------------------------------------------------------------------------*/

#if DEBUG

static void
trunc_form (char buf[],
	    const char *format,
	    double val
)
/* Formats a value as in sprintf(), but does not allow the result to take
   more space than the format specifier allows. */
{
    int32 maxlen = atoi (&format[1]);
    int32 len;
    char longbuf[512];

    sprintf (longbuf, format, val);
    len = strlen (longbuf);

    if (maxlen < 1 || len <= maxlen) {
	strcpy (buf, longbuf);
    } else if (val < 0.0) {
	strcpy (buf, "-inf");
    } else {
	strcpy (buf, "+inf");
    }
}
#endif


/*-------------------------------------------------------------------------*/

#if DEBUG

static void
dump_poly (const polygon * poly)

/* Debug utility to print details about the polygon. */
{
    poly_segment *seg;

    printf ("Polygon Boundary wraps %s\n", poly->bndry_wraps_pos ? "pos" : "neg");
    printf ("%d segments (max %d) contained in rows %d-%d, cols %d-%d\n",
	    poly->num_segs, poly->max_segs,
	    poly->min_row, poly->max_row, poly->min_col, poly->max_col);

    printf ("\n #: %6s %6s %5s %5s %2s %2s %5s %5s %5s %5s %7s %7s\n",
	    "X", "Y",
	    "vx", "vy",
	    "sl", "tp",
	    "dx", "dy",
	    "x1s", "y1s",
	    "rows", "cols");

    seg = poly->segs;
    while (1) {
	char dxbuf[10];
	char dybuf[10];
	char x2buf[10];
	char y1buf[10];

	trunc_form (dxbuf, "%5.1f", seg->dx);
	trunc_form (dybuf, "%5.1f", seg->dy);
	trunc_form (x2buf, "%5.1f", seg->x2);
	trunc_form (y1buf, "%5.1f", seg->y1_start);

	printf ("%2s: %6.2f %6.2f %5.1f %5.1f %2s %2s %5s %5s %5s %5s %3d:%-3d %3d:%-3d\n",
		seg->name,
		seg->x, seg->y,
		seg->vx, seg->vy,
		slope_type_name_tab[seg->slope],
		bndry_type_name_tab[seg->type],
		dxbuf, dybuf,
		x2buf, y1buf,
		seg->min_row, seg->max_row,
		seg->first_col, seg->last_col
	    );

	seg = seg->next;
	if (seg == poly->segs)
	    break;
    }

    printf ("\n");
}

#endif


/*-------------------------------------------------------------------------*/
/* Polygon area computation. */

static double
polygon_area (const polygon * poly	/* array of pointers to input
					   vertices */
)
/* Returns the area bounded by <poly>. Assumes convex polygon. This method is
   from standard CRC analytic geometry. */
{
    poly_segment *s1;
    poly_segment *s2;
    double area = 0;		/* return value */
    int32 n;

    if (poly->num_segs < 3)
	return 0;

    s1 = poly->segs;
    s2 = s1->next;
    for (n = poly->num_segs; n > 0; --n) {

	area += s1->x * s2->y - s1->y * s2->x;
	s1 = s2;
	s2 = s2->next;
    }
    return 0.5 * area;
}


/*-------------------------------------------------------------------------*/

static void
coords_to_poly (int32 num_pts,	/* number of polygon vertices */
		double *coords,	/* coord array: x1, y1, x2, y2, ... */
		polygon * poly,	/* resulting polygon */
		int32 max_row,	/* maximum row */
		int32 max_col	/* maximum column */
)
/* Converts a list of vertex coordinates to an internal polygon data
   structure. The resulting polygon wraps in the positive direction. */
{
    poly_segment *seg;		/* loop ptr */
    int32 n;			/* loop index */
    double *c;			/* ptr to current coordinate */
    double area;		/* polygon area */

    assert (num_pts <= poly->max_segs);

    /* Initialize subject polygon: */

    poly->num_segs = num_pts;
    seg = poly->segs;
    c = coords;

    /* Form a circular linked list with the vertex coordinates: */

    for (n = poly->num_segs; n > 0; --n, ++seg) {
	seg->x = *c++;
	seg->y = *c++;
	seg->ix = (int32) floor (seg->x);
	seg->iy = (int32) floor (seg->y);

	seg->vertex_in_image = TRUE;
	if (seg->ix < 0
		|| seg->ix > max_col
		|| seg->iy < 0
		|| seg->iy > max_row) {
	    seg->vertex_in_image = FALSE;
	}
	if (n == 1)
	    seg->next = poly->segs;
	else
	    seg->next = seg + 1;
    }

    /* Compute the overall polygon wrapping direction. The segment vectors
       must rotate plus or minus 360 degrees for one complete boundary
       traversal.  We check the sign of the area as determined by the cross
       products to determine direction. This works for an arbitrary,
       non-convex polygon. */

    area = polygon_area (poly);

    poly->bndry_wraps_pos = (area > 0);

    /* If the polygon wraps in the negative direction, reverse the segment
       order. The routine clip_polygon_to_pel() requires the polygon to wrap
       in the positive direction. */

    if (area < 0.0) {
	poly->segs->next = &poly->segs[poly->num_segs - 1];
	for (n = poly->num_segs - 1, seg = poly->segs + 1;
		n > 0; --n, ++seg) {
	    seg->next = seg - 1;
	}
    }
    poly->bndry_wraps_pos = TRUE;

    /* Now initialize the rest: */

    poly->min_col = poly->min_row = rcINT32_MAX;
    poly->max_col = poly->max_row = 0;

    for (n = poly->num_segs, seg = poly->segs; n > 0; --n, seg = seg->next) {

	seg->vx = seg->next->x - seg->x;
	seg->vy = seg->next->y - seg->y;

	/* Determine the slope of the segment: */

	if (seg->vy == 0.0)
	    seg->slope = SLOPE_HORZ;
	else if (seg->vx == 0.0)
	    seg->slope = SLOPE_VERT;
	else if ((seg->vx >= 0.0) ^ (seg->vy > 0.0))
	    seg->slope = SLOPE_NEG;
	else
	    seg->slope = SLOPE_POS;

	/* Set the magnitudes of dx and dy so they reflect the change in
	   coordinate for an integral step in pixel coordinate.  Set the
	   signs of dx and dy so they reflect the increase or decrease of the
	   intercepts as the pel row or column is incremented: */

	switch (seg->slope) {
	case SLOPE_HORZ:
	    seg->dx = 1;
	    seg->dy = 0;
	    break;

	case SLOPE_VERT:
	    seg->dx = 0;
	    seg->dy = 1;
	    break;

	case SLOPE_POS:
	    seg->dx = fabs (seg->vx / seg->vy);
	    seg->dy = 1.0 / seg->dx;
	    break;

	case SLOPE_NEG:
	    seg->dx = -fabs (seg->vx / seg->vy);
	    seg->dy = 1.0 / seg->dx;
	    break;
	}

	/* Compute the row and column limits containing the segment: */

#define FLOOR(A) ((int32) floor(A))

	if (seg->y > seg->next->y) {
	    seg->min_row = FLOOR (seg->next->y);
	    seg->max_row = FLOOR (seg->y);
	} else {
	    seg->min_row = FLOOR (seg->y);
	    seg->max_row = FLOOR (seg->next->y);
	}
	if (seg->x > seg->next->x) {
	    seg->first_col = FLOOR (seg->next->x);
	    seg->last_col = FLOOR (seg->x);
	} else {

	    seg->first_col = FLOOR (seg->x);
	    seg->last_col = FLOOR (seg->next->x);
	}
	if (seg->first_col < poly->min_col)
	    poly->min_col = seg->first_col;
	if (seg->last_col > poly->max_col)
	    poly->max_col = seg->last_col;

	if (seg->min_row < poly->min_row)
	    poly->min_row = seg->min_row;
	if (seg->max_row > poly->max_row)
	    poly->max_row = seg->max_row;

#undef FLOOR

#if 0
	seg->name[0] = poly->num_segs - n + '1';
	if (n == 1)
	    seg->name[1] = poly->segs->name[0];
	else
	    seg->name[1] = seg->name[0] + 1;
	seg->name[2] = '\0';
#endif

    }

    /* Determine the segment's boundary type. The type is used to decide in
       which direction lies the polygon interior. This algorithm compares the
       segments's direction, measured by both slope and vector direction,
       with the overall direction that the boundary wraps to enclose the
       polygon (the change in segment vector direction for one complete wrap
       must be plus or minus 360 degrees). This algorithm works for
       non-convex polygons. */

    for (n = poly->num_segs, seg = poly->segs; n > 0; --n, seg = seg->next) {

	switch (seg->slope) {
	case SLOPE_HORZ:
	    if (seg->vx <= 0.0)
		seg->type = BNDRY_TYPE_BOTTOM_LEFT;
	    else
		seg->type = BNDRY_TYPE_TOP_RIGHT;
	    break;

	case SLOPE_VERT:
	    if (seg->vy <= 0.0)
		seg->type = BNDRY_TYPE_TOP_LEFT;
	    else
		seg->type = BNDRY_TYPE_BOTTOM_RIGHT;
	    break;

	case SLOPE_POS:
	    if (seg->vx <= 0.0)
		seg->type = BNDRY_TYPE_BOTTOM_LEFT;
	    else
		seg->type = BNDRY_TYPE_TOP_RIGHT;
	    break;

	case SLOPE_NEG:
	    if (seg->vx <= 0.0)
		seg->type = BNDRY_TYPE_BOTTOM_RIGHT;
	    else
		seg->type = BNDRY_TYPE_TOP_LEFT;
	    break;
	}

	seg->isect_outside = blisect_outside_tab[seg->type];
    }
}


/*-------------------------------------------------------------------------*/
/* Polygon clipping routine. This is used to compute the insersection of the
   polygon and the corner pixels. We then take the area of this polygon to
   determine its gray value. */

static void
clip_polygon_to_pel (polygon * subj_poly,	/* polygon to clip */
		     double pel_x,	/* pel X coord */
		     double pel_y,	/* pel Y coord */
		     polygon * new_poly	/* resulting polygon */
)
/* Produces a clipped polygon in <new_poly> from <poly>, specified by a list
   of points <poly>. The clipping region is pel_x <= x <= pel_x + 1 and pel_y
   <= y <= pel_y + 1.

This function clips against each of the four pel boundaries in turn,
   successively splicing segments into the subject polygon as needed. We take
   advantage of the following REQUIREd assumptions:

The first point in <subj_poly> is assumed to be inside the pel boundaries.

<new_poly> must be allocated by the caller with room for four more segments
   than <subj_poly>.

<subj_poly> must wrap in the positive direction.

<subj_poly> is convex. Therefore, there can be only one resulting clipped
   polygon. */
{
    poly_segment *seg;		/* current segment ptr */
    poly_segment *new_segs;	/* place to add new segments */
    poly_segment *nseg;		/* next-segment ptr */
    poly_segment *seg1;		/* first segment ptr */
    poly_segment *seg2;		/* secopd segment ptr */
    int32 n;			/* loop index */
    double pel_x_p1;		/* precomputation */
    double pel_y_p1;		/* precomputation */

    /* Validate input: */

    assert (subj_poly->bndry_wraps_pos);

    /* Make sure we have space for 4 segments. This is the most we'll add: */

    assert (new_poly->max_segs - subj_poly->num_segs >= 4);

    /* Simple initializations: */

    pel_x_p1 = pel_x + 1.0;
    pel_y_p1 = pel_y + 1.0;

    /* Initialize output polygon from the subject polygon: */

    new_poly->bndry_wraps_pos = TRUE;
    new_poly->num_segs = 0;
    seg = new_poly->segs;
    seg1 = subj_poly->segs;

    while (1) {

	*seg = *seg1;
	++new_poly->num_segs;

	seg->next = seg + 1;

	seg1 = seg1->next;
	if (seg1 == subj_poly->segs)
	    break;
	seg = seg->next;
    }
    seg->next = new_poly->segs;

    /* Initialize x and y intercepts: */

    for (n = new_poly->num_segs, seg = new_poly->segs;
	    n > 0;
	    --n, seg = seg->next) {

	switch (seg->slope) {
	case SLOPE_HORZ:
	    seg->x1 = seg->x;
	    seg->x2 = seg->next->x;
	    seg->y1 = seg->y2 = seg->y;
	    break;

	case SLOPE_VERT:
	    seg->x1 = seg->x2 = seg->x;
	    seg->y1 = seg->y;
	    seg->y2 = seg->next->y;
	    break;

	default:
	    seg->x1 = seg->x + (pel_y - seg->y) * seg->dx;
	    seg->y1 = seg->y + (pel_x - seg->x) * seg->dy;
	    seg->x2 = seg->x1 + seg->dx;
	    seg->y2 = seg->y1 + seg->dy;
	    break;
	}
    }

    /* Initialize space to add new segments: */

    new_segs = &new_poly->segs[new_poly->num_segs];

#if DEBUG
    if (diag_flags & DIAG_POLY_CLIP) {
	printf ("\nStarting Polygon:\n");
	dump_poly (new_poly);
    }
#endif

    /* TOP EDGE CLIP. */
    /* Find the first left segment: */

    for (seg = new_poly->segs; seg->type & LEFT_BNDRY; seg = seg->next);
    for (; !(seg->type & LEFT_BNDRY); seg = seg->next);

    /* Check for intersections of the top pel line with left boundary
       segments. Use the first one we encounter: */

    seg1 = seg2 = NULL;
    while (seg->type & LEFT_BNDRY) {
	nseg = seg->next;
	if ((seg->y <= pel_y && nseg->y >= pel_y)
		|| (seg->y >= pel_y && nseg->y <= pel_y)) {
	    seg1 = seg;
	    break;
	}
	seg = nseg;
    }
    /* If we have a left intersection, we must have a right intersection.
       Find the last right segment that intersects the top: */

    if (seg1) {
	for (; seg->type & LEFT_BNDRY; seg = seg->next);
	while (!(seg->type & LEFT_BNDRY)) {
	    nseg = seg->next;
	    if ((seg->y <= pel_y && nseg->y >= pel_y)
		    || (seg->y >= pel_y && nseg->y <= pel_y)) {
		seg2 = seg;
	    }
	    seg = nseg;
	}
	assert (seg2);

	/* Make the new segment, and splice it into the polygon: */

	nseg = new_segs++;
	nseg->x = seg1->x1;
	nseg->y = nseg->y1 = nseg->y2 = pel_y;
	nseg->x1 = pel_x;
	nseg->x2 = pel_x_p1;
	nseg->type = BNDRY_TYPE_TOP_LEFT;

#if DEBUG
	nseg->y1_start = nseg->y2;
	nseg->slope = SLOPE_HORZ;
	strcpy (nseg->name, "-T");
#endif

	seg2->x = seg2->x1;
	seg2->y = pel_y;

	seg1->next = nseg;
	nseg->next = seg2;
	new_poly->segs = seg1;

#if DEBUG
	if (diag_flags & DIAG_POLY_CLIP_FINE) {
	    printf ("\nAfter top clipping:\n");
	    printf ("seg1 - seg2:  %2s - %2s\n", seg1->name, seg2->name);
	    dump_poly (new_poly);
	}
#endif
    }
    /* BOTTOM EDGE CLIP. */
    /* Find the first right segment: */

    for (seg = new_poly->segs; !(seg->type & LEFT_BNDRY); seg = seg->next);
    for (; seg->type & LEFT_BNDRY; seg = seg->next);

    /* Check for intersections of the bottom pel line with right boundary
       segments. Use the first one we encounter: */

    seg1 = seg2 = NULL;
    while (!(seg->type & LEFT_BNDRY)) {
	nseg = seg->next;
	if ((seg->y <= pel_y_p1 && nseg->y >= pel_y_p1)
		|| (seg->y >= pel_y_p1 && nseg->y <= pel_y_p1)) {
	    seg1 = seg;
	    break;
	}
	seg = nseg;
    }
    /* If we have a right intersection, we must have a left intersection.
       Find the last left segment that intersects the bottom: */

    if (seg1) {
	for (; !(seg->type & LEFT_BNDRY); seg = seg->next);
	while (seg->type & LEFT_BNDRY) {
	    nseg = seg->next;
	    if ((seg->y <= pel_y_p1 && nseg->y >= pel_y_p1)
		    || (seg->y >= pel_y_p1 && nseg->y <= pel_y_p1)) {
		seg2 = seg;
	    }
	    seg = nseg;
	}
	assert (seg2);

	/* Make the new segment, and splice it into the polygon: */

	nseg = new_segs++;
	nseg->x = seg1->x2;
	nseg->y = nseg->y1 = nseg->y2 = pel_y_p1;
	nseg->x1 = pel_x;
	nseg->x2 = pel_x_p1;
	nseg->type = BNDRY_TYPE_BOTTOM_RIGHT;

#if DEBUG
	nseg->y1_start = nseg->y2;
	nseg->slope = SLOPE_HORZ;
	strcpy (nseg->name, "-B");
#endif

	seg2->x = seg2->x2;
	seg2->y = pel_y_p1;

	seg1->next = nseg;
	nseg->next = seg2;
	new_poly->segs = seg1;

#if DEBUG
	if (diag_flags & DIAG_POLY_CLIP_FINE) {
	    printf ("\nAfter bottom clipping:\n");
	    printf ("seg1 - seg2:  %2s - %2s\n", seg1->name, seg2->name);
	    dump_poly (new_poly);
	}
#endif
    }
    /* RIGHT EDGE CLIP. */
    /* Find the first top segment: */

    for (seg = new_poly->segs; seg->type & TOP_BNDRY; seg = seg->next);
    for (; !(seg->type & TOP_BNDRY); seg = seg->next);

    /* Check for intersections of the right pel line with top boundary
       segments. Use the first one we encounter: */

    seg1 = seg2 = NULL;
    while (seg->type & TOP_BNDRY) {
	nseg = seg->next;
	if ((seg->x <= pel_x_p1 && nseg->x >= pel_x_p1)
		|| (seg->x >= pel_x_p1 && nseg->x <= pel_x_p1)) {
	    seg1 = seg;
	    break;
	}
	seg = nseg;
    }
    /* If we have a top intersection, we must have a bottom intersection.
       Find the last bottom segment that intersects the right: */

    if (seg1) {
	for (; seg->type & TOP_BNDRY; seg = seg->next);
	while (!(seg->type & TOP_BNDRY)) {
	    nseg = seg->next;
	    if ((seg->x <= pel_x_p1 && nseg->x >= pel_x_p1)
		    || (seg->x >= pel_x_p1 && nseg->x <= pel_x_p1)) {
		seg2 = seg;
	    }
	    seg = nseg;
	}
	assert (seg2);

	/* Make the new segment, and splice it into the polygon: */

	nseg = new_segs++;
	nseg->x = nseg->x1 = nseg->x2 = pel_x_p1;
	nseg->y = seg1->y2;
	nseg->y1 = pel_y;
	nseg->y2 = pel_y_p1;
	nseg->type = BNDRY_TYPE_TOP_RIGHT;

#if DEBUG
	nseg->y1_start = nseg->y2;
	nseg->slope = SLOPE_VERT;
	strcpy (nseg->name, "-R");
#endif

	seg2->x = pel_x_p1;
	seg2->y = seg2->y2;

	seg1->next = nseg;
	nseg->next = seg2;
	new_poly->segs = seg1;

#if DEBUG
	if (diag_flags & DIAG_POLY_CLIP_FINE) {
	    printf ("\nAfter right clipping:\n");
	    printf ("seg1 - seg2:  %2s - %2s\n", seg1->name, seg2->name);
	    dump_poly (new_poly);
	}
#endif
    }
    /* LEFT EDGE CLIP. */
    /* Find the first bottom segment: */

    for (seg = new_poly->segs; !(seg->type & TOP_BNDRY); seg = seg->next);
    for (; seg->type & TOP_BNDRY; seg = seg->next);

    /* Check for intersections of the left pel line with bottom boundary
       segments. Use the first one we encounter: */

    seg1 = seg2 = NULL;
    while (!(seg->type & TOP_BNDRY)) {
	nseg = seg->next;
	if ((seg->x <= pel_x && nseg->x >= pel_x)
		|| (seg->x >= pel_x && nseg->x <= pel_x)) {
	    seg1 = seg;
	    break;
	}
	seg = nseg;
    }
    /* If we have a bottom intersection, we must have a top intersection.
       Find the last top segment that intersects the left: */

    if (seg1) {
	for (; !(seg->type & TOP_BNDRY); seg = seg->next);
	while (seg->type & TOP_BNDRY) {
	    nseg = seg->next;
	    if ((seg->x <= pel_x && nseg->x >= pel_x)
		    || (seg->x >= pel_x && nseg->x <= pel_x)) {
		seg2 = seg;
	    }
	    seg = nseg;
	}
	assert (seg2);

	/* Make the new segment, and splice it into the polygon: */

	nseg = new_segs++;
	nseg->x = nseg->x1 = nseg->x2 = pel_x;
	nseg->y = seg1->y1;
	nseg->y1 = pel_y;
	nseg->y2 = pel_y_p1;
	nseg->type = BNDRY_TYPE_TOP_RIGHT;

#if DEBUG
	nseg->y1_start = nseg->y2;
	nseg->slope = SLOPE_VERT;
	strcpy (nseg->name, "-R");
#endif

	seg2->x = pel_x;
	seg2->y = seg2->y1;

	seg1->next = nseg;
	nseg->next = seg2;
	new_poly->segs = seg1;

#if DEBUG
	if (diag_flags & DIAG_POLY_CLIP_FINE) {
	    printf ("\nAfter left clipping:\n");
	    printf ("seg1 - seg2:  %2s - %2s\n", seg1->name, seg2->name);
	    dump_poly (new_poly);
	}
#endif
    }
    /* Fix up the number of segments: */

    n = 0;
    seg = new_poly->segs;
    while (1) {
	++n;
	seg = seg->next;
	if (seg == new_poly->segs)
	    break;
    }

    new_poly->num_segs = n;
}


/*-------------------------------------------------------------------------*/

static double
bndry_fract (const poly_segment * seg,	/* boundary segment */
	     int32 x,		/* pel X value as an integer */
	     double pel_x,	/* integral pel X coordinate */
	     double pel_y	/* integral pel Y coordinate */
)
/* Return the appropriate fraction of the pixel at <pel_x, pel_y> outside the
   boundary segment <seg>. */
{
    unsigned char itype;	/* type of segment/pel intersection */
    double area;		/* return value */
    double pel_x_p1;		/* precomputation */
    double pel_y_p1;		/* precomputation */

    /* Don't use this segment if it doesn't intersect this pel: */

    if (seg->min_col > x
	    || seg->max_col < x)
	return 0.0;

    /* There are six types of intersection. Two divide the pel into
       trapezoids, 4 into triangles. Test for the type of intersection, and
       compute the relevant area. The computations are eased by using the
       knowledge that the pel is a unit square. */

    pel_x_p1 = pel_x + 1.0;
    pel_y_p1 = pel_y + 1.0;

    if (seg->slope == SLOPE_VERT) {
	if (seg->x1 <= pel_x) {
	    area = 0.0;
	    itype = ISECT_NONE;
	} else if (seg->x1 > pel_x_p1) {
	    area = 1.0;
	    itype = ISECT_NONE;
	} else {
	    area = seg->x1 - pel_x;
	    itype = ISECT_TOP_BOTTOM;
	}
    } else if (seg->y1 < pel_y) {

	if (seg->y2 < pel_y) {
	    area = 0.0;
	    itype = ISECT_NONE;
	} else if (seg->y2 < pel_y_p1) {
	    /* top-right intersection, triangular region */
	    area = 0.5 * (1.0 - (seg->x1 - pel_x)) * (seg->y2 - pel_y);
	    itype = ISECT_TOP_RIGHT;
	} else {
	    /* top-bottom intersection, trapezoidal region */
	    area = 0.5 * (seg->x1 + seg->x2) - pel_x;
	    itype = ISECT_TOP_BOTTOM;
	}

    } else if (seg->y1 < pel_y_p1) {

	if (seg->y2 < pel_y) {
	    /* left-top intersection, triangular region */
	    area = 0.5 * (seg->x1 - pel_x) * (seg->y1 - pel_y);
	    itype = ISECT_TOP_LEFT;
	} else if (seg->y2 < pel_y_p1) {
	    /* left-right intersection, trapezoidal region */
	    area = 0.5 * (seg->y1 + seg->y2) - pel_y;
	    itype = ISECT_LEFT_RIGHT;
	} else {
	    /* left-bottom intersection, triangular region */
	    area = 0.5 * (seg->x2 - pel_x) * (1.0 - (seg->y1 - pel_y));
	    itype = ISECT_LEFT_BOTTOM;
	}

    } else {

	if (seg->y2 < pel_y) {
	    /* bottom-top intersection, trapezoidal region */
	    area = 0.5 * (seg->x1 + seg->x2) - pel_x;
	    itype = ISECT_TOP_BOTTOM;
	} else if (seg->y2 < pel_y_p1) {
	    /* bottom-right intersection, triangular region */
	    area = 0.5 * (1.0 - (seg->x2 - pel_x))
		* (1.0 - (seg->y2 - pel_y));
	    itype = ISECT_BOTTOM_RIGHT;
	} else {
	    area = 1.0;
	    itype = ISECT_NONE;
	}
    }
    /* We have computed the area. Depending on the boundary type, invert the
       area (subtract it from 1): */

    if (seg->isect_outside[itype] == -1) {
	printf ("Warning! Invalid bndry/isect combination: btype %d, isect %d\n",
		seg->type, itype);
	area = 0.0;
    } else if (!seg->isect_outside[itype]) {
	area = 1.0 - area;
    }

#if DEBUG
    if (diag_flags & DIAG_FILL) {
	char x1buf[10];
	char x2buf[10];
	char y1buf[10];
	char y2buf[10];

	trunc_form (x1buf, "%6.2f", seg->x1);
	trunc_form (x2buf, "%6.2f", seg->x2);
	trunc_form (y1buf, "%6.2f", seg->y1);
	trunc_form (y2buf, "%6.2f", seg->y2);

	printf ("%31s %3.0f: %2s %6s %6s %6s %6s %2s %6.3f %1d\n",
		"", pel_x, seg->name,
		x1buf, x2buf, y1buf, y2buf,
		isect_type_name_tab[itype],
		area,
		seg->isect_outside[itype]);
    }
#endif

    return area;
}


/*-------------------------------------------------------------------------*/

void
rc_fill_poly (rcWindow& image,	/* image to draw into */
	       int32 num_pts,	/* number of polygon vertices */
	       double *coords,	/* array: x1, y1, x2, y2, ... */
	       uint32 colors	/* colors to use */
)
/* Utility function to do gray-scale filling of a polygon defined by the
   vertices whose coordinates are given in <coords>.

This routine REQUIRES that the points be ordered in circular fashion to
   outline a convex polygon. In otherwords, the centroid of the polygon may
   not be contained in any of the boundary segments. */
{
    polygon_a poly;		/* polygon to fill */
    polygon_a new_poly;		/* 2nd, clipped, polygon */
    poly_segment _stk_segs[MAX_STK_SEGS];	/* array of segments on stack */
    poly_segment *segs = _stk_segs;	/* array of segments */
    poly_segment _new_stk_segs[MAX_STK_SEGS + 4];	/* array of segments on
							   stack */
    poly_segment *new_segs = _new_stk_segs;	/* array of segments */
    int32 num_rows;		/* number of rows containing rectangle */
    int32 num_cols;		/* number of columns containing rect */
    int32 row_incr;		/* address offset between rows */
    uint8 *row;		/* pel row pointer */
    uint8 *pel;		/* pel row pointer */
    int32 row_no;			/* loop index */
    int32 col_no;			/* loop index */
    uint8 fgnd;		/* foreground color */
    uint8 color;	/* pel color to set */
    double bgnd_fract;		/* fraction of background color to use */
    int32 x;			/* current pel X coord */
    int32 y;			/* current pel Y coord */
    double f_x;			/* x as a double */
    double f_y;			/* y as a double */
    int32 col_shift;		/* amount column # is shifted */
    int32 clamped_col1;		/* col1 clamped to image limits */
    poly_segment *seg;		/* ptr to current segment */
    int32 n;			/* loop index */
    int32 max_row;		/* maximum row */
    int32 max_col;		/* maximum column */

    /* Validate input: */

    assert(num_pts >= 3);

    if (num_pts > MAX_STK_SEGS) {
	segs = (poly_segment *) malloc ((int32) (num_pts * sizeof (*segs)));
	new_segs = (poly_segment *) malloc ((int32) ((num_pts + 4)
						      * sizeof (*new_segs)));
    }
    /* Build the polygon: */

    memset ((char *) new_poly, 1, sizeof (new_poly));
    new_poly->max_segs = num_pts + 4;

    max_row = image.height() - 1;
    max_col = image.width() - 1;

    poly->max_segs = num_pts;
    poly->segs = segs;
    coords_to_poly (num_pts, coords, poly, max_row, max_col);

    /* If the polygon is completely outside the image, exit now: */

    if (poly->min_col > max_col
	    || poly->max_col < 0
	    || poly->min_row > max_row
	    || poly->max_row < 0)
	return;

    /* Clamp the polygon limits to image boundaries: */

    CLAMP (poly->min_row, 0, max_row);
    CLAMP (poly->max_row, 0, max_row);
    CLAMP (poly->min_col, 0, max_col);
    CLAMP (poly->max_col, 0, max_col);

    num_rows = poly->max_row - poly->min_row + 1;
    num_cols = poly->max_col - poly->min_col + 1;

    /* Extract colors: */

    fgnd = (uint8) (colors & 0xFF);

#if DEBUG
    diag_flags = ((colors >> 8) & 0xFF);
#endif

    /* Remember the intensity values at each vertex. This is to avoid
       time-consuming checking in inner loops; instead we blow away the
       values, and blend later, using the cached original values. */

    for (n = poly->num_segs, seg = poly->segs;
	    n > 0;
	    --n, seg = seg->next) {

	if (seg->vertex_in_image)
	    seg->old_vert_int = image.getPixel (seg->ix, seg->iy);
    }

    /* Determine the X coordinate of the intersection of each segment with
       the line <y=min_row> and the Y coordinate of the intersection of the
       segment with the line <x=min_col>. */

    for (n = poly->num_segs, seg = poly->segs; n > 0; --n, seg = seg->next) {

	switch (seg->slope) {
	case SLOPE_HORZ:
	    seg->x2 = poly->min_col;
	    seg->y1_start = seg->y;
	    break;

	case SLOPE_VERT:
	    seg->x2 = seg->x;
	    seg->y1_start = poly->min_row;
	    break;

	default:
	    seg->x2 = seg->x + (poly->min_row - seg->y) * seg->dx;
	    seg->y1_start = seg->y + (poly->min_col - seg->x) * seg->dy;
	    break;
	}
    }

#if DEBUG
    if (diag_flags & DIAG_ENTRY) {
	dump_poly (poly);
    }
#endif

    /* Loop through rows: */

#if DEBUG
    if (diag_flags & DIAG_FILL) {
	printf ("\n%3s: %2s %2s %2s %3s %3s %3s %3s | %3s: %2s %6s %6s %6s %6s %2s %6s %1s\n\n",
		"row", "SB", "LB", "RB", "C1", "C2", "C3", "C4", "col", "nm", "x1", "x2", "y1", "y2", "it", "area", "I");
    }
#endif

    row_incr = image.rowUpdate ();

    for (row_no = num_rows, y = poly->min_row,
	   row = image.rowPointer(y);
	 row_no > 0;
	 --row_no, y += 1, row += row_incr) {

	/* Initialize the row loop state: */

	poly->left_bndry = NULL;
	poly->right_bndry = NULL;
	poly->single_bndry = TRUE;
	poly->col1 = poly->col3 = INT_MAX;
	poly->col2 = poly->col4 = INT_MIN;

	/* Update the state for each boundary segment. This stuff is used to
	   compute X and Y intercepts, and limits for speed: */

	for (n = poly->num_segs, seg = poly->segs;
		n > 0;
		--n, seg = seg->next) {

	    int32 this_col;	/* utility column number */

	    /* Compute the starting X and Y intercepts. Do the minimum
	       updates required for all segments. Later we do more for the
	       segments actually in use.  */

	    seg->x1 = seg->x2;
	    seg->x2 += seg->dx;
	    seg->y2 = seg->y1_start;

	    /* Check if this segment crosses the current row. If not, ignore
	       it: */

	    if (seg->max_row < y || seg->min_row > y) {
		seg->use_me = FALSE;
		continue;
	    }
	    seg->use_me = TRUE;

	    switch (seg->slope) {
	    case SLOPE_VERT:
		seg->min_col = seg->max_col = seg->first_col;
		break;

	    case SLOPE_HORZ:
		poly->single_bndry = FALSE;
		seg->min_col = seg->first_col;
		seg->max_col = seg->last_col;
		break;

	    case SLOPE_POS:
		this_col = (int32) seg->x1;
		seg->min_col = max (this_col, seg->first_col);
		this_col = (int32) seg->x2;
		seg->max_col = min (this_col, seg->last_col);
		break;

	    case SLOPE_NEG:
		this_col = (int32) seg->x2;
		seg->min_col = max (this_col, seg->first_col);
		this_col = (int32) seg->x1;
		seg->max_col = min (this_col, seg->last_col);
		break;
	    }
	    /* Update boundaries and limits. */

	    if (seg->type & LEFT_BNDRY) {
		if (seg->min_col < poly->col1)
		    poly->col1 = seg->min_col;
		if (seg->max_col > poly->col2)
		    poly->col2 = seg->max_col;

		if (poly->single_bndry) {
		    if (poly->left_bndry == NULL)
			poly->left_bndry = seg;
		    else
			poly->single_bndry = FALSE;
		}
	    } else {

		if (seg->min_col < poly->col3)
		    poly->col3 = seg->min_col;
		if (seg->max_col > poly->col4)
		    poly->col4 = seg->max_col;

		if (poly->single_bndry) {
		    if (poly->right_bndry == NULL)
			poly->right_bndry = seg;
		    else
			poly->single_bndry = FALSE;
		}
	    }
	}

	/* Sanity check: make sure we have left and right boundary segments
	   if single_bndry is still set: */

	if (poly->single_bndry
		&& (poly->left_bndry == NULL || poly->right_bndry == NULL)) {
	    printf ("Error! single boundary without both boundaries!\n");
	    poly->single_bndry = FALSE;
	}
	/* Compensate Y intercept if not starting at leftmost column: */

	col_shift = poly->col1 - poly->min_col;
	if (col_shift > 0) {
	    for (n = poly->num_segs, seg = poly->segs;
		    n > 0;
		    --n, seg = seg->next) {

		seg->y2 += col_shift * seg->dy;
	    }
	}

	/* If this row is outside the image, go to next row: */

	if (poly->col4 < 0 || poly->col1 > max_col)
	    continue;

#if DEBUG
	if (diag_flags & DIAG_FILL) {
	    printf ("%3d: %2d %2s %2s %3d %3d %3d %3d\n",
		    y, poly->single_bndry,
		    poly->single_bndry ? poly->left_bndry->name : "--",
		    poly->single_bndry ? poly->right_bndry->name : "--",
		    poly->col1, poly->col2, poly->col3, poly->col4);
	}
#endif

	/* Loop through pels on this row: */

	clamped_col1 = max (0, poly->col1);

	for (x = clamped_col1, pel = row + x,
		col_no = min (max_col, poly->col4) - clamped_col1 + 1;
		col_no > 0;
		--col_no, ++pel, ++x) {

	    /* Compute the Y intercepts of the segments: */

	    for (n = poly->num_segs, seg = poly->segs;
		    n > 0;
		    --n, seg = seg->next) {

		seg->y1 = seg->y2;
		seg->y2 += seg->dy;
	    }

	    /* Check if this pel is completely inside the rectangle. If so,
	       set it to the foreground color and continue with the next pel.
	       For speed, run through all the interior pels in one inner
	       loop, saving add/accumulates. */

	    if (x > poly->col2 && x < poly->col3) {
		int32 ni_cols_m1;	/* # interior cols less 1 */
		int32 n1;		/* loop index */

		ni_cols_m1 = poly->col3 - poly->col2 - 2;
		for (n1 = ni_cols_m1; n1 > 0; --n1, --col_no, ++pel, ++x) {
		    *pel = fgnd;
		}
		/* We're on the last interior pel. Set it, fix up the Y
		   intercepts, and continue: */

		if (ni_cols_m1 > 0) {
		    double f = ni_cols_m1;	/* convert to double only
						   once */

		    for (n1 = poly->num_segs, seg = poly->segs;
			    n1 > 0;
			    --n1, seg = seg->next) {

			seg->y2 += f * seg->dy;
		    }
		}
		*pel = fgnd;
		continue;
	    }
	    /* This pixel is cut by one of the rectangle boundaries. */
	    /* Compute the fraction of the pixel in background, and the
	       resulting color. */

	    f_x = x;
	    f_y = y;

	    if (poly->single_bndry) {
		bgnd_fract = bndry_fract (poly->left_bndry, x, f_x, f_y)
		    + bndry_fract (poly->right_bndry, x, f_x, f_y);
	    } else {

		bgnd_fract = 0.0;

		for (n = poly->num_segs, seg = poly->segs;
			n > 0;
			--n, seg = seg->next) {

		    if (seg->use_me)
			bgnd_fract += bndry_fract (seg, x, f_x, f_y);
		}
	    }

	    /* Clamp the fraction to be in the range 0 to 1, just in case.
	       This can happen if the pel contains two or more segments
	       intersecting at acute angles. In this case, the color chosen
	       is inaccurate. We fix this later by individually setting all
	       pels containing vertices. */

	    if (bgnd_fract < 0.0)
		bgnd_fract = 0.0;
	    else if (bgnd_fract > 1.0)
		bgnd_fract = 1.0;

#if DEBUG
	    if (diag_flags & DIAG_FILL) {
		printf ("%3d: %26s %3d: %33s %6.3f\n",
			y, "", x, "", bgnd_fract);
	    }
#endif

	    /* If the pel is completely outside the polygon, don't set the
	       color: */

	    if (bgnd_fract > 0.998)
		continue;

	    /* The pel is partially or completely inside the polygon. Set the
	       color: */

	    color = (uint8) (fgnd - bgnd_fract * (fgnd - *pel) + 0.5);

	    *pel = color;
	}
    }
    /* The above algorithm does not work correctly for pels containing a
       corner point. We do these explicitly here: */

#if DEBUG
    if (diag_flags & DIAG_POLY_CLIP) {
	printf ("\nPOLYGON VERTEX PEL CLIPPING:\n\n");
    }
#endif

    for (n = poly->num_segs, seg = poly->segs;
	    n > 0;
	    --n, seg = seg->next) {

	double fgnd_fract;	/* pel fraction that is foreground */

	/* Clear out space for the resulting polygon: */

	memset ((char *) new_segs, 1, sizeof (new_segs));
	new_poly->segs = new_segs;

	/* Compute intersecting polygon and its area for each corner pixel in
	   turn: */

	poly->segs = seg;

	if (!seg->vertex_in_image)
	    continue;

	f_x = seg->ix;
	f_y = seg->iy;

	clip_polygon_to_pel (poly, f_x, f_y, new_poly);
	fgnd_fract = fabs (polygon_area (new_poly));

#if DEBUG
	if (diag_flags & DIAG_POLY_CLIP) {
	    printf ("Polygon clipped at pel %d %d:\n", seg->ix, seg->iy);
	    dump_poly (new_poly);
	    printf ("  area @ %d %d = %6.2f\n", seg->ix, seg->iy, fgnd_fract);
	}
#endif

	if (fgnd_fract < 0.0)
	    fgnd_fract = 0.0;
	else if (fgnd_fract > 1.0)
	    fgnd_fract = 1.0;

	if (fgnd_fract < 0.002)
	    continue;

	color = (uint8) (seg->old_vert_int
			   + fgnd_fract * (fgnd - seg->old_vert_int) + 0.5);

	image.setPixel (seg->ix, seg->iy, color);
    }

    /* Clean up and return: */

    if (segs && segs != _stk_segs)
	free (segs);
    if (new_segs && new_segs != _new_stk_segs)
	free (new_segs);
}

