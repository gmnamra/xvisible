/*
 *
 *$Id $
 *$Log: $
 *
 *
 * Copyright (c) 2007 Reify Corp. All rights reserved.
 */
#include "rc_labelRLE.h"
#include <rc_ip.h>

static int32 rfRLEstimate (const rcWindow& image);

rcLabelRLE::rcLabelRLE(const rcWindow& img)
{
  const int32 height(img.height());
  
  numRuns_ = rfRLEstimate (img);
  alloc_ = new Run[ numRuns_ + height ];
  rat_ = new Run *[ height];
  Run * runs = NULL;
  switch (img.depth ())
    {
    case rcPixel8:
      runs = image2runs<uint8> (img, (uint8 *) img.pelPointer (0,0));
      break;
    case rcPixel16:
      runs = image2runs<uint16> (img, (uint16 *) img.pelPointer (0,0));
      break;
    case rcPixel32:
      runs = image2runs<uint32> (img, (uint32 *) img.pelPointer (0,0));
      break;
    default:
      rmExceptionMacro(<< "rcWindow::notImplemented");
    }
    
  runs->value = 0;
  runs->label = 0;
  runs++->length = 0;
}

template<class T>
rcLabelRLE::Run* rcLabelRLE::image2runs (const rcWindow& img, T* pelPtr)
{
  const int32 height(img.height());  
  const int32 width(img.width());  
  Run* runs = alloc_;
  int32 lastLength;
  uint32 lastValue;
  int32 runsleft = numRuns ();

  for (int y = 0 ; y < height; y++)
  {
    const T *pels = (T *) img.rowPointer(y);
    rat_[y] = runs;
    int32 w = width;
    const T *runStart = pels; //@note pointer size
    lastValue = pels[0];

    while(w--)
    {
      if(*pels != lastValue)
      {
	lastLength = pels - runStart;
	if (lastLength > 0)
	  {
	    runs->label = 0;
	    runs->value = lastValue;
	    runs++->length = lastLength;
	  }
	lastValue = *pels;
	runStart = pels;
      }
      ++pels;
    }

    lastLength = pels - runStart;
    if (lastLength > 0)
      {
	runs->label = 0;
	runs->value = lastValue;
	runs++->length = lastLength;
      }

    const int32 runsUsed = runs - rat_[y];
    runsleft -= runsUsed;

    // make EOR marker
    runs->value = 0;
    runs->length = 0;
    ++runs;
  }

  return runs;
}


rcLabelRLE::~rcLabelRLE()
{
  delete[] alloc_;
  delete[] rat_;
}


static int32 rfRLEstimate (const rcWindow& image)
{
  rmAssert (image.isBound());
  int32 estimate = 0;
  int32 bLast = image.width () - 2;
  int32 height = image.height ();
  for (int32 j = 0; j < image.height(); j++)
    {
      switch (image.depth ())
	{
	case rcPixel8:
	  estimate += rfGradHist1d ((uint8 *) image.pelPointer (0,j), (uint8 *) image.pelPointer (bLast,j));
	  break;
	case rcPixel16:
	  estimate += rfGradHist1d ((uint16 *) image.pelPointer (0,j), (uint16 *) image.pelPointer (bLast,j));
	  break;
	case rcPixel32:
	  estimate += rfGradHist1d ((uint32 *) image.pelPointer (0,j), (uint32 *) image.pelPointer (bLast,j));
	  break;
	default:
	  rmExceptionMacro(<< "rcWindow::notImplemented");
	}
    }
  return estimate+height+height;
}


// Includes and definitions specific to polygon conversion
//

#if 0
enum gpCompassDir {
  gpEast = 0,
  gpNorth,
  gpWest,
  gpSouth,
  gpNorthEast,
  gpSouthEast,
  gpNorthWest,
  gpSouthWest,
  gpCompassDirCnt
};

enum gpRelativeDir {
  gpLeft = 0,
  gpRight,
  gpUp,
  gpUpLeft,
  gpUpRight,
  gpDnLeft,
  gpDnRight,
  gpCenter,
  gpRelativeDirCnt
};

typedef struct gpOffset
{
  rcIPair off[gpRelativeDirCnt];
} gpOffset;

