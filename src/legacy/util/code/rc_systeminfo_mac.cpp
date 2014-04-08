
/*
 *
 *$Id $
 *$Log$
 *
 * Copyright (c) 2007 Reify Corp. All rights reserved.
 */
#include <rc_systeminfo.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach-o/arch.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/host_info.h>
#include <mach/machine.h>
#include <vector>
#include <strstream>


static const int32 sHertzInGig = 1000000000;

static double get_procspeed (void);
static std::string get_architecture (void);
int32 get_numproc (void);


std::string rfGetHostInfo()
{
	std::strstream s;
	s << "Architecture: " << get_architecture () << std::endl;
	s << "Number of Processors: " << get_numproc () << std::endl;
	s << "Speed: " << get_procspeed ()  / (sHertzInGig) << std::endl;
#ifdef __ppc__
	s << "Byte Order: BigEndian " << std::endl;
#else
	s << "Byte Order: LittleEndian " << std::endl;  
#endif
	s << "Physical Memory (GB) : " << rfSystemRam () / (sHertzInGig) << std::endl;
	s << std::ends;
  return std::string (s.str());
}

double rfSystemRam ()
{
  uint64_t mem;
  size_t len = sizeof(mem);

  sysctlbyname("hw.memsize", &mem, &len, NULL, 0);
  return (double) mem;
}



// Platform byte order
reByteOrder rfPlatformByteOrder()
{
#if defined (__ppc__)
  return eByteOrderBigEndian;
#endif
  return eByteOrderLittleEndian;
}

// Platform byte order mark character
uint16 rfPlatformByteOrderMark()
{
#if defined (__ppc__)
  return rcBOM_BE;
#endif
  return rcBOM_LE;
}

// Platform byte order from BOM
reByteOrder rfPlatformByteOrder( const uint16& bom )
{
  switch ( bom ) {
  case rcBOM_BE:
    return eByteOrderBigEndian;
  case rcBOM_LE:
    return eByteOrderLittleEndian;
  default:
    return eByteOrderUnknown;
  }
}

	
int32 get_numproc (void)
{
  size_t len;
  int32 ncpu;
	
  len = sizeof(ncpu);
  sysctlbyname ("hw.ncpu",&ncpu,&len,NULL,0);
  return ncpu;
}
	

static std::string get_architecture (void)
{
  size_t len;
  int retcode;
	std::vector<char> buffer;

  retcode = sysctlbyname ("hw.machine",NULL,&len,NULL,0);
  if (retcode == -1) return std::string ("");
  buffer.resize (len);
  retcode = sysctlbyname ("hw.machine",&buffer[0],&len,NULL,0);
  return std::string (&buffer[0]);
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
