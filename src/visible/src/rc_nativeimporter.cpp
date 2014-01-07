/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	$Id: rc_nativeimporter.cpp 6565 2009-01-30 03:24:44Z arman $
*
*	This file contains the implementation of the import handler
*   for native file formats.
******************************************************************************/

#include <rc_setting.h>
#include <rc_model.h>

#include <rc_experimentimpl.h>
#include <rc_nativeimporter.h>

using namespace std;


//
// rcNativeImporter implementation
//

rcNativeImporter::rcNativeImporter( rcExperimentImpl* experiment,
                                    rcEngineObserver* observer,
                                    rcProgressIndicator* progress ) :
        mExperiment( experiment ),
        mObserver( observer ),
        mProgress( progress ),
        mFileVersion( 0 ),
        mNameMapper( rfGetXMLNameMapper( 0 ) ),
        mTimeMapper( 0 ),
        mProcessedElements( 0 )
{
    rmAssert( mExperiment );
    rmAssert( mObserver );
}

rcNativeImporter::~rcNativeImporter()
{
    delete mTimeMapper;
}

// Import an experiment tree
int rcNativeImporter::importExperiment( const rcXMLElementTree& tree,
                                        const vector<string>& importableSettings )
{
    int errors = 0;

    if ( mProgress )
        mProgress->progress( 0.0 );
    
    rcTimestamp startTime = rcTimestamp::now();
        
    rcXMLElementTree::iterator treeBegin = tree.begin();
    const rcXMLElement& root = *treeBegin;
    
    // First element MUST be <experiment>
    if ( root.type() != eXMLElementExperiment ) {
        cerr << "rcNativeImporter error: invalid root element <" << mNameMapper->elementName( root.type() ) << ">" << endl;
        ++errors;
    } else {
        // Iterate through children of <experiment>
        rcXMLElementTree::sibling_iterator sibBegin = tree.begin( treeBegin );
        rcXMLElementTree::sibling_iterator sibEnd = tree.end( treeBegin );
        
        while ( sibBegin != sibEnd ) {
            rcXMLElementTree::iterator sib = sibBegin;
            
            switch ( sib->type() ) {
                case eXMLElementExperimentSettingCategory:
                    errors += parseSettingCategory( tree, sib, importableSettings );
                    break;

                case eXMLElementExperimentData:
                    errors += parseExperimentData( tree, sib );
                    break;

                case eXMLElementVersion:
                {
                    const rcXMLElement& e = *sib;
                    rcValue version( e.content() );
                    mFileVersion = version.intValue();
                    cerr << "Detected XML file version " << mFileVersion << endl;

                    if ( mFileVersion < rcXMLOldestSupportedFileVersion ) {
                        cerr << "rcNativeImporter error: unsupported obsolete file version " << mFileVersion << endl;
                        ++errors;
                    }
                    else {
                        mNameMapper = rfGetXMLNameMapper( mFileVersion );
                        rmAssert( mNameMapper );
                    }
                }
                break;
                       
                default:
                    cerr << "rcNativeImporter error: unexpected element <" << mNameMapper->elementName( sib->type() ) << ">" << endl;
                    ++errors;
                    break;
            }
            ++sibBegin;
        }
    }

    // Flush scalar writers
    mIdMapper.flushWriters();
    
    if ( errors > 0 ) {
        cerr << "rcNativeImporter error: " << errors << " errors encountered during experiment import" << endl;
    }
    rcTimestamp elapsedTime = rcTimestamp::now() - startTime;
    cout << tree.size() << " XML elements imported from rcXMLElementTree in " << elapsedTime.secs() << " seconds" << endl;
    
    return errors;
}

// private

