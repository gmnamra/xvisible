/*
 COPYRIGHT © 2003 to  2008  Aupperle Services and Contracting.  ALL RIGHTS RESERVED.
																												
 Copyright Notice and Disclaimer of Liability:															

 Aupperle Services and Contracting is henceforth referred to as ASC.						
 
 Purchaser henceforth refers to the original purchaser(s) of the equipment, and/or any legitimate user(s).

 ASC hereby explicitly prohibits any form of reproduction (with the express strict exception for backup
 and archival purposes, which are allowed as stipulated within the License Agreement for ASC
 Software), modification, and/or distribution of this software and/or its associated documentation unless
 explicitly specified in a written agreement signed by both parties.
																												
 To the extent permitted by law, ASC disclaims all other warranties or conditions of any kind, either
 express or implied, including but not limited to all warranties or conditions of merchantability and
 fitness for a particular purpose and those arising by statute or otherwise in law or from a course of	
 dealing or usage of trade. Other written or oral statements by ASC, its representatives, or others do
 not constitute warranties or conditions of ASC.

 ASC makes no guarantees or representations, including but not limited to: the use of, or the result(s)
 of the use of: the software and associated documentation in terms of correctness, accuracy, reliability,
 being current, or otherwise. The Purchaser hereby agree to rely on the software, associated hardware and
 documentation and results stemming from the use thereof solely at their own risk.
																												
 ASC is hereby absolved of any and all liability to the Purchaser, and/or a third party, financial or
 otherwise, arising from any subsequent loss, direct and indirect, and damage, direct and indirect,
 resulting from intended and/or unintended usage of its software, product(s) and documentation, as well
 as additional service(s) rendered by ASC, such as technical support, unless otherwise explicitly
 specified in a written agreement signed by both parties. Under no circumstances shall the terms and	
 conditions of such an agreement apply retroactively.																																						
*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
//#include <vecLib/vecLibTypes.h>

#ifdef __cplusplus
extern "C" {
#endif//of __cplusplus
#ifndef _ASCDCAMAPI_
#define _ASCDCAMAPI_

//Aspect Selectors for a Camera Control
enum ASC_IIDC_CNTL_SELECTORS
{
	MANUAL_CNTL					=	0x00,//manually set a value
	AUTO_CNTL						=	0x01,//switch to auto control
	ON_OFF_CNTL					=	0x02,//stop a control from operating..
	ONE_PUSH_CNTL				=	0x03,// one push to use one time
	U_OR_B_CNTL					=	0x04,//selector for White_Balance register U or B color
	V_OR_R_CNTL					=	0x05,//selector for White_Balance register V or R color
	TARGET_TEMP_CNTL			=	0x06,//selector for TEMPERATURE register reading/writng target temps.
	CURR_TEMP_CNTL				=	0x07,//selector for TEMPERATURE register reading current temp.. 
	TRIG_POLARITY_CNTL			=	0x08,//selector for TRIGGER register changes polarity
	TRIG_MODE_CNTL				=	0x09,//selector for TRIGGER register change modes ..
	TRIG_PARAM_CNTL				=	0x0A,//selector for TRIGGER register set optional paramater
	ON_OFF_ABS_CNTL				=	0x0B//Selector for enabling / disabling absolute control..};
};

//Camera Control Property Selectors
enum ASC_IIDC_PROPERTY_SELECTORS
{	
	ASC_BRIGHTNESS_REG			= 0x0500,		// brightness
	ASC_EXPOSURE_REG				= 0x0504,		// exposure
	ASC_SHARPNESS_REG				= 0x0508,		// sharpness
	ASC_WHITE_BAL_REG				= 0x050C,		// white balance
	ASC_HUE_REG						= 0x0510,		// hue
	ASC_SATURATION_REG			= 0x0514,		// saturation
	ASC_GAMMA_REG					= 0x0518,		// gamma
	ASC_SHUTTER_REG					= 0x051C,		// shutter
	ASC_GAIN_REG						= 0x0520,		// gain
	ASC_IRIS_REG						= 0x0524,		// iris
	ASC_FOCUS_REG						= 0x0528,		// focus
	ASC_TEMPERATURE_REG			= 0x052C,		// temperature
	ASC_TRIGGER_REG					= 0x0530,		// trigger
	ASC_TRIGGER_DELAY_REG		= 0x0534,		//trigger delay
	ASC_WHITE_SHADING_REG		= 0x0538,		//White Shading
	ASC_FRAME_RATE_REG			= 0x053C,		//Frame Rate
	ASC_ZOOM_REG						= 0x0580,		// zoom
	ASC_PAN_REG						= 0x0584,		// pan
	ASC_TILT_REG						= 0x0588,		// tilt
	ASC_OPTICAL_FILTER_REG		= 0x058C		// optical filter
};

//Format Descriptors
/*
	naming convention for Bayer ASC_DISPLAY_FORMATS for "blRG' is
	b = Bayer
	l / m / h =  low medium high quality bayer decoding
	RG / BG / GR / GB indicate what colors and pixel orders are used for the first line of the bayer image
	So RG means first line is red green and first pixel is Red followed by Green
*/
enum ASC_DISPLAY_FORMATS
{
	kASC_ARGB						= 'ARGB',//Use this in all cases EXCEPT if the Camera Outputs Bayer Video
	//Low Quality / Low CPU Usage Bayer
	kLQ_ASCBAYER_RGGB			= 'blRG',//red green alternating pixels then green blue alternating pixels on next line
	kLQ_ASCBAYER_GBRG			= 'blBG',// blue green alternating pixelsthen  green red alternating pixels on next line
	kLQ_ASCBAYER_GRBG			= 'blGR',// green red alternating pixels then blue green alternating pixels on next line
	kLQ_ASCBAYER_BGGR			= 'blGB',// green blue alternating pixels then red green alternating pixels on next line
	
