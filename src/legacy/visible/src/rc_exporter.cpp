/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	$Id: rc_exporter.cpp 6561 2009-01-29 19:10:59Z arman $
*
*	This file contains the implementation of the export handler
*   base class.
******************************************************************************/

#include <rc_model.h>

#include "rc_exporter.h"
#include <rc_writermanager.h>

#if WIN32
using namespace std;
#endif

rcExporter::rcExporter( const char* fileName,
                        rcExperimentFileFormat fileFormat,
                        rcExperiment* experiment,
                        const char* comment,
                        rcProgressIndicator* progress ) :
        mFileName( fileName ),
        mFileFormat( fileFormat ),
        mExperiment( experiment ),
        mExportGeneratorComment( comment ),
        mProgress( progress ),
        mValuesToExport( 0 )
{
    init( cZeroTime );

    // Build a collection of all time stamps
    rcTimestamp current = cZeroTime;

    // Sorted set of timestamps
    set<rcTimestamp> timestamps;
    vector<rcTimestamp> trackOffsets( mExportTracks.size() );
    uint32 activeTracks = 0;
    
    // Precompute some values
    for ( uint32 i = 0; i <  mExportTracks.size(); ++i ) {
        // Track start offsets
        trackOffsets[i] = mExportTracks[i]->getTrackStart();
        if ( mExportActiveIterators[i] ) 
            ++activeTracks;
    }
    
    // Collect a list of all valid timestamps
    while ( activeTracks > 0 )
    {
        // Iterate all the active tracks
        for ( uint32 i = 0; i < mExportTracks.size(); ++i)
        {
            if ( mExportActiveIterators[i] ) {
                rcSegmentIterator* iter = mExportIterators[ i ];
                // Current time
                current = trackOffsets[i] + iter->getSegmentStart();
                timestamps.insert( current );
            }
        }

        // Advance each iterator
        for ( uint32 i = 0; i < mExportTracks.size(); ++i)
        {
            if ( mExportActiveIterators[i] ) {
                rcSegmentIterator* iter = mExportIterators[ i ];
                if ( ! iter->advance( 1 ) ) {
                    // This track is now empty, deactivate it
                    mExportActiveIterators[i] = false;
                    // Reset back to first segment
                    iter->reset();
                    --activeTracks;
                }
            }
        }
    }
    // Iterate all valid timestamps
    mTimes.reserve( timestamps.size() );
    set<rcTimestamp>::iterator ts;
    
    for ( ts = timestamps.begin(); ts != timestamps.end(); ++ts ) {
        // Compute iterator start times
        mTimes.push_back( *ts );
    }

    init( cZeroTime );
}

rcExporter::~rcExporter()
{
    if ( ! mExportIterators.empty() ) {
        // Delete all iterators
        vector<rcSegmentIterator*>::iterator i;
        for( i = mExportIterators.begin(); i < mExportIterators.end(); i++ )
            delete (*i);
    }
}

// protected

