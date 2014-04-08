#ifndef PIXELHPP
#define PIXELHPP
#include <iostream>
#include <iterator>
#include <map>
#include <algorithm>
#include <limits>
#include <assert.h>
#include <OpenCV/cxtypes.h>
#include <rc_types.h>
#include <boost/type_traits.hpp>



	template <typename T> struct IPL_ID;


	template <> struct IPL_ID<uint8> { 	enum { value = IPL_DEPTH_8U }; typedef uint8 traits_value; };
	template <> struct IPL_ID<uint16> { 	enum { value = IPL_DEPTH_16U }; typedef uint16 traits_value;  };
	template <> struct IPL_ID<uint32> { 	enum { value = IPL_DEPTH_32S }; typedef uint32 traits_value;  };
	template <> struct IPL_ID<float> { 	enum { value = IPL_DEPTH_32F }; typedef float traits_value; };
	template <> struct IPL_ID<double> { 	enum { value = IPL_DEPTH_64F }; typedef double traits_value; };

	template <typename T>
	struct PixelTraits
	{
		typedef T value_type;
		typedef T* ptr_type;
		const static bool IsFloatingPoint () { return boost::is_floating_point<T>::value; }
		const static bool IsIntegral ()  { return boost::is_integral<T>::value; }
		const static int Bytes () { return sizeof (T); }
		const static int Bits () { return sizeof (T) * 8; }
		const static T max () throw () { return numeric_limits<T>::max(); }
		const static T min () throw () { return numeric_limits<T>::min(); }
	};
	
	template <> struct PixelTraits<uint8>
	{
		enum { ipl_constant = IPL_DEPTH_8U };
		enum { image_description_constant = 8 };
		enum { apple_constant = k8IndexedPixelFormat};
		enum { bits_per_channel = 8 };
		enum { channels_per_pixel = 1};
	};
	template <> struct PixelTraits<uint32>
	{
		enum { ipl_constant = IPL_DEPTH_32S };
		enum { image_description_constant = 32 };
		enum { apple_constant = k32ARGBPixelFormat };
		enum { bits_per_channel = 8 };
		enum { channels_per_pixel = 4};
	};
	
	template <> struct PixelTraits<float>
	{
		enum { ipl_constant = IPL_DEPTH_32F };
		enum { image_description_constant = 32 };
		enum { apple_constant = k32ARGBPixelFormat };
		enum { bits_per_channel = 32 };
		enum { channels_per_pixel = 1};
	};
	
	

	//
	// Pixel depth type. No support for 24 bit images for now.
	// Using IPL constants
	// IPL_DEPTH_<bit_depth>(S|U|F)
	// E.g.: IPL_DEPTH_8U means an  8-bit unsigned image.
	// IPL_DEPTH_32F means a 32-bit float image.
	//   IPL_DEPTH_8U, IPL_DEPTH_8S,
	//   IPL_DEPTH_16U,IPL_DEPTH_16S,
	//   IPL_DEPTH_32S,IPL_DEPTH_32F,
	//   IPL_DEPTH_64F
	// Using IPL constants
	// IPL_DEPTH_<bit_depth>(S|U|F)
	// E.g.: IPL_DEPTH_8U means an  8-bit unsigned image.
	// IPL_DEPTH_32F means a 32-bit float image.

	enum rcPixel
	{
		rcPixel8 = IPL_DEPTH_8U,
		rcPixel16 = IPL_DEPTH_16U,
		rcPixel32 = IPL_DEPTH_32S,
		rcPixelFloat = IPL_DEPTH_32F,
		rcPixelDouble = IPL_DEPTH_64F,
		rcPixelUnknown = 0
	};



		// Map QuickTime ImageDescription Depth to our types
	    // what depth is this data (1-32) or ( 33-40 grayscale )
		// From an a quickTime imagedescription we can figure out a few things about the frame in the image:
		// depth: Contains the pixel depth specified for the compressed image:
		// Values of 1, 2, 4, 8, 16, 24, and 32 indicate the depth of color images.
		// Values of 34, 36, and 40 indicate 2-bit, 4-bit, and 8-bit grayscale, respectively, for grayscale images.
		// For us the only acceptable images are 8, 16, 24, and 32 in color and 40 which is 8 bit grey.
		// We detect a gray image through size and equality of color mappings
		// TBD: what is the best way of handling 32 bit images? The issue here is how to treat alpha and also byte ordering.


	static ostream& operator << ( ostream& os, const rcPixel& rpe )
	{
		string str = (rpe == rcPixel8) ? " 8Bit Pixel " :
		(rpe == rcPixel16) ? "16Bit Pixel " :
		(rpe == rcPixel32) ? "32Bit Pixel " : "Pixel Unknown";
		os << str;
		return os;
	}


	typedef struct PixelTraits<uint8> rpPixel8;
	typedef struct PixelTraits<uint16> rpPixel16;
	typedef struct PixelTraits<uint32> rpPixel32;
	typedef struct PixelTraits<float> rpPixelFloat;
	typedef struct PixelTraits<double> rpPixelDouble;



#endif

