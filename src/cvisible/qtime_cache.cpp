// Copyright 2003 Reify, Inc.

#include "vf_cinder.hpp"
#include <stdio.h>
#include "rc_types.h"



//#define VID_TRACE

// Only use this for doubles and floats for now
#define ByteSwapN(x) ByteSwap((unsigned char *) &x,sizeof(x))

static void ByteSwap(unsigned char * b, int n)
{
    int i = 0;
    int j = n-1;
    while (i<j)
    {
        std::swap(b[i], b[j]);
        i++, j--;
    }
}


#ifdef VID_TRACE

enum fctName {
    fnGetFrameI = 0,
    fnGetFrameT,
    fnInternalGetFrame,
    fnCacheAlloc,
    fnCacheInsert,
    fnUnlockFrame
};

typedef struct traceInfo {
    fctName  name;
    uint32 dToken;
    bool     enter;
    uint32 frameIndex;
    rcFrame* fp;
} traceInfo;

static const uint32 traceSz = 1000;
static traceInfo traceBuf[traceSz];
static const uint32 traceIndex = 0;
static const uint32 traceCount = 0;
static const rcMutex  traceMutex;
static const uint32 tracePrint = 0;

void vcAddTrace(fctName name, bool enter, uint32 frameIndex,
                rcFrame* fp, uint32 dToken)
{
    rcLock lock(traceMutex);
    
    if (traceIndex == traceSz)
        traceIndex = 0;
    
    traceBuf[traceIndex].name = name;
    traceBuf[traceIndex].dToken = dToken;
    traceBuf[traceIndex].enter =enter;
    traceBuf[traceIndex].frameIndex = frameIndex;
    traceBuf[traceIndex].fp = fp;
    
    traceIndex++; traceCount++;
}

void vcDumpTrace()
{
    rcLock lock(traceMutex);
    
    if (tracePrint)
        return;
    
    tracePrint = 1;
    
    fprintf(stderr, "trace count %d\n", traceCount);
    
    uint32 printCount = traceCount < traceSz ? traceCount : traceSz;
    uint32 myIndex = traceCount == traceSz ? traceIndex : 0;
    
    if (printCount)
        fprintf(stderr,
                "  Token    Name             Enter    frameIndex   frame ptr\n");
    
    for ( ; printCount; printCount--) {
        if (myIndex == traceSz)
            myIndex = 0;
        char* name = 0;
        switch (traceBuf[myIndex].name) {
            case fnGetFrameI:
                name = "getFrameI       ";
                break;
            case fnGetFrameT:
                name = "getFrameT       ";
                break;
            case fnInternalGetFrame:
                name = "internalGetFrame";
                break;
            case fnCacheAlloc:
                name = "cacheAlloc      ";
                break;
            case fnCacheInsert:
                name = "cacheInsert     ";
                break;
            case fnUnlockFrame:
                name = "unlockFrame     ";
                break;
            default:
                name = "INVALID         ";
                break;
        }
        char* enter = "yes";
        if (traceBuf[myIndex].enter == false)
            enter = "NO ";
        
        fprintf(stderr, " %06d %s     %s        %02d         0x%X\n",
                traceBuf[myIndex].dToken, name, enter,
                traceBuf[myIndex].frameIndex, (intptr_t)traceBuf[myIndex].fp);
        myIndex++;
    }
}

uint32 vfVideoCache::getNextToken()
{
    rcLock lock(_cacheMutex); return _debuggingToken++;
}

#endif

map <uint32, vfVideoCache*> vfVideoCache::_activeCachesItoP;
map <vfVideoCache*, uint32> vfVideoCache::_activeCachesPtoI;
uint32                      vfVideoCache::_nextCacheID = 0;
rcMutex                       vfVideoCache::_cacheMgmtMutex;

vfVideoCache* vfVideoCache::vfVideoCacheCtor(const std::string fileName,
                                             uint32 cacheSize,
                                             bool getTOC, bool verbose,
                                             bool prefetch,
                                             uint32 maxMemory,
                                             rcProgressIndicator* pIndicator)
{
    return finishSetup(new vfVideoCache(fileName, cacheSize, getTOC,
                                        verbose, prefetch, maxMemory, pIndicator));
}

vfVideoCache*
vfVideoCache::vfVideoCacheUTCtor(const vector<rcTimestamp>& frameTimes)
{
    return finishSetup(new vfVideoCache(frameTimes));
}

vfVideoCache* vfVideoCache::finishSetup(vfVideoCache* cacheP)
{
    rmAssert(cacheP);
    
    rcLock lock(_cacheMgmtMutex);
    
    ++_nextCacheID;
    rmAssert(_nextCacheID != 0);
    rmAssert(_activeCachesPtoI.find(cacheP) == _activeCachesPtoI.end());
    
    _activeCachesItoP[_nextCacheID] = cacheP;
    _activeCachesPtoI[cacheP] = _nextCacheID;
    
    cacheP->setCacheID(_nextCacheID);
    return cacheP;
}

void vfVideoCache::vfVideoCacheDtor(vfVideoCache* cacheP)
{
    {
        rcLock lock(_cacheMgmtMutex);
        
        map<vfVideoCache*, uint32>::iterator locI = _activeCachesPtoI.find(cacheP);
        if (locI == _activeCachesPtoI.end())
            return;
        
        uint32 cacheID = locI->second;
        
        map<uint32, vfVideoCache*>::iterator locP = _activeCachesItoP.find(cacheID);
        
        rmAssert(locP != _activeCachesItoP.end());
        rmAssert(locI->first == locP->second);
        rmAssert(locP->first == locI->second);
        
        _activeCachesItoP.erase(locP);
        _activeCachesPtoI.erase(locI);
    }
    
    delete cacheP;
}

vfVideoCache::vfVideoCache(std::string fileName, uint32 cacheSize,
                           bool getTOC, bool verbose, bool prefetch,
                           uint32 maxMemory, rcProgressIndicator* pIndicator)
