
#ifndef _QtimeCache_H_
#define _QtimeCache_H_

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

#include "rc_types.h"
#include "rc_thread.h"
#include "cached_frame_buffer.h"
#include "rc_timestamp.h"

#include "vf_utils.hpp"

enum  eQtimeCacheError {
    eQtimeCacheErrorOK = 0,            // No error
    eQtimeCacheErrorFileInit,          // File (system) init error
    eQtimeCacheErrorFileSeek,          // File seek error
    eQtimeCacheErrorFileRead,          // File read error
    eQtimeCacheErrorFileClose,         // File close error
    eQtimeCacheErrorFileFormat,        // File invalid
    eQtimeCacheErrorFileUnsupported,   // File format unsupported
    eQtimeCacheErrorFileRevUnsupported,// File revision unsupported
    eQtimeCacheErrorSystemResources,   // Inadequate system resources
    eQtimeCacheErrorNoSuchFrame,       // Specified frame does not exist
    eQtimeCacheErrorCacheInvalid,      // Previously detected error put
    // cache in invalid state or cache
    // has been deleted
    eQtimeCacheErrorBomUnsupported,    // Unsupported byte order
    eQtimeCacheErrorDepthUnsupported,  // Unsupported image depth
};

// Status type
enum  eQtimeCacheStatus {
    eQtimeCacheStatusOK = 0, // Valid frame
    eQtimeCacheStatusError   // Error occurred, read returned error state for
    // more info.
};


/*  eQtimeCache - Implements a read-only cache of video frames stored
 * in a reify format movie.
 *
 * Within a movie, each video frame has a unique index that is used to
 * identify it. This is referred to as the "frame index" in both this
 * file and the associated implementation file.
 */
class   QtimeCache
{
   
//    friend class frame_ref_t;
//    friend class UT_PlaybackUtils;
    
public:
    typedef cached_frame_ref frame_ref_t;
    
    /*   QtimeCacheCtor - Opens file and initializes data structures. If
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
    static   QtimeCache*   QtimeCacheCtor(const std::string fileName,
                                          uint32 cacheSize,
                                          bool verbose = true,
                                          bool prefetch = false,
                                          uint32 maxMemory = 0,
                                          rcProgressIndicator* pIndicator = 0 );
    
    /*   QtimeCacheDtor - Removes pointer to this cache from the map of
     * active caches, closes the associated file and deletes the cache.
     *
     * Calls by frame_ref_t code to cacheLock() and
     * cacheUnlock() for caches that have already been deleted are
     * treated as noops.
     */
    static void   QtimeCacheDtor(  QtimeCache* cache);
    
    /* Returns the frame at location index within the movie (zero
     * base).
     */
    eQtimeCacheStatus getFrame(uint32 frameIndex,
                               frame_ref_t& frameBuf,
                               eQtimeCacheError* error = 0,
                               bool locked = true);
    
    /* Returns the frame at timestamp time. must be exact match.
     */
    eQtimeCacheStatus getFrame(const rcTimestamp& time,
                               frame_ref_t& frameBuf,
                               eQtimeCacheError* error = 0,
                               bool locked = true);
    
    /* A set of accessor fcts to allow client to manipulate and convert
     * frame timestamps and frame indices.
     */
    // Returns the timestamp in the movie closest to the goal time.
    eQtimeCacheStatus closestTimestamp(const rcTimestamp& goalTime,
                                       rcTimestamp& match,
                                       eQtimeCacheError* error = 0);
    // Returns first timestamp > goalTime. If no such timestamp can be
    // found, the return value is  eQtimeCacheStatusError, and
    //  eQtimeCacheErrorNoSuchFrame is returned in error.
    eQtimeCacheStatus nextTimestamp(const rcTimestamp& goalTime,
                                    rcTimestamp& match,
                                    eQtimeCacheError* error = 0);
    // Returns closest timestamp < goalTime. If no such timestamp can be
    // found, the return value is  eQtimeCacheStatusError, and
    //  eQtimeCacheErrorNoSuchFrame is returned in error.
    eQtimeCacheStatus prevTimestamp(const rcTimestamp& goalTime,
                                    rcTimestamp& match,
                                    eQtimeCacheError* error = 0);
    // Returns the timestamp of the first frame in the movie.
    eQtimeCacheStatus firstTimestamp(rcTimestamp& match,
                                     eQtimeCacheError* error = 0);
    // Returns the timestamp of the last frame in the movie.
    eQtimeCacheStatus lastTimestamp(rcTimestamp& match,
                                    eQtimeCacheError* error = 0);
    // Returns the timestamp for the frame at frameIndex. If no such
    // frameIndex can be found, the return value is
    //  eQtimeCacheStatusError, and  eQtimeCacheErrorNoSuchFrame is
    // returned in error.
    eQtimeCacheStatus frameIndexToTimestamp(uint32 frameIndex,
                                            rcTimestamp& match,
                                            eQtimeCacheError* error = 0);
    // Returns the frame index for the frame at time timestamp (must be
    // exact match). If no such timestamp can be found, the return value
    // is  eQtimeCacheStatusError, and  eQtimeCacheErrorNoSuchFrame is
    // returned in error.
    eQtimeCacheStatus timestampToFrameIndex(const rcTimestamp& timestamp,
                                            uint32& match,
                                            eQtimeCacheError* error = 0);
    
