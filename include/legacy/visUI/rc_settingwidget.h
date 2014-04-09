/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*   $Id: rc_settingwidget.h 7179 2011-02-05 22:25:05Z arman $
*
*   Setting widget base class
*
******************************************************************************/

#ifndef _rcSETTINGWIDGET_H_
#define _rcSETTINGWIDGET_H_

#include <rc_setting.h>
#include <QtGui/QtGui>
#include <QtCore/QtCore>


// Setting widget base class
class rcSettingWidget : public QWidget
{
    Q_OBJECT

public:
    rcSettingWidget( QWidget *parent,
                     const rcSettingInfo& setting,
                     bool createTopLayout = true );
    virtual ~rcSettingWidget();

    QSize sizeHint () const { return fontMetrics().size (0, text () ); }
    
public slots:
    // Update widget visibility/editability state based on setting state
    void updateWidgetState();
    
signals:

protected:
    rcSettingInfo	mSetting;      // Setting bound to this widget
    QBoxLayout*     mTopLayout;    // Top level widget layout
    QLabel*         mDisplayLabel; // Display label
private:
    QString text () const { return QString (mSetting.getDisplayName()); }
    
    
};

#endif // _rcSETTINGWIDGET_H_

