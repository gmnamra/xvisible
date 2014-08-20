#ifndef __VF_UTILS__
#define __VF_UTILS__


#include <boost/algorithm/string.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/math/special_functions.hpp>
#include "rc_fileutils.h"
#include "stlplus_lite.hpp"
#include "rc_filegrabber.h"
#include <random>
#include <iterator>
#include "sshist.hpp"
#include <vector>
#include <list>
#include <queue>
#include <fstream>
#include <mutex>
#include <condition_variable>

#include <boost/operators.hpp>
#include <ctime>

#include "rc_types.h"
#include <sys/param.h>
#include <mach-o/dyld.h>
#include <boost/scoped_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include "Timer.h"
#include "singleton.hpp"
#include "rc_exception.h"

//using namespace ci;
using namespace std;



namespace vf_utils
{

#define HAVE_MICROSEC_CLOCK
    
        class time_spec_t : boost::additive<time_spec_t>, boost::totally_ordered<time_spec_t>
    {
        public:
            
            static time_spec_t get_system_time(void);
            
            time_spec_t(double secs = 0);
            
            time_spec_t(time_t full_secs, double frac_secs = 0);
            
            time_spec_t(time_t full_secs, long tick_count, double tick_rate);
            
            long get_tick_count(double tick_rate) const
        {
            return boost::math::iround(this->get_frac_secs()*tick_rate);
        }
            
            double get_real_secs(void) const
        {
            return this->_full_secs + this->_frac_secs;
        }
            
            time_t get_full_secs(void) const
        {
            return this->_full_secs;
        }
            
            double get_frac_secs(void) const
        {
            return this->_frac_secs;
        }
            
        time_spec_t &operator+=(const time_spec_t & rhs);
        
            
        time_spec_t &operator-=(const time_spec_t & rhs);
        
            bool operator==(const time_spec_t & rhs)
        {
            return get_full_secs() == rhs.get_full_secs() and get_frac_secs() == rhs.get_frac_secs();
        }
        
            bool operator<(const time_spec_t & rhs)
        {
            return (
                    (get_full_secs() < rhs.get_full_secs()) or ((get_full_secs() == rhs.get_full_secs()) and
                    (get_frac_secs() < rhs.get_frac_secs()) ));
        }

        
            //private time storage details
        private: time_t _full_secs; double _frac_secs;
        };
    
        

    
    
    /**
     @brief
     The visible_framework_core class is a singleton used to configure the program basics.
     @remark
     You should only create this singleton once because it destroys the identifiers!
     */
    class civf : public SingletonLite <civf>
    {
    public:
        
        
        
        civf() : _dev_run (false)
        {
            char buffer[1024];
            _epoch_time = boost::posix_time::ptime (boost::gregorian::date (1970, 1, 1));
            
            //   TimeStamp::frequency = ticks_per_second (); // using microssecond clock
            
            uint32_t path_len = 1023;
            if (_NSGetExecutablePath(buffer, &path_len))
                rmExceptionMacro (<< " Could not retrieve executable path" );
            
            _executablePath =  std::string (buffer, strlen (buffer));
            std::cout << "civf_init: " << _executablePath << std::endl;
            
            
        }
        
        ~civf()
        {
            std::cout << "civf_end: " << _executablePath << std::endl;
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
        
        civf(const civf&);
        civf& operator=( const civf& );
        
        int                           _debug_level_logfile;      //!< The debug level for the log file (belongs to OutputHandler)
        bool                          _force_simd_if_present;
        
    };
    
    
 
    // Need this useless class for explicitly instatiating template constructor
    template <typename T>
    struct id {
        typedef T type;
    };

    namespace math
    {
        template<typename Iter>
        inline
        bool contains_nan (Iter start, Iter end)
        {
            while (start != end)
            {
                if (boost::math::isnan(*start)) return true;
                ++start;
            }
            return false;
        }
    }
    
    namespace general_movie
    {
        struct info
        {
            uint32 mEmbeddedCount;
            uint32 mWidth;
            uint32 mHeight;
            double mFps;
            double mTscale;
        };

    }
    
    namespace csv
    {
        typedef std::vector<float> rowf_t;
        typedef std::vector<rowf_t> matf_t;
        typedef std::vector<std::string> row_type;
        typedef std::vector<row_type> rows_type;
        
