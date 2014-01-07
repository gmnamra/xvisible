
#include <rc_thread.h>
#include <rc_model.h>

#include <rc_experimentdomainimpl.h>

#include "ut_model.h"

// Test constants

static const char* cVideoWriterStrings[] = {
    "test-video",          // XML tag name
    "Test Video",          // Display name
    "Test Video Frames",   // Description
    "%i"                   // Printf format string
};
static const uint32 cVideoWriterSizeLimit = 200;

static const char* cScalarWriterStrings[] = {
    "test-change-index",              // XML tag name
    "Test Aggregate Change Index",    // Display name
    "Test Aggregate Change Index",    // Description
    "%-8.8f"                          // Printf format string
};
static const uint32 cScalarWriterSizeLimit = 200;
static const double cScalarWriterExpectedMin = 0.01;
static const double cScalarWriterExpectedMax = 666;

static const char* cGraphicsWriterStrings[] = {
    "test-graphics",          // XML tag name
    "Test Graphics",          // Display name
    "Test Graphics Visualization",   // Description
    "%i"                   // Printf format string
};
static const uint32 cGraphicsWriterSizeLimit = 201;

static const char* cPositionWriterStrings[] = {
    "test-position",          // XML tag name
    "Test Position",          // Display name
    "Test Position Coordinates",   // Description
    "%f,%f"                   // Printf format string
};
static const uint32 cPositionWriterSizeLimit = 202;
static const rcFPair cPositionWriterExpectedMin(-1.0, -2.0);
static const rcFPair cPositionWriterExpectedMax(150.0, 200.0);

class rcTestExperimentObserver : public rcExperimentObserver
{
public:
	rcTestExperimentObserver( void )
	{
        mErrors = 0;
	}

    uint32 errors() {
        return mErrors;
    }
    
	// called to notify the observer that an error occurred.
	virtual void notifyError( const char* errorString )
        {
            cerr << "Error: " << errorString << endl;
            ++mErrors;
        }

	// called to warn the observer of some condition.
	virtual void notifyWarning( const char* warnString )
        {
            cerr << "Warning: " << warnString << endl;
            ++mErrors;
        }

	// called to send status to the observer
	virtual void notifyStatus( const char* statusString )
        {
            rmUnused( statusString );
        }

	// called to notify observer of a state change.
	virtual void notifyState( rcExperimentState newState,
                              bool immediateDispatch )
        {
            rmUnused( immediateDispatch );
            rmUnused( newState );
        }

    // called to notify observer of a time lapse
    virtual void notifyTime( const rcTimestamp& cursorTime )
        {
            rmUnused( cursorTime );
        }
    
    // called to notify observer that cursor time has programatically been changed
    virtual void notifyProgCursorTime( const rcTimestamp& cursorTime )
        {
            rmUnused( cursorTime );
        }

    // Returns the number of eNotifyCursorEvent events queued up for processing
    virtual int32 progCursorTimeEventCount( ) const
        {
            return 0;
        }

    // called to notify observer of a time cursor selection change.
    virtual void notifyCursorTime( const rcTimestamp& cursorTime )
        {
            rmUnused( cursorTime );
        }

    // called to notify observer of a timeline range change
    void notifyTimelineRange( const rcTimestamp& start ,
                              const rcTimestamp& end )
        {
            rmUnused( start );
            rmUnused( end );
        }

    // called to notify observer of a timeline range change
    void notifyEngineTimelineRange( const rcTimestamp& start ,
                                    const rcTimestamp& end )
        {
            rmUnused( start );
            rmUnused( end );
        }

    // called to notify observer of a analysis area change
    void notifyAnalysisRect( const rcRect& rect )
        {
            rmUnused( rect );
        }

    // called to notify observer of a analysis focus rect rotation change
    // angle is in degrees
    void notifyAnalysisRectRotation( const rcAffineRectangle& affine)
        {
            rmUnused( affine );
        }
    
    // called to notify observer of a analysis area change
    void notifyMultiplier( const double& multiplier )
        {
            rmUnused( multiplier );
        }
      
	// called to ask the observer if it should blast an image to
	//	a part of the screen of the observer's choosing.
	virtual bool acceptingImageBlits( void )
        {
            return false;
        }

    virtual void notifyBlitData( const rcWindow* image )
        {
            cerr << "Error: notifyBlitData called: " << image << endl;
            ++mErrors;
        }
    
    virtual void notifyBlitGraphics( const rcVisualGraphicsCollection* graphics )
        {
            cerr << "Error: notifyBlitGraphics called: " << graphics << endl;
            ++mErrors;
        }
    
    virtual void notifyLockApp( bool lock )
        {
            rmUnused( lock );
        }

    virtual void notifyExperimentChange()
        {
        }

    virtual void notifyVideoRect( const rcRect& rect )
        {
            rmUnused( rect );
        }

    virtual bool acceptingPolys ( void )  { return true; }
    
  virtual void notifyPolys ( /* const rcPolygonGroupRef * polys */ ) { /* rmUnused (polys); */ }

    virtual void getPolys ( rcPolygonGroupRef& polys ) { rmUnused (polys); }
	
    virtual bool doneSelecting ( void ) { return true; }

private:
    uint32 mErrors;
};

//
// UT_Model implementation
//

// public

uint32 UT_Model::run()
{
    uint32 errors = 0;

    // Use default domain implementation
    rcExperimentDomainImpl* domain = dynamic_cast<rcExperimentDomainImpl*>(rcExperimentDomainFactory::getExperimentDomain());
    rcUNITTEST_ASSERT( domain != NULL );
    
    // Create a test observer instance and initialize the domain
    rcTestExperimentObserver observer;
    // Initialize domain
    domain->initialize( &observer );

    // Perform tests
    errors += testTracks( domain );
    errors += testObserver( domain );

    // Shut down domain
    domain->shutdown();
    
    return errors;
}

