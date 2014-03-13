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
#include <rc_stlplus.h>
#include <iostreamio.hpp>

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

class rcCSV2dExporter 
{
    rcCSV2dExporter (const char *filename, rcExperiment* experiment, const char* comment, 
                     rcProgressIndicator* progress );
    
    ~rcCSV2dExporter ();

    void operator () (const string& filef, std::deque<deque<double> >& smm)
    {

        
        ofstream output_stream(filef.c_str(), ios::trunc);
            // create and initialise the TextIO wrapper device
        oiotext output(output_stream);
            // now use the device
        uint32 i, j;
        for (i = 0; i < smm.size(); i++)
        {
            deque<double>::iterator ds = smm[i].begin();
            for (j = 0; j < smm.size() - 1; j++)
                 output << *ds++ << ",";
            output << *ds << endl;
         }
         output_stream.flush();  
    }
    
private:
    
    
};
#endif // rcCSVEXPORTER_H
