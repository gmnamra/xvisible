/******************************************************************************
 *   Copyright (c) 2002 Reify Corp. All Rights reserved.
 *
 *	rc_engineimplcapture.cpp
 *
 *	This file contains the engine video capture support.
 *
 ******************************************************************************/
#include <strstream>

#include "rc_engineimpl.h"
#include "rc_engineimplcapture.h"
#include "rc_engineimplsave.h"
#include <rc_dcam.h>

#define DEBUG_LOG 1
#ifdef rcSIMULATED_CAMERA
// Hack to produce dummy "captured" images
class rcDummyCameraGrabber : public rcCameraGrabber {
public:
    // ctor
    rcDummyCameraGrabber( const int32& width,
                          const int32& height,
                          const rcPixel& depth,
                          const double& fps ) :
            mLastError( eFrameErrorOK ), mCtrlFlags(0), mId(0),
            mWidth( width ), mHeight( height ), mDepth( depth ), mLastFrameTime( -1.0 ),
            mFrameInterval( 1.0/fps ), mFrameCount( 0 )
        {
        };

    // virtual dtor
    virtual ~rcDummyCameraGrabber() { };

    // Get acquire state info
    virtual rcFrameGrabberStatus getAcqInfo(rcAcqInfo& ai, bool, bool)
        {
            ai.cameraInfo.name( "Simulated random pixel source" );
            ai.cameraInfo.uid( 1 );
            ai.cameraInfo.mid( 666 );
            if ( mDepth == rcPixel8 )
                ai.cameraType = rcCameraGreyScale;
            else
                ai.cameraType = rcCameraColor;
            ai.maxFramesPerSecond = 1.0/mFrameInterval;
            ai.maxFrameWidth = mWidth;
            ai.maxFrameHeight = mHeight;
            ai.cameraPixelDepth = mDepth;
            ai.curDecimationRate = 1;
            ai.acqState = rcAcqRunning;
            ai.missedFrames = 0;
            ai.acqFlags = rcACQFLAGS_FRAMEAVAIL;
            ai.ctrlFlags = mCtrlFlags;
            mCtrlFlags = 0;
            ai.acqCtrlUpdateID = mId;   // ID of last acq control request accepted

            return eFrameStatusOK;
        }

    // Set acquire control info
    virtual rcFrameGrabberStatus setAcqControl(rcAcqControl& ci, bool, bool)
        {
            mId = ci.acqCtrlUpdateID;
            mCtrlFlags = rcCTRLFLAGS_CTRL;
            return eFrameStatusOK;
        }

    // Set the number of frames to make available
    virtual void setFrameCount(int32) { return; }

    // Add to set of frame buffers to use during acquire
    virtual void addBuffer(rcSharedFrameBufPtr&) { return; }

    //
    // rcFrameGrabber API
    //

    // Returns instance validity
    virtual bool isValid() const { return true; };

    // Get last error
    virtual rcFrameGrabberError getLastError() const { return mLastError; };

    // Start grabbing
    virtual bool start() { return true; };

    // Stop grabbing
    virtual bool stop() { return true; };

    // Returns the number of frames available
    virtual int32 frameCount() { return -1; };

    // Get next frame, assign the frame to ptr
    virtual rcFrameGrabberStatus getNextFrame( rcSharedFrameBufPtr& ptr,
                                               bool isBlocking ) {
            rmUnused( isBlocking );
            ptr = 0;
            rcTimestamp now = rcTimestamp::now();

            if ( mLastFrameTime > 0.0 ) {
                double trueFrameInt = (now - mLastFrameTime).secs();
                double diff = mFrameInterval - trueFrameInt;
                //cerr << "exp " << mFrameInterval << " true " << trueFrameInt << " diff " << diff << endl;
                if ( diff > 0.0 ) {
                    // Simulate a frame rate
                    rcThread::sleep( (long)(diff * 1000) );
                }
            }

            try {
                rcSharedFrameBufPtr frameBufPtr= new rcFrame( mWidth, mHeight, mDepth );
                if ( frameBufPtr ) {
                    rcWindow dummyImage( frameBufPtr );

                    if ( mDepth == rcPixel8 ) {
                        // Synthetic gray sweep
                        frameBufPtr->initGrayColorMap();
                        frameBufPtr->setIsGray( true );

                        for (int32 j = 0; j < mHeight; j++) {
                            uint8 *oneRow = dummyImage.rowPointer (j);
                            memset( oneRow, mFrameCount+j, mWidth*mDepth );
                        }
                    } else if ( mDepth == rcPixel32 ) {
                        // Synthetic color sweep
                        for (int32 j = 0; j < mHeight; j++) {
                            const uint32 pix = mFrameCount*j;
                            uint32 *oneRow = (uint32*) dummyImage.rowPointer (j);

                            for (int32 i = 0; i < mWidth; ++i, ++oneRow)
                                *oneRow = pix;
                        }
                    }

                    ++mFrameCount;
                    frameBufPtr->setTimestamp( rcTimestamp::now() );
                    mLastFrameTime = now;
                    ptr = frameBufPtr;
                    return eFrameStatusOK;
                }
                return eFrameStatusEOF;
            }
            catch (...) {
                mLastError = eFrameErrorOutOfMemory;
            }
            return eFrameStatusError;
    }

