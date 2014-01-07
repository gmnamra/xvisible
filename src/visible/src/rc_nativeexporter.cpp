/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	rc_nativeexporter.cpp
*
*	This file contains the implementation of the export handler
*   for native file formats.
******************************************************************************/

#include <strstream>
#include <vector>

#include <rc_setting.h>
#include <rc_model.h>

#include "rc_nativeexporter.h"

#if WIN32
using namespace std;
#endif

// XML declaration
static const std::string cXmlDeclaration = "<?xml version=\"1.0\" encoding=\"ISO-8859_1\" standalone=\"yes\"?>\n";

// Comment strings
static const std::string cGroupComment( " Measurement group definitions " );
static const std::string cTrackComment( " Measurement track definitions " );
static const std::string cValueComment( " Measurement values " );
static const std::string cTimeComment( " Measurement times " );
static const std::string cSettingCategoryComment( " Application settings " );

//
// rcNativeExporter implementation
//

rcNativeExporter::rcNativeExporter( const char* fileName,
                                    rcExperiment* experiment,
                                    rcExperimentFileFormat format,
                                    const char* comment,
                                    rcProgressIndicator* progress ) :
        rcExporter( fileName, format, experiment, comment, progress ),
        mOutputFile( fileName ), mNameMapper( rfGetXMLNameMapper( rcXMLFileVersion ) )
{
}

rcNativeExporter::~rcNativeExporter()
{
}

// Export experiment to stream
int rcNativeExporter::exportExperiment()
{
    uint32 localErrors = 0;
    int err = 0;

    // Set a large output stream buffer for improved speed
    char streamBuf[65536*2];
    
    mOutputFile.rdbuf()->pubsetbuf( streamBuf, sizeof( streamBuf ) );
  
    if ( mOutputFile.fail() )
        err = EIO;
    if ( mOutputFile.bad() )
        err = EIO;

    if ( !err ) {
        rcTimestamp startTime = rcTimestamp::now();

        if ( mProgress )
            mProgress->progress( 0.0 );
        
        // Element tree
        rcXMLElementTree tree;

        // Construct root element
        rcXMLElement rootElem( eXMLElementExperiment );
        rcXMLElementTree::iterator experimentNode = tree.set_head( rootElem );

        // Construct file version number
        produceVersion( tree, experimentNode );
        
        // Construct (generator) info preamble
        localErrors += producePreamble( tree, experimentNode );
        
        // Construct experiment and generator settings
        localErrors += produceSettings( tree, experimentNode );
         
        // Construct experiment data
        rcXMLElement experimentDataElem( eXMLElementExperimentData );
        rcXMLElementTree::iterator experimentDataNode = tree.append_child( experimentNode, experimentDataElem );

        if ( mProgress )
            mProgress->progress( 5.0 );
          
        // Construct group data
        localErrors += produceGroups( tree, experimentDataNode );
        if ( mProgress )
            mProgress->progress( 7.5 );
         
        // Construct track data
        localErrors += produceTracks( tree, experimentDataNode );
        if ( mProgress )
            mProgress->progress( 10.0 );

        // Construct time data
        localErrors += produceTimes( tree, experimentDataNode );
        if ( mProgress )
            mProgress->progress( 15.0 );
        
        // Construct result data
        localErrors += produceResults( tree, experimentDataNode, 10.0, 50.0 );
        if ( mProgress )
            mProgress->progress( 75.0 );

        // Output XML declaration
#ifdef notyet // The declaration seems to confuse Qt XML parser, need to investigate
        mOutputFile << xmlDeclaration();
#endif
        // Construct element count comment
        localErrors += produceElementCount( mOutputFile, tree );
        
        rcTimestamp elapsedTime = rcTimestamp::now() - startTime;
        cout << tree.size() << " XML elements exported to rcXMLElementTree in " << elapsedTime.secs() << " seconds" << endl;

        startTime = rcTimestamp::now();
        // Output the constructed element data tree
        output( mOutputFile, tree, mProgress, 75.0 );
        
        // Close the file and exit
        mOutputFile.close();

        elapsedTime = rcTimestamp::now() - startTime;
        cout << tree.size() << " XML elements exported to file in " << elapsedTime.secs() << " seconds" << endl;
    
        if ( mOutputFile.fail() ) 
            err = EIO;
        if ( mOutputFile.bad() ) 
            err = EIO;

        if ( mProgress )
            mProgress->progress( 100.0 );
    }
    
    // Check for errors
    if ( localErrors ) {
        fprintf( stderr, "Error: %i errors from rcNativeExporter::exportExperiment()\n", localErrors );
        if ( ! err )
            err = EIO;
    }

    return err;
}

