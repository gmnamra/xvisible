/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.2  2005/10/12 22:04:17  arman
 *incremental
 *
 *Revision 1.1  2005/04/29 21:58:33  arman
 *basic unix based file/directory utils
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcFILEUTILS_H

#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/dir.h>
#include <vector>
#include <deque>
#include <rc_types.h>
#include <rc_pair.h>
#include<cstdio>
#include<list>
#include<iterator>
#include<stdexcept>

extern "C" {
	#include<glob.h>
}

using namespace std;



extern int alphasort ();

void RFY_API rfGetDirEntries(const string & dirname, 
		     vector<std::string> & entries, const char *imageformat = "tif");

void RFY_API rfSortImageFileNames(const vector<std::string> & files, vector<std::string> & entries, const char *imageformat);

bool RFY_API rfIsDirectory (const char * inFullPath);
bool RFY_API rfIsFile (const char * inFullPath);
int32 RFY_API rfGetFileSize (const char *inFullPath);

bool RFY_API rfFileExists(const std::string& filename);

std::string RFY_API rfStripPath( const std::string& fileName );
std::string RFY_API rfStripExtension( const std::string& fileName );
std::string RFY_API rfGetExtension( const std::string& fileName );

std::string RFY_API rfMakeTmpFileName(const char *pathFormat, const char* baseName );
std::string RFY_API rfMakeTmpFileName(const char* baseName );

static const char *defaultBase = "zoubin";

std::string rfDumpMatrix (deque<deque<double> >& matrix, const char *pathFormat = 0,
		       const char* baseName = defaultBase);

#define __rcFILEUTILS_H

#endif /* __rcFILEUTILS_H */
