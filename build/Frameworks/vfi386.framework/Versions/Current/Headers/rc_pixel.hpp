#ifndef PIXELHPP
#define PIXELHPP
#include <iostream>
#include <iterator>
#include <map>
#include <algorithm>
#include <limits>
#include <assert.h>
#include "rc_types.h"
#include "static.hpp"
#include <opencv2/core/core.hpp>


enum rcPixel
{
    rcPixel8 = IPL_DEPTH_8U,
    rcPixel8s = IPL_DEPTH_8S,    
    rcPixel16 = IPL_DEPTH_16U,
    rcPixel16s = IPL_DEPTH_16S,    
    rcPixel32S = IPL_DEPTH_32S,
    rcPixelFloat = IPL_DEPTH_32F,
    rcPixelDouble = IPL_DEPTH_64F,
    rcPixelUnknown = 0
};

struct pixel_bytes
{
    int count (rcPixel rp)
    {
        switch (rp)
        {
            case rcPixel8: case rcPixel8s: return 1; break;
            case rcPixel16: case rcPixel16s: return 2; break;
            case rcPixel32S: return 4; break;            
            case rcPixelFloat: return 4; break;
            case rcPixelDouble: return 8; break;
            default: return 0; break;
        }
        return 0;
    }
};


struct ipl_pixel_type_id
{
    int id (rcPixel rp)
    {
        return static_cast<int>(rp);
    }
};

SINGLETON_FCN(pixel_bytes,get_bytes);
SINGLETON_FCN(ipl_pixel_type_id,get_ipl);


struct rc_pixel_type_enum
{
    rcPixel em ( int rp)
    {
        if (rp == get_ipl().id (rcPixel8)) return rcPixel8;
        if (rp == get_ipl().id (rcPixel16)) return rcPixel16;        
        if (rp == get_ipl().id (rcPixel32S)) return rcPixel32S;  
        if (rp == get_ipl().id (rcPixelFloat)) return rcPixelFloat; 
        if (rp == get_ipl().id (rcPixelDouble)) return rcPixelDouble;         
        return rcPixelUnknown;        
    }
};

SINGLETON_FCN(rc_pixel_type_enum,get_pixel);

// Legacy rcPixelX values were equal to number of bytes 

struct rp_depth
{
  bool validate (int rp_int_val)
{
   if (rp_int_val == get_bytes().count(rcPixel8) ||
       rp_int_val == get_bytes().count(rcPixel16) ||
       rp_int_val == get_bytes().count(rcPixel32S) ||
       rp_int_val == get_ipl().id (rcPixel8) ||
       rp_int_val == get_ipl().id (rcPixel16) || 
       rp_int_val == get_ipl().id (rcPixel32S) ) return true;
    return false;
}
};

SINGLETON_FCN(rp_depth,rc_pixel);

struct rp_versions
{
     rcPixel update (int rp_int_val)
    {
        if (rp_int_val == get_bytes().count(rcPixel8)) return rcPixel8;
        if (rp_int_val == get_bytes().count(rcPixel16)) return rcPixel16;
        if (rp_int_val == get_bytes().count(rcPixel32S)) return rcPixel32S;
        if (rp_int_val == get_ipl().id (rcPixel8)) return rcPixel8;
        if (rp_int_val == get_ipl().id (rcPixel16)) return rcPixel16;
        if (rp_int_val == get_ipl().id (rcPixel32S)) return rcPixel32S;        
        return rcPixelUnknown;
    }
};

SINGLETON_FCN(rp_versions,get_new_version);




static ostream& operator << ( ostream& os, const rcPixel& rpe )
{
    string str = (rpe == rcPixel8) ? " 8Bit Pixel " :
    (rpe == rcPixel16) ? "16Bit Pixel " :
    (rpe == rcPixel32S) ? "32Bit Pixel " : "Pixel Unknown";
    os << str;
    return os;
}





#endif

