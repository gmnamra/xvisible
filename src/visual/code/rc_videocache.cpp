// Copyright 2003 Reify, Inc.

#include <rc_systeminfo.h>
#include <rc_videocache.h>
#include <stdio.h>

#define VID_TRACE

// Only use this for doubles and floats for now
#define ByteSwapN(x) ByteSwap((unsigned char *) &x,sizeof(x))

static void ByteSwap(unsigned char * b, int n)
{
   register int i = 0;
   register int j = n-1;
   while (i<j)
   {
      std::swap(b[i], b[j]);
      i++, j--;
   }
}

// Perform endian reversal if necessary
static void fixEndian( int64& time )
{
    if ( rfPlatformByteOrder() !=  eByteOrderBigEndian) {
	    ByteSwapN (time);
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
static uint32 traceIndex = 0;
static uint32 traceCount = 0;
static rcMutex  traceMutex;
static uint32 tracePrint = 0;

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
                traceBuf[myIndex].frameIndex, (int)traceBuf[myIndex].fp);
        myIndex++;
    }
}

uint32 rcVideoCache::getNextToken()
{
    rcLock lock(_cacheMutex); return _debuggingToken++;
}

#endif

map <uint32, rcVideoCache*> rcVideoCache::_activeCachesItoP;
map <rcVideoCache*, uint32> rcVideoCache::_activeCachesPtoI;
uint32                      rcVideoCache::_nextCacheID = 0;
rcMutex                       rcVideoCache::_cacheMgmtMutex;

rcVideoCache* rcVideoCache::rcVideoCacheCtor(const std::string fileName,
                                             uint32 cacheSize,
                                             bool getTOC, bool verbose,
                                             bool prefetch,
                                             uint32 maxMemory,
                                             rcProgressIndicator* pIndicator)
{
    return finishSetup(new rcVideoCache(fileName, cacheSize, getTOC,
                                        verbose, prefetch, maxMemory, pIndicator));
}

rcVideoCache*
rcVideoCache::rcVideoCacheUTCtor(const vector<rcTimestamp>& frameTimes)
{
    return finishSetup(new rcVideoCache(frameTimes));
}

rcVideoCache* rcVideoCache::finishSetup(rcVideoCache* cacheP)
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

void rcVideoCache::rcVideoCacheDtor(rcVideoCache* cacheP)
{
    {
        rcLock lock(_cacheMgmtMutex);

        map<rcVideoCache*, uint32>::iterator locI = _activeCachesPtoI.find(cacheP);
        if (locI == _activeCachesPtoI.end())
            return;

        uint32 cacheID = locI->second;

        map<uint32, rcVideoCache*>::iterator locP = _activeCachesItoP.find(cacheID);

        rmAssert(locP != _activeCachesItoP.end());
        rmAssert(locI->first == locP->second);
        rmAssert(locP->first == locI->second);

        _activeCachesItoP.erase(locP);
        _activeCachesPtoI.erase(locI);
    }

    delete cacheP;
}

rcVideoCache::rcVideoCache(std::string fileName, uint32 cacheSize,
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
    _frameWidth = _frameHeight = _frameCount = 0;
    _frameDepth = rcPixel8;
    _averageFrameRate = 0.0;
    _baseTime = 0;

    _byteOrder = eByteOrderUnknown;
    if (_fileName.empty()) {
        setError(eVideoCacheErrorFileInit);
        return;
    }

    if ((_movieFile = fopen(_fileName.data(), "r")) == NULL)
    {
        if (_verbose) perror("fopen failed");
        setError(eVideoCacheErrorFileInit);
        return;
    }

    /* Read movie identifier and check that we support this type of file.
     */
    rcMovieFileIdentifier id;

    if (fread(&id, sizeof(rcMovieFileIdentifier), 1, _movieFile) != 1) {
        if (_verbose) perror("Read of identifier failed");
        setError(eVideoCacheErrorFileRead);
        return;
    }
    id.fixEndian();

    if (fseek( _movieFile, 0, SEEK_SET)) {
        if (_verbose) perror("fseek to start failed");
        setError(eVideoCacheErrorFileRead);
        return;
    }

    // Check identifier validity
    if ( !id.isValid() ) {
        if (_verbose) perror("Invalid file identifier");
        setError(eVideoCacheErrorFileFormat);
        return;
    } else if ((id.rev() > movieFormatRevLatest)) {
        if (_verbose) perror("Unsupported file revision");
        setError(eVideoCacheErrorFileRevUnsupported);
        return;
    }

    _rev = id.rev();
    rcVideoCacheError error = eVideoCacheErrorOK;

    // Load headers
    switch ( _rev ) {
        case movieFormatRev0:
            error = headerLoadRev0();
            createDefaultOriginHeader( _rev );
            break;
        case movieFormatRev1:
            // We may read TOC from frames so progress indication is
            // needed for long movies
            error = headerLoadRev1( getTOC );
            createDefaultOriginHeader( _rev );
            break;
        case movieFormatRev2:
            error = headerLoadRev2( getTOC );
            break;
        case movieFormatInvalid:
            error = eVideoCacheErrorFileFormat;
            break;
    }

    if ( error != eVideoCacheErrorOK )
        return;

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
        _prefetchThread = new rcVideoCachePrefetchUnit(*this);
        rmAssert(_prefetchThread);
        _prefetchThread->start();
    }
}

