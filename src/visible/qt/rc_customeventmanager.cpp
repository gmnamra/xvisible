/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_customeventmanager.cpp 6747 2009-03-26 23:10:04Z arman $
 *
 *	This file contains custom event queue manager implementation.
 *
 *****************************************************************************/
#include <iostream>
#include <qapplication.h>
#include <QMutexLocker>
#include "rc_customeventmanager.h"

using namespace std;

//
// Public methods
//

// Ctor
rcCustomEventManager::rcCustomEventManager( QObject* receiver ) : mReceiver( receiver ), mutex  ()
        
{
	 QMutexLocker locker(&mutex);
	
    // Initialize queue counts
    for ( uint32 i = 0; i < rmDim( mQueueCount ); ++i )
        mQueueCount[i] = 0;
}

// Notification that this event has been processed
// Returns number of remaining unprocessed events of this type
int32 rcCustomEventManager::processedEvent( QEvent* e )
{
	return processedEvent( static_cast<rcCustomEvent> (e->type()));
}

// Returns number of events of this type that have been posted but not yet processed
int32 rcCustomEventManager::queuedEvents( rcCustomEvent type )
{
	 QMutexLocker locker(&mutex);

    return mQueueCount[type];
}

// Post event to application.
void rcCustomEventManager::postEvent( QEvent* e )
{
	rcCustomEvent type = static_cast<rcCustomEvent> (e->type());
    
    int32 unprocessed = postedEvent( type );

#ifdef DEBUG_LOG    
    if ( unprocessed > 1 ) {
        cerr << "postEvent(" << type <<  ") " << unprocessed << " events already queued" << endl;
    }
#else
    rmUnused( unprocessed );
#endif
    
    QApplication::postEvent( mReceiver, e );
}

//
// Private methods
//

// Notification that an event of this type has been posted
// Returns number of posted events of this type
int32 rcCustomEventManager::postedEvent( rcCustomEvent type )
{
	 QMutexLocker locker(&mutex);
    rmAssert( type > eNotifyMinEvent && type < eNotifyMaxEvent );
    
    return ++mQueueCount[type];
}

// Notification that an event of this type has been processed
// Returns number of posted events of this type
int32 rcCustomEventManager::processedEvent( rcCustomEvent type )
{
	 QMutexLocker locker(&mutex);
	rmAssert( type > eNotifyMinEvent && type < eNotifyMaxEvent );
    
    if ( mQueueCount[type] <= 0 )
        cerr << "rcCustomEventManager::processedEvent: rcCustomEvent(" << type << ") invalid queue count " <<  mQueueCount[type] << endl;
    return --mQueueCount[type];
}
