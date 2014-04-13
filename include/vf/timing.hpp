#ifndef _TIMING_H_
#define _TIMING_H_

#include <cmath>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>


  /** \brief Posix based timer. Reports time since last reset
    * 
    */
  class RFY_API chronometer
  {
    public:
      chronometer ()
      {
        reset ();
      }

      inline double
      getTime ()
      {
        boost::posix_time::ptime end_time = boost::posix_time::microsec_clock::local_time ();
          return ((double) ((end_time - start_time_).total_microseconds()));
      }

     
      inline void
      reset ()
      {
        start_time_ = boost::posix_time::microsec_clock::local_time ();
      }

    protected:
      boost::posix_time::ptime start_time_;
  };

  /** \brief ScopeTimer
    *
    * Uses destructor to read time from ctor
    * create an instance at the beginning of the function. Example:
    *
    * {
    *   qInv::ScopeTime t1 ("doit");
    *   // ... 
    * }
    */
  class RFY_API ScopeTimer : public chronometer
  {
    public:
      inline ScopeTimer (const char* title)
      {
        title_ = std::string (title);
        start_time_ = boost::posix_time::microsec_clock::local_time ();
      }

      inline ScopeTimer ()
      {
        start_time_ = boost::posix_time::microsec_clock::local_time ();
      }

      inline ~ScopeTimer ()
      {
        double val = this->getTime ();
        std::cerr << title_ << " took " << val << "ms.\n";
      }

    private:
      std::string title_;
  };


#ifndef MEASURE_FUNCTION_TIME
#define MEASURE_FUNCTION_TIME \
  ScopeTime scopeTime(__func__)
#endif

/**
  *   Absolute time
  */
inline double 
getTime ()
{
  boost::posix_time::ptime epoch_time (boost::gregorian::date (1970, 1, 1));
  boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time ();
  return (current_time - epoch_time).total_nanoseconds () * 1.0e-9;
}

/// Executes code, only if secs are gone since last exec.
#ifndef DO_EVERY_TS
#define DO_EVERY_TS(secs, currentTime, code) \
if (1) {\
  static double s_lastDone_ = 0.0; \
  double s_now_ = (currentTime); \
  if (s_lastDone_ > s_now_) \
    s_lastDone_ = s_now_; \
  if ((s_now_ - s_lastDone_) > (secs)) {        \
    code; \
    s_lastDone_ = s_now_; \
  }\
} else \
  (void)0
#endif

/// Executes code, only if secs are gone since last exec.
#ifndef DO_EVERY
#define DO_EVERY(secs, code) \
  DO_EVERY_TS(secs, pcl::getTime(), code)
#endif




#endif  


