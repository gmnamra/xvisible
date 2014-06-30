// Copyright 2003 Reify, Inc.

#ifndef _rcVIDEOCACHE_H_
#define _rcVIDEOCACHE_H_

#include <vector>
#include <deque>
#include <map>
#include <boost/shared_ptr.hpp>

#undef VID_TRACE
#ifdef VID_TRACE
#define ADD_VID_TRACE(NAME, ENTER, FRMI, FRMP, TOKEN) \
                      vcAddTrace((NAME), (ENTER), (FRMI), (FRMP), (TOKEN))
#define DUMP_VID_TRACE() vcDumpTrace()
#define GET_TOKEN() getNextToken()
#else
#define ADD_VID_TRACE(NAME, ENTER, FRMI, FRMP, TOKEN)
#define DUMP_VID_TRACE()
#define GET_TOKEN() 0xFFFFFFFF
#endif

using namespace std;

// util
#include "rc_types.h"
#include "rc_thread.h"

// visual
#include "rc_framebuf.h"
#include "rc_timestamp.h"
#include "rc_moviefileformat.h"

enum rcVideoCacheError {
    eVideoCacheErrorOK = 0,            // No error
    eVideoCacheErrorFileInit,          // File (system) init error
    eVideoCacheErrorFileSeek,          // File seek error
    eVideoCacheErrorFileRead,          // File read error
    eVideoCacheErrorFileClose,         // File close error
    eVideoCacheErrorFileFormat,        // File invalid
    eVideoCacheErrorFileUnsupported,   // File format unsupported
    eVideoCacheErrorFileRevUnsupported,// File revision unsupported
    eVideoCacheErrorSystemResources,   // Inadequate system resources
    eVideoCacheErrorNoSuchFrame,       // Specified frame does not exist
    eVideoCacheErrorCacheInvalid,      // Previously detected error put
                                       // cache in invalid state or cache
                                       // has been deleted
    eVideoCacheErrorBomUnsupported,    // Unsupported byte order
    eVideoCacheErrorDepthUnsupported,  // Unsupported image depth
};

// Status type
enum rcVideoCacheStatus {
  eVideoCacheStatusOK = 0, // Valid frame
  eVideoCacheStatusError   // Error occurred, read returned error state for
                           // more info. 
};


/* rcVideoCache - Implements a read-only cache of video frames stored
 * in a reify format movie.
 *
 * Within a movie, each video frame has a unique index that is used to
 * identify it. This is referred to as the "frame index" in both this
 * file and the associated implementation file.
 */
class RFY_API rcVideoCache
{
  friend class rcSharedFrameBufPtr;
  friend class UT_PlaybackUtils;

 public:
  /* rcVideoCacheCtor - Opens file and initializes data structures. If
   * getTOC is set to true, then the table of contents (a map of
   * timestamps to images) is read in.
   *
   * Note: maxMemory puts a limit on the number of frames that will be
   *       maintained in cache by specifying the maximum number of bytes
   *       of memory the cached frames can take up. Specifying a 0 means
   *       there is no limit.
   *
   *       cacheSize specifies the number of frame buffers to maintain
   *       in cache. Specifying a value of 0 means there is no limit.
   *
   *       The actual number of cache slots to maintain is the minimum of
   *       the following triplet: floor(maxMemory/frameSize), (iff != 0)
   *                              cacheSize,                  (iff != 0)
   *                              frameCount
   *
   *       The previous discussion uses the term "maintain in cache" not
   *       "maximum cache size" because the implementation will attempt
   *       to allocate entries when the cache size is exceeded. When an
   *       overflow is about to occur, the cache first removes any
   *       unlocked frames from the cache. When these run out, it then
   *       allocates additional frames from the heap. While in an
   *       oveflow state, any newly unlocked frames are immediately
   *       removed from the cache. This continues until the number of
   *       frame buffers in cache is within the limits specified to the
   *       ctor.
   *
   */
  static rcVideoCache* rcVideoCacheCtor(const std::string fileName,
					uint32 cacheSize, bool getTOC,
					bool verbose = true,
					bool prefetch = false,
                    uint32 maxMemory = 0,
                    rcProgressIndicator* pIndicator = 0 );