// Returns XML declaration
const std::string& rcNativeExporter::xmlDeclaration() const
{
    return cXmlDeclaration;
}

// private

// Add all setting elements to document tree
// Return true on success
uint32 rcNativeExporter::produceSettings( rcXMLElementTree& tree,
                                            const rcXMLElementTree::iterator& curNode ) const
{
    uint32 errors = 0;
    uint32 nCategories = mExperiment->getNSettingCategories();
    
    if ( nCategories > 0 ) {
        // Add comment
        rcXMLElement commentElem( eXMLElementComment, cSettingCategoryComment );
        tree.append_child( curNode, commentElem );
    }
    
    // Output all settings
    for ( uint32 i = 0; i < nCategories; i++ )
    {
        rcSettingCategory category = mExperiment->getSettingCategory( i );
        uint32 nSettings = category.getNSettings();
        uint32 persistableCount = 0;
        
        // Count the number of persistable setting in this category
        for ( uint32 j = 0; j < nSettings; j++ )
        {
            rcSettingInfo setting = category.getSettingInfo( j );
            if ( setting.isPersistable() )
                ++persistableCount;
        }

        if ( persistableCount > 0 ) {
            rcXMLElement categoryElem( eXMLElementExperimentSettingCategory );
            rcXMLElementTree::iterator categoryNode = tree.append_child( curNode, categoryElem );
            // Add category name and description
            rcXMLElement nameElem( eXMLElementName, category.getName() );
            rcXMLElement descriptionElem( eXMLElementDescription, category.getDescription() );
            tree.append_child( categoryNode, nameElem );
            tree.append_child( categoryNode, descriptionElem );
            
            for ( uint32 j = 0; j < nSettings; j++ )
            {
                rcSettingInfo setting = category.getSettingInfo( j );
                
                // Persistability check
                if ( setting.isPersistable() ) {
                    rcXMLElement settingElem( eXMLElementSetting );
                    const rcXMLElementTree::iterator settingNode = tree.append_child( categoryNode, settingElem );
                    // Add setting name and value
                    rcXMLElement nameElem( eXMLElementName, setting.getTag() );
                    rcXMLElement valueElem( eXMLElementValue, setting.getValue() );
                    
                    tree.append_child( settingNode, nameElem );
                    tree.append_child( settingNode, valueElem );
                }
            }
        }
    }

    return errors;
}

// Add all group elements to document tree
// Return number of errors
uint32 rcNativeExporter::produceGroups( rcXMLElementTree& tree,
                                          const rcXMLElementTree::iterator& curNode ) const
{
    uint32 errors = 0;

    if ( mExportGroups.size() > 0 ) {
        // Add comment
        rcXMLElement commentElem( eXMLElementComment, cGroupComment );
        tree.append_child( curNode, commentElem );
    }

    // Output group info for all groups in export collection
    for ( uint32 i = 0; i < mExportGroups.size(); i++ )
    {
        rcTrackGroup* group = mExperiment->getTrackGroup( mExportGroups[i] );
        rcAttributeList attrs;
        // Group id attribute
        rcAttribute groupAttribute( eXMLAttributeGroupId,
                                    produceIdString( eXMLAttributeGroupId, mExportGroups[i] ) );
        attrs.push_back( groupAttribute );
        rcXMLElementType type = mNameMapper->elementType( group->getTag() );
        if ( type != eXMLElementMax ) {
            rcXMLElement elem( type, attrs );
            rcXMLElementTree::iterator groupNode = tree.append_child( curNode, elem );
            
            rcXMLElement elemName( eXMLElementName, group->getName() );
            rcXMLElement elemDescription( eXMLElementDescription, group->getDescription() ); 
            tree.append_child( groupNode, elemName );
            tree.append_child( groupNode, elemDescription );
        } else {
            fprintf( stderr, "rcNativeExporter error: unknown element <%s> could not be exported\n", group->getTag() );
            ++errors;
        }
    }
    
    return errors;
}

