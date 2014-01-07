

#include <QtGui>

#include <CPlusTest/CPlusTest.h> 
#include <Carbon/Carbon.h> 
#include <iostream>
#include<stlplus.hpp>
#include <ut.h>
#include <string>
#include <unistd.h>
#include "ut_simd.h"


std::string gTestData ("/Users/arman/WorkingCopies/test-content/");


RU_TEST_DECL(ut_simd,gTestData);
RU_TEST_DECL(ut_setting,gTestData);
RU_TEST_DECL(ut_util,gTestData);
RU_TEST_DECL(ut_visual,gTestData);
RU_TEST_DECL(ut_analysis,gTestData);





using namespace QtConcurrent;

const int iterations = 20;


void spin (int &iteration)
{
	// Get all registered tests and run them.                                    
	TestSuite& allTests = TestSuite::allTests();     	
	
	// Get all registered tests and print their name
//	std::list<Test*>& dtests = allTests.tests();
//	std::list<Test*>::iterator dItr = dtests.begin ();
//	for (; dItr != dtests.end(); dItr++)
//		std::cout << (*dItr)->name () << std::endl;
	
	
	TestRun run;                                                                 
	// Create a log for writing out test results                                  
	TestLog log(std::cerr);                                
	run.addObserver(&log);                                                      
	
	allTests.run(run);
	
	
	qDebug() << "iteration" << iteration << "in thread" << QThread::currentThreadId() ;
	//<< "Ran " << run.runCount() << " tests, " << run.failureCount() 	<< " failed." << std::endl; 
	
}


//void spin(int &iteration)
//{
//    const int work = 1000 * 1000 * 40;
//    volatile int v = 0;
 //   for (int j = 0; j < work; ++j)
 //       ++v;
//}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    // Prepare the vector.
    QVector<int> vector;
    for (int i = 0; i < iterations; ++i)
        vector.append(i);

    // Create a progress dialog.
    QProgressDialog dialog;
    dialog.setLabelText(QString("Progressing using %1 thread(s)...").arg(QThread::idealThreadCount()));
 
    // Create a QFutureWatcher and conncect signals and slots.
    QFutureWatcher<void> futureWatcher;
    QObject::connect(&futureWatcher, SIGNAL(finished()), &dialog, SLOT(reset()));
    QObject::connect(&dialog, SIGNAL(canceled()), &futureWatcher, SLOT(cancel()));
    QObject::connect(&futureWatcher, SIGNAL(progressRangeChanged(int, int)), &dialog, SLOT(setRange(int, int)));
    QObject::connect(&futureWatcher, SIGNAL(progressValueChanged(int)), &dialog, SLOT(setValue(int)));

    // Start the computation.
    futureWatcher.setFuture(QtConcurrent::map(vector, spin));

    // Display the dialog and start the event loop.
    dialog.exec();
    
    futureWatcher.waitForFinished();

    // Query the future to check if was canceled.
    qDebug() << "Canceled?" << futureWatcher.future().isCanceled();
}

