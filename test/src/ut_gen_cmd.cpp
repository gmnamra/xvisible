
#include <stdio.h>
#include <ctype.h>
#include <rc_videocache.h>
#include <rc_gen_movie_file.h>
#include <rc_windowhist.h>
#include <rc_imageprocessing.h>
#include <rc_histstats.h>
#include <rc_ip.h>
#include <rc_musclesegment.h>

extern void rfPixel8Max3By3(const rcWindow& srcImg, rcWindow& dstImg);
extern void rfPixel8Min3By3(const rcWindow& srcImg, rcWindow& dstImg);

#define GCMD_MAX_MAXVALUES 40
#define GCMD_MAX_NAMES     2

enum gcmdType {
  gcmdStartFrame = 0,
  gcmdPeriodLth,
  gcmdPeriodCnt,
  gcmdFileNames,
  gcmdStdDevThresh,
  gcmdWindowSz,
  gcmdKineticType,
  gcmdAbsThresh,
  gcmdMorphology,
  gcmdDFT,
  gcmdImgStats,
  gcmdGaussSpatial,
  gcmdKineCorr,
  gcmdGenSpatVar,
  gcmdGenSpatVarSig,
  gcmdPreview,
  gcmdAutoThresh,
  gcmdEnergyAutoThresh,
  gcmdFindExtMuscCellFrm,
  gcmdFindPeriod,
  gcmdVerbose,
  gcmdWhatever,
  gcmdSegment,
  gcmdInvalid
};

typedef struct cmdDesc {
  gcmdType type;
  const char* txtDesc;
  const char* syntax;
  bool     stringInfo;
  bool     integral;
  int32  minIntValid;
  int32  maxIntValid;
  float    minRealValid;
  float    maxRealValid;
  uint32 minValues;
  char*    inputString;
} cmdDesc;

typedef struct cmdResult {
  gcmdType type;
  int32  valInt[GCMD_MAX_MAXVALUES];
  float    valReal[GCMD_MAX_MAXVALUES];
  uint32 valueCnt;
  char*    valString[GCMD_MAX_NAMES];
} cmdResult;

#define GCMD_CMD_CNT 23
#define INPUT_FILES "%s %s"
#define INPUT_INTS "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
#define INPUT_WIN_DIMS "%d %d %d %d"
#define INPUT_KINE_TYPES "%d %d %d %d %d"
#define INPUT_REALS "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f"

static cmdDesc genCmd[GCMD_CMD_CNT] = {
  {
    gcmdStartFrame, "Starting Frame", "SF:", false, true,
    0, rcINT32_MAX, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdPeriodLth, "Beat Period Length", "PL:", false, false,
    0, 0, 1.0, rcFLT_MAX, 1, INPUT_REALS
  },
  {
    gcmdPeriodCnt, "Beat Period Count", "PC:", false, true,
    1, rcINT32_MAX, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdFileNames, "Src/Dest File Names", "F:", true, false,
    0, 0, 0.0, 0.0, 2, INPUT_FILES
  },
  {
    gcmdStdDevThresh, "Std Dev Threshold", "SDT:",
    false, false, 0, 0, 0.0, rcFLT_MAX, 1, INPUT_REALS
  },
  {
    gcmdWindowSz, "Target Window X Off Y Off Width Height", "W:",
    false, true, 0, rcINT32_MAX, 0.0, 0.0, 4, INPUT_WIN_DIMS
  },
  {
    gcmdAbsThresh, "Absolute Threshold Pairs", "ATP:",
    false, false, 0, 0, 0.0, rcFLT_MAX, 0, INPUT_REALS
  },
  {
    gcmdDFT, "DFT Analysis 0/1 (Enable) [Window Size] 0/1 (Invert)",
    "DFT:", false, true, 0, 0x7FFFFFFF, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdKineticType, "Kinetic Type (Var, SD, Vel, Accel, Theta)", "KT:",
    false, true, 0, 4, 0.0, 0.0, 1, INPUT_KINE_TYPES
  },
  {
    gcmdMorphology, "Morphology 0 - D, 1 - C, 2 - E, 3 - O", "M:",
    false, true, 0, 4, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdImgStats, "Generate Image Statistics 0 - False, 1 -True", "ISTAT:",
    false, true, 0, 1, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdGaussSpatial, "Gaussian Smoothing Image Preprocessing (radius)", "G:",
    false, true, 0, 1000000, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdKineCorr, "1D Corr Kinetic Analysis (period pulseWidth)", "KC:",
    false, true, 1, 1000000, 0.0, 0.0, 2, INPUT_INTS
  },
  {
    gcmdGenSpatVar, "Generate Spatial Variation Image [radius]...", "SVAR:",
    false, true, 0, 1000000, 0.0, 0.0, 0, INPUT_INTS
  },
  {
    gcmdGenSpatVarSig,
    "Gen Spatial Var Sig (Radius, Frm, Min Val, Bin Sz, Bin Cnt) ", "SVS:",
    false, false, 0, 0, 0.0, rcFLT_MAX, 5, INPUT_REALS
  },
  {
    gcmdPreview, "Save Preview Frame", "PF:",
    false, true, 0, 1, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdFindExtMuscCellFrm,
    "Find Extended Muscle Index 0/1 (Enable) [Window Size] 0/1 (Invert)",
    "EMI:", false, true, 0, 1000000, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdFindPeriod, "Find Period 0/1 (Enable)", "FP:",
    false, true, 0, 1, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdVerbose, "Verbose 0/1 (Enable)", "V:",
    false, true, 0, 1, 0.0, 0.0, 1, INPUT_INTS
  },
  {
    gcmdWhatever, "Whatever you want", "WHAT:",
    false, false, 0, 0, -rcFLT_MAX, rcFLT_MAX, 0, INPUT_REALS
  },
  {
    gcmdAutoThresh, "Auto Threshold Min Val, Bin Sz, Bin Cnt ", "AT:",
    false, false, 0, 0, 0.0, rcFLT_MAX, 3, INPUT_REALS
  },
  {
    gcmdEnergyAutoThresh,
    "Energy Based Auto Thresh Min Val, Max Val, Bin Sz, Thresh Pct, EMI", "EAT:",
    false, false, 0, 0, 0.0, rcFLT_MAX, 5, INPUT_REALS
  },
  {
    gcmdSegment, "Muscle Cell Segmentation Max Frames Per Period", "SEG:",
    false, false, 0, 0, 0.0, rcFLT_MAX, 1, INPUT_REALS
  }
};

static char readBuffer[1024], srcFile[1024], destFile[1024];