    /* Get error that caused video cache to become invalid.
     */
    eQtimeCacheError getFatalError() const;
    
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
    
    double averageFrameRate() const { return _averageFrameRate; };
    const int64& baseTime() const { return _baseTime; }
    
    uint32 cacheMisses() const;
    uint32 cacheHits() const;
    uint32 cacheSize() const { return _cacheSize; }
    
    const std::string getInputSourceName() const { return _fileName; }
    
    /* Static method for mapping an error value to a string.
     */
    static std::string getErrorString( eQtimeCacheError error);
    
    /* General cache management related static functions. These are
     * helper fcts intended to be used only by frame_ref_t
     * class lock() and unlock() fcts. cacheUnlock() is a wrapper around
     * unlockFrame() and cacheLock() is a wrapper around getFrame(). The
     * main intention behind this is to allow for a well-defined action
     * to occur if rcFrameBufPtr's persist past this life of the cache
     * that owns the underlying rcFrame. See the description of
     *   QtimeCacheDtor() for more details.
     */
    static void cachePrefetch(uint32 cacheIndex, uint32 frameIndex);
    static void cacheUnlock(uint32 cacheIndex, uint32 frameIndex);
    static eQtimeCacheStatus cacheLock(uint32 cacheIndex,
                                       uint32 frameIndex,
                                       frame_ref_t& frameBuf,
                                       eQtimeCacheError* error = 0);
    
    
    
    rcWindow get_window (uint32 frameIndex)
    {
        frame_ref_t frameb ( new rcFrame (frameWidth(), frameHeight(), frameDepth()) );
        eQtimeCacheStatus status = getFrame(frameIndex, frameb, 0, false);
        if(status == eQtimeCacheStatusOK)
            return rcWindow (static_cast<rcFrameRef>(frameb));
        else return rcWindow ();
    }
    

private:
    /*
     * Pimpl quicktime support
     */
    class qtImpl;
    boost::shared_ptr<qtImpl> _impl;
    
    /* Class to be used by   QtimeCache objects to create a thread that
     * will be used to do prefetches of video frames.
     *
     * Creator should call requestSeppuku() to gracefully shut down the
     * thread.
     */
    class   QtimeCachePrefetchUnit : public rcThread
    {
    public:
        QtimeCachePrefetchUnit(  QtimeCache& cacheCtrl);
        
        virtual ~  QtimeCachePrefetchUnit();
        
        /* Must be called to have calls to prefetch() be processed.
         */
        virtual void run();
        
        /* Allow the caller to give the cache a hint as to what frames
         * will be needed next. If the frame index is invalid, the call is
         * ignored.
         */
        void prefetch(uint32 frameIndex);
        
    private:
        QtimeCache&        _cacheCtrl;
        rcMutex              _prefetchMutex;
        deque<uint32>      _prefetchRequests;
        rcConditionVariable  _wait;
    };
    
    /* ctor - Opens file and initializes data structures. See
     * description of   QtimeCacheCtor() for more details.
     */
    QtimeCache(const std::string fileName, uint32 cacheSize, bool verbose, bool prefetch, uint32 maxMemory,
               rcProgressIndicator* pIndicator);
    
    virtual ~  QtimeCache();
    
    /* finishSetup - Inserts newly created video cache into the global
     * list of active caches and gives it a unique ID.
     */
    static   QtimeCache* finishSetup(  QtimeCache* cacheP);
    
    /*   QtimeCacheUTCtor - Allow UT clients to create a video cache
     * object that has "frames" with the specified timestamps. Allows
     * controlled testing of code that needs to manipulate lists of
     * frames based on their timestamps. Attempts to access these
     * "frames" will fail.
     */
    static   QtimeCache*   QtimeCacheUTCtor(const vector<rcTimestamp>& frameTimes);
    QtimeCache(const vector<rcTimestamp>& frameTimes);
    
    void setError( eQtimeCacheError error);
    
    eQtimeCacheStatus internalGetFrame(uint32 frameIndex,
                                       frame_ref_t& frameBuf,
                                       eQtimeCacheError* error,
                                       const uint32 dToken);
    
    eQtimeCacheStatus cacheAlloc(uint32 frameIndex,
                                 frame_ref_t& userFrameBuf,
                                 frame_ref_t*& cacheFrameBufPtr,
                                 const uint32 dToken);
    