    // Get name of input source, ie. file name, camera name etc.
    virtual const std::string getInputSourceName() { return "Simulated random pixel camera"; };

private:
    rcFrameGrabberError     mLastError;
    uint32                mCtrlFlags;
    uint32                mId;
    int32                 mWidth;
    int32                 mHeight;
    rcPixel            mDepth;
    rcTimestamp             mLastFrameTime;
    double                  mFrameInterval;  // Frame interval in seconds
    uint32                mFrameCount;     // Number of produced frames
};
#endif // rcSIMULATED_CAMERA

const uint32 rcEngineMovieCapture::_bufRingSz = 5; // Semi-randomly chosen value

rcEngineMovieCapture::rcEngineMovieCapture(rcEngineImpl* engine,
                                           rcVideoCaptureCtrl& captureCtrl,
                                           const rcMovieSaveCtrlReader& saveCtrl,
                                           rcEngineObserver* observer)
        : _frameBufPtrs(_bufRingSz), _saveRing(_bufRingSz), _buffersInTransmit(0),
          _saveStatus(eMovieFileErr_NoErr), _saveThread(0), _movieSaver(0),
          _engine(engine), _captureCtrl(captureCtrl), _saveCtrl(saveCtrl),
          _observer(observer)
{
    rmAssert(_engine);
    rmAssert(_observer);

    for (uint32 i = 0; i < _frameBufPtrs.size(); i++) {
        bool status = _saveRing.releaseResource(&_frameBufPtrs[i]);
        rmAssert(status == true);
    }

    // set default acq control values
    _acqControl.doAcquire = FALSE;
    _acqControl.resolution = rcAcqFullResXY;
    _acqControl.doShutdown = 0;
    _acqControl.gain = 0;
    _acqControl.shutter = 0;
    _acqControl.binning = 1;
    _acqControl.decimationRate = 1;

    // set default acq info values
    _acqInfo.acqState = rcAcqUnknown; // Ignore other fields until this is valid
}

uint32 rcEngineMovieCapture::delayLeft()
{
    rcLock lock(_saverMutex);

    if (_movieSaver == 0)
        return 0;

    return _movieSaver->saveStatus().getDelayLeft();
}

rcMovieFileState rcEngineMovieCapture::saveState()
{
    rcLock lock(_saverMutex);

    if (_movieSaver == 0)
        return eMovieFileState_Uninitialized;

    return _movieSaver->saveStatus().getState();
}

void rcEngineMovieCapture::saveReset()
{
    rcLock lock(_saverMutex);

    if (_movieSaver &&
        (_movieSaver->saveStatus().getState() == eMovieFileState_Done))
        _movieSaver->reset();
}

/* createGrabber - Create a live video grabber. Returns a pointer to
 * the grabber if a valid grabber is created, NULL otherwise.
 */
