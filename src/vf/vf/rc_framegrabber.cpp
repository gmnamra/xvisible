/*
 *  rc_framegrabber.cpp
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include <rc_framegrabber.h>

// Static method for mapping an error value to a string
std::string rcFrameGrabber::getErrorString( rcFrameGrabberError error )
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

// Virtual dtor
rcFrameGrabber::~rcFrameGrabber()
{
}
