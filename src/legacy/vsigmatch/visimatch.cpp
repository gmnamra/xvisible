/*
 *	$Id:
 *
 *      Copyright (c) 2007 Reify Corp. All rights reserved.
 *      $Log$
 *
 *      MakeRefSig -m xxxxx.mpg  -r signatureFileName
 *      Create a signature of file xxxxx.mpg and store it in a file.
 *
 *   visimatch create report -content foo.mpeg -sigout foofoo.rfysig -reportout fofo.rfyrep
 *
 *      Match -m zzzzz.mpg -u folderOfReferenceSignatures
 *
 *   visimatch create -content tt.mpeg -sigout tt.rfysig -search dir -matchreportout tt.rfymatrep
 *
 *      Create a signature of file zzzz.mpg and match it against all reference
 *      signatures in folderOfReferenceSignatures. The output will be:
 *      Matching zzzzz.mpg
 *      NoMatchFound
 *      or
 *      Matching zzzz.mpg
 *      Match with xxxxxx offsetZ: n1  offset X: n2  duration: n3
 *      <there may be several matches>
 */




//#include <textio.hpp>
#include <fstream>
#include <iostreamio.hpp>
#include <timer.hpp>
#include <time.hpp>
#include <vector>
#include <cli_parser.hpp>

#include <rc_vector2d.h>
#include <rc_macro.h>
#include <rc_mpeg2grabber.h>
#include <rc_similarity.h>
#include <rc_systeminfo.h>
#include <rc_videocache.h>

#include <file_system.hpp>

using namespace std;



static int32 count (cli_parser& pars);
static int32 startframe (cli_parser& pars);
static int32 sample (cli_parser& pars);
static int32 percent (cli_parser& pars);
static rcChannelConversion color (cli_parser& pars);
static string repFile (cli_parser& pars, const string cmd, const string rfyext);
static void _printCommands ();
static string contentFile (cli_parser& pars);
static bool help (cli_parser& pars);
static bool verbose (cli_parser& pars);
static bool version (cli_parser& pars);
static deque<double> loadSig (const string& outf);
static void saveSig (deque<double>& sig, const string& outf);
static string searchDir (cli_parser& pars);
static string tmpDir (cli_parser& pars);
static void smSearch (deque<double>& known, deque<double>& unknown, const string& tmpdir, vector<rc2Dvector>& cfs);
static void _sm2out (const string& id, const string& tmpdir, deque<double> & cm);
static bool genSignatureFromMov (cli_parser&, string& , deque<double> &);
static string genRFYmov (string& val, const string contentf, const string execpath, int32 period, rcChannelConversion conv);
static bool characterize (deque<double>& src, deque<double>& dst, deque<int>& frames, deque<float>& peaks, int percent);
static void to_csv(const std::deque<double> & dm, const std::string& filef, bool row_wise = false);
static bool enclosingDirOk (const std::string val);


#define fileokandwritable(a) ( ! ( (a).empty() || ! is_present ((a)) || \
! is_file ((a)) || ! file_writable ((a)) || \
! is_full_path ((a)) ) )

#define fileokandreadable(a) ( ! ( (a).empty() || ! is_present ((a)) || \
! is_file ((a)) || ! file_readable ((a)) || \
! is_full_path ((a)) ) )

#define dirokandwritable(a) ( ! ( (a).empty() || ! folder_exists ((a)) || \
! folder_writable ((a)) || ! is_full_path ((a))))

#define dirokandreadable(a) ( ! ( (a).empty() || ! folder_exists ((a)) || \
! folder_readable ((a)) || ! is_full_path ((a))))