rcCameraGrabber* rcEngineMovieCapture::createGrabber()
{
    rcCameraGrabber* grabber = 0;

#ifdef rcSIMULATED_CAMERA
    // Hack to produce simulated dummy images
    // Create a grabber to produce random images

    // Gray
    grabber = new rcDummyCameraGrabber( 1024, 768, rcPixel8, 14.0 );

    // Color
    grabber = new rcDummyCameraGrabber( 320, 240, rcPixel32, 14.0 );
#endif

    /* Creates and initializes both shared memory and shared
     * semaphores, and starts up child process RCQCAM.
     */
    grabber = new rcQVideoGrabber("RCDCAM", 0, 0,
                                  sizeof(rcVideoSharedMemLayout));
#if 0
  /* Creates and initializes both shared memory and shared
     * semaphores, and starts up child process RCQCAM.
     */
    grabber = new rcQVideoGrabber("RCQTIME", 0, 0,
                                  sizeof(rcVideoSharedMemLayout));
#endif



    /* Check for grabber creation failure
     */
    if (!grabber) {
        strstream s;
        s << "Grabber creation error: Out of memory error\n" << ends;
        _observer->notifyStatus(s.str());
        cerr << s;
        s.freeze(false); // Without freeze(), strstream::str() leaks
        // the string it returns
        return 0;
    }
    else if (!grabber->isValid()) {
        strstream s;
        s << rcFrameGrabber::getErrorString( grabber->getLastError() )
          << " for " << grabber->getInputSourceName() << ends;
        _observer->notifyStatus(s.str());
        cerr << s;
        s.freeze(false); // Without freeze(), strstream::str() leaks
        // the string it returns
        delete grabber;

        _acqInfo.acqState = rcAcqDisabled;
        return 0;
    }

    return grabber;
}

/* initializeCamera - Initialize the camera settings for the given
 * grabber. Returns true if it is able to initialize the camera, false
 * otherwise.
 */
bool rcEngineMovieCapture::initializeCamera(rcCameraGrabber& grabber,
                                            char*& errorString,
                                            bool& allocError)
{
    errorString = 0;
    allocError = false;

    rcFrameGrabberStatus status;


    // Get the initial camera state
    //
    if ((status = grabber.getAcqInfo(_acqInfo, true, false)) !=
        eFrameStatusOK) { // Figure out error message
        if (status == eFrameStatusError)
            errorString = "Error accessing shared memory, error %s\n";
        else
            errorString = "Unknown error getting acq info, error %s\n";
    }
    else if (_acqInfo.acqState == rcAcqDisabled) {
        if ( _acqInfo.ctrlFlags & rcCTRLFLAGS_ERRLIC )
            errorString = "No DCAM license installed for this host\n";
        else
            errorString = "Couldn't initialize camera\n";
        status = eFrameStatusError;
    }
    else { // Set up initial camera settings and start acquiring
        // Only 8-bit gray is supported now
        if ( _acqInfo.cameraPixelDepth != rcPixel8 ) {
            errorString = "Warning: camera does not support 8-bit monochrome mode\n";
            fprintf(stderr, errorString );
            //return false;
        }
        /* Pre-allocate frame buffers to be used.
         */
        for (uint32 i = 0; i < _bufRingSz; i++) {
            rcSharedFrameBufPtr bufP = new rcFrame(_acqInfo.maxFrameWidth,
                                                    _acqInfo.maxFrameHeight,
                                                    _acqInfo.cameraPixelDepth);
            if (!bufP) {
                errorString = "Not enough memory available to allocate frame buffers\n";
                allocError = true;
                return false;
            }

            /* As an extra effort to make sure everything is set before
             * starting to accept frames, touch one pixel in each row
             * to make sure all the frames have actual been loaded into
             * memory.
            for (uint32 y = 0; y < _acqInfo.maxFrameHeight; y++)
                bufP->setPixel(0, y, 0);
             */

            if ( _acqInfo.cameraPixelDepth == rcPixel8) {
                bufP->initGrayColorMap();
            } else if ( _acqInfo.cameraPixelDepth == rcPixel16)
                rfFillColorMap(0, bufP);

            grabber.addBuffer(bufP);
        }

        _acqControl.gain = _acqInfo.curGain;
        _acqControl.shutter = _acqInfo.curShutter;
			_acqControl.binning = _acqInfo.curBinning;
        _captureCtrl.gain( _acqControl.gain );
        _captureCtrl.shutter( _acqControl.shutter );
			_captureCtrl.binning ( _acqControl.binning);


        _acqControl.doShutdown = 0;
        _acqControl.resolution = rcAcqFullResXY;
        _acqControl.decimationRate = _acqInfo.curDecimationRate;
        _acqControl.acqCtrlUpdateID = _acqInfo.acqCtrlUpdateID+1;
        _acqControl.doAcquire = true;

        if ((status = grabber.setAcqControl(_acqControl, false, true)) !=
            eFrameStatusOK) { // Figure out error messsage
            if (status == eFrameStatusError)
                errorString = "Error accessing shared memory, error %s\n";
            else
                errorString = "Unknown error setting acq control, error %s\n";
        }
    } // End of: if (status == eFrameStatusOK)

    if (eFrameStatusOK == status)
        return true;

    fprintf(stderr, errorString,
            &(rcFrameGrabber::getErrorString(grabber.getLastError()))[0]);
    return false;
}

