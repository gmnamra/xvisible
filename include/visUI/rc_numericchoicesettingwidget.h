/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*   $Id: rc_numericchoicesettingwidget.h 7179 2011-02-05 22:25:05Z arman $
*
*   Generic menu choice text widget
*
******************************************************************************/

#ifndef _UI_RCNUMCHOICESETTINGWIDGET_H_
#define _UI_RCNUMCHOICESETTINGWIDGET_H_

#include "rc_lineedit.h"
#include "rc_settingwidget.h"

class QComboBox;

class rcNumericChoiceSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcNumericChoiceSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcNumericChoiceSettingWidget();

public slots:
	void choiceSelected( int choiceValue );
	void settingChanged( void );
    void settingChanged( const QString& value );

signals:

protected:

private:
    // Return choice index for value
    int findChoice( float value );

	QComboBox*		mWidget;
    rcLineEdit*	    mTextWidget;
};


#endif //  _UI_RCNUMCHOICESETTINGWIDGET_H_
