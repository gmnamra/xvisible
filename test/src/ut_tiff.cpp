/*
 *
 *$Id $
 *$Log$
 *Revision 1.2  2005/10/27 22:58:19  arman
 *z additions and cleanup
 *
 *Revision 1.1  2005/10/25 22:38:08  arman
 *in progress
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "ut_tiff.h"
#include <rc_window.h>
#include <unistd.h> 
#include <rc_fileutils.h>
#include <rc_imagegrabber.h>
#include <rc_ipconvert.h>

UT_tiff::UT_tiff(std::string& basepath)
  : mBasename (basepath)
{
  std::string zser24 ("zser24.stk");
  std::string zser16 ("zser24.stk");
  std::string zser8 ("zser24.stk");
  std::string gal2wfilter1_t1 ("gal2wfilter1_t1.STK");

  mTiffFiles.push_back (zser8);
  mNumFrames.push_back (11);
  mTiffFiles.push_back (zser16);
  mNumFrames.push_back (11);
  mTiffFiles.push_back (zser24);
  mNumFrames.push_back (11);
  mTiffFiles.push_back (gal2wfilter1_t1);
  mNumFrames.push_back (5);

}

UT_tiff::~UT_tiff()
{
  printSuccessMessage( "rc Tiff test", mErrors );
}

uint32 UT_tiff::run()
{
  // Test outputing tiff files
  {
    rcWindow tmp (32, 80);
    tmp.randomFill ();
    std::string fname ("tmp.tiff");
    rcUTCheck (tmp.tiff (fname) == true);
    unlink (fname.c_str ());
  }

  for (uint32 i = 0; i < mTiffFiles.size(); i++)
    {
      std::string fname = mBasename + mTiffFiles[i];
		uint32 frames = mNumFrames[i];
		testFile (fname, frames);
    }
    return 1;
}

  uint32 UT_tiff::testFile (std::string& fname, uint32 frames)
  {
    TIFFImageIO t_importer;
    t_importer.CanReadFile (fname.c_str ());
    t_importer.SetFileName (fname.c_str ());
    t_importer.ReadImageInformation ();
    vector<rcWindow> zstk;
    zstk = t_importer.ReadPages ();
	
    rcUTCheck (zstk.size () == frames);
	
    // Individual frames are stored in a directory with stk name
    // Create a files list and use an image grabber to get rcWindows. 
    vector<std::string> files;
    std::string baseName = rfStripExtension(fname);
    std::string dname = rfStripPath(fname);
    dname = rfStripExtension (dname);
	
    for (uint32 i = 0; i < frames; i++)
      {
	std::ostringstream strm;
	strm << setw (4) << setfill('0') << i;
	std::string zname = baseName + std::string ("/") + dname + std::string (strm.str ()) + std::string (".tif");
	files.push_back (zname);
	cerr << zname << endl;
      }

    rcImageGrabber grabber (files, NULL, 1.0);
    vector<rcWindow> images;

    for( int32 i = 0; ; ++i )
      {
	rcSharedFrameBufPtr framePtr;
	rcFrameGrabberStatus status = grabber.getNextFrame( framePtr, true );
	if ( status != eFrameStatusOK )
	  {
	    break;
	  }
	rcWindow image ( framePtr );
	if (image.depth() == rcPixel16)
	  {
	    image = rfImageConvert168 (image, rcSelectAll);
	  }
	images.push_back (image);
      }

    rcUTCheck (images.size() == frames);
	
    for (uint32 i = 0; i < frames; i++)
      {
	rcUTCheck (images[i].contentCompare (zstk[i], true));
      }
	
    return mErrors;
  }

