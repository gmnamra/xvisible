
#include <rc_types.h>
#include <rc_window.h>
#include <rc_edge.h>
#include <rc_stats.h>
#include <rc_fileutils.h>
#include <rc_ipconvert.h>

class rcEdgeTables
{
 public:
  rcEdgeTables ();
  ~rcEdgeTables () { }

  enum 
    {
      eAtanPrecision = 16,
      eAtanRange = (1 << eAtanPrecision), 
      eMagPrecision = 8,
      eMagTableSize = (1 << (eMagPrecision+eMagPrecision)),
      eMagRange = (1 << eMagPrecision),
      eMaxMag = (eMagRange - 1)
    };


  template<class T>
  const T * angleTable (T) const
  {
    rmAssert (0);
  }


  const uint8 * magnitudeTable () const { return &mMagTable[0]; }

  template<class T>
  T binAtan (int32 y, int32 x, T) const;

 private:
  double mMagScale;
  vector<uint8> mThetaTable;
  vector<uint16> mTheta16Table;
  vector<uint8> mMagTable;
};

rcEdgeTables sEdgeTables;

template<class A, class L>
void sobelEdgeProcess (A* angle, const L* imagePtr, const rcWindow& image, rcWindow& magnitudes, rcWindow& angles)
{
  int32 magUpdate = magnitudes.rowPixelUpdate();
  int32 angleUpdate = angles.rowPixelUpdate();
  int32 srcUpdate = image.rowPixelUpdate();
  L const *src0 = imagePtr;
  uint8 *mag = magnitudes.rowPointer(0);

  int32 h(angles.height());
  int32 width(angles.width());

  static const A dummy (0);
  // For 8 bit images, we divide by 8 (4+4). For 16 bit images, we further divide by 256
  // This could be parameterized as pixel depth is often 12 or 14 bits and not full 16 bits

const int32 normBits = (image.bits() == 16) ? 6+3 : 3;

  const uint8 *magTable = sEdgeTables.magnitudeTable ();
  
  // Sobel kernels are symertic. We use a L C R roll over algorithm
  
  do
  {
    L const *src = src0;
    uint8 *m = mag;
    A*a = angle;
    
    int32 left (src[0] + 2*src[srcUpdate] + src[2*srcUpdate]);
    int32 yKernelLeft (-src[0] + src[2*srcUpdate]);
    src++;
    int32 center (src[0] + 2*src[srcUpdate] + src[2*srcUpdate]);
    int32 yKernelCenter (-src[0] + src[2*srcUpdate]);
    src++;
    int32 w(width);
    
    do
    {
      int32 right(src[0] + 2*src[srcUpdate] + src[2*srcUpdate]);
      int32 yKernelRight(-src[0] + src[2*srcUpdate]);
     
      // Calculate x and y magnitudes
      left = right - left;
      int32 product = yKernelLeft + 2*yKernelCenter + yKernelRight;
      uint8 x = (uint8) (rmABS (left) >> normBits);
      uint8 y = (uint8) (rmABS(product) >> normBits);
      uint32 index = (x << rcEdgeTables::eMagPrecision) | y;
  
      if (index >= rcEdgeTables::eMagTableSize)
	{
	  cerr << index << (int32) x << "," << (int32) y << "," << w <<  "," << h << "," << right << "," << left << endl;
	}
      rmAssertDebug(index < rcEdgeTables::eMagTableSize);
      
      *m = magTable [index]; 
      m++;

      // Angle Table 
      // if both x and y gradient are less than 5 (1 gray level edge)
      // then set the angle to zero.

      // @note add angle threshold
      //      if (rmABS(product) < 3 && rmABS(left) < 3)
      //	*a++ = dummy;
      //      else
	*a++ = sEdgeTables.binAtan (product, left, dummy);
      
      left = center;
      center = right;
      yKernelLeft = yKernelCenter;
      yKernelCenter = yKernelRight;
      src++;
    }while(--w);
    mag += magUpdate;
    angle += angleUpdate;
    src0 += srcUpdate;
  }while(--h);
  
}

