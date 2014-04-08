/* @file
 *  rc_engineimplsettings.cpp
 *
 *
 *  Created by Arman Garakani on 9/28/05.
 *
 */

#include "rc_engineimpl.h"

//@note TBD: ACI options,

std::string rcEngineImpl::focusScript ()
{
  std::string script ("perlvisible ");
  if (isSettingEnabled( cInputMovieFileSettingId ) )
    script += std::string (" -movie ") + std::string (getSettingValue( cInputMovieFileSettingId));
  rcRect r = getSettingValue( cAnalysisRectSettingId );
  script += std::string (" -size ") + rfToString (r.width());
  script += std::string (" -size ") + rfToString (r.height());
  script += std::string (" -position ") + rfToString (r.origin().x());
  script += std::string (" -position ") + rfToString (r.origin().y());
  script += std::string (" -duration ") + rfToString (getSettingValue (cAnalysisFirstFrameSettingId));
  script += std::string (" -duration ") + rfToString (getSettingValue (cAnalysisLastFrameSettingId));
  int32 am = getSettingValue( cAnalysisModeSettingId );
  switch (am)
	{
    case cAnalysisACISlidingWindow:
      script += std::string (" -slidingWindow ") + rfToString (getSettingValue( cAnalysisACISlidingWindowSizeSettingId));
            break;
    case cAnalysisACI:
				script += std::string (" -analysis ") + std::string ("aci");
			break;
#if 0
    case cAnalysisCellTracking:
      script += std::string (" -analysis ") + std::string ("cell");
      int32 ct = getSettingValue( cAnalysisCellTypeId );
      if (ct == cAnalysisCellGeneral)
				script += std::string (" -type ") += std::string ("general");
      else if (ct == cAnalysisCellMuscle)
				script += std::string (" -type ") += std::string ("smooth");
      else if (ct == cAnalysisLabeledFluorescence)
				script += std::string (" -type ") += std::string ("Flu");
      break;
#endif
	}
	
  script += std::string (" -use ") += std::string ("<your Visible.app>");
  script += std::string (" -result ") += std::string ("/tmp/foo");
  script += std::string (" -report ") += std::string ("csv");
  script += std::string (" -graphics ") += std::string ("no");
  return script;
}

//////////// rcSettingSource implementation //////////////////