/* getCameraState - Read the current camera state info. This fct will
 * block and leave this process in control of the shared buffer.
 * Returns true if it is able to read from shared memory, false
 * otherwise.
 */
bool rcEngineMovieCapture::getCameraState(rcCameraGrabber& grabber)
{
    rcFrameGrabberStatus status;

    if ((status = grabber.getAcqInfo(_acqInfo, true, false)) != eFrameStatusOK) {
        char* errorString;

        if (status == eFrameStatusError)
            errorString = "Error accessing shared memory, error %s\n";
        else
            errorString = "Unknown error getting acq info, error %s\n";

        fprintf(stderr, errorString,
                rcFrameGrabber::getErrorString(grabber.getLastError()).data());

        return false;
    }

    return true;
}

bool updateControl( const rcVideoCaptureCtrlReader& captureCtrl,
                    const rcAcqInfo& acqInfo,
                    rcAcqControl& acqControl )
{
    bool update = false;

    if ( acqInfo.acqCtrlUpdateID == acqControl.acqCtrlUpdateID ) {
        uint32 decimationRate = captureCtrl.getDecimationRate();
        if ( decimationRate != acqControl.decimationRate ) {
            acqControl.decimationRate = decimationRate;
            update = true;
        }
        int32 gain = captureCtrl.getGain();
        if ( gain != acqControl.gain ) {
            acqControl.gain = gain;
            update = true;
        }
        int32 shutter = captureCtrl.getShutter();
        if ( shutter != acqControl.shutter ) {
            acqControl.shutter = shutter;
            update = true;
        }
		  int32 binning = captureCtrl.getBinning();
        if ( binning != acqControl.binning ) {
            acqControl.binning = binning;
            update = true;
        }
    }

    if ( update )
        acqControl.acqCtrlUpdateID++;

    return update;
}

/* updateCameraSettings - Update the camera settings used by RCDCAM
 * process. This fct assumes the shared buffer is available and leaves
 * this process in control of the shared buffer.  Returns false if it
 * is unable to write to shared memory, true otherwise.
 */
bool rcEngineMovieCapture::updateCameraSettings(rcCameraGrabber& grabber)
{
    rcFrameGrabberStatus status;

    if (_acqInfo.ctrlFlags & rcCTRLFLAGS_ERRCTRL) {
        fprintf(stderr, "Failed acq ctrl: status 0x%X\n", _acqInfo.ctrlFlags);
#ifdef IMPL_DEBUG_LOG
        cerr << "CurCtrl: " << _acqControl;
        cerr << "SavedCtrl: " << _acqSavedControl;
#endif

        // Control error, restore old values
        if ( _acqInfo.ctrlFlags & rcCTRLFLAGS_ERRSHUTTER ) {
            _captureCtrl.shutter( _acqSavedControl.shutter );
            _acqControl.shutter = _acqSavedControl.shutter;
            _observer->notifyExperimentChange();
        }
        if ( _acqInfo.ctrlFlags & rcCTRLFLAGS_ERRGAIN ) {
            _captureCtrl.gain( _acqSavedControl.gain );
            _acqControl.gain = _acqSavedControl.gain;
            _observer->notifyExperimentChange();
        }
		if ( _acqInfo.ctrlFlags & rcCTRLFLAGS_ERRBINNING ) {
            _captureCtrl.binning( _acqSavedControl.binning);
            _acqControl.binning = _acqSavedControl.binning;
            _observer->notifyExperimentChange();
        }

    }

    // If the user wants to change something and the last update
    // has been handled, send out a new update.
    //

    // Save old values
    _acqSavedControl = _acqControl;

    if ( updateControl( _captureCtrl, _acqInfo, _acqControl ) ) {

        if ((status = grabber.setAcqControl(_acqControl, false, false))
            != eFrameStatusOK) {
            char* errorString;

            if (status == eFrameStatusError)
                errorString = "Error accessing shared memory, error %s\n";
            else
                errorString = "Unknown error setting acq control, error %s\n";

            fprintf(stderr, errorString,
                    rcFrameGrabber::getErrorString(grabber.getLastError()).c_str());

            return false;
        }
    }

    return true;
}