static bool readCmd(FILE* cmdFile, cmdResult& res, uint32& lineNo)
{
  char* input;
  uint32 fnwi, lth;
  
  /* Find the next line that is neither blank nor a comment.
   */
  do {
    input = fgets(readBuffer, 1022, cmdFile);

    if (!input)
      return false;

    /* Caclulate 1st non-white index. Use this to skip blank lines.
     */
    lth = strlen(input);
    for (fnwi = 0; fnwi < lth; fnwi++)
      if (!isspace(input[fnwi]))
	break;
	
  } while ((fnwi == lth) || *input == '#');

  res.type = gcmdInvalid;

  if (lth > 1021) {
    fprintf(stderr, "Error @ line %d: Line too long: > 509 characters\n",
	    lineNo++);
    return true;
  }

  char* curLoc = input;
  uint32 cmdIndex;

  for (cmdIndex = 0; cmdIndex < GCMD_CMD_CNT; cmdIndex++) {
     uint32 cmdNameLth = strlen(genCmd[cmdIndex].syntax);
     if (strncmp(curLoc, genCmd[cmdIndex].syntax, cmdNameLth) == 0) {
       curLoc += cmdNameLth;
       res.type = genCmd[cmdIndex].type;
       break;
     }
  }

  if (res.type == gcmdInvalid) {
    fprintf(stderr, "Error @ line %d: Invalid Cmd: %s\n",
	    lineNo++, input);
    return true;
  }

  res.valueCnt = 0;

  if (genCmd[cmdIndex].stringInfo) {
    res.valueCnt = sscanf(curLoc, genCmd[cmdIndex].inputString,
			  res.valString[0], res.valString[1]);

    if (res.valueCnt < genCmd[cmdIndex].minValues) {
      fprintf(stderr, "Error @ line %d: Too few args (%d) for %s cmd: %s\n",
	      lineNo++, res.valueCnt, genCmd[cmdIndex].syntax, input);
      res.type = gcmdInvalid;
      return true;
    }
  }
  else {
    if (genCmd[cmdIndex].integral) {
      res.valueCnt = sscanf(curLoc, genCmd[cmdIndex].inputString,
			    &res.valInt[0], &res.valInt[1],
			    &res.valInt[2], &res.valInt[3],
			    &res.valInt[4], &res.valInt[5],
			    &res.valInt[6], &res.valInt[7],
			    &res.valInt[8], &res.valInt[9],
			    &res.valInt[10], &res.valInt[11],
			    &res.valInt[12], &res.valInt[13],
			    &res.valInt[14], &res.valInt[15],
			    &res.valInt[16], &res.valInt[17],
			    &res.valInt[18], &res.valInt[19],
			    &res.valInt[20], &res.valInt[21],
			    &res.valInt[22], &res.valInt[23],
			    &res.valInt[24], &res.valInt[25],
			    &res.valInt[26], &res.valInt[27],
			    &res.valInt[28], &res.valInt[29],
			    &res.valInt[30], &res.valInt[31],
			    &res.valInt[32], &res.valInt[33],
			    &res.valInt[34], &res.valInt[35],
			    &res.valInt[36], &res.valInt[37],
			    &res.valInt[38], &res.valInt[39]);
    }
    else {
      res.valueCnt = sscanf(curLoc, genCmd[cmdIndex].inputString,
			    &res.valReal[0], &res.valReal[1],
			    &res.valReal[2], &res.valReal[3],
			    &res.valReal[4], &res.valReal[5],
			    &res.valReal[6], &res.valReal[7],
			    &res.valReal[8], &res.valReal[9],
			    &res.valReal[10], &res.valReal[11],
			    &res.valReal[12], &res.valReal[13],
			    &res.valReal[14], &res.valReal[15],
			    &res.valReal[16], &res.valReal[17],
			    &res.valReal[18], &res.valReal[19],
			    &res.valReal[20], &res.valReal[21],
			    &res.valReal[22], &res.valReal[23],
			    &res.valReal[24], &res.valReal[25],
			    &res.valReal[26], &res.valReal[27],
			    &res.valReal[28], &res.valReal[29],
			    &res.valReal[30], &res.valReal[31],
			    &res.valReal[32], &res.valReal[33],
			    &res.valReal[34], &res.valReal[35],
			    &res.valReal[36], &res.valReal[37],
			    &res.valReal[38], &res.valReal[39]);
    }

    if (res.valueCnt < genCmd[cmdIndex].minValues) {
      fprintf(stderr, "Error @ line %d: Too few args (%d) for %s cmd: %s\n",
	      lineNo++, res.valueCnt, genCmd[cmdIndex].syntax, input);
      res.type = gcmdInvalid;
      return true;
    }

    if (genCmd[cmdIndex].integral) {
      for (uint32 i = 0; i < res.valueCnt; i++)
	if ((res.valInt[i] < genCmd[cmdIndex].minIntValid) ||
	    (res.valInt[i] > genCmd[cmdIndex].maxIntValid)) {
	  fprintf(stderr, "Error @ line %d: arg %d invalid"
		  " MIN %d MAX %d ACT %d for %s cmd: %s\n",
		  lineNo++, i,
		  genCmd[cmdIndex].minIntValid, genCmd[cmdIndex].maxIntValid,
		  res.valInt[i], genCmd[cmdIndex].syntax, input);
	  res.type = gcmdInvalid;
	  return true;
	}
    }
    else {
      for (uint32 i = 0; i < res.valueCnt; i++)
	if ((res.valReal[i] < genCmd[cmdIndex].minRealValid) ||
	    (res.valReal[i] > genCmd[cmdIndex].maxRealValid)) {
	  fprintf(stderr, "Error @ line %d: arg %d invalid"
		  " MIN %1.2f MAX %1.2e ACT %1.2e for %s cmd: %s\n",
		  lineNo++, i,
		  genCmd[cmdIndex].minRealValid, genCmd[cmdIndex].maxRealValid,
		  res.valReal[i], genCmd[cmdIndex].syntax, input);
	  res.type = gcmdInvalid;
	  return true;
	}
    }
  }

  lineNo++;
  return true;
}

