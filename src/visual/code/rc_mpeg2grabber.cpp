/*
 *
 *$Id $
 *$Log: $
 *
 *
 * Copyright (c) 2007 Reify Corp. All rights reserved.
 */
#include <rc_platform.h>
#include <rc_mpeg2grabber.h>

// Creator name
static std::string cCreator ("rcMPEG2Grabber");

static rcMovieFileExpExt generateHeader();

//
// Local utilities
//

//
// rcMPEG2Grabber implementation
//

rcMPEG2Grabber::rcMPEG2Grabber( const std::string fileName, rcCarbonLock* cLock, double frameInterval ) :
  rcFileGrabber( cLock ),
  mSize ( (size_t)-1), mFileName( fileName ),
  mFrameCount(0), mFilePtr (NULL),
  mRefnum( 0 ),mCurrentIndex( 0 ), mGotFirstFrame( false ),
  mFrameInterval( frameInterval )
{
  lock();
    
  OSErr err;

  if ( mFileName.empty() )
    setLastError( eFrameErrorFileInit );
  else {
    mFilePtr = fopen (mFileName.c_str(), "rb");
    if (!mFilePtr)
      setLastError( eFrameErrorFileInit );
  }

  mAlphaPels.data = (void *) NULL;
  mAlphaPels.width =   mAlphaPels.height =   mAlphaPels.rowBytes = 0;

  unlock();
}


rcMPEG2Grabber::~rcMPEG2Grabber()
{
}
    
// Start grabbing
bool rcMPEG2Grabber::start()
{
  lock();

  int accel = 0;

#if defined(__ppc__) 
  accel |= MPEG2_ACCEL_PPC_ALTIVEC;
#elif defined(__i386__)
  accel |= MPEG2_ACCEL_X86_MMX;
  accel |= MPEG2_ACCEL_X86_MMXEXT;
#else
  rmAssert (1);
#endif
  mpeg2_accel(accel);
 
  mDecoder = mpeg2_init ();
  if (mDecoder == NULL)
    setLastError( eFrameErrorQuicktimeInit );
  else
    {
      mInfo = mpeg2_info (mDecoder);
      mpeg2_custom_fbuf (mDecoder, 1);
      rmAssert (mSize == (size_t)-1);
    }

  unlock();
    
  return ( getLastError() == eFrameErrorOK );
}

// Stop grabbing
bool rcMPEG2Grabber::stop()
{
  lock();
    
  mpeg2_close (mDecoder);

  unlock();
    
  return true;
}
    
// Returns the number of frames available
int32 rcMPEG2Grabber::frameCount()
{
  return mFrameCount;
}

 // Generate a reifymovie from this mpeg file
bool rcMPEG2Grabber::getReifyMovie ( const std::string& fName, const rcChannelConversion& conv, 
				     const movieFormatRev& rev, bool overWrite, const float& fInt, 
				     const int32 sample)
{
  lock();

  setLastError( eFrameErrorUnknown );
  rmAssert (sample > 0 && sample < 9999); // some sanity checks on sample

  struct fbuf_s * current_fbuf;
  const movieOriginType origin =  movieOriginConversionExt; 

  rcGenMovieFileError error = eGenMovieFileErrorOK;

  //@note the frame rate is set to 30 fps
  rcGenMovieFile movie(fName, origin,  conv, cCreator, rev, overWrite, fInt);
  rmAssert( movie.valid() ); // @note replace with throw

  // Add an experiment header
  rcMovieFileExpExt expHdr = generateHeader();
  expHdr.title( "MPEG2Grabber" );

  do
    {
      mState = mpeg2_parse (mDecoder);
      switch (mState)
	{
	case STATE_BUFFER:
	  mSize = fread (mBuffer, 1, rcMPEG2Grabber::buffer_size, mFilePtr);
	  mpeg2_buffer (mDecoder, mBuffer, mBuffer + mSize);
	  break;
	case STATE_SEQUENCE:
	  mpeg2_convert (mDecoder, mpeg2convert_rgb24, NULL);
	  for (int32 i = 0; i < 3; i++)
	    {
	      fbuf[i].rgb[0] = (uint8_t *) malloc (3 * mInfo->sequence->width * mInfo->sequence->height);
	      fbuf[i].rgb[1] = fbuf[i].rgb[2] = NULL;
	      if (!fbuf[i].rgb[0])
		{
		  fprintf (stderr, "Could not allocate an output buffer.\n");
		  exit (1);
		}
	      fbuf[i].used = 0;
	    }
	  for (int32 i = 0; i < 2; i++)
	    {
	      current_fbuf = get_fbuf ();
	      mpeg2_set_buf (mDecoder, current_fbuf->rgb, current_fbuf);
	    }
	  break;
	case STATE_PICTURE:
	  current_fbuf = get_fbuf ();
	  mpeg2_set_buf (mDecoder, current_fbuf->rgb, current_fbuf);
	  break;
	case STATE_SLICE:
	case STATE_END:
	case STATE_INVALID_END:
	  if (mInfo->display_fbuf)
	    {
	      if (! ( frameCount() % sample ) )
		movie.addFrame ( add_frame (mInfo->sequence->width, mInfo->sequence->height, mInfo->display_fbuf->buf[0]));
	      mFrameCount++;
	    }

	  if (mInfo->discard_fbuf)
	    ((struct fbuf_s *)mInfo->discard_fbuf->id)->used = 0;
	  if (mState != STATE_SLICE)
	    for (int32 i = 0; i < 3; i++)
	      free (fbuf[i].rgb[0]);
	  break;
	default:
	  break;
	}
    } while (mSize);
                
  setLastError( eFrameErrorOK );
  // Add header and flush
  movie.addHeader( expHdr );
  error = movie.flush();
  unlock();
  return ( error == eGenMovieFileErrorOK );
}


