#ifndef rcTIME_DISPLAY_H
#define rcTIME_DISPLAY_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>
#include <QT3Support/Q3Frame>
#include <rc_timestamp.h>

class rcTimeDisplay : public Q3Frame
{
    Q_OBJECT

public:
    rcTimeDisplay( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~rcTimeDisplay();

public slots:
	void newTime( const rcTimestamp& time );

protected:

private:
	rcTimestamp	_time;
};


#endif // rcTIME_DISPLAY_H
