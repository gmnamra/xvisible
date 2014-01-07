/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	$Id: rc_nativeimporter.h 4391 2006-05-02 18:40:03Z armanmg $
*
*	This file contains the declaration for the implementation of
*	the import handler for native file formats.
******************************************************************************/

#ifndef _rcNATIVEIMPORTER_H_
#define _rcNATIVEIMPORTER_H_

#include <rc_types.h>
#include <rc_engine.h>

#include "rc_xml.h"

class rcExperimentImpl;
class rcEngineObserver;

//
// ID mapper for groups and tracks
//

class rcTrackElement {
  public:
    rcTrackElement() :
        mTrack( 0 ),
        mAnalysisRect(),
        mVersion( 0 ) { };
    rcTrackElement( rcWriter* track, const rcRect& rect ) :
          mTrack( track ),
          mAnalysisRect( rect ),
          mVersion( 0 ) { };

          rcTrackElement( rcWriter* track, const rcRect& rect, uint32 version ) :
          mTrack( track ),
          mAnalysisRect( rect ),
          mVersion( version ) { };
      // Accessors
      bool isValid() const { return mTrack != 0; };
      rcWriter* track() const { return mTrack; };
      const rcRect& rect() const { return mAnalysisRect; };
      const uint32 version() const { return mVersion; };
      
  private:
    rcWriter* mTrack;
    rcRect    mAnalysisRect;
    uint32  mVersion;
};

typedef map<std::string,rcWriterGroup*> rcGroupIdMap;
typedef map<std::string,rcTrackElement> rcTrackIdMap;

class rcIdMapper {
  public:
    
    // Add a new mapping for a group, return number of errors
    uint32 addGroup( const std::string& idString, rcWriterGroup* inst ) {
        uint32 errors = 0;
        
        mGroupMap.insert( make_pair( idString, inst ) );
        // Immediate self-test, it can catch duplicates
        if ( group( idString ) != inst )
            ++errors;

        return errors;
    };
        
     // Add a new mapping for a track, return number of errors
    uint32 addTrack( const std::string& idString, rcTrackElement inst ) {
        uint32 errors = 0;
        
        mTrackMap.insert( make_pair( idString, inst ) );
        // Immediate self-test, it can catch duplicates
        rcTrackElement e = track( idString );
        if ( e.track() != inst.track() ) 
            ++errors;
        if ( e.rect() != inst.rect() ) 
            ++errors;

        return errors;
    };
        
    // Map group id string to group instance
    rcWriterGroup* group( const std::string& idString ) const {
        rcGroupIdMap::const_iterator pos;

        pos = mGroupMap.find( idString );
        if ( pos != mGroupMap.end() )
            return pos->second;
        else
            return 0;
    };

    // Map track id string to writer instance
    rcTrackElement track( const std::string& idString ) const {
        rcTrackIdMap::const_iterator pos;

        pos = mTrackMap.find( idString );
        if ( pos != mTrackMap.end() )
            return pos->second;
        else
            return rcTrackElement();
    };

    // Flush all scalar writers
    void flushWriters()  {
        rcTrackIdMap::iterator pos;

        for ( pos = mTrackMap.begin(); pos !=  mTrackMap.end(); ++pos )
        {
            rcTrackElement e = pos->second;
            if ( e.isValid() ) {
                e.track()->flush();
            }
        }
    };
    
  private:
    rcGroupIdMap mGroupMap;
    rcTrackIdMap mTrackMap;
};

//
// Native file format importer class
//

class rcNativeImporter
{
public:
    // Create a native importer to export data to the designated file.
	rcNativeImporter( rcExperimentImpl* experiment, rcEngineObserver* observer, rcProgressIndicator* progress );
    // Virtual dtor is required
    ~rcNativeImporter();

    // Import experiment XML tree 
    int importExperiment( const rcXMLElementTree& tree,
                          const vector<std::string>& importableSettings );
    
  private:
    // XML tree parsing methods

    // Parse setting category and its children
    uint32 parseSettingCategory( const rcXMLElementTree& tree,
                                   rcXMLElementTree::iterator& sibBegin,
                                   const vector<std::string>& importableSettings );
    
    // Parse setting element and its children
    uint32 parseSetting( const rcXMLElementTree& tree,
                           const rcXMLElementTree::sibling_iterator& sibBegin,
                           int categoryIndex );

    // Parse experiment data and its children
    uint32 parseExperimentData( const rcXMLElementTree& tree,
                                  rcXMLElementTree::iterator& sibBegin );
    
    // Parse group and its children
    uint32 parseGroup( const rcXMLElementTree& tree,
                         rcXMLElementTree::sibling_iterator& sibBegin );

    // Parse time values
    uint32 parseTime( rcXMLElementTree::sibling_iterator& sibBegin );
    
    // Parse track and its children
    uint32 parseTrack( const rcXMLElementTree& tree,
                         rcXMLElementTree::sibling_iterator& sibBegin );

    // Parse frame and its values
    uint32 parseFrame( const rcXMLElementTree& tree,
                         rcXMLElementTree::sibling_iterator& sibBegin );
    
    // Parse value 
    uint32 parseValue( rcXMLElementTree::sibling_iterator& sibBegin, int32 timeId = -1 );

    // Parse graphics collection string, add parsed tokens to collection
    uint32 parseGraphicsCollection( const std::string& content,
                                      rcVisualSegmentCollection& tokens,
                                      uint32 version );
    
    // Experiment mutators
    
    // Set setting value
    uint32 setSettingValue( int settingCategoryIndex, int settingIndec, const std::string& value );

    // Utilities
    
    // Find category with name, return index if found, -1 otherwise
    int findSettingCategory( const std::string& name );
    // Find setting from category, return index if found, -1 otherwise
    int findSetting( int categoryIndex,
                     const std::string& name );
    // Map track type to writer type;
    rcWriterType writerType( rcTrackType type ) const;
    
    rcExperimentImpl*      mExperiment;
    rcEngineObserver*      mObserver;
    rcIdMapper             mIdMapper;           // Track/group id to instance mapper
    vector<std::string>       mImportableSettings; // Names of setting categories to be imported
    rcProgressIndicator*   mProgress;           // Progress indicator
    int32                mFileVersion;        // Detected file version
    const rcXMLNameMapper* mNameMapper;         // Name mapper for detected file version
    const rcXMLTimeMapper* mTimeMapper;         // Time id mapper
    vector <rcTimestamp>   mTimes;              // All timestamps from input
    uint32               mProcessedElements;  // Number of parsed elements
};

#endif // _rcNATIVEIMPORTER_H_
