//  Copyright (c) 2002 Reify Corp. All rights reserved.

#ifndef _RC_ANALYZER_H_
#define _RC_ANALYZER_H_

#include <deque>

#include <rc_framebuf.h>
#include <rc_analysis.h>
#include <rc_similarity.h>
#include <rc_framegrabber.h>

// Internal analyzer state
enum rcAnalyzerState {
  eAnalyzerInitialized = 0,
  eAnalyzerStarted,
  eAnalyzerStopped,
  eAnalyzerCached,
  eAnalyzerError
};

// Entropy calculation mode
enum rcAnalyzerMode {
  // Full cross-correlation is computed between all frames within the temporal window.
  eAnalyzerFullCorrelation = 0,
  // Pairwise cross-correlation is computed for all adjacent frames within the temporal window.
  // Approximation is used for correlation of non-adjacent frames
  eAnalyzerApproximation 
};

// Relative analyzer result origin in a temporal window
enum rcAnalyzerResultOrigin {
  // Computed metric for frames [N, N+1, N+2] will be returned as a value for frame N
  eAnalyzerResultOriginLeft = 0,
  // Computed metric for frames [N, N+1, N+2] will be returned as a value for frame N+1
  eAnalyzerResultOriginCenter,
  // Computed metric for frames [N, N+1, N+2] will be returned as a value for frame N+2
  eAnalyzerResultOriginRight
};

// Analyzer behavior options

class rcAnalyzerOptions {
 public:
  // default ctor
  rcAnalyzerOptions() :
    mCorrParams( rsCorrParams() ),
    mMode( eAnalyzerFullCorrelation ),
    mOrigin( eAnalyzerResultOriginRight ),
    mBoundRect( rcRect() ),
    mWindowSize( 3 ),
    mEntropyDefinition (rcSimilarator::eACI),
    mDepth( rcPixel8 ) {
    };
    
  rcAnalyzerOptions( const rsCorrParams& corrParams,
		     rcAnalyzerMode mode,
		     rcAnalyzerResultOrigin origin,
		     const rcRect& rect,
		     uint32 windowSize,
		     rcSimilarator::rcEntropyDefinition eDef,
		     const rcPixel& depth ) :
    mCorrParams( corrParams ),
    mMode( mode ),
    mOrigin( origin ),
    mBoundRect( rect ),
    mWindowSize( windowSize ),
    mEntropyDefinition( eDef ),
    mDepth( depth ){
      if ( mWindowSize < 1 )
	mWindowSize = 1;
    };

  rcAnalyzerOptions( const rcAnalyzerOptions& o ) :
    mCorrParams( o.corrParams() ),
    mMode( o.mode() ),
    mOrigin( o.origin() ),
    mBoundRect( o.bound() ),
    mWindowSize( o.windowSize() ),
    mEntropyDefinition( o.entropyDefinition() ),
    mDepth( o.depth() ) {
    };
    
  // compiler-generated dtor is OK
  // compiler-generated assignment operator is OK

  // Operators
  bool operator== ( const rcAnalyzerOptions& o ) const {
    // TODO: we must implement rsCorrParams equality operator
    return ( /*mCorrParams == o.corrParams() && */
	    mMode == o.mode() &&
	    mOrigin == o.origin() &&
	    mBoundRect == o.bound() &&
	    mWindowSize == o.windowSize() &&
	    mEntropyDefinition == o.entropyDefinition() &&
	    mDepth == o.depth());
  }
  bool operator!= ( const rcAnalyzerOptions& o ) const {
    return !( o == *this );
  }

  // Accessors
  const rsCorrParams& corrParams() const { return mCorrParams; };
  rcAnalyzerMode mode() const { return mMode; };
  rcAnalyzerResultOrigin origin() const { return mOrigin; };
  const rcRect& bound() const { return mBoundRect; };
  uint32 windowSize() const { return mWindowSize; };
  rcSimilarator::rcEntropyDefinition entropyDefinition() const { return mEntropyDefinition; };
  const rcPixel& depth() const { return mDepth; };
                     