: _lastTouchIndex(0), _verbose(verbose),
_isValid(true), _fatalError(eVideoCacheErrorOK), _fileName(fileName),
_movieFile(NULL), _rev(movieFormatInvalid), _tocExtHdrOffset(-1), _pendingCtrl(_cacheMutex),
_cacheID(0), _cacheMisses(0), _cacheHits(0), _prefetchThread(0),
_progressIndicator(pIndicator)
{
#ifdef VID_TRACE
    _debuggingToken = 0;
#endif
    
     _impl = boost::shared_ptr<qtImpl> ( new qtImpl (fileName) );
    
    if (! _impl->isValid())
    {
        if (_verbose) perror("fopen failed");
        setError(eVideoCacheErrorFileInit);
        return;
    }

    _frameWidth = _impl->frame_width();
    _frameHeight = _impl->frame_height ();
    _frameDepth = rcPixel8;
    _averageFrameRate = _impl->frame_rate ();
    _frameCount = _impl->embeddedCount ();
    
    _baseTime = 0;
    
    _byteOrder = eByteOrderUnknown;
    if (_fileName.empty()) {
        setError(eVideoCacheErrorFileInit);
        return;
    }
    
    int32 fc = _impl->getTOC (_tocItoT, _tocTtoI);
    
    /* First, calculate the cache overflow number based on both the
     * number of frames in the movie and the maximum amount of memory
     * the caller wants to use for this purpose.
     */
    _cacheSize = _frameCount;
    if (maxMemory != 0) {
        if (_cacheSize > (maxMemory / _bytesInFrame)) {
            _cacheSize = maxMemory / _bytesInFrame;
            
            if (_cacheSize == 0) {
                setError(eVideoCacheErrorSystemResources);
                return;
            }
        }
    }
    
    /* If user specified a cache size use it as an upper bound on the
     * cache overflow limit.
     */
    if (cacheSize && (cacheSize < _cacheSize))
        _cacheSize = cacheSize;
    
    _frameCache.resize(_frameCount);
    
    /* Make all the cache entries available for use by pushing them all
     * onto the unused list.
     */
    for (uint32 i = 0; i < _frameCache.size(); i++)
        _unusedCacheFrames.push_back(&_frameCache[i]);
    
    rmAssert(_unusedCacheFrames.size() >= _cacheSize);
    _cacheOverflowLimit = _unusedCacheFrames.size() - _cacheSize;
    
    /* Just about done. If prefetch is enabled, create a prefetch thread
     * here.
     */
    if (prefetch) {
        _prefetchThread = new vfVideoCachePrefetchUnit(*this);
        rmAssert(_prefetchThread);
        _prefetchThread->start();
    }
}


vfVideoCacheError vfVideoCache::tocLoad()
{
    rcLock lock(_diskMutex);
    
    if (!_tocItoT.empty())
        return eVideoCacheErrorOK;
    
    _tocItoT.resize(_frameCount);
    rmAssert(_tocTtoI.empty());
    rmAssert(_impl);
    if (_frameCount == _impl->getTOC (_tocItoT, _tocTtoI)) return eVideoCacheErrorOK;
    return eVideoCacheErrorFileRead;
    
}

vfVideoCache::vfVideoCache(const vector<rcTimestamp>& frameTimes)
: _lastTouchIndex(0), _verbose(false),
_isValid(true), _fatalError(eVideoCacheErrorOK), _fileName(""),
_movieFile(NULL), _rev(movieFormatInvalid), _tocExtHdrOffset(-1), _pendingCtrl(_cacheMutex),
_cacheID(0), _cacheMisses(0), _cacheHits(0), _prefetchThread(0)
{
#ifdef VID_TRACE
    _debuggingToken = 0;
#endif
    rmAssert(frameTimes.size());
    _byteOrder = eByteOrderUnknown;
    
    createDefaultOriginHeader( movieFormatRev1 );
    
    _frameWidth = _frameHeight = 0;
    _frameDepth = rcPixel8;
    _averageFrameRate = 0.0;
    _frameCount = frameTimes.size();
    _baseTime = 0;
    
    /* This stuff is normally done in tocLoad().
     */
    _tocItoT.resize(_frameCount);
    for (uint32 frameIndex = 0; frameIndex < _frameCount; frameIndex++) {
        if (frameIndex != 0)
            rmAssert(frameTimes[frameIndex] > frameTimes[frameIndex-1]);
        
        _tocItoT[frameIndex] = frameTimes[frameIndex];
        _tocTtoI[frameTimes[frameIndex]] = frameIndex;
    }
    
    _cacheSize = _frameCount;
    _frameCache.resize(_frameCount);
    
    /* Make all the cache entries available for use by pushing them all
     * onto the unused list.
     */
    for (uint32 i = 0; i < _frameCache.size(); i++)
        _unusedCacheFrames.push_back(&_frameCache[i]);
    
    rmAssert(_unusedCacheFrames.size() >= _cacheSize);
    _cacheOverflowLimit = _unusedCacheFrames.size() - _cacheSize;
}

vfVideoCache::~vfVideoCache()
{
    rcLock lock(_diskMutex);
    
    if (_movieFile && fclose(_movieFile) && _verbose)
        perror("fclose failed");
    
    _movieFile = NULL;
    
    if (_prefetchThread) {
        /* Want prefetch thread to end itself, but it may be asleep
         * waiting on input. deal with this using 3 step algorithm:
         * 1) Ask thread to kill itself.
         * 2) Send a phony prefetch request to jostle it awake.
         * 3) Join the thread.
         */
        _prefetchThread->requestSeppuku();
        _prefetchThread->prefetch(0);
        int status = _prefetchThread->join();
        if (_verbose) {
            cout << "Prefetch thread close: ";
            if (status)
                cout << "error " << status << endl;
            else
                cout << "OK" << endl;
        }
        delete _prefetchThread;
        _prefetchThread = 0;
    }
    _progressIndicator = 0;
}

