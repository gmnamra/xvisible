// Copyright 2003 Reify, Inc.

#ifndef __rcSYSTEMINFO_H__
#define __rcSYSTEMINFO_H__

#include <rc_types.h>

/* Amount of ram, in bytes.
 */
std::string rfGetHostInfo();
double rfSystemRam ();

// Platform byte order
reByteOrder rfPlatformByteOrder();
// Platform byte order mark character (same as Unicode BOM)
uint16 rfPlatformByteOrderMark();
// reByteOrder rfPlatform byte order from BOM
reByteOrder rfPlatformByteOrder( const uint16& bom );

#endif
