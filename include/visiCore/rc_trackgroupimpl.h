/******************************************************************************
*	rc_trackimpl.h
*
*	This file contains the definition for the implementation of
*	the rcTrack interface.
******************************************************************************/

#ifndef rcTRACKGROUPIMPL_H
#define rcTRACKGROUPIMPL_H

#include <deque>

#if WIN32
using namespace std;
#endif

#include "rc_setting.h"
#include "rc_engine.h"
#include "rc_model.h"

#include "rc_trackimpl.h"

class rcTrackGroupImpl : public rcTrackGroup, public rcWriterGroup
{
public:

	rcTrackGroupImpl( const char* tag , const char* name , const char* description,
                      rcGroupSemantics type )

	{
		_tag = tag;
		_name = name;
		_description = description;
        _type = type;
	}
	// virtual dtor is required
	virtual ~rcTrackGroupImpl();

	//////////////// rcTrackGroup implementation ////////////////////

	// get the meta-data tag for this track
	virtual const char* getTag( void )  const
	{
		return _tag.c_str();
	}

	// get the display name for the track
	virtual const char* getName( void ) const
	{
		return _name.c_str();
	}

	// get the description for the track
	virtual const char* getDescription( void ) const
	{
		return _description.c_str();
	}

    // get semantic type of this group
    virtual rcGroupSemantics getType()
    {
        return _type;
    }
    
	// get the number of data tracks in this group.
	virtual int getNTracks( void ) const
    {
        return _tracks.size();
    }

	// get a particular group data track
	virtual rcTrack* getTrack( int trackNo )
    {
        rmAssertDebug( uint32(trackNo) < _tracks.size() );
        
        return _tracks[ trackNo ];
    }

    // create a new writer
    virtual rcWriter* createWriter( rcWriterType type , const char* tag ,
                                    const char* name , const char* description,
                                    const char* formatString,
                                    uint32 sizeLimit,
                                    const rcRect& analysisRect );

    // release this writer group and all writers within the group.
    virtual void release();
    
private:
	std::string	    	 _tag;
	std::string	    	 _name;
	std::string	    	 _description;
    deque<rcTrackImpl*>  _tracks;
    rcGroupSemantics     _type;
};


#endif // rcTRACKGROUPIMPL_H