vfVideoCacheStatus vfVideoCache::getFrame(uint32 frameIndex,
                                          rcSharedFrameBufPtr& frameBuf,
                                          vfVideoCacheError* error,
                                          bool locked)
{
    /* Before stuffing a new frame reference into here, invalidate any
     * existing reference.
     */
    frameBuf = 0;
    
    const uint32 dToken = GET_TOKEN();
    ADD_VID_TRACE(fnGetFrameI, true, frameIndex, frameBuf.mFrameBuf, dToken);
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        ADD_VID_TRACE(fnGetFrameI, false, frameIndex, frameBuf.mFrameBuf, dToken);
        return eVideoCacheStatusError;
    }
    
    if (frameIndex >= _frameCount) {
        setError(eVideoCacheErrorNoSuchFrame);
        if (error) *error = eVideoCacheErrorNoSuchFrame;
        ADD_VID_TRACE(fnGetFrameI, false, frameIndex, frameBuf.mFrameBuf, dToken);
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheStatus status = eVideoCacheStatusOK;
    if (locked)
        status = internalGetFrame(frameIndex, frameBuf, error, dToken);
    else
        frameBuf.setCachedFrameIndex(_cacheID, frameIndex);
    
    ADD_VID_TRACE(fnGetFrameI, false, frameIndex, frameBuf.mFrameBuf, dToken);
    
    return status;
}

vfVideoCacheStatus vfVideoCache::getFrame(const rcTimestamp& time,
                                          rcSharedFrameBufPtr& frameBuf,
                                          vfVideoCacheError* error,
                                          bool locked)
{
    /* Before stuffing a new frame reference into here, invalidate any
     * existing reference.
     */
    frameBuf = 0;
    
    const uint32 dToken = GET_TOKEN();
    ADD_VID_TRACE(fnGetFrameT, true, 0xFFFFFFFF, frameBuf.mFrameBuf, dToken);
    
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        ADD_VID_TRACE(fnGetFrameT, false, 0xFFFFFFFF, frameBuf.mFrameBuf, dToken);
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheError status = tocLoad();
    if (status != eVideoCacheErrorOK) {
        if (error) *error = status;
        ADD_VID_TRACE(fnGetFrameT, false, 0xFFFFFFFF, frameBuf.mFrameBuf, dToken);
        return eVideoCacheStatusError;
    }
    
    rmAssert(!_tocTtoI.empty());
    
    map<rcTimestamp, uint32>::iterator frameIndexPtr;
    frameIndexPtr = _tocTtoI.find(time);
    
    if (frameIndexPtr == _tocTtoI.end()) {
        setError(eVideoCacheErrorNoSuchFrame);
        if (error) *error = eVideoCacheErrorNoSuchFrame;
        ADD_VID_TRACE(fnGetFrameT, false, 0xFFFFFFFF, frameBuf.mFrameBuf, dToken);
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheStatus status2 = eVideoCacheStatusOK;
    if (locked)
        status2 = internalGetFrame(frameIndexPtr->second, frameBuf, error, dToken);
    else
        frameBuf.setCachedFrameIndex(_cacheID, frameIndexPtr->second);
    
    ADD_VID_TRACE(fnGetFrameT, false, frameIndexPtr->second,
                  frameBuf.mFrameBuf, dToken);
    
    return status2;
}

// Static method for mapping an error value to a string
std::string vfVideoCache::getErrorString(vfVideoCacheError error)
{
    switch (error) {
        case eVideoCacheErrorOK:
            return std::string("Video cache: OK");
        case eVideoCacheErrorFileInit:
            return std::string("Video cache: video file initialization error");
        case eVideoCacheErrorFileSeek:
            return std::string("Video cache: video file seek error");
        case eVideoCacheErrorFileRead:
            return std::string("Video cache: video file read error");
        case eVideoCacheErrorFileClose:
            return std::string("Video cache: video file close error");
        case eVideoCacheErrorFileFormat:
            return std::string("Video cache: video file invalid/corrupted error");
        case eVideoCacheErrorFileUnsupported:
            return std::string("Video cache: video file format unsupported error");
        case eVideoCacheErrorFileRevUnsupported:
            return std::string("Video cache: video file revision unsupported error");
        case eVideoCacheErrorSystemResources:
            return std::string("Video cache: Inadequate system resources");
        case eVideoCacheErrorNoSuchFrame:
            return std::string("Video cache: no video frame at given timestamp/frame index");
        case eVideoCacheErrorCacheInvalid:
            return std::string("Video cache: previous error put cache in invalid state");
        case eVideoCacheErrorBomUnsupported:
            return std::string("Video cache: unsupported byte order error");
        case eVideoCacheErrorDepthUnsupported:
            return std::string("Video cache: unsupported image depth error");
            // Note: no default case to force a compiler warning if a new enum
            // value is defined without adding a corresponding string here.
    }
    
    return std::string(" Video cache: undefined error" );
}

vfVideoCacheError vfVideoCache::getFatalError() const
{
    rcLock lock(const_cast<vfVideoCache*>(this)->_cacheMutex);
    
    return _fatalError;
}

// Returns the timestamp in the movie closest to the goal time.
vfVideoCacheStatus
vfVideoCache::closestTimestamp(const rcTimestamp& goalTime,
                               rcTimestamp& match,
                               vfVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheError status = tocLoad();
    if (status != eVideoCacheErrorOK) {
        if (error) *error = status;
        return eVideoCacheStatusError;
    }
    
    rmAssert(!_tocTtoI.empty());
    
    /* Find timestamp > goal timestamp.
     */
    map<rcTimestamp, uint32>::iterator it;
    it = _tocTtoI.upper_bound(goalTime);
    
    if (it == _tocTtoI.end()) {
        /* Goal is >= last timestamp so just return it.
         */
        it--;
        match = it->first;
    } else if (it == _tocTtoI.begin()) {
        /* Goal is < first timestamp so just return it.
         */
        match = it->first;
    }
    else {
        const rcTimestamp afterGoal = it->first;
        it--;
        const rcTimestamp beforeOrEqualGoal = it->first;
        
        //  @note: goalTime.bi_compare (beforeOrEqualGoal, afterGoal, match);
        if ((afterGoal - goalTime) < (goalTime - beforeOrEqualGoal))
            match = afterGoal;
        else
            match = beforeOrEqualGoal;
    }
    
    return eVideoCacheStatusOK;
}

// Returns first timestamp > goalTime.
vfVideoCacheStatus
vfVideoCache::nextTimestamp(const rcTimestamp& goalTime,
                            rcTimestamp& match,
                            vfVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheError status = tocLoad();
    if (status != eVideoCacheErrorOK) {
        if (error) *error = status;
        return eVideoCacheStatusError;
    }
    
    rmAssert(!_tocTtoI.empty());
    
    /* Find timestamp > goal timestamp.
     */
    map<rcTimestamp, uint32>::iterator it;
    it = _tocTtoI.upper_bound(goalTime);
    
    if (it == _tocTtoI.end()) {
        /* Goal is >= last timestamp. Return error.
         */
        if (error) *error = eVideoCacheErrorNoSuchFrame;
        return eVideoCacheStatusError;
    }
    
    match = it->first;
    
    return eVideoCacheStatusOK;
}

// Returns timestamp closest to goalTime that is < goalTime
vfVideoCacheStatus
vfVideoCache::prevTimestamp(const rcTimestamp& goalTime,
                            rcTimestamp& match,
                            vfVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheError status = tocLoad();
    if (status != eVideoCacheErrorOK) {
        if (error) *error = status;
        return eVideoCacheStatusError;
    }
    
    rmAssert(!_tocTtoI.empty());
    
    /* Find timestamp >= goal timestamp.
     */
    map<rcTimestamp, uint32>::iterator it;
    it = _tocTtoI.lower_bound(goalTime);
    
    if (it == _tocTtoI.begin()) {
        /* Goal is <= first timestamp. Return error.
         */
        if (error) *error = eVideoCacheErrorNoSuchFrame;
        return eVideoCacheStatusError;
    }
    
    it--;
    match = it->first;
    
    return eVideoCacheStatusOK;
}

// Returns the timestamp of the first frame in the movie.
vfVideoCacheStatus
vfVideoCache::firstTimestamp(rcTimestamp& match,
                             vfVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheError status = tocLoad();
    if (status != eVideoCacheErrorOK) {
        if (error) *error = status;
        return eVideoCacheStatusError;
    }
    
    rmAssert(!_tocTtoI.empty());
    
    match = _tocTtoI.begin()->first;
    
    return eVideoCacheStatusOK;
}

// Returns the timestamp of the last frame in the movie.
vfVideoCacheStatus
vfVideoCache::lastTimestamp(rcTimestamp& match,
                            vfVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheError status = tocLoad();
    if (status != eVideoCacheErrorOK) {
        if (error) *error = status;
        return eVideoCacheStatusError;
    }
    
    rmAssert(!_tocTtoI.empty());
    
    map<rcTimestamp, uint32>::iterator it;
    it = _tocTtoI.end();
    it--;
    
    match = it->first;
    
    return eVideoCacheStatusOK;
}

// Returns the timestamp for the frame at frameIndex.
vfVideoCacheStatus
vfVideoCache::frameIndexToTimestamp(uint32 frameIndex,
                                    rcTimestamp& match,
                                    vfVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheError status = tocLoad();
    if (status != eVideoCacheErrorOK) {
        if (error) *error = status;
        return eVideoCacheStatusError;
    }
    
    if (frameIndex >= _tocItoT.size()) {
        /* No frame for given frame index. Return error.
         */
        if (error) *error = eVideoCacheErrorNoSuchFrame;
        return eVideoCacheStatusError;
    }
    
    match = _tocItoT[frameIndex];
    
    return eVideoCacheStatusOK;
}

// Returns the frame index for the frame at time timestamp (must be
// exact match).
vfVideoCacheStatus
vfVideoCache::timestampToFrameIndex(const rcTimestamp& timestamp,
                                    uint32& match,
                                    vfVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }
    
    vfVideoCacheError status = tocLoad();
    if (status != eVideoCacheErrorOK) {
        if (error) *error = status;
        return eVideoCacheStatusError;
    }
    
    rmAssert(!_tocTtoI.empty());
    
    map<rcTimestamp, uint32>::iterator it;
    it = _tocTtoI.find(timestamp);
    
    if (it == _tocTtoI.end()) {
        /* No frame for given timestamp. Return error.
         */
        if (error) *error = eVideoCacheErrorNoSuchFrame;
        return eVideoCacheStatusError;
    }
    
    match = it->second;
    
    return eVideoCacheStatusOK;
}