// Get name of input source, ie. file name, camera name etc.
const std::string rcMPEG2Grabber::getInputSourceName()
{
  return mFileName;
}

//
// Private methods
//
void rcMPEG2Grabber::save_ppm (int width, int height, uint8_t * buf, int num)
{
    char filename[100];
    FILE * ppmfile;

    sprintf (filename, "%d.ppm", num);
    ppmfile = fopen (filename, "wb");
    if (!ppmfile) {
	fprintf (stderr, "Could not open file \"%s\".\n", filename);
	exit (1);
    }
    fprintf (ppmfile, "P6\n%d %d\n255\n", width, height);
    fwrite (buf, 3 * width, height, ppmfile);
    fclose (ppmfile);
}

#define cRGB(r,g,b)  ((0xff << 24) | ((r) << 16) | ((g) << 8) | (b))

rcWindow rcMPEG2Grabber::add_frame (int width, int height, uint8_t * data24)
{
  const int32 bytesInPixel = 3;
  rcWindow frame (width, height, rcPixel32);
  for (int32 j = 0; j < height; j++)
    {
      uint32 *fourbytepels = (uint32 *) frame.rowPointer (j);
      for (int32 i = 0; i < width; i++, data24+=bytesInPixel)
      {
#ifdef __ppc__	
	*fourbytepels++ = rfRgb (data24[0],data24[1],data24[2]);
#else
	uint32 tmp = rfRgb (data24[2],data24[1],data24[0]);
	*fourbytepels++  = ( tmp & 0xff000000 ) | ( tmp & 0xff ) << 16 |
	  ( tmp & 0xff00 ) | ( tmp & 0xff0000 ) >> 16;
#endif
      }
    }
  return frame;
}

#undef cRGB

bool rcMPEG2Grabber::isFormatYv12 ()
{
  if (mInfo && mInfo->sequence->width >> 1 == mInfo->sequence->chroma_width &&
      mInfo->sequence->height >> 1 == mInfo->sequence->chroma_height)
    return true;
  return false;
}


bool rcMPEG2Grabber::isFormat422p ()
{
  if (mInfo && mInfo->sequence->width >> 1 == mInfo->sequence->chroma_width &&
      mInfo->sequence->height == mInfo->sequence->chroma_height)
    return true;
  return false;
}


// Generate experiment (assay) header for movie
static rcMovieFileExpExt generateHeader()
{
    // Generate an experiment header
    rcMovieFileExpExt expHdr;
    expHdr.lensMag( 1.0f );
    expHdr.otherMag( 0.0f );
    expHdr.temperature( 21.0f );
    expHdr.CO2( 0.035f );
    expHdr.O2( 21.0f );
    expHdr.userName( "MPEG2Grabber" );
    expHdr.imagingMode( "WhoKnows" );

    return expHdr;
}


rcFrameGrabberStatus rcMPEG2Grabber::getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking ) 
 { rmUnused (ptr); rmUnused (isBlocking); return eFrameStatusNoFrame; }
