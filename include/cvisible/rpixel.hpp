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
#include <boost/functional/factory.hpp>

using namespace vf_utils;

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



const pixel_ipl_t rpixel8 = IPL_DEPTH_8U;
const pixel_ipl_t rpixel8s = IPL_DEPTH_8S;
const pixel_ipl_t rpixel16 = IPL_DEPTH_16U;
const pixel_ipl_t rpixel16s = IPL_DEPTH_16S;
const pixel_ipl_t rpixel32 = IPL_DEPTH_32F;
const pixel_ipl_t rpixel32s = IPL_DEPTH_32S;
const pixel_ipl_t rpixel_unknown = 0;


struct pixel_type_base
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
class pixel_trait_base : private pixel_type_base
{
public:
    typedef T value_type;
     bool is_floating_point () { return ! boost::is_integral<T>::value;  }
     bool is_integral () { return boost::is_integral<T>::value; }
     bool is_signed () { return boost::is_signed<T>::value; }
     size_t minimum () { return numeric_limits<T>::min(); }
     size_t maximum () { return numeric_limits<T>::max(); }
     int planes () { return  P; }
     int components () { return C; }
     pixel_ipl_t depth ()  { return IPL; }
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



typedef pixel_trait_base<uint8, IPL_DEPTH_8U, 1, 1> pixel8U;
typedef pixel_trait_base<int8, IPL_DEPTH_8S, 1, 1> pixel8S;
typedef pixel_trait_base<uint16, IPL_DEPTH_16U, 1, 1> pixel16U;
typedef pixel_trait_base<int16, IPL_DEPTH_16S, 1, 1> pixel16S;
typedef pixel_trait_base<int32, IPL_DEPTH_32S, 1, 1> pixel32S;
typedef pixel_trait_base<float, IPL_DEPTH_32F, 1, 1> pixel32F;

template <pixel_ipl_t P>
struct pixel_by_id;


template<> struct pixel_by_id<rpixel8> { typedef pixel8U pixel_type; const pixel_ipl_t id = IPL_DEPTH_8U; };
template<> struct pixel_by_id<rpixel8s> { typedef pixel8S pixel_type; const pixel_ipl_t id = IPL_DEPTH_8S; };
template<> struct pixel_by_id<rpixel16> { typedef pixel16U pixel_type; const pixel_ipl_t id = IPL_DEPTH_16U; };
template<> struct pixel_by_id<rpixel16s> { typedef pixel16S pixel_type; const pixel_ipl_t id = IPL_DEPTH_16S; };
template<> struct pixel_by_id<rpixel32s> { typedef pixel32S pixel_type; const pixel_ipl_t id = IPL_DEPTH_32S; };
template<> struct pixel_by_id<rpixel32> { typedef pixel32F pixel_type; const pixel_ipl_t id = IPL_DEPTH_32F; };

struct null_deleter
{
    void operator()(void const *) const
    {
    }
};


template <class T>
pixel_type_base_ref make_pixel_type ()
{
    static T only_one;
    return std::shared_ptr<struct pixel_type_base>  ((pixel_type_base*) &only_one, null_deleter());
}



typedef std::map<pixel_ipl_t, pixel_type_base_ref> pixel_type_map;

class pixel_type_registry : public vf_utils::singleton <pixel_type_registry>
{
public:
    
    pixel_type_registry ()
    {
        std::call_once (mInitialized_flag, &pixel_type_registry::initialize,this);
    }
    
  void initialize ()
    {
        p_m.clear (); p_t.clear ();
        
        p_m[rpixel8] =  make_pixel_type<pixel8U> ();
        p_m[rpixel8s] =  make_pixel_type<pixel8S> ();
        p_m[rpixel16] =  make_pixel_type<pixel16U> ();
        p_m[rpixel16s] =  make_pixel_type<pixel16S> ();
        p_m[rpixel32s] =  make_pixel_type<pixel32S> ();
        p_m[rpixel32] =  make_pixel_type<pixel32F> ();
        
            
        p_t.push_back(IPL_DEPTH_8U);
        p_t.push_back(IPL_DEPTH_8S);
        p_t.push_back(IPL_DEPTH_16U);
        p_t.push_back(IPL_DEPTH_16S);
        p_t.push_back(IPL_DEPTH_32S);
        p_t.push_back(IPL_DEPTH_32F);
        
        self_test ();
    }
    
    const pixel_type_base_ref get_shared (pixel_ipl_t iplt) const
    {
      
        
        pixel_type_map::const_iterator aIt = p_m.find( iplt );
        pixel_type_base_ref return_shared;
        if (aIt != p_m.end()) return_shared = aIt->second;
        return return_shared;
 
    }
    
private:
    pixel_type_map p_m;
    std::vector<pixel_ipl_t> p_t;
    
    // since it is a constexpr it will be statically iniialized (once_flag() noexcept )
    mutable std::once_flag mInitialized_flag;
    
    void self_test ()
    {
        std::for_each (p_t.begin(), p_t.end(), [this] (pixel_ipl_t& ipt)
                       {
                           const pixel_type_base_ref wp = this->get_shared (ipt);
                           std::shared_ptr<pixel_type_base> sp = wp;
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