// Parse setting element and its children
// Return number of errors
uint32 rcNativeImporter::parseSetting( const rcXMLElementTree& tree,
                                         const rcXMLElementTree::sibling_iterator& settingNode,
                                         int categoryIndex )
{
    uint32 errors = 0;
    
    if ( categoryIndex >= 0 ) {
        int settingIndex = -1;

        // Iterate all children
        rcXMLElementTree::iterator settingBegin = tree.begin( settingNode );
        rcXMLElementTree::iterator settingEnd = tree.end( settingNode );

        while ( settingBegin != settingEnd ) {
            const rcXMLElement& e = *settingBegin;
            
            switch ( e.type() ) {
                case eXMLElementName:
                    settingIndex = findSetting( categoryIndex, e.content() );
                    if ( settingIndex < 0 ) {
                        cerr << "rcNativeImporter error: could not find <" << mNameMapper->elementName( e.type() );
                        cerr << "> with name " << e.content() << endl;
                        ++errors;
                    }
                    break;

                case eXMLElementValue:
                    if ( settingIndex >= 0 ) 
                        errors += setSettingValue( categoryIndex, settingIndex, e.content() );
                    else
                        cerr <<  "rcNativeImporter warning: discarded value " << mNameMapper->elementName( e.type() ) << endl;
                    break;

                default:
                    cerr << "rcNativeImporter error: invalid child <" << mNameMapper->elementName( e.type() );
                    cerr << "> in element <" << mNameMapper->elementName( settingNode->type() ) << ">" << endl;
                    ++errors;
                    break;
            }
            ++settingBegin;
        }
    }

    return errors;
}

// Parse setting category element and its children
// Return number of errors
uint32 rcNativeImporter::parseSettingCategory( const rcXMLElementTree& tree,
                                                 rcXMLElementTree::iterator& sibBegin,
                                                 const vector<string>& importableSettings )
{
    uint32 errors = 0;
    const rcXMLElement& e = *sibBegin;
    rcXMLElementType type = e.type();
    int categoryIndex = -1;
                
    rcXMLElementTree::sibling_iterator begin = tree.begin( sibBegin );
    rcXMLElementTree::sibling_iterator end = tree.end( sibBegin );
                
    while ( begin != end ) {
        const rcXMLElement& child = *begin;
        rcXMLElementType childType = child.type();
        
        switch ( childType ) {
            case eXMLElementName:
            {
                // Got category name, see if we can find a matching setting category
                const string& name = child.content();
                bool loadThis = true;
                
                if ( !name.empty() ) {
                    if ( !importableSettings.empty() ) {
                        loadThis = false;
                        // Only import categories which match this list
                        vector<string>::const_iterator i;
                        for ( i = importableSettings.begin(); i < importableSettings.end(); ++i ) {
                            if ( *i == name ) {
                                // We can load this category
                                loadThis = true;
                                break;
                            }
                        }
                    }
                    if ( loadThis ) {
                        int index = findSettingCategory( name );
                        
                        if ( index >= 0 ) {
                            // Found the matching category
                            categoryIndex = index;
                        } else {
                            //++errors; Be tolerant, allow unknown categories from other apps
                            cerr << "rcNativeImporter warning: could not find <" << mNameMapper->elementName( type );
                            cerr << "> with name " << name << endl;
                        }
                    }
                } else {
                    ++errors;
                    cerr << "rcNativeImporter error: no name defined for <" << mNameMapper->elementName( type ) << ">" << endl;
                }
            }
            break;
            
            case eXMLElementDescription:
                // Ignore intentionally
                break;
                
            case eXMLElementSetting:
                errors += parseSetting( tree, begin, categoryIndex );
                break;

            default:
                cerr << "rcNativeImporter error: invalid child <" << mNameMapper->elementName( childType );
                cerr << "> in element <" << mNameMapper->elementName( type ) << ">" << endl;
                ++errors;
                break;
        }
        ++begin;
    } 

    return errors;
}

// Parse experiment data and its children
// Return number of errors
uint32 rcNativeImporter::parseExperimentData( const rcXMLElementTree& tree,
                                                rcXMLElementTree::iterator& sibBegin )
{
    uint32 errors = 0;
    const rcXMLElement& e = *sibBegin;
    rcXMLElementType type = e.type();
    const uint32 totalElements = tree.size();
    
    rcXMLElementTree::sibling_iterator begin = tree.begin( sibBegin );
    rcXMLElementTree::sibling_iterator end = tree.end( sibBegin );
                
    while ( begin != end ) {
        const rcXMLElement& child = *begin;
        rcXMLElementType childType = child.type();

        // Report progress
        if ( mProgress ) {
            if ( !(mProcessedElements % 16) ) {
                mProgress->progress( double((100.0) * mProcessedElements / totalElements) );
            }
            ++mProcessedElements;
        }
        // Parse nodes
        switch ( childType ) {
            case eXMLElementGroup:
                errors += parseGroup( tree, begin );
            break;

            case eXMLElementTimeStamp:
                errors += parseTime( begin );
                break;

            case eXMLElementTrack:
                errors += parseTrack( tree, begin );
                break;

            case eXMLElementFrame:
                errors += parseFrame( tree, begin );
                break;
                
            // This is for backwards compatibility only, eXMLElementTrackValue is produced now
            // by exporters. TODO: obsolete this soon.
            case eXMLElementValue:
                errors += parseValue( begin );
                break;
                
            case eXMLElementTrackValue:
                errors += parseValue( begin );
                break;
                
            default:
                cerr << "rcNativeImporter error: invalid child <" << mNameMapper->elementName( childType );
                cerr << "> in element <" << mNameMapper->elementName( type ) << ">" << endl;
                ++errors;
                break;
        }
        ++begin;
    } 

    return errors;
}

