/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	rc_nativeexporter.h
*
*	This file contains the declaration for the implementation of
*	the export handler for native file formats.
******************************************************************************/

#ifndef _rcNATIVEEXPORTER_H_
#define _rcNATIVEEXPORTER_H_

#include <fstream>

#include <string.h>
#include <rc_types.h>

#include "rc_exporter.h"
#include "rc_xml.h"

//
// Native file format exporter class
//

class rcNativeExporter : public rcExporter
{
public:
    // Create a native exporter to export data to the designated file.
	rcNativeExporter( const char* fileName,
                      rcExperiment* experiment,
                      rcExperimentFileFormat format,
                      const char* comment,
                      rcProgressIndicator* progress );
    // Virtual dtor is required
    virtual ~rcNativeExporter();
    
	// Export an experiment to the designated file
	virtual int exportExperiment();
    
    // Returns XML declaration
    const std::string& xmlDeclaration() const;

  private:
    // XML tree mutators
    
    // Produce all setting elements, return number of errors
    uint32 produceSettings( rcXMLElementTree& tree, const rcXMLElementTree::iterator& cur ) const;
    // Produce all group elements, return number of errors
    uint32 produceGroups( rcXMLElementTree& tree, const rcXMLElementTree::iterator& cur ) const;
    // Produce all track elements, return number of errors
    uint32 produceTracks( rcXMLElementTree& tree, const rcXMLElementTree::iterator& cur ) const;
    // Produce all time elements, return number of errors
    uint32 produceTimes( rcXMLElementTree& tree, const rcXMLElementTree::iterator& cur ) const;
    // Produce all analysis result elements, return number of errors
    uint32 produceResults( rcXMLElementTree& tree, const rcXMLElementTree::iterator& cur,
                             double completedPercentage, double maxPercentage );
    // Produce some setting elements, return number of errors
    uint32 produceSettingRange( uint32 min, uint32 max, rcXMLElementTree& tree, const rcXMLElementTree::iterator& cur ) const;
    // Produce element ID string
    const std::string produceIdString( rcXMLAttributeType type, uint32 id ) const;
    // Produce info preamble (generator info etc.)
    uint32 producePreamble( rcXMLElementTree& tree,
                              const rcXMLElementTree::iterator& curNode ) const;
    // Produce element count
    uint32 produceElementCount( ostream& stream,
                                  const rcXMLElementTree& tree ) const;
    // Produce file format version number
    uint32 produceVersion( rcXMLElementTree& tree,
                             const rcXMLElementTree::iterator& curNode ) const;
    
    // Output stream
    ofstream mOutputFile;

    const rcXMLNameMapper* mNameMapper;         // Name mapper for output file version
};

#endif // _rcNATIVEEXPORTER_H_
