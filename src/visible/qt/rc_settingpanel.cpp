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
		rcSettingCategory category = experiment->getSettingCategory( i );
		QWidget* tab = new rcSettingPage( this, category );
		insertTab( tab , category.getName() );

		setTabToolTip( tab , category.getDescription() );
	}

    // Restore selected tab
    if ( currentPage >= 0 )
        setCurrentPage( currentPage );
    show();
}

