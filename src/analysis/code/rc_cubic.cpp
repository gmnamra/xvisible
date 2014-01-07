
/* This is an implementation of subpixel interpolation to create images
   that are shifted by a non-integer number of pixels.  For the interpolation,
   it uses cubic convolution interpolation, as described in the following
   article:
   "Cubic Convolution Interpolation for Digital Image Processing"
   by Robert G. Keys.  IEEE Transactions on Acoustics, Speech, and Signal
   Processing, Vol. ASSP-29, No. 6, December 1981.  pp. 1153-1160.
   */

#include <rc_math.h>
#include <rc_window.h>


typedef struct
{
    int32 Lower;
    int32 Upper;
} limits_t;

typedef struct
{
    limits_t XLimits;
    limits_t YLimits;
} kernel_limits_t;


#define NEW_PIXEL(x) \
    {                                                   \
	 TargetRow[(x)] =                               \
	        Clip[                                   \
		rfRound(                              \
		    kernel[0] * PixelPtr0[0] +     \
		    kernel[4] * PixelPtr0[1] +     \
		    kernel[8] * PixelPtr0[2] +     \
		    kernel[12] * PixelPtr0[3] +    \
		    kernel[1] * PixelPtr1[0] +     \
		    kernel[5] * PixelPtr1[1] +     \
		    kernel[9] * PixelPtr1[2] +     \
		    kernel[13] * PixelPtr1[3] +    \
		    kernel[2] * PixelPtr2[0] +     \
		    kernel[6] * PixelPtr2[1] +     \
		    kernel[10] * PixelPtr2[2] +    \
		    kernel[14] * PixelPtr2[3] +    \
		    kernel[3] * PixelPtr3[0] +     \
		    kernel[7] * PixelPtr3[1] +     \
		    kernel[11] * PixelPtr3[2] +    \
		    kernel[15] * PixelPtr3[3],x		 \
		    )];                                  \
	 }


static void createKernelCC( double* kernel, double XShift, double YShift,
				kernel_limits_t * Limits);
static double CCCoeff( double FracShift, int32 Offset );
static void CalcLimitsCC( double Shift, limits_t * Limits );

/* Always allocate for now */
/* Only handle 8bit images for now */

rcWindow rfCubicShiftImage(rcWindow& Image,
			 double XShift, double YShift)
{
    kernel_limits_t Limits;
    int32 SrcXLower, DstXLower;
    int32 SrcYLower, DstYLower;
    int32 OldXCorner, OldYCorner;
    int16 Width, Height;
    double kernel[16];
    uint8 * TargetRow;
    int32 x, y;
    rcWindow Shifted;
    register uint8 * PixelPtr0, *PixelPtr1, *PixelPtr2, *PixelPtr3;

    static uint8 ClipBuf[3 * 256];
    static uint8 * Clip = ClipBuf + 256;
    static int32 Initialized = FALSE;
    
    if ( !Initialized )
    {
	int32 i;
	
	/* Cubic convolution might return a pixel value as low as
	   -71.72 or as high as 326.72 (I think).  Instead of using
	   a two-part "if" statement, however, I'll use an
	   array for speed.
	   I'll also initialize a larger array, just in case. */
	for ( i = -255 ; i < 0 ; i++ )
	    Clip[i] = 0;
	for ( i = 0 ; i < 256 ; i++ )
	    Clip[i] = i;
	for ( i = 256 ; i < 512 ; i++ )
	    Clip[i] = 255;
	
	Initialized = TRUE;
    }

    /*  Src         Dst
       Negative Shifts:
       (x0-xshift,y0-yshift)  --> (0,0)
       positive Shifts:
       (x0,y0)  --> (x0+xshift,y0+xshift)

       The dst x0,y0 is defined to be 0,0 in both pos and neg
       shift cases. The client coords is updated with the shifts.

    */
    
    if ( XShift >= 0 )
    {
      SrcXLower = 0;
      DstXLower = (int32) XShift;
    }
    else
    {
      SrcXLower = - (int32) XShift;
      DstXLower = 0;
    }
    
    if ( YShift >= 0 )
    {
      SrcYLower = 0;
      DstYLower = (int32) YShift;
    }
    else
    {
      SrcYLower = - (int32) YShift;
      DstYLower = 0;
    }
    
    Width  = Image.width() - abs((int32)XShift);
    Height = Image.height() - abs((int32)YShift);
    assert(Width > 0);
    assert(Height > 0);

    // If no shift is necessary just return the src
    if ( (XShift == (int32) XShift) && (YShift == (int32) YShift) )
    {
      return Image;
    }
    
    
    createKernelCC(kernel, XShift, YShift, &Limits );
    
    /*
      for ( xx = 0 ; xx < 4 ; xx++ )
      {	
        for ( yy = 0 ; yy < 4 ; yy++ )
           printf("%f ", (double)kernel[xx *4 + yy]);
         putc('\n', stdout);
      }
    */  
    /* Determine the region within the original image that will be iterated
       over to produce the shifted image.  There are two issues that need
       to be considered beyond those considered in the integer case.

       Get rid of the edges.  This means decreasing the
       boundaries based on the kernel.

       The cubic convolution kernel is 4x4, decreasing the size in each
       dimension by 3 (1 on one side, 2 on the far side) 

       However, now to compute shifts that
       contain pixel fractions we need to do the above convolution.
       Customary erosion of the input image region by the 4x4 kernel.

       Src         Dst
       Negative Shifts:
       (x0-xshift+1,y0-yshift+1)  --> (1,1)
       src region is eroded by 1 to allow the kernel

       positive Shifts:
       (x0+1,y0+1)  --> (x0+xshift+1,y0+xshift+1)
       
    */
    Width  -= 3;
    Height -= 3;

    /* Adjust the limits based on kernel. Since the size of the resultant
       image is just the valid region
       */
    SrcXLower += abs(Limits.XLimits.Lower);
    SrcYLower += abs(Limits.YLimits.Lower);
    DstXLower += abs(Limits.XLimits.Lower);
    DstYLower += abs(Limits.YLimits.Lower);

    /* Create Dst image 
     * Dst (0,0) is src (DstXLower,DstYLower)
     */
    Shifted = rcWindow (Width,Height);

    /* Determine value for each interpolated (destination) pixel */
    OldYCorner = SrcYLower + Limits.YLimits.Lower;
    for ( y = 0 ; y < Height ; y++, OldYCorner++ )
    {

      TargetRow = Shifted.pelPointer (DstXLower,y);

      OldXCorner = SrcXLower + Limits.XLimits.Lower;
      
      PixelPtr0 = Image.pelPointer(OldXCorner, OldYCorner);
      PixelPtr1 = Image.pelPointer(OldXCorner, OldYCorner+1);
      PixelPtr2 = Image.pelPointer(OldXCorner, OldYCorner+2);
      PixelPtr3 = Image.pelPointer(OldXCorner, OldYCorner+3);

      for ( x = 0 ;
	   x < Width ;
	   x++, PixelPtr0++, PixelPtr1++, PixelPtr2++, PixelPtr3++ )
	NEW_PIXEL(x);
    }

    return Shifted;
    
}