  /* rcVideoCacheDtor - Removes pointer to this cache from the map of
   * active caches, closes the associated file and deletes the cache.
   *
   * Calls by rcSharedFrameBufPtr code to cacheLock() and
   * cacheUnlock() for caches that have already been deleted are
   * treated as noops.
   */
  static void rcVideoCacheDtor(rcVideoCache* cache);

  /* Returns the frame at location index within the movie (zero
   * base).
   */
  rcVideoCacheStatus getFrame(uint32 frameIndex,
			      rcSharedFrameBufPtr& frameBuf,
			      rcVideoCacheError* error = 0,
			      bool locked = true);

  /* Returns the frame at timestamp time. must be exact match.
   */
  rcVideoCacheStatus getFrame(const rcTimestamp& time,
			      rcSharedFrameBufPtr& frameBuf,
			      rcVideoCacheError* error = 0,
			      bool locked = true);

  /* A set of accessor fcts to allow client to manipulate and convert
   * frame timestamps and frame indices.
   */
  // Returns the timestamp in the movie closest to the goal time.
  rcVideoCacheStatus closestTimestamp(const rcTimestamp& goalTime,
				      rcTimestamp& match,
				      rcVideoCacheError* error = 0);
  // Returns first timestamp > goalTime. If no such timestamp can be
  // found, the return value is eVideoCacheStatusError, and
  // eVideoCacheErrorNoSuchFrame is returned in error.
  rcVideoCacheStatus nextTimestamp(const rcTimestamp& goalTime,
				   rcTimestamp& match,
				   rcVideoCacheError* error = 0);
  // Returns closest timestamp < goalTime. If no such timestamp can be
  // found, the return value is eVideoCacheStatusError, and
  // eVideoCacheErrorNoSuchFrame is returned in error.
  rcVideoCacheStatus prevTimestamp(const rcTimestamp& goalTime,
				   rcTimestamp& match,
				   rcVideoCacheError* error = 0);
  // Returns the timestamp of the first frame in the movie.
  rcVideoCacheStatus firstTimestamp(rcTimestamp& match,
				    rcVideoCacheError* error = 0);
  // Returns the timestamp of the last frame in the movie.
  rcVideoCacheStatus lastTimestamp(rcTimestamp& match,
				   rcVideoCacheError* error = 0);
  // Returns the timestamp for the frame at frameIndex. If no such
  // frameIndex can be found, the return value is
  // eVideoCacheStatusError, and eVideoCacheErrorNoSuchFrame is
  // returned in error.
  rcVideoCacheStatus frameIndexToTimestamp(uint32 frameIndex,
					   rcTimestamp& match,
					   rcVideoCacheError* error = 0);
  // Returns the frame index for the frame at time timestamp (must be
  // exact match). If no such timestamp can be found, the return value
  // is eVideoCacheStatusError, and eVideoCacheErrorNoSuchFrame is
  // returned in error.
  rcVideoCacheStatus timestampToFrameIndex(const rcTimestamp& timestamp,
					   uint32& match,
					   rcVideoCacheError* error = 0);

  /* Get error that caused video cache to become invalid.
   */
  rcVideoCacheError getFatalError() const;
    
  /* Returns instance validity.
   */
  bool isValid() const;

  /* Returns total number of frames in movie file.
   */
  uint32 frameCount() const { return _frameCount; }
  uint32 frameWidth() const { return _frameWidth; }
  uint32 frameHeight() const { return _frameHeight; }
  rcPixel frameDepth() const { return _frameDepth; }
  
  // Offset from start of file to start of frame pixel data
  off_t frameDataOffset( const uint32& frameIndex ) const;
  movieFormatRev rev() const { return _rev; };
  double averageFrameRate() const { return _averageFrameRate; };
  const int64& baseTime() const { return _baseTime; }
   
  uint32 cacheMisses() const;
  uint32 cacheHits() const;
  uint32 cacheSize() const { return _cacheSize; }

  const vector<rcMovieFileOrgExt>& movieFileOrigins() const { return _orgHdrs; }
  const vector<rcMovieFileConvExt>& movieFileConversions() const { return _cnvExtHdrs; }
  const vector<rcMovieFileCamExt>& movieFileCameras() const { return _camExtHdrs; }
  const vector<rcMovieFileExpExt>& movieFileExperiments() const { return _expExtHdrs; }
  
