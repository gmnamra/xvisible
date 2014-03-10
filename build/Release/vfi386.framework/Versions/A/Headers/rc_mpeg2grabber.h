/*
 *  rc_mpeg2grabber.h
 *$Id $
 *$Log: $
 *
 * Copyright (c) 2007 Reify Corp. All rights reserved.
 */

#ifndef _rcMPEG2GRABBER_H_
#define _rcMPEG2GRABBER_H_


#include <vector>
#include <rc_mpeg2.h>
#include <rc_framegrabber.h>
#include <rc_filegrabber.h>
#include <rc_gen_movie_file.h>

using namespace std;

//
// Class to grab frames from a movie file
//

class rcMPEG2Grabber : public rcFileGrabber {
public:
  static const int32 buffer_size = 4096;

  // ctor
  rcMPEG2Grabber( const std::string fileName,   // Input file
		  rcCarbonLock* lock,        // Carbon locker
		  double frameInterval = 0.0 // Forced frame interval in seconds (calls setTimestamp())
		  );
  // virtual dtor
  virtual ~rcMPEG2Grabber();

  //
  // rcFrameGrabber API
  //

  // Start grabbing
  virtual bool start();

  // Stop grabbing
  virtual bool stop();
    
  // Returns the number of frames available
  virtual int32 frameCount();

  // Movie grabbers don't have a cache.
  virtual int32 cacheSize() { return 0; }

  virtual rcFrameGrabberStatus getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking );
 
  // Generate a reifymovie from this mpeg file
  bool getReifyMovie ( const std::string& fName, const rcChannelConversion& conv, 
		       const movieFormatRev& rev, bool overWrite, const float& fInt, const int32 sample = 1 );

  // Get name of input source, ie. file name, camera name etc.
  virtual const std::string getInputSourceName();
    
private:
  struct fbuf_s {
    uint8_t * rgb[4];
    int used;
  } fbuf[3];

  struct fbuf_s * get_fbuf (void)
  {
    int i;
    for (i = 0; i < 3; i++)
      if (!fbuf[i].used) {
	fbuf[i].used = 1;
	return fbuf + i;
      }
	  
    setLastError( eFrameErrorFileInit );
    return NULL;
  }

  rcWindow add_frame (int width, int height, uint8_t * buf);
  void save_ppm (int width, int height, uint8_t * buf, int num);
  // Count movie frames
  int32 countFrames();
  bool isFormatYv12 ();
  bool isFormat422p ();

  uint8_t                 mBuffer[buffer_size];
  mpeg2dec_t *            mDecoder;
  const mpeg2_info_t *    mInfo;
  mpeg2_state_t           mState;
  size_t                  mSize;
  vImage_Buffer           mAlphaPels;
  
  const std::string          mFileName;
  FILE*                   mFilePtr;
  int32                 mFrameCount;       // Number of frames in a movie
  short                   mRefnum;           // Movie ref num
  int32                 mCurrentIndex;     // Current index within movie
  bool                    mGotFirstFrame;    // true after first frame captured
  double                  mFrameInterval;    // Used to force a fixed frame interval
};

#endif // _rcMPEG2GRABBER_H_

