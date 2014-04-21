
#include <stlplus_lite.hpp>
#include <iostream> // std::cout
#include <fstream>
#include <string>
#include <algorithm> // std::sort, std::copy
#include <iterator> // std::ostream_iterator
#include <sstream> // std::istringstream
#include <vector>
#include <cctype> // std::isdigit
#include <rc_timestamp.h>
#include <iomanip>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/dir.h>
#include <deque>
#include <boost/foreach.hpp>

#include "rc_fileutils.h"




static bool nocase_compare (char c1, char c2)
{
	return toupper(c1) == toupper (c2);
}

bool rfNocaseEqual (const std::string& s1, const std::string& s2)
{
	return equal (s1.begin (), s1.end (), s2.begin (), nocase_compare);
}

void RFY_API rfGetFilesFromSeparatedList (const std::string& inputFile, vector<std::string> & fileNames, int frame_count, int frame_index, const char token)
{
    fileNames.resize (0);    
    if ( !inputFile.empty() )
    {
        vector<const char*> files;
        std::string::size_type str = 0, idx = 0;
        int32 i = 0;
        int32 firstIndex = frame_index;
        int32 frameCount = frame_count;
        
        while ( str < inputFile.size() ) {
            idx = inputFile.find( ';', str );
            if ( idx != std::string::npos ) {
                if ( i >= firstIndex ) {
                    if ( frameCount >= 0 && fileNames.size() == uint32(frameCount) )
                        break;
                    std::string oneFile = inputFile.substr( str, idx - str );
                    fileNames.push_back( oneFile );
                }
                str = idx + 1;
                ++i;
            } else
                break;
        }
#ifdef DEBUG_LOG
        cerr << "Exporting " <<  fileNames.size() << " images " << endl;
        for ( uint32 x = 0; x < fileNames.size(); ++x ) {
            cerr << fileNames[x] << endl;
        }
#endif    
    }
}



static int alphasorthalf (const void*d1, const void*d2);
static int numericsort(const void*A, const void*B, int cs);
static int numericsort(const void*A, const void*B);
static int casenumericsort(const void*A, const void*B);

bool numeric_compare (const std::string& A, const std::string& B);
bool natural_compare (const std::string& A, const std::string& B);


static std::string staticTempName = std::string ("/var/tmp/rfytmpXXXXXX"); 

std::string rfGetTempFile (std::string& foo)
{
	
    std::string& tmptemplate = (foo == std::string ()) ? staticTempName : foo;        
    return std::string (mktemp((char *) tmptemplate.c_str()));
}



bool rf_sensitive_case_compare (const std::string& str1, const std::string& str2)
{
    for(unsigned int i=0; i<str1.length(); i++){
        if(str1[i] != str2[i])
            return false;
    }
    return true;
}


bool rf_insensitive_case_compare (const std::string& str1, const std::string& str2)
{
    for(unsigned int i=0; i<str1.length(); i++){
        if(toupper(str1[i]) != toupper(str2[i]))
            return false;
    }
    return true;
}


void rfSortImageFileNames(const vector<std::string> & entries, vector<std::string> & files, const char *imageformat)
{
    if (entries.empty () ) return;
    
    files.resize (0);
    vector<std::string>::const_iterator filet = entries.begin();
    for (; filet < entries.end(); filet++)
    {
        files.push_back(*filet);
    }
    
    if (strncmp(imageformat, "tif", 3) == 0)    
    {
        std::sort (files.begin(), files.end(), numeric_compare);
    }
    
    if (strncmp(imageformat, "jpg", 3) == 0)
    {
        std::sort (files.begin(), files.end(), natural_compare);
    }
    
	if (strncmp(imageformat, "png", 3) == 0)
    {
        std::sort (files.begin(), files.end(), natural_compare);
    }
    
}



void rfGetDirEntries(const string & dirname, 
                     vector<std::string> & entries, const char *imageformat)
{
    
    if (! folder_exists (dirname) ) return;
    std::string wildcard ("*.");    
    std::string iformat (imageformat);
    wildcard = wildcard + iformat;
    
    vector<std::string> files = folder_wildcard (dirname,wildcard, false, true );
    
    if (strncmp(imageformat, "tif", 3) == 0)    
    {
        std::sort (files.begin(), files.end(), numeric_compare);
    }
    
    if (strncmp(imageformat, "jpg", 3) == 0)
    {
        std::sort (files.begin(), files.end(), natural_compare);
    }
    
	if (strncmp(imageformat, "png", 3) == 0)
    {
        std::sort (files.begin(), files.end(), natural_compare);
    }
    
	entries.resize (0);
    vector<std::string>::const_iterator filet = files.begin();
    for (; filet < files.end(); filet++)
    {
        entries.push_back(create_filespec (dirname, *filet));
    }
}




