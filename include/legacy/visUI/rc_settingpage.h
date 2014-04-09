#ifndef UI_RCSETTINGPAGE_H
#define UI_RCSETTINGPAGE_H


#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include <rc_setting.h>

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
