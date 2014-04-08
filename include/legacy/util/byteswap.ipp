
#ifndef _UTILS_BYTESWAP_IPP
#define _UTILS_BYTESWAP_IPP
#define qInv_INLINE         __forceinline

/***********************************************************************
 * Platform-specific implementation details for byteswap below:
 **********************************************************************/
#if defined(BOOST_MSVC) //http://msdn.microsoft.com/en-us/library/a3140177%28VS.80%29.aspx
    #include <cstdlib>

    qInv_INLINE boost::uint16_t qInv::byteswap(boost::uint16_t x){
        return _byteswap_ushort(x);
    }

    qInv_INLINE boost::uint32_t qInv::byteswap(boost::uint32_t x){
        return _byteswap_ulong(x);
    }

    qInv_INLINE boost::uint64_t qInv::byteswap(boost::uint64_t x){
        return _byteswap_uint64(x);
    }

#elif defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 2

    qInv_INLINE boost::uint16_t qInv::byteswap(boost::uint16_t x){
        return (x>>8) | (x<<8); //DNE return __builtin_bswap16(x);
    }

    qInv_INLINE boost::uint32_t qInv::byteswap(boost::uint32_t x){
        return __builtin_bswap32(x);
    }

    qInv_INLINE boost::uint64_t qInv::byteswap(boost::uint64_t x){
        return __builtin_bswap64(x);
    }

#elif defined(qInv_PLATFORM_MACOS)
    #include <libkern/OSByteOrder.h>

    qInv_INLINE boost::uint16_t qInv::byteswap(boost::uint16_t x){
        return OSSwapInt16(x);
    }

    qInv_INLINE boost::uint32_t qInv::byteswap(boost::uint32_t x){
        return OSSwapInt32(x);
    }

    qInv_INLINE boost::uint64_t qInv::byteswap(boost::uint64_t x){
        return OSSwapInt64(x);
    }

#elif defined(qInv_PLATFORM_LINUX)
    #include <byteswap.h>

    qInv_INLINE boost::uint16_t qInv::byteswap(boost::uint16_t x){
        return bswap_16(x);
    }

    qInv_INLINE boost::uint32_t qInv::byteswap(boost::uint32_t x){
        return bswap_32(x);
    }

    qInv_INLINE boost::uint64_t qInv::byteswap(boost::uint64_t x){
        return bswap_64(x);
    }

#else 

    qInv_INLINE boost::uint16_t qInv::byteswap(boost::uint16_t x){
        return (x>>8) | (x<<8);
    }

    qInv_INLINE boost::uint32_t qInv::byteswap(boost::uint32_t x){
        return (boost::uint32_t(qInv::byteswap(boost::uint16_t(x&0xfffful)))<<16) | (qInv::byteswap(boost::uint16_t(x>>16)));
    }

    qInv_INLINE boost::uint64_t qInv::byteswap(boost::uint64_t x){
        return (boost::uint64_t(qInv::byteswap(boost::uint32_t(x&0xffffffffull)))<<32) | (qInv::byteswap(boost::uint32_t(x>>32)));
    }

#endif



#endif /* _UTILS_BYTESWAP_IPP */
