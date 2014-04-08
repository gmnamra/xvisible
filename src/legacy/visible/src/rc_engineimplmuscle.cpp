/******************************************************************************
 *  Copyright (c) 2004 Reify Corp. All rights reserved.
 *
 *	rc_engineimpl_muscle.cpp
 *
 *	This file contains an implementation of a muscle tracking
 *	application.
 *
 ******************************************************************************/

#include "rc_engineimpl.h"
#include <rc_fileutils.h>

#define SHOWREJECTS 0
#define CellPolyBufferSize 5.0

// Perform motion tracking analysis on a focus area
int32 rcEngineImpl::muscleTracker(rcEngineFocusData* focus,
				    const vector<rcWindow>& focusImages,
				    int32 cacheSz, double s_msPerFrame)
{
	
  //@note
  //If autoSelect is on (default), use segmenter to find phase and frequency and continue to find 
  //Cell locations. 
  //If autoSelect is off, still use the segmenter to find phase and frequency but accept end positions
  //for cell localizations
  if (!_videoCacheP) return 0;
  double msPerFrame = s_msPerFrame;
  rmAssert(msPerFrame > 0);
	
  // Get some of the parameters
  int32 minTargetSize = getSettingValue(cAnalysisMotionTrackingTargetSettingId);
  const double minCellArea = minTargetSize * minTargetSize;
  rcOrganismInfo::OrganismName oname = static_cast<rcOrganismInfo::OrganismName>
    (static_cast<int>(getSettingValue( cAnalysisCellTypeId)));
  rcOrganismInfo::OrganismType otype (rcOrganismInfo::eCell);
  bool autoSelect = oname != rcOrganismInfo::eCardioMyocyteSelected;
	 
  // Ddebugging movie is generated if developer mode is on
  rcGenMovieFile debugMovie (std::string ("/Users/Shared/test.rfymov"), 
			     movieOriginSynthetic, "Test Code",
			     movieFormatRevLatest, true, 30);
	 
  // Create a segmenter 
  rcMuscleSegmenter segmenter (developerDebugging() ? &debugMovie : NULL);
  _observer->notifyStatus( "SpatioTemporal Analysis..." );
	 
  // Calculate how many frames to use for determination of periodicity
  const float framesPerMs = (float)(1/msPerFrame);
  const float maxFramesPerPeriod = 2000*framesPerMs;
  float framesPerPeriod  = -1;
	 
  // AutoGen parameter is 0 (if auto) and hertz if specified by the user
  const int32 autoGen = getSettingValue(cAnalysisMuscleAutoGenPeriodId);
  if (autoGen > 0)
    {
      int32 msPerPeriod = 1000 / autoGen; // 1000 msec in a second
      rmAssert(msPerPeriod > 0);
	framesPerPeriod = msPerPeriod / (double) msPerFrame;
    }
	 
  // Check 3 period need with movie size
  uint32 threePeriods = (uint32)(6000*framesPerMs);
  if (threePeriods > (focusImages.size() - 1))
    threePeriods = (focusImages.size() - 1);
	 
  if (1) printf("%s %s framesPerMs %f maxFramesPerPeriod %f framesPerPeriod %f threePeriods %d w %d h %d\n",
		autoGen >= 0 ? "Paced" : "Not Paced" , (autoGen == 0) ? " Enabled " : " Specified",
		framesPerMs, maxFramesPerPeriod, framesPerPeriod, threePeriods,
		focusImages[0].width(), focusImages[0].height());
	 
  // Now call the segmenter to segment
  // @note that this call will calculate phase, frquency of the paced plate as well as cell locations
  int32 segStatus = segmenter.segment(focusImages, 0, (int32)threePeriods, 
					autoSelect);

  //@note debugging 
  {
    rcWindow segImg, freqImg, emiImg;
    segmenter.hackInsertSegmentedImages (segImg, freqImg, emiImg);
    std::string moviefn = std::string ("/Users/arman/tmpdir/") + rfStripPath (_movieFile);
    
    if (segImg.isBound())
      {
	std::string rawFileName = moviefn + std::string ("_seg.tif");
	cerr << "Result File Name: " << rawFileName << endl;
	segImg.tiff (rawFileName);
      }
    if (emiImg.isBound())
      {
	std::string rawFileName = moviefn + std::string ("_emi.tif");
	cerr << "Result File Name: " << rawFileName << endl;
	emiImg.tiff (rawFileName);
      }
    if (freqImg.isBound())
      {
	std::string rawFileName = moviefn + std::string ("_freq.tif");
	cerr << "Result File Name: " << rawFileName << endl;
	freqImg.tiff (rawFileName);
      }
  }	 

  if (segStatus >= 0)
    cerr << "fba: " << framesPerPeriod << " seg: " << segmenter.framesPerPeriod() << endl;  
	 
  // If we are auto selecting and had errors 
  if (autoSelect && segStatus < 0)
    {
      cerr << "Segmenter returned: " << segStatus << endl;
      return segStatus;
    }
	 
  // If we are not on autoSelect mode, continue filtering the segmenter results
  // Otherwise use specific data
  // Declare the cell polygon groups
  vector<rcPolygon> cells, ocells, rejects;	
  vector<vector<rc2Fvector> > specs;
  vector<float> rads;
	 
  if (!autoSelect && segStatus != rcMuscleSegmenter::cInvalidPeriod)
    {
      const double minActivePct = 0.25;
      segStatus = segmenter.filterByActivity(minActivePct, focusImages, 0, (int32)threePeriods, 
					     maxFramesPerPeriod, framesPerPeriod);

      // Get Polygon Info From the Engine 
   //   vector<rcPolygon>& pgs = const_cast<rcPolygonGroupRef*> (polys)_selectedPolys.polys ();
      for (uint32 i = 0; i < _selectedPolys.size (); i++)
      {
		const rcPolygon& p = _selectedPolys.polys ()[i];
	rcAffineRectangle ar = p.minimumAreaEnclosingRect ();
	bool side = ar.cannonicalSize().x() > ar.cannonicalSize().y();
	vector<rc2Fvector> c0;
	if (side)
	  {
	    c0.push_back (ar.affineToImage(rc2Fvector(0.0f, 0.5f)));
	    c0.push_back (ar.affineToImage(rc2Fvector(1.0f, 0.5f)));
	  }
	else
	  {
	    c0.push_back (ar.affineToImage(rc2Fvector(0.5f, 0.0f)));
	    c0.push_back (ar.affineToImage(rc2Fvector(0.5f, 1.0f)));
	  }
	
	specs.push_back (c0);
	rads.push_back (c0[0].distance (c0[1]));
      }

    }
  else // AutoSelect and segStatus >= 0
    {
      segmenter.filterBySize(minCellArea);
      const double minAspectRatio = 1.5;
      segmenter.filterByAR(minAspectRatio);
      const int32 minDelta = 18;
      segmenter.filterAtEdge(minDelta);
      const double minActivePct = 0.25;
      segStatus = segmenter.filterByActivity(minActivePct, focusImages, 0, (int32)threePeriods, maxFramesPerPeriod, framesPerPeriod);
		 
      if (segStatus < 0)
	return segStatus;
		 
      segmenter.getCells(ocells);
    }
	 
  // Create a buffer around a cell 
  for (uint32 i = 0; i < ocells.size(); i++)
    {
      cells.push_back (ocells[i].discBuffer (CellPolyBufferSize));
    }
	 
  double statusUpdateInterval = 1.5;
	 
  const rc2Fvector centerPixelOffset; // ((float)0.5, (float)0.5);
	 
  bool displayMotionVectors =
    getSettingValue(cAnalysisMotionTrackingMotionVectorDisplaySettingId);
  bool displayBodies =
    getSettingValue(cAnalysisMotionTrackingBodyVectorDisplaySettingId);
  bool displayBodyHistories =
    getSettingValue(cAnalysisMotionTrackingBodyHistoryVectorDisplaySettingId);
  bool displaySegmentation =
    getSettingValue(cAnalysisMuscleSegmentationVectorDisplaySettingId);
	 
  // Create graphics writers if applicable
  createGraphicsWriters(displayMotionVectors, displayBodies, displayBodyHistories, displaySegmentation);
	 
  // If we have a graphics track, draw results
  if (_segmentVectorWriter != 0 && (cells.size () || specs.size () ) )
    {
      printf("made it %d cells\n", (int) rmMax (cells.size(), specs.size ()) );
		 
      rcVisualGraphicsCollection graphics;
      rcVisualSegmentCollection& segments = graphics.segments();
		 
      // Green color, line width of 1 display pixel, 0.0 pixel origin
      rcVisualStyle validCell(rfRgb(128, 255, 20), 1, rc2Fvector(0.0f, 0.0f));
      // Set drawing style
      segments.push_back(validCell);
		 
      const uint32 cellCnt = cells.size();
      const uint32 rejectCnt = rejects.size();
      uint32 elmtCnt = cellCnt + rejectCnt;
      for (uint32 ci = 0; ci < cellCnt; ci++)
	elmtCnt += cells[ci].vertexCnt();
		 
      for (uint32 ri = 0; ri < rejectCnt; ri++)
	elmtCnt += rejects[ri].vertexCnt();
		 
      if (displayBodies)
	{
	  segments.reserve(elmtCnt + 1);
	  for (uint32 ci = 0; ci < cellCnt; ci++)
	    rfPolygonToSegmentsCollection(cells[ci], segments);
	}

      segmenter.getRejects(rejects);		 
      produceMuscleDebugResults (rejects, segmenter, segments);
		 
      // Hack for displaying segmenter results on the graphics track just once. 
      rcRect focusRect(focus->focusRect());
      const rc2Fvector focusOffset(focusRect.x(), focusRect.y());
      offsetGraphics(graphics, focusOffset + centerPixelOffset);
      _segmentVectorWriter->writeValue(focusImages[0].frameBuf()->timestamp(), focusRect, graphics);
      {
	rcVisualGraphicsCollection nullGraphics;
	_segmentVectorWriter->writeValue(focusImages.back().frameBuf()->timestamp(),
					 focusRect, nullGraphics);
      }
    }  // _segmentVectorWriter != 0
	 
  _observer->notifyStatus( "Processing experiment..." );
	 
  if (cells.size() == 0 && specs.size () == 0)
    return 0;
	 
  // Create a grabber to read the image vector
  rcVectorGrabber imageGrabber(focusImages, cacheSz);
	 
  if (! imageGrabber.isValid()) 
    {
      reportOnBadGrabber (imageGrabber);
      return 0;
    }
	 
  rcRect focusRect = focus->focusRect();
  rcIRect rect(focusRect.x(), focusRect.y(),
	       focusRect.width(), focusRect.height());
	 
  // @note do we need these ? 
  // Specify tracker options
  const int32 maxSpeed =
    getSettingValue(cAnalysisMotionTrackingSpeedSettingId );
	 
  // Set aggregate speed writer expected max values
  rcScalarWriter* speedWriter = focus->cellSpeedMeanWriter();
  if (speedWriter) speedWriter->setExpectedMaxValue(maxSpeed);
	 
	 
  // Figure out phase and frequency to pass to kinetoscope
  // @note emi is the index of the fully extended muscle for a paced
  // experiment. The interface to Kinetescope expects # of frames 
	 
  int32 emi (0);
  if ((emi = segmenter.extendedMuscleCellIndex ()) > 0)
    {
      float kinePhase = (float) emi;
      kinePhase = fmod (kinePhase, segmenter.framesPerPeriod());
      emi = (int32) kinePhase;
    }

  cerr <<  "Pacing @ " <<  segmenter.framesPerPeriod() << " Starting Phase: " << emi << endl; 
  rcEngineProgress progressIndicator(this, _observer, "MyoSight Processing"
				     "...%.2f%% complete", statusUpdateInterval);
  progressIndicator.progress(0.0);
	 
  // Create a Kinetoscope
	 
  rmAssert (emi >= 0);
  rcKinetoscopeParams params; params.phase = emi;params.c = otype;params.n = oname;
  rcKinetoscope motionTracker(imageGrabber, rect, params); 
  if (cells.size())
    motionTracker.preSegmentedBodies(cells);
  else if (specs.size() && rads.size () && specs.size() == rads.size ())
    motionTracker.preSegmentedBodies(specs, rads); 
  motionTracker.minMobSizeInPixels(minTargetSize);
  motionTracker.sourceMotion (rcKinetoscope::eUnderSampled);
  motionTracker.sourceMotion (rcKinetoscope::eContractile);
  motionTracker.periodicMotion (segmenter.framesPerPeriod ());
  motionTracker.averageFramesPerSecond (_videoCacheP->averageFrameRate());
	 
  int32 maxFrames = imageGrabber.frameCount();
  if ( maxFrames < 0 )
    maxFrames = 1;
	 
  // Time stamp of previous frame
  //@note a constructed motiontracker already has the first frame
  // ProduceCellResults is called at (moving) time: 2nd / later stage of the motion 
  // All results need to be at moving time.
  // @note this needs to be redone to support both operational modes. 
  // In isLabelProtein() after ctor we have the first results ready
  bool firstCell = true;
  rcTimestamp prevTimeStamp = motionTracker.fixed().frameBuf()->timestamp(); // return fixed
  focus->setTrackStartTimes( prevTimeStamp );       
	 
  //   int32 maxFrames = 0;
  //     // Time stamp of previous frame
  //     rcTimestamp prevTimeStamp = cZeroTime;
  //     bool firstFrame = true;
  //     bool firstCell = true;
  uint32 analysisCount = 0;
  while (motionTracker.step(1))
    {
      ++analysisCount;
      rcTimestamp curTimeStamp = motionTracker.resultTime ();  // returns fixed for paced cardiomyocyte
      rcTimestamp frameInt = curTimeStamp - prevTimeStamp;
		 
      if (analysisCount > 1 && frameInt <= 0.0)
	rmExceptionMacro (<< "Error: invalid frame interval " << frameInt.secs() << "f[" << analysisCount+1 << "]" << endl);
		 
      if (analysisCount > 1)
	{
	  // Development/debug image and graphics
	  rcVisualGraphicsCollection debugGraphics;
	  motionTracker.visualDebugGraphics( debugGraphics );
	  rcWindow debugImage = motionTracker.debug();
	  produceDebugResults( focus, debugImage, debugGraphics, focusRect, curTimeStamp );
			 
	  // Produce results for each cell
	  produceCellResults(focus, motionTracker, focusRect, curTimeStamp, firstCell);
	}
		 
      // Update progress bar
      if (progressIndicator.progress((100.0 * (analysisCount+1))/maxFrames)) {
	// Abort
	_observer->notifyStatus("Analysis stopped");
	goto stop;
      }
		 
      // Roll Over
      prevTimeStamp = curTimeStamp;
      motionTracker.advance ();  // moving -> fixed, moving to fixed
    }
	 
 stop:
  // Temporary: Now Dump the results stored in the cell
  // @note: make a file name for the results. 
  // Movie file path is used if it exists and that we are running in the
  // cmd line mode.
		 
  // Update elapsed time to refresh display
  _observer->notifyTime(getElapsedTime());
  return analysisCount;
}


