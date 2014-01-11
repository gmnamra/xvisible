/******************************************************************************
* @file  Copyright (c) 2003 Reify Corp. All Rights reserved.
*
*	$Id: rc_writermanager.cpp 6917 2009-07-02 14:38:33Z arman $
*
*	This file contains the implementation for engine writer manager class.
*
******************************************************************************/

#include <rc_writermanager.h>
#include <rc_xml.h>

#include "rc_writermanagerimpl.h"

// TODO: get XML tag names from XML name mapper or DTD

// Note: each entry in the array must have a unique SemanticType, XMLTag and DisplayName.
// Once an XML tag has been chosen, it can never be changed. Name, description and format can be changed.
// @note ADD_Track

static const rcWriterInfo sWriterInfoArray[] = {
    //
    //            SemanticType  DataType      XMLTag   DisplayName  Description         FormatString
    //
    rcWriterInfo( eWriterVideo, eVideoWriter, "video", "Raw Video", "Raw Video Frames", "%i" ),
    rcWriterInfo( eWriterACI, eScalarWriter, "change-index", "Aggregate Change Index", "Aggregate Change Index", "%-8.8f" ),
    rcWriterInfo( eWriterACIWindow, eScalarWriter,"sliding-change-index", "Aggregate Change Index Short Term", "Aggregate Change Index Short Term", "%-8.8f" ),
    rcWriterInfo( eWriterACIPeriod, eScalarWriter, "Periodicity", "ACI Periodicity", "ACI Periodicity", "%-8.8f" ),
    rcWriterInfo( eWriterLuminanceSum, eScalarWriter, "Luminance", "Luminance Sum", "Luminance Sum", "%-8.8f" ),	
    rcWriterInfo( eWriterCellCount, eScalarWriter, "locomotive-body-count", "Locomotive Body Count", "Total Locomotive Body Count", "%-4.0f" ),
    rcWriterInfo( eWriterBodyPosition, ePositionWriter, "locomotive-body-position", "Locomotive Body Position", "Locomotive Body Position (x y)", "(%-4.0f, %-4.0f)" ),
    rcWriterInfo( eWriterBodySpeedMean, eScalarWriter, "aggregate-locomotive-body-speed-mean", "Average Locomotive Body Speed", "Average Locomotive Body Speed (pixels/s)", "%-8.2f" ),
    rcWriterInfo( eWriterBodySpeedMin, eScalarWriter, "aggregate-locomotive-body-speed-min", "Locomotive Body Min Speed", "Locomotive Body Min Speed (pixels/s)", "%-8.2f" ),
    rcWriterInfo( eWriterBodySpeedMax, eScalarWriter, "aggregate-locomotive-body-speed-max", "Locomotive Body Max Speed", "Locomotive Body Max Speed (pixels/s)", "%-8.2f" ),
    rcWriterInfo( eWriterBodySpeedStdDev, eScalarWriter, "aggregate-locomotive-body-speed-std-dev", "Locomotive Body Speed Standard Deviation", "Locomotive Body Speed Standard Deviation (pixels/s)", "%-8.8f" ),
    rcWriterInfo( eWriterBodySpeedDirection, ePositionWriter, "locomotive-body-speed-direction", "Locomotive Body Speed and Direction", "Locomotive Body Speed (pixels/s) and Direction (degrees)", "(%-4.0f, %-4.0f)" ),
    rcWriterInfo( eWriterBodyVelocity, ePositionWriter, "locomotive-body-velocity", "Locomotive Body Velocity", "Locomotive Body Velocity Vector (x y) pixels/s", "(%-9.0f, %-9.0f)" ),
    rcWriterInfo( eWriterBodyMeanSquareDisplacements, eScalarWriter, "locomotive-body-mean-square-displacement", "Mean Square of Cell Displacements", "Mean Square of Cell Displacements", "%-8.2f" ),
    rcWriterInfo( eWriterBodyQuality, eScalarWriter, "locomotive-body-quality", "Locomotive Body Quality", "Locomotive Body Quality", "%-8.8f" ),
    rcWriterInfo( eWriterBodyDistance, eScalarWriter, "locomotive-body-distance", "Locomotive Body Distance Traveled", "Locomotive Body Distance Traveled (pixels)", "%-8.2f" ),
    rcWriterInfo( eWriterBodyPersistence, eScalarWriter, "locomotive-body-persistence", "Locomotive Body Persistence", "Locomotive Body Directional Persistence", "%-8.2f" ),
    rcWriterInfo( eWriterBodyState, eScalarWriter, "locomotive-body-state", "Locomotive Body State", "Locomotive Body State", "%0.f" ),
    rcWriterInfo( eWriterBodyBounds, ePositionWriter, "cell-boundaries", "Cell Boundaries", "Cell Boundaries", "%-8.2f" ),
    rcWriterInfo( eWriterBodyLength, eScalarWriter, "cell-length", "Cell Length", "Cell Length (pixels)", "%-8.3f" ),
    rcWriterInfo( eWriterACIPreview, eScalarWriter, "sliding-change-index-preview", "Aggregate Change Index Sliding Window Preview", "Aggregate Change Index - Sliding Window Preview", "%-8.8f" ),
    rcWriterInfo( eWriterMotionVector, eGraphicsWriter, "locomotion-vector-graphics", "Motion Vector Graphics", "Motion Vector Graphics", "%i" ),
    rcWriterInfo( eWriterBodyVector, eGraphicsWriter, "locomotive-body-graphics", "Locomotive Body Graphics", "Locomotive Body Graphics", "%i" ),
    rcWriterInfo( eWriterBodyHistoryVector, eGraphicsWriter, "locomotive-body-boundary-graphics", "Cell Boundary Graphics", "Cell Boundary Graphics", "%i" ),
    rcWriterInfo( eWriterBodyPersistenceMean, eScalarWriter, "aggregate-locomotive-body-persistence-mean", "Average Locomotive Body Persistence", "Average Locomotive Body Persistence", "%-8.5f" ),
    rcWriterInfo( eWriterBodyObjectPersistenceMean, eScalarWriter, "aggregate-locomotive-body-object-persistence-mean", "Average Locomotive Body-to-Object Persistence ", "Average Locomotive Body-to-Object Persistence", "%-8.5f" ),
    rcWriterInfo( eWriterBodyBodyPersistenceMean, eScalarWriter, "aggregate-locomotive-body-body-persistence-mean", "Average Locomotive Body-to-Body Persistence ", "Average Locomotive Body-to-Body Persistence", "%-8.5f" ),
    rcWriterInfo( eWriterTreatmentObjectCount, eScalarWriter, "treatment-object-count", "Treatment Object Count", "Treatment Object Count", "%-4.0f" ),
    rcWriterInfo( eWriterVideoDevelopment, eVideoWriter, "development-video", "Development Images", "Development Image Frames", "%i" ),
    rcWriterInfo( eWriterGraphicsDevelopment, eGraphicsWriter, "development-graphics", "Development Graphics", "Development Graphics", "%i" ),
    rcWriterInfo( eWriterBodyScaleX, eScalarWriter, "cell-axis-x", "Locomotive Body X Axis Scale", "Locomotive Body X Axis Scale Percentage", "%-3.1f%%" ),
    rcWriterInfo( eWriterBodyScaleY, eScalarWriter, "cell-axis-y", "Locomotive Body Y Axis Scale", "Locomotive Body Y Axis Scale Percentage", "%-3.1f%%" ),
    rcWriterInfo( eWriterBodyScaleXMean, eScalarWriter, "cell-axis-x-median", "Group Locomotive Body X Axis Scale", "Locomotive Body X Axis Scale Percentage", "%-3.1f%%" ),
    rcWriterInfo( eWriterBodyScaleYMean, eScalarWriter, "cell-axis-y-median", "Group Locomotive Body Y Axis Scale", "Locomotive Body Y Axis Scale Percentage", "%-3.1f%%" ),
    rcWriterInfo( eWriterBodyCircularity, eScalarWriter, "cell-circularity", "Locomotive Body Circularity", "Locomotive Body Circularity", "%-2.1f" ),
    rcWriterInfo( eWriterBodyEllipseRatio, eScalarWriter, "cell-EllipseRatio", "Locomotive Body EllipseRatio", "Locomotive Body EllipseRatio", "%-2.1f" ),
    rcWriterInfo( eWriterBodyCircularityMean, eScalarWriter, "cell-circularity-median", " Group Locomotive Body Circularity", "Locomotive Body Circularity", "%-2.1f" ),
    rcWriterInfo( eWriterBodyEllipseRatioMean, eScalarWriter, "cell-EllipseRatio-median", "Group Locomotive Body EllipseRatio", "Locomotive Body EllipseRatio", "%-2.1f" ),
    rcWriterInfo( eWriterBodyMajor, eScalarWriter, "cell-Major", "Locomotive Body Major Dimension", "Locomotive Body Major Dimension", "%-8.2f%" ),
    rcWriterInfo( eWriterCardiacShortening, eScalarWriter, "Shortening", "Shortening", "Cardiac Shortening", "%-3.2f%" ),
    rcWriterInfo( eWriterBodyMajorMean, eScalarWriter, "cell-Major-median", "Group Locomotive Body Major Dimension", "Locomotive Body Major Dimension", "%-8.2f%" ),
    rcWriterInfo( eWriterCardiacShorteningMean, eScalarWriter, "cell-shortenning-median", "Group Shortenning", "Group Shortenning", "%-3.2f%" ),

    rcWriterInfo( eWriterContractionFreq, eScalarWriter, "cfreq", "Contraction Frequency", "Contraction Frequency", "%-2.2f%" ),
    rcWriterInfo( eWriterContractionFreqMean, eScalarWriter, "cfreq-mean", "Mean Contraction Frequency", "Contraction Frequency", "%-2.2f%" ),

    rcWriterInfo( eWriterSegmentVector, eGraphicsWriter, "cell-segmentation-info", "Cell Segmentation Info", "Cell Segmentation", "%i" ),
    rcWriterInfo( eWriterPlotter, eGraphicsWriter, "Time Series Plot", "Time Series Plot", "Time Series Plot Visual Analysis", "%i" ),
    rcWriterInfo( eWriterTipDistance, eScalarWriter, "tip-distance", "Tip Distance Traveled", "Tip Distance Traveled (pixels)", "%-6.2f" ),    
};