/*
 * TruePeak - This is a 3 pixel operator that selects the middle pixel
 * if it is a peak. This pixel is deemed a peak if either
 * a) it is greater than both its neighbours
 * b) it is greater than a neighbour on the "black" side of the image
 *    and equal to the neighbour on the other side
 *
 * This is fairly inefficient implementation. 
 */

template<>  const uint8 * rcEdgeTables::angleTable(uint8) const
{
  return &mThetaTable[0];
}

template<>  const uint16 * rcEdgeTables::angleTable(uint16) const
{
  return &mTheta16Table[0];
}


// Max Magnitude from one of the kernels is ([1 2 1] * 255) / 4 or 255
// We are using 7bits for magnitude and 16 bits for precision for angle. 
// @note really wasteful. Have to design later. 

rcEdgeTables::rcEdgeTables ()
  :  mMagScale (255 / 256.)
{
  int32 x, y, mapped;
  double realVal;
  mMagTable.resize (eMagTableSize);
  mThetaTable.resize (eAtanRange + 1);
  mTheta16Table.resize (eAtanRange + 1);

   // Magnitude Table: Linear Scale. 
   uint8 *atIndex = &mMagTable [0];
   for(x = 0; x <= eMaxMag; x++)
     for(y = 0; y <= eMaxMag; y++)
      {
	realVal = sqrt (double (rmSquare (y) + rmSquare (x)));
	mapped = (int32) (rmMin(realVal * mMagScale, 255.0) + 0.5);
	*atIndex++ = uint8 (mapped);
      }

   for (int32 i = 0; i <= eAtanRange; ++i)
    {
      rcRadian rd (atan((double) i / eAtanRange));
      rcAngle8 r8 (rd);
      rcAngle16 r16 (rd);

      mThetaTable[i] = r8.basic ();
      mTheta16Table[i] = r16.basic ();
    }

}

template<class T>
inline T rcEdgeTables::binAtan (int y, int x, T) const
{
  static T mPIover2  (1 << ((sizeof(T)*8)-2)), mPI (1 << ((sizeof(T)*8)-1)), m3PIover2 (mPIover2 + mPI);
  static int32 precision (eAtanPrecision);

  const T *ThetaTable = angleTable (mPI);

  if (y < 0)
    {
      y = -y;
      if (x >= 0)
	{
	  if (y <= x) return       - ThetaTable[(y << precision) / x];
	  else	      return ( m3PIover2 + ThetaTable[(x << precision) / y]);
	}
      else
	{
	  x = -x;
	  if (y <= x) return ( mPI + ThetaTable[(y << precision) / x]);
	  else	      return ( m3PIover2 - ThetaTable[(x << precision) / y]);
	}
    }
    
  else
    {
      if (x >= 0)
	{
	  if (x == 0 && y == 0) return 0;
	  if (y <= x) return 	              ThetaTable [(y << precision) / x];
	  else	      return (mPIover2 - ThetaTable[(x << precision) / y]);
	}
      else
	{
	  x = -x;
	  if (y <= x) return (mPI  - ThetaTable[(y << precision) / x]);
	  else	      return (mPIover2  + ThetaTable[(x << precision) / y]);
	}
    }
}




// Computes the 8-connected direction from the 8 bit angle
// @todo support 16bit and true peak
static inline int32 getAxis (uint8 val)
{
  return ((val + (1 << (8 - 4))) >> (8 - 3)) % 8;
}

static inline int32 getAngle16Axis (uint16 val)
{
  return ((val + (1 << (16 - 4))) >> (16 - 3)) % 8;
}