int32 rfGetFileSize (const char* sFileName)
{
    return file_size (std::string (sFileName));
}

// Not really part of c++
// int32 rfGetFileSize (const char *inFullPath)
// {
//   struct stat     statbuf;
//   if (stat(inFullPath, &statbuf) == 0)
//     return ((intmax_t)statbuf.st_size);
//   return -1;
// }

// Function which checks whether an file path is a directory.  Should
// be portable b/w Windows and UNIX.

bool rfIsDirectory (const char * inFullPath)
{
    return folder_exists (std::string (inFullPath));
}


static int alphasorthalf (const void*d1, const void*d2)
{
    struct dirent *dir1 = *(struct dirent **)d1;
    struct dirent *dir2 = *(struct dirent **)d2;
    
    return strcmp(dir1->d_name,dir2->d_name);
}




// Utilities

// Strip path from filename
std::string rfStripPath( const std::string& fileName )
{
    return filename_part (fileName);   
}

// Strip extension (.rfymov etc.) from filename
std::string rfStripExtension( const std::string& fileName )
{
    return basename_part (fileName);
}

// get extension (.rfymov etc.) from filename
std::string rfGetExtension( const std::string& fileName )
{
    return extension_part (fileName);
}

#define DFN_EXT_IS(_EXT_)\
bool rf_ext_is_##_EXT_ (const std::string& filename)\
{\
std::string _EXT_ (#_EXT_);\
return rf_insensitive_case_compare(rfGetExtension(filename), _EXT_);\
}

DFN_EXT_IS(rfymov);
DFN_EXT_IS(mov);
DFN_EXT_IS(stk);




/*
 * 'numericsort()' - Compare two directory entries, possibly with
 *                   a case-insensitive comparison...
 */


static int numericsort(const void *A, const void *B, int cs) {
    struct dirent *dir1 = *(struct dirent **)A;
    struct dirent *dir2 = *(struct dirent **)B;
    
    const char* a = dir1->d_name;
    const char* b = dir2->d_name;
    int ret = 0;
    for (;;) {
        if (isdigit(*a & 255) && isdigit(*b & 255)) {
            int diff,magdiff;
            while (*a == '0') a++;
            while (*b == '0') b++;
            while (isdigit(*a & 255) && *a == *b) {a++; b++;}
            diff = (isdigit(*a & 255) && isdigit(*b & 255)) ? *a - *b : 0;
            magdiff = 0;
            while (isdigit(*a & 255)) {magdiff++; a++;}
            while (isdigit(*b & 255)) {magdiff--; b++;}
            if (magdiff) {ret = magdiff; break;} /* compare # of significant digits*/
            if (diff) {ret = diff; break;}	/* compare first non-zero digit */
        } else {
            if (cs) {
                /* compare case-sensitive */
                if ((ret = *a-*b)) break;
            } else {
                /* compare case-insensitve */
                if ((ret = tolower(*a & 255)-tolower(*b & 255))) break;
            }
            
            if (!*a) break;
            a++; b++;
        }
    }
    if (!ret) return 0;
    else return (ret < 0) ? -1 : 1;
}

/*
 * 'casenumericsort()' - Compare directory entries with case-sensitivity.
 */

bool numeric_compare (const std::string& A, const std::string& B)
{
    return numericsort(A.c_str(), B.c_str(), 0) == 0;
}


int rfFilename_match(const char *s, const char *p)
{
    int matched;
    
    for (;;) {
        switch(*p++) {
                
            case '?' :	// match any single character
                if (!*s++) return 0;
                break;
                
            case '*' :	// match 0-n of any characters
                if (!*p) return 1; // do trailing * quickly
                while (!rfFilename_match(s, p)) if (!*s++) return 0;
                return 1;
                
            case '[': {	// match one character in set of form [abc-d] or [^a-b]
                if (!*s) return 0;
                int reverse = (*p=='^' || *p=='!'); if (reverse) p++;
                matched = 0;
                char last = 0;
                while (*p) {
                    if (*p=='-' && last) {
                        if (*s <= *++p && *s >= last ) matched = 1;
                        last = 0;
                    } else {
                        if (*s == *p) matched = 1;
                    }
                    last = *p++;
                    if (*p==']') break;
                }
                if (matched == reverse) return 0;
                s++; p++;}
                break;
                
            case '{' : // {pattern1|pattern2|pattern3}
            NEXTCASE:
                if (rfFilename_match(s,p)) return 1;
                for (matched = 0;;) {
                    switch (*p++) {
                        case '\\': if (*p) p++; break;
                        case '{': matched++; break;
                        case '}': if (!matched--) return 0; break;
                        case '|': case ',': if (matched==0) goto NEXTCASE;
                        case 0: return 0;
                    }
                }
            case '|':	// skip rest of |pattern|pattern} when called recursively
            case ',':
                for (matched = 0; *p && matched >= 0;) {
                    switch (*p++) {
                        case '\\': if (*p) p++; break;
                        case '{': matched++; break;
                        case '}': matched--; break;
                    }
                }
                break;
            case '}':
                break;
                
            case 0:	// end of pattern
                return !*s;
                
            case '\\':	// quote next character
                if (*p) p++;
            default:
                if (tolower(*s) != tolower(*(p-1))) return 0;
                s++;
                break;
        }
    }
}

bool natural_compare (const std::string& a, const std::string& b)
{
    if (a.empty())
        return true;
    if (b.empty())
        return false;
    if (std::isdigit(a[0]) && !std::isdigit(b[0]))
        return true;
    if (!std::isdigit(a[0]) && std::isdigit(b[0]))
        return false;
    if (!std::isdigit(a[0]) && !std::isdigit(b[0]))
    {
        if (a[0] == b[0])
            return natural_compare(a.substr(1), b.substr(1));
        return (std::toupper(a[0]) < std::toupper(b[0]));
    }
    
    // Both strings begin with digit --> parse both numbers
    std::istringstream issa(a);
    std::istringstream issb(b);
    int ia, ib;
    issa >> ia;
    issb >> ib;
    if (ia != ib)
        return ia < ib;
    
    // Numbers are the same --> remove numbers and recurse
    std::string anew, bnew;
    std::getline(issa, anew);
    std::getline(issb, bnew);
    return (natural_compare(anew, bnew));
}

// Construct a temporary file name
std::string rfMakeTmpFileName(const char *pathFormat, const char* baseName )
{
    char buf[2048];
    static const char *defaultFormat = "/tmp/um%d_%s";
    
    // Use current time in seconds as a pseudo-unique prefix
    double secs = rcTimestamp::now().secs();
    snprintf( buf, rmDim(buf), pathFormat ? pathFormat : defaultFormat, 
             uint32(secs), baseName );
    return std::string( buf );
}

// Construct a temporary file name
std::string rfMakeTmpFileName(const char* baseName )
{
    return rfMakeTmpFileName (0, baseName);
}




// Putout a SS Matrix as represented by a deque<deque<double>>
std::string rfDumpMatrix (deque<deque<double> >& matrix, const char *pathFormat, const char* baseName)
{
    std::string fname = rfMakeTmpFileName (pathFormat, baseName) + std::string (".txt");
    
    ofstream file (fname.c_str ());
    if (! file)
    {
        return std::string ("");
    }
    
    deque<deque <double> >::iterator rowI = matrix.begin ();
    deque<deque <double> >::iterator rowend = matrix.end();
    
    ios::fmtflags oldflags = file.flags ();
    file.setf (ios::fixed);
    file << setprecision(12);
    for (; rowI < rowend; rowI++)
    {
        deque<double>::iterator cellI = (*rowI).begin ();
        deque<double>::iterator cellBend = (*rowI).end();
        advance (cellBend, -1);
        for (; cellI < cellBend; cellI++)
            file << *cellI << ",";
        file << *cellI << endl;
    }
    file.flags (oldflags);
    return fname;
}

bool rfFileExists(const std::string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}


// CSV File Support


csv::rows_type csv::to_rows(std::istream &input)
{
    csv::rows_type rows;
    std::string line;
    //for each line in the input stream
    while (std::getline(input, line))
    {
        csv::row_type row(1, "");
        bool in_quote = false;
        char last_ch, next_ch = ' ';
        //for each character in the line
        BOOST_FOREACH(char ch, line)
        {
            last_ch = next_ch;
            next_ch = ch;
            //catch a quote character and change the state
            //we handle double quotes by checking last_ch
            if (ch == '"')
            {
                in_quote = not in_quote;
                if (last_ch != '"') continue;
            }
            //a comma not inside quotes is a column delimiter
            if (not in_quote and ch == ',')
            {
                row.push_back("");
                continue;
            }
            //if we got here we record the character
            row.back() += ch;
        }
        rows.push_back(row);
    }
    return rows;
}

