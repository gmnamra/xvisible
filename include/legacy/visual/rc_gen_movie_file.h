/****************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *   File Name      rc_gen_movie_file.h
 *   Creation Date  09/10/2003
 *   Author         Peter Roberts
 *
 * Encapsulation of code needed to generate a movie from a series of
 * rcWindows.
 *
 ***************************************************************************/

#ifndef _rcGEN_MOVIE_FILE_H_
#define _rcGEN_MOVIE_FILE_H_

#include <rc_types.h>
#include <rc_window.h>
#include <rc_moviefileformat.h>
#include <rc_ipconvert.h>


enum rcGenMovieFileError {
  eGenMovieFileErrorOK = 0,
  eGenMovieFileErrorNull = 1,
  eGenMovieFileErrorExists = 2,
  eGenMovieFileErrorOpen = 3,
  eGenMovieFileErrorWrite = 4,
  eGenMovieFileErrorSeek = 5,
  eGenMovieFileErrorClose = 6,
  eGenMovieFileErrorFlush = 7
};

/* rcGenMovieFile - Creates a Reify format movie from a client specified
 * sequence of frames.
 */
class rcGenMovieFile
{
 public:

  /* ctor
   *
   * fName is output file name.
   *
   * orig is data origin.
   *
   * creator is human-redable data creator name
   *
   * conv is used for channel conversion if images are 32-bit
   *
   * fInt is frame interval in seconds. If argument is <= 0, then the
   * frame interval from the movie is used.
   *
   * If overErite is true, then overwriting of an existing file is
   * permitted.
   *
   * rev is file format revision to use for output file.
   */
    
    rcGenMovieFile( const std::string& fName );
    rcGenMovieFile( const std::string& fName, const movieOriginType& orig, const std::string& creator,
                    const movieFormatRev& rev, bool overWrite, const float& fInt );
    rcGenMovieFile( const std::string& fName, const rcChannelConversion& conv, 
                    const movieFormatRev& rev, bool overWrite, const float& fInt );
    rcGenMovieFile( const std::string& fName, const movieOriginType& orig,
                    const rcChannelConversion& conv, const std::string& creator,
                    const movieFormatRev& rev, bool overWrite,
                    const float& fInt );
    // dtor
    ~rcGenMovieFile();

    //
    // Accessors
    //
    const rcChannelConversion& conversion() const { return _conversion; };
    const std::string& fileName() const { return  _fileName; };
    rcIPair frameSize() const;

    // Is this instance valid
    bool valid() const;
    // Return last error
    rcGenMovieFileError lastError() const;

    //
    // Mutators
    //
    
  /* addFrame -Add frame to the end of the movie and write it to
   * disk immediately.
   *
   * Note: Only 8 bit frames and 32-bit are supported, and all frames must have
   * the same width/height. 32-bit frame channels are converted to 8-bit using _conversion.
   */
  rcGenMovieFileError addFrame(const rcWindow& frame);

  /* addFramePair -Add a pair frames to the end of the movie and write it to
   * disk immediately.
   *
   * Note: Only 8 bit frames as input are supported, 
   * 32-bit frame are written with the first frame as R G B. 
   * The second frame is written as alpha. 
   */
  rcGenMovieFileError addFrame (const rcWindow& frame, const rcWindow& alpha);

  /* Add a conversion header to list of movie headers. It will be written
     to disk by flush().
   */
  rcGenMovieFileError addHeader( const rcMovieFileConvExt& hdr );
  /* Add a camera header to list of movie headers. It will be written
     to disk by flush().
   */
  rcGenMovieFileError addHeader( const rcMovieFileCamExt& hdr );
  /* Add an origin header to list of movie headers. It will be written
     to disk by flush().
   */
  rcGenMovieFileError addHeader( const rcMovieFileOrgExt& hdr );
  /* Add an experiment header to list of movie headers. It will be written
     to disk by flush().
   */
  rcGenMovieFileError addHeader( const rcMovieFileExpExt& hdr );
  
  // Writes extension headers to disk and closes file.
  rcGenMovieFileError flush();

 private:
  // Open output stream
  rcGenMovieFileError open(const std::string& fName, bool overwrite);
  // Write one frame to stream
  rcGenMovieFileError write( const rcWindow& frame, const int64& timestamp );
  // Initialize movie header
  rcGenMovieFileError initHeader( const rcWindow& frame );
    
  std::string                   _fileName; // Output file name
  vector<int64>            _toc;      // TOC timestamp collection
  vector<rcMovieFileConvExt> _cnvHdrs;  // Conversion header collection
  vector<rcMovieFileCamExt>  _camHdrs;  // Camera header collection
  vector<rcMovieFileOrgExt>  _orgHdrs;  // Origin header collection
  vector<rcMovieFileExpExt>  _expHdrs;  // Experiment header collection
   
  movieOriginType       _origin;        // Data origin
  rcChannelConversion   _conversion;    // Channel conversion to use for 32-bit images
  rcMovieFileFormat     _movieHdr;      // Movie header
  rcMovieFileFormat2    _movieHdr2;     // Movie header rev2 
  std::string              _creator;       // Movie creator name
  movieFormatRev        _rev;           // Movie format file revision

  FILE*               _saveStream;    // Output stream
  rcGenMovieFileError _lastError;     // Last operation error
  bool                _overWrite;     // Overwrite file if it exists already
  bool                _flushed;       // Has stream been flushed
  float               _frameInterval; // Frame interval in seconds, if < 0 values come from rcWindows
  bool                _verbose;       // Diagnostic output verbosity
  rcPixel        _inputDepth;    // Input frame depth
};

#endif
