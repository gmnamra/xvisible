
/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*   $Id: rc_thumbwheelsettingwidget.h 6411 2009-01-29 05:39:13Z arman $
*
*
******************************************************************************/

#ifndef _rcTHUMBWHEEL_H_
#define _rcTHUMBWHEEL_H_

#include "rc_settingwidget.h"
#include <qslider.h>
//Added by qt3to4:
#include <QLabel>

class QLabel;

// Note: QThumbWheel is Qt sample code, rcThumbWheelSettingWidget is Reify code

// Thumb wheel setting widget

class rcThumbWheelSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcThumbWheelSettingWidget( QWidget *parent, const rcSettingInfo& setting );
    ~rcThumbWheelSettingWidget();

public slots:
    virtual void valueChanged( int );
    void settingChanged();
    void updateRotation( float angle );
    
signals:

protected:

private:
    // Update value display label
    void updateLabel();
    
    QSlider*        mWheel;
    QLabel*         mValueLabel;
};

#endif // _rcTHUMBWHEEL_H_
