// Copyright 2002 Reify, Inc.


#include <ut_rect.h>
#include <ut_thread.h>
#include <ut_atomic.h>
#include <ut_simd.h>
#include <ut_stats.h>
#include <ut_timestamp.h>
#include <ut_systeminfo.h>
#include <ut_rect.h>

#include <rc_fileutils.h>

#if defined(TIFF_TEST)
#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif
  extern int short_tag_main(int argc, char **argv);
  extern int ascii_tags_main(int argc, char **argv);
  extern int long_tag_main(int argc, char **argv);
  extern int strip_rw_main(int argc, char **argv);

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif
#endif



/// basic functor to print the current STLGlobIterator item, aka a matched file path
class ShowItem : public std::unary_function< const std::string& , void > {

public:
  /**
     \brief Basic ctor
     \param prefix text to prefix each printed item
     \param s output stream through which the line will be printed
  */
  ShowItem( const std::string& prefix , std::ostream& s )
    :
    prefix_( prefix ) ,
    out_( s )
  {
    return ;
  }

  /// print the matched filename (provided by the iterator)
  void operator()( const std::string& item ){
    out_ << prefix_ << ": \"" << item << "\"" << std::endl ;
    return ;
  } // operator()


protected:
  // empty


private:
  const std::string prefix_ ;
  std::ostream& out_ ;

} ; // class ShowItem


/// return values for main()
enum {
  ERROR_ARGS = 1
} ;

static bool nocase_compare (char c1, char c2)
{
	return toupper(c1) == toupper (c2);
}


int ut_util( std::string& resourcePath )
{


    
  uint32 errors = 0;

  try {

    {
#ifdef NDEBUG
      // In a non-debug build this assertion should be a NOP
      rmAssertDebug( 0 );
#endif            
    }
		
    {
      std::string foo ("foo");
      std::string FOO ("FOO");
      if (equal (foo.begin (), foo.end (), FOO.begin (), nocase_compare) )
	cerr << "Passed std::string Equality test" << endl;
      else
	cerr << "Failed std::string Equality test" << endl;			
			
    }

#if defined (TIFF_TEST)
    {
	  
      rmAssert (short_tag_main (0, (char **) NULL) == 0);
      rmAssert (ascii_tags_main(0, (char **) NULL) == 0);
      rmAssert (long_tag_main(0, (char **) NULL) == 0);
      rmAssert (strip_rw_main(0, (char **) NULL) == 0);
      cerr << endl <<"Passed Tiff Libary Tests" << endl;
    }
#endif
	  


#if defined (TEST_GLOB)	  
    {
      {

	rcGlob dir( "/usr/include/*.h" ) ;

	for( int ix = 2 ; ix < argc ; ++ix ){
	  try{
	    dir.push_back( argv[ix] ) ;
	  }catch( std::runtime_error& e ){
	    std::cerr << e.what() << std::endl ;
	  }
	}

		  
	/*
	  passes through the rcGlob twice, to confirm that the iterator operations
	  do not modify the value in the parent container.
	*/

	std::cout << "[Round 1]" << std::endl ;
	std::for_each( dir.begin() , dir.end() , ShowItem( "item" , std::cout ) ) ;

	std::cout << '\n' << "[Round 2]" << std::endl ;
	std::for_each( dir.begin() , dir.end() , ShowItem( "item" , std::cout ) ) ;

	std::cout << '\n' << "[done]" << std::endl ;

      }

    }
#endif	  

    // Stats test
    {
      UT_stats test;
      errors += test.run();
    }
        
    // System info tests
    {
      UT_Systeminfo test;
      errors += test.run();
    }

        
    // rcTimestamp tests
    {
      UT_Timestamp test;
      errors += test.run();
    }
        

    // atomicValue template tests
    {
      UT_AtomicValue test;
      errors += test.run();
    }

    // Thread tests
    {
      UT_Thread test;
      errors += test.run();
    }


    // pair / rect tests
    {
      UT_Rect rtest;
      errors += rtest.run();

    }

  }
  catch ( exception& e ) {
    fprintf(stderr, "error: exception \"%s\" thrown\n", e.what() );
    ++errors;
  }
  catch (...) {
    fprintf(stderr, " error: Unknown exception thrown\n");
    ++errors;
  }

  return errors;
}
