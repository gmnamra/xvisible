/* @file
 *  bcnvrt_main.cpp
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 * Quicktime specific code taken from Apple's QTMakeMovie demo code.
 *
 */

#include <QuickTime/QuickTime.h>

#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>
#include <rc_types.h>
#include <rc_qtime.h>
#include <rc_videocache.h>
#include <rc_framebuf.h>
#include <rc_moviegrabber.h>
#include <rc_imagegrabber.h>
#include <rc_gen_movie_file.h>
#include <rc_movieconverter.h>
#include <rc_fileutils.h>
#include <rc_ipconvert.h>
#include <rc_mpeg2grabber.h>

#define kVideoTimeScale 1000 // 1000 units per second
// Application info
const char* const cAppName = "Batchconvert";
const char* const cVersionName = "1.1";
const char* const cAppBuildDate = __DATE__ " " __TIME__ ;
// Creator name
static std::string cCreator;

// Verbosity
static bool verbose = true;
static const double cUpdateStatusInterval = 1.0;

// Video cache size 
static const uint32 cVideoCacheMem = 1024*1024*32;
static const uint32 cVideoCacheSize = 10;

// Defaults
#define defFPS 30
#define defFrameInterval 1/defFPS

/******************************************************************************
 * Progress indication utility class
 ******************************************************************************/

class rcConversionProgress: public rcProgressIndicator {
public:
  // ctor, specify updte interval in seconds
  rcConversionProgress( const std::string& message,
											 const rcTimestamp& updateInterval ) :
	mFormatMessage( message ),
	mUpdateInterval( updateInterval ), 
	mLastUpdate( cZeroTime ) { }
  // virtual dtor
  virtual ~rcConversionProgress() { };
	
  // Call this to update status bar with completion percentage
  virtual bool progress( double percentComplete ) {
    const rcTimestamp now = rcTimestamp::now();
    // If interval is zero, update with every call
    // Otherwise update after mUpdateInterval seconds have passed since last update
    if ( mUpdateInterval == cZeroTime || 
				( (now - mLastUpdate) > mUpdateInterval ) ) {
      mLastUpdate = now;
      snprintf( mMessageBuf, rmDim( mMessageBuf ), mFormatMessage.c_str(), percentComplete );
      // Display progress to stdout
      cout << mMessageBuf << endl;
    }
    return false;
  }
	
  virtual bool progress( uint32 percentComplete ) {
    return progress( double(percentComplete) );
  }
	
private:
  const std::string    mFormatMessage;
  char              mMessageBuf[1024];
  rcTimestamp       mUpdateInterval; // Update interval in seconds
  rcTimestamp       mLastUpdate;     // Last time progress was updated
};

static rcConversionProgress* createProgressIndicator( int32 frameCount,
																										 const char* message )
{
  const rcTimestamp updateInterval(cUpdateStatusInterval);
  char buf[1024];
  snprintf( buf, rmDim(buf), message,
					 frameCount );
  return new rcConversionProgress( buf, updateInterval );
}

// Compare two frames
static OSErr rfCompareFrames(rcSharedFrameBufPtr srcPtr,  rcSharedFrameBufPtr destPtr)
{
  if (srcPtr->width() != destPtr->width() ||
      srcPtr->height() != destPtr->height() ||
      srcPtr->depth() != destPtr->depth()) {
    cerr << "Width/Height/Depth mismatch: Src ("
		<< srcPtr->width() << "," << srcPtr->height() << ","
		<< srcPtr->depth() << ") Dest ("
		<< destPtr->width() << "," << destPtr->height() << ","
		<< destPtr->depth() << ")" << endl;
    return -1;
  }
	
  for (int32 x = 0; x < srcPtr->width(); x++) {
    for (int32 y = 0; y < srcPtr->height(); y++) {
      if (srcPtr->getPixel(x, y) != destPtr->getPixel(x, y)) {
				cerr << "Pixel mismatch at (" << x << "," << y << ") Src: " 
				<< srcPtr->getPixel(x, y)
				<< " Dest: " << destPtr->getPixel(x, y) << endl;
				return -1;
      }
    }
  }
	
  return noErr;
}

