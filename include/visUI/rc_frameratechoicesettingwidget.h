#ifndef UI_RCFRAMERATECHOICESETTINGWIDGET_H
#define UI_RCFRAMERATECHOICESETTINGWIDGET_H

#include "rc_lineedit.h"
#include "rc_settingwidget.h"

class QComboBox;

class rcFramerateChoiceSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcFramerateChoiceSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcFramerateChoiceSettingWidget();

public slots:
	void choiceSelected( int choiceValue );
	void settingChanged( void );
    void settingChanged( const QString& value );
    void intervalChanged( const QString& value );

signals:

protected:

private:
    // Return choice index for value
    int findChoice( float value );

	QComboBox*		_widget;
    rcLineEdit*		_framerateWidget;
    rcLineEdit*		_intervalWidget;
};

class rcRateChoiceSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcRateChoiceSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcRateChoiceSettingWidget();

public slots:
	void choiceSelected( int choiceValue );
	void settingChanged( void );
    void settingChanged( const QString& value );

signals:

protected:

private:
    // Return choice index for value
    int findChoice( float value );

	QComboBox*		_widget;
    rcLineEdit*		_rateWidget;
};


#endif // UI_RCFRAMERATECHOICESETTINGWIDGET_H
