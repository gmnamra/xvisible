/******************************************************************************
*	rc_csvexporter.cpp
*
*	This file contains the implementation of the export handler
*   for CSV file formats.
******************************************************************************/

#include <rc_setting.h>
#include <rc_model.h>

#include "rc_csvexporter.h"

#if WIN32
using namespace std;
#endif

// Utility to replace commas with spaces
static void stripCommas( std::string& str )
{
    int32 pos = 0;
    int32 size = int32(str.size());
    
    while ( pos >= 0 && pos < size ) {
        pos = str.find( ",", pos );
        if ( pos >= 0 ) {
            str.replace( pos, 1 ," " );
            ++pos;
        }
    }
}

rcCSVExporter::rcCSVExporter( const char* fileName,
                              rcExperiment* experiment,
                              const char* comment,
                              rcProgressIndicator* progress ) :
        rcExporter( fileName, eExperimentCSVFormat, experiment, comment, progress )
{
}

rcCSVExporter::~rcCSVExporter()
{
}

int rcCSVExporter::exportExperiment( )
{
    int err = 0;
    
    int i, j;
    ofstream outfile( mFileName.c_str() );

    // Set a large output stream buffer for improved speed
    char streamBuf[65536*2];
    
    outfile.rdbuf()->pubsetbuf( streamBuf, sizeof( streamBuf ) );
    
     if ( outfile.fail() ) 
        err = EIO;
    if ( outfile.bad() )
        err = EIO;

    if ( mProgress )
        mProgress->progress( 0.0 );
    
    rcTimestamp startTime = rcTimestamp::now();
    rcExperimentDomain* domain = rcExperimentDomainFactory::getExperimentDomain();
    rcTimestamp experimentStart = domain->getExperimentStart();
    
    // output generator info
    std::string comment( "Document generator, " );
    comment += mExportGeneratorComment;
    outfile << comment << endl;

    // output settings at top of file
    int nCategories = mExperiment->getNSettingCategories();
    for (i = 0; i < nCategories; ++i)
    {
        rcSettingCategory category = mExperiment->getSettingCategory( i );
        int nSettings = category.getNSettings();
        for (j = 0; j < nSettings; ++j)
        {
            rcSettingInfo setting = category.getSettingInfo( j );
            
            // Persistability check
            if ( setting.isPersistable() ) {
                outfile << setting.getDisplayName() << ", "
                        << setting.getValue() << endl;
            }
        }
        outfile << endl;
    }

    std::string timeString = experimentStart.localtime();
    if ( timeString.size() )
        outfile << "Experiment video capture date, " << timeString << endl;

    // Iterate all valid timestamps
    vector<rcTimestamp> times( mTimes.size() );
    
    for ( uint32 i = 0; i < mTimes.size(); ++i ) {
        // Compute iterator start times from experiment start
        times[i] = mTimes[i] - experimentStart;
    }

    const uint32 trackCount = mExportTracks.size();
    // Maximum number of columns to export
    const uint32 maxColumns = 255;
    // Count of cells exported
    uint32 cellCount = 0;
    
    // Export tracks in consecutive ranges
    // TODO: dynamically adjustable range lengths so that tracks from the same
    // cell group do not get split into two separate ranges
    for ( uint32 firstTrack = 0; firstTrack < trackCount; firstTrack += maxColumns ) {
        uint32 lastTrack = firstTrack + maxColumns - 1;
        if ( lastTrack > (trackCount-1) )
            lastTrack = trackCount-1;
        if ( firstTrack <= lastTrack ) {
            // Export one range
            err += exportTracks( outfile, firstTrack, lastTrack, times, cellCount );
        }
    }
    // close the file and exit.
    outfile.close();

    rcTimestamp elapsedTime = rcTimestamp::now() - startTime;
    cout << cellCount << " CSV data cells exported to file in " << elapsedTime.secs() << " seconds" << endl;

    if ( outfile.fail() ) 
        err = EIO;
    if ( outfile.bad() ) 
        err = EIO;
    
    return err;
}

// Export a range of tracks
int rcCSVExporter::exportTracks( ofstream& outfile,
                                 uint32 firstTrack, uint32 lastTrack,
                                 const vector<rcTimestamp> times,
                                 uint32& cellCount )
{
    int err = 0;
    uint32 i;

    outfile << "Data tracks, [" << firstTrack << "-" << lastTrack << "]" << endl;
            
    // output group info
    outfile << "Measurement group name ";
    for (i = firstTrack; i <= lastTrack; ++i)
    {
        rcTrackGroup* group = mExperiment->getTrackGroup( mExportIteratorGroups[i] );
        std::string name( group->getName() );
        stripCommas( name );
        outfile << ", " << name;
    }
    outfile << endl;
    
    outfile << "Measurement group description ";
    for (i = firstTrack; i <= lastTrack; ++i)
    {
        rcTrackGroup* group = mExperiment->getTrackGroup( mExportIteratorGroups[i] );
        std::string descr( group->getDescription() );
        stripCommas( descr );
        outfile << ", " << descr;
    }
    outfile << endl;

    // output a line with the descriptions of the tracks
    outfile << "Frame / Time in seconds";
#ifdef ANALYSIS_AREA_PER_FRAME       
    outfile << ", Analysis area [x y w h]";
#endif
    
    for (i = firstTrack; i <= lastTrack; ++i)
    {
        rcTrack* track = mExportTracks[ i ];
        outfile << ", " << track->getDescription();
    }
    outfile << endl;

    // Iterate all the active tracks
    for ( uint32 a = 0; a < times.size(); ++a ) {
        // Init all iterators with the correct start time
        init( times[a] );
        for ( uint32 i = firstTrack; i <= lastTrack; ++i)
        {
            rcSegmentIterator* iter = mExportIterators[ i ];
            
            if ( i == firstTrack ) {
                // Output time as first column
	      outfile << times[a].secs();
	      //                outfile << a+1 << " / " <<  times[a].secs();
#ifdef ANALYSIS_AREA_PER_FRAME
                // output current focus area
                rcValue focus( iter->getFocus() );
                outfile << ", " << focus;
#endif
            } 
            outfile << ", ";
            bool hasValue = iter->contains( times[a] );
            if ( hasValue ) {
                if ( !produceValue( outfile, iter,  mExportTracks[i]->getTrackType(), eExperimentCSVFormat ) ) {
                    fprintf( stderr, "rcCSVExporter error: produceValue() failed\n" );
                }
            } else {
                // No defined value at this time stamp
                outfile << " ";
            }
            
            if ( mProgress && !(cellCount % 256)) {
                mProgress->progress( 100.0*cellCount/mValuesToExport );
            }
            ++cellCount;
        }
        outfile << endl;
    }

    return err;
}