#define VLIBRARY_USAGE_HELP             \t-[no]help               # this help!
#define VLIBRARY_USAGE_SIGOUT           \t-sigout <name>=<path>   # create a signature file (.csv) to use
#define VLIBRARY_USAGE_CONTENT          \t-content <path>         # use content at <path> (only .mpg is supported)
#define VLIBRARY_USAGE_TMPDIR2USE       \t-tmpDir2Use <path>      # use directory at <path> for temporary storage
#define VLIBRARY_USAGE_SAMPLE           \t-sample <integer>       # use every <integer> default is 1
#define VLIBRARY_USAGE_COLOR            \t-[no]color              # use full color default is mean color
#define VLIBRARY_USAGE_STARTFRAME       \t-startFrame <integer>   # starting at frame <integer> default is 0
#define VLIBRARY_USAGE_COUNT            \t-count <integer>        # use <integer> frames default is all
#define VLIBRARY_USAGE_RELAXATION_PCT   \t-relax_pct <integer>    # use <integer> 0-100 above is relaxed default is 90
#define VLIBRARY_USAGE_SEARCH           \t-search <path>          # report match of content and every .rfysig file in the directory
#define VLIBRARY_USAGE_REPOUT           \t-searchrep <name>=<path>   # create a report file (.rfyrep) to use
#define VLIBRARY_USAGE_VERSION          \t-[no]version            # print product version
#define VLIBRARY_USAGE_VERBOSE          \t-[no]verbose            # print runtime info


// Note there are 5 args with default values. These will appear in the parse output.

cli_definition_t _commands [] =
{
    {"content", cli_value_kind, cli_single_mode, "VLIBRARY_USAGE_CONTENT", NULL},
    {"sigout", cli_value_kind, cli_single_mode, "VLIBRARY_USAGE_SIGOUT", NULL},
    {"tmpDir2Use", cli_value_kind, cli_single_mode, "VLIBRARY_USAGE_TMPDIR2USE", NULL},
    {"count", cli_value_kind, cli_single_mode, "VLIBRARY_USAGE_COUNT", "0"},
    {"relax_pct", cli_value_kind, cli_single_mode, "VLIBRARY_USAGE_RELAXATION_PCT", "90"},      
    {"startFrame", cli_value_kind, cli_single_mode, "VLIBRARY_USAGE_STARTFRAME", "0"},
    {"sample", cli_value_kind, cli_single_mode, "VLIBRARY_USAGE_SAMPLE", "1"},
    {"color", cli_switch_kind, cli_single_mode, "VLIBRARY_USAGE_COLOR", "no"},
    {"version", cli_switch_kind, cli_single_mode, "VLIBRARY_USAGE_VERSION", "no"},
    {"help", cli_switch_kind, cli_single_mode, "VLIBRARY_USAGE_HELP", "no"},
    {"verbose", cli_switch_kind, cli_single_mode, "VLIBRARY_USAGE_VERBOSE", "no"},
    END_CLI_DEFINITIONS
};



// built of the unix app static done (add catch)
// matrix out of rcSimilarator done
// persistence of matrix data done
// restore of matrix data done
// generate autocorr of SM (how it repeats itself)

// match of 2 SMs
// result file
//
// support ini file as parameter setting
// tmp file storage

