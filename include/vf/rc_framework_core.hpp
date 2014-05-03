
#ifndef _visible_framework_core_H__
#define _visible_framework_core_H__

#include "rc_types.h"
#include <sys/param.h>
#include <mach-o/dyld.h>
#include <boost/scoped_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include "Timer.h"
#include "singleton.hpp"
#include "rc_exception.h"


using namespace boost;
using namespace boost::date_time;
using namespace std;


/**
 @brief
 The visible_framework_core class is a singleton used to configure the program basics.
 @remark
 You should only create this singleton once because it destroys the identifiers!
 */
class RFY_API visible_framework_core : public SingletonLite <visible_framework_core>
{
public:
    
    
    
    visible_framework_core() : _dev_run (false)
    {
        char buffer[1024];
        _epoch_time = boost::posix_time::ptime (boost::gregorian::date (1970, 1, 1));
        
        //   TimeStamp::frequency = ticks_per_second (); // using microssecond clock
        
        uint32_t path_len = 1023;
        if (_NSGetExecutablePath(buffer, &path_len))
            rmExceptionMacro (<< " Could not retrieve executable path" );
        
        _executablePath =  std::string (buffer, strlen (buffer));
        std::cout << "vf_init: " << _executablePath << std::endl;
        
        
    }
    
    ~visible_framework_core()
    {
        std::cout << "vf_end: " << _executablePath << std::endl;
    }
    
    //! Returns the path to the executable folder 
    static const std::string& getExecutablePath()
    { return instance()._executablePath; }
    //! Returns the path to the log files 
    
    static const std::string& getLogPath()
    { return instance()._logPath; }
    
    //! Return trrue for runs in the build directory (not installed)
    static bool isDevelopmentRun() { return instance()._dev_run; }
    
    static bool force_simd (bool v) { instance()._force_simd_if_present = v; return instance()._force_simd_if_present; }
    
    /**
     *   Absolute time
     */
    double get_absolute_time ()
    {
        boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time ();
        return (current_time - _epoch_time).total_nanoseconds () * 1.0e-9;
    }
    
    boost::posix_time::ptime get_ptime () 
    {
        return boost::posix_time::microsec_clock::local_time ();
    }
    
    int64 get_ticks () 
    {
        boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time ();
        return (current_time - _epoch_time).ticks();
        
    }

    // Timestamps are ticks from epoch. Check to see if it is at least seconds after epoch
    bool check_distance_from_epoch (const int64& base, int64 seconds_after)
    {
        return base > seconds_after * ticks_per_second();
    }
    
    int64 ticks_per_second ()
    {
        return micro_res::to_tick_count(0,0,1,0);
    }
    
    
private:
    boost::posix_time::ptime _epoch_time;
    
    //! Path to the parent directory of the ones above if program was installed with relativ paths
    std::string _executablePath;        //!< Path to the executable
    std::string _logPath;               //!< Path to the log files folder
    bool                     _dev_run;               //!< True for runs in the build directory (not installed)
    
    visible_framework_core(const visible_framework_core&); 
    int                           _debug_level_logfile;      //!< The debug level for the log file (belongs to OutputHandler)
    bool                          _force_simd_if_present;
    
};

static const visible_framework_core* visible_framework_core_instance_ptr;
static __attribute__ ((constructor)) void vf_init(void)
{
    visible_framework_core c;
    visible_framework_core_instance_ptr = & c.instance ();
}



#endif /* _visible_framework_core_H__ */
