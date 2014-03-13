/*
 *
 * $Id: rc_persistencemanager.h 6565 2009-01-30 03:24:44Z arman $
 *
 *
 * Persistence management functionality
 *
 * Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 */

#ifndef _rcPERSISTENCEMANAGER_H_
#define _rcPERSISTENCEMANAGER_H_

#include <rc_types.h>
#include <ostream>

/******************************************************************************
*	Constants
******************************************************************************/


/******************************************************************************
*	Types
******************************************************************************/

//  rcExperimentAttributes struct definition

struct rcExperimentAttributes
{
    int         frameWidth;     // get the width of the input source frame
    int         frameHeight;    // get the height of the input source frame
    int         frameDepth;     // get the depth of input source frame, bytes per pixel
    bool        liveInput;      // returns true if the input source is real-time.
    bool        liveStorage;    // returns true if the input is being stored to disk
    std::string    fileName;       // file name of the experiment
    std::string    title;          // title of the experiment
    std::string    inputName;      // image input source name (typically filename)
    std::string    userName;       // experimenter name
    std::string	comments;       // comments  
    std::string	lensMag;        // microscope lens maginification
    std::string	otherMag;       // other maginification
    std::string    imagingMode;    // imaging mode    
    std::string    cellType;       // cell/organism type
    std::string    treatment1;     // chemical/physical treatment 1   
    std::string    treatment2;     // chemical/physical treatment 2 
    std::string    temperature;    // temperature in degrees C  
    std::string    CO2;            // CO2 level %  
    std::string    O2;             // O2 level %   

	friend std::ostream& operator<<(std::ostream&, rcExperimentAttributes& attr);

};

// export/import/save file formats
enum rcExperimentFileFormat
{
    eExperimentCSVFormat = 0        // Comma-separated-value format for Excel import
    , eExperimentNativeFormat         // Native experiment data XML format
  , eExperimentNativeMovieFormat    // Native movie file format
  , eExperimentQuickTimeMovieFormat // QuickTime movie file format
  , eExperimentQuickTimeImageFormat // QuickTime image file format
  , eExperimentMolDevSTKFormat      // Molecular Devices modified metamorph TIFF stack
  , eExperiment2DCSVFormat         // Native experiment data XML format    
  , eExperimentAllFormat            // Synthetic value, denotes all applicable formats
};

// Import operation mode
enum rcExperimentImportMode {
    eExperimentImportAllSettings = 0,        // Import application + experiment settings
    eExperimentImportExperimentSettings,     // Import experiment settings only
    eExperimentImportAllData,                // Import (measurement) data only
    eExperimentImportAll                     // Import all settings and all data
};

// Export operation mode
enum rcExperimentExportMode {
    eExperimentExportAllSettings = 0,        // Export application + experiment settings
    eExperimentExportExperimentSettings,     // Export experiment settings only
    eExperimentExportAllData,                // Export (measurement) data only
    eExperimentExportAll                     // Export all settings and all data
};

// Data track semantics, what the data in the track represents
enum rcTrackSemantics
{
    eTrackSemanticsUnknown = 0,         // Sentinel
    eTrackSemanticsVideo,               // Raw video writer
    eTrackSemanticsVideoBoundary,       // Cell boundary video/graphic writer
    eTrackSemanticsACI,                 // Aggregate change index
    eTrackSemanticsCellCount,           // Cell count
    eTrackSemanticsMotionVector,        // Motion vector visualization writer
    eTrackSemanticsBodyVector,          // Locomotive body vector visualization writer
    eTrackSemanticsBodyPosition,        // Locomotive body position (x,u) writer
    eTrackSemanticsBodySpeedMean,       // Locomotive mean body speed writer
    eTrackSemanticsBodySpeedMin,        // Locomotive min body speed writer
    eTrackSemanticsBodySpeedMax,        // Locomotive max body speed writer
    eTrackSemanticsBodySpeedStdDev,     // Locomotive body speed standard deviation writer
    eTrackSemanticsBodySpeedDirection,  // Locomotive body velocity vector writer
    eTrackSemanticsBodyState            // Locomotive body state writer
};

// Track group semantics
enum rcGroupSemantics
{
    eGroupSemanticsUnknown = 0,        // Sentinel
    eGroupSemanticsGlobalGraphics,     // Global graphics group
    eGroupSemanticsGlobalMeasurements, // Global measurements group
    eGroupSemanticsBodyMeasurements,   // Locomotive body group
    eGroupSemanticsCameraPreview       // Camera preview data group
};

/******************************************************************************
*	rcPersistenceManager interface definition.
*
*	This is the top-level persistence manager singleton object.
******************************************************************************/

class rcPersistenceManager
{
public:
    // virtual dtor is required
    virtual ~rcPersistenceManager() { };

    // Track type name
    //const std::string& trackTypeXMLName( rcTrackType type ) const = 0;

    // File format extension for export
    virtual const std::string fileFormatExportExtension( rcExperimentFileFormat format ) const = 0;
    // File format export filter for dialog boxes
    virtual const std::string fileFormatExportFilter( rcExperimentFileFormat format ) const = 0;
    // File format export caption for dialog boxes
    virtual const std::string fileFormatExportCaption( rcExperimentFileFormat format ) const = 0;
    
    // File format import filter for dialog boxes
    virtual const std::string fileFormatImportFilter( rcExperimentFileFormat format ) const = 0;
    // File format import caption for dialog boxes
    virtual const std::string fileFormatImportCaption( rcExperimentFileFormat format ) const = 0;

    // Set data generator comment (done once during startup)
    virtual void setGeneratorComment( const std::string& comment ) = 0;
    // Get data generator comment
    virtual const std::string generatorComment() = 0;

    // Set current data import directory
    virtual void setImportDirectory( const std::string& dir ) = 0;
    // Get current data import directory
    virtual const std::string getImportDirectory() = 0;
};

/******************************************************************************
*	rcPersistenceManagerFactory class definition
*
*	The rcPersistenceManagerFactory class consists of a single static
*	method to get the linked-in (statically or dynamically) implementation
*	of the rcPersistenceManager interface.
******************************************************************************/

class rcPersistenceManagerFactory
{
public:
	static rcPersistenceManager* getPersistenceManager( const char* implKey = 0 );
};

#endif // _rcPERSISTENCEMANAGER_H_