bool vfVideoCache::isValid() const
{
    rcLock lock(const_cast<vfVideoCache*>(this)->_cacheMutex);
    
    return _isValid;
}

// Offset from start of file to start of frame data
off_t vfVideoCache::frameDataOffset( const uint32& frameIndex ) const
{
    off_t movieRecordSz = sizeof(int64) + _bytesInFrame;
    
    switch ( _rev ) {
        case movieFormatRev0:
        case movieFormatRev1:
            return sizeof(rcMovieFileFormat) + movieRecordSz*frameIndex;
        case movieFormatRev2:
            return sizeof(rcMovieFileFormat2) + movieRecordSz*frameIndex;
        case movieFormatInvalid:
            break;
    }
    
    return 0;
}

uint32 vfVideoCache::cacheMisses() const
{
    rcLock lock(const_cast<vfVideoCache*>(this)->_cacheMutex);
    
    return _cacheMisses;
}

uint32 vfVideoCache::cacheHits() const
{
    rcLock lock(const_cast<vfVideoCache*>(this)->_cacheMutex);
    
    return _cacheHits;
}

void vfVideoCache::setError (vfVideoCacheError error)
{
    rcLock lock(_cacheMutex);
    
    /* Don't bother setting this if a fatal error has already occurred.
     */
    if (_isValid == false)
        return;
    
    switch (error) {
        case eVideoCacheErrorFileInit:
        case eVideoCacheErrorFileSeek:
        case eVideoCacheErrorFileRead:
        case eVideoCacheErrorFileClose:
        case eVideoCacheErrorFileFormat:
        case eVideoCacheErrorFileUnsupported:
        case eVideoCacheErrorFileRevUnsupported:
        case eVideoCacheErrorSystemResources:
        case eVideoCacheErrorBomUnsupported:
        case eVideoCacheErrorDepthUnsupported:
            _fatalError = error;
            _isValid = false;
            break;
            
        case eVideoCacheErrorNoSuchFrame:
        case eVideoCacheErrorOK:
            break;
            
        case eVideoCacheErrorCacheInvalid:
            rmAssert(0);
            break;
    }
}