// Parse group and its children
// Return number of errors
uint32 rcNativeImporter::parseGroup( const rcXMLElementTree& tree,
                                       rcXMLElementTree::sibling_iterator& sibBegin )
{
    uint32 errors = 0;
    const rcXMLElement& e = *sibBegin;
    rcXMLElementType type = e.type();

    string name;
    string description;
    const string id = e.attributeValue( eXMLAttributeGroupId );
        
    rcXMLElementTree::sibling_iterator begin = tree.begin( sibBegin );
    rcXMLElementTree::sibling_iterator end = tree.end( sibBegin );
                
    while ( begin != end ) {
        switch ( begin->type() ) {
            case eXMLElementName:
                name = begin->content();
            break;
            
            case eXMLElementDescription:
                description = begin->content();
                break;
                
            default:
                cerr << "rcNativeImporter error: invalid child <" << mNameMapper->elementName( begin->type() );
                cerr << "> in element <" << mNameMapper->elementName( type ) << ">" << endl;
                ++errors;
                break;
        }
        ++begin;
    } 

    if ( !name.empty() && !description.empty() && !id.empty() ) {
        // Create writer group
        // TODO: add group semantics mapping from name for backwards compatibility
        rcGroupSemantics type = eGroupSemanticsUnknown;
        const string gMeasurementsOldName( "Measurements " );
        const string gMeasurementsName( "Global Measurements" );
        const string gGraphicsName( "Graphical data" );
        const string gCellName( "Cell " );
        
        if ( name.find( gMeasurementsName ) != string::npos ) {
            // Detect global measurements group
            type = eGroupSemanticsGlobalMeasurements;
        } else if ( name.find( gGraphicsName ) != string::npos ) {
            // Detect global graphics group
            type = eGroupSemanticsGlobalGraphics;
        } else if ( name.find( gMeasurementsOldName ) != string::npos &&
                    name.find( gCellName ) == string::npos ) {
            // Detect old global measurements group
            type = eGroupSemanticsGlobalMeasurements;
        } else {
            // Assume all other groups are cell measurement groups
            type = eGroupSemanticsBodyMeasurements;
        }
        
        rcWriterGroup* newGroup = mObserver->createWriterGroup( mNameMapper->elementName( eXMLElementGroup ).c_str(),
                                                                name.c_str(),
                                                                description.c_str(),
                                                                type );
        // Add a name-to-instance mapping
        if ( mIdMapper.addGroup( id, newGroup ) ) {
            // Error, probably a duplicate id string
            cerr << "rcNativeImporter error: addGroup failed for group-id " << id << endl;
            ++errors;
        }
    } else {
        if ( name.empty() ) 
            cerr << "rcNativeImporter error: no name defined for <" << mNameMapper->elementName( type ) << ">" << endl;
        if ( description.empty() ) 
            cerr << "rcNativeImporter error: no description defined for <" << mNameMapper->elementName( type ) << ">" << endl;
        if ( id.empty() ) 
            cerr << "rcNativeImporter error: no group id defined for <" << mNameMapper->elementName( type ) << ">" << endl;
        ++errors;
    }

    return errors;
}

// Parse time value
// Return number of errors
uint32 rcNativeImporter::parseTime( rcXMLElementTree::sibling_iterator& sibBegin )
{
    uint32 errors = 0;
    const rcXMLElement& e = *sibBegin;
    const string timeId = e.attributeValue( eXMLAttributeTimeId );
    
    rcTimestamp parsedTime;
    if ( convertStringToTimestamp( parsedTime, e.content().c_str() ) ) {
        if ( parsedTime < cZeroTime ) {
            parsedTime = cZeroTime;
            cerr << "rcNativeImporter error: negative time " << parsedTime;
            cerr << " for time-id " << timeId << endl;
            ++errors;
        } else
            mTimes.push_back( parsedTime );
    } else
        ++errors;

    return errors;
}

