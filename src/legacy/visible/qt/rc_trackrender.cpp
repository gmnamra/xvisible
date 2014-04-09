/*
 *
 *$Id $
 *$Log$
 *Revision 1.1  2005/10/13 14:43:17  arman
 *moving track render stuff in to its own file
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <strstream>


#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include <rc_model.h>

#include "rc_modeldomain.h"
#include "rc_trackmanager.h"
#include "rc_trackrender.h"

//
// Track rendering information classes
//

// Generic rendering info base class
rcTrackRenderInfo::rcTrackRenderInfo( int trackGroupNo,
                                      int trackNo,
                                      rcTrack* track,
                                      rcTimestamp startTime,
                                      rcTimestamp endTime,
                                      const QRect& dataRect, bool logDisp ) :
  _track( track ), _startTime( startTime ), _endTime( endTime ), _dataRect( dataRect ), _logDisplay (logDisp)
{
    _tickMarkPen = QPen( QColor(25,25,25) , 1 );
    _tickMarkFont = QFont("Helvetica", 10, QFont::Normal, false);
    rcTrackManager* tmp = rcTrackManager::getTrackManager();
    rcTrackInfo trackInfo = tmp->getTrackInfo( trackGroupNo , trackNo );
    _dataPen = trackInfo.getPen();
    
    _isEnabled = trackInfo.isEnabled() && tmp->isTrackGroupEnabled( trackGroupNo );
    _isHilited = tmp->isHiliteTrack( trackGroupNo , trackNo );
}

// 2D widget render info base class
rc2DTrackRenderInfo::rc2DTrackRenderInfo( int trackGroupNo,
                                          int trackNo,
                                          rcTrack* track,
                                          const rcFPair& min,
                                          const rcFPair& max,
                                          const QRect& dataRect ) :
        rcTrackRenderInfo( trackGroupNo, trackNo, track, cZeroTime, cZeroTime, dataRect ),
        mMinValue( min ), mMaxValue( max ),
        mXAxisName( "X" ), mYAxisName( "Y" ),
        mXAxisNameNeg( "X" ), mYAxisNameNeg( "Y" ),
        mXFormatString( "%.0f" ), mYFormatString( "%.0f" ),
        mTickCount( rcIPair( 10, 10 ) )
{
}

// 2D widget position mode render info class
rcPositionTrackRenderInfo::rcPositionTrackRenderInfo( int trackGroupNo,
                                                      int trackNo,
                                                      rcTrack* track,
                                                      const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, rcFPair(0.0,0.0), rcFPair(0.0,0.0), dataRect )
{
    rcPositionTrack* positionTrack = dynamic_cast<rcPositionTrack*>( track );
    if ( positionTrack ) {
        minValue( positionTrack->getCurrentMinimumValue() );
        maxValue( positionTrack->getCurrentMaximumValue() );
    }
}
    
rcPositionTrackRenderInfo::rcPositionTrackRenderInfo( int trackGroupNo,
                                                      int trackNo,
                                                      rcTrack* track,
                                                      const rcFPair& min,
                                                      const rcFPair& max,
                                                      const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, min, max, dataRect )
{
    xFormatString( "%.1f" );
    yFormatString( "%.1f" );
}

// 2D widget speed+direction mode render info class
rcSpeedDirectionTrackRenderInfo::rcSpeedDirectionTrackRenderInfo( int trackGroupNo,
                                                                  int trackNo,
                                                                  rcTrack* track,
                                                                  const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, rcFPair( 0.0, 0.0 ), rcFPair( 5.0, 360 ), dataRect )
{
    init();
}

rcSpeedDirectionTrackRenderInfo::rcSpeedDirectionTrackRenderInfo( int trackGroupNo,
                                                                  int trackNo,
                                                                  rcTrack* track,
                                                                  const rcFPair& min,
                                                                  const rcFPair& max,
                                                                  const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, min, max, dataRect )
{
    init();
}

// private

void rcSpeedDirectionTrackRenderInfo::init()
{
    xAxisName( "Speed" );
    yAxisName( "Angle" );
    xAxisNameNeg( "Speed" );
    yAxisNameNeg( "Angle" );
    xFormatString( "%.2f" );
    yFormatString( "%.1f" );
    tickCount( rcIPair( 20, 18 ) );
}

// rc2D widget speed+direction mode render info class
rcSpeedDirectionCompassTrackRenderInfo::rcSpeedDirectionCompassTrackRenderInfo( int trackGroupNo,
                                                                                int trackNo,
                                                                                rcTrack* track,
                                                                                const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, rcFPair( 0.0, 0.0 ), rcFPair( 5.0, 360.0 ), dataRect )
{
    init();
}

rcSpeedDirectionCompassTrackRenderInfo::rcSpeedDirectionCompassTrackRenderInfo( int trackGroupNo,
                                                                                int trackNo,
                                                                                rcTrack* track,
                                                                                const rcFPair& min,
                                                                                const rcFPair& max,
                                                                                const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, min, max, dataRect )
{
    init();
}

// private

void rcSpeedDirectionCompassTrackRenderInfo::init()
{
    xAxisName( "Speed 0 deg" );
    yAxisName( "Speed 90 deg");
    xAxisNameNeg( "Speed 180 deg" );
    yAxisNameNeg( "Speed 270 deg");
    xFormatString( "%.2f" );
    yFormatString( "%.2f" );
    tickCount( rcIPair( 20, 20 ) );
}

// 2D widget persistence compass mode render info class
rcPersistenceCompassTrackRenderInfo::rcPersistenceCompassTrackRenderInfo( int trackGroupNo,
                                                                          int trackNo,
                                                                          rcTrack* track,
                                                                          const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, rcFPair( 0.0, 0.0 ), rcFPair( 1.0, 360.0 ), dataRect )
{
    init();
}

rcPersistenceCompassTrackRenderInfo::rcPersistenceCompassTrackRenderInfo( int trackGroupNo,
                                                                          int trackNo,
                                                                          rcTrack* track,
                                                                          const rcFPair& min,
                                                                          const rcFPair& max,
                                                                          const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, min, max, dataRect )
{
    init();
}

// private

void rcPersistenceCompassTrackRenderInfo::init()
{
    xAxisName( "Angle 0 deg" );
    yAxisName( "Angle 90 deg");
    xAxisNameNeg( "Angle 180 deg" );
    yAxisNameNeg( "Angle 270 deg");
    xFormatString( "%.1f" );
    yFormatString( "%.1f" );
    tickCount( rcIPair( 0, 0 ) );
}

// 2D widget persistence histogram mode render info class

rcPersistenceHistogramTrackRenderInfo::rcPersistenceHistogramTrackRenderInfo( int trackGroupNo,
                                                                              int trackNo,
                                                                              rcTrack* track,
                                                                              const rcFPair& min,
                                                                              const rcFPair& max,
                                                                              const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, min, max, dataRect )
{
    init( max );
}

// private

void rcPersistenceHistogramTrackRenderInfo::init( const rcFPair& max )
{
    rmUnused( max );
    
    xAxisName( "Persistence" );
    yAxisName( "Percentage");
    xAxisNameNeg( "Persistence" );
    yAxisNameNeg( "Percentage");
    xFormatString( "%.1f" );
    yFormatString( "%.1f%%" );

    tickCount( rcIPair( 20, 20 ) );
}

// 2D widget distance track render info class

rcDistanceTrackRenderInfo::rcDistanceTrackRenderInfo( int trackGroupNo,
                                                      int trackNo,
                                                      rcTrack* track,
                                                      const rcFPair& min,
                                                      const rcFPair& max,
                                                      const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, min, max, dataRect )
{
    init( max );
}

// private

void rcDistanceTrackRenderInfo::init( const rcFPair& max )
{
    rmUnused( max );
    
    xAxisName( "Cell #" );
    yAxisName( "Distance");
    xAxisNameNeg( "Cell #" );
    yAxisNameNeg( "Distance");
    xFormatString( "%.0f" );
    yFormatString( "%.2f" );
    int32 maxCells = static_cast<int32>(max.x());
    if ( maxCells > 25 )
        maxCells = 25;
    tickCount( rcIPair( maxCells, 20 ) );
}



// 2D widget persistence compass mode render info class
rcACICompassTrackRenderInfo::rcACICompassTrackRenderInfo( int trackGroupNo,
                                                                          int trackNo,
                                                                          rcTrack* track,
                                                                          const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, rcFPair( 0.0, 0.0 ), rcFPair( 1.0, 360.0 ), dataRect )
{
    init();
}

rcACICompassTrackRenderInfo::rcACICompassTrackRenderInfo( int trackGroupNo,
                                                                          int trackNo,
                                                                          rcTrack* track,
                                                                          const rcFPair& min,
                                                                          const rcFPair& max,
                                                                          const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, min, max, dataRect )
{
    init();
}

// private

void rcACICompassTrackRenderInfo::init()
{
    xAxisName( "Angle 0 deg" );
    yAxisName( "Angle 90 deg");
    xAxisNameNeg( "Angle 180 deg" );
    yAxisNameNeg( "Angle 270 deg");
    xFormatString( "%.1f" );
    yFormatString( "%.1f" );
    tickCount( rcIPair( 0, 0 ) );
}

// 2D widget persistence histogram mode render info class

rcACIHistogramTrackRenderInfo::rcACIHistogramTrackRenderInfo( int trackGroupNo,
                                                                              int trackNo,
                                                                              rcTrack* track,
                                                                              const rcFPair& min,
                                                                              const rcFPair& max,
                                                                              const QRect& dataRect ) :
        rc2DTrackRenderInfo( trackGroupNo, trackNo, track, min, max, dataRect )
{
    init( max );
}

// private

void rcACIHistogramTrackRenderInfo::init( const rcFPair& max )
{
    rmUnused( max );
    
    xAxisName( "ACI" );
    yAxisName( "Percentage");
    xAxisNameNeg( "ACI" );
    yAxisNameNeg( "Percentage");
    xFormatString( "%.2f" );
    yFormatString( "%.1f%%" );

    tickCount( rcIPair( 20, 20 ) );
}
