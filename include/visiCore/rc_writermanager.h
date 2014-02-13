/******************************************************************************
*   @files Copyright (c) 2003 Reify Corp. All Rights reserved.
*
*	$Id: rc_writermanager.h 7172 2011-02-01 00:31:41Z arman $
*
*	This file contains the definitions for engine writer manager class.
*
******************************************************************************/

#ifndef _rcWRITER_MANAGER_H_
#define _rcWRITER_MANAGER_H_

#include <rc_engine.h>
#include <rc_ipconvert.h>
#include <rc_similarity.h>

#if WIN32
using namespace std;
#endif

/******************************************************************************
*	Forwards
******************************************************************************/

/******************************************************************************
*	Constants
******************************************************************************/

/******************************************************************************
*	Types
******************************************************************************/
// Writer semantics, what the data in the writer represents
// Note: always add new types to the end of the list (before eWriterUnknown) so we are backwards compatible


enum rcWriterSemantics
{
    eWriterVideo = 0,           // Raw video
    eWriterACI,                 // Aggregate change index
    eWriterACIWindow,           // Aggregate change index in sliding temporal window
    eWriterCellCount,           // Cell count
    eWriterMotionVector,        // Motion vector visualization
    eWriterBodyVector,          // Locomotive body vector visualization
    eWriterBodyHistoryVector,   // Locomotive body history vector visualization
    eWriterBodyPosition,        // Locomotive body position (x,u)
    eWriterBodySpeedMean,       // Locomotive mean body speed
    eWriterBodySpeedMin,        // Locomotive min body speed
    eWriterBodySpeedMax,        // Locomotive max body speed
    eWriterBodySpeedStdDev,     // Locomotive body speed standard deviation
    eWriterBodySpeedDirection,  // Locomotive body speed vector (speed,direction)
    eWriterBodyVelocity,        // Locomotive body velocity vector (x,y)
    eWriterBodyMeanSquareDisplacements, // Mean square of locomotive body displacements
    eWriterBodyQuality,         // Locomotive body quality measure
    eWriterBodyDistance,        // Locomotive body travel distance
    eWriterBodyPersistence,     // Locomotive body directional persistence
    eWriterBodyState,           // Locomotive body state (moving, dead, dividing etc.)
    eWriterBodyBounds,          // Cell bounds (edges)
    eWriterBodyLength,          // Cell length (pixels)
    eWriterACIPreview,          // Aggregate change index, camera preview
    eWriterBodyPersistenceMean, // Locomotive mean body persistence
    eWriterBodyObjectPersistenceMean, // Locomotive body mean cell-to-object persistence
    eWriterBodyBodyPersistenceMean,   // Locomotive body mean cell-to-cell persistence
    eWriterTreatmentObjectCount,      // Number of treatment objects
    eWriterVideoDevelopment,          // Video track for internal development and debugging
    eWriterGraphicsDevelopment,       // Graphics track for internal development and debugging
    eWriterBodyScaleX,                // Scale percentage of cell X axis
    eWriterBodyScaleY,                // Scale percentage of cell Y axis
    eWriterBodyScaleXMean,            // Mean scale percentage of cell X axis
    eWriterBodyScaleYMean,            // Mean scale percentage of cell Y axis
    eWriterBodyCircularity,           // Circularity
    eWriterBodyEllipseRatio,          // EllipseRatio
    eWriterBodyCircularityMean,       // Mean Circularity
    eWriterBodyEllipseRatioMean,      // Mean EllipseRatio
    eWriterBodyMajor,                // Major Dimension
    eWriterBodyMinor,                // Minor Dimension
    eWriterBodyMajorMean,            // Mean Major Dimension
    eWriterBodyMinorMean,            // Mean Minor Dimension
    eWriterSegmentVector,             // Cell Segmentation Info
    eWriterPlotter,                   // Time Series information
    eWriterACIPeriod,
    eWriterContraction,
    eWriterContractionFreq,           // Contraction Frequency
    eWriterContractionFreqMean,       // Mean Contraction Frequency
    eWriterCardiacShortening,           // Cardiac Shortening
    eWriterCardiacShorteningMean,       // Mean Cardiac Shortening
    eWriterUnknown                    // Sentinel after last valid value
    // @note ADD_Track
};

// Sliding window origin
enum rcWriterWindowOrigin
{
    eWindowOriginLeft = 0,
    eWindowOriginCenter,
    eWindowOriginRight
};

/******************************************************************************
*	rcWriterManager class definition
*
******************************************************************************/

class rcWriterManager {
  public:
    // ctor
    rcWriterManager( rcEngineObserver* observer );

    // Create a new writer group
	rcWriterGroup* createWriterGroup( const char* name ,
                                      const char* description,
                                      rcGroupSemantics type );

    // Create a new writer of any type
    virtual rcWriter* createWriter( rcWriterGroup* group,
                                    rcWriterSemantics type,
                                    uint32 sizeLimit,
                                    const rcRect& analysisRect );

    // Create a new scalar writer wih expected min/max values
    virtual rcScalarWriter* createScalarWriter( rcWriterGroup* group,
                                                rcWriterSemantics type,
                                                uint32 sizeLimit,
                                                const rcRect& analysisRect,
                                                const double& expectedMin,
                                                const double& expectedMax );

    // Create a new position writer wih expected min/max values
    virtual rcPositionWriter* createPositionWriter( rcWriterGroup* group,
                                                    rcWriterSemantics type,
                                                    uint32 sizeLimit,
                                                    const rcRect& analysisRect,
                                                    const rcFPair& expectedMin,
                                                    const rcFPair& expectedMax );

    // Create a new graphics writer
    virtual rcGraphicsWriter* createGraphicsWriter( rcWriterGroup* group,
                                                    rcWriterSemantics type,
                                                    uint32 sizeLimit,
                                                    const rcRect& analysisRect );

     // Create a new video writer wih expected min/max values
    virtual rcVideoWriter* createVideoWriter( rcWriterGroup* group,
                                              rcWriterSemantics type,
                                              uint32 sizeLimit,
                                              const rcRect& analysisRect );

    // Static utility methods

    // Map writer type to name
    static std::string writerName( rcWriterSemantics id );
    // Map writer type to XML tag
    static std::string writerTag( rcWriterSemantics id );
    // Map writer name to type
    static rcWriterSemantics nameType( const std::string& name );
    // Map writer tag to type
    static rcWriterSemantics tagType( const std::string& tag );

    // Get human-readable sliding window attribute string
    static std::string optionString( rcWriterSemantics type,
                                  rcWriterWindowOrigin origin,
                                  uint32 windowSize );
    // Get human-readable channel attribute string
    static std::string channelString( rcChannelConversion c );

    // Update track name and description to reflect entropy definition
    static void renameEntropyTrack( rcWriter* writer, rcSimilarator::rcEntropyDefinition def,
                                    rcPixel inputDepth, rcChannelConversion channelConversion );
  //Update a tracks name and description
  static void renameTrack( rcWriter* writer, std::string& name, std::string& desc);


  private:
    rcEngineObserver* mObserver;
};

#endif // _rcWRITER_MANAGER_H_
