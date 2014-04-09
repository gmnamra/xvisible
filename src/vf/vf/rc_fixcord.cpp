/*
 *
 *$Id $
 *$Log$
 *Revision 1.3  2003/06/10 20:21:12  arman
 *removed prenormalization for now
 *
 *Revision 1.2  2003/04/08 19:18:30  sami
 *Inlining changes
 *
 *Revision 1.1  2002/12/24 17:13:05  arman
 *Open Source Cordic Polar <--> Cartesian Fixed Point Implementation
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include <rc_types.h>



/* Copyright (C) 1981-1999 Ken Turkowski.
 *
 * All rights reserved.
 *
 * Warranty Information
 *  Even though I have reviewed this software, I make no warranty
 *  or representation, either express or implied, with respect to this
 *  software, its quality, accuracy, merchantability, or fitness for a
 *  particular purpose.  As a result, this software is provided "as is,"
 *  and you, its user, are assuming the entire risk as to its quality
 *  and accuracy.
 *
 * This code may be used and freely distributed as long as it includes
 * this copyright notice and the above warranty information.
 */


/* To rotate a vector through an angle of theta, we calculate:
 *
 *	x' = x cos(theta) - y sin(theta)
 *	y' = x sin(theta) + y cos(theta)
 *
 * The rotate() routine performs multiple rotations of the form:
 *
 *	x[i+1] = cos(theta[i]) { x[i] - y[i] tan(theta[i]) }
 *	y[i+1] = cos(theta[i]) { y[i] + x[i] tan(theta[i]) }
 *
 * with the constraint that tan(theta[i]) = pow(2, -i), which can be
 * implemented by shifting. We always shift by either a positive or
 * negative angle, so the convergence has the ringing property. Since the
 * cosine is always positive for positive and negative angles between -90
 * and 90 degrees, a predictable magnitude scaling occurs at each step,
 * and can be compensated for instead at the end of the iterations by a
 * composite scaling of the product of all the cos(theta[i])'s.
 */


//#define FASTER
#define DEGREES 1
//#define RADIANS 1
# define COSCALE 0x11616E8E	/* 291597966 = 0.2715717684432241 * 2^30, valid for j>13 */

static int32 arctantab[] = {

#ifdef DEGREES		/* 16 fractional bits */
# define QUARTER (90L << 16)
# define MAXITER 22	/* the resolution of the arctan table */
	4157273, 2949120, 1740967, 919879, 466945, 234379, 117304, 58666,
	29335, 14668, 7334, 3667, 1833, 917, 458, 229,
	115, 57, 29, 14, 7, 4, 2, 1
#else /* !DEGREES */

# ifdef RADIANS	/* 16 fractional bits */
#  define QUARTER ((int32)(rkPI / 2.0 * (1L << 16)))
#  define MAXITER 16	/* the resolution of the arctan table */
	72558, 51472, 30386, 16055, 8150, 4091, 2047, 1024,
	512, 256, 128, 64, 32, 16, 8, 4,
	2, 1
#  else /* !RADIANS && !DEGREES */

#  define BRADS 1
#  define QUARTER (1L << 30)
#  define MAXITER 29	/* the resolution of the arctan table */
	756808418, 536870912, 316933406, 167458907, 85004756, 42667331,
	21354465, 10679838, 5340245, 2670163, 1335087, 667544, 333772, 166886,
	83443, 41722, 20861, 10430, 5215, 2608, 1304, 652, 326, 163, 81, 41,
	20, 10, 5, 3, 1
# endif /* !RADIANS && !DEGREES */
#endif /* !DEGREES */

};


typedef int32 TFract;	/* f * 2^30 */
inline TFract FFracMul(TFract a, TFract b);


static void PseudoRotate(int32 *px, int32 *py, register int32 theta)
{
	register int i;
	register int32 x, y, xtemp;
	register int32 *arctanptr;

	x = *px;
	y = *py;

	/* Get angle between -90 and 90 degrees */
	while (theta < -QUARTER) {
		x = -x;
		y = -y;
		theta += 2 * QUARTER;
	}
	while (theta > QUARTER) {
		x = -x;
		y = -y;
		theta -= 2 * QUARTER;
	}

	/* Initial pseudorotation, with left shift */
	arctanptr = arctantab;
	if (theta < 0) {
		xtemp = x + (y << 1);
		y     = y - (x << 1);
		x     = xtemp;
		theta += *arctanptr++;
	}
	else {
		xtemp = x - (y << 1);
		y     = y + (x << 1);
		x     = xtemp;
		theta -= *arctanptr++;
	}

	/* Subsequent pseudorotations, with right shifts */
	for (i = 0; i <= MAXITER; i++) {
		if (theta < 0) {
			xtemp = x + (y >> i);
			y     = y - (x >> i);
			x     = xtemp;
			theta += *arctanptr++;
		}
		else {
			xtemp = x - (y >> i);
			y     = y + (x >> i);
			x     = xtemp;
			theta -= *arctanptr++;
		}
	}

	*px = x;
	*py = y;
}


