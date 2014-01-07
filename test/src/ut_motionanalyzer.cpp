// Copyright (c) 2003 Reify Corp. All rights reserved.

#include <stdlib.h>

#include <rc_window.h>
#include "ut_motionanalyzer.h"

const uint32 imageCnt = 10;
const double corrThreshold = 0.5;

const uint32 setIntermediate = 0;
const uint32 setHorizontal   = 1;
const uint32 setVertical     = 2;
const uint32 setOverThresh   = (setHorizontal | setVertical);

UT_MotionAnalyzer::UT_MotionAnalyzer()
{
}

UT_MotionAnalyzer::~UT_MotionAnalyzer()
{
  printSuccessMessage("Motion Analyzer test", mErrors);
}

uint32 UT_MotionAnalyzer::run()
{
  /* Generate test data used in testing all test sequences.
   */
  genTestData();

  vector<uint32> frameID(imageCnt);
  vector<rcSharedFrameBufPtr> sequence(imageCnt);

  /* Don't bother trying all the different possibilities in the first
   * location because they should all test the same cases.
   */
  frameID[0] = setHorizontal;
  sequence[0] = _frames[frameID[0]];

  /* Recursive fct that generate test sequences and calls test fct.
   */
  genTestSequence(frameID, sequence, 1);

  return mErrors;
}