  const std::string getInputSourceName() const { return _fileName; }

  /* Static method for mapping an error value to a string.
   */
  static std::string getErrorString(rcVideoCacheError error);

 private:

  /* Class to be used by rcVideoCache objects to create a thread that
   * will be used to do prefetches of video frames.
   *
   * Creator should call requestSeppuku() to gracefully shut down the
   * thread.
   */
  class rcVideoCachePrefetchUnit : public rcThread
  {
  public:
    rcVideoCachePrefetchUnit(rcVideoCache& cacheCtrl);
    
    virtual ~rcVideoCachePrefetchUnit();
    
    /* Must be called to have calls to prefetch() be processed.
     */
    virtual void run();

    /* Allow the caller to give the cache a hint as to what frames
     * will be needed next. If the frame index is invalid, the call is
     * ignored.
     */
    void prefetch(uint32 frameIndex);
    
  private:
    rcVideoCache&        _cacheCtrl;
    rcMutex              _prefetchMutex;
    deque<uint32>      _prefetchRequests;
    rcConditionVariable  _wait;
  };

  /* ctor - Opens file and initializes data structures. See
   * description of rcVideoCacheCtor() for more details.
   */
  rcVideoCache(const std::string fileName, uint32 cacheSize, bool getTOC,
               bool verbose, bool prefetch, uint32 maxMemory,
               rcProgressIndicator* pIndicator);

  virtual ~rcVideoCache();

  /* finishSetup - Inserts newly created video cache into the global
   * list of active caches and gives it a unique ID.
   */
  static rcVideoCache* finishSetup(rcVideoCache* cacheP);

  /* rcVideoCacheUTCtor - Allow UT clients to create a video cache
   * object that has "frames" with the specified timestamps. Allows
   * controlled testing of code that needs to manipulate lists of
   * frames based on their timestamps. Attempts to access these
   * "frames" will fail.
   */
  static rcVideoCache* rcVideoCacheUTCtor(const vector<rcTimestamp>& frameTimes);
  rcVideoCache(const vector<rcTimestamp>& frameTimes);

  void setError(rcVideoCacheError error);

  rcVideoCacheStatus internalGetFrame(uint32 frameIndex,
				      rcSharedFrameBufPtr& frameBuf,
				      rcVideoCacheError* error,
				      const uint32 dToken);

  rcVideoCacheStatus cacheAlloc(uint32 frameIndex,
				rcSharedFrameBufPtr& userFrameBuf,
				rcSharedFrameBufPtr*& cacheFrameBufPtr,
				const uint32 dToken);

  rcVideoCacheStatus cacheInsert(uint32 frameIndex,
				 rcSharedFrameBufPtr& cacheFrameBuf,
				 const uint32 dToken);

  // Header reading methods
  void createDefaultOriginHeader( movieFormatRev rev );
  rcVideoCacheError headerLoadRev0();
  rcVideoCacheError headerLoadRev1( bool getTOC );
  rcVideoCacheError headerLoadRev2( bool getTOC );
  
  rcVideoCacheError tocLoad();
  rcVideoCacheError tocLoadFromFrames();
  rcVideoCacheError tocLoadFromTOC();

  rcVideoCacheError orgLoad();
  rcVideoCacheError cnvLoad();
  rcVideoCacheError camLoad();
  rcVideoCacheError expLoad();
  
  void setCacheID(uint32 cacheID) { _cacheID = cacheID; }

  /* Tell the cache the frame at location index is currently not
   * referenced anywhere and is a candidate for reuse.
   */
  void unlockFrame(uint32 frameIndex);

  /* Pass this frame index to the prefetch thread and ask it to load
   * this frame into cache. If prefetch thread isn't available, this
   * is a noop.
   */
  void prefetchFrame(uint32 frameIndex);

  /* Support for bidirectional frame index <==> last use
   * mapping. (Unlocked, cached frames)
   */
  map<int64, uint32>               _unlockedFramesLtoI;
  map<uint32, int64>               _unlockedFramesItoL;
  int64                              _lastTouchIndex;