// Parse track and its children
// Return number of errors
uint32 rcNativeImporter::parseTrack( const rcXMLElementTree& tree,
                                       rcXMLElementTree::sibling_iterator& sibBegin )
{
    uint32 errors = 0;
    const rcXMLElement& e = *sibBegin;
    rcXMLElementType type = e.type();

    string elementContent[eXMLElementMax];
    bool requiredElement[eXMLElementMax];
    bool optionalElement[eXMLElementMax];
    
    // TODO: implement this in a DTD class
    for ( uint32 i = 0; i < eXMLElementMax; i++ ) {
        requiredElement[i] = false;
        optionalElement[i] = false;
    }
    // Required elements
    requiredElement[eXMLElementType] = true;
    requiredElement[eXMLElementValueType] = true;
    requiredElement[eXMLElementName] = true;
    requiredElement[eXMLElementDescription] = true;
    requiredElement[eXMLElementAnalysisRect] = true;
    requiredElement[eXMLElementDisplayFormatString] = true;
    requiredElement[eXMLElementSizeLimit] = true;
    //requiredElement[eXMLElementExpectedMin] = true;
    //requiredElement[eXMLElementExpectedMax] = true;
    // Optional elements
    optionalElement[eXMLElementTrackStart] = true;
    optionalElement[eXMLElementExpectedMin] = true;
    optionalElement[eXMLElementExpectedMax] = true;
    optionalElement[eXMLElementTrackVersion] = true;
    optionalElement[eXMLElementVersion] = true;
    
    const string trackId = e.attributeValue( eXMLAttributeTrackId );
    const string groupId = e.attributeValue( eXMLAttributeGroupId );
        
    rcXMLElementTree::sibling_iterator begin = tree.begin( sibBegin );
    rcXMLElementTree::sibling_iterator end = tree.end( sibBegin );

    while ( begin != end ) {
        rcXMLElementType childType = begin->type();

        if ( requiredElement[childType] ) {
            elementContent[childType] = begin->content();         
        } else {
            if ( optionalElement[childType] ) {
                elementContent[childType] = begin->content();
            } else {
                cerr << "rcNativeImporter error: invalid child <" << mNameMapper->elementName( childType );
                cerr << "> in element <" << mNameMapper->elementName( type ) << ">" << endl;
                ++errors;
            }
        }
        ++begin;
    } 

    bool isValid = true;
    // Check if all required elements were present
    for ( uint32 i = 0; i < eXMLElementMax; i++ ) {
        if ( requiredElement[i] && elementContent[i].empty() ) {
            isValid = false;
            break;
        }
    }
    
    if ( isValid ) {
        rcWriterGroup* group = mIdMapper.group( groupId );
      
        if ( group ) {
            // Use int formatting as a default
            if ( elementContent[eXMLElementDisplayFormatString].empty() )
                elementContent[eXMLElementDisplayFormatString] = "%i";

            // TODO: track type mapping
            rcTrackType trackType = rcTrack::type( elementContent[eXMLElementValueType] );
            rcWriterType wType = writerType( trackType );
            
            // Create track
            rcValue sizeLimit( elementContent[eXMLElementSizeLimit] );
            rcValue aRect( elementContent[eXMLElementAnalysisRect] );
            
            rcWriter* newTrack = group->createWriter( wType,
                                                      elementContent[eXMLElementType].c_str(),
                                                      elementContent[eXMLElementName].c_str(),
                                                      elementContent[eXMLElementDescription].c_str(),
                                                      elementContent[eXMLElementDisplayFormatString].c_str(),
                                                      static_cast<unsigned>(sizeLimit.intValue()),
                                                      aRect.rectValue() );
            if ( newTrack ) {
                rcTimestamp parsedStart;
                // All tracks that can be imported can also be exported
                newTrack->setExportable( true );
                if ( convertStringToTimestamp( parsedStart, elementContent[eXMLElementTrackStart].c_str() ) ) {
                    if ( parsedStart < cZeroTime ) {
                        parsedStart = cZeroTime;
                        cerr << "rcNativeImporter error: negative track start time " << parsedStart;
                        cerr << " for track-id " << trackId << endl;
                        ++errors;
                    }
                    // Set track start time so that value timestamp offsets are correct
                    newTrack->setTrackStart( parsedStart );
                }
                // Track versioning
                if ( elementContent[eXMLElementTrackVersion].empty() ) {
                    elementContent[eXMLElementTrackVersion] = elementContent[eXMLElementVersion];
                    if ( elementContent[eXMLElementTrackVersion].empty() ) 
                        elementContent[eXMLElementTrackVersion] = "0";
                }

                // Name-to-instance mapping
                rcValue version( elementContent[eXMLElementTrackVersion] );
                rcTrackElement e( newTrack, aRect.rectValue(), version.intValue() );

                if ( mIdMapper.addTrack( trackId, e ) ) {
                    // Error, probably a duplicate id string
                    cerr << "rcNativeImporter error: addGroup failed for track-id " << trackId << endl;
                    ++errors;
                }

                // Get expected min/max values for the track
                rcValue min( elementContent[eXMLElementExpectedMin] );
                rcValue max( elementContent[eXMLElementExpectedMax] );
                
                switch ( wType ) {
                    case eScalarWriter:
                    {
                        // Track type specfic values
                        rcScalarWriter* scalarTrack = dynamic_cast<rcScalarWriter*>( newTrack );
                        if ( scalarTrack ) {
                            scalarTrack->setExpectedMinValue( min.doubleValue() );
                            scalarTrack->setExpectedMaxValue( max.doubleValue() );
                        } else {
                            ++errors;
                            cerr << "rcNativeImporter error: dynamic_cast<rcScalarWriter*> failed for track " << trackId << endl;
                        }
                    }
                    break;
                    
                    case ePositionWriter:
                    {
                        // Track type specfic values
                        rcPositionWriter* positionTrack = dynamic_cast<rcPositionWriter*>( newTrack );
                        if ( positionTrack ) {
                            positionTrack->setExpectedMinValue( min.positionValue() );
                            positionTrack->setExpectedMaxValue( max.positionValue() );
                        } else {
                            ++errors;
                            cerr << "rcNativeImporter error: dynamic_cast<rcPositionWriter*> failed for track " << trackId << endl;
                        }
                    }
                    break;

                    case eGraphicsWriter:
                    {
                        // Track type specfic values
                        rcGraphicsWriter* graphicsTrack = dynamic_cast<rcGraphicsWriter*>( newTrack );
                        if ( graphicsTrack ) {
                            if ( e.version() == 0 ) {
                                cerr << "rcNativeImporter error: no version number";
                                cerr << " for graphics track-id " << trackId << endl;
                                ++errors;
                            }
                        } else {
                            ++errors;
                            cerr << "rcNativeImporter error: dynamic_cast<rcGraphicsWriter*> failed for track " << trackId << endl;
                        }
                    }
                    break;
                    
                    default:
                        // Track type not implemented yet
                        cerr << "rcNativeImporter warning: track type " << elementContent[eXMLElementValueType];
                        cerr << " import not implemented yet" << endl;
                        rmAssert( 0 );
                        break;
                }
            } else {
                cerr << "rcNativeImporter error: track creation failed for track-id " << trackId << endl;
                ++errors;
            }
        } else {
            // Unknown group id
            cerr << "rcNativeImporter error: no group found with attribute " << groupId  << endl;
            ++errors;
        }
    } else {
        // Required element(s) missing
        for ( uint32 i = 0; i < eXMLElementMax; i++ ) {
            if ( requiredElement[i] && elementContent[i].empty() ) {
                cerr << "rcNativeImporter error: required element <" << mNameMapper->elementName( static_cast<rcXMLElementType>(i) ) << ">";
                cerr << " missing from <" << mNameMapper->elementName( type ) << ">" << endl;
                ++errors;
            }
        }
    }
    
    return errors;
}