uint32 UT_Model::testTracks( rcExperimentDomainImpl* domain )
{
    mErrors = 0;
    rcUNITTEST_ASSERT( domain != NULL );
    
    rcExperiment* experiment = domain->getExperiment();
    rcUNITTEST_ASSERT( experiment != NULL );

    // Static rcTrack method tests
    for ( int i = 0; i < eMaxTrack; ++i ) {
        rcTrackType type = static_cast<rcTrackType>(i);
        // Test type <-> name mapping
        const std::string& name = rcTrack::name( type );
        rcUNITTEST_ASSERT( type == rcTrack::type( name ) );
    }
    
    // Writer group tests
    {
        const char* tag = "test-tag";
        const char* name = "test-name";
        const char* descr = "test-descr";
        
        rcWriterGroup* wGroup = domain->createWriterGroup( tag ,
                                                           name ,
                                                           descr,
                                                           eGroupSemanticsUnknown );
        // There must be one group now
        rcUNITTEST_ASSERT( experiment->getNTrackGroups() == 1 );
        rcTrackGroup* tGroup = experiment->getTrackGroup( 0 );
        rcUNITTEST_ASSERT( tGroup != NULL );
        rcRect analysisRect( 0, 0, 100, 100 );
        
        if ( tGroup ) {
            int curTrackCount = 0;
            rcUNITTEST_ASSERT( !strcmp( tGroup->getTag(), tag ) );
            rcUNITTEST_ASSERT( !strcmp( tGroup->getName(), name ) );
            rcUNITTEST_ASSERT( !strcmp( tGroup->getDescription(), descr ) );
            // Group should be empty
            rcUNITTEST_ASSERT( tGroup->getNTracks() == curTrackCount );
            {
                // Create a video writer
                rcWriter* writer = wGroup->createWriter( eVideoWriter,
                                                         cVideoWriterStrings[0],
                                                         cVideoWriterStrings[1],
                                                         cVideoWriterStrings[2],
                                                         cVideoWriterStrings[3],
                                                         cVideoWriterSizeLimit,
                                                         analysisRect );
                ++curTrackCount;
                rcUNITTEST_ASSERT( tGroup->getNTracks() == curTrackCount );
                rcTrack* track = tGroup->getTrack( curTrackCount-1 );
                // Check track base class
                rcUNITTEST_ASSERT( track != NULL );
                rcUNITTEST_ASSERT( track->getTrackType() == eVideoTrack );
                rcUNITTEST_ASSERT( !strcmp( track->getTag(), cVideoWriterStrings[0] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getName(), cVideoWriterStrings[1] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getDescription(), cVideoWriterStrings[2] ) );
                rcUNITTEST_ASSERT( track->getSizeLimit() == cVideoWriterSizeLimit );
                rcUNITTEST_ASSERT( !strcmp( track->getDisplayFormatString(), cVideoWriterStrings[3] ) );
                
                rcVideoWriter* videoWriter = dynamic_cast<rcVideoWriter*>( writer );
                rcVideoTrack* videoTrack = dynamic_cast<rcVideoTrack*>( track );
                
                // Test video track read/write/iteration
                testVideoTrack( videoWriter, videoTrack, cVideoWriterSizeLimit );
            }
            {
                // Create a scalar writer
                rcWriter* writer = wGroup->createWriter( eScalarWriter,
                                                         cScalarWriterStrings[0],
                                                         cScalarWriterStrings[1],
                                                         cScalarWriterStrings[2],
                                                         cScalarWriterStrings[3],
                                                         cScalarWriterSizeLimit,
                                                         analysisRect );
                ++curTrackCount;
                rcUNITTEST_ASSERT( tGroup->getNTracks() == curTrackCount );
                rcTrack* track = tGroup->getTrack( curTrackCount-1 );
                // Check track base class
                rcUNITTEST_ASSERT( track != NULL );
                rcUNITTEST_ASSERT( track->getTrackType() == eScalarTrack );
                rcUNITTEST_ASSERT( !strcmp( track->getTag(), cScalarWriterStrings[0] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getName(), cScalarWriterStrings[1] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getDescription(), cScalarWriterStrings[2] ) );
                rcUNITTEST_ASSERT( track->getSizeLimit() == cScalarWriterSizeLimit );
                rcUNITTEST_ASSERT( !strcmp( track->getDisplayFormatString(), cScalarWriterStrings[3] ) );
                
                rcScalarWriter* scalarWriter = dynamic_cast<rcScalarWriter*>( writer );
                rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );
                // Check writer derived class
                scalarWriter->setExpectedMinValue( cScalarWriterExpectedMin );
                scalarWriter->setExpectedMaxValue( cScalarWriterExpectedMax );
                    
                // Check track derived class
                rcUNITTEST_ASSERT( scalarTrack->getExpectedMinimumValue() == cScalarWriterExpectedMin );
                rcUNITTEST_ASSERT( scalarTrack->getExpectedMaximumValue() == cScalarWriterExpectedMax );
                rcUNITTEST_ASSERT( scalarTrack->getCurrentMinimumValue() == 1.0 );
                rcUNITTEST_ASSERT( scalarTrack->getCurrentMaximumValue() == 0.0 );
                
                // Test scalar track read/write/iteration
                testScalarTrack( scalarWriter, scalarTrack, cScalarWriterSizeLimit, false );

                // Create another scalar writer
                writer = wGroup->createWriter( eScalarWriter,
                                               cScalarWriterStrings[0],
                                               cScalarWriterStrings[1],
                                               cScalarWriterStrings[2],
                                               cScalarWriterStrings[3],
                                               cScalarWriterSizeLimit,
                                               analysisRect );
                ++curTrackCount;
                rcUNITTEST_ASSERT( tGroup->getNTracks() == curTrackCount );
                track = tGroup->getTrack( curTrackCount-1 );
                // Check track base class
                rcUNITTEST_ASSERT( track != NULL );
                rcUNITTEST_ASSERT( track->getTrackType() == eScalarTrack );
                rcUNITTEST_ASSERT( !strcmp( track->getTag(), cScalarWriterStrings[0] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getName(), cScalarWriterStrings[1] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getDescription(), cScalarWriterStrings[2] ) );
                rcUNITTEST_ASSERT( track->getSizeLimit() == cScalarWriterSizeLimit );
                rcUNITTEST_ASSERT( !strcmp( track->getDisplayFormatString(), cScalarWriterStrings[3] ) );
                
                scalarWriter = dynamic_cast<rcScalarWriter*>( writer );
                scalarTrack = dynamic_cast<rcScalarTrack*>( track );

                // Test scalar track read/write/iteration
                //testScalarTrackStart( scalarWriter, scalarTrack, 1 );
                testScalarTrackStart( scalarWriter, scalarTrack, 2 );
            }
            {
                // Create a graphics writer
                rcWriter* writer = wGroup->createWriter( eGraphicsWriter,
                                                         cGraphicsWriterStrings[0],
                                                         cGraphicsWriterStrings[1],
                                                         cGraphicsWriterStrings[2],
                                                         cGraphicsWriterStrings[3],
                                                         cGraphicsWriterSizeLimit,
                                                         analysisRect );
                ++curTrackCount;
                rcUNITTEST_ASSERT( tGroup->getNTracks() == curTrackCount );
                rcTrack* track = tGroup->getTrack( curTrackCount-1 );
                // Check track base class
                rcUNITTEST_ASSERT( track != NULL );
                rcUNITTEST_ASSERT( track->getTrackType() == eGraphicsTrack );
                rcUNITTEST_ASSERT( !strcmp( track->getTag(), cGraphicsWriterStrings[0] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getName(), cGraphicsWriterStrings[1] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getDescription(), cGraphicsWriterStrings[2] ) );
                rcUNITTEST_ASSERT( track->getSizeLimit() == cGraphicsWriterSizeLimit );
                rcUNITTEST_ASSERT( !strcmp( track->getDisplayFormatString(), cGraphicsWriterStrings[3] ) );
                
                rcGraphicsWriter* graphicsWriter = dynamic_cast<rcGraphicsWriter*>( writer );
                rcGraphicsTrack* graphicsTrack = dynamic_cast<rcGraphicsTrack*>( track );
                
                // Test graphics track read/write/iteration
                testGraphicsTrack( graphicsWriter, graphicsTrack, cGraphicsWriterSizeLimit );
            }
            {
                // Create a position writer
                rcWriter* writer = wGroup->createWriter( ePositionWriter,
                                                         cPositionWriterStrings[0],
                                                         cPositionWriterStrings[1],
                                                         cPositionWriterStrings[2],
                                                         cPositionWriterStrings[3],
                                                         cPositionWriterSizeLimit,
                                                         analysisRect );
                ++curTrackCount;
                rcUNITTEST_ASSERT( tGroup->getNTracks() == curTrackCount );
                rcTrack* track = tGroup->getTrack( curTrackCount-1 );

                // Check track base class
                rcUNITTEST_ASSERT( track != NULL );
                rcUNITTEST_ASSERT( track->getTrackType() == ePositionTrack );
                rcUNITTEST_ASSERT( !strcmp( track->getTag(), cPositionWriterStrings[0] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getName(), cPositionWriterStrings[1] ) );
                rcUNITTEST_ASSERT( !strcmp( track->getDescription(), cPositionWriterStrings[2] ) );
                rcUNITTEST_ASSERT( track->getSizeLimit() == cPositionWriterSizeLimit );
                rcUNITTEST_ASSERT( !strcmp( track->getDisplayFormatString(), cPositionWriterStrings[3] ) );
                
                rcPositionWriter* positionWriter = dynamic_cast<rcPositionWriter*>( writer );
                rcPositionTrack* positionTrack = dynamic_cast<rcPositionTrack*>( track );

                // Check writer derived class
                positionWriter->setExpectedMinValue( cPositionWriterExpectedMin );
                positionWriter->setExpectedMaxValue( cPositionWriterExpectedMax );
                    
                // Check track derived class
                rcUNITTEST_ASSERT( positionTrack->getExpectedMinimumValue() == cPositionWriterExpectedMin );
                rcUNITTEST_ASSERT( positionTrack->getExpectedMaximumValue() == cPositionWriterExpectedMax );
                rcUNITTEST_ASSERT( positionTrack->getCurrentMinimumValue() == rcFPair(1.0, 1.0) );
                rcUNITTEST_ASSERT( positionTrack->getCurrentMaximumValue() == rcFPair(0.0, 0.0) );
                
                // Test position track read/write/iteration
                testPositionTrack( positionWriter, positionTrack, cPositionWriterSizeLimit );
            }
        }
    }

    // Performance test
    iterationPerformanceGraphics( domain, experiment );
    iterationPerformanceScalar( domain, experiment );
    
    printSuccessMessage( "rcTrack test", mErrors );

    return mErrors;
}

