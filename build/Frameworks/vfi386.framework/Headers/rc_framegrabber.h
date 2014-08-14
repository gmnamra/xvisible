/*
 *  rc_framegrabber.h
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _rcFRAMEGRABBER_H_
#define _rcFRAMEGRABBER_H_


#include <vector>
#include <algorithm>
#include <iterator>
#include "rc_types.h"
#include "rc_window.h"

class rcFrameRef;

// Error type
enum class rcFrameGrabberError : int32 {
    OK = 0,            // No error
    Unknown,           // Unknown error
    Internal,          // Internal logic error
    Init,              // Unspecified init error
    FileInit,          // File (system) init error
    FileRead,          // File read error
    FileClose,         // File close error
    FileFormat,        // File invalid
    FileUnsupported,   // File format unsupported
    FileRevUnsupported,// File revision unsupported
    SystemResources,   // Inadequate system resources (shared memory,
                                  // heap)
    IPC,               // Error communicating with peer process
    QuicktimeInit,     // QuickTime init error
    QuicktimeRead,     // QuickTime read error
    CameraInit,        // Camera init error
    CameraCapture,     // Camera error during capture
    UnsupportedFormat, // Unsupported image format
    UnsupportedDepth,  // Unsupported image pixel depth
    NotImplemented,    // This feature not implemented yet
    FrameNotAvailable, // User specified no waiting and frame isn't
                                  // available
    OutOfMemory,       // Ran out of memory
    InvalidOptions     // Invalid options specified
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
#if 0
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

    // Get next frame, assign the frame to ptr. If the return value 
    // call getLastError() for details.
    virtual rcFrameGrabberStatus getNextFrame( rcFrameRef& , bool isBlocking ) = 0;

    // Get name of input source, ie. file name, camera name etc.
    virtual const std::string getInputSourceName() = 0;
    
    // Static method for mapping an error value to a string
    static std::string getErrorString( rcFrameGrabberError error );
};
#endif


template<typename frame_type_t>
class RFY_API rcFrameGrabberT
{
public:
    
    // Virtual dtor
    virtual ~rcFrameGrabberT() {}
    
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
    
    // Get next frame, assign the frame to ptr. If the return value
    // call getLastError() for details.
    virtual rcFrameGrabberStatus getNextFrame( frame_type_t& , bool isBlocking ) = 0;
    
    // Get name of input source, ie. file name, camera name etc.
    virtual const std::string getInputSourceName() = 0;
    
    // Static method for mapping an error value to a string
    static std::string getErrorString( rcFrameGrabberError error )
    {
            switch ( error ) {
                case rcFrameGrabberError::OK:
                    return std::string( "Frame grabber: OK" );
                case rcFrameGrabberError::Unknown:
                    return std::string( "Frame grabber: unknown error" );
                case rcFrameGrabberError::Init:
                    return std::string( "Frame grabber: unknown initialization error" );
                case rcFrameGrabberError::FileInit:
                    return std::string( "Frame grabber: file not found" );
                case rcFrameGrabberError::FileClose:
                    return std::string( "Frame grabber: file close failed" );
                case rcFrameGrabberError::FileFormat:
                    return std::string( "Frame grabber: file is corrupted" );
                case rcFrameGrabberError::FileUnsupported:
                    return std::string( "Frame grabber: unsupported file format" );
                case rcFrameGrabberError::FileRevUnsupported:
                    return std::string( "Frame grabber: unsupported file revision" );
                case rcFrameGrabberError::FileRead:
                    return std::string( "Frame grabber: file read failed" );
                case rcFrameGrabberError::SystemResources:
                    return std::string("Frame grabber: inadequate system resources");
                case rcFrameGrabberError::IPC:
                    return std::string("Frame grabber: error communicating with peer process");
                case rcFrameGrabberError::CameraInit:
                    return std::string( "Frame grabber: camera initialization failed" );
                case rcFrameGrabberError::CameraCapture:
                    return std::string( "Frame grabber: camera capture failed" );
                case rcFrameGrabberError::QuicktimeInit:
                    return std::string( "Frame grabber: QuickTime initialization failed" );
                case rcFrameGrabberError::QuicktimeRead:
                    return std::string( "Frame grabber: QuickTime read failed" );
                case rcFrameGrabberError::UnsupportedDepth:
                    return std::string( "Frame grabber: got an image with unsupported depth" );
                case rcFrameGrabberError::UnsupportedFormat:
                    return std::string( "Frame grabber: got an image with unsupported format" );
                case rcFrameGrabberError::NotImplemented:
                    return std::string( "Frame grabber: feature not implemented yet" );
                case rcFrameGrabberError::FrameNotAvailable:
                    return std::string( "Frame grabber: frame not available" );
                case rcFrameGrabberError::Internal:
                    return std::string( "Frame grabber: internal logic error" );
                case rcFrameGrabberError::OutOfMemory:
                    return std::string( "Frame grabber: out of memory" );
                case rcFrameGrabberError::InvalidOptions:
                    return std::string( "Frame grabber: invalid options specified" );
                    // Note: no default case to force a compiler warning if a new enum value
                    // is defined without adding a corresponding string here.
            }
            
            return std::string(" Frame grabber: undefined error" );
    }
};

typedef rcFrameGrabberT<rcFrameRef> rcFrameGrabber;

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

class rcVectorGrabber : public rcFrameGrabber {
  public:
    // ctor
  rcVectorGrabber( const vector<rcWindow>& images, int32 cacheSz = 0 ) :
        mImages( images ),
        mCurrentIndex( 0 ),
	mLastError( rcFrameGrabberError::OK ),
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
    virtual rcFrameGrabberStatus getNextFrame( rcFrameRef& fptr, bool isBlocking ) {
        rmUnused( isBlocking );
        
        if ( mCurrentIndex < mImages.size() ) {
            // TODO: we need geometry info to allow cropping
            fptr = mImages[mCurrentIndex++].frameBuf();
            return eFrameStatusOK;
        } else {
            fptr = 0;
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
