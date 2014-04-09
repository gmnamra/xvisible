/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_textlinesettingwidget.cpp 6489 2009-01-29 05:39:24Z arman $
*
*
******************************************************************************/

#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include "rc_lineedit.h"
#include "rc_textlinesettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

rcTextLineSettingWidget::rcTextLineSettingWidget( QWidget* parent, const rcSettingInfo& setting )
	: rcSettingWidget( parent, setting )
{
	mWidget = new rcLineEdit( this , setting.getTag() );
	mTopLayout->addWidget( mWidget );

	QToolTip::add( mWidget , setting.getDescription() );

    // update the text widget with the current value of the setting.
	settingChanged();
    
	connect( mWidget , SIGNAL( textCommited( const QString& ) ) ,
			 this    , SLOT( valueChanged( const QString& ) ) );

	// connect this widget to be notified by the updateSettings()
	//	signal from the model domain
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );
}

rcTextLineSettingWidget::~rcTextLineSettingWidget()
{
    QToolTip::remove( mWidget );
}

// Update the setting value with the string just entered in
//	the QLineEdit widget.
void rcTextLineSettingWidget::valueChanged( const QString& text )
{
	mSetting.setValue( text.latin1() );

	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// Update the text shown in the QLineEdit widget with the
//	current value in the setting.
void rcTextLineSettingWidget::settingChanged( void )
{
	string currentValue = mSetting.getValue();
	mWidget->setText( currentValue.c_str() );

    // Update visibility/editability
    updateWidgetState();
}