  // Mutators
  void setCorrParams( rsCorrParams p ) { mCorrParams = p; };
  void setMode( rcAnalyzerMode m ) { mMode = m; };
  void setOrigin( rcAnalyzerResultOrigin o ) { mOrigin = o; };
  void setBound( const rcRect& r ) { mBoundRect = r; };
  void setWindowSize( uint32 s ) {
    if ( s < 1 )
      s = 1;
    mWindowSize = s;
  };
  void setEntropyDefinition( rcSimilarator::rcEntropyDefinition e ) {
    mEntropyDefinition = e;
  }
  void setDepth( rcPixel depth ) { mDepth = depth; }

    
 private:
  rsCorrParams           mCorrParams; // Parameters for optokinetic correlation
  rcAnalyzerMode         mMode;       // Computation mode
  rcAnalyzerResultOrigin mOrigin;     // Result origin
  rcRect                 mBoundRect;  // Bounding rect, analyze only the contents inside the rect
  uint32               mWindowSize; // Sliding window size
  rcSimilarator::rcEntropyDefinition mEntropyDefinition; // Definition for entropy
  rcPixel								mDepth;     // Input image depth
};

class rcAnalyzerResult {
 public:

  rcAnalyzerResult() : mEntropy(-1.0), mFrame(0) { };

  // Accessors

  // Visual entropy of this frame
  double entropy() const { return mEntropy; };
  // Frame buffer
  const rcSharedFrameBufPtr& frameBuf() const { return mFrame; };

  // Mutators
    
  void setEntropy( double e ) { mEntropy = e; };
  void setFrameBuf( const rcSharedFrameBufPtr& r ) { mFrame= r; };
    
 private:
  double mEntropy;
  rcSharedFrameBufPtr mFrame;
};

class rcSimilarator;

// Class to correlate image data using a sliding temporal window.

class rcAnalyzer {
 public:
  // ctor/dtor
  rcAnalyzer( const rcAnalyzerOptions& options, // Analyzer options
	      rcFrameGrabber& inputSource, bool longTerm = false);    // Frame input source
  ~rcAnalyzer();

  //
  // Accessors
  //
    
  // Get options
  const rcAnalyzerOptions& getOptions() const;
    
  // Get last error value.
  rcFrameGrabberError getLastError() const;

  // Get at LongTermCache values
  const vector<float>& longTermEntropies () const;

  // Get at offsets from motion compensation if any
  const vector<rc2Fvector>& offsets () const;

  //
  // Mutators
  //
    
  // Get next result. If the return value is eFrameError call getLastError() for details.
  // If argument isBlocking is true, the call waits until a result is available.
  rcFrameGrabberStatus getNextResult( rcAnalyzerResult& result, bool isBlocking );

  // Reset analyzer, discard all cached final and intermediate results.
  // setOptions() will call this automatically.
  rcFrameGrabberStatus reset();

  // Set new options
  rcFrameGrabberStatus setOptions( const rcAnalyzerOptions& options );
  void setOffsets (vector<rc2Fvector>& offs) { mOffsets = offs; }    
 private:
  // Start frame grabber.
  bool start();
  // Stop frame grabber. 
  bool stop();
  // Pop available result
  rcFrameGrabberStatus popResult( rcAnalyzerResult& result );
  // Push result from vector to result stack
  void pushResult( const vector<double>& results );
  // Push undefind result to result stack
  void pushUndefinedResult();
  // Check options validity
  bool isValid( const rcAnalyzerOptions& options ) const;
    
  rcAnalyzerOptions   mOptions;     // Analyzer options
  rcFrameGrabber&     mInputSource; // Input source for frames
  rcFrameGrabberError mLastError;   // Last error from frame grabber
  rcAnalyzerState     mState;       // Internal state
  deque<rcSharedFrameBufPtr>      mFrames;      // Collection of frame buffers
  deque<double>       mResults;      // Collection of cached results
  rcSimilaratorRef     mEnergyCalc;   // Signal calculating object ptr
  bool                 mFilled;       // pipeline filled
  vector<rcWindow>    mImages;       // images for filling
 vector<rc2Fvector> mOffsets;
};

// Stream output operator declarations
ostream& operator << ( ostream& os, const rcAnalyzerOptions& o );

#endif // _RC_ANALYZER_H_