// Add all track elements to document tree
// Return number of errors
uint32 rcNativeExporter::produceTracks( rcXMLElementTree& tree,
                                          const rcXMLElementTree::iterator& curNode ) const
{
    uint32 errors = 0;

    if ( mExportIterators.size() > 0 ) {
        // Add comment
        rcXMLElement commentElem( eXMLElementComment, cTrackComment );
        tree.append_child( curNode, commentElem );
    }
     
    for ( uint32 i = 0; i < mExportIterators.size(); i++ )
    {
        rcTrack* track = mExportTracks[ i ];
        rcAttributeList attrs;
        attrs.reserve( 2 );
        
        // Track id attribute
        rcAttribute trackAttribute( eXMLAttributeTrackId,
                                    produceIdString( eXMLAttributeTrackId, i ) );
        attrs.push_back( trackAttribute );
        // Group id attribute
        rcAttribute groupAttribute( eXMLAttributeGroupId,
                                    produceIdString( eXMLAttributeGroupId, mExportIteratorGroups[i] ) );
        attrs.push_back( groupAttribute );
        // Produce open tag with all attributes
        rcXMLElement trackElem( eXMLElementTrack, attrs );
        rcXMLElementTree::iterator trackNode = tree.append_child( curNode, trackElem );

        // Track name, type, value type, description
        rcXMLElement typeElem( eXMLElementType, track->getTag() );
        rcXMLElement valueTypeElem( eXMLElementValueType, rcTrack::name( track->getTrackType() ) );
        rcXMLElement nameElem( eXMLElementName, track->getName() );
        rcXMLElement descriptionElem( eXMLElementDescription, track->getDescription() );

        tree.append_child( trackNode, typeElem );
        tree.append_child( trackNode, valueTypeElem );
        tree.append_child( trackNode, nameElem );
        tree.append_child( trackNode, descriptionElem );

        // Produce generic elements/attributes
        // Size limit
        rcValue size( static_cast<int>(track->getSizeLimit()) );

        // Track start time
        ostrstream timestr;
        timestr <<  track->getTrackStart() << ends;
        rcValue start( timestr.str() );
        timestr.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
        // Elements for size limit, format string, start time
        rcXMLElement sizeElem( eXMLElementSizeLimit, size.stringValue() );
        rcXMLElement formatElem( eXMLElementDisplayFormatString, track->getDisplayFormatString() );
        rcXMLElement startElem( eXMLElementTrackStart, start.stringValue() );
        tree.append_child( trackNode, sizeElem );
        tree.append_child( trackNode, formatElem );
        tree.append_child( trackNode, startElem );
        // Track analysis area
        ostrstream str;
        rcRect analysisRect = track->getAnalysisRect();
        rcValue rect( analysisRect );
        if ( !analysisRect.width() || !analysisRect.height() )
            cerr << "rcNativeExporter::produceTracks() warning: dimensioless analysis rect " << analysisRect
                 << " for track " << track->getName() << endl;
        str << rect << ends;
        rcXMLElement analysisAreaElem( eXMLElementAnalysisRect, str.str() );
        tree.append_child( trackNode, analysisAreaElem );
        str.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns

        int32 versionNumber = 0;
        
        if ( track->getTrackType() == eScalarTrack ) {
            // Produce scalar-specific elements
            rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );
            if ( scalarTrack != 0 ) {
                rcValue min( scalarTrack->getExpectedMinimumValue() );
                rcValue max( scalarTrack->getExpectedMaximumValue() );

                // Expected min/max 
                rcXMLElement minElem( eXMLElementExpectedMin, min.stringValue() );
                rcXMLElement maxElem( eXMLElementExpectedMax, max.stringValue() );
                tree.append_child( trackNode, minElem );
                tree.append_child( trackNode, maxElem );
                // Persistence version number
                versionNumber = rcScalarTrack::ePersistenceVersion;
            }
        } else if ( track->getTrackType() == ePositionTrack ) {
            // Produce position-specific elements
            rcPositionTrack* positionTrack = dynamic_cast<rcPositionTrack*>( track );
            if ( positionTrack != 0 ) {
                rcValue min( positionTrack->getExpectedMinimumValue() );
                rcValue max( positionTrack->getExpectedMaximumValue() );

                // Expected min/max
                rcXMLElement minElem( eXMLElementExpectedMin, min.stringValue() );
                rcXMLElement maxElem( eXMLElementExpectedMax, max.stringValue() );
                tree.append_child( trackNode, minElem );
                tree.append_child( trackNode, maxElem );
                // Persistence version number
                versionNumber = rcPositionTrack::ePersistenceVersion;
            }
        } else if ( track->getTrackType() == eGraphicsTrack ) {
            // Persistence version number
            versionNumber = rcGraphicsTrack::ePersistenceVersion;
             // Catch track and token version mismatch
            if ( int(rcVisualSegment::ePersistenceVersion) != versionNumber ) {
                cerr << "rcNativeExporter internal error: graphics track version " << rcGraphicsTrack::ePersistenceVersion;
                cerr << " is different from graphics token version " << rcVisualSegment::ePersistenceVersion << endl;
                ++errors;
            }
        }

        // Persistence version number
        rcValue version( versionNumber );
        rcXMLElement versionElem( eXMLElementTrackVersion, version.stringValue() );
        tree.append_child( trackNode, versionElem );
    }
    
    return errors;
}

