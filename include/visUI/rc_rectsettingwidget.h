#ifndef UI_RCRECTSETTINGWIDGET_H
#define UI_RCRECTSETTINGWIDGET_H

#include "rc_settingwidget.h"

class QString;
class QSpinBox;
class QPushButton;

class rcRectSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcRectSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcRectSettingWidget();

public slots:
	void valueChanged( void );
	void settingChanged( void );
    void settingChanged( const rcRect& );
    void selectAll();
    
protected:

private:
    void setMaxValues();
    
	QSpinBox*		_xWidget;
	QSpinBox*		_yWidget;
	QSpinBox*		_wWidget;
	QSpinBox*		_hWidget;
    QPushButton*    _selectWidget;
    bool            _ignoreValueChanged;
};


#endif // UI_RCRECTSETTINGWIDGET_H
