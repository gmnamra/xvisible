/*
 *
 * $Id: rc_main.cpp 6025 2008-09-08 01:14:07Z arman $
 *
 * This file contains implementation for an encrypted host info generator.
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <sys/time.h>
#include <stdio.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <strstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach-o/arch.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/host_info.h>
#include <mach/machine.h>
#include <vector>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>



// Note: variable and function names have been obfuscated to
// make it more difficult to determine how the executable works

static const unsigned int mT = 8;
static const unsigned int mB = 64;

// The actual key is ""y/fnr.8v"
// We do a bit of scrambling here...
// The correct char positions are 8, 12, 33, 2, 44, 32, 23, 47
static const char a[] = "2lnkjhtdy5+1/34u dgionu893475h...fdqpwnq/yber34vx.(85#er";

using namespace std;

// We do not want to bring in other libraries. So just enough to get some host info
static const int sHertzInGig = 1000000000;

static double get_procspeed (void);
static std::string get_architecture (void);
static int get_numproc (void);
static double rfSystemRam ();

static std::string rfGetHostInfo()
{
  strstream s;
  s << "Architecture: " << get_architecture () << endl;
  s << "Number of Processors: " << get_numproc () << endl;
  s << "Speed: " << get_procspeed ()  / (sHertzInGig) << endl;
#ifdef __ppc__
  s << "Byte Order: BigEndian " << endl;
#else
  s << "Byte Order: LittleEndian " << endl;
#endif
  s << "Physical Memory (GB) : " << rfSystemRam () / (sHertzInGig) << endl;
  s << ends;
std::cout << std::string (s.str()) << std::endl;
	
  return string (s.str());
}

static double rfSystemRam ()
{
  uint64_t mem;
  size_t len = sizeof(mem);

  sysctlbyname("hw.memsize", &mem, &len, NULL, 0);
  return (double) mem;
}


int get_numproc (void)
{
  size_t len;
  int ncpu;

  len = sizeof(ncpu);
  sysctlbyname ("hw.ncpu",&ncpu,&len,NULL,0);
  return ncpu;
}


static string get_architecture (void)
{
  size_t len;
  int retcode;
  vector<char> buffer;

  retcode = sysctlbyname ("hw.machine",NULL,&len,NULL,0);
  if (retcode == -1) return string ("");
  buffer.resize (len);
  retcode = sysctlbyname ("hw.machine",&buffer[0],&len,NULL,0);
  return string (&buffer[0]);
}

static double get_procspeed (void)
{
  size_t len;
  uint64_t procspeed;
  int retcode;

  len = sizeof(procspeed);
  retcode = sysctlbyname ("hw.cpufrequency",&procspeed,&len,NULL,0);
  if (retcode == -1) return - 1.0;

  // procspeed is returned in hertz we should return Mhz
  return (double)procspeed;
}

static   double mGetTime ( void )
   {
    struct timeval t;

    // Warning: gettimeofday values do not always monotonically increase
    gettimeofday( &t, NULL );
    uint64_t time = t.tv_sec + t.tv_usec;
    return (double) time * 1e-6;
   }

/*
 * This example shows an FTP upload, with a rename of the file just after
 * a successful upload.
 *
 * Example based on source code provided by Erick Nuwendam. Thanks!
 */

#define UPLOAD_FILE_AS  "while-uploading.txt"
#define REMOTE_URL      "ftp://reify.serveftp.net/"  UPLOAD_FILE_AS
#define RENAME_FILE_TO  "renamed-and-fine.txt"

static int curlit (char *LOCAL_FILE)
{
  CURL *curl;
  CURLcode res;
  FILE * hd_src ;
  int hd ;
  struct stat file_info;
  char buf_2[2096];
  static char *defaultUser = "defaultUser";
  // Use current time in seconds as a pseudo-unique prefix
  double secs = mGetTime ();
  char *loginStr = 0;
  loginStr = getlogin ();
  loginStr = (loginStr == NULL) ? defaultUser : loginStr;
  snprintf( buf_2, sizeof (buf_2), "RNTO %s%d.txt", loginStr, (unsigned int) (secs*1000000));

  struct curl_slist *headerlist=NULL;
  char buf_1 [] = "RNFR " UPLOAD_FILE_AS;

  /* get the file size of the local file */
  hd = open(LOCAL_FILE, O_RDONLY) ;
  fstat(hd, &file_info);
  close(hd) ;

  /* get a FILE * of the same file, could also be made with
     fdopen() from the previous descriptor, but hey this is just
     an example! */

  hd_src = fopen(LOCAL_FILE, "rb");

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* build a list of commands to pass to libcurl */
    headerlist = curl_slist_append(headerlist, buf_1);
    headerlist = curl_slist_append(headerlist, buf_2);

    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, TRUE) ;
    curl_easy_setopt(curl, CURLOPT_VERBOSE, FALSE) ;
    curl_easy_setopt(curl, CURLOPT_FTPPORT, "-") ;
    curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, FALSE) ;


    /* specify target */
    curl_easy_setopt(curl,CURLOPT_URL, REMOTE_URL);

    /* pass in that last of FTP commands to run after the transfer */
    curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

    curl_easy_setopt(curl, CURLOPT_USERPWD, "licenserobot:license4me");

    /* NOTE: if you want this example to work on Windows with libcurl as a
       DLL, you MUST also provide a read callback with
       CURLOPT_READFUNCTION. Failing to do so will give you a crash since a
       DLL may not use the variable's memory when passed in to it from an app
       like this. */

    /* Set the size of the file to upload (optional).  If you give a *_LARGE
       option you MUST make sure that the type of the passed-in argument is a
       curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
       make sure that to pass in a type 'long' argument. */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
		     (curl_off_t)file_info.st_size);

    /* Now run off and do what you've been told! */
    res = curl_easy_perform(curl);

    /* clean up the FTP commands list */
    curl_slist_free_all (headerlist);

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(hd_src); /* close the local file */

  curl_global_cleanup();
  return 0;
}



