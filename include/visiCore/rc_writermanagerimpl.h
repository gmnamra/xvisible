/******************************************************************************
*   @files Copyright (c) 2003 Reify Corp. All Rights reserved.
*
*	$Id: rc_writermanagerimpl.h 4391 2006-05-02 18:40:03Z armanmg $
*
*	This file contains writer name-type mapping classes
*
******************************************************************************/

#ifndef _rcWRITER_MANAGERIMPL_H_
#define _rcWRITER_MANAGERIMPL_H_

// Writer info class
class rcWriterInfo {
  public:
    // ctor
    rcWriterInfo( rcWriterSemantics type,
                  rcWriterType trackType,
                  const std::string& tagName,
                  const std::string& displayName,
                  const std::string& description,
                  const std::string& formatString ) :
        mType( type ), mTrackType( trackType ), mTagName( tagName ), mDisplayName( displayName ),
        mDescription( description ),mFormatString( formatString ) { }
        
    // Accessors
    rcWriterSemantics type() const      { return mType; }
    rcWriterType trackType() const      { return mTrackType; }
    const std::string& tag() const         { return mTagName; }
    const std::string& name() const        { return mDisplayName; }
    const std::string& description() const { return mDescription; }
    const std::string& format() const      { return mFormatString; }
    
  private:
    rcWriterSemantics mType;         // Semantic type
    rcWriterType      mTrackType;    // Track data type
    std::string          mTagName;      // XML tag name
    std::string          mDisplayName;  // Display name shown in widget
    std::string          mDescription;  // Desacription shown by tooltip
    std::string          mFormatString; // Printf format string for widget display
};

// Name-to-type mapper
typedef map<std::string,rcWriterSemantics>       rcWriterTypeMap;
// Tag-to-type mapper
typedef map<std::string,rcWriterSemantics>       rcWriterTagMap;
// Type-to-name mapper
typedef map<rcWriterSemantics, rcWriterInfo>  rcWriterNameMap;

// Writer mapper class (name, type, tag)

class rcWriterMapper {
public:
    // ctor
    rcWriterMapper( const rcWriterInfo* writerArray, uint32 size )
    {
        // Populate all maps
        for ( uint32 i = 0; i < size; ++i ) {
            rmAssert( writerArray[i].type() != eWriterUnknown );

            // Fill name map            
            mNameMap.insert( make_pair( writerArray[i].type(), writerArray[i] ) );
            // Immediate self-test, it can catch duplicates
            rmAssert( info( writerArray[i].type() ).type() == writerArray[i].type() );

            // Fill type map
            mTypeMap.insert( make_pair( writerArray[i].name(), writerArray[i].type() ) );
            // Immediate self-test, it can catch duplicates
            rmAssert( type( writerArray[i].name() ) == writerArray[i].type() );

            // Fill tag map
            mTagMap.insert( make_pair( writerArray[i].tag(), writerArray[i].type() ) );
            // Immediate self-test, it can catch duplicates
            rmAssert( typeTag( writerArray[i].tag() ) == writerArray[i].type() );
        }
        // Check for duplicate types in the list
        rmAssert( mNameMap.size() == size );
        // Check for duplicate display names in the list
        rmAssert( mTypeMap.size() == size );
        // Check for duplicate tag names in the list
        rmAssert( mTagMap.size() == size );
    }

    // Accessors
    
    // Return writer info
    const rcWriterInfo info( rcWriterSemantics type ) const {
        rcWriterNameMap::const_iterator pos = mNameMap.find( type );
        if ( pos != mNameMap.end() )
            return pos->second;
        else
            return rcWriterInfo( eWriterUnknown, eScalarWriter, "Unknown", "Unknown", "Unknown", "" );
    }

    // Return writer XML tag
    rcWriterSemantics typeTag( const std::string& name ) const {
        rcWriterTagMap::const_iterator pos = mTagMap.find( name );
        if ( pos != mTagMap.end() )
            return pos->second;
        else
            return eWriterUnknown;
    }

    // Return writer type
    const rcWriterSemantics type( const std::string& name ) const {
        rcWriterTypeMap::const_iterator pos = mTypeMap.find( name );
        if ( pos != mTypeMap.end() )
            return pos->second;
        else
            return eWriterUnknown;
    }
    
private:
    rcWriterNameMap mNameMap;
    rcWriterTypeMap mTypeMap;
    rcWriterTagMap  mTagMap;
};

#endif // _rcWRITER_MANAGERIMPL_H_
