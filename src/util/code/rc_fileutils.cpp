/*
 *
 *$Id $
 *$Log$
 *Revision 1.4  2005/10/12 22:04:17  arman
 *incremental
 *
 *Revision 1.3  2005/05/08 21:28:53  arman
 *added .tiff and .TIFF
 *
 *Revision 1.2  2005/05/04 01:49:56  arman
 *tif filter now is case independent
 *
 *Revision 1.1  2005/04/29 21:59:18  arman
 *get .tif files from a directory.
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_timestamp.h>
#include "rc_fileutils.h"
#include <sys/stat.h>
#include <iomanip>
#include <fstream>

typedef struct direct    ENTRY;


static int alphasorthalf (const void*d1, const void*d2);
static int numericsort(const void*A, const void*B, int cs);
static int numericsort(const void*A, const void*B);
static int casenumericsort(const void*A, const void*B);

static int tiffilter(struct direct * dir);
static int jpgfilter(struct direct * dir);
static int pngfilter(struct direct * dir);

void rfGetDirEntries(const string & dirname, 
		     vector<std::string> & entries, const char *imageformat)
// pre: dirname is directory accessible for reading
// post: entries contains pointers to direct structs for each
//       file/subdirectory (aka file) in dirname    
{
  direct ** darray;
  int entryCount (0);

  if (strncmp(imageformat, "tif", 3) == 0)
    {
      entryCount = scandir(
			   const_cast<char *>(dirname.c_str()), &darray,
			   tiffilter, numericsort);
    }

  if (strncmp(imageformat, "jpg", 3) == 0)
    {
      entryCount = scandir(
			   const_cast<char *>(dirname.c_str()), &darray,
			   jpgfilter, alphasorthalf);
    }

	if (strncmp(imageformat, "png", 3) == 0)
    {
		entryCount = scandir(
							 const_cast<char *>(dirname.c_str()), &darray,
							 pngfilter, alphasorthalf);
    }
	

  for(int k=0; k < entryCount; k++)
    {
      std::string name (darray[k]->d_name);
      name = dirname + std::string ("/") + name;
      entries.push_back(name);
    }
}



int32 rfGetFileSize (const char* sFileName)
{
  std::ifstream f;
  f.open(sFileName, std::ios_base::binary | std::ios_base::in);
  if (!f.good() || f.eof() || !f.is_open()) { return 0; }
  f.seekg(0, std::ios_base::beg);
  std::ifstream::pos_type begin_pos = f.tellg();
  f.seekg(0, std::ios_base::end);
  return static_cast<int>(f.tellg() - begin_pos);
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
  struct stat dirstat;
  if (stat(inFullPath, &dirstat) == 0)
    {
      if ((dirstat.st_mode & S_IFDIR) == S_IFDIR)
	return true;
    }
  return false;
}

static int tiffilter(struct direct * dir)
// post: returns 1/true iff name of dir ends in .cpp    
{
  char * s = dir->d_name;
  int cpplen = strlen(s) - 4;   // index of start of . in .cpp
  if (cpplen >= 0) {
    if (
	(strncmp(s+cpplen, ".tif",4) == 0) ||
	(strncmp(s+cpplen, ".TIF",4) == 0))
      {
	return 1;
      }
  }
  if (cpplen >= 1) {
    if (
	(strncmp(s+cpplen-1, ".tiff",5) == 0) ||
	(strncmp(s+cpplen-1, ".TIFF",5) == 0)) 
      {
	return 1;
      }
  }

  return 0;
}

static int jpgfilter(struct direct * dir)
// post: returns 1/true iff name of dir ends in .cpp    
{
  char * s = dir->d_name;
  int cpplen = strlen(s) - 4;   // index of start of . in .cpp
  if (cpplen >= 0) {
    if (
	(strncmp(s+cpplen, ".jpg",4) == 0) ||
	(strncmp(s+cpplen, ".JPG",4) == 0))
      {
	return 1;
      }

  }

  return 0;
}

static int pngfilter(struct direct * dir)
// post: returns 1/true iff name of dir ends in .cpp    
{
	char * s = dir->d_name;
	int cpplen = strlen(s) - 4;   // index of start of . in .cpp
	if (cpplen >= 0) {
		if (
			(strncmp(s+cpplen, ".png",4) == 0) ||
			(strncmp(s+cpplen, ".png",4) == 0))
		{
			return 1;
		}
		
	}
	
	return 0;
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
  // Get file name without preceding path
  const std::string slash( "/" );
    
  uint32 s = fileName.find_last_of( slash );
  if ( s != std::string::npos ) {
    uint32 len = fileName.size() - s - 1;
    if ( len > 0 ) 
      return fileName.substr( s+1, len );
  }

  return fileName;
}

// Strip extension (.rfymov etc.) from filename
std::string rfStripExtension( const std::string& fileName )
{
  // Get file name without extension
  const std::string dot( "." );
    
  uint32 s = fileName.find_last_of( dot );
  if ( s != std::string::npos ) 
    return fileName.substr( 0, s );

  return fileName;
}

// get extension (.rfymov etc.) from filename
std::string rfGetExtension( const std::string& fileName )
{
  // Get file name without extension
  const std::string dot( "." );
    
  uint32 s = fileName.find_last_of( dot );
  if ( s != std::string::npos ) 
    return fileName.substr( s , std::string::npos );

  return fileName;
}


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

static int casenumericsort(const void *A, const void *B) {
  return numericsort(A, B, 0);
}

/*
 * 'numericsort()' - Compare directory entries with case-sensitivity.
 */