rcVideoCache::rcVideoCache(const vector<rcTimestamp>& frameTimes)
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

rcVideoCache::~rcVideoCache()
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

rcVideoCacheStatus rcVideoCache::getFrame(uint32 frameIndex,
                                          rcSharedFrameBufPtr& frameBuf,
                                          rcVideoCacheError* error,
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

    rcVideoCacheStatus status = eVideoCacheStatusOK;
    if (locked)
        status = internalGetFrame(frameIndex, frameBuf, error, dToken);
    else
        frameBuf.setCachedFrameIndex(_cacheID, frameIndex);

    ADD_VID_TRACE(fnGetFrameI, false, frameIndex, frameBuf.mFrameBuf, dToken);

    return status;
}

rcVideoCacheStatus rcVideoCache::getFrame(const rcTimestamp& time,
                                          rcSharedFrameBufPtr& frameBuf,
                                          rcVideoCacheError* error,
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

    rcVideoCacheError status = tocLoad();
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

    rcVideoCacheStatus status2 = eVideoCacheStatusOK;
    if (locked)
        status2 = internalGetFrame(frameIndexPtr->second, frameBuf, error, dToken);
    else
        frameBuf.setCachedFrameIndex(_cacheID, frameIndexPtr->second);

    ADD_VID_TRACE(fnGetFrameT, false, frameIndexPtr->second,
                  frameBuf.mFrameBuf, dToken);

    return status2;
}

// Static method for mapping an error value to a string
std::string rcVideoCache::getErrorString(rcVideoCacheError error)
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

rcVideoCacheError rcVideoCache::getFatalError() const
{
    rcLock lock(const_cast<rcVideoCache*>(this)->_cacheMutex);

    return _fatalError;
}

// Returns the timestamp in the movie closest to the goal time.
rcVideoCacheStatus
rcVideoCache::closestTimestamp(const rcTimestamp& goalTime,
                               rcTimestamp& match,
                               rcVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }

    rcVideoCacheError status = tocLoad();
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
        rcTimestamp afterGoal = it->first;
        it--;
        rcTimestamp beforeOrEqualGoal = it->first;

        if ((afterGoal - goalTime) < (goalTime - beforeOrEqualGoal))
            match = afterGoal;
        else
            match = beforeOrEqualGoal;
    }

    return eVideoCacheStatusOK;
}

// Returns first timestamp > goalTime.
rcVideoCacheStatus
rcVideoCache::nextTimestamp(const rcTimestamp& goalTime,
                            rcTimestamp& match,
                            rcVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }

    rcVideoCacheError status = tocLoad();
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
rcVideoCacheStatus
rcVideoCache::prevTimestamp(const rcTimestamp& goalTime,
                            rcTimestamp& match,
                            rcVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }

    rcVideoCacheError status = tocLoad();
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
rcVideoCacheStatus
rcVideoCache::firstTimestamp(rcTimestamp& match,
                             rcVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }

    rcVideoCacheError status = tocLoad();
    if (status != eVideoCacheErrorOK) {
        if (error) *error = status;
        return eVideoCacheStatusError;
    }

    rmAssert(!_tocTtoI.empty());

    match = _tocTtoI.begin()->first;

    return eVideoCacheStatusOK;
}

