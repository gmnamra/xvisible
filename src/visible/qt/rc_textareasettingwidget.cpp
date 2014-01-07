/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_textareasettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*
******************************************************************************/

#include <qapplication.h>
#include <q3textedit.h>
#include <qtooltip.h>

#include "rc_textedit.h"
#include "rc_textareasettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

rcTextAreaSettingWidget::rcTextAreaSettingWidget( QWidget* parent, const rcSettingInfo& setting )
	: rcSettingWidget( parent, setting )
{
    // Default text area height
    int lines = 5;
    
    const rcTextAreaArgs* argsP = static_cast<const rcTextAreaArgs*>(setting.getXArgs());
	if ( argsP ) {
        lines = argsP->minLines;
    }
	mWidget = new rcTextEdit( this, lines, setting.getTag() );
    mTopLayout->addWidget( mWidget );

    // Reduce default point size
    mWidget->zoomOut(1);
    
	QToolTip::add( mWidget , setting.getDescription() );

	connect( mWidget , SIGNAL( textCommited( const QString& ) ) ,
			 this    , SLOT( valueChanged( const QString& ) ) );

	// update the text widget with the current value of the setting.
	settingChanged();

	// connect this widget to be notified by the updateSettings()
	//	signal from the model domain
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );
}

rcTextAreaSettingWidget::~rcTextAreaSettingWidget()
{
    QToolTip::remove( mWidget );
}

// Update the setting value with the string just entered in
//	the QTextEdit widget.
void rcTextAreaSettingWidget::valueChanged( const QString& text )
{
	mSetting.setValue( text.latin1() );

	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// Update the text shown in the QTextEdit widget with the
//	current value in the setting.
void rcTextAreaSettingWidget::settingChanged( void )
{
	string currentValue = mSetting.getValue();
	mWidget->setText( currentValue.c_str() );

    // Update visibility/editability
    updateWidgetState();
}