void UT_MotionAnalyzer::genTestData()
{
  srand(0);

  _frames.resize(setOverThresh*imageCnt);

  const rcRect horizontal  (16, 32, 96, 64);
  const rcRect vertical    (32, 16, 64, 96);
  const uint32 imgWidth = 128;
  const uint32 imgHeight = 128;

  /* Initialize all the frame buffers with unique images. This step
   * will create 3 sets of images. One set will be largely black on
   * the left side of each image, another set will be largely black on
   * the right side of each image and the last set will be an
   * intermediate with black on both sides of each image.
   */
  for (uint32 i = 0; i < imageCnt; i++) {
    rcSharedFrameBufPtr horFrame = new rcFrame(imgWidth, imgHeight,
					       rcPixel8);
    rcSharedFrameBufPtr verFrame = new rcFrame(imgWidth, imgHeight,
					       rcPixel8);
    rcSharedFrameBufPtr crossFrame = new rcFrame(imgWidth, imgHeight,
					       rcPixel8);
    rmAssert(horFrame);
    rmAssert(verFrame);
    rmAssert(crossFrame);

    rcWindow horWinBackground(horFrame);
    rcWindow verWinBackground(verFrame);
    rcWindow crossWinBackground(crossFrame);
    
    horWinBackground.setAllPixels(255);
    verWinBackground.setAllPixels(255);
    crossWinBackground.setAllPixels(255);

    rcWindow horWinObject(horFrame, horizontal);
    rcWindow verWinObject(verFrame, vertical);
    rcWindow horCrossWinObject(crossFrame, horizontal);
    rcWindow verCrossWinObject(crossFrame, vertical);

    horWinObject.setAllPixels(0);
    verWinObject.setAllPixels(0);
    horCrossWinObject.setAllPixels(0); verCrossWinObject.setAllPixels(0);
    
    const uint32 row = i;
    for (uint32 col = 0; col < 10; col++) {
      horWinObject.setPixel(row, col, 255);
      verWinObject.setPixel(row, col, 255);
    }
    for (uint32 col = 0; col < 5; col++) {
      horCrossWinObject.setPixel(row+10, col, 255);
      verCrossWinObject.setPixel(row+10, col, 255);
    }
    
    /* Inject a little noise into the images to hopefully create
     * unique correlation score for the correlation matrix.
     */
    uint32 randCnt = (uint32)((rand() >> 4) & 0xFF);
    while (randCnt--) {
      uint32 x = (uint32)((rand() >> 4) & 0x7F);
      uint32 y = (uint32)((rand() >> 4) & 0x7F);
      uint32 v = (uint32)((rand() >> 4) & 0xFF);
      horWinBackground.setPixel(x, y, v);
    }

    randCnt = (uint32)((rand() >> 4) & 0xFF);
    while (randCnt--) {
      uint32 x = (uint32)((rand() >> 4) & 0x7F);
      uint32 y = (uint32)((rand() >> 4) & 0x7F);
      uint32 v = (uint32)((rand() >> 4) & 0xFF);
      verWinBackground.setPixel(x, y, v);
    }

    randCnt = (uint32)((rand() >> 4) & 0xFF);
    while (randCnt--) {
      uint32 x = (uint32)((rand() >> 4) & 0x7F);
      uint32 y = (uint32)((rand() >> 4) & 0x7F);
      uint32 v = (uint32)((rand() >> 4) & 0xFF);
      crossWinBackground.setPixel(x, y, v);
    }

    _frames[i + imageCnt*setHorizontal] = horFrame;
    _frames[i + imageCnt*setVertical] = verFrame;
    _frames[i + imageCnt*setIntermediate] = crossFrame;
  }

  _corrScores.resize(imageCnt*setOverThresh);
  _energyScores.resize(imageCnt*setOverThresh);
  for (uint32 i = 0; i < _corrScores.size(); i++) {
    _corrScores[i].resize(_corrScores.size());
    _corrScores[i][i] = 1.0;
    _energyScores[i].resize(_energyScores.size());
    _energyScores[i][i] = rcMotionAnalyzer::corrToEnergy(_corrScores[i][i]);
  }

  /* Now generate a cross-correlation matrix between all the test
   * images.
   */
  rsCorrParams params; // Default params OK.
  rcCorr res;

  for (uint32 i = 0; i < (_corrScores.size()-1); i++) {
    rmAssert(i < _frames.size());
    rcWindow winI(_frames[i]);

    for (uint32 j = i+1; j < _corrScores.size(); j++) {
      rmAssert(j < _frames.size());
      rcWindow winJ(_frames[j]);

      rfCorrelate (winI, winJ, params, res);

      _corrScores[i][j] = _corrScores[j][i] = res.r();
      _energyScores[i][j] = _energyScores[j][i] =
	rcMotionAnalyzer::corrToEnergy(_corrScores[i][j]);
    }
  }

  /* Make sure corrThreshold is able to distinguish between the
   * correlation scores returned for intra-set vs inter-sets frames to
   * allow the test algorithms to control which frames will be saved
   * and which will not.
   */
  for (uint32 i = 0; i < imageCnt; i++)
    for (uint32 j = 0; j < imageCnt; j++) {
      rmAssert(_corrScores[i][j] > corrThreshold);
      rmAssert(_corrScores[i + imageCnt][j + imageCnt] > corrThreshold);
      rmAssert(_corrScores[i + imageCnt*2][j + imageCnt*2] > corrThreshold);
    }

  for (uint32 i = 0; i < imageCnt; i++)
    for (uint32 j = imageCnt; j < imageCnt*2; j++) {
      rmAssert(_corrScores[i][j] > corrThreshold);
      rmAssert(_corrScores[i][j + imageCnt] > corrThreshold);
    }

  for (uint32 i = imageCnt; i < imageCnt*2; i++)
    for (uint32 j = imageCnt*2; j < imageCnt*3; j++)
      rmAssert(_corrScores[i][j] < corrThreshold);

#if 0
  /* Print it out to make sure everything looks OK.
   */
  printf("Normailzed correlation matrix\n");
  for (uint32 row = 0; row < _corrScores.size(); row++) {
    for (uint32 col = 0; col < _corrScores.size(); col++)
      printf("%02d ", (uint32)(_corrScores[row][col]*100));
    printf("\n");
  }

  printf("\nEnergy matrix\n");
  for (uint32 row = 0; row < _energyScores.size(); row++) {
    for (uint32 col = 0; col < _energyScores.size(); col++)
      printf("%02d ", (uint32)(_energyScores[row][col]*100));
    printf("\n");
  }
  printf("\n");
#endif
}

