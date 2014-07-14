/*
 * $Id: rc_correlationwindow.h 7291 2011-03-05 01:57:35Z arman $
 *
 * Wrapper cache class for rcWindow
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 */

#ifndef _RC_CORRELATIONWINDOW_H_
#define _RC_CORRELATIONWINDOW_H_

#include "rc_window.h"

// This class can cache pixel sum and pixel sum squared values for reuse

template <class T>
class rcCorrelationWindow
{
  public:
    //
    // ctors
    //
    rcCorrelationWindow() :
        mWindow(),
        mSum( 0.0 ),
        mSumSquares( 0.0 ),
        mSumValid( false ) {}

    rcCorrelationWindow( const rcWindow& w ) :
        mWindow( w ),
        mSum( 0.0 ),
        mSumSquares( 0.0 ),
        mSumValid( false ) {
            // Verify that depths match
            if ( sizeof(T) != static_cast<int>(w.bytes()) )
                throw general_exception( "rcCorrelationWindow ctor: invalid rcWindow depth" );
        }
    // default copy ctor and assignment OK

    //
    // Accessors
    //
    inline const rcWindow& window() const { return mWindow; };
    inline bool sumValid() const          { return mSumValid; }
    inline double sum() const             { rmAssertDebug( mSumValid ); return mSum; }
    inline double sumSquares() const      { rmAssertDebug( mSumValid ); return mSumSquares; }
    // Window geometry
    inline int32 x() const      { return mWindow.x(); }
    inline int32 y() const      { return mWindow.y(); }
    inline int32 width() const  { return mWindow.width(); }
    inline int32 height() const { return mWindow.height(); }
    // Pixel data
    inline rcPixel depth() const                  { return mWindow.depth(); }
    inline int bytes() const                  { return mWindow.bytes(); }    
    inline const uint8* rowPointer (uint32 y) const { return mWindow.rowPointer( y ); }
    inline int32 rowUpdate () const                  { return mWindow.rowUpdate(); }
    
    // Row update in pixels (of size T)
    inline int32 rowPelUpdate () const                     { return mWindow.rowUpdate()/sizeof(T); }
    // Warning: these are dangerous casts
    inline const T* rowPelPointer (uint32 y) const          { return (const T*)(mWindow.rowPointer( y )); }
    inline const T* pelPointer (uint32 x, uint32 y) const { return (const T*)(mWindow.pelPointer( x, y )); };
    
    inline const rcFrameRef& frameBuf() const { return mWindow.frameBuf(); }
    inline rcFrameRef& frameBuf()             { return mWindow.frameBuf(); }
    
    //
    // Mutators
    //
    inline void sum( double s )              { mSum = s; mSumValid = true; };
    inline void sumSquares( double s )       { mSumSquares = s; rmAssertDebug( mSumValid ); };
    inline void invalidate()                 { mSumValid = false; };
    // Window geometry
    inline bool translate (const rcIPair& o) {
        bool mutated = mWindow.translate( o );
        if ( mutated )
            invalidate();
        return mutated; }
        
  private:
    rcWindow mWindow;             // Frame window
    double   mSum;                // Sum of pixel values
    double   mSumSquares;         // Sum of pixel value squares
    bool     mSumValid;           // Is sum valid
};

#endif //  _RC_CORRELATIONWINDOW_H_
