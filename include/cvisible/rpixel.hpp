#ifndef __PIXELHPP
#define __PIXELHPP
#include <iostream>
#include <iterator>
#include <map>
#include <algorithm>
#include <limits>
#include <assert.h>
#include "static.hpp"
#include <opencv2/core/core.hpp>
#include "config.h"
#include "vf_utils.hpp"


//
//static ostream& operator << ( ostream& os, const rpixel& rpe )
//{
//    string str = (rpe == rpixel8) ? " 8Bit Pixel " :
//    (rpe == rpixel16) ? "16Bit Pixel " :
//    (rpe == rpixel32S) ? "32Bit Pixel " : "Pixel Unknown";
//    os << str;
//    return os;
//}

typedef int64 pixel_ipl_t;

class pixel_type_base
{
    public:
    virtual pixel_ipl_t depth () = 0;
    virtual bool is_floating_point () = 0;
    virtual bool is_integral () = 0;
    virtual bool is_signed () = 0;
    virtual size_t minimum () = 0;
    virtual size_t maximum () = 0;
    virtual int planes () = 0;
    virtual int components () = 0;
    virtual  int bytes () = 0;
    virtual int bits () = 0;
};

typedef std::shared_ptr<pixel_type_base> pixel_type_base_ref;
typedef std::weak_ptr<pixel_type_base> pixel_type_weaker;

template<typename T, pixel_ipl_t IPL, int P, int C>
class pixel_trait_base : public pixel_type_base
{
public:
    typedef T value_type;
    static const pixel_ipl_t depth_ipl_id = IPL;
     bool is_floating_point () { return ! boost::is_integral<T>::value;  }
     bool is_integral () { return boost::is_integral<T>::value; }
     bool is_signed () { return boost::is_signed<T>::value; }
     size_t minimum () { return numeric_limits<T>::min(); }
     size_t maximum () { return numeric_limits<T>::max(); }
     int planes () { return  P; }
     int components () { return C; }
    virtual pixel_ipl_t depth () override { return IPL; }
     int bytes () { return sizeof(value_type) * components (); }
     int bits () { return bytes ()* 8 ; }
};

template
class pixel_trait_base<uint8, IPL_DEPTH_8U, 1, 1>;
template
class pixel_trait_base<int8, IPL_DEPTH_8S, 1, 1>;
template
class pixel_trait_base<uint16, IPL_DEPTH_16U, 1, 1>;
template
class pixel_trait_base<int16, IPL_DEPTH_16S, 1, 1>;
template
class pixel_trait_base<int32, IPL_DEPTH_32S, 1, 1>;
template
class pixel_trait_base<float, IPL_DEPTH_32F, 1, 1>;



class pixel8U : public pixel_trait_base<uint8, IPL_DEPTH_8U, 1, 1> {};
class pixel8S : public pixel_trait_base<int8, IPL_DEPTH_8S, 1, 1> {};
class pixel16U : public pixel_trait_base<uint16, IPL_DEPTH_16U, 1, 1> {};
class pixel16S : public pixel_trait_base<int16, IPL_DEPTH_16S, 1, 1> {};
class pixel32S : public pixel_trait_base<int32, IPL_DEPTH_32S, 1, 1> {};
class pixel32F : public pixel_trait_base<float, IPL_DEPTH_32F, 1, 1> {};

typedef std::map<pixel_ipl_t, pixel_type_base_ref> pixel_type_map;

class pixel_type_registry : public vf_utils::singleton <pixel_type_registry>
{
public:
    
  pixel_type_registry ()
    {
        p_m[pixel8U::depth_ipl_id] =  pixel_type_base_ref (new pixel8U);
        p_m[pixel8S::depth_ipl_id]  = pixel_type_base_ref (new pixel8S);
        p_m[pixel16U::depth_ipl_id]  = pixel_type_base_ref (new pixel16U);
        p_m[pixel16S::depth_ipl_id]  = pixel_type_base_ref (new pixel16S);
        p_m[pixel32S::depth_ipl_id]  = pixel_type_base_ref (new pixel32S);
        p_m[pixel32F::depth_ipl_id]  = pixel_type_base_ref (new pixel32F);
        
        p_t.push_back(IPL_DEPTH_8U);
        p_t.push_back(IPL_DEPTH_8S);
        p_t.push_back(IPL_DEPTH_16U);
        p_t.push_back(IPL_DEPTH_16S);
        p_t.push_back(IPL_DEPTH_32S);
        p_t.push_back(IPL_DEPTH_32F);
        
        self_test ();
    }
    
    const pixel_type_weaker get_weaker (pixel_ipl_t iplt) const
    {
        pixel_type_map::const_iterator aIt = p_m.find( iplt );
        pixel_type_weaker return_weaker;
        if (aIt != p_m.end()) return_weaker = aIt->second;
        return return_weaker;
 
    }
    
private:
    pixel_type_map p_m;
    std::vector<pixel_ipl_t> p_t;
    
    void self_test ()
    {
        std::for_each (p_t.begin(), p_t.end(), [this] (pixel_ipl_t& ipt)
                       {
                           const pixel_type_weaker wp = this->get_weaker (ipt);
                           assert (wp.lock());
                           std::shared_ptr<pixel_type_base> sp = wp.lock ();
                           assert(sp->depth() == ipt);
                       }
                       );
    }
};



//@todo 24 bit / 3 planes
//class rpixel8U3 : public pixel_trait_base<uint8, IPL_DEPTH_8U, 3, 1> {};
//class rpixel8U3 : public pixel_trait_base<uint8, IPL_DEPTH_8U, 1, 3> {};


template<typename P> pixel_ipl_t get_depth () { return P::depth(); }
template<typename P> int get_bytes () { return P::bytes(); }


const pixel_ipl_t rpixel8 = pixel8U::depth_ipl_id;
const pixel_ipl_t rpixel16 = pixel16U::depth_ipl_id;
const pixel_ipl_t rpixel32 = pixel32F::depth_ipl_id;
const pixel_ipl_t rpixel_unknown = 0;





//
// RGB component handling functions
//
// Color component (r,g,b) value range is 0-255

#define rfAlpha(rgb) 	(((rgb) >> 24) & 0xff)
#define rfRed(rgb) 	(((rgb) >> 16) & 0xff)
#define rfGreen(rgb) 	(((rgb) >> 8) & 0xff)
#define rfBlue(rgb) 	((rgb) & 0xff)
#define rfRgb(r,g,b)  ((0xff << 24) | (((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))

//inline uint32 rfGray( uint32 r, uint32 g, uint32 b ) { return (r+g+b+2)/3; }




#endif // __PIXELHPP


