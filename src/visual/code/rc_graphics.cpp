/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_graphics.cpp 7247 2011-02-25 17:14:49Z arman $
 *
 * Implementation for scalable graphics objects
 *
 *****************************************************************************/

#include <rc_graphics.h>
#include <rc_edge.h>
#include <rc_histstats.h>
//
// Class rcVisualSegment implementation
//

rcVisualSegment::rcVisualSegment() :
        mP1( 0.0f, 0.0f ),
        mP2( 0.0f, 0.0f ),
        mType( eUnknown )
{
}

rcVisualSegment::rcVisualSegment( rcVisualSegmentType type, const rc2Fvector& pt1, const rc2Fvector& pt2 ) :
    mP1( pt1 ),
    mP2( pt2 ),
    mType( type )
{
}

rcVisualSegment::~rcVisualSegment()
{
    //mType = eUnknown;
}


// Computes the 8-connected direction from the 8 bit angle
// @todo support 16bit and true peak
static inline int32 getAxis (uint8 val)
{
  return ((val + 16) / 32) % 8;
}

static rcGradientDir<float> sGr;

void rfGradientImageToVisualSegment (const rcWindow& peaks, const rcWindow& angle,  rcVisualSegmentCollection& v)
{
  rmAssert (peaks.size() == angle.size());
  for (int32 j = 2; j < angle.height () - 2; j++)
    {
      const uint8 *mag = peaks.pelPointer(2, j);  // 5x5 processing
      const uint8 *ang = angle.pelPointer(2, j);
      for (int32 i = 2; i < angle.width () - 2; i++, mag++, ang++)
	{
	  if (! (*mag) ) continue;
	  rcAngle8 d (*ang);
	  rc2Fvector p1 ((float) i + 0.5f, (float) j + 0.5f);
	  rc2Fvector p2 = p1 + rc2Fvector (1.0f, rcRadian (d));
	  rcVisualArrow arrow( p1, p2);
	  v.push_back( arrow );
	}
    }
}


void rfImageGradientToVisualSegment (const rcWindow& image, rcVisualSegmentCollection& v)
{
    rcWindow mag (image.width(),image.height(), image.depth());
    rcWindow ang (image.width(),image.height(), image.depth());
    rcWindow peaks (image.width(),image.height(), image.depth());    
    rfSobel (image, mag, ang, false);
    rcWindow pwin (peaks, 1, 1, image.width()-2, image.height()-2);
		rcHistoStats h (pwin);
    rfSpatialEdge (mag, ang, pwin, (int) h.mean(), false);
    rfGradientImageToVisualSegment (peaks, ang, v);
}




	  
