/*
 *  rc_assemblyfcts_mac.cpp
 *  
 *
 *  Created by Peter Roberts
 *  Copyright (c) Reify Corp., 2002. All rights reserved.
 *
 */
#include "rc_assemblymacros.h"

/* Place for assembly line functions that cannot be used directly from rc_assemblymacros.h
 * (because, for example, they need to be used in templates).
 */
 

void genSync()
{
  __genSync();
}