// get the current value of a setting from the setting source
const rcValue rcEngineImpl::getSettingValue( int settingId )
{
	rcLock lock( _settingMutex);
	assertInitialized();
	
	switch (settingId)
	{
         
        case cAnalysisContractionThresholdId:
        return _contractionThreshold;
            
		case cInputModeSettingId:
                    return _inputMode;

            case cTemplateFileNameId:
									 return _model2useAbsPathFileName;
			
		case cFrameRateSettingId:
			return _frameRate;
		case cInputMovieFileSettingId:
			return _movieFile;

		case cInputImageFilesSettingId:
			return _imageFileNames;
			
		case cAnalysisRectSettingId:
			return _analysisRect;
			
		case cAnalysisACIOptionSettingId:
			return _aciOptions;
			
		case cAnalysisACISlidingWindowSizeSettingId:
				return _slidingWindowSize;
			
		case cAnalysisACISlidingWindowOriginSettingId:
				return int(_slidingWindowOrigin);
			
    case cAnalysisMotionTrackingSampleSettingId:
    case cAnalysisACIStepSettingId:
      return _aciTemporalSampling;
    case cAnalysisACINormalizeSettingId:
      return _doNormalize;
    case cImagePreMappingSettingId:
      return (_ippMode);
			
		case cAnalysisMotionTrackingTargetSettingId:
			return _motionTrackingTargetSize;
			
    case   cAnalysisMotionTrackingMultiplierSettingId:
      return _motionTrackingTargetSizeMultiplier;
			
		case cAnalysisMotionTrackingSpeedSettingId:
			return _motionTrackingMaxSpeed;
			
		case cAnalysisMotionTrackingMotionVectorDisplaySettingId:
			return _motionTrackingMotionVectorDisplay;
			
		case cAnalysisMuscleSegmentationVectorDisplaySettingId:
			return _segmentationVectorDisplay;
			
		case cAnalysisMotionTrackingBodyVectorDisplaySettingId:
			return _motionTrackingBodyVectorDisplay;
			
		case cAnalysisMotionTrackingBodyHistoryVectorDisplaySettingId:
			return _motionTrackingBodyHistoryVectorDisplay;
			
		case cAnalysisMotionTrackingFrameSampleSettingId:
			return _motionTrackingFrameSampling;
			
		case cAnalysisMotionTrackingStepSettingId:
			return _aciTemporalSampling;
			

		case cPlaybackCtrlSettingId:
			if (_player && _player->playbackStatus().getState() ==
					eMoviePlaybackState_Exit) {
				int ctrlInfo = _playbackCtrl.getCtrlInfo();
				ctrlInfo |= rcSettingInfo::eCurrentStateStopped;
				_playbackCtrl.ctrlInfo(ctrlInfo);
				_player->reset();
			}
			return _playbackCtrl.getCtrlInfo();
			
		case cAnalysisModeSettingId:
			return _analysisMode;
			
		case cAnalysisCellTypeId:
			return _cellType;
			
		case cAnalysisStatTypeId:
			return _statType;
			
		case cAnalysisMuscleAutoGenPeriodId:
			return _msPerContraction;
			
		case cVisualizeCellTextSettingId:
			return _showCellText;
			
		case cVisualizeCellPositionSettingId:
			return _showCellPosition;
			
		case cVisualizeCellPathSettingId:
			return _showCellPath;
			
		case cExportMotionVectorsSettingId:
			return _exportMotionVectors;
			
		case cExportBodyGraphicsSettingId:
			return _exportBodyGraphics;
			
		case cVisualizeFocusRotationSettingId:
			return _analysisAffine;
			
		case cAnalysisFirstFrameSettingId:
			return _analysisFirstFrame;
			
		case cAnalysisLastFrameSettingId:
			return _analysisLastFrame;
			
		case cAnalysisChannelSettingId:
			return _channelConversion;
			
		case cInputMovieGeneratorSettingId:
			return _captureLog;
			
		case cInputCameraHeaderSettingId:
			return _cameraInfo;
			
		case cInputConversionHeaderSettingId:
			return _conversionInfo;
			
    case cAnalysisObjectSettingId:
      return _analysisTreatmentObject;
			
		case cAnalysisDevelopmentVideoDisplaySettingId:
			return _developmentVideoEnabled;
			break;
			
		case cAnalysisDevelopmentGraphicsDisplaySettingId:
			return _developmentGraphicsEnabled;
			
		case cVisualizeDevelopmentSettingId:
			return _visualizeDevelopment;
			
    case   cGlobalMotionEstId:
      return _enableGlobalMotionEst;
			
	}
	
	return 0;
}