// Add all time elements to document tree
// Return number of errors
uint32 rcNativeExporter::produceTimes( rcXMLElementTree& tree,
                                         const rcXMLElementTree::iterator& curNode ) const
{
    uint32 errors = 0;

    if ( mTimes.size() ) {
        // Add comment
        rcXMLElement commentElem( eXMLElementComment, cTimeComment );
        tree.append_child( curNode, commentElem );
        // Add comment with start time in human readable form
        std::string startTime = " Time 0: ";
        startTime += mTimes[0].localtime();
        rcXMLElement commentElem2( eXMLElementComment, startTime );
        tree.append_child( curNode, commentElem2 );
    }
        
    for ( uint32 i = 0; i < mTimes.size(); ++i ) {
        rcAttributeList attrs;
        // Time id attribute
        rcAttribute timeAttribute( eXMLAttributeTimeId,
                                    produceIdString( eXMLAttributeTimeId, i ) );
        attrs.push_back( timeAttribute );
        // Time value
        ostrstream timestr;                
        timestr << mTimes[i] << ends;
        timestr.freeze();
        
        // Produce open tag with all attributes
        rcXMLElement timeElem( eXMLElementTimeStamp, attrs, timestr.str() );
        tree.append_child( curNode, timeElem );
    }

    return errors;
}