	//Medium Quality / Medium CPU Usage Bayer
	kMQ_ASCBAYER_RGGB			= 'bmRG',//red green alternating pixels then green blue alternating pixels on next line
	kMQ_ASCBAYER_GBRG			= 'bmBG',// blue green alternating pixelsthen  green red alternating pixels on next line
	kMQ_ASCBAYER_GRBG 		= 'bmGR',// green red alternating pixels then blue green alternating pixels on next line
	kMQ_ASCBAYER_BGGR			= 'bmGB',// green blue alternating pixels then red green alternating pixels on next line
	
	//High Quality / SLOW Extreme CPU Usage Bayer
	kHQ_ASCBAYER_RGGB			= 'bhRG',//red green alternating pixels then green blue alternating pixels on next line
	kHQ_ASCBAYER_GBRG			= 'bhBG',// blue green alternating pixelsthen  green red alternating pixels on next line
	kHQ_ASCBAYER_GRBG			= 'bhGR',// green red alternating pixels then blue green alternating pixels on next line
	kHQ_ASCBAYER_BGGR			= 'bhGB'// green blue alternating pixels then red green alternating pixels on next line
 };
 
 enum ASC_FORMAT7_COLOUR_ID //for use with ASCDCAMSet7MR(...colorCodingID..) parameters
 {
 	kMONO8_ID						= 0,//8 bit per pixel monochrome
 	kYUV411_ID						= 1,//12 bit per pixel yuv 411
 	kYUV422_ID						= 2,//16 bit per pixel yuv 422
 	kYUV444_ID						= 3,//24 bit per pixel yuv 444
 	kRGB24_ID						= 4,//24 bit per pixel RGB24
 	kMONO16_ID						= 5,//16 bit per pixel  mono16
 	kRGB48_ID						= 6,//48 bit per pixel RGB48
 	//IIDC 1.3.1 or higher specifcation only - also camera dependent too..
 	kBAYER8_ID						= 9,//8 bit per pixel Bayer
 	kBAYER16_ID					= 10//16 bits per pixel Bayer
 };
 
  enum ASC_CODEC_FORMATS //for V2.0 movie recording API call ASCDCAMCreateMovie()
 {
 	kASC_YUV		 = kComponentVideoCodecType,//both YUV411 and YUV422 are delivered as kComponentVideoCodecType
 	kASC_RGB		=	kRawCodecType,//YUV444, Mono8,Mono16, RGB24, ARGB32 and RGB48 formats are delivered as kRawCodecType
 	kASC_RGGB	=	'RGGB',//These four Bayer formats record a Bayer Codec - which is only available as a separate licensable item.
 	kASC_GBRG	=	'GBRG',
 	kASC_GRBG	=	'GRBG',
 	kASC_BGGR	=	'BGGR'
 };

