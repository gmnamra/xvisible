
#ifndef __rcSTATICSETTINGS_H
#define __rcSTATICSETTINGS_H

//#define rcROTATION_CTRL
//#define SLIDING_WINDOW_ORIGIN
#define SLIDING_WINDOW_DURATION
//#define CONTRACTION_DURATION

    //#include <rc_capture.hpp>
    //#include <rc_kinetoscope.h>

// input mode constant definitions

enum rcInputMode {
  eFile= 0,
  eCamera,
  eCmd
};

enum rcInputSource {
  eMovie = 0,
  eImages
};

enum rcIPPMode {
  eIPPNone=0,
  eAnalyzeRawDepth,
  eAnalysisAndDisplayRawDepth
};


// Settings id constants
static const int cInputModeSettingId = 0;
static const int cFrameRateSettingId = 1;
static const int cInputMovieFileSettingId = 2;
static const int cInputImageFilesSettingId = 3;

static const int cAnalysisRectSettingId = 5;
static const int cAnalysisACISlidingWindowSizeSettingId = 6;
static const int cAnalysisACISlidingWindowOriginSettingId = 7;

static const int cCameraFrameRateSettingId = 10;
static const int cCameraAcquireTimeSettingId = 11;
static const int cCameraAcquireDelayTimeSettingId = 12;
static const int cOutputMovieFileSettingId = 13;

// Motion tracking settings
static const int cAnalysisMotionTrackingTargetSettingId = 14;
static const int cAnalysisMotionTrackingSpeedSettingId = 15;
static const int cAnalysisMotionTrackingSampleSettingId = 16;
static const int cAnalysisMotionTrackingMotionVectorDisplaySettingId = 17;
static const int cAnalysisMotionTrackingBodyVectorDisplaySettingId = 18;
static const int cAnalysisMotionTrackingFrameSampleSettingId = 19;
static const int cAnalysisMotionTrackingStepSettingId = 20;

static const int cPlaybackCtrlSettingId = 21;
static const int cAnalysisMotionTrackingBodyHistoryVectorDisplaySettingId = 22;
static const int cVisualizeFocusRotationSettingId = 23;
static const int cAnalysisModeSettingId = 24;
static const int cVisualizeCellTextSettingId = 25;
static const int cVisualizeCellPositionSettingId = 26;
static const int cVisualizeCellPathSettingId = 27;

// Export settings
static const int cExportMotionVectorsSettingId = 28;

// ACI options
static const int cAnalysisACIOptionSettingId = 29;
static const int cAnalysisACIOptionSettingDiffusionId = 30;

static const int cAnalysisFirstFrameSettingId = 31;
static const int cAnalysisLastFrameSettingId = 32;

static const int cAnalysisChannelSettingId = 33;
static const int cInputMovieGeneratorSettingId = 34;

static const int cInputCameraHeaderSettingId = 35;
static const int cInputConversionHeaderSettingId = 36;

static const int cExportBodyGraphicsSettingId = 37;

static const int cAnalysisObjectSettingId = 38;
static const int cAnalysisTreatmentObjectSizeSettingId = 39;
static const int cVisualizeDevelopmentSettingId = 40;

// Muscle analysis
static const int cAnalysisMuscleAutoGenPeriodId = 41;
static const int cAnalysisContractionThresholdId = 42;

static const int cAnalysisCellTypeId = 43;
static const int cAnalysisStatTypeId = 44;
static const int cAnalysisMuscleSegmentationVectorDisplaySettingId = 45;
static const int cAnalysisMuscleMsPerContractionDefault = 0; // implies AutoGen
static const int cAnalysisMuscleSizeInPixelsDefault = 15;

// Result output
static const int cOutputFileSettingId = 46;
static const int cOutputFormatSettingId = 47;


// Sliding window result origin
static const int cAnalysisSlidingWindowOriginLeft = 48;
static const int cAnalysisSlidingWindowOriginCenter = 49;
static const int cAnalysisSlidingWindowOriginRight = 50;