// set the new value of a setting to the setting source.
void rcEngineImpl::setSettingValue( int settingId , const rcValue& value )
{
	bool captureThreadStart = false;
	bool importThreadStart = false;
	bool playbackThreadStart = false;
	bool captureThreadStop = false;
	bool importThreadStop = false;
	bool playbackThreadStop = false;
	
	{ // Locked block
		rcLock lock( _settingMutex);
		
		assertInitialized();
		
		switch (settingId)
		{
			case cInputModeSettingId:
				_observer->notifyStatus(""); // Clear status bar
				
				_inputMode = (rcInputMode)(int)value;
				
				switch (_inputMode)
            {
#if 0
                case eCamera:
					{
						_fpsCalculator.reset();
						_cameraInfo = "";
						_frameRate = rcDEFAULT_FRAMES_PER_SEC;
						captureThreadStart = true;
						importThreadStop = true;
						playbackThreadStop = true;
						_analysisMode = cAnalysisACISlidingWindow;
						_aciOptions = static_cast<rcSimilarator::rcEntropyDefinition> (static_cast<int> (value));
						break;
					}
#endif
					case eFile:
					{
						_frameRate = 0;
						captureThreadStop = true;
						break;
					}
						
					case eCmd:
					{
						_frameRate = 1; // @todo Frame rate should be read from image files.
						captureThreadStop = true;
						break;
					}
						// Note: no default case to force a compiler warning if a
						// new enum value is defined without adding a corresponding
						// case statement here.
				}
				updateHeaderLogs();
				break;
				
			case cFrameRateSettingId:
				_frameRate = value;
				break;
				
			case cCameraFrameRateSettingId:
			{
				int inputValue = (int)value;
                setTimeLineRange();
#if 0
				if (inputValue >= 1) {
					_captureCtrl.decimationRate((uint32)(int)value);
					// Update timeline range in camera mode when
					// frame rate changes
					if ( _inputMode != eFile )

                        }
#endif
			}
				break;
#if 0
			case cCameraAcquireTimeSettingId:
			{
				int inputValue = (int)value;
				if (inputValue >= 0) {
					// Throttle max capture time based on max number of frames that can be
					// captured
					const double maxRate = _movieCapture->captureStatus().getMaxFramesPerSecond();
					const int maxSeconds = (uint32)(cMaxSaveFrameCount/(1.0*maxRate/_captureCtrl.getDecimationRate())) -1;
					int maxTime = maxSeconds/60;
					if ( inputValue > maxTime )
						inputValue = maxTime;
					_saveCtrl.frameCaptureTime((uint32)inputValue);
				}
			}
				break;
				
			case cCameraAcquireDelayTimeSettingId:
			{
				int inputValue = (int)value;
				if (inputValue >= 0)
					_saveCtrl.frameCaptureDelayTime((uint32)inputValue);
			}
				break;
#endif

			case cInputMovieFileSettingId:
				_movieFile = (std::string) value;
				_inputMode = (_batchMode) ? eCmd : eFile;
				_inputSource = eMovie;
				importThreadStart = true;
				importThreadStop = true; // (_inputMode == eCmd) ? false : true;
				captureThreadStop = true;
				playbackThreadStop = true;
				break;
				
			case cInputImageFilesSettingId:
				_imageFileNames = (std::string) value;
				_inputMode = (_batchMode) ? eCmd : eFile;
				_inputSource = eImages;
				importThreadStart = true;
				importThreadStop = true;
				captureThreadStop = true;
				playbackThreadStop = true;
				break;
#if 0
			case cOutputMovieFileSettingId:
				// Set capture output file name as movie input file name
				_movieFile = (std::string) value;
				_saveCtrl.outputFileName(_movieFile);
				_saveCtrl.movieFileName(_movieFile);
				break;
#endif

			case cAnalysisRectSettingId:
				if (_inputMode == eFile || _inputMode == eCmd)
					_analysisRect = value;
				break;
				
			case cAnalysisACIOptionSettingId:
				_aciOptions = static_cast<rcSimilarator::rcEntropyDefinition> (static_cast<int> (value));
				break;
				
			case cAnalysisACISlidingWindowSizeSettingId:
			{
				int newSize = value;
				// Zero size window is not allowed
				if ( newSize < 2 )
					newSize = 2;
                    //		if (_inputMode == eCamera)
                    //			_captureCtrl.slidingWindowSize(value);
                    //		else
					_slidingWindowSize = value;
			}
				break;
				
			case cAnalysisACISlidingWindowOriginSettingId:
			{
				int val = value;
                    //	if (_inputMode == eCamera)
                    //		_captureCtrl.slidingWindowOrigin((rcAnalyzerResultOrigin)val);
                    //	else
					_slidingWindowOrigin = rcAnalyzerResultOrigin(val);
			}
				break;
				
			case cAnalysisACIStepSettingId:
			{
				int val = value;
				_aciTemporalSampling = val;
				break;
			}
				
			case cAnalysisACINormalizeSettingId:
			{
				int val = value;
				_doNormalize = val;
				break;
			}
			case cAnalysisMotionTrackingTargetSettingId:
				_motionTrackingTargetSize = value;
				break;
				
			case  cAnalysisMotionTrackingMultiplierSettingId:
				_motionTrackingTargetSizeMultiplier = value;
				break;
				
			case cAnalysisMotionTrackingSampleSettingId:
	      _aciTemporalSampling = value;
				break;
				
			case cAnalysisMotionTrackingSpeedSettingId:
				_motionTrackingMaxSpeed = value;
				break;
				
			case cAnalysisMotionTrackingMotionVectorDisplaySettingId:
				_motionTrackingMotionVectorDisplay = value;
				break;
				
			case cAnalysisMuscleSegmentationVectorDisplaySettingId:
				_segmentationVectorDisplay = value;
				break;
				
			case cAnalysisMotionTrackingBodyVectorDisplaySettingId:
				_motionTrackingBodyVectorDisplay = value;
				break;
				
			case cAnalysisMotionTrackingBodyHistoryVectorDisplaySettingId:
				_motionTrackingBodyHistoryVectorDisplay = value;
				break;
				
			case cAnalysisMotionTrackingFrameSampleSettingId:
				_motionTrackingFrameSampling = value;
				break;
				
				// Use same control for temporal sampling
				//            case cAnalysisMotionTrackingStepSettingId:
				//                _motionTrackingStep = value;
				//                break;
				
			case cPlaybackCtrlSettingId:
			{
				int newCtrlInfo = value;
				int playbackCtrlInfo = _playbackCtrl.getCtrlInfo();
				int chgdCtrlInfo = newCtrlInfo ^ playbackCtrlInfo;
				if (chgdCtrlInfo & rcSettingInfo::eCurrentStateStopped) {
					if (newCtrlInfo & rcSettingInfo::eCurrentStateStopped)
						playbackThreadStop = true;
					else {
						importThreadStop = true;
						playbackThreadStart = true;
						playbackThreadStop = true;
					}
				}
				const int32 maxPlaybackSpeed = 1000000;
				int32 speed = newCtrlInfo & rcSettingInfo::eCurrentSpeedMask;
				if (speed > maxPlaybackSpeed) {
					speed = maxPlaybackSpeed;
					newCtrlInfo =
		      (newCtrlInfo & rcSettingInfo::eCurrentStateMask) |
		      maxPlaybackSpeed;
				}
				_playbackCtrl.ctrlInfo(newCtrlInfo);
				
				if (playbackThreadStart) {
					rmAssert(_player);
					rmAssert(_videoCacheP);
					
					if (newCtrlInfo & rcSettingInfo::eCurrentStateRev)
						speed = -speed;
					
					fprintf(stderr, "time %f speed %d\n",
									_observer->getCursorTime().secs(), speed);
					
					_player->setupTimeCtrlPlayback(*_videoCacheP,
																				 _observer->getCursorTime(),
																				 speed,
																				 5);
					rmAssert(_player->playbackStatus().getState() ==
									 eMoviePlaybackState_Initialized);
				}
			}
				break;
				
			case cImagePreMappingSettingId:
			{
				int val = value;
				_ippMode = (rcIPPMode) val;
			}
	      break;

			case cTemplateFileNameId:
                            _model2useAbsPathFileName = (std::string) value;
                	      break;
              
			case cAnalysisModeSettingId:
				_analysisMode = value;
				if (_analysisMode != cAnalysisCellTracking) 
                {
                        //					_cellType = cAnalysisCellGeneral;
					_msPerContraction = cAnalysisMuscleMsPerContractionDefault;
				}
				break;
#if 0
			case cAnalysisCellTypeId:
				_cellType = value;
				if (! (_cellType == cAnalysisCellMuscle || _cellType == cAnalysisCellMuscleSelected) )
					{
						_msPerContraction = cAnalysisMuscleMsPerContractionDefault;
					}
				_motionTrackingTargetSize = 5;
				break;
#endif
			case cAnalysisMuscleAutoGenPeriodId:
	      _msPerContraction = value;
	      break;
				
			case cVisualizeCellTextSettingId:
				_showCellText = value;
				break;
			case cVisualizeCellPositionSettingId:
				_showCellPosition = value;
				break;
			case cVisualizeCellPathSettingId:
				_showCellPath = value;
				break;
				
			case cExportMotionVectorsSettingId:
				_exportMotionVectors = value;
				if ( _motionVectorWriter ) {
					// Set track exportability
					rcWriter* writer = dynamic_cast<rcWriter*>(_motionVectorWriter);
					if ( writer )
						writer->setExportable( _exportMotionVectors );
				}
				break;
				
			case cExportBodyGraphicsSettingId:
				_exportBodyGraphics = value;
				if ( _bodyVectorWriter ) {
					// Set track exportability
					rcWriter* writer = dynamic_cast<rcWriter*>(_bodyVectorWriter);
					if ( writer ) {
						writer->setExportable( _exportBodyGraphics );
					}
				}
				break;
				
			case cVisualizeFocusRotationSettingId:
	      _analysisAffine = value;
				break;
				
			case cAnalysisFirstFrameSettingId:
				_analysisFirstFrame = value;
				if ( _analysisFirstFrame > int(framesLoaded())-1 )
					_analysisFirstFrame = framesLoaded()-1;
				if ( _analysisFirstFrame > _analysisLastFrame )
					_analysisFirstFrame = _analysisLastFrame;
				break;
				
			case cAnalysisLastFrameSettingId:
				_analysisLastFrame = value;
				if ( _analysisLastFrame > int(framesLoaded())-1 )
					_analysisLastFrame = framesLoaded()-1;
				if ( _analysisLastFrame < _analysisFirstFrame )
					_analysisLastFrame = _analysisFirstFrame;
				break;
				
			case cAnalysisChannelSettingId:
				_channelConversion = static_cast<rcChannelConversion>(int(value));
				break;
				
			case cAnalysisObjectSettingId:
				_analysisTreatmentObject = value;
				break;
				
				//            case cAnalysisTreatmentObjectSizeSettingId:
				//                _analysisTreatmentObjectSize = value;
				//                break;
				
			case cAnalysisDevelopmentVideoDisplaySettingId:
				_developmentVideoEnabled = value;
				break;
				
			case cAnalysisDevelopmentGraphicsDisplaySettingId:
				_developmentGraphicsEnabled = value;
				break;
				
			case cVisualizeDevelopmentSettingId:
				_visualizeDevelopment = value;
				break;
				
            case cAnalysisContractionThresholdId:
            _contractionThreshold = value;
            
			case cAnalysisStatTypeId:
	      _statType = value;
	      break;
				
			case   cGlobalMotionEstId:
				_enableGlobalMotionEst = value;
				break;
#if 0
			case cInputGainSettingId:
				_captureCtrl.gain(int(value));
				break;
			case cInputShutterSettingId:
				_captureCtrl.shutter(int(value));
				break;
			case cInputBinningSettingId:
	      _captureCtrl.binning(int(value));
				break;
#endif
		} // End of: switch (settingId)
	} // End of: Locked block
	
	/* Done setting new values. Start and stop any threads based on
	 * these new settings. Do here both to combine logic and to
	 * guarantee capture thread can grab _settingMutex.
	 */
	
	if (importThreadStop && _importThread && _importThread->isRunning()) {
		cout << "    import thread shutdown ";
		int status = _importThread->join();
		if (status)
			cout << "error " << status << endl;
		else
			cout << "OK" << endl;
	}
	
	
	if (captureThreadStop && _captureThread && _captureThread->isRunning()) {
		cout << "    capture thread shutdown ";
		int status = _captureThread->join();
		if (status)
			cout << "error " << status << endl;
		else
			cout << "OK" << endl;
		_observer->notifyStatus("Ready"); // Clear status bar
	}
	
	if (playbackThreadStop && _playbackThread && _playbackThread->isRunning()) {
		cout << "    playback thread shutdown ";
		int status = _playbackThread->join();
		if (status)
			cout << "error " << status << endl;
		else
			cout << "OK" << endl;
		_observer->notifyStatus("Ready"); // Clear status bar
	}
	
	if (importThreadStart && _importThread)
		{
			_importThread->start();
		}
	
	if (captureThreadStart && _captureThread) {
		_captureThread->start();
	}
	
	if (playbackThreadStart && _playbackThread) {
		_playbackThread->start();
	}
	
	uint32 activeThreads = 0;
	activeThreads += (_captureThread && _captureThread->isRunning()) ? 1 : 0;
	activeThreads += (_importThread && _importThread->isRunning()) ? 1 : 0;
	activeThreads += (_playbackThread && _playbackThread->isRunning()) ? 1 : 0;
	rmAssert(activeThreads < 2);
}

