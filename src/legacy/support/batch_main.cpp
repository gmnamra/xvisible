#include <stdio.h>

#include <rc_types.h>
#include <rc_systeminfo.h>

#include <rc_window.h>
#include <rc_videocache.h>
#include <rc_windowhist.h>

#include <rc_similarity.h>

static std::string entropyDefinitionName( rcSimilarator::rcEntropyDefinition def )
{
    switch ( def ) {
        case rcSimilarator::eACI:
            return "Aggregate Change Index";
        case rcSimilarator::eVisualEntropy:
            return "Visual Entropy";
        case rcSimilarator::eRMSVisualEntropy:
            return "Diffusion Index";
    }
    
    return "Unknown Entropy Measurement";
}

int printUsage()
{
    fprintf(stderr, "Usage: batchanalysis [ -a ] [ -o off ] [ -p period ] "
            "[ -c cnt ] [ -f sx ] [ -m pct ] vidfile [vidfile]...\n"
            "\n Generates energy values based on normalized correlation over"
            " an entire video.\n Video must be in Reify format.\n\n"
            " -a        Enables correlation approximation (Default: disabled)\n"
            " -o off    Starting frame offset within movie (>=0) (Default: 0)\n"
            " -p period Take 1 out of every period frames (>=1) (Default: 1)\n"
            " -c cnt    Maximum count of frames to use (>=1) (Default: all)\n"
            " -d def    Entropy definition: aci - aggregate change index\n"
            "                               vis - visual entropy\n"
            "                               dif - diffusion index\n"
            "           (Default: aci)\n"
            " -f sx     Results format: s - single line, no times, comma separated\n"
            "                           x - dump self-similarity matrix\n"
            " -m pct    Pct of phys RAM to set aside for cache (0<pct<=100)\n"
            "           (Default: 50%% exhaustive, 5%% approximate)\n");
    return 1;
}

