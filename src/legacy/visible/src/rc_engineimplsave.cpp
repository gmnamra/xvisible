/******************************************************************************
 *   Copyright (c) 2002 Reify Corp. All Rights reserved.
 *
 *	rc_engineimplsave.cpp
 *
 *	This file contains the engine video save support.
 *
 ******************************************************************************/

#include "rc_engineimpl.h"
#include "rc_engineimplsave.h"
#include <rc_sparsehist.h>
#include <rc_timinginfo.h>

static const uint32 delayTimeToSeconds = 60;

/* Function for getting the error string associated with a particular
 * movie file save error.
 */
static std::string getMovieErrorString(rcMovieFileErr error)
{
  switch (error) {
    case eMovieFileErr_NoErr:
      return std::string("Movie File Save: OK");
    case eMovieFileErr_Aborted:
      return std::string("Movie File Save: Movie Save Aborted");
    case eMovieFileErr_OpenFailed:
      return std::string("Movie File Save: File Open Failed");
    case eMovieFileErr_WriteFailed:
      return std::string("Movie File Save: File Write Failed");
    case eMovieFileErr_NullInput:
      return std::string("Movie File Save: Empty Movie");
    case eMovieFileErr_UnsupportedType:
      return std::string("Movie File Save: Unsupported Video Format");
    case eMovieFileErr_NotHomogenous:
      return std::string("Movie File Save: Video Format Not Homogenous");
    case eMovieFileErr_SeekFailed:
      return std::string("Movie File Save: File Seek Failed");
    case eMovieFileErr_CloseFailed:
      return std::string("Movie File Save: File Close Failed");
    case eMovieFileErr_VariableFrameRate:
      return std::string("Movie File Save: Variable Frame Rate");
    case eMovieFileErr_MaxFramesExceeded:
      return std::string("Movie File Save: Maximum allowed frame count exceeded");
    /* Note: no default case to force a compiler warning if a new enum
     * value is defined without adding a corresponding string here.
     */
  };

  return std::string("Movie File Save: undefined error");
}

/******************************************************************************
 *	Movie Saver instantiation
 ******************************************************************************/

rcEngineMovieSaver::rcEngineMovieSaver(rcEngineImpl* engine,
                                       const rcMovieSaveCtrlReader& saveCtrl,
                                       const rcAcqInfo& acqInfo,
                                       rcEngineObserver* observer,
                                       rcBidirectionalRing<rcSharedFrameBufPtr>& saveRing,
                                       rcConditionVariable& buffersInTransmit)
        : _saveStream(NULL), _hist(6), _histOverflow(0), _engine(engine),
          _saveCtrl(saveCtrl), _acqInfo(acqInfo), _observer(observer), _saveRing(saveRing),
          _buffersInTransmit(buffersInTransmit), _curResetValue(0), _prevResetValue(0)
{
  rmAssert(_engine);
  rmAssert(_observer);
}

void rcEngineMovieSaver::reset()
{
  rmAssert(eMovieFileState_Done == _saveStatus.getState());

  uint32 curValue;
  _curResetValue.getValue(curValue);
  _curResetValue.setValue(curValue+1);
}

void rcEngineMovieSaver::error_cleanup(rcSharedFrameBufPtr* sbp)
{
  rmAssert(sbp);
  *sbp = 0; // Free resource
  bool retval = _saveRing.releaseResource(sbp);
  rmAssert(retval);
  _buffersInTransmit.decrementVariable(1, rcEngineMovieCapture::_bufRingSz);
}

/********************************************************************/
/* uninitializedHdlr - Called at initialization time. Its one       */
/* function is to make sure all time critical handlers have been    */
/* called in an effort to make sure their code has all been swapped */
/* in.                                                              */
/********************************************************************/

rcMovieFileState
rcEngineMovieSaver::uninitializedHdlr(rcMovieFileState prevState)
{
  rmAssert(prevState == eMovieFileState_Uninitialized);

  saveHdlr(eMovieFileState_Uninitialized);

  return eMovieFileState_Wait;
}