// return whether this setting is current enabled.
bool rcEngineImpl::isSettingEnabled( int settingId )
{
  if (_inputMode == eCmd) return true;
	
  rcLock lock( _settingMutex);
  assertInitialized();
	
  switch (settingId)
	{
    case cInputMovieFileSettingId:
      return ((_inputMode == eFile && _inputSource == eMovie) ||
							(_batchMode && _inputMode == eCmd && _inputSource == eMovie));
			
      //@note: batch mode does not suppport image file set yet
    case cInputImageFilesSettingId:
			return ((_inputMode == eFile && _inputSource == eImages) ||
							(_batchMode && _inputMode == eCmd && _inputSource == eImages));
			
			
    case cCameraFrameRateSettingId:
    case cCameraAcquireTimeSettingId:
    case cCameraAcquireDelayTimeSettingId:
    case cOutputMovieFileSettingId:
    case cInputGainSettingId:
    case cInputShutterSettingId:
    case cInputBinningSettingId:
      return _inputMode == eCamera;
			
    case cFrameRateSettingId:
      return _inputMode != eCamera;
			
    case cAnalysisRectSettingId:
    case cVisualizeFocusRotationSettingId:
      return getState() != eEngineUninitialized;
			
    case cAnalysisACISlidingWindowSizeSettingId:
    case cAnalysisACISlidingWindowOriginSettingId:
                //    if (_inputMode == eCamera)
                //			return  _captureCtrl.getSlidingWindowEnabled();
                //    else
				return _analysisMode == cAnalysisACISlidingWindow;
			
			
    case cAnalysisACIStepSettingId:
      return getState() != eEngineUninitialized &&
			_inputMode == eFile &&
			(_analysisMode == cAnalysisACI || _analysisMode == cAnalysisACISlidingWindow ||
			 _analysisMode == cAnalysisCellTracking );
			
    case   cGlobalMotionEstId:
    case cAnalysisACIOptionSettingId:
      return getState() != eEngineUninitialized &&
			_inputMode == eFile &&
			(_analysisMode == cAnalysisACI || _analysisMode == cAnalysisACISlidingWindow);
			
			
			
    case cPlaybackCtrlSettingId:
      return getState() != eEngineUninitialized;
			
    case cAnalysisObjectSettingId:   // Used to indicate Lineage Processing and other options
		{
		}
			
    case cAnalysisMotionTrackingMotionVectorDisplaySettingId:
    case cAnalysisMotionTrackingBodyVectorDisplaySettingId:
    case cAnalysisMotionTrackingTargetSettingId:
    case cAnalysisCellTypeId:
      return getState() != eEngineUninitialized &&  _analysisMode == cAnalysisCellTracking;
			
                //   case cAnalysisMotionTrackingSpeedSettingId:
                //     return getState() != eEngineUninitialized &&  _analysisMode == cAnalysisCellTracking &&
                //			! (_cellType == cAnalysisCellMuscle || _cellType == cAnalysisCellMuscleSelected);
			
    case cAnalysisMuscleAutoGenPeriodId:
    case cAnalysisMuscleSegmentationVectorDisplaySettingId:
        return getState() != eEngineUninitialized;
			
    case cAnalysisContractionThresholdId:
        return true;
                //     return getState() != eEngineUninitialized &&
                //		_analysisMode == cAnalysisCellTracking &&
                //		_cellType == cAnalysisCellMuscle;
			
      // This is not needed now, it can be enabled for internal debugging
    case cAnalysisMotionTrackingBodyHistoryVectorDisplaySettingId:
      if (!developerDebugging () )
				return getState() != eEngineUninitialized && _analysisMode == cAnalysisCellTracking;
      else
				return false;
      // Disable these until right default values have been determined
    case cAnalysisMotionTrackingFrameSampleSettingId:
    case cAnalysisMotionTrackingSampleSettingId:
      return false;
			
			
    case cAnalysisFirstFrameSettingId:
    case cAnalysisLastFrameSettingId:
      return (_inputMode == eFile);
			
    case cAnalysisChannelSettingId:
			return (_inputMode == eFile || _inputMode == eCmd) && (framesLoaded() > 0) &&
			(_frameDepth == rcPixel32 || _frameDepth == rcPixel16);
			
    case cInputMovieGeneratorSettingId:
      return true;
			
    case cInputCameraHeaderSettingId:
      return true;
			
    case cInputConversionHeaderSettingId:
      return (_inputMode == eFile);
			
    case cAnalysisDevelopmentVideoDisplaySettingId:
    case cAnalysisDevelopmentGraphicsDisplaySettingId:
    case cVisualizeDevelopmentSettingId:
      return developerDebugging();
	}
	
  return true;
}

