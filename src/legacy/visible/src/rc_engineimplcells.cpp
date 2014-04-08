/******************************************************************************
*   @file Copyright (c) 2003 Reify Corp. All Rights reserved.
*
*	$Id: rc_engineimplcells.cpp 6910 2009-06-30 03:42:07Z arman $
*
*	This file contains the implementation for cell result/track management
*
******************************************************************************/

#include <rc_engineimplbase.h>

#if 1
//
// rcCellWriterGroup class implementation
//

rcCellWriterGroup::rcCellWriterGroup() :
        mCellGroup( 0 ), mKey( 0 )
{
}

// @note ADD_Track

rcCellWriterGroup::rcCellWriterGroup( rcWriterManager* manager, const rcVisualFunction& bfun,
                                      const rcRect& imageRect, const rcRect& analysisRect,
                                      const rcTimestamp& trackStart,
                                      const char* label) :
  mKey( bfun.id())
{
  double maxDist = sqrt(double(imageRect.width()) * imageRect.width() + imageRect.height() * imageRect.height());
  uint32 maxSpeed = (uint32) log2 (maxDist);

    // Create cell group
    mCellGroup = manager->createWriterGroup( label,
                                             label,
                                             eGroupSemanticsBodyMeasurements );
    // Create writers Based on Biological Function Type
    if (bfun.isCardiacCell ())
      {

	// Cell center position writer
	rcPositionWriter* pWriter = manager->createPositionWriter( mCellGroup,
								   eWriterBodyPosition,
								   INT_MAX,
								   analysisRect,
								   rcFPair( imageRect.x(), imageRect.y() ) ,
								   rcFPair( imageRect.width(), imageRect.height() ) );
	rcWriter* writer = pWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyPosition, writer) );

	// Cell Major axis scale writer, value is [10, 200]
	rcScalarWriter * sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterBodyMajor,
					       INT_MAX,
					       analysisRect,
					       10.0,
					       200.0 );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyMajor, writer) );

	sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterCardiacShortening,
					       INT_MAX,
					       analysisRect,
					       0.01,
					       1.0 );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterCardiacShortening, writer) );

	// Cell contratcion frequency writer (is being used for shortening) , value is [0.1 to 10]
	sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterContractionFreq,
					       INT_MAX,
					       analysisRect,
					       0.01,
					       10.0 );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterContractionFreq, writer) );

      }
    else
      {

	// Cell center position writer
	rcPositionWriter* pWriter = manager->createPositionWriter( mCellGroup,
								   eWriterBodyPosition,
								   INT_MAX,
								   analysisRect,
								   rcFPair( imageRect.x(), imageRect.y() ) ,
								   rcFPair( imageRect.width(), imageRect.height() ) );
	rcWriter* writer = pWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyPosition, writer) );

	// Cell speed+direction writer
	pWriter = manager->createPositionWriter( mCellGroup,
						 eWriterBodySpeedDirection,
						 INT_MAX,
						 analysisRect,
						 rcFPair( 0.0, 0.0 ),
						 rcFPair( maxSpeed, 360.0 ) );
	writer = pWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodySpeedDirection, writer) );
    
	// Cell quality writer
	rcScalarWriter * sWriter = manager->createScalarWriter( mCellGroup,
								eWriterBodyQuality,
								INT_MAX,
								analysisRect,
								0.0,
								1.0 );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	writer->setExportable( true ); 
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyQuality, writer) );

	// Cell travel distance writer

	sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterBodyDistance,
					       INT_MAX,
					       analysisRect,
					       0.0,
					       maxDist );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyDistance, writer) );

	// Cell persistence writer, value is [-1-1]
	sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterBodyPersistence,
					       INT_MAX,
					       analysisRect,
					       -1.0,
					       1.0 );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyPersistence, writer) );

	// Cell X axis scale writer, value is [0.1-10]
	sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterBodyScaleX,
					       INT_MAX,
					       analysisRect,
					       50.0,
					       200.0 );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyScaleX, writer) );

	// Cell Y axis scale writer, value is [0.1-10]
	sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterBodyScaleY,
					       INT_MAX,
					       analysisRect,
					       50.0,
					       200.0 );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyScaleY, writer) );

	// Cell Circularity writer, value is [0.1-10]
	sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterBodyCircularity,
					       INT_MAX,
					       analysisRect,
					       0.0,
					       1.0 );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyCircularity, writer) );

	// Cell EllipseRatio writer, value is [0.1-10]
	sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterBodyEllipseRatio,
					       INT_MAX,
					       analysisRect,
					       0.1,
					       10.0);
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyEllipseRatio, writer) );


    
	// Cell state writer, value is an enum
	// @todo real min / max
	sWriter = manager->createScalarWriter( mCellGroup,
					       eWriterBodyState,
					       INT_MAX,
					       analysisRect,
					       0.0,
					       20.0 );
	writer = sWriter->down_cast();
	writer->setTrackStart( trackStart );
	// Enable export when state values have been implemented
	writer->setExportable(true);
	mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyState, writer) );
      }

}

