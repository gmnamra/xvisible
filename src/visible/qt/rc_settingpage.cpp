// @file
#include <qapplication.h>
#include <qlayout.h> 
#include <q3groupbox.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3BoxLayout>
#include <Q3VBoxLayout>
#include <rc_sparsehist.h>
#include <rc_macro.h>

#include <rc_modeldomain.h>
#include "rc_textlinesettingwidget.h"
#include "rc_textareasettingwidget.h"
#include "rc_checkboxsettingwidget.h"
#include "rc_radiochoicesettingwidget.h"
#include "rc_menuchoicesettingwidget.h"
#include "rc_filechoicesettingwidget.h"
#include "rc_playbacksettingwidget.h"
#include "rc_filesavesettingwidget.h"
#include "rc_rectsettingwidget.h"
#include "rc_spinboxsettingwidget.h"
#include "rc_frameratechoicesettingwidget.h"
#include "rc_thumbwheelsettingwidget.h"
#include "rc_textchoicesettingwidget.h"
#include "rc_numericchoicesettingwidget.h"
#include "rc_settingpage.h"
#include "rc_appconstants.h"


rcSettingPage::rcSettingPage( QWidget* parent, const rcSettingCategory& category )
	: QWidget( parent, category.getTag() )
{
	_category = category;

    // Make the top-level layout; a vertical box to contain all widgets
    // and sub-layouts.
    Q3BoxLayout* layout = new Q3VBoxLayout( this , cUIsettingLabelSpacing/2, cUIsettingLabelSpacing );

    // Findout if we have a group of checkboxes or pairs of spin box
    int nSettings = _category.getNSettings();
    rcSparseHistogram checkgids, spingids;

    for (int i = 0; i < nSettings; i++)
      {
	rcSettingInfo setting = _category.getSettingInfo( i );

	switch (setting.getDisplayType())
	  {
	  case eCheckbox:		// setting is an on/off value
	    checkgids.add (setting.getGroupId ());
	    break;
            
	  case eSpinbox:			// setting is a spinbox (int value)
	    spingids.add (setting.getGroupId ());
	    break;
	  }
      }

    rcSparseHistogram::sparseArray mapRef = checkgids.getArray();
    rcSparseHistogram::sparseArray::const_iterator start = mapRef.begin();
    rcSparseHistogram::sparseArray::const_iterator end = mapRef.end();
    
    // If there are groups of checkboxes with 4 or more checkboxes 
    // put them in a grid layout.
    vector<int32> checks;
    for (; start != end; start++)
      {
	if ((*start).second >= 4) checks.push_back ((*start).first);
      }

    // Add a setting widget for each setting in the category
    nSettings = _category.getNSettings();
    for (int i = 0; i < nSettings; i++)
      {
	rcSettingInfo setting = _category.getSettingInfo( i );
	QWidget* widget = 0;

	// Any group laid out separately skip
	bool valid (true);
	for (uint32 gr = 0; gr < checks.size(); gr++)
	  if (setting.getGroupId() == checks[gr])
	    {
	      valid = false;
	      break;
	    }

 	if (!valid) continue;

	switch (setting.getDisplayType())
	  {
	  case eRect:
	    widget = new rcRectSettingWidget( this , setting );
	    break;

	  case eTextLine:		// setting is a single line of text.
	    widget = new rcTextLineSettingWidget( this , setting );
	    break;

	  case eTextArea:		// setting may contain multiple lines of text
	    widget = new rcTextAreaSettingWidget( this , setting );
	    break;

	  case eCheckbox:		// setting is an on/off value
	    widget = new rcCheckboxSettingWidget( this , setting );
	    break;

	  case eRadioChoice:	// setting is a multi-choice value
	    widget = new rcRadioChoiceSettingWidget( this , setting );
	    break;

	  case eMenuChoice:	// setting is a multi-choice value
	    widget = new rcMenuChoiceSettingWidget( this , setting );
	    break;

	  case eFileChoice:		// setting is a file name (text + "browse" button)
	  case eMultiFileChoice:	// setting is a semicolon separated list of file names (text + "browse" button)
	  case eDirectoryChoice:	// setting is a directory name (text + "browse" button)
	    widget = new rcFileChoiceSettingWidget( this , setting );
	    break;

	  case eFileSaveChoice:  // setting is a file name (text + "browse" button)
	    widget = new rcFileSaveSettingWidget( this , setting );
	    break;
            
	  case eSpinbox:			// setting is a spinbox (int value)
	    widget = new rcSpinboxSettingWidget( this , setting );
	    break;

	  case eFramerateChoice:	// setting is a special frame rate multi-choice/line-edit value
            widget = new rcFramerateChoiceSettingWidget( this , setting );
            break;

	  case eRateChoice:	// setting is a special rate multi-choice/line-edit value
            widget = new rcRateChoiceSettingWidget( this , setting );
            break;
            
	  case ePlaybackChoice:	// setting is a video playback control
            widget = new rcPlaybackSettingWidget( this , setting );
            break;

	  case eThumbWheel: // setting is a thumb wheel
            widget = new rcThumbWheelSettingWidget( this, setting );
            break;

	  case eTextChoice:	// setting is a text multi-choice/line-edit value
            widget = new rcTextChoiceSettingWidget( this , setting );
            break;

	  case eNumericChoice:	// setting is a numeric multi-choice/line-edit value
            widget = new rcNumericChoiceSettingWidget( this , setting );
            break;
            
	  case ePoint:		// setting is a rectangle (text + "define" button)
	  default:
	    rmExceptionMacro(<< "unknown setting type" );
	  }
	layout->addWidget( widget );
      }

    if (checks.size())
      {
	for (uint32 gr = 0; gr < checks.size(); gr++)
	  {
	    mapRef = checkgids.getArray();
	    start = mapRef.begin();
	    end = mapRef.end();

	    uint32 count;
	    for (; start != end; start++)
	      if ((*start).first == checks[gr]) 
		count = (*start).second;


	    QGridLayout *grid = new QGridLayout (layout, 2, 3, 10 );

	    // Add checkboxes belonging to this group
	    nSettings = _category.getNSettings();
	    for (int i = 0, j = 0; i < nSettings; i++)
	      {
		rcSettingInfo setting = _category.getSettingInfo( i );
		QWidget* widget = 0;
		// this group
		if (setting.getGroupId() != checks[gr]) continue;
		if (setting.getDisplayType() == eCheckbox)
		  {
		    widget = new rcCheckboxSettingWidget( this , setting );
		    grid->addWidget (widget, j / 3, j % 3);
		    j++;
		  }
	      }
	  }
      }

    layout->addStretch();
}


rcSettingPage::rcSettingPage( QWidget* parent, QWidget* child)
	: QWidget( parent)
{
    // Make the top-level layout; a vertical box to contain all widgets
    // and sub-layouts.
    Q3BoxLayout* layout = new Q3VBoxLayout( this , cUIsettingLabelSpacing/2, cUIsettingLabelSpacing );
    child->reparent (this, child->geometry().topLeft (), false);
    layout->addWidget(child);
    layout->addStretch();
}

rcSettingPage::~rcSettingPage()
{
}
