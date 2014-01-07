#ifndef UI_RCTEXTAREASETTINGWIDGET_H
#define UI_RCTEXTAREASETTINGWIDGET_H

#include "rc_settingwidget.h"
#include <rc_textedit.h>

class rcTextAreaSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcTextAreaSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcTextAreaSettingWidget();

public slots:
	void valueChanged( const QString& text );
	void settingChanged( void );

signals:

protected:

private:
	rcTextEdit*		mWidget;
};


#endif // UI_RCTEXTAREASETTINGWIDGET_H