using namespace std;

// Returns an iterator containing the primary (built-in) Ethernet interface. The caller is responsible for
// releasing the iterator after the caller is done with it.
static kern_return_t FEI(io_iterator_t *matchingServices)
{
  kern_return_t		kernResult;
  mach_port_t			masterPort;
  CFMutableDictionaryRef	matchingDict;
  CFMutableDictionaryRef	propertyMatchDict;

  // Retrieve the Mach port used to initiate communication with I/O Kit
  kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
  if (KERN_SUCCESS != kernResult)
    {
      printf("Error: IOMasterPort returned %d\n", kernResult);
      return kernResult;
    }

  matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);

  if (NULL == matchingDict)
    {
      printf("Error: IOServiceMatching returned a NULL dictionary.\n");
    }
  else {
    propertyMatchDict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
						   &kCFTypeDictionaryKeyCallBacks,
						   &kCFTypeDictionaryValueCallBacks);

    if (NULL == propertyMatchDict)
      {
	printf("Error: CFDictionaryCreateMutable returned a NULL dictionary.\n");
      }
    else {
      CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface), kCFBooleanTrue);
      CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey), propertyMatchDict);
      CFRelease(propertyMatchDict);
    }
  }

  kernResult = IOServiceGetMatchingServices(masterPort, matchingDict, matchingServices);
  if (KERN_SUCCESS != kernResult)
    {
      printf("Error: IOServiceGetMatchingServices returned %d\n", kernResult);
    }

  return kernResult;
}