/********************************************************************/
/* waitHdlr - Spin loops on usleep until user starts movie save.    */
/********************************************************************/

rcMovieFileState rcEngineMovieSaver::waitHdlr(rcMovieFileState prevState)
{
  rmAssert((prevState == eMovieFileState_Uninitialized) ||
	   (prevState == eMovieFileState_Done));

  _saveFileName = _saveCtrl.getOutputFileName();

  /* Note: This would best be done with a condition variable instead
   * of a polling loop.
   */
  while (_saveFileName.empty()) {
    usleep(100000); // Sleep for 100ms

    if (seppukuRequested()) {
      _saveStatus.status(eMovieFileErr_Aborted);
      return eMovieFileState_Done;
    }

    _saveFileName = _saveCtrl.getOutputFileName();
  }

  if (_engine->getState() == eEngineStopped)
    _engine->start();

  rmAssert(_engine->getState() == eEngineRunning);

  uint32 captureTime = _saveCtrl.getFrameCaptureTime();

  _saveStatus.captureTime(captureTime * 60);

  uint32 delayTime = _saveCtrl.getFrameCaptureDelayTime();

  if (delayTime) {
    _saveStatus.delayLeft(delayTime * delayTimeToSeconds * 10);
    return eMovieFileState_Delay;
  }

  return eMovieFileState_Open;
}

/********************************************************************/
/* delayHdlr - Called if user specified delay before save. Spin     */
/* loops on usleep until either delay period expires or the user    */
/* cancels the movie save.                                          */
/********************************************************************/

rcMovieFileState rcEngineMovieSaver::delayHdlr(rcMovieFileState prevState)
{
  rmAssert(prevState == eMovieFileState_Wait);

  uint32 delayTime = _saveCtrl.getFrameCaptureDelayTime();

  if (delayTime) {
    uint32 delayTenthsSeconds = _saveStatus.getDelayLeft();
    rmAssert((delayTenthsSeconds & 1) == 0);

    do {
      usleep(200000); // sleep 200 ms

      if (seppukuRequested()) {
	_saveStatus.status(eMovieFileErr_Aborted);
	return eMovieFileState_Done;
      }

      if (_engine->getState() == eEngineStopped) {
	_observer->notifyWarning("Movie save aborted during delay "
				"period.\nNo movie saved.");
	_saveStatus.status(eMovieFileErr_Aborted);
	return eMovieFileState_Done;
      }

      delayTenthsSeconds -= 2;
      _saveStatus.delayLeft(delayTenthsSeconds);
    } while (delayTenthsSeconds);
  }

  return eMovieFileState_Open;
}

/********************************************************************/
/* openHdlr - Opens/Creates the movie file and writes a dummy       */
/* header.                                                          */
/********************************************************************/