// Parse frame element
// It contains a time attribute and its children are all values for that time
uint32 rcNativeImporter::parseFrame( const rcXMLElementTree& tree,
                                       rcXMLElementTree::sibling_iterator& sibBegin )
{
    uint32 errors = 0;
    const rcXMLElement& e = *sibBegin;
    rcXMLElementType type = e.type();
    int32 id = -1;
    rcTimestamp parsedStamp(-1.0);
    
    // Create time mapper if it's not created yet
    if ( !mTimeMapper ) {
        mTimeMapper = new rcXMLTimeMapper( mTimes );
    }

    if ( mTimeMapper ) {
        const string timeId = e.attributeValue( eXMLAttributeTimeId );
        if ( !timeId.empty() ) {
            id = rcValue( timeId );
            parsedStamp = mTimeMapper->timeValue( id );
            if ( parsedStamp < cZeroTime )
                id = -1;
        }
    }

    if ( id < 0 ) {
        cerr << "rcNativeImporter error: invalid time idx " << id << " and timestamp value "
             << parsedStamp << " for <" << mNameMapper->elementName( type ) << ">" << endl;
        ++errors;
    } 
    
    rcXMLElementTree::sibling_iterator begin = tree.begin( sibBegin );
    rcXMLElementTree::sibling_iterator end = tree.end( sibBegin );
                
    while ( begin != end ) {
        ++mProcessedElements;
        switch ( begin->type() ) {
            case eXMLElementTrackValue:
                errors += parseValue( begin, id );
                break;
            
            default:
                cerr << "rcNativeImporter error: invalid child <" << mNameMapper->elementName( begin->type() );
                cerr << "> in element <" << mNameMapper->elementName( type ) << ">" << endl;
                ++errors;
                break;
        }
        ++begin;
    }

    return errors;
}