vfVideoCacheStatus
vfVideoCache::internalGetFrame(uint32 frameIndex,
                               rcSharedFrameBufPtr& frameBufPtr,
                               vfVideoCacheError* error,
                               const uint32 dToken)
{
    ADD_VID_TRACE(fnInternalGetFrame, true, frameIndex,
                  frameBufPtr.mFrameBuf, dToken);
    
    /* Read in the frame using the following algorithm:
     *
     * Step 1: Call cache allocation fct to check and see if the frame
     * is already in cache. If so, frameBufPtr will be pointed towards
     * this frame and processing will be complete.
     *
     * Step 2: If not, the cache allocation fct will return a pointer to
     * an available frame buffer from within the cache. Read the frame
     * data in from disk and store it in this frame buffer. (Note: If
     * there aren't any cache slots available, an error is returned.)
     *
     * Step 3: Call cache insert fct to add this to the set of available
     * cached frames. This will also set frameBufPtr to point towards
     * the newly cached frame.
     *
     * Note that this gets broken into three steps to allow the cache
     * mutex to be freed during the possibly costly disk read operation,
     * with the idea of allowing other threads concurrent access to the
     * cache.
     */
    rcSharedFrameBufPtr* cacheFrameBufPtr;
    
    if (cacheAlloc(frameIndex, frameBufPtr, cacheFrameBufPtr, dToken) ==
        eVideoCacheStatusOK) {
        ADD_VID_TRACE(fnInternalGetFrame, false, frameIndex,
                      frameBufPtr.mFrameBuf, dToken);
        return eVideoCacheStatusOK;
    }
    
    /* If status is error but no frame was provided some other thread
     * is already loading the frame into memory. Go to sleep until
     * that thread completes the work.
     */
    if (!cacheFrameBufPtr) {
        _pendingCtrl.wait(frameBufPtr.mFrameBuf);
        rmAssert(frameBufPtr.mFrameBuf != 0);
        ADD_VID_TRACE(fnInternalGetFrame, false, frameIndex,
                      frameBufPtr.mFrameBuf, dToken);
        return eVideoCacheStatusOK;
    }
    
    rmAssert(cacheFrameBufPtr->refCount() == 1);
    
    rcTimestamp timestamp;
    double timeInfo;
    
    {
        rcLock lock(_diskMutex);
        rmAssert(_impl);
        
        /* Read in the frame from disk.
         */
        _impl->seekToFrame(frameIndex);
        if (! _impl->checkPlayable())
        {
            setError(eVideoCacheErrorFileSeek);
            if (error) *error = eVideoCacheErrorFileSeek;
            /* Restore cache ID and frame index which were cleared by
             * calling function.
             */
            frameBufPtr.mCacheCtrl = _cacheID;
            frameBufPtr.mFrameIndex = frameIndex;
            return eVideoCacheStatusError;
        }
        timeInfo = _impl->getCurrentTime ();
           timestamp = rcTimestamp::from_seconds(timeInfo);
        
      //  if (fread((*cacheFrameBufPtr)->alignedRawData(), _bytesInFrame, 1,
      //            _movieFile) != 1) {
        if (_impl->checkNewFrame ())
        {
            _impl->getSurfaceAndCopy (*cacheFrameBufPtr);
        }
        else
        {
            if (_verbose) perror("fread of frame data during cache fill failed");
            setError(eVideoCacheErrorFileRead);
            if (error) *error = eVideoCacheErrorFileRead;
            /* Restore cache ID and frame index which were cleared by
             * calling function.
             */
            frameBufPtr.mCacheCtrl = _cacheID;
            frameBufPtr.mFrameIndex = frameIndex;
            return eVideoCacheStatusError;
        }
    } // End of: { rcLock lock(_diskMutex);
    
    (*cacheFrameBufPtr)->setTimestamp(timestamp);
    
    rmAssert(cacheFrameBufPtr->refCount() == 1);
    
    vfVideoCacheStatus status = cacheInsert(frameIndex, *cacheFrameBufPtr,
                                            dToken);
    
    rmAssert(frameBufPtr->frameIndex() == frameIndex);
    
    ADD_VID_TRACE(fnInternalGetFrame, false, frameIndex, frameBufPtr.mFrameBuf,
                  dToken);
    
    return status;
}