// Given an iterator across a set of Ethernet interfaces, return the MAC address of the last one.
static kern_return_t GMA(io_iterator_t intfIterator, UInt8 *MACAddress)
{
  io_object_t		intfService;
  io_object_t		controllerService;
  kern_return_t	kernResult = KERN_FAILURE;

  // IOIteratorNext retains the returned object, so release it when we're done with it.
  while ((intfService = IOIteratorNext(intfIterator)))
    {
      CFTypeRef	MACAddressAsCFData;
      // IORegistryEntryGetParentEntry retains the returned object, so release it when we're done with it.
      kernResult = IORegistryEntryGetParentEntry( intfService,
						  kIOServicePlane,
						  &controllerService );

      if (KERN_SUCCESS != kernResult)
        {
	  printf("Error: IORegistryEntryGetParentEntry returned 0x%08x\n", kernResult);
        }
      else {
	// Retrieve the MAC address property from the I/O Registry in the form of a CFData
	MACAddressAsCFData = IORegistryEntryCreateCFProperty( controllerService,
							      CFSTR(kIOMACAddress),
							      kCFAllocatorDefault,
							      0);
	if (MACAddressAsCFData)
	  {
	    // Get the raw bytes of the MAC address from the CFData
	    CFDataGetBytes((CFDataRef)MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
	    CFRelease(MACAddressAsCFData);
	  }

	// Done with the parent Ethernet controller object so we release it.
	(void) IOObjectRelease(controllerService);
      }

      // Done with the Ethernet interface object so we release it.
      (void) IOObjectRelease(intfService);
    }

  return kernResult;
}


// Get host id
// Allocates the returned buffer, caller must free it
static char* gh()
{
  kern_return_t	kernResult = KERN_SUCCESS; // on PowerPC this is an int (4 bytes)
  char* h = 0;
  size_t len = kIOEthernetAddressSize;
  if ( len < mT )
    len = mT;

  // Allocate memory for id
  UInt8* buf = new UInt8[len];

  io_iterator_t	intfIterator;
  // Initialize the returned address
  memset(buf, 0, len);

  kernResult = FEI(&intfIterator);

  if ( KERN_SUCCESS == kernResult ) {
    kernResult = GMA(intfIterator, buf);

    if ( KERN_SUCCESS == kernResult ) {
      if ( buf ) {
	// No hexification
	h = new char[len+1];
	// Null-terminate
	h[len*2] = 0;
	memcpy( h, buf, len );
      }
      delete [] buf;
    } else {
      printf("error: GMA returned 0x%08x\n", kernResult);

    }
  } else {
    printf("error: FEI returned 0x%08x\n", kernResult);
  }

  (void) IOObjectRelease(intfIterator);	// Release the iterator.

  return h;
}


static void bB( const char* input, char* output )
{
  // Produce a 64-byte binary array from 8 bytes
  for ( unsigned int byte = 0; byte < mT; byte++ ) {
    for ( unsigned int bit = 0; bit < 8; bit++ ) {
      output[byte*8+bit] = (input[byte] & (1 << bit)) > 0 ? 1 : 0;
    }
  }
}

static void rB( const char* input, char* output )
{
  unsigned int byte;

  // Produce a 8-byte array from a 64-byte binary array
  for ( byte = 0; byte < mT; byte++ ) {
    char b = 0;
    for ( unsigned int bit = 0; bit < 8; bit++ ) {
      if ( input[byte*8+bit] )
	b += (1 << bit);
    }
    output[byte] = b;
  }
  output[byte] = 0;
}

static int x( char* input,
              char* output,
              bool encr )
{
  char tmp[mB];

  // Store input buffer because encrypt overwrites it
  memcpy( tmp, input, mB );
  // Encrypt/decrypt
  encrypt( input, encr );
  memcpy( output, input, mB );
  // Restore input buffer
  memcpy( input, tmp, mB );
  return 0;
}


int main( int argc, char** argv )
{
  int result = 0;
  char hostid_binary[mB];
  char key[mB];
  char* hid = gh();

  if ( argc == 2 ) {
    char* a = argv[1];
    if ( a && strlen( a ) == 2 ) {
      // Secret option
      if ( a[0] == '-' && a[1] == '6' ) {
	// Display unencrypted hostid in hex
	fprintf( stdout, "Host: " );
	for ( unsigned int i = 0; i < mT; i++ ) {
	  unsigned char c = hid[i];
	  fprintf( stdout, "%.2x", unsigned(c) );
	}
	fprintf( stdout, "\n" );
      }
    }
  }
  if ( argc == 3 ) {
    char* a = argv[1];
    if ( a && strlen( a ) == 2 ) {
      if ( a[0] == '-' && a[1] == '-' ) {
	char *b = argv[2];
	if ( b && strlen( b ) == 12 ) {
	  for ( unsigned int i = 0; i < mT; i++ ) {
	    hid[i] = b[i];
	  }
	  fprintf( stdout, "This Host: " );
	  for ( unsigned int i = 0; i < mT; i++ ) {
	    fprintf( stdout, "%.2x", hid[i] );
	  }
	  fprintf( stdout, "\n" );
	}
      }
    }
  }

  if ( hid ) {
    // Build binary input array
    bB( hid, hostid_binary );
    // Build binary encryption key array
    // Do a bit of scrambling here
    // The correct key byte positions are 8, 12, 33, 2, 44, 32, 23, 47
    char b[mT+1] = "985nlsjf";
    b[0] = a[15];
    b[6] = a[23];
    b[0] = a[8];
    b[3] = a[2];
    b[5] = a[44];
    b[2] = a[33];
    b[4] = a[44];
    b[7] = a[1];
    b[1] = a[12];
    b[7] = a[47];
    b[5] = a[32];

    bB( b, key );

    // Use host info key
    setkey( key );
    if ( 1 ) {
      char encrypted[mB];
      result += x( hostid_binary, encrypted, 0 );

      char text[mB];
      rB( encrypted, text );


      // create a tmp file and scp it to me.
      ofstream rawFile ("/Users/Shared/rfyhostinfo.txt");
      ostream& output1 = (rawFile.is_open()) ? rawFile : cerr;
      string hostinfo = rfGetHostInfo();
      output1 << hostinfo;

      for ( unsigned int i = 0; i < mT; i++ ) {
	unsigned char c = text[i];
	output1 << setw (2) << hex << unsigned(c);

      }
      output1 << endl;
      curlit ("/Users/Shared/rfyhostinfo.txt");

    } else {
      fprintf( stderr, "error: sk() failed\n" );
      result++;
    }
  } else {
    fprintf( stderr, "error: GMA() failed\n" );
    result++;
  }

  delete [] hid;

  return result;
}



#if 0


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <strstream>
#include <iomanip>
#include <CoreFoundation/CoreFoundation.h>

// Returns the serial number as a CFString.
// It is the caller's responsibility to release the returned CFString when done with it.
void CopySerialNumber(CFStringRef *serialNumber)
{
  if (serialNumber != NULL) {
    *serialNumber = NULL;

    io_service_t    platformExpert = IOServiceGetMatchingService(kIOMasterPortDefault,
								 IOServiceMatching("IOPlatformExpertDevice"));

    if (platformExpert) {
      CFTypeRef serialNumberAsCFString =
	IORegistryEntryCreateCFProperty(platformExpert,
					CFSTR(kIOPlatformSerialNumberKey),
					kCFAllocatorDefault, 0);
      if (serialNumberAsCFString) {
	*serialNumber = serialNumberAsCFString;
      }

      IOObjectRelease(platformExpert);
    }
  }
}

#endif