// Returns the timestamp of the last frame in the movie.
rcVideoCacheStatus
rcVideoCache::lastTimestamp(rcTimestamp& match,
                            rcVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }

    rcVideoCacheError status = tocLoad();
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
rcVideoCacheStatus
rcVideoCache::frameIndexToTimestamp(uint32 frameIndex,
                                    rcTimestamp& match,
                                    rcVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }

    rcVideoCacheError status = tocLoad();
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
rcVideoCacheStatus
rcVideoCache::timestampToFrameIndex(const rcTimestamp& timestamp,
                                    uint32& match,
                                    rcVideoCacheError* error)
{
    if (!isValid()) {
        if (error) *error = eVideoCacheErrorCacheInvalid;
        return eVideoCacheStatusError;
    }

    rcVideoCacheError status = tocLoad();
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

bool rcVideoCache::isValid() const
{
    rcLock lock(const_cast<rcVideoCache*>(this)->_cacheMutex);

    return _isValid;
}

// Offset from start of file to start of frame data
off_t rcVideoCache::frameDataOffset( const uint32& frameIndex ) const
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

uint32 rcVideoCache::cacheMisses() const
{
    rcLock lock(const_cast<rcVideoCache*>(this)->_cacheMutex);

    return _cacheMisses;
}

uint32 rcVideoCache::cacheHits() const
{
    rcLock lock(const_cast<rcVideoCache*>(this)->_cacheMutex);

    return _cacheHits;
}

void rcVideoCache::setError (rcVideoCacheError error)
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

rcVideoCacheStatus
rcVideoCache::internalGetFrame(uint32 frameIndex,
                               rcSharedFrameBufPtr& frameBufPtr,
                               rcVideoCacheError* error,
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
    int64 timeInfo;

    {
        rcLock lock(_diskMutex);
        rmAssert(_movieFile);

        /* Read in the frame from disk.
         */
        off_t offset = frameDataOffset( frameIndex );
        if (fseeko(_movieFile, offset, SEEK_SET)) {
            if (_verbose) perror("fseek during cache fill failed");
            setError(eVideoCacheErrorFileSeek);
            if (error) *error = eVideoCacheErrorFileSeek;
            ADD_VID_TRACE(fnInternalGetFrame, false, frameIndex,
                          frameBufPtr.mFrameBuf, dToken);
            /* Restore cache ID and frame index which were cleared by
             * calling function.
             */
            frameBufPtr.mCacheCtrl = _cacheID;
            frameBufPtr.mFrameIndex = frameIndex;
            return eVideoCacheStatusError;
        }

        if (fread(&timeInfo, sizeof(int64), 1, _movieFile) != 1) {
            if (_verbose) perror("fread of timestamp during cache fill failed");
            setError(eVideoCacheErrorFileRead);
            if (error) *error = eVideoCacheErrorFileRead;
            ADD_VID_TRACE(fnInternalGetFrame, false, frameIndex,
                          frameBufPtr.mFrameBuf, dToken);
            /* Restore cache ID and frame index which were cleared by
             * calling function.
             */
            frameBufPtr.mCacheCtrl = _cacheID;
            frameBufPtr.mFrameIndex = frameIndex;
            return eVideoCacheStatusError;
        }
        fixEndian(timeInfo);
        timestamp = timeInfo;

        if (fread((*cacheFrameBufPtr)->alignedRawData(), _bytesInFrame, 1,
                  _movieFile) != 1) {
            if (_verbose) perror("fread of frame data during cache fill failed");
            setError(eVideoCacheErrorFileRead);
            if (error) *error = eVideoCacheErrorFileRead;
            ADD_VID_TRACE(fnInternalGetFrame, false, frameIndex,
                          frameBufPtr.mFrameBuf, dToken);
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

    rcVideoCacheStatus status = cacheInsert(frameIndex, *cacheFrameBufPtr,
                                            dToken);

    rmAssert(frameBufPtr->frameIndex() == frameIndex);

    ADD_VID_TRACE(fnInternalGetFrame, false, frameIndex, frameBufPtr.mFrameBuf,
                  dToken);

    return status;
}

rcVideoCacheStatus
rcVideoCache::cacheAlloc(uint32 frameIndex,
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
                        (int)bufPtr->mFrameBuf);
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
                    (int)cacheFrameBufPtr->mFrameBuf);
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
                    (int)cacheFrameBufPtr->mFrameBuf);
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

rcVideoCacheStatus
rcVideoCache::cacheInsert(uint32 frameIndex,
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
void rcVideoCache::createDefaultOriginHeader( movieFormatRev rev )
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

rcVideoCacheError rcVideoCache::headerLoadRev0()
{
    rcMovieFileFormat movieHdr;

    // Legacy format is always big-endian
    _byteOrder = eByteOrderBigEndian;

    /* Read movie header and check that we support this type of file.
     */
    if (fread(&movieHdr, sizeof(movieHdr), 1, _movieFile) != 1) {
        if (_verbose) perror("fread of header failed");
        setError(eVideoCacheErrorFileRead);
        return _fatalError;
    }
    movieHdr.fixEndian();

    if ((movieHdr.rowUpdate() & 0xF) != 0) {
        if (_verbose) {
            char buf[256];
            snprintf( buf, rmDim(buf), "Invalid header row update %i",
                      movieHdr.rowUpdate() );
            perror(buf);
        }
        setError(eVideoCacheErrorFileUnsupported);
        return _fatalError;
    }

    _bytesInFrame = movieHdr.bytesInFrame();
    _frameCount = movieHdr.frameCount();
    _frameWidth = movieHdr.width();
    _frameHeight = movieHdr.height();
    _averageFrameRate = movieHdr.averageFrameRate();
    _baseTime = movieHdr.baseTime();

    if (_verbose)
        cerr << movieHdr << endl;

    return eVideoCacheErrorOK;
}

rcVideoCacheError rcVideoCache::headerLoadRev1( bool getTOC )
{
    rcVideoCacheError error = headerLoadRev0();

    if ( error == eVideoCacheErrorOK ) {
        /* Find the location of supported extensions, ignoring all others.
         */
        off_t offset = frameDataOffset( _frameCount );
        rcMovieFileExt ext;

        do {
            if (fseeko(_movieFile, offset, SEEK_SET)) {
                if (_verbose) perror("Seek to header extension failed");
                setError(eVideoCacheErrorFileSeek);
                return _fatalError;
            }

            if (fread(&ext, sizeof(ext), 1, _movieFile) != 1) {
                if (_verbose) perror("Read of header extension failed");
                setError(eVideoCacheErrorFileRead);
                return _fatalError;
            }
            ext.fixEndian(_byteOrder);

            switch ( ext.type() ) {
                case movieExtensionTOC:
                    _tocExtHdrOffset = offset;
                    break;
                case movieExtensionEOF:
                    break;
                default:
                    cerr << "Warning: unsupported extension " << ext.type() << " found in rev" << rev()
                         << " file " << _fileName << endl;
                    break;
            }
            offset += ext.offset();
        } while (ext.type() != movieExtensionEOF);
    }

    if ( getTOC ) {
        error = tocLoad();
        if ( error != eVideoCacheErrorOK)
            return error;
    }

    return error;
}

rcVideoCacheError rcVideoCache::headerLoadRev2( bool getTOC )
{
    rcVideoCacheError error = eVideoCacheErrorOK;
    rcMovieFileFormat2 movieHdr;

    /* Read movie header and check that we support this type of file.
     */
    if (fread(&movieHdr, sizeof(movieHdr), 1, _movieFile) != 1) {
        if (_verbose) perror("Read of header2 failed");
        setError(eVideoCacheErrorFileRead);
        return _fatalError;
    }

    // check the bom (note: before endian fix)
    if ( rfPlatformByteOrder( movieHdr.bom() ) == eByteOrderUnknown ) {
        if (_verbose) perror("Unsupported BOM");
        setError(eVideoCacheErrorBomUnsupported);
        return _fatalError;
    }

    movieHdr.fixEndian();

    if ( movieHdr.depth() < rcPixel8 || movieHdr.depth() > rcPixel32 ) {
        if (_verbose) {
            char buf[256];
            snprintf( buf, rmDim(buf), "Unsupported pixel depth %i",
                      movieHdr.depth()*8 );
            perror(buf);
        }
        setError(eVideoCacheErrorDepthUnsupported);
        return _fatalError;
    }
    if ( movieHdr.extensionOffset() == 0 ) {
        if (_verbose) perror("Invalid extension offset");
        setError(eVideoCacheErrorFileFormat);
        return _fatalError;
    }

    _bytesInFrame = movieHdr.bytesInFrame();
    _frameCount = movieHdr.frameCount();
    _frameWidth = movieHdr.width();
    _frameHeight = movieHdr.height();
    _frameDepth = movieHdr.depth();
    _averageFrameRate = movieHdr.averageFrameRate();
 // Legacy format is always big-endian
    _byteOrder = rfPlatformByteOrder (movieHdr.bom() );
    _bytesInFrame = movieHdr.height() * movieHdr.rowUpdate();

    /* Find the location of supported extensions, ignoring all others.
     */
    off_t offset = frameDataOffset( _frameCount );
    rcMovieFileExt ext;

    do {
        if (fseeko(_movieFile, offset, SEEK_SET)) {
            if (_verbose) perror("Seek to header extension failed");
            setError(eVideoCacheErrorFileSeek);
            return _fatalError;
        }

        if (fread(&ext, sizeof(ext), 1, _movieFile) != 1) {
            if (_verbose) perror("Read of header extension failed");
            setError(eVideoCacheErrorFileRead);
            return _fatalError;
        }
        ext.fixEndian(_byteOrder);

        switch ( ext.type() ) {
            case movieExtensionTOC:
                _tocExtHdrOffset = offset;
                break;
            case movieExtensionORG:
                _orgExtHdrOffsets.push_back(offset);
                break;
            case movieExtensionCNV:
                _cnvExtHdrOffsets.push_back(offset);
                break;
            case movieExtensionCAM:
                _camExtHdrOffsets.push_back(offset);
                break;
            case movieExtensionEXP:
                _expExtHdrOffsets.push_back(offset);
                break;
            case movieExtensionEOF:
                break;
            default:
                cerr << "Warning: unsupported extension " << ext.type() << " found in rev" << rev()
                     << " file " << _fileName << endl;
                break;
        }

        offset += ext.offset();
    } while (ext.type() != movieExtensionEOF);

    if (_verbose)
        cerr << movieHdr << endl;

    if ( getTOC ) {
        error = tocLoad();
        if ( error != eVideoCacheErrorOK)
            return error;
    }
    if ( !_orgExtHdrOffsets.empty() ) {
        error =  orgLoad();
        if ( error != eVideoCacheErrorOK )
            return error;
    }

    if ( !_cnvExtHdrOffsets.empty() ) {
        error = cnvLoad() ;
        if ( error != eVideoCacheErrorOK )
            return error;
    }

    if ( !_camExtHdrOffsets.empty() ) {
        error = camLoad() ;
        if ( error != eVideoCacheErrorOK )
            return error;
    }

    if ( !_expExtHdrOffsets.empty() ) {
        error = expLoad() ;
        if ( error != eVideoCacheErrorOK )
            return error;
    }

    return error;
}

rcVideoCacheError rcVideoCache::tocLoad()
{
    rcLock lock(_diskMutex);

    if (!_tocItoT.empty())
        return eVideoCacheErrorOK;

    _tocItoT.resize(_frameCount);
    rmAssert(_tocTtoI.empty());
    rmAssert(_movieFile);

    if (_tocExtHdrOffset != -1)
        return tocLoadFromTOC();

    return tocLoadFromFrames();
}

rcVideoCacheError rcVideoCache::tocLoadFromFrames()
{
    if (fseek(_movieFile, sizeof(rcMovieFileFormat), SEEK_SET)) {
        if (_verbose) perror("fseek during toc build failed");
        setError(eVideoCacheErrorFileSeek);
        return eVideoCacheErrorFileSeek;
    }

    for (uint32 frameIndex = 0; frameIndex < _frameCount; frameIndex++) {
        int64 timeInfo = 0;

        /* Skip seek for first frame because it was done before entering
         * loop.
         */
        if (frameIndex && fseek(_movieFile, _bytesInFrame, SEEK_CUR)) {
            if (_verbose) perror("fseek during toc build failed");
            setError(eVideoCacheErrorFileSeek);
            return eVideoCacheErrorFileSeek;
        }

        if (fread(&timeInfo, sizeof(int64), 1, _movieFile) != 1) {
            if (_verbose) perror("fread during toc build failed");
            setError(eVideoCacheErrorFileRead);
            return eVideoCacheErrorFileRead;
        }
        fixEndian(timeInfo);

        rcTimestamp timestamp(timeInfo);

        _tocItoT[frameIndex] = timestamp;
        _tocTtoI[timestamp] = frameIndex;
        if ( _progressIndicator )
             _progressIndicator->progress( 100.0 * frameIndex/_frameCount );
    }

    return eVideoCacheErrorOK;
}

rcVideoCacheError rcVideoCache::tocLoadFromTOC()
{
    rmAssert(_tocExtHdrOffset != -1);

    if (fseeko(_movieFile, _tocExtHdrOffset, SEEK_SET)) {
        if (_verbose) perror("fseeko during toc build failed");
        setError(eVideoCacheErrorFileSeek);
        return eVideoCacheErrorFileSeek;
    }

    rcMovieFileTocExt tocHdr;
    if (fread(&tocHdr, sizeof(tocHdr), 1, _movieFile) != 1) {
        if (_verbose) perror("fread during toc build failed");
        setError(eVideoCacheErrorFileRead);
        return eVideoCacheErrorFileRead;
    }
    tocHdr.fixEndian(_byteOrder);

    if (_verbose)
        cerr << tocHdr << endl;
    rmAssert(tocHdr.type() == movieExtensionTOC);
    if (tocHdr.count() != _frameCount) {
        if (_verbose) cerr << "toc header count " << tocHdr.count()
                           << " != frame count " << _frameCount << endl;
        setError(eVideoCacheErrorFileFormat);
        return eVideoCacheErrorFileFormat;
    }

    for (uint32 frameIndex = 0; frameIndex < _frameCount; frameIndex++) {
        int64 timeInfo = 0;

        if (fread(&timeInfo, sizeof(int64), 1, _movieFile) != 1) {
            if (_verbose) perror("fread during toc build failed");
            setError(eVideoCacheErrorFileRead);
            return eVideoCacheErrorFileRead;
        }
        fixEndian(timeInfo);
        rcTimestamp timestamp(timeInfo);

        _tocItoT[frameIndex] = timestamp;
        _tocTtoI[timestamp] = frameIndex;
    }

    return eVideoCacheErrorOK;
}

// Load origin extension
rcVideoCacheError rcVideoCache::orgLoad()
{
    for ( uint32 i = 0; i < _orgExtHdrOffsets.size(); ++i ) {
        rmAssert( _orgExtHdrOffsets[i] != -1);

        if (fseeko(_movieFile, _orgExtHdrOffsets[i], SEEK_SET)) {
            if (_verbose) perror("fseeko during org build failed");
            setError(eVideoCacheErrorFileSeek);
            return eVideoCacheErrorFileSeek;
        }

        rcMovieFileOrgExt orgHdr;
        if (fread(&orgHdr, sizeof(orgHdr), 1, _movieFile) != 1) {
            if (_verbose) perror("fread during org build failed");
            setError(eVideoCacheErrorFileRead);
            return eVideoCacheErrorFileRead;
        }
        orgHdr.fixEndian(_byteOrder);

        rmAssert(orgHdr.type() == movieExtensionORG);
        _orgHdrs.push_back(orgHdr);

        if (_verbose)
            cerr << orgHdr << endl;
    }

    return eVideoCacheErrorOK;
}

// Load conversion extensions
rcVideoCacheError rcVideoCache::cnvLoad()
{
    for ( uint32 i = 0; i < _cnvExtHdrOffsets.size(); ++i ) {
        rmAssert(_cnvExtHdrOffsets[i] != -1);

        if (fseeko(_movieFile, _cnvExtHdrOffsets[i] , SEEK_SET)) {
            if (_verbose) perror("fseeko during cnv build failed");
            setError(eVideoCacheErrorFileSeek);
            return eVideoCacheErrorFileSeek;
        }

        rcMovieFileConvExt cnvHdr;
        if (fread(&cnvHdr, sizeof(cnvHdr), 1, _movieFile) != 1) {
            if (_verbose) perror("fread during cnv build failed");
            setError(eVideoCacheErrorFileRead);
            return eVideoCacheErrorFileRead;
        }
        cnvHdr.fixEndian(_byteOrder);
        rmAssert(cnvHdr.type() == movieExtensionCNV);
        _cnvExtHdrs.push_back( cnvHdr );

        if (_verbose)
            cerr << cnvHdr << endl;
    }

    return eVideoCacheErrorOK;
}

// Load camera extensions
rcVideoCacheError rcVideoCache::camLoad()
{
    for ( uint32 i = 0; i < _camExtHdrOffsets.size(); ++i ) {
        rmAssert(_camExtHdrOffsets[i] != -1);

        if (fseeko(_movieFile, _camExtHdrOffsets[i] , SEEK_SET)) {
            if (_verbose) perror("fseeko during cam build failed");
            setError(eVideoCacheErrorFileSeek);
            return eVideoCacheErrorFileSeek;
        }

        rcMovieFileCamExt camHdr;
        if (fread(&camHdr, sizeof(camHdr), 1, _movieFile) != 1) {
            if (_verbose) perror("fread during cam build failed");
            setError(eVideoCacheErrorFileRead);
            return eVideoCacheErrorFileRead;
        }
        camHdr.fixEndian(_byteOrder);
        rmAssert(camHdr.type() == movieExtensionCAM);
        _camExtHdrs.push_back( camHdr );

        if (_verbose)
            cerr << camHdr << endl;
    }

    return eVideoCacheErrorOK;
}

// Load experiment extensions
rcVideoCacheError rcVideoCache::expLoad()
{
    for ( uint32 i = 0; i < _expExtHdrOffsets.size(); ++i ) {
        rmAssert(_expExtHdrOffsets[i] != -1);

        if (fseeko(_movieFile, _expExtHdrOffsets[i] , SEEK_SET)) {
            if (_verbose) perror("fseeko during exp build failed");
            setError(eVideoCacheErrorFileSeek);
            return eVideoCacheErrorFileSeek;
        }

        rcMovieFileExpExt expHdr;
        if (fread(&expHdr, sizeof(expHdr), 1, _movieFile) != 1) {
            if (_verbose) perror("fread during exp build failed");
            setError(eVideoCacheErrorFileRead);
            return eVideoCacheErrorFileRead;
        }
        expHdr.fixEndian(_byteOrder);
        rmAssert(expHdr.type() == movieExtensionEXP);
        _expExtHdrs.push_back( expHdr );

        if (_verbose)
            cerr << expHdr << endl;
    }

    return eVideoCacheErrorOK;
}

void rcVideoCache::unlockFrame(uint32 frameIndex)
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

void rcVideoCache::prefetchFrame(uint32 frameIndex)
{
    if (_prefetchThread)
        _prefetchThread->prefetch(frameIndex);
}

void rcVideoCache::cachePrefetch(uint32 cacheID, uint32 frameIndex)
{
    rcLock lock(_cacheMgmtMutex);

    map<uint32, rcVideoCache*>::iterator loc = _activeCachesItoP.find(cacheID);
    if (loc == _activeCachesItoP.end())
        return;

    rcVideoCache* cacheP = loc->second;
    cacheP->prefetchFrame(frameIndex);
}

void rcVideoCache::cacheUnlock(uint32 cacheID, uint32 frameIndex)
{
    rcLock lock(_cacheMgmtMutex);

    map<uint32, rcVideoCache*>::iterator loc = _activeCachesItoP.find(cacheID);
    if (loc == _activeCachesItoP.end())
        return;

    rcVideoCache* cacheP = loc->second;
    cacheP->unlockFrame(frameIndex);
}

rcVideoCacheStatus rcVideoCache::cacheLock(uint32 cacheID, uint32 frameIndex,
                                           rcSharedFrameBufPtr& frameBuf,
                                           rcVideoCacheError* error)
{
    rcLock lock(_cacheMgmtMutex);

    map<uint32, rcVideoCache*>::iterator loc = _activeCachesItoP.find(cacheID);
    if (loc != _activeCachesItoP.end()) {
        rcVideoCache* cacheP = loc->second;
        return cacheP->getFrame(frameIndex, frameBuf, error);
    }

    if (error)
        *error = eVideoCacheErrorCacheInvalid;

    return eVideoCacheStatusError;
}

/************************************************************************/
/************************************************************************/
/*                                                                      */
/*                rcVideoCachePrefetchUnit Implementation               */
/*                                                                      */
/************************************************************************/
/************************************************************************/

rcVideoCache::rcVideoCachePrefetchUnit::rcVideoCachePrefetchUnit(rcVideoCache& cacheCtrl)
        : _cacheCtrl(cacheCtrl), _wait(0)
{
    rmAssert(cacheCtrl.isValid());
}

rcVideoCache::rcVideoCachePrefetchUnit::~rcVideoCachePrefetchUnit()
{
}

void rcVideoCache::rcVideoCachePrefetchUnit::run()
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
            rcVideoCacheError error;
            rcSharedFrameBufPtr frameBuf;
            rcVideoCacheStatus status = _cacheCtrl.getFrame(frameIndex, frameBuf,
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

void rcVideoCache::rcVideoCachePrefetchUnit::prefetch(uint32 frameIndex)
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
