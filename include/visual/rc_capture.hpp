/* Copyright 2002-2004 Reify, Inc.
 *
 * Generic, camera related definitions
 */

#ifndef _rcCAPTURE_H_
#define _rcCAPTURE_H_

#include <map>
#include <iostream>
#include <singleton.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/foreach.hpp>

#include <rc_vector2d.h>
#include <rc_dcam.h>



/* The "actual" number of frames that are put out per second by a
 * camera that has an output format basedon RS170. The two firewire
 * cameras that we use both seem to have done that. RS170 is documented
 * as getting 29.97 fps. Quicktime indicates something closer to the
 * given value. The reason for this discrepency should be investigated.
 */
	//#define rcRS170_FRAMES_PER_SEC 29.957058
	// For now make user interface easier to explain by just using 30
#define rcRS170_FRAMES_PER_SEC 30.0
#define rcDEFAULT_FRAMES_PER_SEC (rcRS170_FRAMES_PER_SEC/4)


using namespace boost;

	// Might be constructed from a initialization file.
class dcam_formats 
{
private:
	std::vector<rcCameraFormat> _format0, _format1, _format2, _format7;
	
public:
	
	dcam_formats ()
	{
		_format0.push_back(rcCameraFormat( 160, 120, 0, 0, rcCameraColorFormatYUV444, 63ul));
		_format0.push_back(rcCameraFormat( 320, 240, 0, 1, rcCameraColorFormatYUV422, 255ul));
		_format0.push_back(rcCameraFormat( 640, 480, 0, 2, rcCameraColorFormatYUV411, 255ul));
		_format0.push_back(rcCameraFormat( 640, 480, 0, 3, rcCameraColorFormatYUV422, 255ul));
		_format0.push_back(rcCameraFormat( 640, 480, 0, 4, rcCameraColorFormatRGB24, 255ul));
		_format0.push_back(rcCameraFormat( 640, 480, 0, 5, rcCameraColorFormatGray8, 255ul));
		_format0.push_back(rcCameraFormat( 640, 480, 0, 6, rcCameraColorFormatGray16, 255ul));
		_format0.push_back(rcCameraFormat( 640, 480, 0, 6, rcCameraColorFormatGray16, 255ul));			
		_format1.push_back(rcCameraFormat( 800, 600, 1, 0, rcCameraColorFormatYUV422, 127ul));
		_format1.push_back(rcCameraFormat( 800, 600, 1, 1, rcCameraColorFormatRGB24, 63ul));
		_format1.push_back(rcCameraFormat( 800, 600, 1, 2, rcCameraColorFormatGray8, 63ul));
		_format1.push_back(rcCameraFormat( 1024,768, 1, 3, rcCameraColorFormatYUV422, 255ul));
		_format1.push_back(rcCameraFormat( 1024,768, 1, 4, rcCameraColorFormatRGB24, 255ul));
		_format1.push_back(rcCameraFormat( 1024,768, 1, 5, rcCameraColorFormatGray8, 255ul));
		_format1.push_back(rcCameraFormat( 800, 600, 1, 6, rcCameraColorFormatGray16, 255ul));
		_format1.push_back(rcCameraFormat( 1024,768, 1, 7, rcCameraColorFormatGray16, 254ul));
		_format2.push_back(rcCameraFormat( 1280, 960, 2, 0, rcCameraColorFormatYUV422, 252ul));
		_format2.push_back(rcCameraFormat( 1280, 960, 2, 1, rcCameraColorFormatRGB24, 252ul));
		_format2.push_back(rcCameraFormat( 1280, 960, 2, 2, rcCameraColorFormatGray8, 254ul));
		_format2.push_back(rcCameraFormat( 1600,1200, 2, 3, rcCameraColorFormatYUV422, 252ul));
		_format2.push_back(rcCameraFormat( 1600,1200, 2, 4, rcCameraColorFormatRGB24, 248ul));
		_format2.push_back(rcCameraFormat( 1600,1200, 2, 5, rcCameraColorFormatGray8, 254ul));
		_format2.push_back(rcCameraFormat( 1280, 960, 2, 6, rcCameraColorFormatGray16, 252ul));
		_format2.push_back(rcCameraFormat( 1600,1200, 2, 7, rcCameraColorFormatGray16, 252ul));
		_format7.push_back(rcCameraFormat( 1280, 960, 7, 0, rcCameraColorFormatGray8, 16ul));
		_format7.push_back(rcCameraFormat( 640, 476, 7, 1, rcCameraColorFormatGray8, 0ul, 54.0f, 2, 2));
		_format7.push_back(rcCameraFormat( 640, 476, 7, 1, rcCameraColorFormatGray8, 0ul, 49.0f, 1, 2));	
	}
	