// Parse value
// Return number of errors
uint32 rcNativeImporter::parseValue( rcXMLElementTree::sibling_iterator& sibBegin,
                                       int32 id )
{
    uint32 errors = 0;
    
    // Create time mapper if it's not created yet
    if ( !mTimeMapper ) {
        mTimeMapper = new rcXMLTimeMapper( mTimes );
    }
    
    const rcXMLElement& e = *sibBegin;
    const rcXMLElementType type = e.type();
    const string trackId = e.attributeValue( eXMLAttributeTrackId );
    const string content = e.content();

    if ( !content.empty() && !trackId.empty() ) {
        // Find track
        rcTrackElement tr = mIdMapper.track( trackId );
        
        if ( tr.isValid() ) {
            if ( mTimeMapper ) {
                rcTimestamp parsedStamp(-1.0);
                bool validTime = false;

                // No valid time from frame, old file format
                if ( id < 0 ) {
                    // Time id
                    const string timeId = e.attributeValue( eXMLAttributeTimeId );
    
                    if ( timeId.empty() ) {
                        // Oldest file format with full time stamp
                        const string timeStamp = e.attributeValue( eXMLAttributeTime );
                        validTime = convertStringToTimestamp( parsedStamp, timeStamp );
                    } else {
                        id = rcValue( timeId ); // Warning: assigning to function argument
                        parsedStamp = mTimeMapper->timeValue( id );
                        validTime = (parsedStamp >= cZeroTime);
                    }
                } else {
                    parsedStamp = mTimeMapper->timeValue( id );
                    validTime = (parsedStamp >= cZeroTime);
                }

                if ( validTime ) {
                    rcValue value( content );
                    rcWriter* track = tr.track();
                    if ( track->getWriterType() == eScalarWriter ) {
                        rcScalarWriter* scalarTrack = dynamic_cast<rcScalarWriter*>( track );
                        
                        if ( scalarTrack != 0 ) {
                            scalarTrack->writeValue( parsedStamp,
                                                     tr.rect(),
                                                     value.doubleValue() );
                        } else {
                            cerr << "rcNativeImporter error: dynamic_cast<rcScalarWriter*> failed for track " << trackId << endl;
                            ++errors;
                        }
                    } else if ( track->getWriterType() == ePositionWriter ) {
                        rcPositionWriter* positionTrack = dynamic_cast<rcPositionWriter*>( track );
                        
                        if ( positionTrack != 0 ) {
                            positionTrack->writeValue( parsedStamp,
                                                       tr.rect(),
                                                       value.positionValue() );
                        } else {
                            cerr << "rcNativeImporter error: dynamic_cast<rcPositionWriter*> failed for track " << trackId << endl;
                            ++errors;
                        }
                    } else if ( track->getWriterType() == eGraphicsWriter ) {
                        rcGraphicsWriter* graphicsTrack = dynamic_cast<rcGraphicsWriter*>( track );
                        
                        if ( graphicsTrack != 0 ) {
                            rcVisualGraphicsCollection graphics;
                            
                            errors += parseGraphicsCollection( content, graphics.segments(), tr.version() );
                            
                            graphicsTrack->writeValue( parsedStamp,
                                                       tr.rect(),
                                                       graphics );
                        } else {
                            cerr << "rcNativeImporter error: dynamic_cast<rcGraphicsWriter*> failed for track " << trackId << endl;
                            ++errors;
                        }
                    } else {
                        cerr << "rcNativeImporter warning: value import for track type " << track->getWriterType() << " is unimplemented" << endl;
                        ++errors;
                    }
                }
                else {
                    cerr << "rcNativeImporter error: invalid time stamp value " << parsedStamp << " for <" << mNameMapper->elementName( type ) << ">" << endl;
                    ++errors;
                }
            } else {
                cerr << "rcNativeImporter internal error: no time id mapper available" << endl;
                ++errors;
            }
        } else {
            // Unknown track id
            cerr << "rcNativeImporter error: no track found with id " << trackId  << endl;
            ++errors;
        }

    } else {
        if ( content.empty() ) 
            cerr << "rcNativeImporter error: no value defined for <" << mNameMapper->elementName( type ) << ">" << endl;
        if ( trackId.empty() ) 
            cerr << "rcNativeImporter error: no track id defined for <" << mNameMapper->elementName( type ) << ">" << endl;
        ++errors;
    }

    return errors;
}