static int numericsort(const void *A, const void *B) {
  return numericsort(A, B, 1);
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

/*
Sample code for CUJ article,
"Custom Containers & Iterators for STL-Friendly Code"
(March 2005)
by Ethan McCallum


Copyright (c) 2004, Ethan McCallum.

Permission is granted to use this code without restriction as long as this
copyright notice appears in all source files.
*/

/*
Implementation of the rcGlob and rcGlobIterator classes.
*/


// - - - - - - - - - - - - - - - - - - - -


rcGlobIterator::rcGlobIterator()
	:
	sequence_( NULL ) ,
	current_()
{
	return ;
} // rcGlobIterator:: "end-range" ctor

rcGlobIterator::rcGlobIterator( char** sequence )
	:
	sequence_( sequence ) ,
	current_( *( sequence_ ) )
{

	++sequence_ ; // advance pointer here to simplify logic of operator++()
	return ;

} // rcGlobIterator:: "beginning-" or "mid-range" ctor

rcGlobIterator::rcGlobIterator( const rcGlobIterator& other )
	:
	sequence_( other.sequence_ ) ,
	current_( other.current_ )
{

	// it's safe to blindly copy the values from the other object;
	// sequence_ is invalidated only when the parent rcGlob container
	// changes, not when the other pointer changes.
	return ;

} // copy ctor


rcGlobIterator::self_reference rcGlobIterator::operator=( const rcGlobIterator& other ){

	if( this == &other ){
		return( *this ) ;
	}


	// it's safe to blindly copy the values from the other object;
	// sequence_ is invalidated only when the parent rcGlob container
	// changes, not when the other pointer changes.

	sequence_ = other.sequence_ ;
	current_ = other.current_ ;

	return( *this ) ;

} // assignment operator


rcGlobIterator::self_reference rcGlobIterator::operator++(){


	if( NULL != *sequence_ ){
		current_ = *sequence_ ;
		++sequence_ ;
	} else{
		current_ = "[past the end!]" ;
		sequence_ = NULL ;
	}

	return( *this ) ;

} // rcGlobIterator::operator++ (pre)

rcGlobIterator::self_type rcGlobIterator::operator++(int){

	self_type result( *this ) ;
	++( *this ) ;

	return( result ) ;

} // rcGlobIterator::operator++ (post)


bool rcGlobIterator::operator==( const self_reference other ){
	return( other.sequence_ == this->sequence_ ) ;
} // rcGlobIterator::oeprator==

bool rcGlobIterator::operator!=( const self_reference other ){
	return( ! ( (*this) == other ) ) ;
} // rcGlobIterator::operator!=

rcGlobIterator::reference rcGlobIterator::operator*(){
	return( current_ ) ;
} // rcGlobIterator::operator*()

rcGlobIterator::reference rcGlobIterator::operator->(){
	return( *( *this ) ) ;
} // rcGlobIterator::operator->

// - - - - - - - - - - - - - - - - - - - -


int rcGlob::globErrorHandler( const char* error_path , int error_num ){

	std::cerr << "Unable to traverse directory \"" << error_path << "\": " << std::strerror( error_num ) << std::endl ;

	/*
	return something nonzero to cause
	glob() to halt processing with a 
	GLOB_ABORT value
	*/
	return( 0 ) ;

} // rcGlob::globErrorHandler()


rcGlob::rcGlob()
	:
	glob_()
{

	return ;

} // rcGlob::ctor()

rcGlob::rcGlob( const std::string& inPattern )
	throw( std::runtime_error )
	:
	glob_()
{

	push_back( inPattern ) ;

	return ;

} // rcGlob::ctor( const std::string& )

rcGlob::~rcGlob()
	throw()
{

	globfree( &glob_ ) ;

	return ;

} // rcGlob::dtor


void rcGlob::push_back( const std::string& inPattern )
	throw( std::runtime_error )
{

	/*
	the glob() call both initiates a glob_t object and, when using
	the GLOB_DOOFFS and GLOB_APPEND flags, appends additional matches
	to the match list (glob_t.gl_pathv).
	*/

	int globResult = glob(
		inPattern.c_str() ,
		GLOB_MARK | GLOB_DOOFFS | GLOB_APPEND , 
		rcGlob::globErrorHandler ,
		&glob_
	) ;

	if( 0 != globResult ){
		throw( std::runtime_error( "Encountered error expanding pattern" ) ) ;
	}

	return ;

} // rcGlob::push_back()


rcGlob::const_iterator rcGlob::begin() const {

	rcGlob::const_iterator result( glob_.gl_pathv ) ;
	return( result ) ;
	
} // rcGlob::begin()


rcGlob::const_iterator rcGlob::end() const {
	rcGlob::const_iterator result ;
	return( result ) ;
} // rcGlob::end()

rcGlob::size_type rcGlob::size() const throw() {

	return( glob_.gl_pathc ) ;

} // rcGlob::size()

// - - - - - - - - - - - - - - - - - - - -
// ----