// ACI options
static const int cAnalysisACIStepSettingId = 51;
static const int cGlobalMotionEstId = 52;
static const int cAnalysisACINormalizeSettingId = 53;
// Input Image Mapping Option
static const int cImagePreMappingSettingId = 54;
static const int   cAnalysisMotionTrackingMultiplierSettingId = 55;
static const int cTemplateFileNameId = 56;

const int cInputGainSettingId = 140;
const int cInputShutterSettingId = 141;
const int cInputBinningSettingId = 150;

static const int cAnalysisDevelopmentVideoDisplaySettingId = 1000;
static const int cAnalysisDevelopmentGraphicsDisplaySettingId = 1001;

/******************************************************************************
 *	Setting choices
 ******************************************************************************/

// choices for selecting an input source
static const rcSettingChoice inputModeChoices[] = 
  {
    rcSettingChoice( eFile	,	"File import" , "Select a data file as the input source to analyze" ),
   // rcSettingChoice( eCamera,	"Camera"	  , "Select the camera as the input source to analyze" ),
    rcSettingChoice( 0 , 0 , 0 )
  };

// Choices for selecting the capture frame rate
static const rcSettingChoice frameRateChoices[] = 
  {
    rcSettingChoice( 7.5	,	"7.5 fps"	, "capture at 7.5 fps" ),
    rcSettingChoice( 3.25	,   "3.25 fps"	, "capture at 3.25 fps" ),
    rcSettingChoice( 1.625	,	"1.625 fps" , "capture at 1.625 fps" ),
    rcSettingChoice( 0.8125	,	"0.8125 fps", "capture at 0.8125 fps" ),
    rcSettingChoice( 1		,	"Custom"	, "capture at custom fps" ),
    rcSettingChoice( 0		,	"Native"	, "use frame rate from input data" ),    
    rcSettingChoice( 0 , 0 , 0 )
  };

// Choices for analysis operations
static const rcSettingChoice analysisChoices[] = 
  {
//    rcSettingChoice( cAnalysisCellTracking,	    "Morphometry", "Morphometry" ),
    rcSettingChoice( cAnalysisACISlidingWindow, "Short Term ACI"	    , "Short Term ACI" ),
    rcSettingChoice( cAnalysisACI,	            "Long Term ACI" , "Long Term ACI" ),
//    rcSettingChoice( cAnalysisTemplateTracking,	            "Target Tracking" , "Target Tracking" ),
    rcSettingChoice( 0 , 0 , 0 )
  };

// Choices for analysis operations
static const rcSettingChoice statChoices[] = 
  {
    rcSettingChoice( cAnalysisStatMean, "Group Mean", "Group Mean" ),
    rcSettingChoice( cAnalysisStatMedian, "Group Median", "Group Median" ),
    rcSettingChoice( 0 , 0 , 0 )
  };


// Choices for ACI settings
static const rcSettingChoice ACIOptions[] = 
  {
    rcSettingChoice((int32) rcSimilarator::eACI, "Aggregate Change Index", "Decreases with Functional Self-Similarity" ),
    rcSettingChoice((int32) rcSimilarator::eVisualEntropy, "Aggregate Similarity Index", "Increases with Functional Self-Similarity" ),
    rcSettingChoice( 0 , 0 , 0 )
  };

// Choices for Options
static const rcSettingChoice processOptions[] = 
  {
    rcSettingChoice (eNoProcessingSelected, "None", "None"),
    rcSettingChoice ( eTemplateTracking , "Target Tracking", "Target Position"),
    rcSettingChoice( 0 , 0 , 0 )
  };



// TODO: hook these up with rcPersistenceManager

// choices for specifying the file name filter for movie file selection
static const rcSettingChoice movieFileFilterChoices[] =
  {
    // Qt seems to be case-sensitive...bummer
    rcSettingChoice( -1, "unused", "unused" ),
    rcSettingChoice( 0 , 0 , 0 )
  };

