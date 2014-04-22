/*
 *  rc_imagegrabber.h
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcIMAGEGRABBER_H_
#define _rcIMAGEGRABBER_H_

#include <vector>


#include <rc_framegrabber.h>
#include <rc_filegrabber.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

//
// Class to grab frames from image files
//

class rcImageGrabber : public rcFileGrabber
{
  public:
    // ctor
    rcImageGrabber( const vector<std::string>& fileNames,
                    rcCarbonLock* lock,
                    double frameInterval = 0.0, // Forced frame interval in seconds (calls setTimestamp())
                    bool nameSort = true)
:
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
    
    // virtual dtor
  virtual ~rcImageGrabber() {}

    //
    // rcFrameGrabber API
    //
    
    // Start grabbing
    virtual bool start()
{
   if (mFileNames.empty ()) return false;   
    lock ();
    mCurrentIndex = 0;
    unlock ();
    return true;
}
      

    // Stop grabbing
    virtual bool stop()
{
    if (mFileNames.empty ()) return false;    
    lock();
    mCurrentIndex = mFileNames.size () + 1;
    unlock();
    
    return true;
}
      
    
    // Returns the number of frames available
    virtual int32 frameCount()
{
    if ( mFileNames.size() > 0 )
        return mFileNames.size();
    else
        return -1;
}
      

    // Image grabbers don't have a cache.
    virtual int32 cacheSize() { return 0; }

    // Get next frame, assign the frame to ptr
    virtual rcFrameGrabberStatus getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking )
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
    virtual const std::string getInputSourceName()
{
    if ( !mFileNames.empty() && mCurrentIndex < mFileNames.size() )
            return mFileNames[mCurrentIndex];

    // We don't even have a file...
    return "empty file";
}
      
    
  private:
    vector<std::string>        mFileNames; 
    uint32                mCurrentIndex; // Current index to mFileHandles
    double                  mFrameInterval;    // Used to force a fixed frame interval
    rcTimestamp             mCurrentTimeStamp; // Current frame timestamp
};

#endif // _rcIMAGEGRABBER_H_