// return whether this setting is current changable.
bool rcEngineImpl::isSettingEditable( int settingId )
{
	rcLock lock( _settingMutex);
	
	switch (settingId)
	{
		case cFrameRateSettingId:
			if ( _inputMode == eFile || _inputMode == eCmd )
				return _processCount == 0;
			else
				return false;
			
		case cCameraFrameRateSettingId:
		case cCameraAcquireTimeSettingId:
		case cCameraAcquireDelayTimeSettingId:
		case cOutputMovieFileSettingId:
			return _inputMode == eCamera &&
			getState() == eEngineStopped;
		case cInputMovieFileSettingId:
		case cInputImageFilesSettingId:
		case cInputModeSettingId:
			return _processCount == 0;
		case cAnalysisRectSettingId:
		case cVisualizeFocusRotationSettingId:
		case cAnalysisMotionTrackingMotionVectorDisplaySettingId:
		case cAnalysisMotionTrackingBodyVectorDisplaySettingId:
		case cAnalysisMotionTrackingBodyHistoryVectorDisplaySettingId:
		case cAnalysisMotionTrackingStepSettingId:
		case cAnalysisMotionTrackingFrameSampleSettingId:
		case cAnalysisMotionTrackingTargetSettingId:
		case cAnalysisMuscleAutoGenPeriodId:
		case cAnalysisMuscleSegmentationVectorDisplaySettingId:
		case cAnalysisCellTypeId:
		case cAnalysisModeSettingId:
		case cAnalysisMotionTrackingSpeedSettingId:
			if ((_inputMode != eFile) || (getState() == eEnginePlayback))
				return false;
			break;
			
		case cPlaybackCtrlSettingId:
			if ((_inputMode != eFile) || !_videoCacheP)
				return false;
			break;
			
			// Disable these until right default values have been determined
		case cAnalysisMotionTrackingSampleSettingId:
			return false;
			
		case cAnalysisACISlidingWindowSizeSettingId:
		case cAnalysisACISlidingWindowOriginSettingId:
		case cAnalysisContractionThresholdId:        
             //	if (_inputMode != eFile) {
              //			if ( _movieCapture->saveState() == eMovieFileState_Save )
               	//		return false;
            return true;

		case cAnalysisFirstFrameSettingId:
		case cAnalysisLastFrameSettingId:
			return (framesLoaded() > 0);
			
	    // We compute this at load time. Disable it after the first time.
		case cGlobalMotionEstId:
			return (framesLoaded() > 0 && _offsetsToFirst.size() == 0);
			
	}
	
	return getState() != eEngineRunning;
}