int main(int argc, char** argv)
{
    timer fromDstart;
    string argv0 (argv[0]);
    string execpath = install_path (argv0);
    cerr << flush;
    error_handler errh (fout, 0);
    //  errh.add_message_file(create_filespec(execpath,"messages.txt"));
    bool verboseOn = false;
    bool versionOn = false;
    try
    {
        
        cli_parser pars (argv, _commands, errh);
        bool isHelp =  help (pars);
        if (isHelp) return -1;
        if (argc < 2 || pars.size () <= 5) // See note above
        {
            _printCommands ();
            return -1;
        }
        
        versionOn = version (pars);
        if (versionOn)
        {
            cerr << " vsigmatch version 1.1 Copyright (c) 2014 Reify Corp. All rights reserved. " << endl;
            return -1;
        }
        
        
        verboseOn = verbose (pars);
        
        // get the content file if we have
        string contentf = contentFile (pars);
        string contentType = extension_part(contentf);
        if (contentf.empty() || contentType.empty()) {cerr << "Content file incorrect" << endl; return -1;}
        
        // Check to see if the outfile is ok ?
        string outf = repFile (pars, string ("sigout"), string ("csv"));
        string tmpdirname = tmpDir (pars);
        int relax_pct = percent (pars);
        if (verboseOn) 
        {
            if (tmpdirname.empty() ) {cerr << "tmp directory is incorrect" << endl;}
            else  {cerr << "tmp directory is correct" << endl;}
        }
        
        if (verboseOn) { cerr << "A Parsing ok " << fromDstart.text() << endl;      fromDstart.reset (); }
        
        // This Clips Signature
        std::deque<double> fixed;
        
        // Are we mpeg or rfymov
        bool isMOV = contentType.compare ("mov") == 0;
        bool isAVI = contentType.compare ("avi") == 0;        
        bool isRFY = contentType.compare ("rfymov") == 0;
        
        if ((isMPG || isRFY)  && !outf.empty())
        {
            // Make an extracted part, Create a dst file for the rfymov file
            int32 period = sample (pars);
            period = (period < 1) ? 1 : period;
            const rcChannelConversion conv = color (pars);
            string rfymovf = (isMPG) ? genRFYmov (tmpdirname, contentf, execpath, period, conv) : contentf;
            if (verboseOn && isMPG) { cerr << "B extracted audio, converted to rfymov for fast access " << fromDstart.text() << endl;fromDstart.reset (); }
            bool smIsOk = genSignatureFromMov (pars, rfymovf, fixed);
           
            if (smIsOk)
            {
                saveSig (fixed, outf); 
                if (verboseOn) 
                { 
                    cerr << "C signature created ok " << fromDstart.text() << endl; fromDstart.reset ();
                }
            }
        }
        else
        {
            if (contentf.empty() || contentType.empty())
            {
                cerr << "file type is incorrect" << endl; return -1;
            }
        }
        
    }
    catch(error_handler_limit_error& exception)
    {
        cerr << "Handler error: report to Reify" << endl;
    }
    catch(std::exception& x)
    {
        cerr << "Caught exception " << x.what() << endl;
        cerr << "STL error: report to Reify" << endl;
    }
}

    // Two major options:
    // A. IN (MPG) OUT (rfysig) OUT (optional rfyrep)
    // B. IN (MPG) IN (Dir of rfysigs) OUT (Dir of rfyrep)
    

static bool genSignatureFromMov (cli_parser& pars, string& rfymovf, deque<double>& _entropies)
{
    double physMem (rfSystemRam () / rmBytesInGig); // In GigaBytes
    const double maxMem = rmBytesInGig / 4.; // Minimum 256 MB
    physMem = rmMin (physMem / 4 , maxMem);
    uint32 maxMemToCache = (uint32) (physMem * rmBytesInGig);
    rcVideoCache* _videoCacheP = rcVideoCache::rcVideoCacheCtor(rfymovf, 0, true, false,true,
                                                                maxMemToCache, 0 );
    
    if (! _videoCacheP->frameCount() ) return false;
    
    // VideoCache has grabbed all frames at sampling specified
    // Now at the offset and count specified, run similarator
    // get other vars
    int32 offset = startframe (pars);
    offset = (offset < 0) ? 0 : offset;
    int32 _count = count (pars);
    _count = (_count < 0) ? 0 : _count;
    
    int32 framesToUse = (int32) (_videoCacheP->frameCount()) - offset;
    if (_count > 0 && framesToUse > _count) framesToUse = _count;
    int32 frameIndex = offset;
    vector<rcWindow> movie;
    for (int32 usedFrames = 0; usedFrames++ < framesToUse;frameIndex += 1)
        movie.push_back(rcWindow(*_videoCacheP, (uint32) (frameIndex)));
    
    int32 cacheSize = (uint32)(_videoCacheP->cacheSize());
    // Do not accesss images in the vector. They are cached.
    rcSimilaratorRef simi ( new rcSimilarator (rcSimilarator::eExhaustive,
                                               _videoCacheP->frameDepth(),
                                               framesToUse,
                                               cacheSize,
                                               rcSimilarator::eLowSpT,
                                               true));
    simi->fill(movie);
    bool ok = simi->entropies (_entropies, rcSimilarator::eVisualEntropy);
    rcVideoCache::rcVideoCacheDtor (_videoCacheP);
    return ok;
}