// choices for specifying the file name filter for movie file selection
static const rcSettingChoice imageFileFilterChoices[] =
  {
    rcSettingChoice( -1, "unused", "unused" ),
    rcSettingChoice( 0 , 0 , 0 )
  };

static const rcSettingChoice movieSaveFilterChoices[] =
  {
    rcSettingChoice( -1, "unused", "unused" ),
    rcSettingChoice( 0 , 0 , 0 )
  };

// choices for selecting sliding window origin
static const rcSettingChoice slidingWindowOriginChoices[] = 
  {
    rcSettingChoice( eWindowOriginLeft,	  "Left"  , "Result for frame T is computed from [T+1, T+2, ...]" ),
    rcSettingChoice( eWindowOriginCenter, "Center", "Result for frame T is computed from [..., T-1, T+1, ...]" ),
    rcSettingChoice( eWindowOriginRight,  "Right" , "Result for frame T is computed from [...,T-2, T-1]" ),
    rcSettingChoice( 0 , 0 , 0 )
  };

// choices for 32 or 16 -bit image channel handling
static const rcSettingChoice channelChoices[] = 
  {
    rcSettingChoice( rcSelectRed,	  "Red / First"	     , "Red, 1st,  channel is used" ),
    rcSettingChoice( rcSelectGreen,	  "Green / Second"	 , "Green, 2nd, channel is used" ),
    rcSettingChoice( rcSelectBlue,	  "Blue / Third"	 , "Blue, 3rd, channel is used" ),
    rcSettingChoice( rcSelectAverage, "Mean / Normalized"	 , "Average of all channels is used" ),
    rcSettingChoice( rcSelectAll,	  "All"	     , "Full Depth is used" ),
    rcSettingChoice( 0 , 0 , 0 )
  };


// Note: Need to allow 1 value less than the actual lower limit or QT
// gets confused after "New" is pressed and displays wrong value.
//

// ctor args for spinboxes
static rcSpinBoxArgs cCameraFramerateArgs = { (1000/30), false, true, 0, 150 };


static rcSpinBoxArgs cACIStepArgs = { 1, false, false, 1, 10};

// rotation thumwheel args
static rcThumbWheelArgs cRotationArgs = { 0, 360, 1.0, 0, 1 };

#if 0
// Choices for selecting pacing Frequency
static const rcSettingChoice pacingFreqChoices[] = 
  {
    rcSettingChoice( 5		,	"5 Hz"	, "Five Hertz" ),
    rcSettingChoice( 4		,	"4 Hz"	, "Four Hertz" ),
    rcSettingChoice( 3		,	"3 Hz"	, "Three Hertz" ),
    rcSettingChoice( 2		,	"2 Hz"	, "Two Hertz" ),
    rcSettingChoice( 1		,	"1 Hz"	, "One Hertz" ),
    rcSettingChoice( 0		,	"Auto"	, "Automatically Measure Pacing Frequency" ),    
    rcSettingChoice( 0 , 0 , 0 )
  };
#endif


static rcSpinBoxArgs cContractionThresholdArgs = { 1, false, false, 1, 100 };

// text area args
static rcTextAreaArgs cCameraHeaderArgs= { 5 };
static rcTextAreaArgs cCaptureHeaderArgs= { 6 };
static rcTextAreaArgs cConversionHeaderArgs= { 6 };

/******************************************************************************
 *	Setting specs
 ******************************************************************************/

