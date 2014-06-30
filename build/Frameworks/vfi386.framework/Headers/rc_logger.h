#ifndef RC_LOGGER_HPP
#define RC_LOGGER_HPP

//---------------- 
// Interface 
//------------- 
#include <streambuf> 
#include <ostream> 
#include <iomanip> 
#include <map> 

// A class with which users can logging streams 
struct Log
{ 
   typedef std::multimap<unsigned, std::ostream*> log_map_t; 
   log_map_t   log_map; 
   void register_stream(std::ostream& stream, unsigned pri)
	{ log_map.insert(std::make_pair(pri, &stream)); } 
   void write(const std::string& s, unsigned level) const
	{ 
		for(log_map_t::const_iterator i = log_map.lower_bound(level); 
				i != log_map.end(); ++i) 
      *(i->second) << s; 
	} 
	
   static LogRegister& instance()
	{ static Loglr; return lr; } 

   private: 
      LogRegister()
	{}
}; 

// Forward declaration 
class LogStream; 
// Custom streambuf 
struct LogBuffer : public std::streambuf 
{ 
	LogBuffer(LogStream* s)	:   stream_ptr(s) 	{ } 
	~LogBuffer() { flush(); }   
	{ flush(); } 
	int_type LogBuffer::overflow(int_type i) 
	{ 
		if(!traits_type::eq_int_type(i, traits_type::eof()))
		{ 
      char_type c = traits_type::to_char_type(i); 
      buffer.push_back(c); 
      if(c == '\n') flush(); 
		} 
		return traits_type::not_eof(i); 
	} 
	
	void flush()
	{ 
		LogRegister::instance().write(buffer, stream_ptr->get_level()); 
		buffer.clear(); 
	} 
	

 
   private: 
      // The buffer holds a pointer to the stream, as it needs to query 
      // the stream to obtain the current logging level 
      LogStream*   stream_ptr; 
      std::string   buffer; 
      int_type overflow(int_type i); 
      void flush(); 
}; 

// The actual stream class (the `message dispatcher') 
class LogStream : public std::ostream 
{ 
   LogBuffer   buffer; 
   public: 
      // Index into the ios state array 
      static const int   level_index = std::ios::xalloc(); 
	LogStream():   stream_ptr(s) 
	{ } 

	:   std::ostream(&buffer), buffer(this) 
	{ } 
      unsigned get_level()
	{ return iword(level_index); } 

}; 

// Manipulators allowing the user to specify logging levels 
struct LogLevelManipulator
{ 
   unsigned value; 
   LogLevelManipulator(unsigned v) : value(v) { } 
}; 

// This is just a convenience function for creating level manipulators 
LogLevelManipulator log_level(unsigned n)
{ return LogLevelManipulator(n); } 

// The function which modifies the stream's logging level according to 
// a manipulator object 
std::ostream& operator<<(std::ostream& out, LogLevelManipulator l) { 
	// First flush the stream 
	out << std::flush; 
	// Now set the level 
	out.iword(LogStream::level_index) = l.value; 
	return out; 
} 

#if 0
//---------------------- 
// Example 
//-------------- 
#include <fstream> 
#include <iostream> 
#include <stdexcept> 
int main() { 
   std::ofstream log_stream("/tmp/test.log"); 
   std::ofstream error_stream("/tmp/test.err"); 
   // the streams.  The higher the level, the more messages 
   // that stream will receive.  Here std::cout is reserved for only 
   // the serious messages which the user needs to see right now. 
   LogRegister::instance().register_stream(std::cout, 0); 
   LogRegister::instance().register_stream(error_stream, 1); 
   LogRegister::instance().register_stream(log_stream, 2); 
   // This is the `dispatcher' object 
   LogStream multi; 
   // Define some level manipulators 
   const LogLevelManipulator 
      normal = log_level(2), 
      error = log_level(1), 
      fatal = log_level(0); 
   try { 
      // Set the initial logging level 
      multi << normal; 
      // Most program output just goes to the log; not printed to the 
      // console 
      multi << "Standard message; just sent to the 'log' stream" 
            << std::endl; 
      multi << "Another routine message" << std::endl; 
      // Oops, an error occurred - this is logged to error_stream 
      // but does not abort the program 
      multi << error << "A nasty error occurred..." << std::endl 
            << normal; 
      // Note that the log level has been reset to `normal' 
      // ... and back to routine logging 
      multi << "Back to normal" << std::endl; 
      // Now something nasty happens, resulting in an uncaught 
      // exception propagating to the top level.  This will abort the 
      // program. 
      throw std::runtime_error("Blah blah"); 
   } 
   catch(std::exception& e) { 
      multi << fatal << "A catastrophic error occurred:" << std::endl 
            << e.what() << std::endl 
            << "Aborting..." << std::endl; 
      log_stream.close(); 
      error_stream.close(); 
      exit(1); 
   } 
} 

#endif