/* Although a kernel is, conceptually, a 2D array, implement it as a 1D array
   since the offset calculations are strange anyway, given the variable
   limits */
void createKernelCC( double* kernel, double XShift, double YShift,
			  kernel_limits_t * Limits)
{
    int32 xs, ys;
    
    assert( Limits != NULL );

    /* The cubic-convolution kernel is 4 x 4 */

    XShift = -XShift;
    YShift = -YShift;
    
    CalcLimitsCC( XShift, &Limits->XLimits );
    CalcLimitsCC( YShift, &Limits->YLimits );

    assert( Limits->XLimits.Upper - Limits->XLimits.Lower + 1 == 4 );
    assert( Limits->YLimits.Upper - Limits->YLimits.Lower + 1 == 4 );

    for ( xs = 0 ; xs < 4 ; xs++ )
	for ( ys = 0 ; ys < 4 ; ys++ )
	    kernel[xs * 4 + ys] =
		CCCoeff( XShift - (int32) XShift, xs + Limits->XLimits.Lower ) *
		CCCoeff( YShift - (int32) YShift, ys + Limits->YLimits.Lower );

}


double CCCoeff( double FracShift, int32 Offset )
{
    double s, s2, s3;
    
    /* If the fractional shift is negative, mirror it and the offset to
       make them positive. */
    if ( FracShift < 0 )
    {
	FracShift = - FracShift;
	Offset    = - Offset;
    }

    assert( FracShift < 1.0 );
    assert( (Offset >= -1) && (Offset <= 2) );

    /* Let's not get fancy and efficient.  The two formulas come from
       the article, eqn (15).

       s is the distance from the center of the new pixel to the center
       of the old pixel specified by Offset. */

    s = fabs( Offset - FracShift );
    s2 = s * s;
    s3 = s * s2;

    switch ( Offset )
    {
    case 2:
    case -1: /* 1 < s < 2 */
	return -0.5 * s3 + 2.5 * s2 - 4 * s + 2;

    case 0:
    case 1:  /* 0 < s < 1 */
	return 1.5 * s3 - 2.5 * s2 + 1;
    }

    /* If you like, note the special cases:
       s = 0      ==> 1
       s = 1 or 2 ==> 0
       */

    return (0.0);  /* prevent compiler from complaining about no return value */
}

    
    
void CalcLimitsCC( double Shift, limits_t * Limits )
{
    assert( Limits != NULL );
        
    /* Only the fractional part of the shift matters for the kernel.
       Clearly, the integral part of the shift just determines which
       pixels will be sampled.  The fractional part determines the
       weightings.  Keep the sign of the shift. */


    /* The requested shift may be positive or negative.  Which four pixels
       are chosen for sampling depends on the sign of the shift.
       (   -1, 0 )   ==> s = -2, -1,  0, +1
       [    0, 1 )   ==> s = -1,  0, +1, +2
       
       Note that, ordinarily, you'd think that zero was a problem, since
       you have no real basis for choosing -1..2 or -2..1.  However, CC
       has the nice property that coefficients vanish at integral nonzero
       shifts, so it doesn't matter.
       */

    if ( Shift - (int32) Shift >= 0 )
    {
	Limits->Lower = -1;
	Limits->Upper = +2;
    } else {
	Limits->Lower = -2;
	Limits->Upper = +1;
    }

    return;
}