//FITS file export descriptor
enum		ASC_FITS_FILE_FORMAT
{
	kASCFileTypeFITS			=	'FITS' //Only available for monochrome 8 or 16 bit images
};

typedef struct
{
	UInt32		imageWidth;
	UInt32		imageHeight;
	UInt32		imageBytes;//imageWidth * imageHeight * bytes per pixel..
	void*			ImageBufferPtr;//reference to the data buffer do not Dispose of it..
	void*			userDataPtr;//anything you want..
	double		frameDurationMS;//duration of this frame in milliseconds accurate to +/- 0.125 ms or 1 FW packet
	double		elapsedTimeMS;//elapsed time since the Camera started transmititng video in milliseconds..
											//the elapsed time will be reset to zero if the camera stops or pasues video transmission
} ASC_CallbackStruct;

typedef void (*ASC_CallbackProcPtr) (ASC_CallbackStruct); //callback for data procs..
/*
	Your Callback function is declared as
	void myDisplayCallback(ASC_CallbackStruct theCallBackData);
*/

//structure containing Gross Camera Properties for formats, modes, frame rates and controls for Veriosn 1.0 and 2.0 of SDK..
typedef struct {
	Boolean			format0;//has format0
	Boolean			format0Modes[8];//what modes are supported
	Boolean			format0ModesFrameRates[48];//what frame rates are supported.. 1.875 to 60 fps..
	
	Boolean 			format1;//has format1
	Boolean			format1Modes[8];//what modes are supported
	Boolean			format1ModesFrameRates[48];//what frame rates are supported.. 1.875 to 60 fps..
	
	Boolean 			format2;//has format2
	Boolean			format2Modes[8];//what modes are supported
	Boolean			format2ModesFrameRates[48];//what frame rates are supported by these modes.. 1.875 to 60 fps..
	
	Boolean 			format7;//has format7
	Boolean			format7Modes[8];//what modes of Format 7's are supported
	
	//other properties this camera may support..
	Boolean			hasBright,hasExpos,hasSharp,hasWhiteB,hasHue,hasSat,hasGain;
	Boolean			hasGamma,hasShutter,hasIris,hasFocus;
	Boolean			hasTemp,hasTrigger,hasZoom,hasPan,hasTilt,hasOpticalFilter;
	UInt32			uniqueID;//unique firewire device id - use this in conjunction ASCDCAMOpenACamera() to open this camera..
	char				deviceName[80];//name of the model..
} CameraCapabilitiesStruct;

//structure containing Gross Camera Properties for formats, modes, frame rates and controls for verions 2.0 of SDK..
typedef struct {
	UInt32			versionID;//currently 0x00020000 upper 16 bit word of 0x0002 is Major verions,  Lower 16 bit word is minor version..
	Boolean			format0;//has format0
	Boolean			format0Modes[8];//what modes are supported
	Boolean			format0ModesFrameRates[64];//what frame rates are supported.. 1.875 to 240 fps..
	
	Boolean 			format1;//has format1
	Boolean			format1Modes[8];//what modes are supported
	Boolean			format1ModesFrameRates[64];//what frame rates are supported.. 1.875 to 240 fps..
	
	Boolean 			format2;//has format2
	Boolean			format2Modes[8];//what modes are supported
	Boolean			format2ModesFrameRates[64];//what frame rates are supported by these modes, 1.875 to 240 fps..
	
	Boolean 			format7;//has format7
	Boolean			format7Modes[8];//what modes of Format 7's are supported
	
	//other properties this camera may support..
	Boolean			hasBright,hasExpos,hasSharp,hasWhiteB,hasHue,hasSat,hasGain;
	Boolean			hasGamma,hasShutter,hasIris,hasFocus;
	Boolean			hasTemp,hasTrigger,hasZoom,hasPan,hasTilt,hasOpticalFilter;
	Boolean			hasTriggerDelay,hasWhiteShading,hasFrameRate;
	UInt32			uniqueID;//unique firewire device id - use this in conjunction ASCDCAMOpenACamera() to open this camera..
	UInt32			vendorID;//Unique ID for the vendor/manufacturer of the camera.
	char				deviceName[80];//name of the camer model..
	
	//IIDC 1.3.x Absolute properties this camera may support..
	Boolean			hasAbsBright,hasAbsExpos,hasAbsSharp,hasAbsWhiteB,hasAbsHue,hasAbsSat,hasAbsGain;
	Boolean			hasAbsGamma,hasAbsShutter,hasAbsIris,hasAbsFocus;
	Boolean			hasAbsTemp,hasAbsTrigger,hasAbsZoom,hasAbsPan,hasAbsTilt,hasAbsOpticalFilter;
	Boolean			hasAbsTriggerDelay,hasAbsWhiteShading,hasAbsFrameRate;
} CameraCapabilitiesStruct2;