int main( int argc, char ** argv )
{

    uint32 offset = 0;
    uint32 period = 1;
    uint32 count = 0xFFFFFFFF;
    double pctMem = -1.0;
    bool approx = false;
    bool singleLine = false;
    bool dumpMatrix = false;
    int switchCnt = 1;
    rcSimilarator::rcEntropyDefinition def = rcSimilarator::eACI;
           
    for (int i = 1; i < argc; i++) {
        if (*(argv[i]) != '-')
            continue;

        switchCnt++;
        switch (char choice = *(argv[i] + 1)) {
            case 'a':
                approx = true;
                break;

            case 'o':
            case 'p':
            case 'c':
            case 'm':
            case 'f':
            case 'd':
                if (++i >= argc)
                    return printUsage();

                int32 temp;
                if ((choice != 'f' && choice!= 'd') && 
                    ((sscanf(argv[i], "%d", &temp) != 1) || (temp < 0)))
                    return printUsage();
      
                if (choice == 'o')
                    offset = (uint32)temp;
                else if (choice == 'p') {
                    if (temp == 0)
                        return printUsage();
                    period = (uint32)temp;
                }
                else if (choice == 'c') {
                    if (temp == 0)
                        return printUsage();
                    count = (uint32)temp;
                }
                else if (choice == 'm') {
                    if (temp == 0 || temp > 100)
                        return printUsage();
                    pctMem = temp/100.;
                }
                else if (choice == 'f') {
                    uint32 len = strlen(argv[i]);

                    if (len > 2)
                        return printUsage();
                    const char choice1 = *(argv[i]);
                    const char choice2 = (len == 2) ? *(argv[i] + 1) : choice1;
                    if (((choice1 != 's') && (choice1 != 'x')) ||
                        ((choice2 != 's') && (choice2 != 'x')))
                        return printUsage();
                    if ((choice1 == 's') || (choice2 == 's'))
                        singleLine = true;
                    if ((choice1 == 'x') || (choice2 == 'x'))
                        dumpMatrix = true;
                }
                else if (choice == 'd') {
                    uint32 len = strlen(argv[i]);

                    if (len != 3) {
                        fprintf( stderr, "Error: invalid entropy definition %s\n",
                                 argv[i] );
                        return printUsage();
                    }
                    if (!strcmp(argv[i], "aci" ) )
                        def = rcSimilarator::eACI;
                    else if (!strcmp(argv[i], "vis" ))
                        def = rcSimilarator::eVisualEntropy;
                    else if (!strcmp(argv[i], "dif" ))
                        def = rcSimilarator::eRMSVisualEntropy;
                    else {
                        fprintf( stderr, "Error: invalid entropy definition %s\n",
                                 argv[i] );
                        return printUsage();
                    }
                }
                else
                    rmAssert(0);
                break;
            default:
                return printUsage();
        } // End of: switch(*(argv[i] + 1))
    } // End of: for (int i = 1; i < argc; i++)

    if (argc == switchCnt)
        return printUsage();

    if (pctMem == -1) {
        if (approx)
            pctMem = .05;
        else
            pctMem = .50;
    }

    for (int i = 1; i < argc; i++) {
        if (*(argv[i]) == '-') {
            if (*(argv[i] + 1) != 'a') i++;
            continue;
        }
        std::string fileName(argv[i]);
        cerr << "Processing file: " << fileName
             << (approx ? " using corr approx" : " using full corr") << endl;
        cout << "Processing file: " << fileName
             << (approx ? " using corr approx" : " using full corr") << endl;

        uint32 physRAM = (uint32)(rfPhysicalRAMSize() * pctMem);
 
        rcVideoCache* cacheP = 
            rcVideoCache::rcVideoCacheCtor(fileName, 0, false, true, false, physRAM);

        if (!cacheP->isValid()) {
            cerr << "Open error: "
                 << rcVideoCache::getErrorString(cacheP->getFatalError())
                 << endl;
            rcVideoCache::rcVideoCacheDtor(cacheP);
            continue;
        }

        rcSimilarator* analysis = 0;
        int32 cacheSize = (uint32)(cacheP->cacheSize());
        uint32 framesToUse = (cacheP->frameCount() - offset)/period;
        if (framesToUse > count)
            framesToUse = count;
    
        cerr << "Cache size: " << cacheSize << " offset: " << offset << " period: "
             << period << " frame count: " << framesToUse
             << " pctMem: " << pctMem << endl;

        uint32 frameIndex = offset;

        vector<rcWindow> movie;
        for (uint32 usedFrames = 0; usedFrames++ < framesToUse;
             frameIndex += period)
            movie.push_back(rcWindow(*cacheP, frameIndex));

	rcSimilarator::rcCorrelationDefinition cdl =
	  (def == rcSimilarator::eACI) ? rcSimilarator::eNorm : 
	  (def == rcSimilarator::eVisualEntropy) ? rcSimilarator::eNorm : 
	  (def == rcSimilarator::eRMSVisualEntropy) ? rcSimilarator::eRelative : rcSimilarator::eNorm;

        if (approx)
            analysis = new rcSimilarator(rcSimilarator::eApproximate,
                                         movie[0].depth(), movie.size(), 
                                         (cacheSize > 10 ? 10 : cacheSize), cdl, true);
        else
            analysis = new rcSimilarator(rcSimilarator::eExhaustive,
                                         movie[0].depth(), movie.size(), 
                                         cacheSize, cdl, true);

        rmAssert(analysis);

        deque<double> entropyScores;
        analysis->fill(movie);
 
        bool worked = analysis->entropies(entropyScores, def);

        if (worked) {
            if (singleLine) {
                cout << (entropyScores[0]);
                for (uint32 i = 1; i < entropyScores.size(); i++)
                    cout << ", " << (entropyScores[i]);
                cout << endl;
            }
            else {
                cout << "Time (seconds) , " << entropyDefinitionName( def ) << endl;
                rcTimestamp baseTime;
                if (cacheP->frameIndexToTimestamp(0, baseTime, 0) != eVideoCacheStatusOK) {
                    fprintf(stderr, "Couldn't read base timestamp %d\n", i);
                }
                frameIndex = offset;
                for (uint32 i = 0; i < entropyScores.size(); i++, frameIndex += period) {
                    rcTimestamp time;
                    if (cacheP->frameIndexToTimestamp(frameIndex, time, 0) !=
                        eVideoCacheStatusOK) {
                        fprintf(stderr, "Couldn't read timestamp %d\n", i);
                    }
                    time = time - baseTime;
                    cout << time.secs() << ", " << (entropyScores[i]) << endl;
                }
            }

            if (dumpMatrix)
                cout << *analysis << endl;

            fprintf(stderr, "\nTotal cache hits %d misses %d\n",
                    cacheP->cacheHits(), cacheP->cacheMisses());
        }
        else {
            fprintf(stderr, "Analysis failed/aborted\n");
            return 1;
        }

        delete analysis;
        rcVideoCache::rcVideoCacheDtor(cacheP);
    }
    return 0;
}