// Set setting value
uint32 rcNativeImporter::setSettingValue( int settingCategoryIndex,
                                            int settingIndex,
                                            const string& value )
{
    uint32 errors = 0;
    
    if ( settingCategoryIndex >= 0 && settingIndex >= 0 ) {
        int nCategories = mExperiment->getNSettingCategories();
        
        if ( settingCategoryIndex < nCategories ) {
            rcSettingCategory category = mExperiment->getSettingCategory( settingCategoryIndex );
            int32 nSettings = category.getNSettings();
            
            if ( settingIndex < nSettings ) {
                rcSettingInfo setting = category.getSettingInfo( settingIndex );
                rcValue newValue( value );
                // String value will be converted automatically to the appropriate
                // internal setting type
                setting.setValue( newValue );
            } else {
                ++errors; // Invalid setting index
            }
        } else {
            ++errors; // Invalid setting category index
        }
    } else {
        ++errors; // Invalid setting/category index           
    }

    return errors;
}

// Find category with name, return index if found, -1 otherwise
int rcNativeImporter::findSettingCategory( const string& name )
{
    int index = -1;
    int nCategories = mExperiment->getNSettingCategories();

    // TODO: replace linear search with something faster
    for ( int32 i = 0; i < nCategories; i++ )
    {
        rcSettingCategory category = mExperiment->getSettingCategory( i );
        const string categoryName( category.getName() );
        if ( name == categoryName ) {
            // Found a category with the same name, return index
            index = i;
            break;
        }
    }

    return index;
}

// Find setting from category, return index if found, -1 otherwise
int rcNativeImporter::findSetting( int categoryIndex,
                                   const string& name )
{
    int index = -1;
    
    rcSettingCategory category = mExperiment->getSettingCategory( categoryIndex );
    uint32 nSettings = category.getNSettings();

    // TODO: replace linear search with something faster
    for ( uint32 i = 0; i < nSettings; i++ )
    {
        rcSettingInfo setting = category.getSettingInfo( i );
        const string settingName( setting.getTag() );
        if ( name == settingName ) {
            // Found a setting with the same name, return index
            index = i;
            break;
        }
    }

    return index;
}

// Map track type to writer type;
rcWriterType rcNativeImporter::writerType( rcTrackType type ) const
{
    switch( type ) {
        case eVideoTrack:
            return eVideoWriter;
        case eScalarTrack:
            return eScalarWriter;
        case ePositionTrack:
            return ePositionWriter;
        case eGraphicsTrack:
            return eGraphicsWriter;
        case eMaxTrack:
            rmAssert( 0 );
            return static_cast<rcWriterType>(0xFF);
    }

    return static_cast<rcWriterType>(0xFF);
}

