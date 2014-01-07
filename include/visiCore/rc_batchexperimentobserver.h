/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.2  2004/06/20 22:56:04  arman
 *moved to silence
 *
 *Revision 1.1  2004/06/16 18:40:30  arman
 *batch experimentor
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcBATCHEXPERIMENTOBSERVER_H
#define __rcBATCHEXPERIMENTOBSERVER_H

#include <rc_experimentdomainimpl.h>


static bool silent (true);

class rcBatchExperimentObserver : public rcExperimentObserver
{
public:
	rcBatchExperimentObserver( void )
	{
        mErrors = 0;
	}

    uint32 errors() {
        return mErrors;
    }
    
	// called to notify the observer that an error occurred.
	virtual void notifyError( const char* errorString )
        {
            if ( !silent ) {
                rcTimestamp now = rcTimestamp::now();
                cerr << "Visible[cmdLine]" << " error at " << now.localtime()
                     << ": " << errorString << endl;
            }

            ++mErrors;
        }

	// called to warn the observer of some condition.
	virtual void notifyWarning( const char* warnString )
        {
            if ( !silent ) {
                rcTimestamp now = rcTimestamp::now();
                cerr << "Visible[cmdLine]" << " warning at " << now.localtime()
                     << ": " << warnString << endl;
            }
            ++mErrors;
        }

	// called to send status to the observer
	virtual void notifyStatus( const char* statusString )
        {
            if ( !silent ) {
                rcTimestamp now = rcTimestamp::now();
                cerr << "Visible[cmdLine]" << " status at "  << now.localtime()
                     << ": " << statusString << endl;
            }
        }

	// called to notify observer of a state change.
	virtual void notifyState( rcExperimentState newState,
                              bool i )
        {
            rmUnused( i );
            rmUnused( newState );
        }

    // called to notify observer of a time lapse
    virtual void notifyTime( const rcTimestamp& cursorTime )
        {
            rmUnused( cursorTime );
        }
    
    // called to notify observer of programtic update to cursor time
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

    // called to notify observer of a analysis focus rect rotation
    void notifyAnalysisRectRotation( const rcAffineRectangle& affine )
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
            if ( !silent ) 
                cerr << "Visible[cmdLine]"  << " error: notifyBlitData called: " << image << endl;
            ++mErrors;
        }
    
    virtual void notifyBlitGraphics( const rcVisualGraphicsCollection* graphics )
        {
            if ( !silent ) 
                cerr << "Visible[cmdLine]" << " error: notifyBlitGraphics called: " << graphics << endl;
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

  virtual bool acceptingPolys ( void ) { return true;  }
    
  virtual void notifyPolys (  ) {  }

    virtual void getPolys ( rcPolygonGroupRef& polys ) {rmUnused (polys); }
	
  virtual bool doneSelecting ( void ) { return true; }

private:
    uint32 mErrors;
};

#endif /* __rcBATCHEXPERIMENTOBSERVER_H */