rcMovieFileState rcEngineMovieSaver::openHdlr(rcMovieFileState prevState)
{
  rmAssert((prevState == eMovieFileState_Wait) ||
	   (prevState == eMovieFileState_Delay));

  fprintf(stderr, "MS: output file name %s\n", _saveFileName.data());
  rmAssert(_saveFileName.size());

  _saveStream = fopen(_saveFileName.data(), "w+");

  if (_saveStream == NULL) {
    perror("fopen failed");
    _saveStatus.status(eMovieFileErr_OpenFailed);
    return eMovieFileState_Error;
  }

  /* Write out a dummy header indicating the movie is invalid.  Will
   * only be changed to a valid header at the very end when we know
   * the movie is good.
   */
  rcMovieFileFormat2 movieHdr(movieFormatInvalid);
  // Write basic header
  if ( movieHdr.write( _saveStream ) ) {
      _saveStatus.status(eMovieFileErr_WriteFailed);
      return eMovieFileState_Error;
  }

#ifdef rcWRITE_TOC
  // Reserve plenty of space for TOC so it's fast to assign values
  // TODO: estimate number of frames to be captured
  _toc = vector<int64>(cMaxSaveFrameCount,0);
#endif

  /* Should be good to go now, but there are problems with frames
   * getting lost at the beginning of a movie save. Investigation
   * revealed that the action of starting to save large amounts of
   * data was a cause of these symptoms. Why this happens is unknown
   * and should be investigated further. To work around this, simulate
   * starting a movie save by writing a bunch of random data to disk,
   * seeking back to the end of the movie header and then waiting a
   * short time to let any missed frames to to work there way out of
   * the video stream.
   */
#if 0
  /*
   * Commented this out to prevent inexplicable disk write out freeze at start
   * of capture. Don't know why this works, but we'll try it out.
   *
   * peter
   *
   * 6/21/05
   */
  char* data = (char*)malloc(1000000);
  rmAssert(data);
  for (int i = 0; i < 10; i++) {
    if (fwrite(data, 1000000, 1, _saveStream) != 1) {
      perror("1st fwrite failed");
      if (fclose(_saveStream))
	perror("fclose failed");
      _saveStatus.status(eMovieFileErr_WriteFailed);
      free(data);
      return eMovieFileState_Error;
    }
    // fflush(_saveStream);
  }

  free(data);
#endif

  if (fseek(_saveStream, sizeof(_movieHdrRev2), SEEK_SET)) {
    perror("fseek failed");
    if (fclose(_saveStream))
      perror("fclose failed");
    _saveStatus.status(eMovieFileErr_SeekFailed);
    return eMovieFileState_Error;
  }

#ifdef rcWRITE_TOC
  // Touch first value again
  _toc[0] = 0;
#endif

  for (int i = 0; i < 10; i++)
    usleep(200000);

  return eMovieFileState_Save;
}

/*********************************************************************/
/* saveHdlr - Saves frames to the movie file and generates most      */
/* values for the final movie header.                                */
/*                                                                   */
/* Note: This is the one function where realtime constraints must be */
/* stringently enforced. No slow or unpredictable processing should  */
/* be performed here that doesn't directly relate to the capture and */
/* storage of a movie.                                               */
/*********************************************************************/

