/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	rc_exporter.h
*
*	This file contains the declaration for the exporter base class.
*
******************************************************************************/

#ifndef rcEXPORTER_H
#define rcEXPORTER_H

#include <ostream>
#include <vector>
#include <string.h>

#include <rc_types.h>

class rcExperiment;
class rcTrack;
class rcScalarSegmentIterator;

class rcExporter
{
public:
    // ctor/dtor
	rcExporter( const char* fileName,
                rcExperimentFileFormat fileFormat,
                rcExperiment* experiment,
                const char* comment,
                rcProgressIndicator* progress );

    virtual ~rcExporter();

	// Export an experiment to the designated file
    // Returns errno
	virtual int exportExperiment() = 0;

protected:
    // Init all containers and iterators
    void init( rcTimestamp startTime );
    // Produce a single value to stream, return TRUE with success
    bool produceValue( ostream& stream, rcSegmentIterator* iter, rcTrackType type, rcExperimentFileFormat fileFormat );

    // Is track exportable in this file format
    bool isExportable( rcTrack* track ) const;
    
    const std::string                  mFileName;
    rcExperimentFileFormat          mFileFormat;
    rcExperiment*                   mExperiment;
    vector<uint32>                mExportGroups; // Indexes of groups to be exported
    vector<rcTrack*>                mExportTracks;
    vector<const char*>           mExportTrackFormatStrings;
    vector<rcSegmentIterator*>      mExportIterators;
    vector<uint32>                mExportIteratorGroups;   // The associated group of each iterator
    std::string                        mExportGeneratorComment; // Comment from generator (application)
    vector<bool>                    mExportActiveIterators;  // Flags for active iterators
    rcProgressIndicator*            mProgress;               // Progress indicator to be called periodically
    uint32                        mValuesToExport;         // Number of data values to export (used for progress)
    vector<rcTimestamp>             mTimes;                  // All timestamps
private:
    // Return true if group has exportable tracks
    bool hasExportableTracks( rcTrackGroup* group ) const;
    // Does track have valid segments
    bool hasValidSegments( rcTrack* track ) const;
};

#endif // rcEXPORTER_H
