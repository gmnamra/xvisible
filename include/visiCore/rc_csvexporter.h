/******************************************************************************
*	rc_csvexporter.h
*
*	This file contains the declaration for the implementation of
*	the export handler for CSV file formats.
******************************************************************************/

#ifndef rcCSVEXPORTER_H
#define rcCSVEXPORTER_H

#include <string.h>
#include <rc_types.h>

#include "rc_exporter.h"

class rcCSVExporter : public rcExporter
{
public:
    // Create a CSV exporter to export data to the designated file.
	rcCSVExporter( const char* fileName,
                   rcExperiment* experiment,
                   const char* comment,
                   rcProgressIndicator* progress );
    // Virtual dtor is required
    virtual ~rcCSVExporter();
    
	// Export an experiment to the designated file
	virtual int exportExperiment();

  private:
    // Export a range of tracks
    int exportTracks( ofstream& outfile,
                      uint32 firstTrack, uint32 lastTrack,
                      const vector<rcTimestamp> times,
                      uint32& cellCount );
};

#endif // rcCSVEXPORTER_H