vfVideoCacheStatus
vfVideoCache::cacheAlloc(uint32 frameIndex,
                         rcSharedFrameBufPtr& userFrameBuf,
                         rcSharedFrameBufPtr*& cacheFrameBufPtr,
                         const uint32 dToken)
{
#ifndef VID_TRACE
    rcUNUSED(dToken);
#endif
    
    rcLock lock(_cacheMutex);
    ADD_VID_TRACE(fnCacheAlloc, true, frameIndex, userFrameBuf.mFrameBuf, dToken);
    
    /* First, look to see if the frame has been cached. If so, return
     * it to the caller.
     */
    map<uint32, rcSharedFrameBufPtr*>::iterator cachedEntryPtr;
    cachedEntryPtr = _cachedFramesItoB.find(frameIndex);
    
    if (cachedEntryPtr != _cachedFramesItoB.end()) {
        rcSharedFrameBufPtr* bufPtr = cachedEntryPtr->second;
        rmAssert((*bufPtr)->frameIndex() == frameIndex);
        
        /* If frame is in the unlocked map, remove it so no one else
         * tries to grab it.
         */
        map<uint32, int64>::iterator unlockedPtrItoL;
        unlockedPtrItoL = _unlockedFramesItoL.find(frameIndex);
        
        if (unlockedPtrItoL != _unlockedFramesItoL.end()) {
#ifdef VID_TRACE
            if (bufPtr->refCount() != 1) {
                fprintf(stderr, "C token %d fi %d frame* 0x%X\n", dToken, frameIndex,
                        (intptr_t)bufPtr->mFrameBuf);
                DUMP_VID_TRACE();
            }
#endif
            rmAssert(bufPtr->refCount() == 1);
            
            int64 lastUseIndex = unlockedPtrItoL->second;
            
            map<int64, uint32>::iterator unlockedPtrLtoI;
            unlockedPtrLtoI = _unlockedFramesLtoI.find(lastUseIndex);
            
            rmAssert(unlockedPtrLtoI != _unlockedFramesLtoI.end());
            rmAssert(unlockedPtrLtoI->second == frameIndex);
            
            _unlockedFramesItoL.erase(unlockedPtrItoL);
            _unlockedFramesLtoI.erase(unlockedPtrLtoI);
        }
        
        rmAssert(bufPtr->mCacheCtrl == 0);
        userFrameBuf.setCachedFrame(*bufPtr, _cacheID, frameIndex);
        
        _cacheHits++;
        ADD_VID_TRACE(fnCacheAlloc, false, frameIndex, userFrameBuf.mFrameBuf,
                      dToken);
        return eVideoCacheStatusOK;
    } // End of: if (cachedEntryPtr != _cachedFramesItoB.end())
    
    cacheFrameBufPtr = 0;
    
    /* Not currently in cache. If load is pending add the user frame
     * buffer to the pending list and return to caller to allow
     * him to wait for load to complete.
     */
    map<uint32, vector<rcSharedFrameBufPtr*> >::iterator pendingEntry;
    pendingEntry = _pending.find(frameIndex);
    if (pendingEntry != _pending.end()) {
        vector<rcSharedFrameBufPtr*>& pendingBuffers = pendingEntry->second;
        pendingBuffers.push_back(&userFrameBuf);
        return eVideoCacheStatusError;
    }
    
    /* Frame is neither in cache nor pending. Allocate a frame buffer
     * and return it to the caller. The caller is responsible for
     * loading the frame from disk and seeing that it gets linked
     * into the cache.
     */
    if ((_unusedCacheFrames.size() > _cacheOverflowLimit) ||
        (_unlockedFramesLtoI.empty())) {
        /* Grab an unused cache entry if either:
         * a) We haven't reached the cache's overflow limit, or
         * b) The unlocked portion of the cache is empty.
         */
        rmAssert(!_unusedCacheFrames.empty());
        cacheFrameBufPtr = _unusedCacheFrames.front();
        rmAssert(cacheFrameBufPtr->refCount() < 2);
        rmAssert(cacheFrameBufPtr->mCacheCtrl == 0);
        _unusedCacheFrames.pop_front();
        if (*cacheFrameBufPtr == 0) {
            *cacheFrameBufPtr =
            rcSharedFrameBufPtr(new rcFrame(frameWidth(),
                                            frameHeight(),
                                            frameDepth()));
            rmAssert(*cacheFrameBufPtr != 0);
            (*cacheFrameBufPtr)->cacheCtrl(_cacheID);
        }
#ifdef VID_TRACE
        if (cacheFrameBufPtr->refCount() != 1) {
            fprintf(stderr, "A token %d fi %d refCount %d frame* 0x%X\n", dToken,
                    frameIndex, cacheFrameBufPtr->refCount(),
                    (intptr_t)cacheFrameBufPtr->mFrameBuf);
            DUMP_VID_TRACE();
        }
#endif
    }
    else if (!_unlockedFramesLtoI.empty()) {
        /* Reuse the least-recently-used unlocked frame. Step 1 is to
         * figure out the LRU frame's frame index and then clear the
         * related info from the unlocked frame maps.
         */
        map<int64, uint32>::iterator unlockedPtrLtoI;
        unlockedPtrLtoI = _unlockedFramesLtoI.begin();
        rmAssert(unlockedPtrLtoI != _unlockedFramesLtoI.end());
        
        uint32 oldFrameIndex = unlockedPtrLtoI->second;
        
        map<uint32, int64>::iterator unlockedPtrItoL;
        unlockedPtrItoL = _unlockedFramesItoL.find(oldFrameIndex);
        
        rmAssert(unlockedPtrItoL != _unlockedFramesItoL.end());
        rmAssert(unlockedPtrItoL->first == unlockedPtrLtoI->second);
        rmAssert(unlockedPtrItoL->second == unlockedPtrLtoI->first);
        
        _unlockedFramesItoL.erase(unlockedPtrItoL);
        _unlockedFramesLtoI.erase(unlockedPtrLtoI);
        
        /* Step 2 - Remove association of frame buffer with old frame
         * index.
         */
        cachedEntryPtr = _cachedFramesItoB.find(oldFrameIndex);
        rmAssert(cachedEntryPtr != _cachedFramesItoB.end());
        cacheFrameBufPtr = cachedEntryPtr->second;
        if (!((*cacheFrameBufPtr)->frameIndex() == oldFrameIndex))
            printf("error %d %d\n", (*cacheFrameBufPtr)->frameIndex(),
                   oldFrameIndex);
        rmAssert((*cacheFrameBufPtr)->frameIndex() == oldFrameIndex);
        (*cacheFrameBufPtr)->frameIndex(0);
        
        _cachedFramesItoB.erase(cachedEntryPtr);
        
#ifdef VID_TRACE
        if (cacheFrameBufPtr->refCount() != 1) {
            fprintf(stderr, "B token %d fi %d refCount %d  frame* 0x%X\n", dToken,
                    frameIndex, cacheFrameBufPtr->refCount(),
                    (intptr_t)cacheFrameBufPtr->mFrameBuf);
            DUMP_VID_TRACE();
        }
#endif
        
        rmAssert(cacheFrameBufPtr->refCount() == 1);
    }
    else {
        rmAssert(0); // Should always be able to find a free frame buffer ptr.
        return eVideoCacheStatusError;
    }
    
    /* Setting cacheFrameBufPtr to non-null and returning the error
     * status acts as a signal that the frame should be read from disk.
     */
    rmAssert(cacheFrameBufPtr);
    rmAssert(cacheFrameBufPtr->mCacheCtrl == 0);
    rmAssert((*cacheFrameBufPtr)->frameIndex() == 0);
    _cacheMisses++;
    
    /* As final step, create an entry in the pending map to allow all
     * the consumers of this frame to get serviced once the frame is
     * available.
     */
    pair<map<uint32, vcPendingFills>::iterator, bool> ret;
    ret = _pending.insert(pair<uint32, vcPendingFills>(frameIndex,
                                                       vcPendingFills()));
    rmAssert(ret.second); // Check that insert succeeded
    
    vcPendingFills& pendingBuffers = ret.first->second;
    pendingBuffers.push_back(&userFrameBuf);
    
    ADD_VID_TRACE(fnCacheAlloc, false, frameIndex+20,
                  cacheFrameBufPtr->mFrameBuf, dToken);
    return eVideoCacheStatusError;
}

