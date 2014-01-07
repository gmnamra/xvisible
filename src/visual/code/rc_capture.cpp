/*
 *  $Id: rc_capture.cpp 7249 2011-02-26 02:16:06Z arman $
 *
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 */

#include <rc_capture.hpp>
#include <rc_gen_capture.h>

// Display utilities
std::ostream& operator << ( std::ostream& os, const struct rcAcqControl& c )
{
	os << "acquire: " << c.doAcquire
	<< " resolution " << c.resolution
#ifdef VD_CONTROLS_WORK
	<< " gain " << c.gain
	<< " shutter " << c.shutter
#endif
	<< " decimationRate " << c.decimationRate
	<< " shutdown " << c.doShutdown
	<< " ID " << c.acqCtrlUpdateID << endl;
	
	return os;
}

std::ostream& operator << ( std::ostream& os, const struct rcAcqInfo& c )
{
	os << "cameraType: " << c.cameraType
	<< " maxFPS: " << c.maxFramesPerSecond
	<< " " << c.maxFrameWidth << "x" << c.maxFrameHeight << "x" << c.cameraPixelDepth*8 << endl
	<< "cameraHdr: " << c.cameraInfo
#ifdef VD_CONTROLS_WORK
	<< " defGain " << c.defaultGain << " defShuter " << c.defaultShutter
	<< " curGain " << c.curGain << " curShutter " << c.curShutter
#endif
	<< " curDecimationRate " << c.curDecimationRate << " curRes " << c.curResolution
	<< " acqState " << c.acqState << " missedFrames " << c.missedFrames
	<< "acqFlags " << c.acqFlags << " ctrlFlags " << c.ctrlFlags
	<<  "ID " << c.acqCtrlUpdateID
	<< endl;
	
	return os;
}


ostream& operator << ( ostream& os, const rcCameraColorFormat& c )
{
	switch ( c ) {
		case rcCameraColorFormatYUV444:
			os << "YUV444";
			break;
			
		case rcCameraColorFormatYUV422:
			os << "YUV422";
			break;
			
		case rcCameraColorFormatYUV411:
			os << "YUV411";
			break;
			
		case rcCameraColorFormatRGB24:
			os << "RGB24";
			break;
			
		case rcCameraColorFormatGray8:
			os << "Gray8";
			break;
			
		case rcCameraColorFormatGray16:
			os << "Gray16";
			break;
			
		case rcCameraColorFormatUnknown:
		default:
			os << "unknown";
			break;
	}
	
	return os;
}


