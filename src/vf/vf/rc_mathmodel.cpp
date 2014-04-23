/*
 *
 *$Id $
 *$Log$
 *Revision 1.8  2005/08/30 20:57:16  arman
 *Cell Lineage
 *
 *Revision 1.8  2005/07/21 22:09:30  arman
 *incremental
 *
 *Revision 1.7  2005/03/29 23:33:18  arman
 *hack for now for tangent field polarity
 *
 *Revision 1.6  2005/03/25 22:04:22  arman
 *added tangent field
 *
 *Revision 1.5  2003/12/05 20:38:42  arman
 *added cone
 *
 *Revision 1.4  2003/12/01 10:32:25  arman
 *bug fix
 *
 *Revision 1.3  2003/11/15 23:16:06  arman
 *added laplacian of Gaussian
 *
 *Revision 1.2  2003/11/15 22:21:13  arman
 *added difference of gaussian
 *
 *Revision 1.1  2003/11/14 19:38:56  arman
 *Mathematically generated images
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "rc_mathmodel.h"
#include "rc_vector2d.h"

#define MAIN \
x = (double)(h - nw) / nw;\
y = (double)(v - nh) / nh;\
R = rmSquare (x) + rmSquare (y);\
E = - (R)  * st;\
G = exp(E);\
G *= sp;\
switch (d)\
{\
case eGabor:\
S = cos( rk2PI * f * (x * cos(mOrientation) - y * sin(mOrientation)) + mPhase.Double());\
S *= mAmplitude;\
L = (S * G + 1) / 2;\
break;\
case eGauss:\
L = G;\
break;\
case eDoGauss:\
rmAssert (rsp >= 0.0);\
rmAssert (rst >= 0.0);\
G2 = exp( - (rmSquare (x) + rmSquare (y))  * rst);\
G2 *= rsp;\
L = G - G2;\
break;\
case eLoGauss:\
G2 = exp(E);\
G2 *= (1 + E);\
G2 *= sl;\
L = G2;\
break;\
case eCone:\
L = 1 / sqrt (1 + R);\
break;\
default:\
L = G;\
break;\
}

void rcMathematicalImage::mMakeImage (rcWindow& frame, distribution d, polarity p)
{
    int32    v, h;
    double  x, y;
    double  f, st, sp, sl, rst (-1.0), rsp (-1.0);
    double  E, S, G, G2, L, R;
    int32 nw = frame.width() / 2;
    int32 nh = frame.height() / 2;
    double minV (rcDBL_MAX);
    double maxV (rcDBL_MIN);
    
    /* pre-calculation */
    f = frame.width() / mFrequency;
    st = 1.0 / (2.0 * rmSquare (mStd));
    sp = 1.0 / (sqrt (rk2PI) * mStd);
    sl = -1.0 / (rkPI * pow (mStd, 4));
    if (mRatio > 0.001)
    {
        rst = 1.0 / (2.0 * rmSquare (mStd * mRatio));
        rsp = 1.0 / (sqrt (rk2PI) * mStd * mRatio);
    }
    
    for (h = 0; h < frame.width(); h++)
    {
        for (v = 0; v < frame.height(); v++)
        {
            
            MAIN;
            
            if (L < minV) minV = L;
            if (L > maxV) maxV = L;
            
        }
    }
    
    uint32 umax = (frame.depth() == rcPixel8) ? rcUINT8_MAX :
    (frame.depth() == rcPixel16) ? rcUINT16_MAX :
    (frame.depth() == rcPixel32S) ? rcUINT32_MAX : 0;
    rmAssert (umax);
    double scale = ((double) umax) / (maxV - minV);
    
    // Now we know the min and Max and map it for the integer types and copy directly for the double
    
    
    for (h = 0; h < frame.width(); h++)
    {
        for (v = 0; v < frame.height(); v++)
        {
            MAIN;
            L -= minV;
            L *= scale;
            uint32 pel = (p == eLowPeak) ? (umax - (uint32) L) : ((uint32) L);
            
            frame.setPixel (h, v, pel);
            
        }
    }
}


void rcMathematicalImage::mMakeField (rcWindow& frame, distribution d, float dr)
{
    int32 cX = frame.width() / 2;
    int32 cY = frame.height() / 2;
    rcPixel depth = frame.depth();
    rc2Fvector center ((float) cX, (float) cY);
    
    for (int32 j = 0; j < frame.height(); j++)
        for (int32 i = 0; i < frame.width(); i++)
        {
            rc2Fvector p ((float) i, (float) j);
            p = p - center;
            if (p.isNull() || p.l2() < dr)
            {
                frame.setPixel (i, j, 0);	  
                continue;
            }
            
            rcRadian rd = arctan ((double) p.y(), (double) -p.x());
            
            switch (depth)
            {
                case rcPixel8:
                {
                    rcAngle8 d (rd);
                    frame.setPixel (i, j, 255 - d.basic());
                    break;
                }                    
                case rcPixel16:
                {                    
                    rcAngle16 d16 (rd);
                    frame.setPixel (i, j, d16.basic());
                    break;
                }                    
                case rcPixel32S:
                {                    
                    rcAngle16 dff (rd);
                    frame.setPixel (i, j, (int32)dff.basic());
                    break;
                }
                case rcPixelFloat:
                    frame.setPixel (i, j, (float) rd.basic());
                    break;
                    
                case rcPixelDouble:
                    frame.setPixel (i, j, rd.basic());
                    break;
                    
                default:
                    rmAssert (0);
            }
        }
}