const gpOffset xlat[gpCompassDirCnt] =
  {
    { { rcIPair(0, -1), rcIPair(0, 1), rcIPair(1, 0), rcIPair(1, -1), 
	rcIPair(1, 1), rcIPair(-1, -1), rcIPair(-1, 1), rcIPair(0, 0) } }, // E

    { { rcIPair(-1, 0), rcIPair(1, 0), rcIPair(0, -1), rcIPair(-1, -1), 
	rcIPair(1, -1), rcIPair(-1, 1), rcIPair(1, 1), rcIPair(0, 0) } }, // N

    { { rcIPair(0, 1), rcIPair(0, -1), rcIPair(-1, 0), rcIPair(-1, 1), 
	rcIPair(-1, -1), rcIPair(1, 1), rcIPair(1, -1), rcIPair(0, 0) } }, // W

    { { rcIPair(1, 0), rcIPair(-1, 0), rcIPair(0, 1), rcIPair(1, 1), 
	rcIPair(-1, 1), rcIPair(1, -1), rcIPair(-1, -1), rcIPair(0, 0) } }, // S

    { { rcIPair(-1, -1), rcIPair(1, 1), rcIPair(1, -1), rcIPair(0, -1), 
	rcIPair(1, 0), rcIPair(-1, 0), rcIPair(0, 1), rcIPair(0, 0) } }, // NE

    { { rcIPair(1, -1), rcIPair(-1, 1), rcIPair(1, 1), rcIPair(1, 0), 
	rcIPair(0, 1), rcIPair(0, -1), rcIPair(-1, 0), rcIPair(0, 0) } }, // SE

    { { rcIPair(-1, 1), rcIPair(1, -1), rcIPair(-1, -1), rcIPair(-1, 0), 
	rcIPair(0, -1), rcIPair(0, 1), rcIPair(1, 0), rcIPair(0, 0) } }, // NW

    { { rcIPair(1, 1), rcIPair(-1, -1), rcIPair(-1, 1), rcIPair(0, 1), 
	rcIPair(-1, 0), rcIPair(1, 0), rcIPair(0, -1), rcIPair(0, 0) } } // SW
  };

const gpCompassDir dirMap[3][3] =
  {
    { gpNorthWest, gpWest, gpSouthWest },

    { gpNorth, gpCompassDirCnt, gpSouth },

    { gpNorthEast, gpEast, gpSouthEast }
  };

#define GPSRCHSEQSZ 6

static gpRelativeDir orthoSearchSeq[GPSRCHSEQSZ] =
  { gpRight, gpUp, gpUpRight, gpLeft, gpUpLeft, gpDnLeft };

static gpRelativeDir diagSearchSeq[GPSRCHSEQSZ] =
  { gpRight, gpUpRight, gpUpLeft, gpUp, gpDnLeft, gpLeft };

static gpRelativeDir* searchSeq[gpCompassDirCnt] =
  { orthoSearchSeq, orthoSearchSeq, orthoSearchSeq, orthoSearchSeq,
    diagSearchSeq, diagSearchSeq, diagSearchSeq, diagSearchSeq };

static gpRelativeDir originCheck[gpCompassDirCnt] =
  { gpUp, gpUp, gpUp, gpUp, gpUpLeft, gpUpLeft, gpUpLeft, gpUpLeft };

static gpRelativeDir orthoFillSeq[4] =
  { gpUpLeft, gpLeft, gpDnLeft, gpRelativeDirCnt };

static gpRelativeDir diagFillSeq[3] =
  { gpUpLeft, gpDnLeft, gpRelativeDirCnt };

static gpRelativeDir* fillSeq[gpCompassDirCnt] =
  { orthoFillSeq, orthoFillSeq, orthoFillSeq, orthoFillSeq,
    diagFillSeq, diagFillSeq, diagFillSeq, diagFillSeq };

// Debugging support for polygon conversion
//

#ifdef DYNAMIC_DEBUG
int __polygonDebug = 0;
#define GPDEBUG1 __polygonDebug
#define GPDEBUG2 __polygonDebug
#else
#define GPDEBUG1 0
#define GPDEBUG2 0
#endif

static char* compassStr[gpCompassDirCnt] =
  { "East", "North", "West", "South", "NorthEast",
    "SouthEast", "NorthWest", "SouthWest"
  };

static char* relDirStr[gpRelativeDirCnt] =
  { "Left", "Right", "Up", "UpLeft", "UpRight",
    "DnLeft", "DnRight", "Center"
  };

static char* pxlClass(uint16 bgPix, uint16 fgPix, uint16 ptPix,
		      uint16 pix)
{
  if (pix == bgPix)
    return "BG";
  else if (pix == fgPix)
    return "FG";
  else
    rmAssert(pix == ptPix);

  return "PT";
}

#endif
