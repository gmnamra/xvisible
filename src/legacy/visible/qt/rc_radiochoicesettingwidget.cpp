/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_radiochoicesettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*
******************************************************************************/

#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include "rc_choiceradiobutton.h"
#include "rc_radiochoicesettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

rcRadioChoiceSettingWidget::rcRadioChoiceSettingWidget( QWidget* parent, const rcSettingInfo& setting )
	: rcSettingWidget( parent, setting )
{
	QBoxLayout* radioLayout = new QHBoxLayout( mTopLayout );
	int nChoices = setting.getNChoices();

	for (int i = 0; i < nChoices; i++)
	{
		rcSettingChoice choice = setting.getChoice( i );
		rcChoiceRadioButton* widget = new rcChoiceRadioButton( this , choice );
		connect( widget , SIGNAL( choiceSelected( int ) ) , this , SLOT( choiceSelected( int ) ) );
		radioLayout->addWidget( widget );
		QToolTip::add( widget , choice.getDescription() );
	}

	// update the radio buttons with the current value of the setting.
	settingChanged();

	// connect this widget to be notified by the updateSettings()
	//	signal from the model domain
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );
}

rcRadioChoiceSettingWidget::~rcRadioChoiceSettingWidget()
{
    // Remove all tool tips
    QObjectList list = queryList( "rcChoiceRadioButton" , 0, false , false );
	for (unsigned int i = 0; i < list.count(); i++)
	{
		rcChoiceRadioButton* button = (rcChoiceRadioButton*) list.at( i );
        QToolTip::remove( button );    
	}
}

// All radio button widgets' choiceSelected(int) signal are
//	routed to this slot, which sets the new setting value and
//	then forwards to the settingChanged() slot to update the
//	radio button display.
void rcRadioChoiceSettingWidget::choiceSelected( int choiceValue )
{
	mSetting.setValue( choiceValue );

	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// Since we're not using a QButtonGroup, we need to handle the
//	radio button "exclusivity" ourselves.  When the setting
//	changes, update the state of all radio buttons so that
//	only the one representing the current value of the setting
//	is "turned on".
void rcRadioChoiceSettingWidget::settingChanged( void )
{
	int currentValue = mSetting.getValue();
	QObjectList list = queryList( "rcChoiceRadioButton" , 0, false , false );
	for (unsigned int i = 0; i < list.count(); i++)
	{
		rcChoiceRadioButton* button = (rcChoiceRadioButton*) list.at( i );
		button->setChecked( currentValue == button->getChoiceValue() );
        
	}

    // Update visibility/editability
    updateWidgetState();

    // Hack alert: this is brittle 
    // Send request for input source only
    if ( ! strcmp( mSetting.getTag(), "input-source" ) )
        rcModelDomain::getModelDomain()->requestInputSource( currentValue );
}

