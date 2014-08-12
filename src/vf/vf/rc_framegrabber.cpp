/*
 *  rc_framegrabber.cpp
 *
 *  Created by Sami Kukkonen on Fri Sep 27 11:47:32 EDT 2002
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include "rc_framegrabber.h"

// Static method for mapping an error value to a string
std::string rcFrameGrabber::getErrorString( rcFrameGrabberError error )
{
    switch ( error ) {
        case rcFrameGrabberError::eFrameErrorOK :
            return std::string( "Frame grabber: OK" );
        case rcFrameGrabberError::eFrameErrorUnknown:
            return std::string( "Frame grabber: unknown error" );
        case rcFrameGrabberError::eFrameErrorInit:
            return std::string( "Frame grabber: unknown initialization error" );
        case rcFrameGrabberError::eFrameErrorFileInit:
            return std::string( "Frame grabber: file not found" );
        case rcFrameGrabberError::eFrameErrorFileClose:
            return std::string( "Frame grabber: file close failed" );
        case rcFrameGrabberError::eFrameErrorFileFormat:
            return std::string( "Frame grabber: file is corrupted" );
        case rcFrameGrabberError::eFrameErrorFileUnsupported:
            return std::string( "Frame grabber: unsupported file format" );
        case rcFrameGrabberError::eFrameErrorFileRevUnsupported:
            return std::string( "Frame grabber: unsupported file revision" );
        case rcFrameGrabberError::eFrameErrorFileRead:
            return std::string( "Frame grabber: file read failed" );
        case rcFrameGrabberError::eFrameErrorSystemResources:
            return std::string("Frame grabber: inadequate system resources");
        case rcFrameGrabberError::eFrameErrorIPC:
            return std::string("Frame grabber: error communicating with peer process");
        case rcFrameGrabberError::eFrameErrorCameraInit:
            return std::string( "Frame grabber: camera initialization failed" );
        case rcFrameGrabberError::eFrameErrorCameraCapture:
            return std::string( "Frame grabber: camera capture failed" );
        case rcFrameGrabberError::eFrameErrorQuicktimeInit:
            return std::string( "Frame grabber: QuickTime initialization failed" );
        case rcFrameGrabberError::eFrameErrorQuicktimeRead:
            return std::string( "Frame grabber: QuickTime read failed" );
        case rcFrameGrabberError::eFrameErrorUnsupportedDepth:
            return std::string( "Frame grabber: got an image with unsupported depth" );
        case rcFrameGrabberError::eFrameErrorUnsupportedFormat:
            return std::string( "Frame grabber: got an image with unsupported format" );
        case rcFrameGrabberError::eFrameErrorNotImplemented:
            return std::string( "Frame grabber: feature not implemented yet" );
        case rcFrameGrabberError::eFrameErrorFrameNotAvailable:
            return std::string( "Frame grabber: frame not available" );
        case rcFrameGrabberError::eFrameErrorInternal:
            return std::string( "Frame grabber: internal logic error" );
        case rcFrameGrabberError::eFrameErrorOutOfMemory:
            return std::string( "Frame grabber: out of memory" );
        case rcFrameGrabberError::eFrameErrorInvalidOptions:
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
