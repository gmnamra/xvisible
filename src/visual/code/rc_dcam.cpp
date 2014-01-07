/*
 *
 * $Id: rc_dcam.cpp 7203 2011-02-15 07:09:36Z arman $
 *
 * This file contains ASC DCAM support code.
 *
 * Copyright (c) 2003-2004 Reify Corp. All rights reserved.
 *
 */

#include "rc_dcam.h"
#include <rc_moviefileformat.h>

static std::string dcam_propertyName( int type );
//static std::string dcam_cntlName( int type );


// Get one proprety from camera
static OSStatus getOneProperty( void* cameraPtr, UInt32 type, UInt32 cntl, UInt32* value )
{
	OSStatus err = ASCDCAMGetPropertyValue( cameraPtr,
																				 type,
																				 cntl,
																				 value );
	
	return err;
}

ostream& operator<<(ostream& os, rcCameraProperty c)
{
		os << dcam_propertyName( c.type() ) << "-" << dcam_cntlName( c.cntl() ) << ": "
		<< c.curValue() << " [" << c.minValue() << "-"
		<< c.maxValue() << "]"
		<< " Active " << int(c.active())
		<< " CanReadOut " << int(c.canReadOut())
		<< " CanOnePush " << int(c.canOnePush())
		<< " CanOnOff " << int(c.canOnOff())
		<< " CanAuto " << int(c.canAuto())
		<< " CanManual " << int(c.canManual());
		return os;
}

// Get one cntl property
static rcCameraProperty GetCameraProperty( void* cameraPtr, UInt32 type, UInt32 cntl )
{
	rcCameraProperty prop( type, cntl );
	
	OSStatus err = ASCDCAMGetPropertyCapability( cameraPtr,
																							type,
																							&prop.minValue(),
																							&prop.maxValue(),
																							&prop.canOnePush(),
																							&prop.canReadOut(),
																							&prop.canOnOff(),
																							&prop.canAuto(),
																							&prop.canManual() );
	if ( err == 0 ) {
		if ( prop.canReadOut() ) {
			prop.active() = 1;
			err = getOneProperty( cameraPtr,
													 type,
													 cntl,
													 &prop.curValue() );
			if ( err != 0 )
				prop.active() = 0;
		}
	} else {
#ifdef DEBUG_LOG
		cerr << "ASCDCAMGetPropertyCapability " <<  propertyName( type ) << " error " << err << endl;
#endif
	}
	return prop;
}

// Set capability field
static void rfSetCapability( CameraCapabilitiesStruct& cap,
														int type )
{
	switch ( type ) {
		case ASC_BRIGHTNESS_REG:
			cap.hasBright = 1;
			break;
		case ASC_EXPOSURE_REG:
			cap.hasExpos = 1;
			break;
		case ASC_SHARPNESS_REG:
			cap.hasSharp = 1;
			break;
		case ASC_WHITE_BAL_REG:
			cap.hasWhiteB = 1;
			break;
		case ASC_HUE_REG:
			cap.hasHue = 1;
			break;
		case ASC_SATURATION_REG:
			cap.hasSat = 1;
			break;
		case ASC_GAMMA_REG:
			cap.hasGamma = 1;
			break;
		case ASC_SHUTTER_REG:
			cap.hasShutter = 1;
			break;
		case ASC_GAIN_REG:
			cap.hasGain = 1;
			break;
		case ASC_IRIS_REG:
			cap.hasIris = 1;
			break;
		case ASC_FOCUS_REG:
			cap.hasFocus = 1;
			break;
		case ASC_TEMPERATURE_REG:
			cap.hasTemp = 1;
			break;
		case ASC_TRIGGER_REG:
			cap.hasTrigger = 1;
			break;
		case ASC_ZOOM_REG:
			cap.hasZoom = 1;
			break;
		case ASC_PAN_REG:
			cap.hasPan = 1;
			break;
		case ASC_TILT_REG:
			cap.hasTilt = 1;
			break;
		case ASC_OPTICAL_FILTER_REG:
			cap.hasOpticalFilter = 1;
			break;
		default:
			cerr << "Error: unknown control " << type << endl;
			break;
	}
}