uint32 rfSpatialEdge (rcWindow& magImage, rcWindow& angleImage, 
			rcWindow& peaks, uint8 threshold, bool angleLabeled)
{
  assert(magImage.width () > 0);
  assert(angleImage.height () > 0);

  int32 width(magImage.width());
  int32 height(angleImage.height());
  int32 magUpdate (magImage.rowUpdate());

  // Peak detection is a 3x3 processing
  rcWindow zeroc;
  if (peaks.isBound() && (peaks.width() == width - 2) && (peaks.height() == height - 2))
    zeroc = peaks;
  else
    zeroc = rcWindow (width - 2, height - 2);

  zeroc.setAllPixels (uint32 (0));
  
  uint32 numPeaks = 0;
  
  int32 x,y;
  
  switch (angleImage.depth()) 
    {
    case rcPixel8:
  // Main part  
  for(y = 0; y < zeroc.height(); y++)
  {
    const uint8 *mag = magImage.rowPointer(y+1);  // 3x3 processing
    const uint8 *ang = angleImage.rowPointer(y+1);
    mag+=1;
    ang+=1;
    
    uint8 *dest = zeroc.rowPointer(y);
    
    for(x = 0; x < zeroc.width(); x++,mag++,ang++,dest++)
    {
      if(*mag >= threshold)
      {
        uint8 m1,m2;
        
        int32 angle = getAxis(*ang);
        switch(angle)
        {
          case 0:
          m1 = *(mag - 1);
          m2 = *(mag + 1);
          break;

          case 1:
          m1 = *(mag - 1 - magUpdate);
          m2 = *(mag + 1 + magUpdate);
          break;

          case 2:
          m1 = *( mag - magUpdate);
          m2 = *( mag + magUpdate);
          break;
          
          case 3:
          m1 = *( mag + 1 - magUpdate);
          m2 = *( mag - 1 + magUpdate);
          break;

          case 4:
          m2 = *( mag - 1);
          m1 = *( mag + 1);
          break;

          case 5:
          m2 = *( mag - 1 - magUpdate);
          m1 = *( mag + 1 + magUpdate);
          break;

          case 6:
          m2 = *( mag - magUpdate);
          m1 = *( mag + magUpdate);
          break;

          case 7:
          m2 = *( mag + 1 - magUpdate);
          m1 = *( mag - 1 + magUpdate);
          break;
          default:
              m1 = m2 = 0;
              assert (1);
        }
        
        if((*mag > m1 && *mag >= m2))
        {
          numPeaks++;
          *dest = (angleLabeled) ? ((*ang >> 1) + 127) : *mag;
        }
        else
          *dest = 0;
      }
      else
        *dest = 0;
    }
  }
  break;

    case rcPixel16:
  // Main part  
  for(y = 0; y < zeroc.height(); y++)
  {
    const uint8 *mag = magImage.rowPointer(y+1);  // 3x3 processing
    const uint16 *ang = (uint16*) angleImage.rowPointer(y+1);
    mag+=1;
    ang+=1;
    
    uint8 *dest = zeroc.rowPointer(y);
    
    for(x = 0; x < zeroc.width(); x++,mag++,ang++,dest++)
    {
      if(*mag >= threshold)
      {
        uint8 m1,m2;
        
        int32 angle = getAngle16Axis(*ang);
        switch(angle)
        {
          case 0:
          m1 = *(mag - 1);
          m2 = *(mag + 1);
          break;

          case 1:
          m1 = *(mag - 1 - magUpdate);
          m2 = *(mag + 1 + magUpdate);
          break;

          case 2:
          m1 = *( mag - magUpdate);
          m2 = *( mag + magUpdate);
          break;
          
          case 3:
          m1 = *( mag + 1 - magUpdate);
          m2 = *( mag - 1 + magUpdate);
          break;

          case 4:
          m2 = *( mag - 1);
          m1 = *( mag + 1);
          break;

          case 5:
          m2 = *( mag - 1 - magUpdate);
          m1 = *( mag + 1 + magUpdate);
          break;

          case 6:
          m2 = *( mag - magUpdate);
          m1 = *( mag + magUpdate);
          break;

          case 7:
          m2 = *( mag + 1 - magUpdate);
          m1 = *( mag - 1 + magUpdate);
          break;
          default:
              m1 = m2 = 0;
              assert (1);
        }
        
        if((*mag > m1 && *mag >= m2))
        {
          numPeaks++;
          *dest = *mag;
        }
        else
          *dest = 0;
      }
      else
        *dest = 0;
    }
  }
  break;
    default:
      rmAssert(0);
    }


  peaks = zeroc;
  return numPeaks;
  
}


