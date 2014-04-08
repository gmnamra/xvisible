
#ifndef _visible_framework_core_H__
#define _visible_framework_core_H__

#include "rc_types.h"
#include <sys/param.h>
#include <mach-o/dyld.h>
#include <boost/scoped_ptr.hpp>
#include "singleton.hpp"
#include "rc_exception.h"



using namespace std;

namespace Visible
{

	namespace framework
    {

            // A class to encapsulate a table of pre-computed squares
        class square_table : public std::vector<uint32>
        {
            enum { squares_table_size = 511 };

        public:
                // ctor
            square_table()
            {
                resize (squares_table_size);
                for (int i = 0; i < squares_table_size; i++)
                    at(i) = i * i;
            }
        };


        /**
         @brief
         The visible_framework_core class is a singleton used to configure the program basics.
         @remark
         You should only create this singleton once because it destroys the identifiers!
         */
        class visible_framework_core : public SingletonLite <visible_framework_core>
        {
        public:



            visible_framework_core() : _dev_run (false)
            {
                char buffer[1024];
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

            static const square_table& i_plus_m_table () { return instance()._8u_square_table; }


                //! Returns the path to the executable folder 
            static const std::string& getExecutablePath()
            { return instance()._executablePath; }
                //! Returns the path to the log files 

            static const std::string& getLogPath()
            { return instance()._logPath; }

                //! Return trrue for runs in the build directory (not installed)
            static bool isDevelopmentRun() { return instance()._dev_run; }


        private:
            square_table _8u_square_table;


                //! Path to the parent directory of the ones above if program was installed with relativ paths
            std::string _executablePath;        //!< Path to the executable
            std::string _logPath;               //!< Path to the log files folder
            bool                     _dev_run;               //!< True for runs in the build directory (not installed)
            
            visible_framework_core(const visible_framework_core&); 
            int                           _debug_level_logfile;      //!< The debug level for the log file (belongs to OutputHandler)
            
        };
        
        static const visible_framework_core* visible_framework_core_instance_ptr;
        static __attribute__ ((constructor)) void vf_init(void)
        {
            visible_framework_core c;
            visible_framework_core_instance_ptr = & c.instance ();
        }
    };
	
	
}

#endif /* _visible_framework_core_H__ */