  /* Maps a frame index to its cached location.
   */
  map<uint32, rcSharedFrameBufPtr*> _cachedFramesItoB;

  /* List of available cache frames entries that don't point to a
   * valid frame.
   */
  deque<rcSharedFrameBufPtr*>         _unusedCacheFrames;

  /* Support for bidirectional frame index <==> timestamp
   * mapping. (Table of contents).
   */
  vector<rcTimestamp>                  _tocItoT;
  map<rcTimestamp, uint32>           _tocTtoI;

  /* List of cached frames.
   */
  vector<rcSharedFrameBufPtr>         _frameCache;

  typedef vector<rcSharedFrameBufPtr*> vcPendingFills;

  /* List of pending cache fills
   */
  map<uint32, vcPendingFills>        _pending;

  bool                                 _verbose;
  bool                                 _isValid;
  rcVideoCacheError                    _fatalError;
  const std::string                       _fileName;
  FILE*                                _movieFile;
  movieFormatRev                       _rev;
  uint32                             _bytesInFrame;
  uint32                             _frameCount;
  uint32                             _frameWidth;
  uint32                             _frameHeight;
  rcPixel                         _frameDepth;
  double                               _averageFrameRate;
  int64                              _baseTime;
  reByteOrder                          _byteOrder; // Byte order of file
  int64                              _tocExtHdrOffset;
  vector<int64>                      _orgExtHdrOffsets;
  vector<rcMovieFileOrgExt>            _orgHdrs;
  vector<int64>                      _cnvExtHdrOffsets;
  vector<rcMovieFileConvExt>           _cnvExtHdrs;
  vector<int64>                      _camExtHdrOffsets;
  vector<rcMovieFileCamExt>            _camExtHdrs;
  vector<int64>                      _expExtHdrOffsets;
  vector<rcMovieFileExpExt>            _expExtHdrs;
  
  rcMutex                              _cacheMutex;
  rcMutex                              _diskMutex;
  rcSignalPending                      _pendingCtrl;

  uint32                             _cacheOverflowLimit;
  uint32                             _cacheSize;
  uint32                             _cacheID;
  uint32                             _cacheMisses;
  uint32                             _cacheHits;

  rcVideoCachePrefetchUnit*            _prefetchThread;
  // Progress indicator for sow operations (ie. TOC from frames)
  rcProgressIndicator*                 _progressIndicator;
  /* General cache management related data and their mutex.
   */
  static map<uint32, rcVideoCache*>  _activeCachesItoP;
  static map<rcVideoCache*, uint32>  _activeCachesPtoI;
  static uint32                      _nextCacheID;
  static rcMutex                       _cacheMgmtMutex;

  /* General cache management related static functions. These are
   * helper fcts intended to be used only by rcSharedFrameBufPtr
   * class lock() and unlock() fcts. cacheUnlock() is a wrapper around
   * unlockFrame() and cacheLock() is a wrapper around getFrame(). The
   * main intention behind this is to allow for a well-defined action
   * to occur if rcFrameBufPtr's persist past this life of the cache
   * that owns the underlying rcFrame. See the description of
   * rcVideoCacheDtor() for more details.
   */
  static void cachePrefetch(uint32 cacheIndex, uint32 frameIndex);
  static void cacheUnlock(uint32 cacheIndex, uint32 frameIndex);
  static rcVideoCacheStatus cacheLock(uint32 cacheIndex,
				      uint32 frameIndex,
				      rcSharedFrameBufPtr& frameBuf,
				      rcVideoCacheError* error = 0);

//#ifdef VID_TRACE
  uint32                             _debuggingToken;
  uint32                             getNextToken();
//#endif
};

class VideoCacheReleaser
{
public:
	void operator() (rcVideoCache* vc)
	{
		if (vc != 0)
			rcVideoCache::rcVideoCacheDtor (vc);
	}
};

typedef boost::shared_ptr<rcVideoCache> shared_video_cache;

#define _shared_video_cache_create(name,_1,_2,_3,_4,_5,_6) sharedVideoCache name (rcVideoCache::rcVideoCacheCtor((_1), (_2), (_3), (_4), (_5), (_6) , (cc)), VideoCacheReleaser ())


#endif