// Init all containers and iterators
void rcExporter::init( rcTimestamp startTime )
{
    if ( ! mExportIterators.empty() ) {
        // Delete all iterators
        vector<rcSegmentIterator*>::iterator i;
        for( i = mExportIterators.begin(); i < mExportIterators.end(); i++ )
            delete (*i);
    }

    mExportGroups.clear();
    mExportTracks.clear();
    mExportTrackFormatStrings.clear();
    mExportIterators.clear();
    mExportIteratorGroups.clear();
    mExportActiveIterators.clear();
    mValuesToExport = 0;
    uint32 maxSegments = 0;
    
    mExportIteratorGroups.reserve( mExperiment->getNTrackGroups() );
    mExportGroups.reserve( mExperiment->getNTrackGroups() );

    // TODO: gather the total number of exportable values in mValuesToExport for progress reporting
    for ( int32 i = 0; i < mExperiment->getNTrackGroups(); i++ )
    {
        rcTrackGroup* group = mExperiment->getTrackGroup( i );
        
        if ( hasExportableTracks( group ) ) {
            mExportGroups.push_back( i );

            uint32 nTracks = group->getNTracks();
            for ( uint32 j = 0; j < nTracks; j++ )
            {
                rcTrack* track = group->getTrack( j );
                rcTrackType type = track->getTrackType();
                rcSegmentIterator* iter = 0;
                
                if ( isExportable( track ) ) {
                    switch ( type ) {
                        case eScalarTrack:
                        {
                            rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );
                            iter = scalarTrack->getDataSegmentIterator( startTime );
                            // Add only non-empty tracks
                            mExportIterators.push_back( iter );
                        }
                        break;
                        
                        case ePositionTrack:
                        {
                            rcPositionTrack* positionTrack = dynamic_cast<rcPositionTrack*>( track );
                            iter = positionTrack->getDataSegmentIterator( startTime );
                            // Add only non-empty tracks
                            mExportIterators.push_back( iter );
                        }
                        break;

                        case eGraphicsTrack:
                        {
                            rcGraphicsTrack* graphicsTrack = dynamic_cast<rcGraphicsTrack*>( track );
                            iter = graphicsTrack->getDataSegmentIterator( startTime );
                            mExportIterators.push_back( iter );
                        }
                        break;
                        
                        default:
                            rmAssert( 0 ); // Unsupported track type
                            break;
                    }
                    if ( iter ) {
                        const uint32 segments = iter->getSegmentCount();
                        mValuesToExport += segments;
                        if ( segments > maxSegments )
                            maxSegments = segments;
                    }
                    // Each iteator is active by default
                    mExportActiveIterators.push_back( true );
                    // Store group id for each iterator
                    mExportIteratorGroups.push_back( i );
                            
                    // Tracks
                    mExportTracks.push_back( track );
                    // Display format strings
                    mExportTrackFormatStrings.push_back( track->getDisplayFormatString() );
                }
            }                
        }
    }

    if ( mFileFormat == eExperimentCSVFormat ) {
        // In CSV format we must produce a value for every time for every track
        mValuesToExport = mExportTracks.size() * maxSegments;
    }
}

// Round float to have 2 digits after decimal point
#define rmRound2( arg ) (int(((arg)*100.0+0.5))/100.0)
// Round float to have 3 digits after decimal point
#define rmRound3( arg ) (int(((arg)*1000.0+0.5))/1000.0)
// Round float to have 4 digits after decimal point
#define rmRound4( arg ) (int(((arg)*10000.0+0.5))/10000.0)

