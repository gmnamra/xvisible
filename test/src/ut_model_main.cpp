/******************************************************************************
 *   Copyright (c) 2003 Reify Corp. All Rights reserved.
 *
 *	$Id: ut_model_main.cpp 6266 2009-01-15 19:33:27Z arman $
 *
 *	This file contains unit test main function.
 *
 ******************************************************************************/

#include "ut_model.h"

int main( int argc, char ** argv )
{

    rcUNUSED( argc );
    rcUNUSED( argv );
    
    uint32 errors = 0;

    try {
        {
            UT_Model test;
            errors += test.run();
        }
    }
    catch ( exception& e ) {
		fprintf(stderr, "%s error: exception \"%s\" thrown\n", argv[0] ? argv[0] : "", e.what() );
        ++errors;
	}
	catch (...) {
		fprintf(stderr, "%s error: Unknown exception thrown\n", argv[0] ? argv[0] : "");
        ++errors;
	}

    return errors;
}