//Struct
typedef struct {
	Boolean				isInUse;//flag - indicates if camera is in use or not..
	UInt32				manufacturerID;
	UInt32				uniqueID;
	char					deviceName[80];
} dcamCameraListStruct;

typedef struct {
	UInt32							numCamerasFound;
	dcamCameraListStruct		theCameras[63];
} dcamCameraList;

extern	OSStatus	ASCDCAMGetListOfCameras(dcamCameraList *pTheCameraList);

extern	void*			ASCDCAMOpenACamera(UInt32	uniqueID);

extern	OSStatus	ASCDCAMCloseACamera(void* pTheCamera);

extern	OSStatus	ASCDCAMGetCameraProperties(void* pTheCamera, CameraCapabilitiesStruct *cameraProperties);

extern	OSStatus	ASCDCAMGetCameraProperties2(void* pTheCamera, CameraCapabilitiesStruct2 *cameraProperties2);

extern	OSStatus	ASCDCAMGetPropertyCapability(	void* pTheCamera, UInt32 cameraPropertySelector,UInt32 *minVal,UInt32 *maxVal,
																			Boolean *canOnePush,Boolean *canReadOut, Boolean *canOnOff, Boolean *canAuto,Boolean *canManual);
																			
extern	OSStatus	ASCDCAMGetPropertyValue(void* pTheCamera, UInt32 cameraPropertySelector,UInt32 whichControlSelector,UInt32 *value);

extern	OSStatus	ASCDCAMSetPropertyValue(void* pTheCamera, UInt32 cameraPropertySelector,UInt32 whichControlSelector,UInt32 value);

extern	OSStatus	ASCDCAMGetFMRProperties(	void* pTheCamera,UInt32	format,UInt32 mode, double frameRate,
																		UInt32 *maxWidth, UInt32 *maxHeight,UInt32 *widthInc,UInt32 *heightInc,
																		UInt32 *leftInc,UInt32 *topInc,UInt32 *maxPacketSize,UInt32 *packetSizeInc,
																		UInt32 *supportedColorCoding);

extern	OSStatus	ASCDCAMSetFMR(void* pTheCamera,UInt32	format,UInt32 mode, double frameRate);

extern	OSStatus	ASCDCAMSet7MR(	void* pTheCamera,UInt32 mode,UInt32 left,UInt32 top,
														UInt32 width,UInt32 height, UInt32 colorCodingID,UInt32 bytesPerPacket);
															
extern	OSStatus	ASCDCAMGetFMRLTWHCB(	void* pTheCamera,UInt32 *format,UInt32 *mode, double *frameRate,
																	UInt32 *left,UInt32 *top,UInt32 *width, UInt32 *height,
																	OSType *cCCCType, UInt32 *bitsPerPixel);

extern	OSStatus	ASCDCAMSetQuadlet(void* pTheCamera, UInt16 adddressHi, UInt32 addressLo, Boolean isAbsoluteAddress,UInt32 quadletValue);

extern	OSStatus	ASCDCAMGetQuadlet(void* pTheCamera, UInt16 adddressHi, UInt32 addressLo, Boolean isAbsoluteAddress,UInt32 *quadletValue);