// capture setting specs
static const rcSettingInfoSpec captureSettings[] =
  {
    {	// setting to select the input source
      cInputModeSettingId,
      "input-source",
      "Source",
      "Selects the input source",
      eMenuChoice,
      0,
      inputModeChoices, 1
    },
    {	// setting to select input files frame rate
      cFrameRateSettingId,
      "frame-rate",
      "Frame rate",
      "Sets the input frame rate",
      eFramerateChoice,
      0,
      frameRateChoices, 2
    },
    {	// setting to select the camera cature frame rate
      cCameraFrameRateSettingId,
      "frame interval",
      "Frame interval\n(ms)",
      "Sets frame interval in ms per frame",
      eSpinbox,
      &cCameraFramerateArgs,
      0, 2
    },
    {	// setting to select the camera cature frame rate
      cCameraAcquireTimeSettingId,
      "capture time",
      "Capture time\n(minutes)",
      "Sets number of minutes of data to capture",
      eSpinbox,
      0,
      0, 2
    },
    {	// setting to select the camera cature frame rate
      cCameraAcquireDelayTimeSettingId,
      "pre-capture delay time",
      "Pre-capture delay time\n(minutes)",
      "Sets number of minutes to wait before starting to capture",
      eSpinbox,
      0,
      0, 2
    },
    {	// setting to select the camera gain
      cInputGainSettingId,
      "camera-gain",
      "Gain    ",
      "Sets the camera gain",
      eSpinbox,
      0,
      0, 2
    },
    {	// setting to select the camera shutter
      cInputShutterSettingId,
      "camera-shutter",
      "Shutter ",
      "Sets the camera shutter speed",
      eSpinbox,
      0,
      0, 2
    },
    {	// setting to select the camera Binning
      cInputBinningSettingId,
      "camera-Binning",
      "Binning ",
      "Sets the camera Binning",
      eSpinbox,
      0,
      0, 2
    },
    {   // setting to show the movie file to analyze
      cInputMovieFileSettingId,
      "movie-file",
      "Input data",
      "Input data file to analyze",
      eFileChoice,
      0,
      movieFileFilterChoices, 4
    },
     {   // setting to show the image files to analyze
      cInputImageFilesSettingId,
      "image-files",
      "Input images",
      "Input image files to analyze",
      eDirectoryChoice,
      0,
      imageFileFilterChoices, 4
    },
    {   // setting to select the movie file to analyze
      cOutputMovieFileSettingId,
      "movie-file",
      "Output movie",
      "Output movie file",
      eFileSaveChoice,
      0,
      movieSaveFilterChoices, 4
    },
    {	// setting to show camera information
      cInputCameraHeaderSettingId,
      "input-camera-headers",
      "Camera information",
      "Capture camera information",
      eTextArea,
      &cCameraHeaderArgs,
      0, 4
    },
    {	// setting to show input data generator
      cInputMovieGeneratorSettingId,
      "input-movie-headers",
      "Capture data information",
      "Capture data information",
      eTextArea,
      &cCaptureHeaderArgs,
      0, 4
    },
    {	// setting to show conversion information
      cInputConversionHeaderSettingId,
      "input-conversion-headers",
      "Capture conversion information",
      "Capture conversion information",
      eTextArea,
      &cConversionHeaderArgs,
      0, 4
    },
    { 0 , 0 , 0 , 0 , 0 , 0 , 0, 0 }
  };

