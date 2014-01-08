/******************************************************************************
*   @file Copyright (c) 2003 Reify Corp. All Rights reserved.
*
*	$Id: rc_engineimplfocus.cpp 6917 2009-07-02 14:38:33Z arman $
*
*	This file contains the implementation for engine focus data class.
*
******************************************************************************/

#include <rc_engineimplbase.h>

// Constants

// TODO: implement group name manager
static const char* cMeasurementWriterGroupStrings[] = {
    "Global Measurements",       // Display name
    "Global Measurement Group",  // Description
};

//
//	rcEngineFocusData class implementation
//
// @note ADD_Track

rcEngineFocusData::rcEngineFocusData( rcWriterManager* writerManager, const rcRect& analysisRect, const rcRect& imageRect,
                                      uint32 imageCount, rcAnalysisMode analysisMode,
                                      uint32 sizeLimit, uint32 slidingWindowSize,
                                      rcAnalyzerResultOrigin slidingWindowOrigin, uint32& groupCount,
                                      uint32 maxSpeed, int treatmentObjectType, int cellType) :
        mWriterManager( writerManager ),
	mcShortnWriter (0),
	mcMeanShortnWriter (0),
        mCfreqWriter (0),
        mcMeanFreqWriter (0),
        mEnergyWriter( 0 ),
        mSlidingEnergyWriter( 0 ),
        mPeriodEnergyWriter( 0 ),
        mCellCountWriter( 0 ),
        mCellSpeedMeanWriter( 0 ),
        mCellPersistenceMeanWriter( 0 ),
        mCellObjectPersistenceMeanWriter( 0 ),
        mCellCellPersistenceMeanWriter( 0 ),
        mCellMeanSquareDisplacementWriter( 0 ),
        mCellAxisXMeanWriter( 0 ),
        mCellAxisYMeanWriter( 0 ),
        mCellMajorMeanWriter( 0 ),
        mCellMinorMeanWriter( 0 ),
        mTipDistanceWriter( 0 ),
        mDevelopmentVideoWriter( 0 ),
        mDevelopmentGraphicsWriter( 0 ),
        mCellWriters( 0 ),
        mCellLengthWriter( 0 ),
        mCellBoundsWriter( 0 ),
        mAnalysisRect( analysisRect ),
        mImageRect( imageRect ),
        mWriterGroup( 0 ),
        mMaxSpeed( maxSpeed )
{
    rmAssert( mWriterManager );
    rmUnused( imageCount );
    
    // Create default writer group
    std::string name = cMeasurementWriterGroupStrings[0];
    std::string descr = cMeasurementWriterGroupStrings[1];
    char buf[128];
    snprintf( buf, rmDim( buf ), " %i", ++groupCount );
    name += buf;
    snprintf( buf, rmDim( buf ), " area [%i %i %i %i]",
              analysisRect.x(), analysisRect.y(), analysisRect.width(), analysisRect.height() );
    descr += buf;
    mWriterGroup = mWriterManager->createWriterGroup( name.c_str(),
                                                      descr.c_str(),
                                                      eGroupSemanticsGlobalMeasurements );
// @note ADD_Track

    switch ( analysisMode ) {
        case eAnalysisACISlidingWindow:
        {
            rcWriterWindowOrigin origin = eWindowOriginLeft;
            
            switch ( slidingWindowOrigin ) {
                case eAnalyzerResultOriginLeft:
                    origin = eWindowOriginLeft;
                    break;
                case eAnalyzerResultOriginCenter:
                    origin = eWindowOriginCenter;
                    break;
                case eAnalyzerResultOriginRight:
                    origin = eWindowOriginRight;
                    break;
            }

            mSlidingEnergyWriter = mWriterManager->createScalarWriter( mWriterGroup,
                                                                       eWriterACIWindow,
                                                                       sizeLimit,
                                                                       analysisRect,
                                                                       0.0,   // expected min
                                                                       1.0 ); // expected max

            rcWriter* writer =  mSlidingEnergyWriter->down_cast();
            mGlobalWriters.push_back( writer );
            // Add a human-readable string listing window options to track name and description
            std::string optionString = rcWriterManager::optionString( eWriterACIWindow, origin, slidingWindowSize );
            std::string nameWithOptions = writer->getName() + optionString;
            std::string descrWithOptions = writer->getDescription() + optionString;
            writer->setName( nameWithOptions.c_str() );
            writer->setDescription( descrWithOptions.c_str() );

            mPeriodEnergyWriter = mWriterManager->createScalarWriter( mWriterGroup,
                                                                       eWriterACIPeriod,
                                                                       sizeLimit,
                                                                       analysisRect,
                                                                       0.0,   // expected min
                                                                       1.0 ); // expected max

            writer =  mPeriodEnergyWriter->down_cast();
            mGlobalWriters.push_back( writer );
            // Add a human-readable string listing window options to track name and description
	    optionString = rcWriterManager::optionString( eWriterACIPeriod, origin, slidingWindowSize );
            nameWithOptions = writer->getName() + optionString;
            descrWithOptions = writer->getDescription() + optionString;
            writer->setName( nameWithOptions.c_str() );
            writer->setDescription( descrWithOptions.c_str() );
        }
        break;

			case eAnalysisTemplateTracking:
        {
            mTipDistanceWriter = mWriterManager->createScalarWriter( mWriterGroup,
                                                                eWriterTipDistance,
                                                                sizeLimit,
                                                                analysisRect,
                                                                0.0,   // expected min
                                                                256.0 ); // expected max
            rcWriter* writer =  mTipDistanceWriter->down_cast();
            mGlobalWriters.push_back(writer);
            // Add a human-readable string listing window options to track name and description
            std::string nameWithOptions = writer->getName();
            std::string descrWithOptions = writer->getDescription();
            writer->setName( nameWithOptions.c_str() );
            writer->setDescription( descrWithOptions.c_str() );

            // TODO: add a human-readable string listing analysis options to track name and description
        }
				
        case eAnalysisACI:
        {
            mEnergyWriter = mWriterManager->createScalarWriter( mWriterGroup,
                                                                eWriterACI,
                                                                sizeLimit,
                                                                analysisRect,
                                                                0.0,   // expected min
                                                                1.0 ); // expected max
            rcWriter* writer =  mEnergyWriter->down_cast();
            mGlobalWriters.push_back(writer);
            mPeriodEnergyWriter = mWriterManager->createScalarWriter( mWriterGroup,
                                                                       eWriterACIPeriod,
                                                                       sizeLimit,
                                                                       analysisRect,
                                                                       0.0,   // expected min
                                                                       1.0 ); // expected max

            writer =  mPeriodEnergyWriter->down_cast();
            mGlobalWriters.push_back( writer );
            // Add a human-readable string listing window options to track name and description
            std::string nameWithOptions = writer->getName();
            std::string descrWithOptions = writer->getDescription();
            writer->setName( nameWithOptions.c_str() );
            writer->setDescription( descrWithOptions.c_str() );

            // TODO: add a human-readable string listing analysis options to track name and description
        }
        break;
        
            default:
            break;

#if 0
        case eAnalysisCellTracking:
        {
            // Writer for total cell count
            mCellCountWriter = mWriterManager->createScalarWriter( mWriterGroup,
                                                                   eWriterCellCount,
                                                                   sizeLimit,
                                                                   analysisRect,
                                                                   0.0,     // expected min
                                                                   300.0 ); // expected max
            mGlobalCellWriters.push_back( mCellCountWriter->down_cast() );

	    switch (cellType)
	      {
	      case cAnalysisCellGeneral:
	      case cAnalysisOrganismFluCluster:
	      case cAnalysisLabeledFluorescence:

		// Writer for square of cell displacements
		mCellMeanSquareDisplacementWriter = mWriterManager->createScalarWriter( mWriterGroup,
											eWriterBodyMeanSquareDisplacements,
											sizeLimit,
											analysisRect,
											0.0,
											20.0 );
		mGlobalCellWriters.push_back( mCellMeanSquareDisplacementWriter->down_cast() );
		// Writer for mean cell speed
		// TODO: get better min/max values from kinetoscope params
		mCellSpeedMeanWriter = mWriterManager->createScalarWriter( mWriterGroup,
									   eWriterBodySpeedMean,
									   sizeLimit,
									   analysisRect,
									   0.0,    // expected min
									   10.0 ); // expected max
		mGlobalCellWriters.push_back( mCellSpeedMeanWriter->down_cast() );

		mCellPersistenceMeanWriter = mWriterManager->createScalarWriter( mWriterGroup,
										 eWriterBodyPersistenceMean,
										 sizeLimit,
										 analysisRect,
										 -1.0,     // expected min
										 1.0 );    // expected max
		mGlobalCellWriters.push_back( mCellPersistenceMeanWriter->down_cast() );

		// Cell-to-cell persistence
		mCellCellPersistenceMeanWriter = mWriterManager->createScalarWriter( mWriterGroup,
										     eWriterBodyBodyPersistenceMean,
										     sizeLimit,
										     analysisRect,
										     -1.0,     // expected min
										     1.0 );    // expected max
		mGlobalCellWriters.push_back( mCellCellPersistenceMeanWriter->down_cast() );

		// Writer for mean X axis scale percentage
		// TODO: get better min/max values from kinetoscope params
		mCellAxisXMeanWriter = mWriterManager->createScalarWriter( mWriterGroup,
									   eWriterBodyScaleXMean,
									   sizeLimit,
									   analysisRect,
									   50.0,    // expected min
									   200.0 ); // expected max
		mGlobalCellWriters.push_back( mCellAxisXMeanWriter->down_cast() );

		// Writer for mean Y axis scale percentage
		// TODO: get better min/max values from kinetoscope params
		mCellAxisYMeanWriter = mWriterManager->createScalarWriter( mWriterGroup,
									   eWriterBodyScaleYMean,
									   sizeLimit,
									   analysisRect,
									   50.0,    // expected min
									   200.0 ); // expected max
		mGlobalCellWriters.push_back( mCellAxisYMeanWriter->down_cast() );


		// Circularity Ratio
		mCellCircMeanWriter = mWriterManager->createScalarWriter( mWriterGroup,
									  eWriterBodyCircularityMean,
									  sizeLimit,
									  analysisRect,
									  0.0,    // expected min
									  1.0 ); // expected max
		mGlobalCellWriters.push_back( mCellCircMeanWriter->down_cast() );

		// Ellipse Ratio
		mCellEllipseMeanWriter = mWriterManager->createScalarWriter( mWriterGroup,
									     eWriterBodyEllipseRatioMean,
									     sizeLimit,
									     analysisRect,
									     0.1,    // expected min
									     10.0 ); // expected max
		mGlobalCellWriters.push_back( mCellEllipseMeanWriter->down_cast() );

		break;

	      case cAnalysisCellMuscle:
	      case cAnalysisCellMuscleSelected:
	      case cAnalysisCellMuscleCalcium:
		{
	    
		  // TODO: get better min/max values from kinetoscope params
		  mCellMajorMeanWriter = mWriterManager->createScalarWriter( mWriterGroup,
									     eWriterBodyMajorMean,
									     sizeLimit,
									     analysisRect,
									     10.0,    // expected min
									     200.0 ); // expected max
		  mGlobalCellWriters.push_back( mCellMajorMeanWriter->down_cast() );

		  mcMeanShortnWriter = mWriterManager->createScalarWriter( mWriterGroup,
									     eWriterCardiacShorteningMean,
									     sizeLimit,
									     analysisRect,
									     0.01,    // expected min
									     1.0 ); // expected max
		  mGlobalCellWriters.push_back( mcMeanShortnWriter->down_cast() );

		  // TODO: get better min/max values from kinetoscope params
		  mcMeanFreqWriter = mWriterManager->createScalarWriter( mWriterGroup,
									     eWriterContractionFreqMean,
									     sizeLimit,
									     analysisRect,
									     0.1,    // expected min
									     10.0 ); // expected max
		  mGlobalCellWriters.push_back( mcMeanFreqWriter->down_cast() );

	    
		  break;
		}
	      }
            createCellWriters();
        }
        break;
#endif

    }

    // Developement/debugging writers
    mDevelopmentVideoWriter = mWriterManager->createVideoWriter( mWriterGroup,
                                                                 eWriterVideoDevelopment,
                                                                 sizeLimit,
                                                                 analysisRect );
    mGlobalWriters.push_back( mDevelopmentVideoWriter->down_cast() );
    mDevelopmentGraphicsWriter = mWriterManager->createGraphicsWriter( mWriterGroup,
                                                                       eWriterGraphicsDevelopment,
                                                                       sizeLimit,
                                                                       analysisRect );
    mGlobalWriters.push_back( mDevelopmentGraphicsWriter->down_cast() );

    // Plotting Graphics
    mPlotterWriter = mWriterManager->createGraphicsWriter( mWriterGroup,
							   eWriterPlotter,
							   sizeLimit,
							   analysisRect );
    mGlobalWriters.push_back( mPlotterWriter->down_cast() );

}

