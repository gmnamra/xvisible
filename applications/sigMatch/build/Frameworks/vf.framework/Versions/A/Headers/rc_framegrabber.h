/*
 *  rc_framegrabber.h
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcFRAMEGRABBER_H_
#define _rcFRAMEGRABBER_H_

#include <rc_types.h>

class rcSharedFrameBufPtr;

// Error type
enum rcFrameGrabberError {
    eFrameErrorOK = 0,            // No error
    eFrameErrorUnknown,           // Unknown error
    eFrameErrorInternal,          // Internal logic error
    eFrameErrorInit,              // Unspecified init error
    eFrameErrorFileInit,          // File (system) init error
    eFrameErrorFileRead,          // File read error
    eFrameErrorFileClose,         // File close error
    eFrameErrorFileFormat,        // File invalid
    eFrameErrorFileUnsupported,   // File format unsupported
    eFrameErrorFileRevUnsupported,// File revision unsupported
    eFrameErrorSystemResources,   // Inadequate system resources (shared memory,
                                  // heap)
    eFrameErrorIPC,               // Error communicating with peer process
    eFrameErrorQuicktimeInit,     // QuickTime init error
    eFrameErrorQuicktimeRead,     // QuickTime read error
    eFrameErrorCameraInit,        // Camera init error
    eFrameErrorCameraCapture,     // Camera error during capture
    eFrameErrorUnsupportedFormat, // Unsupported image format
    eFrameErrorUnsupportedDepth,  // Unsupported image pixel depth
    eFrameErrorNotImplemented,    // This feature not implemented yet
    eFrameErrorFrameNotAvailable, // User specified no waiting and frame isn't
                                  // available
    eFrameErrorOutOfMemory,       // Ran out of memory
    eFrameErrorInvalidOptions     // Invalid options specified
};

// Status type
enum rcFrameGrabberStatus {
    eFrameStatusEOF = 0,    // There will be no more frames ever, stop requesting them
    eFrameStatusOK,         // Valid frame
    eFrameStatusNotStarted, // Grabbing process has not yet started
    eFrameStatusNoFrame,    // No frame returned, one may be available later
    eFrameStatusError       // Fatal error, call getLastError() for details
};

// Abstract interface for getting frames from a device or a file
// Implementations may use one or more threads internally

class RFY_API rcFrameGrabber {
  public:

    // Virtual dtor 
    virtual ~rcFrameGrabber();
    
    // Called after construction to verify that the grabber is functional.
    // If the return value is false, an error occurred and getLastError()
    // can be called to get the error status.
    virtual bool isValid() const = 0;

    // Get last error value.
    virtual rcFrameGrabberError getLastError() const = 0;
    
    // Start grabbing.
    // If the return value is false, an error occurred and getLastError()
    // can be called to get the error status.
    virtual bool start() = 0;

    // Stop grabbing,
    // If the return value is false, an error occurred and getLastError()
    // can be called to get the error status.    
    virtual bool stop() = 0;
    
    // Returns an estimate of the number of frames available. There is no guarantee 
    // that this number is accurate, more or less frames may be returned.
    // Value -1 denotes unknown, potentially an infinite number
    // if the source is a device like a camera
    virtual int32 frameCount() = 0;

    // Returns the size of the cache, in frames, or 0 if there is no
    // cache.
    virtual int32 cacheSize() = 0;

    // Get next frame, assign the frame to ptr. If the return value eFrameError
    // call getLastError() for details.
    virtual rcFrameGrabberStatus getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking ) = 0;

    // Get name of input source, ie. file name, camera name etc.
    virtual const std::string getInputSourceName() = 0;
    
    // Static method for mapping an error value to a string
    static std::string getErrorString( rcFrameGrabberError error );
};

//
// Class for global locking during Carbon/QuickTime calls. Classes
// that implement rcFrameGrabber interface may need to use it.
//

// Carbon and QuickTime calls are not thread-safe so a global lock
// is required if multiple threads are used. Every call to Carbon/QuickTime
// methods must be wrapped in a lock.

class rcCarbonLock {
  public:
	virtual ~rcCarbonLock () {}
    virtual void lock() = 0;
    virtual void unlock() = 0;
};

//
// A convenience class to offer rcFrameGrabber API to a vector of images.
// TODO: Replace this with one that takes generic iterator arguments instead of a vector.
//

#include <vector>
#include <algorithm>
#include <iterator>
#include <rc_window.h>

class rcVectorGrabber : public rcFrameGrabber {
  public:
    // ctor
  rcVectorGrabber( const vector<rcWindow>& images, int32 cacheSz = 0 ) :
        mImages( images ),
        mCurrentIndex( 0 ),
	mLastError( eFrameErrorOK ),
	mCacheSz(cacheSz) {
    };
    // virtual dtor
    virtual ~rcVectorGrabber() { };

    //
    // rcFrameGrabber API
    //
    
    // Returns instance validity
    virtual bool isValid() const { return !mImages.empty(); };

    // Get last error
    virtual rcFrameGrabberError getLastError() const { return mLastError; };

    // Start grabbing
    virtual bool start() { return true; };

    // Stop grabbing
    virtual bool stop() { return true; };
    
    // Returns the number of frames available
    virtual int32 frameCount() { return mImages.size(); };

    virtual int32 cacheSize() { return mCacheSz; }

    // Get next frame, assign the frame to ptr
    virtual rcFrameGrabberStatus getNextFrame( rcSharedFrameBufPtr& ptr, bool isBlocking ) {
        rmUnused( isBlocking );
        
        if ( mCurrentIndex < mImages.size() ) {
            // TODO: we need geometry info to allow cropping
            ptr = mImages[mCurrentIndex++].frameBuf();
            return eFrameStatusOK;
        } else {
            ptr = 0;
            return eFrameStatusEOF;
        }
    }
    // Get name of input source, ie. file name, camera name etc.
    virtual const std::string getInputSourceName() { return "Vector of images"; };
    
  private:
    const vector<rcWindow>& mImages;       // The reference must remain valid for the lifetime of this instance
    uint32                mCurrentIndex; // Current index to mImages
    rcFrameGrabberError     mLastError;
    int32                 mCacheSz;
};

#endif
