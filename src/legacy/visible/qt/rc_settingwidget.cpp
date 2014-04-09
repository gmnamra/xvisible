/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*   $Id: rc_settingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*   Setting widget base class
*
******************************************************************************/

#include "rc_settingwidget.h"
#include "rc_appconstants.h"


#include <QtGui/QtGui>
#include <QtCore/QtCore>


rcSettingWidget::rcSettingWidget( QWidget *parent,
                                  const rcSettingInfo& setting,
                                  bool createTopLayout ) :
        QWidget( parent, setting.getTag() ), mSetting( setting )
{
    if ( createTopLayout ) {
        // Top level layout
        mTopLayout = new QHBoxLayout( this );
        
        // Add label with setting display nam
	if (strlen (setting.getDisplayName()))
	  {
	    mDisplayLabel = new QLabel( setting.getDisplayName() , this );

	    mDisplayLabel->setFixedWidth( cUIsettingLabelWidth * 2);
	    mDisplayLabel->setAlignment( cUIWordBreakAlignment );
	    mTopLayout->addWidget( mDisplayLabel );
	    mTopLayout->addSpacing( cUIsettingLabelSpacing );
	  }

    } else {
        // Derived class is responsible for all layout
        mTopLayout = 0;
        mDisplayLabel = 0;
    }
}

rcSettingWidget::~rcSettingWidget()
{
}

/*!
  Update widget visibility/editability state based on setting state.
*/

void rcSettingWidget::updateWidgetState()
{
    // Disable the widget if the setting is currently not editable.
	setEnabled( mSetting.isEditable() );

	// Hide the widget if the setting is currently not enabled.
	if ( isHidden() && mSetting.isEnabled() ) {
		show();
	} else if ( !isHidden() && !mSetting.isEnabled() ) {
		hide();
	}
}
