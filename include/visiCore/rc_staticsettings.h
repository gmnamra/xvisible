
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


static uint8 sTarget_Image_tip [128] [152] = 
{{207,177,159,144,135,147,147,129,105,84,75,72,87,102,123,129,144,144,156,159,162,171,174,162,174,168,171,165,162,174,168,150,138,132,135,150,171,192,216,219,225,225,219,213,198,177,168,159,165,168,165,162,165,162,168,168,171,165,159,159,150,147,150,144,159,168,165,171,171,162,171,168,168,171,177,171,165,171,162,165,162,165,162,165,159,162,162,168,168,165,162,162,159,165,162,159,162,162,159,147,147,144,147,159,162,171,168,162,162,156,156,147,147,144,156,156,147,156,150,147,147,147,150,156,156,150,150,147,156,156,156,156,150,150,150,147,147,150,159,150,150,156,144,147,141,144,150,135,141,138,141,147},
{186,159,138,132,132,135,135,114,99,84,75,75,96,108,123,132,138,138,150,159,165,162,162,159,162,159,162,162,162,168,162,150,141,138,144,159,174,195,216,219,225,225,216,213,189,174,165,162,171,165,165,165,162,165,162,168,165,168,168,165,165,165,162,150,156,165,162,165,168,159,165,162,162,168,168,168,159,168,162,165,159,159,162,165,162,162,162,162,162,159,159,165,162,171,171,159,162,165,156,156,147,150,147,150,159,165,159,159,156,150,150,138,141,138,147,147,144,150,147,144,144,144,144,144,147,144,141,138,147,150,156,156,150,147,150,141,141,147,150,150,144,156,144,144,138,138,144,135,144,141,144,147},
{177,159,141,138,138,132,132,108,96,81,75,75,99,114,126,135,141,150,162,162,165,156,156,156,156,159,159,159,162,165,159,150,150,156,162,171,183,198,216,225,216,219,213,210,186,171,168,162,171,171,168,165,162,165,165,159,159,168,174,171,171,171,165,162,156,165,162,159,162,156,156,159,156,159,159,165,162,171,162,162,162,150,159,162,162,156,162,159,156,156,156,162,162,171,171,162,159,159,156,159,147,150,150,147,159,162,159,162,150,150,147,135,138,138,141,141,144,147,144,135,138,141,141,138,141,141,138,138,141,144,147,150,150,144,144,135,135,138,144,156,147,156,141,138,141,132,138,135,144,144,144,144},
{177,171,156,144,144,135,138,111,90,75,72,72,99,126,141,147,147,165,171,168,168,156,156,150,150,162,159,156,159,159,165,162,165,174,177,186,195,204,219,219,216,216,207,204,177,171,171,165,168,174,168,168,168,162,165,156,162,168,171,177,171,162,162,165,156,165,165,156,159,156,150,159,150,159,159,159,165,174,171,162,165,156,156,159,159,147,159,156,156,156,156,159,159,165,165,162,156,159,150,156,150,150,147,147,159,162,156,162,147,147,144,141,144,144,144,144,147,144,141,132,132,135,141,141,138,138,144,138,141,144,141,144,141,138,141,132,132,138,138,147,147,144,144,132,138,126,135,135,141,138,135,135},
{165,150,132,132,144,144,147,129,93,66,60,72,111,138,150,165,150,159,168,168,174,156,147,150,150,162,159,162,168,162,171,174,183,186,186,192,204,213,225,225,225,213,195,186,174,174,171,171,171,174,171,174,174,168,171,159,168,168,171,183,174,162,162,162,156,162,165,159,156,156,150,159,162,162,159,150,159,162,171,162,165,162,159,156,156,150,156,147,150,147,156,156,156,159,159,159,159,156,150,150,150,150,141,147,156,162,150,156,141,141,138,141,144,147,150,150,147,141,138,135,132,135,138,138,135,135,138,132,138,144,141,138,138,135,141,129,129,138,135,138,138,135,138,120,120,114,129,129,138,132,126,132},
{147,132,111,120,138,144,150,138,102,69,54,66,111,141,150,165,156,162,165,159,162,147,150,147,150,168,162,165,171,168,186,183,192,186,189,195,204,219,219,234,219,213,189,177,174,165,177,168,171,168,171,171,165,168,171,159,162,165,168,174,171,165,165,159,150,156,162,159,156,156,150,156,162,150,147,150,159,150,171,165,165,168,159,156,150,156,156,144,150,144,156,150,156,156,156,159,162,147,156,156,141,147,141,156,156,165,147,147,138,138,135,141,138,144,147,147,144,141,141,132,129,132,135,138,138,138,135,129,132,138,138,141,141,132,135,129,129,129,132,135,132,129,123,108,102,108,126,123,135,126,123,120},
{162,138,126,129,138,138,138,105,102,75,72,78,120,147,165,171,171,165,168,156,150,147,141,138,162,171,177,177,183,195,192,177,195,183,192,198,216,219,237,210,219,204,186,174,165,168,174,150,156,159,156,156,156,156,156,141,159,156,162,165,159,162,156,156,162,159,159,162,159,159,159,138,156,147,159,156,150,156,159,141,159,150,150,156,156,144,150,138,144,150,156,147,156,147,159,141,162,144,150,147,135,138,138,132,144,156,147,141,135,135,132,126,135,129,144,135,138,129,126,120,126,126,135,132,129,123,120,108,120,129,138,141,135,132,123,105,120,114,114,114,120,114,111,96,96,99,102,102,105,105,105,99},
{174,147,138,132,138,135,135,114,102,84,78,87,120,147,174,186,183,174,168,159,159,147,141,156,168,183,195,204,204,207,207,186,189,177,189,195,216,228,234,213,207,192,177,174,171,165,174,150,150,156,156,159,150,156,159,147,156,150,156,162,156,159,162,165,171,168,171,168,165,165,159,150,159,150,150,156,147,159,156,150,156,150,159,156,150,147,150,147,147,150,150,150,156,150,156,150,156,147,144,141,138,141,144,144,144,147,147,144,135,132,132,132,138,135,141,135,132,126,120,120,126,126,132,129,126,120,111,108,123,129,135,138,135,132,123,114,120,111,114,114,114,120,120,108,105,99,96,96,99,102,105,102},
{162,144,138,132,138,135,138,114,99,87,81,87,120,147,177,192,186,177,162,162,156,141,138,162,174,192,216,228,219,216,210,186,183,171,183,198,219,231,231,210,192,177,171,174,174,168,168,150,147,156,150,159,150,159,162,159,156,147,147,156,156,159,165,168,168,168,171,168,165,165,156,147,147,147,141,144,132,150,147,156,156,150,159,150,147,150,147,144,138,135,135,144,150,147,147,147,138,144,141,135,138,138,135,138,135,138,135,129,123,120,129,129,129,129,129,129,120,120,111,108,114,120,126,126,120,114,108,108,120,126,132,132,129,126,120,108,108,105,108,114,108,108,108,108,105,99,93,90,93,96,99,102},
{150,144,132,135,132,138,132,111,102,84,84,84,123,144,174,195,189,183,165,162,156,144,141,159,177,204,228,237,231,213,195,177,171,168,177,207,219,231,225,198,186,171,171,171,174,174,162,147,150,156,147,150,156,162,159,162,156,150,144,147,159,159,159,156,156,156,156,150,150,150,147,156,147,150,135,135,120,135,129,144,147,144,144,138,141,147,141,132,120,120,114,135,135,141,138,132,123,135,132,129,129,129,123,123,123,129,120,111,114,111,123,123,120,120,111,111,99,102,96,105,108,111,111,114,105,102,96,102,111,111,120,120,114,111,105,99,96,96,99,108,99,99,96,90,90,90,90,90,90,90,90,93},
{159,150,135,135,135,138,132,108,102,81,90,90,123,147,171,192,195,189,168,165,147,147,144,159,183,210,228,231,225,204,183,168,159,165,183,210,225,231,210,189,183,168,165,165,168,174,165,147,156,150,147,144,156,159,159,159,150,150,144,144,150,150,150,138,144,144,138,138,141,141,147,147,141,141,135,132,126,129,123,129,135,132,129,129,129,135,135,129,120,120,111,129,126,135,129,123,120,120,114,120,114,120,114,111,105,108,102,102,114,111,114,111,111,102,102,96,93,90,90,99,102,105,105,108,99,99,90,93,99,99,102,102,99,99,93,99,99,96,93,102,93,96,96,78,81,84,87,90,90,87,84,84},
{159,144,135,129,138,132,129,111,102,84,90,96,114,144,165,189,198,192,174,162,144,147,147,168,186,210,219,219,213,192,183,159,156,162,183,210,219,225,204,189,171,165,162,156,162,168,171,159,159,150,156,144,156,156,159,162,150,150,147,144,144,144,147,141,147,144,138,135,135,138,147,147,141,138,135,129,123,120,114,120,123,120,120,126,120,123,129,132,129,126,120,126,114,123,120,108,114,105,105,108,102,108,108,99,96,99,96,102,108,108,114,99,102,90,96,87,93,93,93,93,96,99,99,102,96,99,96,93,96,96,96,96,96,96,93,96,102,99,96,99,96,99,96,78,81,81,81,84,84,81,81,78},
{159,141,135,126,129,120,120,111,105,93,96,99,114,138,162,189,195,195,186,162,150,150,156,174,186,204,210,210,204,186,177,150,156,159,174,207,216,216,204,183,168,162,156,150,159,159,165,162,162,156,150,141,150,150,156,162,150,147,141,135,135,138,141,144,144,141,132,123,123,123,126,126,126,126,120,120,114,111,114,105,108,105,105,111,105,108,111,120,120,120,111,111,102,105,102,93,102,96,99,96,90,93,93,96,93,96,99,102,102,105,111,93,93,84,87,81,87,90,93,93,96,93,96,96,93,93,93,90,93,90,90,90,90,93,90,87,96,93,96,93,90,90,90,81,81,81,78,78,75,75,72,75},
{156,141,135,129,126,114,120,114,114,105,105,96,126,138,162,177,183,189,183,156,156,150,156,174,183,198,204,204,195,177,168,150,150,156,174,198,213,216,204,171,168,156,150,156,156,159,150,156,159,150,141,135,141,147,144,156,147,138,129,120,123,129,132,135,129,129,114,102,102,96,93,99,96,96,90,96,93,90,93,90,96,96,90,96,90,96,93,96,99,99,99,99,90,93,90,84,90,84,90,81,81,81,84,90,87,87,90,96,96,96,93,81,81,72,78,75,81,90,93,90,93,87,93,90,93,93,96,84,87,81,81,81,84,87,84,78,87,84,87,81,78,78,84,78,78,81,81,75,72,66,60,72},
{159,141,135,129,129,120,129,105,120,114,126,120,138,147,162,165,177,168,174,168,168,162,168,162,177,186,192,186,189,171,156,126,135,147,171,195,213,213,192,147,162,156,150,150,147,147,138,141,144,144,141,138,135,135,138,120,132,123,120,102,96,105,105,99,99,90,90,87,81,84,81,72,81,75,81,75,75,72,75,57,63,66,75,72,69,60,57,51,69,75,75,75,78,75,69,51,60,60,69,60,66,63,66,60,75,75,81,78,75,69,63,45,60,57,63,75,84,87,90,66,78,69,72,84,84,87,84,69,78,72,81,81,78,78,78,60,78,69,72,69,72,69,69,63,81,75,75,75,66,63,51,48},
{159,141,141,135,141,132,126,120,126,120,138,132,147,159,165,174,177,174,183,168,171,174,171,174,174,183,189,186,186,174,162,141,144,156,177,204,219,213,183,159,156,147,156,156,144,144,141,147,147,147,144,132,123,114,114,114,114,108,108,90,84,87,90,90,87,75,72,69,69,75,78,72,72,72,72,63,60,60,57,51,54,60,66,63,63,54,54,51,60,66,66,72,72,66,57,51,57,57,66,60,66,63,63,66,72,78,75,72,66,60,60,54,57,51,57,69,81,81,87,66,69,66,66,72,75,78,72,72,72,75,78,72,78,72,78,66,78,72,72,66,69,63,69,66,78,78,78,75,66,63,54,54},
{159,156,147,135,135,138,135,135,132,123,144,138,156,171,174,177,177,177,183,171,171,177,174,183,174,183,183,183,177,177,165,156,156,168,186,204,213,204,171,162,150,144,150,147,141,144,138,141,141,138,129,111,93,81,75,87,87,84,84,72,66,66,69,66,63,57,54,54,54,60,60,63,60,63,57,51,48,45,42,42,45,45,51,48,51,48,48,42,48,54,57,57,60,57,51,48,54,54,63,60,66,60,60,60,60,72,66,66,60,57,63,51,54,54,60,66,75,72,81,69,66,66,63,63,69,66,63,69,63,72,72,66,75,72,75,72,75,72,72,63,66,60,66,72,75,78,78,72,69,63,57,60},
{147,144,138,135,135,138,132,138,135,129,144,144,162,171,174,177,177,177,174,174,168,177,177,174,174,177,171,174,174,174,162,162,165,177,192,195,198,189,162,150,147,138,138,135,132,138,129,126,123,111,96,81,63,51,45,54,54,57,54,51,54,48,48,42,42,39,42,42,42,45,45,51,48,51,42,45,39,36,39,36,39,33,39,36,39,42,42,39,39,42,48,45,45,48,42,45,48,51,60,54,57,57,57,54,57,63,60,60,57,57,57,45,51,54,57,60,69,69,72,75,69,66,63,60,63,63,60,60,60,63,66,66,69,75,75,75,72,72,72,60,66,60,66,72,75,81,75,66,69,69,60,60},
{144,135,129,138,138,132,132,135,141,144,150,156,168,165,174,168,174,171,165,171,162,168,171,165,171,171,162,165,165,168,168,174,177,192,195,192,192,177,159,138,141,132,123,120,120,120,114,105,99,84,72,60,48,39,36,39,42,42,36,45,45,36,39,33,30,30,27,30,33,39,42,45,42,45,36,42,39,33,36,33,36,27,33,30,36,42,45,45,36,36,39,39,39,39,36,45,48,54,57,48,51,54,54,51,54,51,54,54,63,63,57,48,48,45,51,54,63,69,66,72,69,63,60,63,60,60,60,57,63,57,60,66,63,72,72,69,72,69,69,60,69,63,66,75,75,78,72,63,69,69,60,60},
{147,138,132,135,135,132,141,138,147,156,150,159,168,162,174,159,162,162,159,162,150,162,162,162,165,162,156,162,159,162,174,186,198,210,198,186,177,165,156,129,132,123,111,108,99,96,96,84,75,66,54,51,45,42,42,42,39,42,36,45,39,33,36,33,27,27,24,27,33,36,42,39,36,39,39,39,42,45,42,36,39,27,33,33,39,48,51,51,45,39,36,33,33,36,36,48,48,57,57,45,45,51,54,54,57,45,48,51,63,69,63,54,51,45,60,60,60,69,66,66,66,57,60,63,60,60,60,60,63,63,60,63,60,63,69,63,69,66,66,60,72,66,66,72,75,75,72,69,66,66,63,57},
{147,135,135,132,138,135,138,141,144,147,147,150,162,159,171,150,156,159,159,156,150,159,156,156,159,156,159,159,162,165,177,189,207,213,189,168,150,141,138,123,114,108,99,90,81,69,66,54,48,42,39,39,39,39,36,42,36,39,36,39,36,36,36,33,30,30,27,27,33,36,39,39,39,36,42,42,48,51,48,42,42,33,36,39,42,54,57,54,54,45,36,30,33,39,45,54,54,57,57,45,45,54,54,69,63,54,48,48,57,63,60,57,51,54,66,60,60,66,66,69,60,63,63,66,69,66,66,63,60,66,60,63,60,60,63,60,69,63,66,66,72,66,69,69,78,75,75,75,69,66,63,54},
{141,135,138,126,135,135,132,138,138,141,147,147,150,150,159,147,150,156,156,150,147,150,150,150,156,156,162,162,171,171,174,189,204,198,171,150,135,120,114,108,96,93,84,72,60,48,36,27,24,21,24,27,30,30,27,33,27,36,33,33,33,42,33,36,30,33,27,30,36,36,42,45,48,39,45,45,48,48,51,45,48,36,42,42,48,60,63,57,54,45,33,33,39,45,54,63,57,60,57,48,48,57,54,72,60,60,51,54,57,60,60,63,51,54,57,54,63,63,60,72,57,69,72,72,78,75,75,63,60,66,63,63,60,63,63,57,69,57,66,69,69,66,69,63,81,78,78,78,72,69,60,57},
{150,132,135,126,126,129,129,132,144,150,156,156,147,156,159,141,150,147,144,144,144,144,141,126,150,150,168,174,183,189,192,177,186,186,168,138,114,99,93,66,66,54,42,36,30,30,21,9,21,15,21,21,21,27,24,18,24,21,24,27,24,33,36,30,39,30,39,36,42,42,45,42,48,42,48,45,42,48,45,33,45,42,54,57,60,57,57,54,54,51,51,51,51,57,57,60,66,63,57,60,57,57,63,51,63,60,60,63,60,60,63,51,57,63,63,60,57,60,60,63,72,66,75,72,75,69,72,57,57,63,60,60,60,57,63,51,72,60,69,66,63,63,69,54,72,75,69,75,75,66,63,57},
{150,138,135,132,129,132,129,141,150,159,162,156,150,159,162,165,162,147,141,141,141,144,147,159,171,162,168,186,192,192,189,171,174,168,150,126,99,84,78,63,54,39,39,30,24,21,24,15,18,21,24,21,24,21,27,21,24,27,27,27,30,33,39,39,48,42,48,42,45,39,45,48,48,54,51,48,48,48,51,45,54,54,63,66,66,66,63,66,66,60,60,60,63,63,63,66,69,66,63,60,60,60,66,54,60,60,63,69,66,66,69,66,69,69,63,60,57,60,63,72,75,72,78,75,81,72,78,69,69,75,69,69,66,63,66,63,75,63,75,72,66,66,75,66,78,78,72,75,75,69,69,69},
{150,138,141,135,132,132,132,147,156,162,165,156,150,162,165,171,165,147,138,138,138,150,165,174,186,177,177,189,186,189,177,156,147,135,120,99,81,66,60,51,48,30,33,21,24,21,24,21,18,21,27,21,27,21,27,24,33,30,33,33,33,39,42,45,51,48,54,48,48,42,45,57,51,57,51,54,57,51,54,48,51,57,63,69,69,72,69,69,66,66,63,60,60,60,63,72,69,69,66,63,60,66,66,57,60,63,63,72,75,72,78,75,72,72,63,63,60,66,69,78,78,78,78,75,81,75,81,78,75,78,78,75,75,72,72,72,78,72,81,75,72,72,78,75,81,78,75,75,75,72,72,75},
{150,129,141,126,135,129,135,144,156,162,162,150,150,162,168,162,159,144,138,141,144,162,168,177,186,183,177,189,177,174,162,129,114,96,81,69,60,54,51,36,39,27,21,18,24,24,21,21,21,21,27,21,27,24,30,30,39,36,39,42,36,45,45,48,48,51,54,51,54,48,51,57,57,54,57,57,57,57,57,57,54,60,63,66,63,72,66,66,66,72,66,60,57,60,66,78,78,78,75,72,72,75,78,66,63,63,60,69,78,78,84,75,72,75,69,69,66,72,72,78,78,75,78,72,78,72,75,75,75,78,78,75,75,75,75,75,78,75,81,75,78,78,78,75,81,81,81,78,75,69,69,72},
{141,123,135,120,132,129,135,144,156,159,159,156,156,162,171,159,156,141,135,144,156,162,162,177,183,177,171,174,165,162,147,108,90,66,51,42,42,42,42,30,27,24,21,21,24,27,27,21,30,24,27,27,33,27,36,39,42,45,45,45,45,45,51,54,54,57,57,54,60,57,60,57,63,63,66,63,57,66,66,63,60,66,69,72,69,78,75,72,75,78,75,69,69,72,75,72,78,75,72,78,75,75,81,72,69,69,66,75,81,81,87,81,81,81,75,75,72,72,72,75,81,75,84,75,78,75,72,75,75,75,78,78,78,78,75,78,75,78,78,78,81,81,81,78,81,84,87,84,78,72,72,69},
{132,123,126,120,126,129,138,147,156,159,159,159,159,162,171,156,156,144,138,147,159,165,165,174,177,177,165,156,144,135,126,90,72,51,33,27,30,30,33,30,24,24,24,30,30,30,36,27,36,36,30,36,42,33,45,42,48,48,51,51,48,51,54,60,57,60,60,57,57,57,60,63,69,72,72,69,69,72,75,75,75,75,75,75,75,78,75,81,81,78,81,81,78,81,81,72,81,78,78,84,84,78,84,75,75,78,78,84,87,87,87,90,87,84,81,78,78,78,78,81,84,78,90,81,81,81,78,75,78,78,81,84,84,87,84,84,75,78,81,84,84,81,87,78,81,84,81,81,78,72,75,72},
{126,120,123,126,129,132,141,150,156,162,159,159,162,162,168,150,159,150,147,150,159,171,174,174,174,165,147,129,114,96,84,66,57,42,30,24,24,27,30,27,24,24,21,24,33,33,39,36,39,39,36,42,42,39,48,45,54,51,54,54,48,54,54,57,57,60,63,60,60,63,63,75,72,75,75,75,78,78,78,78,81,75,78,78,78,81,81,81,84,78,84,84,78,84,78,75,78,81,81,87,84,81,81,78,81,84,87,87,87,90,87,90,90,84,87,81,84,81,84,84,84,81,90,81,87,84,81,78,81,81,87,90,93,96,93,87,78,81,81,84,87,81,87,75,84,81,78,81,78,75,81,78},
{126,120,126,126,132,132,141,156,150,162,162,159,162,162,165,156,159,156,159,162,162,171,177,168,171,159,132,93,78,66,63,48,45,39,33,30,27,30,30,30,27,21,24,24,39,36,45,39,42,39,39,45,33,45,45,48,54,54,54,54,51,51,57,54,57,60,69,69,66,75,69,78,78,78,78,78,78,78,81,84,87,78,81,78,81,78,81,81,90,81,87,87,78,87,81,81,81,87,90,90,90,87,84,78,81,84,87,87,84,90,90,87,90,81,90,81,87,81,81,84,81,81,87,78,87,81,84,81,84,81,87,93,93,96,90,87,81,84,75,84,90,81,84,75,87,84,81,87,84,78,84,84},
{120,111,114,114,129,132,141,141,156,150,159,162,162,165,150,150,165,168,171,162,168,171,165,150,150,114,84,57,42,45,36,27,33,33,33,30,30,30,27,18,24,21,21,27,36,45,45,36,54,42,51,42,42,48,51,39,48,51,48,54,48,48,54,51,63,69,78,75,78,75,81,69,78,78,75,78,81,78,78,81,93,93,96,87,90,78,81,78,93,87,93,93,96,93,96,84,93,93,93,87,87,84,93,78,96,96,93,90,90,87,90,75,81,81,96,84,93,81,81,69,84,78,90,87,84,90,90,78,93,90,96,90,93,96,90,81,90,84,96,87,93,90,93,72,87,87,90,90,90,84,96,72},
{123,111,114,120,132,135,138,159,165,174,171,168,168,162,156,174,171,174,177,174,171,174,171,141,114,75,57,54,51,42,27,30,30,27,33,36,30,33,30,21,27,21,27,30,39,42,45,42,51,48,57,54,57,57,60,48,54,57,54,57,54,57,63,60,69,75,81,81,84,81,87,75,78,78,78,75,78,81,81,87,90,87,93,87,87,81,87,87,93,90,90,96,99,93,96,93,102,96,93,90,87,81,93,87,96,93,96,96,93,87,96,87,90,90,93,90,87,84,78,81,90,87,93,90,90,93,96,96,93,93,96,96,99,93,93,93,96,93,96,87,96,90,96,84,93,93,96,96,96,93,99,93},
{120,111,114,123,135,141,147,162,162,174,171,171,174,162,159,168,168,171,174,174,165,165,150,111,75,42,33,45,48,39,24,27,30,30,33,33,33,36,36,24,30,27,30,33,42,45,48,54,51,57,60,63,69,63,63,54,57,60,60,63,63,66,69,69,75,81,87,84,90,90,90,81,78,78,78,78,78,81,87,90,90,84,87,87,90,84,90,93,90,96,96,102,102,93,96,96,102,96,93,90,90,87,96,96,96,93,99,99,96,93,102,93,96,96,93,96,84,90,84,84,90,87,90,90,93,93,99,96,90,96,96,93,102,93,96,93,93,99,96,90,99,87,93,93,93,96,96,96,96,96,96,102},
{114,111,114,126,138,150,162,165,168,168,171,171,174,168,162,159,162,168,165,162,147,138,123,78,45,24,24,36,42,33,30,24,30,30,30,33,36,39,39,27,36,30,33,36,48,51,54,60,54,60,63,66,72,69,66,63,60,60,63,69,72,75,75,75,75,81,87,87,90,93,90,78,84,81,78,84,84,87,93,90,93,87,90,87,93,90,93,93,90,102,105,108,99,96,93,96,93,93,93,84,87,96,96,96,99,96,96,96,99,99,105,93,96,93,96,96,93,99,93,90,90,93,90,93,99,102,111,99,99,102,99,96,105,102,102,105,99,108,102,96,99,84,90,96,96,102,96,99,93,96,96,99},
{111,111,114,126,141,156,165,171,177,171,171,168,162,165,162,162,162,156,156,141,129,111,93,63,30,21,24,36,42,33,30,27,27,27,27,33,39,42,42,36,39,36,33,39,51,54,60,57,57,60,66,69,72,78,72,69,63,60,66,72,81,81,78,75,78,84,90,93,93,93,87,78,87,87,84,90,96,96,99,90,96,96,96,93,93,90,93,93,96,105,108,108,96,99,96,99,93,93,90,81,84,93,90,93,99,96,93,90,99,102,105,96,99,87,96,90,99,99,105,99,96,96,93,96,105,105,120,105,108,102,102,102,102,105,102,108,102,105,99,96,96,93,93,93,99,105,99,102,93,93,93,90},
{111,111,114,129,138,150,156,159,162,162,162,162,159,159,156,159,162,147,144,126,114,93,72,54,27,21,24,36,42,36,30,27,21,24,27,33,36,39,45,42,45,42,39,42,54,54,60,60,63,60,69,69,72,78,75,72,69,66,72,78,87,84,81,81,81,90,93,99,96,93,90,81,87,93,93,96,102,105,102,93,96,99,99,96,93,93,99,96,102,102,105,102,96,102,102,99,96,96,90,87,87,90,93,93,96,96,96,93,99,102,108,105,105,90,96,84,102,99,108,102,96,96,90,93,99,102,111,105,105,99,105,105,96,108,108,99,96,99,96,102,99,99,96,93,102,105,102,102,96,93,99,90},
{114,114,114,129,135,144,144,147,144,144,147,150,159,156,156,159,159,147,129,105,90,75,57,48,30,24,24,27,33,33,27,24,21,27,27,30,33,39,48,42,48,45,42,45,57,54,60,60,69,60,72,69,72,72,72,72,75,75,78,81,87,84,87,84,87,96,96,102,96,96,96,87,93,99,102,102,108,111,108,96,96,96,93,96,93,96,102,96,105,102,99,99,99,99,105,105,102,102,96,99,96,99,102,99,99,96,99,99,105,105,111,108,105,99,96,90,102,99,108,102,99,96,90,96,96,102,108,105,102,105,108,111,102,111,114,105,102,108,102,111,102,102,96,96,102,105,108,105,108,105,108,105},
{114,120,120,126,129,138,141,147,144,141,138,138,138,144,150,159,150,138,111,81,51,48,42,45,30,24,21,21,27,30,24,24,21,24,21,27,39,42,45,39,45,48,45,48,60,54,60,57,66,57,69,69,75,69,72,69,78,78,81,81,84,81,90,87,93,96,96,102,96,99,99,90,99,99,102,111,111,108,114,96,96,93,87,93,96,102,105,96,102,99,96,96,105,96,105,111,108,108,105,105,102,108,105,102,99,96,102,102,111,111,111,105,96,105,96,102,105,105,108,102,96,93,87,96,96,102,108,108,99,108,102,108,111,105,108,108,102,108,99,108,102,105,99,96,99,102,108,105,111,114,114,114},
{120,114,120,135,144,147,147,138,156,138,135,135,132,150,147,132,141,114,84,54,21,21,21,24,30,30,24,24,24,21,21,15,24,21,30,36,45,48,48,30,45,45,51,48,51,51,54,45,60,66,72,75,69,75,78,78,96,93,87,90,84,84,90,72,96,93,99,105,102,111,111,90,108,96,105,120,108,114,123,96,99,96,96,99,102,111,105,90,96,96,99,96,105,105,108,102,120,105,111,114,111,105,114,102,114,108,108,108,114,114,105,96,105,102,108,108,111,108,114,96,102,96,105,105,105,111,111,96,105,102,111,102,108,105,105,96,105,105,99,99,105,108,105,96,111,102,114,114,108,108,105,99},
{111,114,123,135,141,141,147,150,156,150,147,144,147,156,150,132,132,99,75,54,27,21,21,27,27,30,36,33,27,27,27,30,36,39,39,48,48,54,51,48,57,54,60,57,54,54,60,57,66,75,81,78,78,84,84,93,99,99,96,99,93,96,99,90,105,99,108,111,108,120,108,99,105,96,108,114,114,126,114,108,105,108,108,105,108,111,114,102,105,108,108,105,111,111,111,105,123,111,114,120,114,114,120,111,114,108,114,111,111,114,114,108,111,111,108,105,114,114,108,105,108,102,111,108,111,120,114,105,105,102,111,111,111,108,108,114,108,111,111,111,114,120,114,114,120,114,120,114,120,108,111,105},
{108,114,120,126,135,135,138,150,147,156,156,150,156,150,147,126,114,87,63,45,27,24,21,24,24,30,36,33,33,33,33,42,42,45,48,51,45,48,45,54,60,60,63,66,60,66,69,66,72,78,84,84,84,90,90,102,99,96,96,96,96,96,105,96,105,99,105,108,108,114,99,105,102,99,108,111,120,126,114,114,108,120,114,108,108,108,120,108,111,114,111,111,111,111,111,108,120,111,120,120,114,120,120,120,114,111,120,114,114,114,120,123,111,120,111,111,114,123,111,105,108,108,114,114,123,126,129,111,108,111,111,120,114,114,114,120,105,108,120,120,120,126,126,123,123,123,120,114,120,108,111,108},
{105,111,114,120,129,132,129,135,138,144,144,150,150,141,144,108,99,75,51,33,21,21,24,18,24,24,24,30,36,36,36,36,39,39,45,39,42,42,39,57,60,63,60,66,63,69,66,69,72,78,84,84,87,90,90,102,96,96,93,96,96,96,108,99,108,102,105,108,108,111,102,105,105,105,108,108,114,120,120,114,120,123,114,108,105,105,114,111,114,120,114,120,111,114,114,108,114,108,114,120,111,120,120,114,123,120,123,120,123,120,114,126,111,123,120,120,111,123,114,111,111,111,114,114,126,126,129,111,111,120,114,111,120,120,123,120,108,108,120,114,111,120,120,120,126,120,123,120,111,108,105,108},
{108,111,120,120,123,132,126,129,135,138,147,156,150,144,144,99,81,60,39,24,21,21,21,18,21,21,24,33,39,39,42,33,36,39,42,42,54,57,57,57,60,60,60,69,69,75,72,72,75,81,84,87,90,90,93,96,99,99,90,93,96,93,108,96,111,108,108,108,105,108,108,105,108,111,108,111,114,114,126,114,129,126,120,111,105,108,114,114,114,123,123,126,120,123,123,111,120,111,111,123,114,120,123,120,126,126,123,120,126,123,114,126,114,126,120,123,111,120,114,114,114,114,120,120,126,126,129,111,114,120,120,111,126,120,123,120,120,120,114,114,114,120,120,123,129,126,129,126,111,111,99,108},
{111,114,126,126,123,132,132,138,144,144,159,159,150,150,138,87,60,39,24,21,27,21,15,21,21,24,36,42,45,48,48,39,42,51,48,60,72,81,81,63,60,63,69,78,78,84,81,78,81,87,90,93,96,93,99,93,102,99,93,99,96,96,108,99,120,114,111,111,108,108,114,111,114,114,114,120,120,120,126,120,132,126,120,120,111,114,114,120,120,126,126,129,126,129,132,120,126,120,120,129,126,129,129,129,129,126,123,120,126,123,123,120,120,129,120,126,120,123,123,123,123,123,126,126,129,132,132,120,120,111,126,120,129,120,126,114,126,126,120,120,123,126,126,126,129,132,132,129,114,111,99,111},
{114,120,129,129,126,132,135,144,150,150,156,156,144,138,120,72,39,24,18,21,33,24,18,21,21,27,39,45,51,51,51,45,48,57,54,66,75,78,81,75,72,69,78,81,81,81,75,78,81,90,93,96,99,93,99,99,105,102,105,108,99,102,111,105,114,114,111,114,114,120,120,114,120,120,120,123,123,123,123,129,129,129,126,123,120,120,120,123,123,126,132,132,132,135,141,123,126,123,123,132,126,135,132,135,132,129,129,123,129,123,126,120,120,126,123,132,129,135,132,135,138,129,135,129,129,132,129,123,123,120,129,129,129,126,129,126,126,126,123,120,123,126,123,129,126,132,129,126,120,120,111,123},
{120,126,126,126,132,132,135,144,150,147,144,147,135,114,93,54,27,21,15,18,30,27,24,21,24,27,30,42,51,51,54,48,48,51,54,57,66,60,69,75,75,72,81,78,84,81,72,75,78,90,93,93,96,93,93,96,96,96,105,108,96,99,105,99,102,102,105,108,114,120,111,111,123,120,120,126,120,120,123,135,126,132,132,126,126,126,123,120,120,120,126,123,129,129,138,123,123,120,123,129,126,135,129,135,135,135,132,129,132,126,126,129,120,123,129,135,132,138,138,135,141,132,135,132,132,135,132,123,129,129,132,132,129,135,129,129,120,123,126,126,126,132,129,132,129,129,126,129,123,132,126,132},
{126,123,132,135,141,150,147,147,150,150,144,129,102,81,66,21,24,15,21,24,27,30,21,18,27,36,39,45,42,42,45,36,51,42,57,60,63,63,63,63,75,78,81,81,84,84,81,72,84,93,99,90,99,90,96,87,96,99,108,108,108,102,102,96,105,111,108,114,120,114,114,105,123,120,129,129,123,126,126,126,129,132,132,129,132,132,135,120,138,123,126,120,120,114,123,111,120,129,132,132,129,129,132,120,132,129,138,132,132,129,123,108,126,114,132,132,126,126,120,114,126,132,138,135,138,135,135,120,132,114,126,135,129,135,132,120,129,120,135,129,129,129,123,114,129,123,135,126,129,126,129,126},
{129,126,138,144,156,150,156,156,156,150,147,135,105,72,48,21,18,15,24,21,24,30,21,24,33,39,42,48,48,48,54,51,57,48,54,54,63,63,72,78,84,81,84,81,90,93,93,93,99,105,105,99,102,96,96,99,108,111,114,111,111,108,114,105,108,114,120,120,120,120,120,123,126,132,135,132,132,132,126,129,129,132,135,135,138,138,138,132,144,135,138,129,114,120,129,126,132,138,138,135,132,132,132,132,138,132,144,138,132,132,129,132,135,132,129,126,126,123,126,126,129,129,135,135,138,138,138,135,132,123,123,132,135,135,135,135,138,126,138,135,138,132,129,129,132,126,132,129,132,129,129,126},
{129,132,138,144,150,150,156,156,156,156,147,132,96,57,30,21,15,18,24,21,24,27,21,24,33,42,48,51,54,54,57,57,57,54,54,51,63,60,75,78,81,78,78,78,90,93,93,93,96,102,105,108,114,111,114,108,108,111,108,111,108,111,114,114,111,120,123,120,114,120,120,123,120,132,135,135,138,129,129,126,129,135,138,138,138,141,141,135,144,141,138,132,123,129,129,135,138,141,141,138,132,132,132,135,141,135,144,141,135,135,132,135,132,129,120,123,123,129,138,132,135,126,129,135,141,138,144,135,126,126,120,126,135,138,144,138,138,129,132,126,132,129,132,138,135,135,132,129,129,132,135,129},
{135,135,138,144,147,156,150,150,156,150,141,114,81,48,27,15,18,21,21,21,30,24,27,21,27,36,45,54,57,57,57,60,60,63,60,54,66,57,72,75,78,75,78,84,93,96,93,99,96,96,99,105,108,108,108,114,111,114,102,114,108,114,114,120,120,123,123,114,114,120,114,120,123,132,132,135,135,123,126,123,129,135,135,135,135,138,141,138,141,138,135,132,132,135,129,138,138,141,138,135,135,135,135,129,135,135,138,138,135,135,132,132,129,129,114,120,120,126,135,132,138,129,132,138,141,138,144,132,132,126,123,126,129,132,138,138,135,132,129,123,129,126,135,144,138,144,132,132,129,132,138,132},
{138,135,138,147,150,159,150,147,150,150,138,111,78,45,24,9,15,21,21,27,33,27,33,27,30,36,42,51,57,60,63,63,69,66,69,63,69,66,72,72,78,78,84,87,99,99,96,93,90,90,93,102,105,111,111,114,120,120,111,120,111,120,126,123,129,126,123,120,120,120,114,123,129,135,138,138,132,129,129,129,129,132,135,138,138,138,138,144,141,135,144,135,138,138,135,141,141,141,138,138,138,138,138,126,132,138,138,141,138,135,129,132,129,141,123,126,126,120,132,129,138,135,138,141,141,138,144,132,138,126,129,138,135,135,135,141,135,129,123,123,132,129,138,144,138,141,132,138,135,135,138,135},
{132,135,135,144,147,147,150,150,150,147,138,120,84,42,18,9,12,21,27,27,33,30,39,36,39,42,48,57,63,60,63,60,69,69,78,72,72,69,69,75,78,81,84,87,96,96,96,96,96,93,99,102,105,111,114,120,123,114,120,114,123,120,126,126,135,132,126,126,126,123,123,126,129,138,141,138,135,138,138,132,135,135,135,135,138,138,141,141,141,138,147,138,144,144,144,147,147,141,138,138,135,138,138,135,135,141,144,147,141,138,135,132,135,141,135,138,135,129,135,129,138,138,144,141,141,135,141,132,141,141,141,150,147,141,144,141,135,135,129,126,132,135,144,138,135,138,135,138,135,138,138,138},
{132,135,132,138,138,144,150,150,147,141,132,108,75,36,12,9,9,21,33,27,33,36,36,42,45,45,54,63,66,63,60,60,69,69,81,75,78,72,69,81,87,87,87,87,96,96,96,99,102,96,105,99,102,108,120,132,132,120,123,120,129,123,129,129,135,138,135,132,132,129,126,129,132,135,138,135,132,138,135,138,138,141,138,135,132,138,141,138,141,141,144,141,150,150,147,150,147,141,138,135,135,135,138,138,138,144,144,150,141,138,138,135,144,135,141,138,135,138,135,135,138,138,141,141,141,138,138,144,147,156,147,150,150,141,150,144,144,141,135,132,135,141,150,135,138,138,135,132,129,135,135,138},
{141,135,132,144,144,156,147,147,147,138,120,90,57,27,12,6,15,24,33,27,33,39,36,42,42,42,48,60,63,63,63,69,72,69,78,75,81,78,78,81,90,90,90,87,93,93,93,102,105,99,108,96,99,102,111,135,132,129,120,123,120,126,129,132,132,138,141,135,132,132,129,126,129,129,132,135,138,135,138,144,141,138,135,135,135,135,138,147,141,141,144,150,150,147,147,150,147,141,138,135,135,138,138,135,135,144,138,144,144,144,138,135,141,129,135,132,135,138,138,138,138,135,138,138,141,141,138,144,144,147,147,150,156,147,150,144,141,138,132,132,135,138,141,144,144,135,135,129,126,129,126,129},
{141,135,138,159,147,159,159,138,132,111,81,48,21,9,3,3,15,30,27,27,39,42,45,30,39,42,42,54,63,60,69,54,75,60,78,75,84,93,90,87,96,96,102,93,96,99,99,90,111,105,111,102,105,105,111,108,126,111,120,120,120,123,129,120,135,132,138,141,135,141,141,126,138,126,138,132,135,138,141,144,147,144,150,141,144,150,144,138,150,141,159,150,147,150,147,141,138,135,138,135,135,144,138,135,150,150,150,156,147,147,147,144,156,141,150,144,150,144,150,132,144,135,141,141,141,141,132,135,144,150,150,150,159,165,159,138,144,138,144,141,144,147,144,126,144,135,135,141,129,138,135,123},
{147,147,147,162,150,156,162,144,135,114,84,48,21,9,6,18,21,27,24,24,39,45,42,45,45,42,51,63,63,66,69,60,78,69,81,78,93,93,96,102,99,102,108,99,108,105,108,105,111,111,111,111,114,111,120,114,132,126,132,129,129,129,129,132,135,135,141,138,138,147,144,138,144,138,144,138,138,144,141,147,156,150,156,156,156,156,147,144,147,150,159,150,156,147,147,147,147,150,150,150,156,147,150,147,156,150,150,147,147,150,150,159,162,150,156,147,150,147,150,144,141,144,147,144,147,141,141,135,141,147,147,147,156,162,156,147,150,147,144,144,150,156,156,150,150,150,141,144,144,144,144,141},
{147,150,150,159,150,150,162,144,135,111,81,48,21,12,12,24,24,27,24,27,39,48,45,48,54,54,57,63,66,75,75,69,78,78,81,78,93,84,93,93,96,105,114,102,111,111,114,108,108,120,111,114,114,120,129,123,135,132,141,135,135,132,132,135,135,138,138,138,138,144,141,138,138,138,141,138,138,144,141,144,150,150,150,159,150,150,144,147,147,156,156,150,159,147,150,150,147,156,156,150,159,156,156,159,150,156,150,147,156,159,156,168,162,159,156,150,150,150,150,147,141,147,150,144,147,144,141,138,138,141,147,147,150,156,156,147,147,156,147,150,156,156,156,156,150,162,150,150,156,150,150,147},
{141,147,147,156,156,150,159,141,129,105,75,42,21,18,18,21,21,24,24,30,42,48,48,48,51,60,60,60,72,78,81,72,78,81,81,78,93,84,90,93,105,105,114,108,108,108,111,102,102,111,120,123,123,126,129,126,135,132,138,129,132,135,135,138,138,144,141,141,141,138,144,141,141,141,141,144,141,147,144,150,156,159,150,162,156,156,147,144,150,150,156,156,156,159,156,156,147,147,156,150,156,165,162,162,150,156,150,147,159,162,162,162,150,159,150,150,156,156,156,144,147,141,150,144,147,156,141,144,141,141,147,150,150,150,156,144,144,150,147,150,150,156,150,144,150,150,147,156,147,150,150,144},
{141,147,150,159,165,156,159,135,123,96,66,36,21,18,21,21,21,21,24,33,48,51,42,51,45,54,60,63,69,75,78,69,75,78,84,87,96,93,99,96,105,99,105,102,105,105,108,99,105,114,129,129,129,132,132,129,135,132,135,126,135,138,138,138,144,150,147,144,144,141,150,150,150,150,144,156,144,150,147,156,162,165,159,165,159,159,159,144,156,147,156,156,147,162,156,156,156,147,147,150,156,165,168,165,156,150,147,150,159,162,165,156,144,156,147,156,156,159,156,144,159,141,156,156,150,162,150,150,144,144,150,156,156,156,159,147,150,150,147,150,147,159,150,147,147,141,141,147,144,150,144,150},
{150,159,165,165,171,165,162,132,120,90,57,30,18,18,21,24,21,21,24,30,42,48,45,45,48,54,60,60,63,72,72,69,78,78,87,90,96,96,99,99,102,105,111,105,114,108,120,105,120,123,129,126,126,132,138,132,141,135,138,132,141,141,144,141,141,150,150,147,147,147,150,150,150,150,147,159,150,150,156,159,162,165,159,159,159,159,162,150,156,156,156,156,147,159,156,156,150,150,150,150,156,159,162,165,162,156,156,159,159,159,162,156,147,156,150,159,159,162,156,150,156,144,159,162,159,162,162,156,156,150,156,159,159,159,162,150,156,150,150,156,150,156,150,147,141,147,147,150,150,156,150,156},
{156,159,168,165,171,165,159,138,123,93,57,27,18,18,21,21,27,27,30,30,36,48,48,39,51,57,60,57,60,69,69,69,75,78,84,87,90,90,93,96,102,105,108,99,105,102,111,114,120,120,120,129,132,132,135,135,144,141,144,138,144,138,147,141,138,144,147,147,156,156,150,150,150,156,156,159,162,162,162,162,162,168,165,162,168,162,162,156,156,159,150,156,150,159,159,165,156,162,162,156,159,159,156,162,162,162,165,168,162,159,162,159,159,156,156,162,162,162,159,156,147,147,159,159,159,159,162,159,162,159,159,162,159,159,165,150,159,159,159,156,156,150,147,150,141,159,159,159,162,162,162,156},
{147,150,165,159,162,159,156,147,129,96,57,27,15,18,21,24,24,24,30,33,36,48,45,42,45,54,60,63,66,66,72,69,75,81,84,87,90,87,93,90,102,90,99,99,105,105,120,114,120,129,114,135,135,135,138,138,147,141,141,138,141,135,144,144,144,141,144,156,159,159,159,156,150,156,156,156,162,162,159,162,162,165,171,165,171,165,159,156,156,159,156,159,156,165,165,168,168,165,165,162,156,156,150,162,162,162,168,171,165,165,165,162,162,150,156,162,162,162,162,156,147,150,159,156,162,162,159,159,165,162,159,162,162,162,168,162,162,162,162,156,162,150,156,165,156,159,156,165,165,162,162,159},
{150,150,150,156,156,156,150,138,123,87,48,15,3,12,21,18,27,21,36,33,45,48,54,39,48,54,60,66,66,66,72,63,81,84,93,93,87,93,93,81,99,96,102,99,111,114,114,111,132,129,132,126,138,135,138,129,141,141,141,141,141,150,150,135,144,144,156,156,159,159,156,147,162,156,156,156,162,159,156,147,168,162,174,168,168,165,162,144,156,150,162,159,165,171,171,162,183,171,174,162,156,156,162,144,162,156,156,159,168,171,168,156,162,159,159,165,165,162,165,150,162,159,165,171,171,177,171,156,168,159,168,162,162,171,171,159,162,159,156,162,162,162,165,165,165,174,171,165,162,156,156,147},
{150,156,156,150,150,156,147,144,129,96,57,21,6,6,15,24,27,33,42,42,51,51,51,54,54,63,63,72,69,75,69,75,87,90,96,96,93,96,96,96,111,111,111,108,123,123,126,123,132,132,135,132,138,135,141,144,147,147,147,150,150,156,156,144,144,147,156,156,159,159,162,162,168,159,162,162,162,159,162,159,174,168,177,171,171,168,165,162,165,162,168,165,162,165,168,174,186,183,174,162,159,159,159,159,159,159,159,165,165,174,177,177,174,168,165,162,162,162,165,162,168,165,171,177,177,183,174,174,177,171,171,162,162,162,165,165,162,162,156,159,162,171,171,171,168,171,174,171,171,162,156,156},
{156,159,159,150,150,156,144,150,135,99,57,21,3,0,9,27,24,33,39,45,54,57,60,63,57,66,69,75,72,78,69,81,90,99,105,108,105,105,105,108,120,123,120,114,129,126,132,132,132,138,135,138,141,138,147,150,144,144,147,150,156,150,147,150,147,156,150,156,156,156,162,162,165,159,159,162,162,159,165,165,174,171,174,174,171,171,168,171,171,168,171,165,165,168,171,171,174,183,171,165,165,168,168,168,159,165,165,174,165,174,183,177,174,171,171,165,165,165,162,171,174,171,177,183,183,189,177,183,177,177,171,165,171,162,165,171,165,162,156,156,162,171,171,171,168,174,174,171,171,165,162,147},
{162,162,159,156,156,156,144,150,129,90,48,15,0,3,12,24,24,30,30,45,54,60,66,60,60,63,72,72,75,75,72,87,93,108,114,120,120,111,111,111,123,123,123,120,129,126,132,138,135,141,135,138,141,138,147,150,141,144,144,150,156,147,150,147,150,162,156,159,156,156,156,156,165,162,159,165,168,165,168,162,171,171,171,177,171,174,171,165,168,168,165,165,171,174,174,174,174,177,174,165,168,171,171,168,165,174,168,177,171,171,174,174,171,168,168,168,168,171,174,171,174,174,177,177,183,189,183,174,174,174,171,171,174,174,174,171,168,168,159,159,162,168,165,162,165,177,177,168,165,165,168,144},
{162,162,159,162,159,156,147,141,123,84,45,18,6,12,21,27,30,33,36,51,48,57,57,60,66,66,72,72,75,72,78,90,96,111,114,111,111,108,114,111,123,123,123,123,129,129,132,138,138,144,135,135,141,138,147,156,147,150,150,150,150,147,150,147,156,162,162,165,156,159,147,150,159,162,162,168,168,165,165,156,162,168,165,174,171,177,174,165,168,168,165,168,171,177,183,177,183,174,177,165,171,174,171,168,171,174,171,174,177,168,171,174,177,174,174,177,174,174,177,171,171,174,177,177,186,192,186,174,177,171,174,174,171,183,174,171,171,168,162,165,165,165,165,162,165,174,174,168,168,168,168,150},
{165,162,159,165,165,162,156,135,114,84,51,27,18,18,21,36,39,42,45,48,48,63,60,66,72,75,72,78,75,75,78,90,99,114,120,114,114,114,120,114,129,123,126,129,129,135,132,138,141,144,141,138,141,141,156,162,162,162,159,156,150,150,156,150,159,165,171,171,159,156,150,156,156,165,168,177,168,159,165,150,159,165,165,174,171,177,177,174,171,174,171,174,171,174,177,174,183,171,183,174,177,183,177,174,177,171,171,168,174,168,174,177,186,183,183,186,183,174,177,174,174,174,177,183,186,189,189,174,183,171,174,177,168,183,171,174,174,171,162,168,168,171,174,168,162,168,174,177,183,174,168,156},
{162,162,162,165,165,165,156,135,114,84,51,27,18,18,21,39,39,45,45,48,51,66,69,72,75,75,78,81,81,81,81,96,102,120,129,126,126,126,126,123,129,129,126,129,129,129,132,138,147,147,147,138,144,144,159,168,174,174,168,162,156,159,162,156,159,165,171,168,162,156,156,159,159,171,174,186,174,165,168,159,165,171,171,174,174,174,177,183,174,183,174,177,171,171,177,174,183,174,177,177,183,183,177,183,177,171,177,165,171,171,171,183,183,177,174,177,177,183,186,174,174,174,174,183,186,186,186,177,183,177,177,177,168,177,177,177,174,171,165,168,171,177,177,168,162,168,174,183,186,177,168,162},
{162,162,165,165,168,168,156,141,120,81,42,21,9,15,18,33,39,45,45,48,54,57,72,75,75,72,84,78,90,87,84,96,99,108,114,114,114,120,120,126,126,129,123,126,126,123,129,135,156,156,150,141,150,156,162,162,171,171,168,159,156,162,162,156,162,165,168,162,165,156,156,150,156,168,168,183,174,168,168,165,171,177,177,177,177,174,174,174,174,186,171,174,174,174,174,171,174,174,174,183,183,177,177,183,177,174,183,165,171,177,165,183,177,177,177,177,183,186,186,174,177,174,171,177,183,183,183,183,177,189,177,177,174,174,177,177,171,171,165,165,168,174,171,165,162,171,177,177,183,177,174,168},
{162,156,171,165,168,159,156,132,111,81,54,30,21,18,18,15,33,39,42,48,57,57,66,51,66,75,87,93,90,87,81,87,96,99,108,108,114,111,120,108,126,123,123,132,132,129,126,123,144,138,147,150,159,156,156,150,159,156,159,156,150,162,162,156,162,165,165,159,162,162,165,150,162,168,177,177,177,186,186,174,183,174,183,174,183,183,177,168,183,177,186,186,177,183,183,168,177,177,177,177,183,177,183,168,177,174,174,165,174,177,168,168,177,171,177,186,183,192,189,171,177,171,174,183,183,192,195,177,186,186,177,165,162,168,177,168,174,174,165,171,165,171,183,162,171,177,186,171,186,171,177,165},
{168,162,171,165,168,162,162,138,120,87,57,33,21,21,21,24,36,45,51,54,63,63,63,60,72,81,87,96,90,90,87,96,99,105,114,111,114,114,114,120,126,120,123,129,129,129,135,126,138,138,144,156,147,156,156,150,156,156,156,150,156,162,168,168,171,171,171,162,165,165,162,165,171,168,177,177,183,189,192,186,183,186,174,177,177,177,183,177,183,186,189,186,183,183,177,177,186,183,183,177,183,183,186,174,177,174,174,174,177,177,171,171,174,174,177,189,189,192,195,174,174,174,177,183,189,192,195,186,192,186,186,177,171,177,183,177,177,174,171,174,171,174,177,174,183,183,192,183,177,171,171,165},
{171,171,174,165,171,165,168,144,126,93,63,39,24,21,21,30,36,48,48,54,60,69,72,72,78,81,84,93,87,93,93,96,99,105,114,111,120,120,120,126,129,123,123,129,129,135,141,132,141,138,147,150,150,150,159,150,150,156,150,150,159,168,174,174,174,177,177,165,168,171,168,174,177,174,186,186,189,189,189,189,177,186,174,186,183,189,192,189,186,192,192,186,186,183,177,186,192,192,192,189,189,183,183,183,177,177,177,183,177,177,183,171,171,177,177,186,192,186,195,183,174,183,183,186,192,192,192,189,192,186,189,186,177,189,183,189,183,177,171,174,174,177,174,183,186,183,192,183,177,174,171,171},
{165,174,177,174,174,168,171,144,126,96,63,42,27,24,24,21,30,39,36,45,57,75,84,75,75,78,75,84,84,93,96,93,99,102,108,114,120,114,120,126,132,129,129,132,138,141,138,135,150,141,150,147,156,150,150,150,150,156,156,159,165,171,177,171,171,174,174,168,174,177,177,183,183,177,183,183,186,186,189,189,186,189,183,195,183,195,192,192,195,192,195,192,186,189,177,192,195,192,192,192,192,189,189,186,177,186,177,189,183,177,189,177,174,183,177,183,189,183,189,189,183,189,186,186,189,189,186,177,189,177,186,186,177,189,183,189,183,183,174,177,174,177,177,183,186,183,183,174,174,177,174,174},
{162,174,177,183,177,168,168,144,126,96,66,42,30,24,24,21,27,33,33,51,60,72,84,78,78,78,75,84,87,96,102,102,105,108,111,120,120,120,120,126,135,135,138,138,141,144,135,144,150,156,156,156,150,150,147,147,156,156,165,168,168,174,174,168,174,171,174,171,177,183,183,177,177,177,177,174,183,183,186,183,186,189,192,198,189,195,192,189,195,189,195,195,189,192,186,192,195,192,195,195,195,192,195,192,183,186,177,192,189,186,189,186,186,189,186,189,192,189,192,192,195,192,189,189,183,183,183,171,183,174,183,183,174,186,183,186,189,186,177,177,174,177,186,186,189,192,183,171,171,168,174,177},
{162,174,174,183,174,171,168,147,129,99,66,42,30,24,24,21,33,42,45,57,60,66,75,81,81,84,84,87,96,99,108,111,108,114,120,123,126,129,126,135,135,138,144,141,141,147,141,156,156,156,162,159,150,147,150,150,162,162,171,174,171,177,171,165,171,171,174,177,186,186,189,174,174,177,183,174,183,177,183,177,177,192,195,198,198,192,195,192,195,192,192,198,198,192,195,186,192,189,198,198,204,195,198,192,186,186,183,189,192,192,192,189,195,189,192,195,192,195,195,195,204,192,189,189,177,177,177,171,183,183,186,183,174,189,189,186,192,192,183,183,174,177,192,186,189,195,192,174,171,165,177,183},
{168,174,171,183,174,177,171,162,141,105,72,45,30,24,21,27,39,48,54,57,60,63,72,81,84,84,90,90,102,99,108,111,111,120,120,129,129,135,132,141,138,138,144,141,144,156,147,156,156,150,165,150,156,147,156,162,165,168,174,177,174,174,168,165,168,171,177,177,189,189,189,186,177,174,174,174,186,183,189,186,186,195,195,195,195,189,189,195,195,195,198,204,204,198,198,195,195,189,195,198,198,195,204,192,189,186,189,189,192,195,195,192,195,192,192,195,192,195,195,198,198,195,192,189,186,186,186,186,192,189,189,186,183,192,195,192,192,192,183,183,177,183,186,183,189,189,195,177,177,171,183,177},
{171,174,171,186,174,186,177,171,150,114,78,48,30,21,21,36,39,42,51,51,60,66,75,75,81,81,90,87,99,93,105,114,114,114,114,129,126,129,129,141,141,141,144,141,147,159,147,156,150,156,156,150,150,150,156,165,168,174,177,177,177,171,168,171,171,174,183,174,186,186,177,189,183,177,177,177,192,189,192,186,192,192,189,192,192,195,189,198,198,195,204,204,204,210,198,195,195,192,198,204,204,195,198,195,189,177,192,189,195,195,192,195,198,195,192,195,192,189,192,204,192,195,192,189,192,192,192,195,198,195,192,189,186,192,195,198,192,186,177,183,177,177,174,177,192,186,192,174,183,177,177,174},
{162,168,171,177,174,177,177,165,171,147,111,66,30,21,9,21,27,39,42,45,60,72,72,66,75,84,90,96,96,105,108,105,108,111,111,120,123,129,135,129,132,138,138,138,141,144,150,144,150,156,150,159,159,162,162,168,174,174,183,177,177,174,177,171,183,186,192,183,192,183,177,177,183,177,186,186,189,195,192,186,189,186,189,189,186,189,183,189,192,195,204,210,207,198,195,183,195,204,192,192,195,204,204,186,192,189,189,189,189,195,204,186,204,189,195,192,186,204,186,186,186,192,195,195,195,195,192,189,192,189,192,192,192,198,198,189,192,189,186,186,186,183,189,186,186,189,177,177,186,189,177,171},
{168,171,174,174,171,171,171,174,171,150,114,75,45,21,15,18,24,36,39,45,60,69,72,75,81,84,96,105,108,114,114,108,108,108,111,120,123,129,132,132,141,147,147,144,144,147,156,159,165,168,165,171,165,168,171,171,177,177,183,177,183,177,177,177,186,189,192,186,192,192,189,177,186,177,186,189,189,192,195,192,192,192,192,192,192,192,192,204,204,207,210,213,213,210,207,192,195,204,198,198,195,198,204,192,198,195,195,192,192,198,204,198,213,195,195,192,189,207,195,198,195,195,195,198,198,204,198,192,192,198,195,195,195,189,195,192,195,195,195,195,195,192,195,192,189,192,183,183,189,192,186,174},
{171,171,174,174,171,168,168,171,162,150,111,78,57,21,12,12,21,30,36,48,57,66,69,72,75,81,96,111,120,123,120,120,114,111,111,114,120,120,120,132,141,150,150,144,144,150,159,162,165,171,168,168,162,165,171,177,183,177,183,177,183,177,183,189,195,195,189,192,189,195,192,177,192,177,186,192,192,195,198,198,204,195,198,198,195,204,204,210,210,210,210,213,213,213,213,198,198,204,204,207,195,198,198,195,198,198,204,198,195,204,204,213,216,195,195,195,195,213,207,210,204,204,198,204,204,210,207,198,189,204,198,195,198,186,195,192,195,198,198,198,198,198,198,192,192,189,177,183,189,192,186,177},
{168,168,177,171,174,168,171,174,168,159,126,96,72,39,21,12,21,27,33,45,54,54,60,63,69,75,90,102,111,120,114,111,111,108,111,114,120,123,123,129,138,144,144,141,141,144,150,162,162,165,162,162,156,162,171,183,183,183,183,177,177,183,183,186,198,198,189,204,189,198,189,186,192,183,189,195,195,198,198,204,207,198,204,204,198,207,204,207,207,207,207,210,210,210,210,207,204,207,207,210,204,204,195,192,195,198,204,198,204,210,207,210,210,195,195,198,204,216,213,213,204,207,204,210,207,213,207,204,192,198,198,195,192,189,195,192,192,195,192,195,195,198,204,192,192,189,183,186,189,192,186,183},
{171,168,177,171,174,168,171,168,174,162,141,111,81,57,33,18,21,24,33,42,45,45,51,60,69,78,87,96,102,111,114,108,108,111,111,114,120,123,126,132,135,135,138,138,141,144,144,168,165,165,165,168,165,171,174,177,177,183,177,177,177,183,186,177,192,198,195,210,195,204,192,189,192,189,195,195,198,204,204,198,198,204,204,204,204,204,204,198,204,210,213,213,213,213,210,210,213,216,207,213,213,210,195,192,195,195,204,204,204,210,207,204,207,198,204,207,207,213,216,210,204,210,210,213,213,213,207,207,204,192,198,195,186,195,198,192,192,195,195,192,195,198,198,192,192,192,189,195,195,192,183,186},
{174,174,177,171,168,165,165,162,171,162,150,126,93,72,45,21,18,21,30,42,45,48,51,63,72,81,90,96,102,114,123,114,120,120,114,114,114,120,126,135,132,132,135,144,150,156,156,168,165,171,171,174,171,174,174,177,177,183,177,177,177,186,186,177,195,204,204,210,204,204,192,195,192,195,198,195,204,204,207,198,204,204,207,207,204,207,204,207,210,216,225,225,225,219,219,213,225,225,210,216,225,219,204,198,198,198,204,204,204,207,207,207,207,210,213,213,207,210,213,210,207,210,216,213,216,213,207,210,213,195,198,195,183,195,195,192,192,195,195,195,192,195,192,189,189,189,186,195,192,189,177,183},
{174,174,174,174,168,165,165,168,171,168,162,147,123,90,63,21,18,21,30,39,48,54,60,66,72,75,84,93,99,111,114,120,120,120,120,123,126,132,132,138,135,132,135,147,159,162,165,162,165,174,171,171,171,174,171,177,177,183,177,183,183,186,189,189,198,204,210,204,207,198,192,198,195,198,198,198,210,198,210,204,210,204,210,210,207,213,210,213,216,219,225,225,225,225,225,216,219,225,213,219,225,225,213,207,210,204,204,207,207,210,210,210,210,213,216,213,207,204,210,213,210,207,213,207,210,207,204,210,213,207,207,198,192,195,192,195,192,195,195,192,189,192,189,186,186,186,183,192,189,183,174,183},
{168,171,171,177,171,171,171,171,165,168,159,150,135,90,63,33,21,21,33,39,45,54,57,72,72,69,75,84,93,99,99,114,114,114,123,129,135,141,144,144,138,132,135,144,156,159,159,165,168,177,174,171,171,177,177,177,177,186,183,186,186,189,189,192,198,204,210,195,207,198,198,198,195,204,195,198,213,195,213,213,210,210,210,213,213,216,216,216,216,216,216,216,216,219,225,216,216,216,213,219,219,225,216,210,213,207,207,210,210,213,216,210,210,216,216,210,204,198,213,213,213,204,210,192,204,198,204,207,207,213,210,207,207,195,195,204,198,198,195,189,189,192,192,189,186,186,183,192,189,186,177,186},
{162,162,168,174,168,171,159,162,171,174,171,168,150,129,102,54,36,33,24,36,51,57,63,63,66,72,84,90,96,108,108,102,105,114,129,135,138,144,141,144,144,141,141,138,144,159,168,165,177,174,171,183,177,174,189,165,168,186,186,186,195,192,204,192,192,198,198,207,207,207,207,207,210,216,213,204,207,213,216,204,210,207,219,216,219,219,225,207,225,219,228,228,213,225,228,210,213,216,219,225,216,219,216,210,225,213,213,213,207,213,216,207,219,213,204,198,204,207,213,195,210,198,204,207,198,207,198,198,207,213,210,207,198,198,195,192,192,189,189,186,186,189,192,186,183,177,183,186,183,183,183,171},
{162,171,171,177,174,171,171,171,177,174,174,174,165,147,126,81,48,33,27,42,57,63,66,72,72,75,87,96,105,114,111,105,108,123,132,138,141,144,141,150,156,156,150,144,147,162,174,171,177,171,174,183,177,177,183,183,177,189,192,195,207,207,210,207,198,204,195,204,210,210,213,216,216,219,216,210,210,216,219,213,216,210,216,219,225,231,231,228,234,225,225,225,216,219,225,216,216,216,219,225,219,231,228,216,228,216,216,216,210,216,219,210,219,216,204,204,204,204,213,198,210,204,204,207,204,207,204,198,207,216,213,210,204,204,198,192,195,195,192,189,186,186,189,189,186,183,186,186,183,177,177,177},
{162,177,174,174,171,165,171,174,177,174,174,174,168,156,138,90,60,42,33,36,51,60,69,75,72,75,87,102,111,120,111,108,111,126,135,138,141,141,138,156,159,159,156,150,156,168,177,174,177,168,174,183,177,183,177,192,189,192,198,210,216,216,216,210,198,207,198,210,216,213,216,225,216,219,219,216,213,219,225,219,219,213,216,225,228,234,234,237,231,225,216,225,225,216,228,231,231,228,225,228,225,231,231,219,228,219,219,216,210,216,216,210,216,216,204,204,210,207,213,204,210,204,204,207,204,207,207,204,207,213,213,213,204,198,198,195,198,198,192,195,186,183,183,186,183,183,183,183,177,177,177,183},
{165,177,171,171,165,159,162,165,174,174,174,174,168,159,141,96,72,45,27,21,42,51,60,69,69,75,84,96,108,111,111,111,114,126,129,135,138,138,138,147,147,150,150,150,159,168,174,174,177,171,177,177,177,183,174,189,192,192,207,213,213,216,210,204,195,210,204,213,216,210,216,219,216,216,219,219,213,216,225,213,219,213,219,216,225,231,234,231,225,225,216,225,228,216,231,237,237,234,228,228,225,228,225,231,231,228,228,225,219,225,219,207,210,216,204,207,216,210,216,204,207,198,198,198,198,204,204,204,204,210,210,207,195,195,195,195,195,198,189,198,186,183,183,177,174,174,177,177,174,174,177,177},
{171,174,174,171,162,165,156,156,171,177,174,174,174,171,156,126,90,48,24,21,39,45,48,63,69,78,84,93,99,108,120,111,114,129,132,135,138,135,141,144,144,150,147,150,162,165,174,174,174,177,177,177,177,183,177,183,192,195,207,213,210,210,204,210,204,216,207,207,213,210,216,213,213,219,225,219,216,216,228,210,219,213,228,216,228,225,228,231,231,237,228,228,228,216,234,231,231,237,234,237,234,231,231,231,225,225,225,219,219,225,219,207,210,216,207,210,219,213,216,207,204,198,198,195,198,198,204,204,204,207,204,204,195,195,195,192,192,192,183,192,177,183,177,174,171,171,174,174,174,174,177,177},
{174,174,177,177,168,174,165,162,174,174,174,174,174,174,168,147,114,78,51,36,39,45,48,57,66,78,84,90,99,105,120,111,120,132,135,138,141,138,144,147,147,159,156,156,162,165,177,174,177,183,183,177,177,183,186,183,192,195,207,207,204,204,198,210,207,216,210,210,213,213,219,210,216,219,225,225,219,219,231,216,225,213,234,225,237,228,231,228,234,237,231,228,219,216,231,231,231,237,237,240,237,237,234,234,228,228,228,225,228,231,219,213,213,216,210,213,219,213,213,210,198,204,198,195,198,195,198,204,204,207,204,204,195,192,192,189,183,183,174,177,174,177,171,171,171,171,174,174,174,174,177,174},
{171,174,174,183,174,177,174,171,171,168,168,171,171,171,168,156,138,120,90,69,54,51,48,54,60,72,78,87,96,102,114,108,114,132,135,141,144,138,147,147,147,159,159,159,165,168,177,174,186,186,186,186,183,189,192,192,195,195,198,198,198,198,198,204,204,213,210,213,213,213,216,213,216,216,216,225,225,228,234,231,234,225,240,231,246,237,240,231,234,228,225,225,216,219,228,234,231,237,234,237,237,234,237,234,225,225,228,219,225,228,216,213,216,216,213,213,219,216,210,210,198,204,198,192,195,192,195,198,198,207,204,204,195,192,186,183,174,174,171,168,168,171,165,168,168,171,174,174,171,174,177,168},
{168,174,171,174,174,171,177,171,165,159,168,174,171,171,171,156,147,135,108,93,75,63,39,48,54,63,69,81,93,93,105,108,114,129,129,138,144,138,150,144,144,156,156,159,171,168,177,177,189,186,189,189,186,195,195,198,195,195,192,192,195,198,204,204,204,210,210,213,210,210,207,213,216,213,213,219,228,231,237,237,240,234,240,234,240,249,252,246,246,219,219,225,216,225,225,234,228,231,225,231,234,234,237,240,225,228,228,219,225,225,213,213,213,213,213,216,219,219,210,210,195,204,195,189,192,186,192,192,195,204,198,204,192,189,174,177,171,171,171,165,165,165,159,162,162,165,171,171,171,174,183,168},
{171,177,174,171,168,171,177,165,171,162,171,168,171,174,162,147,141,144,138,129,99,72,45,21,42,57,69,72,78,90,111,102,114,123,123,126,138,147,147,141,147,150,159,159,168,171,186,186,192,192,189,192,192,192,195,186,195,195,195,192,192,207,198,195,213,204,216,195,210,192,198,198,210,210,216,213,219,225,234,219,237,234,231,237,234,240,252,228,246,231,231,228,237,231,228,219,228,225,231,231,228,237,225,213,231,219,228,225,213,219,210,198,216,204,207,216,210,210,204,186,189,189,189,192,186,174,174,168,189,186,192,189,177,171,162,150,159,171,162,156,162,159,156,156,162,168,183,186,186,195,192,189},
{174,177,174,171,168,171,177,174,174,168,174,168,171,171,162,159,150,156,150,141,120,93,66,45,57,63,72,81,90,96,111,114,123,126,123,129,141,150,150,150,156,162,162,168,171,177,189,189,195,195,192,192,195,195,195,198,204,204,204,204,198,210,198,207,216,213,216,207,210,204,207,204,213,216,219,225,225,228,234,228,237,234,234,234,234,237,240,234,237,237,234,237,237,240,228,225,231,228,234,234,231,237,228,213,228,216,225,219,216,225,213,207,216,195,204,210,204,210,210,192,192,189,186,183,177,171,168,168,177,177,183,174,171,162,159,162,162,171,165,162,168,171,165,174,177,177,186,192,195,207,210,210},
{171,174,174,171,171,174,177,177,174,174,174,171,174,168,165,165,159,156,150,147,135,120,99,69,69,57,63,72,84,90,102,111,114,120,123,129,141,147,147,156,162,168,168,177,174,183,186,189,195,195,192,195,195,195,198,198,207,207,213,213,213,219,210,213,216,225,213,216,210,210,210,210,213,225,219,231,228,231,234,234,237,237,237,231,231,237,231,237,231,234,240,237,237,240,225,225,231,231,234,234,231,237,228,216,225,216,219,219,216,225,213,213,213,195,198,198,195,204,207,192,189,186,177,174,171,168,165,162,162,162,165,159,159,159,159,159,165,171,168,174,183,177,177,183,186,186,189,192,198,213,216,213},
{165,168,171,174,171,171,171,174,168,174,171,171,177,168,171,162,156,147,147,150,144,135,123,96,84,60,51,57,69,81,96,99,105,111,114,123,135,141,138,150,159,165,168,174,174,177,183,186,192,192,192,192,195,195,198,198,207,207,216,219,219,228,225,216,219,225,216,219,213,216,216,216,213,228,216,234,225,237,234,237,234,237,240,231,231,237,225,237,234,237,249,246,240,246,237,228,231,231,231,228,231,231,228,219,225,216,216,216,213,213,207,207,204,195,195,189,186,192,189,183,174,174,171,165,165,165,162,156,156,147,156,147,150,162,159,162,162,171,177,177,189,192,177,189,192,195,198,210,213,219,225,213},
{165,165,168,171,171,171,168,171,168,174,171,174,183,171,177,156,150,150,159,162,159,147,138,129,111,78,54,51,57,75,96,99,108,111,111,114,126,138,141,144,156,162,171,174,177,174,177,186,189,189,192,195,195,195,204,198,210,210,219,219,216,225,225,216,225,219,225,216,225,216,225,225,216,228,216,234,228,240,234,237,234,237,240,234,234,237,225,234,237,246,240,255,237,246,246,234,231,231,228,225,228,228,228,219,216,216,216,213,207,204,195,195,192,195,198,183,177,177,165,165,162,165,165,159,159,162,159,150,156,150,150,147,159,168,165,177,183,177,177,189,195,195,192,195,204,210,216,228,231,228,234,219},
{165,162,165,171,174,174,174,174,171,177,174,183,177,174,177,162,165,162,168,171,171,165,162,159,135,96,60,48,48,69,93,96,105,114,120,120,126,141,150,147,159,162,171,174,183,177,183,186,189,189,192,198,198,204,210,204,213,213,219,219,213,219,225,216,228,216,231,219,237,225,231,228,225,228,225,234,234,240,234,237,237,237,237,234,234,237,231,231,240,240,246,252,237,237,249,240,234,237,228,225,228,225,225,216,213,216,213,210,204,192,189,186,186,195,195,177,174,168,156,150,150,159,159,156,156,156,150,147,156,162,162,162,183,183,186,204,198,198,204,198,207,213,204,207,210,216,219,231,234,231,234,231},
{162,156,159,168,174,177,177,174,177,174,174,183,171,174,177,171,174,171,174,174,177,171,174,168,150,120,84,69,57,60,72,81,93,105,111,111,123,135,144,147,156,162,168,174,177,174,174,183,186,189,195,198,204,207,213,210,216,213,219,225,216,219,225,216,228,219,234,228,240,231,234,231,225,225,228,228,234,237,237,237,246,237,234,237,237,237,237,240,249,240,252,246,240,237,246,240,234,240,231,225,228,216,216,213,207,210,204,192,186,174,174,174,174,183,174,168,165,156,150,144,147,150,156,156,156,150,156,156,162,174,177,186,198,195,204,210,213,213,216,219,216,219,216,225,228,225,219,231,234,228,234,234},
{159,156,159,168,174,174,168,171,174,171,174,183,168,177,174,171,177,177,183,177,177,171,177,171,165,150,126,108,81,63,57,72,84,93,96,96,105,123,132,147,150,159,162,171,168,168,168,177,183,186,192,198,198,207,213,216,219,213,219,225,216,225,225,213,225,225,231,234,240,234,234,234,228,216,219,219,234,237,237,237,249,237,234,237,234,234,240,249,246,246,252,240,237,240,234,237,231,240,231,219,225,210,210,204,195,195,186,171,162,150,156,162,162,162,150,156,147,138,150,138,144,147,150,156,156,156,162,165,177,186,195,204,207,198,204,213,213,213,216,216,225,234,228,234,234,228,219,228,228,219,225,225},
{150,156,156,162,168,162,168,150,165,162,174,177,168,177,174,171,183,174,189,183,174,171,174,168,171,165,162,135,114,90,69,57,63,69,81,75,93,108,129,126,144,147,141,159,150,162,168,168,171,183,195,210,213,210,207,195,213,204,219,219,219,228,231,213,219,228,228,234,237,237,231,225,228,228,228,225,225,228,234,237,240,240,240,234,234,237,240,237,246,240,234,234,237,237,231,213,213,213,213,198,192,189,183,168,168,171,165,150,147,138,132,123,135,141,135,132,135,129,138,132,138,144,156,162,171,177,186,177,192,198,195,198,207,210,210,195,210,213,213,225,219,225,225,213,225,225,219,213,216,216,216,213},
{159,159,159,162,165,162,162,159,165,162,171,174,171,177,177,177,177,177,183,177,183,168,183,177,183,177,174,159,138,123,102,84,78,66,72,75,96,105,120,111,135,138,141,150,147,156,165,171,171,183,192,207,213,216,213,198,216,207,225,225,228,234,237,231,231,234,231,234,234,237,237,231,231,231,225,219,225,231,237,240,246,249,246,240,240,240,246,234,237,234,228,228,228,228,219,213,204,195,192,186,183,174,165,144,147,150,147,144,138,129,123,114,129,132,132,132,141,141,144,147,156,162,168,177,186,192,198,207,213,213,210,213,216,213,210,210,213,216,213,216,213,219,216,216,225,225,216,216,216,219,213,219},
{162,162,162,162,162,165,162,165,162,162,162,168,174,177,183,174,174,174,177,174,186,168,177,177,183,177,174,168,150,144,126,105,105,90,84,78,90,93,108,99,114,120,126,135,138,144,159,168,171,174,186,195,204,207,207,198,216,204,216,219,225,231,234,234,231,231,228,228,228,234,237,231,228,225,216,213,216,228,234,234,237,237,237,234,234,237,237,219,225,219,213,210,210,210,204,195,183,171,171,171,168,159,144,135,135,135,135,138,135,123,123,120,132,135,138,138,150,159,162,174,177,186,192,198,207,210,216,213,213,213,216,225,225,225,225,213,213,213,213,213,213,225,213,216,216,216,213,213,219,219,213,216},
{162,159,162,162,165,171,168,168,162,165,159,162,177,177,186,168,174,162,171,171,171,171,171,171,177,174,174,171,156,156,132,123,129,123,111,96,90,90,93,84,96,99,108,111,123,129,141,156,159,165,171,183,186,186,189,204,213,198,213,213,216,219,225,225,225,228,225,225,219,228,231,216,213,210,204,198,204,207,213,213,213,216,216,216,216,216,216,207,204,204,195,192,189,186,183,171,162,150,150,150,150,138,129,135,132,132,135,135,132,126,129,135,150,156,162,159,165,177,186,195,198,207,210,216,219,225,228,219,216,213,213,216,219,219,228,216,213,213,210,213,213,216,210,207,210,213,210,210,216,216,216,207},
{168,162,162,165,165,168,171,171,165,165,159,162,177,177,186,171,174,156,159,159,165,171,174,174,177,174,174,171,159,162,144,144,141,141,138,132,120,102,87,78,84,87,96,99,105,105,114,129,138,147,159,168,171,171,171,195,207,192,204,204,207,207,213,207,216,219,213,213,213,216,213,204,204,195,192,192,192,192,189,189,189,189,192,192,195,195,195,183,177,174,174,168,165,162,162,144,141,135,129,129,129,126,123,132,126,132,135,135,138,141,147,159,174,183,186,183,189,198,204,213,213,216,219,225,228,231,231,219,213,204,198,204,204,210,219,210,210,213,207,216,216,213,210,198,204,213,210,210,213,210,213,195},
{171,165,162,162,162,162,168,171,168,165,162,165,174,177,183,171,168,165,159,156,171,165,174,177,177,174,171,168,168,168,162,150,147,159,150,150,141,129,105,93,87,90,93,93,99,96,99,102,114,132,144,156,159,162,162,177,192,186,192,189,192,192,204,195,204,210,204,204,204,204,195,189,186,183,177,174,174,171,168,165,162,162,165,168,171,171,168,159,156,150,150,147,141,141,141,111,120,123,123,123,123,126,129,132,129,135,138,141,150,165,171,183,192,195,204,198,210,216,213,216,216,216,219,225,225,225,225,210,198,189,186,192,195,204,213,204,207,210,204,213,213,210,213,204,207,216,213,210,210,204,210,192},
{168,168,162,159,162,162,165,168,171,165,171,171,168,177,174,171,174,174,171,165,177,168,174,183,177,177,174,171,177,177,174,150,156,162,150,156,147,144,132,123,105,102,99,99,99,96,93,93,102,114,126,132,135,141,144,162,174,171,177,171,171,171,183,177,177,186,177,183,183,183,177,165,159,150,144,144,144,141,141,132,129,129,132,135,138,138,135,138,135,132,135,132,126,126,129,102,111,123,129,132,135,138,144,144,144,147,150,159,174,183,192,195,198,204,204,198,213,216,213,216,216,213,216,216,216,216,216,216,210,198,192,195,198,204,213,213,210,213,204,207,204,204,204,204,204,216,213,210,210,195,198,195},
{162,168,162,159,162,165,171,165,174,165,174,174,165,174,171,174,189,174,183,174,177,177,177,177,174,177,174,171,183,174,174,156,156,156,141,156,150,147,138,141,120,120,111,111,105,99,90,90,96,105,108,111,114,123,126,138,156,150,156,144,144,141,156,147,144,144,144,150,150,156,159,141,132,123,111,111,114,114,114,102,99,99,102,108,111,108,105,129,123,123,126,123,114,114,123,120,126,132,138,144,144,147,156,159,162,162,159,171,186,189,195,198,198,204,198,192,210,210,213,216,213,210,210,213,213,210,210,216,216,207,198,195,195,204,213,213,210,213,204,198,198,204,204,198,198,210,210,210,210,195,195,198},
{165,168,162,159,150,159,168,162,159,165,168,171,168,174,168,171,174,177,177,174,174,177,174,168,174,171,168,183,183,171,186,156,156,156,156,159,156,150,144,138,135,144,147,147,132,120,99,87,96,108,111,105,102,99,102,90,111,120,120,108,111,108,111,111,120,120,108,108,111,105,102,90,93,93,93,93,93,93,87,66,72,78,75,81,93,96,93,108,120,120,126,129,126,132,135,141,144,144,147,159,162,168,174,174,183,183,177,183,186,189,198,186,204,204,204,198,198,210,198,207,195,198,198,198,207,204,198,195,198,195,204,198,198,204,207,192,198,198,198,207,204,198,210,189,195,198,195,204,204,189,204,186},
{162,168,162,162,156,162,165,162,159,165,165,168,165,168,165,177,174,174,174,174,174,174,168,171,177,177,174,189,189,183,192,171,168,165,162,162,162,162,162,150,156,150,147,150,141,141,135,114,123,129,129,126,120,114,111,93,105,108,108,96,102,105,108,108,111,111,102,102,102,93,90,84,87,93,93,90,90,90,87,72,78,81,81,90,102,111,111,129,135,138,141,147,144,150,156,150,159,162,165,174,174,177,186,189,192,189,186,183,186,189,195,195,204,207,204,198,204,204,198,207,195,198,195,198,204,204,198,195,195,195,198,204,198,204,204,195,195,198,204,204,204,204,210,198,198,204,195,204,207,189,198,192},
{156,165,162,162,159,162,162,156,150,159,159,162,159,165,162,177,171,168,174,177,177,174,168,174,183,183,183,192,195,189,198,186,183,174,168,168,171,171,171,165,168,156,147,147,141,156,156,147,147,147,144,141,138,132,129,108,111,114,108,99,105,114,120,108,111,111,105,102,99,90,87,78,87,96,99,99,96,96,96,90,93,96,96,105,120,129,132,147,150,147,156,159,156,159,162,156,165,171,174,186,183,177,183,192,195,192,189,186,189,192,195,204,204,204,195,195,198,198,195,204,195,198,189,192,195,198,198,198,195,195,195,204,195,204,198,198,195,198,204,198,198,204,207,198,198,204,198,204,204,192,195,195},
{147,156,159,159,159,165,159,147,144,150,150,156,156,159,159,168,165,165,174,177,174,177,174,174,177,183,183,189,192,189,195,183,183,177,174,174,174,174,177,177,174,168,165,159,156,165,162,162,156,150,147,147,147,141,135,129,132,132,126,114,123,129,132,114,120,123,111,111,111,102,99,93,99,111,120,120,120,120,114,114,114,120,123,129,138,144,144,156,156,150,156,159,156,159,162,156,168,174,183,189,186,174,177,189,192,192,186,186,192,195,198,198,198,198,192,189,189,192,195,198,192,198,189,192,189,195,198,204,198,195,198,204,195,198,195,195,195,195,195,195,192,195,198,195,195,204,195,198,195,192,195,189},
{147,150,156,159,162,168,165,150,147,150,150,156,156,162,159,162,162,168,174,171,171,177,183,174,174,177,177,183,186,183,186,177,177,177,177,177,183,183,183,183,177,174,177,171,174,174,171,165,159,150,150,150,150,147,144,138,144,144,138,135,135,135,138,129,132,132,123,123,126,120,120,111,114,123,129,132,132,132,129,132,135,141,147,156,156,159,159,162,165,159,162,168,165,168,171,168,183,186,192,195,192,186,186,192,192,192,186,186,189,192,195,195,198,192,192,186,183,192,192,192,192,204,192,195,189,195,195,198,204,195,204,195,192,192,195,189,195,189,192,195,189,192,198,195,195,198,192,195,192,192,192,183},
{150,156,156,162,165,168,168,165,159,162,159,162,159,165,162,162,168,171,174,168,165,171,174,177,174,177,177,177,183,177,183,174,177,177,183,183,186,189,192,177,177,177,177,177,183,174,177,171,168,162,159,159,159,159,156,147,156,156,147,150,141,141,141,147,147,147,138,138,141,138,138,132,132,132,138,138,141,141,141,144,147,156,168,171,171,171,174,174,177,174,174,177,177,177,183,177,189,189,192,195,192,186,189,189,189,189,186,183,189,192,192,192,195,192,195,189,177,189,192,189,189,198,192,198,192,192,195,195,198,192,204,192,192,195,198,186,192,186,189,192,189,192,198,198,195,195,189,195,189,189,186,177},
{159,159,159,165,165,165,165,168,162,165,162,162,159,162,162,162,168,168,171,168,168,174,171,177,171,177,177,174,177,177,177,171,174,174,177,177,183,189,192,177,186,177,177,174,174,174,177,177,174,171,168,168,168,168,171,165,165,168,162,162,156,150,150,162,165,162,156,156,159,156,150,150,147,147,147,150,156,162,171,162,162,168,177,183,177,177,183,177,189,186,183,177,177,177,177,183,189,186,183,183,183,183,189,183,177,177,177,177,189,195,192,192,192,192,195,192,177,183,189,183,186,192,189,195,192,189,189,189,192,189,195,195,195,198,204,186,189,183,186,189,183,192,195,195,195,192,183,195,186,183,183,177},
{162,162,159,165,162,162,162,162,159,159,156,159,156,162,159,159,162,162,168,168,174,177,171,174,171,174,177,171,177,177,174,165,168,171,174,174,177,183,186,174,183,174,183,168,168,174,174,177,177,177,174,174,174,177,183,183,174,177,171,168,162,165,159,162,168,168,162,162,165,162,159,156,150,150,150,156,162,174,189,177,174,174,183,183,174,174,183,174,189,186,183,177,174,177,174,189,189,183,174,177,177,183,189,174,171,171,174,174,183,192,189,189,186,189,192,189,183,174,183,171,177,183,183,186,189,183,186,186,186,186,192,204,198,207,207,189,189,183,186,183,174,186,186,186,192,189,177,192,177,177,183,177},
{162,165,162,165,168,162,156,147,150,150,150,150,150,159,165,147,162,165,165,162,168,174,165,162,162,168,168,162,168,171,159,156,159,165,168,174,174,177,174,168,174,171,183,171,168,168,171,174,177,174,177,174,174,177,177,168,177,183,168,177,165,168,171,162,168,168,162,171,174,177,162,150,150,162,165,171,177,183,192,189,189,183,186,189,192,204,207,183,192,192,189,192,183,183,186,168,168,171,171,177,177,183,183,165,165,162,162,165,171,174,177,174,174,177,189,192,189,183,177,171,171,183,183,183,183,177,177,168,177,186,195,192,198,189,192,183,183,183,174,177,177,171,177,171,186,192,192,183,174,177,177,171},
{162,168,168,171,165,159,162,159,162,156,156,156,150,159,156,150,159,165,168,162,168,174,162,165,159,165,174,171,168,171,171,165,168,168,171,174,174,177,177,177,177,174,186,177,168,171,174,177,183,177,186,189,192,192,189,174,177,177,177,174,171,171,174,174,174,174,177,177,171,174,171,162,162,171,171,171,177,183,186,192,189,192,195,198,198,204,207,192,195,204,195,195,189,192,192,165,165,174,177,183,177,177,171,171,171,168,168,168,171,174,177,174,174,177,186,189,186,183,177,174,177,186,186,183,183,177,183,183,186,189,198,192,195,183,177,183,177,186,186,186,186,186,183,177,189,189,186,177,174,177,177,174},
{159,162,171,174,162,156,168,162,165,159,162,162,156,162,159,159,162,168,168,162,165,168,156,159,156,162,174,174,174,177,177,174,174,174,174,174,174,177,177,189,183,183,183,183,168,171,177,177,183,174,189,192,198,195,189,183,177,177,183,174,174,174,177,183,177,177,192,189,177,174,177,168,171,177,174,177,186,186,186,192,189,195,198,198,204,198,204,198,198,207,204,204,195,198,195,177,171,177,183,177,177,186,174,174,174,174,171,174,174,177,177,177,183,183,186,186,189,186,183,177,183,189,189,183,183,183,189,189,189,186,189,186,192,177,177,177,177,189,195,186,189,195,177,189,192,186,183,183,177,183,177,177},
{162,162,168,171,162,150,159,156,162,159,165,168,165,171,168,168,171,174,171,165,165,162,147,147,156,162,165,168,177,183,174,177,177,174,174,174,174,177,183,189,183,183,177,177,168,174,177,183,183,171,183,177,186,183,183,186,168,174,174,177,168,177,174,177,177,177,195,195,186,174,171,171,174,177,177,183,189,192,192,186,186,189,192,195,198,195,204,198,204,207,204,204,195,198,198,189,183,186,183,177,177,186,177,177,174,174,174,174,177,183,183,183,186,186,183,186,189,192,189,183,183,189,186,183,186,186,192,189,186,183,186,177,189,177,186,174,177,183,189,183,186,189,174,189,189,183,186,186,186,186,177,177},
{165,162,165,168,165,159,159,150,156,165,168,168,171,171,177,174,177,183,177,174,171,162,156,147,156,162,165,168,174,177,171,177,174,174,174,174,183,186,189,186,183,186,174,177,174,177,177,186,189,177,183,168,171,174,183,186,168,174,174,177,171,177,174,171,177,177,192,186,183,171,165,174,174,174,183,186,186,186,189,183,186,186,195,198,195,198,204,198,204,204,198,204,195,195,198,186,186,189,183,177,171,177,177,177,177,174,174,174,177,183,186,177,183,183,177,183,189,192,192,186,186,186,183,183,186,186,189,189,192,192,195,189,192,177,183,168,177,174,177,183,177,177,174,186,186,183,189,195,192,189,177,183},
{168,168,165,168,168,165,162,156,159,171,171,168,174,168,177,168,174,177,174,177,174,165,162,156,156,159,168,171,171,171,177,174,174,177,177,183,186,189,189,177,183,186,177,177,177,177,174,183,192,189,195,177,177,177,183,183,174,174,177,177,177,177,177,165,177,177,183,174,174,174,171,183,177,174,183,183,177,177,177,177,186,189,198,207,204,207,204,198,204,204,195,204,195,195,198,192,192,189,183,177,174,177,183,177,177,177,177,177,177,183,183,177,177,183,183,186,189,189,192,192,189,189,183,177,183,177,183,189,195,198,207,192,195,183,186,168,174,171,174,183,177,174,177,183,183,183,192,195,192,186,177,183},
{171,168,165,168,165,159,159,156,159,168,171,171,174,171,174,162,165,171,171,171,174,162,159,156,156,162,168,174,171,171,177,174,174,177,183,183,186,186,186,174,174,183,183,177,177,171,168,177,192,192,207,192,192,177,177,177,183,177,177,183,183,177,177,165,174,177,177,177,174,177,177,177,177,171,177,177,174,177,174,177,183,189,198,204,198,198,198,204,204,204,195,204,195,195,198,198,192,189,183,183,177,183,177,174,177,183,183,186,183,183,183,177,183,186,189,189,186,186,189,192,192,192,186,177,177,177,183,192,192,189,192,186,192,186,189,177,171,177,183,183,177,177,183,177,177,177,186,189,186,186,183,177},
{162,159,162,171,168,162,162,150,156,162,168,174,177,174,174,162,159,168,171,171,171,165,159,150,159,165,165,168,174,177,174,171,174,177,183,183,183,177,174,171,171,174,183,174,174,162,165,183,189,186,204,189,192,177,174,174,183,183,174,189,177,177,177,174,174,177,183,186,174,177,174,171,174,171,171,177,177,183,174,183,177,186,189,192,195,192,198,204,198,198,195,198,195,198,195,195,186,189,186,186,183,177,165,171,174,183,186,189,189,186,186,177,177,186,189,189,183,177,183,192,192,195,189,177,177,177,186,189,186,177,177,171,183,177,183,186,171,183,189,177,177,177,177,183,177,174,183,183,177,183,183,174},
{156,156,159,162,162,168,165,144,147,156,156,162,168,162,171,156,159,162,162,162,162,162,162,156,162,168,171,174,174,174,171,168,171,174,183,183,183,177,177,159,159,165,168,165,168,168,162,171,183,192,198,192,183,174,168,174,174,177,177,177,177,174,174,165,171,177,183,174,171,171,171,162,168,162,174,168,177,174,177,165,171,174,183,183,174,183,177,171,174,186,189,192,195,198,204,174,183,186,177,174,177,174,165,168,171,168,183,177,189,183,183,174,174,177,177,183,183,177,177,171,183,189,186,177,174,183,192,171,177,174,171,174,171,168,177,171,174,177,171,162,168,174,168,171,171,174,174,174,174,177,177,171},
{156,162,165,168,165,162,162,156,156,162,159,162,165,156,162,159,162,159,159,156,156,159,162,162,165,168,174,177,177,177,174,177,177,174,174,174,174,177,177,162,162,171,174,171,174,174,168,174,183,189,192,189,183,174,171,177,177,177,177,177,174,171,168,165,168,174,177,174,174,171,171,162,168,162,174,168,177,174,177,171,174,177,177,177,183,186,189,174,177,186,189,189,192,195,198,186,189,189,177,174,177,177,171,177,183,174,183,174,177,171,171,174,174,174,174,177,177,177,177,174,174,183,186,177,183,186,183,174,183,177,174,177,174,171,177,174,171,177,174,165,168,174,171,171,171,174,174,177,177,174,174,171}};


#endif /* __rcSTATICSETTINGS_H */