static void genHist(rcWindow& histImg, const rcWindow& kineticImg,
		    rcOptoKineticImage& kinetic)
{
  const int32 width = histImg.width(), height = histImg.height();
  if (width < 514 || height < 100) {
    printf("Warning: Destination image too small. No histogram image created\n");
    return;
  }

  float minVal, maxVal, mean, stdDev, scale;

  kinetic.genKineticStats(kineticImg, minVal, maxVal, mean, stdDev);
  scale = maxVal - minVal;
  printf("STATS: minVal %f maxVal %f mean %f stdDev %f scale %f\n",
	 minVal, maxVal, mean, stdDev, scale);

  vector<float> linMap(255);

  for (uint32 i = 0; i < linMap.size(); i++) {
    linMap[i] = ((i+1)*scale)/256. + minVal;
  }

  kinetic.genEightBitImg(kineticImg, histImg, linMap);
  rc256BinHist linHist(256);
  rfGenDepth8Histogram(histImg, linHist);

  vector<float> logMap(255);
  const double exp = 2.0;
  double eToTheN = 1.0;
  
  for (uint32 i = 0; i < logMap.size(); i++) {
    eToTheN *= exp;
    logMap[i] = ((eToTheN-1)*scale)/eToTheN + minVal;
  }

  kinetic.genEightBitImg(kineticImg, histImg, logMap);
  rc256BinHist logHist(256);
  rfGenDepth8Histogram(histImg, logHist);

#if 0
  for (uint32 i = 0; i < logMap.size(); i++)
    printf("%d  Linear: %f Log base %f: %f\n", i, linMap[i], exp, logMap[i]);
#endif

  histImg.setAllPixels(255);
  int32 spacing = (width - 512 - 2)/4;
  
  uint32 maxHistVal = 0;
  for (uint32 i = 0; i < linHist.size(); i++)
    if (maxHistVal < linHist[i])
      maxHistVal = linHist[i];
  double printScale = ((double)(height - 2))/maxHistVal;

  printf("spacing %d max %d scale %f\nlin:", spacing, maxHistVal, printScale);
  for (int32 x = 0; x < (int32)linHist.size(); x++) {
    int32 y = (int32)(linHist[x] * printScale);
    rmAssert(y < height);
    rcWindow line(histImg, x + spacing, height - y, 1, y);
    printf(" %d", y);
    line.setAllPixels(0);
  }
  printf("\n");
  
  maxHistVal = 0;
  for (uint32 i = 0; i < logHist.size(); i++)
    if (maxHistVal < logHist[i])
      maxHistVal = logHist[i];
  printScale = ((double)(height - 2))/maxHistVal;

  printf("max %d scale %f\nlog:", maxHistVal, printScale);
  for (int32 x = 0; x < (int32)logHist.size(); x++) {
    int32 y = (int32)(logHist[x] * printScale);
    rmAssert(y < height);
    rcWindow line(histImg, x + 3*spacing + 256 + 2, height - y, 1, y);
    printf(" %d", y);
    line.setAllPixels(0);
  }
  printf("\n");
}


UT_kineticimage::UT_kineticimage(char* movieName) : _mName(movieName)
{
}

UT_kineticimage::~UT_kineticimage()
{
  printSuccessMessage( "KINETIC IMAGE test", mErrors );
}

uint32 UT_kineticimage::run()
{
  runGenerator();
  return mErrors;
}

