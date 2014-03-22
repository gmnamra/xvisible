/*
 *  rc_imagegrabber.cpp
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

//#include <rc_qtime.h>
#include <rc_imagegrabber.h>
#include <rc_ipconvert.h>

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
    lock();
    
    rmAssert( mFileHandles.empty() );
        
    // Sort names just in case
    //    if (nameSort)
    //      qtime::rfImageNameSort( mFileNames );

#ifdef DEBUG
    // Paranoia, verify sort order
    for (vector<std::string>::iterator name = mFileNames.begin(); name < mFileNames.end()-1; name++ ) {
        char* str1 = (char*)name->c_str();
        char* str2 = (char*)(name+1)->c_str();
        int num1 = rfImageFrameNum( str1 );
        int num2 = rfImageFrameNum( str2 );
        rmAssert( num1 <= num2 );
    }
#endif
  
    if ( mFileHandles.empty() )
        setLastError( eFrameErrorFileInit );

    unlock();
}


rcImageGrabber::~rcImageGrabber()
{
    // Delete specs
    for (vector<FSSpec*>::iterator f = mFileHandles.begin(); f != mFileHandles.end(); f++)
        delete *f;
}
    
// Start grabbing
bool rcImageGrabber::start()
{
    if ( getLastError() == eFrameErrorOK )
        return true;
    else
        return false;
}

// Stop grabbing
bool rcImageGrabber::stop()
{
    lock();
    

    
    unlock();
    
    return true;
}
    
// Returns the number of frames available
int32 rcImageGrabber::frameCount()
{
    if ( mFileHandles.size() > 0 )
        return mFileHandles.size();
    else
        return -1;
}

// Get next frame, assign the frame to ptr
rcFrameGrabberStatus rcImageGrabber::getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking )
{
    lock();
    rcFrameGrabberStatus ret = eFrameStatusError;    
//    setLastError( eFrameErrorUnknown );
//    ptr = NULL;

//    
//    if ( isBlocking ) {
//        if ( mCurrentIndex < mFileHandles.size() ) {
//            rcWindow image;
//            FSSpec *spec = mFileHandles[mCurrentIndex];
//            ++mCurrentIndex;
//
//            // Get the frame and convert it from 32 to 8 bit if possible
//            //  OSErr status = rfImageFileToRcWindow( spec, image, mImporter );
//
//            if ( status == noErr ) {
//                ptr = image.frameBuf();
//                if ( mFrameInterval > 0.0 ) {
//                    // Force a fixed frame interval
//                    image.frameBuf()->setTimestamp( mCurrentTimeStamp );
//                    mCurrentTimeStamp += mFrameInterval;
//                }
//                ret = eFrameStatusOK;
//                setLastError( eFrameErrorOK );
//            } else {
//                ret = eFrameStatusError;
//                // TODO: analyze OSErr and map it to proper eFrameError
//                setLastError( eFrameErrorFileRead );
//            }
//        } else {
//            ret = eFrameStatusEOF;
//            setLastError( eFrameErrorOK );
//        }
//    } else {
//         // Non-blocking operation not implemented yet
//        setLastError( eFrameErrorNotImplemented );
//        ret = eFrameStatusError;
//    }

    unlock();
    
    return ret;
}

// Get name of input source, ie. file name, camera name etc.
const std::string rcImageGrabber::getInputSourceName()
{
    if ( !mFileNames.empty() ) {
        if ( mCurrentIndex < mFileNames.size() )
            return mFileNames[mCurrentIndex];
        else
            return mFileNames[0];
    }
    // We don't even have a file...
    return "empty file";
}
