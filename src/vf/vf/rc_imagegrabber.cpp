

#include "rc_imagegrabber.h>
#include "rc_fileutils.h>
#include <cinder/Channel.h>
#include <cinder/ImageIo.h>

using namespace std;
using namespace ci;

// #define DEBUG
// #define DEBUG_LOG

//
// rcImageGrabber implementation
//

rcImageGrabber::rcImageGrabber( const vector<std::string>& fileNames, rcCarbonLock* cLock, double frameInterval, bool nameSort ) :
        rcFileGrabber( cLock ),
        mFileNames( fileNames ),
        mCurrentIndex( 0 ),
        mFrameInterval( frameInterval ), mCurrentTimeStamp( frameInterval )
{
    if ( mFileNames.empty() )
        setLastError( eFrameErrorFileInit );
         
    // Sort names just in case
        if (nameSort)
            rfSortImageFileNames ( fileNames, mFileNames, "jpg");
   
}


rcImageGrabber::~rcImageGrabber()
{

}
    
// Start grabbing
bool rcImageGrabber::start()
{
   if (mFileNames.empty ()) return false;   
    lock ();
    mCurrentIndex = 0;
    unlock ();
    return true;
}

// Stop grabbing
bool rcImageGrabber::stop()
{
    if (mFileNames.empty ()) return false;    
    lock();
    mCurrentIndex = mFileNames.size () + 1;
    unlock();
    
    return true;
}
    
// Returns the number of frames available
int32 rcImageGrabber::frameCount()
{
    if ( mFileNames.size() > 0 )
        return mFileNames.size();
    else
        return -1;
}

// Get next frame, assign the frame to ptr
rcFrameGrabberStatus rcImageGrabber::getNextFrame( rcFrameRef& ptr, bool isBlocking )
{
    lock();
    rcFrameGrabberStatus ret = eFrameStatusError;    
    setLastError( eFrameErrorUnknown );
    ptr = NULL;

    
    if ( isBlocking ) {
        if ( mCurrentIndex < mFileNames.size() )
        {
            //Get the frame and convert it from 32 to 8 bit if possible
            ci::Channel8u ci_image = ci::loadImage (mFileNames[mCurrentIndex]);
            
            if (ci_image == 0)
            {
                ret = eFrameStatusError;
                setLastError( eFrameErrorFileRead );  
            }
            else
            {
                ptr =  new rcFrame (ci_image) ;
                rcWindow image (ptr);
                if ( image.isBound () )
                {
                    if ( mFrameInterval > 0.0 )
                    {
                        //   Force a fixed frame interval
                        image.frameBuf()->setTimestamp( mCurrentTimeStamp );
                        mCurrentTimeStamp += mFrameInterval;
                    }
                    ret = eFrameStatusOK;
                    setLastError( eFrameErrorOK );
                }
            }
        }
    }
    else
    {
        //  Non-blocking operation not implemented yet
        setLastError( eFrameErrorNotImplemented );
        ret = eFrameStatusError;
    }

    unlock();
    
    return ret;
}

// Get name of input source, ie. file name, camera name etc.
const std::string rcImageGrabber::getInputSourceName()
{
    if ( !mFileNames.empty() && mCurrentIndex < mFileNames.size() )
            return mFileNames[mCurrentIndex];

    // We don't even have a file...
    return "empty file";
}







