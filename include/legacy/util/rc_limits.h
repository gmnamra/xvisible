/*
 *
 *$Header $
 *$Id $
 *$Log: $
 *
 *
 * Copyright (c) 2006 Reify Corp. All rights reserved.
 */
#ifndef __RC_LIMITS_H
#define __RC_LIMITS_H

#include <limits.h>

#define rcUINT8_MAX    numeric_limits<unsigned char>::max()
#define rcUINT8_MIN    numeric_limits<unsigned char>::min()
#define rcUINT16_MAX   numeric_limits<unsigned short>::max()
#define rcUINT16_MIN    numeric_limits<unsigned short>::min()
#define rcUINT32_MAX   0xffffffff // numeric_limits<unsigned int>::max()
#define rcUINT32_MIN    numeric_limits<unsigned int>::min()

#define rcINT8_MAX    numeric_limits<signed char>::max()
#define rcINT8_MIN    numeric_limits<signed char>::min()
#define rcINT16_MAX   numeric_limits<signed short>::max()
#define rcINT16_MIN    numeric_limits<signed short>::min()
#define rcINT32_MAX   numeric_limits<signed int>::max()
#define rcINT32_MIN    numeric_limits<signed int>::min()

#define rcFLT_MIN  1.17549435e-38F // numeric_limits<float>::min()
#define rcFLT_MAX numeric_limits<float>::max()
#define rcFLT_EPSILON numeric_limits<float>::epsilon ()

#define rcDBL_MIN  2.2250738585072014e-308 // numeric_limits<double>::min()
#define rcDBL_MAX numeric_limits<double>::max()
#define rcDBL_EPSILON numeric_limits<double>::epsilon ()

#define rcUINT24_MAX	0xffffff
#define rcUINT64_MAX   0xffffffffffffffff

// Some other constants
#define rkPI (3.1415926535897932384626433832795)
#define rk2PI (2*rkPI)
#define rkRadian (rkPI / 180.0)


#endif /* __RC_LIMITS_H */