// Parse graphics token string, add parsed tokens to collection
uint32 rcNativeImporter::parseGraphicsCollection( const string& content,
                                                    rcVisualSegmentCollection& tokens,
                                                    uint32 version )
{
    uint32 errors = 0;

    if ( version == rcVisualSegment::ePersistenceVersion == rcGraphicsTrack::ePersistenceVersion ) {
        int pos = 0;
        int count = 0;
        char* string = const_cast<char*>(content.c_str());
        int len = content.size();
        char* p = 0;
        
        // Warning: commas are replaced with NULLs to make sscanf operation faster.
        // sscanf calls strlen() which will be slow for large strings
        
        // Try comma-separated format
        if ( (p = strchr( string, ',' ) ) ) 
            *p = 0; // Warning, input string is mutated
        
        if ( sscanf( string, "%i%n", &count, &pos ) == 1) {
            string += pos+1;
            
            if ( count > 0 ) {
                tokens.reserve( count );
                
                for( int i = 0; i < count; ++i ) {
                    if ( !string || pos >= len ) {
                        cerr << "rcNativeImporter error: premature graphics token sscanf EOF" << endl;
                        ++errors;
                        break;
                    }
                    if ( (p = strchr( string, ',' ) ) ) 
                        *p = 0; // Warning, input string is mutated
                    // Determine token type
                    int type = 0;
                    if (sscanf( string, "%i%n" ,
                                &type, &pos ) == 1) {
                        string += pos+1;
                        rcVisualSegment::rcVisualSegmentType t = rcVisualSegment::rcVisualSegmentType(type);
                        if ( t <= rcVisualSegment::eUnknown || t >= rcVisualSegment::eLast ) {
                            cerr << "rcNativeImporter error: invalid graphics token type " << t << endl;
                            ++errors;
                            continue;
                        }
                        if ( t ==  rcVisualSegment::eStyle ) {
                            uint32 color;
                            float f[3];
                            bool valid = true;
                            
                            // Read color int field
                            if ( (p = strchr( string, ',' ) ) ) 
                                *p = 0; // Warning, input string is mutated
                            if (sscanf( string, "%i%n" ,
                                        &color, &pos ) == 1) {
                                string += pos+1;
                            } else {
                                cerr << "rcNativeImporter error: graphics style token field " << 0 << " read failed at line position " << pos << endl;
                                ++errors;
                                valid = false;
                            }
                            if ( valid ) {
                                // Read 3 float fields
                                for ( int field = 0; field < 3; ++field ) {
                                    if ( (p = strchr( string, ',' ) ) ) 
                                        *p = 0; // Warning, input string is mutated
                                    if (sscanf( string, "%f%n" ,
                                                &f[field], &pos ) == 1) {
                                        string += pos+1;
                                    } else {
                                        cerr << "rcNativeImporter error: graphics token field " << field+1 << " read failed at line position " << pos << endl;
                                        ++errors;
                                        valid = false;
                                        break;
                                    }
                                }
                            }
                            if ( valid ) {
                                rcVisualStyle s( color, uint32(f[0]),
                                                 rc2Fvector( f[1], f[2]) );
                                tokens.push_back( s );
                            }
                        } else {
                            float f[4];
                            bool valid = true;
                            // Read 4 float fields
                            for ( int field = 0; field < 4; ++field ) {
                                if ( (p = strchr( string, ',' ) ) ) 
                                    *p = 0; // Warning, input string is mutated
                                if (sscanf( string, "%f%n",
                                            &f[field], &pos ) == 1) {
                                    string += pos+1;
                                } else {
                                    cerr << "rcNativeImporter error: graphics token field " << field << " read failed at line position " << pos << endl;
                                    ++errors;
                                    valid = false;
                                    break;
                                }
                            }
                            if ( valid ) {
                                rcVisualSegment s( t,
                                                   rc2Fvector( f[0], f[1] ),
                                                   rc2Fvector( f[2], f[3] ) );
                                tokens.push_back( s );
                            }
                        }
                    } else {
                        cerr << "rcNativeImporter error: graphics token type field read failed at line position " << pos << endl;
                        ++errors;
                    }
                }
            }
        } else {
            cerr << "rcNativeImporter error: graphics segment count failed " << endl;
            ++errors;
        }

    } else {
        // Catch track and token version mismatch
        if ( int(rcVisualSegment::ePersistenceVersion) != int(rcGraphicsTrack::ePersistenceVersion) ) {
            cerr << "rcNativeImporter internal error: graphics track version " << rcGraphicsTrack::ePersistenceVersion;
            cerr << " is different from graphics token version " << rcVisualSegment::ePersistenceVersion << endl;
            ++errors;
        }
        
        // File version different from current version
        if ( version > rcGraphicsTrack::ePersistenceVersion ) {
            cerr << "rcNativeImporter error: graphics track version " << version;
            cerr << " is newer than current version " << rcVisualSegment::ePersistenceVersion << endl;
            ++errors;
        } else {
            // File has an old version
            // TODO: implement compatibility transformation for old versions
            cerr << "rcNativeImporter error: graphics track version " << version;
            cerr << " is older than current version " << rcVisualSegment::ePersistenceVersion << endl;
            ++errors;
        }
    }
        return errors;;
}