// analysis setting specs
static const rcSettingInfoSpec analysisSettings[] =
  {
      {	// setting to select the input source
          cInputModeSettingId,
          "input-source",
          "Source",
          "Selects the input source",
          eMenuChoice,
          0,
          inputModeChoices, 1
      },
      {   // setting to show the movie file to analyze
          cInputMovieFileSettingId,
          "movie-file",
          "Input data",
          "Input data file to analyze",
          eFileChoice,
          0,
          movieFileFilterChoices, 4
      },
      {   // setting to show the image files to analyze
          cInputImageFilesSettingId,
          "image-files",
          "Input images",
          "Input image files to analyze",
          eDirectoryChoice,
          0,
          imageFileFilterChoices, 4
      },
      {   // setting to show the image files to analyze
          cInputImageFilesSettingId,
          "image-files",
          "Input images",
          "Input image files to analyze",
          eDirectoryChoice,
          0,
          imageFileFilterChoices, 1
      },
    {   // I don't know what this is, just a dummy boolean checkbox setting
      cAnalysisRectSettingId,
      "analysis-rect",
      "",
      "Select the area of each input frame to analyze",
      eRect,
      0,
      0, 1
    },
#ifdef ROTATED_ROI //rc_build_option_selected_myocyte
    {	
      cVisualizeFocusRotationSettingId,
      "visualize-focus-rotation",
      "Rotate ",
      "Rotate analysis area (angle in degrees)",
      eThumbWheel,
      &cRotationArgs,
      0, 1
    },
#endif    
    {	// setting to select the first frame to analyze
      cAnalysisFirstFrameSettingId,
      "analysis-first-frame",
      "Start @ ",
      "Selects the first frame to analyze",
      eSpinbox,
      0,
      0, 2
    },
    {	// setting to select the last frame to analyze
      cAnalysisLastFrameSettingId,
      "analysis-last-frame",
      "End after ",
      "Selects the last frame to analyze",
      eSpinbox,
      0,
      0, 2
    },
    {	// setting to select the analysis operation
      cAnalysisModeSettingId,
      "analysis-operation",
      "Operation ",
      "Selects the analysis operation",
      eMenuChoice,
      0,
      analysisChoices, 3
    },
    {	// setting to select the channel conversion operation
      cAnalysisChannelSettingId,
      "canalysis-channel",
      "Process ",
      "Selects the image channel(s) to be used",
      eMenuChoice,
      0,
      channelChoices, 3
    },
    {	// setting to select the analysis operation
      cAnalysisStatTypeId,
      "Stat. Method",
      "Use",
      "Statistical Measure of Cell Population",
      eMenuChoice,
      0,
      statChoices, 3
    },
    {	// ACI options
      cAnalysisACIOptionSettingId,
      "aci-options",
      "Options ",
      "Selects the ACI option",
      eMenuChoice,
      0,
      ACIOptions, 4
    },
  #ifdef SLIDING_WINDOW_SIZE
    {	
      cAnalysisACISlidingWindowSizeSettingId,
      "aci-sw-size",
      "Duration ",
      "Number of frames used for ACI computation\nSpecify an odd number (3,5,7...) for a balanced window",
      eSpinbox,
      0,
      0, 4
    },
#endif
 
#ifdef SLIDING_WINDOW_ORIGIN
    {	
      cAnalysisACISlidingWindowOriginSettingId,
      "aci-sw-origin",
      "Reporting Origin",
      "Relative frame origin for ACI results",
      eMenuChoice,
      0,
      slidingWindowOriginChoices, 4
    },
#endif
    {	// setting to select the treatment object type
      cAnalysisObjectSettingId,
      "Enable ",
      "Enable ",
      "Enables Advanced Processing",
      eMenuChoice,
      0,
      processOptions, 9
    },
     {	
      cAnalysisACIStepSettingId,
      "frame-step",
      "Use Every ",
      "Number of frames to step during each analysis iteration",
      eSpinbox, 
      &cACIStepArgs,
      0, 4
    },
   {	
      cAnalysisACINormalizeSettingId,
      "auto norm",
      "Auto Norm",
      "Normalized",
      eCheckbox,
      0,
      0, 4
    },
    {	
      cAnalysisContractionThresholdId,
      "contraction-detection-threshold",
      "Contraction Relaxation Threshold",
      " 0.0 - 1.0",
      eSpinbox,
      &cContractionThresholdArgs,
      0, 6
    },
    { 0 , 0 , 0 , 0 , 0 , 0 , 0, 0}
  };

