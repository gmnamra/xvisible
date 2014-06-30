
#ifndef _UTILS_ByteSWAP_HPP
#define _UTILS_ByteSWAP_HPP


#include <boost/cstdint.hpp>

/*! \file byteswap.hpp
 * Provide fast byteswaping routines for 16, 32, and 64 bit integers,
 * by using the system's native routines/intrinsics when available.
 */



    //! perform a byteswap on a 16 bit integer
    boost::uint16_t byteswap(boost::uint16_t);

    //! perform a byteswap on a 32 bit integer
    boost::uint32_t byteswap(boost::uint32_t);

    //! perform a byteswap on a 64 bit integer
    boost::uint64_t byteswap(boost::uint64_t);


    union big_endian_int16
       {
       boost::uint8_t ub_ [sizeof(boost::uint16_t)];
       boost::uint16_t us_;
       boost::int16_t s_;

       boost::int16_t int16_val ()
          {
          return (int16_t) byteswap(us_);
          }

       };


 

#include <util/byteswap.ipp>

#endif /* _UTILS_ByteSWAP_HPP */
