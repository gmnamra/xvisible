#ifndef UI_RCMENUCHOICESETTINGWIDGET_H
#define UI_RCMENUCHOICESETTINGWIDGET_H

#include "rc_settingwidget.h"
#include "rc_lineedit.h"

class QComboBox;

class rcMenuChoiceSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcMenuChoiceSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcMenuChoiceSettingWidget();

public slots:
	void choiceSelected( int choiceValue );
	void settingChanged( void );

signals:

protected:

private:
    int findChoice( float value );
    
	QComboBox*		mWidget;
};


#endif // UI_RCMENUCHOICESETTINGWIDGET_H
