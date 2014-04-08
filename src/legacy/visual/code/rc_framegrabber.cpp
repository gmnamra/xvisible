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
        case eFrameErrorOK:
            return std::string( "Frame grabber: OK" );
        case eFrameErrorUnknown:
            return std::string( "Frame grabber: unknown error" );
        case eFrameErrorInit:
            return std::string( "Frame grabber: unknown initialization error" );
        case eFrameErrorFileInit:
            return std::string( "Frame grabber: file not found" );
        case eFrameErrorFileClose:
            return std::string( "Frame grabber: file close failed" );
        case eFrameErrorFileFormat:
            return std::string( "Frame grabber: file is corrupted" );
        case eFrameErrorFileUnsupported:
            return std::string( "Frame grabber: unsupported file format" );
        case eFrameErrorFileRevUnsupported:
            return std::string( "Frame grabber: unsupported file revision" );
        case eFrameErrorFileRead:
            return std::string( "Frame grabber: file read failed" );
        case eFrameErrorSystemResources:
            return std::string("Frame grabber: inadequate system resources");
        case eFrameErrorIPC:
            return std::string("Frame grabber: error communicating with peer process");
        case eFrameErrorCameraInit:
            return std::string( "Frame grabber: camera initialization failed" );
        case eFrameErrorCameraCapture:
            return std::string( "Frame grabber: camera capture failed" );
        case eFrameErrorQuicktimeInit:
            return std::string( "Frame grabber: QuickTime initialization failed" );
        case eFrameErrorQuicktimeRead:
            return std::string( "Frame grabber: QuickTime read failed" );
        case eFrameErrorUnsupportedDepth:
            return std::string( "Frame grabber: got an image with unsupported depth" );
        case eFrameErrorUnsupportedFormat:
            return std::string( "Frame grabber: got an image with unsupported format" );
        case eFrameErrorNotImplemented:
            return std::string( "Frame grabber: feature not implemented yet" );
        case eFrameErrorFrameNotAvailable:
            return std::string( "Frame grabber: frame not available" );
        case eFrameErrorInternal:
            return std::string( "Frame grabber: internal logic error" );
        case eFrameErrorOutOfMemory:
            return std::string( "Frame grabber: out of memory" );
        case eFrameErrorInvalidOptions:
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
