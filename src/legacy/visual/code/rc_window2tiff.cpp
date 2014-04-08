/* $Id: rc_window2tiff.cpp 7297 2011-03-07 00:17:55Z arman $
 *
 * Project:  libtiff tools
 * Purpose:  Convert raw byte sequences in TIFF images
 * Author:   Andrey Kiselev, dron@remotesensing.org
 *
 ******************************************************************************
 * Copyright (c) 2002, Andrey Kiselev <dron@remotesensing.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include "tif_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <ctype.h>
#include <rc_window.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#if HAVE_IO_H
# include <io.h>
#endif

#include "tiffio.h"

#ifndef HAVE_GETOPT
extern int getopt(int, char**, char*);
#endif

#ifndef O_BINARY
# define O_BINARY 0
#endif

typedef enum {
	PIXEL,
	BAND
} InterleavingType;

static	uint16 compression = (uint16) COMPRESSION_NONE;
static	int jpegcolormode = JPEGCOLORMODE_RGB;
static	int quality = 75;		/* JPEG quality */
static	uint16 predictor = 0;

static	int processCompressOptions(char*);


bool rcWindow::tiff (std::string& outfile, bool doCompress) const // int argc, char* argv[])
{
  uint32	_width = width(), length = height(), linebytes;
  uint32	nbands = 1;		    /* number of bands in input image*/
  off_t	hdr_size = 0;		    /* size of the header to skip */
  TIFFDataType dtype = TIFF_BYTE;
  int16	_depth = 1;		    /* bytes per pixel in input image */
  InterleavingType interleaving = PIXEL;  /* interleaving type flag */
  uint32  rowsperstrip = (uint32) 1;
  uint16	photometric = PHOTOMETRIC_MINISBLACK;
  uint16	config = PLANARCONFIG_CONTIG;
  uint16	fillorder = FILLORDER_MSB2LSB;
  int	fd;
  TIFF	*out;

  uint32 row;
  unsigned char *buf = NULL;

  switch (depth())
    {
    case rcPixel8:
      dtype = TIFF_BYTE;
      _depth = 1;
      break;
    case rcPixel16:
      dtype = TIFF_SHORT;
      _depth = 2;
      break;
    case rcPixel32:
      dtype = isD32Float() ? TIFF_FLOAT : TIFF_LONG;
      _depth = 4;
      break;
    case rcPixelDouble:
      dtype = TIFF_DOUBLE;
      _depth = 8;
      break;
    default:
    case rcPixelUnknown:
      return false;
    }

  // Photometric needs to be handle
  out = TIFFOpen(outfile.c_str(), "wb");
  if (out == NULL)
    {
      cerr << outfile << "Cannot open file for output. " << endl;
      return 0;
    }
  TIFFSetField(out, TIFFTAG_IMAGEWIDTH, _width);
  TIFFSetField(out, TIFFTAG_IMAGELENGTH, length);
  TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, nbands);
  TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, _depth * 8);
  TIFFSetField(out, TIFFTAG_FILLORDER, fillorder);
  TIFFSetField(out, TIFFTAG_PLANARCONFIG, config);
  TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photometric);
  switch (dtype) {
  case TIFF_BYTE:
  case TIFF_SHORT:
  case TIFF_LONG:
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
    break;
  case TIFF_SBYTE:
  case TIFF_SSHORT:
  case TIFF_SLONG:
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
    break;
  case TIFF_FLOAT:
  case TIFF_DOUBLE:
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    break;
  default:
    TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_VOID);
    break;
  }

  if (doCompress) compression = (uint16) -1;

  if (compression == (uint16) -1)
    compression = COMPRESSION_PACKBITS;
  TIFFSetField(out, TIFFTAG_COMPRESSION, compression);
  switch (compression) {
  case COMPRESSION_JPEG:
    if (photometric == PHOTOMETRIC_RGB
	&& jpegcolormode == JPEGCOLORMODE_RGB)
      photometric = PHOTOMETRIC_YCBCR;
    TIFFSetField(out, TIFFTAG_JPEGQUALITY, quality);
    TIFFSetField(out, TIFFTAG_JPEGCOLORMODE, jpegcolormode);
    break;
  case COMPRESSION_LZW:
  case COMPRESSION_DEFLATE:
    if (predictor != 0)
      TIFFSetField(out, TIFFTAG_PREDICTOR, predictor);
    break;
  }
  switch(interleaving) {
  case BAND:				/* band interleaved data */
    linebytes = _width * _depth;
    buf = (unsigned char *)_TIFFmalloc(linebytes);
    break;
  case PIXEL:				/* pixel interleaved data */
  default:
    linebytes = _width * nbands * _depth;
    break;
  }

  TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, rowsperstrip));
  lseek(fd, hdr_size, SEEK_SET);		/* Skip the file header */
  for (row = 0; row < length; row++)
    {
      if (TIFFWriteScanline(out,  (unsigned char *) rowPointer (row), row, 0) < 0)
	{
	  cerr << outfile << ": scanline" << row << ": Write error." << endl;
	  break;
	}
    }

  if (buf)
    _TIFFfree(buf);
  TIFFClose(out);

  return true;
}

#if 0
static int
processCompressOptions(char* opt)
{
	if (strcmp(opt, "none") == 0)
		compression = COMPRESSION_NONE;
	else if (strcmp(opt, "packbits") == 0)
		compression = COMPRESSION_PACKBITS;
	else if (strncmp(opt, "jpeg", 4) == 0) {
		char* cp = strchr(opt, ':');
		if (cp && isdigit((int)cp[1]))
			quality = atoi(cp+1);
		if (cp && strchr(cp, 'r'))
			jpegcolormode = JPEGCOLORMODE_RAW;
		compression = COMPRESSION_JPEG;
	} else if (strncmp(opt, "lzw", 3) == 0) {
		char* cp = strchr(opt, ':');
		if (cp)
			predictor = atoi(cp+1);
		compression = COMPRESSION_LZW;
	} else if (strncmp(opt, "zip", 3) == 0) {
		char* cp = strchr(opt, ':');
		if (cp)
			predictor = atoi(cp+1);
		compression = COMPRESSION_DEFLATE;
	} else
		return (0);
	return (1);
}

#endif