// Set global track start times
void rcEngineFocusData::setTrackStartTimes( rcTimestamp start )
{
    for ( uint32 i = 0; i < mGlobalWriters.size(); ++i ) {
        mGlobalWriters[i]->setTrackStart( start );
    }
}

// Set cell track start times
void rcEngineFocusData::setCellTrackStartTimes( rcTimestamp start )
{
     for ( uint32 i = 0; i < mGlobalCellWriters.size(); ++i ) {
        mGlobalCellWriters[i]->setTrackStart( start );
    }
}

#ifdef CELLS
// Create cell results writers
void rcEngineFocusData::createCellWriters( )
{
  mCellWriters = new rcCellWriterGroupCollection( mWriterManager, mImageRect, mAnalysisRect ); // Pass Cell Type
}
#endif

rcEngineFocusData::~rcEngineFocusData()
{
    // The writers will be freed by observer, just reset the pointers
    mWriterGroup = 0;
    
    mEnergyWriter = 0;
    mSlidingEnergyWriter = 0;
    mPeriodEnergyWriter = 0;
    mCellCountWriter = 0;
    mCellSpeedMeanWriter = 0;
    mCellPersistenceMeanWriter = 0;
    mCellObjectPersistenceMeanWriter = 0;
    mCellCellPersistenceMeanWriter = 0;
    mCellMeanSquareDisplacementWriter = 0;
    mCellBoundsWriter = 0;
    mCellLengthWriter = 0;
    mDevelopmentVideoWriter = 0;
    mDevelopmentGraphicsWriter = 0;
    mCellCircMeanWriter = 0;
    mCellEllipseMeanWriter = 0;
    mCellAxisYMeanWriter = 0;
    mCellAxisXMeanWriter = 0;
    
    mGlobalWriters.clear();
    mGlobalCellWriters.clear();

}

// Clear all result containers
void rcEngineFocusData::clearFocusData()
{
    mCellCountResults.clear();
    mTimeStamps.clear();
}

// Write gathered data to writer(s)
void rcEngineFocusData::writeFocusData( rcTimestamp currentTime, int32 i )
{
    if ( i >= 0 ) {
        // Locomotive Body count
        if ( mCellCountWriter != 0 ) {
            if ( uint32(i) < mCellCountResults.size() ) {
                mCellCountWriter->writeValue( currentTime, focusRect(), mCellCountResults[i] );
            }
        }
    } else {
        flushFocusData();
    }
}

// Flush all writers
void rcEngineFocusData::flushFocusData()
{
    for ( uint32 i = 0; i < mGlobalWriters.size(); ++i ) 
        mGlobalWriters[i]->flush();
    
    for ( uint32 i = 0; i < mGlobalCellWriters.size(); ++i ) 
        mGlobalCellWriters[i]->flush();
    
        //    if ( mCellWriters != 0 ) mCellWriters->flush();
}