rcCellWriterGroup::~rcCellWriterGroup()
{
    // No need to delete writers, observer does it
}

uint32 rcCellWriterGroup::size() const
{
    return mWriters.size();
}
    
rcWriter* rcCellWriterGroup::getWriter( rcWriterSemantics type ) const
{
    map<rcWriterSemantics, rcWriter*>::const_iterator w = mWriters.find( type );

    if ( w != mWriters.end() )
        return w->second;
    else
        return 0;
}




void rcCellWriterGroup::writeValues( const rcTimestamp& timestamp, const rcRect& focus, const rcVisualFunction& result )
{
#ifdef NOIMP
    // Write position value
  rcFPair pos (result.position().x(), result.position().y());
    rcWriter* writer = getWriter( eWriterBodyPosition );
    if ( writer ) {
        rcPositionWriter* pWriter = dynamic_cast<rcPositionWriter*>( writer );
        pWriter->writeValue( timestamp, focus, pos);
    }
    // Write velocity value
    rcFPair vel (result.velocity().x(), result.velocity().y());
    writer = getWriter( eWriterBodyVelocity );
    if ( writer ) {
        rcPositionWriter* pWriter = dynamic_cast<rcPositionWriter*>( writer );
        pWriter->writeValue( timestamp, focus, vel);
    }
    // Write speed+direction value
    writer = getWriter( eWriterBodySpeedDirection );
    if ( writer ) {
        rcPositionWriter* pWriter = dynamic_cast<rcPositionWriter*>( writer );
        const rc2Fvector v( result.velocity().x(), result.velocity().y() );
        // Transform velocity vector to speed+angle
        if ( !v.isNull() ) {
            rcDegree degAngle(rcRadian( v.angle() ));
            const float angle = degAngle.norm().Double();
            pWriter->writeValue( timestamp, focus, rcFPair(v.len(), angle) );
        } else {
            pWriter->writeValue( timestamp, focus, rcFPair(v.x(), v.y()) );
        }
    }
    // Write quality measure
    writer = getWriter( eWriterBodyQuality );
    if ( writer ) {
        rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        double v = result.rmsQuality();
        sWriter->writeValue( timestamp, focus, v );
    }
    // Write distance traveled
    writer = getWriter( eWriterBodyDistance );
    if ( writer ) {
        rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        double v = result.distance();
        sWriter->writeValue( timestamp, focus, v );
    }

    // Write persistence
    writer = getWriter( eWriterBodyPersistence );
    if ( writer ) {
        rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        double v = result.persistence();
        sWriter->writeValue( timestamp, focus, v );
    }
    
    // Write state
    writer = getWriter( eWriterBodyState );
    if ( writer ) {
        rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        // Enum to double
        double v = static_cast<double>(result.state());
        sWriter->writeValue( timestamp, focus, v );
    }

    // Write scale X 
    const rcFPair& scale = result.scale();
    if ( scale.x() > 0.0 ) {
    	writer = getWriter( eWriterBodyScaleX );
    	if ( writer ) {
        	rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        	// Enum to double
        	double v = static_cast<double>(scale.x());
        	sWriter->writeValue( timestamp, focus, v );
    	}
    }
    
    // Write scale Y
    if ( scale.y() > 0.0 ) {
    	writer = getWriter( eWriterBodyScaleY );
    	if ( writer ) {
        	rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        	// Enum to double
        	double v = static_cast<double>(scale.y());
        	sWriter->writeValue( timestamp, focus, v );
    	}
    }

    // Write Circularity
    if ( result.circularity() > 0.0 ) {
    	writer = getWriter( eWriterBodyCircularity);
    	if ( writer ) {
        	rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        	// Enum to double
        	double v = static_cast<double>(result.circularity ());
        	sWriter->writeValue( timestamp, focus, v );
    	}
    }
    
    // Write ellipseRatio
    if ( result.ellipseRatio() > 0.0 ) {
    	writer = getWriter( eWriterBodyEllipseRatio);
    	if ( writer ) {
        	rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        	// Enum to double
        	double v = static_cast<double>(result.ellipseRatio());
        	sWriter->writeValue( timestamp, focus, v );
    	}
    }


    // Write Major
    const rcFPair& dimension = result.dimensions ();
    if ( dimension.x() > 0.0 ) {
    	writer = getWriter( eWriterBodyMajor );
    	if ( writer ) {
        	rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        	// Enum to double
        	double v = static_cast<double>(dimension.x());
        	sWriter->writeValue( timestamp, focus, v );
    	}
    }
    
    const float shortn = result.shortenning ();
    if ( shortn >= 0.0 ) {
    	writer = getWriter( eWriterCardiacShortening);
    	if ( writer ) {
        	rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
        	// Enum to double
        	double v = static_cast<double>(shortn);
        	sWriter->writeValue( timestamp, focus, v );
    	}
    }

    // Contraction Frequency writer
    const float cfreq = result.contractionFrequency ();
    if ( cfreq > 0.0 ) {
      writer = getWriter( eWriterContractionFreq);
      if ( writer ) {
	rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
	// Enum to double
	double v = static_cast<double>(cfreq);
	sWriter->writeValue( timestamp, focus, v );
      }
    }
#endif

}