uint32 UT_Model::testObserver( rcExperimentDomainImpl* domain )
{
	int i;
    mErrors = 0;
    rcUNITTEST_ASSERT( domain != NULL );
    
    if ( domain != NULL ) {
        // create a test observer instance and initialize the domain
        rcTestExperimentObserver observer;
        domain->initialize( &observer );
        
        // the domain starts up with a fresh (empty) experiment
        rcExperiment* experiment = domain->getExperiment();
        rcUNITTEST_ASSERT( experiment != NULL );
        
        if ( experiment != NULL ) {
            // Check out the settings
            int nSettings = experiment->getNSettingCategories();
            for (i = 0; i < nSettings; i++)
            {
                rcSettingCategory settings = experiment->getSettingCategory( i );
                rcUNITTEST_ASSERT( settings.getTag() != 0 );
                rcUNITTEST_ASSERT( settings.getName() != 0 );
                rcUNITTEST_ASSERT( settings.getDescription() != 0 );
            }
            
            // create new experiment
            domain->newExperiment();
            // run the experiment for a bit
            domain->startExperiment();
            rcThread::sleep( 1000 );
            domain->stopExperiment();

            experiment = domain->getExperiment();
            // get the track data
            int nTrackGroups = experiment->getNTrackGroups();
            rcUNITTEST_ASSERT( nTrackGroups > 0 );
            
            for (int i = 0; i < nTrackGroups; i++)
            {
                rcTrackGroup* trackGroup = experiment->getTrackGroup( i );
                rcUNITTEST_ASSERT( trackGroup != NULL );
                
                int nTracks = trackGroup->getNTracks();
                rcUNITTEST_ASSERT( nTracks > 0 );
                
                for (int j = 0; j < nTracks; j++)
                {
                    rcTrack* track = trackGroup->getTrack( j );
                    rcUNITTEST_ASSERT( track != NULL );
                    rcUNITTEST_ASSERT( track->getName() != NULL );
                    
                    if ( track->getTrackType() == eScalarTrack ) {
                        rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );
                        if (scalarTrack != 0)
                        {
                            rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( 0.0 );
                            rcUNITTEST_ASSERT( iter != NULL );
                            
                            while (iter->hasNextSegment())
                            {
                                rcTimestamp length = iter->getSegmentLength();
                                rcUNITTEST_ASSERT( length > 0.0 );
                                rcTimestamp segOfst = iter->getSegmentOffset();
                                rcUNITTEST_ASSERT( segOfst == 0.0 );
                                rcTimestamp trackOfst = iter->getTrackOffset();
                                rcUNITTEST_ASSERT( trackOfst >= 0.0 );
                                double value = iter->getValue();
                                rcUNITTEST_ASSERT( value > 0.0 );
                                iter->advance( length );
                            }
                            delete iter;
                        }
                    }
                }
            }
        }
        // Get observer errors
        mErrors += observer.errors();
    }
    
    printSuccessMessage( "rcExperimentObserver test", mErrors );
    
	return mErrors;
}

// private

// Test segment consistency
void UT_Model::testSegment( rcSegmentIterator* iter )
{
    // Basic segment tests
    const rcTimestamp length = iter->getSegmentLength();
    rcUNITTEST_ASSERT( length > 0.0 );
    const rcTimestamp segOfst = iter->getSegmentOffset();
    rcUNITTEST_ASSERT( segOfst >= 0.0 );
    const rcTimestamp trackOfst = iter->getTrackOffset();
    rcUNITTEST_ASSERT( trackOfst >= 0.0 );
}

// Test video track read/write/iteration

void UT_Model::testVideoTrack( rcVideoWriter* videoWriter,
                               rcVideoTrack* videoTrack,
                               int32 trackSizeLimit )
{
    rcUNITTEST_ASSERT( videoWriter != 0 );
    rcUNITTEST_ASSERT( videoTrack != 0 );

    // Fill writer to the limit
    const int testCount = trackSizeLimit;
    const rcTimestamp frameInterval = 0.067;
    rcTimestamp t = 0.0;
    vector<rcWindow> testImages;
    
    // Track should be empty
    {
        rcVideoSegmentIterator* iter = videoTrack->getDataSegmentIterator( 0.0 );
        const rcWindow& v = iter->getValue();
        rcUNITTEST_ASSERT( !v.isBound() );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        delete iter;
    }

    // Track should be empty 
    {
        // Flush should do nothing for an empty track
        videoWriter->flush();
        rcVideoSegmentIterator* iter = videoTrack->getDataSegmentIterator( 0.0 );
        const rcWindow& v = iter->getValue();
        rcUNITTEST_ASSERT( !v.isBound() );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        delete iter;
    }
    
    // Write values to track
    for ( int i = 0; i < testCount; i++ ) {
        rcWindow testImage( 80, 60 );
        testImage.randomFill();
        rcRect focusRect( i, i, testCount, testCount );
        testImage.frameBuf()->setTimestamp( t );
        videoWriter->writeValue( t, focusRect, &testImage );
        testImages.push_back( testImage );
        t = t + frameInterval;
    }
    videoWriter->flush();

    // Time-based forward iteration
    {
        // Read values from track
        rcVideoSegmentIterator* iter = videoTrack->getDataSegmentIterator( 0.0 );
        rcUNITTEST_ASSERT( iter != NULL );

        const rcWindow& value = iter->getValue();
        rcUNITTEST_ASSERT( value.isBound() );
        int segmentCount = 0;
        
        while ( iter->hasNextSegment() )
        {
            rcUNITTEST_ASSERT( iter->getSegmentIndex() == segmentCount );
            rcUNITTEST_ASSERT( iter->getSegmentLength() == frameInterval );
            rcUNITTEST_ASSERT( iter->contains( iter->getSegmentStartAbsolute() ) );
            rcUNITTEST_ASSERT( iter->contains( iter->getSegmentStartAbsolute() +  iter->getSegmentLength().secs()/2 ) );
            rcUNITTEST_ASSERT( iter->contains( iter->getSegmentStartAbsolute() +  iter->getSegmentLength() ) );
            rcRect expectedRect( segmentCount, segmentCount, testCount, testCount );
            testVideoSegment( iter, testImages[segmentCount], expectedRect );
            iter->advance( frameInterval );
            ++segmentCount;
        }
        ++segmentCount; // One extra because we started at 0

        // Test reset
        iter->reset();
        rcUNITTEST_ASSERT( iter->getSegmentIndex() == 0 );
        rcRect expectedRect( 0, 0, testCount, testCount );
        testVideoSegment( iter, testImages[0], expectedRect );
        
        rcUNITTEST_ASSERT( segmentCount == testCount );
        if ( segmentCount != testCount )
            cerr << "video segment count error: got " << segmentCount << ", expected " << testCount << endl;
        rcUNITTEST_ASSERT( segmentCount <= trackSizeLimit );
        delete iter;
    }
    
    // Time-based random access
    {
        vector<rcWindow>::iterator i;

        // Access all images by timestamp
        for ( i = testImages.begin(); i < testImages.end(); i++ ) {
            // Note: old interval offset implementation is off by one frame interval
            rcTimestamp t = i->frameBuf()->timestamp();
            int index = i - testImages.begin();
            rcRect expectedRect( index, index, testImages.size(), testImages.size() );
            
            rcVideoSegmentIterator* iter = videoTrack->getDataSegmentIterator( t );
            rcUNITTEST_ASSERT( iter->getTrackOffset() == t );
            rcUNITTEST_ASSERT( iter != NULL );
            rcUNITTEST_ASSERT( iter->getSegmentIndex() == index );
            testVideoSegment( iter, *i, expectedRect );
            delete iter;
        }

        // Access all images by timestamp with a segment offset
        rcTimestamp expectedOffset = frameInterval.secs()/3;

        for ( i = testImages.begin(); i < testImages.end(); i++ ) {
            // Note: old interval offset implementation is off by one frame interval
            rcTimestamp t = i->frameBuf()->timestamp() + expectedOffset;
            int index = i - testImages.begin();
            rcRect expectedRect( index, index, testImages.size(), testImages.size() );
            
            testVideoSegment( videoTrack, t, *i, expectedRect, expectedOffset );
        }
    }

    // Index-based iteration
    
    // Test positive index offsets
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 1 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 2 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 3 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 7 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 16 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, testCount );
    
    // Test negative index offsets
    rcTimestamp lastSegmentTime = frameInterval.secs() * (testCount+1);
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -1 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -2 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -3 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -7 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -16 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, -testCount );
    
    // Overflow test
    {
        rcRect focusRect0( 0, 0, testCount, testCount );
        rcRect focusRect1( 1, 1, testCount, testCount );
         
        // Check first segment
        testVideoSegment( videoTrack, 0.0, testImages[0], focusRect0, 0.0 );
        
        // Write one more value, it should cause the first entry to be purged
        rcTimestamp lastTime = frameInterval.secs() * (testCount+1);
        rcWindow testImage( testCount*2, testCount*2 );
        testImage.randomFill();
        testImage.frameBuf()->setTimestamp( lastTime );
        videoWriter->writeValue( lastTime, focusRect0, &testImage );

        // Check first segment again, it should be different now
        testVideoSegment( videoTrack, 0.0, testImages[1], focusRect1, 0.0 );
    }
}