// Compare Reify movie to QT movie
static OSErr rfCompareMovies(char* srcFile, char* destFile,
                             bool srcReify, bool destReify,
                             rcChannelConversion conversion,
                             bool reversePixels,
                             bool progressUpdates )
{
  rmAssert( (srcReify + destReify) == 1 );
	
  rcConversionProgress* prog = 0;
	
  rcVideoCache* rfyCacheP = 
	rcVideoCache::rcVideoCacheCtor(srcFile, cVideoCacheSize, false, false, false, cVideoCacheMem);
	
  if (!rfyCacheP->isValid()) {
    cerr << "Open error: "
		<< rcVideoCache::getErrorString(rfyCacheP->getFatalError())
		<< endl;
    rcVideoCache::rcVideoCacheDtor(rfyCacheP);
    return -1;
  }
	
  std::string dirName (dirname(destFile));
  std::string destFullName;
  if (dirName == ".") {
    char* buffer = (char*)malloc(2048);
    if (!buffer || getcwd(buffer, 2048) == 0)
      return -1;
		
    destFullName = std::string(buffer) + std::string("/") + std::string(basename(destFile));
		
    free(buffer);
  }
  else
    destFullName = std::string(dirName) + std::string("/") + std::string(basename(destFile));
	
  rcMovieGrabber qtGrabber(destFullName, 0);
  
  if (!qtGrabber.isValid() || !qtGrabber.start())
    return -1;
	
  if ( progressUpdates ) {
    prog = createProgressIndicator( qtGrabber.frameCount(),
																	 "Comparing %i frames...%%.2f%%%% complete" );
  }
	
  rmAssert(qtGrabber.frameCount() <= (int32)rfyCacheP->frameCount());
  if (qtGrabber.frameCount() < (int32)rfyCacheP->frameCount())
    fprintf(stderr, "Warning: Quicktime movie smaller than Reify movie %d < %d.\n"
						"Comparing %d frames.\n", qtGrabber.frameCount(),
						rfyCacheP->frameCount(), qtGrabber.frameCount());
	
  rcSharedFrameBufPtr rfyPtr;
  rcSharedFrameBufPtr qtPtr;
  for (int32 fi = 0; fi < qtGrabber.frameCount(); fi++) {
    rcFrameGrabberStatus fStatus = qtGrabber.getNextFrame(qtPtr, true);
    rcVideoCacheStatus vStatus = rfyCacheP->getFrame(fi, rfyPtr, 0);
    if ((fStatus != eFrameStatusOK) ||
				(vStatus != eVideoCacheStatusOK)) {
      cerr << "Error: fStatus " << fStatus << " vStatus " << vStatus << endl;
      return -1;
    }
    // Convert from 32 to 8
    if ( qtPtr->depth() == rcPixel32 ) {
      rcWindow frame( qtPtr );
      rcWindow frame8(frame.width(), frame.height());
      rfRcWindow32to8(frame, frame8, conversion);
      qtPtr = frame8.frameBuf();
    } else if ( qtPtr->depth() == rcPixel8 ) {
      rcWindow image( qtPtr );
      if ( reversePixels ) {
				rfReversePixels8( image );
      }
    }
		
    OSErr err = noErr;
    if ( srcReify )
      err = rfCompareFrames(rfyPtr, qtPtr);
    else
      err = rfCompareFrames(qtPtr, rfyPtr);
    if ( err != noErr )
      return err;
		
    if ( prog )
      prog->progress( double(fi)/ qtGrabber.frameCount() * 100 );
  }
	
  delete prog;
	
  return 0;
}

// Compare two Reify movies
static OSErr rfCompareReifyMovies(char* srcFile, char* destFile,
                                  bool srcReify, bool destReify,
                                  bool progressUpdates )
{
  rmAssert( srcReify && destReify );
	
  rcConversionProgress* prog = 0;
	
  rcVideoCache* rfyCacheP = 
	rcVideoCache::rcVideoCacheCtor(srcFile, cVideoCacheSize, false, false, false, cVideoCacheMem);
  rcVideoCache* rfyDestCacheP = 
	rcVideoCache::rcVideoCacheCtor(destFile, cVideoCacheSize, false, false, false, cVideoCacheMem);
	
  if (!rfyCacheP->isValid()) {
    cerr << "Open error: "
		<< rcVideoCache::getErrorString(rfyCacheP->getFatalError())
		<< endl;
    rcVideoCache::rcVideoCacheDtor(rfyCacheP);
    return -1;
  }
  if (!rfyDestCacheP->isValid()) {
    cerr << "Open error: "
		<< rcVideoCache::getErrorString(rfyDestCacheP->getFatalError())
		<< endl;
    rcVideoCache::rcVideoCacheDtor(rfyDestCacheP);
    return -1;
  }
	
  if ( progressUpdates ) {
    prog = createProgressIndicator( rfyDestCacheP->frameCount(),
																	 "Comparing %i frames...%%.2f%%%% complete" );
  }
  if (rfyDestCacheP->frameCount() != rfyCacheP->frameCount())
    fprintf(stderr, "Warning: destination movie size %i differs from movie size %i.\n"
						"Comparing %d frames.\n", rfyDestCacheP->frameCount(),
						rfyCacheP->frameCount(), rfyDestCacheP->frameCount());
	
  rcSharedFrameBufPtr srcPtr;
  rcSharedFrameBufPtr destPtr;
  for (uint32 fi = 0; fi < rfyDestCacheP->frameCount(); fi++) {
    rcVideoCacheStatus srcStatus = rfyCacheP->getFrame(fi, srcPtr, 0);
    rcVideoCacheStatus destStatus = rfyDestCacheP->getFrame(fi, destPtr, 0);
    if ((srcStatus != eVideoCacheStatusOK) ||
				(destStatus != eVideoCacheStatusOK)) {
      cerr << "Error: srcStatus " << srcStatus << " destStatus " << destStatus << endl;
      return -1;
    }
    OSErr err = rfCompareFrames(srcPtr, destPtr);
    if ( err != noErr )
      return err;
    if ( prog )
      prog->progress( double(fi)/rfyDestCacheP->frameCount() * 100 );
  }
	
  delete prog;
	
  return 0;
}