vfVideoCacheStatus
vfVideoCache::cacheInsert(uint32 frameIndex,
                          rcSharedFrameBufPtr& cacheFrameBuf,
                          const uint32 dToken)
{
#ifndef VID_TRACE
    rcUNUSED(dToken);
#endif
    
    rcLock lock(_cacheMutex);
    ADD_VID_TRACE(fnCacheInsert, true, frameIndex, userFrameBuf.mFrameBuf, dToken);
    
    rmAssert(cacheFrameBuf.mCacheCtrl == 0);
    rmAssert(cacheFrameBuf.refCount() == 1);
    
    /* Frame shouldn't be in cache
     */
    rmAssert(_cachedFramesItoB.find(frameIndex) == _cachedFramesItoB.end());
    
    rmAssert(cacheFrameBuf->frameIndex() == 0);
    cacheFrameBuf->frameIndex(frameIndex);
    _cachedFramesItoB[frameIndex] = &cacheFrameBuf;
    
    /* Frame is loaded into cache and ready to use. Set up reference
     * to it in all the pending frame buffer ptrs.
     */
    map<uint32, vcPendingFills>::iterator pendingEntry;
    pendingEntry = _pending.find(frameIndex);
    rmAssert(pendingEntry != _pending.end());
    
    vcPendingFills& pendingBuffers = pendingEntry->second;
    uint32 pendingCount = pendingBuffers.size();
    rmAssert(pendingCount);
    
    for (uint32 i = 0; i < pendingCount; i++)
        (pendingBuffers[i])->setCachedFrame(cacheFrameBuf, _cacheID,
                                            frameIndex);
    
    /* Finished with list of pending frame buffers. Delete entry
     * from pending map.
     */
    _pending.erase(pendingEntry);
    
    rmAssert(cacheFrameBuf.refCount() == int32(pendingCount + 1));
    
    /* Broadcast signal to waiting threads so they can check and see if
     * their frame buffer is ready.
     */
    if (pendingCount > 1)
        _pendingCtrl.broadcast();
    
    ADD_VID_TRACE(fnCacheInsert, false, frameIndex, userFrameBuf.mFrameBuf, dToken);
    return eVideoCacheStatusOK;
}

// Create default origin header
void vfVideoCache::createDefaultOriginHeader( movieFormatRev rev )
{
    rmAssert( rev < movieFormatRev2 );
    
    std::string creator;
    if ( rev == movieFormatRev0 )
        creator = "Visible/Batchconvert";
    else
        creator = "Batchconvert";
    rcMovieFileOrgExt orgHdr( movieOriginCaptureUncertified, baseTime(),
                             frameCount(), frameWidth(), frameHeight(), frameDepth(),
                             rev, creator.c_str() );
    _orgHdrs.push_back( orgHdr );
}


void vfVideoCache::unlockFrame(uint32 frameIndex)
{
#ifdef VID_TRACE
    const uint32 dToken = GET_TOKEN();
#endif
    
    rcLock lock(_cacheMutex);
    ADD_VID_TRACE(fnUnlockFrame, true, frameIndex, 0, dToken);
    
    map<uint32, rcSharedFrameBufPtr*>::iterator cachedEntryPtr;
    cachedEntryPtr = _cachedFramesItoB.find(frameIndex);
    
    /* If someone else has already removed this frame from the
     * cache there is nothing left to do.
     */
    if (cachedEntryPtr == _cachedFramesItoB.end()) {
        ADD_VID_TRACE(fnUnlockFrame, false, frameIndex, 0, dToken);
        return;
    }
    
    /* Check to see if some other thread has locked this frame
     * between the time the calling frame buffer decremented the
     * count and this function call locked the cache mutex.
     */
    rcSharedFrameBufPtr* bufPtr = cachedEntryPtr->second;
    const uint32 refCount = (*bufPtr).refCount();
    if (refCount > 1) {
        ADD_VID_TRACE(fnUnlockFrame, false, frameIndex + 10,
                      (*bufPtr).mFrameBuf, dToken);
        return;
    }
    
    rmAssert((*bufPtr).refCount() == 1);
    rmAssert((*bufPtr)->frameIndex() == frameIndex);
    rmAssert((*bufPtr)->cacheCtrl() == _cacheID);
    
    /* Check to see if some other thread has locked and unlocked this
     * frame between the time the calling frame buffer decremented the
     * count and this function call locked the cache mutex.
     */
    map<uint32, int64>::iterator unlockedPtrItoL;
    unlockedPtrItoL = _unlockedFramesItoL.find(frameIndex);
    
    if (unlockedPtrItoL != _unlockedFramesItoL.end()) {
        ADD_VID_TRACE(fnUnlockFrame, false, frameIndex + 20,
                      (*bufPtr).mFrameBuf, dToken);
        return;
    }
    
    /* Have confirmed this frame buffer should be added to unlocked
     * maps.
     *
     * Note: Wrap around is bad. Could implement an algorithm to
     * compress last touch values down, but for now I think it is
     * good enough to just use a 64 bit counter.
     */
    rmAssert(_lastTouchIndex >= 0);
    rmAssert(_unlockedFramesLtoI.find(_lastTouchIndex) == _unlockedFramesLtoI.end());
    
    _unlockedFramesLtoI[_lastTouchIndex] = frameIndex;
    _unlockedFramesItoL[frameIndex] = _lastTouchIndex;
    _lastTouchIndex++;
    
    /* If the cache is currently overflowing its bounds, remove the LRU
     * element.
     */
    if (_unusedCacheFrames.size() < _cacheOverflowLimit) {
        /* Remove the least-recently-used unlocked frame. Step 1 is to
         * figure out the LRU frame's frame index and then clear the
         * related info from the unlocked frame maps.
         */
        map<int64, uint32>::iterator unlockedPtrLtoI;
        unlockedPtrLtoI = _unlockedFramesLtoI.begin();
        rmAssert(unlockedPtrLtoI != _unlockedFramesLtoI.end());
        
        uint32 oldFrameIndex = unlockedPtrLtoI->second;
        
        map<uint32, int64>::iterator unlockedPtrItoL;
        unlockedPtrItoL = _unlockedFramesItoL.find(oldFrameIndex);
        
        rmAssert(unlockedPtrItoL != _unlockedFramesItoL.end());
        rmAssert(unlockedPtrItoL->first == unlockedPtrLtoI->second);
        rmAssert(unlockedPtrItoL->second == unlockedPtrLtoI->first);
        
        _unlockedFramesItoL.erase(unlockedPtrItoL);
        _unlockedFramesLtoI.erase(unlockedPtrLtoI);
        
        /* Step 2 is to remove it from the cache entirely.
         */
        cachedEntryPtr = _cachedFramesItoB.find(oldFrameIndex);
        rmAssert(cachedEntryPtr != _cachedFramesItoB.end());
        bufPtr = cachedEntryPtr->second;
        _cachedFramesItoB.erase(cachedEntryPtr);
        
        /* Finally, free the associated buffer and put it back on the
         * unused list.
         */
        *bufPtr = 0;
        _unusedCacheFrames.push_back(bufPtr);
        return;
    }
    
    ADD_VID_TRACE(fnUnlockFrame, false, frameIndex, (*bufPtr).mFrameBuf, dToken);
}

