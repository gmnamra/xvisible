/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 *	$Id: ut_systeminfo.cpp 6124 2008-09-26 20:46:18Z arman $
 *
 ******************************************************************************/

#include <strstream>
#include <unistd.h>

#include <rc_systeminfo.h>

#include "ut_systeminfo.h"

UT_Systeminfo::UT_Systeminfo()
{
}

UT_Systeminfo::~UT_Systeminfo()
{
    printSuccessMessage( "Systeminfo test", mErrors );
}

uint32 UT_Systeminfo::run()
{
    // RAM test
	cerr << rfGetHostInfo () << endl;

    rcUNITTEST_ASSERT( rfSystemRam () > 0 );

    // Byte order tests
    rcUNITTEST_ASSERT( rfPlatformByteOrder() != eByteOrderUnknown );
		//  rcUNITTEST_ASSERT( rfPlatformByteOrder() == rfPlatformByteOrder(rfPlatformByteOrderMark()) );
    rcUNITTEST_ASSERT( rcBOM_BE != rcBOM_LE );
    
    rcUNITTEST_ASSERT( eByteOrderBigEndian == rfPlatformByteOrder(rcBOM_BE) );
    rcUNITTEST_ASSERT( eByteOrderLittleEndian == rfPlatformByteOrder(rcBOM_LE) );
		//  rcUNITTEST_ASSERT( eByteOrderUnknown == rfPlatformByteOrder(0) );
		//    rcUNITTEST_ASSERT( eByteOrderUnknown == rfPlatformByteOrder(666) );

    if ( rfPlatformByteOrder() == eByteOrderBigEndian ) {
        rcUNITTEST_ASSERT( rfPlatformByteOrderMark() == rcBOM_BE );
    } else if ( rfPlatformByteOrder() == eByteOrderLittleEndian ) {
        rcUNITTEST_ASSERT( rfPlatformByteOrderMark() == rcBOM_LE );
    }

#if defined(__PPC__)
    rcUNITTEST_ASSERT( eByteOrderBigEndian == rfPlatformByteOrder() );
    rcUNITTEST_ASSERT( rcBOM_BE == rfPlatformByteOrderMark() );
#endif

    
    return mErrors;  
}