// Convert Reify movie to non-Reify movie
static int rfConvertMovie(const std::string& movieFile,
                          rcMovieConverterError& error,
                          const float& frameInterval,
                          uint32 firstIndex, uint32 frameCount,
                          uint32 offset, uint32 period,
                          bool reversePixels,
                          const std::string& outputFile,
                          bool progressUpdates, bool dstIsReify )
{
  int count = 0;
	
  if ( !movieFile.empty() ) {
    rcMovieConverterOptionsQT opt;
    opt.frameInterval( frameInterval );
    opt.firstFrameIndex( firstIndex );
    opt.frameCount( frameCount );
    opt.frameOffset( offset );
    opt.samplePeriod( period );
    opt.reversePixels( reversePixels );
    opt.creator( cCreator );
    opt.verbose( verbose );
    rcConversionProgress* prog = 0;
		
    if ( progressUpdates ) {
      prog = createProgressIndicator( frameCount,
																		 "Converting %i frames to QuickTime format...%%.2f%%%% complete" );
    }
		
    if (!dstIsReify)
		{
			rcMovieConverterToQT converter( opt,
																		 NULL,   // No lock needed in a single-threaded non-GUI app
																		 prog );
			error = converter.convert( movieFile, outputFile );
		}
    else
		{
			rcMovieConverterToRfy converter( opt, NULL, prog );
			error = converter.convert( movieFile, outputFile );
		}
		
    if ( error != eMovieConverterErrorOK ) {
      cerr << rcMovieConverter::getErrorString( error ) << endl;
    } else
      count = frameCount;
    delete prog;
  }
	
  return count;
}

// Convert non-Reify movie to Reify movie
static int rfConvertTwoMovies(const std::string& movieFile,
															const std::string& movie2File,
															rcMovieConverterError& error,
															const float& frameInterval,
															uint32 firstIndex, uint32 frameCount,
															uint32 offset, uint32 period,
															bool reversePixels,
															rcChannelConversion conversion,
															movieFormatRev rev,
															const std::string& outputFile,
															bool progressUpdates,
															movieOriginType mot, 
															std::string& typeinfo)
{
  int count = 0;
	
  if ( !movieFile.empty() ) {
    rcMovieConverterOptionsRfy opt;
    opt.frameInterval( frameInterval );
    opt.firstFrameIndex( firstIndex );
    opt.frameCount( frameCount );
    opt.frameOffset( offset );
    opt.samplePeriod( period );
    opt.reversePixels( reversePixels );
    opt.creator( cCreator );
    opt.channelConversion( conversion );
    opt.rev( rev );
    opt.verbose( verbose );
		
    rcConversionProgress* prog = 0;
		
    if ( progressUpdates ) {
      prog = createProgressIndicator( frameCount,
																		 "Converting %i frames to Reify format...%%.2f%%%% complete" );
    }
    rcMovieConverterToRfy converter( opt, NULL, prog );
		
    if (!movie2File.empty())
      error = converter.convert( movieFile, movie2File, outputFile, mot, typeinfo );
    else
      error = converter.convert( movieFile, outputFile );
		
    if ( error != eMovieConverterErrorOK ) {
      cerr << rcMovieConverter::getErrorString( error ) << endl;
    } else
      count = frameCount;
    delete prog;
  }
	
  return count;
}

// Count frames in a non-Reify movie
static uint32 rfCountMovieFrames(const std::string& movieFile)
{
  uint32 count = 0;
  rcFrameGrabberError error;
	
  if ( !movieFile.empty() ) {
    // Create movie grabber
    rcMovieGrabber grabber( movieFile, NULL, 1.0 );
		
    if ( grabber.isValid() ) {
      bool started = grabber.start();
			
      if ( started ) {
				count = grabber.frameCount();
      }
    }
    error = grabber.getLastError();
    if ( error != eFrameErrorOK )
      cerr << rcFrameGrabber::getErrorString( error ) << " " << movieFile << endl;
  }
	
  return count;
}

