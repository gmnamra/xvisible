// Copyright 2002 Reify, Inc.

/* This file defines image processing utility fcts required by the system.
 */

#ifndef _rcIMAGEPROCESSING_H_
#define _rcIMAGEPROCESSING_H_

#include <rc_window.h>

/* Generate a version of an 8 bit, grey scale src image that is
 * half-res in x and y. Odd pixels/rows will be ignored. In other
 * words, a source image that is 5 by 5 will result in a 2 by 2, not a
 * 3 by 3, destination image. The row update for the generated image
 * will always be widthInPixels/2.
 */
extern void rcGenHalfRes(char* src, char* dest, uint32 widthInPixels,
			 uint32 heightInLines, uint32 srcRowUpdate,
			 bool invert);

void rfAndImage(const rcWindow& srcWin, const rcWindow& maskWin,
		rcWindow& destWin);

void rfTemporalMedian (const vector<rcWindow>& srcWin, rcWindow& destWin);


#endif