// Produce single value to stream, return true on success
bool rcExporter::produceValue( ostream& stream,
                               rcSegmentIterator* iter,
                               rcTrackType type,
                               rcExperimentFileFormat fileFormat )
{
    bool success = false;
    
    switch( type ) {
        case eScalarTrack:
        {
            rcScalarSegmentIterator* diter = dynamic_cast<rcScalarSegmentIterator*>(iter);
            if ( diter ) {
                // Output with high accuracy
                stream.precision( 12 );
                double v = diter->getValue();
                stream << v;
                success = true;
             }
         }
         break;
                    
         case ePositionTrack:
         {
             rcPositionSegmentIterator* diter = dynamic_cast<rcPositionSegmentIterator*>(iter);
             if ( diter ) {
                 stream.precision( 8 );
                 stream.setf( ios::fixed );
                 stream.unsetf( ios::showpoint );
                 stream.unsetf( ios::floatfield );

                 const rcFPair position = diter->getValue();
                 // Accuracy 4 digits after decimal point
                 const double x = rmRound4(position.x());
                 const double y = rmRound4(position.y());
                 
                 switch ( fileFormat ) {
                     case eExperimentNativeFormat:
                     {
                         stream << x << "," << y;
                         success = true;
                     }
                     break;
                         
                     case eExperimentCSVFormat:
                     {
                         // We cannot use a comma-separated pair, use
                         // a space between x and y coordinates
                         stream << x << " " << y;
                         success = true;
                     }
                     break;
                         
                     default:
                         rmAssert( 0 );
                         break;
                 }
             }
         }
         break;

        case eGraphicsTrack:
        {
            rcGraphicsSegmentIterator* diter = dynamic_cast<rcGraphicsSegmentIterator*>(iter);
            if ( diter ) {
                stream.precision( 8 );
                stream.setf( ios::fixed );
                stream.unsetf( ios::showpoint );
                stream.unsetf( ios::floatfield );
                const rcVisualGraphicsCollection c = diter->getValue();
                // First number is token count
                stream << c.size();
                
                const rcVisualSegmentCollection& lines = c.segments();
                rcVisualSegmentCollection::const_iterator line;
                // Iterate all graphics tokens
                for ( line = lines.begin(); line != lines.end(); line++ ) {
                    rcVisualSegment::rcVisualSegmentType type = line->type();
                    // Output type and four points
                    rc2Fvector p1 = line->p1();
                    rc2Fvector p2 = line->p2();
                    if ( type == rcVisualSegment::eStyle ) {
                        // Special case for graphics style tokens
                        const rcVisualStyle& s = static_cast<const rcVisualStyle&>(*line);
                        // Accuracy 2 digits after decimal point
                        // Output color as an int, 3 fields as floats
                        stream << "," << type << "," << s.color() << "," << rmRound2(p1.y()) << "," << rmRound2(p2.x()) << "," << rmRound2(p2.y());
                    } else {
                        // Accuracy 2 digits after decimal point
                        // Produce 4 floats for regular graphics tokens
                        stream << "," << type << "," << rmRound2(p1.x()) << "," << rmRound2(p1.y()) << "," << rmRound2(p2.x()) << "," << rmRound2(p2.y());
                    }
                }
                success = true;
            }
        }
        break;
        
        default:
            // Unsupported track type
            rmAssert( 0 );
            stream << " ";
            break;
    }
    
    return success;
}

// Is track exportable in this file format
bool rcExporter::isExportable( rcTrack* track ) const
{
    bool exportable = track->isExportable();

    if ( exportable ) {
        rcTrackType type = track->getTrackType();
        
        if ( mFileFormat == eExperimentCSVFormat ) {
            // Do not export graphics tracks in CSV format
            if ( type == eGraphicsTrack )
                exportable = false;
            else if ( type == ePositionTrack ) {
                // Disable speed+direction track value export to CSV
                if ( rcWriterManager::tagType( track->getTag() ) == eWriterBodySpeedDirection )
                    exportable = false;
            } 
        } 
#if 0
else if ( mFileFormat == eExperimentNativeGraphicsFormat ) {
            if ( type != eGraphicsTrack )
                exportable = false;
        }
#endif
    }

    // Check segment count
    if ( exportable )
        exportable = hasValidSegments( track );
    
    return exportable;
}

// private

// Return true if group has exportable tracks
bool rcExporter::hasExportableTracks( rcTrackGroup* group ) const
{
    bool exportable = false;
    
    uint32 nTracks = group->getNTracks();

    for ( uint32 j = 0; j < nTracks; j++ )
    {
        rcTrack* track = group->getTrack( j );
        
        if ( isExportable( track ) ) {
            exportable = true;
            break;
        }
    }

    return exportable;
}

// Does track have valid segments
bool rcExporter::hasValidSegments( rcTrack* track ) const
{
    uint32 count = 0;
    
    switch ( track->getTrackType() ) {
        case eScalarTrack:
        {
            rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );
            count = scalarTrack->getSegmentCount();
        }
        break;
        
        case ePositionTrack:
        {
            rcPositionTrack* positionTrack = dynamic_cast<rcPositionTrack*>( track );
            count = positionTrack->getSegmentCount();
        }
        break;
        
        case eGraphicsTrack:
        {
            rcGraphicsTrack* graphicsTrack = dynamic_cast<rcGraphicsTrack*>( track );
            count = graphicsTrack->getSegmentCount();
        }
        break;
        
        default:
            rmAssert( 0 ); // Unsupported track type
            break;
    }

    if ( count > 0 )
        return true;

    return false;
}
