#ifndef UI_RCRADIOCHOICESETTINGWIDGET_H
#define UI_RCRADIOCHOICESETTINGWIDGET_H

#include "rc_settingwidget.h"

class rcRadioChoiceSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcRadioChoiceSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcRadioChoiceSettingWidget();

public slots:
	void choiceSelected( int choiceValue );
	void settingChanged( void );

signals:

protected:

private:
};


#endif // UI_RCRADIOCHOICESETTINGWIDGET_H