        // c++11
        static bool is_number(const std::string& s)
        {
            return !s.empty() && std::find_if(s.begin(),
                                              s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
        }
        
        //! Convert an input stream to csv rows.
        
        static rows_type to_rows(std::ifstream &input)
        {
            if (!input.is_open())
                perror("error while opening file");
            
            csv::rows_type rows;
            std::string line;
            //for each line in the input stream
            while (std::getline(input, line))
            {
                boost::trim_if(line, boost::is_any_of("\t\r "));
                csv::row_type row;
                boost::split(row, line, boost::is_any_of("\t\r ,"));
                
                rows.push_back(row);
            }
            if (input.bad())
                perror("error while reading file");
            
            // Handle cases where file conained one long delimated sequence of numbers
            // which were parsed in to 1 row and many columns
            if ( ! rows.empty () && (rows[0].size() / rows.size() ) == rows[0].size() )
            {
                int columns = rows[0].size ();
                assert (rows.size () == 1);
                rows_type crows;
                const row_type& col = rows[0];
                for (int i = 0; i < columns; i++)
                {
                    row_type row (1, col[i]);
                    crows.push_back (row);
                }
                
                rows = crows;
            }
            
            return rows;
        }
        
        // Returns starting row of numeric data as a positive number if it is a legacy visible file or
        // -1 * number of rows if it is not
        static int is_legacy_visible_output (csv::rows_type& rows)
        {
            if (rows.size () < 1) return false;
            const csv::row_type& row = rows[0];
            bool is_visible = false;
            for (int i = 0; i < row.size(); i++)
            {
                if (row[i].find ("Visible") != string::npos ) { is_visible = true; break; }
            }
            if (! is_visible ) return -1 * rows.size ();
            int last_row = -1;
            for (int rr = 1; rr < rows.size(); rr++)
            {
                const csv::row_type& row = rows[rr];
                for (int i = 0; i < row.size(); i++)
                {
                    if (row[i].find ("seconds") != string::npos )
                    {
                        last_row = rr + ( rr < (rows.size() - 1) ? 1 : 0) ; break;
                    }
                }
                if (last_row > 0) break;
            }
            return last_row;
        }
        
        // Returns starting row of numeric data as a positive number if it is a legacy visible file or
        // -1 * number of rows if it is not
        static int file_is_legacy_visible_output (std::string& fqfn)
        {
            std::ifstream istream (fqfn);
            csv::rows_type rows = csv::to_rows (istream);
            return is_legacy_visible_output ( rows);
            
        }
        
        static bool validate_matf (matf_t tm)
        {
            if (tm.empty()) return false;
            size_t d = tm.size();
            if (d <= 0 || d > 10000 ) return false; // arbitrary upper limit. I know !!
            for (int rr=0; rr < d; rr++)
                if (tm[rr].size() != d) return false;
            return true;
            
        }
        
        
        static bool csv2vectors ( const std::string &csv_file, std::vector<vector<float> >& m_results, bool only_visible_format, bool columns_if_possible, bool verbose = false)
        {
            {
                m_results.resize (0);
                std::ifstream istream (csv_file);
                csv::rows_type rows = csv::to_rows (istream);
                int visible_row = is_legacy_visible_output (rows);
                int start_row = visible_row < 0 ? 0 : visible_row;
                if ( only_visible_format && visible_row < 0 ) return false;
                if (visible_row < 0 ) assert (start_row >= 0 && start_row < rows.size ());
                
                
                
                std::vector<vector<float> > datas;
                std::vector<vector<float> > column_wise;
                datas.resize (0);
                column_wise.resize (0);
                
                sshist  column_width (16);
                
                // Get All rows
                for (int rr = start_row; rr < rows.size(); rr++)
                {
                    const csv::row_type& row = rows[rr];
                    vector<float> data;
                    for (int t = 0; t < row.size(); t++)
                    {
                        char *end_ptr;
                        float f = strtof(row[t].c_str(), &end_ptr);
                        if (end_ptr == row[t].c_str()) continue;
                        data.push_back (f);
                    }
                    datas.push_back(data);
                    column_width.add (data.size());
                }
                
                sshist::sorted_container_t shist;
                column_width.get_value_sorted (shist);
                if (verbose)
                {
                    for (auto pt : shist) std::cout << "[" << pt.first << "]: " << pt.second << std::endl;
                }
                
                if (! shist.empty () && datas.size() && columns_if_possible) // if there were all same number of columns
                {
                    uint32 cw = shist.front().first;
                    column_wise.resize(cw);
                    for (uint i = 0; i < datas.size (); i++)
                    {
                        const vector<float>& vc = datas[i];
                        if (vc.size () != cw) continue;
                        
                        for (uint cc=0; cc<cw;cc++)
                            column_wise[cc].push_back(vc[cc]);
                    }
                }
                
                if (!column_wise.empty ())
                {
                    assert (columns_if_possible);
                    m_results = column_wise;
                }
                else if ( ! datas.empty () )
                {
                    m_results = datas;
                }
                
                return ! m_results.empty ();
            }
            
        }
    }
     namespace gen_filename
    {
        
        static bool insensitive_case_compare (const std::string& str1, const std::string& str2)
        {
            for(unsigned int i=0; i<str1.length(); i++){
                if(toupper(str1[i]) != toupper(str2[i]))
                    return false;
            }
            return true;
        }
        
        class random_name
        {
            
            std::string _chars;
            std::mt19937 mBase;
        public:
            
