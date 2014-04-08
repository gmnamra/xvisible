/*
 *  rc_windowhist.h
 *  framebuf
 *
 *  Created by Peter Roberts on Tue May 14 2002.
 *  Copyright (c) 2002 Reify, Inc. All rights reserved.
 *
 */


#ifndef __rcWINDOWHIST_H__
#define __rcWINDOWHIST_H__


#include <vector>
using namespace std;

#include <rc_types.h>
#include <rc_window.h>

typedef vector<uint32> rc256BinHist;

// Generates a histogram for window src, in the given histogram buffer.
// Requires: Must be an 8 bit image.
// Returns: A reference to histogram.

rc256BinHist& rfGenDepth8Histogram(const rcWindow& src, rc256BinHist& histogram);

#endif