// Test one video segment
void UT_Model::testVideoSegment( rcVideoSegmentIterator* iter,
                                 const rcWindow& expectedImage,
                                 const rcRect& expectedRect )
{
    rcUNITTEST_ASSERT( iter != NULL );

    if ( iter ) {
        // Basic consistency check
        testSegment( iter );
        // Image tests
        const rcWindow& value = iter->getValue();
        rcUNITTEST_ASSERT( value.isBound() );
        rcUNITTEST_ASSERT( expectedImage.frameBuf() == value.frameBuf() );
        rcUNITTEST_ASSERT( expectedImage.getPixel(0,0) == value.getPixel(0,0) );
        rcUNITTEST_ASSERT( expectedImage.frameBuf()->timestamp() == value.frameBuf()->timestamp() );
        if ( expectedImage.frameBuf()->timestamp() != value.frameBuf()->timestamp() ) {
            cerr << "video timestamp error: got " << value.frameBuf()->timestamp();
            cerr << " expected " <<  expectedImage.frameBuf()->timestamp() << endl;
        }
        const rcRect focusRect = iter->getFocus();
        rcUNITTEST_ASSERT( focusRect == expectedRect );
        if ( focusRect != expectedRect )
            cerr << "video focus rect error: got " << focusRect << ", expected " << expectedRect << endl;
    }
}

// Test one positions segment
void UT_Model::testPositionSegment( rcPositionSegmentIterator* iter,
                                    const rcFPair& expectedPosition,
                                    const rcRect& expectedRect )
{
    rcUNITTEST_ASSERT( iter != NULL );

    if ( iter ) {
        // Basic segment tests
        testSegment( iter );
        
        // Position tests
        const rcFPair value = iter->getValue();
        rcUNITTEST_ASSERT( value.x() > 0 && value.y() > -51.0 );
        rcUNITTEST_ASSERT( expectedPosition == value );
        const rcRect focusRect = iter->getFocus();
        rcUNITTEST_ASSERT( focusRect == expectedRect );
        if ( focusRect != expectedRect )
            cerr << "positions focus rect error: got " << focusRect << ", expected " << expectedRect << endl;
    }
}

// Test one video segment
void UT_Model::testVideoSegment( rcVideoTrack* videoTrack,
                                 const rcTimestamp& time,
                                 const rcWindow& expectedImage,
                                 const rcRect& expectedRect,
                                 const rcTimestamp& expectedSegOffset )
{
    rcVideoSegmentIterator* iter = videoTrack->getDataSegmentIterator( time );
    rcUNITTEST_ASSERT( iter->getTrackOffset() == time );
    rcUNITTEST_ASSERT( iter->getSegmentOffset() == expectedSegOffset );
    testVideoSegment( iter, expectedImage, expectedRect );
    delete iter;
}

// Test index-based iteration
void UT_Model::testVideoIndexIteration( rcVideoTrack* videoTrack,
                                        int32 trackSizeLimit,
                                        vector<rcWindow>& testValues,
                                        const rcTimestamp& startTime,
                                        int32 indexOffset )
{
    rcVideoSegmentIterator* iter = videoTrack->getDataSegmentIterator( startTime );
    rcUNITTEST_ASSERT( iter != NULL );
    
    const rcWindow& v = iter->getValue();
    rcUNITTEST_ASSERT( v.isBound() );
    int32 segmentIndex = iter->getSegmentIndex();
    int32 segmentCount = 0;
    bool nextSegment = false;

    if ( segmentIndex >= 0 ) {
        segmentCount = 1;
        nextSegment = true;
    }

    while ( nextSegment )
    {
        rcUNITTEST_ASSERT( segmentIndex >= 0 );
        rcUNITTEST_ASSERT( segmentIndex < int32(testValues.size()) );
        rcRect expectedRect( segmentIndex, segmentIndex, testValues.size(), testValues.size() );
        testVideoSegment( iter, testValues[segmentIndex], expectedRect );
        nextSegment = iter->advance( indexOffset );
        if ( nextSegment ) {
            segmentIndex += indexOffset;
            ++segmentCount;
            rcUNITTEST_ASSERT( segmentIndex == iter->getSegmentIndex() );
        }
    }
    rcUNITTEST_ASSERT( segmentCount <= trackSizeLimit );

    if ( segmentIndex >= 0 ) {
        int32 expectedCount = int32( 1.0 * testValues.size()/abs(indexOffset) + 0.5 );
        rcUNITTEST_ASSERT( expectedCount == segmentCount );
        
        if ( expectedCount != segmentCount )
            cerr << "video segment count error: got " << segmentCount << ", expected " << expectedCount << endl;
    }
    delete iter;

    // Test explicit locking/unlocking
    iter = videoTrack->getDataSegmentIterator( startTime );
    segmentIndex = iter->getSegmentIndex();
    if ( segmentIndex >= 0 ) {
        segmentCount = 1;
        nextSegment = true;
    } else {
        segmentCount = 0;
        nextSegment = false;
    }
    iter->lock();
    while ( nextSegment )
    {
        rcUNITTEST_ASSERT( segmentIndex >= 0 );
        rcUNITTEST_ASSERT( segmentIndex < int32(testValues.size()) );
        rcRect expectedRect( segmentIndex, segmentIndex, testValues.size(), testValues.size() );
        testVideoSegment( iter, testValues[segmentIndex], expectedRect );
        nextSegment = iter->advance( indexOffset, false );
        if ( nextSegment ) {
            segmentIndex += indexOffset;
            ++segmentCount;
            rcUNITTEST_ASSERT( segmentIndex == iter->getSegmentIndex() );
        }
    }
    iter->unlock();
    rcUNITTEST_ASSERT( segmentCount <= trackSizeLimit );

    if ( segmentIndex >= 0 ) {
        int32 expectedCount = int32( 1.0 * testValues.size()/abs(indexOffset) + 0.5 );
        rcUNITTEST_ASSERT( expectedCount == segmentCount );
        
        if ( expectedCount != segmentCount )
            cerr << "video segment count error: got " << segmentCount << ", expected " << expectedCount << endl;
    }
    delete iter;
}

// Test scalar track read/write/iteration