static int printUsage()
{
  fprintf(stderr, "Usage: batchconvert -s srcFile.{rfymov,mov,avi}  -s srcDir (of tiff images) -s srcFile.{rfymov,mov,avi} \n -d destFile.{rfymov,mov} [-f count]\n"
					"                   [-o offset] [-p period] [-r time] [-c channel] [-0] [-1] [-2] [-i] [-t]\n\n"
					" Takes a Reify\\Quicktime movie as input and converts it into Quicktime\\Reify\n"
					" format.\n\n"
					" -s src     Specifies an input movie file\n OR"
					" -s dir     Specifies directory of tif images \n"
					" -z src     Specifies a second input movie file to be used as alpha (special option) \n"
					" -d dest    Location of resulting file. Quicktime file must end with"
					" .mov.\n            Reify file must end with .rfymov.\n"
					" -f count   Maximum number of frames in movie. If more are required,\n"
					"            multiple movies are made with _DDDofDDD appended to end of\n"
					"            file name (Default: All frames)\n"
					" -o offset  Starting frame offset within movie (>=0) (Default: 0)\n"
					" -x x-offset  Region of Interest  (>=0) (Default: 0)\n"
					" -y y-offset  Region of Interest  (>=0) (Default: 0)\n"
					" -w width  Region of Interest  (>=0, 0 means original width) (Default: 0)\n"
					" -h  height  Region of Interest  (>=0, 0 means original height) (Default: 0)\n"
					" -p period  Take 1 out of every period frames (>=1) (Default: 1)\n"
					" -r time    Force frame interval in seconds. If this option is\n"
					"            not specified, native times from input movie are used.)\n"
					" -c channel Channel conversion for 32-bit data. Valid values are\n"
					"            red, green, blue, avg, max, all (Default: avg). Ignored if\n"
					"            input is 8-bit data.\n"
					" -0         Force generation of a rev0 Reify file. Default is to\n"
					"            create new file in latest format. Ignored if output\n"
					"            is Quicktime file.\n"
					" -1         Force generation of a rev1 Reify file. Default is to\n"
					"            create new file in latest format. Ignored if output\n"
					"            is Quicktime file.\n"
					" -2         Force generation of a rev2 Reify file. Default is to\n"
					"            create new file in latest format. Ignored if output\n"
					"            is Quicktime file.\n"
					" -v         Invert pixels (255 becomes 0). Applies to 8-bit Quicktime input only.\n"
					" -m kernelWidth  Generates a temporal median movie (only 3 deep kernet is supported. \n"
					" -i         Generate file info only. Displays movie info, such as\n"
					"            frame count, but does not generate "
					"-C      If combining channels mark them multichannel flu"
					"-G      If combining channels mark them gray plus multichannel flu"
					"-A       If combining channels mark them gray plus flu on alpha"
					"Quicktime\\Reify movie.\n"
					" -t         Test src and dest movie equivalence.\n"
					"            Only validates if converting full movie to 1 file\n"
					" -u         Enable progress status messages during conversion.\n\n");
  return 1;
}