// Add all analysis result elements to document tree
// Return number of errors
uint32 rcNativeExporter::produceResults( rcXMLElementTree& tree,
                                           const rcXMLElementTree::iterator& curNode,
                                           double completedPercentage, double maxPercentage ) 
{
    uint32 errors = 0;
    uint32 processedElements = 0;
    
    if ( mExportIterators.size() > 0 ) {
        rcExperimentDomain* domain = rcExperimentDomainFactory::getExperimentDomain();
        rcTimestamp experimentStart = domain->getExperimentStart();
    
        // Add comment
        rcXMLElement commentElem( eXMLElementComment, cValueComment );
        tree.append_child( curNode, commentElem );
        
        // Produce values for each track
        rcTimestamp current = cZeroTime;
        vector<rcAttribute> trackIdAttributes( mExportIterators.size() );
        uint32 activeTracks = 0;

        // Time mapper
        rcXMLTimeMapper timeMap( mTimes );
        
        // Precompute some values
        for ( uint32 i = 0; i < mExportIterators.size(); i++ ) {
            // Track id attribute
            const rcAttribute trackAttribute( eXMLAttributeTrackId,
                                              produceIdString( eXMLAttributeTrackId, i ) );
            trackIdAttributes[i] = trackAttribute;
            if ( mExportActiveIterators[i] ) {
                rcSegmentIterator* iter = mExportIterators[ i ];
                // Deactivate empty tracks
                if ( iter->getSegmentCount() > 0 )
                    ++activeTracks;
                else
                    mExportActiveIterators[i] = false;
            }
        }
        
        if ( activeTracks > 0 )
        {
            // Iterate all the active tracks
            for ( uint32 a = 0; a < mTimes.size(); ++a ) {
                rcTimestamp time = mTimes[a] - experimentStart;
                // Init all iterators with the correct start time
                init( time );
                // Current absolute time
                current = mTimes[a];

                // Time id attribute
                int idx = a;

                if ( idx < 0 ) {
                    ostrstream timestr;                
                    timestr << current << ends;
                    timestr.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
                    fprintf( stderr, "rcNativeExporter error: time value %s cannot be mapped to index\n",
                             timestr.str());
                    ++errors;
                } else {
                    rmAssertDebug( current == timeMap.timeValue( idx ) );
                    if ( time < cZeroTime ) {
                        ostrstream timestr;                
                        timestr << current << ends;
                        timestr.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
                        fprintf( stderr, "rcNativeExporter error: current time value %s is negative\n",
                             timestr.str());
                        ++errors;
                    }
                }
                rcAttribute timeIdAttribute( eXMLAttributeTimeId,
                                             produceIdString( eXMLAttributeTimeId, idx ) );
                // Produce frame element
                rcAttributeList attrs;
                attrs.push_back( timeIdAttribute );
                rcXMLElement frameElem( eXMLElementFrame, attrs );
                const rcXMLElementTree::iterator& timeNode = tree.append_child( curNode, frameElem );
                         
                // Output all the values
                for ( uint32 i = 0; i < mExportIterators.size(); i++)
                {
                    if ( mExportActiveIterators[i] ) {
                        rcSegmentIterator* iter = mExportIterators[ i ];
                    
                        // Skip if iterator doesn't point to a valid segment
                        if ( !iter->contains( time ) )
                            continue;
                        
                        rcAttributeList attrs;
                        // Track id attribute
                        attrs.push_back( trackIdAttributes[i] );
                        
                        // Get value
                        ostrstream str;
                        rcTrackType type =  mExportTracks[i]->getTrackType();
                        if ( !produceValue( str, iter, type, eExperimentNativeFormat ) ) {
                            fprintf( stderr, "rcNativeExporter error: produceValue() failed\n" );
                            ++errors;
                        }
                        str << ends;
                        str.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
                        // Produce value element
                        rcXMLElement valueElem( eXMLElementTrackValue, attrs, std::string( str.str() ) );
                        tree.append_child( timeNode, valueElem );
                        
                        if ( mProgress ) {
                            if ( !(processedElements % 256)) {
                                // Progress range is [completedPercentage-maxPercentage] %
                                double completed = 1.0 * processedElements / mValuesToExport;
                                mProgress->progress( completedPercentage + (maxPercentage-completedPercentage) * completed);
                            }
                            ++processedElements;
                        }
                    }
                    
                }
            }
        }
    }

    return errors;
}

// Produce element ID strings
const std::string rcNativeExporter::produceIdString( rcXMLAttributeType type,
                                                  uint32 id ) const
{
    rmUnused( type );
    char buf[32];

    snprintf( buf, rmDim(buf), "%i", id );
    
    return std::string( buf );
}

// Produce info preamble (generator info etc.)
// Return number of errors
uint32 rcNativeExporter::producePreamble( rcXMLElementTree& tree,
                                            const rcXMLElementTree::iterator& curNode ) const
{
    uint32 errors = 0;
   
    // Add generator comment
    std::string comment( " Document generator: " );
    comment += mExportGeneratorComment + " ";
    rcXMLElement commentElem( eXMLElementComment, comment );
    tree.append_child( curNode, commentElem );

    return errors;
}

// Produce element count comment to stream
// Return number of errors
uint32 rcNativeExporter::produceElementCount( ostream& stream,
                                                const rcXMLElementTree& tree ) const
{
    uint32 errors = 0;

    // Add element count (useful for progress indication)
    char buf[128];
    if ( !snprintf( buf, rmDim(buf), rcXMLElementCountFormat.c_str(), tree.size( ) ) ) 
        ++errors;
    else
        stream << buf << endl;

    return errors;
}

// Produce file format version number
// Return number of errors
uint32 rcNativeExporter::produceVersion( rcXMLElementTree& tree,
                                           const rcXMLElementTree::iterator& curNode ) const
{
    uint32 errors = 0;
    // Persistence version number
    rcValue version( rcXMLFileVersion );
    rcXMLElement versionElem( eXMLElementVersion, version.stringValue() );
    tree.append_child( curNode, versionElem );

    return errors;
}

