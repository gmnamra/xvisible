/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_customeventmanager.h 7179 2011-02-05 22:25:05Z arman $
 *
 *	This file contains custom event queue manager definition.
 *
 *****************************************************************************/
#ifndef _UI_rcCUSTOMEVENTMGR_H_
#define _UI_rcCUSTOMEVENTMGR_H_
#include <rc_timestamp.h>
#include <rc_model.h>
#include <QMutex>
#include <QEvent>
#include "lpwidget.h"
#include <rc_uitypes.h>
// Forward declarations

class QObjcect;




// Local types

enum rcCustomEvent
{
    eNotifyMinEvent = QEvent::User // Sentinel
  , eNotifyTimeEvent
  , eNotifyCursorEvent
  , eNotifyErrorEvent
  , eNotifyWarningEvent
  , eNotifyStatusEvent
  , eNotifyStateEvent
  , eNotifyTimelineRangeEvent
  , eNotifyBlitEvent
  , eNotifyBlitGraphicsEvent
  , eNotifyAnalysisRectEvent
  , eNotifyMultiplierEvent
  , eNotifyExperimentChange
  , eNotifyVideoRectEvent
  , eNotifyAnalysisRectRotationEvent
  , eNotifyPlotRequestEvent
  , eNotifyPlot2dRequestEvent    
  , eNotifyPolygonGroupEvent
  , eNotifyMaxEvent // Sentinel
};

template <class T, int id>
class rcBaseEvent: public QEvent
	{
	public:
		rcBaseEvent( const T& d) : QEvent((QEvent::Type)id), payLoad(d) {}
		void clearData () { payLoad = T (); }
		const T myData() const { return payLoad; }
	private:
		const T payLoad; // passed by copy
	};


typedef rcBaseEvent<rcTimestamp, eNotifyTimeEvent> rcBaseTimeEvent;
typedef rcBaseEvent<rcTimestamp, eNotifyCursorEvent> rcBaseCursorEvent;
typedef rcBaseEvent<QString, eNotifyErrorEvent> rcBaseErrorEvent;
typedef rcBaseEvent<QString, eNotifyWarningEvent> rcBaseWarningEvent;
typedef rcBaseEvent<QString, eNotifyStatusEvent> rcBaseStatusEvent;
typedef rcBaseEvent<rcExperimentState, eNotifyStateEvent> rcBaseStateEvent;
typedef rcBaseEvent<rcTimestampPair, eNotifyTimelineRangeEvent> rcBaseTimelineRangeEvent;
typedef rcBaseEvent<rcWindow, eNotifyBlitEvent> rcBaseBlitEvent;
typedef rcBaseEvent<rcVisualGraphicsCollection, eNotifyBlitGraphicsEvent> rcBaseBlitGraphicsEvent;
typedef rcBaseEvent<rcRect, eNotifyAnalysisRectEvent> rcBaseAnalysisRectEvent;
typedef rcBaseEvent<rcRect, eNotifyVideoRectEvent> rcBaseVideoRectEvent;
typedef rcBaseEvent<double, eNotifyMultiplierEvent> rcBaseMultiplierEvent;
typedef rcBaseEvent<rcAffineRectangle, eNotifyAnalysisRectRotationEvent> rcBaseAnalysisRectRotationEvent;
typedef rcBaseEvent<int32, eNotifyExperimentChange> rcBaseExperimentChangeEvent;
typedef rcBaseEvent<SharedCurveDataRef, eNotifyPlotRequestEvent> rcBasePlotRequestEvent;
typedef rcBaseEvent<SharedCurveData2dRef, eNotifyPlot2dRequestEvent> rcBasePlot2dRequestEvent;
typedef rcBaseEvent<rcPolygonGroupRef, eNotifyPolygonGroupEvent> rcBasePolysEvent;


//
// Class for custom event (queue) management
//

class rcCustomEventManager
{
  public:
    rcCustomEventManager( QObject* receiver );
    
    // Returns number of events of this type that have been posted but not yet processed
    int32 queuedEvents( rcCustomEvent type );

    // Post event to application
    void postEvent( QEvent* e );

    // Notification that an event has been processed
    // Returns number of remaaining unprocessed events of this type
    int32 processedEvent( QEvent* e );
    
  private:
    // Notification that an event of this type has been posted
    // Returns number of posted events of this type
    int32 postedEvent( rcCustomEvent type );
    // Notification that an event of this type has been processed
    // Returns number of remaining unprocessed events of this type
    int32 processedEvent( rcCustomEvent type );
    
    QObject*       mReceiver;                    // Receiver of events
    QMutex        mutex;                  // Mutex to protect mQueueCount
    int32        mQueueCount[eNotifyMaxEvent]; // Unprocessed event counts
};


#endif _UI_rcCUSTOMEVENTMGR_H_