void UT_Model::testScalarTrack( rcScalarWriter* scalarWriter,
                                rcScalarTrack* scalarTrack,
                                int32 trackSizeLimit,
                                bool perturbedIntervals )
{
    rcUNITTEST_ASSERT( scalarWriter != 0 );
    rcUNITTEST_ASSERT( scalarTrack != 0 );

    // Fill writer to the limit
    const int testCount = trackSizeLimit;
    const rcTimestamp frameInterval = 0.066;
    double valueInterval = 0.5;
    rcTimestamp t = 0.0;
    vector<double> testValues;
    vector <rcTimestamp> frameIntervals;
    
    // Track should be empty
    {
        rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( 0.0 );
        double v = iter->getValue();
        rcUNITTEST_ASSERT( v == 0.0 );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        delete iter;
    }

    // Track should be empty
    {
        // Flush should no nothing for an empty track
        scalarWriter->flush();
        rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( 0.0 );
        double v = iter->getValue();
        rcUNITTEST_ASSERT( v == 0.0 );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        // Double flush should have no effect
        scalarWriter->flush();
        v = iter->getValue();
        rcUNITTEST_ASSERT( v == 0.0 );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        delete iter;
    }

    if ( ! perturbedIntervals ) {
        // Write values with constant interval to track
        for ( int i = 0; i < testCount; i++ ) {
            double value = valueInterval * (i+1);
            rcRect focusRect( i, i, testCount, testCount );
            scalarWriter->writeValue( t, focusRect, value );
            testValues.push_back( value );
            frameIntervals.push_back( frameInterval );
            t = t + frameInterval;
        }
        frameIntervals.push_back( frameInterval );
    } else {
        rcTimestamp frameInt = 0.0;
        t = 0.0;
        // Write values with varying interval to track
        for ( int i = 0; i < testCount; i++ ) {
            frameInt = frameInterval + (0.000334 * !(i % 3));
            double value = valueInterval * (i+1);
            rcRect focusRect( i, i, testCount, testCount );

            scalarWriter->writeValue( t, focusRect, value );
            testValues.push_back( value );
            frameIntervals.push_back( frameInt );
            t = t + frameInt;
        }
        frameIntervals.push_back( frameInt );
    }
    scalarWriter->flush();
    
    // Test current min/max values
    rcUNITTEST_ASSERT( scalarTrack->getCurrentMinimumValue() == valueInterval );
    rcUNITTEST_ASSERT( scalarTrack->getCurrentMaximumValue() == valueInterval * testCount );
    
    // Time-based forward iteration
    {
        // Read values from track
        rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( 0.0 );
        rcUNITTEST_ASSERT( iter != NULL );
        
        double value = iter->getValue();
        rcUNITTEST_ASSERT( value > 0.0 );
        int segmentCount = 0;
        
        while ( iter->hasNextSegment() )
        {
            rcTimestamp length = iter->getSegmentLength();
            rcUNITTEST_ASSERT( uint32(segmentCount+1) < frameIntervals.size() );
            rcUNITTEST_ASSERT( length == frameIntervals[segmentCount] );
            rcRect expectedRect( segmentCount, segmentCount, testCount, testCount );
            testScalarSegment( iter, testValues[segmentCount], expectedRect );
            iter->advance( frameIntervals[segmentCount] );
            ++segmentCount;
        }
        ++segmentCount; // One extra because we started at 0
        
        rcUNITTEST_ASSERT( testCount == segmentCount );
        if ( segmentCount != testCount )
            cerr << "scalar segment count error: got " << segmentCount << ", expected " << testCount << endl;
        rcUNITTEST_ASSERT( segmentCount <= trackSizeLimit );

        // Test reset
        iter->reset();
        rcUNITTEST_ASSERT( iter->getSegmentIndex() == 0 );
        rcRect eRect( 0, 0, testCount, testCount );
        testScalarSegment( iter, testValues[0], eRect );
        
        delete iter;
    }

    // Time-based random access
    {
        vector<double>::iterator i;
        t = 0.0;
        
        // Access all values by timestamp
        for ( i = testValues.begin(); i < testValues.end(); i++ ) {
            int index = i - testValues.begin();
            rcRect expectedRect( index, index, testValues.size(), testValues.size() );
            
            testScalarSegment( scalarTrack, t, *i, expectedRect, 0.0 );
            rcUNITTEST_ASSERT( uint32(index+1) < frameIntervals.size() );
            t = t + frameIntervals[index];
        }

        // Access all values by timestamp with a segment offset
        rcTimestamp expectedOffset = frameIntervals[0].secs()/2;
        t = 0.0 + expectedOffset;
        for ( i = testValues.begin(); i < testValues.end(); i++ ) {
            int index = i - testValues.begin();
            rcRect expectedRect( index, index, testValues.size(), testValues.size() );
            
            testScalarSegment( scalarTrack, t, *i, expectedRect, expectedOffset );
            rcUNITTEST_ASSERT( uint32(index+1) < frameIntervals.size() );
            t = t + frameIntervals[index];
        }
    }

    // Test iteration before start of track
    {
        rcTimestamp startTime = -1.0;
        for ( startTime = -1.0; startTime < 0.0; startTime += 0.1 ) {
            rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( startTime );
            rcUNITTEST_ASSERT( iter != NULL );
            
            int32 segmentIndex = iter->getSegmentIndex();
            rcUNITTEST_ASSERT( segmentIndex == -1 );
            rcUNITTEST_ASSERT( iter->getValue() == 0.0 );
            delete iter;
        }
    }
    
    // Index-based iteration
    
    // Test positive index offsets
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, 0.0, 1 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, frameInterval.secs()/2, 1 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, 0.0, 2 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, 0.0, 3 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, 0.0, 7 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, 0.0, 16 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, 0.0, testCount );
    
    // Test negative index offsets
    rcTimestamp lastSegmentTime = frameInterval.secs() * (testCount+1);
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, lastSegmentTime, -1 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, lastSegmentTime - frameInterval.secs()/2, -1 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, lastSegmentTime, -2 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, lastSegmentTime, -3 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, lastSegmentTime, -7 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, lastSegmentTime, -16 );
    testScalarIndexIteration( scalarTrack, trackSizeLimit, testValues, 0.0, -testCount );
    
    // Overflow test
    {
        rcRect focusRect0( 0, 0, testCount, testCount );
        rcRect focusRect1( 1, 1, testCount, testCount );
         
        // Check first segment
        testScalarSegment( scalarTrack, 0.0, testValues[0], focusRect0, 0.0 );
        
        // Write one more value, it should cause the first entry to be purged
        rcTimestamp lastTime = frameInterval.secs() * (testCount+1);
        scalarWriter->writeValue( lastTime, focusRect0, 777 );

        // Check first segment again, it should be different now
        testScalarSegment( scalarTrack, 0.0, testValues[1], focusRect1, 0.0 );
    }

    // After overflow, test iteration before start of first segment
    {
        rcRect focusRect0( 0, 0, testCount, testCount );
        
        // Write a few  more values
        rcTimestamp lastTime = frameInterval.secs() * (testCount+10);
        scalarWriter->writeValue( lastTime, focusRect0, 888 );
        scalarWriter->writeValue( lastTime + lastTime, focusRect0, 999 );

        rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( 0.0 );
        // Get start time of first segment
        rcTimestamp firstSegTime = iter->getSegmentStart();
        delete iter;

        rcTimestamp startTime = 0.0;
        for ( startTime = 0.0; startTime < firstSegTime; startTime += frameInterval.secs()/10 ) {
            rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( startTime );
            rcUNITTEST_ASSERT( iter != NULL );

            // Advancing before start of first segment
            int32 segmentIndex = iter->getSegmentIndex();
            // We should only get first segment
            rcUNITTEST_ASSERT( segmentIndex == 0 );
            rcUNITTEST_ASSERT( startTime < iter->getSegmentStart() );
            if ( segmentIndex != 0 ) {
                cerr << "segmentIndex " << segmentIndex << " segStart " << iter->getSegmentStart() << endl;
            }
            delete iter;
        }
    }
}

