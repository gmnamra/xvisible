/*
 *
 * $Id: rc_featuredefs.h 7211 2011-02-17 17:15:01Z arman $
 *
 * This file contains feature conditionalization definitions.
 *
 * Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcFEATUREDEFS_H_
#define _rcFEATUREDEFS_H_

// Symbols to enable various features and options in Visible

/******************************************************************************
 *	Analysis features
 ******************************************************************************/

// Muscle length analysis
#define rcMUSCLE_ANALYSIS 1

// Write TOC extension header for captured movies
#define rcWRITE_TOC 1
// Write CAM extension header for captured movies
#define rcWRITE_CAM 1
// Write EXP extension header for captured movies
#define rcWRITE_EXP 1

/******************************************************************************
 *	Debugging options
 ******************************************************************************/

// Engine state debugging output
//#define STATE_CHG_DEBUG 1

// Make this true to enable some developer-only options
extern bool gDeveloperDebugging;

// Enable simulated camera input
//#define rcSIMULATED_CAMERA 1

#endif // _rcFEATURESDEFS_H_