/*
 * Run Sobel on image
 */
void rfSobelEdge (const rcWindow& image, rcWindow& magnitudes,rcWindow& angles)
{
  assert(magnitudes.width() == angles.width());
  assert(image.width() == magnitudes.width() + 2);
  assert(magnitudes.height() == angles.height());
  assert(image.height() == magnitudes.height() + 2);

  switch (image.depth())
    {
    case rcPixel8:
      {
	switch (angles.depth())
	  {
	  case rcPixel16:
	  {
	    sobelEdgeProcess<uint16,uint8> ((uint16*) (angles.rowPointer (0)),
					(const uint8*) (image.rowPointer (0)), image, magnitudes, angles);
	    break;
	  }
	  case rcPixel8:
	    {
	      sobelEdgeProcess<uint8,uint8> ((uint8*) (angles.rowPointer (0)),
						 (const uint8*) (image.rowPointer (0)), image, magnitudes, angles);
	      break;
	    }
	  default:
	    rmExceptionMacro(<< "rcWindow::notImplemented");
	  }
	break;
      }
    case rcPixel16:
      {
	switch (angles.depth())
	  {
	  case rcPixel16:
	  {
	    sobelEdgeProcess<uint16,uint16> ((uint16*) (angles.rowPointer (0)),
					(const uint16*) (image.rowPointer (0)), image, magnitudes, angles);
	    break;
	  }
	  case rcPixel8:
	    {
	      sobelEdgeProcess<uint8,uint16> ((uint8*) (angles.rowPointer (0)),
						  (const uint16*) (image.rowPointer (0)), image, magnitudes, angles);
	      break;
	    }
	  default:
	    rmExceptionMacro(<< "rcWindow::notImplemented");
	  }
	break;
      }
    default:
      rmExceptionMacro(<< "rcWindow::notImplemented");
    }
}


void rfSobel (const rcWindow& image, rcWindow& magnitudes,rcWindow& angles, bool highPrecAngle)
{
  if (!(image.isBound() && image.width() > 3 && image.height() > 3))
    rmExceptionMacro(<< "rcWindow::badparams");    

  if (! (angles.isBound() && angles.size() == image.size() && (angles.depth() == highPrecAngle ? rcPixel16 : rcPixel8)))
    angles = rcWindow (image.width(), image.height(),  highPrecAngle ? rcPixel16 : rcPixel8);
  // else angles is bound, right size and right depth

  if (! (magnitudes.isBound() && magnitudes.size () == image.size() && magnitudes.depth() != rcPixel8))
    magnitudes = rcWindow (image.width(), image.height(), rcPixel8);
  // else angles is bound, right size and right depth

  rfSetWindowBorder(magnitudes, 0.0);
  rfSetWindowBorder(angles, 0.0);

  rcWindow grad ((magnitudes), 1, 1, (magnitudes).width()-2, (magnitudes).height()-2); 
  rcWindow angle ((angles), 1, 1, (angles).width()-2, (angles).height()-2); 
  rfSobelEdge (image, grad, angle); 

}

/* In this function
 * If image is 8 bit, angle and mag are 8 bit
 * If image is 16 bit, angle is 16 bit and mag 8 bit
 */