// Writer name-type mapper instance
static const rcWriterMapper sWriterMapper( sWriterInfoArray, rmDim( sWriterInfoArray ) );

// public

rcWriterManager::rcWriterManager( rcEngineObserver* observer ) :
        mObserver( observer )
{
    rmAssert( observer != NULL );
}

 // Create a new writer group
rcWriterGroup* rcWriterManager::createWriterGroup( const char* name ,
                                                   const char* description,
                                                   rcGroupSemantics type )
{
    const rcXMLNameMapper* nameMapper = rfGetXMLNameMapper( rcXMLFileVersion );
    
    std::string groupTag = nameMapper->elementName( eXMLElementGroup );
    
    return mObserver->createWriterGroup( groupTag.c_str(),
                                         name,
                                         description,
                                         type );
}

// Create a new writer
rcWriter* rcWriterManager::createWriter( rcWriterGroup* group,
                                         rcWriterSemantics type, 
                                         uint32 sizeLimit,
                                         const rcRect& analysisRect )
{
    rmAssert( group != NULL );
    rcWriter* writer = 0;

    const rcWriterInfo info = sWriterMapper.info( type );

    if ( info.type() != eWriterUnknown ) {
        writer = group->createWriter( info.trackType(),
                                      info.tag().c_str(),
                                      info.name().c_str(),
                                      info.description().c_str(),
                                      info.format().c_str(),
                                      sizeLimit,
                                      analysisRect );
    }
    
    return writer;
}

