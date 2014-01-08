/*
 *  timing.h
 *  framebuf
 *
 *  Created by Eric Shalkey on Tue May 21 2002.
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcTIMING_H_
#define _rcTIMING_H_

#include <iostream>

#include <sys/time.h>
#include <rc_types.h>

// Note: constructing one starts it as well. 

class rcTime
{
public:
  rcTime ()
  { start (); }

   // Compiler generated dtor, assignment and copy are ok
   
   void start ()
   {
      
      UInt64 startTime, elapsedTime;
      double t0;

      //Calibrate the clock
      t0 = mGetTime ();
      startTime = mReadTBR();
      //spin for a second while we wait for a few ticks of the clock to go by
      while( mGetTime() - t0 < 1.0 )
         {}
      elapsedTime = mReadTBR() - startTime;
      mConversionFactor = 1.0/ (elapsedTime );

      mStartTime = mReadTBR();
   }

   void reset () { start (); }

   void end ()
   {
      mElapsedTime = mReadTBR() - mStartTime;
   }

   double seconds ()
   {
      return mElapsedTime * mConversionFactor;
   }

   double sec ()
   {
     return seconds ();
   }

   double milliseconds ()
   {
      return mElapsedTime * mConversionFactor * 1000;
   }
   double msec ()
   {
     return milliseconds ();
   }


   double microseconds ()
   {
      return mElapsedTime * mConversionFactor * 1000000;
   }
   double usec ()
   {
     return microseconds ();
   }


   rcUInt64 getUnixTime ()
   {
      timeval tv;
      gettimeofday(&tv,NULL);
      return (rcUInt64)((tv.tv_sec)*1000000 + tv.tv_usec);
   }

   void printUnixTime (rcUInt64 time)
   {
      cout << time << " microsecs\n";
   }

   
private:
   
   double mGetTime ( void )
   {
      UInt64 time;
      Microseconds( (UnsignedWide*) &time );
      return (double) time * 1e-6;
   }

   //Read the contents of the TBR and return as a UInt64
   UInt64 mReadTBR( void )
      {
#ifdef __ppc__	  
	register unsigned int temp1 asm ("r4");
	register unsigned int temp2 asm ("r5");
	__asm__ volatile
	  ("0: mftbu r3\n"
	   "mftb r4\n"
	   "mftbu r5\n"
	   "cmpw r3, r5\n"
	   "bne- 0b\n");
	return ((UInt64)(temp2) << 32) + temp1;
#else
	return getUnixTime ();
#endif	
      }
   
   rcUInt64 mStartTime, mElapsedTime;
   double mConversionFactor;
};




#endif   // _rcTIMING_H_