///////////////I N T E R N A L /////////////////////////

void dump(dump_context& context, const deque<double>& data)
{
    dump_deque(context, data);
}

void restore(restore_context& context, deque<double>& data)
{
    restore_deque(context, data);
}

static deque<double> loadSig (const string& inf)
{
    deque<double> dm;
    restore_from_file (inf, dm, 0);
    return dm;
}


//// outf is a complete file specification
static void saveSig (deque<double>& sig, const string& outf)
{
    dump_to_file (sig, outf, 0);
}


static void _sm2out (const string& id, const string& tmpdir, deque<double> & cm)
{
    // create and open the IOstream device
    string namf = id + to_string (time_now ());
    string textext ("txt");
    string filef = create_filespec (tmpdir, namf, textext);
    ofstream output_stream(filef.c_str());
    // create and initialise the TextIO wrapper device
    oiotext output(output_stream);
    // now use the device
    uint32 j;
    deque<double>::iterator ds = cm.begin();
    for (j = 0; j < cm.size() - 1; j++)
        output << *ds++ << ",";
    output << *ds << endl;
    output_stream.flush();
}


static bool help (cli_parser& pars)
{
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("help") == 0)
        {
            if (pars.switch_value(i))
            {
                _printCommands ();
                return true;
            }
        }
    return false;
}

static bool verbose (cli_parser& pars)
{
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("verbose") == 0)
        {
            if (pars.switch_value(i))
            {
                return true;
            }
        }
    return false;
}


static bool version (cli_parser& pars)
{
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("version") == 0)
        {
            if (pars.switch_value(i))
            {
                return true;
            }
        }
    return false;
}

static string contentFile (cli_parser& pars)
{
    string val;
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("content") == 0)
        {
            val = pars.string_value (i);
            break;
        }
    
    bool valisOk = fileokandreadable (val);
    if (! valisOk) return string ();
    return val;
    
}


static int32 startframe (cli_parser& pars)
{
    int32 val = - 1;
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("startFrame") == 0)
        {
            val = to_int (pars.string_value (i));
            break;
        }
    
    return val;
}

static int32 count (cli_parser& pars)
{
    int32 val = - 1;
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("count") == 0)
        {
            val = to_int (pars.string_value (i));
            break;
        }
    
    return val;
}

static int32 sample (cli_parser& pars)
{
    int32 val = - 1;
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("sample") == 0)
        {
            val = to_int (pars.string_value (i));
            break;
        }
    
    return val;
}

static int32 percent (cli_parser& pars)
{
    int32 val = - 1;
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("relax_pct") == 0)
        {
            val = to_int (pars.string_value (i));
            break;
        }
    
    return val;
}

static rcChannelConversion color (cli_parser& pars)
{
    rcChannelConversion conv = rcSelectAverage;
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("color") == 0)
            if (pars.switch_value(i))
            {
                conv = rcSelectAll;
                return conv;
            }
    return conv;
}

static string repFile (cli_parser& pars, const string cmd, const string rfyext)
{
    string val;
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare (cmd) == 0)
        {
            val = pars.string_value (i);
            break;
        }
    
    string foldername = folder_part (val);
    string basename = basename_part (val);
    bool dirisOk = dirokandwritable (foldername);
    if (! dirisOk) return string ();
    val = create_filespec (foldername, basename, rfyext);
    return val;
    
}

