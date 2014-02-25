#include <qapplication.h>
#include <qlayout.h> 

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
    QBoxLayout* layout = new QVBoxLayout( this , cUIsettingLabelSpacing/2, cUIsettingLabelSpacing );
    layout->setContentsMargins(7, 7, 7, 7);

	// Add a setting widget for each setting in the category
	int nSettings = _category.getNSettings();
	for (int i = 0; i < nSettings; i++)
	{
		rcSettingInfo setting = _category.getSettingInfo( i );
		QWidget* widget = 0;
		switch (setting.getDisplayType())
		{
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

		case eRect:			// setting is a rectangle (text + "define" button)
		//	widget = new rcRectSettingWidget( this , setting );
			break;
            
		case eSpinbox:			// setting is a spinbox (int value)
			widget = new rcSpinboxSettingWidget( this , setting );
			break;

        case eFramerateChoice:	// setting is a special frame rate multi-choice/line-edit value
            widget = new rcFramerateChoiceSettingWidget( this , setting );
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

	layout->addStretch();
}

rcSettingPage::~rcSettingPage()
{
}
