/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/10/13 14:42:38  arman
 *moving track render in to its own file
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcTRACKRENDER_H
#define __rcTRACKRENDER_H


//
// Track rendering information classes
//
// TODO: complete refactoring needed to remove redundancy and improve efficiency

// Generic rendering info base class
class rcTrackRenderInfo
{
  public:
    rcTrackRenderInfo( int trackGroupNo,
                       int trackNo,
                       rcTrack* track,
                       rcTimestamp startTime,
                       rcTimestamp endTime,
                       const QRect& dataRect, bool logDisplay = false );
    
    // TODO: make these members private
    rcTrack*	_track;
    rcTimestamp	_startTime;
    rcTimestamp	_endTime;
    double		_minValue;
    double		_maxValue;
    QRect		_dataRect;
    QPen        _tickMarkPen;
    QPen		_dataPen;
    bool		_isEnabled;
    bool		_isHilited;
    QFont       _tickMarkFont;
    bool        _logDisplay;
};

// 2D widget render info base class
class rc2DTrackRenderInfo : public rcTrackRenderInfo
{
  public:
    rc2DTrackRenderInfo( int trackGroupNo,
                         int trackNo,
                         rcTrack* track,
                         const rcFPair& min,
                         const rcFPair& max,
                         const QRect& dataRect );
    
    // Accessors
    const rcFPair& minValue() const { return mMinValue; }
    const rcFPair& maxValue() const { return mMaxValue; }
    const std::string& xAxisName() const { return mXAxisName; }
    const std::string& yAxisName() const { return mYAxisName; }
    const std::string& xAxisNameNeg() const { return mXAxisNameNeg; }
    const std::string& yAxisNameNeg() const { return mYAxisNameNeg; }
    const std::string& xFormatString() const { return mXFormatString; }
    const std::string& yFormatString() const { return mYFormatString; }
    const rcIPair& tickCount() const { return mTickCount; }

    // Mutators
    void minValue( const rcFPair& v ) { mMinValue = v; }
    void maxValue(  const rcFPair& v ) { mMaxValue = v; }
    void xAxisName( const std::string& n ) { mXAxisName = n; }
    void yAxisName( const std::string& n ) { mYAxisName = n; }
    void xAxisNameNeg( const std::string& n ) { mXAxisNameNeg = n; }
    void yAxisNameNeg( const std::string& n ) { mYAxisNameNeg = n; }
    void xFormatString( const std::string& s ) { mXFormatString = s; }
    void yFormatString( const std::string& s ) { mYFormatString = s; }
    void tickCount( const rcIPair& c ) { mTickCount = c; }
    
  protected:
    rcFPair mMinValue;      // Scale min value
    rcFPair mMaxValue;      // Scale max value
    std::string mXAxisName;    // X axis name
    std::string mYAxisName;    // Y axis name
    std::string mXAxisNameNeg;    // negative X axis name
    std::string mYAxisNameNeg;    // negative Y axis name
    std::string mXFormatString;// Format string for X axis scale ticks
    std::string mYFormatString;// Format string for Y axis scale ticks
    rcIPair mTickCount;     // Number of scale ticks for X and Y axes
};

// 2D widget position mode render info class
class rcPositionTrackRenderInfo : public rc2DTrackRenderInfo
{
  public:
    rcPositionTrackRenderInfo( int trackGroupNo,
                               int trackNo,
                               rcTrack* track,
                               const QRect& dataRect );
    
    rcPositionTrackRenderInfo( int trackGroupNo,
                               int trackNo,
                               rcTrack* track,
                               const rcFPair& min,
                               const rcFPair& max,
                               const QRect& dataRect );
};

// 2D widget speed+direction mode render info class
class rcSpeedDirectionTrackRenderInfo : public rc2DTrackRenderInfo
{
  public:
    rcSpeedDirectionTrackRenderInfo( int trackGroupNo,
                                     int trackNo,
                                     rcTrack* track,
                                     const QRect& dataRect );

    rcSpeedDirectionTrackRenderInfo( int trackGroupNo,
                                     int trackNo,
                                     rcTrack* track,
                                     const rcFPair& min,
                                     const rcFPair& max,
                                     const QRect& dataRect );

  private:
    void init();
};

// 2D widget speed+direction in a 360 degree compass mode render info class
class rcSpeedDirectionCompassTrackRenderInfo : public rc2DTrackRenderInfo
{
  public:
    rcSpeedDirectionCompassTrackRenderInfo( int trackGroupNo,
                                             int trackNo,
                                             rcTrack* track,
                                             const QRect& dataRect );

    rcSpeedDirectionCompassTrackRenderInfo( int trackGroupNo,
                                             int trackNo,
                                             rcTrack* track,
                                             const rcFPair& min,
                                             const rcFPair& max,
                                             const QRect& dataRect );

  private:
    void init();
};

// 2D widget persistence direction in a 360 degree compass mode render info class
class rcPersistenceCompassTrackRenderInfo : public rc2DTrackRenderInfo
{
  public:
    rcPersistenceCompassTrackRenderInfo( int trackGroupNo,
                                          int trackNo,
                                          rcTrack* track,
                                          const QRect& dataRect );

    rcPersistenceCompassTrackRenderInfo( int trackGroupNo,
                                          int trackNo,
                                          rcTrack* track,
                                          const rcFPair& min,
                                          const rcFPair& max,
                                          const QRect& dataRect );

  private:
    void init();
};


// 2D widget persistence histogram render info class
class rcPersistenceHistogramTrackRenderInfo : public rc2DTrackRenderInfo
{
  public:
    rcPersistenceHistogramTrackRenderInfo( int trackGroupNo,
                                           int trackNo,
                                           rcTrack* track,
                                           const rcFPair& min,
                                           const rcFPair& max,
                                           const QRect& dataRect );

  private:
    void init( const rcFPair& max );
};

// 2D widget ACIdirection in a 360 degree compass mode render info class
class rcACICompassTrackRenderInfo : public rc2DTrackRenderInfo
{
  public:
    rcACICompassTrackRenderInfo( int trackGroupNo,
                                          int trackNo,
                                          rcTrack* track,
                                          const QRect& dataRect );

    rcACICompassTrackRenderInfo( int trackGroupNo,
                                          int trackNo,
                                          rcTrack* track,
                                          const rcFPair& min,
                                          const rcFPair& max,
                                          const QRect& dataRect );

  private:
    void init();
};

// 2D widget ACI histogram render info class
class rcACIHistogramTrackRenderInfo : public rc2DTrackRenderInfo
{
  public:
    rcACIHistogramTrackRenderInfo( int trackGroupNo,
                                           int trackNo,
                                           rcTrack* track,
                                           const rcFPair& min,
                                           const rcFPair& max,
                                           const QRect& dataRect );

  private:
    void init( const rcFPair& max );
};

// 2D widget distance render info class
class rcDistanceTrackRenderInfo : public rc2DTrackRenderInfo
{
  public:
    rcDistanceTrackRenderInfo( int trackGroupNo,
                               int trackNo,
                               rcTrack* track,
                               const rcFPair& min,
                               const rcFPair& max,
                               const QRect& dataRect );

  private:
    void init( const rcFPair& max );
};


#endif /* __rcTRACKRENDER_H */
