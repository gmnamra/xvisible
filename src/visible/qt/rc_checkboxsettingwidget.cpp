/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_checkboxsettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*
******************************************************************************/

#include <qapplication.h>
#include <qcheckbox.h>
#include <qtooltip.h>

#include "rc_checkboxsettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

rcCheckboxSettingWidget::rcCheckboxSettingWidget( QWidget* parent, const rcSettingInfo& setting )
	: rcSettingWidget( parent, setting )
{
    mWidget = new QCheckBox( 0, this , setting.getTag() );
	mTopLayout->addWidget( mWidget );

	mTopLayout->addStretch();

	QToolTip::add( mWidget, setting.getDescription() );

	// update the combo box with the current value of the setting.
	settingChanged();

    connect( mWidget , SIGNAL( toggled( bool ) ) , this , SLOT( valueChanged( bool ) ) );
        
	// connect this widget to be notified by the updateSettings()
	// signal from the model domain
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );
}

rcCheckboxSettingWidget::~rcCheckboxSettingWidget()
{
    QToolTip::remove( mWidget );
}

// The checkbox widget's toggled(bool) signal is connected
//	to this slot, so when the checkbox is toggled we can
//	set the setting with the new value.
void rcCheckboxSettingWidget::valueChanged( bool newValue )
{
	bool currentValue = mSetting.getValue();
	if ( newValue != currentValue ) 
		mSetting.setValue( newValue );

	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// This slot causes us to update the currently displayed
//	checkbox state based on the value of the setting.
void rcCheckboxSettingWidget::settingChanged( void )
{
	bool currentValue = mSetting.getValue();
    if ( mWidget->isChecked() != currentValue )
        mWidget->setChecked( currentValue );

    // Update visibility/editability
    updateWidgetState();
}