	int get_supported_formats (const CameraCapabilitiesStruct& cap, std::vector<rcCameraFormat>& format)
	{
		format.clear ();
		if ( cap.format2 )
		{
			
			BOOST_ASSERT (_format2.size() == ( sizeof (cap.format2Modes) / sizeof(Boolean)));
			for ( int i = _format2.size() - 1; i >= 0; --i )
				if ( cap.format2Modes[i] )
					format.push_back( _format2[i] );
		}
		if ( cap.format1 )
		{
			BOOST_ASSERT (_format1.size() == ( sizeof (cap.format1Modes) / sizeof(Boolean)));
			if ( cap.format1 )
				for ( int i = _format1.size() - 1; i >= 0; --i )											
					if ( cap.format1Modes[i] )
						format.push_back( _format1[i] );
		}
		if ( cap.format0 )
		{
			BOOST_ASSERT (_format0.size() == ( sizeof (cap.format0Modes) / sizeof(Boolean)));
			if ( cap.format0 )
				for ( int i = _format0.size() - 1; i >= 0; --i )											
					if ( cap.format0Modes[i] )
						format.push_back( _format0[i] );
		}
		if ( cap.format7 )
		{
				// If we have format 7. This is camera specific. So we need it from some place @todo
			int num = sizeof (cap.format7Modes) / sizeof(Boolean);
			for (int i = 0; i < num; i++)
				if ( cap.format7Modes[i] )
					format.push_back( _format7[i] );
		}
		
		return format.size();
		
	}
	
		// Display utilities
	
	ostream& displayFormat( ostream& os, 		 uint16 format, uint16 mode )
	{
		switch( format )
		{
			case 0:
				os << _format0[mode] << _format0[mode].frames_per_second() << std::endl;
				break;
				
			case 1:
				os << _format1[mode] << _format1[mode].frames_per_second() << std::endl;
				break;
				
			case 2:
				os << _format2[mode] << _format2[mode].frames_per_second() << std::endl;
				break;
				
			case 7:
				os << _format2[mode] << _format2[mode].frames_per_second() << std::endl;					
				break;
			default:
				os << " Unrecognized format and mode " << std::endl;
		}
		
		os << endl;
		
		return os;
	}
	
};

static dcam_formats sDCAMformats;


class dcam_camera_closer
{
public:
	void operator () (void* cameraPtr)
	{
		if (cameraPtr != 0)
		{
			ASCDCAMCloseACamera (cameraPtr);
		}
	}
};

	//
	// Class that contains camera information that is not
	// available dynamically. Specification values have to be
	// gathered from external sources such as manufacturer documentation.
	//

typedef std::vector<rcCameraProperty, std::allocator<rcCameraProperty> > dcam_properties;

class rcCameraSpecification
{
public:
	
		// ctor/dtor
	rcCameraSpecification( const dcam_properties & id, const rc2Fvector& s ) :
	mId( id ), mSensorElementSize( s ) {}
	rcCameraSpecification() : mSensorElementSize(-1.0f, -1.0f) {}
	~rcCameraSpecification() {}
	
		// Accessors
	const rc2Fvector& sensorElementSize() const { return mSensorElementSize; }
	const dcam_properties & id() const { return mId; }
	bool valid() const { return ! mId.empty(); }
	
		// Operators
	bool operator < ( const rcCameraSpecification& o ) const
	{
		return (mId.size() == o.id().size() && mSensorElementSize.len() < o.sensorElementSize().len() );
	}
	