void vfVideoCache::prefetchFrame(uint32 frameIndex)
{
    if (_prefetchThread)
        _prefetchThread->prefetch(frameIndex);
}

void vfVideoCache::cachePrefetch(uint32 cacheID, uint32 frameIndex)
{
    rcLock lock(_cacheMgmtMutex);
    
    map<uint32, vfVideoCache*>::iterator loc = _activeCachesItoP.find(cacheID);
    if (loc == _activeCachesItoP.end())
        return;
    
    vfVideoCache* cacheP = loc->second;
    cacheP->prefetchFrame(frameIndex);
}

void vfVideoCache::cacheUnlock(uint32 cacheID, uint32 frameIndex)
{
    rcLock lock(_cacheMgmtMutex);
    
    map<uint32, vfVideoCache*>::iterator loc = _activeCachesItoP.find(cacheID);
    if (loc == _activeCachesItoP.end())
        return;
    
    vfVideoCache* cacheP = loc->second;
    cacheP->unlockFrame(frameIndex);
}

vfVideoCacheStatus vfVideoCache::cacheLock(uint32 cacheID, uint32 frameIndex,
                                           rcSharedFrameBufPtr& frameBuf,
                                           vfVideoCacheError* error)
{
    rcLock lock(_cacheMgmtMutex);
    
    map<uint32, vfVideoCache*>::iterator loc = _activeCachesItoP.find(cacheID);
    if (loc != _activeCachesItoP.end()) {
        vfVideoCache* cacheP = loc->second;
        return cacheP->getFrame(frameIndex, frameBuf, error);
    }
    
    if (error)
        *error = eVideoCacheErrorCacheInvalid;
    
    return eVideoCacheStatusError;
}

/************************************************************************/
/************************************************************************/
/*                                                                      */
/*                vfVideoCachePrefetchUnit Implementation               */
/*                                                                      */
/************************************************************************/
/************************************************************************/

vfVideoCache::vfVideoCachePrefetchUnit::vfVideoCachePrefetchUnit(vfVideoCache& cacheCtrl)
: _cacheCtrl(cacheCtrl), _wait(0)
{
    rmAssert(cacheCtrl.isValid());
}

vfVideoCache::vfVideoCachePrefetchUnit::~vfVideoCachePrefetchUnit()
{
}

void vfVideoCache::vfVideoCachePrefetchUnit::run()
{
    uint32 frameIndex = 0;
    bool active = true;
    
    bool killMe = false;
    while (!killMe) {
        bool wait = false;
        {
            rcLock lock(_prefetchMutex);
            
            if (_prefetchRequests.empty())
                wait = true;
            else {
                frameIndex = _prefetchRequests.front();
                _prefetchRequests.pop_front();
            }
        }
        
        /* Guard against chance that above check of prefetch queue
         * didn't occur AFTER dtor gets called. (Because dtor
         * makes dummy prefetch request with disk mutex locked - this
         * will cause hang when this fuction tries to load frame from
         * disk)
         */
        killMe = seppukuRequested();
        if (wait) {
            int32 curVal = _wait.waitUntilGreaterThan(0);
            _wait.decrementVariable(curVal, 0);
        }
        else if (active && !killMe) {
            /* Have a frame to prefetch. Do this by using getFrame() to load
             * the frame into cache. Since we don't want to lock this frame
             * into memory, merely load it into cache, the frame is set up
             * in a temporary frame buffer.
             */
            vfVideoCacheError error;
            rcSharedFrameBufPtr frameBuf;
            vfVideoCacheStatus status = _cacheCtrl.getFrame(frameIndex, frameBuf,
                                                            &error);
            /* Stop processing if something bad happens */
            if ((status == eVideoCacheStatusError) &&
                (error != eVideoCacheErrorNoSuchFrame)) {
                cerr << "Prefetch error: " << _cacheCtrl.getErrorString(error) << endl;
                active = false;
            }
        }
    }
}

void vfVideoCache::vfVideoCachePrefetchUnit::prefetch(uint32 frameIndex)
{
    bool empty;
    {
        rcLock lock(_prefetchMutex);
        empty = _prefetchRequests.empty();
        
        _prefetchRequests.push_back(frameIndex);
    }
    
    if (empty)
        _wait.incrementVariable(1, 0);
}
    


