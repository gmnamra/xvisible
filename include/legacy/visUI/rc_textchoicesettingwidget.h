/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*   $Id: rc_textchoicesettingwidget.h 6565 2009-01-30 03:24:44Z arman $
*
*   Generic menu choice text widget
*
******************************************************************************/

#ifndef _UI_RCTEXTCHOICESETTINGWIDGET_H_
#define _UI_RCTEXTCHOICESETTINGWIDGET_H_

#include "rc_lineedit.h"
#include "rc_settingwidget.h"

class QComboBox;

class rcTextChoiceSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcTextChoiceSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcTextChoiceSettingWidget();

public slots:
	void choiceSelected( int choiceValue );
	void settingChanged( void );
    void settingChanged( const QString& value );

signals:

protected:

private:
    // Return choice index for value
    int findChoice( const std::string& value );
    int findChoice( float value );
    
	QComboBox*		mWidget;
    rcLineEdit*	    mTextWidget;
};


#endif //  _UI_RCTEXTCHOICESETTINGWIDGET_H_