void rcCellWriterGroup::flush()
{
    map<rcWriterSemantics, rcWriter*>::iterator w;

    for ( w = mWriters.begin(); w != mWriters.end(); ++w )
        w->second->flush();
}

//
// rcCellWriterGroupCollection class implementation
//

rcCellWriterGroupCollection::rcCellWriterGroupCollection( rcWriterManager* manager,
                                                          const rcRect& imageRect,
                                                          const rcRect& analysisRect) :
        mManager( manager ), mImageRect( imageRect ), mAnalysisRect( analysisRect )
{

}

bool rcCellWriterGroupCollection::hasCell( const rcCellKey& key ) const
{
    map<rcCellKey, rcCellWriterGroup>::const_iterator w = mCellWriters.find( key );

    if ( w != mCellWriters.end() )
        return true;
    else
        return false;
}

rcCellWriterGroup* rcCellWriterGroupCollection::getCell( const rcCellKey& key )
{
    map<rcCellKey, rcCellWriterGroup>::iterator w = mCellWriters.find( key );

    if ( w != mCellWriters.end() )
        return &w->second;
    else
        return 0;
}

uint32 rcCellWriterGroupCollection::size() const
{
    return mCellWriters.size();
}



void rcCellWriterGroupCollection::addCell( const rcVisualFunction& result,
                                           const rcTimestamp& trackStart)
{
    char buf[512];
    // Compose cell group using cell Id 
    // @note Cell Info Label is generated here
    snprintf( buf, rmDim(buf), "Cell %lu", result.id());
    
    rcCellWriterGroup cell( mManager, result, mImageRect,
                            mAnalysisRect, trackStart, buf);
    mCellWriters[result.id()] = cell;
}

void rcCellWriterGroupCollection::flush()
{
    map<rcCellKey, rcCellWriterGroup>::iterator w;
    for ( w = mCellWriters.begin(); w != mCellWriters.end(); ++w )
        w->second.flush();
}