// Get a vector of all properties
int rfGetCameraProperties( void* cameraPtr,
													vector<rcCameraProperty>& properties )
{
	rmAssert( properties.empty() );
	
	properties.push_back( GetCameraProperty( cameraPtr, ASC_BRIGHTNESS_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_EXPOSURE_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_SHARPNESS_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_WHITE_BAL_REG, U_OR_B_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_WHITE_BAL_REG, V_OR_R_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_HUE_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_SATURATION_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_GAIN_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_GAMMA_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_SHUTTER_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_IRIS_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_FOCUS_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_TEMPERATURE_REG, TARGET_TEMP_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_TEMPERATURE_REG, CURR_TEMP_CNTL ) );
#ifdef notyet
	// TODO: Sony cameras return an error for this, investigate
	properties.push_back( GetCameraProperty( cameraPtr, ASC_TRIGGER_REG, TRIG_POLARITY_CNTL ) );
#endif
	properties.push_back( GetCameraProperty( cameraPtr, ASC_TRIGGER_REG, TRIG_MODE_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_TRIGGER_REG, TRIG_PARAM_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_ZOOM_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_PAN_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_TILT_REG, MANUAL_CNTL ) );
	properties.push_back( GetCameraProperty( cameraPtr, ASC_OPTICAL_FILTER_REG, MANUAL_CNTL ) );
	
	return properties.size();
}

// Get maximum frame rate
double rfGetCameraMaxFrameRate( void* cameraPtr )
{
	double maxFrameRate = 0.0;
	
	CameraCapabilitiesStruct cap;
	OSStatus err = ASCDCAMGetCameraProperties(cameraPtr, &cap);
	
	if ( err !=0 ) return -1.0;
	
	// Populate possible framerates for format0,1,2 according to Table in DCAM documentation
	for ( short mode = 0; mode < 3; ++mode )
	{
		const short start = mode * 6;
		double framerate = 1.875;
		double cur = 0.0;
		for ( short i = start; i < start+6; ++i )
		{
			if ( mode == 0 ) {
				if ( cap.format0ModesFrameRates[i] == true )
					cur = framerate;
			} else if ( mode == 1 )
			{
				if ( cap.format1ModesFrameRates[i] == true )
					cur = framerate;
			} else if ( mode == 2 )
			{
				if ( cap.format2ModesFrameRates[i] == true )
					cur = framerate;
			}
			framerate *= 2.0;
		}
		if ( cur > maxFrameRate )
			maxFrameRate = cur;
	}
	
	// If we have format 7. This is camera specific. So we need it from some place @todo
	if (cap.format7) 
	{
		maxFrameRate = std::max (60.0, maxFrameRate);
	} 
	else
	{
		cerr << "ASCDCAMGetCameraProperties error " << err << endl;
	}
	
	return maxFrameRate;
}

// Get true camera capabilities
OSStatus rfGetCameraCapabilities( void* cameraPtr,
																 CameraCapabilitiesStruct& capabilities )
{
	OSStatus err = ASCDCAMGetCameraProperties( cameraPtr, &capabilities );
	// Unfortunately these propertiers cannot be fully trusted:
	// some cameras offer capabilities not returned here.
	
	// Find capabilities by brute force search.
	vector<rcCameraProperty> props;
	int count = rfGetCameraProperties( cameraPtr, props );
	if ( count > 0 ) {
		for ( uint32 i = 0; i < props.size(); ++i ) {
			const rcCameraProperty& p = props[i];
			if ( p.active() ) {
				rfSetCapability( capabilities, p.type() );
			}
		}
	}
	
	return err;
}




	// Utilities
std::string dcam_cntlName( int type )
{
	switch ( type ) {
		case MANUAL_CNTL:
			return "Manual";
		case AUTO_CNTL:
			return "Auto";
		case ON_OFF_CNTL:
			return "On-off";
		case ONE_PUSH_CNTL:
			return "One-push";
		case U_OR_B_CNTL:
			return "U-B";
		case V_OR_R_CNTL:
			return "V-R";
		case TARGET_TEMP_CNTL:
			return "Target";
		case CURR_TEMP_CNTL:
			return "Curr";
		case TRIG_POLARITY_CNTL:
			return "Polarity";
		case TRIG_MODE_CNTL:
			return "Mode";
		case TRIG_PARAM_CNTL:
			return "Param";
	}
	
	return "Unknown control";
}

	// Return name of property
