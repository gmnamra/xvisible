#include <qapplication.h>
#include <qtooltip.h>
#include <qobject.h>

#include "rc_model.h"

#include "rc_menubar.h"
#include "rc_modeldomain.h"
#include "rc_settingpage.h"
#include "rc_settingpanel.h"


rcSettingPanel::rcSettingPanel( QWidget* parent, const char* name, Qt::WFlags f )
	: QTabWidget( parent, name )
{
    rcUNUSED( f );
	setMinimumWidth( 430 );
    updatePages();

	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
			 this   , SLOT( updateState( rcExperimentState ) ) );
    connect( domain , SIGNAL( updateDebugging( void ) ) ,
             this   , SLOT( updatePages( void ) ) );
#ifdef HIPPOW
    connect ( domain, SIGNAL (updateTrackingDisplayGL (void)),
	     this, SLOT (requestTrackingDisplayGL(void)));
#endif
}

rcSettingPanel::~rcSettingPanel()
{
}

void rcSettingPanel::updateState( rcExperimentState state )
{
    switch( state ) {
        case eExperimentEmpty:          // experiment was created but not run
            updatePages();
        case eExperimentRunning:        // experiment is running
        case eExperimentPlayback:       // experiment is playing back
        case eExperimentEnded:          // experiment has ended
        case eExperimentLocked:         // experiment has ended and been saved
        default:
            break;
    }
}

#ifdef HIPPOW
// Request OpenGL tracking window display
// @todo create new pages in the canvas for different trackplots
void rcSettingPanel::requestTrackingDisplayGL()
{
  rcModelDomain* domain = rcModelDomain::getModelDomain();
  CanvasWindow * window = new CanvasWindow (0, "Untitled", 0);
  window -> show ();
  window->setGeometry (50, 50, 480, 320);
  domain-> getHippWindowCtl ()-> createInspector ();
  domain-> getHippWindowCtl ()->newWindow (window);
  domain-> getHippWindowCtl ()->quitOnLastWindowClose ();

  QWidget* tab = new rcSettingPage( this,
				    domain-> getHippWindowCtl ()-> getInspector ());
  insertTab( tab , QString ("Track Inspector"));
  setTabToolTip(tab, QString ("Track Visualization"));
}
#endif

void rcSettingPanel::updatePages( void )
{
    int i;
    // Save currently selected tab
    int currentPage = currentPageIndex();

    hide();
    // Delete any previous settings pages
    for (i = count() - 1; i >= 0 ; i--)
    {
        QWidget* aPage = page( i );
        removePage( aPage );
        delete aPage;
    }
    
    // add the new settings pages
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	rcExperiment* experiment = domain->getExperiment();
	int nPages = experiment->getNSettingCategories();
	for (i = 0; i < nPages; i++)
	{
	  rcSettingCategory category = experiment->getSettingCategory(i);
	  if (category.getName () == "Capture")
	    {
	      QWidget* tab = new rcSettingPage( this, category );
	      insertTab( tab , category.getName() );
	      setTabToolTip( tab , category.getDescription() );
	    }
	}


	for (i = 0; i < nPages; i++)
	{
	  rcSettingCategory category = experiment->getSettingCategory(i);
	  if (category.getName () == "Analyze")
	    {
	      QWidget* tab = new rcSettingPage( this, category );
	      insertTab( tab , category.getName() );
	      setTabToolTip( tab , category.getDescription() );
	    }
	}

	for (i = 0; i < nPages; i++)
	{
	  rcSettingCategory category = experiment->getSettingCategory(i);
	  if (category.getName () == "Visualize")
	    {
	      QWidget* tab = new rcSettingPage( this, category );
	      insertTab( tab , category.getName() );
	      setTabToolTip( tab , category.getDescription() );
	    }
	}

	for (i = 0; i < nPages; i++)
	{
	  rcSettingCategory category = experiment->getSettingCategory(i);
	  if (category.getName () == "Experiment")
	    {
	      QWidget* tab = new rcSettingPage( this, category );
	      insertTab( tab , category.getName() );
	      setTabToolTip( tab , category.getDescription() );
	    }
	}


    // Restore selected tab
    if ( currentPage >= 0 )
        setCurrentPage( currentPage );
    show();
}

