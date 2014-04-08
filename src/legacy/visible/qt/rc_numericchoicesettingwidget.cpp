/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*   $Id: rc_numericchoicesettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*   Generic menu choice text widget with a text line
*
******************************************************************************/

#include <qapplication.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <Q3BoxLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>

#include "rc_numericchoicesettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

const int cTextWidth = 80;

rcNumericChoiceSettingWidget::rcNumericChoiceSettingWidget( QWidget* parent, const rcSettingInfo& setting )
        : rcSettingWidget( parent, setting, false )
{
    // We create our own laytout without using base class layout
    Q3BoxLayout* vLayout = new Q3VBoxLayout( this );
	Q3BoxLayout* layout = new Q3HBoxLayout( vLayout );

	QLabel* label = new QLabel( setting.getDisplayName() , this );
	label->setFixedWidth( cUIsettingLabelWidth );

	label->setAlignment( cUIWordBreakAlignment );
	layout->addWidget( label );
	layout->addSpacing( cUIsettingLabelSpacing );

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
	layout->addWidget( mWidget );
    layout->addSpacing( 10 );
    
	QToolTip::add( mWidget , setting.getDescription() );

    // A text widget for arbitrary frame rates
    mTextWidget = new rcLineEdit( this , setting.getTag() );
    mTextWidget->setFixedWidth( cTextWidth );
    QToolTip::add(mTextWidget , setting.getDescription() );
    float value = mSetting.getValue();
    mTextWidget->setText( QString( "%1" )
                          .arg( value ) );
    layout->addWidget(mTextWidget );
    layout->addStretch();    

	// update the combo box with the current value of the setting.
	settingChanged();

    connect( mWidget , SIGNAL( activated( int ) ) , this , SLOT( choiceSelected( int ) ) );
        
    connect(mTextWidget , SIGNAL( textCommited( const QString& ) ) ,
             this    , SLOT( settingChanged( const QString& ) ) );
        
	// connect this widget to be notified by the updateSettings()
	//	signal from the model domain
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );
}

rcNumericChoiceSettingWidget::~rcNumericChoiceSettingWidget()
{
    QToolTip::remove( mWidget );
    QToolTip::remove(mTextWidget );
}

// The combo box widget's activated(int) signal is connected
//	to this slot, so when a menu item is selected we need to
//	set the setting with the choice value corresponding to the
//	index of menu item.
void rcNumericChoiceSettingWidget::choiceSelected( int choiceIndex )
{
	int nChoices = mSetting.getNChoices();
	if ((choiceIndex >= 0) && (choiceIndex < nChoices))
	{
		rcSettingChoice choice = mSetting.getChoice( choiceIndex );
		mSetting.setValue( choice.getFloatValue() );
	}

	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// This slot causes us to update the currently displayed
//	combo box item based on the value of the setting.
void rcNumericChoiceSettingWidget::settingChanged( void )
{
	float currentValue = mSetting.getValue();
	int nChoices = mSetting.getNChoices();
	int i = findChoice( currentValue );

    if ( i >= nChoices ) {
        // find "custom" value 0
        i = findChoice( 0 );
    }
	if (i < nChoices) 
		mWidget->setCurrentItem( i );

    mTextWidget->setText( QString( "%1" )
                          .arg( currentValue ) );

    // Update visibility/editability
    updateWidgetState();
}

// This slot causes us to update the currently displayed
//  combo box item based on the value of the setting.
void rcNumericChoiceSettingWidget::settingChanged( const QString& value )
{
    if ( value.length() ) {
        bool ok = false;
        float currentValue = value.toFloat( &ok );   
        
        if ( currentValue > 0 ) {
            mSetting.setValue( currentValue );
        } else {
            // Illegal value, revert to previous value
        }
    }
    // notify everybody about the change
    rcModelDomain::getModelDomain()->notifySettingChange();
}

// private

int rcNumericChoiceSettingWidget::findChoice( float value )
{
    int nChoices = mSetting.getNChoices();
    int i;

    for (i = 0; i < nChoices; i++)
    {
        rcSettingChoice choice = mSetting.getChoice( i );
        if (choice.getFloatValue() == value)
            break;
    }

    return i;
}
        