// Test track with non-zero start time
void UT_Model::testScalarTrackStart( rcScalarWriter* scalarWriter,
                                     rcScalarTrack* scalarTrack,
                                     int32 trackSizeLimit )
{
    rcUNITTEST_ASSERT( scalarWriter != 0 );
    rcUNITTEST_ASSERT( scalarTrack != 0 );

    // Fill writer to the limit
    const int testCount = trackSizeLimit;
    const rcTimestamp frameInterval = 2.0;
    const rcTimestamp trackStart = 1.0;
    double valueInterval = 0.5;
    rcTimestamp t = trackStart;
    vector<double> testValues;
    vector <rcTimestamp> frameIntervals;

    dynamic_cast<rcWriter*>(scalarWriter)->setTrackStart( trackStart );
    
    // Track should be empty
    {
        // Flush should no nothing for an empty track
        scalarWriter->flush();
        rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( 0.0 );
        double v = iter->getValue();
        rcUNITTEST_ASSERT( v == 0.0 );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        // Double flush should have no effect
        scalarWriter->flush();
        v = iter->getValue();
        rcUNITTEST_ASSERT( v == 0.0 );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        delete iter;
    }

    t = trackStart;
    // Write values with constant interval to track
    for ( int i = 0; i < testCount; i++ ) {
        double value = valueInterval * (i+1);
        rcRect focusRect( i, i, testCount, testCount );
        scalarWriter->writeValue( t, focusRect, value );
        testValues.push_back( value );
        frameIntervals.push_back( frameInterval );
        t += frameInterval;
    }
    frameIntervals.push_back( frameInterval );
    scalarWriter->flush();
    
    // Test current min/max values
    rcUNITTEST_ASSERT( scalarTrack->getCurrentMinimumValue() == valueInterval );
    rcUNITTEST_ASSERT( scalarTrack->getCurrentMaximumValue() == valueInterval * testCount );
    
    // Index-based forward iteration
    {
        // Read values from track
        rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( trackStart );
        rcUNITTEST_ASSERT( iter != NULL );
        
        double value = iter->getValue();
        rcUNITTEST_ASSERT( value > 0.0 );
        int segmentCount = 0;
        
        for (;;) {
            rcTimestamp length = iter->getSegmentLength();
            rcUNITTEST_ASSERT( uint32(segmentCount+1) < frameIntervals.size() );
            rcUNITTEST_ASSERT( length == frameIntervals[segmentCount] );
            rcRect expectedRect( segmentCount, segmentCount, testCount, testCount );
            testScalarSegment( iter, testValues[segmentCount], expectedRect );
            if ( ! iter->advance( 1 ) )
                break;
            ++segmentCount;
        }
        ++segmentCount;
        
        // Test reset
        // Note: because of late track start, reset should reset to 0.0 which
        // is before start of track
        iter->reset();
        rcUNITTEST_ASSERT( iter->getSegmentIndex() == -1 );
        rcUNITTEST_ASSERT( iter->getValue() == 0 );
        
        rcUNITTEST_ASSERT( testCount == segmentCount );
        if ( testCount != segmentCount )
            cerr << "scalar segment count error: got " << segmentCount << ", expected " << testCount << endl;
        rcUNITTEST_ASSERT( segmentCount <= trackSizeLimit );
        delete iter;
    }

    // Test iteration before start of track
    {
        rcTimestamp startTime = 0.0;
        rcTimestamp incr = 0.1;
        
        for ( startTime = 0.0; startTime < trackStart; startTime += incr ) {
            rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( startTime );
            rcUNITTEST_ASSERT( iter != NULL );
            
            int32 segmentIndex = iter->getSegmentIndex();
            rcUNITTEST_ASSERT( segmentIndex == -1 );
            delete iter;
        }
    }
}

// Test one scalar segment
void UT_Model::testScalarSegment( rcScalarSegmentIterator* iter,
                                  double expectedValue,
                                  const rcRect& expectedFocus )
{
    rcUNITTEST_ASSERT( iter != NULL );

    if ( iter ) {
        // Basic consistency check
        testSegment( iter );
        
        // value tests
        double value = iter->getValue();
        rcUNITTEST_ASSERT( value == expectedValue );
        if ( value != expectedValue )
            cerr << "scalar value error: got " << value << ", expected " << expectedValue << endl;
        const rcRect focusRect = iter->getFocus();
        rcUNITTEST_ASSERT( focusRect == expectedFocus );
        if ( focusRect != expectedFocus )
            cerr << "scalar focus rect error: got " << focusRect << ", expected " << expectedFocus << endl;
    }
}



// Test one scalar segment
void UT_Model::testScalarSegment( rcScalarTrack* scalarTrack,
                                  const rcTimestamp& time,
                                  double expectedValue,
                                  const rcRect& expectedRect,
                                  const rcTimestamp& expectedSegOffset )
{
    rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( time );
    rcUNITTEST_ASSERT( iter->getTrackOffset() == time );
    rcUNITTEST_ASSERT( iter->getSegmentOffset() == expectedSegOffset );
    if ( iter->getSegmentOffset() != expectedSegOffset ) {
        cerr << "getSegmentOffset returned " << iter->getSegmentOffset();
        cerr << ", expected " << expectedSegOffset << endl;
    }
    
    testScalarSegment( iter, expectedValue, expectedRect );
    delete iter;
}

// Test index-based iteration
void UT_Model::testScalarIndexIteration( rcScalarTrack* scalarTrack,
                                         int32 trackSizeLimit,
                                         vector<double>& testValues,
                                         const rcTimestamp& startTime,
                                         int32 indexOffset )
{
    rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( startTime );
    rcUNITTEST_ASSERT( iter != NULL );
    
    double v = iter->getValue();
    rcUNITTEST_ASSERT( v != 0.0 );
    int32 segmentIndex = iter->getSegmentIndex();
    int32 segmentCount = 0;
    bool nextSegment = false;

    if ( segmentIndex >= 0 ) {
        segmentCount = 1;
        nextSegment = true;
    }

    while ( nextSegment )
    {
        rcUNITTEST_ASSERT( segmentIndex >= 0 );
        rcUNITTEST_ASSERT( segmentIndex < int32(testValues.size()) );
        rcRect expectedRect( segmentIndex, segmentIndex, testValues.size(), testValues.size() );
        testScalarSegment( iter, testValues[segmentIndex], expectedRect );
        nextSegment = iter->advance( indexOffset );
        if ( nextSegment ) {
            segmentIndex += indexOffset;
            ++segmentCount;
            rcUNITTEST_ASSERT( segmentIndex == iter->getSegmentIndex() );
        }
    }
    rcUNITTEST_ASSERT( segmentCount <= trackSizeLimit );

    if ( segmentIndex >= 0 ) {
        int32 expectedCount = int32( 1.0 * testValues.size()/abs(indexOffset) + 0.5 );
        rcUNITTEST_ASSERT( expectedCount == segmentCount );
        
        if ( expectedCount != segmentCount )
            cerr << "scalar segment count error: got " << segmentCount << ", expected " << expectedCount << endl;
    }
   
    delete iter;
}
    
// Test iteration performance
void  UT_Model::iterationPerformanceScalar( rcExperimentDomainImpl* domain,
                                            rcExperiment* experiment )
{
    const char* tag = "speed-tag";
    const char* name = "speed-name";
    const char* descr = "speed-descr";
    const int nValues = 1000;
    
    rcWriterGroup* wGroup = domain->createWriterGroup( tag ,
                                                       name ,
                                                       descr,
                                                       eGroupSemanticsGlobalMeasurements );
        
    rcTrackGroup* tGroup = experiment->getTrackGroup( experiment->getNTrackGroups()-1 );
    rcUNITTEST_ASSERT( tGroup != NULL );
    rcRect analysisRect;
    
    if ( tGroup ) {
        // Create a scalar writer
        rcWriter* writer = wGroup->createWriter( eScalarWriter,
                                                 cScalarWriterStrings[0],
                                                 cScalarWriterStrings[1],
                                                 cScalarWriterStrings[2],
                                                 cScalarWriterStrings[3],
                                                 nValues,
                                                 analysisRect );
        rcTrack* track = tGroup->getTrack( tGroup->getNTracks()-1 );
        // Check track base class
        rcUNITTEST_ASSERT( track != NULL );
        rcUNITTEST_ASSERT( track->getTrackType() == eScalarTrack );
                
        rcScalarWriter* scalarWriter = dynamic_cast<rcScalarWriter*>( writer );
        rcScalarTrack* scalarTrack = dynamic_cast<rcScalarTrack*>( track );

        // Write values
        for ( int i = 0; i < nValues; i++ ) {
            double value = 1.0 * i;
            rcRect focusRect;
            scalarWriter->writeValue( rcTimestamp( value ), focusRect, value );
        }

        // Test performance

        // Fetch each value once
        rcTimestamp start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            double value = 1.0 * i;
            rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( value );
            delete iter;
        }
        rcTimestamp finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcScalarSegmentIterator ctor[%i-%i] x %i, %.2f ms, %.2f ms/call\n",
                 0, nValues, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );
        
        // Fetch first value N times
        start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( 0.0 );
            delete iter;
        }
        finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcScalarSegmentIterator ctor[%i] x %i, %.2f ms, %.2f ms/call\n",
                 0, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );
         
        // Fetch last value N times
        start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( (nValues + 1) * 1.0 );
            delete iter;
        }
        finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcScalarSegmentIterator ctor[%i] x %i, %.2f ms, %.2f ms/call\n",
                 nValues, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );

        // Fetch before first value N times
        start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( -1.0 );
            delete iter;
        }
        finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcScalarSegmentIterator ctor[%i] x %i, %.2f ms, %.2f ms/call\n",
                 -1, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );

        // Fetch value after last N times
        start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            rcScalarSegmentIterator* iter = scalarTrack->getDataSegmentIterator( (nValues + 10) * 1.0 );
            delete iter;
        }
        finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcScalarSegmentIterator ctor[%i] x %i, %.2f ms, %.2f ms/call\n",
                 nValues+10, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );
        fprintf( stderr, "\n" );
    }
}