rcMovieFileState rcEngineMovieSaver::saveHdlr(rcMovieFileState prevState)
{
  if (prevState == eMovieFileState_Uninitialized) {
    /* If any functions different than those used by openHdlr are
     * required, try to make nop use of them here before returning.
     */
    return eMovieFileState_Uninitialized;
  }

  rmAssert(prevState == eMovieFileState_Open);

  /* Note: Something should always get sent - even if its just a NULL
   * buffer ptr.
   */
  if (_saveRing.availNewResources() == 0) {
    _buffersInTransmit.waitUntilGreaterThan(0);
    if (!_saveRing.availNewResources()) {
      if (fclose(_saveStream))
	perror("fclose failed");
      rmAssert(0);
    }
  }

  rcSharedFrameBufPtr* sbp = _saveRing.takeResource();
  if (!sbp) {
    if (fclose(_saveStream))
      perror("fclose failed");
    rmAssert(0);
  }

  if (!*sbp) {
    if (fclose(_saveStream))
      perror("fclose failed");
    _saveStatus.status(eMovieFileErr_NullInput);
    error_cleanup(sbp);
    return eMovieFileState_Error;
  }

  if ((*sbp)->rowPad()) {
    if (fclose(_saveStream))
      perror("fclose failed");
    _saveStatus.status(eMovieFileErr_UnsupportedType);
    error_cleanup(sbp);
    return eMovieFileState_Error;
  }

  /* Initialize the movie header with the info from the
   * first frame.
   */
  _movieHdrRev2 = rcMovieFileFormat2( *sbp, movieFormatRevLatest );

  size_t bytesInFrame = _movieHdrRev2.bytesInFrame();
  bool firstLoop = true;

  while (*sbp) {
      if ((static_cast<int32>(_movieHdrRev2.width()) != (*sbp)->width()) ||
          (static_cast<int32>(_movieHdrRev2.height()) != (*sbp)->height()) || (*sbp)->rowPad()) {
      if (fclose(_saveStream))
	perror("fclose failed");
      _saveStatus.status(eMovieFileErr_NotHomogenous);
      error_cleanup(sbp);
      return eMovieFileState_Error;
    }

    if (firstLoop)
      firstLoop = false;
    else {
      rcTimestamp diff = (*sbp)->timestamp() - _prevTime;
      if (!_hist.add(uint32(diff.secs() * 1000)))
	_histOverflow++;
    }

    int64 timestamp = (*sbp)->timestamp()._timestamp;
    _prevTime = (*sbp)->timestamp();

    if (fwrite(&timestamp, sizeof(int64), 1, _saveStream) != 1) {
      perror("fwrite of timestamp failed");
      if (fclose(_saveStream))
	perror("fclose failed");
      _saveStatus.status(eMovieFileErr_WriteFailed);
      error_cleanup(sbp);
      return eMovieFileState_Error;
    }

    if (fwrite((*sbp)->alignedRawData(), bytesInFrame, 1, _saveStream) != 1) {
      perror("fwrite of frame data failed");
      if (fclose(_saveStream))
	perror("fclose failed");
      _saveStatus.status(eMovieFileErr_WriteFailed);
      error_cleanup(sbp);
      return eMovieFileState_Error;
    }
    //   fflush(_saveStream);
    *sbp = 0; // Free frame buffer

    if (!_saveRing.releaseResource(sbp)) {
      if (fclose(_saveStream))
	perror("fclose failed");
      rmAssert(0);
    }

    _buffersInTransmit.decrementVariable(1, rcEngineMovieCapture::_bufRingSz);

    if (_saveRing.availNewResources() == 0) {
      _buffersInTransmit.waitUntilGreaterThan(0);
      if (!_saveRing.availNewResources()) {
	if (fclose(_saveStream))
	  perror("fclose failed");
	rmAssert(0);
      }
    }

    sbp = _saveRing.takeResource();
    if (!sbp) {
      if (fclose(_saveStream))
	perror("fclose failed");
      rmAssert(0);
    }

    const uint32 frameCount = _movieHdrRev2.frameCount();
#ifdef rcWRITE_TOC
    if ( frameCount < cMaxSaveFrameCount )
        _toc[frameCount] = timestamp;
    else {
        _saveStatus.status(eMovieFileErr_MaxFramesExceeded);
        perror("Maximum allowed frame count exceeded");
        return eMovieFileState_Error;
    }
#endif
    _movieHdrRev2.frameCount(frameCount+1);

  } // End of: while (*sbp)

  if (!_saveRing.releaseResource(sbp)) {
    if (fclose(_saveStream))
      perror("fclose failed");
    rmAssert(0);
  }

  _buffersInTransmit.decrementVariable(1, rcEngineMovieCapture::_bufRingSz);

  return eMovieFileState_Close;
}

/********************************************************************/
/* errorHdlr - Does some randmow cleanup and sets state to done.    */
/********************************************************************/
rcMovieFileState rcEngineMovieSaver::errorHdlr(rcMovieFileState)
{
  std::string eMsg = "Error!: " + getMovieErrorString(_saveStatus.getStatus());

  fprintf(stderr, "Got warning: %s\n", eMsg.c_str());
  _observer->notifyError(eMsg.c_str());

  return eMovieFileState_Done;
}

/********************************************************************/
/* closeHdlr - Completes setting up the movie header and stores     */
/* it to disk. Closes the movie and generates any necessary         */
/* warning info. Also entry point for the save of any XML data      */
/* associated with this movie save.                                 */
/********************************************************************/