// analysis setting specs
static const rcSettingInfoSpec visualizationSettings[] =
  {
    {   // setting to control movie playback
      cPlaybackCtrlSettingId,
      "playback",
      "Movie playback",
      "Movie playback control",
      ePlaybackChoice,
      0,
      0, 1
    },
     {	
      cVisualizeCellTextSettingId,
      "visualize-cell-text",
      "Instant Information",
      "Display  name for all selected locomotive bodies",
      eCheckbox,
      0,
      0, 3
    },
    {	
      cVisualizeCellPositionSettingId,
      "visualize-cell-position",
      "Position",
      "Display center position for all selected locomotive bodies",
      eCheckbox,
      0,
      0, 3
    },
    {	
      cVisualizeCellPathSettingId,
      "visualize-cell-path",
      "Migration Path",
      "Display motion path for all selected locomotive bodies",
      eCheckbox,
      0,
      0, 3
    },
    {	
      cExportMotionVectorsSettingId,
      "export-motion-graphics",
      "Export motion graphics",
      "Export all motion graphics (can be slow)",
      eCheckbox,
      0,
      0, 3
    },
    {	
      cExportBodyGraphicsSettingId,
      "export-body-graphics",
      "Export morphometry graphics",
      "Export all locomotive body graphics",
      eCheckbox,
      0,
      0, 3
    },
    {	
      cVisualizeDevelopmentSettingId,
      "visualize-development-data",
      "Internal Use",
      "Display internal development images/graphics instead of regular images/graphics",
      eCheckbox,
      0,
      0, 3
    },
    {	
      cAnalysisMotionTrackingMotionVectorDisplaySettingId,
      "motion-vector-display",
      "Motion graphics",
      "Produce motion graphics",
      eCheckbox,
      0,
      0, 4
    },
    {	
      cAnalysisMotionTrackingBodyVectorDisplaySettingId,
      "motion-locomotive-body-display",
      "Morphometry graphics",
      "Produce graphics for all detected locomotive bodies",
      eCheckbox,
      0,
      0, 4
    },
    {	
      cAnalysisMotionTrackingBodyHistoryVectorDisplaySettingId,
      "motion-locomotive-body-history-display",
      "Morphometry history graphics",
      "Produce history graphics for all detected locomotive bodies",
      eCheckbox,
      0,
      0, 4
    },
    {	
      cAnalysisMuscleSegmentationVectorDisplaySettingId,
      "muscle-segmentation-display",
      "Muscle segmentation display graphics",
      "Show muscle segmentation information",
      eCheckbox,
      0,
      0, 4
    },
    {	
      cAnalysisDevelopmentVideoDisplaySettingId,
      "development-video-display",
      "Development images",
      "Produce image frames for development/debugging",
      eCheckbox,
      0,
      0, 4
    },
    {	
      cAnalysisDevelopmentGraphicsDisplaySettingId,
      "development-graphics-display",
      "Development graphics",
      "Produce graphics for development/debugging",
      eCheckbox,
      0,
      0, 4
    },
    { 0 , 0 , 0 , 0 , 0 , 0 , 0, 0 }
  };

// export setting specs
static const rcSettingInfoSpec exportSettings[] =
  {
    {	
      cExportMotionVectorsSettingId,
      "export-motion-graphics",
      "Export motion graphics",
      "Export all motion graphics (can be slow)",
      eCheckbox,
      0,
      0, 10
    },
    {	
      cExportBodyGraphicsSettingId,
      "export-body-graphics",
      "Export locomotive body graphics",
      "Export all locomotive body graphics",
      eCheckbox,
      0,
      0, 10
    },
    { 0 , 0 , 0 , 0 , 0 , 0 , 0, 0}
  };

// The capture setting category spec
static const rcSettingCategorySpec captureSettingsSpec =
  {
    "capture-settings" , "Capture" , "Settings that control video capturing"	, captureSettings
  };

// the analysis setting category spec
static const rcSettingCategorySpec analysisSettingsSpec =
  {
    "analysis-settings" , "Analyze" , "Settings that control data analysis"	, analysisSettings
  };

// the analysis setting category spec
static const rcSettingCategorySpec visualizationSettingsSpec =
  {
    "visualization-settings" , "Visualize" ,
    "Settings that control data visualization and export", visualizationSettings
  };

// the export setting category spec
static const rcSettingCategorySpec exportSettingsSpec =
  {
    "export-settings" , "Export" , "Settings that control data export"	, exportSettings
  };


#endif /* __rcSTATICSETTINGS_H */
