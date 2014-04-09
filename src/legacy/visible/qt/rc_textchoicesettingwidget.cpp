/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*   $Id: rc_textchoicesettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*   Generic menu choice text widget with a text line
*
******************************************************************************/


#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include "rc_textchoicesettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

#define QStrFrcStr(foo) QString ((foo).c_str())

const int cTextWidth = 150;

rcTextChoiceSettingWidget::rcTextChoiceSettingWidget( QWidget* parent, const rcSettingInfo& setting )
        : rcSettingWidget( parent, setting, false )
{
    // We create our own laytout without using base class layout
    QBoxLayout* vLayout = new QVBoxLayout( this );
	QBoxLayout* layout = new QHBoxLayout( vLayout );

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
    layout->setStretchFactor( mWidget, 0 );

    layout->addSpacing( 10 );

	QToolTip::add( mWidget , setting.getDescription() );

    // A text widget for arbitrary frame rates
    mTextWidget = new rcLineEdit( this , setting.getTag() );
    mTextWidget->setFixedWidth( cTextWidth );
    QToolTip::add(mTextWidget , setting.getDescription() );
    std::string value = mSetting.getValue();
    layout->addWidget(mTextWidget );
    layout->setStretchFactor( mTextWidget, 100 );
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

rcTextChoiceSettingWidget::~rcTextChoiceSettingWidget()
{
    QToolTip::remove( mWidget );
    QToolTip::remove(mTextWidget );
}

// The combo box widget's activated(int) signal is connected
//	to this slot, so when a menu item is selected we need to
//	set the setting with the choice value corresponding to the
//	index of menu item.
void rcTextChoiceSettingWidget::choiceSelected( int choiceIndex )
{
	int nChoices = mSetting.getNChoices();
	if ((choiceIndex >= 0) && (choiceIndex < nChoices))
	{
		rcSettingChoice choice = mSetting.getChoice( choiceIndex );
		mSetting.setValue( choice.getText() );
	}

	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// This slot causes us to update the currently displayed
//	combo box item based on the value of the setting.
void rcTextChoiceSettingWidget::settingChanged( void )
{
	std::string currentValue = mSetting.getValue();
	int nChoices = mSetting.getNChoices();
	int i = findChoice( currentValue );

    if ( i >= nChoices ) {
        // find "other" value
        i = findChoice( 0.0 );
    }

    // Final fallback, use last value
    if ( i >= nChoices )
        i = nChoices-1;

    mWidget->setCurrentItem( i );
    rcSettingChoice choice = mSetting.getChoice( i );
    if ( choice.getValue() > 0 )
        mTextWidget->setText( choice.getText() );
    else
        mTextWidget->setText( QString( "%1" )
                              .arg( QStrFrcStr (currentValue ) ));

    // Update visibility/editability
    updateWidgetState();
}

// This slot causes us to update the currently displayed
//  combo box item based on the value of the setting.
void rcTextChoiceSettingWidget::settingChanged( const QString& value )
{
    std::string currentValue = value.latin1();
    if ( value.length() )
        mSetting.setValue( currentValue );

    // notify everybody about the change
    rcModelDomain::getModelDomain()->notifySettingChange();
}

// private

int rcTextChoiceSettingWidget::findChoice( const std::string& value )
{
    int nChoices = mSetting.getNChoices();
    int i = 0;

    if ( value.size() ) {
        for (i = 0; i < nChoices; i++) {
            rcSettingChoice choice = mSetting.getChoice( i );
            if ( !strcasecmp(choice.getText(), value.c_str() ) )
                break;
        }
    }

    return i;
}

int rcTextChoiceSettingWidget::findChoice( float value )
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
