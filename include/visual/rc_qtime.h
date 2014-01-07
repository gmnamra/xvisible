/*
 *  rc_qtime.h
 *  framebuf
 *
 *  Created by Arman Garakani on Sat May 18 2002.
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcQTIME_H_
#define _rcQTIME_H_

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <QuickTime/ImageCompression.h> // for image loading and decompression
#include <QuickTime/QuickTimeComponents.h> // for file type support
#include <QuickTime/ImageCompression.h>
#include <ApplicationServices/ApplicationServices.h>

#include <rc_window.h>

#include <vector>
using namespace std;

//
// Image Utilities
//

// Image Import Functions
// NOTE: these are obsolete, use rcFrameGrabber API instead

// Returns a vector of rcWindows corresponding to the image files
OSErr rfImageFileToRcWindow (vector <rcWindow>& images, vector <std::string>& fileNames);
// Returns a vector of rcWindows corresponding to the frames read from a movie file
OSErr rfMovieFileToRcWindow (vector <rcWindow>& images, const std::string& fileName);

//
// QuickTime utilities
//

void getPixelInfoFromImageDesc (ImageDescriptionHandle anImageDesc, rcPixel& pd,
                                uint32& pixelFormat, bool& isGray, bool& isWhiteReversed,
                                CTabHandle *ctb);
OSErr getNextVideoSample( Movie theMovie, rcWindow& image, Media media, TimeValue fromTimePoint,
                          bool reduceGrayTo8bit = FALSE );

//
// Image processing utilities
//

// Fill rcFrame color map using QuickTime color table values
void rfFillColorMap( const CTabHandle ctb, rcSharedFrameBufPtr& frame, bool reverse = false );

//
// File handling utilities
//

// Returns a sequence number (if any) from image name (ie. 2 for image002.tif)
int rfImageFrameNum (char * const fileName);
// Sorts a vector of filenames by their sequence number (ie. "image001.tif, image002.tif")
void rfImageNameSort( vector <std::string>& fileNames );
// Check validity of FSSpec
OSErr ValidFSSpec(const FSSpec *spec);
// Given a Posix file path, return a correspondng FSSpec
FSSpec rfMakeFSSpecFromPosixPath(const char* file_name, bool isDirectory);

#endif // _rcQTIME_H_
