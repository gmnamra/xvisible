/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All Rights reserved.
 *
 *	$Id: ut_model.h 6267 2009-01-15 19:35:29Z arman $
 *
 *	This file contains definitions for model unit tests.
 *
 ******************************************************************************/

#ifndef _rcUT_MODEL_H_
#define _rcUT_MODEL_H_

#include <rc_unittest.h>
#include <rc_rect.h>
#include <rc_window.h>
#include <rc_graphics.h>

class rcVideoWriter;
class rcVideoTrack;
class rcVideoSegmentIterator;
class rcScalarWriter;
class rcScalarTrack;
class rcScalarSegmentIterator;
class rcGraphicsWriter;
class rcGraphicsTrack;
class rcGraphicsSegmentIterator;
class rcPositionWriter;
class rcPositionTrack;
class rcPositionSegmentIterator;
class rcExperimentDomainImpl;
class rcExperiment;
class rcSegmentIterator;

class UT_Model : public rcUnitTest {
public:

    UT_Model() { };
    ~UT_Model() { };

    virtual uint32 run();

    // Test dummy observer using standin engine
    uint32 testObserver( rcExperimentDomainImpl* domain );
    // Test tracks/writers
    uint32 testTracks( rcExperimentDomainImpl* domain );

  private:
    // Test segment consistency
    void testSegment( rcSegmentIterator* iter );
    
    // Test video track read/write/iteration
    void testVideoTrack( rcVideoWriter* videoWriter,
                         rcVideoTrack* videoTrack,
                         int32 trackSizeLimit );
    // Test one video segment
    void testVideoSegment( rcVideoSegmentIterator* iter,
                           const rcWindow& expectedImage,
                           const rcRect& expectedFocus );
    // Test one video segment
    void testVideoSegment( rcVideoTrack* videoTrack,
                           const rcTimestamp& time,
                           const rcWindow& expectedImage,
                           const rcRect& expectedRect,
                           const rcTimestamp& expectedSegOffset );

    // Test index-based iteration
    void testVideoIndexIteration( rcVideoTrack* videoTrack,
                                  int32 trackSizeLimit,
                                  vector<rcWindow>& testValues,
                                  const rcTimestamp& startTime,
                                  int32 indexOffset );
    
    // Test scalar track read/write/iteration
    void testScalarTrack( rcScalarWriter* scalarWriter,
                          rcScalarTrack* scalarTrack,
                          int32 trackSizeLimit,
                          bool perturbedIntervals );
    // Test scalar track read/write/iteration with non-zero track start time
    void testScalarTrackStart( rcScalarWriter* scalarWriter,
                               rcScalarTrack* scalarTrack,
                               int32 trackSizeLimit );
    
    // Test one scalar segment
    void testScalarSegment( rcScalarSegmentIterator* iter,
                            double expectedValue,
                            const rcRect& expectedFocus );
    // Test one scalar segment
    void testScalarSegment( rcScalarTrack* scalarTrack,
                            const rcTimestamp& time,
                            double expectedValue,
                            const rcRect& expectedRect,
                            const rcTimestamp& expectedSegOffset );
    // Test index-based iteration
    void testScalarIndexIteration( rcScalarTrack* scalarTrack,
                                   int32 trackSizeLimit,
                                   vector<double>& testValues,
                                   const rcTimestamp& startTime,
                                   int32 indexOffset );
    // Test iteration performance
    void iterationPerformanceScalar( rcExperimentDomainImpl* domain,
                                     rcExperiment* experiment );
    // Test iteration performance
    void iterationPerformanceGraphics( rcExperimentDomainImpl* domain,
                                       rcExperiment* experiment );
    
    // Test graphics track read/write/iteration
    void testGraphicsTrack( rcGraphicsWriter* graphicsWriter,
                            rcGraphicsTrack* graphicsTrack,
                            int32 trackSizeLimit );

    // Test one graphics segment
    void testGraphicsSegment( rcGraphicsSegmentIterator* iter,
                              const rcVisualGraphicsCollection& expectedGraphics,
                              const rcRect& expectedRect );
    
    // Test position track read/write/iteration
    void testPositionTrack( rcPositionWriter* graphicsWriter,
                            rcPositionTrack* graphicsTrack,
                            int32 trackSizeLimit );

    // Test one position segment
    void testPositionSegment( rcPositionSegmentIterator* iter,
                              const rcFPair& expectedPosition,
                              const rcRect& expectedRect );
};

#endif // _rcUT_MODEL_H_