/* sendShutdown - Capture thread is shutting down. Tell peer class to
 * shut itself down.
 */
void rcEngineMovieCapture::sendShutdown(rcCameraGrabber& grabber)
{
    _acqControl.doShutdown = 1;
    _acqControl.acqCtrlUpdateID++;

    rcFrameGrabberStatus status;

    if ((status = grabber.setAcqControl(_acqControl, true, true))
        != eFrameStatusOK) {
        char* errorString;

        if (status == eFrameStatusError)
            errorString = "Error accessing shared memory, error %s\n";
        else
            errorString = "Unknown error setting acq control, error %s\n";

        fprintf(stderr, errorString,
                rcFrameGrabber::getErrorString(grabber.getLastError()).c_str());
    }
}

/* startSaveThread - Capture thread has been able to start up and
 * connect to a usable camera. Start up movie saver thread. This
 * thread will wait until the user provides an output file. This will
 * act as the key to put the system in the "running" state and begin
 * the movie save process.
 */
void rcEngineMovieCapture::startSaveThread()
{
    /* Validate initial state.
     */
    rmAssert(!_saveThread);
    rmAssert(!_movieSaver);
    rmAssert(_saveRing.availReleasedResources() == _frameBufPtrs.size());
    rmAssert(_saveRing.availNewSlots() == _frameBufPtrs.size());
    rmAssert(_saveRing.availReleasedSlots() == 0);
    rmAssert(_saveRing.availNewResources() == 0);
    rmAssert(_buffersInTransmit.decrementVariable(0, -1) == 0);
    for (uint32 i = 0; i < _frameBufPtrs.size(); i++)
        rmAssert(!_frameBufPtrs[i]); // Make sure its currently NULL

    /* Set up and start save thread.
     */
    {
        rcLock lock(_saverMutex);
        _movieSaver = new rcEngineMovieSaver(_engine, _saveCtrl, _acqInfo, _observer,
                                             _saveRing, _buffersInTransmit);
    }

    rmAssert(_movieSaver);
    _saveThread = new rcThread(_movieSaver);
    rmAssert(_saveThread);
    _saveThread->setPriority(eHighPriority);
    _saveThread->start();
}

/* - closeSaveThread - Signal movie save thread to finish writing
 * frames and write out a valid movie header.
 */
void rcEngineMovieCapture::closeSaveFile()
{
    rmAssert(_saveThread);
    rmAssert(_movieSaver);

    /* If movie save is still proceeding without error, send out NULL
     * frame to inform save thread that the movie is complete.
     */
    if ((_movieSaver->saveStatus().getStatus() == eMovieFileErr_NoErr) &&
        (_movieSaver->saveStatus().getState() == eMovieFileState_Save)) {
        if (_saveRing.availReleasedResources() == 0) {
            _buffersInTransmit.waitUntilLessThan(_bufRingSz);
            rmAssert(_saveRing.availReleasedResources());
        }
        // Update acquisition info so cam header will use up-to-date
        // camera settings
        _movieSaver->updateAcqInfo( _acqInfo );
        rcSharedFrameBufPtr* sbp = _saveRing.recoverResource();
        rmAssert(sbp);
        *sbp = 0; // Send NULL frame as signal that movie is done

        bool retval = _saveRing.giveResource(sbp);
        rmAssert(retval == true);

        _buffersInTransmit.incrementVariable(1, 0);
    }
}

