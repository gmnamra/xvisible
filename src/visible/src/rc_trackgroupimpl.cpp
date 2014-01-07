/******************************************************************************
*	rcTrackGroupImpl.cpp
*
*	This file contains the definition for the implementation of
*	the rcTrackGroup and rcWriterGroup interfaces.
******************************************************************************/

#include "rc_scalartrackimpl.h"
#include "rc_videotrackimpl.h"
#include "rc_graphicstrackimpl.h"
#include "rc_positiontrackimpl.h"

#include "rc_trackgroupimpl.h"

// destructor
rcTrackGroupImpl::~rcTrackGroupImpl()
{
    release();
}

// create a new writer
rcWriter* rcTrackGroupImpl::createWriter( rcWriterType type , const char* tag ,
										  const char* name , const char* description,
										  const char* formatString,
										  uint32 sizeLimit, const rcRect& analysisRect )
{
	rcTrackImpl* track = 0;

 	switch (type)
	{
        case eVideoTrack:		// track contains video frames
            track = new rcVideoTrackImpl( tag, name, description, formatString, sizeLimit );
            break;

        case eScalarTrack:		// track contains scalar data
            track = new rcScalarTrackImpl( tag , name , description, formatString, sizeLimit );
            break;

        case eGraphicsTrack:		// track contains graphigs visualization data
            track = new rcGraphicsTrackImpl( tag , name , description, formatString, sizeLimit );
            break;

        case ePositionTrack:	// track contains <x,y> position data
            track = new rcPositionTrackImpl( tag , name , description, formatString, sizeLimit );
            break;
            
        default:
	  rmExceptionMacro (<< "unsupported track type" );
	}

    track->setAnalysisRect( analysisRect );
	_tracks.push_back( track );
	return track;
}

// release this writer group and all writers within the group.
void rcTrackGroupImpl::release()
{
    for ( uint32 i = 0; i < _tracks.size(); i++ ) {
        delete _tracks[i];
        _tracks[i] = 0;
    }
}