static void PseudoPolarize(int32 *argx, int32 *argy)
{
	register int32 theta;
	register int32 yi, i;
	register int32 x, y;
	register int32 *arctanptr;

	x = *argx;
	y = *argy;

	/* Get the vector into the right half plane */
	theta = 0;
	if (x < 0) {
		x = -x;
		y = -y;
		theta = 2 * QUARTER;
	}

	if (y > 0)
		theta = - theta;
	
	arctanptr = arctantab;
	if (y < 0) {	/* Rotate positive */
		yi = y + (x << 1);
		x  = x - (y << 1);
		y  = yi;
		theta -= *arctanptr++;	/* Subtract angle */
	}
	else {		/* Rotate negative */
		yi = y - (x << 1);
		x  = x + (y << 1);
		y  = yi;
		theta += *arctanptr++;	/* Add angle */
	}

	for (i = 0; i <= MAXITER; i++) {
		if (y < 0) {	/* Rotate positive */
			yi = y + (x >> i);
			x  = x - (y >> i);
			y  = yi;
			theta -= *arctanptr++;
		}
		else {		/* Rotate negative */
			yi = y - (x >> i);
			x  = x + (y >> i);
			y  = yi;
			theta += *arctanptr++;
		}
	}

	*argx = x;
	*argy = theta;
}


#ifndef FASTER
/* FxPreNorm() block normalizes the arguments to a magnitude suitable for
 * CORDIC pseudorotations.  The returned value is the block exponent.
 */
static int
FxPreNorm(int32 *argx, int32 *argy)
{
	register int32 x, y;
	int signx, signy;
	register int shiftexp;

	shiftexp = 0;		/* Block normalization exponent */
	signx = signy = 1;

	if ((x = *argx) < 0) {
		x = -x;
		signx = -signx;
	}
	if ((y = *argy) < 0) {
		y = -y;
		signy = -signy;
	}
	/* Prenormalize vector for maximum precision */
	if (x < y) {	/* |y| > |x| */
		while (y < (1 << 27)) {
			x <<= 1;
			y <<= 1;
			shiftexp--;
		}
		while (y > (1 << 28)) {
			x >>= 1;
			y >>= 1;
			shiftexp++;
		}
	}
	else {		/* |x| > |y| */
		while (x < (1 << 27)) {
			x <<= 1;
			y <<= 1;
			shiftexp--;
		}
		while (x > (1 << 28)) {
			x >>= 1;
			y >>= 1;
			shiftexp++;
		}
	}

	*argx = (signx < 0) ? -x : x;
	*argy = (signy < 0) ? -y : y;
	return(shiftexp);
}
#endif FASTER


/* Return a unit vector corresponding to theta.
 * sin and cos are fixed-point fractions.
 */
void rfFxUnitVec(int32 *cos, int32 *sin, int32 theta)
{
	*cos = COSCALE;
	*sin = 0;
	PseudoRotate(cos, sin, theta);
}


/* Fxrotate() rotates the vector (argx, argy) by the angle theta. */
void rfFxRotate(int32 *argx, int32 *argy, int32 theta)
{
#ifndef FASTER
	int shiftexp;
#endif FASTER

	if (((*argx == 0) && (*argy == 0)) || (theta == 0))
		return;

#ifndef FASTER
	shiftexp = FxPreNorm(argx, argy);  /* Prenormalize vector */
#endif FASTER
	PseudoRotate(argx, argy, theta);   /* Perform CORDIC pseudorotation */
	*argx = FFracMul(*argx, COSCALE);	/* Compensate for CORDIC enlargement */
	*argy = FFracMul(*argy, COSCALE);
#ifndef FASTER
	if (shiftexp < 0) {		/* Denormalize vector */
		*argx >>= -shiftexp;
		*argy >>= -shiftexp;
	}
	else {
		*argx <<= shiftexp;
		*argy <<= shiftexp;
	}
#endif FASTER
}


int32 rfFxAtan2(int32 x, int32 y)
{
	if ((x == 0) && (y == 0))
		return(0);
#ifndef FASTER
	FxPreNorm(&x, &y);	/* Prenormalize vector for maximum precision */
#endif FASTER
	PseudoPolarize(&x, &y);	/* Convert to polar coordinates */
	return(y);
}


void rfFxPolarize(int32 *argx, int32 *argy)
{
#ifndef FASTER
	int shiftexp;
#endif FASTER

	if ((*argx == 0) && (*argy == 0)) {
		return;
	}

#ifndef FASTER
	/* Prenormalize vector for maximum precision */
	shiftexp = FxPreNorm(argx, argy);
#endif FASTER

	/* Perform CORDIC conversion to polar coordinates */
	PseudoPolarize(argx, argy);

	/* Scale radius to undo pseudorotation enlargement factor */
	*argx = FFracMul(*argx, COSCALE);

#ifndef FASTER
	/* Denormalize radius */
	*argx = (shiftexp < 0) ? (*argx >> -shiftexp) : (*argx << shiftexp);
#endif FASTER
}


inline int32 FFracMul(int32 a, int32 b)	     
{
	/* This routine should be written in assembler to calculate the
	 * high part of the product, i.e. the product when the operands
	 * are considered fractions.
	 */
	return((a >> 15) * (b >> 15));
}


