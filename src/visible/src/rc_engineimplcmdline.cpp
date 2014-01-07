/*
 *
 *$Id $
 *$Log$
 *Revision 1.15  2006/01/15 22:56:51  arman
 *selective myo
 *
 *Revision 1.14  2005/12/06 23:57:19  arman
 *fixed commandline and reused displacement for range when aci is used.
 *
 *Revision 1.13  2005/08/30 21:08:51  arman
 *Cell Lineage
 *
 *Revision 1.13  2005/08/01 21:20:14  arman
 *added maxDisplacement
 *
 *Revision 1.12  2005/04/08 05:18:31  arman
 *further cleanup of settings
 *
 *Revision 1.11  2005/04/08 03:49:53  arman
 *implemented the graphics options
 *
 *Revision 1.10  2005/04/08 03:11:51  arman
 *fixed a bug in command processing of report type
 *
 *Revision 1.9  2005/02/18 00:42:31  arman
 *fixed incorrect return upon error
 *
 *Revision 1.8  2004/09/24 21:01:02  arman
 *added new organism types
 *
 *Revision 1.7  2004/08/26 22:38:23  arman
 *removed -f to be required
 *
 *Revision 1.6  2004/08/18 20:16:18  arman
 *fixed a bug in cell type settting
 *
 *Revision 1.5  2004/08/12 02:04:39  arman
 *added slidingWindow mode to ACI
 *
 *Revision 1.4  2004/08/01 04:49:40  arman
 *added a help text
 *
 *Revision 1.3  2004/06/20 22:55:19  arman
 *fixed help and fixed default processing
 *
 *Revision 1.2  2004/06/16 16:17:43  arman
 *added cell type and Merge
 *
 *Revision 1.1  2004/06/16 15:22:07  arman
 *comand line interface to Visible
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "rc_engineimpl.h"
#include <rc_parametermap.h>
#include <rc_fileutils.h>

//////////// Command Line Initialization
bool rcEngineImpl::setFromArgs (int32 argc, char **argv)
{
  _batchMode = true;

  rcCmdLine cmd;
  if (cmd.splitLine (argc, argv) < 1)
    {
      cerr << "Too Few Arguments" << endl;
      return false;
    }

  if (cmd.hasSwitch ("-h"))
    {
      rcVisibleBatchUsage (cerr);
      return 0;
    }

  string ACI ("aci"), VEN ("vis"), DIF ("dif"), CELL ("cell"), CARDIO ("myo");
  string EML ("eml"), CSV ("csv"), MCA ("mca"), SMOOTH ("smooth"), FLU ("flu");
  string BODY ("body"), MOTION ("motion"), ALL ("all"), EXPOSURE ("exp"), DRG ("drg");
  string DRGAUTOMASK ("drgautomask");
  string NOP (""), TEMPLATE ("target");

  string analysis, cell, report, graphics, movie, result, advanced, exportImages, tiffdir, model2use;

  // Get the required arguments
  try
    {
      analysis = cmd.getArgument ("-analysis", 0);
      result = cmd.getArgument ("-result", 0);
    }
  catch (...)
    {
      rmExceptionMacro (<< "-analysis -result options are required" << endl);
    }

  try
    {
      movie = cmd.getArgument ("-movie", 0);
    }
  catch (...)
    {
        cout << "Not a movie, is it a directory of tiffs \n";
      try
      {
        tiffdir = cmd.getArgument ("-dirOfTiffs", 0);
      }
      catch (...)
      {
          rmExceptionMacro (<< "either movie or tiff directory options are required" << endl);
      }
    }

  // Get the optional stuff
  // @note: some parameters are set using setSettingId and some directly in the engine
  // the choice largely mirrors what takes place in rc_engineimpl.cpp

  report = cmd.getSafeArgument ("-report", 0, "csv");
  cell = cmd.getSafeArgument ("-type", 0, "general");
  graphics = cmd.getSafeArgument ("-graphics", 0, "no");
  advanced = cmd.getSafeArgument ("-advanced", 0, "no");
  exportImages = cmd.getSafeArgument ("-exportImages", 0, "");

  _cLineStartFrame  = cmd.atoi(cmd.getSafeArgument("-Frame", 0, "-1"));
  _cLineEndFrame  = cmd.atoi(cmd.getSafeArgument("-Frame", 1, "-1"));
  _cLineStartFrame = rmMin (_cLineStartFrame, _cLineEndFrame);
  _cLineEndFrame = rmMax (_cLineStartFrame, _cLineEndFrame);
  _cLineRectTL = rcIPair (cmd.atoi(cmd.getSafeArgument("-TopLeft", 0, "-1")),
	      cmd.atoi(cmd.getSafeArgument("-TopLeft", 1, "-1")));
  _cLineRectSize = rcIPair (cmd.atoi(cmd.getSafeArgument("-Size", 0, "-1")),
			    cmd.atoi(cmd.getSafeArgument("-Size", 1, "-1")));
  int32 maxDisp = cmd.atoi(cmd.getSafeArgument("-MaxDisplacement", 0, "2"));
  rcValue maxTrackDisp (maxDisp);
  int32 channel = cmd.atoi (cmd.getSafeArgument("-Channel", 0, "0"));
  rcValue channelVal (int(channel));

  // @note: First things first, we only accept movies and are in command mode
  rcValue cmdline (eCmd);
  setSettingValue( cInputModeSettingId, cmdline);

  setSettingValue ( cAnalysisChannelSettingId, channelVal);
  setSettingValue (cAnalysisMotionTrackingSpeedSettingId, maxTrackDisp);
  setSettingValue (cGlobalMotionEstId, maxTrackDisp);
  int32 halfWindowSize = cmd.atoi(cmd.getSafeArgument("-halfWindow", 0, "0"));
  int32 windowSize = halfWindowSize * 2 + 1;
  setSettingValue (cAnalysisACISlidingWindowSizeSettingId, windowSize);

  int32 sformat = (int) eExperimentNativeFormat;

  sformat = (int) eExperimentNativeFormat;

  if (report == CSV)
   {
      sformat = (int) eExperimentCSVFormat;
    }
  else if (report == ALL)
    {
      sformat = (int) eExperimentAllFormat;
    }

  _saveFormat = (rcExperimentFileFormat) sformat;

 

  if (advanced == DRG)
    {
      setSettingValue (cAnalysisObjectSettingId, eMacroDRGdiffusionProcessing);
      _useMask = 0;
    }

 	
  //  Set to the default

 if (halfWindowSize < 1)
   {
     setSettingValue( cAnalysisModeSettingId, cAnalysisACI);
   }
 else
   {
     setSettingValue( cAnalysisModeSettingId, cAnalysisACISlidingWindow);
     setSettingValue (cAnalysisACISlidingWindowOriginSettingId, eAnalyzerResultOriginCenter);
   }

 setSettingValue( cAnalysisACIOptionSettingId, rcSimilarator::eACI);
 setSettingValue( cAnalysisFirstFrameSettingId, 0);
 setSettingValue (cAnalysisCellTypeId, cAnalysisCellGeneral);

  // Set Rect if passed and valid

  if (cell == CARDIO)
    {
      setSettingValue (cAnalysisCellTypeId, cAnalysisCellMuscle);
    }

  if (cell == FLU)
    {
      setSettingValue (cAnalysisCellTypeId, cAnalysisLabeledFluorescence);
    }


  if (analysis == VEN)
    {
      cerr << "analysis: " << VEN << endl;
      setSettingValue( cAnalysisACIOptionSettingId, rcSimilarator::eVisualEntropy);
      //      _aciOptions = rcSimilarator::eVisualEntropy;
    }
	else if (analysis == TEMPLATE)
    {
      setSettingValue( cAnalysisModeSettingId,cAnalysisTemplateTracking);
    }
  else
    {
      cerr << "Default ACI Option Selected" << endl;
    }

  if (graphics == MOTION)
    {
      setSettingValue (cExportMotionVectorsSettingId, true);
    }
  else if (graphics == BODY)
    {
      setSettingValue (cExportBodyGraphicsSettingId, true);
    }

  if (movie != NOP)
  {
    rcValue inputFile(movie);
    setSettingValue( cInputMovieFileSettingId, inputFile);
    _outputFileName = result;
    _exportImagesDir = exportImages;
   return true;
  }
  else
  {
      rmAssert (tiffdir != NOP);
      vector<std::string> files;
      //@note gets tiff files: .tif or .TIF
      rfGetDirEntries (tiffdir, files);

      if (files.size())
      {
          // Iterate all selected files
          string selectedFiles;
          vector<std::string>::iterator it = files.begin();
          while (it != files.end())
          {
              string sg (it->c_str ());

              if (sg != NOP)
              {
                  // Create a list of semicolon separated file names
                  selectedFiles += sg;
                  selectedFiles +=";";
              }
              ++it;
          }

          if (!selectedFiles.empty())
          {
              rcValue inputDir (selectedFiles);
              setSettingValue( cInputImageFilesSettingId, inputDir);
              _outputFileName = result;
              return true;
          }
      }

  }

  rmExceptionMacro (<< "Visible Command Line Error " << endl);

}





ostream& rcVisibleBatchUsage (ostream& o)
{
  o << endl;
  o << endl <<
    "       -movie [movieFile]                                                                  " << endl <<
    "       -dirOfTiffs [tiffdir]                                                               " << endl <<
    "       -result [resultFile]                                                                 " << endl <<
    "       -analysis aci|vis|dif|cell   Analysis mode:    aci  - Exhaustive aggregate change index" << endl <<
    "                                               vis  - Exhaustive visual entropy        " << endl <<
    "                                               dif  - Exhaustive diffusion index       " << endl <<
    "                                               cell - Cell tracking (default)          " << endl <<
    "                                               target - Target tracking (default)          " << endl <<	
    "       -halfWindow (half window size:            applies to aci/vis/dif                         " << endl <<
    "       -type general|myo|smooth|Flu.    Cell Type: general (default) myocyte              " << endl <<
    "       -report csv|eml            Results format:   csv - Comma separated values            " << endl <<
    "                                               xml - Native Reify XML format           " << endl <<
    "                                               default: Native                         " << endl <<
    "       -graphics no|body|motion|all Graphics output:  no     - None                           " << endl <<
    "                                               body   - Locomotive body (default)      " << endl <<
    "                                               motion - Motion vector                  " << endl <<
    "                                               all    - All                            " << endl <<
    "       -Frame (2 numbers to indicate start/finish  (default: entire frame)             " << endl <<
    "       -TopLeft (2 numbers first indicates X, Second Y)                                " << endl <<
    "       -Size (2 numbers first indicates width, Second height                           " << endl <<
    "       -MaxDisplacement (in Pixels maximum change in position in between sequential frames " << endl <<
    "       -Channel (-1 all, 0, average, 1, 2, 3 (r & g & b or reverse)                    " << endl <<
    "       -advanced: (exp - LongExposure, drg - Dorsal Assay, drgautomask - Dorsal Assay AutoMask)              " << endl <<
    "       -exportImages [exportDirectory]                                         " << endl << endl <<
    "Example: Visible -analysis aci -type myo -graphics no -movie /Users/john/Desktop/foo.rfymov -result /Users/john/Desktop/foo " << endl;

  return o;
}