rcMovieFileState rcEngineMovieSaver::closeHdlr(rcMovieFileState prevState)
{
  rmAssert(prevState == eMovieFileState_Save);

  rcMovieFileErr finalStatus = eMovieFileErr_NoErr;
  uint32 id = 1;

  uint32 min, max;
  _hist.range(min, max);

  if (_movieHdrRev2.frameCount() == 1)
      _movieHdrRev2.averageFrameRate(1); // Undefined, just put a number in here
  else if ((_hist.binsUsed() <= 2) && ((max - min) < 2))
      _movieHdrRev2.averageFrameRate(1000./_hist.average());
  else {
      rcTimestamp diff = _prevTime - rcTimestamp(_movieHdrRev2.baseTime());
      _movieHdrRev2.averageFrameRate((_movieHdrRev2.frameCount()-1)/diff.secs());
    finalStatus = eMovieFileErr_VariableFrameRate;
  }

#ifdef rcWRITE_TOC
  // Resize to actual size
  _toc.resize(_movieHdrRev2.frameCount());
  // Write TOC
  rcMovieFileTocExt tocHdr( _toc.size() );
  if ( tocHdr.write( _saveStream, _toc ) ) {
      _saveStatus.status(eMovieFileErr_WriteFailed);
      return eMovieFileState_Error;
  }
#endif

  // Get generator name
  rcPersistenceManager* pm = rcPersistenceManagerFactory::getPersistenceManager();
  std::string generator = pm->generatorComment();
  // Write origin header
  rcMovieFileOrgExt orgHdr( movieOriginCaptureCertified, _movieHdrRev2.baseTime(), _movieHdrRev2.frameCount(),
                            _movieHdrRev2.width(), _movieHdrRev2.height(), _movieHdrRev2.depth(),
                            movieFormatRev(_movieHdrRev2.rev()), generator.c_str() );
  orgHdr.id( id++ );
  if ( orgHdr.write( _saveStream ) ) {
      _saveStatus.status(eMovieFileErr_WriteFailed);
      return eMovieFileState_Error;
  }

#ifdef rcWRITE_EXP
  const rcExperimentAttributes expAttr = _observer->getExperimentAttributes();
  // Write experiment info header
  rcMovieFileExpExt expHdr;

  expHdr.title( expAttr.title.c_str() );
  expHdr.userName( expAttr.userName.c_str() );
  expHdr.treatment1(expAttr.treatment1.c_str());
  expHdr.treatment2(expAttr.treatment2.c_str());
  expHdr.cellType(expAttr.cellType.c_str());
  expHdr.imagingMode(expAttr.imagingMode.c_str());
  expHdr.comment(expAttr.comments.c_str());

  try {
      // rcValue type conversion may throw en exception
      // catch it here so that movie save will not be compromised
      expHdr.lensMag(rcValue(expAttr.lensMag));
      expHdr.otherMag(rcValue(expAttr.otherMag));
      expHdr.temperature(rcValue(expAttr.temperature));
      expHdr.CO2(rcValue(expAttr.CO2));
      expHdr.O2(rcValue(expAttr.O2));
  } catch (general_exception& x) {
      cerr << "caught general_exception: " << x.what() << endl;
  }
  catch (...) {
      cerr << "caught unknown exception" << endl;
  }

  expHdr.id( id++ );

  if ( expHdr.write( _saveStream ) ) {
      _saveStatus.status(eMovieFileErr_WriteFailed);
      return eMovieFileState_Error;
  }
  // Update GUI
  _engine->setCaptureInfo( orgHdr, expHdr );
#else
  _engine->setCaptureInfo( orgHdr );
#endif

#ifdef rcWRITE_CAM
  // Write camera info header
  rcMovieFileCamExt camHdr = _acqInfo.cameraInfo;
  camHdr.id( id++ );

  if ( camHdr.write( _saveStream ) ) {
      _saveStatus.status(eMovieFileErr_WriteFailed);
      return eMovieFileState_Error;
  }
  // Update GUI
  _engine->setCameraInfo( camHdr );
#endif

  // Write EOF header
  rcMovieFileExt eofHdr( movieExtensionEOF );
  if ( eofHdr.write( _saveStream ) ) {
      _saveStatus.status(eMovieFileErr_WriteFailed);
      return eMovieFileState_Error;;
  }

  /* Just about done. Rewind, write header and close file.
   */
  if (fseek(_saveStream, 0, SEEK_SET)) {
    perror("fseek failed");
    if (fclose(_saveStream))
      perror("fclose failed");
    _saveStatus.status(eMovieFileErr_SeekFailed);
    return eMovieFileState_Error;
  }

  // Update rev2 header
  const off_t movieRecordSz = sizeof(int64) + _movieHdrRev2.bytesInFrame();
  // Offset to start of headers
  _movieHdrRev2.extensionOffset( sizeof(_movieHdrRev2) + sizeof(_movieHdrRev2) +
                                 movieRecordSz*_movieHdrRev2.frameCount());
  // Write rev2 header
  if ( _movieHdrRev2.write( _saveStream ) ) {
      _saveStatus.status(eMovieFileErr_WriteFailed);
      return eMovieFileState_Error;
  }

  if (fclose(_saveStream)) {
    perror("fclose failed");
    _saveStatus.status(eMovieFileErr_CloseFailed);
    return eMovieFileState_Error;
  }

  // Stop the engine
   if (_engine->getState() == eEngineRunning)
       _engine->stop();
  // Flush writers before saving
  _engine->frameFlush();

  _engine->resumePreview();
  rcExperimentFileFormat fileFormat = eExperimentNativeFormat;

  // Save experiment XML data
  rcPersistenceManager* persistenceManager =
    rcPersistenceManagerFactory::getPersistenceManager();
  std::string nativeExt = persistenceManager->fileFormatExportExtension(fileFormat);
  std::string reifyMovieExt =
    persistenceManager->fileFormatExportExtension( eExperimentNativeMovieFormat );

  // Choose movie save file name as base name
  std::string experimentFileName;
  std::string movieFile = _saveCtrl.getMovieFileName();
  std::string fileExt(movieFile, movieFile.size()-reifyMovieExt.length(),
		   reifyMovieExt.length());
  // Strip movie extension
  if (fileExt == reifyMovieExt) {
      std::string baseName(movieFile, 0, movieFile.size()-reifyMovieExt.length());
      experimentFileName = baseName;
  }
  experimentFileName += nativeExt;
  int error = _observer->notifyRequestSave(experimentFileName.c_str(),
					   fileFormat);
  // Report error if necessary
  if (error) {
      // Error during XML save!
      std::string errorString("Capture metadata save error:\n");
      errorString += strerror( error );
      errorString += " for file " + experimentFileName;
      _observer->notifyError( errorString.c_str());
  }

  rmAssert(_saveRing.availReleasedResources() ==
	   rcEngineMovieCapture::_bufRingSz);
  rmAssert(_saveRing.availNewSlots() == rcEngineMovieCapture::_bufRingSz);
  rmAssert(_saveRing.availReleasedSlots() == 0);
  rmAssert(_saveRing.availNewResources() == 0);

  {
    int64 bytesInMovie = ((_movieHdrRev2.bytesInFrame()) +
                            sizeof(int64))*_movieHdrRev2.frameCount();
    bytesInMovie += sizeof(_movieHdrRev2);
    fprintf(stderr, "Expected file size %12.0f frame count %d fps %f\n",
            (double)bytesInMovie, _movieHdrRev2.frameCount(),
            _movieHdrRev2.averageFrameRate());
  }

  if (finalStatus == eMovieFileErr_VariableFrameRate) {
    const rcSparseHistogram::sparseArray& histMap = _hist.getArray();
    rcSparseHistogram::sparseArray::const_iterator start = histMap.begin();
    rcSparseHistogram::sparseArray::const_iterator end = histMap.end();
    _warningMsg[0] = 0;
    uint32 curOffset = 0;
    for ( ; start != end; start++) {
      curOffset += strlen(_warningMsg + curOffset);
      const uint32 blen = rcMOVIE_WARN_MSG_SZ - curOffset;

      snprintf(_warningMsg + curOffset, blen, "\nInterval: %5d ms Count: %4d",
	       (*start).first, (*start).second);
    }

    if (_histOverflow) {
      curOffset += strlen(_warningMsg + curOffset);
      const uint32 blen = rcMOVIE_WARN_MSG_SZ - curOffset;

      snprintf(_warningMsg + curOffset, blen, "\nInterval: Various  Count: %4d",
	       _histOverflow);
    }

    std::string eMsg = "Warning! " +
      getMovieErrorString(eMovieFileErr_VariableFrameRate) + _warningMsg;
    fprintf(stderr, "Got warning: %s\n", eMsg.c_str());
    _observer->notifyWarning(eMsg.c_str());
  }
  else
    _observer->notifyStatus("Capture complete");

  _saveStatus.status(finalStatus);

  return eMovieFileState_Done;
}