rcCellWriterGroup::rcCellWriterGroup( rcWriterManager* manager, const rcVisualTarget& bfun,
																		 const rcRect& imageRect, const rcRect& analysisRect,
																		 const rcTimestamp& trackStart,
																		 const char* label) : mKey( bfun.id())
{
  double maxDist = sqrt(double(imageRect.width()) * imageRect.width() + imageRect.height() * imageRect.height());
  uint32 maxSpeed = (uint32) log2 (maxDist);
	
	// Create cell group
	mCellGroup = manager->createWriterGroup( label,
																					label,
																					eGroupSemanticsBodyMeasurements );
			
			// Cell center position writer
			rcPositionWriter* pWriter = manager->createPositionWriter( mCellGroup,
																																eWriterBodyPosition,
																																INT_MAX,
																																analysisRect,
																																rcFPair( imageRect.x(), imageRect.y() ) ,
																																rcFPair( imageRect.width(), imageRect.height() ) );
			rcWriter* writer = pWriter->down_cast();
			writer->setTrackStart( trackStart );
			mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyPosition, writer) );
			
			// Cell speed+direction writer
			pWriter = manager->createPositionWriter( mCellGroup,
																							eWriterBodySpeedDirection,
																							INT_MAX,
																							analysisRect,
																							rcFPair( 0.0, 0.0 ),
																							rcFPair( maxSpeed, 360.0 ) );
			writer = pWriter->down_cast();
			writer->setTrackStart( trackStart );
			mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodySpeedDirection, writer) );
			
			// Cell quality writer
			rcScalarWriter * sWriter = manager->createScalarWriter( mCellGroup,
																														 eWriterBodyQuality,
																														 INT_MAX,
																														 analysisRect,
																														 0.0,
																														 1.0 );
			writer = sWriter->down_cast();
			writer->setTrackStart( trackStart );
			writer->setExportable( true ); 
			mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyQuality, writer) );
			
			// Cell travel distance writer
			
			sWriter = manager->createScalarWriter( mCellGroup,
																						eWriterBodyDistance,
																						INT_MAX,
																						analysisRect,
																						0.0,
																						maxDist );
			writer = sWriter->down_cast();
			writer->setTrackStart( trackStart );
			mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyDistance, writer) );
			
			// Cell persistence writer, value is [-1-1]
			sWriter = manager->createScalarWriter( mCellGroup,
																						eWriterBodyPersistence,
																						INT_MAX,
																						analysisRect,
																						-1.0,
																						1.0 );
			writer = sWriter->down_cast();
			writer->setTrackStart( trackStart );
			mWriters.insert( pair<rcWriterSemantics, rcWriter*>(eWriterBodyPersistence, writer) );
			
	}
	




void rcCellWriterGroupCollection::addCell( const rcVisualTarget& result,
																					const rcTimestamp& trackStart)
{
	char buf[512];
	// Compose cell group using cell Id 
	// @note Cell Info Label is generated here
	snprintf( buf, rmDim(buf), "Target %lu", result.id());
	
	rcCellWriterGroup cell( mManager, result, mImageRect,
												 mAnalysisRect, trackStart, buf);
	mCellWriters[result.id()] = cell;
}



void rcCellWriterGroup::writeValues( const rcTimestamp& timestamp, const rcRect& focus, const rcVisualTarget& result )
{
	// Write position value
  rcFPair pos (result.position().x(), result.position().y());
	rcWriter* writer = getWriter( eWriterBodyPosition );
	if ( writer ) {
		rcPositionWriter* pWriter = dynamic_cast<rcPositionWriter*>( writer );
		pWriter->writeValue( timestamp, focus, pos);
	}
	// Write velocity value
	rcFPair vel (result.velocity().x(), result.velocity().y());
	writer = getWriter( eWriterBodyVelocity );
	if ( writer ) {
		rcPositionWriter* pWriter = dynamic_cast<rcPositionWriter*>( writer );
		pWriter->writeValue( timestamp, focus, vel);
	}
	// Write speed+direction value
	writer = getWriter( eWriterBodySpeedDirection );
	if ( writer ) {
		rcPositionWriter* pWriter = dynamic_cast<rcPositionWriter*>( writer );
		const rc2Fvector v( result.velocity().x(), result.velocity().y() );
		// Transform velocity vector to speed+angle
		if ( !v.isNull() ) {
			rcDegree degAngle(rcRadian( v.angle() ));
			const float angle = degAngle.norm().Double();
			pWriter->writeValue( timestamp, focus, rcFPair(v.len(), angle) );
		} else {
			pWriter->writeValue( timestamp, focus, rcFPair(v.x(), v.y()) );
		}
	}
	// Write quality measure
	writer = getWriter( eWriterBodyQuality );
	if ( writer ) {
		rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
		double v = result.rmsQuality();
		sWriter->writeValue( timestamp, focus, v );
	}
	// Write distance traveled
	writer = getWriter( eWriterBodyDistance );
	if ( writer ) {
		rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
		double v = result.distance();
		sWriter->writeValue( timestamp, focus, v );
	}
	
	// Write persistence
	writer = getWriter( eWriterBodyPersistence );
	if ( writer ) {
		rcScalarWriter* sWriter = dynamic_cast<rcScalarWriter*>( writer );
		double v = result.persistence();
		sWriter->writeValue( timestamp, focus, v );
	}
	
}

#endif
