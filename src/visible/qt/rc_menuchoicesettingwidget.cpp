/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_menuchoicesettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*
******************************************************************************/

#include <qapplication.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qlineedit.h>

#include "rc_menuchoicesettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

const int cFrameRateWidth = 80;

rcMenuChoiceSettingWidget::rcMenuChoiceSettingWidget( QWidget* parent, const rcSettingInfo& setting )
	: rcSettingWidget( parent, setting )
{
  mWidget = new QComboBox( false, this , setting.getTag() );
	int nChoices = setting.getNChoices();
	for (int i = 0; i < nChoices; i++)
	{
		rcSettingChoice choice = setting.getChoice( i );
        QString text( choice.getText() );
        // Add one space before text for better layout
		mWidget->insertItem( QString( " %1" )
                             .arg( text ) );
	}
	mTopLayout->addWidget( mWidget );
    mTopLayout->addSpacing( 10 );
    
	QToolTip::add( mWidget , setting.getDescription() );

	mTopLayout->addStretch();
	// update the combo box with the current value of the setting.
	settingChanged();

    connect( mWidget , SIGNAL( activated( int ) ) , this , SLOT( choiceSelected( int ) ) );

	// connect this widget to be notified by the updateSettings()
	//	signal from the model domain
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );
}

rcMenuChoiceSettingWidget::~rcMenuChoiceSettingWidget()
{
     QToolTip::remove( mWidget );
}

// The combo box widget's activated(int) signal is connected
//	to this slot, so when a menu item is selected we need to
//	set the setting with the choice value corresponding to the
//	index of menu item.
void rcMenuChoiceSettingWidget::choiceSelected( int choiceIndex )
{
	int nChoices = mSetting.getNChoices();
	if ((choiceIndex >= 0) && (choiceIndex < nChoices))
	{
		rcSettingChoice choice = mSetting.getChoice( choiceIndex );
		mSetting.setValue( choice.getValue() );
	}

	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// This slot causes us to update the currently displayed
// combo box item based on the value of the setting.
void rcMenuChoiceSettingWidget::settingChanged( void )
{
	float currentValue = mSetting.getValue();
	int nChoices = mSetting.getNChoices();
	int i = findChoice( currentValue );

	if (i < nChoices) 
		mWidget->setCurrentItem( i );

    // Update visibility/editability
    updateWidgetState();

    // Hack alert: this is brittle 
    // Send request for input source only
    if ( ! strcmp( mSetting.getTag(), "input-source" ) )
        rcModelDomain::getModelDomain()->requestInputSource( currentValue );

}

// private

int rcMenuChoiceSettingWidget::findChoice( float value )
{
    int nChoices = mSetting.getNChoices();
    int i;

    for (i = 0; i < nChoices; i++)
    {
        rcSettingChoice choice = mSetting.getChoice( i );
        if (choice.getValue() == value)
            break;
    }

    return i;
}