	bool operator == ( const rcCameraSpecification& o ) const
	{
		return (mId.size() == o.id().size() && mSensorElementSize == o.sensorElementSize());
	}
	bool operator != ( const rcCameraSpecification& o ) const {
		return ! (*this == o);
	}    
	
		// TODO: add mappings between raw Firewire register values and physical units
		// ie. shutter control numeric value <-> exposure time in milliseconds
private:
	dcam_properties mId;                // Camera Properties 
	rc2Fvector mSensorElementSize; // Sensor element dimensions in micrometers
};

	//
	// Camera id-specification map
	//

class rcCameraMapper : Singleton<rcCameraMapper>
{
public:
		//
		// Camera specification mapper
		//
		// Bimap with key access on left view, key access
		// history on right view, and associated value.
	typedef int dummy_type;
	typedef boost::bimaps::bimap<
	UInt32,
	rcCameraProperty
	> property_map_type;
	
	typedef boost::bimaps::bimap<
	UInt32,
	std::vector<rcCameraFormat>
	> format_map_type;
	
	typedef property_map_type::value_type property;
	typedef format_map_type::value_type format;	
	
		// ctor
	rcCameraMapper()
	{
	}
	
	const property_map_type& propertyMap () const { return _CameraPropertyMap; }
	const format_map_type& formatMap () const { return _CameraFormatMap; }
	