int main(int argc, char** argv)
{
  char* srcFile = 0;
  char* src2File = 0;
  char* destFile = 0;
  uint32 offset = 0;
  uint32 maxFrameCnt = 0xFFFFFFFF;
  uint32 period = 1;
  bool infoOnly = false;
  bool destReify = true;
  bool validate = false;
  movieFormatRev rev = movieFormatRevLatest;
  bool srcReify = true;
  bool srcMPEG2 = true;
  bool srcIsDirectory = false;
  bool src2Reify = true;
  bool progressUpdates = false;
  bool reversePixels = false;
  bool multichannelOptionIsCCC(false), multichannelOptionIsGCC(false), multichannelOptionIsGA(false);
  rcIRect roi;
  int32 tmp;
  int32 medianTemporalKernelWidth (-1);
  std::string ti;
  movieOriginType mot;
	
  float frameInterval = 0.0; // Use native time from input movie
  char channelStr[32];
  rcChannelConversion channelConv = rcSelectAll;
  // Creator name
  cCreator = cAppName;
  cCreator += " ";
  cCreator += cVersionName;
  cCreator += " ";
  cCreator += cAppBuildDate;
	
  for (int i = 1; i < argc; i++) {
    if (*(argv[i]) != '-')
      return printUsage();
		
    char cmdType = *(argv[i] + 1);
    if (((i + 1) >= argc) &&
				(cmdType != 'i') && (cmdType != 't') && (cmdType != 'u') && (cmdType != 'v')
				&& (cmdType != '0')  && (cmdType != '1')  && (cmdType != '2') )
      return printUsage();
		
    switch (cmdType) {
			case 's':
				if (srcFile)
					return printUsage();
				srcFile = argv[++i];
				srcIsDirectory = rfIsDirectory (srcFile);
				if (!srcIsDirectory)
				{
					int lth = strlen(srcFile);
					if ((lth < 8) ||
							((strcmp(&srcFile[lth-7], ".rfymov") != 0) &&
							 (strcmp(&srcFile[lth-7], ".RFYMOV") != 0)))
						srcReify = false;
					if ((lth < 5) ||
							(strcmp(&srcFile[lth-4], ".mpg") != 0))
						srcMPEG2 = false;
				}
				break;
				
			case 'z':
				if (src2File)
					return printUsage();
				src2File = argv[++i];
      {
				int lth = strlen(src2File);
				if ((lth < 8) ||
						((strcmp(&src2File[lth-7], ".rfymov") != 0) &&
						 (strcmp(&src2File[lth-7], ".RFYMOV") != 0)))
					src2Reify = false;
      }
				break;
				
			case 'd':
				if (destFile)
					return printUsage();
				destFile = argv[++i];
      {
				int lth = strlen(destFile);
				if (lth < 5)
					return printUsage();
				if ((strcmp(&destFile[lth-4], ".mov") == 0) ||
						(strcmp(&destFile[lth-4], ".MOV") == 0))
					destReify = false;
				else {
					if ((lth < 8) ||
							((strcmp(&destFile[lth-7], ".rfymov") != 0) &&
							 (strcmp(&destFile[lth-7], ".RFYMOV") != 0)))
						return printUsage();
				}
      }
				break;
				
			case 'o':
				if (sscanf(argv[++i], "%d", &offset) != 1)
					return printUsage();
				break;
				
			case 'x':
				if (sscanf(argv[++i], "%d", &tmp) != 1)
					return printUsage();
				roi = rcIRect (tmp, roi.origin().y(), roi.width(), roi.height());
				break;
				
			case 'y':
				if (sscanf(argv[++i], "%d", &tmp) != 1)
					return printUsage();
				roi = rcIRect (roi.origin().x(), tmp, roi.width(), roi.height());
				break;
				
			case 'w':
				if (sscanf(argv[++i], "%d", &tmp) != 1)
					return printUsage();
				roi = rcIRect (roi.origin().x(), roi.origin().y(), tmp, roi.height());
				break;
				
			case 'h':
				if (sscanf(argv[++i], "%d", &tmp) != 1)
					return printUsage();
				roi = rcIRect (roi.origin().x(), roi.origin().y(), roi.width(), tmp);
				break;
				
			case 'f':
				if ((sscanf(argv[++i], "%d", &maxFrameCnt) != 1) || (maxFrameCnt <= 0))
					return printUsage();
				break;
				
			case 'p':
				if ((sscanf(argv[++i], "%d", &period) != 1) || (period <= 0))
					return printUsage();
				break;
				
			case 'm':
				if ((sscanf(argv[++i], "%d", &medianTemporalKernelWidth) != 1) || (medianTemporalKernelWidth != 3))
					return printUsage();
				break;
				
			case 'r':
				if ((sscanf(argv[++i], "%f", &frameInterval) != 1) )
					return printUsage();
				else if (frameInterval <= 0.0) {
					frameInterval = defFrameInterval;
					cerr << "Iinvalid frame interval: Default used " << frameInterval << endl;
					break;
				}
				
				break;
			case 'c':
				if ((sscanf(argv[++i], "%16s", channelStr) != 1) )
					return printUsage();
				if (!strcmp( channelStr, "red" ) )
					channelConv = rcSelectRed;
				else if (!strcmp( channelStr, "green" ) )
					channelConv = rcSelectGreen;
				else if (!strcmp( channelStr, "blue" ) )
					channelConv = rcSelectBlue;
				else if (!strcmp( channelStr, "avg" ) )
					channelConv = rcSelectAverage;
				else if (!strcmp( channelStr, "max" ) )
					channelConv = rcSelectMax;
				else if (!strcmp( channelStr, "all" ) )
					channelConv = rcSelectAll;
				else {
					cerr << "Error: invalid channel conversion " << channelStr << endl;
					return 1;
				}
				break;
			case 'i':
				infoOnly = true;
				break;
				
			case 't':
				validate = true;
				break;
				
			case '0':
				rev = movieFormatRev0;
				break;
			case '1':
				rev = movieFormatRev1;
				break;
			case '2':
				rev = movieFormatRev2;
				break;
				
			case 'u':
				progressUpdates = true;
				break;
				
			case 'v':
				reversePixels = true;
				break;
				
			case 'C':
				multichannelOptionIsCCC = true;
				break;
				
			case 'G':
				multichannelOptionIsGCC = true;
				break;
				
			case 'A':
				multichannelOptionIsGA = true;
				break;
				
			default:
				return printUsage();
    } // End of: switch(*(argv[i] + 1))
  } // End of: for (int i = 1; i < argc; i++)
	
  if (!srcFile || !destFile)
    return printUsage();
	
  if ( !srcReify && !destReify ) {
    cerr << "Error: conversion from non-Reify to non-Reify movie not supported yet" << endl;
    return 1;
  }
	
  uint32 frameCnt = 0;
  rcVideoCache* cacheP = 0;
	
  if (srcMPEG2 && destReify)
	{
		std::string srcF (srcFile);
		std::string dstF (destFile);
		rcMPEG2Grabber mpegg (srcF, NULL);
		mpegg.start ();
		const rcChannelConversion conv = rcSelectAll;
		const movieFormatRev rev = movieFormatRevLatest;
		const float fInt = frameInterval;
		mpegg.getReifyMovie (dstF, conv, rev, true, fInt);
		return 0;
	}
	
  if (srcIsDirectory)
	{
		vector<std::string> files;
		std::string sf (srcFile);
		rfGetDirEntries (sf, files, "png");
		
		if (!files.size()) return 0;
		
		if (infoOnly)
		{
			for (uint32 i = 0; i < files.size(); i++)
				cerr << files[i] << endl;
			return 0;
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

                        framePtr->setTimestamp (rcTimestamp (i*frameInterval) );
			rcWindow image;
			rcIRect fRect (0, 0, framePtr->width(), framePtr->height());
			if (roi.isNull() || ! fRect.contains (roi))
			{
				image = rcWindow ( framePtr );
			}
			else
			{
				image = rcWindow (framePtr, roi);
			}
			
			if (image.depth() == rcPixel16)
			{
				image = rfImageConvert168 (image, channelConv);
			}
			
			if (image.depth() == rcPixel32)
			{
				image = rfImageConvert32to8 (image, channelConv);
			}
			
			images.push_back (image);
		}
		
		if (!images.size()) return 0;
		
		// If temporal median is selected generate a new image sequence
		vector<rcWindow>filtered;
		if (medianTemporalKernelWidth == 3)
		{
			vector<rcWindow> stupid (3);
			
			for (uint32 i = 1; i < images.size() - 1; i++)
			{
				rcWindow tmp (images[0].width (), images[0].height (), images[0].depth ());
				stupid[0] = images[i-1];stupid[1] = images[i];stupid[2] = images[i+1];
				rfTemporalMedian (stupid, tmp);
				filtered.push_back (tmp);
			}
		}
		
		std::string df (destFile);
		if (frameInterval == 0.0f) frameInterval = 1.0f;
		rcGenMovieFile dm (df, movieOriginConversionExt, channelConv, cCreator, movieFormatRevLatest, true, frameInterval);
		const vector<rcWindow>& imagesToWrite = (filtered.size ()) ? filtered : images;
		
		//@note a bug in movieFile header generation requires a frameBuf passed in. 
		for (uint32 i = 0; i < imagesToWrite.size(); i++)
		{
			rcWindow frameBug (imagesToWrite[i].width(), imagesToWrite[i].height(), imagesToWrite[i].depth());
			frameBug.copyPixelsFromWindow (imagesToWrite[i]);
			rcGenMovieFileError e = dm.addFrame (frameBug);
			if (e != eGenMovieFileErrorOK)
				return 0;
		}
		dm.flush ();
		return 0;
	} // End of Directory Import
	
  if ( srcReify )
	{
		cacheP = rcVideoCache::rcVideoCacheCtor(srcFile, cVideoCacheSize, false, false, false, cVideoCacheMem);
		if (!cacheP->isValid()) {
			cerr << "Open error: "
			<< rcVideoCache::getErrorString(cacheP->getFatalError())
			<< endl;
			rcVideoCache::rcVideoCacheDtor(cacheP);
			return 0;
		}
		frameCnt = (cacheP->frameCount() - offset + period - 1)/period;
		
		// If temporal median is selected generate a new image sequence
		if (medianTemporalKernelWidth == 3)
		{
			rcSharedFrameBufPtr srcPtr;
			vector<rcWindow> images;
			for (uint32 fi = 0; fi < cacheP->frameCount(); fi++)
			{
				rcVideoCacheStatus srcStatus = cacheP->getFrame(fi, srcPtr, 0);
				if ((srcStatus != eVideoCacheStatusOK))
				{
					cerr << "Error: srcStatus " << srcStatus << endl;
					return -1;
				}
				rcWindow tmp (srcPtr);
				images.push_back (tmp);
			}
			
			vector<rcWindow>filtered;
			if (medianTemporalKernelWidth == 3)
			{
				vector<rcWindow> stupid (3);
				
				for (uint32 i = 1; i < images.size() - 1; i++)
				{
					rcWindow tmp (images[0].width (), images[0].height (), images[0].depth ());
					stupid[0] = images[i-1];stupid[1] = images[i];stupid[2] = images[i+1];
					rfTemporalMedian (stupid, tmp);
					filtered.push_back (tmp);
				}
			}
			
			std::string df (destFile);
			if (frameInterval == 0.0f) frameInterval = 1.0f;
			rcGenMovieFile dm (df, movieOriginConversionExt, rcSelectAll, cCreator, movieFormatRevLatest, true, frameInterval);
			
			//@note a bug in movieFile header generation requires a frameBuf passed in. 
			for (uint32 i = 0; i < filtered.size(); i++)
			{
				rcWindow frameBug (filtered[i].width(), filtered[i].height(), filtered[i].depth());
				frameBug.copyPixelsFromWindow (filtered[i]);
				rcGenMovieFileError e = dm.addFrame (frameBug);
				if (e != eGenMovieFileErrorOK)
					return 0;
			}
			
			return 0;
		}
	} else {
		uint32 movieFrames = rfCountMovieFrames( srcFile );
		frameCnt = (movieFrames - offset + period - 1)/period;
		
	}
	
  uint32 frame2Cnt = 0;
  rcVideoCache* cache2P = 0;
	
  if ( src2Reify && src2File ) {
    cache2P = rcVideoCache::rcVideoCacheCtor(src2File, cVideoCacheSize, false, false, false, cVideoCacheMem);
    if (!cache2P->isValid()) {
      cerr << "Open error: "
			<< rcVideoCache::getErrorString(cacheP->getFatalError())
			<< endl;
      rcVideoCache::rcVideoCacheDtor(cache2P);
      return 0;
    }
    frame2Cnt = (cache2P->frameCount() - offset + period - 1)/period;
		
  } else if (src2File) {
    uint32 movieFrames = rfCountMovieFrames( src2File );
    frame2Cnt = (movieFrames - offset + period - 1)/period;
  }
	
  int repOffset = -1;
  int specialHackFlag = 0;
  int reps = 1;
	
  if (maxFrameCnt >= frameCnt)
    maxFrameCnt = frameCnt;
  else if (destReify) {
    int nameLen = strlen(destFile);
    int chkPt = nameLen - 7;
    repOffset = nameLen - 6;
    rmAssert(chkPt > 0);
    char formatString[17] = { '_', 'd', 'd', 'd', 'o', 'f', 'd', 'd', 'd', '.', 
		'r', 'f', 'y', 'm', 'o', 'v', 0 };
		
    formatString[10] = destFile[chkPt+1];
    formatString[11] = destFile[chkPt+2];
    formatString[12] = destFile[chkPt+3];
    formatString[13] = destFile[chkPt+4];
    formatString[14] = destFile[chkPt+5];
    formatString[15] = destFile[chkPt+6];
		
    reps = (frameCnt + maxFrameCnt - 1)/maxFrameCnt;
    if (((reps-1)*maxFrameCnt + 1) == frameCnt) {
      specialHackFlag = 1;
      reps -= 1;
    }
    if (reps > 999)
      return printUsage();
		
    snprintf(&formatString[6], 4, "%03d", reps);
    formatString[9] = '.';
    char* newName = (char*)malloc(nameLen + strlen(formatString));
    rmAssert(newName);
    destFile[chkPt] = 0;
    strcpy(newName, destFile);
    strcpy(&newName[chkPt], formatString);
    destFile = newName;
  }
  else {
    int nameLen = strlen(destFile);
    int chkPt = nameLen - 4;
    repOffset = nameLen - 3;
    rmAssert(chkPt > 0);
    char formatString[14] = { '_', 'd', 'd', 'd', 'o', 'f',
		'd', 'd', 'd', '.', 'm', 'o', 'v', 0 };
		
    formatString[10] = destFile[chkPt+1];
    formatString[11] = destFile[chkPt+2];
    formatString[12] = destFile[chkPt+3];
		
    reps = (frameCnt + maxFrameCnt - 1)/maxFrameCnt;
    if (((reps-1)*maxFrameCnt + 1) == frameCnt) {
      specialHackFlag = 1;
      reps -= 1;
    }
    if (reps > 999)
      return printUsage();
		
    snprintf(&formatString[6], 4, "%03d", reps);
    formatString[9] = '.';
    char* newName = (char*)malloc(nameLen + strlen(formatString));
    rmAssert(newName);
    destFile[chkPt] = 0;
    strcpy(newName, destFile);
    strcpy(&newName[chkPt], formatString);
    destFile = newName;
  }
	
  if ( frameCnt > 0 )
    fprintf(stderr, "Will create %d %s movie%s using period %d offset %d\n",
						reps, (destReify ? "Reify" : "Quicktime"),
						((reps == 1) ? "" : "s"), period, offset);
  if (infoOnly) {
    rcVideoCache::rcVideoCacheDtor(cacheP);
    return 0;
  }
	
	
  int rep = 1;
  uint32 totalConvertedFrames = 0;
  rcTimestamp duration = rcTimestamp::now();
	
  for (uint32 firstIndex = 0; 
       firstIndex < frameCnt;
       firstIndex += maxFrameCnt)
	{
		uint32 framesToUse = maxFrameCnt;
		
		if ((firstIndex + framesToUse) > frameCnt)
			framesToUse = frameCnt - firstIndex;
		if (specialHackFlag && (rep == reps))
		{
			framesToUse += 1;
			maxFrameCnt += 1;
			rmAssert((firstIndex + maxFrameCnt) >= frameCnt);
		}
		
		if (repOffset > 0)
		{
			snprintf(&destFile[repOffset], 4, "%03d", rep++);
			destFile[repOffset+3] = 'o';
		}
		
		uint32 convertedFrames = 0;
		
		std::string rf (srcFile), df (destFile);
		
		// Handle the special case of combining two monochrome images in to one
		if (srcFile && src2File && frameCnt != 0 && frameCnt == frame2Cnt && srcReify && src2Reify &&
				cacheP && cache2P && destFile)
		{
			// Call movie combine 
			
			uint32 convertedFrames = 0;
			
			bool failed = true;
			
			// combine frames
			if (multichannelOptionIsCCC)
			{
				ti = std::string ("movieOriginMultiChannelFlu");
				mot = movieOriginMultiChannelFlu;
			}
			else if (multichannelOptionIsGCC)
			{
				ti = std::string (" movieOriginGrayPlusMultiChannelFlu");
				mot =  movieOriginGrayPlusMultiChannelFlu;
			}
			else
			{
				ti = std::string ("movieOriginGrayIsVisibleAlphaIsFlu");
				mot =  movieOriginGrayIsVisibleAlphaIsFlu;
			}
			
			
			rcMovieConverterError error = eMovieConverterErrorUnknown;
			convertedFrames = rfConvertTwoMovies( std::string (srcFile), std::string (src2File), error,
																					 frameInterval, firstIndex,
																					 framesToUse, offset, period,
																					 reversePixels, channelConv,
																					 rev,
																					 std::string( destFile ),
																					 progressUpdates, mot, ti );
			if ( error == eMovieConverterErrorOK ) 
				failed = false;
			
			if (failed) {
				fprintf(stderr, "File Conversion failed\n");
				break;
			} else
				fprintf(stderr, "Combined %i-frame movie %s\n", convertedFrames, destFile);
			
			rcVideoCache::rcVideoCacheDtor(cache2P);
		} // End of movie combine
		
		else if (destReify)
		{
			bool failed = true;
			// Convert frames
			rcMovieConverterError error = eMovieConverterErrorUnknown;
			convertedFrames = rfConvertMovie( rf, error,
																			 frameInterval, firstIndex,
																			 framesToUse, offset, period,
																			 reversePixels, df, 
																			 progressUpdates, destReify);
			if ( error == eMovieConverterErrorOK ) 
				failed = false;
			
			if (failed) {
				fprintf(stderr, "File Conversion failed\n");
				break;
			} else
				fprintf(stderr, "Created %i-frame movie %s\n", convertedFrames, destFile);
		} 
		else 
		{
			bool failed = true;
			// Convert frames
			rcMovieConverterError error = eMovieConverterErrorUnknown;
			convertedFrames = rfConvertMovie( std::string (srcFile), error,
																			 frameInterval, firstIndex,
																			 framesToUse, offset, period,
																			 reversePixels, 
																			 std::string( destFile ),
																			 progressUpdates, destReify );
			if ( error == eMovieConverterErrorOK ) 
				failed = false;
			if (failed) {
				fprintf(stderr, "File Conversion failed\n");
				break;
			}
			else
				fprintf(stderr, "Created %i-frame movie %s\n", framesToUse, destFile);
			convertedFrames = framesToUse;                       
		}
		
		if (validate && (offset == 0) && (period == 1) && (reps == 1)) {
			fprintf(stderr, "Comparing source and destination files...\n");
			OSErr err = -1;
			if ( srcReify && !destReify )
				err = rfCompareMovies(srcFile, destFile, srcReify, destReify,
															channelConv, reversePixels, progressUpdates);
			else if ( destReify && !srcReify )
				err = rfCompareMovies(destFile, srcFile, srcReify, destReify,
															channelConv, reversePixels, progressUpdates);
			else if ( srcReify && destReify )
				err = rfCompareReifyMovies(destFile, srcFile, srcReify, destReify, progressUpdates);
			else
				fprintf(stderr, "QT-to-QT movie comparison not supported ");
			if (err != noErr)
				fprintf(stderr, "File comparison FAILURE, err no %d\n", (int)err);
			else
				fprintf(stderr, "File comparison OK\n");
		}
		
		totalConvertedFrames += convertedFrames;
	}
	
  duration = rcTimestamp::now() - duration;
  cerr << totalConvertedFrames << " frames converted in " << duration.secs() << " seconds, "
	<< totalConvertedFrames/duration.secs() << " frames per second" << endl;
	
  rcVideoCache::rcVideoCacheDtor(cacheP);
	
  return 0;
}


