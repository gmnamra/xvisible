//  Copyright (c) 2002 Reify Corp. All rights reserved.

#include <algorithm> // Defines copy fct

#include <rc_analyzer.h>
#include <rc_similarity.h>

const double cUndefinedEntropy = -1.0;


// Class to correlate image data using a sliding temporal window.

rcAnalyzer::rcAnalyzer( const rcAnalyzerOptions& options, rcFrameGrabber& inputSource, bool longTerm ) :
  mOptions( options ), mInputSource( inputSource ),
  mLastError( eFrameErrorOK ), mState( eAnalyzerInitialized ), mEnergyCalc( new rcSimilarator () ), mFilled (false)
{
  if ( !inputSource.isValid() ) {
    mLastError = inputSource.getLastError();
    mState = eAnalyzerError;
  }
  else if ( !isValid( options ) ) {
    // Window size smaller than 1 is absurd, we cannot allow it
    mState = eAnalyzerError;
    mLastError = eFrameErrorInit;
  }

  if (mOptions.windowSize() > 1) {
    rcSimilarator::rcMatrixGeneration genType = rcSimilarator::eExhaustive;
    if (mOptions.mode() == eAnalyzerApproximation)
      genType = rcSimilarator::eApproximate;

    mEnergyCalc = boost::shared_ptr<rcSimilarator> (new rcSimilarator(genType,mOptions.depth(),mOptions.windowSize(), mInputSource.cacheSize()*90/100));
    mEnergyCalc->longTermCache (longTerm);
    rmAssert(mEnergyCalc);
  }
}

rcAnalyzer::~rcAnalyzer()
{
  if ( mState == eAnalyzerStarted )
    stop();
}

const vector<float>& rcAnalyzer::longTermEntropies() const
{
  return mEnergyCalc->longTermEntropy ();
}

const vector<rc2Fvector>& rcAnalyzer::offsets() const
{
  return mOffsets;
}

// Result accessor
rcFrameGrabberStatus rcAnalyzer::getNextResult( rcAnalyzerResult& result, bool isBlocking )
{
  result.setEntropy( cUndefinedEntropy );
  result.setFrameBuf( 0 ); 
  rcRect clipRect;
  static double zd (0.0);
  deque <double> results (mOptions.windowSize(), zd);
  vector <double> vresults (results.size(), zd);

  // Error checks
  if ( !isValid( mOptions ) )
    return eFrameStatusError;
  if ( mState == eAnalyzerStopped ) 
    return eFrameStatusEOF;
  if ( mState == eAnalyzerError ) 
    return eFrameStatusError;

  if ( mState == eAnalyzerInitialized )
    start();

  if ( mState == eAnalyzerCached ) {
    rmAssert( mFrames.size() == mResults.size() );

    if ( mFrames.empty() ) 
      return eFrameStatusEOF;
    else
      return popResult( result );
  }

  int32 i = 0;
  if ( mState == eAnalyzerStarted ) {
    rcSharedFrameBufPtr framePtr;
    rcFrameGrabberStatus status = eFrameStatusError;
    
    if ( isBlocking ) {
      for( ;; ) {
	status = mInputSource.getNextFrame( framePtr, isBlocking );
                
	if ( status == eFrameStatusOK ) {
	  mLastError = eFrameErrorOK;
	  // Add new frame to queue
	  mFrames.push_back( framePtr );

	  if ( mOptions.bound().empty() ) {
	    // If bounding rect is empty, analyze whole image
	    // Use the size of the first frame
	    clipRect = rcRect( 0, 0, mFrames.front()->width(), mFrames.front()->height() );
	  }
	  else
	    {
	      clipRect = mOptions.bound();

	      if (mOffsets.size())
		{
		  static const int32 dummy (0);
		  clipRect.translate (rcIPair (rfRound (mOffsets[i].x(), dummy), rfRound (mOffsets[i].y(), dummy)));
		}
	    }

	  rcWindow w(framePtr, clipRect );
	  i++;

	  // mImages is only used for initial images to fill the pipe
	  if (!mFilled)
	    mImages.push_back( w );	      

	  if (mImages.size() <  mOptions.windowSize() )
	    {
	      pushUndefinedResult();
	    }
	  
	  else if (!mFilled && mImages.size() == mOptions.windowSize() )
	    {
	      if (mImages.size() > 1)
		{
		  mEnergyCalc->fill (mImages);
		  mFilled = mEnergyCalc->entropies (results, mOptions.entropyDefinition());
		  rmAssert(mFilled);
		  vresults.assign (results.begin(), results.end());
		}

	      // Push the appropriate result(s)
	      pushResult( vresults );
	      // Pop a result
	      popResult( result );
	      break;
	    }
	  else
	    {
	      // Queue is full enough for popping

	      rmAssert(mEnergyCalc);
	      mEnergyCalc->update (w);
	      bool gotSignal = mEnergyCalc->entropies (results, mOptions.entropyDefinition());
	      rmAssert(gotSignal);
	      vresults.assign (results.begin(), results.end());

	      // Push the appropriate result(s)
	      pushResult( vresults );
	      // Pop a result
	      popResult( result );
	      break;
	    }
	} else if ( status == eFrameStatusEOF )
	  {
	    // No more frames are available.
	    // Compute results and cache them. Subsequent calls will
	    // pull results from the cache.
	    if ( !mFrames.empty() )
 	      {
 		mState = eAnalyzerCached;

		// Produce the necessary undefined values
		while ( mResults.size() < mFrames.size())
		  {
		    mResults.push_back( cUndefinedEntropy );
		  }
		status = popResult( result );
	    } 
	    else
	      {
		// Never got any frames
		mLastError = eFrameErrorOK;
	      }
	    break;
	  }
	else {
	  // getNextFrame() failed, bail out
	  mLastError = mInputSource.getLastError();
	  break;
	}
      }
    } else {
      // Non-blocking operation not implemented yet
      mLastError = eFrameErrorNotImplemented;
      status = eFrameStatusError;
    }

    return status;
  } else {
    // Frame grabber never started!
  }
    
  return eFrameStatusError;
}

