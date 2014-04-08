/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 ******************************************************************************/

#ifndef UI_RCPLAYBACKSETTINGWIDGET_H
#define UI_RCPLAYBACKSETTINGWIDGET_H

#include "rc_settingwidget.h"

class QSpinBox;
class QPushButton;

class rcPlaybackSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcPlaybackSettingWidget(QWidget* parent, const rcSettingInfo& setting);
    ~rcPlaybackSettingWidget();

public slots:
    void settingChanged();
    void valueChanged();
    void stopVideo();
    void fwdVideo();
    void revVideo();
    
protected:

private:
    
    QSpinBox*     _speedWidget;
    QPushButton*  _revWidget;
    QPushButton*  _fwdWidget;
    QPushButton*  _stopWidget;
    int           _currentValueBits;
};


#endif // UI_RCPLAYBACKSETTINGWIDGET_H