void rfDirectionSignal (const rcWindow& image, const rcWindow& mask, vector<double>& signal)
{
	rcWindow mag, ang;

	rfSobel (image, mag, ang, image.depth () == rcPixel16);

	if (mask.isBound ())
	{
#ifdef _DEBUG		
		static const char *defaultFormat = "/tmp/um%d_%s";
		static const char *baseName = "ih.tiff";
		rcWindow masked = rfImageAddMask (image, mask, 0);
		std::string mfile = rfMakeTmpFileName(defaultFormat, baseName);
		masked.tiff (mfile);
#endif		
	}	

	static const double zd (0.0);
	rmAssert (image.depth () == ang.depth ());
	vector<double> histogram (1 << ang.bits(), zd);

	uint32 lastRow = ang.height() - 1, row = 0;
	const uint32 opsPerLoop = 8;
	uint32 unrollCnt = ang.width() / opsPerLoop;
	uint32 unrollRem = ang.width() % opsPerLoop;

	if (!mask.isBound ())
	{
		if (ang.depth () == rcPixel8)
		{
			for ( ; row <= lastRow; row++)
			{
				const uint8* pixelPtr = ang.rowPointer(row);
				const uint8* magPtr = mag.rowPointer(row);	

				for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
				{
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;						
				}

				for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++)
					histogram[*pixelPtr++]+= *magPtr++;
			}
		}
		else if (ang.depth () == rcPixel16)
		{
			for ( ; row <= lastRow; row++)
			{
				const uint16* pixelPtr = (uint16*) ang.rowPointer(row);
				const uint8* magPtr = mag.rowPointer(row);	

				for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
				{
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;
					histogram[*pixelPtr++]+= *magPtr++;						
				}

				for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++)
					histogram[*pixelPtr++]+= *magPtr++;	
			}
		}
		else
		{
			rmAssert (0);
		}	
	}						
	else
	{
		if (ang.depth () == rcPixel8)
		{
			for ( ; row <= lastRow; row++)
			{
				const uint8* pixelPtr = ang.rowPointer(row);
				const uint8* maskPtr = mask.rowPointer(row);	

				for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
				{
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;						
				}

				for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++)
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
			}
		}
		else if (ang.depth () == rcPixel16)
		{
			for ( ; row <= lastRow; row++)
			{
				const uint16* pixelPtr = (uint16*) ang.rowPointer(row);
				const uint8* maskPtr = mask.rowPointer(row);	

				for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
				{
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;						
				}

				for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++)
					histogram[*pixelPtr++]+= (!*maskPtr++) ? 1 : 0;	
			}
		}
		else
		{
			rmAssert (0);
		}
	}
//	double sumMag = rfSum (histogram);
//	vector<double>::iterator dItr = histogram.begin ();
//	for (; dItr != histogram.end(); dItr++) *dItr = *dItr / sumMag;
	signal.clear ();
	swap (signal, histogram);
}


	
#if 0
#define MASKFETCH \
if (*maskPtr++) \
{\
	pixelPtr++;magPtr++; \
}\
else \
{\
	histogram[*pixelPtr++]+= *magPtr++;\
}
	
void rfDirectionSignal (const rcWindow& image, const rcWindow& mask, vector<double>& signal)
{
	rmAssert(image.depth() == rcPixel8);	
	rcWindow mag, ang;
	rfSobel (image, mag, ang);
	static const double zd (0.0);
	vector<double> histogram (256, zd);
	rmAssert(histogram.size () == 256);


	uint32 lastRow = ang.height() - 1, row = 0;
	const uint32 opsPerLoop = 8;
	uint32 unrollCnt = ang.width() / opsPerLoop;
	uint32 unrollRem = ang.width() % opsPerLoop;

	for ( ; row <= lastRow; row++)
	{
		const uint8* pixelPtr = ang.rowPointer(row);
		const uint8* magPtr = mag.rowPointer(row);	
		const uint8* maskPtr = mask.rowPointer(row);	

		for (uint32 touchCount = 0; touchCount < unrollCnt; touchCount++)
		{
			MASKFETCH;
			MASKFETCH;
			MASKFETCH;
			MASKFETCH;
			MASKFETCH;
			MASKFETCH;
			MASKFETCH;
			MASKFETCH;																					
		}

		for (uint32 touchCount = 0; touchCount < unrollRem; touchCount++)
		{
			MASKFETCH;
		}

	}

	// Fold mod 180
	signal.resize (128, zd);
	vector<double>::iterator sItr = signal.begin ();	
	vector<double>::iterator dItr = histogram.begin ();
	vector<double>::iterator mItr = histogram.begin ();	
	advance (dItr, 128);
	for (; sItr != signal.end(); sItr++, dItr++, mItr++) *sItr = *dItr + *mItr;
}

#endif