// return whether this setting is currently persistable.
bool rcEngineImpl::isSettingPersistable( int settingId )
{
	bool enabled = isSettingEnabled( settingId );
	
	rcLock lock( _settingMutex);
	assertInitialized();
	bool persist = false;
	
	switch ( settingId )
	{
			// Persist these so file import works
		case cFrameRateSettingId:
			persist = enabled;
			break;
			
		case cInputImageFilesSettingId:
			if ( enabled && _inputMode != eCmd )
	      persist = enabled;
			break;
			
		case cInputMovieFileSettingId:
			if ( enabled || _inputMode == eCamera )
				persist = true;
			break;
			
		default:
			persist = false;
			break;
	}
	
	return persist;
}

void rcEngineImpl::focusMyocyteSelection (const rcIRect& rect, vector<rc2Fvector>& ends, float& rad)
{
  ends.clear ();
  bool side = rect.width() > rect.height();
	
#if 1
  // Focus images are just the windows so ul of the rect has to be subtracted
  if (_focusRotation == 0)
    {
      if (side)
				{
					ends.push_back (rc2Fvector(0.0f, (float) rect.height()/2.0f));
					ends.push_back (rc2Fvector( (float) rect.width(), (float) rect.height()/2.0f));
					rad = rect.height()/2.0f;
				}
      else
				{
					ends.push_back (rc2Fvector((float) rect.width() / 2.0f, 0.0f));
					ends.push_back (rc2Fvector((float) rect.width() / 2.0f, (float) rect.height()));
					rad = (float) rect.width()/2.0f;
				}
    }
  else
    {
      // Create an affine rectangle at 0 degree and then rotate it
			
    }
#endif
}