extern	OSStatus	ASCDCAMAllocateReleaseIsochChannel(void* pTheCamera,Boolean allocateIsoch);

extern	OSStatus	ASCDCAMStartStopVideo(void* pTheCamera,Boolean startVideo);

extern	OSStatus	ASCDCAMResumePauseVideo(void* pTheCamera, Boolean resumeVideo); 

extern	OSStatus	ASCDCAMConvertRawFrameToGWorld(void* pTheCamera, void* srcPtr, 
																				UInt32 width, UInt32 height,OSType cCCCType,
																				UInt32 bitsPerPixel,  GWorldPtr	*theGWorld);

extern	OSStatus	ASCDCAMConvertRawFrameToARGB32(	void* pTheCamera, void* srcPtr, UInt32 width, UInt32 height,OSType cCCCType,
																					UInt32 bitsPerPixel, void *destPtr,UInt32	rowByteWidth);
																				
extern	OSStatus	ASCDCAMGrabRawFrame(void* pTheCamera, void * bufferPtr, UInt32 timeOutDurationMS );

extern	OSStatus	ASCDCAMGrabARGB32Frame(void* pTheCamera, void *destARGB32Ptr, OSType displayFormat, UInt32 timeOutDurationMS);


extern	OSStatus	ASCDCAMSaveFrameToDisk(void* pTheCamera,FSSpec fileLocation, OSType fileFormat,OSType displayFormat, UInt32 timeOutDurationMS);

extern	OSStatus	ASCDCAMCancelGrabFrame(void* pTheCamera);

extern	OSStatus	ASCDCAMAssignCallback(void* pTheCamera, ASC_CallbackProcPtr callBackPtr, void* userDataPtr);

extern	OSStatus	ASCDCAMSetIODelayTimes(void* pTheCamera,UInt32 IOWriteDelayMS,UInt32 IOReadDelayMS);

//---------------- New API calls introduced in version 2.00.00 of the Framework..
extern	OSStatus	ASCDCAMConvertRaw16FrameToRGB48(void* pTheCamera, void* srcPtr, UInt32 width, UInt32 height,OSType cCCCType,
																						UInt32 bitsPerPixel, void* destPtr,UInt32	rowByteWidth);

extern	OSStatus	ASCDCAMSaveGWorldtoDisk(void* pTheCamera, GWorldPtr	theGWorld, FSSpec fileLocation, OSType fileFormat);

extern	OSStatus	ASCDCAMSaveRAWFrametoDisk(	void* pTheCamera, void * bufferPtr, UInt32 width, UInt32 height, UInt32 bitsPerPixel, 
																			FSSpec fileLocation, OSType fileFormat,OSType displayFormat);

extern	OSStatus 	ASCDCAMGetABSPropertyCapability(void* pTheCamera, UInt32 cameraPropertySelector,
																				float *minVal,float *maxVal,Boolean *canReadOut,Boolean *canManual);
																				
extern	OSStatus  	ASCDCAMGetABSPropertyValue(void* pTheCamera,UInt32 cameraPropertySelector,UInt32 whichAbsControlSelector,float *value);
extern	OSStatus  	ASCDCAMSetABSPropertyValue(void* pTheCamera,UInt32 cameraPropertySelector,UInt32 whichAbsControlSelector,float value);


extern	void*			ASCDCAMCreateMovie( void* pTheCamera,double expectedFrameDurationMS, TimeScale	trackTimeScale,UInt32 width, 
															UInt32 height,OSType cCCCType,UInt32 bitsPerPixel, UInt32 rowByteWidth,
															FSSpec fileLocation,OSStatus *theErr);

extern	OSStatus	ASCDCAMAddMovieFrame(void* pMovieData, void * framePtr ,double timeInMilliseconds);

extern	OSStatus 	ASCDCAMCloseMovie(void* pMovieData);

//---------------- New API calls introduced in version 2.01.00 of the Framework..
extern	OSStatus	ASCDCAMSixteenBitUpShiftAmount(void* pTheCamera, UInt32 theBitShiftAmount);

#ifdef __cplusplus
}
#endif//__cplusplus
#endif //_ASCDCAMAPI_