static string searchDir (cli_parser& pars)
{
    string val;
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("search") == 0)
        {
            val = pars.string_value (i);
            break;
        }
    
    bool dirOk = dirokandwritable (val);
    if (! dirOk) return string ();
    return val;
}

static string tmpDir (cli_parser& pars)
{
    string val;
    for (uint32 i = 0; i < pars.size (); i++)
        if (pars.name (i).compare ("tmpDir2Use") == 0)
        {
            val = pars.string_value (i);
            bool tmpdirOk = dirokandwritable (val);
            if (! tmpdirOk) return string ();
            return val;
        }
    return val;
}

static bool enclosingDirOk (const std::string val)
{
    string foldername = folder_part (val);
    return dirokandwritable (foldername);    
}


static string genRFYmov (string& val, const string contentf, const string execpath, int32 period, rcChannelConversion conv)
{
    string m2vf = create_filespec (val, filename_part (contentf) + string (".mpg"));
    string rfymovf = create_filespec (val, filename_part (contentf) + string (".rfymov"));
    if (m2vf.empty() || rfymovf.empty()) return string ();
    
    // First we need to extract the video track
    string m2vprog ("/extract_m2v -s 0xe0 ");
    string cmd = execpath + m2vprog + contentf + string (" > ") + m2vf;
    std::system( cmd.c_str() );
    std::string whichMovie = std::string (m2vf);
    if (file_size (m2vf) == 0) whichMovie = std::string (contentf);
    
    // Convert mpg to rfymov and store it loclly
    // ini : channelConveson, frameInterval, extractm2v
    rcMPEG2Grabber mpegg (whichMovie, NULL);
    mpegg.start ();
    const movieFormatRev rev = movieFormatRevLatest;
    const float fInt = 1/29.97f;
    mpegg.getReifyMovie (std::string (rfymovf), conv, rev, true, fInt, period);
    
    // Delete the extracted version @todo maybe cache ?
    file_delete (m2vf);
    return rfymovf;
}

//
//static std::vector<rcWindow> gen_similarity_windows (rcWindow& image, rcISize& kernel)
//{
//  rcIRect path (kernel.width()/2, kernel.height()/2, image.width() - kernel.width() + 1, 
//                image.height() - kernel.height() + 1);
//    std::vector<rcWindow> waves;
//    if ((path.origin().x() < 0 || path.origin().y() < 0 || path.width() >= image.width() || path.height() >= image.height())) return waves;
//    for (int y = path.origin().y(); y <= path.ur().y(); y++)
//        for (int x = path.origin().y(); y <= path.ur().y(); y++)        
//}


static void _printCommands ()
{
    cerr << "Copyright Reify Corporation 2003-2013 All rights reserved" << endl;
    cerr << "\t-[no]help               \t this help!" << endl;
    cerr << "\t-sigout <name>=<path>   \t create a signature file (.rfysig) to use " << endl;
    cerr << "\t-content <path>         \t use content at <path> (only .mpg & rfysig are supported) " << endl;
    cerr << "\t-tmpDir2Use <path>      \t use directory at <path> for temporary storage " << endl;
    cerr << "\t-sample <integer>       \t use every <integer> from mpeg default is 1 " << endl;
    cerr << "\t-relax_pct <integer>    \t use <integer> 0-100 above is relaxed" << endl;    
    cerr << "\t-[no]color              \t use full color default: use mean color " << endl;
    cerr << "\t-startFrame <integer>   \t starting at frame <integer> of the rendered movie default is 0 " << endl;
    cerr << "\t-count <integer>        \t use <integer> frames of the rendered movie default is all " << endl;
    cerr << "\t-search <path>          \t report match of content and every .rfysig file in the directory " << endl;
    cerr << "\t-searchrep <name>=<path>   \t create a report file (.rfymat) to use " << endl;
    cerr << "\t-[no]version            \t display product version " << endl;
    cerr << "\t-[no]verbose            \t display runtime info " << endl;

    
}