bool rcEngineImpl::produceMuscleDebugResults (const vector<rcPolygon>& rejects, rcMuscleSegmenter& segmenter,  rcVisualSegmentCollection& segments)
{
  int32 startIndex, endIndex;
  segmenter.rejectRange(startIndex, endIndex, rcMuscleSegmenter::cRejCodeSize);
  if (startIndex != endIndex) {
    // Red color, line width of 1 display pixel, 0.0 pixel origin
    rcVisualStyle rejectCell(rfRgb(255, 0, 0), 1, rc2Fvector(0.0f, 0.0f));
    // Set drawing style
    segments.push_back(rejectCell);
    for (int32 ri = startIndex; ri < endIndex; ri++)
      rfPolygonToSegmentsCollection(rejects[ri], segments);
  }
	
  segmenter.rejectRange(startIndex, endIndex, rcMuscleSegmenter::cRejCodeAR);
  if (startIndex != endIndex) {
    // Red color, line width of 2 display pixel, 0.0 pixel origin
    rcVisualStyle rejectCell(rfRgb(128, 128, 0), 2, rc2Fvector(0.0f, 0.0f));
    // Set drawing style
    segments.push_back(rejectCell);
    for (int32 ri = startIndex; ri < endIndex; ri++)
      rfPolygonToSegmentsCollection(rejects[ri], segments);
  }
	
  segmenter.rejectRange(startIndex, endIndex, rcMuscleSegmenter::cRejCodeEdge);
  if (startIndex != endIndex) {
    // Red color, line width of 2 display pixel, 0.0 pixel origin
    rcVisualStyle rejectCell(rfRgb(0, 0, 0), 4, rc2Fvector(0.0f, 0.0f));
    // Set drawing style
    segments.push_back(rejectCell);
    for (int32 ri = startIndex; ri < endIndex; ri++)
      rfPolygonToSegmentsCollection(rejects[ri], segments);
  }
	
	
  segmenter.rejectRange(startIndex, endIndex,
			rcMuscleSegmenter::cRejCodeActivity);
  if (startIndex != endIndex) {
    // Red color, line width of 4 display pixel, 0.0 pixel origin
    rcVisualStyle rejectCell(rfRgb(0, 10, 128), 2, rc2Fvector(0.0f, 0.0f));
    // Set drawing style
    segments.push_back(rejectCell);
    for (int32 ri = startIndex; ri < endIndex; ri++)
      rfPolygonToSegmentsCollection(rejects[ri], segments);
  }	
}



#if 0
{
  std::string rawFileName = makeTmpName (std::string (".raw"));
  cerr << "Result File Name: " << rawFileName << endl;
  ofstream rawFile (rawFileName.c_str());
  ostream& output1 = (rawFile.is_open()) ? rawFile : cerr;
  const rcStatistics& popu = motionTracker.populationMeasures ();
  if (popu.n())
    output1 << motionTracker.visualBodies().size() << "\t" << popu.min() << "\t" << 
      popu.max () << "\t" << popu.mean () << "\t" << popu.stdDev () << endl;
		 
}
	 
{
  std::string resFileName = makeTmpName (std::string (".fl"));
  std::string resFile2Name = makeTmpName (std::string (".cra"));
  cerr << "Result File Name: " << resFileName << endl;
  ofstream resFile (resFileName.c_str());
  ofstream resFile2 (resFile2Name.c_str());
		 
  ostream& output1 = (resFile.is_open()) ? resFile : cerr;
  ostream& output2 = (resFile2.is_open ()) ? resFile2 : cerr;
  motionTracker.temporalResults (output1, output2);
  if (resFile.is_open()) resFile.close();
  if (resFile2.is_open()) resFile2.close();
}
#endif
	 