// Test iteration performance
void  UT_Model::iterationPerformanceGraphics( rcExperimentDomainImpl* domain,
                                              rcExperiment* experiment )
{
    const char* tag = "speed-tag";
    const char* name = "speed-name";
    const char* descr = "speed-descr";
    const int nValues = 100;
    const int nVectors = 10000;
    rcRect analysisRect;
    
    rcWriterGroup* wGroup = domain->createWriterGroup( tag ,
                                                       name ,
                                                       descr,
                                                       eGroupSemanticsGlobalGraphics );
        
    rcTrackGroup* tGroup = experiment->getTrackGroup( experiment->getNTrackGroups()-1 );
    rcUNITTEST_ASSERT( tGroup != NULL );
        
    if ( tGroup ) {
        // Create a graphics writer
        rcWriter* writer = wGroup->createWriter( eGraphicsWriter,
                                                 cGraphicsWriterStrings[0],
                                                 cGraphicsWriterStrings[1],
                                                 cGraphicsWriterStrings[2],
                                                 cGraphicsWriterStrings[3],
                                                 nValues,
                                                 analysisRect);
        rcTrack* track = tGroup->getTrack( tGroup->getNTracks()-1 );
        // Check track base class
        rcUNITTEST_ASSERT( track != NULL );
        rcUNITTEST_ASSERT( track->getTrackType() == eGraphicsTrack );
                
        rcGraphicsWriter* graphicsWriter = dynamic_cast<rcGraphicsWriter*>( writer );
        rcGraphicsTrack* graphicsTrack = dynamic_cast<rcGraphicsTrack*>( track );

        // Write values
        for ( int i = 0; i < nValues; i++ ) {
            double value = 1.0 * i;
            rcRect focusRect;
            rcVisualSegmentCollection lines;
            rcStyle style;
            
            // Produce a collection of graphics
            for ( int x = 0; x < nVectors; x++ ) {
                rc2Fvector p1( x+i, nVectors/x+i );
                rc2Fvector p2( nVectors-x+i, x+i );
                rcVisualLine line( p1, p2 );
                lines.push_back( line );
            }
            rcVisualGraphicsCollection graphics( style, lines );
            graphicsWriter->writeValue( rcTimestamp( value ), focusRect, graphics );
        }

        // Test performance

        // Fetch each value once
        rcTimestamp start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            double value = 1.0 * i;
            rcGraphicsSegmentIterator* iter = graphicsTrack->getDataSegmentIterator( value );
            delete iter;
        }
        rcTimestamp finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcGraphicsSegmentIterator %i ctor[%i-%i] x %i, %.2f ms, %.2f ms/call\n",
                 nVectors, 0, nValues, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );
        
        // Fetch first value N times
        start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            rcGraphicsSegmentIterator* iter = graphicsTrack->getDataSegmentIterator( 0.0 );
            delete iter;
        }
        finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcGraphicsSegmentIterator %i ctor[%i] x %i, %.2f ms, %.2f ms/call\n",
                 nVectors, 0, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );
         
        // Fetch last value N times
        start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            rcGraphicsSegmentIterator* iter = graphicsTrack->getDataSegmentIterator( (nValues + 1) * 1.0 );
            delete iter;
        }
        finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcGraphicsSegmentIterator %i ctor[%i] x %i, %.2f ms, %.2f ms/call\n",
                 nVectors, nValues, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );

        // Fetch before first value N times
        start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            rcGraphicsSegmentIterator* iter = graphicsTrack->getDataSegmentIterator( -1.0 );
            delete iter;
        }
        finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcGraphicsSegmentIterator %i ctor[%i] x %i, %.2f ms, %.2f ms/call\n",
                 nVectors, -1, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );

        // Fetch value after last N times
        start = rcTimestamp::now();
        for ( int i = 0; i < nValues; i++ ) {
            rcGraphicsSegmentIterator* iter = graphicsTrack->getDataSegmentIterator( (nValues + 10) * 1.0 );
            delete iter;
        }
        finish = rcTimestamp::now() - start;
        fprintf( stderr, "Performance: rcGraphicsSegmentIterator %i ctor[%i] x %i, %.2f ms, %.2f ms/call\n",
                 nVectors, nValues+10, nValues, finish.secs()*1000,  finish.secs()*1000/nValues );
        fprintf( stderr, "\n" );
    }
}