static std::string dcam_propertyName( int type )
{
	switch ( type ) {
		case ASC_BRIGHTNESS_REG:
			return "Brightness";
		case ASC_EXPOSURE_REG:
			return "Exposure";
		case ASC_SHARPNESS_REG:
			return "Sharpness";
		case ASC_WHITE_BAL_REG:
			return "White balance";
		case ASC_HUE_REG:
			return "Hue";
		case ASC_SATURATION_REG:
			return "Saturation";
		case ASC_GAMMA_REG:
			return "Gamma";
		case ASC_SHUTTER_REG:
			return "Shutter";
		case ASC_GAIN_REG:
			return "Gain";
		case ASC_IRIS_REG:
			return "Iris";
		case ASC_FOCUS_REG:
			return "Focus";
		case ASC_TEMPERATURE_REG:
			return "Temperature";
		case ASC_TRIGGER_REG:
			return "Trigger";
		case ASC_ZOOM_REG:
			return "Zoom";
		case ASC_PAN_REG:
			return "Pan";
		case ASC_TILT_REG:
			return "Tilt";
		case ASC_OPTICAL_FILTER_REG:
			return "Optical Filter";
			
	}
	return std::string("Unknown property");
}

// Set appropriate cam hdr field
static void setProperty( rcMovieFileCamExt& info,
												const rcCameraProperty& p )
{
	//cerr << "setProperty " << propertyName( p.type() ) << ":" << p.curValue() << endl;
	
	switch ( p.type() ) {
		case ASC_BRIGHTNESS_REG:
			info.brightness( p.curValue() );
			break;
		case ASC_EXPOSURE_REG:
			info.exposure( p.curValue() );
			break;
		case ASC_SHARPNESS_REG:
			info.sharpness( p.curValue() );
			break;
		case ASC_WHITE_BAL_REG:
			if ( p.cntl() == U_OR_B_CNTL )
				info.whiteBalanceUB( p.curValue() );
			else if ( p.cntl() == V_OR_R_CNTL )
				info.whiteBalanceVR( p.curValue() );
			else
				cerr << "Error: unknown white balance control "
				<< dcam_cntlName( p.cntl() ) << endl;
			break;
		case ASC_HUE_REG:
			info.hue( p.curValue() );
			break;
		case ASC_SATURATION_REG:
			info.saturation( p.curValue() );
			break;
		case ASC_GAMMA_REG:
			info.gamma( p.curValue() );
			break;
		case ASC_SHUTTER_REG:
			info.shutter( p.curValue() );
			break;
		case ASC_GAIN_REG:
			info.gain( p.curValue() );
			break;
		case ASC_IRIS_REG:
			info.iris( p.curValue() );
			break;
		case ASC_FOCUS_REG:
			info.focus( p.curValue() );
			break;
		case ASC_TEMPERATURE_REG:
			// TODO: store target temp also
			if ( p.cntl() == CURR_TEMP_CNTL )
				info.temperature( p.curValue() );
			break;
		case ASC_TRIGGER_REG:
			// TODO: implement trigger control
			break;
		case ASC_ZOOM_REG:
			info.zoom( p.curValue() );
			break;
		case ASC_PAN_REG:
			info.pan( p.curValue() );
			break;
		case ASC_TILT_REG:
			info.tilt( p.curValue() );
			break;
		case ASC_OPTICAL_FILTER_REG:
			info.filter( p.curValue() );
			break;
		default:
			cerr << "Error: unknown control " << p.type() << endl;
			break;
	}
}

// Set camera header fields
void setProperties( rcMovieFileCamExt& info,
									 const CameraCapabilitiesStruct& cap )
{
	info.name( cap.deviceName );
	info.uid( cap.uniqueID );
}

// Get current camera information
OSStatus rfGetCameraInformation( void* cameraPtr,
																const CameraCapabilitiesStruct& capabilities,
																rcMovieFileCamExt& info )
{
	setProperties( info, capabilities );
	
	// Find capabilities by brute force search.
	vector<rcCameraProperty> props;
	int count = rfGetCameraProperties( cameraPtr, props );
	
	if ( count > 0 ) {
		for ( uint32 i = 0; i < props.size(); ++i ) {
			const rcCameraProperty& p = props[i];
			if ( p.active() ) {
				setProperty( info, p );
			}
		}
	}
	
	return noErr;
}




ostream& operator << ( ostream& os, const dcamCameraListStruct& cam )
{
	os << "Camera name: " << (char*)&cam.deviceName
	<< " MFGID: " << cam.manufacturerID << " UID: " << cam.uniqueID
	<< " InUse: " << (cam.isInUse? "1" : "0") << endl;
	
	return os;
}

ostream& operator << ( ostream& os, const ASC_CallbackStruct& cs )
{
	os << "Callback Width: " << cs.imageWidth << " Height: " << cs.imageHeight
	<< " Bytes: " << cs.imageBytes << " Depth: " << cs.imageBytes / (cs.imageWidth*cs.imageHeight)
	<< endl;
	
	return os;
}