void rcEngineMovieCapture::saveComplete()
{
    rmAssert(_movieSaver->saveStatus().getState() == eMovieFileState_Done);

    /* If an error happened during save process, some frames may
     * still be in transit. Clear these out.
     */
    if (_movieSaver->saveStatus().getStatus() != eMovieFileErr_NoErr) {
        rcSharedFrameBufPtr* sbp;

        while (_saveRing.availNewResources()) {
            sbp = _saveRing.takeResource();
            rmAssert(sbp);
            *sbp = 0; // Free resource

            bool retval = _saveRing.releaseResource(sbp);
            rmAssert(retval);

            _buffersInTransmit.decrementVariable(1, -1);
        }
    }
}

void rcEngineMovieCapture::killSaveThread()
{
    rmAssert(_saveThread);
    rmAssert(_movieSaver);

    rcMovieFileState state = _movieSaver->saveStatus().getState();
    while (state == eMovieFileState_Uninitialized) {
        usleep(1);
        state = _movieSaver->saveStatus().getState();
    }

    rmAssert((state == eMovieFileState_Done) || (state == eMovieFileState_Wait) ||
             (state == eMovieFileState_Delay));

    /* Now wait for save thread to finish writing movie file
     * and return.
     */
    int status = _saveThread->join();
    if (status)
        cout << "save thread join error " << status << endl;
    else
        cout << "save thread join OK" << endl;

    delete _saveThread;
    _saveThread = 0;

    {
        rcLock lock(_saverMutex);
        delete _movieSaver;
        _movieSaver = 0;
    }
}