// Create a new scalar writer wih expected min/max values
rcScalarWriter* rcWriterManager::createScalarWriter( rcWriterGroup* group,
                                                     rcWriterSemantics type, 
                                                     uint32 sizeLimit,
                                                     const rcRect& analysisRect,
                                                     const double& expectedMin,
                                                     const double& expectedMax )
{
    rmAssert( group != NULL );
    rcScalarWriter* writer = 0;

    const rcWriterInfo info = sWriterMapper.info( type );

    if ( info.type() != eWriterUnknown && info.trackType() == eScalarWriter ) {
        writer = dynamic_cast<rcScalarWriter*>(group->createWriter( info.trackType(),
                                                                    info.tag().c_str(),
                                                                    info.name().c_str(),
                                                                    info.description().c_str(),
                                                                    info.format().c_str(),
                                                                    sizeLimit,
                                                                    analysisRect ) );
        if ( writer ) {
            writer->setExpectedMinValue( expectedMin );
            writer->setExpectedMaxValue( expectedMax );
        }
    }
    
    return writer;
}

// Create a new position writer wih expected min/max values
rcPositionWriter* rcWriterManager::createPositionWriter( rcWriterGroup* group,
                                                         rcWriterSemantics type, 
                                                         uint32 sizeLimit,
                                                         const rcRect& analysisRect,
                                                         const rcFPair& expectedMin,
                                                         const rcFPair& expectedMax )
{
    rmAssert( group != NULL );
    rcPositionWriter* writer = 0;

    const rcWriterInfo info = sWriterMapper.info( type );

    if ( info.type() != eWriterUnknown && info.trackType() == ePositionWriter ) {
        writer = dynamic_cast<rcPositionWriter*>(group->createWriter( info.trackType(),
                                                                      info.tag().c_str(),
                                                                      info.name().c_str(),
                                                                      info.description().c_str(),
                                                                      info.format().c_str(),
                                                                      sizeLimit,
                                                                      analysisRect ) );
        if ( writer ) {
            writer->setExpectedMinValue( expectedMin );
            writer->setExpectedMaxValue( expectedMax );
        }
    }
    
    return writer;
}