// Reset analyzer, discard all cached final and intermediate results.
rcFrameGrabberStatus rcAnalyzer::reset()
{
  mLastError = eFrameErrorOK;
  mState = eAnalyzerInitialized;
    
  mResults.clear();
  mFrames.clear();
  mImages.clear ();

  rmAssert( mResults.empty() );
  rmAssert( mFrames.empty() );

  return eFrameStatusOK;
}

// Get options
const rcAnalyzerOptions& rcAnalyzer::getOptions() const
{
  return mOptions;
}

// Set new options
rcFrameGrabberStatus rcAnalyzer::setOptions( const rcAnalyzerOptions& options )
{
  rcFrameGrabberStatus status = eFrameStatusError;
  mLastError = eFrameErrorInvalidOptions;
    
  if ( isValid( options ) ) {
    mOptions = options;
    // Take no chances, flush everything
    status = reset();
  }

  return status;
}


// Get last error value.
rcFrameGrabberError rcAnalyzer::getLastError() const
{
  return mLastError;
}

// private

// Start analyzng.
// If the return value is false, an error occurred and getLastError()
// can be called to get the error status.

bool rcAnalyzer::start()
{
  if ( mInputSource.start() ) {
    mLastError = mInputSource.getLastError();
    mState = eAnalyzerStarted;
    return true;
  }
  else {
    mLastError = mInputSource.getLastError();
    mState = eAnalyzerError;
    return false;
  }
}

// Stop analyzing.
// If the return value is false, an error occurred and getLastError()
// can be called to get the error status.

bool rcAnalyzer::stop()
{
  if ( mInputSource.stop() ) {
    mLastError = mInputSource.getLastError();
    mState = eAnalyzerStopped;
    return true;
  }
  else {
    mLastError = mInputSource.getLastError();
    mState = eAnalyzerError;
    return false;
  }
}

// Pop accumulated result value
rcFrameGrabberStatus rcAnalyzer::popResult( rcAnalyzerResult& result )
{
  rmAssert( !mFrames.empty() );
  rmAssert( !mResults.empty() );

  rcFrameGrabberStatus status = eFrameStatusError;
    
  if ( !mFrames.empty() && !mResults.empty() ) {
    // Get first available result
    result.setEntropy( mResults.front() );
    result.setFrameBuf( mFrames.front() );

    //    Remove front elements
    mFrames.pop_front();
    deque<rcSharedFrameBufPtr>(mFrames).swap(mFrames);

    mResults.pop_front();
    deque<double>(mResults).swap(mResults);
    mLastError = eFrameErrorOK;
    status = eFrameStatusOK;
  }
  else {
    mLastError = eFrameErrorInternal;
  }

  return status;
}

// Push a result value from results vecor to result stack
void rcAnalyzer::pushResult( const vector<double>& results )
{
  if ( results.empty() ) {
    mResults.push_back( cUndefinedEntropy );
  } else {
    // Choose result based on origin
    switch ( mOptions.origin() ) {
    case eAnalyzerResultOriginLeft:
      mResults.push_back( results.front() );
      break;
    case eAnalyzerResultOriginCenter:
      mResults.push_back( results[ mOptions.windowSize()/2] );
      break;
    case eAnalyzerResultOriginRight:
      mResults.push_back( results.back() );
      break;
    default:
      rmAssert( 0 );
      break;
    }
  }
}

// Push an undefined result value to result stack
void rcAnalyzer::pushUndefinedResult()
{
  switch ( mOptions.origin() ) {
  case eAnalyzerResultOriginLeft:
    break;
  case eAnalyzerResultOriginCenter:
    if ( mFrames.size() <=  mOptions.windowSize()/2 )
      mResults.push_back( cUndefinedEntropy );
    break;
  case eAnalyzerResultOriginRight:
    mResults.push_back( cUndefinedEntropy );
    break;
  default:
    rmAssert( 0 );
    break;
  }
}

// Check validity of options
bool rcAnalyzer::isValid( const rcAnalyzerOptions& options ) const
{
  if ( options.windowSize() < 1 )
    return false;

  return true;
}

// Stream output operator implementations
ostream& operator << ( ostream& os, const rcAnalyzerOptions& o )
{
  // Output basic options
  // TODO: display rsCorrParams
  os << "Mode " << o.mode();
  os << " origin " << o.origin();
  os << " rect " << o.bound();
  os << " window " << o.windowSize();
            
  return os;
}
    