void
UT_MotionAnalyzer::calcExpectedValues(vector<uint32>& frameID,
				      vector<rcSharedFrameBufPtr>& sequence,
				      vector<rcMotionAnalyzerResult>& exp)
{
  exp.resize(frameID.size());
  
  exp[0].entropy(-1);
  exp[0].saveMe(true);
  exp[0].rateWarning(false);
  exp[0].frameBuf(sequence[0]);
  
  uint32 lastSaved = frameID[0];
  uint32 lastSavedSet = lastSaved/frameID.size();
  uint32 prevSet = lastSavedSet;
  /* In the case where the only reason for saving the current frame is
   * that the correlation of the next frame with the the last saved
   * frame exceeds the corr. thresold, we need to remember that this
   * is the case so that the next frame gets saved as
   * well. saveOverride does this.
   */
  bool saveOverride = false;

  for (uint32 i = 1; i < frameID.size(); i++) {
    /* Do easy part first - store buffer address
     */
    exp[i].frameBuf(sequence[i]);

    /* Now do the rest of the calculations
     */
    uint32 current = frameID[i];
    uint32 curSet = current/frameID.size();
    rmAssert(curSet < setOverThresh);

    if (i == (frameID.size() - 1)) { // Always save last image
      exp[i].saveMe(true);
    }
    else {
      uint32 nextSet = frameID[i+1]/frameID.size();
      if (((lastSavedSet | curSet) == setOverThresh) ||
	  ((prevSet | curSet) == setOverThresh) ||
	  ((nextSet | curSet) == setOverThresh) ||
	  saveOverride) {
	exp[i].saveMe(true);
	saveOverride = false;
      }
      else if ((lastSavedSet | nextSet) == setOverThresh) {
	exp[i].saveMe(true);
	saveOverride = true;
      }
      else
	exp[i].saveMe(false);
    }

    if ((prevSet | curSet) == setOverThresh)
      exp[i].rateWarning(true);
    else
      exp[i].rateWarning(false);

    exp[i].entropy(_energyScores[current][lastSaved]);

    if (exp[i].saveMe()) {
      lastSaved = current;
      lastSavedSet = curSet;
    }

    prevSet = curSet;
  } // End of: for ( ...; i < frameID.size(); ...) 
}

void
UT_MotionAnalyzer::testSequence(vector<uint32>& frameID,
				vector<rcSharedFrameBufPtr>& sequence)
{
  /* Calculate the expected results.
   */
  vector<rcMotionAnalyzerResult> exp;

  calcExpectedValues(frameID, sequence, exp);

  rmAssert(exp.size() == sequence.size());

  /* Create motion analyzer and test it.
   */
  rcIPair tileDim((int32)sequence[0]->width(),
		  (int32)sequence[0]->height());
  rcRect frameGeom(0, 0, sequence[0]->width(), sequence[0]->height());

  for (uint32 block = 0; block < 2; block++) {
    utGrabber grabber(sequence);
    rcMotionAnalyzer analyzer(corrThreshold, tileDim, frameGeom, grabber);
    bool isBlocking = block == 1;
    
    for (uint32 i = 0; i < frameID.size(); ) {
      rcMotionAnalyzerResult act;

      rcFrameGrabberStatus status = analyzer.getNextResult(act, isBlocking);

      switch (status) {
      case eFrameStatusOK:
	rcUNITTEST_ASSERT(exp[i].entropy() == act.entropy());
	rcUNITTEST_ASSERT(exp[i].saveMe() == act.saveMe());
	rcUNITTEST_ASSERT(exp[i].frameBuf() == act.frameBuf());
	i++;
	break;

      case eFrameStatusNoFrame:
	rcUNITTEST_ASSERT(isBlocking == false);
	break;

      case eFrameStatusEOF:
      case eFrameStatusNotStarted:
      case eFrameStatusError:
	fprintf(stderr, "Invalid Status %d\n", status);
	rcUNITTEST_ASSERT(0);
	i++;
	break;
      } // End of: switch (status)
    } // End of: for (uint32 i = 0; i < frameID.size(); )
  } // End of: for (uint32 block = 0; block < 2; block++)
}

void
UT_MotionAnalyzer::genTestSequence(vector<uint32>& frameID,
				   vector<rcSharedFrameBufPtr>& sequence,
				   uint32 index)
{
  rmAssert(index <= imageCnt);
  rmAssert(index <= frameID.size());
  rmAssert(index <= sequence.size());

  if (index == sequence.size()) {
    testSequence(frameID, sequence);
    return;
  }

  for (uint32 i = 0; i < 3*imageCnt; i += imageCnt) {
    frameID[index] = i + index;
    rmAssert(frameID[index] < _frames.size());
    sequence[index] = _frames[frameID[index]];
    genTestSequence(frameID, sequence, index + 1);
  }
}