// Acquire live video from camera
void rcEngineMovieCapture::run()
{
    char* errorString = 0;
    rcCameraGrabber* grabber = createGrabber();

    /* Init counters to track how many frames were lost because the
     * capture thread couldn't process them fast enough vs how many
     * were lost because the movie save thread couldn't save them
     * fast enough.
     */
    uint32 missedFramesCurValue = 0;
    uint32 missedFramesOnCapture = 0;
    uint32 missedFramesOnSave = 0;

    if (grabber) {
        _observer->notifyStatus( "Initializing camera..." );

        bool allocError;

        if (initializeCamera(*grabber, errorString, allocError)) {
            rcVideoCaptureState captureState = eVideoCaptureState_Uninitialized;
            startSaveThread();

            _observer->notifyStatus( "Initializing camera...OK" );
            rsCorrParams corrParams;
            // Analyzer options
            rcAnalyzerOptions startOpt(corrParams,
                                       eAnalyzerFullCorrelation,
                                       _captureCtrl.getSlidingWindowOrigin(),
                                       _captureStatus.getCaptureRect(),
                                       _captureCtrl.getSlidingWindowSize(),
                                       _captureCtrl.getEntropyDefinition(),
                                       _acqInfo.cameraPixelDepth);
            // Create an analyzer
            rcAnalyzer analyzer(startOpt, *grabber);
            rcFrameGrabberStatus status = eFrameStatusOK;
            std::string cameraStatus;

            // Loop until grabber is exhausted or engine is shut down
            while (!seppukuRequested() && (_acqInfo.acqState != rcAcqDisabled)) {
                /* Read the current camera state info into _acqInfo.
                 */
                if (!getCameraState(*grabber)) {
                    _acqInfo.acqState = rcAcqDisabled;
                    break;
                }

                if ((captureState == eVideoCaptureState_MovieSave) &&
                    (missedFramesCurValue != _acqInfo.missedFrames)) {
                    rmAssert(_acqInfo.missedFrames > missedFramesCurValue);
                    missedFramesOnCapture += _acqInfo.missedFrames - missedFramesCurValue;
                    missedFramesCurValue = _acqInfo.missedFrames;
                }

                if (captureState == eVideoCaptureState_Uninitialized) {
                    captureState = eVideoCaptureState_MovieWait;
                    rcRect videoRect(0, 0, _acqInfo.maxFrameWidth,
                                     _acqInfo.maxFrameHeight);

                    if (_acqInfo.curResolution == rcAcqHalfResXY)
                        videoRect = rcRect(0, 0, _acqInfo.maxFrameWidth/2,
                                           _acqInfo.maxFrameHeight/2);

                    _captureStatus.captureRect(videoRect);
                    _captureStatus.captureDepth(_acqInfo.cameraPixelDepth);
                    _captureStatus.maxFramesPerSecond(_acqInfo.maxFramesPerSecond);

                    _observer->notifyVideoRect(videoRect);
                    _observer->notifyAnalysisRect(videoRect);
                    _engine->setFrameSize(videoRect.width(), videoRect.height(), _acqInfo.cameraPixelDepth);
                    _observer->notifyMultiplier(1000/_acqInfo.maxFramesPerSecond);

                    rcAnalyzerOptions newOpt(corrParams,
                                             eAnalyzerFullCorrelation,
                                             _captureCtrl.getSlidingWindowOrigin(),
                                             _captureStatus.getCaptureRect(),
                                             _captureCtrl.getSlidingWindowSize(),
                                             _captureCtrl.getEntropyDefinition(),
                                             _acqInfo.cameraPixelDepth);
                    if (newOpt != startOpt) {
                        analyzer.setOptions(newOpt);
                    }

                    /* Format string to show on status line. First, make sure string
                     * is null terminated.
                     */

                    std::string cameraName = std::string(_acqInfo.cameraInfo.name());

                    cameraStatus = std::string("Camera: ") + cameraName +
                        std::string("    Action: ");
                    // Write camera info header
                    _engine->setCameraInfo( _acqInfo.cameraInfo );
                    // Start preview track
                    _engine->startPreview();
                }

                /* When we first detect that the movie saver thread has
                 * entered the "save" state we want to start sending frames
                 * to the movie saver. Initialize the grabber's frame
                 * capture count and set the "saving movie" state variable
                 * to true.
                 */
                if ((captureState == eVideoCaptureState_MovieWait) &&
                    (_movieSaver->saveStatus().getState() == eMovieFileState_Save)) {
                    uint32 captureTime = _movieSaver->saveStatus().getCaptureTime();

                    // cMaxSaveFrameCount sets the ultimate upper limit for
                    // number of captured frames
                    if (captureTime) {
                        uint32 frameCaptureCount =
                            (uint32)(((captureTime*_acqInfo.maxFramesPerSecond)/
                                        _acqInfo.curDecimationRate) + .4999);
                        if ( frameCaptureCount > cMaxSaveFrameCount-1 )
                            frameCaptureCount = cMaxSaveFrameCount-1;
                        grabber->setFrameCount(frameCaptureCount);
                    } else
                        grabber->setFrameCount(cMaxSaveFrameCount-1);

                    missedFramesCurValue = _acqInfo.missedFrames;
                    missedFramesOnCapture = missedFramesOnSave = 0;
                    captureState = eVideoCaptureState_MovieSave;
                } // End of: if ((captureState == eVideoCaptureState_MovieWait) && ...)

                /* See if frame grabber settings need to be changed, but only
                 * if we aren't currently saving a movie.
                 */
                if ((captureState == eVideoCaptureState_MovieWait) &&
                    !updateCameraSettings(*grabber)) {
                    _acqInfo.acqState = rcAcqDisabled;
                    break;
                }

                if ((captureState == eVideoCaptureState_MovieSave) &&
                    (_engine->getState() == eEngineStopped)) {
                    rcTimestamp now = rcTimestamp::now();
                    fprintf(stderr, "Capture stopped at %s\n",
                            now.localtime().c_str());
                    grabber->setFrameCount(-1);
                    closeSaveFile();
                    captureState = eVideoCaptureState_MovieClose;
                }

                rcAnalyzerResult result;
                status = analyzer.getNextResult(result, true);
                if (status == eFrameStatusOK) {
                    /* If movie save is active handle this activity first
                     */
                    bool frameSaved = false; // Has this frame been saved to disk

                    if ((captureState == eVideoCaptureState_MovieSave) &&
                        result.frameBuf()) {
                        if (_movieSaver->saveStatus().getState() != eMovieFileState_Save) {
                            rcTimestamp now = rcTimestamp::now();
                            fprintf(stderr, "Movie saver aborted at %s\n",
                                    now.localtime().c_str());
                            grabber->setFrameCount(-1);
                            closeSaveFile();
                            captureState = eVideoCaptureState_MovieClose;
                        }
                        else if (_saveRing.availReleasedResources() != 0) {
                            rcSharedFrameBufPtr* sbp = _saveRing.recoverResource();
                            rmAssert(sbp);
                            *sbp = result.frameBuf();

                            bool retval = _saveRing.giveResource(sbp);
                            rmAssert(retval == true);

                            _buffersInTransmit.incrementVariable(1, 0);
                            frameSaved = true;
                        }
                        else
                            missedFramesOnSave++;
                    } // End of: if ((captureState==eVideoCaptureState_MovieSave) && ...

                    /* Now give analyzer and GUI a chance to process result
                     */
                    uint32 missedFrames = _acqInfo.missedFrames;
                    if (captureState == eVideoCaptureState_MovieSave)
                        missedFrames = missedFramesOnCapture;

                    _engine->frameInterrupt(result, cameraStatus, missedFrames,
                                            missedFramesOnSave, frameSaved);
                    // Set new options if applicable
                    rcAnalyzerOptions newOpt(corrParams,
                                             eAnalyzerFullCorrelation,
                                             _captureCtrl.getSlidingWindowOrigin(),
                                             _captureStatus.getCaptureRect(),
                                             _captureCtrl.getSlidingWindowSize(),
                                             _captureCtrl.getEntropyDefinition(),
                                             _acqInfo.cameraPixelDepth );

                    if (newOpt != analyzer.getOptions() ) {
                        analyzer.setOptions(newOpt);
                    }

                }
                else if (status == eFrameStatusEOF) {
                    rcTimestamp now = rcTimestamp::now();
                    fprintf(stderr, "Grabber reached EOF at %s\n",
                            now.localtime().c_str());
                    grabber->setFrameCount(-1);
                    analyzer.reset();
                    closeSaveFile();
                    captureState = eVideoCaptureState_MovieClose;
                }
                else {
                    rcTimestamp now = rcTimestamp::now();
                    fprintf(stderr, "Got some other error at %s\n",
                            now.localtime().c_str());
                    grabber->setFrameCount(-1);
                    closeSaveFile();
                    captureState = eVideoCaptureState_MovieClose;
                    rcFrameGrabberError error = analyzer.getLastError();
                    _observer->notifyError(rcFrameGrabber::getErrorString(error).c_str());
                    break; // Bail out of while loop
                } // End of: if (status == eFrameStatusOK) ...
                //         else if (status == eFrameStatusEOF) ... else ...

                /* Wait until movie saver says it is done before cleaning up shared
                 * ring.
                 */
                if ((captureState == eVideoCaptureState_MovieClose) &&
                    (_movieSaver->saveStatus().getState() == eMovieFileState_Done)) {
                    saveComplete();
                    captureState = eVideoCaptureState_MovieWait;
                }
            } //End of: while (!seppukuRequested()&&(_acqInfo.acqState!=rcAcqDisabled))

            /* Before returning make sure that any pending movie save has a
             * chance to complete.
             */
            if ((captureState == eVideoCaptureState_MovieSave)) {
                closeSaveFile();
                captureState = eVideoCaptureState_MovieClose;
            }

            if (captureState == eVideoCaptureState_MovieClose) {
                while (_movieSaver->saveStatus().getState() != eMovieFileState_Done)
                    usleep(10000); // sleep 10ms
                saveComplete();
                captureState = eVideoCaptureState_MovieWait;
            }

            killSaveThread();
        } else { // Error initing camera
            rmAssert(errorString);

            std::string eMsg = "";

            if (!allocError)
                std::string eMsg = rcFrameGrabber::getErrorString(grabber->getLastError());

            const size_t blen = 256;
            char buf[blen];
            snprintf(buf, blen, errorString, eMsg.c_str());
            eMsg = std::string(buf);

            fprintf(stderr, "%s", eMsg.c_str());
            _engine->start(); _engine->stop();
            if (allocError)
                _observer->notifyStatus("Cannot Initialize Camera");
            else
                _observer->notifyStatus("No Camera");
            _observer->notifyError(eMsg.c_str());
            _acqInfo.acqState = rcAcqDisabled;
        } // End of: if (initializeCamera(*grabber)) ... else // Error initing camera
    } else { // Grabber creation failure
        _acqInfo.acqState = rcAcqDisabled;
    }

    rmAssert(!_saveThread);
    rmAssert(!_movieSaver);

    if (_acqInfo.acqState != rcAcqDisabled)
        sendShutdown(*grabber);

    delete grabber;

    _acqInfo.acqState = rcAcqUnknown;
}
