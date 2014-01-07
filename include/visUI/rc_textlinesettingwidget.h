#ifndef UI_RCTEXTLINESETTINGWIDGET_H
#define UI_RCTEXTLINESETTINGWIDGET_H

#include "rc_settingwidget.h"

class QLineEdit;

class rcTextLineSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcTextLineSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcTextLineSettingWidget();

public slots:
	void valueChanged( const QString& text );
	void settingChanged( void );

signals:

protected:

private:
	QLineEdit*		mWidget;
};


#endif // UI_RCTEXTLINESETTINGWIDGET_H
