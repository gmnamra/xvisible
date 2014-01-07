#include <qapplication.h>
#include <qmessagebox.h> 

#include <rc_mainwindow.h>
#include <rc_modeldomain.h>

#include "rc_appconstants.h"



int main( int argc, char** argv )
{
	QApplication app( argc, argv );

	try
	{
		rcModelDomain modelDomain( &app , "domain" );

		rcMainWindow mainwindow;
		app.setMainWidget( &mainwindow );

		mainwindow.show();

		return app.exec();
	}
	catch (general_exception& x)
	{
        fprintf( stderr, "Caught rcExeption %s\n", x.what() );
		QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
		exit( 1 );
	}
	catch (exception& x)
	{
        fprintf( stderr, "Caught exeption %s\n", x.what() );
		QMessageBox::critical( 0 , cAppName , x.what() , 1 , 0 );
		exit( 1 );
	}
    catch (...)
    {
        fprintf( stderr, "Caught unknown exeption \n" );
        QMessageBox::critical( 0 , cAppName , "Unknown exception" , 1 , 0 );
        exit( 1 );
    }
    
}