            random_name ()
            {
                _chars = std::string (
                                      "abcdefghijklmnopqrstuvwxyz"
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                      "1234567890");
            }
            
            int32_t nextInt( int32_t v )
            {
                if( v <= 0 ) return 0;
                return mBase() % v;
            }
            
            std::string get_anew ()
            {
                std::string ns;
                for(int i = 0; i < 8; ++i) ns.push_back (_chars[nextInt(_chars.size()-1)]);
                rmAssert (ns.length() == 8);
                return ns;
            }
        };
        
    }
    
    namespace dict
    {
        
        /*!
         * A templated dictionary class with a python-like interface.
         */
        template <typename Key, typename Val> class dict
        {
        public:
            /*!
             * Create a new empty dictionary.
             */
            dict(void);
            
            /*!
             * Input iterator constructor:
             * Makes boost::assign::map_list_of work.
             * \param first the begin iterator
             * \param last the end iterator
             */
            template <typename InputIterator>
            dict(InputIterator first, InputIterator last);
            
            /*!
             * Get the number of elements in this dict.
             * \return the number of elements
             */
            std::size_t size(void) const;
            
            /*!
             * Get a list of the keys in this dict.
             * Key order depends on insertion precedence.
             * \return vector of keys
             */
            std::vector<Key> keys(void) const;
            
            /*!
             * Get a list of the values in this dict.
             * Value order depends on insertion precedence.
             * \return vector of values
             */
            std::vector<Val> vals(void) const;
            
            /*!
             * Does the dictionary contain this key?
             * \param key the key to look for
             * \return true if found
             */
            bool has_key(const Key &key) const;
            
            /*!
             * Get a value in the dict or default.
             * \param key the key to look for
             * \param other use if key not found
             * \return the value or default
             */
            const Val &get(const Key &key, const Val &other) const;
            
            /*!
             * Get a value in the dict or throw.
             * \param key the key to look for
             * \return the value or default
             */
            const Val &get(const Key &key) const;
            
            /*!
             * Set a value in the dict at the key.
             * \param key the key to set at
             * \param val the value to set
             */
            void set(const Key &key, const Val &val);
            
            /*!
             * Get a value for the given key if it exists.
             * If the key is not found throw an error.
             * \param key the key to look for
             * \return the value at the key
             * \throw an exception when not found
             */
            const Val &operator[](const Key &key) const;
            
            /*!
             * Set a value for the given key, however, in reality
             * it really returns a reference which can be assigned to.
             * \param key the key to set to
             * \return a reference to the value
             */
            Val &operator[](const Key &key);
            
            /*!
             * Pop an item out of the dictionary.
             * \param key the item key
             * \return the value of the item
             * \throw an exception when not found
             */
            Val pop(const Key &key);
            
        private:
            typedef std::pair<Key, Val> pair_t;
            std::list<pair_t> _map; //private container
        };

    }
    
    namespace thread_safe
    {
        template<typename Data>
        class concurrent_queue
        {
        private:
            std::queue<Data> the_queue;
            mutable std::mutex the_mutex;
            std::condition_variable the_condition_variable;
        public:
            
            concurrent_queue () {}
            
            void push(Data const& data)
            {
                std::lock_guard<std::mutex> lock(the_mutex);
                the_queue.push(std::move(data));
                the_condition_variable.notify_one();
            }
            
            bool empty() const
            {
                std::lock_guard<std::mutex> lock(the_mutex);
                return the_queue.empty();
            }
            
            bool try_pop(Data& popped_value)
            {
                std::lock_guard<std::mutex> lock(the_mutex);
                if(the_queue.empty())
                {
               //     std::cout << "poping on empty " << std::endl;
                    return false;
                }
                
                popped_value=std::move (the_queue.front());
                the_queue.pop();
             //static_cast<int32>(status)    std::cout << "poped " << std::endl;
                return true;
            }
            
            void wait_and_pop(Data& popped_value)
            {
                std::unique_lock<std::mutex> lock(the_mutex);
                the_condition_variable.wait (lock, [this] { return ! empty(); } );
                std::cout << "poped " << std::endl;                
                popped_value=std::move (the_queue.front());
                the_queue.pop();
            }
            
        };
        
        struct cancelled_error {};
        
        class cancellation_point
        {
        public:
            cancellation_point(): stop_(false) {}
            
            void cancel() {
                std::unique_lock<std::mutex> lock(mutex_);
                stop_ = true;
                cond_.notify_all();
            }
            
            template <typename P>
            void wait(const P& period) {
                std::unique_lock<std::mutex> lock(mutex_);
                if (stop_ || cond_.wait_for(lock, period) == std::cv_status::no_timeout) {
                    stop_ = false;
                    throw cancelled_error();
                }
            }
        private:
            bool stop_;
            std::mutex mutex_;
            std::condition_variable cond_;
        };

    }
}

// @todo fix this #include "dict.ipp"

#endif