/********************************************************************/
/* doneHdlr - Make sure engine is in the stopped state and then     */
/* wait for capture thread to decide whether or not to restart      */
/* the save state machine.                                          */
/********************************************************************/

rcMovieFileState rcEngineMovieSaver::doneHdlr(rcMovieFileState)
{
  /* Regardless of the outcome of the movie save, the engine should be
   * stopped.
   */
  if (_engine->getState() == eEngineRunning) {
    _engine->stop();
  }

  rmAssert(_engine->getState() != eEngineRunning);

  uint32 curResetValue;

  /* Note: This would best be done with a condition variable instead
   * of a polling loop.
   */
  while (_curResetValue.getValue(curResetValue) == _prevResetValue) {
    if (seppukuRequested()) {
      _saveStatus.status(eMovieFileErr_Aborted);
      return eMovieFileState_Exit;
    }

    usleep(100000); // Sleep for 100ms
  }

  _prevResetValue = curResetValue;
  _saveStatus.delayLeft(0);
  _saveStatus.captureTime(0);
  _saveStatus.status(eMovieFileErr_NoErr);
  _hist.reset();
  _histOverflow = 0;

  return eMovieFileState_Wait;
}

// Update acquisition info (camera control settings etc.)
void rcEngineMovieSaver::updateAcqInfo( const rcAcqInfo& acqInfo )
{
    _acqInfo = acqInfo;
}
void rcEngineMovieSaver::run()
{
  rcMovieFileState curState  = _saveStatus.getState();
  rcMovieFileState prevState = curState;

  for ( ; curState != eMovieFileState_Exit; ) {
    rcMovieFileState newState = eMovieFileState_Uninitialized; // Quiet compiler

    switch (curState) {
    case eMovieFileState_Uninitialized:
      newState = uninitializedHdlr(prevState);
      break;

    case eMovieFileState_Wait:
      newState = waitHdlr(prevState);
      break;

    case eMovieFileState_Delay:
      newState = delayHdlr(prevState);
      break;

    case eMovieFileState_Open:
      newState = openHdlr(prevState);
      break;

    case eMovieFileState_Save:
      newState = saveHdlr(prevState);
      break;

    case eMovieFileState_Close:
      newState = closeHdlr(prevState);
      break;

    case eMovieFileState_Error:
      newState = errorHdlr(prevState);
      break;

    case eMovieFileState_Done:
      newState = doneHdlr(prevState);
      break;

    case eMovieFileState_Exit:
      rmAssert(0);
      break;
    } // End of: switch (curState)

    rmAssert(newState != eMovieFileState_Uninitialized);
    prevState = curState;
    curState = newState;
    _saveStatus.state(curState);
  } // for ( ; curState != eMovieFileState_Done; )

  return;
}
