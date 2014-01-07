// @file
#ifndef UI_RCSETTINGPAGE_H
#define UI_RCSETTINGPAGE_H

#include <qwidget.h>

#include <rc_setting.h>

//    void requestTrackingDisplayGL();



class rcSettingPage : public QWidget
{
    Q_OBJECT

public:
    rcSettingPage( QWidget* parent , const rcSettingCategory& category );
    rcSettingPage( QWidget* parent, QWidget* child);
    ~rcSettingPage();

public slots:

signals:

protected:

private:
	rcSettingCategory	_category;
};


#endif // UI_RCSETTINGPAGE_H
