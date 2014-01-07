/*
 *  AssemblyMacros.h
 *  
 *
 *  Created by Ian Ollmann, Ph. D. on Sun Mar 25 2001.
 *  Copyright (c) 2001. All rights reserved.
 *
 */

#ifndef _rcASSEMBLYMACROS_H_
#define _rcASSEMBLYMACROS_H_

#include <Carbon/Carbon.h>


#ifdef __ppc__

//Zero a cacheline        
inline void __dcbz( void *buffer, int buff_offset ) 			
{ 							
    __asm__ __volatile__ ("dcbz %0,%1" 			
        : 						
        : "b%" (buffer), "r" (buff_offset));		
}

//Zero a cacheline        
inline void __dcbt( void *buffer, int buff_offset ) 			
{ 							
    __asm__ __volatile__ ("dcbt %0,%1" 			
        : 						
        : "b%" (buffer), "r" (buff_offset));		
}

//Store word byte reversed
#define __stwbrx( data, addr, offset )			\
({ 							\
    __asm__ __volatile__ ("stwbrx %0,%1,%2" 		\
        : 						\
        : "r" (data), "b" (addr), "r" (offset)		\
        : "memory" );					\
})

//Read word byte reversed
inline long __lwbrx( void *addr, int offset )		
{ 							
    long result;					
    __asm__ __volatile__ ("lwbrx %0,%1,%2" 		
        : "=r" (result)					
        : "b" (addr), "r" (offset)			
        : "memory" );					
    return result;					
}

//Count leading zeros in the word
inline long __cntlzw( long value )
{
    long result;
    __asm__ __volatile__ ("cntlzw %0, %1"
        : "=r" (result)
        : "r" (value) );
    return result;
}

//Rotate left word immediate and mask insert
//Should be used as result = __rlwimi( result, value, i, i2. i3 ); -- arg1 and return value are the same
//rotate, bounds1 and bounds2 must be fixed integer constants
#define __rlwimi( result, value, rotate, bounds1, bounds2 )	\
result; 							\
({								\
    __asm__ __volatile__ ("rlwimi %0,%1,%2,%3,%4" 		\
        : "+r" (result )					\
        :  "r" (value), "i" (rotate), "i" (bounds1), "i" (bounds2 ) );	\
})

#endif

//On ppc, arguably no such clobber is required, because anyone who
//expects memory ordering between threads has to use a 'sync'
//instruction anyway.  The actual compare_and_swap procedure has two
//'sync' instructions surrounding the main code, which are 'asm
//volatile' and therefore memory barriers.
#ifndef __ppc__
#define __gensync() /* nothing */
#else
#define __gensync() __asm__ __volatile__ ("sync")
#endif

inline void __genSync()
{
	__gensync();
}

#endif // _rcASSEMBLYMACROS_H_