void UT_Model::testGraphicsTrack( rcGraphicsWriter* graphicsWriter,
                                  rcGraphicsTrack* graphicsTrack,
                                  int32 trackSizeLimit )
{
    rcUNITTEST_ASSERT( graphicsWriter != 0 );
    rcUNITTEST_ASSERT( graphicsTrack != 0 );

#if 1    
    rmUnused( trackSizeLimit );
    // Track should be empty
    {
        rcGraphicsSegmentIterator* iter = graphicsTrack->getDataSegmentIterator( 0.0 );
        const rcVisualGraphicsCollection g = iter->getValue();
        uint32 count = iter->getCount();

        rcUNITTEST_ASSERT( count == 0 );
        rcUNITTEST_ASSERT( g.empty() );
        rcUNITTEST_ASSERT( g.size() == 0 );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        delete iter;
    }

    // Track should be empty 
    {
        // Flush should do nothing for an empty track
        graphicsWriter->flush();
        rcGraphicsSegmentIterator* iter = graphicsTrack->getDataSegmentIterator( 0.0 );
        const rcVisualGraphicsCollection g = iter->getValue();
        uint32 count = iter->getCount();
        
        rcUNITTEST_ASSERT( count == 0 );
        rcUNITTEST_ASSERT( g.empty() );
        rcUNITTEST_ASSERT( g.size() == 0 );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        delete iter;
    }

    // Fill writer to the limit
    const int testCount = trackSizeLimit;
    const rcTimestamp frameInterval = 0.067;
    rcTimestamp t = 0.0;

    rcVisualGraphicsCollectionCollection expectedGraphics;  
            
    // Write values to track
    for ( int i = 0; i < testCount; i++ ) {
        rcRect focusRect( i, i, testCount, testCount );
       
        rcVisualSegmentCollection lines;
        rcStyle style( rfRgb( 56, 57, 58), i, rc2Fvector() );;
            
        // Produce a collection of graphics
        rc2Fvector p1( i, i );
        rc2Fvector p2( i*2, i*i );
        rcVisualLine line( p1, p2 );
        lines.push_back( line );

        rcVisualGraphicsCollection graphics( style, lines );
        graphicsWriter->writeValue( t, focusRect, graphics );
        expectedGraphics.push_back( graphics );
        t = t + frameInterval;
    }
    graphicsWriter->flush();

    // Time-based forward iteration
    {
        // Read values from track
        rcGraphicsSegmentIterator* iter = graphicsTrack->getDataSegmentIterator( 0.0 );
        rcUNITTEST_ASSERT( iter != NULL );
        
        const rcVisualGraphicsCollection& value = iter->getValue();
        rcUNITTEST_ASSERT( !value.empty() );
        rcUNITTEST_ASSERT( value.size() > 0 );
        int segmentCount = 0;
        
        while ( iter->hasNextSegment() )
        {
            rcUNITTEST_ASSERT( iter->getSegmentIndex() == segmentCount );
            rcUNITTEST_ASSERT( iter->getSegmentLength() == frameInterval );
            
            rcRect expectedRect( segmentCount, segmentCount, testCount, testCount );
            testGraphicsSegment( iter, expectedGraphics[segmentCount], expectedRect );
            iter->advance( frameInterval );
            ++segmentCount;
        }
        ++segmentCount; // One extra because we started at 0

        // Test reset
        iter->reset();
        rcUNITTEST_ASSERT( iter->getSegmentIndex() == 0 );
        rcRect expectedRect( 0, 0, testCount, testCount );
        testGraphicsSegment( iter, expectedGraphics[0], expectedRect );
        
        delete iter;       
        
        rcUNITTEST_ASSERT( segmentCount == testCount );
        if ( segmentCount != testCount )
            cerr << "graphics segment count error: got " << segmentCount << ", expected " << testCount << endl;
        rcUNITTEST_ASSERT( segmentCount <= trackSizeLimit );
    }
    
#else // TODO: Implement these soon    
    // Time-based random access
    {
        vector<rcWindow>::iterator i;

        // Access all images by timestamp
        for ( i = testImages.begin(); i < testImages.end(); i++ ) {
            // Note: old interval offset implementation is off by one frame interval
            rcTimestamp t = i->frameBuf()->timestamp() - frameInterval;
            int index = i - testImages.begin();
            rcRect expectedRect( index, index, testImages.size(), testImages.size() );
            
            rcVideoSegmentIterator* iter = videoTrack->getDataSegmentIterator( t );
            rcUNITTEST_ASSERT( iter->getTrackOffset() == t );
            rcUNITTEST_ASSERT( iter != NULL );
            rcUNITTEST_ASSERT( iter->getSegmentIndex() == index );
            testVideoSegment( iter, *i, expectedRect );
            delete iter;
        }

        // Access all images by timestamp with a segment offset
        rcTimestamp expectedOffset = frameInterval.secs()/3;

        for ( i = testImages.begin(); i < testImages.end(); i++ ) {
            // Note: old interval offset implementation is off by one frame interval
            rcTimestamp t = i->frameBuf()->timestamp() - frameInterval + expectedOffset;
            int index = i - testImages.begin();
            rcRect expectedRect( index, index, testImages.size(), testImages.size() );
            
            testVideoSegment( videoTrack, t, *i, expectedRect, expectedOffset );
        }
    }

    // Index-based iteration
    
    // Test positive index offsets
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 1 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 2 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 3 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 7 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, 16 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, testCount );
    
    // Test negative index offsets
    rcTimestamp lastSegmentTime = frameInterval.secs() * (testCount+1);
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -1 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -2 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -3 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -7 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, lastSegmentTime, -16 );
    testVideoIndexIteration( videoTrack, trackSizeLimit, testImages, 0.0, -testCount );
    
    // Overflow test
    {
        rcRect focusRect0( 0, 0, testCount, testCount );
        rcRect focusRect1( 1, 1, testCount, testCount );
         
        // Check first segment
        testVideoSegment( videoTrack, 0.0, testImages[0], focusRect0, 0.0 );
        
        // Write one more value, it should cause the first entry to be purged
        rcTimestamp lastTime = frameInterval.secs() * (testCount+1);
        rcWindow testImage( testCount*2, testCount*2 );
        testImage.randomFill();
        testImage.frameBuf()->setTimestamp( lastTime );
        videoWriter->writeVideoFrame( lastTime, focusRect0, &testImage );

        // Check first segment again, it should be different now
        testVideoSegment( videoTrack, 0.0, testImages[1], focusRect1, 0.0 );
    }
#endif    
}

// Test one graphics segment
void  UT_Model::testGraphicsSegment( rcGraphicsSegmentIterator* iter,
                                     const rcVisualGraphicsCollection& expectedGraphics,
                                     const rcRect& expectedRect )
{
    rcUNITTEST_ASSERT( iter != NULL );

    if ( iter ) {
        // Basic segment tests
        testSegment( iter );
        
        // Position tests
        const rcVisualGraphicsCollection value = iter->getValue();
        rcUNITTEST_ASSERT( value.size() > 0 );
        rcUNITTEST_ASSERT( value.size() == expectedGraphics.size() );
        rcUNITTEST_ASSERT( value.style() == expectedGraphics.style() );
        rcUNITTEST_ASSERT( value.segments() == expectedGraphics.segments() );
        const rcRect focusRect = iter->getFocus();
        rcUNITTEST_ASSERT( focusRect == expectedRect );
        if ( focusRect != expectedRect )
            cerr << "graphics focus rect error: got " << focusRect << ", expected " << expectedRect << endl;
    }
}

// Test position track read/write/iteration
void UT_Model::testPositionTrack( rcPositionWriter* positionWriter,
                                  rcPositionTrack* positionTrack,
                                  int32 trackSizeLimit )
{
    rcUNITTEST_ASSERT( positionWriter != 0 );
    rcUNITTEST_ASSERT( positionTrack != 0 );

    rmUnused( trackSizeLimit );
    // Track should be empty
    {
        rcPositionSegmentIterator* iter = positionTrack->getDataSegmentIterator( 0.0 );
        const rcFPair v = iter->getValue();

        rcUNITTEST_ASSERT( v == rcFPair() );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        delete iter;
    }

    // Track should be empty 
    {
        // Flush should do nothing for an empty track
        positionWriter->flush();
         rcPositionSegmentIterator* iter = positionTrack->getDataSegmentIterator( 0.0 );
        const rcFPair v = iter->getValue();

        rcUNITTEST_ASSERT( v == rcFPair() );
        rcUNITTEST_ASSERT( !iter->hasNextSegment() );
        delete iter;
    }

    // Fill writer to the limit
    const int testCount = trackSizeLimit;
    const rcTimestamp frameInterval = 0.067;
    rcTimestamp t = 0.0;
    vector<rcFPair> testPositions;
    
    // Write values to track
    for ( int i = 0; i < testCount; i++ ) {
        rcRect focusRect( i, i, testCount, testCount );
        rcFPair position( i+1, i-50 );
        positionWriter->writeValue( t, focusRect, position );
        testPositions.push_back( position );
        t = t + frameInterval;
    }
    positionWriter->flush();

    // Test current min/max values
    rcUNITTEST_ASSERT( positionTrack->getCurrentMinimumValue() == rcFPair(1.0, -50.0) );
    rcUNITTEST_ASSERT( positionTrack->getCurrentMaximumValue() == rcFPair( 1.0*testCount, testCount-50-1 ) );
    
    // Time-based forward iteration
    {
        // Read values from track
        rcPositionSegmentIterator* iter = positionTrack->getDataSegmentIterator( 0.0 );
        rcUNITTEST_ASSERT( iter != NULL );
        
        const rcFPair value = iter->getValue();
        rcUNITTEST_ASSERT( value.x() > 0 && value.y() > -51.0 );
        int segmentCount = 0;
        
        while ( iter->hasNextSegment() )
        {
            rcUNITTEST_ASSERT( iter->getSegmentIndex() == segmentCount );
            rcUNITTEST_ASSERT( iter->getSegmentLength() == frameInterval );
            
            rcRect expectedRect( segmentCount, segmentCount, testCount, testCount );
            testPositionSegment( iter, testPositions[segmentCount], expectedRect );
            iter->advance( frameInterval );
            ++segmentCount;
        }
        ++segmentCount; // One extra because we started at 0

        // Test reset
        iter->reset();
        rcUNITTEST_ASSERT( iter->getSegmentIndex() == 0 );
        rcRect expectedRect( 0, 0, testCount, testCount );
        testPositionSegment( iter, testPositions[0], expectedRect );
        
        delete iter;
        
        rcUNITTEST_ASSERT( segmentCount == testCount );
        if ( segmentCount != testCount )
            cerr << "positions segment count error: got " << segmentCount << ", expected " << testCount << endl;
        rcUNITTEST_ASSERT( segmentCount <= trackSizeLimit );

        
    }
    
    // TODO: add more tests
}
