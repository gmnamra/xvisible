/*
 *
 *$Id $
 *$Log$
 *Revision 1.5  2003/10/26 01:37:36  arman
 *removed unnecessary include
 *
 *Revision 1.4  2003/03/03 19:31:00  sami
 *MT-safety comment
 *
 *Revision 1.3  2003/02/28 21:19:07  sami
 *Check Altivec only once, cache the result
 *
 *Revision 1.2  2003/01/09 12:41:42  arman
 *Switched altivec detection to use sysctl instead of legacy mac stuff
 *
 *Revision 1.1  2002/12/31 20:43:00  arman
 *Central location for detecting SIMD support for Macs
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_types.h>
#include <sys/sysctl.h>

static bool sForceAltiVec = TRUE;
static int sHasAltivec = -1;

// This function will also be defined under PC when MMX is present
// TODO: this code is not MT-safe, make it safe.

bool rfHasSIMD ( )
{
#if defined (__ppc__)
    if ( sHasAltivec < 0 ) {
        // Detect Altivec only once
        int selectors[2] = { CTL_HW, HW_VECTORUNIT };
        int hasVectorUnit = 0;
        size_t length = sizeof(hasVectorUnit);
        int error = sysctl(selectors, 2, &hasVectorUnit, &length, NULL, 0);
        if( 0 == error) {
            sHasAltivec = hasVectorUnit;
        }
        else
            sHasAltivec = 0;
    }
    
    return (sHasAltivec != 0 && sForceAltiVec);
#elseif
	return false;
#endif	

}


bool rfForceSIMD (bool v)
{
   sForceAltiVec = v;
   return v;
}



