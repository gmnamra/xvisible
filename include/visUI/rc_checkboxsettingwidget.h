#ifndef UI_RCCHECKBOXSETTINGWIDGET_H
#define UI_RCCHECKBOXSETTINGWIDGET_H

#include "rc_settingwidget.h"

class QCheckBox;

class rcCheckboxSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcCheckboxSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcCheckboxSettingWidget();

public slots:
	void valueChanged( bool newValue );
	void settingChanged( void );

signals:

protected:

private:
	QCheckBox*		mWidget;
};


#endif // UI_RCCHECKBOXSETTINGWIDGET_H