	void Fill_dcam_info ()
	{
		OSStatus										err;
		dcamCameraList							theCameraList;
		
		if (ASCDCAMGetListOfCameras(&theCameraList) != 0 && theCameraList.numCamerasFound == 0) return;
		for ( uint32 i = 0; i < theCameraList.numCamerasFound; ++i )
		{
			dcamCameraListStruct camera = theCameraList.theCameras[i];
			boost::shared_ptr<void> cameraPtr = boost::shared_ptr<void>(ASCDCAMOpenACamera(camera.uniqueID), dcam_camera_closer() );
			if ( cameraPtr )
			{
				CameraCapabilitiesStruct theProperties;				
				if (ASCDCAMGetCameraProperties(get_pointer(cameraPtr), &theProperties) != 0) continue;
				dump_capabilities (std::cout, theProperties);
				
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_BRIGHTNESS_REG, MANUAL_CNTL) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_EXPOSURE_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_SHARPNESS_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_WHITE_BAL_REG, U_OR_B_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_WHITE_BAL_REG, V_OR_R_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_HUE_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_SATURATION_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_GAIN_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_GAMMA_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_SHUTTER_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_IRIS_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_FOCUS_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_TEMPERATURE_REG, TARGET_TEMP_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_TEMPERATURE_REG, CURR_TEMP_CNTL ) ) );
#ifdef notyet
					// TODO: Sony cameras return an error for this, investigate
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_TRIGGER_REG, TRIG_POLARITY_CNTL ) ) );
#endif
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_TRIGGER_REG, TRIG_MODE_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_TRIGGER_REG, TRIG_PARAM_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_ZOOM_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_PAN_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_TILT_REG, MANUAL_CNTL ) ) );
				_CameraPropertyMap.insert ( property ( camera.uniqueID, GetCameraProperty( get_pointer(cameraPtr), ASC_OPTICAL_FILTER_REG, MANUAL_CNTL ) ) );
				
					// Check Formats supported
				std::vector<rcCameraFormat> _supported_formats;
				int num_formats = sDCAMformats.get_supported_formats (theProperties, _supported_formats );
				std::cerr << num_formats << " Found \n";
				_CameraFormatMap.insert ( format ( camera.uniqueID, _supported_formats) );
				
				
			}
		}
		
		dump (std::cerr);
	}
	
	
		// Get format specification
	const rcCameraFormat get_format ( UInt32 format, UInt32 mode )
	{
		rcCameraFormat f;
		
	}
	
	const std::vector<rcCameraFormat>& get_all_supported_formats ()
	{
			//		return _supported_formats;
	}
	
	
	
		// Map size
	const uint32 size() const { return _CameraPropertyMap.size(); }
	
	ostream& dump (ostream& os)
	{
		property_map_type::left_map m = _CameraPropertyMap.left;
		for ( property_map_type::left_map::const_iterator iter(m.begin()),
			 iend(m.end());
			 iter != iend;
			 ++iter )
		{
			os << iter->first << "-->" << iter->second
			<< std::endl;
		}
		
		for ( format_map_type::const_iterator iter(_CameraFormatMap.begin()),
			 iend(_CameraFormatMap.end());
			 iter != iend;
			 ++iter )
		{
			os << "Camera " << iter->left << " < ---- > ";
			const std::vector<rcCameraFormat>& vec = iter->right;
			std::vector<rcCameraFormat>::const_iterator formatItr = vec.begin();
			for (; formatItr < vec.end(); formatItr++)
			{
				os << "Format: " << (int) formatItr->format() << " Mode: " << (int) formatItr->mode() << std::endl;
			}
		}
		
		return os;
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
	
private:
	property_map_type  _CameraPropertyMap;
	format_map_type _CameraFormatMap;
		
		// Get one proprety from camera
	static OSStatus getOneProperty( void* cameraPtr, UInt32 type, UInt32 cntl, UInt32* value )
	{
		OSStatus err = ASCDCAMGetPropertyValue( cameraPtr,
											   type,
											   cntl,
											   value );
		
		return err;
	}
	
	
		// Get one cntl property
	rcCameraProperty GetCameraProperty( void* cameraPtr, UInt32 type, UInt32 cntl )
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
	
	
	static void dump_capabilities ( ostream& os, const CameraCapabilitiesStruct& cap )
	{
		os << "Camera name: " << (char*)&cap.deviceName << " UID: " <<  cap.uniqueID
		<< endl;
		dcam_formats df;
		
		os << "Formats:" << endl;
		
		if ( cap.format0 ) {
			for ( int i = 0; i < 7; ++i )
				if ( cap.format0Modes[i] )
					os << "Format 0: " << " Mode : " << i << std::endl;
		}
		if ( cap.format1 ) {
			for ( int i = 0; i < 7; ++i )
				if ( cap.format1Modes[i] )
					os << "Format 1: " << " Mode : " << i << std::endl;					
		}
		if ( cap.format2 ) {
			for ( int i = 0; i < 7; ++i )
				if ( cap.format2Modes[i] )
					os << "Format 2: " << " Mode : " << i << std::endl;					
		}
		
		if ( cap.format7 ) {
			os << "Format 7:" << endl;
				// If we have format 7. This is camera specific. So we need it from some place @todo
			int num = sizeof (cap.format7Modes) / sizeof(bool);
			for (int i = 0; i < num; i++)
				if ( cap.format7Modes[i] )
					os << "Format 2: " << " Mode : " << i << std::endl;										
			
			os << endl;
		}
		
		os << "Controls:" << endl;
		if ( cap.hasBright )
			os << "Brightness ";
		if ( cap.hasExpos )
			os << "Exposure ";
		if ( cap.hasSharp )
			os << "Sharpness ";
		if ( cap.hasWhiteB )
			os << "WhiteBalance ";
		if ( cap.hasHue )
			os << "Hue ";
		if ( cap.hasSat )
			os << "Saturation ";
		if ( cap.hasGain )
			os << "Gain ";
		if ( cap.hasGamma )
			os << "Gamma ";
		if ( cap.hasShutter )
			os << "Shutter ";
		if ( cap.hasIris )
			os << "Iris ";
		if ( cap.hasFocus )
			os << "Focus ";
		if ( cap.hasTemp )
			os << "Tempr ";
		if ( cap.hasTrigger )
			os << "Trigger ";
		if ( cap.hasZoom )
			os << "Zoom ";
		if ( cap.hasPan )
			os << "Pan ";
		if ( cap.hasTilt )
			os << "Tilt ";
		if ( cap.hasOpticalFilter )
			os << "Filter ";
		
		os << endl;
		
	}
	
	
	
};


	// Display utilities
ostream& operator << ( ostream& os, const rcCameraSpecification& c );


#endif // #ifndef _rcCAPTURE_H_
