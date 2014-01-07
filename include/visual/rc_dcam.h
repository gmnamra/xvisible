/*
 *
 * $Id: rc_dcam.h 7284 2011-03-01 23:32:47Z arman $
 *
 * This file contains ASC DCAM support code.
 *
 * Copyright (c) 2003-2004 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcDCAM_H_
#define _rcDCAM_H_

#include <iostream>
#include <vector>
#include <ASC_DCAM_DEV/ASC_DCAM_API.h>

#include <rc_types.h>
#include <rc_framebuf.h>
#include <rc_vector2d.h>
#include <boost/tuple/tuple.hpp>
#include <boost/dynamic_bitset.hpp>

using namespace std;
class rcMovieFileCamExt;
using namespace boost;
std::string dcam_cntlName( int type );
static std::string dcam_propertyName( int type );

class rcCameraMapper;

// Firewire camera color format
enum rcCameraColorFormat
{
		
	
	rcCameraColorFormatUnknown = 0,
	rcCameraColorFormatYUV444,
	rcCameraColorFormatYUV422,
	rcCameraColorFormatYUV411,
	rcCameraColorFormatRGB24,
	rcCameraColorFormatGray8,
	rcCameraColorFormatGray16
};

// Camera format/mode combination class

class rcCameraFormat : boost::tuple<rcIPair,uint8,uint8,rcCameraColorFormat,unsigned long,uint8,uint8,float>
{
public:
	rcCameraFormat()
	{
		}
	
	rcCameraFormat( uint16 width,
								 uint16 height,
								 uint8  format,
								 uint8  mode,
								rcCameraColorFormat color, unsigned long dynamic_bitset_ulong,
								 float specific_fps = 0.0f, uint8 bin_x = 1, uint8 bin_y = 1)
	:
		boost::tuple<rcIPair,uint8,uint8,rcCameraColorFormat,unsigned long,uint8,uint8,float>  (boost::make_tuple(rcIPair (width,height),format, mode , color, dynamic_bitset_ulong, specific_fps, bin_x, bin_y))
	{
	}
	// Accessors
	const int32& width() const { return get<0>().first; }
	const int32& height() const { return get<0>().second; }
	const uint8&  format() const { return get<1>(); }
	const uint8&  mode() const { return get<2>(); }
	const rcCameraColorFormat& color() const { return get<3>(); }
	const float frames_per_second ()
	{
		dynamic_bitset<>  maskfps (8,get<4>());
		if (maskfps.none())
			return get<5>();
		else 
		{
			float maxfps = 0.0f;
			for (int i = 0; i < 8; i++)
				if (maskfps[i])
				{
					float tmp = frames_per_second (i);
					if (tmp > maxfps)
						maxfps = tmp;
				}
			return maxfps;
		}
	}
	
		// Operators
	bool operator < ( const rcCameraFormat& o ) const
	{
		return format() < o.format();
	}
	
	const uint8& bin_x () const { return get<6>(); }
	const uint8& bin_y () const { return get<7>(); }	
	
	
	rcPixel depth() const
	{
		switch ( get<3>() )
		{
			case rcCameraColorFormatYUV444:
			case rcCameraColorFormatYUV422:
			case rcCameraColorFormatYUV411:
			case rcCameraColorFormatRGB24:
				return rcPixel32;
			case rcCameraColorFormatGray8:
				return rcPixel8;
			case rcCameraColorFormatGray16:
				return rcPixel16;
			default:
				return rcPixelUnknown;
		}
	}
	
	bool isMono() const 
	{
		switch ( get<3>() ) 
		{
			case rcCameraColorFormatGray8:
			case rcCameraColorFormatGray16:
				return true;
			default:
				return false;
		}
	}
	
	bool isMono8 () const
	{
		switch ( get<3>() ) 
		{
			case rcCameraColorFormatGray8:
				return true;
			default:
				return false;
		}
	}

	friend ostream& operator << ( ostream& os, const rcCameraFormat& c )
	{
		os << "format " << int(c.format()) << " mode " << int(c.mode())
		<<  " " << c.width() << "x" << c.height() << "x" << c.depth()*8
		<< " " << c.color();
		
		return os;
	}
	
	float frames_per_second (int i)
	{
		return 1.875 * ( 1 << i );
	}
	

	private:
	
};

// Camera (control) property class

class rcCameraProperty : boost::tuple< boost::tuple<UInt32,UInt32,UInt32,UInt32,UInt32>, Boolean,Boolean,Boolean,Boolean,Boolean,bool>
{
public:
	rcCameraProperty() 	{};
	rcCameraProperty( UInt32 type, UInt32 cntl )
	:
		boost::tuple< boost::tuple<UInt32,UInt32,UInt32,UInt32,UInt32>, Boolean,Boolean,Boolean,Boolean,Boolean,bool> (make_tuple (make_tuple(type,cntl,(UInt32) -1, UInt32(-1), UInt32(-1)), false,false,false,false,false,false) )
	{	}
	
	// Accessors
	const UInt32& type() const { return get<0>().get<0>(); }
	const UInt32& cntl() const { return get<0>().get<1>(); }
	const UInt32& minValue() const { return get<0>().get<2>(); }
	const UInt32& maxValue() const { return get<0>().get<3>(); }
	const UInt32& curValue() const { return get<0>().get<4>(); }
	
	const Boolean& canOnePush() const { return get<1>(); }
	const Boolean& canReadOut() const { return get<2>(); }
	const Boolean& canOnOff() const { return get<3>(); }
	const Boolean& canAuto() const { return get<4>(); }
	const Boolean& canManual() const { return get<5>(); }
	const bool& active() const    { return get<6>(); }
	
	// Operators
	bool operator < ( const rcCameraProperty& o ) const
	{
		return type() < o.type();
	}
	
	friend ostream& operator << (ostream& os, const rcCameraProperty& c);
	
	// Mutators
	
	 UInt32& type()  { return get<0>().get<0>(); }
	 UInt32& cntl()  { return get<0>().get<1>(); }
	 UInt32& minValue()  { return get<0>().get<2>(); }
	 UInt32& maxValue()  { return get<0>().get<3>(); }
	 UInt32& curValue()  { return get<0>().get<4>(); }
	
	 Boolean& canOnePush()  { return get<1>(); }
	 Boolean& canReadOut()  { return get<2>(); }
	 Boolean& canOnOff()  { return get<3>(); }
	 Boolean& canAuto()  { return get<4>(); }
	 Boolean& canManual()  { return get<5>(); }
	 bool& active()     { return get<6>(); }
	
};



// Camera query utilities

// Get one cntl property
rcCameraProperty rfGetCameraProperty( void* cameraPtr,
																		 UInt32 type,
																		 UInt32 cntl );
// Populate a vector of cntl properties
int rfGetCameraProperties( void* cameraPtr,
													vector<rcCameraProperty>& properties );
// Get maxmimum frame rate
double rfGetCameraMaxFrameRate( void* cameraPtr );
// Get true camera capabilities 
OSStatus rfGetCameraCapabilities( void* cameraPtr,
																 CameraCapabilitiesStruct& capabilities );
// Get current camera information 
OSStatus rfGetCameraInformation( void* cameraPtr,
																const CameraCapabilitiesStruct& capabilities,
																rcMovieFileCamExt& info );

// Display utilities
ostream& operator << ( ostream& os, const dcamCameraListStruct& cam );
ostream& operator << ( ostream& os, const CameraCapabilitiesStruct& cap );





#endif //  _rcDCAM_H_
