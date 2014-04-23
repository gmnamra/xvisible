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
#include "rc_types.h"
#include "rc_pair.h"
#include <cstdio>
#include <list>
#include <iterator>
#include <stdexcept>
#include <deque>
#include <vector>
using namespace std;



extern int alphasort ();

bool RFY_API rf_ext_is_rfymov (const std::string& filename);
bool RFY_API rf_ext_is_mov (const std::string& filename);
bool RFY_API rf_ext_is_stk (const std::string& filename);

void RFY_API rfGetDirEntries(const string & dirname, 
		     vector<std::string> & entries, const char *imageformat = "tif");

void RFY_API rfGetFilesFromSeparatedList (const std::string& listed_files, vector<std::string> & entries, int frame_count, int frame_index, const char token = ';' );

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
std::string RFY_API rfGetTempFile (std::string foo = std::string ());

bool RFY_API rf_sensitive_case_compare (const std::string& str1, const std::string& str2);
bool RFY_API rf_insensitive_case_compare (const std::string& str1, const std::string& str2);

static const char *defaultBase = "zoubin";

std::string RFY_API rfDumpMatrix (deque<deque<double> >& matrix, const char *pathFormat = 0,
		       const char* baseName = defaultBase);


namespace csv
{
   typedef std::vector<std::string> row_type;
   typedef std::vector<row_type> rows_type;
        
   //! Convert an input stream to csv rows.
   RFY_API rows_type to_rows(std::istream &input);
        
} //namespace csv


#define __rcFILEUTILS_H

#endif /* __rcFILEUTILS_H */
