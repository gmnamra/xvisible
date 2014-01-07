#ifndef _UI_rcSPINBOXSETTINGWIDGET_H_
#define _UI_rcSPINBOXSETTINGWIDGET_H_

#include <qspinbox.h>

#include "rc_settingwidget.h"

/* rcSpinbox - QSpinbox derived class that allows 
 */
class rcSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    rcSpinBox(double multiplier, bool valueDivisor, int minValue, int maxValue,
	      int step = 1, QWidget* parent = 0, const char* name = 0);

    void setMultiplier(double multiplier)
    { rmAssert(multiplier); _multiplier = multiplier; }

protected:
    int mapTextToValue(bool* ok);
    QString mapValueToText(int v);

private:
    
    double _multiplier;
    bool   _valueDivisor;
};

class rcSpinboxSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcSpinboxSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcSpinboxSettingWidget();

public slots:
	void valueChanged( void );
	void settingChanged( void );
	void settingChanged( double multiplier );
    
protected:

private:
    
	QSpinBox*	_spinboxWidget;
	rcSpinBox*      _rcSpinboxWidget;
};

#endif // _UI_rcSPINBOXSETTINGWIDGET_H_