void UT_kineticimage::runGenerator()
{
  FILE* info;
  if (!(info = fopen(_mName, "r"))) {
    perror("Cannot open command file");
    return;
  }

  cmdResult cmd;
  cmd.valString[0] = srcFile; cmd.valString[1] = destFile;
  uint32 lineNo = 0;

  vector<uint32> startFrames, periodCnt, morph;
  vector<int32> acRadius;
  vector<float> periodLth, stdDevThresh, absThreshMin, absThreshMax;
  vector<rcOptoKineticImage::kineType> kineTypes;
  bool dftEnabled = false, emiEnabled = false, energyInvert = false;
  bool fpEnabled = false;
  bool verbose = true;
  uint32 energyWinSz = 0;
  uint32 kine1DPeriod = 0, kine1DPulse = 0;
  uint32 gaussRadius = 0;
  bool statsEnabled = false;
  bool previewEnabled = false;
  char *processSrc = 0, *processDest = 0;
  uint32 winXOff = 0, winYOff = 0, winW = 0, winH = 0;
  bool eatEnabled = false;
  float eatMinVal = 0.0, eatMaxVal = 0.0, eatThreshPct = 0.0;
  uint32 eatSDFrmIndex = 0;
  uint32 svsSDFrmIndex = 0;
  bool svsEnabled = false;
  float svsMinBinVal = 0.0, svsBinSz = 0.0;
  int32 svsBinCnt = 0;
  float atMinBinVal = 0.0, atBinSz = 0.0;
  int32 atBinCnt = 0;
  float segValue = 0.0;
  bool segEnabled = false;
  
  while (readCmd(info, cmd, lineNo)) {

    switch (cmd.type)
    {
    case gcmdStartFrame:
      startFrames.resize(cmd.valueCnt);
      for (uint32 i = 0; i < cmd.valueCnt; i++)
	startFrames[i] = (uint32)cmd.valInt[i];
      break;

    case gcmdPeriodLth:
      periodLth.resize(cmd.valueCnt);
      for (uint32 i = 0; i < cmd.valueCnt; i++)
	periodLth[i] = cmd.valReal[i];
      break;

    case gcmdPeriodCnt:
      periodCnt.resize(cmd.valueCnt);
      for (uint32 i = 0; i < cmd.valueCnt; i++)
	periodCnt[i] = (uint32)cmd.valInt[i];
      break;

    case gcmdFileNames:
      processSrc = cmd.valString[0];
      processDest = cmd.valString[1];
      break;

    case gcmdStdDevThresh:
      stdDevThresh.resize(cmd.valueCnt);
      for (uint32 i = 0; i < cmd.valueCnt; i++)
	stdDevThresh[i] = cmd.valReal[i];
      break;

    case gcmdAbsThresh:
      if (cmd.valueCnt < 2) {
	absThreshMin.clear();
	absThreshMax.clear();
      }
      else {
	uint32 cnt = cmd.valueCnt/2;
	for (uint32 i = 0; i < cnt; i++) {
	  absThreshMin.push_back(cmd.valReal[i*2]);
	  absThreshMax.push_back(cmd.valReal[i*2+1]);
	}
      }
      break;

    case gcmdDFT:
      dftEnabled = (bool)cmd.valInt[0];
      printf("dft enabled %d %d 0x%X\n", cmd.valInt[0], cmd.valInt[1],
	     cmd.valInt[1]);
      if (dftEnabled) {
	if (cmd.valueCnt > 1)
	  energyWinSz = (uint32)cmd.valInt[1];
	else
	  energyWinSz = 0;
	if (cmd.valueCnt > 2)
	  energyInvert = (bool)cmd.valInt[2];
	else
	  energyInvert = false;
      }
      else {
	energyWinSz = 0;
	energyInvert = false;
      }
      break;

    case gcmdFindExtMuscCellFrm:
      emiEnabled = (bool)cmd.valInt[0];
      if (emiEnabled) {
	if (cmd.valueCnt > 1)
	  energyWinSz = (uint32)cmd.valInt[1];
	else
	  energyWinSz = 0;
	if (cmd.valueCnt > 2)
	  energyInvert = (bool)cmd.valInt[2];
	else
	  energyInvert = false;
      }
      else {
	energyWinSz = 0;
	energyInvert = false;
      }
      break;

    case gcmdFindPeriod:
      fpEnabled = (bool)cmd.valInt[0];
      break;

    case gcmdVerbose:
      verbose = (bool)cmd.valInt[0];
      break;

    case gcmdPreview:
      previewEnabled = (bool)cmd.valInt[0];
      break;

    case gcmdGaussSpatial:
      gaussRadius = (uint32)cmd.valInt[0];
      break;

    case gcmdKineCorr:
      kine1DPeriod = (uint32)cmd.valInt[0];
      kine1DPulse = (uint32)cmd.valInt[1];
      if (kine1DPeriod < kine1DPulse || !kine1DPeriod || !kine1DPulse)
	kine1DPeriod = kine1DPulse = 0;
      else {
	kineTypes.resize(1);
	kineTypes[0] = rcOptoKineticImage::eKineTypeDummy;
      }
      break;

    case gcmdAutoThresh:
      atMinBinVal = cmd.valReal[0];
      atBinSz = cmd.valReal[1];
      atBinCnt = (int32)cmd.valReal[2];
      eatEnabled = false;
      break;

    case gcmdEnergyAutoThresh:
      eatMinVal = cmd.valReal[0];
      eatMaxVal = cmd.valReal[1];
      atBinSz = cmd.valReal[2];
      eatThreshPct = cmd.valReal[3];
      eatSDFrmIndex = (uint32)(cmd.valReal[4]);
      eatEnabled = true;
      break;

    case gcmdImgStats:
      statsEnabled = (bool)cmd.valInt[0];
      break;

    case gcmdWindowSz:
      winXOff = (uint32)cmd.valInt[0];
      winYOff= (uint32)cmd.valInt[1];
      winW = (uint32)cmd.valInt[2];
      winH = (uint32)cmd.valInt[3];
      break;

    case gcmdKineticType:
      kineTypes.resize(cmd.valueCnt);
      for (uint32 i = 0; i < cmd.valueCnt; i++)
	kineTypes[i] = (rcOptoKineticImage::kineType)cmd.valInt[i];
      break;

    case gcmdGenSpatVarSig:
      acRadius.resize(1);
      acRadius[0] = (int32)cmd.valReal[0];
      svsSDFrmIndex = (uint32)cmd.valReal[1];
      svsMinBinVal = cmd.valReal[2];
      svsBinSz = cmd.valReal[3];
      svsBinCnt = (int32)cmd.valReal[4];
      svsEnabled = true;
      break;

    case gcmdSegment:
      segValue = cmd.valReal[0];
      segEnabled = segValue > 0.0;
      break;

    case gcmdGenSpatVar:
      acRadius.resize(cmd.valueCnt);
      for (uint32 i = 0; i < cmd.valueCnt; i++)
	acRadius[i] = (int32)cmd.valInt[i];
      svsEnabled = false;
      break;

    case gcmdMorphology:
      morph.resize(cmd.valueCnt);
      for (uint32 i = 0; i < cmd.valueCnt; i++)
	morph[i] = (uint32)cmd.valInt[i];
      break;

    case gcmdInvalid:
#if 0
      fprintf(stderr, "Error @ line %d\n", lineNo-1);
#endif
      break;

    case gcmdWhatever:
      if (cmd.valueCnt < 6)
	printf("Too few args %d\n", cmd.valueCnt);
      else {
	printf("Creating dummy movie\n");
	rcGenMovieFile destMovie("/Users/proberts/temp.rfymov",
				 movieOriginSynthetic, "Test Code",
				 movieFormatRevLatest, true, 30);
	rcWindow temp(640, 480);
	temp.setAllPixels(0);
	rcWindow tWin(temp, 100, 100, 150, 50);
	tWin.setAllPixels(65);
	destMovie.addFrame(temp);
	tWin.setAllPixels(64);
	destMovie.addFrame(temp);
	tWin.setAllPixels(255);
	destMovie.addFrame(temp);
	destMovie.addFrame(temp);
	tWin.setAllPixels(128);
	for (uint32 i = 4; i < 15; i++)
	  destMovie.addFrame(temp);
	destMovie.flush();
      }
      break;

    default:
      rmAssert(0);
    } // End of: switch (cmd.type)
   
    if (processSrc) {
      if (verbose) {
	printf("Line: %d\nSF:", lineNo-1);
	for (uint32 i = 0; i < startFrames.size(); i++)
	  printf(" %d", startFrames[i]);
	printf("\nPC:");
	for (uint32 i = 0; i < periodCnt.size(); i++)
	  printf(" %d", periodCnt[i]);
	printf("\nPL:");
	for (uint32 i = 0; i < periodLth.size(); i++)
	  printf(" %f", periodLth[i]);
	printf("\nSDT:");
	for (uint32 i = 0; i < stdDevThresh.size(); i++)
	  printf(" %1.2e", stdDevThresh[i]);
	printf("\nDFT: %s Window Sz %d Invert %s",
	       dftEnabled ? "ENABLED" : "DISABLED",
	       energyWinSz,
	       energyInvert ? "ENABLED" : "DISABLED");
	printf("\nEMI: %s Window Sz %d Invert %s",
	       emiEnabled ? "ENABLED" : "DISABLED",
	       energyWinSz,
	       energyInvert ? "ENABLED" : "DISABLED");
	printf("\nFP: %s", fpEnabled ? "ENABLED" : "DISABLED");
	printf("\nAT: %s min bin val %f bin sz %f bin cnt %d",
	       ((atBinSz && !eatEnabled) ? "ENABLED" : "DISABLED"),
	       atMinBinVal, atBinSz, atBinCnt);
	printf("\nEAT: %s min val %f max val %f bin sz %f thresh pct %f emi %d",
	       ((atBinSz && eatEnabled) ? "ENABLED" : "DISABLED"),
	       eatMinVal, eatMaxVal, atBinSz, eatThreshPct, eatSDFrmIndex);
	printf("\nPF: %s", previewEnabled ? "ENABLED" : "DISABLED");
	printf("\nSEG: %s mfpp %f", (segEnabled ? "ENABLED" : "DISABLED"),
	       segValue);
	printf("\nISTAT: %s", statsEnabled ? "ENABLED" : "DISABLED");
	printf("\nKT:");
	for (uint32 i = 0; i < kineTypes.size(); i++)
	  printf(" %d", kineTypes[i]);
	printf("\nKC: Period %d Pulse Lth %d", kine1DPeriod, kine1DPulse);
	printf("\nM:");
	for (uint32 i = 0; i < morph.size(); i++)
	  printf(" %d", morph[i]);
	printf("\nG: Radius %d", gaussRadius);
	printf("\nATP:");
	for (uint32 i = 0; i < absThreshMin.size(); i++)
	  printf(" (%1.2e %1.2e)", absThreshMin[i], absThreshMax[i]);
	printf("\nSVAR:");
	if (!svsEnabled) {
	  for (uint32 i = 0; i < acRadius.size(); i++)
	    printf(" %d", acRadius[i]);
	}
	printf("\nSVS:");
	if (svsEnabled)
	  printf(" Radius %d SD Index %d min val %f bin sz %f bin cnt %d",
		 acRadius[0], svsSDFrmIndex, svsMinBinVal, svsBinSz, svsBinCnt);
	printf("\nSrc: %s Dest: %s\n", processSrc, processDest);
	printf("Window: (%d, %d) %d X %d\n\n", winXOff, winYOff, winW, winH);
      }
      else
	printf("SRC: %s\n", processSrc);

      rmAssert(processDest);

      /* Step 1 - Script has specified a new file to work on. Open
       * files, set up defaults and do any final parameter validation.
       */
      bool energyEnabled = dftEnabled | emiEnabled;
      if (kineTypes.empty() && acRadius.empty() &&!energyEnabled
	  && !statsEnabled && !svsEnabled && ! segEnabled) {
	fprintf(stderr, "Error @ line %d: No Analysis Types Specified\n", lineNo-1);
	processSrc = processDest = 0;
	continue;
      }
      else if (!kineTypes.empty() && !acRadius.empty()) {
	fprintf(stderr, "Error @ line %d: Both Kinetic and "
		"Spatial Analyses Specified\n", lineNo-1);
	processSrc = processDest = 0;
	continue;
      }
      else if (dftEnabled && emiEnabled) {
	fprintf(stderr, "Error @ line %d: Both DFT and "
		"EMI Analyses Specified\n", lineNo-1);
	processSrc = processDest = 0;
	continue;
      }
      vector<rcOptoKineticImage::kineType> kt(kineTypes);
      vector<int32> rad(acRadius);

      rcVideoCache* srcMovie =
	rcVideoCache::rcVideoCacheCtor(processSrc, 20, true, verbose);
      rmAssert(srcMovie);
      if (!srcMovie->isValid()) {
	fprintf(stderr, "Error @ line %d: Cannot open movie %s\n", lineNo-1,
		processSrc);
	processSrc = processDest = 0;
	rcVideoCache::rcVideoCacheDtor(srcMovie);
	continue;
      }

      vector<uint32> sf(startFrames), pc(periodCnt);
      vector<float> pl(periodLth);
      if (sf.empty())
	sf.push_back(0);

      if (pc.empty())
	pc.push_back(1);

      if (pl.empty())
	pl.push_back(60.0);

      vector<float> sdt(stdDevThresh);
      if (sdt.empty())
	sdt.push_back(1.0);

      uint32 xo, yo, w, h;

      if ((winXOff | winYOff | winW | winH) == 0) { // If not set, use defaults
	xo = yo = 0;
	w = srcMovie->frameWidth();
	h = srcMovie->frameHeight();
      }
      else {
	if ((winW == 0) || ((winXOff + winW) >= srcMovie->frameWidth()) ||
	    (winH == 0) || ((winYOff + winH) >= srcMovie->frameHeight())) {
	  fprintf(stderr, "Error @ line %d: Invalid AOI dimensions "
		  "(%d, %d) %d X %d, for movie %d X %d\n", lineNo-1,
		  winXOff, winYOff, winW, winH, srcMovie->frameWidth(),
		  srcMovie->frameHeight());
	  processSrc = processDest = 0;
	  rcVideoCache::rcVideoCacheDtor(srcMovie);
	  continue;
	}
	xo = winXOff;
	yo = winYOff;
	w = winW;
	h = winH;
      }

      rcGenMovieFile destMovie(processDest, movieOriginSynthetic, "Test Code",
			       movieFormatRevLatest, true, 30);
      uint32 dFrmCnt = 0; // Since rcGenMovieFile API doesn't support this

      rcRect cropRect(xo, yo, w, h);
      vector<rcWindow> focus;
      for (uint32 fi = 0; fi < srcMovie->frameCount(); fi++) {
	rcWindow frame(*srcMovie, fi);
	if (gaussRadius) {
	  rcWindow frameWin(frame, cropRect);
	  rcWindow gaussedWin(cropRect.size());
	  rfGaussianConv(frameWin, gaussedWin, gaussRadius*2 + 1);
	  focus.push_back(gaussedWin);
	}
	else
	  focus.push_back(rcWindow(frame, cropRect));

      }

      if (previewEnabled) {
	rcWindow temp(cropRect.size());
	temp.copyPixelsFromWindow(focus[0]);
	destMovie.addFrame(temp);
	dFrmCnt++;
      }

      vector<uint8> pMap(256);
      for (uint32 pi = 1; pi < 255; pi++)
	pMap[pi] = 128;
      pMap[0] = 0;
      pMap[255] = 255;

      /* Step 2 - Analyze new file.
       */
      uint32 analysisSz = kt.size() ? kt.size() : rad.size();

      uint32 azEnd = energyEnabled ? 1 : (analysisSz ? analysisSz : 1);

      for (uint32 azi = 0; azi < azEnd; azi++)
	for (uint32 sfi = 0; sfi < sf.size(); sfi++) {
	  const uint32 startFrame = sf[sfi];
	  for (uint32 pli = 0; pli < pl.size(); pli++)
	    for (uint32 pci = 0; pci < pc.size(); pci++) {
	      uint32 frameCount = (uint32)(pl[pli]*pc[pci] + 0.5);
	      if (frameCount < 2) {
		fprintf(stderr, "Error @ line %d: Period combination"
			" (PC * PL) == %d < 2\n", lineNo-1, frameCount);
		continue;
	      }
	      else if ((startFrame + frameCount) > srcMovie->frameCount()) {
		fprintf(stderr, "Error @ line %d: Period combination"
			" (PC * PL) == %d > Movie Lth %d\n", lineNo-1,
			frameCount, srcMovie->frameCount());
		continue;
	      }

	      const uint32 endFrame = startFrame + frameCount;

	      /* Whoopee! Ready to analyze movies.
	       */
	      if (segEnabled) {
		rcMuscleSegmenter segmenter(&destMovie);
		if ((startFrame == 0) && (endFrame == 15) &&
		    (focus.size() == 15)) {
		  const uint32 sdIndex = 2, freqIndex = 3, imgIndex = 1;
		  rcWindow segSdWin(focus[sdIndex], 1, 1,
				    focus[sdIndex].width()-2,
		                    focus[sdIndex].height()-2);
		  rcWindow segFreqWin(focus[freqIndex], 1, 1,
				      focus[freqIndex].width()-2,
				      focus[freqIndex].height()-2);
		  segmenter.hackInsertSegmentedImages(segSdWin, segFreqWin,
						      focus[imgIndex]);
		}
		segmenter.segment(focus, (int32)startFrame,(int32)endFrame);

		const double minCellArea = 15.0*15.0;
		segmenter.filterBySize(minCellArea);
		
		const double minAspectRatio = 1.5;
		segmenter.filterByAR(minAspectRatio);

		const int32 minDelta = 18;
		segmenter.filterAtEdge(minDelta);

		const double minActivePct = 0.25;
		segmenter.filterByActivity(minActivePct, focus,
					   (int32)startFrame,
					   (int32)endFrame, segValue);

		vector<rcPolygon> cells;
		segmenter.getCells(cells);

		continue;
	      }

	      rcOptoKineticImage kinetic((kt.size() == 0) ?
					 rcOptoKineticImage::eKineTypeThetaEntropy :
					 kt[azi]);

	      for (uint32 fi = startFrame; fi < endFrame; fi++)
		kinetic.push(focus[fi]);

	      if (statsEnabled) {
		fprintf(stderr, "IMAGE STATS:\n");
		double minMean = 1e99, maxMean = -1;
		for (uint32 fi = startFrame; fi < endFrame; fi++) {
		  rcHistoStats s(focus[fi]);
		  fprintf(stderr, "%d: n %d min %d max %d median %d "
			  "mean %f sd %f var %f\n",
			  fi, s.n(), s.min(), s.max(), s.median(),
			  s.mean(), s.sDev(), s.var());
		  if (s.mean() < minMean)
		    minMean = s.mean();
		  if (s.mean() > maxMean)
		    maxMean = s.mean();
		}
		fprintf(stderr, "MAX MEAN %f - MIN MEAN %f = %f\n",
			maxMean, minMean, maxMean-minMean);

		if (kt.size() == 0)
		  continue;
	      }

	      rcWindow processedImg, mappedImg(cropRect.size());
	      if (analysisSz) { //Then generate either kinetic or spatial image
		if (kine1DPeriod) {
		  rcWindow phaseImg(cropRect.size());
		  processedImg = rcWindow(cropRect.size(), rcPixel32);
		  rcWindow processedImg2(cropRect.size(), rcPixel32);
		  kinetic.gen1DCorrKineticImgs(kine1DPeriod, kine1DPulse,
					       mappedImg, phaseImg,
					       processedImg, processedImg2);

		  destMovie.addFrame(mappedImg);
		  dFrmCnt++;
		  destMovie.addFrame(phaseImg);
		  dFrmCnt++;
		  for (int32 y = 0; y < processedImg.height(); y++) {
		    float* destP = (float*)processedImg.rowPointer(y);
		    uint8* kP = (uint8*)mappedImg.rowPointer(y);
		    uint8* pP = (uint8*)phaseImg.rowPointer(y);
		    for (int32 x = 0; x < processedImg.width(); x++) {
		      *destP++ = (float)(*kP++) * (float)(*pP++);
		    }
		  }
		      
		}
		else if (kt.size()) {
		  processedImg = rcWindow(cropRect.size(), rcPixel32);
		  kinetic.genKineticImg(processedImg);
		}
		else {
		  rcIPair searchSz(cropRect.width() - 2*(rad[azi]+1),
				   cropRect.height() - 2*(rad[azi]+1));
		  processedImg = rcWindow(searchSz, rcPixel32);
		  uint32 sdFrame = startFrame;
		  if (eatEnabled && (eatSDFrmIndex >= startFrame) &&
		      (eatSDFrmIndex < endFrame))
		    sdFrame = eatSDFrmIndex;
		  else if (svsEnabled && (svsSDFrmIndex >= startFrame) &&
			   (svsSDFrmIndex < endFrame))
		    sdFrame = svsSDFrmIndex;

		  printf("SD frame %d\n", sdFrame);
		  genSpatialSDImg(focus[sdFrame], processedImg, rad[azi]);

#if 0
		  double maxVal = -1.0;
		  uint32 maxLoc = 0;
		  for (uint32 fi = startFrame; fi < endFrame; fi++) {
		    double val = genSpatialSDSum(focus[fi], rad[azi]);
		    if (val > maxVal) {
		      maxVal = val;
		      maxLoc = fi;
		    }
		  }
		  printf("MAX SPATIAL SD @%d: %f\n", maxLoc, maxVal);
#endif
		}
		
		float minVal, maxVal, mean, stdDev;
		kinetic.genKineticStats(processedImg, minVal, maxVal,  mean,
					stdDev);
		printf("minVal %f maxVal %f mean %f stdDev %f\n\n",
		       minVal, maxVal, mean, stdDev);

		if (svsEnabled) {
		  rcWindow edgeImg(processedImg.width(), processedImg.height(),
				   rcPixel32);

		  vector<uint32> pixelCnt(endFrame-startFrame);
		  uint32 maxPixelCnt = 0;
		  for (uint32 i = startFrame; i < endFrame; i++) {
		    genSpatialSDImg(focus[i], edgeImg, rad[azi]);
		    float segVal = segPoint(edgeImg, svsMinBinVal, svsBinSz,
					    svsBinCnt, true);

		    printf("Seg Val[%d] = %f\n", i, segVal);
		    uint32 pCnt = 0;
		    for (int32 y = 0; y < edgeImg.height(); y++) {
		      float* ePtr = (float*)edgeImg.rowPointer(y);
		      for (int32 x = 0; x < edgeImg.width(); x++, ePtr++) {
			if (*ePtr > segVal)
			  pCnt++;
		      }
		    }
		    pixelCnt[i-startFrame] = pCnt;
		    if (pCnt > maxPixelCnt)
		      maxPixelCnt = pCnt;
		  }
		  printf("maxPixelCnt %d\nInput signal:\n", maxPixelCnt);
		  for (uint32 i = 0; i < pixelCnt.size(); i++)
		    printf("%d: %0.25f\n", i, ((float)pixelCnt[i])/maxPixelCnt);
		} // End if: if (svsEnabled) {
		else if (atBinSz > 0.0 &&
		    (
		     ((atBinCnt > 1) && !eatEnabled) ||
		     (eatEnabled && (eatMaxVal > eatMinVal) &&
		      (eatThreshPct >= 0.0) && (eatThreshPct <= 1.0)
		      )
		     )
		    ) {
		  float binPoint;
		  if (eatEnabled) {
		    vector<rcWindow> movieWin;
		    rmAssert(focus.size());
		    int32 w = processedImg.width();
		    int32 h = processedImg.height();
		    int32 xo = (focus[0].width() - w)/2;
		    int32 yo = (focus[0].height() - h)/2;
		    for (uint32 i = startFrame; i < endFrame; i++) {
		      rcWindow tempWin(focus[i], xo, yo, w, h);
		      movieWin.push_back(tempWin);
		    }
		    binPoint = segPointByEnergy(processedImg, eatMinVal,
						eatMaxVal, atBinSz,
						eatThreshPct, movieWin);
		  }
		  else
		    binPoint = segPoint(processedImg, atMinBinVal,
					atBinSz, atBinCnt, false);

		  printf("Segmentation Point: %f\n", binPoint);
		  
		  rcWindow mappedWin;
		  if (kt.size())
		    mappedWin = mappedImg;
		  else {
		    mappedImg.setAllPixels(0);
		    mappedWin = rcWindow(mappedImg, rad[azi]+1, rad[azi]+1,
					 cropRect.width() - 2*(rad[azi]+1), 
					 cropRect.height() - 2*(rad[azi]+1));
		  }
		  
		  vector<float> vMap(255);
		  for(uint32 i = 0; i < 255; i++)
		    vMap[i] = binPoint;
		  
		  kinetic.genEightBitImg(processedImg, mappedWin, vMap);
		  for (uint32 mi = 0; mi < morph.size(); mi++) {
		    rcWindow temp(mappedWin.width()-2, mappedWin.height()-2);
		    rcWindow mWin(mappedWin, 1, 1,
				  mappedWin.width()-2, mappedWin.height()-2);
		    switch (morph[mi]) {
		    case 0: // Dilate
		      rfPixel8Max3By3(mappedWin, temp);
		      break;
		      
		    case 1: // Close
		      rfPixel8Max3By3(mappedWin, temp);
		      mWin.copyPixelsFromWindow(temp);
		      rfPixel8Min3By3(mappedWin, temp);
		      break;
		      
		    case 2: // Erode
		      rfPixel8Min3By3(mappedWin, temp);
		      break;
		      
		    case 3: // Open
		      rfPixel8Min3By3(mappedWin, temp);
		      mWin.copyPixelsFromWindow(temp);
		      rfPixel8Max3By3(mappedWin, temp);
		      break;
		      
		    default:
		      rmAssert(0);
		      break;
		    }
		    mWin.copyPixelsFromWindow(temp);
		  }
		  destMovie.addFrame(mappedImg);
		  dFrmCnt++;
		} // End of: else if (atBinSz > 0.0 && ... )
		else if (absThreshMin.empty()) {
		  for (uint32 sdti = 0; sdti < sdt.size(); sdti++) {
		    rcWindow mappedWin;
		    if (kt.size())
		      mappedWin = mappedImg;
		    else {
		      mappedImg.setAllPixels(0);
		      mappedWin = rcWindow(mappedImg, rad[azi]+1, rad[azi]+1,
					   cropRect.width() - 2*(rad[azi]+1), 
					   cropRect.height() - 2*(rad[azi]+1));
		    }
		    
		    kinetic.genEightBitImg(processedImg, mappedWin,
					   sdt[sdti], 0);
		    for (uint32 mi = 0; mi < morph.size(); mi++) {
		      rcWindow temp(mappedWin.width()-2, mappedWin.height()-2);
		      rcWindow mWin(mappedWin, 1, 1,
				    mappedWin.width()-2, mappedWin.height()-2);
		      switch (morph[mi]) {
		      case 0: // Dilate
			rfPixel8Max3By3(mappedWin, temp);
			break;
			
		      case 1: // Close
			rfPixel8Max3By3(mappedWin, temp);
			mWin.copyPixelsFromWindow(temp);
			rfPixel8Min3By3(mappedWin, temp);
			break;
			
		      case 2: // Erode
			rfPixel8Min3By3(mappedWin, temp);
			break;
			
		      case 3: // Open
			rfPixel8Min3By3(mappedWin, temp);
			mWin.copyPixelsFromWindow(temp);
			rfPixel8Max3By3(mappedWin, temp);
			break;
			
		      default:
			rmAssert(0);
			break;
		      }
		      mWin.copyPixelsFromWindow(temp);
		    }
		    destMovie.addFrame(mappedImg);
		    dFrmCnt++;
#if 0
		    rcWindow histImg(cropRect.size());
		    genHist(histImg, processedImg, kinetic);
		    destMovie.addFrame(histImg);
		    dFrmCnt++;
#endif
		  } // End of: for (uint32 sdti = 0; sdti < sdt.size(); sdti++) {
		} // End of: else if (absThreshMin.empty()) {
		else {
		  for (uint32 ati = 0; ati < absThreshMin.size(); ati++) {
		    rcWindow mappedWin;
		    if (kt.size())
		      mappedWin = mappedImg;
		    else {
		      mappedImg.setAllPixels(0);
		      mappedWin = rcWindow(mappedImg, rad[azi]+1, rad[azi]+1,
					   cropRect.width() - 2*(rad[azi]+1), 
					   cropRect.height() - 2*(rad[azi]+1));
		    }
		    
		    vector<float> vMap(255);
		    for(uint32 i = 0; i < 128; i++)
		      vMap[i] = absThreshMin[ati];
		    for(uint32 i = 128; i < 255; i++)
		      vMap[i] = absThreshMax[ati];
		    
		    kinetic.genEightBitImg(processedImg, mappedWin, vMap);
		    for (uint32 mi = 0; mi < morph.size(); mi++) {
		      rcWindow temp(mappedWin.width()-2, mappedWin.height()-2);
		      rcWindow mWin(mappedWin, 1, 1,
				    mappedWin.width()-2, mappedWin.height()-2);
		      switch (morph[mi]) {
		      case 0: // Dilate
			rfPixel8Max3By3(mappedWin, temp);
			break;
			
		      case 1: // Close
			rfPixel8Max3By3(mappedWin, temp);
			mWin.copyPixelsFromWindow(temp);
			rfPixel8Min3By3(mappedWin, temp);
			break;
			
		      case 2: // Erode
			rfPixel8Min3By3(mappedWin, temp);
			break;
			
		      case 3: // Open
			rfPixel8Min3By3(mappedWin, temp);
			mWin.copyPixelsFromWindow(temp);
			rfPixel8Max3By3(mappedWin, temp);
			break;
			
		      default:
			rmAssert(0);
			break;
		      }
		      mWin.copyPixelsFromWindow(temp);
		    }
		    destMovie.addFrame(mappedImg);
		    dFrmCnt++;
#if 0
		    rcWindow histImg(cropRect.size());
		    genHist(histImg, processedImg, kinetic);
		    destMovie.addFrame(histImg);
		    dFrmCnt++;
#endif
		  } // End of: for (... ; ati < absThreshMin.size(); ...) {
		} // End of: if (absThreshMin.empty()) .. else ...
	      } // End of: if (analysisSz)
	      
	      if (energyEnabled) {
		deque<double> signal;
		uint32 sinWinSz = (energyWinSz > 1) ? energyWinSz : frameCount;
		if (sinWinSz > frameCount)
		  sinWinSz = frameCount;

		if (energyWinSz != 0x7FFFFFFF) {
		  
		  /* First calculate the nominal rate at which cells are
		   * beating by generating the exhaustive energy curve
		   * for the sample period and then performing the FFT
		   * on this curve.
		   */
		  rcSimilarator energy(rcSimilarator::eExhaustive,
				       rcPixel8, sinWinSz, 0);
		  
		  if (analysisSz) {
		    if (energyInvert) {
		      for (int32 y = 0; y < mappedImg.height(); y++)
			for (int32 x = 0; x < mappedImg.width(); x++) {
			  uint32 val = mappedImg.getPixel(x, y);
			  if (val > 0)
			    mappedImg.setPixel(x, y, 0x00);
			  else
			    mappedImg.setPixel(x, y, 0xFF);
			}
		    }
		    else {
		      for (int32 y = 0; y < mappedImg.height(); y++)
			for (int32 x = 0; x < mappedImg.width(); x++) {
			  uint32 val = mappedImg.getPixel(x, y);
			  if (val && val != 0xFF)
			    mappedImg.setPixel(x, y, 0xFF);
			}
		    }
		    destMovie.addFrame(mappedImg);
		    dFrmCnt++;
		    energy.setMask(mappedImg);
		    
		  }
		  vector<rcWindow> eMovie;
		  
		  if (sinWinSz == frameCount) {
		    for (uint32 fi = startFrame; fi < endFrame; fi++)
		      eMovie.push_back(focus[fi]);
		    energy.fill(eMovie);
		    
		    energy.entropies(signal, rcSimilarator::eVisualEntropy);
		  }
		  else {
		    uint32 fi;
		    uint32 lastWinFrame = startFrame + sinWinSz;
		    for (fi = startFrame; fi < lastWinFrame; fi++)
		      eMovie.push_back(focus[fi]);
		    
		    bool rval = energy.fill(eMovie);
		    rmAssert(rval);
		    
		    const uint32 targetValue = sinWinSz/2;
		    deque<double> tempSig;
		    energy.entropies(tempSig, rcSimilarator::eVisualEntropy);
		    signal.push_back(tempSig[targetValue]);
		    
		    for ( ; fi < endFrame; fi++) {
		      rval = energy.update(focus[fi]);
		      rmAssert(rval);
		      
		      energy.entropies(tempSig, rcSimilarator::eVisualEntropy);
		      signal.push_back(tempSig[targetValue]);
		    }
		  } // End of: if (sinWinSz == frameCount) ... else ...
		} // End of: if (energyWinSz != 0x7FFFFFFF) {

		if (dftEnabled) {
		  if (energyWinSz != 0x7FFFFFFF) {
		    /* Generate FT data.
		     */
		    const float rkDegree = 180/rkPI;
		    rs1DFourierResult rslt;
#if 0
		    const uint32 ctrlCnt = 6;
		    uint32 ctrl[ctrlCnt] = {
		      0, rc1DFourierRaisedCosine,
		      rc1DFourierForceFFT,
		      rc1DFourierForceFFT|rc1DFourierRaisedCosine,
		      rc1DFourierForceFFT|rc1DFourierZeroFill,
		      rc1DFourierForceFFT|rc1DFourierZeroFill|rc1DFourierRaisedCosine
		    };
		    char* ctrlInfo[ctrlCnt] = {
		      "No Pad No Raised Cosine", "No Pad Raised Cosine",
		      "Pad No Raised Cosine", "Pad Raised Cosine",
		      "Double Pad No Raised Cosine", "Double Pad Raised Cosine"
		    };
#else
		    const uint32 ctrlCnt = 1;
		    uint32 ctrl[ctrlCnt] = {
		      rc1DFourierForceFFT|rc1DFourierRaisedCosine
		    };
		    char* ctrlInfo[ctrlCnt] = {
		      "Pad Raised Cosine"
		    };
#endif
		    for (uint32 i = 0; i < ctrlCnt; i++) {
		      rf1DFourierAnalysis(signal, rslt, ctrl[i]);
#if 0
		      for (int32 ii = 0; ii < rslt.workingSz/2; ii++)
			fprintf(stderr, "%d: %0.20f %0.20f\n", ii+1,
				rslt.real[ii], rslt.imag[ii]);
#endif
		      fprintf(stderr, "%s: MAX: FREQ %f PHASE %f AMPLITUDE %f "
			      "FRAMES PER PERIOD %f DC %f\n\n", ctrlInfo[i],
			      rslt.peakFrequency, rslt.peakPhase*rkDegree,
			      rslt.peakAmplitude, rslt.workingSz/rslt.peakFrequency,
			      rslt.amplitude[0]);
		      vector<uint32> peakLocs;
		      vector<float> interLocs, interVals;
		      rf1DPeakDetect(rslt.amplitude, peakLocs, interLocs,
				     interVals, 0.10);
		      fprintf(stderr, "Peak Amplitudes:\n");
		      for (uint32 ii = 0; ii < peakLocs.size(); ii++) {
			fprintf(stderr, " (%d %f) inter: %f", peakLocs[ii],
				rslt.amplitude[peakLocs[ii]],
				interLocs[ii]);
			if (ii)
			fprintf(stderr, " diff %f\n",
				interLocs[ii] - interLocs[ii-1]);
			else
			  fprintf(stderr, " diff %f\n", interLocs[ii]);
		      }
		      fprintf(stderr, "\n");
		    }
		  }
		  else {
		    printf("Making freq images\n");
		    rcWindow freqImg(cropRect.size(), rcPixel32);
		    kinetic.genFreqImg(freqImg, pl[pli]);
		    rmPrintFloatImage(freqImg);
		    
		    if (absThreshMin.size()) {
		      for (uint32 ati = 0; ati < absThreshMin.size(); ati++) {
			rcWindow mappedWin(cropRect.size());
			vector<float> vMap(255);
			for(uint32 i = 0; i < 255; i++)
			  vMap[i] = absThreshMin[ati];

			kinetic.genEightBitImg(freqImg, mappedWin, vMap);
			for (uint32 mi = 0; mi < morph.size(); mi++) {
			  rcWindow temp(mappedWin.width()-2, mappedWin.height()-2);
			  rcWindow mWin(mappedWin, 1, 1,
					mappedWin.width()-2, mappedWin.height()-2);
			  switch (morph[mi]) {
			  case 0: // Dilate
			    rfPixel8Max3By3(mappedWin, temp);
			    break;
			    
			  case 1: // Close
			    rfPixel8Max3By3(mappedWin, temp);
			    mWin.copyPixelsFromWindow(temp);
			    rfPixel8Min3By3(mappedWin, temp);
			    break;
			    
			  case 2: // Erode
			    rfPixel8Min3By3(mappedWin, temp);
			    break;
			    
			  case 3: // Open
			    rfPixel8Min3By3(mappedWin, temp);
			    mWin.copyPixelsFromWindow(temp);
			    rfPixel8Max3By3(mappedWin, temp);
			    break;
			    
			  default:
			    rmAssert(0);
			    break;
			  }
			  mWin.copyPixelsFromWindow(temp);
			}
			destMovie.addFrame(mappedWin);
			dFrmCnt++;
		      }
		    }
		    else {
		      rcWindow freqEBImg(cropRect.size());
		      kinetic.genEightBitImg(freqImg, freqEBImg, 0.0, 
					     (vector<uint8>*)0);
		      
		      destMovie.addFrame(freqEBImg);
		      dFrmCnt++;
		    }
		  }
		} // End of: if (dftEnabled) {
		else {
		  int32 movieOffset = (sinWinSz==frameCount) ? 0 : sinWinSz/2;
		  int32 movieOrigin = startFrame + movieOffset;
		  /* Note: To work properly with mask, this next call
		   * should take the mask as input.
		   */
		  int32 frmIndex = calcExtendedMuscleCellFrame(signal,
								 focus,
								 movieOrigin);
		  int32 mdlIndex = frmIndex + movieOrigin;
		  fprintf(stderr, "EXTENDED MUSCLE CELL INDEX: %d\n", mdlIndex);
		  if (fpEnabled) {
		    float maxFPP = (srcMovie->averageFrameRate() + 0.5) * 2;
		    float period = calcMusclePeriod(focus, startFrame, endFrame,
						    mdlIndex, maxFPP);
		    fprintf(stderr, "CALCULATED PERIOD: %f\n\n", period);
		  }
		}
	      } // End of: if (energyEnabled) {
	    } // End of: for (uint32 pci = 0; pci < pc.size(); pci++) {
	} // End of: for (uint32 sfi = 0; sfi < sf.size(); sfi++) {

      /* Step 3 - Clean up before returing to processing loop.
       */
      if (dFrmCnt) {
	printf("Saving movie with %d frames\n", dFrmCnt);
	destMovie.flush();
      }
      processSrc = processDest = 0;
      focus.clear();
      rcVideoCache::rcVideoCacheDtor(srcMovie);
    } // End of: if (processSrc) {
  } // End of: while (readCmd(info, cmd, lineNo)) {

  fclose(info);
}
