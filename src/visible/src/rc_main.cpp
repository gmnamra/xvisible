/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_main.cpp 7241 2011-02-23 04:13:39Z arman $
 *
 *	This file contains application main function implementation.
 *
 ******************************************************************************/
#include <qapplication.h>
#include <qmessagebox.h>
#include <rc_engineimpl.h>
#include <rc_batchexperimentobserver.h>

#include "rc_main_window.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"
#include <ostream>

#include <rc_resource_ctrl.h>

using namespace legacy_qtime;


// These are declared in rc_appconstants.h
// and defined by each application

// Setting label layout constants
const int cUIsettingLabelWidth = 67;
const int cUIsettingLabelSpacing = 3;

// Application info
const char* const cAppName = "Visible";
const char* const cVersionName = "2.0b";
const char* const cAppBuildDate = __DATE__ " " __TIME__ ;


extern void modelTest( int argc , char** argv );

int main( int argc, char** argv )
{


  rcExecWithShmem::setPathName(argv[0]);

  // Test if this is a bundle app open. Note: Find better ways
  if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'h')
    {
      rcVisibleBatchUsage (cerr);
      return 0;
    }

  if (argc == 1 || (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'p' && argv[1][2] == 's' && argv[1][3] == 'n'))
    {
      cerr << "Starting Visible Application" << endl;



      QApplication app( argc, argv );

      try
	{
	  rcModelDomain modelDomain( &app);

	  rcMainWindow mainwindow;
	  app.setMainWidget( &mainwindow );

	  mainwindow.setCaption( "Untitled" );
	  mainwindow.show();

	  return app.exec();
	}
      catch (general_exception& x)
	{
	  fprintf( stderr, "Caught general_exception %s %s %d\n", x.what(), x.GetFile (), x.GetLine () );
	  QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
	  exit( 1 );
	}
      catch (...)
	{
	  fprintf( stderr, "Caught unknown exception \n" );
	  QMessageBox::critical( 0 , cAppName , "Unknown exception" , 1 , 0 );
	  exit( 1 );
	}
    }

  try
    {
      cerr << "Not Implemented Yet: Visible Command Line" << endl;
      developerDebugging (false); // not sure this is necessary but conceptually it needs to be done

#if 0
      // Set application (generator) info
      rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
      // Concatenate application name, version and build date
      std::string comment = cAppName;
      comment += " ";
      comment += cVersionName;
      comment += " ";
      comment += cAppBuildDate;
      persistenceManager->setGeneratorComment( comment );

      // Use default domain implementation
      rcExperimentDomainImpl* domain = dynamic_cast<rcExperimentDomainImpl*>(rcExperimentDomainFactory::getExperimentDomain());
      if (domain == 0)
	rmExceptionMacro ( << "Can't find experiment domain" );

      // Create a batch observer instance and initialize the domain
      rcBatchExperimentObserver observer;
      // Initialize domain. Also gets an Engine. Engine is a singleton
      domain->initialize( &observer);

      // Start new experiment
      domain->newExperiment();

      // Setup engine from Args
      rcEngineImpl* engine = dynamic_cast<rcEngineImpl*>(rcEngineFactory::getEngine( NULL ));
      engine->setFromArgs (argc, argv);

      // Perform analysis
      domain->startExperiment();  // Calls engine->start ();
      domain->processExperiment(); // Calls engine->process (); starts analyzer thread
      domain->stopExperiment(); // Calls engine->stop ();

      // Save the Experiment: locks the experiment
      domain->saveExperiment (engine->outputFile().c_str(), engine->outputFormat(), NULL);

      // Shut down domain
      domain->shutdown();
#endif
    }

  catch (general_exception& x)
    {
      cerr << "Caught general_exception: " << x.what() << endl;
      exit( 1 );
    }
  catch (...)
    {
      cerr << "Caught unknown exception" << endl;
      exit( 1 );
    }
}