    eQtimeCacheStatus cacheInsert(uint32 frameIndex,
                                  frame_ref_t& cacheFrameBuf,
                                  const uint32 dToken);
    
    // Header reading methods
    //        void createDefaultOriginHeader( movieFormatRev rev );
    
    eQtimeCacheError tocLoad();
    
    
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
    map<uint32, frame_ref_t*> _cachedFramesItoB;
    
    /* List of available cache frames entries that don't point to a
     * valid frame.
     */
    deque<frame_ref_t*>         _unusedCacheFrames;
    
    /* Support for bidirectional frame index <==> timestamp
     * mapping. (Table of contents).
     */
    vector<rcTimestamp>                  _tocItoT;
    map<rcTimestamp, uint32>           _tocTtoI;
    
    /* List of cached frames.
     */
    vector<frame_ref_t>         _frameCache;
    
    typedef vector<frame_ref_t*> vcPendingFills;
    
    /* List of pending cache fills
     */
    map<uint32, vcPendingFills>        _pending;
    
    bool                                 _verbose;
    bool                                 _isValid;
    eQtimeCacheError                    _fatalError;
    const std::string                       _fileName;
    vf_utils::general_movie::info      m_ginfo;
    uint32                             _bytesInFrame;
    uint32                             _frameCount;
    uint32                             _frameWidth;
    uint32                             _frameHeight;
    rcPixel                         _frameDepth;
    double                               _averageFrameRate;
    int64                              _baseTime;
    
    rcMutex                              _cacheMutex;
    rcMutex                              _diskMutex;
    rcSignalPending                      _pendingCtrl;
    
    uint32                             _cacheOverflowLimit;
    uint32                             _cacheSize;
    uint32                             _cacheID;
    uint32                             _cacheMisses;
    uint32                             _cacheHits;
    
    QtimeCachePrefetchUnit*            _prefetchThread;
    // Progress indicator for sow operations (ie. TOC from frames)
    rcProgressIndicator*                 _progressIndicator;
    
    /* General cache management related data and their mutex.
     */
    
public:
    
    class CacheManager
    {
    public:
        
        QtimeCache* register_cache ( QtimeCache* cacheP)
        {
            rmAssert(cacheP);
            boost::lock_guard<boost::mutex> locky (_cacheMgmtMutex);
            
            ++_nextCacheID;
            rmAssert(_nextCacheID != 0);
            rmAssert(_activeCachesPtoI.find(cacheP) == _activeCachesPtoI.end());
            
            _activeCachesItoP[_nextCacheID] = cacheP;
            _activeCachesPtoI[cacheP] = _nextCacheID;
            
            cacheP->setCacheID(_nextCacheID);
            return cacheP;
        }
        
        void  remove( QtimeCache* cacheP)
        {
            boost::lock_guard<boost::mutex> locky (_cacheMgmtMutex);
            
            map< QtimeCache*, uint32>::iterator locI = _activeCachesPtoI.find(cacheP);
            if (locI == _activeCachesPtoI.end())
                return;
            
            uint32 cacheID = locI->second;
            
            map<uint32,  QtimeCache*>::iterator locP = _activeCachesItoP.find(cacheID);
            
            rmAssert(locP != _activeCachesItoP.end());
            rmAssert(locI->first == locP->second);
            rmAssert(locP->first == locI->second);
            
            _activeCachesItoP.erase(locP);
            _activeCachesPtoI.erase(locI);
            
            delete cacheP;
        }
        
        
        QtimeCache* cacheById (uint32 cacheID)
        {
            boost::lock_guard<boost::mutex> locky (_cacheMgmtMutex);
            
            map<uint32,  QtimeCache*>::iterator loc = _activeCachesItoP.find(cacheID);
            if (loc == _activeCachesItoP.end())
                return 0;
            
            QtimeCache* cacheP = loc->second;
            return cacheP;
        }
        
        
        map<uint32,   QtimeCache*>  _activeCachesItoP;
        map<  QtimeCache*, uint32>  _activeCachesPtoI;
        uint32                      _nextCacheID;
        boost::mutex                _cacheMgmtMutex;
        
   
        
    private:
           
        
    };
    
    
    
    //#ifdef VID_TRACE
    uint32                             _debuggingToken;
    uint32                             getNextToken();
    //#endif
};

class QtimeCacheReleaser
{
public:
    void operator() (  QtimeCache* vc)
    {
        if (vc != 0)
            QtimeCache::  QtimeCacheDtor (vc);
    }
};

typedef boost::shared_ptr<  QtimeCache> SharedQtimeCache;

#define _shared_qtime_cache_create(name,_1,_2,_3,_4,_5,_6) sharedQtimeCache name (  QtimeCache::  QtimeCacheCtor((_1), (_2), (_3), (_4), (_5), (_6) , (cc)), QtimeCacheReleaser ())




#endif
