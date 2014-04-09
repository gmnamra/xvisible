
#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include <rc_uitypes.h>

#include "rc_modeldomain.h"

#include "rc_statusbar.h"

rcStatusBar::rcStatusBar( QWidget* parent, const char* name, Qt::WFlags f )
	: QStatusBar( parent, name )
{
    rcUNUSED( f );
	rcModelDomain* domain = rcModelDomain::getModelDomain();

    setSizeGripEnabled( false );
    
    // Model connections
    connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
             this   , SLOT( updateState( rcExperimentState ) ) );

    connect( domain , SIGNAL( updateStatus( const char* ) ) ,
             this   , SLOT( updateStatus( const char* ) ) );
    

    // in case we don't get an initial signal
    updateState( eExperimentEmpty );
}

rcStatusBar::~rcStatusBar()
{
}

void rcStatusBar::updateState( rcExperimentState state )
{
    switch( state ) {
    case eExperimentEmpty:	        // experiment was created but not run
        message("Ready");
        break;

    case eExperimentRunning:		// experiment is running
        message("Running...");
        break;

    case eExperimentPlayback:		// experiment is running
        message("Playing...");
        break;

    case eExperimentEnded:		    // experiment has ended
        message("Ready");
        break;

    case eExperimentLocked:		    // experiment has ended and been saved
        message("Saved");
        break;

    default:
        break;
    }
}

void rcStatusBar::updateStatus( const char* statusString )
{
    if ( statusString ) {
        message( statusString );
    }
    else {
        clear();
    }
}