// Create a new graphics writer wih expected min/max values
rcGraphicsWriter* rcWriterManager::createGraphicsWriter( rcWriterGroup* group,
                                                         rcWriterSemantics type, 
                                                         uint32 sizeLimit,
                                                         const rcRect& analysisRect)
{
    rmAssert( group != NULL );
    rcGraphicsWriter* writer = 0;

    const rcWriterInfo info = sWriterMapper.info( type );

    if ( info.type() != eWriterUnknown && info.trackType() == eGraphicsWriter ) {
        writer = dynamic_cast<rcGraphicsWriter*>(group->createWriter( info.trackType(),
                                                                      info.tag().c_str(),
                                                                      info.name().c_str(),
                                                                      info.description().c_str(),
                                                                      info.format().c_str(),
                                                                      sizeLimit,
                                                                      analysisRect) );
    }
    
    return writer;
}

// Create a new video writer wih expected min/max values
rcVideoWriter* rcWriterManager::createVideoWriter( rcWriterGroup* group,
                                                   rcWriterSemantics type, 
                                                   uint32 sizeLimit,
                                                   const rcRect& analysisRect )
{
    rmAssert( group != NULL );
    rcVideoWriter* writer = 0;

    const rcWriterInfo info = sWriterMapper.info( type );

    if ( info.type() != eWriterUnknown && info.trackType() == eVideoWriter ) {
        writer = dynamic_cast<rcVideoWriter*>(group->createWriter( info.trackType(),
                                                                   info.tag().c_str(),
                                                                   info.name().c_str(),
                                                                   info.description().c_str(),
                                                                   info.format().c_str(),
                                                                   sizeLimit,
                                                                   analysisRect) );
    }
    
    return writer;
}

// Map writer type to name
std::string rcWriterManager::writerName( rcWriterSemantics type )
{
    const rcWriterInfo info = sWriterMapper.info( type );

    return info.name();
}

// Map writer type to XML tag
std::string rcWriterManager::writerTag( rcWriterSemantics type )
{
    const rcWriterInfo info = sWriterMapper.info( type );

    return info.tag();
}

// Map writer name to type
rcWriterSemantics rcWriterManager::nameType( const std::string& name )
{
    return sWriterMapper.type( name );
}

// Map writer tag to type
rcWriterSemantics rcWriterManager::tagType( const std::string& tag )
{
    return sWriterMapper.typeTag( tag );
}

// Get human-readable sliding window attribute string
std::string rcWriterManager::optionString( rcWriterSemantics type, 
                                        rcWriterWindowOrigin origin,
                                        uint32 windowSize )
{
    rmUnused( type );
    
    std::string options;
    char originStr = '#';
    
    switch ( origin ) {
        case eWindowOriginLeft:
            originStr = 'L';
            break;
        case eWindowOriginCenter:
            originStr = 'C';
            break;
        case eWindowOriginRight:
            originStr = 'R';
            break;
    }
    char buf[256];
    
    // Add window size and origin indicator to string
    snprintf( buf, rmDim(buf), " [%i%c]",
              windowSize, originStr );
    
    options += buf;

    return options;
}

// Get human-readable channel attribute string
std::string rcWriterManager::channelString( rcChannelConversion c )
{
    std::string str;
    std::string channel;
    
    switch ( c ) {
        case rcSelectGreen:
            channel = "Green";
            break;
        case rcSelectRed:
            channel = "Red";
            break;
        case rcSelectBlue:
            channel = "Blue";
            break;
        case rcSelectAverage:
            channel = "Avg";
            break;
        case rcSelectMax:
            channel = "Max";
            break;
        case rcSelectAll:
            channel = "All";
            break;
    }
    
    char buf[256];
    
    // Add window size and origin indicator to string
    snprintf( buf, rmDim(buf), " [%s]",
              channel.c_str() );
    
    str += buf;

    return str;
}


// Update track name and description to reflect entropy definition
void rcWriterManager::renameTrack( rcWriter* writer, std::string& name, std::string& desc)
{
    writer->setName ( name.c_str() );
    writer->setDescription ( desc.c_str() );
}

// Update track name and description to reflect entropy definition
void rcWriterManager::renameEntropyTrack( rcWriter* writer, rcSimilarator::rcEntropyDefinition def,
                                          rcPixel inputDepth, rcChannelConversion channelConversion )
{
    std::string name;
    
    switch (def) {
        case rcSimilarator::eACI:
            name = "Aggregate Change Index";
            break;
        case rcSimilarator::eVisualEntropy:
            name = "Mean Similarity Index";
            break;

        default:
            cerr <<  "Unknown Similarity Option " << def << endl;
            rmAssert( 0 );
    }

    if ( inputDepth == rcPixel32 ) {
        std::string channnelString = rcWriterManager::channelString( channelConversion );
        name += channnelString;
    }

    writer->setName ( name.c_str() );
    writer->setDescription ( name.c_str() );
}

// private

